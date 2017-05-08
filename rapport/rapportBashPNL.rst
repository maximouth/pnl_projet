.. footer:: page ###Page###

==============================
Rapport de Projet **Bash** PNL
==============================

--------------------------------------------
PNL -  Programmation au coeur du noyau linux
--------------------------------------------

|
|
|
|

*Ayrault Maxime* **3203694** - *Sivarajah Sivapiryan* **3201389**

|
|
|
|
|
|
|

	   
----------------------------------------------------------

Introduction
============

|
|

|

| Ce document présente le rapport du projet **Bash** de l'UE *PNL*.
| Le projet a pour but de réaliser un invite de commande pour le noyau linux et de le tester
| sur une machine virtuelle. 
| Lors de ce Projet nous nous sommes servis d'une VM *arch linux* en version noyau 4.2.3.
| Nous avons découvert l'utilisation d'*ioctl*, l'intégration d'un module dans un noyau ainsi 
| que l'architecture du noyau et les workqueue.
|

Ce projet se décompose en 2 parties :

 - L'application utilisateur.
 - Le module noyau.

|

Ce projet a été développé sous git :
 https://github.com/maximouth/pnl_projet

|
|
|
|
|
|
|
|

 

--------------------------------------------



I) Mode d'emploi
================

| 

Pour faciliter la compilation, le transfert et l'initialisation des différents modules et applications, nous avons créé deux petits scripts Bash :

  - upload.sh
  - init.sh

|
|
|
|
| upload.sh sert si l'on a la partition root montée dans le dossier vm_root du répertoire courant

.. code:: sh
   
   #!/bin/bash
   
   rm -f vm_root/projet/our_mod.ko
   
   umount vm_root/
   mount root.img vm_root/
   
   make clean all
   
   cp our_mod.ko vm_root/projet/
   cp our_mod.h  vm_root/projet/
   cp main.c     vm_root/projet/
   cp init.sh    vm_root/
   cp toto.c     vm_root/projet/
   cp sleep.c    vm_root/projet/
   
   sync
   
   ./qemu-run-externKernel.sh
   
|
|

upload.sh permet de :

 - démonter /monter la partition,
 - compiler le module,
 - copier différents fichiers sur la partition,
 - synchroniser pour Ãªtre sur que tout soit bien écrit
 - lancer la machine virtuelle.


	  
|
|
|
|
|
|
|
|
| init.sh sert une fois la machine virtuelle démarrée

.. code:: sh

  #!/bin/bash
  
  cd projet
  dmesg -C
  
  insmod our_mod.ko
  mknod /dev/hello c 245 0
  
  dmesg
  
  make -f Makefile_app
  
  gcc -Wall -o toto.x toto.c
  ./toto.x &

  gcc -o sleep sleep.c
  ./sleep &
	  
  ps
  
  ./Projet.x /dev/hello
  
|
|

init.sh permet de :

 - nettoyer le dmesg,
 - insérer le module,
 - créer le neud,
 - lancer une petite application (toto.x) en arrière plan 
 - compiler l'application utilisateur et la lancer.


|
|
|
|
|
|
|
|
|
|
|

---------------------------

|
|
|



II) architecture du projet
==========================

|

Notre projet se décompose en 3 parties :

 - le fichier *our_mod.h* contient les différentes structures que nous
   allons utiliser tout au long du projet.
 - le fichier *main.c* correspond à l'application utilisateur qui
   récupère les demandes de l'utilisateur et effectue les appels système.
 - le fichier *our_mod.c* correspond à notre module. Il s'occupe de
   récupérer les appels système à **ioctl** et d'effectuer les
   actions correspondantes à ce que l'utilisateur a demandé.

|



III) structures
==============

|
| Pour les appels à **ioctl** nous avons dû créer une structure nous permettant de traiter les différentes
| commandes.   
|

.. code:: c

  struct commande {
    char *nom;
    char **param;
    int asynchrone;
    char *retour;
  };

Notre structure est composée de plusieurs champs :

 - **nom**, qui sert à contenir le nom de la commande.
 - **param**, un tableau de *char**, contenant les différents
   arguments que l'utilisateur peut passer avec la commande.
 - **asynchrone**, qui indique si la commande a été tapée avec un "&"
   à la fin, ce qui permet de la faire s'exécuter en arrière plan.
 - **retour**, qui est un pointeur contenant la chaine de caractère à
   afficher en retour de l'appel système.

