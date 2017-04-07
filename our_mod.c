/* linux definition */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/string.h>

/* our definition */
#include "our_mod.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NOUS, 2017");
MODULE_DESCRIPTION("Module bash en ioctl");

/*  EXAMPLE of module PARAM 
 * module_param (PORT_LED0, int, 0);
 * MODULE_PARM_DESC (PORT_LED0, "num de port de la led 0");
*/

static struct commande *command_list;
static int cmd_cpt = 0;

/***********************************************************************
 *   IOCTL fonctions definition
 * 
 **********************************************************************/

/* parcourrir les diffentes commandes utilisées  */
static char * io_list (int max) {
  int i,y = 0;
  char retour[1024];

  for ( i = 0; i < max ; i ++) {
    pr_info ("nom : %s ", command_list[i].nom);
    strcpy (retour, command_list[i].nom);
    strcpy (retour, " ");
    while (command_list[i].param[y] != NULL) {
      pr_info ("param %d : %s ", y, command_list[i].param[y]);
      strcpy (retour, command_list[i].param[y]);
      strcpy (retour, " ");
      y++;
    }
    strcpy (retour, "\n");
  }

  return retour;
}


/***********************************************************************
 *   END IOCTL fonctions definition
 * 
 **********************************************************************/


/***********************************************************************
 *   IOCTL syscall
 * 
 **********************************************************************/
long device_ioctl(struct file *filp, unsigned int request, unsigned long param) {
  char * retour;
  struct commande* args = (struct commande *) param;
  struct commande args_cpy;
  int i = 0;
  
  /* copy_from_user */
  copy_from_user (&args_cpy, args, sizeof (struct commande));
    
  /* remplissage structure arg*/
  /*
   * parcourir les parametres recu en arg et faire des strcpy
   *
   */
  strcpy (command_list[cmd_cpt].nom , args_cpy.nom);
  while (args_cpy.param[i] != NULL) {
    strcpy (command_list[cmd_cpt].param[i] , args_cpy.param[i]);
    i ++;
  }
  
  
  /* case request is */
  switch (request) {

  case LIST_IO :
    pr_info ("into list ioctl");
    cmd_cpt ++;
    retour = io_list (cmd_cpt);
    cmd_cpt --;
    break;

  case FG_IOR :
    pr_info ("into fg ioctl");
    cmd_cpt ++;

    break;

  case KILL_IOR :
    pr_info ("into kill ioctl");
    cmd_cpt ++;

    break;

  case WAIT_IOR :
    pr_info ("into wait ioctl");
    cmd_cpt ++;

    break;

  case MEMINFO_IO :
    pr_info ("into meminfo ioctl");
    cmd_cpt ++;

    break;

  case MODINFO_IOR :
    pr_info ("into modinfo ioctl");
    cmd_cpt ++;

    break;

  default :
    pr_info ("ioctl function that does not exist");
  }

  /* return copy value to the user  */

  
  
  return 0;
}


/* operation of our driver */
static struct file_operations fops_mod = {
  .unlocked_ioctl = device_ioctl
};

static int major;


/***********************************************************************
 *   INIT device function
 * 
 **********************************************************************/
static int __init mon_module_init(void) {
  int i = 0, y = 0;
  
  pr_info("Coucou toi..!\n");
  major = register_chrdev (0, "our_mod", &fops_mod);

  command_list = kmalloc (10 * sizeof (struct commande), __GFP_WAIT | __GFP_FS);
  for ( i = 0 ; i < 10 ; i ++) {
    //command_list[i].nom = kmalloc (10 * sizeof (char), __GFP_WAIT | __GFP_FS);
    //command_list[i].param = kmalloc (10 * sizeof (char), __GFP_WAIT | __GFP_FS);
    for ( y = 0 ; y < 10 ; y ++) {
      //command_list[i].param[y] = "\0";
    }
    //command_list[i].nom = "\0";
    pr_info ("structure command list initialise\n");
      
  }
  
  
    return 0;
}

/***********************************************************************
 *   EXIT device function
 * 
 **********************************************************************/
static void __exit mon_module_cleanup(void) {

  int i = 0;
  unregister_chrdev (major, "our_mod");
  pr_info("liberation numero majeur\n");
 
  /* faire free sur la structure commandelist  */
  for ( i = 0 ; i < 10 ; i ++) {
    //kfree (command_list[i]. nom);
    //kfree (command_list[i].param);
  }
  //kfree (command_list);
  pr_info("free des structures\n");
 
  
  pr_info("au revoir toi..!\n");
  
  return ;
}


module_init(mon_module_init);
module_exit(mon_module_cleanup);
