#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

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

  // map shared memory to process address space
  addr = mmap(NULL, STORAGE_SIZE, PROT_READ, MAP_SHARED, fd, 0);
  if (addr == MAP_FAILED)
  {
    perror("mmap");
    return 30;
  }

//   place data into memory
//   memcpy(data, addr, DATA_SIZE);
// addr->messages[0].random
//   printf("PID %d: Read from shared memory: \"%s\"\n", pid, data);
    printf("PID %d: Read from shared memory: \"%d\"\n", pid,addr->messages[0].random );

  // shm_open cleanup
  fd = shm_unlink(STORAGE_ID);
  if (fd == -1)
  {
    perror("unlink");
    return 100;
  }

  return 0;
}
