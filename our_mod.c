/* linux definition */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <linux/hugetlb.h>
#include <linux/mman.h>
#include <linux/mmzone.h>
#include <linux/proc_fs.h>
#include <linux/quicklist.h>
#include <linux/seq_file.h>
#include <linux/swap.h>
#include <linux/vmstat.h>
#include <linux/atomic.h>
#include <linux/vmalloc.h>
#ifdef CONFIG_CMA
#include <linux/cma.h>
#endif
#include <asm/page.h>
#include <asm/pgtable.h>

/* our definition */
#include "our_mod.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NOUS, 2017");
MODULE_DESCRIPTION("Module bash en ioctl");
MODULE_VERSION("1.0");

static int PORT_LED0;
static int PORT_LED1;

module_param (PORT_LED0, int, 0);
MODULE_PARM_DESC (PORT_LED0, "num de port de la led 0");
module_param (PORT_LED1, int, 0);
MODULE_PARM_DESC (PORT_LED1, "num de port de la led 1");



/*  EXAMPLE of module PARAM 
 * module_param (PORT_LED0, int, 0);
 * MODULE_PARM_DESC (PORT_LED0, "num de port de la led 0");
*/

static struct commande *command_list;
static int cmd_cpt;

/***********************************************************************
 *   IOCTL fonctions definition
 * 
 **********************************************************************/

/* parcourrir les diffentes commandes utilisées  */
static char * io_list (int max) {
  int i,y = 0;
  // faire un kmalloc je pense pour pouvoir passer retour sans perte de donnée
  char *retour = kmalloc (1024 * sizeof (char), GFP_KERNEL);
  char str[15];

  strcat (retour, "id|command\n");
  
  for ( i = 0; i < max ; i ++) {
    pr_info ("nom : %s \n", command_list[i].nom);
    sprintf(str, "%d", i);
    strcat (retour, str);
    strcat (retour, " ");
    strcat (retour, command_list[i].nom);
    strcat (retour, " ");
    while ( (command_list[i].param[y] != NULL) && (y < 10)) {
      pr_info ("param %d : %s \n", y, command_list[i].param[y]);
      strcat (retour, command_list[i].param[y]);
      strcat (retour, " ");
      y++;
    }
    strcat (retour, "\n");
  }
    strcat (retour, "\n");
  return retour;
}

/* afficher l'etat de la memoire */
static char * io_meminfo (void) {
  struct sysinfo i;
  char *retour = kmalloc (1024 * sizeof (char), GFP_KERNEL);
  char str[15];
  
  si_meminfo(&i);
  // rajouter si_swapinfo(&i);
  // impossible inserer module si c'est ecrit
  
  strcat(retour, "MemTotal :");
  sprintf(str, "%ld", i.totalram);
  strcat (retour, str);
  strcat(retour, "\n");

  strcat(retour, "MemFree :");
  sprintf(str, "%ld", i.freeram);
  strcat (retour, str);
  strcat(retour, "\n");

  strcat(retour, "Buffers :");
  sprintf(str, "%ld", i.bufferram);
  strcat (retour, str);
  strcat(retour, "\n");

  strcat(retour, "HighTotal :");
  sprintf(str, "%ld", i.totalhigh);
  strcat (retour, str);
  strcat(retour, "\n");

  strcat(retour, "HighFree :");
  sprintf(str, "%ld", i.freehigh);
  strcat (retour, str);
  strcat(retour, "\n");

  strcat(retour, "LowTotal :");
  sprintf(str, "%ld", i.totalram - i.totalhigh);
  strcat (retour, str);
  strcat(retour, "\n");

  strcat(retour,"LowFree :");
  sprintf(str, "%ld", i.freeram - i.freehigh);
  strcat (retour, str);
  strcat(retour, "\n");	  
	  
  return retour;
}

