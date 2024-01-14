#include <linux/limits.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "gestion_jobs.h"
#include <stdbool.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include "signaux.h"
#include <dirent.h>
#include <fcntl.h>

const char *etat_str[] = { "RUNNING", "STOPPED", "DETACHED", "KILLED", "DONE"};



int jobs(char **argument, int nbr_arguments, struct Job* jobs, int nbr_jobs) {
    
    int i = 0;
    if (jobs == NULL) return 1;
    char etat[10];
    int p=nbr_arguments -1;


    char *resultat = strstr(argument[p] , "%");

    while (i < nbr_jobs) {
             
            strcpy(etat,jobs[i].etat);
            if (strcmp(jobs[i].etat,"DONE")==0) strcpy(etat,"Done");
            else if (strcmp(jobs[i].etat,"RUNNING")==0) strcpy(etat,"Running");
            else if (strcmp(jobs[i].etat,"STOPPED")==0) strcpy(etat,"Stopped");
            else if (strcmp(jobs[i].etat,"DETACHED")==0) strcpy(etat,"Detached");
            else if (strcmp(jobs[i].etat,"KILLED")==0) strcpy(etat,"Killed");    

            if (resultat !=NULL){
                        memmove(resultat, resultat + 1, strlen(resultat));
                        int num_pid = atoi(resultat);
                        if(i+1==num_pid){
                        printf("[%d]\t%d\t%s\t%s\n", jobs[i].numero_job + 1, jobs[i].processus[0], etat,jobs[i].command);
                            if (nbr_arguments >= 2 && strcmp(argument[1],"-t")==0)
                        {
                            get_child_processes(getpgid(jobs[i].processus[0]));
                        }
                        if ( jobs[i].affiche == 1)
                        {
                            jobs[i].affiche = 0;
                        }
                        break;
                        }
                        //else{
                          //  if (getpgid(jobs[i].processus[0]) == getpgid(num_pid))
                          //  {
                          //     printf("[%d]", jobs[i].numero_job + 1);
                           //    get_etat_processe_externe(resultat);
                           //    break;
                           // }
                            
                       // }
                        
                }


            else {if ((strcmp(jobs[i].etat,etat_str[3])!=0)) {
                
                

               if (!(strcmp(jobs[i].etat,etat_str[4])==0 && (jobs[i].affiche==0))){


                  
                             
                printf("[%d]\t%d\t%s\t%s\n", jobs[i].numero_job + 1, jobs[i].processus[0], etat,jobs[i].command);
                            
                   if (nbr_arguments >= 2 && strcmp(argument[1],"-t")==0)
                   {
                    get_child_processes(getpgid(jobs[i].processus[0]));
                   }
                    

                    if ( jobs[i].affiche == 1)
                    {
                        jobs[i].affiche = 0;
                    }
                }
               
                
            }}
  
        i++;
    }
    return 0;
}


