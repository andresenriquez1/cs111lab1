#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    
    char *commands[argc - 1];
    for (int i = 1; i < argc; i++) {
        commands[i - 1] = argv[i]; //put the given user input in commands easier to track 
    }

    int pipe_fds[2];
    pipe_fds[0] = 1;
    pipe_fds[1] = 2;

    for (int i = 0; i < argc - 1; i++) {
        int result_fromPipe = pipe(pipe_fds);
        if (result_fromPipe == -1) {
            perror("Failed pipe creation\n");
            exit(EXIT_FAILURE);
        }

        int child_pid = fork(); //tells us whos running, we can alsop decide via wait command
        if (child_pid == 0) {
            // Child process
            if (i == argc - 2) {
                // Last process
                if (execlp(commands[i], commands[i], NULL) == -1) {
                    perror("Execlp failed\n");
                    exit(EXIT_FAILURE);
                }
            } else {
                // middle process between parent and child
                dup2(pipe_fds[1], STDOUT_FILENO);
                close(pipe_fds[0]);
                close(pipe_fds[1]);
                if (execlp(commands[i], commands[i], NULL) == -1) {
                    perror("Execlp failed\n");
                    exit(EXIT_FAILURE);
                }
            }
        } else if (child_pid > 0) {
            // Parent process
            int status;
            waitpid(child_pid, &status, 0);
            if (WEXITSTATUS(status) != 0) {
                perror("Failed the child process\n");
                exit(EXIT_FAILURE);
            }
            dup2(pipe_fds[0], STDIN_FILENO);
            close(pipe_fds[0]);
            close(pipe_fds[1]);
        } else {
            perror("Fork failed\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
