/*
 * 1.3  Afficher un fichier à l'envers
 *
 * L'objectif de cette section est de définir une commande retourne permettant
 * d'afficher le contenu d’un fichier à l'envers. Ainsi si un fichier fic contient
 * exactement trois octets "abc", retourne fic affichera "cba".
 * 
 * ------------------------------------------------------------------------------------
 *
 * Exercice 16 (Version économe)   Proposez une implantation de retourne qui :
 * 
 *     charge les TAILLE_TAMPON derniers octets du fichier,
 *     les affiche en ordre inverse,
 *     charge les TAILLE_TAMPON octets précédents,
 *     ... 
 * 
 *     Pensez au cas où la taille du fichier n'est pas un multiple de TAILLE_TAMPON: que faut-il faire?
 *     En quoi cet algorithme est-il plus économe? 
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void reverse_buf(char *buf, ssize_t buf_size);

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

	ssize_t buf_size = st.st_blksize;
	char *buf = malloc(buf_size);
	if (buf == NULL) {
		printf("Error : buffer allocation failed\n");
		exit(EXIT_FAILURE);
	}
	
	ssize_t total_bytes_read = 0;
	while (total_bytes_read < st.st_size) {
		off_t file_offset = (st.st_size - 1) - total_bytes_read - buf_size;
		ssize_t bytes_to_read = buf_size;
		// compute the correct offset
		if (st.st_size - total_bytes_read < buf_size) {
			bytes_to_read = st.st_size - total_bytes_read;
			file_offset = 0;
		}
		// fill the buffer
		ssize_t bytes_read = 0;
		while (bytes_read < bytes_to_read) {
			bytes_read += pread(fd, buf + bytes_read, bytes_to_read - bytes_read, file_offset + bytes_read);
		}
		reverse_buf(buf, bytes_to_read);
		ssize_t bytes_written = 0;
		while (bytes_written < bytes_to_read) {
			bytes_written += write(STDOUT_FILENO, buf + bytes_written, bytes_to_read - bytes_written);
		}
		total_bytes_read += bytes_read;
	}

	free(buf);
	exit(EXIT_SUCCESS);
}

void reverse_buf(char *buf, ssize_t buf_size) {
	ssize_t rev_index = 0;
	ssize_t last_buf_index = buf_size - 1;
	while (rev_index < (buf_size / 2)) {
		char byte = buf[rev_index];
		buf[rev_index] = buf[last_buf_index - rev_index];
		buf[last_buf_index - rev_index] = byte;
		rev_index++;
	}
}