// a enlever
void get_etat_processe_externe(char* pid){
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%s/stat", pid);
    int stat_fd = open(path, O_RDONLY);
            if (stat_fd != -1) {
                pid_t pid, ppid, gid;  // Ajout de gid
                char state, cmd[1024];
                ssize_t bytesRead = read(stat_fd, cmd, sizeof(cmd) - 1);
                close(stat_fd);

                // Vérifier bytesRead avant d'accéder à cmd
                if (bytesRead > 0) {
                    cmd[bytesRead] = '\0';
                   sscanf(cmd, "%d %*s %c %d %d ", &pid, &state, &ppid, &gid);
                   


                    // Construire le chemin complet du fichier cmdline
                    snprintf(path, PATH_MAX, "/proc/%d/cmdline", pid);

                    // Ouvrir le fichier cmdline avec open
                    int fd = open(path, O_RDONLY);
                    if (fd != -1) {
                        // Lire le contenu du fichier cmdline
                        bytesRead = read(fd, cmd, sizeof(cmd) - 1);
                        close(fd);

                        // Vérifier bytesRead avant d'accéder à cmd
                        if (bytesRead > 0) {
                            cmd[bytesRead] = '\0';
                            // Si le GID correspond au GID du processus parent, afficher les informations
                            
                                char state_str[15];
                                switch (state) {
                                    case 'R':
                                        strcpy(state_str, "Running");
                                        break;
                                    case 'S':
                                        strcpy(state_str, "Sleeping");
                                        break;
                                    case 'D':
                                        strcpy(state_str, "Disk Sleep");
                                        break;
                                    case 'Z':
                                        strcpy(state_str, "Zombie");
                                        break;
                                    case 'T':
                                        strcpy(state_str, "Stopped");
                                        break;
                                    default:
                                        strcpy(state_str, "Unknown");
                                }
                                state_str[strlen(state_str)]='\0';

                                printf("\t%d\t%s %s\n", pid, state_str, cmd);
                                // Ajoutez ici le code pour afficher d'autres informations selon vos besoins
                            
                        }
                    }
                }
            }

return;

}
void get_child_processes(pid_t parent_gid) {
    DIR *dir;
    struct dirent *entry;
    char path[PATH_MAX];

    // Ouvrir le répertoire /proc
    dir = opendir("/proc");
    if (dir == NULL) {
        perror("Erreur lors de l'ouverture du répertoire /proc");
        exit(EXIT_FAILURE);
    }

    // Parcourir les entrées du répertoire
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) {
            // Construire le chemin complet du fichier stat
            snprintf(path, PATH_MAX, "/proc/%s/stat", entry->d_name);

            // Ouvrir le fichier stat avec open
            int stat_fd = open(path, O_RDONLY);
            if (stat_fd != -1) {
                pid_t pid, ppid, gid;  // Ajout de gid
                char state, cmd[1024];
                ssize_t bytesRead = read(stat_fd, cmd, sizeof(cmd) - 1);
                close(stat_fd);

                // Vérifier bytesRead avant d'accéder à cmd
                if (bytesRead > 0) {
                    cmd[bytesRead] = '\0';
                   sscanf(cmd, "%d %*s %c %d %d ", &pid, &state, &ppid, &gid);
                   


                    // Construire le chemin complet du fichier cmdline
                    snprintf(path, PATH_MAX, "/proc/%d/cmdline", pid);

                    // Ouvrir le fichier cmdline avec open
                    int fd = open(path, O_RDONLY);
                    if (fd != -1) {
                        // Lire le contenu du fichier cmdline
                        bytesRead = read(fd, cmd, sizeof(cmd) - 1);
                        close(fd);

                        // Vérifier bytesRead avant d'accéder à cmd
                        if (bytesRead > 0) {
                            cmd[bytesRead] = '\0';
                            // Si le GID correspond au GID du processus parent, afficher les informations
                            if (gid == parent_gid && gid !=pid) {
                                char state_str[15];
                                switch (state) {
                                    case 'R':
                                        strcpy(state_str, "Running");
                                        break;
                                    case 'S':
                                        strcpy(state_str, "Sleeping");
                                        break;
                                    case 'D':
                                        strcpy(state_str, "Disk Sleep");
                                        break;
                                    case 'Z':
                                        strcpy(state_str, "Zombie");
                                        break;
                                    case 'T':
                                        strcpy(state_str, "Stopped");
                                        break;
                                    default:
                                        strcpy(state_str, "Unknown");
                                }
                                state_str[strlen(state_str)]='\0';

                                printf("\t|%d\t%s %s\n", pid, state_str, cmd);
                                // Ajoutez ici le code pour afficher d'autres informations selon vos besoins
                            }
                        }
                    }
                }
            }
        }
    }

    closedir(dir);
}


int jobs_err(struct Job* jobs, int nbr_jobs) {
    int i = 0;
    if (jobs == NULL) return 1;

    while (i < nbr_jobs) {
        if ((jobs[nbr_jobs-i-1].affiche == 1)&&  jobs[nbr_jobs-i-1].avant == 0)
        {
            
        
            jobs[nbr_jobs-i-1].affiche =0;
            if (strcmp(jobs[nbr_jobs-i-1].etat,"DONE")==0)
            {
                fprintf(stderr,"[%d]\t%d\tDone\t%s\n",jobs[i].numero_job + 1, jobs[i].processus[0], jobs[nbr_jobs-i-1].command);
            }else if (strcmp(jobs[nbr_jobs-i-1].etat,"RUNNING")==0)
            {
                fprintf(stderr,"[%d]\t%d\tRunning\t%s\n",jobs[i].numero_job + 1, jobs[i].processus[0], jobs[nbr_jobs-i-1].command);
            }
            else if (strcmp(jobs[nbr_jobs-i-1].etat,"KILLED")==0)
            {
                fprintf(stderr,"[%d]\t%d\tKilled\t%s\n",jobs[i].numero_job + 1, jobs[i].processus[0], jobs[nbr_jobs-i-1].command);
            }
            else if (strcmp(jobs[nbr_jobs-i-1].etat,"STOPPED")==0)
            {
                fprintf(stderr,"[%d]\t%d\tStopped\t%s\n",jobs[i].numero_job + 1, jobs[i].processus[0], jobs[nbr_jobs-i-1].command);
            }
        }             
  
        i++;
    }
    return 0;
}

int nb_jobs_encours(struct Job* jobs, int nbr_jobs) {
    int i = 0;
    if (jobs == NULL) return 0;
    int cpt =0;

    while (i < nbr_jobs) {
        if ((strcmp(jobs[i].etat,etat_str[4])!=0)&&(strcmp(jobs[i].etat,etat_str[3])!=0))
        {
            cpt++;
        }
        i++;
    }
    return cpt;
}

