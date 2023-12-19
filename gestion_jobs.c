#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "gestion_jobs.h"
#include <stdbool.h>
#include <sys/wait.h>
const char *etat_str[] = { "RUNNING", "STOPPED", "DETACHED", "KILLED", "DONE"};



int jobs(struct Job* jobs, int nbr_jobs) {
    int i = 0;
    if (jobs == NULL) return 0;

    while (i < nbr_jobs) {
       
            printf("[%d] %d %s %s\n", jobs[i].numero_job, jobs[i].processus[0], jobs[i].etat, jobs[i].command);
  
        i++;
    }
    return 1;
}


struct Job* creer_jobs(int nombre_jobs, pid_t processus, char* commande) {
    // Incrementer le nombre de job avant l'appel à cette fonction

    struct Job* resultat = malloc(sizeof(struct Job));
    if (resultat == NULL) {
        // Gérer l'échec de l'allocation mémoire
        return NULL;
    }
    
    resultat->numero_job = nombre_jobs;
    strncpy(resultat->command, commande, MAX_COMMAND_LENGTH - 1);
    resultat->command[MAX_COMMAND_LENGTH - 1] = '\0';
    resultat->nbr_processus = 1;
    strcpy(resultat->etat,etat_str[0]); // running
    //resultat->processus = malloc(NBR_MAX_PROCESSUS * sizeof(pid_t));
    resultat->processus[0] = processus; // un processus

    return resultat;
}

void maj_jobs(struct Job* jobs, int nbr_jobs) {
    bool tous_finis, termine_signal, detache;
    int termine_stop;
    int i = 0, terminated_count = 0;
    if (jobs == NULL) return;
    while (i < nbr_jobs) {
        tous_finis = true;
        termine_signal = false;
        termine_stop = 0;
        terminated_count = 0;
        detache = false;
        
        if (strcmp(jobs[i].etat, etat_str[4]) != 0) { // Le job n'est pas done

            for (int j = 0; j < jobs[i].nbr_processus; j++) {
               // printf("lyes2sshs %s \n",jobs[i].command);
                // On va parcourir les processus
                pid_t processus = jobs[i].processus[j];
                int status;
                int resultat = waitpid(processus, &status, WNOHANG | WUNTRACED);
                if (resultat == 0) {
                    // Le processus n'a pas encore terminé
                    tous_finis = false;
                } else if (resultat > 0) { // Processus a fini
                    if (WIFEXITED(status)) { // Soit done soit detached
                        terminated_count++;

                        // TODO: Vérifier si les processus non lancés du shell ont terminé
                        pid_t pgid = getpgid(processus);
                        int status2;
                        int resultat2 = waitpid(-pgid, &status2, WNOHANG);

                        if (resultat2 == -1) {
                            perror("waitpid");
                            exit(EXIT_FAILURE);
                        } else if (resultat2 == 0) {
                            // Au moins un processus du groupe est encore en cours d'exécution.
                            detache = true;
                        }

                    } else {
                        // Le processus a été tué par un signal
                        if (WIFSTOPPED(status)) {
                            termine_stop++;
                        } else if (WIFSIGNALED(status)) {
                            termine_signal = true; // killed
                        }
                    }
                }
            }

            if (tous_finis) {
                if (termine_signal) { // killed
                    strcpy(jobs[i].etat, etat_str[3]);
                } else if (detache) { // detached
                    strcpy(jobs[i].etat, etat_str[2]);
                } else if (termine_stop == (jobs[i].nbr_processus - terminated_count)) { // stopped
                    strcpy(jobs[i].etat, etat_str[1]);
                } else {
                    // done, ne doit pas y'avoir de detached
                    strcpy(jobs[i].etat, etat_str[4]);
                }
            } else { // !(tous_finis)
                // running
                strcpy(jobs[i].etat, etat_str[0]);
            }
        }

        i++;
    }
}

int is_stopped(struct Job* jobs, int nbr_jobs){
   int i =0;
   while (i < nbr_jobs) {
      if (strcmp(jobs[i].etat, etat_str[1]) == 0) return 0;
      i++;
    
    }
   return 1;
}

int is_running(struct Job* jobs, int nbr_jobs){
   int i =0;
   while (i < nbr_jobs) {
      if (strcmp(jobs[i].etat, etat_str[0]) == 0) return 0;
      i++;
    
    }
   return 1;

}
