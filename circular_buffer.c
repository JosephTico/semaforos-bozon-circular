#include "circular_buffer.h"


void initialize_cbuffer( circular_buffer *buffer){
    buffer->consumers_killed_by_id=0;
    buffer->next_message_to_produce=0;
    buffer->next_message_to_consume=0;
    buffer->current_consumers=0;
    buffer->current_producers=0;
    buffer->total_consumers=0;
    buffer->total_producers=0;
    buffer->total_messages=0;
    buffer->kill_producers=false;
};


void increase_next_message_to_consume(circular_buffer *buffer)
{
    buffer->next_message_to_consume = buffer->next_message_to_consume + 1;
    if (buffer->next_message_to_consume == buffer->buffer_size)
    {
        buffer->next_message_to_consume = 0;
    }
};

void increase_next_message_to_produce(circular_buffer *buffer)
{
    buffer->next_message_to_produce = buffer->next_message_to_produce + 1;
    if (buffer->next_message_to_produce == buffer->buffer_size)
    {
        buffer->next_message_to_produce = 0;
    }
};
cbuffer_message generate_message(pid_t pid, bool kill)
{
    cbuffer_message new_messsage;
    new_messsage.consumed = false;
    new_messsage.random = rand() % 6;
    new_messsage.content = rand() % 500;
    new_messsage.producer_id = pid;
    if (kill){
        new_messsage.type = KILL_CONSUMER;
    }
    else{
        new_messsage.type = NORMAL;
    }
    return new_messsage;
};



void print_buffer(circular_buffer *buffer){
    printf("CURRENT BUFFER:\n " );
    printf("kill_producers: %d\n ", buffer->kill_producers );
    printf("total_messages: %d\n ", buffer->total_messages );
    printf("total_producers: %d\n ", buffer->total_producers );
    printf("total_consumers: %d\n ", buffer->total_consumers );
    printf("current_producers: %d\n ", buffer->current_producers );
    printf("current_consumers: %d\n ", buffer->current_consumers );
    printf("consumers_killed_by_id: %d\n ", buffer->consumers_killed_by_id );
    printf("\n" );
    int i=0;
    for(i=0;i<buffer->buffer_size;i++){
      print_message(&buffer->messages[i]);
    }
};


void print_message(cbuffer_message *message){
    printf("[prod_id:%d, random:%d, consumed:%d   ",message->producer_id,message->random,message->consumed);
    if (message->type==KILL_CONSUMER){
      printf(", type:KILL_CONSUMER" );
    }
    else{
      printf(", type:NORMAL" );
    }
    printf("]\n");
};



void increase_consumers_killed_by_id(circular_buffer *buffer)
{
    buffer->consumers_killed_by_id = buffer->consumers_killed_by_id + 1;
}

cbuffer_message consume_message(circular_buffer *buffer)
{
    buffer->messages[buffer->next_message_to_consume].consumed = true;
    increase_next_message_to_consume(buffer);
    return buffer->messages[buffer->next_message_to_consume];
};




