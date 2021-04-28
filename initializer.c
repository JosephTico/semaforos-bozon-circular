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
  int len;
  pid_t pid;
  circular_buffer *addr;

  // Get argv
  if (argc < 3)
  {
    printf("error: missing command line arguments\n");
    return -1;
  }
  char buffer_name[strlen(argv[1])];
  strcpy(buffer_name, argv[1]);
  int buffer_size = atoi(argv[2]);

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
  printf("ARGV BUFFER SIZE: %i\n", buffer_size);

  // Initialize the circular buffer
  circular_buffer initbuffer;
  initbuffer.buffer_size = buffer_size;
  size_t messages_size = sizeof(cbuffer_message) * buffer_size;
  size_t const STORAGE_SIZE = sizeof(initbuffer) + messages_size;

  printf("STORAGE SIZE: %li\n", STORAGE_SIZE);
  printf("BUFFER SIZE: %i\n", initbuffer.buffer_size);

  // inicializar semaforo
  sem_t *sem_mem_id = sem_open(sem_mem_name, O_CREAT, 0600, 0);
  if (sem_mem_id == SEM_FAILED)
  {
    perror("SEMAPHORE_MEMORY_SYNC  : [sem_open] Failed\n");
  }

  sem_t *sem_pro_id = sem_open(sem_prod_name, O_CREAT, 0600, buffer_size);
  if (sem_pro_id == SEM_FAILED)
  {
    perror("SEMAPHORE_MEMORY_SYNC  : [sem_open] Failed\n");
  }

  sem_t *sem_con_id = sem_open(sem_con_name, O_CREAT, 0600, 0);
  if (sem_con_id == SEM_FAILED)
  {
    perror("SEMAPHORE_MEMORY_SYNC  : [sem_open] Failed\n");
  }

  pid = getpid();

  // get shared memory file descriptor (NOT a file)
  fd = shm_open(buffer_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
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
//   sem_wait(sem_mem_id);

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
