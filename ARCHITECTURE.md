 ## Structure Job:
- numero_job: Numéro attribué au job.
- processus: Tableau de PIDs représentant les processus associés au job, incluant le processus père.
- etat: Chaîne de caractères indiquant l'état actuel du job (ex. "Running").
- command: Chaîne de caractères représentant la commande exécutée par le job.
- nbr_processus: Nombre de processus associés au job.
- affiche: Indicateur booléen pour déterminer si le job doit être affiché.
- avant: Indicateur booléen pour indiquer si le job est en avant-plan.

 ## Fichier main :
Lorsqu'une commande est introduite, on vérifie d'abord si c'est une commande en arrière-plan. Si c'est le cas, on vérifie si elle contient une redirection. Si c'est le cas, on appelle la fonction execute_redirections (fichier redirection.c), qui change les descripteurs par défaut selon la commande. Ensuite, on exécute la fonction cmdArrierePlan (fichier arriere_plan.c).
Si ce n'est pas une commande en arrière-plan, on vérifie si elle contient des substitutions. Si c'est le cas, on effectue un fork pour créer le fils qui va changer son groupe d'abord, puis exécute execute_subcommands (fichier redirection.c). Cette dernière traite même les cas des substitutions imbriquées, des substitutions qui contiennent des pipelines, et même des redirections des sorties et entrées standards et d'erreurs. Pour le père, il effectue un wait bloquant pour attendre son fils et met à jour le tableau des jobs.

Si elle ne contient pas de substitution, on vérifie si c'est une commande pipeline. Si oui, on effectue un fork, et le fils appelle la fonction execute_pipes (fichier redirection.c) après avoir changé son groupe. execute_pipes permet aussi de gérer les redirections des sorties et entrées standards, ainsi que des erreurs. Le père effectue un wait bloquant et met à jour le tableau des jobs.

Si elle ne contient ni substitution ni pipeline, on vérifie si elle contient des redirections de sortie, d'entrée standard ou d'erreur. Si c'est le cas, elle appelle la fonction execute_redirections (fichier redirection.c), qui change les descripteurs par défaut selon la commande.

Ensuite, on vérifie si c'est une commande interne telle que "pwd", "jobs", "kill", "?", "cd", "exit", "fg", "bg", etc. Si oui, on appelle la fonction appropriée pour la commande (elles se trouvent dans le fichier commandes_internes.c). Pour la commande "jobs", avant d'appeler sa fonction adaptée, on met à jour d'abord les jobs avec maj_job (fichier gestion_jobs.c).

si c'est une commande externe, on crée un fils auquel on assigne un numéro de groupe (le pid de ce dernier) et on remet le traitement par défaut des signaux avec reset_signaux_groupe(pid_t pgid), puis le processus fils exécute execvp.Notre jsh passe la main au nouveau groupe et attend sa terminaison pour récupérer le contrôle du terminal.Le père vérifie et met à jour l'état du job du fils.
Enfin, on libère les ressources inutiles, on met à jour les jobs et on affiche les jobs qui ont changé d'état. On boucle ensuite pour afficher à nouveau le prompt.

 ## Fichier redirection :
### Redirections

On utilise un tableau pour stocker les symboles des redirections, puis on traite chaque redirection par une fonction qui redirige la sortie ou l’entrée vers le fichier indiqué (si aucun fichier n’est donné une erreur sera générée).

Pour vérifier et parser une commandline on utilise ces fonctions : 
- `commandline_is_redirection`: pour vérifier si une commandline contient une redirection.

- `extract_redirection`: pour extraire les redirections et les fichiers qui correspondent et les stocker dans une structure `Redirection`.

- `extractCommandAndArg`: Pour extraire la commande et ses arguments sans les redirections.

Ensuite, nous exécutons les redirections avec la fonction `execute_redirections`.
Une fois les redirections executées, nous liberons les redirection stockées et nous réinitialisons les redirections par la fonction `reset_redirections`

### Pipelines
On vérifie si une commandline contient des pipes en utilisant la fonction `commandline_is_pipe`
Si c’est le cas: on extrait les commandes entre les pipes par la fonction `extract_pipe_commands`, et les stocke dans un tableau de commandes.
 Pour exécuter les commandes entre les pipes:

