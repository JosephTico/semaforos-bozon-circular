#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "circular_buffer.h"



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
  addr = mmap(NULL, STORAGE_SIZE, PROT_READ, MAP_SHARED, fd, 0);
  if (addr == MAP_FAILED)
  {
    perror("mmap");
    return 30;
  }

  printf("PID %d: Read from shared memory: \"%s\"\n", pid, (char *)addr);

  printf("PID %d: Read from shared memory block 2: \"%s\"\n", pid, (char *)addr + 50);

  return 0;
}