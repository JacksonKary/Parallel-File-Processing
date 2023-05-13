#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_WORD_LEN 25

/*
 * Counts the number of occurrences of words of different lengths in a text
 * file and stores the results in an array.
 * file_name: The name of the text file from which to read words
 * counts: An array of integers storing the number of words of each possible
 *     length.  counts[0] is the number of 1-character words, counts [1] is the
 *     number of 2-character words, and so on.
 * Returns 0 on success or -1 on error.
 */
int count_word_lengths(const char *file_name, int *counts) {
    // open file for reading
    FILE *fp = fopen(file_name, "r");
    if (fp == NULL) {
        perror("fopen");
        return -1;
    }
    char c;
    int len = 0;
    size_t nread;
    // loop through file, counting the length of each word. Length is determined when white space or EOF is reached
    while((nread = fread(&c, sizeof(char), 1, fp)) == 1) {
        if (isspace(c)) {  // we get to assume whitspace or lowercase alpha, otherwise could check "if (!isalpha(c))"
            if (len > 0 && len <= MAX_WORD_LEN) {  // current char is whitespace, check if len > 0: meaning previous char was last letter of a word
                counts[len - 1]++;  // +1 word of length len, stored in index (len - 1) in counts
            }
            len = 0;  // whitespace interrupts and resets word length (len)
            continue;
        }  // else: isalpha(c)
        len++;
        // if (len > MAX_WORD_LEN) {  // error case, but we get to assume valid input, so this isn't requried
        //     // error
        // }
    }
    if (ferror(fp)) {  // fread returns 0 on error, but also returns 0 on EOF. Explicitly check for error
        perror("fread");
        fclose(fp);
        return -1;
    } else if (feof(fp)) {  // after reaching EOF, check if the last char of the file was the end of a word
        if (len > 0 && len <= MAX_WORD_LEN) {
            counts[len - 1]++;  // +1 word of length len, stored in index (len - 1) in counts
        }
    }

    if (fclose(fp) != 0) {
        perror("fclose");
        return -1;
    }

    return 0;
}

/*
 * Processes a particular file (counting the number of words of each length)
 * and writes the results to a file descriptor.
 * This function should be called in child processes.
 * file_name: The name of the file to analyze.
 * out_fd: The file descriptor to which results are written
 * Returns 0 on success or -1 on error
 */
int process_file(const char *file_name, int out_fd) {
    int results[MAX_WORD_LEN];
    memset(results, 0, sizeof(results));  // initialize & clear array
    if (count_word_lengths(file_name, results) != 0) {
        return -1;
    }
    if (write(out_fd, results, sizeof(results))  == -1) {
        perror("write");
        return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 1) {
        // no files to consume, return immediately
        return 0;
    }

    // create a pipe for child processes to write their results
    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) {  // create pipe and check for errors
        perror("pipe");
        return 1;
    }

    // fork a child to analyze each specified file (names are argv[1], argv[2], ...)
    for (int i = 1; i < argc; i++) {
        pid_t child_pid = fork();
        if (child_pid == -1) {  // fork error
            perror("fork");
            close(pipe_fds[0]);
            close(pipe_fds[1]);
            exit(1);  // exit process with error, should be noticed by the waiting parent
        } else if (child_pid == 0) {  // child process
            // close read end of pipe
            if (close(pipe_fds[0]) == -1) {
                perror("close");
                close(pipe_fds[0]);
                exit(1);  // exit process with error, should be noticed by the waiting parent
            }
            // call process_file in child, results written to pipe
            if (process_file(argv[i], pipe_fds[1]) != 0) {
                close(pipe_fds[1]);
                exit(1);  // exit process with error, should be noticed by the waiting parent
            }
            // close write end of pipe
            if (close(pipe_fds[1]) == -1) {
                perror("close");
                close(pipe_fds[1]);
                exit(1);  // exit process with error, should be noticed by the waiting parent
            }
            exit(0);  // exit normally
        }
    }
    // only reached by parent process
    // close write end of pipe
    if (close(pipe_fds[1]) == -1) {
        perror("close");
        close(pipe_fds[0]);
        return 1;
    }
    
    // aggregate all the results together by reading from the pipe in the parent
    int results[MAX_WORD_LEN];  // holds final results/sum of results
    memset(results, 0, sizeof(results));  // initialize & clear array
    int buffer[MAX_WORD_LEN];  // buffer
    memset(buffer, 0, sizeof(buffer));  // initialize & clear array
    int nbytes;
    // as long as there are children still writing to the pipe, be ready to add their results to the total
    while ((nbytes = read(pipe_fds[0], &buffer, sizeof(buffer))) > 0) {
        for(int i = 0; i < MAX_WORD_LEN; i++) {
            results[i] += buffer[i];
        }
    }
    if (nbytes == -1) {  // read failed, close pipe and return with error
        perror("read");
        close(pipe_fds[0]);
        return 1;
    }

    if (close(pipe_fds[0]) == -1) {  // close pipe normally, job is complete
        perror("close");
        return 1;
    }

    // print out the total count of words of each length
    for (int i = 1; i <= MAX_WORD_LEN; i++) {
        printf("%d-Character Words: %d\n", i, results[i - 1]);
    }
    // wait for children to finish, check their exit status for errors
    int status;
    int ret_val = 0;
    for (int i = 1; i < argc; i++) {
        wait(&status);
        if (WIFEXITED(status)) {  // check if exited normally
            int child_ret_val = WEXITSTATUS(status);  // check the return value
            if (child_ret_val != 0) {  // if child exited abnormally, return error
                ret_val = 1;  // wait for all children even if one failed
            }
        } else {  // child process terminated abnormally
            ret_val = 1;  // wait for all children even if one failed
        }
    }

    return 0;
}
