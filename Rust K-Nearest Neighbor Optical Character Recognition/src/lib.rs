use std::fs;

//this may result in non-deterministic behavior
//use std::collections::HashMap;

//use this for deterministic behavior
//use hash_hasher::HashedMap;

type Feature = u8;
type Label = u8;
type Index = usize;

pub struct LabeledFeatures {
    ///feature set
    pub features: Vec<Feature>,

    ///classification of feature set
    pub label: Label,
}

///magic number used at start of MNIST data file
const DATA_MAGIC: u32 = 0x803;

///magic number used at start of MNIST label file
const LABEL_MAGIC: u32 = 0x801;

fn first_4_bytes(data: &[u8])
	-> u32
{
	((data[0] as u32) << 24) | ((data[1] as u32) << 16) | ((data[2] as u32) << 8) | (data[3] as u32)
}

///return labeled-features with features read from data_dir/data_file_name
///and labels read from data_dir/label_file_name
pub fn read_labeled_data(data_dir: &str,
			 data_file_name: &str, label_file_name: &str)
			 -> Vec<LabeledFeatures>
{
    // following line will be replaced during your implementation
    let data_path: String = format!("{}/{}", data_dir, data_file_name);
    let label_path: String = format!("{}/{}", data_dir, label_file_name);
    let data: Vec<u8> = fs::read(&data_path).expect(&format!("failed to read data file at path: {}", data_path));
    let labels: Vec<u8> = fs::read(&label_path).expect(&format!("failed to read label file at path: {}", label_path));
    assert_eq!(first_4_bytes(&data), DATA_MAGIC, "expected first four bytes of data data to be {} but found {}", DATA_MAGIC, first_4_bytes(&data));
    assert_eq!(first_4_bytes(&labels), LABEL_MAGIC, "expected first four bytes of label file to be {} but found {}", LABEL_MAGIC, first_4_bytes(&labels));
    let n: usize = first_4_bytes(&data[4..]) as usize;
    assert_eq!(n, first_4_bytes(&labels[4..]) as usize, "number of images is {} but number of labels is {}", n, first_4_bytes(&labels[4..]) as usize);
    let n_rows: usize = first_4_bytes(&data[8..]) as usize;
    let n_cols: usize = first_4_bytes(&data[12..]) as usize;
    assert_eq!(n_rows, n_cols, "number of rows is {} but number of columns is {}", n_rows, n_cols);
    let mut results: Vec<LabeledFeatures> = Vec::with_capacity(n);
    for i in 0..n {
    	let mut image: Vec<Feature> = Vec::with_capacity(n_rows * n_cols);
    	image.extend_from_slice(&data[16 + i * (n_rows * n_cols) .. 16 + (i + 1) * n_rows * n_cols]);
    	let res: LabeledFeatures = LabeledFeatures {features: image, label: labels[8 + i]};
    	results.push(res);
    }
    results
}

fn distance(i1: &[Feature], i2: &[Feature])
	-> u64
{
	let mut distance: u64 = 0;
	for i in 0 .. i1.len() {
		distance += (i1[i] as i32 - i2[i] as i32).pow(2) as u64;
	}
	distance
}

fn mode(slice: &[Distance])
	-> Index
{
	let mut table = [0u32; 10];
	for d in slice {
		table[d.label as usize] += 1;
	}
	let max = table.into_iter().max().unwrap();
	let index: u8 = table.into_iter().position(|x| x == max).unwrap() as u8;
	slice.iter().find(|&d| d.label == index).unwrap().index
}

#[derive(PartialEq, Eq, PartialOrd, Ord)]
struct Distance {
	distance: u64,
	label: u8,
	index: usize
}

///Return the index of an image in training_set which is among the k
///nearest neighbors of test and has the same label as the most
///common label among the k nearest neigbors of test.
pub fn knn(training_set: &Vec<LabeledFeatures>, test: &Vec<Feature>, k: usize)
       -> Index
{
	let mut distances: Vec<Distance> = Vec::with_capacity(training_set.len());
	for i in 0 .. training_set.len() {
		let d: Distance = Distance {distance: distance(&training_set[i].features, &test), label: training_set[i].label, index: i};
		distances.push(d);
	}
	distances.sort();
	let first_k: &[Distance] = &distances[..k];
	mode(&first_k)
}

