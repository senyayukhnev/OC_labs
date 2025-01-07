#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_LINE_SIZE 1024
#define SHARED_MEM_SIZE 4096

int main() {
    pid_t pid1, pid2;
    char filename1[MAX_LINE_SIZE], filename2[MAX_LINE_SIZE];
    char buffer[MAX_LINE_SIZE];

    const char* shared_mem1 = "/shared_mem1";
    const char* shared_mem2 = "/shared_mem2";

    int shm_fd1 = shm_open(shared_mem1, O_RDWR | O_CREAT, 0666);
    int shm_fd2 = shm_open(shared_mem2, O_RDWR | O_CREAT, 0666);
    if (shm_fd1 == -1 || shm_fd2 == -1) {
        exit(EXIT_FAILURE);
    }

    ftruncate(shm_fd1, SHARED_MEM_SIZE);
    ftruncate(shm_fd2, SHARED_MEM_SIZE);

    char* mem1 = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd1, 0);
    char* mem2 = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd2, 0);
    if (mem1 == MAP_FAILED || mem2 == MAP_FAILED) {
        exit(EXIT_FAILURE);
    }

    write(STDOUT_FILENO, "Enter filename for output file for child 1: ", 44);
    ssize_t len = read(STDIN_FILENO, filename1, MAX_LINE_SIZE);
    filename1[len - 1] = '\0';

    write(STDOUT_FILENO, "Enter filename for output file for child 2: ", 44);
    len = read(STDIN_FILENO, filename2, MAX_LINE_SIZE);
    filename2[len - 1] = '\0';

    pid1 = fork();
    if (pid1 == 0) {
        execl("./child", "child", filename1, shared_mem1, NULL);
        exit(EXIT_FAILURE);
    }

    pid2 = fork();
    if (pid2 == 0) {
        execl("./child", "child", filename2, shared_mem2, NULL);
        exit(EXIT_FAILURE);
    }

    int count = 0;
    while (1) {
        write(STDOUT_FILENO, "Enter a line (or 'exit' to quit): ", 34);
        len = read(STDIN_FILENO, buffer, MAX_LINE_SIZE);
        if (len <= 0) break;
        buffer[len - 1] = '\0';

        if (strcmp(buffer, "exit") == 0) {
            strcpy(mem1, "exit");
            strcpy(mem2, "exit");
            kill(pid1, SIGUSR1);
            kill(pid2, SIGUSR1);
            break;
        }

        count++;
        if (count % 2 == 0) {
            strcpy(mem2, buffer);
            kill(pid2, SIGUSR1);
        } else {
            strcpy(mem1, buffer);
            kill(pid1, SIGUSR1);
        }
    }

    wait(NULL);
    wait(NULL);

    shm_unlink(shared_mem1);
    shm_unlink(shared_mem2);

    return 0;
}
