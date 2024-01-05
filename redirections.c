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
#include "gestion_jobs.h"
#include "utils.h"
#include "commandes_internes.h"
#include "signaux.h"

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
char *extractCommandAndArgs(char *commandLine, int index)
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

int commandline_is_pipe(char *commandline)
{
    // Verifie si la ligne de commande contient un pipe
    // Renvoie 1 si la ligne de commande contient un/des pipe(s) et bien formé(s)
    // Renvoie 0 sinon

    char *commandline_tmp = strdup(commandline);
    char *token = strtok(commandline_tmp, " ");
    int code_retour = 0;

    while (token != NULL)
    {
        if (strcmp(token, "|") == 0)
        {
            if ((token = strtok(NULL, " ")) == NULL || strcmp(token, "|") == 0)
            {
                free(commandline_tmp);
                return 0;
            }
            code_retour = 1;
        }
        else
        {
            token = strtok(NULL, " ");
        }
    }
    free(commandline_tmp);
    return code_retour;
}

void extract_pipe_commands(char *commandline, char *commands[], int *nb_commands)
{

    char *commandline_tmp = strdup(commandline);
    char *token, *reste = commandline_tmp;
    int i = 0;

    while ((token = strstr(reste, " | ")) != NULL && i < NBR_MAX_PROCESSUS)
    {
        *token = '\0'; // Remplacer "  |  " par '\0' pour terminer la chaîne actuelle
        commands[i++] = strdup(reste);
        reste = token + 3; // Passer au caractère suivant après " | "
    }

    // Ajouter le dernier segment s'il existe
    if (*reste != '\0' && i < NBR_MAX_PROCESSUS)
    {
        commands[i++] = strdup(reste);
    }

    *nb_commands = i;
    free(commandline_tmp);
}

void free_elements(CommandElement elements[], int num_elements)
{
    for (int i = 0; i < num_elements; i++)
    {
        free(elements[i].content);
    }
}

int execute_pipes(char *commandline, char *rep_precedent)

