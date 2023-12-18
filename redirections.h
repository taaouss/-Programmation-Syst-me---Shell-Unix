#ifndef REDIRECTIONS_H
#define REDIRECTIONS_H

extern int exit_code;

int lecture(char *fic);
int sans_ecrasement_stdout(char *fic);
int avec_ecrasement_stdout(char *fic);
int concat_stdout(char *fic);
int concat_stderr(char *fic);

#endif