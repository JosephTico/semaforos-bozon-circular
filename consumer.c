#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <time.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include "circular_buffer.h"
#include "colorprint.h"

bool running = true;
pid_t pid;
circular_buffer *addr;
double total_wait_time = 0.0;
double total_blocked_time = 0.0;
int total_consumed_messages = 0;

sem_t *sem_mem_id;
sem_t *sem_pro_id;
sem_t *sem_con_id;



void print_stats()
{
  struct tms cpu_times;
  times(&cpu_times);
  printf_color(1, "[cyan]Tiempo de espera total:[/cyan] [yellow]%f s[/yellow]\n", total_wait_time);
  printf_color(1, "[cyan]Tiempo bloqueado total:[/cyan] [yellow]%f s[/yellow]\n", total_blocked_time);
  printf_color(1, "[cyan]Mensajes consumidos:[/cyan] [yellow]%i[/yellow]\n", total_consumed_messages);
  printf_color(1, "[cyan]Tiempo de usuario:[/cyan] [yellow]%ld[/yellow]\n", (long)cpu_times.tms_utime);
  printf_color(1, "[cyan]Tiempo de kernel:[/cyan] [yellow]%ld[/yellow]\n", (long)cpu_times.tms_stime);
}

void exit_by_id()
{
  printf_color(1, "[bb][lw][info][/lw][/bb] Consumidor finalizado por ID (aleatorio). PID: %d.\n", pid);
  print_stats();
}

void exit_by_finalizer()
{
  printf_color(1, "[bb][lw][info][/lw][/bb] Consumidor cerrado por finalizador. PID: %d.\n", pid);
  print_stats();
}

void intHandler(int dummy)
{
  printf("Finalización forzada.\n");
  addr->current_consumers--;
  print_stats();
  exit(0);
}

int main(int argc, char *argv[])
{
  signal(SIGINT, intHandler);

  const gsl_rng_type *T;
  gsl_rng *r;
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc(T);
  gsl_ran_poisson(r, 5);

  int fd;
  time_t rawtime;
  struct tm *timeinfo;

  // Get argv
  if (argc < 3)
  {
    printf_color(1, "[red][br][lw][error][/lw][/br] No se han dado todos los argumentos necesarios.[/red]\n");
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

  pid = getpid();

  printf_color(1, "[bb][lw][info][/lw][/bb] Inicializando consumidor. PID: %d.\n", pid);
  printf_color(1, "[bb][lw][info][/lw][/bb] Nombre de buffer dado: %s.\n", buffer_name);
  printf_color(1, "[bb][lw][info][/lw][/bb] Tiempo de espera promedio: %i ms.\n\n", wait_time);

  // get shared memory file descriptor (NOT a file)
  fd = shm_open(buffer_name, O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1)
  {
    printf_color(1, "[br][lw][error][/lw][/br] [red]El buffer no se ha inicializado.[/red]\n");
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
    printf_color(1, "[br][lw][error][/lw][/br] [red]No se ha podido asignar suficiente memoria al proceso.[/red]\n");
    return 30;
  }

  // Initialize semaphores
  sem_mem_id = sem_open(sem_mem_name, O_CREAT, 0600, 1);
  if (sem_mem_id == SEM_FAILED)
  {
    perror("SEMAPHORE_MEMORY_SYNC  : [sem_open] Failed\n");
  }

  sem_pro_id = sem_open(sem_prod_name, O_CREAT, 0600, addr->buffer_size);
  if (sem_pro_id == SEM_FAILED)
  {
    perror("SEMAPHORE_MEMORY_SYNC  : [sem_open] Failed\n");
  }

  sem_con_id = sem_open(sem_con_name, O_CREAT, 0600, 0);
  if (sem_con_id == SEM_FAILED)
  {
    perror("SEMAPHORE_MEMORY_SYNC  : [sem_open] Failed\n");
  }

  sem_wait(sem_mem_id);
  addr->current_consumers++;
  addr->total_consumers++;
  sem_post(sem_mem_id);

  int current_slot = 0;
  long long t = 0;

  while (running)
  {
    t = current_timestamp();

    sem_wait(sem_con_id);
    sem_wait(sem_mem_id);

    // Add to total blocked time
    t = current_timestamp() - t;
    total_blocked_time = total_blocked_time + t / 1000.0;

    current_slot = addr->next_message_to_consume;

    cbuffer_message message = consume_message(addr);
    total_consumed_messages++;

    printf_color(1, "\n[bb]--------------------------------------------[/bb]\n");
    printf_color(1, "[cyan]Consumidor PID:[/cyan] [yellow]%i[/yellow]\n", pid);
    printf_color(1, "[cyan]Recibido mensaje del productor PID:[/cyan] [yellow]%i[/yellow]\n", message.producer_id);
    printf_color(1, "[cyan]Leído del slot [yellow]%i[/yellow] del buffer[/cyan]\n", current_slot);
    printf_color(1, "[cyan]Contenido del mensaje:[/cyan] [yellow]%i[/yellow]\n", message.content);
    printf_color(1, "[cyan]Número mágico:[/cyan] [yellow]%i[/yellow]\n", message.random);

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    printf_color(1, "[cyan]Hora actual:[/cyan] [yellow]%s[/yellow]", asctime(timeinfo));
    printf_color(1, "[cyan]Consumidores conectados:[/cyan] [yellow]%i[/yellow]\n", addr->current_consumers);
    printf_color(1, "[cyan]Productores conectados:[/cyan] [yellow]%i[/yellow]\n", addr->current_producers);

    printf_color(1, "[bb]--------------------------------------------[/bb]\n\n");

    sem_post(sem_mem_id);
    sem_post(sem_pro_id); //+1 al semaforo para que consuman

    if (message.type == KILL_CONSUMER)
    {
      running = false;
      sem_wait(sem_con_id);
      addr->current_consumers--;
      sem_post(sem_mem_id);
      exit_by_finalizer();
    }
    else if (message.random == pid % 6)
    {
      running = false;
      sem_wait(sem_con_id);
      addr->current_consumers--;
      addr->consumers_killed_by_id++;
      sem_post(sem_mem_id);
      exit_by_id();
    }
    else
    {
      int random_wait = gsl_ran_poisson(r, wait_time);
      //       int random_wait = gsl_ran_exponential(r,2000);
      printf_color(1, "[green]*********** Esperando %d ms ***********[/green]\n", random_wait);
      total_wait_time = total_wait_time + random_wait / 1000.0;
      //       sleep(random_wait/1000);
      usleep(random_wait * 1000);
    }
  }

  gsl_rng_free(r);
  return 0;
}
