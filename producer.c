#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define STORAGE_ID "SHM_TEST"
#define STORAGE_SIZE 50 * 1024 * 1024
#define DATA_SIZE 30

int main(int argc, char *argv[])
{
    int res;
    int fd;
    char data[DATA_SIZE];
    pid_t pid;
    void *addr;

    pid = getpid();

    // get shared memory file descriptor (NOT a file)
    fd = shm_open(STORAGE_ID, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        perror("open");
        return 10;
    }

    // map shared memory to process address space
    addr = mmap(NULL, STORAGE_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
    {
        perror("mmap");
        return 30;
    }

    char *block2 = "HOLA mUndO DESDE BLOQUE2";
    strcpy(addr + 50, block2);
    printf("PID %d: Wrote to block2 \"%s\"\n", pid, block2);

    printf("PID %d: Read from shared memory: \"%s\"\n", pid, (char *)addr);

    return 0;
}