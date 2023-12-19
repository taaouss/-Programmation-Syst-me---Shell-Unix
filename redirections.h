#ifndef REDIRECTIONS_H
#define REDIRECTIONS_H

extern int exit_code;

int lecture(char *fic);
int sans_ecrasement_stdout(char *fic);
int avec_ecrasement_stdout(char *fic);
int concat_stdout(char *fic);
int concat_stderr(char *fic);
int sans_ecrasement_stdin(char *fic);
int avec_ecrasement_stdin(char *fic);
void is_redirection(char *commandline, int *index, char **redirection);
char *extractCommandAndArgs(const char *commandLine, int index);
char *extractRedirectionFileName(const char *commandLine, int index);
int execute_redirection(char *redirection, char *redirectionFileName);
void reset_redirections(int stdin_copy, int stdout_copy, int stderr_copy);

#endif