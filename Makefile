CC = gcc
CFLAGS = -g
EXEC = main
SOURCE = src/main.c
LIBS = -lm

all: $(EXEC)
	./$(EXEC)

$(EXEC): $(SOURCE)
	$(CC) $(CFLAGS) -o $(EXEC) $(SOURCE) $(LIBS)

clean:
	rm -f $(EXEC)
