# Student Name: Liam Bush
# Student Number: 24227223
# Student Name: Amir Husain
# Student Number: 23380159

## Unified Makefile with selectable platforms: setonix and kaya
## Use: `make setonix` or `make kaya`

# make kaya 
KAYA_CC ?= mpicc
KAYA_CFLAGS ?= -fopenmp -Wall

# make setonix
SETONIX_CC = cc
SETONIX_CFLAGS = -fopenmp -Wall

SRC=main.c matrix.c conv2d_stride.c
OBJ=$(SRC:.c=.o)
HEADERS=matrix.h conv2d_stride.h

BIN=Convolution_stride

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

run:
	./$(BIN) $(ARGS)

clean:
	rm -f *.o $(BIN)

# Build using setonix 
setonix:
	$(MAKE) CC=$(SETONIX_CC) CFLAGS="$(SETONIX_CFLAGS)" all

# Build for kaya 
kaya:
	$(MAKE) CC=$(KAYA_CC) CFLAGS="$(KAYA_CFLAGS)" all

