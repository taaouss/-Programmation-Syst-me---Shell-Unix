

#ifndef ARRIERE_PLAN_H 
#define ARRIERE_PLAN_H 
int is_cmdArrierePlan(char **args,int nb_args) ;
 int modifie_args(char **args, int nb_args,char **chaine); 
 int cmdArrierePlan (char **args,int nombre_jobs,struct Job  tab_jobs[],int nb_args, size_t len, char * chaine); 
 #endif

