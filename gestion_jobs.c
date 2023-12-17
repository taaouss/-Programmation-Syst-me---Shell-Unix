#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "gestion_jobs.h"


int jobs(struct Job *jobs, int nbr_jobs){
  
  int i=0 ;
  if (jobs == NULL) return 0;

  while( i< nbr_jobs){
    if(jobs[i] !=NULL){
    fprintf("[%d] %d %s  %s \n",jobs[i].numero_job ,jobs[i].pid_pere ,jobs[i].state ,jobs[i].command) ;
    }
    i++;
  }
  return 1 ;
}

struct Job* creer_jobs(int nombre_jobs, pid_t processus ,char* commande){
    //incremmenter le nombre de job avant l'appel à cette fonction

    struct Job resultat = malloc(sizeof(struct Job));
     
    resultat->numero_job = nombre_jobs;
    strncpy(resultat->command, commande, MAX_COMMAND_LENGTH - 1);
    resultat->command[MAX_COMMAND_LENGTH - 1] = '\0';
    resultat->nbr_processus = 1;
    resultat->etat = etat_str[1];
    resultat->processus = malloc(NBR_MAX_PROCESSUS * sizeof(pid_t));
    resultat->processus[0] = processus; // un processus
     // struct Job* ptr = resultat ;
     return resultat ; //liberer resultat aussi pour eviter les erreurs valgrin
}

void maj_jobs(struct Job *jobs, int nbr_jobs){

bool tous_finis =false , un_termine= false , termine_signal=false  ;
int i=0 ;
  if (jobs == NULL) return 0;

  while( i< nbr_jobs){
    tous_finis =true ;
    un_termine= false ;
    termine_signal=false ;

   if(jobs[i].etat != etat_str[5]){ // e job n'est pas done 
      
      for (int j = 0; j < jobs[i].nbr_processus; j++) {
        //on va parcourir les processus
                pid_t processus = jobs[i].processus[j];
                int status;
                 if (waitpid(processus, &status, WNOHANG) == 0) {
                    // Le processus n'a pas encore terminé
                   tous_finis =false ;
                   un_termine= true ;
                    if (WIFSIGNALED(status)) {// Le processus a été tué par un signal
                       termine_signal=true ;
                    }

                }
           
           
            }


            if(tous_finis) {//done

                strcpy(jobs[i].etat, etat_str[5]);
                
              }
            else if (termine_signal) { // signal

                strcpy(jobs[i].etat, etat_str[4]);
            
            }
        
      }      

      i++;
      }

  }
