#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <wait.h>
#include <limits.h>
#include "redirections.h"

char mes_symboles[7][4] = {"<", ">", ">|", ">>", "2>>", "2>", "2>|"};
int exit_code = 0;

// cmd < fic
int lecture(char *fic)
{
    int fd = open(fic, O_RDONLY);
    if (fd == -1)
    {
        exit_code = 1;
        return 1;
    }
    dup2(fd, 0);
    return 0;
}

// cmd > fic
int sans_ecrasement_stdout(char *fic)
{
    int fd = open(fic, O_CREAT | O_EXCL | O_WRONLY, 0666);
    if (fd == -1)
    {
        if (errno == EEXIST)
        {
            write(2, "le fichier existe déjà\n", strlen("le fichier existe déjà\n"));
        }
        exit_code = 1;
        return 1;
    }
    else
    {
        dup2(fd, 1);
        return 0;
    }
}

// cmd >| fic
int avec_ecrasement_stdout(char *fic)
{
    int fd = open(fic, O_CREAT | O_TRUNC | O_WRONLY, 0666);
    if (fd == -1)
    {
        exit_code = 1;
        return 1;
    }
    dup2(fd, 1);
    return 0;
}

// cmd >> fic
int concat_stdout(char *fic)
{
    int fd = open(fic, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd == -1)
    {
        exit_code = 1;
        return 1;
    }
    dup2(fd, 1);
    return 0;
}

// cmd 2>> fic
int concat_stderr(char *fic)
{
    int fd = open(fic, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd == -1)
    {
        exit_code = 1;
        return 1;
    }
    dup2(fd, 2);
    return 0;
}
