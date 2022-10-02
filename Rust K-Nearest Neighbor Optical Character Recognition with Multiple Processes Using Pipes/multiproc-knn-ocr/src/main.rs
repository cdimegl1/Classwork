use std::path::Path;
use std::env;
use std::error::Error;
use nix::{unistd::*, fcntl::*, sys::wait::wait};
use std::os::unix::io::RawFd;

use knn_ocr::{read_labeled_data, knn, LabeledFeatures};

const TEST_DATA: &str = "t10k-images-idx3-ubyte";
const TEST_LABELS: &str  = "t10k-labels-idx1-ubyte";
const TRAINING_DATA: &str  = "train-images-idx3-ubyte";
const TRAINING_LABELS: &str  = "train-labels-idx1-ubyte";

fn usage(prog_path: &str, err: &str) -> ! {
    let prog_name =
	Path::new(prog_path).file_name().unwrap().to_str().unwrap();
    eprintln!("{}\nusage: {} DATA_DIR [N_TESTS [K [N_PROC]]]", err, prog_name);
    std::process::exit(1);
}

struct Args {
    data_dir: String,
    k: usize,
    n_test: i32,
    n_proc: usize,
}

impl Args {
    fn new(argv: &Vec<String>) -> Result<Args, Box<dyn Error>> {
	let argc = argv.len();
	if argc < 2 || argc > 5 {
	    Err("must be called with 1 ... 4 args")?;
	}
	let data_dir = argv[1].clone();
	let mut n_test: i32 = -1;
	let mut k: usize = 3;
    let mut n_proc: usize = 4;
	if argc > 2 {
	    n_test = argv[2].parse::<i32>()?;
	}
	if argc == 4 {
	    k = argv[3].parse::<usize>()?;
	    if k == 0 {
		Err("k must be positive")?;
	    }
	}
    if argc == 5 {
        n_proc = argv[4].parse::<usize>()?;
        if n_proc == 0 {
            Err("n_proc must be positive")?;
        }
    }
	Ok(Args { data_dir, k, n_test, n_proc })
    }
    
}

struct IndexedFeatures {
	index: usize,
	features: Vec<u8>,
}

struct WorkerProcess {
	index: usize,
	in_pipe: (RawFd, RawFd),
}

fn usize_to_bytes(u: usize) -> Vec<u8> {
	let mut result: Vec<u8> = Vec::with_capacity(std::mem::size_of::<usize>());
	for i in 0..(std::mem::size_of::<usize>()) {
		result.push((255 & (u >> (8 * i))) as u8);	
	}
	result.reverse();
	result
}

fn read_usize(slice: &[u8]) -> usize {
	let mut result: usize = 0;
	for i in 0..(std::mem::size_of::<usize>()) {
		result |= (slice[i] as usize) << (std::mem::size_of::<usize>() * 8 - (8 * (i + 1)));
	}
	result
}

