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

.. code-block:: shell

 # Set the variable pointing to the top directory of where your workspaces are located
 export ICS_WSROOT=/users/$USERNAME/workspaces
 
 # Create the top workspaces directory if needed
 if [ ! -d "$ICS_WSROOT" ]; then
   mkdir $ICS_WSROOT  
 fi
 
 # Set the variable pointing to the ICS tools directory
 export ICS_START=/ics/itools/unx/bin
 
 # Source a script for setting up the shell to be used with ICS
 . $ICS_START/icssetup.sh

 # Create the new workspace named ‘ws_llorg’ of the ‘llorg’ project
 # and checkout the sources of the latest revision (‘head’)
 ics mk ws_llorg llorg head

Tutorial: Resume work on an exisiting workspace
-----------------------------------------------

This tutorial shows how to resume work on an existing workspace.
We will open an ICS shell, then update the source code to the latest revision
and finally build the project.
Assume that a workspace was previously created under `/users/$USERNAME/workspaces/xmain_0`

In order to build, you must specify the configuration of the workspace which consists of:

#. variant - derived from project and hosting OS. You can get a list of available variants by
   doing `ics list variants`.
#. build type - 'debug' and 'prod' are commonly used. `ics list buildtypes` will give the full list.

.. code-block:: shell

 # Set the variable pointing to the top directory of where your workspaces are located
 export ICS_WSROOT=/users/$USERNAME/workspaces
  
 # Set the variable pointing to the ICS tools directory
 export ICS_START=/ics/itools/unx/bin
 
 # Source a script for setting up the shell to be used with ICS
 . $ICS_START/icssetup.sh
 
 # Set the current workspace to the exisitng 'xmain_0' workspace.
 # ICS will automatically change the current directory to the top of the workspace.
 ics set ws xmain_0
 
 # Update the sources in the workspace to leatest revision ('head')
 ics update head
 
 # Before you build, choose a build configuration.
 # Let's choose variant='xmainefi2linux' and  build type='debug'
 ics set config -ws xmainefi2linux debug
 
 # Start the build. You can pass the '-j' flag for concurrency.
 ics build -j16

If all went well the build artifacts can be found under `$ICS_WSROOT/xmain_0/builds/xmainefi2linux_debug/`
