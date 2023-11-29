
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "commandes_internes.h"

// Fonction pour afficher le répertoire de travail actuel 
int pwd()
{
    char cwd[PATH_MAX];
    // Utilise getcwd pour obtenir le chemin du répertoire de travail actuel
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        // Affiche le chemin
        printf("%s\n", cwd);
        return 0;
    }
    else
    {
        // En cas d'erreur
        perror("getcwd");
        return 1;
    }
}

// Fonction pour changer le répertoire de travail 
int cd(char *argument, char *rep_precedent)
{
    // Sauvegarde du répertoire de travail actuel avant tout changement
    char precedent_tmp[PATH_MAX];
    getcwd(precedent_tmp, sizeof(precedent_tmp));
    if (argument != NULL)
    {
        // Si l'argument est "-", revenir au répertoire précédent
        if (strcmp(argument, "-") == 0)
        {
            if (chdir(rep_precedent) != 0)
            {
                return 1;// En cas d'erreur
            }
        }
        else
        {
            // Changer vers le répertoire spécifié par l'argument
            if (chdir(argument) != 0)
            {
                return 1;// En cas d'erreur
            }
        }
    }
    else
    {
        // Pas d'argument, retour au répertoire personnel (HOME)
        if (chdir(getenv("HOME")) != 0)
        {
            return 1;// En cas d'erreur
        }
    }
    // Mise à jour la variable rep_precedent avec le répertoire précédent sauvgarder au debut de la fonction
    strcpy(rep_precedent, precedent_tmp);
    return 0;
}
