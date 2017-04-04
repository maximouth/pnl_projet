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
  /* to store the command (length max 7 char see doc)*/
  char * command = malloc (7 * sizeof (char));
  /* a counter */
  int i = 0, y = 0;
  /* tab of arguments max 10  */
  char * command_arg[10];
  /* readding buf */
  char * read_buf = malloc (1 * sizeof (char));
  unsigned long req = 0;
  
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


  /* get space for the argument */
  for (i = 0 ; i < 10 ; i++) {
    command_arg[i] = malloc (10 * sizeof (char));
  }
  
  printf ("Bienvenu! \n");

  while (1) {


    
    
    printf ("Veuillez entrer votre commande :\n");
    fflush (stdin);

    for (i = 0 ; i < 6 ; i++) {
      command[i] = '\0'; 
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
      command[i] = read_buf[0];
      i++;
      read (stdin_fd, read_buf, 1);
    }

#ifdef DEBUG
    printf ("read command : %s\n", command);
    fflush (stdin);
#endif

    /* gestion des arguments plus casse couille...  */

    /* clear the arguments  */
    for (y = 0 ; y < 6 ; y++) {
      for (i = 0 ; i < 6 ; i++) {
	    command_arg[y][i] = '\0'; 
	  }
    }    

    /* get the command argument only if there are some needed *  
     * not for list or meminfo command                        *
     */

    /* only one for fg and modinfo  */
    if ( strcmp (command, "fg") == 0 ||
	 strcmp (command, "modinfo") == 0   ) {

      y = 0;
      i = 0;
      read (stdin_fd, read_buf, 1);
      
      /* reading the command name */
      while ( (read_buf[0] != '\n' ) ) {

#ifdef DEBUG
	//	printf ("lecture du while BUF : %c, i : %d\n", read_buf[0], i);
#endif	  
	command_arg[y][i] = read_buf[0];
	i++;
	read (stdin_fd, read_buf, 1);
      }
      
#ifdef DEBUG
      printf ("read command arg %d : %s\n", y, command_arg[y]);
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
	command_arg[y][i] = read_buf[0];
	i++;
	read (stdin_fd, read_buf, 1);
      }
      
#ifdef DEBUG
      printf ("read command arg %d : %s\n", y, command_arg[y]);
      fflush (stdin);
#endif
      //      read (stdin_fd, read_buf, 1);
      y++;
      }

    }      

    /*
     * call the wright ioctl function
     */
    
    if (strcmp (command, "fg") == 0 ) {
      req = FG_IOR;
    }
    else if (strcmp (command, "kill") == 0 ) {
      req = KILL_IOR;
    }
    else if (strcmp (command, "wait") == 0 ) {
      req = WAIT_IOR;
    }
    else if (strcmp (command, "meminfo") == 0 ) {
      req = MEMINFO_IO;
    }
    else if (strcmp (command, "modinfo") == 0 ) {
      req = MODINFO_IOR;
    }
    else if (strcmp (command, "list") == 0 ) {
      req = LIST_IO;
    }
    
    ioctl (module_fd, req, command_arg);

  }

  
  return 0;
}
