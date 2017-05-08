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

| Ce document pr�sente le rapport du projet **Bash** de l'UE *PNL*.
| Le projet a pour but de r�aliser un invite de commande pour le noyau linux et de le tester
| sur une machine virtuelle. 
| Lors de ce Projet nous nous sommes servis d'une VM *arch linux* en version noyau 4.2.3.
| Nous avons d�couvert l'utilisation d'*ioctl*, l'int�gration d'un module dans un noyau ainsi 
| que l'architecture du noyau et les workqueue.
|

Ce projet se d�compose en 2 parties :

 - L'application utilisateur.
 - Le module noyau.

|

Ce projet a �t� d�velopp� sous git :
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

Pour faciliter la compilation, le transfert et l'initialisation des diff�rents modules et applications, nous avons cr�� deux petits scripts Bash :

  - upload.sh
  - init.sh

|
|
|
|
| upload.sh sert si l'on a la partition root mont�e dans le dossier vm_root du r�pertoire courant

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

 - d�monter /monter la partition,
 - compiler le module,
 - copier diff�rents fichiers sur la partition,
 - synchroniser pour être sur que tout soit bien �crit
 - lancer la machine virtuelle.


	  
|
|
|
|
|
|
|
|
| init.sh sert une fois la machine virtuelle d�marr�e

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
 - ins�rer le module,
 - cr�er le neud,
 - lancer une petite application (toto.x) en arri�re plan 
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

Notre projet se d�compose en 3 parties :

 - le fichier *our_mod.h* contient les diff�rentes structures que nous
   allons utiliser tout au long du projet.
 - le fichier *main.c* correspond � l'application utilisateur qui
   r�cup�re les demandes de l'utilisateur et effectue les appels syst�me.
 - le fichier *our_mod.c* correspond � notre module. Il s'occupe de
   r�cup�rer les appels syst�me � **ioctl** et d'effectuer les
   actions correspondantes � ce que l'utilisateur a demand�.

|



III) structures
==============

|
| Pour les appels � **ioctl** nous avons d� cr�er une structure nous permettant de traiter les diff�rentes
| commandes.   
|

.. code:: c

  struct commande {
    char *nom;
    char **param;
    int asynchrone;
    char *retour;
  };

Notre structure est compos�e de plusieurs champs :

 - **nom**, qui sert � contenir le nom de la commande.
 - **param**, un tableau de *char**, contenant les diff�rents
   arguments que l'utilisateur peut passer avec la commande.
 - **asynchrone**, qui indique si la commande a �t� tap�e avec un "&"
   � la fin, ce qui permet de la faire s'ex�cuter en arri�re plan.
 - **retour**, qui est un pointeur contenant la chaine de caract�re �
   afficher en retour de l'appel syst�me.

|
|
|
| Dans le fichier nous avons aussi les ``#define`` des diff�rents num�ros de fonction d'appel � **ioctl**.

.. code:: c
  
  #define LIST_IO     10
  #define FG_IOR      11
  #define KILL_IOR    12
  #define WAIT_IOR    13
  #define MEMINFO_IO  14
  #define MODINFO_IOR 15
  

IV) Appli utilisateur
=====================


| Notre application utilisateur prend en argument le nom du noeud sur lequel est rattach� notre module.
| Elle se lance avec :

.. code:: bash

	  ./Projet.x /dev/xxx

| Et voici ce que nous obtenons en la lan�ant.
|

.. image:: launch.png
   :scale: 50 %
   :alt: lancement application utilisateur
   :align: center

|


L'utilisateur peut entrer les commandes suivantes qui seront trait�es par notre appel syst�me :

 - **list**, qui affiche les processus en cours d'ex�cution dans le module.
 - **fg <id>**, qui remet en premier plan une application en train de
   s'ex�cuter de fa�on asynchrone.
 - **wait <pid> [<pid>...]**, qui permet d'attendre la fin d'un processus donn�
   en argument.
 - **kill <signal> <pid>**, qui permet d'envoyer un signal au processus
   point� par le pid.
 - **meminfo**, qui affiche l'�tat actuel de la m�moire.
 - **modinfo <name>**, affiche les diff�rentes informations li�es au
   module pass� en argument.

|
| Si une autre commande, que celles pr�vues, est demand�e par l'utilisateur ou si il y a une faute de frappe,
| une erreur est retourn�e et l'utilisateur est invit� � lancer une nouvelle commande.
|

------------------------------------

