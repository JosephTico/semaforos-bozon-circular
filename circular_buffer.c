#include "circular_buffer.h"

void initialize_cbuffer( circular_buffer *buffer){
    buffer->consumers_killed_by_id=0;
    buffer->next_message_to_produce=0;
    buffer->next_message_to_consume=0;
    buffer->total_consumers=0;
    buffer->total_producers=0;
    buffer->total_messages=0;
    int i;
    for(i=0;i<CBUFFER_SIZE;i++){
        buffer->messages[i].consumed=true;
        buffer->messages[i].random=i;

    }
};
void increase_next_message_to_consume(circular_buffer *buffer){
    buffer->next_message_to_consume=buffer->next_message_to_consume+1;
    if (buffer->next_message_to_consume == CBUFFER_SIZE){
        buffer->next_message_to_consume=0;
    }
};


void increase_next_message_to_produce(circular_buffer *buffer){
    buffer->next_message_to_produce = buffer->next_message_to_produce+1;
    if (buffer->next_message_to_produce == CBUFFER_SIZE){
        buffer->next_message_to_produce = 0;
    }
};
cbuffer_message generate_message(pid_t pid){
    cbuffer_message new_messsage;
    new_messsage.consumed=false;
    new_messsage.random=rand() % 50;
    new_messsage.content=rand() % 500;
    new_messsage.producer_id=pid;
    return new_messsage;
    
};



cbuffer_message consume_message(circular_buffer *buffer){
//     buffer->messages[buffer->next_message_to_consume].consumed=true;
    return buffer->messages[buffer->next_message_to_consume];
};
