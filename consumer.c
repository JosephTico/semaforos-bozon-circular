#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "circular_buffer.h"
#include <semaphore.h>

#define DATA_SIZE 30

bool running = true;
pid_t pid;

void exit_by_id()
{
  printf("Consumidor finalizado por id:\n PID %d: \n", pid);
}

void exit_by_finalizer()
{
  printf("Consumidor finalizado por finalizador:\n PID %d: \n", pid);
}

int main(int argc, char *argv[])
{
  int res;
  int fd;
  char data[DATA_SIZE];
  circular_buffer *addr;

  // Get argv
  if (argc < 3)
  {
    printf("error: missing command line arguments\n");
    return -1;
  }
  char buffer_name[strlen(argv[1])];
  strcpy(buffer_name, argv[1]);
  int wait_time = atoi(argv[2]);

  // Initialize strings for semaphore names
  char *sem_mem_name_base = "semaphore_memory_";
  char *sem_con_name_base = "semaphore_consumers_";
  char *sem_prod_name_base = "semaphore_producers_";

  char sem_mem_name[strlen(buffer_name) + strlen(sem_mem_name_base)];
  char sem_con_name[strlen(buffer_name) + strlen(sem_con_name_base)];
  char sem_prod_name[strlen(buffer_name) + strlen(sem_prod_name_base)];

  strcpy(sem_mem_name, sem_mem_name_base);
  strcat(sem_mem_name, buffer_name);
  strcpy(sem_con_name, sem_con_name_base);
  strcat(sem_con_name, buffer_name);
  strcpy(sem_prod_name, sem_prod_name_base);
  strcat(sem_prod_name, buffer_name);

  printf("ARGV BUFFER NAME: %s\n", buffer_name);
  printf("ARGV WAIT TIME: %i\n", wait_time);

  pid = getpid();

  printf("NEW CONSUMER:\n PID %d\n", pid);

  // get shared memory file descriptor (NOT a file)
  fd = shm_open(buffer_name, O_RDWR, S_IRUSR | S_IWUSR);
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

  printf("STORAGE SIZE: %i\n", shared_memory_size);
  printf("BUFFER SIZE: %i\n", addr->buffer_size);

  // Initialize semaphores
  sem_t *sem_mem_id = sem_open(sem_mem_name, O_CREAT, 0600, 1);
  if (sem_mem_id == SEM_FAILED)
  {
    perror("SEMAPHORE_MEMORY_SYNC  : [sem_open] Failed\n");
  }

  sem_t *sem_pro_id = sem_open(sem_prod_name, O_CREAT, 0600, addr->buffer_size);
  if (sem_pro_id == SEM_FAILED)
  {
    perror("SEMAPHORE_MEMORY_SYNC  : [sem_open] Failed\n");
  }

  sem_t *sem_con_id = sem_open(sem_con_name, O_CREAT, 0600, 0);
  if (sem_con_id == SEM_FAILED)
  {
    perror("SEMAPHORE_MEMORY_SYNC  : [sem_open] Failed\n");
  }

  sem_wait(sem_mem_id);
  addr->current_consumers++;
  addr->total_consumers++;
  sem_post(sem_mem_id);

  while (running)
  {
    sem_wait(sem_con_id);
    sem_wait(sem_mem_id);
    cbuffer_message message = consume_message(addr);
    printf("NUEVO MENSAJE:\n PID %d: Random: %d\n", message.producer_id, message.random);
    sem_post(sem_mem_id);
    sem_post(sem_pro_id); //+1 al semaforo para que consuman

    if (message.type == KILL_CONSUMER)
    {
      running = false;
      addr->current_consumers--;
      exit_by_finalizer();
    }
    else if (message.random == pid % 6)
    {
      running = false;
      addr->current_consumers--;
      addr->consumers_killed_by_id++;
      exit_by_id();
    }
    else
    {
      sleep(1);
    }
  }
  /*
  printf("PID %d: Read from shared memory: \"%s\"\n", pid, (char *)addr);

  printf("PID %d: Read from shared memory block 2: \"%s\"\n", pid, (char *)addr + 50);*/

  return 0;
}