{
    char *pipe_commands[MAX_SUBCOMMANDS];
    int nb_pipe_commands = 0;
    int in_fd = STDIN_FILENO; // Pour le premier pipe, on lit depuis l'entrée standard
    char *args[NBR_MAX_ARGUMENTS];
    char *commande;
    int i = 0;
    int index;
    int error_in_redirections = 0;
    int nb_redirections = 0;
    int code_retour = 0;
    Redirection *redirections;
    pid_t pid;
    int status_cmd_externe;

    extract_pipe_commands(commandline, pipe_commands, &nb_pipe_commands);
    int pipefd[2 * (nb_pipe_commands - 1)]; // Tableau pour stocker les descripteurs de fichiers des pipes
    for (int j = 0; j < nb_pipe_commands; j++)
    {
        if (j < nb_pipe_commands - 1) // Pas besoin de pipe pour la dernière commande
        {
            if (pipe(pipefd + j * 2) < 0) // Création du pipe
            {
                perror("Erreur lors de la création du pipe");
            }
        }

        pid_t pipe_pid = fork(); // Création du processus pour exécuter la commande

        if (pipe_pid < 0)
        {
            perror("Erreur lors de la création du processus fils");
        }

        else if (pipe_pid == 0)
        {
            // printf("Processus fils %d\n", getpid());
            if (in_fd != 0) // Si ce n'est pas la première commande
            {
                dup2(in_fd, 0); // Redirige stdin vers l'extrémité de lecture du pipe précédent
                close(in_fd);
            }

            if (j < nb_pipe_commands - 1) // Pas besoin de pipe pour la dernière commande
            {
                close(pipefd[j * 2]);
                dup2(pipefd[j * 2 + 1], 1);
                close(pipefd[j * 2 + 1]);
            }
            // Pour l'instant on duplique le code

            extract_args(strdup(pipe_commands[j]), args, &commande, &pipe_commands[j], &i, strlen(pipe_commands[j]));

            // fprintf(stderr, "commande extraite: %s\n", commande);
            // Vérifier si la commande est une redirection
            index = commandline_is_redirection(pipe_commands[j]);
            error_in_redirections = 0;
            if (index != -1)
            {
                extract_redirections(pipe_commands[j], &redirections, &error_in_redirections, &nb_redirections);
                if (error_in_redirections == 0)
                {
                    error_in_redirections = execute_redirections(redirections, nb_redirections);
                    code_retour = error_in_redirections;
                    pipe_commands[j] = extractCommandAndArgs(pipe_commands[j], index);
                    extract_args(strdup(pipe_commands[j]), args, &commande, &pipe_commands[j], &i, strlen(pipe_commands[j]));
                }
            }

            if (error_in_redirections != 0)
            {
                perror("Erreur lors de la redirection\n");
                code_retour = 1;
            }

            else //(error_in_redirections == 0)
            {
                // fprintf(stderr, "commande : %s\n", commande);
                if (strcmp(commande, "pwd") == 0)
                {
                    if (args[1] == NULL)
                    {
                        if ((code_retour = pwd()) != 0)
                            perror("Erreur lors de l'exécution de pwd\n");
                    }
                    else
                    {
                        // printf("args[1] = %s\n", args[1]);
                        perror("Commande incorrecte \n");
                        code_retour = 1;
                    }
                }
                else if (strcmp(commande, "?") == 0)
                {
                    if (args[1] == NULL)
                    {
                        printf("%d\n", code_retour);
                        code_retour = 0;
                    }
                    else
                    {
                        perror("Commande incorrecte \n");
                        code_retour = 1;
                    }
                }
                else if (strcmp(commande, "cd") == 0)
                {
                    // fprintf(stderr, "cd\n");
                    // fprintf(stderr, "args[1] : %s\n", args[1]);
                    if ((code_retour = cd(args[1], rep_precedent)) != 0)
                        perror("Erreur lors de la commande cd");
                }
                else
                {
                    // Créer un nouveau processus pour exécuter la commande externe

                    pid = fork();

                    switch (pid)
                    {
                    case -1:
                        perror("Erreur lors de la création du processus fils");
                        break;
                    case 0:
                        // Code du processus fils : exécuter la commande externe
                        setpgid(getpid(), getpid()); // Mettre le processus fils dans un nouveau groupe de processus
                        reset_signaux_groupe(getpid());
                        execvp(commande, args);
                        perror("Erreur lors de l'exécution de la commande");
                        exit(3); // Valeur de sortie arbitraire en cas d'erreur
                        break;
                    default:
                        // Code du processus parent : attendre que le processus fils se termine
                        tcsetpgrp(STDIN_FILENO, pid);
                        do
                        {
                            waitpid(pid, &status_cmd_externe, WUNTRACED | WCONTINUED);
                        } while (!(WIFEXITED(status_cmd_externe)) && !(WIFSIGNALED(status_cmd_externe)) && !(WIFSTOPPED(status_cmd_externe)) && !(WIFCONTINUED(status_cmd_externe)));

                        // Restaurer le contrôle au shell JSH
                        tcsetpgrp(STDIN_FILENO, getpgrp());

                        if (WIFSTOPPED(status_cmd_externe))
                        {
                            // int recu = WSTOPSIG(status_cmd_externe);
                            // if (recu == 19 || recu == 20)
                            // {
                            //     new_job = creer_jobs(nb_job, pid, pipe_commands[j], args, 1);
                            //     strcpy(new_job->etat, etat_str[1]);
                            //     new_job->affiche = 1;
                            //     new_job->avant = 1;
                            //     tab_jobs[nb_job] = *new_job;
                            //     free(new_job);
                            //     nb_job++;
                            // }
                        }

                        code_retour = WEXITSTATUS(status_cmd_externe);
                        break;
                    }
                }
            }
            // fprintf(stderr, "-----------------------------------------\n");
            // fprintf(stderr, "Processus fils %d execute %s\n", getpid(), pipe_commands[j]);
            // fprintf(stderr, "code_retour de la commande executée par le fils: %d\n", code_retour);
            exit(code_retour);
        }
        else
        {
            if (in_fd != STDIN_FILENO)
            {
                close(in_fd);
            }

            if (j < nb_pipe_commands - 1)
            {
                close(pipefd[j * 2 + 1]);
                in_fd = pipefd[j * 2];
            }
        }
        // maj_jobs(tab_jobs, nb_job);
        // jobs_err(tab_jobs, nb_job);
    }
    int status;
    for (int i = 0; i < nb_pipe_commands; i++)
    {
        wait(&status);
    }
    if (WIFEXITED(status))
    {
        code_retour = WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status))
    {
        code_retour = WTERMSIG(status);
    }
    else if (WIFSTOPPED(status))
    {
        code_retour = WSTOPSIG(status);
    }
    else if (WIFCONTINUED(status))
    {
        code_retour = 0;
    }
    else
    {
        code_retour = 1;
    }
    return code_retour;
}

