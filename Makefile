CC = gcc
CFLAGS = -Wall -Wextra
LIBS = -lreadline

jsh: main.c commandes_internes.c formatage_prompt.c mystring.c gestion_jobs.c redirections.c arriere_plan.c utils.c signaux.c
	$(CC) $(CFLAGS)  -o jsh main.c commandes_internes.c formatage_prompt.c mystring.c gestion_jobs.c redirections.c arriere_plan.c  utils.c signaux.c $(LIBS)

run: jsh
	./jsh

clean:
	rm -f jsh
