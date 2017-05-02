#ifndef _OUR_MOD_H
#define _OUR_MOD_H


struct commande {
  char *nom;
  char **param;
  int asynchrone;
  char *retour;
};


/* definition of number for ioctl request  */
#define LIST_IO     10
#define FG_IOR      11
#define KILL_IOR    12
#define WAIT_IOR    13
#define MEMINFO_IO 14
#define MODINFO_IOR 15


#endif
