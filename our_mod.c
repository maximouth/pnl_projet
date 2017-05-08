/* linux definition */
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/cdev.h>
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
#include <linux/pid_namespace.h>
#include <linux/init_task.h>
#include <linux/syscalls.h>
#include <linux/init.h>
#include <linux/bootmem.h>
#include <linux/hash.h>
#include <linux/pid.h>

/* our definition */
#include "our_mod.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AySi, 2017");
MODULE_DESCRIPTION("Module bash en ioctl");
MODULE_VERSION("1.0");

static int PORT_LED0;
static int PORT_LED1;

module_param (PORT_LED0, int, 0);
MODULE_PARM_DESC (PORT_LED0, "num de port de la led 0");

module_param (PORT_LED1, int, 0);
MODULE_PARM_DESC (PORT_LED1, "num de port de la led 1");



/****************************************/
/***** Global variables declaration *****/

static struct commande *command_list;
static int cmd_cpt;

static int flags[10];
static char *retour_async;

struct work_user {
	struct work_struct wk_ws;
	char **param;
	char *retour;
	int async;
};

static int flag;

static struct workqueue_struct *work_station;

DECLARE_WAIT_QUEUE_HEAD(cond_wait_queue);

/***** End global variables declaration *****/
/********************************************/

/**************************************/
/***** ioctl functions definition *****/

static void io_list(struct work_struct *work);
static void io_fg(struct work_struct *work);
static void io_wait(struct work_struct *work);
static void io_kill(struct work_struct *work);
static void io_meminfo(struct work_struct *work);
static void io_modinfo(struct work_struct *work);


static void io_list(struct work_struct *work)
{

	int i, y = 0;
	char *retour = kmalloc(1024 * sizeof(char), GFP_KERNEL);
	char str[15];
	struct work_user *wu;
	int cpy;

	wu = container_of(work, struct work_user, wk_ws);

	if (wu->async == 1) {
		retour_async = kmalloc(1024 * sizeof(char), GFP_KERNEL);
		cpy = cmd_cpt - 1;
		pr_info("cpy : %d\n", cpy);
		flags[cpy] = 0;
		flag = 1;
		cmd_cpt++;
		wake_up(&cond_wait_queue);
		wait_event_interruptible(cond_wait_queue, flags[cpy] != 0);
		flags[cpy] = 0;
	}

	strcat(retour, "id|command\n");

	for (i = 0; i < cmd_cpt; i++) {
		pr_info("nom : %s\n", command_list[i].nom);
		sprintf(str, "%d", i);
		strcat(retour, str);
		strcat(retour, " ");
		strcat(retour, command_list[i].nom);
		strcat(retour, " ");
		while ((command_list[i].param[y] != NULL) && (y < 10)) {
			pr_info("param %d : %s\n", y, wu->param[y]);
			strcat(retour, command_list[i].param[y]);
			strcat(retour, " ");
			y++;
		}
		strcat(retour, "\n");
	}

	if (wu->async == 1)
		retour_async = retour;
	else
		wu->retour = retour;
	flag = 1;
	wake_up(&cond_wait_queue);

	pr_info("retour : %s\n", wu->retour);

}

static void io_fg(struct work_struct *work)
{

	char *retour = kmalloc(1024 * sizeof(char), GFP_KERNEL);
	struct work_user *wu;
	long int id;

	wu = container_of(work, struct work_user, wk_ws);

	kstrtol(wu->param[0], 10, &id);
	if ((id > 10) || (id < 0)) {
		retour = "wrong id";
		wu->retour = retour;
	} else {
		pr_info("iddsqfrsd : %ld\n", id);
		flags[id] = 1;
		wake_up(&cond_wait_queue);
		wait_event(cond_wait_queue, flag != 0);
		wu->retour = retour_async;
	}

}

