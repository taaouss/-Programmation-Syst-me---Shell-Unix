#include "gestion_jobs.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  
#include <string.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


int is_cmdArrierePlan(char **args,int nb_args) {
    if (nb_args== 1)
    {
        if (strstr(args[0],"&")) return 1;
        else return 0;
    }
    else{
        if (strstr(args[nb_args-1],"&")) return 1;
        else return 0;
    }
}
void enlever_dernier_caractere(char *chaine) {
    // Vérifiez que la chaîne n'est pas vide
    size_t longueur = strlen(chaine);
    if (longueur > 0) {
        // Modifiez le dernier caractère en le remplaçant par '\0'
        chaine[longueur - 1] = '\0';
    }
}

int modifie_args(char **args, int nb_args,char **chaine){
   enlever_dernier_caractere(*chaine);
    if (nb_args== 1)
    {
      enlever_dernier_caractere(args[0]);
    }
    else{
        if (strlen(args[nb_args-1])==1)
        {
            args[nb_args-1]=NULL;
            nb_args = nb_args -1;
        }else{
            enlever_dernier_caractere(args[nb_args-1]);
        }
        
    }
    return nb_args;
}

int cmdArrierePlan (char **args,int nombre_jobs,struct Job tab_jobs[],int nb_args, size_t len){
  int r = fork();
  if (r == -1) {
    perror("fork");
    exit(1);
  }
  struct Job* job;
  char *chaine=malloc(len *sizeof(char));
  strcpy(chaine,args[0]);
  for (int i = 1; i < nb_args; i++)
  {
    strcat(chaine," ");
    strcat(chaine,args[i]);
  }
  strcat(chaine,"\0");
  
  job = creer_jobs(nombre_jobs, r ,chaine);
  tab_jobs[nombre_jobs]= *job;
  switch (r){
    case 0:
    
   

    
    execvp(args[0],args);
    perror("Erreur lors de l'exécution de la commande");
    exit(1); // Valeur de sortie arbitraire en cas d'erreur
    break;
    default:
    return 1;
    break;
  }


}
