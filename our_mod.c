/* linux definition */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

/* our definition */
#include "our_mod.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NOUS, 2017");
MODULE_DESCRIPTION("Module bash en ioctl");

/*  EXAMPLE of module PARAM 
 * module_param (PORT_LED0, int, 0);
 * MODULE_PARM_DESC (PORT_LED0, "num de port de la led 0");
*/

//int device_ioctl (int fd, int request, int param) {
long device_ioctl(struct file *filp, unsigned int request, unsigned long param) {
  
  /* case request is */
  switch (request) {

  case LIST_IO :

  case FG_IO :

  case KILL_IO :

  case WAIT_IO :

  case MEMINFO_IO :

  case MODINFO_IO :

  default :
    pr_info ("ioctl function that does not exist");
  }
  
  return 0;
}


/* operation of our driver */
static struct file_operations fops_mod = {
  .unlocked_ioctl = device_ioctl
};
static int major;

static int __init mon_module_init(void) {

  pr_info("Coucou toi..!\n");
  major = register_chrdev (0, "our_mod", &fops_mod);
  /* ajout dans un fs avec numero maj et tout */

  /* petit print de toutes nos variables et tout pour voir si tout va bien :)  */
   return 0;
}


static void __exit mon_module_cleanup(void) {

  pr_info("au revoir toi..!\n");
  unregister_chrdev (major, "our_mod");
  return ;
}


module_init(mon_module_init);
module_exit(mon_module_cleanup);
