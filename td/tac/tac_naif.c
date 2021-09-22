/*
 * 1.3  Afficher un fichier à l’envers
 *
 * L'objectif de cette section est de définir une commande retourne permettant d'afficher
 * le contenu d'un fichier à l'envers. Ainsi si un fichier fic contient exactement trois
 * octets "abc", retourne fic affichera "cba".
 * 
 * ------------------------------------------------------------------------------------
 *
 * Exercice 14 (Version simpliste)
 * 	Proposez une implantation de retourne utilisant lseek (ou pread) pour aller lire
 * 	le fichier source octet par octet en commençant par le dernier.
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv) {
	
	if (argc != 2) {
		printf("Error : unexpected number of args\nExpected : \"%s path_to_file\"\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	errno = 0;
	int fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	struct stat st;
	errno = 0;
	if (fstat(fd, &st) != 0) {
		perror("stat");
		exit(EXIT_FAILURE);
	}

	char buf[1];
	ssize_t bytes_read = 0;
	for (off_t offset = st.st_size; offset > 0; offset -= bytes_read) {
		bytes_read = pread(fd, buf, 1, offset-1);
		write(STDOUT_FILENO, buf, bytes_read);
	}
	exit(EXIT_SUCCESS);
}
