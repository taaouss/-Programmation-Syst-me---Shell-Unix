CC = gcc
CFLAGS = -Wall -Wextra
LIBS = -lreadline

main: main.c
	$(CC) $(CFLAGS) -o jsh main.c $(LIBS)

run: main
	./jsh

clean:
	rm -f jsh