|
|
|
| Dans le fichier nous avons aussi les ``#define`` des différents numéros de fonction d'appel à **ioctl**.

.. code:: c
  
  #define LIST_IO     10
  #define FG_IOR      11
  #define KILL_IOR    12
  #define WAIT_IOR    13
  #define MEMINFO_IO  14
  #define MODINFO_IOR 15
  

IV) Appli utilisateur
=====================


| Notre application utilisateur prend en argument le nom du noeud sur lequel est rattaché notre module.
| Elle se lance avec :

.. code:: bash

	  ./Projet.x /dev/xxx

| Et voici ce que nous obtenons en la lançant.
|

.. image:: launch.png
   :scale: 50 %
   :alt: lancement application utilisateur
   :align: center

|


L'utilisateur peut entrer les commandes suivantes qui seront traitées par notre appel système :

 - **list**, qui affiche les processus en cours d'exécution dans le module.
 - **fg <id>**, qui remet en premier plan une application en train de
   s'exécuter de façon asynchrone.
 - **wait <pid> [<pid>...]**, qui permet d'attendre la fin d'un processus donné
   en argument.
 - **kill <signal> <pid>**, qui permet d'envoyer un signal au processus
   pointé par le pid.
 - **meminfo**, qui affiche l'état actuel de la mémoire.
 - **modinfo <name>**, affiche les différentes informations liées au
   module passé en argument.

|
| Si une autre commande, que celles prévues, est demandée par l'utilisateur ou si il y a une faute de frappe,
| une erreur est retournée et l'utilisateur est invité à lancer une nouvelle commande.
|

------------------------------------

| Dans ce programme, on commence par initialiser la structure commande qui servira tout au
| long de la durée de l'application
| Puis on teste si le noeud existe, si il n'existe pas l'application s'arrÃªte sinon, on ouvre l'entrée
| standard pour pouvoir lire les commandes de l'utilisateur.
| 
| On entre ensuite dans la boucle ``while (1)`` du programme qui est la boucle principale. C'est ici
| qu'on lit les différentes commandes et arguments, et où on fait l'appel à l'ioctl 
|
.. code:: c

   ioctl (module_fd, req, &commande);	  

|
| *module_fd* correspond au descripteur de fichier du noeud ``/dev/xxx``.
| *req* correspond au numéro de la fonction à appeler.
| *&commande* correspond à l'adresse de la structure commande contenant tous les arguments à
| passer à la fonction appelée.

Il y a ensuite l'affichage de la chaine de caractères en retour de l'**ioctl**.


V) Module
=========

|
|

Notre module est découpé en 4 morceaux :
 - **initialisation** du module
 - **destruction** du module
 - la fonction **ioctl**
 - les différentes **fonctions** appelées par l'appel système.




Initialisation
&&&&&&&&&&&&&&

|
| Dans l'initialisation nous commençons par trouver un numéro majeur à notre module pour qu'il
| ait une place dans le ``/proc/devices`` ce qui nous permettera de créer un noeud plus tard avec
| un ``mknod``

.. code :: c

  major = register_chrdev (0, "our_mod", &fops_mod);

|
| ``fops_mod`` contient la liste des appels système que notre module gère et les fonctions associées
| à chaque appel.
| Pour le projet nous n'avons que **ioctl** qui est représenté.


| Pour pouvoir sauvegarder la fonction qui 'exécute à un instant donné, nous avons besoin d'un
| tableau de ``structure commande`` *(voir list)*

| Dans l'initialisation nous continuons par allouer de la mémoire kernel avec des kmalloc
| pour chaque composant du tableau, nous avons limité le nombre de commandes
| en simultanées à 10.
|
| Dans l'initialisation du module, nous avons aussi besoin de creer une workqueue.

.. code:: C
	  
  work_station = create_workqueue("worker");

| Nous finissons par verifier qu'elle est bien crée et l'initialisation du module est finie.
|
|
|
|
|
|
|
|
|
|
|
|
|
|

Destruction
&&&&&&&&&&&

|
| Pour la destruction nous devons retirer notre module des numéros  majeurs de devices afin
| de ne pas saturer la liste.

.. code :: c

  unregister_chrdev (major, "our_mod");