| Dans ce programme, on commence par initialiser la structure commande qui servira tout au
| long de la dur�e de l'application
| Puis on teste si le noeud existe, si il n'existe pas l'application s'arrête sinon, on ouvre l'entr�e
| standard pour pouvoir lire les commandes de l'utilisateur.
| 
| On entre ensuite dans la boucle ``while (1)`` du programme qui est la boucle principale. C'est ici
| qu'on lit les diff�rentes commandes et arguments, et o� on fait l'appel � l'ioctl 
|
.. code:: c

   ioctl (module_fd, req, &commande);	  

|
| *module_fd* correspond au descripteur de fichier du noeud ``/dev/xxx``.
| *req* correspond au num�ro de la fonction � appeler.
| *&commande* correspond � l'adresse de la structure commande contenant tous les arguments �
| passer � la fonction appel�e.

Il y a ensuite l'affichage de la chaine de caract�res en retour de l'**ioctl**.


V) Module
=========

|
|

Notre module est d�coup� en 4 morceaux :
 - **initialisation** du module
 - **destruction** du module
 - la fonction **ioctl**
 - les diff�rentes **fonctions** appel�es par l'appel syst�me.




Initialisation
&&&&&&&&&&&&&&

|
| Dans l'initialisation nous commen�ons par trouver un num�ro majeur � notre module pour qu'il
| ait une place dans le ``/proc/devices`` ce qui nous permettera de cr�er un noeud plus tard avec
| un ``mknod``

.. code :: c

  major = register_chrdev (0, "our_mod", &fops_mod);

|
| ``fops_mod`` contient la liste des appels syst�me que notre module g�re et les fonctions associ�es
| � chaque appel.
| Pour le projet nous n'avons que **ioctl** qui est repr�sent�.


| Pour pouvoir sauvegarder la fonction qui 'ex�cute � un instant donn�, nous avons besoin d'un
| tableau de ``structure commande`` *(voir list)*

| Dans l'initialisation nous continuons par allouer de la m�moire kernel avec des kmalloc
| pour chaque composant du tableau, nous avons limit� le nombre de commandes
| en simultan�es � 10.
|
| Dans l'initialisation du module, nous avons aussi besoin de creer une workqueue.

.. code:: C
	  
  work_station = create_workqueue("worker");

| Nous finissons par verifier qu'elle est bien cr�e et l'initialisation du module est finie.
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
| Pour la destruction nous devons retirer notre module des num�ros  majeurs de devices afin
| de ne pas saturer la liste.

.. code :: c

  unregister_chrdev (major, "our_mod");

| Une fois notre module retir� des num�ros majeurs, il faut lib�rer la m�moire pour �viter
| les fuites m�moire.
| Nous faisons donc des ``kfree`` pour lib�rer les ressources allou�es � notre structure.

| Nous devons aussi detruire la worqueue cr�e dans le init.
| 
|
|

Ioctl
&&&&&


Notre fonction ``device_ioctl`` est appel�e pour chaque appel � **ioctl**.

| Nous commen�ons par copier la structure ``commande`` pass�e en  argument par l'utilisateur,
| vers les adresses adressables côt� kernel grâce � la fonction ``copy_from_user``. Nous
| ramenons du côt� kernel chaque partie de la structure.
|
| Avec les valeurs r�cup�r�es nous les copions dans notre structure ``work_user`` *(voir gestion workqueue)*
| afin de pouvoir les utiliser dans les diff�rentes fonctions suivante.
|
| Nous entrons ensuite dans un switch qui appelle la fonction correspondante au num�ro
| pass� en argument et qui renvoie � l'utilisateur la chaine de caract�re � afficher grâce � la
| fonction ``copy_to_user``.
| Nous lib�rons enfin le pointeur contenant cette chaine pour �viter les fuites m�moire.
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
| *command_list* est une variable globale d�finie comme un tableau appartenant �  ``struct commande``.
| Le compteur global *cmd_cpt* permet de connaître le nombre de commande en cours de traitement � un instant donn�.
|
| A chaque demande d'analyse d'une commande par l'utilisateur, nous incr�mentons le compteur et remplissons la
| premi�re case libre du tableau avec les informations fournies par la structure ``commande``.
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

| Une fois la commande trait�e nous d�cr�mentons le compteur.
| 
| Pour afficher la commande s'ex�cutant � un moment donn�, nous parcourons simplement le tableau jusqu'� la
| case num�ro *cpt_cmd-1* qui correspond � la position de la commande recherch�e, en affichant le num�ro
| **ID** de la tache et le **nom** de cette tâche.

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

| Avec cette fonction, nous r�cup�rons la tâche mise en attente dans la waitqueue pour 
| qu'elle finisse de s'ex�cuter afin d'obtenir sa valeur de retour.
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
| La fonction wait sert � se mettre en attente active sur une liste de pid. Quand l'un des processus
| correspondant au pid est fini, la fonction indique � l'utilisateur que l'un des processus c'est termin�.

