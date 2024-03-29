#include "gestion_jobs.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  
#include <string.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include "signaux.h"
#include "redirections.h"

// verifie si la cmd est en arriere plan
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
// fonction de traitment de chaine de caractere  pour enlver le dernier argument utliser pour un cmd qui termine par &

void enlever_dernier_caractere(char *chaine) {
    // Vérifiez que la chaîne n'est pas vide
    size_t longueur = strlen(chaine);
    if (longueur > 0) {
        // Modifiez le dernier caractère en le remplaçant par '\0'
        chaine[longueur - 1] = '\0';
    }
}

// fonction de traitment des argument et chaine de caractere  pour enlver le dernier argument utliser pour un cmd qui termine par &
int modifie_args(char **args, int nb_args,char **chaine){
   enlever_dernier_caractere(*chaine);
    if (nb_args== 1)
    {
      enlever_dernier_caractere(args[0]);
    }
    else{
        if (strcmp(args[nb_args-1],"&")==0) 
        {
            free(args[nb_args-1]);
            args[nb_args-1]=NULL;
            nb_args = nb_args -1;
        }else{
            enlever_dernier_caractere(args[nb_args-1]);
        }
        
    }
    return nb_args;
}

int cmdArrierePlan (char **args,int nombre_jobs,struct Job tab_jobs[], size_t len,char * chaine){
  int r = fork();
  if (r == -1) {
    perror("fork");
    exit(0);
  }
  
  switch (r){
    
    case 0:
       
        setpgid(getpid(),getpid());  // changment du groupe pour le job
        reset_signaux_groupe(getpid());//mettre le traitement par defaut des signaux 
       // printf("lyess %s \n",chaine);
        CommandElement elements[MAX_ELEMENTS];
        int contains_substitution = 0;
        int nb_elements = 0;
        
        if ((extract_and_verify_subcommands(chaine, elements, &nb_elements, &contains_substitution)))// on verfie si il y a une substitution 
        {
            int pi[2];
            execute_subcommands(elements, nb_elements, pi, 0, chaine, tab_jobs, nombre_jobs - 1);
            exit(0);
        }
        else if (commandline_is_pipe(chaine)){ // on verifie si il y a des pipe line
            execute_pipes(chaine, NULL);
            exit(0);
        }



        
        else execvp(args[0],args); // si il y as ni substitution ni pipeline
        perror("Erreur lors de l'exécution de la commande");
        exit(1); // Valeur de sortie arbitraire en cas d'erreur
    break;

    default:
        struct Job* job;
        if (isspace(chaine[len - 2]) != 0)
            {
            chaine[len - 2]='\0';
            }
        
        job = creer_jobs(nombre_jobs, r ,chaine,0);// creation du structure du job 
        tab_jobs[nombre_jobs]= *job; // mise a jour du tableau des jobs
        free(job);
        fprintf(stderr,"[%d]\t%d\tRunning\t%s\n",tab_jobs[nombre_jobs].numero_job + 1,tab_jobs[nombre_jobs].processus[0], tab_jobs[nombre_jobs].command);
        return 0;
        break;
  }


}
