#Student Number: 23380159
#By Amir Husain
CC=gcc
CFLAGS=-fopenmp -Wall

SRC=main.c matrix.c
OBJ=$(SRC:.c=.o)


all: Convolution_stride

Convolution_stride: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run:
	./all: Convolution_stride $(ARGS)

clean:
	rm -f *.o all: Convolution_stride