static void io_wait(struct work_struct *work)
{

	char *retour = kmalloc(1024 * sizeof(char), GFP_KERNEL);
	struct work_user *wu;
	int cpy;
	struct pid **tab;
	struct task_struct **pid_list;
	int i = 0, tmp = 0, quit = 0;
	long int rr;

	wu = container_of(work, struct work_user, wk_ws);

	pr_info("dans le wait\n");
	if (wu->async == 1) {
		/* retour_async = kmalloc(1024 * sizeof(char), GFP_KERNEL); */
		cpy = cmd_cpt - 1;
		pr_info("cpy : %d\n", cpy);
		flags[cpy] = 0;
		flag = 1;
		cmd_cpt++;
		wake_up(&cond_wait_queue);
		wait_event_interruptible(cond_wait_queue, flags[cpy] != 0);
		flags[cpy] = 0;
	}

	pr_info("dans le wait apres if\n");

	tab = kmalloc(10 * sizeof(struct pid *), GFP_KERNEL);
	pid_list = kmalloc(10 * sizeof(struct task_struct *), GFP_KERNEL);

	for (i = 0; i < 10; i++) {
		tab[i] = kmalloc(sizeof(struct pid), GFP_KERNEL);
		pid_list[i] = kmalloc(sizeof(struct task_struct), GFP_KERNEL);
	}

	pr_info("dans le wait struc init\n");

	i = 0;

	/* fetch all the pid structure */
	while ((wu->param[i][0] != '\0') && (i < 10)) {
		pr_info("dans le while %d", i);
		if (wu->param[i] != NULL) {
			kstrtol(wu->param[i], 10, &rr);
			tab[i] = find_vpid(rr);
		}
		i++;
		tmp = i;
	}

	pr_info("remplissage pid fait\n");

	/* wait until one of them is no longer alive */
	while (1) {
		pr_info("dans le while\n");
		for (i = 0; i < tmp; i++) {
			if (tab[i] != NULL)
				pid_list[i] = get_pid_task(tab[i], PIDTYPE_PID);
			if (pid_list[i] != NULL) {
				if (!pid_alive(pid_list[i])) {
					quit = 1;
					pr_info("pid : %s dead", wu->param[i]);
				}
				pr_info("pid : %s alive", wu->param[i]);
			}
		}
		if ((quit == 1) || (tmp == 0)) {
			retour = "one of process is dead\n";
			if (wu->async == 1)
				retour_async = retour;
			else
				wu->retour = retour;
			flag = 1;
			wake_up(&cond_wait_queue);
			pr_info("retour : %s\n", wu->retour);
		}
	}
	for (i = 0; i < 10; i++) {
		kfree(tab[i]);
		kfree(pid_list[i]);
	}
	kfree(tab);
	kfree(pid_list);

}

static void io_kill(struct work_struct *work)
{

	char *retour = kmalloc(1024 * sizeof(char), GFP_KERNEL);
	int res_pid = 0;
	long rr;
	struct pid *pid;
	struct work_user *wu;
	int cpy;

	wu = container_of(work, struct work_user, wk_ws);

	if (wu->async == 1) {
		/* retour_async = kmalloc(1024 * sizeof(char), GFP_KERNEL); */
		cpy = cmd_cpt - 1;
		pr_info("cpy : %d\n", cpy);
		flags[cpy] = 0;
		flag = 1;
		cmd_cpt++;
		wake_up(&cond_wait_queue);
		wait_event_interruptible(cond_wait_queue, flags[cpy] != 0);
		flags[cpy] = 0;
	}

	/* get pid const struct */
	res_pid = kstrtol(wu->param[1], 10, &rr);

	pr_info("commande pid : %s\n", wu->param[1]);
	pr_info("res pid : %d, rr : %ld\n", res_pid, rr);

	pid = find_vpid(rr);

	/* get signal number */
	res_pid = kstrtol(wu->param[0], 10, &rr);

	pr_info("signal: %s\n", wu->param[0]);
	pr_info("res signal : %d, rr : %ld\n", res_pid, rr);

	if (pid == NULL)
		strcat(retour, "No such active pid\n");
	else if (kill_pid(pid, rr, 1) == 0)
		strcat(retour, "kill succed\n");
	else
		strcat(retour, "kill failed\n");

	if (wu->async == 1)
		retour_async = retour;
	else
		wu->retour = retour;
	flag = 1;
	wake_up(&cond_wait_queue);

}

static void io_meminfo(struct work_struct *work)
{

	struct work_user *wu;
	struct sysinfo i;
	char *retour = kmalloc(1024 * sizeof(char), GFP_KERNEL);
	char str[15];
	int cpy;

	wu = container_of(work, struct work_user, wk_ws);

	if (wu->async == 1) {
		/* retour_async = kmalloc(1024 * sizeof(char), GFP_KERNEL); */
		cpy = cmd_cpt - 1;
		pr_info("cpy : %d\n", cpy);
		flags[cpy] = 0;
		flag = 1;
		cmd_cpt++;
		wake_up(&cond_wait_queue);
		wait_event_interruptible(cond_wait_queue, flags[cpy] != 0);
		flags[cpy] = 0;
	}

	si_meminfo(&i);

	strcat(retour, "MemTotal :");
	sprintf(str, "%ld", i.totalram);
	strcat(retour, str);
	strcat(retour, "\n");

	strcat(retour, "MemFree :");
	sprintf(str, "%ld", i.freeram);
	strcat(retour, str);
	strcat(retour, "\n");

	strcat(retour, "Buffers :");
	sprintf(str, "%ld", i.bufferram);
	strcat(retour, str);
	strcat(retour, "\n");

	strcat(retour, "HighTotal :");
	sprintf(str, "%ld", i.totalhigh);
	strcat(retour, str);
	strcat(retour, "\n");

	strcat(retour, "HighFree :");
	sprintf(str, "%ld", i.freehigh);
	strcat(retour, str);
	strcat(retour, "\n");

	strcat(retour, "LowTotal :");
	sprintf(str, "%ld", i.totalram - i.totalhigh);
	strcat(retour, str);
	strcat(retour, "\n");

	strcat(retour, "LowFree :");
	sprintf(str, "%ld", i.freeram - i.freehigh);
	strcat(retour, str);
	strcat(retour, "\n");

	if (wu->async == 1)
		retour_async = retour;
	else
		wu->retour = retour;
	flag = 1;
	wake_up(&cond_wait_queue);


}

