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

*Ayrault Maxime* **3203694** - *Sivarajah Sivapiryan* **32xxxxx**

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

Pour faciliter la compilation, le transfert et l'initialisation des différents modules, nous avons crée 2 petits scripts Bash :

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
 - syncroniser pour être sur que tout soit bien écrit
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

---------------------------

|

II) Appli utilisateur
=====================

III) Module
===========

IV) Conclusion
==============    


.. image:: trame.png
   :scale: 250 %
   :alt: trame protocale DCC
   :align: center


.. code:: VHDL
