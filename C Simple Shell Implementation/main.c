#include "parser.h"
#include <sys/wait.h>
#include <signal.h>

void printcmd(struct cmd *cmd)
{
    struct backcmd *bcmd = NULL;
    struct execcmd *ecmd = NULL;
    struct listcmd *lcmd = NULL;
    struct pipecmd *pcmd = NULL;
    struct redircmd *rcmd = NULL;

    int i = 0;
    
    if(cmd == NULL)
    {
        PANIC("NULL addr!");
        return;
    }
    

    switch(cmd->type){
        case EXEC:
            ecmd = (struct execcmd*)cmd;
            if(ecmd->argv[0] == 0)
            {
                goto printcmd_exit;
            }

            MSG("COMMAND: %s", ecmd->argv[0]);
            for (i = 1; i < MAXARGS; i++)
            {            
                if (ecmd->argv[i] != NULL)
                {
                    MSG(", arg-%d: %s", i, ecmd->argv[i]);
                }
            }
            MSG("\n");

            break;

        case REDIR:
            rcmd = (struct redircmd*)cmd;

            printcmd(rcmd->cmd);

            if (0 == rcmd->fd_to_close)
            {
                MSG("... input of the above command will be redirected from file \"%s\". \n", rcmd->file);
            }
            else if (1 == rcmd->fd_to_close)
            {
                MSG("... output of the above command will be redirected to file \"%s\". \n", rcmd->file);
            }
            else
            {
                PANIC("");
            }

            break;

        case LIST:
            lcmd = (struct listcmd*)cmd;

            printcmd(lcmd->left);
            MSG("\n\n");
            printcmd(lcmd->right);
            
            break;

        case PIPE:
            pcmd = (struct pipecmd*)cmd;

            printcmd(pcmd->left);
            MSG("... output of the above command will be redrecited to serve as the input of the following command ...\n");            
            printcmd(pcmd->right);

            break;

        case BACK:
            bcmd = (struct backcmd*)cmd;

            printcmd(bcmd->cmd);
            MSG("... the above command will be executed in background. \n");    

            break;


        default:
            PANIC("");
    
    }
    
    printcmd_exit:

    return;
}

int sig_handler_pid = 0;

void sigint_handler(int signum) {
	if(sig_handler_pid) {
		kill(sig_handler_pid, SIGKILL);
	} else {
		printf("\nCtrl-C catched. But currently there is no foreground process running.\n%s", SHELL_PROMPT);
	}
}


int runcmd(struct cmd * command) {
	struct execcmd * exec_command = NULL;
	struct listcmd * list_command = NULL;
	struct pipecmd * pipe_command = NULL;
	struct redircmd * redir_command = NULL;
	struct backcmd * back_command = NULL;
	int fd[2];
	int file;
	int saved_fd;
	int child_status;
	int pid = 0;
	switch(command->type) {
		case EXEC:
			exec_command = (struct execcmd *)command;
			if(exec_command->argv[0] == NULL)
				return 0;
			if(!(pid = fork())) {
				execvp(exec_command->argv[0], exec_command->argv);
			} else {
				sig_handler_pid = pid;
				waitpid(pid, &child_status, 0);
				sig_handler_pid = 0;
				if(child_status) {
					if(!WIFSIGNALED(child_status))
						printf("\nNon-zero exit code (%u) detected\n", WEXITSTATUS(child_status));
				}
			}
			break;
		case LIST:
			list_command = (struct listcmd *)command;
			runcmd(list_command->left);
			runcmd(list_command->right);
			break;
		case PIPE:
			pipe_command = (struct pipecmd *)command;
			pipe(fd);
			if(!fork()) {
				dup2(fd[1], STDOUT_FILENO);
				close(fd[0]);
				close(fd[1]);
				runcmd(pipe_command->left);
				exit(0);
			}
			if(!fork()) {
				dup2(fd[0], STDIN_FILENO);
				close(fd[0]);
				close(fd[1]);
				runcmd(pipe_command->right);
				exit(0);
			}
			close(fd[0]);
			close(fd[1]);
			wait(NULL);
			wait(NULL);
			break;
		case REDIR:
			redir_command = (struct redircmd *)command;
			if(redir_command->fd_to_close == 1) {
				file = open(redir_command->file, O_CREAT|O_RDWR|O_TRUNC, S_IRUSR | S_IWUSR);
				saved_fd = dup(STDOUT_FILENO);
				dup2(file, STDOUT_FILENO);
			} else {
				file = open(redir_command->file, O_RDONLY, S_IRUSR | S_IWUSR);
				saved_fd = dup(STDIN_FILENO);
				dup2(file, STDIN_FILENO);
			}
			close(file);
			runcmd(redir_command->cmd);
			if(redir_command->fd_to_close) {
				dup2(saved_fd, STDOUT_FILENO);
			} else {
				dup2(saved_fd, STDIN_FILENO);
			}
			close(saved_fd);
			break;
		case BACK:
			back_command = (struct backcmd *)command;
			exec_command = (struct execcmd *)back_command->cmd;
			if(!(pid = fork())) {
				execvp(exec_command->argv[0], exec_command->argv);
			} else {
			return pid;
			}
			break;
	}
	return 0;
}

int main(void)
{
    static char buf[1024];
    int fd;

    setbuf(stdout, NULL);
	
	signal(SIGINT, sigint_handler);
	int orphan = 0;
    // Read and run input commands.
    while(getcmd(buf, sizeof(buf)) >= 0)
    {
		struct cmd * command;
        command = parsecmd(buf);
		if(command->type == BACK) {
			orphan = runcmd(command);
		} else {
			runcmd(command);
			if(orphan) {
				waitpid(orphan, NULL, 0);
				orphan = 0;
			}
		}
        // printcmd(command); // TODO: run the parsed command instead of printing it
    }

    PANIC("getcmd error!\n");
    return 0;
}