static void io_modinfo(struct work_struct *work)
{

	struct work_user *wu;
	char *retour = kmalloc(1024 * sizeof(char), GFP_KERNEL);
	struct module *mod;
	int i = 0;
	char str[15];
	int cpy;

	wu = container_of(work, struct work_user, wk_ws);

	if (wu->async == 1) {
		/* retour_async = kmalloc(1024 * sizeof(char), GFP_KERNEL); */
		cpy = cmd_cpt - 1;
		pr_info("cpy : %d\n", cpy);
		flags[cpy] = 0;
		flag = 1;
		cmd_cpt++;
		wake_up(&cond_wait_queue);
		wait_event_interruptible(cond_wait_queue, flags[cpy] != 0);
		flags[cpy] = 0;
	}

	mod = find_module(wu->param[0]);

	if (mod != NULL) {
		strcat(retour, "version ");
		strcat(retour, mod->version);
		strcat(retour, "\n");
		strcat(retour, "name ");
		strcat(retour, mod->name);
		strcat(retour, "\n");
		strcat(retour, "base addr ");
		strcat(retour, mod->module_core);
		strcat(retour, "\n");
		strcat(retour, "nb param ");
		sprintf(str, "%u", mod->num_kp);
		strcat(retour, str);
		strcat(retour, " :\n");

		while (i < mod->num_kp) {
			strcat(retour, "->param ");
			sprintf(str, "%d", i);
			strcat(retour, str);
			strcat(retour, " : name ");
			strcat(retour, mod->kp[i].name);
			strcat(retour, "\n");
			i++;
		}

	} else {
		strcat(retour, "module inexistant\n");
	}

	if (wu->async == 1)
		retour_async = retour;
	else
		wu->retour = retour;
	flag = 1;
	wake_up(&cond_wait_queue);

	pr_info("fini modinfo\n");


}

/***** End ioctl functions definition *****/
/******************************************/

/*************************/
/***** ioctl syscall *****/

