/* Include lib C  */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

/* our Include */
#include "our_mod.h"

int main (int argc, char ** argv) {
  /* file descriptor stdin */
  int stdin_fd;
  /* file descriptor for kernet module */
  int module_fd;
  /* a counter */
  int i = 0, y = 0;
  /* readding buf */
  char * read_buf = malloc (1 * sizeof (char));
  unsigned long req = 0;
  struct commande commande;

  commande.nom = malloc (10 * sizeof (char));
  for (i = 0 ; i < 10 ; i ++) {
    commande.nom[i] = '\0';
  }

  commande.param = malloc (10 * sizeof (char));

  for (i = 0 ; i < 8 ; i ++) {
    commande.param[i] = malloc (10 * sizeof (char));
	  
    for (y = 0 ; y < 10 ; y ++) {
      commande.param[i][y] = '\0'; 
    }
  }

  commande.retour[0] = 'a';
  commande.retour[1] = 'b';
  commande.retour[2] = '\0';
      
  
  printf ("struct commande initialisée\n");
  fflush (stdin);
   
  
  if (argc != 2) {
    perror ("mauvaise utilisation\n./Projet.x \"chemin du module\"\n");
    exit (1);
  }

  /* open stdin */
  if ( (stdin_fd  = open ("/dev/null", O_RDWR )) < 0) {
    perror ("error open stdin");
    exit (1);
  }

  /* get the stdin fd */
  if (dup2(STDIN_FILENO, stdin_fd) == -1) {
    perror ("dup2 stdin");
    exit(1);
  }
  
  /* open module to have acces to it */
  if ( (module_fd = open (argv[1], O_RDWR)) < 0) {
      perror ("error open module");
      exit (1);
    }
  
  printf ("Bienvenu! \n");

  while (1) {
    
    printf ("Veuillez entrer votre commande :\n");
    fflush (stdin);

  for (i = 0 ; i < 10 ; i ++) {
    commande.nom[i] = '\0'; 
  }
    
    /* starting the main program  */
    i = 0;

    /* wait for something to read */
    while (i == 0) {
      i = read (stdin_fd, read_buf, 1);
      
    }
    i = 0;
    
    /* reading the command name */
    while ( (read_buf[0] != '\n') &&
	    (read_buf[0] != '\r') &&
	    (read_buf[0] != ' ' ) 
	    ) {

#ifdef DEBUG
      printf ("lecture du while BUF : %c, i : %d\n", read_buf[0], i);
#endif	  
      commande.nom[i] = read_buf[0];
      i++;
      read (stdin_fd, read_buf, 1);
    }

#ifdef DEBUG
    printf ("read command : %s\n", commande.nom);
    fflush (stdin);
#endif

    /* gestion des arguments plus casse couille...  */

    //    clear the arguments
    for (i = 0 ; i < 4 ; i ++) {
      for (y = 0 ; y < 10 ; y ++) {
    	commande.param[i][y] = '\0';
	printf ("clear i %d ; y %d\n", i , y);
	fflush (stdin);	
      }
    }

    /* get the command argument only if there are some needed *  
     * not for list or meminfo command                        *
     */

    /* only one for fg and modinfo  */
    if ( strcmp (commande.nom, "fg") == 0 ||
	 strcmp (commande.nom, "modinfo") == 0   ) {

      y = 0;
      i = 0;
      read (stdin_fd, read_buf, 1);
      
      /* reading the command name */
      while ( (read_buf[0] != '\n' ) ) {

#ifdef DEBUG
	//	printf ("lecture du while BUF : %c, i : %d\n", read_buf[0], i);
#endif	  
	commande.param[y][i] = read_buf[0];
	i++;
	read (stdin_fd, read_buf, 1);
      }
      
#ifdef DEBUG
      printf ("read command arg %d : %s\n", y, commande.param[y]);
      fflush (stdin);
#endif
    }

  /* at least one for the other  */
    else {

      y = 0;
      /* reading the command name */
      while ( (read_buf[0] != '\n' ) ) {
	i = 0;

      read (stdin_fd, read_buf, 1);
	
	while (read_buf[0] != ' '   &&
	       read_buf[0] != '\n'
	       )  {
#ifdef DEBUG
	  //	  printf ("lecture du while BUF : %c, i : %d\n", read_buf[0], i);
#endif	  
	commande.param[y][i] = read_buf[0];
	i++;
	read (stdin_fd, read_buf, 1);
      }
      
#ifdef DEBUG
      printf ("read command arg %d : %s\n", y, commande.param[y]);
      fflush (stdin);
#endif
      //      read (stdin_fd, read_buf, 1);
      y++;
      }

    }      

    /*
     * call the wright ioctl function
     */
    
    if (strcmp (commande.nom, "fg") == 0 ) {
      req = FG_IOR;
    }
    else if (strcmp (commande.nom, "kill") == 0 ) {
      req = KILL_IOR;
    }
    else if (strcmp (commande.nom, "wait") == 0 ) {
      req = WAIT_IOR;
    }
    else if (strcmp (commande.nom, "meminfo") == 0 ) {
      req = MEMINFO_IO;
    }
    else if (strcmp (commande.nom, "modinfo") == 0 ) {
      req = MODINFO_IOR;
    }
    else if (strcmp (commande.nom, "list") == 0 ) {
      req = LIST_IO;
    }
    
    ioctl (module_fd, req, &commande);
    printf ("%s\n", commande.retour);
  }

  
  return 0;
}