struct Job* creer_jobs(int nombre_jobs, pid_t processus, char* commande, int avant) {
   
    struct Job* resultat = malloc(sizeof(struct Job));
    if (resultat == NULL) {
        // Gérer l'échec de l'allocation mémoire
        return NULL;
    }
    resultat->numero_job = nombre_jobs;
    resultat->affiche=0;
    strncpy(resultat->command, commande, MAX_COMMAND_LENGTH - 1);
    resultat->command[MAX_COMMAND_LENGTH - 1] = '\0';
    resultat->nbr_processus = 1;
    resultat->avant = avant ;
    strcpy(resultat->etat,etat_str[0]); // running
    resultat->processus[0] = processus; 

    return resultat;
}

void maj_jobs(struct Job* jobs, int nbr_jobs) {
    bool tous_finis, termine_signal, detache, run_again;
    int termine_stop;
    int i = 0, terminated_count = 0;
    if (jobs == NULL) return;
   
    while (i < nbr_jobs) {
        tous_finis = true;
        termine_signal = false;
        termine_stop = 0;
        terminated_count = 0;
        detache = false;
        run_again =false ;
        if ((strcmp(jobs[i].etat, etat_str[4]) != 0) && (strcmp(jobs[i].etat, etat_str[3]) != 0)){ // Le job n'est pas done et n'est pas killed

            for (int j = 0; j < jobs[i].nbr_processus; j++) {
                // On va parcourir les processus de chaque Job

                pid_t processus = jobs[i].processus[j];
                pid_t pgid = getpgid(processus);
                int status;
                int resultat = waitpid(processus, &status, WNOHANG | WUNTRACED| WCONTINUED);
                if (resultat == 0) {
                    //le processus est encours d'execution

                   tous_finis = false; 
                    
                } else  { 

                 // Processus a fini
                   if (WIFCONTINUED(status)) {
                    //le processus reprend son execution
                    if ((strcmp(jobs[i].etat, etat_str[1])==0)){            
                      run_again=true ;
                        }
                    } else if (WIFEXITED(status)) { 
                        //le processus a terminé avec un code de retour
    
                        terminated_count++;                        
                        if (pgid == -1) {
                            perror("getpgid");
                            exit(EXIT_FAILURE);
                        }

                        int status2;
                        int resultat2 = waitpid(-pgid, &status2, WNOHANG);
                        if (errno == ECHILD) {
                        // Aucun enfant à attendre (tous les processus du groupe ont terminé)
                        }
                        else if (resultat2 == -1) {
                            perror("waitpid pour le groupe");
                            exit(EXIT_FAILURE);
                        } else if (resultat2 == 0) {
                            // Au moins un processus du groupe est encore en cours d'exécution.
                            detache = true;
                        }

                    }else {
                        // Le processus a été tué par un signal
                       if (WIFSTOPPED(status)) {
                               int recu = WSTOPSIG(status);
                              // int recu = WTERMSIG(status);
                              //printf("recu :%d \n",recu);
                                if (recu == 19 || recu == 20) {
                                   //printf(" aqli5\n");
                                termine_stop++;
                                }
                               
                        } else { //le processus a été tué
                           
                               termine_signal = true;

                            }
                            
                        }
                    }
                
            
             //on vérifie maintenant quelest l'etat du job 

                    if (tous_finis) {
                    
                    if (run_again){ 
                        //running 
                        strcpy(jobs[i].etat, etat_str[0]);
                        jobs[i].affiche=1;
                    }
                    else if (termine_signal) { 
                        // killed
                        jobs[i].affiche=1;
                        strcpy(jobs[i].etat, etat_str[3]);

                        } else if (detache) { 
                            // detached
                            strcpy(jobs[i].etat, etat_str[2]);

                        }
                        else if ((termine_stop != 0) && (termine_stop == (jobs[i].nbr_processus - terminated_count)) ){ 
                            // stopped
                            strcpy(jobs[i].etat, etat_str[1]);
                            jobs[i].affiche=1;
                            jobs[i].avant =0; /**/

                        }else{                 
                            //done
                            jobs[i].affiche=1;
                            strcpy(jobs[i].etat, etat_str[4]);

                        } 
                    
                    } 
            }
        }
     i++;
    }

}

int is_stopped(struct Job* jobs, int nbr_jobs){
   int i =0;
   while (i < nbr_jobs) {
      if (strcmp(jobs[i].etat, etat_str[1]) == 0) return 1;
      i++;
    
    }
   return 0;
}

int is_running(struct Job* jobs, int nbr_jobs){
   int i =0;
   while (i < nbr_jobs) {
      if (strcmp(jobs[i].etat, etat_str[0]) == 0) return 1;
      i++;
    
    }
   return 0;

}

void liberer_job(struct Job* job) {
    free(job);
}


