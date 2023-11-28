#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include "mystring.h"
#include "commandes_internes.h"
#include "formatage_prompt.h"

// Définir la taille maximale pour le répertoire courant
#define MAX_CWD_SIZE 512

int main()
{
    system("clear");
    char *rep_precedent = malloc(sizeof(char) * PATH_MAX);
    getcwd(rep_precedent, sizeof(char) * PATH_MAX); // pour avoir le rep courant
    rl_outstream = stderr;
    char *buf;
    int code_retour = 0, status;
    char *args[20];
    while (1)
    {
        buf = readline(afficher_prompt());
        if (buf == NULL)
        {
            return code_retour;
            break;
        }
        else
        {
            size_t len = strlen(buf);
            if (len == 0)
            {
                continue;
            }
            else
            {
                if (len > 0 && buf[len - 1] == '\n')
                    buf[len - 1] = '\0';
                char *commande = strtok(buf, " ");
                char *argument;
                int i = 0;
                args[i] = strdup(commande);
                i++;
                while ((argument = strtok(NULL, " ")) != NULL && i < 19)
                {
                    args[i] = strdup(argument);
                    i++;
                }
                args[i] = NULL;
                if (commande != NULL)
                {
                    if (strcmp(commande, "pwd") == 0)
                    {
                        if (args[1] == NULL)
                        {
                            if ((code_retour = pwd()) != 0)
                                printf("Erreur pwd\n");
                        }
                        else
                        {
                            printf("pwd n'a pas d'argument\n");
                        }
                    }
                    else if (strcmp(commande, "?") == 0)
                    {
                        printf("%d\n", code_retour);
                        code_retour = 0;
                    }
                    else if (strcmp(commande, "cd") == 0)
                    {
                        if ((code_retour = cd(args[1], rep_precedent)) != 0)
                            perror("Erreur lors de la commande cd");
                    }
                    else if (strcmp(commande, "exit") == 0)
                    {
                        if (args[1] != NULL)
                        {
                            code_retour = atoi(args[1]);
                        }
                        free(rep_precedent);
                        exit(code_retour);
                    }
                    else
                    {
                        int pid = fork();
                        switch (pid)
                        {
                        case -1:
                            perror("Erreur lors de la création du processus fils");
                            break;
                        case 0:

                            execvp(commande, args);
                            perror("Erreur lors de l'exécution de la commande");
                            exit(3); // arrightid parceque ila9 adifagh le fils g la boucle while
                            break;
                        default:
                            waitpid(pid, &status, 0);
                            code_retour = WEXITSTATUS(status);

                            break;
                        }
                    }
                }
            }

            // add_history(buf);
        }
    }
    free(buf);
    free(rep_precedent);
    return 0;
}
