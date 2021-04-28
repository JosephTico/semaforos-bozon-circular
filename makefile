OBJS	= consumer.o  finalizer.o initializer.o producer.o circular_buffer.o
# SOURCE	= consumer.c  finalizer.c initializer.c producer.c

# SOURCE	= initializer.c circular_buffer.o
# HEADER	= circular_buffer.h
# OUT	= initializer 
OUT	= finalizer consumer initializer producer
CC	 = gcc
FLAGS	 = -g  -Wall 
LFLAGS	 = -lm -pthread -lrt -lgsl -lgslcblas

all: $(OBJS)

	$(CC) $(FLAGS)  finalizer.o circular_buffer.o  -o finalizer $(LFLAGS)
	$(CC) $(FLAGS)  initializer.o circular_buffer.o  -o initializer $(LFLAGS)
	$(CC) $(FLAGS)  producer.o circular_buffer.o  -o producer $(LFLAGS)
	$(CC) $(FLAGS)  consumer.o circular_buffer.o  -o consumer $(LFLAGS)
	
clean:
	rm -f $(OBJS) $(OUT)

	
