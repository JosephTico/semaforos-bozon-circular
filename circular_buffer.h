#ifndef CIRCULAR_BUFFER
#define CIRCULAR_BUFFER

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>


#define CBUFFER_SIZE 20
#define SEMAPHORE_MEMORY_SYNC "cbuffer_memory"
#define SEMAPHORE_CONSUMERS "cbuffer_consumers"
#define SEMAPHORE_PRODUCERS "cbuffer_producers"
#define STORAGE_ID "SHM_TEST"
#define STORAGE_SIZE sizeof( circular_buffer)





typedef struct 
{
    int random;
    pid_t producer_id;
    int content;
    bool consumed;
}cbuffer_message;



typedef struct 
{
    int total_messages;
    int total_producers;
    int total_consumers;
    int next_message_to_consume;
    int next_message_to_produce;
    int consumers_killed_by_id;
    cbuffer_message messages[CBUFFER_SIZE];
}circular_buffer;


void initialize_cbuffer(circular_buffer *buffer);
void increase_next_message_to_consume(circular_buffer *buffer);
void increase_next_message_to_produce(circular_buffer *buffer);
cbuffer_message generate_message(pid_t pid);
cbuffer_message consume_message(circular_buffer *buffer);














#endif
