#define TAILLE_TAMPON 1024

struct kfile_s {
	// file descriptor (comes from open(2))
	int fd;
	// relatifs au buffer
	unsigned char tampon[TAILLE_TAMPON];
	unsigned int position_tampon;
}

typedef struct kfile_s KFILE;

unsigned char kputc(unsigned char c, KFILE *kfile) {
	assert(kfile != NULL);
	assert(kfile->position_tampon < TAILLE_TAMPON);

	kfile->tampon[kfile->position_tampon] = c;
	kfile->position_tampon++;

	if (kfile->position_tampon == TAILLE_TAMPON) {
		// le tampon est plein, il faut écrire dans le fichier
		kflush(kfile);
	}

	return c;
}

void kflush(KFILE *kfile) {
	assert(kfile != NULL);

	int octets_ecrits = write(kfile->fd, kfile->tampon, kfile->position_tampon);
	// insuffisant ! write peut écrire moins que demandé dans le flux
	assert(octets_ecrits == kfile->position_tampon);

	kfile->position_tampon = 0;

	return;
}

KFILE *kopen(const char *path) {
	KFILE *kfile = (KFILE *) malloc(sizeof(KFILE));
	assert(kfile != NULL);

	kfile->fd = open(path, O_WRONLY|O_CREAT, umask(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
	assert(kfile->fd != -1);

	kfile->position_tampon = 0;

	return kfile;
}

/* Pour récupérer le mode d'ouverture du fichier          *
 * vous pouvez utiliser fcntl(2) avec la commande F_GETFL */

int kclose(KFILE *kfile) {
	assert(kfile != NULL);

	//vider le tampon avant de fermer le fichier
	kflush(kfile);
	int return_code = close(kfile->fd);
	assert(return_code == 0);

	free(kfile);
}