/* afficher l'etat de la memoire */
static char * io_modinfo (int val) {
  char *retour = kmalloc (1024 * sizeof (char), GFP_KERNEL);
  // faire find_mod
  struct module * mod;
  int i = 0;
  char str[15];
    
  mod = find_module (command_list[val-1].param[0]);

  if (mod != NULL) {
    strcat (retour, "version " );
    strcat (retour , mod->version);
    strcat (retour, "\n" );
    strcat (retour, "name " );
    strcat (retour , mod->name);
    strcat (retour, "\n" );
    strcat (retour, "base addr " );
    strcat (retour , mod->module_core);
    strcat (retour, "\n" );
    strcat (retour, "nb param " );
    sprintf(str, "%u", mod->num_kp);
    strcat (retour , str);
    strcat (retour, " :\n" );

    /* reste l'adresse à trouver */

    /* afficher la valeur du parametre aussi */
    while (i < mod->num_kp ) {
      strcat (retour, "->param ");
      sprintf(str, "%d", i);
      strcat (retour , str);
      strcat (retour , " : name ");
      strcat (retour , mod->kp[i].name );
      strcat (retour, "\n" );
      i++;
    }
    
  }
  else {
    strcat (retour, "module inexistant\n");
  }

  pr_info ("fini modinfo\n");
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
  char *retour = "toto\0";
  struct commande* args = (struct commande *) param;
  struct commande args_cpy;
  int i = 0;
  
  /* copy_from_user */
  if (copy_from_user (&args_cpy, args, sizeof (struct commande)) != 0) {
    panic ("into 1st copy");
  }
  if (copy_from_user (&args_cpy.nom, args->nom, sizeof (args->nom)) != 0) {
    panic ("into 2nd copy");
  }
  if (copy_from_user (&args_cpy.param, args->param, sizeof (args->param)) != 0) {
    panic ("into 3rd copy");
  }
  for (i = 0 ; i < 10 ; i++) {
    if (copy_from_user (&args_cpy.param[i], args->param[i], sizeof (args->param[i])) != 0) {
      panic ("into copy arg %d", i);
    }
  }
  
  /* /\* remplissage structure arg*\/ */
  /* /\* */
  /*  * parcourir les parametres recu en arg et faire des strcpy */
  /*  * */
  /*  *\/ */

  pr_info ("nom %s \n" , args->nom);
  i = 0;
  
  while ( args->param[i] != NULL) {
    pr_info ("param %d %s \n" , i, args->param[i]);
    i++;
  }

  
  strcpy (command_list[cmd_cpt].nom , args->nom);
  i = 0;
  pr_info ("strcopy fait \n");
  
  while (i < 10) {
    if (args->param[i] != NULL) { 
      strcpy (command_list[cmd_cpt].param[i] , args->param[i]);
    }
    pr_info ("remplissage arg %d : %s\n", i, command_list[cmd_cpt].param[i] );
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
    retour = io_meminfo();
    cmd_cpt --;

    break;

  case MODINFO_IOR :
    pr_info ("into modinfo ioctl");
    cmd_cpt ++;
    retour = io_modinfo(cmd_cpt);
    cmd_cpt --;

    break;

  default :
    pr_info ("ioctl function that does not exist");
  }

  /* return copy value to the user  */

  copy_to_user (args->retour, retour, strlen (retour));
  
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

  cmd_cpt = 0;
  
  command_list = kmalloc (10 * sizeof (struct commande), GFP_KERNEL);
  for ( i = 0 ; i < 10 ; i ++) {
    command_list[i].nom   = kmalloc (10 * sizeof (char), GFP_KERNEL);
    command_list[i].param = kmalloc (10 * sizeof (char*), GFP_KERNEL);
    for ( y = 0 ; y < 10 ; y ++) {
      command_list[i].param[y] = kmalloc (10 * sizeof (char), GFP_KERNEL);
      command_list[i].nom[y]   = '\0';
    }

    //pr_info ("structure command list initialise\n");
      
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
    kfree (command_list[i]. nom);
    kfree (command_list[i].param);
  }
  kfree (command_list);
  pr_info("free des structures\n");
 
  
  pr_info("au revoir toi..!\n");
  
  return ;
}


module_init(mon_module_init);
module_exit(mon_module_cleanup);
