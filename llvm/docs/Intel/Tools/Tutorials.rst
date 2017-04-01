=========
Tutorials
=========

.. contents::
   :local:

Tutorial: Create an ICS workspace
---------------------------------

This tutorial shows how to create a new workspace from scratch.
To create a workspace you need to define the project that workspace will be created for.
For a full list of projects you run:

.. code-block:: console

 ics ls project

Here are some commonly used projects:

- `mainline` – The classic Intel Compiler (ICC)
- `xmain` – Intel Compiler based on LLVM (ICX)
- `llorg` – The open-source LLVM project (Clang)

If you are working on more than one project, you will need to create a workspace for each.

Each workspace has a unique name that is used as an identifier for the workspace and is also the name of
the top directory under which the workspace is located.

.. code-block:: console

 # Set the variable pointing to the top directory of where your workspaces are located
 export ICS_WSROOT=/users/$user/workspaces
 
 # Create the top workspaces directory if needed
 if [ ! -d "$ICS_WSROOT" ]; then
   mkdir $ICS_WSROOT  
 fi
 
 # Set the variable pointing to the ICS tools directory
 export ICS_START=/ics/itools/unx/bin
 
 # Source a script for setting up the shell to be used with ICS
 . $ICS_START/icssetup.sh

 # Create the new workspace named ‘ws_llorg’ of the ‘llorg’ project
 # and checkout the sources of the latest commits (‘head’)
 ics mk ws_llorg llorg head