| Une fois notre module retiré des numéros majeurs, il faut libérer la mémoire pour éviter
| les fuites mémoire.
| Nous faisons donc des ``kfree`` pour libérer les ressources allouées à notre structure.

| Nous devons aussi detruire la worqueue crée dans le init.
| 
|
|

Ioctl
&&&&&


Notre fonction ``device_ioctl`` est appelée pour chaque appel à **ioctl**.

| Nous commençons par copier la structure ``commande`` passée en  argument par l'utilisateur,
| vers les adresses adressables cÃ´té kernel grÃ¢ce à la fonction ``copy_from_user``. Nous
| ramenons du cÃ´té kernel chaque partie de la structure.
|
| Avec les valeurs récupérées nous les copions dans notre structure ``work_user`` *(voir gestion workqueue)*
| afin de pouvoir les utiliser dans les différentes fonctions suivante.
|
| Nous entrons ensuite dans un switch qui appelle la fonction correspondante au numéro
| passé en argument et qui renvoie à l'utilisateur la chaine de caractère à afficher grÃ¢ce à la
| fonction ``copy_to_user``.
| Nous libérons enfin le pointeur contenant cette chaine pour éviter les fuites mémoire.
| 


Fonctions
&&&&&&&&&

|
|

List
####

|
| Pour expliquer le fonctionnement de la fonction ``list`` nous devons d'abord expliquer la gestion
| de la variable *command_list*.
| *command_list* est une variable globale définie comme un tableau appartenant à  ``struct commande``.
| Le compteur global *cmd_cpt* permet de connaÃ®tre le nombre de commande en cours de traitement à un instant donné.
|
| A chaque demande d'analyse d'une commande par l'utilisateur, nous incrémentons le compteur et remplissons la
| première case libre du tableau avec les informations fournies par la structure ``commande``.
| 
.. code:: c

  strcpy (command_list[cmd_cpt].nom , args->nom);
  i = 0;
  
  while (i < 10) {
    if (args->param[i] != NULL) { 
      strcpy (command_list[cmd_cpt].param[i] , args->param[i]);
    }
    i ++;
  }

| Une fois la commande traitée nous décrémentons le compteur.
| 
| Pour afficher la commande s'exécutant à un moment donné, nous parcourons simplement le tableau jusqu'à la
| case numéro *cpt_cmd-1* qui correspond à la position de la commande recherchée, en affichant le numéro
| **ID** de la tache et le **nom** de cette tÃ¢che.

.. image:: list.png
   :scale: 50 %
   :alt: screen list
   :align: center


|
|
|
|

Fg 
###

| Avec cette fonction, nous récupérons la tÃ¢che mise en attente dans la waitqueue pour 
| qu'elle finisse de s'exécuter afin d'obtenir sa valeur de retour.
| Voir gestion des workqueue pour comprendre.
|

.. image:: fg.png
   :scale: 50 %
   :alt: screen fg
   :align: center


|


Wait
####

| 
| La fonction wait sert à se mettre en attente active sur une liste de pid. Quand l'un des processus
| correspondant au pid est fini, la fonction indique à l'utilisateur que l'un des processus c'est terminé.

.. image:: wait.png
   :scale: 50 %
   :alt: screen wait
   :align: center


|

Kill
####

|

| Cette fonction sert à pouvoir envoyer un signal à un processus désigné par son **pid**.
| Nous récupérons le *numéro du signal* à envoyer et le *pid* dans la structure ``commande`` copiée dans
| l'**ioctl**


| Avant de pouvoir envoyer un signal à un *processus* nous devons d'abord commencer par vérifier si
| le processus corespondant existe. Nous le faisons grÃ¢ce à la fonction ``find_vpid()`` qui  retourne un
| pointeur sur la structure **pid** correspondant au processus qui possède le pid passé en argument.

.. code:: c

  pid = find_vpid( num_pid );	  

| Si le pointeur est ``NULL``, nous retournons que le pid n'existe pas.
| 
| Si il existe nous envoyons le signal au processus demandé grÃ¢ce à la fonction ``kill_pid`` et nous
| testons sa valeur de retour. Si la fonction s'exécute correctement, un acquittement est retourné, sinon
| la fonstion indique que le processus n'a pas été interrompu.

