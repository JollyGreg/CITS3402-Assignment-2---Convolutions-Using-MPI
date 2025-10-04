# Student Name: Liam Bush
# Student Number: 24227223
# Student Name: Amir Husain
# Student Number: 23380159

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
