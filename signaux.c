#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>



void set_signaux(){
  
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = SIG_IGN ;

  sigaction(SIGTSTP, &action, NULL);
  sigaction(SIGQUIT, &action, NULL);
  sigaction(SIGTERM,&action,NULL);
  sigaction(SIGINT,&action,NULL);
  sigaction(SIGTTIN,&action,NULL);
  sigaction(SIGTTOU,&action,NULL);
 
}

void reset_signaux(){

    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = SIG_DFL;

    sigaction(SIGTSTP, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTTIN, &action, NULL);
    sigaction(SIGTTOU, &action, NULL);
}


void reset_signaux_groupe(pid_t pgid) {
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = SIG_DFL;

    // Réinitialiser les signaux uniquement pour le groupe spécifié
    if (pgid > 0) {
        setpgid(0, pgid);  // Définir le groupe de processus actuel pour le groupe spécifié
        sigaction(SIGTSTP, &action, NULL);
        sigaction(SIGQUIT, &action, NULL);
        sigaction(SIGTERM, &action, NULL);
        sigaction(SIGINT, &action, NULL);
        sigaction(SIGTTIN, &action, NULL);
        sigaction(SIGTTOU, &action, NULL);
    }

}