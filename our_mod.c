/* linux definition */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

/* our definition */
#include "our_mod.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NOUS, 2017");
MODULE_DESCRIPTION("Module bash en ioctl");

/*  EXAMPLE of module PARAM 
 * module_param (PORT_LED0, int, 0);
 * MODULE_PARM_DESC (PORT_LED0, "num de port de la led 0");
*/


/* function for our ioctl  
 * put the #define for request number in our_mod.h 
 *
*/
int device_ioctl (int fd, int request, int param) {

  /* 
   * test if fd existe  
   * si non retourner une erreur 
  */
  if (fcntl(fd, F_GETFL) < 0 && errno == EBADF) {
    // file descriptor is invalid or closed
    perror ("wrong file descriptor");
    exit (2);
  }

  
  /* case request is */
  switch (request) {

  case LIST_IO :

  case FG_IO :

  case KILL_IO :

  case WAIT_IO :

  case MEMINFO_IO :

  case MODINFO_IO :

  default :
    perror ("ioctl function that does not exist");
    exit (2);
  }
  
  return 0;
}


/* operation of our driver */
static struct file_operations Fops { .ioctl = device_ioctl };


static int __init mon_module_init(void) {

  pr_info("Coucou toi..!\n");

  /* ajout dans un fs avec numero maj et tout */



  /* petit print de toutes nos variables et tout pour voir si tout va bien :)  */
   return 0;
}


static void __exit mon_module_cleanup(void) {

  pr_info("au revoir toi..!\n");

  return 0;
}


module_init(mon_module_init);
module_exit(mon_module_cleanup);