.. code:: c

  if (kill_pid (pid, num_signal, 1) == 0)

.. image:: kill.png
   :scale: 50 %
   :alt: screen fg
   :align: center

  
|

Meminfo
#######

|
| Cette fonction a pour but d'afficher l'état de la mémoire de la machine à un instant donné. Pour
| cela les fonctions ``si_meminfo`` et ``si_swapinfo`` sont utilisées. Elles remplissent la
| ``structure sysinfo`` avec les informations trouvées.

.. code:: c

  si_meminfo (&i);
  si_swapinfo(&i);

Ce qui permet d'accéder aux différents champs de la
structure ``sysinfo`` pour en afficher le contenu

.. image:: meminfo.png
   :scale: 50 %
   :alt: screen meminfo
   :align: center


|


Modinfo
#######

|
| Cette fonction permet d'afficher les informations d'un module chargé dans le noyau.
| Nous commençons par récupérer le pointeur vers la ``structure module`` avec la fonction ``fin module``.

.. code:: c

  mod = find_module (command_list[val-1].param[0]);	 

| Nous testons ensuite si le module existe bien, sinonune chaine de caractère est retournée indiquant  à
| l'utilisateur que le module demandé n'existe pas.
|
| Si il existe  la ``structure module`` est parcourue, pour en extraire les informations voulues telles
| que la version, le nom, l'adresse de base, le nombre de paramètres et les différents paramètres.

.. image:: modinfo.png
   :scale: 50 %
   :alt: screen modinfo
   :align: center


|


Gestion synchrone/asynchrone
&&&&&&&&&&&&&&&&&&&&&&&&&&&&

|
| Nous allons maintenant vous expliquer la gestion des fonctions synchrone et asynchrones de
| notre projet. Comme expliqué précedement nous créons notre workqueue lors de
| l'initialisation de notre module.
|
| Mais nous avons aussi besoin d'une structure contenant toutes les information que nous voulons
| passer aux différentes fonctions.

.. code:: C

  struct work_user {
    struct work_struct wk_ws;
    char ** param;
    char * retour;
    int async;
  };
  	  
| Nous avons crée une structure générique qui servira pour toutes les fonctions.
|
| Nous avons aussi deux variables globale qui servent de condition de reveil

.. code:: C

  static int flags[10];	 
  static int flag = 0;

| Du coté **ioctl** nous nous traitons les fonctions synchrones ou asynchrones de la même façon, nous
| precisons juste le type dans la strucutre.
| 
| Voici la suite d'instructions que nous utilisons pour chaque cas :

.. code:: C

 case FCT_IO :

   cmd_cpt ++;
   flag = 0;
   INIT_WORK(&(wk->wk_ws), io_fct);
   schedule_work(&(wk->wk_ws));
   wait_event(cond_wait_queue, flag != 0);
   flag = 0;
   cmd_cpt --;

 break;

|
|
|
|
|
|
|
|
|
|
|
|
| Du coté fonction nous utilisons aussi la même suite de d'instructions.

.. code:: C

  if ( wu->async == 1) {
    cpy = cmd_cpt - 1;
    flags[cpy] = 0;
    flag = 1;
    cmd_cpt++;
    wake_up(&cond_wait_queue);    
    wait_event_interruptible (cond_wait_queue, flags[cpy] != 0);
    flags[cpy] = 0;
  }
	  
| Voici ce qu'il se passe si l'on est asynchrone, nous reston bloqué ici jusqu'a ce qu'il y ai un
| appel à fg qui passe notre condition à ``vrai``.
| FG se contente de mettre la valeur contenue dans la bonne case du table flags[] à 1 et de faire
| un ``wakeup`` pour que la fonction reveillée puisse s'executer.



------------------------------------------
   
VI) Conclusion
==============    

| Les 3 fichiers de notre projet ne remontent pas d'erreurs lorqu'ils passent le **checkpath**. Notre
| module au travers des différentes parties définies dans ce rapport, réalise bien toutes les fonctions
| attendues de façon synchrone et  asynchrone.
| Nous avons limité volontairement le nombre de paramètres  à 10 afin de nous focaliser davantage 
| sur les fonctions  à écrire. 
|

.. .. image:: trame.png
   :scale: 250 %
   :alt: trame protocale DCC
   :align: center


.. code:: VHDL