long device_ioctl(struct file *filp, unsigned int request, unsigned long param)
{
	char *retour = kmalloc(1024 * sizeof(char), GFP_KERNEL);
	struct commande *args = (struct commande *)param;
	struct commande args_cpy;
	int i = 0;
	struct work_user *wk;

	pr_info("nom %s\n", args->nom);
	i = 0;

	while (i < 10) {
		pr_info("param %d %s\n", i, args->param[i]);
		i++;
	}

	/* copy_from_user */
	if (copy_from_user(&args_cpy, args, sizeof(struct commande)) != 0)
		panic("into 1st copy");
	if (copy_from_user(&args_cpy.nom, args->nom, sizeof(args->nom)) != 0)
		panic("into 2nd copy");
	if (copy_from_user(&args_cpy.param, args->param, sizeof(args->param))
	    != 0)
		panic("into 3rd copy");

	pr_info("nom %s\n", args->nom);
	i = 0;

	while (i < 10) {
		pr_info("param %d %s\n", i, args->param[i]);
		i++;
	}

	/* For workqueue */

	wk = kmalloc(sizeof(struct work_user), GFP_KERNEL);
	wk->param = kmalloc(10 * sizeof(char *), GFP_KERNEL);

	for (i = 0; i < 10; i++)
		wk->param[i] = kmalloc(10 * sizeof(char), GFP_KERNEL);

	wk->retour = kmalloc(1024 * sizeof(char), GFP_KERNEL);
	wk->retour[0] = '\0';

	strcpy(command_list[cmd_cpt].nom, args->nom);
	i = 0;
	pr_info("strcopy fait\n");

	while (i < 10) {
		if (args->param[i] != NULL) {
			strcpy(command_list[cmd_cpt].param[i], args->param[i]);
			strcpy(wk->param[i], args->param[i]);
		}
		pr_info("remplissage arg %d : %s\n", i,
			command_list[cmd_cpt].param[i]);
		i++;
	}

	wk->async = args->asynchrone;

	/* case request is */
	switch (request) {

	case LIST_IO:
		pr_info("into list ioctl");
		cmd_cpt++;

		flag = 0;
		INIT_WORK(&(wk->wk_ws), io_list);
		schedule_work(&(wk->wk_ws));
		/*
		 * queue_work(work_station, &(wk->wk_ws));
		 * retour = io_list (cmd_cpt);
		 */
		pr_info("avant wait");
		wait_event(cond_wait_queue, flag != 0);
		pr_info("apres wait");
		flag = 0;
		cmd_cpt--;
		break;

	case FG_IOR:
		pr_info("into fg ioctl");
		cmd_cpt++;

		flag = 0;
		INIT_WORK(&(wk->wk_ws), io_fg);
		schedule_work(&(wk->wk_ws));
		/* queue_work(work_station, &(wk->wk_ws)); */
		pr_info("avant wait");
		wait_event(cond_wait_queue, flag != 0);
		pr_info("apres wait");
		cmd_cpt--;
		break;

	case KILL_IOR:
		pr_info("into kill ioctl");
		cmd_cpt++;

		flag = 0;
		INIT_WORK(&(wk->wk_ws), io_kill);
		schedule_work(&(wk->wk_ws));
		pr_info("avant wait");
		wait_event(cond_wait_queue, flag != 0);
		pr_info("apres wait");
		flag = 0;
		cmd_cpt--;
		break;

	case WAIT_IOR:
		pr_info("into wait ioctl");
		cmd_cpt++;

		flag = 0;
		INIT_WORK(&(wk->wk_ws), io_wait);
		schedule_work(&(wk->wk_ws));
		pr_info("avant wait");
		wait_event(cond_wait_queue, flag != 0);
		pr_info("apres wait");
		flag = 0;
		cmd_cpt--;
		break;

	case MEMINFO_IO:
		pr_info("into meminfo ioctl");
		cmd_cpt++;

		/* if synchrone */
		flag = 0;
		INIT_WORK(&(wk->wk_ws), io_meminfo);
		schedule_work(&(wk->wk_ws));
		pr_info("avant wait");
		wait_event(cond_wait_queue, flag != 0);
		pr_info("apres wait");
		cmd_cpt--;
		break;

	case MODINFO_IOR:
		pr_info("into modinfo ioctl");
		cmd_cpt++;

		flag = 0;
		INIT_WORK(&(wk->wk_ws), io_modinfo);
		schedule_work(&(wk->wk_ws));
		pr_info("avant wait");
		wait_event(cond_wait_queue, flag != 0);
		pr_info("apres wait");
		cmd_cpt--;
		break;

	default:
		wk->retour = "wrong entry\n";
		pr_info("ioctl function that does not exist");
	}

	copy_to_user(args->retour, wk->retour, strlen(wk->retour));
	pr_info("RETOUR %s", retour);
	kfree(retour);
	for (i = 0; i < 10; i++)
		kfree(wk->param[i]);
	kfree(wk->param);
	kfree(wk->retour);
	kfree(wk);
	return 0;

}

/***** End ioctl syscall *****/
/*****************************/

/****************************************/
/***** Device init & exit functions *****/

/* operation of our driver */
static const struct file_operations fops_mod = {
	.unlocked_ioctl = device_ioctl
};

static int major;

static int __init mon_module_init(void)
{
	int i = 0, y = 0;

	pr_info("\n****** Insertion module ! ******\n");
	major = register_chrdev(0, "our_mod", &fops_mod);

	cmd_cpt = 0;

	retour_async = kmalloc(1024 * sizeof(char), GFP_KERNEL);
	command_list = kmalloc(10 * sizeof(struct commande), GFP_KERNEL);
	for (i = 0; i < 10; i++) {
		command_list[i].nom = kmalloc(10 * sizeof(char), GFP_KERNEL);
		command_list[i].param =
		    kmalloc(10 * sizeof(char *), GFP_KERNEL);
		for (y = 0; y < 10; y++) {
			command_list[i].param[y] = kmalloc(10 * sizeof(char),
							   GFP_KERNEL);
			command_list[i].nom[y] = '\0';
		}
	}

	work_station = create_workqueue("worker");

	if (work_station == NULL) {
		pr_alert("Workqueue creation failed in init");
		return -1;
	}

	return 0;
}

static void __exit mon_module_cleanup(void)
{
	int i = 0, y = 0;

	for (i = 0; i < 10; i++) {
		for (y = 0; y < 10; y++)
			kfree(command_list[i].param[y]);
		kfree(command_list[i].nom);
		kfree(command_list[i].param);
	}
	kfree(retour_async);
	kfree(command_list);
	destroy_workqueue(work_station);
	unregister_chrdev(major, "our_mod");
	pr_info("\n****** Suppression module ! ******\n");
}

module_init(mon_module_init);
module_exit(mon_module_cleanup);

/***** End device init & exit functions *****/
/********************************************/
