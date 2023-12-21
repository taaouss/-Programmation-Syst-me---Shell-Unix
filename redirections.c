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

const char mes_symboles[7][4] = {"<", "2>>", ">>", "2>|", "2>", ">|", ">"};

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

int token_is_redirection(char *token)
{
    // verifie si le token est un symbole de redirection
    for (int i = 0; i < 7; i++)
    {
        if (strcmp(token, mes_symboles[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

int commandline_is_redirection(char *commandline)
{
    // verifie si la ligne de commande contient un symbole de redirection
    // renvoie l'indice du premier symbole de redirection trouvé
    int index = -1;
    char *commandline_tmp = strdup(commandline);
    char *token = strtok(commandline_tmp, " ");
    int token_length = 0;

    while (token != NULL)
    {
        if (token_is_redirection(token))
        {
            index = token_length;
            break;
        }
        token_length += strlen(token) + 1; // +1 pour l'espace
        token = strtok(NULL, " ");
    }

    free(commandline_tmp);
    return index;
}

void extract_redirections(char *commandline, Redirection **redirections, int *erreur, int *nb_redirections)
{
    char *file_name;
    char *commandline_tmp = strdup(commandline);
    char *token = strtok(commandline_tmp, " ");
    // int i = 0;
    *erreur = 0;
    *nb_redirections = 0;
    int capacity = 10;

    // Allouer de la mémoire pour le tableau de redirections
    *redirections = (Redirection *)malloc(sizeof(Redirection) * capacity);
    if (!*redirections)
    {
        *erreur = 1;
        return;
    }

    while (token != NULL)
    {
        if (token_is_redirection(token))
        {
            file_name = strtok(NULL, " ");

            // Vérifier si le nom du fichier n'est pas une redirection
            if (file_name == NULL || token_is_redirection(file_name))
            {
                *erreur = 1;
                break;
            }

            if (*nb_redirections >= capacity)
            {
                capacity *= 2;
                *redirections = realloc(*redirections, capacity * sizeof(Redirection));
                if (!*redirections)
                {
                    *erreur = 1;
                    break;
                }
            }

            // Sauvegarder la redirection et le nom du fichier
            (*redirections)[*nb_redirections].redirection = strdup(token);
            (*redirections)[*nb_redirections].redirectionFileName = strdup(file_name);
            (*nb_redirections)++;

            token = strtok(NULL, " "); // Passer au token suivant après le nom du fichier

            // verifier si le token suivant est un symbole de redirection (ex: cmd >| fic fic2)
            if (token != NULL && !token_is_redirection(token))
            {
                *erreur = 1;
                break;
            }
        }

        else
        {
            token = strtok(NULL, " "); // Pas une redirection, passer au token suivant
        }
    }
    if (*erreur && (*redirections != NULL))
    {
        for (int j = 0; j < *nb_redirections; j++)
        {
            free((*redirections)[j].redirection);
            free((*redirections)[j].redirectionFileName);
        }
        free(*redirections);
        *redirections = NULL;
    }
    free(commandline_tmp);
    return;
}

// Fonction pour extraire la commande et les arguments
char *extractCommandAndArgs(const char *commandLine, int index)
{
    char *result = NULL;

    if (index != -1)
    {
        // Allouer de la mémoire pour la sous-chaîne
        result = (char *)malloc(sizeof(char) * (index + 1));
        if (result == NULL)
        {
            // Gestion de l'erreur d'allocation
            perror("Échec de l'allocation de mémoire");
            return NULL;
        }
        // Copier la sous-chaîne
        strncpy(result, commandLine, index);
        // Ajouter le caractère de fin de chaîne
        result[index] = '\0';
    }
    else
    {
        // Si pas de redirection, copier toute la ligne de commande
        result = strdup(commandLine);
        if (result == NULL)
        {
            // Gestion de l'erreur d'allocation
            perror("Échec de l'allocation de mémoire");
            return NULL;
        }
    }
    free(commandLine);
    return result;
}

int execute_redirection(char *redirection, char *redirectionFileName)
{
    int code_retour = 0;
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

void free_redirections(Redirection *redirections, int nb_redirections)
{
    for (int i = 0; i < nb_redirections; i++)
    {
        free(redirections[i].redirection);
        free(redirections[i].redirectionFileName);
    }
    free(redirections);
    return;
}

int execute_redirections(Redirection *redirections, int nb_redirections)
{
    int code_retour = 0;
    int stdin_copy = dup(0);
    int stdout_copy = dup(1);
    int stderr_copy = dup(2);
    for (int i = 0; i < nb_redirections; i++)
    {
        code_retour = execute_redirection(redirections[i].redirection, redirections[i].redirectionFileName);
        if (code_retour != 0)
        {
            // printf("Erreur in\n");
            // printf("redirection: %s, file: %s\n", redirections[i].redirection, redirections[i].redirectionFileName);
            // printf("code retour: %d\n", code_retour);
            reset_redirections(stdin_copy, stdout_copy, stderr_copy);
            free_redirections(redirections, nb_redirections);
            return code_retour;
        }
    }
    free_redirections(redirections, nb_redirections);
    return code_retour;
}

// reset les redirections
void reset_redirections(int stdin_copy, int stdout_copy, int stderr_copy)
{
    dup2(stdin_copy, 0);
    dup2(stdout_copy, 1);
    dup2(stderr_copy, 2);
}