int extract_and_verify_subcommands(char *commandline, CommandElement elements[], int *num_elements, int *contains_substitution)
{
    char *commandline_tmp = strdup(commandline);
    char *token = strtok(commandline_tmp, " ");
    *num_elements = 0;
    *contains_substitution = 0;
    int in_subcommand = 0;
    char subcommand[MAX_COMMAND_LENGTH] = "";
    int nested_level = 0;

    while (token != NULL && *num_elements < MAX_ELEMENTS)
    {
        if (strcmp(token, "<(") == 0)
        {
            if (in_subcommand)
            {
                // Nested subcommand
                nested_level++;
                strcat(subcommand, " ");
                strcat(subcommand, token);
            }
            else
            {
                // Start of a new subcommand
                in_subcommand = 1;
                (*contains_substitution) = 1;
                subcommand[0] = '\0';
                // strcat(subcommand, token);
            }
        }
        else if (strcmp(token, ")") == 0)
        {
            if (in_subcommand)
            {
                if (nested_level > 0)
                {
                    // Inside a nested subcommand
                    nested_level--;
                    strcat(subcommand, " ");
                    strcat(subcommand, token);
                }
                else
                {
                    // End of the current subcommand
                    in_subcommand = 0;
                    elements[*num_elements].content = strdup(subcommand);
                    elements[*num_elements].type = 1;
                    (*num_elements)++;
                    nested_level = 0;
                }
            }
            else
            {
                // Normal case, just store the argument
                elements[*num_elements].content = strdup(token);
                elements[*num_elements].type = 0;
                (*num_elements)++;
            }
        }
        else
        {
            if (in_subcommand)
            {
                // Inside a subcommand, concatenate elements
                strcat(subcommand, subcommand[0] != '\0' ? " " : "");
                strcat(subcommand, token);
            }
            else
            {
                // Normal case, store the argument
                elements[*num_elements].content = strdup(token);
                elements[*num_elements].type = 0;
                (*num_elements)++;
            }
        }

        token = strtok(NULL, " ");
    }

    // If the last element is a subcommand, add it to the list
    if (in_subcommand)
    {
        elements[*num_elements].content = strdup(subcommand);
        elements[*num_elements].type = 1;
        (*num_elements)++;
    }

    return *contains_substitution;
}

