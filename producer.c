#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <semaphore.h>
#include "circular_buffer.h"

int main(int argc, char *argv[])
{
    int res;
    int fd;
    pid_t pid;
    circular_buffer *addr;

    pid = getpid();

    // get shared memory file descriptor (NOT a file)
    fd = shm_open(STORAGE_ID, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        perror("open");
        return 10;
    }

    // Get shared memory size from file descriptor
    struct stat finfo;
    fstat(fd, &finfo);
    off_t shared_memory_size = finfo.st_size;

    // map shared memory to process address space
    addr = mmap(NULL, shared_memory_size, PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
    {
        perror("mmap");
        return 30;
    }

    // Initialize semaphores
    sem_t *sem_mem_id = sem_open(SEMAPHORE_MEMORY_SYNC, O_CREAT, 0600, 1);
    if (sem_mem_id == SEM_FAILED)
    {
        perror("SEMAPHORE_MEMORY_SYNC  : [sem_open] Failed\n");
    }

    sem_t *sem_pro_id = sem_open(SEMAPHORE_PRODUCERS, O_CREAT, 0600, addr->buffer_size);
    if (sem_pro_id == SEM_FAILED)
    {
        perror("SEMAPHORE_MEMORY_SYNC  : [sem_open] Failed\n");
    }

    sem_t *sem_con_id = sem_open(SEMAPHORE_CONSUMERS, O_CREAT, 0600, 0);
    if (sem_con_id == SEM_FAILED)
    {
        perror("SEMAPHORE_MEMORY_SYNC  : [sem_open] Failed\n");
    }

    printf("STORAGE SIZE: %i\n", shared_memory_size);
    printf("BUFFER SIZE: %i\n", addr->buffer_size);

    int i = 0;
    while (true)
    {
        sem_wait(sem_pro_id);
        sem_wait(sem_mem_id);
        addr->messages[addr->next_message_to_produce] = generate_message(pid);
        increase_next_message_to_produce(addr);
        printf("producido %d\n", i);
        i++;
        sem_post(sem_mem_id);
        sem_post(sem_con_id); //+1 al semaforo para que consuman
        sleep(1);
    }

    return 0;
}
