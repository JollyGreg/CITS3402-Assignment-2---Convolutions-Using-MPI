# Student Name: Liam Bush
# Student Number: 24227223
# Student Name: Amir Husain
# Student Number: 23380159

CC=mpicc
CFLAGS=-fopenmp -Wall

SRC=main.c matrix.c conv2d_stride.c
OBJ=$(SRC:.c=.o)
HEADERS=matrix.h conv2d_stride.h

all: Convolution_stride

Convolution_stride: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

run:
	./Convolution_stride $(ARGS)

clean:
	rm -f *.o Convolution_stride
