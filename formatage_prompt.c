#include "mystring.h"
#include "formatage_prompt.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <linux/limits.h>
char *afficher_prompt()
{
    int longueurMax = 30;
    struct string *cwd = string_new(PATH_MAX);
    struct string *chaine = string_new(52); // 21 pour les couleurs
    if (chaine->data == NULL)
    {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    string_append(chaine, "\001\033[32m\002");
    string_append(chaine, "[0]");
    string_append(chaine, "\001\033[36m\002");
    getcwd(cwd->data, cwd->capacity);
    cwd->length = strlen(cwd->data);
    if (cwd->length > (size_t)(longueurMax - 5))
    {
        string_append(chaine, "...");
        string_append(chaine, (cwd->data + cwd->length - (longueurMax - 8)));
    }
    else
    {
        string_append(chaine, cwd->data);
    }
    string_append(chaine, "\001\033[00m\002");
    string_append(chaine, "$ ");
    string_delete(cwd);
    char *lyes =malloc ((52)* sizeof(char)); 
    strcpy(lyes,chaine->data);
    string_delete(chaine);
    return lyes;
}