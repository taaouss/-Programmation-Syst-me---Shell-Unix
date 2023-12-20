#ifndef REDIRECTIONS_H
#define REDIRECTIONS_H

extern int exit_code;

typedef struct
{
    char *redirection;
    char *redirectionFileName;
} Redirection;

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
char *extractCommandAndArgs(const char *commandLine, int index);
// void is_redirection(char *commandline, int *index, char **redirection);
// char *extractRedirectionFileName(const char *commandLine, int index);
int execute_redirection(char *redirection, char *redirectionFileName);
void reset_redirections(int stdin_copy, int stdout_copy, int stderr_copy);
int execute_redirections(Redirection *redirections, int nb_redirections);

#endif