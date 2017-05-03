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
| sur sur une machine virtuelle. 
| Lors de ce Projet nous nous sommes servis d'une VM *arch linux* avec comme version du noyau 4.2.3.
| Nous avons decouvert l'utilisation d'*ioctl* et l'int�gration d'un module dans un noyau ainsi 
| que l'architecture du noyau elle m�me.
|

Ce projet se d�compose en 2 �tapes :

 - L'application utilisateur.
 - Le module noyau.

|

Ce projet a �t� developp� sous git :
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

Pour faciliter la compilation, le transfert et l'initialisation des diff�rents modules et applications, nous avons cr�e 2 petits scripts Bash :

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


upload.sh sert si l'on a la partition root mont�e dans le dossier vm_root du r�pertoire courant :

 - demonter /monter la partition,
 - compiler le module,
 - copier differents fichier sur la partition,
 - syncroniser pour être sur que tout soit bien �crit
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

init.sh sert une fois la machine virtuelle d�marr�e � :

 - nettoyer le dmesg,
 - inserer le module,
 - creer le neud,
 - lancer une petite application en arri�re plan 
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

Notre projet se d�compose en 3 parties :

 - le fichier *our_mod.h* contenant les differentes structures que nous
   allons utiliser tout au long du projet.
 - le fichier *main.c* qui correspond � l'application utilisateur qui
   recup�re les demande de l'utilisateur et effectue les appels systeme.
 - le fichier *our_mod.c* qui est notre module. Il s'occupe de
   recuperer les appels systeme **ioctl** et d'effectuer les actions
   correspondantes � se que l'utilisateur � demand�.

|



III) structures
==============

|
| Pour les appels � **ioctl** nous avons du cr�er une structure nous permettant de traiter les diff�rentes
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
 - **param**, un tableau de *char**, contenant les differents
   arguments que l'utilisateur peut passer avec la commande.
 - **asynchrone**, qui indique si la commande a �t� tap�e avec un "&"
   � la fin, ce qui permettera de la faire s'executer en arri�re plan.
 - **retour**, qui est un pointeur contenant la chaine de caract�re a
   afficher en retour de l'appel systeme.

|
|
|
| Dans le fichier nous avons aussi les ``#define`` des diff�rents
numero de fonction d'appel � **ioctl**.

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

| Notre application utilisateur prends en argument le nom du noeud sur lequel est ratach� notre module.
| Elle se lance avec :

.. code:: bash

	  ./Projet.x /dev/xxx

|
| Et voici ce que l'on obtiens en la lan�ant.
|

.. image:: launch.png
   :scale: 50 %
   :alt: lancement application utilisateur
   :align: center

|


L'utilisateur peut entrer un certain nombre de commande qui seront trait�es par notre appel systeme :

 - **list**, qui affiche les processus en cours d'execution dans le module.
 - **fg <id>**, qui remet en premier plan une application qui s'executait de
   fa�on asynchrone.
 - **wait <pid> [<pid>...]**, qui permet d'attendre la fin d'un processus donn�
   en argument.
 - **kill <signal> <pid>**, qui permet d'envoyer un signal au processus
   point� par le pid.
 - **meminfo**, qui affiche l'�tat actuel de la m�moire.
 - **modinfo <name>**, affiche les diff�rentes informations li�es au
   module pass� en argument.

|
| Si une autre commande est demand� par l'utilisateur ou si il y a une faute de frappe, on re�oit une
| erreur et l'utilisateur est invit� � lancer une nouvelle commande.
|

------------------------------------

| On commence par initialiser notre structure commande qui nous servira tout au long de la dur�e
| de l'application
| Puis on teste si le noeud existe, si il n'existe pas l'application s'arr�te.
| On ouvre ensuite l'entr�e standard pour pouvoir lire dessus les commandes de l'utilisateur.
| 
| On entre ensuite dans la boucle ``while (1)`` du programme qui est la boucle principale. C'est ici que 
| l'on lit les diff�rentes commandes et arguments, et ou on fait l'appel � l'ioctl 
|
.. code:: c

   ioctl (module_fd, req, &commande);	  

|
| *module_fd* corresponds au descripteur de fichier du noeud ``/dev/xxx``.
| *req* corresponds au numero de la fonction � appeler.
| *&commande* correspond � l'adresse de la structure commande contenant tout les arguments �
| passer � la fonction demand�e.

Il y a ensuite l'affichage de la chaine de caract�res de retour de l'**ioctl**.


V) Module
=========

|
|

Le fichier de notre module est decoup� en 4 morceaux :
 - **initialisation** du mode
 - **destruction** du module
 - la fonction **ioctl**
 - les differentes **fonctions** appel�es par l'appel systeme.




Initialisation
&&&&&&&&&&&&&&

|
| Dans l'initialisation nous commen�ons par trouver un numero majeur � notre module pour qu'il
| ai une place dans le ``/proc/devices`` qui nous permettera de creer un noeud plus tard avec 
| un ``mknod``

.. code :: c

  major = register_chrdev (0, "our_mod", &fops_mod);

|
| fops_mod contient la liste des appels systeme que notre module g�re et les fonctions associ�es
| a chaque appel.
| Ici nous n'avons que **ioctl** qui est repr�sent�.


| Pour pouvoir saugevarder qu'elle fonction s'execute � quel moment, nous avons besoin d'un
| tableau de ``structure commande``

| Dans L'initialisation nous continons par allouer de la m�moire kernel avec des kmalloc 
| pour chaques composant du tableau, nous avons limit� le nombre de commandes en simultan�es
| � 10.

|

Destruction
&&&&&&&&&&&

|
| Pour la destruction nous devons retirer notre module des numeros  majeurs de devices afin
| de ne pas saturer la liste.

.. code :: c

  unregister_chrdev (major, "our_mod");

|

| Une fois notre module retir� des num�ros majeurs, ils faut libererla m�moire pour �viter
| les fuites m�moire.
| Nous faisons donc des ``kfree`` pour liberer notre structure.

|
|
|
|

Ioctl
&&&&&


Notre fonction ``device_ioctl`` est appel�e pour chaque appel � ioctl.

| Nous commen�ons par copier la structure ``commande`` pass�e en  argument par l'utilisateur,
| donc du c�t� utilisateur, vers les adresses adressables cot� kernel gr�ce � la fonction
| ``copy_from_user``. Nous ramenons du cot� kernel chaques parties de la structure.
| 
| 
| Nous entrons ensuite dans un switch qui appel la fonction correspondante au numero
| pass� en argument. Et renvoi � l'utilisateur la chaine de caract�re � afficher grace � la
| fonction ``copy_to_user``.
| Nous liberons enfin le pointeur contenant cette chaine pour eviter les fuites m�moires.
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

Notre module r�alise bien toutes les fonctions attendues de fa�on
synchrone ou asynchrone.
Toutes les fonctions fonctionnent bien comme il faut.

.. image:: trame.png
   :scale: 250 %
   :alt: trame protocale DCC
   :align: center


.. code:: VHDL

rajouter que le nombre de param est limit� � 10.	  
