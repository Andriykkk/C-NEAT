CC = gcc
CFLAGS = -g
EXEC = main
SOURCE = src/main.c src/utils.c src/neat.c src/mutations.c src/node.c
LIBS = -lm -fopenmp

all: $(EXEC)
	./$(EXEC)

$(EXEC): $(SOURCE)
	$(CC) $(CFLAGS) -o $(EXEC) $(SOURCE) $(LIBS)

clean:
	rm -f $(EXEC)
