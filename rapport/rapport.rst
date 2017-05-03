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
| sur sur une machine virtuelle. 
| Lors de ce Projet nous nous sommes servis d'une VM *arch linux* avec comme version du noyau 4.2.3.
| Nous avons decouvert l'utilisation d'*ioctl* et l'intégration d'un module dans un noyau ainsi 
| que l'architecture du noyau elle même.
|

Ce projet se décompose en 2 étapes :

 - L'application utilisateur.
 - Le module noyau.

|

Ce projet a été developpé sous git :
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

Pour faciliter la compilation, le transfert et l'initialisation des différents modules et applications, nous avons crée 2 petits scripts Bash :

  - upload.sh
  - init.sh

|
|

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
   
   sync
   
   ./qemu-run-externKernel.sh
   
|
|


upload.sh sert si l'on a la partition root montée dans le dossier vm_root du répertoire courant :

 - demonter /monter la partition,
 - compiler le module,
 - copier differents fichier sur la partition,
 - syncroniser pour Ãªtre sur que tout soit bien écrit
 - lancer la machine virtuelle.


	  
|
|
|
|
|
|
|
|


.. code:: sh

  #!/bin/bash
  
  cd projet
  dmesg -C
  
  insmod our_mod.ko
  mknod /dev/hello c 245 0
  
  dmesg
  
  make -f Makefile_app
  gcc toto.c
  
  ./a.out &
  
  ps
  
  ./Projet.x /dev/hello
  
|
|

init.sh sert une fois la machine virtuelle démarrée à :

 - nettoyer le dmesg,
 - inserer le module,
 - creer le neud,
 - lancer une petite application en arrière plan 
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
|
|
|

---------------------------

|
|
|
|
|
|


II) architecture du projet
==========================

|

Notre projet se décompose en 3 parties :

 - le fichier *our_mod.h* contenant les differentes structures que nous
   allons utiliser tout au long du projet.
 - le fichier *main.c* qui correspond à l'application utilisateur qui
   recupère les demande de l'utilisateur et effectue les appels systeme.
 - le fichier *our_mod.c* qui est notre module. Il s'occupe de
   recuperer les appels systeme **ioctl** et d'effectuer les actions
   correspondantes à se que l'utilisateur à demandé.

|



III) structures
==============

|
| Pour les appels à **ioctl** nous avons du créer une structure nous permettant de traiter les différentes
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
 - **param**, un tableau de *char**, contenant les differents
   arguments que l'utilisateur peut passer avec la commande.
 - **asynchrone**, qui indique si la commande a été tapée avec un "&"
   à la fin, ce qui permettera de la faire s'executer en arrière plan.
 - **retour**, qui est un pointeur contenant la chaine de caractère a
   afficher en retour de l'appel systeme.

|
|
|
| Dans le fichier nous avons aussi les ``#define`` des différents
numero de fonction d'appel à **ioctl**.

.. code:: c
  
  #define LIST_IO     10
  #define FG_IOR      11
  #define KILL_IOR    12
  #define WAIT_IOR    13
  #define MEMINFO_IO  14
  #define MODINFO_IOR 15
  



IV) Appli utilisateur
=====================

|

| Notre application utilisateur prends en argument le nom du noeud sur lequel est rataché notre module.
| Elle se lance avec :

.. code:: bash

	  ./Projet.x /dev/xxx

|
| Et voici ce que l'on obtiens en la lançant.
|

.. image:: launch.png
   :scale: 50 %
   :alt: lancement application utilisateur
   :align: center

|


L'utilisateur peut entrer un certain nombre de commande qui seront traitées par notre appel systeme :

 - **list**, qui affiche les processus en cours d'execution dans le module.
 - **fg <id>**, qui remet en premier plan une application qui s'executait de
   façon asynchrone.
 - **wait <pid> [<pid>...]**, qui permet d'attendre la fin d'un processus donné
   en argument.
 - **kill <signal> <pid>**, qui permet d'envoyer un signal au processus
   pointé par le pid.
 - **meminfo**, qui affiche l'état actuel de la mémoire.
 - **modinfo <name>**, affiche les différentes informations liées au
   module passé en argument.

