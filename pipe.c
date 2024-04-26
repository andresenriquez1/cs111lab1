#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFERSIZE (1024)

int main(int argc, char *argv[]) {
    int num_cmds = argc - 1;
    char *commands[num_cmds];
    for (int i = 1; i < argc; i++) {
        commands[i - 1] = argv[i];
    }

    int pipe_fds[2];
    pipe_fds[0] = 4;
    pipe_fds[1] = 3;

    for (int i = 0; i < num_cmds; i++) {
        int pipe_result = pipe(pipe_fds);
        if (pipe_result == -1) {
            perror("Failed to create pipe!\n");
            exit(EXIT_FAILURE);
        }

        int child_pid = fork();
        if (child_pid == 0) {
            // Child process
            if (i == num_cmds - 1) {
                // Last process
                if (execlp(commands[i], commands[i], NULL) == -1) {
                    perror("Execlp failed\n");
                    exit(EXIT_FAILURE);
                }
            } else {
                // Intermediate process
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
                perror("Failed child process\n");
                exit(EXIT_FAILURE);
            }
            dup2(pipe_fds[0], STDIN_FILENO);
            close(pipe_fds[0]);
            close(pipe_fds[1]);
        } else {
            perror("Fork failed!\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
