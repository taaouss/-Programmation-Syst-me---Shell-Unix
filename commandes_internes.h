#ifndef COMMANDES_INTERNES_H
#define COMMANDES_INTERNES_H
#include "gestion_jobs.h"



int pwd();

int cd(char *argument, char *rep_precedent);

int kill_commande(char **argument, int nbr_arguments, struct Job *jobs, int nbr_jobs) ;

int fg_commande(struct Job* jobs, int nbr_jobs, char* arg);

//int bg_commande(struct Job* jobs, int nbr_jobs, char* arg);

#endif