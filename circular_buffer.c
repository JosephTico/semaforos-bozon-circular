#include <sys/time.h>

#include "circular_buffer.h"
#include "colorprint.h"

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
    printf_color(1, "[bb][lw][info][/lw][/bb] Buffer cerrado satisfactoriamente.");

    printf_color(1, "[cyan]Total de mensajes:[/cyan] [yellow]%i[/yellow]\n", buffer->total_messages);
    printf_color(1, "[cyan]Total de productores:[/cyan] [yellow]%i[/yellow]\n", buffer->total_producers);
    printf_color(1, "[cyan]Total de consumidores:[/cyan] [yellow]%i[/yellow]\n", buffer->total_consumers);
    printf_color(1, "[cyan]Consumidores finalizados por ID:[/cyan] [yellow]%i[/yellow]\n", buffer->consumers_killed_by_id);

    printf("\n" );
    printf_color(1, "[bg]Mensajes en el buffer:[/bg]\n");
    int i = 0;
    for(i=0;i<buffer->buffer_size;i++){
        printf_color(1, "[[cyan]slot[/cyan]: [yellow]%i[/yellow], ", i);
        print_message(&buffer->messages[i]);
    }
};


void print_message(cbuffer_message *message){
    printf_color(1, "[cyan]producer_pid[/cyan]: [yellow]%d[/yellow], ", message->producer_id);
    printf_color(1, "[cyan]content:[/cyan] [yellow]%i[/yellow], ", message->content);
    printf_color(1, "[cyan]magic_number[/cyan]: [yellow]%i[/yellow], ", message->random);
    printf_color(1, "[cyan]consumed[/cyan]: [yellow]%i[/yellow]", message->consumed);
    printf("]\n");
};



void increase_consumers_killed_by_id(circular_buffer *buffer)
{
    buffer->consumers_killed_by_id = buffer->consumers_killed_by_id + 1;
}

cbuffer_message consume_message(circular_buffer *buffer)
{
    buffer->messages[buffer->next_message_to_consume].consumed = true;
    cbuffer_message tmp_message = buffer->messages[buffer->next_message_to_consume];
    increase_next_message_to_consume(buffer);
    return tmp_message;
};

long long current_timestamp()
{
    struct timeval te;
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;
    return milliseconds;
}
