/* Include lib C  */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

/* our Include */
#include "our_mod.h"

int main(int argc, char **argv)
{
	/* file descriptor */
	int stdin_fd;
	int module_fd;
	int i = 0, y = 0;
	/* readding buf */
	char *read_buf = malloc(1 * sizeof(char));
	unsigned long req = 0;
	struct commande commande;

	commande.nom = malloc(10 * sizeof(char));
	commande.param = malloc(10 * sizeof(char *));
	for (i = 0; i < 10; i++)
		commande.param[i] = malloc(10 * sizeof(char));

	fflush(stdin);

	if (argc != 2) {
		perror("Utilisation : Projet.x /dev/module\n");
		exit(1);
	}

	/* open stdin */
	stdin_fd = open("/dev/null", O_RDWR);
	if (stdin_fd < 0) {
		perror("Error open stdin !\n");
		exit(1);
	}

	/* get the stdin_fd */
	if (dup2(STDIN_FILENO, stdin_fd) == -1) {
		perror("Error dup2 stdin !\n");
		exit(1);
	}

	/* open module to have acces to it */
	module_fd = open(argv[1], O_RDWR);
	if (module_fd < 0) {
		perror("Error open module !\n");
		exit(1);
	}

	while (1) {
		commande.retour = malloc(1024 * sizeof(char));
		for (i = 0; i < 1024; i++)
			commande.retour[i] = '\0';

		printf("Veuillez entrer votre commande :\n");
		fflush(stdin);

		commande.asynchrone = 0;

		for (i = 0; i < 10; i++)
			commande.nom[i] = '\0';
		i = 0;
		/* wait for something to read */
		while (i == 0)
			i = read(stdin_fd, read_buf, 1);
		i = 0;

		/* reading the command name */
		while ((read_buf[0] != '\n') &&
		       (read_buf[0] != '\r') &&
		       (read_buf[0] != ' ')) {

#ifdef DEBUG
			printf("lecture du while BUF : %c, i : %d\n",
			       read_buf[0],
			       i);
#endif
			commande.nom[i] = read_buf[0];
			i++;
			read(stdin_fd, read_buf, 1);
		}

#ifdef DEBUG
		printf("read command : %s\n", commande.nom);
		fflush(stdin);
#endif

		/* clear the arguments */
		for (i = 0; i < 10; i++) {
			for (y = 0; y < 10; y++)
				commande.param[i][y] = '\0';
		}

		/*
		 * get the command argument only if there are some needed
		 * not for list or meminfo command
		 */

		/* only one for fg and modinfo  */
		if (strcmp(commande.nom, "fffff") == 0 ||
		    strcmp(commande.nom, "gkgkgkgk") == 0) {

			y = 0;
			i = 0;
			read(stdin_fd, read_buf, 1);

			/* reading the command name */
			while ((read_buf[0] != '\n')) {
				commande.param[y][i] = read_buf[0];
				i++;
				read(stdin_fd, read_buf, 1);
			}

			if (commande.param[y][0] == '&')
				commande.asynchrone = 1;
#ifdef DEBUG
			printf("read command arg %d : %s\n",
			       y,
			       commande.param[y]);
			fflush(stdin);
#endif
		}

		/* at least one for the other  */
		else {
			y = 0;
			/* reading the command name */
			while ((read_buf[0] != '\n')) {
				i = 0;

				read(stdin_fd, read_buf, 1);

				while (read_buf[0] != ' ' &&
				       read_buf[0] != '\n') {
					commande.param[y][i] = read_buf[0];
					i++;
					read(stdin_fd, read_buf, 1);
				}

#ifdef DEBUG
				printf("read command arg %d : %s\n", y,
				       commande.param[y]);
				fflush(stdin);
#endif
				/* read (stdin_fd, read_buf, 1); */
				y++;
			}

			if (y != 0)
				y = y - 1;

			if (commande.param[y][0] == '&')
				commande.asynchrone = 1;
		}

		/*
		 * ioctl function call or exit
		 */

		if (strcmp(commande.nom, "fg") == 0) {
			req = FG_IOR;
		} else if (strcmp(commande.nom, "kill") == 0) {
			req = KILL_IOR;
		} else if (strcmp(commande.nom, "wait") == 0) {
			req = WAIT_IOR;
		} else if (strcmp(commande.nom, "meminfo") == 0) {
			req = MEMINFO_IO;
		} else if (strcmp(commande.nom, "modinfo") == 0) {
			req = MODINFO_IOR;
		} else if (strcmp(commande.nom, "list") == 0) {
			req = LIST_IO;
		} else if (strcmp(commande.nom, "quit") == 0 ||
			   strcmp(commande.nom, "exit") == 0 ||
			   strcmp(commande.nom, "q") == 0) {
			printf("Terminaison du programme !\n");
			return 0;
		}

		/* printf("asynchrone : %d\n", commande.asynchrone); */
		ioctl(module_fd, req, &commande);

		if (commande.asynchrone == 0) {
			while (commande.retour[0] == '\0') {
				sleep(1);
				printf("dans le while\n");
			}
		}
		/* printf("while fini\n"); */
		fflush(stdin);
		printf("retour : %s", commande.retour);
		if (commande.retour != NULL)
			free(commande.retour);
	}
	return 0;
}
