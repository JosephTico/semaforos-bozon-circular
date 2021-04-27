#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include "circular_buffer.h"

#define DATA "Hello, World! From PID %d"

int main(int argc, char *argv[])
{
  int res;
  int fd;
  int len;
  pid_t pid;
  circular_buffer *addr;

  // inicializar semaforo
  sem_t *sem_mem_id = sem_open(SEMAPHORE_MEMORY_SYNC, O_CREAT, 0600, 1);
  if (sem_mem_id == SEM_FAILED)
  {
    perror("SEMAPHORE_MEMORY_SYNC  : [sem_open] Failed\n");
  }

  sem_t *sem_pro_id = sem_open(SEMAPHORE_PRODUCERS, O_CREAT, 0600, CBUFFER_SIZE);
  if (sem_pro_id == SEM_FAILED)
  {
    perror("SEMAPHORE_MEMORY_SYNC  : [sem_open] Failed\n");
  }

  sem_t *sem_con_id = sem_open(SEMAPHORE_CONSUMERS, O_CREAT, 0600, 0);
  if (sem_con_id == SEM_FAILED)
  {
    perror("SEMAPHORE_MEMORY_SYNC  : [sem_open] Failed\n");
  }

  pid = getpid();
  //   sprintf(data, DATA, pid);

  // get shared memory file descriptor (NOT a file)
  fd = shm_open(STORAGE_ID, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1)
  {
    perror("open");
    return 10;
  }

  // extend shared memory object as by default it's initialized with size 0
  res = ftruncate(fd, STORAGE_SIZE);
  if (res == -1)
  {
    perror("ftruncate");
    return 20;
  }

  // map shared memory to process address space
  addr = mmap(NULL, STORAGE_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
  if (addr == MAP_FAILED)
  {
    perror("mmap");
    return 30;
  }

  // TODO: Aqui se deben setear e inicializar todo, empezando a partir de addr
  sem_wait(sem_mem_id);
  circular_buffer initbuffer;
  memcpy(addr, &initbuffer, STORAGE_SIZE);
  initialize_cbuffer(addr);
  sem_post(sem_mem_id);

  // mmap cleanup
  res = munmap(addr, STORAGE_SIZE);
  if (res == -1)
  {
    perror("munmap");
    return 40;
  }

  return 0;
}
