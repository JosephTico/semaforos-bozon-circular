#ifndef CIRCULAR_BUFFER
#define CIRCULAR_BUFFER

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define SEMAPHORE_MEMORY_SYNC "cbuffer_memory"
#define SEMAPHORE_CONSUMERS "cbuffer_consumers"
#define SEMAPHORE_PRODUCERS "cbuffer_producers"
#define STORAGE_ID "SHM_TEST"

typedef struct
{
    int random;
    pid_t producer_id;
    int content;
    bool consumed;
} cbuffer_message;

typedef struct
{
    int buffer_size;
    int total_messages;
    int total_producers;
    int total_consumers;
    int next_message_to_consume;
    int next_message_to_produce;
    int consumers_killed_by_id;
    cbuffer_message messages[];
} circular_buffer;

void initialize_cbuffer(circular_buffer *buffer);
void increase_next_message_to_consume(circular_buffer *buffer);
void increase_next_message_to_produce(circular_buffer *buffer);
cbuffer_message generate_message(pid_t pid);
cbuffer_message consume_message(circular_buffer *buffer);

#endif
