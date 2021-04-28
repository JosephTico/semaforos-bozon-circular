#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <sys/stat.h>

#include "circular_buffer.h"

int main(int argc, char *argv[])
{
  int res;
  int fd;
  pid_t pid;
  circular_buffer *addr;

  // Get argv
  if (argc < 2)
  {
    printf("error: missing command line arguments\n");
    return -1;
  }
  char buffer_name[strlen(argv[1])];
  strcpy(buffer_name, argv[1]);

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

  pid = getpid();

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

  printf("STORAGE SIZE: %li\n", shared_memory_size);
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
  addr->kill_producers = true;
  sem_post(sem_mem_id);
  sem_post(sem_pro_id);

  
  
  sem_wait(sem_mem_id);
  int producers_alive = addr->current_consumers;
  sem_post(sem_mem_id);


  while (producers_alive > 0)
  {
    sem_wait(sem_mem_id);
    producers_alive = addr->current_consumers;
    sem_post(sem_mem_id);
  }
  sleep(5);
  
  
  
  sem_wait(sem_mem_id);
  int consumers_alive = addr->current_consumers;
  sem_post(sem_mem_id);
  sem_wait(sem_pro_id);
  
//   sem_wait(sem_pro_id);
  while (consumers_alive > 0)
  {
    sem_wait(sem_pro_id);
    sem_wait(sem_mem_id);
    addr->messages[addr->next_message_to_produce] = generate_message(pid, true);
    increase_next_message_to_produce(addr);
    addr->total_messages++;
    consumers_alive--;
    sem_post(sem_mem_id);
  }
  print_buffer(addr);

  //   place data into memory
  //   memcpy(data, addr, DATA_SIZE);
  // addr->messages[0].random
  //   printf("PID %d: Read from shared memory: \"%s\"\n", pid, data);
  //   printf("PID %d: Read from shared memory: \"%d\"\n", pid, addr->messages[0].random);

  sem_close(sem_con_id);
  sem_unlink(sem_con_name);

  sem_close(sem_mem_id);
  sem_unlink(sem_mem_name);

  sem_close(sem_pro_id);
  sem_unlink(sem_prod_name);

  // shm_open cleanup
  fd = shm_unlink(buffer_name);
  if (fd == -1)
  {
    perror("unlink");
    return 100;
  }

  return 0;
}
