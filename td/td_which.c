/* Commencer par le main */

#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char const *const *get_path_dirs();
bool which(char const *const command, char const *const *const path_directories);

/* Contrat : 
 *  - EXIT_SUCCESS si les toutes commandes passées en
 * paramètre ont été trouvées dans au moins un dossier du $PATH
 *  - EXIT_FAILURE sinon */
int main (int argc, char *argv[]) {
	/* On s'assure que l'utilisateur a donné au moins une commande à chercher */
	if (argc == 1) {
		fprintf(stderr, "Error : expected command names to look for.\n"
				"Usage : %s command1 [command2 ...]", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* On récupère les répertoires du PATH
	 * ça nous permet de les `free()` à la fin de l'exécution */
	char const *const *const path_directories = get_path_dirs();

	/* Flag mis à EXIT_FAILURE dès lors qu'une commande n'a pas été trouvée  */
	int status = EXIT_SUCCESS;

	/* Pour chaque commande passée en argument */
	for (int command_n = 1; command_n < argc; command_n++) {
		/* les chercher dans les différents répertoires du PATH */
		/* si on ne trouve pas la commande dans tous les répertoires */
		if (!which(argv[command_n], path_directories)) {
			/* `which` échoue */
			status = EXIT_FAILURE;
		}
	}

	/* On libère la mémoire qu'on a alloué avec malloc */
	free(path_directories);

	/* On sort soit avec SUCCESS soit avec FAILURE */
	exit(status);
}

/* Contrat :
 *   - retourne TRUE ou FALSE en fonction de si la commande a été trouvée ou non
 *   - Affiche le chemin de la commande si trouvée, affiche un message d'erreur sinon*/
bool which(char const *const command, char const *const *const path_directories) {
	
	/* déclare un tableau qui contiendra la concaténation
	 * de la commande et du répertoire */
	/* PATHMAX est la taille maximum possible du path, def. dans <limits.h> */
	char command_path_buffer[PATH_MAX+1];

	for (char const *const *dir_name_ptr = path_directories; *dir_name_ptr != NULL; dir_name_ptr++) {
		/* concaténation de deux chaines dans command_path_buffer */
		snprintf(command_path_buffer, PATH_MAX, "%s/%s", *dir_name_ptr, command);
		/* on vérifie que le binaire existe et qu'il est exécutable
		 * retourne 0 en cas de réussite */
		int return_code = access(command_path_buffer, X_OK);
		/* Si l'appel réussi */
		if (return_code == 0) {
			/* on affiche le PATH de la commande */
			printf("%s\n", command_path_buffer);
			return true;
		}
	}

	/* Si la commande n'a été trouvée dans aucun des répertoires du PATH */
	printf("%s : command not found\n", command);
	return false;
}

char const *const *get_path_dirs() {
	/* Récupère la chaine de la variable d'environnement PATH */
	char *path = getenv("PATH");
	/* On s'assure que la variable d'environnement existe */
	if(path == NULL) {
		perror("getenv");
		exit(EXIT_FAILURE);	
	}

	unsigned dir_count = 1;
	/* For each character of the path */
	for (char *path_char_ptr = path; *path_char_ptr != '\0'; path_char_ptr++) {
		/* Look for the delimiter ':' */
		if(*path_char_ptr == ':') {
			/* increase number of directories when found */
			dir_count++;
			/* replace the delimiter with null bytes to make it a valid string */
			*path_char_ptr = '\0';
		}
	}

	/* On crée un tableau du nombre de répertoires + 1 pour délimiter la fin du tableau */
	char const **const path_dirs = malloc((dir_count + 1) * sizeof(char *));
	if(path_dirs == NULL){
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	/* On rempli le tableau avec les pointeurs de début de chaines des répertoires */
	for(unsigned i = 0; i < dir_count; i++) {
		path_dirs[i] = path;
		path += strlen(path) + 1;
	}
	/* On met le pointeur nul à la fin du tableau pour savoir où est la fin */
	path_dirs[dir_count] = NULL;
	return path_dirs;
}
