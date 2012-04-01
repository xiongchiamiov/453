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
    int pipeFile[2];
    
    pipe(pipeFile);
    
    if (fork() == 0) {
        return doLsChild(pipeFile);
    }
    if (fork() == 0) {
        return doSortChild(pipeFile);
    }
    
    /*printf("Parent\n");*/
    return EXIT_SUCCESS;
}

int doLsChild(int pipeFile[]) {
    /*printf("Child 1\n");*/
    
    dup2(pipeFile[1], STDOUT_FILENO);
    close(pipeFile[0]);
    /*write(pipeFile[1], "hello", 6);*/
    close(pipeFile[1]);
    execlp("ls", "ls");
    return EXIT_SUCCESS;
}

int doSortChild(int pipeFile[]) {
    /*printf("Child 2\n");*/
    
    int outFile = open("outfile",
                       O_RDWR | O_CREAT | O_TRUNC,
                       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    
    dup2(pipeFile[0], STDIN_FILENO);
    dup2(outFile, STDOUT_FILENO);
    close(pipeFile[0]);
    close(pipeFile[1]);
    close(outFile);
    
    execlp("sort", "sort", "-r", (char*)NULL);
    return EXIT_SUCCESS;
}