|
| Si une autre commande est demandé par l'utilisateur ou si il y a une faute de frappe, on reçoit une
| erreur et l'utilisateur est invité à lancer une nouvelle commande.
|

------------------------------------

| On commence par initialiser notre structure commande qui nous servira tout au long de la durée
| de l'application
| Puis on teste si le noeud existe, si il n'existe pas l'application s'arrête.
| On ouvre ensuite l'entrée standard pour pouvoir lire dessus les commandes de l'utilisateur.
| 
| On entre ensuite dans la boucle ``while (1)`` du programme qui est la boucle principale. C'est ici que 
| l'on lit les différentes commandes et arguments, et ou on fait l'appel à l'ioctl 
|
.. code:: c

   ioctl (module_fd, req, &commande);	  

|
| *module_fd* corresponds au descripteur de fichier du noeud ``/dev/xxx``.
| *req* corresponds au numero de la fonction à appeler.
| *&commande* correspond à l'adresse de la structure commande contenant tout les arguments à
| passer à la fonction demandée.

Il y a ensuite l'affichage de la chaine de caractères de retour de l'**ioctl**.


V) Module
=========

|
|

Le fichier de notre module est decoupé en 4 morceaux :
 - **initialisation** du mode
 - **destruction** du module
 - la fonction **ioctl**
 - les differentes **fonctions** appelées par l'appel systeme.




Initialisation
&&&&&&&&&&&&&&

|
| Dans l'initialisation nous commençons par trouver un numero majeur à notre module pour qu'il
| ai une place dans le ``/proc/devices`` qui nous permettera de creer un noeud plus tard avec 
| un ``mknod``

.. code :: c

  major = register_chrdev (0, "our_mod", &fops_mod);

|
| fops_mod contient la liste des appels systeme que notre module gère et les fonctions associées
| a chaque appel.
| Ici nous n'avons que **ioctl** qui est représenté.


| Pour pouvoir saugevarder qu'elle fonction s'execute à quel moment, nous avons besoin d'un
| tableau de ``structure commande``

| Dans L'initialisation nous continons par allouer de la mémoire kernel avec des kmalloc 
| pour chaques composant du tableau, nous avons limité le nombre de commandes en simultanées
| à 10.

|

Destruction
&&&&&&&&&&&

|
| Pour la destruction nous devons retirer notre module des numeros  majeurs de devices afin
| de ne pas saturer la liste.

.. code :: c

  unregister_chrdev (major, "our_mod");

|

| Une fois notre module retiré des numéros majeurs, ils faut libererla mémoire pour éviter
| les fuites mémoire.
| Nous faisons donc des ``kfree`` pour liberer notre structure.

|
|
|
|

Ioctl
&&&&&


Notre fonction ``device_ioctl`` est appelée pour chaque appel à ioctl.

| Nous commençons par copier la structure ``commande`` passée en  argument par l'utilisateur,
| donc du côté utilisateur, vers les adresses adressables coté kernel grâce à la fonction
| ``copy_from_user``. Nous ramenons du coté kernel chaques parties de la structure.
| 
| 
| Nous entrons ensuite dans un switch qui appel la fonction correspondante au numero
| passé en argument. Et renvoi à l'utilisateur la chaine de caractère à afficher grace à la
| fonction ``copy_to_user``.
| Nous liberons enfin le pointeur contenant cette chaine pour eviter les fuites mémoires.
| 
|

Fonctions
&&&&&&&&&
|

List
####

|

|

Fg 
###

|

Wait
####

|

Kill
####

|

Modinfo
#######

|

Meminfo
#######

|
|

Gestion synchrone/asynchrone
&&&&&&&&&&&&&&&&&&&&&&&&&&&&

|

explication des workqueue/waitqueue

|

------------------------------------------
   
VI) Conclusion
==============    

Les 3 fichiers de notre projet passent bien le **checkpath**.

Notre module réalise bien toutes les fonctions attendues de façon
synchrone ou asynchrone.
Toutes les fonctions fonctionnent bien comme il faut.

.. image:: trame.png
   :scale: 250 %
   :alt: trame protocale DCC
   :align: center


.. code:: VHDL

rajouter que le nombre de param est limité à 10.	  
