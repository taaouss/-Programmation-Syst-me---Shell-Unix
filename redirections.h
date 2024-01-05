#ifndef REDIRECTIONS_H
#define REDIRECTIONS_H

// extern int exit_code;

typedef struct
{
    char *redirection;
    char *redirectionFileName;
} Redirection;

typedef struct
{
    char *content;
    int type; // 0 if not a subcommand, 1 if it is a subcommand
} CommandElement;

#define MAX_SUBCOMMANDS 10
#define MAX_ELEMENTS 20

int lecture(char *fic);
int sans_ecrasement_stdout(char *fic);
int avec_ecrasement_stdout(char *fic);
int concat_stdout(char *fic);
int concat_stderr(char *fic);
int sans_ecrasement_stdin(char *fic);
int avec_ecrasement_stdin(char *fic);
int token_is_redirection(char *token);
int commandline_is_redirection(char *commandline);
void extract_redirections(char *commandline, Redirection **redirections, int *erreur, int *nb_redirections);
char *extractCommandAndArgs(char *commandLine, int index);
void free_redirections(Redirection *redirections, int nb_redirections);
int execute_redirection(char *redirection, char *redirectionFileName);
void reset_redirections(int stdin_copy, int stdout_copy, int stderr_copy);
int execute_redirections(Redirection *redirections, int nb_redirections);
int commandline_is_pipe(char *commandline);
void extract_pipe_commands(char *commandline, char *commands[], int *nb_commands);
// void free_subcommands(char *subcommands[], int num_subcommands);
// int extract_and_verify_subcommands(char *commandline, char *subcommands[], int *num_subcommands, int *is_really_substitution);
void free_elements(CommandElement elements[], int num_elements);
int execute_pipes(char *commandline, char *rep_precedent);


int extract_and_verify_subcommands(char *commandline, CommandElement elements[], int *num_elements, int *contains_substitution);

int execute_subcommands(CommandElement elements[], int num_elements,int pipe_tmp[],int rec,char *commandline);

#endif