fn main() {
    let argv: Vec<String> = env::args().collect();
    let args;
    match Args::new(&argv) {
	Err(err) => usage(&argv[0], &err.to_string()),
	Ok(a) => args = a,
    };
    let train_data =
	read_labeled_data(&args.data_dir, TRAINING_DATA, TRAINING_LABELS);
	let out_pipe: (RawFd, RawFd) = pipe().expect("pipe failed");
	let mut processes: Vec<WorkerProcess> = Vec::with_capacity(args.n_proc);	
	for i in 0..args.n_proc {
		processes.push(WorkerProcess { index: i, in_pipe: pipe().expect("pipe failed") });
	}
	for j in 0..args.n_proc {
		match unsafe { fork() } {
			Ok(ForkResult::Child) => {
				close(out_pipe.0).expect("close from child failed");
				for proc in &processes {
					if proc.index != j {
						close(proc.in_pipe.1).expect("close from child failed");
						close(proc.in_pipe.0).expect("close from child failed");
					} else {
						close(proc.in_pipe.1).expect("close from child failed");
					}
				}
				println!("Worker #{} PID: {}", j + 1, getpid());
				let mut data: Vec<IndexedFeatures> = Vec::new();
				let mut bytes: Vec<u8> = Vec::new();
				let mut buf: Vec<u8> = Vec::with_capacity(fcntl(processes[j].in_pipe.0, FcntlArg::F_GETPIPE_SZ).unwrap() as usize);	
				unsafe { buf.set_len(fcntl(processes[j].in_pipe.0, FcntlArg::F_GETPIPE_SZ).unwrap() as usize); }
				loop {
					let read_res: usize = read(processes[j].in_pipe.0, &mut buf).expect("read from child failed");
					if read_res == 0 {
						break;
					}
					bytes.extend_from_slice(&buf[..read_res]);
				}
				close(processes[j].in_pipe.0).expect("close from child failed");
				for k in 0..(bytes.len() / (784 + std::mem::size_of::<usize>())) {
					data.push( IndexedFeatures { index: read_usize(&bytes[k * (784 + std::mem::size_of::<usize>())..]), features: bytes[k * (784 + std::mem::size_of::<usize>()) + std::mem::size_of::<usize>()..(k + 1) * (784 + std::mem::size_of::<usize>())].to_vec() });
				}
				for feature in &data {
					let mut results: Vec<u8> = Vec::new();
					results.extend_from_slice(&usize_to_bytes(feature.index));
					results.extend_from_slice(&usize_to_bytes(knn(&train_data, &feature.features, args.k)));
					write(out_pipe.1, &results).expect("write from child failed");
				}
				close(out_pipe.1).expect("close from child failed");
				std::process::exit(0);
			}
			Ok(ForkResult::Parent {child: _}) => {}
			Err(_) => {
				println!("fork failed");
				std::process::exit(1);
			}
		}
	}

	close(out_pipe.1).expect("close from parent failed");
	for proc in &processes {
		close(proc.in_pipe.0).expect("close from parent failed");
	}

	let test_data = read_labeled_data(&args.data_dir, TEST_DATA, TEST_LABELS);
	let n: usize = if args.n_test <= 0 {
			test_data.len()
		} else {
			args.n_test as usize
		};
	let mut test_index: usize = 0;
	let classifications_per_process: usize = n / args.n_proc;
	let classifications_remainder: usize = n % args.n_proc;
	for i in 0..classifications_remainder {
		let mut features_to_write: Vec<u8> = Vec::with_capacity((784 + std::mem::size_of::<usize>()) * (classifications_per_process + 1));
		for _ in 0..(classifications_per_process + 1) {
			let sample: &LabeledFeatures = &test_data[test_index];	
			features_to_write.extend_from_slice(&usize_to_bytes(test_index));
			features_to_write.extend_from_slice(&sample.features);
			test_index += 1;
		}
		write(processes[i].in_pipe.1, &features_to_write).expect("write from parent failed");
		close(processes[i].in_pipe.1).expect("close from parent failed");
	}
	for i in classifications_remainder..args.n_proc {
		let mut features_to_write: Vec<u8> = Vec::with_capacity((784 + std::mem::size_of::<usize>()) * classifications_per_process);
		for _ in 0..classifications_per_process {
			let sample: &LabeledFeatures = &test_data[test_index];
			features_to_write.extend_from_slice(&usize_to_bytes(test_index));
			features_to_write.extend_from_slice(&sample.features);
			test_index += 1;
		}
		write(processes[i].in_pipe.1, &features_to_write).expect("write from parent failed");
		close(processes[i].in_pipe.1).expect("close from parent failed");
	}

	let mut buf: [u8; std::mem::size_of::<usize>() * 2] = [0; std::mem::size_of::<usize>() * 2];
	let mut ok: usize = 0;

	loop {
		let read_res: usize = read(out_pipe.0, &mut buf).expect("read from parent failed");	
		if read_res == 0 {
			break;
		}
		let i = read_usize(&buf);
		let nearest_index = read_usize(&buf[std::mem::size_of::<usize>()..]);
		let expected = test_data[i].label;
		let predicted = train_data[nearest_index].label;
		if predicted == expected {
			ok += 1;
		}
		else {
			let digits = b"0123456789";
			println!("{}[{}] {}[{}]",
		     char::from(digits[predicted as usize]), nearest_index,
		     char::from(digits[expected as usize]), i);
		}
	}
	close(out_pipe.0).expect("close from parent failed");
	println!("{}% success", (ok as f64)/(n as f64)*100.0);

	for _ in 0..args.n_proc {
		wait().expect("wait from parent failed");
	}
}

