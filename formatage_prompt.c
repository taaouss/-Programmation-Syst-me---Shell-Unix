#include "mystring.h"
#include "formatage_prompt.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <linux/limits.h>



// Fonction pour afficher le prompt
char *afficher_prompt(int nb_job)
{
    // la variable pour la longueur maximale du prompt
    int longueurMax = 30;
    // Création de deux chaînes de caractères dynamiques avec la librairie "mystring"
    // cwd sert a stocker le chemin du répertoire de travail actuel
    // chaine sert a stocker le prompt 

    char str[20];  // Assurez-vous que la taille du tableau est suffisante
    sprintf(str, "%d", nb_job);

    // Utilisation de strlen pour obtenir la longueur de la chaîne
    size_t length_job = strlen(str);


    struct string *cwd = string_new(PATH_MAX);
    struct string *chaine = string_new(52); // 30 pour le prompt + 1 pour '/0' +21 pour les couleurs
    // Vérifie si l'allocation de mémoire a réussi
    if (chaine->data == NULL ||cwd->data == NULL )
    {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    // Ajout des couleurs au prompt
    string_append(chaine, "\001\033[32m\002");// Couleur verte
    string_append(chaine, "[");
    string_append(chaine, str);
    string_append(chaine, "]");
    string_append(chaine, "\001\033[36m\002");// Couleur cyan

    // Obtenir le répertoire de travail actuel
    getcwd(cwd->data, cwd->capacity);
    cwd->length = strlen(cwd->data);

    //Si la longueur du répertoire de travail actuel dépasse 25 obtenue par ce calcule (30 -(3 +2)) 
    if (cwd->length > (size_t)(longueurMax - 4- length_job))
    {
        string_append(chaine, "...");
        //Concaténer a chaine les 22 dernier caractére du répertoire de travail actuel
        string_append(chaine, (cwd->data + cwd->length - (longueurMax - 7-length_job)));
    }
    else
    {
        //Concaténer a chaine le répertoire de travail actuel
        string_append(chaine, cwd->data);
    }

    // Ajout de la couleur par défaut et du symbole du prompt ($)
    string_append(chaine, "\001\033[00m\002");
    string_append(chaine, "$ ");
    // Libération de la mémoire utilisée par la chaîne cwd
    string_delete(cwd);

    // Allocation de mémoire pour le résultat et copie de la chaîne de caractères
    char *resultat =malloc ((52)* sizeof(char)); 
    if (resultat == NULL )
    {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    strcpy(resultat,chaine->data);
    // Libération de la mémoire utilisée par la chaîne chaine
    string_delete(chaine);
    return resultat;
}