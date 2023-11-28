#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h> 

// Définir la taille maximale pour le répertoire courant
#define MAX_CWD_SIZE 512




int pwd(void) {
    char cwd[MAX_CWD_SIZE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
        return 0;
    } else {
        perror("getcwd");
        return 1;
    }
}

int cd(char *argument,char *rep_precedent) {
    char precedent_tmp[PATH_MAX];
    getcwd(precedent_tmp, sizeof(precedent_tmp));
    if (argument != NULL) {
        // Changer le répertoire
        if ( strcmp(argument,"-")==0){
        if (chdir(rep_precedent) != 0) {
            return 1;
        }
        }
        else{
        if (chdir(argument) != 0) {
            return 1;
        }
        }
    } else {
        // Pas d'argument, retour au répertoire personnel
        if (chdir(getenv("HOME")) != 0) {
            return 1;
        }
    }
    strcpy(rep_precedent,precedent_tmp);
    return 0;
}
char *afficher_prompt(){

 int longueurMax = 30;
 char *chaine =malloc ((PATH_MAX+10)* sizeof(char));
 if (chaine == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
 strcpy(chaine, "\001\033[32m\002");
 strcat(chaine,"[0]");
 strcat(chaine, "\001\033[36m\002");

char cwd[PATH_MAX];
getcwd(cwd, sizeof(cwd));
size_t cwd_length = strlen(cwd);
    
if (cwd_length > (size_t)(longueurMax - 6)) {
    strcat(chaine, "...");
    strcat(chaine, cwd + cwd_length - (longueurMax - 8)); 
} else {
    strcat(chaine, cwd);
}

    strcat(chaine, "\001\033[00m\002");

    strcat(chaine, "$ ");

    return chaine;
}

int main() {
    system("clear");
    // int job = 0;
    char rep_courant[PATH_MAX]="jsh";
    char rep_precedent[PATH_MAX];
    getcwd(rep_precedent, sizeof(rep_precedent));// pour avoir le rep courant
    rl_outstream = stderr;
    // using_history();
    char *buf;
   // rl_num_chars_to_read = 30000;
    int code_retour = 0  , status;
    //char string[1024]; 
    while (1) {
       // sprintf(string, "[%d] %s > ", job , rep_courant);
        //buf = readline(string);
        buf = readline(afficher_prompt());
        if (buf == NULL) {
            fprintf(stderr, "Erreur de lecture de la commande.\n");
            break;
        } else {
            size_t len = strlen(buf);
            if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';

            char *commande = strtok(buf, " ");
            char *argument = strtok(NULL, " ");

            if (commande != NULL) {
                if (strcmp(commande, "pwd") == 0) {
                    if (argument == NULL) {
                        if ((code_retour  = pwd()) != 0)
                            printf("Erreur pwd\n");
                    } else {
                        printf("pwd n'a pas d'argument\n");
                    }
                } else if (strcmp(commande, "?") == 0) {
                    printf("%d\n",code_retour);
                    code_retour =0;
                } else if (strcmp(commande, "cd") == 0) {
                        if ((code_retour = cd(argument,rep_precedent)) != 0) perror("Erreur lors de la commande cd");
                        getcwd(rep_courant, sizeof(rep_courant)); //m-a-j du rep courant
                    
                     } else if (strcmp(commande, "exit") == 0) {
                         if (argument != NULL) {
                            code_retour = atoi(argument);
                         }
                         exit(code_retour);   
                    } 
                    else{
                    int pid = fork();
                    switch (pid){
                        case -1:
                            perror("Erreur lors de la création du processus fils");
                            break;
                        case 0 :
                            execlp(commande, commande, argument, NULL);
                            perror("Erreur lors de l'exécution de la commande");
                            exit(EXIT_FAILURE);// arrightid parceque ila9 adifagh le fils g la boucle while 
                            break;
                        default:
                            waitpid(pid, &status, 0);
                            code_retour = WEXITSTATUS(status);
                           
                            break;
                         }
                     }

                    }
                    
                
            }

           // add_history(buf);
            free(buf);
        }
    
    return 0;
}