int execute_subcommands(CommandElement elements[], int num_elements, int pipe_tmp[], int rec, char *commandline)
{

    char *args[NBR_MAX_ARGUMENTS];
    size_t len = 0;
    int cpt = 0;
    char *buf_tmp = NULL, *commande;
    char cmd_pipe[PATH_MAX];
    pid_t pid;
    Redirection *redirections = NULL;
    int nb_redirections = 0;
    int pipes[num_elements][2];

    // Création des processus fils avec les pipes
    for (int i = 0; i < num_elements - 1; i++)
    {

        if (elements[i + 1].type == 1)
        {

            if (pipe(pipes[i]) == -1)
            {
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            pid = fork();

            if (pid == -1)
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (pid == 0)
            { // Processus fils
                // Fermeture des pipes inutiles
                setpgid(getppid(), getppid());
                //  printf("lyessspid2 \n");
                for (int j = 0; j < i; j++)
                {
                    close(pipes[j][0]); // Fermeture du descripteur de lecture
                    close(pipes[j][1]); // Fermeture du descripteur d'écriture
                }

                if (dup2(pipes[i][1], 1) == -1)
                {
                    perror("dup2");
                    return 1;
                }

                close(pipes[i][0]);
                close(pipes[i][1]);
                int nb_elements_tmp = 0;
                CommandElement elements_tmp[MAX_ELEMENTS];
                int contains_substitution_tmp = 0;

                //  fprintf(stderr, "lyess %s\n", elements[i + 1].content);
                if (extract_and_verify_subcommands(elements[i + 1].content, elements_tmp, &nb_elements_tmp, &contains_substitution_tmp))
                {
                    int pipe_tmp[2];
                    if (pipe(pipe_tmp) == -1)
                    {
                        perror("pipe");
                        exit(EXIT_FAILURE);
                    }
                    if (!fork())
                        execute_subcommands(elements_tmp, nb_elements_tmp, pipe_tmp, 1, elements[i + 1].content);
                    else
                        wait(NULL);
                    close(pipe_tmp[1]);
                    strcpy(cmd_pipe, elements[i + 1].content);
                    extract_args(cmd_pipe, args, &commande, &buf_tmp, &cpt, len);
                    // free(cmd_pipe);

                    snprintf(args[1], sizeof(pipe_tmp[0]) + 9, "/dev/fd/%d", pipe_tmp[0]);
                    args[2] = NULL;
                }
                else
                {
                    strcpy(cmd_pipe, elements[i + 1].content);
                    extract_args(cmd_pipe, args, &commande, &buf_tmp, &cpt, len);
                    if (commandline_is_pipe(elements[i + 1].content))
                    {
                        // 2- Pipe
                        execute_pipes(elements[i + 1].content, commandline);
                        // fprintf(stderr, "fin \n");
                        exit(0);
                    }
                }
                //   fprintf(stderr, "exec %s \n",elements[i + 1].content);
                execvp(commande, args);
                perror("erreur");
            }
        }
    }
    if (pid != 0)
    {

        while (wait(NULL) != -1)
        {
        }
        for (int i = 0; i < num_elements - 1; i++)
        {
            close(pipes[i][1]); // Descripteur de lecture
        }
        cpt = 0;
        strcpy(cmd_pipe, elements[0].content);
        extract_args(elements[0].content, args, &commande, &buf_tmp, &cpt, len);
        for (int i = 0; i < num_elements - 1; i++)
        {
            args[i + cpt] = NULL;
            if (args[i + cpt] == NULL)
            {
                args[i + cpt] = malloc(sizeof(pipes[i][0]) + 9);
            }

            if (elements[i + 1].type == 1)
            {
                snprintf(args[i + cpt], sizeof(pipes[i][0]) + 9, "/dev/fd/%d", pipes[i][0]);
            }
            else
            {
                strcpy(args[i + cpt], elements[i + 1].content);
            }
        }
        args[cpt + num_elements - 1] = NULL;

        // for (int i = 0; i <= cpt + num_elements -1; i++)
        // {
        //    printf("lyess %s %d %d \n",args[i],i,cpt + num_elements);
        //   }

        if (rec == 1)
        {
            dup2(pipe_tmp[1], 1);
            close(pipe_tmp[0]);
            close(pipe_tmp[1]);
        }

        execvp(commande, args);
        perror("Erreur lors de l'exécution de la commande");
        exit(3);
    }

    return 1;
}

int redirections_with_substituions(const char *commandLine, char **extracted)
{
    const char *current = commandLine;
    int found = 0;

    while (*current != '\0')
    {
        if (*current == '<')
        {
            if (*(current + 1) == ' ' && *(current + 2) == '<' && *(current + 3) == '(' && *(current + 4) == ' ')
            {
                // Commencer l'extraction ici
                current += 5; // Ignorer "< <( "
                const char *start = current;
                int parenCount = 1; // Compter le niveau d'imbrication des parenthèses

                while (*current != '\0' && parenCount > 0)
                {
                    if (*current == '(')
                    {
                        parenCount++;
                    }
                    else if (*current == ')')
                    {
                        parenCount--;
                    }
                    current++;
                }

                if (parenCount == 0)
                {
                    int length = current - start - 1;        // Exclure la parenthèse fermante finale
                    *extracted = (char *)malloc(length + 1); // Allouer la mémoire pour la chaîne extraite
                    if (*extracted != NULL)
                    {
                        strncpy(*extracted, start, length);
                        (*extracted)[length] = '\0'; // Ajouter un caractère de fin de chaîne
                        found = 1;
                    }
                    break;
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        current++;
    }
    return found;
}