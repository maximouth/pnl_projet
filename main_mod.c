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

#define NB_CMD 14

const char list_cmd[NB_CMD][1024] = {
	"^list &\0$",
	"^list\0$",
	"^fg [0-9]+ &\0$",
	"^fg [0-9]+\0$",
	"^kill [0-9]+ [0-9]+ &\0$",
	"^kill [0-9]+ [0-9]+\0$",
	"^pid [0-9]+( [0-9])* &\0$",
	"^pid [0-9]+( [0-9])*\0$",
	"^meminfo &\0$",
	"^meminfo\0$",
	"^modinfo [:alnum:]+ &\0$",
	"^modinfo [:alnum:]+\0$",
	"^exit\0$",
	"^quit\0$"
};

int cmd_number(char *ptr, int size)
{
	char str[size];
	int i = 0;
	regex_t preg[NB_CMD];

	while (isspace(*ptr))
		ptr++;

	if (*ptr == '\0')
		return -1;

	str[0] = *ptr;
	while (i < size - 1) {
		if (isspace(*ptr) && isspace(*(ptr + 1))) {
			ptr++;
			continue;
		}
		ptr++;
		i++;
		str[i] = *(ptr);
	}

	for (i = 0; i < NB_CMD; i++) {
		if (regcomp(&preg[i], list_cmd[i], REG_NOSUB | REG_EXTENDED))
			return -1;
	}

	for (i = 0; i < NB_CMD; i++) {
		if (regexec(&preg[i], str, 0, NULL, 0) == 0)
			return i;
	}

	for (i = 0; i < NB_CMD; i++)
		regfree(&preg[i]);
	return -1;
}

int main(int argc, char **argv)
{
	/* file descriptor */
	int stdin_fd;
	int module_fd;
	int i = 0, y = 0, taille = 100;
	char read_buf[taille];
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

		/* clear the arguments */
		for (i = 0; i < 10; i++) {
			for (y = 0; y < 10; y++)
				commande.param[i][y] = '\0';
		}

		/*
		 * ioctl function call or exit
		 */

		switch (cmd_number(read_buf, taille)) {
		case 0:
			req = LIST_IO;
			commande.asynchrone = 1;
			break;
		case 1:
			req = LIST_IO;
			commande.asynchrone = 0;
			break;
		case 2:
			req = FG_IO;
			commande.asynchrone = 1;
			break;
		case 3:
			req = FG_IO;
			commande.asynchrone = 0;
			break;
		case 4:
			req = KILL_IO;
			commande.asynchrone = 1;
			break;
		case 5:
			req = KILL_IO;
			commande.asynchrone = 0;
			break;
		case 6:
			req = PID_IO;
			commande.asynchrone = 1;
			break;
		case 7:
			req = PID_IO;
			commande.asynchrone = 0;
			break;
		case 8:
			req = MEMINFO_IO;
			commande.asynchrone = 1;
			break;
		case 9:
			req = MEMINFO_IO;
			commande.asynchrone = 0;
			break;
		case 10:
			req = MODINFO_IO;
			commande.asynchrone = 1;
			break;
		case 11:
			req = MODINFO_IO;
			commande.asynchrone = 0;
			break;
		case 12:
		case 13:
			printf("Terminaison du prgramme !\n");
			return 0;
		default:
			printf("Commande inconnue !\n");
			printf("Reessayez ou exit/quit pour quitter\n");
		}

		ioctl(module_fd, req, &commande);

		if (commande.asynchrone == 0) {
			while (commande.retour[0] == '\0') {
				sleep(1);
				printf("dans le while\n");
			}
		}
		fflush(stdin);
		printf("retour : %s", commande.retour);
		if (commande.retour != NULL)
			free(commande.retour);
	}
	return 0;
}
