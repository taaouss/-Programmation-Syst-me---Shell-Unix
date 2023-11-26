CC = gcc
CFLAGS = -Wall -Wextra
LIBS = -lreadline

main: main.c
	$(CC) $(CFLAGS) -o main main.c $(LIBS)

run: main
	./main

clean:
	rm -f main
