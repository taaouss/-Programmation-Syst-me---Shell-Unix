
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "commandes_internes.h"
#include "gestion_jobs.h"
#include "signaux.h"
#include <sys/types.h>
#include <sys/wait.h>


// Fonction pour afficher le répertoire de travail actuel 
int pwd()
{
    char cwd[PATH_MAX];
    // Utilise getcwd pour obtenir le chemin du répertoire de travail actuel
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        // Affiche le chemin
        printf("%s\n", cwd);
        return 0;
    }
    else
    {
        // En cas d'erreur
        perror("getcwd");
        return 1;
    }
}

// Fonction pour changer le répertoire de travail 
int cd(char *argument, char *rep_precedent)
{
    // Sauvegarde du répertoire de travail actuel avant tout changement
    char precedent_tmp[PATH_MAX];
    getcwd(precedent_tmp, sizeof(precedent_tmp));
    if (argument != NULL)
    {
        // Si l'argument est "-", revenir au répertoire précédent
        if (strcmp(argument, "-") == 0)
        {
            if (chdir(rep_precedent) != 0)
            {
                return 1;// En cas d'erreur
            }
        }
        else
        {
            // Changer vers le répertoire spécifié par l'argument
            if (chdir(argument) != 0)
            {
                return 1;// En cas d'erreur
            }
        }
    }
    else
    {
        // Pas d'argument, retour au répertoire personnel (HOME)
        if (chdir(getenv("HOME")) != 0)
        {
            return 1;// En cas d'erreur
        }
    }
    // Mise à jour la variable rep_precedent avec le répertoire précédent sauvgarder au debut de la fonction
    strcpy(rep_precedent, precedent_tmp);
    return 0;
}

int kill_commande(char **argument, int nbr_arguments, struct Job *jobs, int nbr_jobs) {
    int signal, p;

    if (nbr_arguments == 3) {  
        //il ya le signal comme argument
        memmove(argument[1], argument[1] +1, strlen(argument[1]));
        signal = atoi(argument[1] );
        
        if (signal <= 0) {
            fprintf(stderr, "Erreur : Signal non valide\n");
            return 1; 
        } 
    } else {
        //on met le signal par defaut
        signal = SIGTERM;
    }

    if (nbr_arguments == 3) {
        p = 2;
    } else {
        p = 1;
    }

    char *resultat = strstr(argument[p] , "%");
    if (resultat == NULL) {
        //ici on envoie le signal a un processus 

        pid_t pid = atoi(argument[p]);
        int i = 0;
        while (i < nbr_jobs) {
            if (strcmp(jobs[i].etat, etat_str[4]) != 0) { 
                // Le job n'est pas done car s'il est done, il est déjà tué
                for (int j = 0; j < jobs[i].nbr_processus; j++) {

                    if (jobs[i].processus[j] == (pid_t)pid) {
                        int result =kill(jobs[i].processus[j], signal);
                        if (result != 0){
                        // La fonction kill a échoué
                        perror("kill"); // Affiche un message d'erreur lié à errno
                        printf("Échec de la tentative de tuer le processus. Code de retour : %d\n", result);
                        }
                    }
                }
            }
        i++;
       
        }
     
     } else {
        
        //ici on envoit le signal kill a un job 
        memmove(resultat, resultat + 1, strlen(resultat));
        int num = atoi(resultat);
        if (num > 0 && (num - 1) <= nbr_jobs) { 
            if (strcmp(jobs[num - 1].etat, etat_str[4]) != 0) {
                for (int j = 0; j < jobs[num - 1].nbr_processus; j++) {

                   int result= kill(-jobs[num - 1].processus[j], signal);
                
                      if (result != 0) {
                        // La fonction kill a échoué
                        perror("kill"); // Affiche un message d'erreur lié à errno
                        printf("Échec de la tentative de tuer le processus. Code de retour : %d\n", result);
                    }
                }
            }
            else {
                fprintf(stderr, "Erreur : Le job est déjà terminé (done)\n");
                return 1;
            }
        }
        else {    
            fprintf(stderr, "Erreur : Numéro de job incorrect\n");
            return 1;
        }
    }
    return 0;
}

int fg_commande(struct Job* jobs, int nbr_jobs, char* arg){

    //extraction de l'argument
    char *resultat = strstr(arg , "%");
    memmove(resultat, resultat + 1, strlen(resultat));
    int job_number= atoi(resultat);

    //verifier si job_number est correct
    if(job_number < 0 || job_number > nbr_jobs){

        fprintf(stderr, "Numéro de job invalide\n");
        return 1;
    }

    //choisir le job dans le tableau 
    struct Job* job_fg = &jobs[job_number - 1];
    
    //verifier si le job est en arriere plan 
    if (job_fg->avant == 1)  {
        
        fprintf(stderr, "Le job est en avant plan deja \n");
         return 0; 
    }
     //le job doit etre running 
     if (strcmp(job_fg->etat, etat_str[0]) != 0) {
        fprintf(stderr, "Le job n'est pas en cours d'exécution\n");
        return 1 ;
    }

    //ramener un job en avant plan
    tcsetpgrp(STDIN_FILENO, getpgid(job_fg->processus[0]));

    waitpid(job_fg->processus[0], NULL, WUNTRACED | WCONTINUED);

    // Restaurer le contrôle au shell JSH
    tcsetpgrp(STDIN_FILENO, getpgrp());

    return 0;
}

 int bg_commande(struct Job* jobs, int nbr_jobs, char* arg){
      
    //extraction de l'argument
    char *resultat = strstr(arg , "%");
    memmove(resultat, resultat + 1, strlen(resultat));
    int job_number= atoi(resultat);

    //verifier si job_number est correct
    if(job_number < 0 || job_number > nbr_jobs){

        fprintf(stderr, "Numéro de job invalide\n");
        return 1;
    }

    //choisir le job dans le tableau 
    struct Job* job_fg = &jobs[job_number - 1];

    //verifier si le job a été lancé en avant plan 

    if (job_fg->avant == 0)  {
        
        fprintf(stderr, "Le job est en avant plan deja \n");
         return 0; 
    }
     //le job doit etre stopped
     if (strcmp(job_fg->etat, etat_str[1]) != 0) {
        fprintf(stderr, "Le job n'est pas stoopé \n");
        return 1 ;
    }
   
    // lancer le job en arriere plan 

}



