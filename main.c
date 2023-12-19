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

#define NBR_MAX_ARGUMENTS 20




int main()
{

    // Nettoyer le terminal
    system("clear");

    // Rediriger la sortie de readline vers stderr
    rl_outstream = stderr;
    struct Job tab_jobs[20];//tableau de jobs
    char *buf, *affiche, *commande, *argument;
    int code_retour = 0, status;
    char *args[NBR_MAX_ARGUMENTS];
    size_t len;
    int pid, nb_job =0;
    char *rep_precedent = malloc(sizeof(char) * PATH_MAX);

    getcwd(rep_precedent, sizeof(char) * PATH_MAX);

    while (1)
   {    // faut faire ici une maj du tableau des jobs 
        maj_jobs(tab_jobs,nb_job);


        // Afficher le prompt et lire la commande de l'utilisateur
        affiche = afficher_prompt();
        buf = readline(affiche);
        free(affiche);

        // Vérifier si la commande est NULL (par exemple, fin de fichier)
        if (buf == NULL)
        {
            return code_retour;
            break;
        }
        else
        {
            len = strlen(buf);
            // on vérifie si on a bien une commande en entrée et non  pas une ligne vide
            if (len != 0) 
            {

                //enlever le saut de ligne causé parn Entrer du clavier
                if (len > 0 && buf[len - 1] == '\n')

                    buf[len - 1] = '\0';

                // Extraire la commande et les arguments
                commande = strtok(buf, " ");
                int i = 0;
                args[i] = strdup(commande);
                i++;
                while ((argument = strtok(NULL, " ")) != NULL && i < NBR_MAX_ARGUMENTS)
                {
                    args[i] = strdup(argument);
                    i++;
                }
                args[i] = NULL;

                if (commande != NULL)
                {
                    if(is_cmdArrierePlan(args,i)){
                        i = modifie_args(args,i);
                        code_retour=cmdArrierePlan (args,nb_job,tab_jobs,i,len);
                        nb_job++;
                        printf("job cmd %s\n",(tab_jobs[0]).etat);
                        
                    }
                    else{
                    if (strcmp(commande, "pwd") == 0)
                    {
                        // Exécuter la commande pwd
                        // verifier si cette commande n'a pas d'arguments en entrée
                        //sinon la commande est incorrecte
                        
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
                    else if(strcmp(commande, "jobs") == 0){
                      if ((code_retour = jobs( tab_jobs,nb_job)) == 1) perror("Erreur lors de la commande JOBS");
                    }
                    else if (strcmp(commande, "?") == 0)
                    {
                        // Afficher le code de retour
                        // verifier si cette commande n'a pas d'arguments en entrée
                        //sinon la commande est incorrecte

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
                    else if (strcmp(commande, "exit") == 0)
                    {
                           // Sortir du programme avec un code de retour optionnel

                            if (is_stopped(tab_jobs, nb_job) || is_running(tab_jobs,nb_job)) {

                                printf("Avertissement : Des jobs sont en cours d'exécution ou suspendus.\n");  

                             } else {

                                if (args[1] != NULL)  code_retour = atoi(args[1]);
                                printf("argumet %d",code_retour) ;
                                free(rep_precedent);
                                exit(code_retour);
                             }
                            
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
                            execvp(commande, args);
                            perror("Erreur lors de l'exécution de la commande");
                            exit(3); // Valeur de sortie arbitraire en cas d'erreur
                            break;
                        default:
                            // Code du processus parent : attendre que le processus fils se termine
                            waitpid(pid, &status, 0);
                            code_retour = WEXITSTATUS(status);
                            break;
                        }
                    }
                    }
                }

                // Libérer la mémoire allouée pour les arguments
                for (int j = 0; j < i; j++)
                {
                    free(args[j]);
                }
            }

            // Libérer la mémoire allouée pour la commande
            free(buf);
           
        }
    }

    // Libérer la mémoire allouée pour le répertoire précédent
    free(rep_precedent);
    for (int i = 0; i < nb_job; i++) {
    liberer_job(&tab_jobs[i]);
    }


    return 0;
}
