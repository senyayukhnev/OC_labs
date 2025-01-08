#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define MAX_LINE_LENGTH 1024
#define SHARED_MEM_SIZE 4096

volatile sig_atomic_t data_ready = 0;

void handle_signal(int sig) {
    data_ready = 1;
}

char* remove_vowels(const char *str) {
    int len = strlen(str);
    char* new_str = (char*)malloc((len + 1) * sizeof(char));
    if (!new_str) return NULL;

    int pos = 0;
    const char* vowels = "AaEeIiOoUu";
    for (int i = 0; i < len; ++i) {
        char ch = str[i];
        int is_vowel = 0;
        for (int j = 0; vowels[j] != '\0'; ++j) {
            if (ch == vowels[j]) {
                is_vowel = 1;
                break;
            }
        }
        if (!is_vowel) {
            new_str[pos++] = ch;
        }
    }
    new_str[pos] = '\0';
    return new_str;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        write(STDERR_FILENO, "Usage: child <filename> <shared_mem_name>\n", 42);
        exit(EXIT_FAILURE);
    }

    const char* filename = argv[1];
    const char* shared_mem_name = argv[2];

    int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file == -1) {
        exit(EXIT_FAILURE);
    }

    int shm_fd = shm_open(shared_mem_name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        close(file);
        exit(EXIT_FAILURE);
    }

    char* shared_mem = mmap(NULL, SHARED_MEM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        close(file);
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    signal(SIGUSR1, handle_signal);

    while (1) {
        while (!data_ready) pause();
        data_ready = 0;

        if (strcmp(shared_mem, "exit") == 0) break;

        char* str = remove_vowels(shared_mem);
        if (str) {
            write(file, str, strlen(str));
            write(file, "\n", 1);
            free(str);
        }
    }

    munmap(shared_mem, SHARED_MEM_SIZE);
    close(shm_fd);
    close(file);

    return 0;
}
