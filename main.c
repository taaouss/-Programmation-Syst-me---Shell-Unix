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
#include "gestion_jobs.h"
#include "arriere_plan.h"
#include "utils.h"
#include "redirections.h"
#include "signaux.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main()
{
    // ignorer les signaux par JSH
    set_signaux();

    // Nettoyer le terminal
    system("clear");

    // Rediriger la sortie de readline vers stderr
    rl_outstream = stderr;

    struct Job tab_jobs[20]; // tableau de jobs
    char *buf, *affiche, *commande, *buf_tmp = NULL;
    int code_retour = 0, status;
    char *args[NBR_MAX_ARGUMENTS];
    size_t len;
    int pid, nb_job = 0;
    char rep_precedent[100];
    int i = 0;

    // Variables pour les redirections
    int index = -1;
    Redirection *redirections = NULL;
    int nb_redirections = 0;
    int error_in_redirections = 0; // Pour verifier si erreur dans les fichiers de redirections

    // Variables pour la gestion des jobs
    struct Job *new_job;

    getcwd(rep_precedent, sizeof(char) * PATH_MAX);

    // Variables pour la substitution de processus
    int contains_substitution = 0;
    int nb_elements = 0;
    CommandElement elements[MAX_ELEMENTS];

    int stdin_copy = dup(STDIN_FILENO);
    int stdout_copy = dup(STDOUT_FILENO);
    int stderr_copy = dup(STDERR_FILENO);

    // Boucler jusqu'à la fin du shell
    while (1)
    {
        // mise à jour des jobs
        maj_jobs(tab_jobs, nb_job);

        // Afficher le prompt et lire la commande de l'utilisateur
        affiche = afficher_prompt(nb_jobs_encours(tab_jobs, nb_job));
        buf = readline(affiche);
        free(affiche);

        // Vérifier si la commande est NULL (par exemple, fin de fichier)
        if (buf == NULL || feof(stdin))
        {
            free(buf_tmp);
            return (code_retour);
        }

        len = strlen(buf);

        // on vérifie si on a bien une commande en entrée et non pas une ligne vide
        if (len == 0) // si on a une ligne vide on reboucle
        {
            free(buf);
            continue;
        }

        // Extraire la commande et les arguments
        extract_args(buf, args, &commande, &buf_tmp, &i, len);

        if (commande == NULL)
        {
            // Libérer la mémoire allouée pour les arguments
            for (int j = 0; j < i; j++)
            {
                free(args[j]);
            }
            // Libérer la mémoire allouée pour la commande
            free(buf);
            continue;
        }
        // Executer les conditions selon l'ordre de priorité des redirections et arrière plan
        // 0- Verfier si la commande est en arrière plan
        // 1- Substitution de processus
        // 2- Pipe
        // 3- Redirections

        // 0- Vérifier si la commande est en arrière-plan
        if (is_cmdArrierePlan(args, i))
        {
            i = modifie_args(args, i, &buf_tmp);

            // verifier si il y a des redirections avant de mettre en arriere plan
            index = commandline_is_redirection(buf_tmp);
            error_in_redirections = 0;
            if (index != -1)
            {
                extract_redirections(buf_tmp, &redirections, &error_in_redirections, &nb_redirections);
                if (error_in_redirections == 0)
                {
                    error_in_redirections = execute_redirections(redirections, nb_redirections);
                    code_retour = error_in_redirections;
                    buf_tmp = extractCommandAndArgs(buf_tmp, index);

                    extract_args(buf_tmp, args, &commande, &buf_tmp, &i, strlen(buf_tmp));
                }
            }
            if (error_in_redirections != 0)
            {
                perror("Erreur lors de la redirection\n");
                code_retour = 1;
            }

            // mettre en arriere plan
            code_retour = cmdArrierePlan(args, nb_job, tab_jobs, len, buf_tmp);
            nb_job++;
        }
        else if ((contains_substitution = extract_and_verify_subcommands(buf_tmp, elements, &nb_elements, &contains_substitution)))
        {
            // 1- Substitution de processus
            /* code */
            mkfifo("jobs", 0666);
            int r = fork();

            if (!r)
            {
                setpgid(getpid(), getpid());
                int pi[2];
                new_job = creer_jobs(nb_job, getpid(), buf_tmp, 1); // avant d 1
                tab_jobs[nb_job] = *new_job;
                free(new_job);
                nb_job++;
                execute_subcommands(elements, nb_elements, pi, 0, buf_tmp, tab_jobs, nb_job - 1);
                
            }
            else
            {

                int status;
                new_job = creer_jobs(nb_job, r, buf_tmp, 1); // avant d 1
                tab_jobs[nb_job] = *new_job;
                free(new_job);
                nb_job++;

                int fd_tube = open("jobs", O_RDONLY | O_NONBLOCK);
                // printf("chaalalla\n");
                waitpid(r, &status, WUNTRACED);
                // wait(&status);
                // printf("chaalalla\n");
                struct Job tab_jobs_tmp[20];
                
                int nb_car = read(fd_tube, tab_jobs_tmp, sizeof(tab_jobs_tmp));
               /* if (nb_car == 110)
                    tab_jobs[nb_job] = tab_jobs_tmp[nb_job];
                else{
                new_job = creer_jobs(nb_job, r, buf_tmp, 1); // avant d 1
                tab_jobs[nb_job] = *new_job;
                free(new_job);
                }
              nb_job++;*/

                if (WIFSTOPPED(status)) // on peut pas le mettre a jour avec la method maj_job car le processus avec le pid r va etre desparetre et supprimer de la table de processus
                {
                    strcpy(tab_jobs[nb_job - 1].etat, etat_str[1]);
                    tab_jobs[nb_job - 1].affiche = 1;
                    tab_jobs[nb_job - 1].avant = 0;
                    jobs_err(tab_jobs, nb_job);
                }
                if (WIFEXITED(status)) // on peut pas le mettre a jour avec la method maj_job car le processus avec le pid r va etre desparetre et supprimer de la table de processus
                {
                    // printf("lyess\n");
                    strcpy(tab_jobs[nb_job - 1].etat, etat_str[4]);
                }
                for (int i = 0; i < nb_elements; i++)
                {
                    if (elements != NULL)
                        free(elements[i].content);
                }
            }
        }
        else if (commandline_is_pipe(buf_tmp))
        {
            // 2- Pipe
            int r = fork();
            if (!r)
            {
                setpgid(getpid(), getpid());
                execute_pipes(buf_tmp, rep_precedent);
                exit(0);
            }
            else
            {
                new_job = creer_jobs(nb_job, r, buf_tmp, 1); // avant d 1
                tab_jobs[nb_job] = *new_job;
                free(new_job);
                nb_job++;
                int status;
                waitpid(r, &status, WUNTRACED);

                if (WIFSTOPPED(status)) // on peut pas le mettre a jour avec la method maj_job car le processus avec le pid r va etre desparetre et supprimer de la table de processus
                {
                    strcpy(tab_jobs[nb_job - 1].etat, etat_str[1]);
                    tab_jobs[nb_job - 1].affiche = 1;
                    tab_jobs[nb_job - 1].avant = 0;
                    jobs_err(tab_jobs, nb_job);
                }
                if (WIFEXITED(status)) // on peut pas le mettre a jour avec la method maj_job car le processus avec le pid r va etre desparetre et supprimer de la table de processus
                {
                    // printf("lyess\n");
                    strcpy(tab_jobs[nb_job - 1].etat, etat_str[4]);
                }
            }

            // strcpy(tab_jobs[nb_job-1].etat,etat_str[3]);
            //  tab_jobs[nb_job-1].avant = 1;
            //  tab_jobs[nb_job-1].affiche=0;

            // cette fonction est la duplication des commandes du main en attendant de faire une fonction
            // qui prend en paramètre la commande et les arguments et qui exécute la commande
        }
        else
        {
            // 3- Redirections
            // Dans ce cas une commande simple sans pipe ni arriere plan ni substitution
            // Donc une seule commande avec ou sans redirections

            index = commandline_is_redirection(buf_tmp);
            error_in_redirections = 0;
            if (index != -1)
            {
                extract_redirections(buf_tmp, &redirections, &error_in_redirections, &nb_redirections);
                if (error_in_redirections == 0)
                {
                    error_in_redirections = execute_redirections(redirections, nb_redirections);
                    code_retour = error_in_redirections;
                    buf_tmp = extractCommandAndArgs(buf_tmp, index);

                    extract_args(buf_tmp, args, &commande, &buf_tmp, &i, strlen(buf_tmp));
                }
            }
            if (error_in_redirections != 0)
            {
                perror("Erreur lors de la redirection\n");
                code_retour = 1;
            }
            else //(error_in_redirections == 0) par défaut, erreur_in_redirections est initialisé à 0
            {
                if (strcmp(commande, "pwd") == 0)
                {
                    // Exécuter la commande pwd
                    // verifier si cette commande n'a pas d'arguments en entrée
                    // sinon la commande est incorrecte

                    if (args[1] == NULL)
                    {
                        if ((code_retour = pwd()) != 0)
                            perror("Erreur lors de l'exécution de pwd\n");
                    }
                    else
                    {
                        perror("Commande incorrecte \n");
                    }
                }
                else if (strcmp(commande, "jobs") == 0)
                {
                    maj_jobs(tab_jobs, nb_job);
                    if ((code_retour = jobs(args, i, tab_jobs, nb_job)) == 1)
                        perror("Erreur lors de la commande JOBS");
                }
                else if (strcmp(commande, "?") == 0)
                {
                    // Afficher le code de retour
                    // verifier si cette commande n'a pas d'arguments en entrée
                    // sinon la commande est incorrecte

                    if (args[1] == NULL)
                    {
                        printf("%d\n", code_retour);
                        code_retour = 0;
                    }
                    else
                    {
                        perror("Commande incorrecte \n");
                    }
                }
                else if (strcmp(commande, "cd") == 0)
                {
                    // Exécuter la commande cd
                    if ((code_retour = cd(args[1], rep_precedent)) != 0)
                        perror("Erreur lors de la commande cd");
                }
                else if (strcmp(commande, "kill") == 0)
                {
                    // Exécuter la commande kill
                    if ((code_retour = kill_commande(args, i, tab_jobs, nb_job)) != 0)
                    {
                        perror("Erreur lors de la commande kill");
                    }
                    else
                    {
                        maj_jobs(tab_jobs, nb_job);
                        maj_jobs(tab_jobs, nb_job);
                    }
                }
                else if (strcmp(commande, "exit") == 0)
                {
                    // Sortir du programme avec un code de retour optionnel

                    if (is_stopped(tab_jobs, nb_job) || is_running(tab_jobs, nb_job))
                    {
                        fprintf(stderr, "exit\n");
                        fprintf(stderr, "There are stopped jobs\n");
                        code_retour = 1;
                    }
                    else
                    {
                        if (args[1] != NULL)
                            code_retour = atoi(args[1]);
                        exit(code_retour);
                    }
                }
                else if (strcmp(commande, "fg") == 0)
                {
                    if ((code_retour = fg_commande(tab_jobs, nb_job, args[1])) != 0)
                        perror("Erreur lors de la commande fg");
                }
                else if (strcmp(commande, "bg") == 0)
                {
                    if ((code_retour = bg_commande(tab_jobs, nb_job, args[1])) != 0)
                        perror("Erreur lors de la commande fg");
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
                            waitpid(pid, &status, WUNTRACED | WCONTINUED);
                        } while (!(WIFEXITED(status)) && !(WIFSIGNALED(status)) && !(WIFSTOPPED(status)) && !(WIFCONTINUED(status)));

                        // Restaurer le contrôle au shell JSH
                        tcsetpgrp(STDIN_FILENO, getpgrp());

                        if (WIFSTOPPED(status))
                        {
                            int recu = WSTOPSIG(status);
                            if (recu == 19 || recu == 20)
                            {
                                new_job = creer_jobs(nb_job, pid, buf_tmp, 1); // avant d 1
                                strcpy(new_job->etat, etat_str[1]);
                                new_job->affiche = 1;
                                new_job->avant = 0; /****************************************/
                                tab_jobs[nb_job] = *new_job;
                                free(new_job);
                                nb_job++;
                            }
                        }

                        code_retour = WEXITSTATUS(status);
                        break;
                    }
                }
            }

            maj_jobs(tab_jobs, nb_job);
            jobs_err(tab_jobs, nb_job);
        }

        // Libérer la mémoire allouée pour les arguments
        for (int j = 0; j < i; j++)
        {
            free(args[j]);
        }

        // Libérer la mémoire allouée pour la commande
        free(buf);

        // free(commande);
        // free(redirections);

        reset_redirections(stdin_copy, stdout_copy, stderr_copy);
    }

    // Libérer la mémoire allouée pour le répertoire précédent
    free(rep_precedent);
    free(buf_tmp);
    free(args);
    for (int i = 0; i < nb_job; i++)
    {
        liberer_job(&tab_jobs[i]);
    }

    return 0;
}
