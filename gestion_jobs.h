#ifndef GESTION_JOBS_H
#define  GESTION_JOBS_H
#include <sys/types.h>

#define NBR_MAX_PROCESSUS 20
#define MAX_COMMAND_LENGTH 20

struct Job {
    int numero_job;
    pid_t processus[NBR_MAX_PROCESSUS];
    char * etat;
    char command[MAX_COMMAND_LENGTH];
    int nbr_processus;
};

const char *etat_str[] = { "RUNNING", "STOPPED", "DETACHED", "KILLED", "DONE"};

int jobs(struct Job *jobs, int nbr_jobs);

struct Job* creer_jobs(int nombre_jobs, pid_t pere ,char* commande);

void maj_jobs(struct Job *jobs, int nbr_jobs) ;

#endif