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

  
  
  sem_wait(sem_mem_id);
  addr->kill_producers=true;
  sem_post(sem_mem_id);
  sem_post(sem_pro_id);
  
  
  
  
  //   place data into memory
  //   memcpy(data, addr, DATA_SIZE);
  // addr->messages[0].random
  //   printf("PID %d: Read from shared memory: \"%s\"\n", pid, data);
//   printf("PID %d: Read from shared memory: \"%d\"\n", pid, addr->messages[0].random);

  
  
  
  sem_close(sem_con_id);
  sem_unlink(SEMAPHORE_CONSUMERS);

  sem_close(sem_mem_id);
  sem_unlink(SEMAPHORE_MEMORY_SYNC);

  sem_close(sem_pro_id);
  sem_unlink(SEMAPHORE_PRODUCERS);

  // shm_open cleanup
  fd = shm_unlink(STORAGE_ID);
  if (fd == -1)
  {
    perror("unlink");
    return 100;
  }

  return 0;
}
