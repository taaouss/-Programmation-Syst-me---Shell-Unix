#ifndef GESTION_JOBS_H
#define  GESTION_JOBS_H
#include <sys/types.h>

#define NBR_MAX_PROCESSUS 20
#define MAX_COMMAND_LENGTH 20

struct Job {
    int numero_job;
    pid_t processus[NBR_MAX_PROCESSUS];
    char  etat[10];
    char command[MAX_COMMAND_LENGTH];
    int nbr_processus;
};



int jobs(struct Job *jobs, int nbr_jobs);

struct Job* creer_jobs(int nombre_jobs, pid_t pere ,char* commande);

void maj_jobs(struct Job *jobs, int nbr_jobs) ;

int is_stopped(struct Job* jobs, int nbr_jobs);

int is_running(struct Job* jobs, int nbr_jobs);

void liberer_job(struct Job* job);

#endif