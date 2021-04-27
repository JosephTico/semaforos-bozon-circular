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

typedef enum
{
    KILL_CONSUMER,
    NORMAL
} cbuffer_message_type;

typedef struct
{
    int random;
    pid_t producer_id;
    int content;
    bool consumed;
    cbuffer_message_type type;
} cbuffer_message;

typedef struct
{
    int buffer_size;
    int kill_producers;
    int total_messages;
    int total_producers;
    int total_consumers;
    int current_producers;
    int current_consumers;
    int next_message_to_consume;
    int next_message_to_produce;
    int consumers_killed_by_id;
    cbuffer_message messages[];
} circular_buffer;

void initialize_cbuffer(circular_buffer *buffer);
void increase_next_message_to_consume(circular_buffer *buffer);
void increase_next_message_to_produce(circular_buffer *buffer);
void increase_consumers_killed_by_id(circular_buffer *buffer);
cbuffer_message generate_message(pid_t pid, bool kill);
cbuffer_message consume_message(circular_buffer *buffer);
void print_buffer(circular_buffer *buffer);
void print_message(cbuffer_message *message);





#endif
