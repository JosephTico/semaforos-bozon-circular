#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <semaphore.h>
#include "circular_buffer.h"


pid_t pid;
bool running = true;



void exit_by_finalizer(){
  printf("Productor finalizado por finalizador:\n PID %d: \n", pid );

}




int main(int argc, char *argv[])
{
    int res;
    int fd;
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
    addr->current_producers++;
    addr->total_producers++;
    sem_post(sem_mem_id);

    
    
    
    
    int i = 0;
    while (running)
    {
        sleep(1);
        
        sem_wait(sem_pro_id);
        sem_wait(sem_mem_id);
        if(addr->kill_producers){
          running=false;
          addr->current_producers--;
          exit_by_finalizer();
          sem_post(sem_pro_id);
          
        }
        else{
          addr->messages[addr->next_message_to_produce] = generate_message(pid, false);
          increase_next_message_to_produce(addr);
          addr->total_messages++;
          printf("producido %d\n", addr->total_messages);
          i++;
        }

        sem_post(sem_mem_id);
        sem_post(sem_con_id); //+1 al semaforo para que consuman
    }

    return 0;
}