- On crée un tableau de descripteurs de pipe (`pipefd`).
- On parcours le tableau des commandes:
    - On crée un pipe pour chaque commande sauf la dernière.
    - On crée un processus fils pour exécuter chaque commande. 

- Les processus fils s'exécutent tous en même temps pour former une architecture d’un père qui supervise tous ses fils.


### Substitution
On utilise la structure CommandElement, qui contient deux champs : content pour la commande et type (si type = 1, cela indique une substitution, sinon 0).

Pour les fonctions qui concernent les substitutions, on a principalement deux : une qui vérifie s'il y a une substitution. Si c'est le cas, elle extrait les arguments de la commande et donne son type dans un tableau CommandElement.
L'autre fonction, pour exécuter les substitutions, le père commence par boucler pour créer ses fils (les arguments qui contiennent des substitutions). Pour chaque fils, elle crée un tube anonyme. Les fils ferment les descripteurs de leur frère, ainsi que ceux qui sont inutiles, notamment leur pipe en lecture. Il(le fils) redirige ensuite sa sortie vers le tube. Ensuite, il vérifie s'il contient également une substitution (c'est-à-dire dans le cas des substitutions imbriquées). Si c'est le cas, il appelle récursivement la fonction. Ensuite, il vérifie s'il contient des pipelines (cas de pipelines imbriqués). Enfin, il exécute sa commande. revenant encore au père, il ferme les tubes inutiles,met à jour le tableau du job, change le tableau de ses arguments qui contiennent des substitutions vers le chemin du tube, et exécute la commande principale qui contient les substitutions.

## Fichier arriere_plan :

Contient principalement une fonction qui exécute une commande en arrière-plan elle fork d'abord le père ajoute son fils au tableau des jobs et affiche l'état du job et le fils vérifie s'il y a une substitution et/ou une pipeline et appelle les fonction qui convient (redirection.c) et exécute sa commande.

## Fichier gestion_jobs :

La fonction jobs permet d'afficher les jobs. Elle boucle sur le tableau des jobs et, s'il est possible de les afficher (satisfait les conditions d'affichage), elle les affiche. La fonction est adaptée pour les options -t et % en appelant les fonctions get_child_processes (qui se trouve dans le même fichier) pour afficher les petits fils selon le GID pour l'option -t et, pour l'option %, elle affiche simplement le job qui convient.

get_child_processes on fait un parcours du repertoire /Proc pour chercher les processus qui sont dans le meme groupe passé en paramètre et les affiche ( bien sur avec d'autre condition)

La fonction jobs_err sert à afficher les jobs qui ont changé d'état.

La fonction cree_jobs sert à initialiser la structure de données de Job.

La fonction maj_job nous sert à mettre à jour l’état de nos jobs, cela se fait en parcourant notre table de jobs et en faisant un wait non bloquant sur chaque job ce qui nous permettra de connaître son état et faire une modification si cela est nécessaire 

## Fichier utils :

La fonction extract_args permet d'extraire les arguments d'une commande et de les mettre dans un tableau avec un simple parcours de la chaîne de caractères.

## Fichier commandes_interne :

pwd() utilise principalement getcwd.

cd() sauvegarde son répertoire actuel. Si elle contient "-", comme argument, elle revient au répertoire précédent  Sinon, elle change le répertoire vers l'argument donné , ensuite elle met à jour la variable du répertoire précédent.

Les fonctions kill :Elle analyse les arguments fournis, détermine le signal de terminaison à utiliser, puis envoie ce signal soit à un processus spécifié par son PID, soit à tous les processus d'un job identifié par son numéro.

 fg: extrait le numéro de job de l'argument, vérifie sa validité, met en avant-plan le job correspondant après avoir vérifié son statut, et attendant sa terminaison ou son arrêt, avant de restaurer le contrôle au shell.

bg : extrait le numéro de job de l'argument, vérifie sa validité, lance le job en arrière-plan après avoir confirmé qu'il est arrêté, et permettant ainsi à ce processus de continuer son exécution en arrière-plan.




