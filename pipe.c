#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {

    if((argc -1) <= 1 ) //only got one command cant do anything need at least 2
    {
        perror("Need at minimum 2 arguments to run\n");
        exit(EXIT_FAILURE);

    }
    char *commands [argc-1];

    for (int i = 1; i < argc; i++) {
        commands[i - 1] = argv[i];
    }

    int pipe_fds[2];
    pipe_fds[0] = 4;
    pipe_fds[1] = 3;

    for (int i = 0; i < argc-1; i++) {
        int piped_results = pipe(pipe_fds);
        if (piped_results == -1) {
            perror("Pipe Failed, could not create Pipe\n");
            exit(EXIT_FAILURE);
        }

        int child_pid = fork(); //tells us who is running.
        if (child_pid == 0) { // we have a child process
       
            if (i == argc-1) {
                // Last process
                if (execlp(commands[i], commands[i], NULL) == -1) {
                    perror("Execlp command failed\n");
                    exit(EXIT_FAILURE);
                }
            } else {
                // Intermediate process
                dup2(pipe_fds[1], STDOUT_FILENO);
                close(pipe_fds[0]);
                close(pipe_fds[1]);
                if (execlp(commands[i], commands[i], NULL) == -1) {
                    perror("Execlp command failed\n");
                    exit(EXIT_FAILURE);
                }
            }
        } else if (child_pid >= 1) {
            // The Parent process
            int status;
            waitpid(child_pid, &status, 0);
            if (WEXITSTATUS(status) != 0) {
                perror("Child proces Failed\n");
                exit(EXIT_FAILURE);
            }
            dup2(pipe_fds[0], STDIN_FILENO);
            close(pipe_fds[0]); //need tp close both to ensure memory isnt leaked, best practice rn even when small program
            close(pipe_fds[1]);
        } else {
            perror("Fork command failed\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