.. image:: wait.png
   :scale: 50 %
   :alt: screen wait
   :align: center


|

Kill
####

|

| Cette fonction sert � pouvoir envoyer un signal � un processus d�sign� par son **pid**.
| Nous r�cup�rons le *num�ro du signal* � envoyer et le *pid* dans la structure ``commande`` copi�e dans
| l'**ioctl**


| Avant de pouvoir envoyer un signal � un *processus* nous devons d'abord commencer par v�rifier si
| le processus corespondant existe. Nous le faisons grâce � la fonction ``find_vpid()`` qui  retourne un
| pointeur sur la structure **pid** correspondant au processus qui poss�de le pid pass� en argument.

.. code:: c

  pid = find_vpid( num_pid );	  

| Si le pointeur est ``NULL``, nous retournons que le pid n'existe pas.
| 
| Si il existe nous envoyons le signal au processus demand� grâce � la fonction ``kill_pid`` et nous
| testons sa valeur de retour. Si la fonction s'ex�cute correctement, un acquittement est retourn�, sinon
| la fonstion indique que le processus n'a pas �t� interrompu.

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
| Cette fonction a pour but d'afficher l'�tat de la m�moire de la machine � un instant donn�. Pour
| cela les fonctions ``si_meminfo`` et ``si_swapinfo`` sont utilis�es. Elles remplissent la
| ``structure sysinfo`` avec les informations trouv�es.

.. code:: c

  si_meminfo (&i);
  si_swapinfo(&i);

Ce qui permet d'acc�der aux diff�rents champs de la
structure ``sysinfo`` pour en afficher le contenu

.. image:: meminfo.png
   :scale: 50 %
   :alt: screen meminfo
   :align: center


|


Modinfo
#######

|
| Cette fonction permet d'afficher les informations d'un module charg� dans le noyau.
| Nous commen�ons par r�cup�rer le pointeur vers la ``structure module`` avec la fonction ``fin module``.

.. code:: c

  mod = find_module (command_list[val-1].param[0]);	 

| Nous testons ensuite si le module existe bien, sinonune chaine de caract�re est retourn�e indiquant  �
| l'utilisateur que le module demand� n'existe pas.
|
| Si il existe  la ``structure module`` est parcourue, pour en extraire les informations voulues telles
| que la version, le nom, l'adresse de base, le nombre de param�tres et les diff�rents param�tres.

.. image:: modinfo.png
   :scale: 50 %
   :alt: screen modinfo
   :align: center


|


Gestion synchrone/asynchrone
&&&&&&&&&&&&&&&&&&&&&&&&&&&&

|
| Nous allons maintenant vous expliquer la gestion des fonctions synchrone et asynchrones de
| notre projet. Comme expliqu� pr�cedement nous cr�ons notre workqueue lors de
| l'initialisation de notre module.
|
| Mais nous avons aussi besoin d'une structure contenant toutes les information que nous voulons
| passer aux diff�rentes fonctions.

.. code:: C

  struct work_user {
    struct work_struct wk_ws;
    char ** param;
    char * retour;
    int async;
  };
  	  
| Nous avons cr�e une structure g�n�rique qui servira pour toutes les fonctions.
|
| Nous avons aussi deux variables globale qui servent de condition de reveil

.. code:: C

  static int flags[10];	 
  static int flag = 0;

| Du cot� **ioctl** nous nous traitons les fonctions synchrones ou asynchrones de la m�me fa�on, nous
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
| Du cot� fonction nous utilisons aussi la m�me suite de d'instructions.

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
	  
| Voici ce qu'il se passe si l'on est asynchrone, nous reston bloqu� ici jusqu'a ce qu'il y ai un
| appel � fg qui passe notre condition � ``vrai``.
| FG se contente de mettre la valeur contenue dans la bonne case du table flags[] � 1 et de faire
| un ``wakeup`` pour que la fonction reveill�e puisse s'executer.



------------------------------------------
   
VI) Conclusion
==============    

| Les 3 fichiers de notre projet ne remontent pas d'erreurs lorqu'ils passent le **checkpath**. Notre
| module au travers des diff�rentes parties d�finies dans ce rapport, r�alise bien toutes les fonctions
| attendues de fa�on synchrone et  asynchrone.
| Nous avons limit� volontairement le nombre de param�tres  � 10 afin de nous focaliser davantage 
| sur les fonctions  � �crire. 
|

.. .. image:: trame.png
   :scale: 250 %
   :alt: trame protocale DCC
   :align: center


.. code:: VHDL

