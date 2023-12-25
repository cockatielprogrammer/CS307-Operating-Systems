//Ulas Meric 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int pipe_fd[2];
    pid_t pid1, pid2;

    if (pipe(pipe_fd) < 0) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    pid1 = fork();

    if (pid1 < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid1 == 0) { // Child process 1 (man ls)
        printf("I'm SHELL process, with PID: %d - execution is completed, you can find the results in output.txt\n", getpid);
         printf("I'm MAN process, with PID: %d - My command is: %s\n", getpid(), "ls");


        close(pipe_fd[0]); 
        dup2(pipe_fd[1], STDOUT_FILENO); 
        close(pipe_fd[1]);
         
        
        execlp("man", "man", "ls", (char *)NULL);
         
        
        perror("exec for man ls failed");
        exit(EXIT_FAILURE);
    }

    
    pid2 = fork();

    if (pid2 < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid2 == 0) { // Child process 2 (grep file)   --> 2 child processes to provide concurrent actions 
        
        printf("I'm GREP process, with PID: %d - My command is: %s\n", getpid(), "file");
        close(pipe_fd[1]);
        
        dup2(pipe_fd[0], STDIN_FILENO);
        close(pipe_fd[0]);

        
        freopen("output.txt", "w", stdout);

        
        
        //execlp("grep", "grep", "file", (char *)NULL);
        execlp("grep", "grep", "-w", "file", (char *)NULL);

        
        //execlp("grep", "grep", "-F \"file\"", (char *)NULL);

        

        
        perror("exec for grep file failed");
        exit(EXIT_FAILURE);
    }

    
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    // parent process
   

    
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    printf("I'm SHELL process, with PID: %d - execution is completed, you can find the results in output.txt\n", getpid);

    return 0;
}
