#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>

//ATTENTION: si le répertoire courant est toujours ./
//la variable courant elle correspond à un chemin du style "./../.."
//qui évoluera au fur et au mesure qu'on remonte à la racine.

//On limitera les tailles des chemins PATH_MAX



//retourne le nom du répertoire correspondant au chemin courant
char *nom_du_repertoire(char * courant) {
  // à adapter à partir de la question précédente
  struct stat st;
  if (lstat(courant, &st) != 0) {
    write(STDERR_FILENO,"erreur lstat",12);
 	//perror("lstat");
 	exit(1);
  }
  //pour recuperer le numero d'inode
  ino_t inode_number = st.st_ino;
    
   //ouvrir le repertoire pere
   char *  pere = malloc (PATH_MAX* sizeof(char));
   strcpy(pere,"../");
   strcat(pere,courant);
   DIR *dirp =opendir(pere);
   free(pere);
   struct dirent *entry;
   if (dirp == NULL) {
     write(STDERR_FILENO,"opendir pere",12);
   // perror("opendir pere");
     exit(1);
  }
  while ((entry=readdir(dirp))!=NULL){ 
  if(entry->d_ino == inode_number) return entry->d_name;
  }

  return "not_found_erreur";

  }
  
  



//test si le dossier courant est une racine 
int est_racine(char * courant) {
struct stat st;
  if (lstat(courant, &st) != 0) {
 	write(STDERR_FILENO,"erreur lstat",12);
 	exit(1);
  }
  //on recupere le numero d'inode du dossier courant 
 ino_t inode_number_rep_courant = st.st_ino;

 char *  pere = malloc (PATH_MAX* sizeof(char));
 strcpy(pere,"../");
 strcat(pere,courant);

 //on recupere le numero d'inode du dossier pere 
 if (lstat(pere, &st) != 0) {
 	write(STDERR_FILENO,"erreur lstat",12);
 	exit(1);
  }
 ino_t  inode_number_pere = st.st_ino;
 free(pere);
 //on comprare le inode du pere et celui du dossier courant
 return (inode_number_pere == inode_number_rep_courant);

}



//c'est là qu'on fait la récursion
char *  construit_chemin_aux(char * courant, char * pwd) {
char *  pere = malloc (PATH_MAX* sizeof(char));
 if (est_racine(courant)) 
 {
  strcpy(pwd,"/");
  return pwd;
  }

 else{ 
 strcpy(pere,"../");
 strcat(pere,courant);
 construit_chemin_aux(pere,pwd);
 if (strcmp(pwd,"/") != 0 ) strcat(pwd,"/");
 strcat(pwd,nom_du_repertoire(courant));
 free(pere);
 return pwd;
 }
 
}

//cette fonction retournera la référence absolue du répertoire courant ./
char *  construit_chemin() {
  //pwd est un chemin de style "/bidule/truc" où "bidule/truc" est la fin du pwd, à la fin ce sera le pwd
  char *  pwd = malloc (PATH_MAX* sizeof(char));
  // courant est un chemin du style "./../.."
  char *courant = malloc (PATH_MAX* sizeof(char));
  //sprintf(courant,"."); 
  strcpy(courant,"."); 
  //sprintf(pwd,"/");
  strcpy(pwd,"/");
  pwd = construit_chemin_aux(courant, pwd);
  free(courant);

  return pwd;
}


int main(int argc, char **argv) {
  
  char * pwd=NULL;
  
  pwd = construit_chemin();
 printf("mon pwd : %s\n", pwd);
 free(pwd);
  return 0;  
}
