#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <wait.h>
#include <limits.h>
#include <stdio.h>
#include "redirections.h"

char mes_symboles[7][4] = {"<", ">", ">|", ">>", "2>>", "2>", "2>|"};

// cmd < fic
int lecture(char *fic)
{
    int fd = open(fic, O_RDONLY);
    if (fd == -1)
    {
        return 1;
    }
    dup2(fd, 0); // redirection de l'entrée standard vers le fichier
    return 0;
}

// cmd > fic
int sans_ecrasement_stdout(char *fic)
{
    int fd = open(fic, O_CREAT | O_EXCL | O_WRONLY, 0666);
    if (fd == -1)
    {
        if (errno == EEXIST)
        {
            write(2, "le fichier existe déjà\n", strlen("le fichier existe déjà\n")); // 2 pour stderr
        }
        return 1;
    }
    else
    {
        dup2(fd, 1); // redirection de la sortie standard vers le fichier
        return 0;
    }
}

// cmd >| fic
int avec_ecrasement_stdout(char *fic)
{
    int fd = open(fic, O_CREAT | O_TRUNC | O_WRONLY, 0666);
    if (fd == -1)
    {
        return 1;
    }
    dup2(fd, 1);
    return 0;
}

// cmd >> fic
int concat_stdout(char *fic)
{
    int fd = open(fic, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd == -1)
    {
        return 1;
    }
    dup2(fd, 1);
    return 0;
}

// cmd 2>> fic
int concat_stderr(char *fic)
{
    int fd = open(fic, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd == -1)
    {
        return 1;
    }
    dup2(fd, 2);
    return 0;
}

// cmd 2> fic
int sans_ecrasement_stderr(char *fic)
{
    int fd = open(fic, O_CREAT | O_EXCL | O_WRONLY, 0666);
    if (fd == -1)
    {
        if (errno == EEXIST)
        {
            write(2, "le fichier existe déjà\n", strlen("le fichier existe déjà\n")); // 2 pour stderr
        }
        return 1;
    }
    else
    {
        dup2(fd, 2); // redirection de la sortie standard vers le fichier
        return 0;
    }
}

// cmd 2>| fic
int avec_ecrasement_stderr(char *fic)
{
    int fd = open(fic, O_CREAT | O_TRUNC | O_WRONLY, 0666);
    if (fd == -1)
    {
        return 1;
    }
    dup2(fd, 2);
    return 0;
}

void is_redirection(char *commandline, int *index, char **redirection)
{
    // parcoure la ligne de commande et renvoie l'index du symbole de redirection s'il y en a un
    // sinon renvoie -1
    int i = 0;
    char *redirectiontmp = NULL;
    char *commandline_tmp = strdup(commandline);
    while (i < 7)
    {
        redirectiontmp = strstr(commandline_tmp, mes_symboles[i]);
        if (redirectiontmp != NULL)
        {
            // *index = redirectiontmp - commandline;
            *index = strlen(commandline) - strlen(redirectiontmp);
            *redirection = strtok(redirectiontmp, " ");
            if (*redirection != NULL)
            {
                return;
            }
        }
        i++;
    }
    *index = -1;
    *redirection = NULL;
    free(commandline_tmp);
}

// Fonction pour extraire la commande et les arguments
char *extractCommandAndArgs(const char *commandLine, int index)
{
    char *result;

    if (index != -1)
    {
        // Allouer de la mémoire pour la sous-chaîne
        result = (char *)malloc(sizeof(char) * (index + 1));
        // Copier la sous-chaîne
        strncpy(result, commandLine, index);
        // Ajouter le caractère de fin de chaîne
        result[index] = '\0';
    }
    else
    {
        // Si pas de redirection, copier toute la ligne de commande
        result = strdup(commandLine);
    }

    return result;
}

// Extraction du nom du fichier de redirection
char *extractRedirectionFileName(const char *commandLine, int index)
{
    // Allouer de la mémoire pour la sous-chaîne
    int length = strlen(commandLine) - index;
    char *result = (char *)malloc(length + 1); // +1 pour le caractère nul
    if (!result)
    {
        return NULL; // Échec de l'allocation
    }
    // Copier la sous-chaîne
    strncpy(result, commandLine + index, length);
    result[length] = '\0'; // Ajouter le caractère de fin de chaîne

    // Vérifier si plusieurs fichiers sont spécifiés
    char *resulttmp = strdup(result);
    if (!resulttmp)
    {
        free(result);
        return NULL; // Échec de l'allocation
    }

    char *token = strtok(resulttmp, " ");
    int cpt = 0;
    while (token != NULL)
    {
        cpt++;
        token = strtok(NULL, " ");
    }
    free(resulttmp); // Libérer la mémoire allouée par strdup

    if (cpt == 1)
    {
        return result; // Un seul fichier spécifié
    }
    else
    {
        free(result); // Plusieurs fichiers spécifiés ou aucun
        return NULL;
    }
}

int execute_redirection(char *redirection, char *redirectionFileName)
{
    int code_retour;
    if (strcmp(redirection, "<") == 0)
    {
        code_retour = lecture(redirectionFileName);
    }
    else if (strcmp(redirection, ">") == 0)
    {
        code_retour = sans_ecrasement_stdout(redirectionFileName);
    }
    else if (strcmp(redirection, ">|") == 0)
    {
        code_retour = avec_ecrasement_stdout(redirectionFileName);
    }
    else if (strcmp(redirection, ">>") == 0)
    {
        code_retour = concat_stdout(redirectionFileName);
    }
    else if (strcmp(redirection, "2>>") == 0)
    {
        code_retour = concat_stderr(redirectionFileName);
    }
    else if (strcmp(redirection, "2>") == 0)
    {
        code_retour = sans_ecrasement_stderr(redirectionFileName);
    }
    else if (strcmp(redirection, "2>|") == 0)
    {
        code_retour = avec_ecrasement_stderr(redirectionFileName);
    }
    else
    {
        code_retour = 1;
    }
    return code_retour;
}

// reset les redirections
void reset_redirections(int stdin_copy, int stdout_copy, int stderr_copy)
{
    dup2(stdin_copy, 0);
    dup2(stdout_copy, 1);
    dup2(stderr_copy, 2);
}