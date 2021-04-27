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
cbuffer_message generate_message(pid_t pid, bool kill){
    cbuffer_message new_messsage;
    new_messsage.consumed=false;
    new_messsage.random=rand() % 6;
    new_messsage.content=rand() % 500;
    new_messsage.producer_id=pid;
    if(kill){
      new_messsage.type=KILL_CONSUMER;
    }
    else{
      new_messsage.type=NORMAL;
    }
    return new_messsage;
    
};






void increase_consumers_killed_by_id(circular_buffer *buffer){
    buffer->consumers_killed_by_id = buffer->consumers_killed_by_id+1;
}


cbuffer_message consume_message(circular_buffer *buffer){
    buffer->messages[buffer->next_message_to_consume].consumed=true;
    increase_next_message_to_consume(buffer);
    return buffer->messages[buffer->next_message_to_consume];
};
