
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "commandes_internes.h"

int pwd()
{
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("%s\n", cwd);
        return 0;
    }
    else
    {
        perror("getcwd");
        return 1;
    }
}
int cd(char *argument, char *rep_precedent)
{
    char precedent_tmp[PATH_MAX];
    getcwd(precedent_tmp, sizeof(precedent_tmp));
    if (argument != NULL)
    {
        // Changer le répertoire
        if (strcmp(argument, "-") == 0)
        {
            if (chdir(rep_precedent) != 0)
            {
                return 1;
            }
        }
        else
        {
            if (chdir(argument) != 0)
            {
                return 1;
            }
        }
    }
    else
    {
        // Pas d'argument, retour au répertoire personnel
        if (chdir(getenv("HOME")) != 0)
        {
            return 1;
        }
    }
    strcpy(rep_precedent, precedent_tmp);
    return 0;
}
