/**
 * CPE 453 Spring 2012
 * Lab 01
 * https://users.csc.calpoly.edu/~pnico/class/s12/cpe453/lab/lab01/lab01.pdf
 * 
 * Run the pipeline `ls | sort -r > outfile`
 *
 * @author James Pearson
 */

#include "pipeit.h"

/** 
 * The main thing.
 * @param argc the number of tokens on the input line.
 * @param argv an array of tokens.
 * @return 0 on success, 1-255 on failure
 */
int main(int argc, char *argv[]) {
    int pid[2];
    int pipeFile[2];
    int status[2];
    
    pipe(pipeFile);
    
    /* Fork and exec our first child, for ls.*/
    if ((pid[0] = fork()) == 0) {
        /* This used to be in a function, but vogon was really unhappy with it;
         * 'ls: cannot access ?|:' */
        /*printf("Child 1\n");*/
        
        if (dup2(pipeFile[1], STDOUT_FILENO) == -1) {
            return EXIT_FAILURE;
        }
        close(pipeFile[0]);
        close(pipeFile[1]);
        return execlp("ls", "ls");
    } else if (pid[0] == -1) {
        return EXIT_FAILURE;
    }
    /* And the second one, sort.*/
    if ((pid[1] = fork()) == 0) {
        return doSortChild(pipeFile);
    } else if (pid[1] == -1) {
        return EXIT_FAILURE;
    }
    
    /*printf("Parent\n");*/
    /* We aren't using these, and the sort-child needs 'em closed to know it's
     * done. */
    close(pipeFile[0]);
    close(pipeFile[1]);
    
    while (waitpid(pid[0], &status[0], 0) == -1) {};
    /*printf("Exit status of %d was %d\n", pid[0], WEXITSTATUS(status[0]));*/
    while (waitpid(pid[1], &status[1], 0) == -1) {};
    /*printf("Exit status of %d was %d\n", pid[1], WEXITSTATUS(status[1]));*/
    
    return (WEXITSTATUS(status[0]) == EXIT_SUCCESS
         && WEXITSTATUS(status[1]) == EXIT_SUCCESS)
           ? EXIT_SUCCESS
           : EXIT_FAILURE;
}

int doLsChild() {
    return execlp("ls", "ls");
}

/**
 * Do the stuff for the child who execs `sort`.
 * @param pipeFile a two-element array with file descriptors for the pipe.
 * @return None; exec takes over the process.
 */
int doSortChild(int pipeFile[]) {
    /*printf("Child 2\n");*/
    
    int outFile = open("outfile",
                       O_RDWR | O_CREAT | O_TRUNC,
                       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    
    if (dup2(pipeFile[0], STDIN_FILENO) == -1) {
        return EXIT_FAILURE;
    }
    if (dup2(outFile, STDOUT_FILENO) == -1) {
        return EXIT_FAILURE;
    }
    close(pipeFile[0]);
    close(pipeFile[1]);
    close(outFile);
    
    return execlp("sort", "sort", "-r", (char*)NULL);
}

