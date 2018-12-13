=====================================
Library versioning and driver version
=====================================

Overview
========

This document describes how OpenCL libraries versions and driver versions
behave from release to release and how it affects regular development process.


Introduction
============

The versioning for OpenCL libraries was implemented for Linux to avoid conflicts
between multiple OpenCL Runtimes installed in the system.

For Windows the described problem will be fixed by delay loading of libraries.

Implementation details
======================

Versioning is set as 'x.y' via SOVERSION, where 'x' equals LLVM_VERSION_MAJOR
and 'y' is an internal agreed digit (originally it is a number of the following
release). All the deliverable libraries must follow this rule. SOVERSION
mechanism creates a symbolic link to a library as well with the same name but
versioning postfix dropped.

Driver version reported follows the same rule, until we decide to make a public
release. In that case it must be changed on the release branch to be aligned
with an external (marketing) version of the release. For now it's agreed to be
set as 'year.#release'. One can see details in the table below.


Example
=======

                     | Library version | Driver version
-------------------------------------------------------
FPGA emu development |       8.0       |      8.0
-------------------------------------------------------
CPU development      |       8.1       |      8.1
-------------------------------------------------------
FPGA emu release     |       8.0       |      19.0
-------------------------------------------------------
CPU release          |       8.1       |      19.1
-------------------------------------------------------

As an example let's see evolution of intelocl library.

1. Regular development
----------------------
libintelocl.so -> libintelocl.so.8.0
libintelocl.so.8.0

Driver version is reported as 8.0.

2. FPGA emulator release
------------------------
libintelocl.so -> libintelocl.so.8.0
libintelocl.so.8.0

Driver version is reported as 19.0

3. Regular development
----------------------
libintelocl.so -> libintelocl.so.8.1
libintelocl.so.8.1

Driver version is reported as 8.1.

4. CPU compiler release
-----------------------
libintelocl.so -> libintelocl.so.8.1
libintelocl.so.8.1

Driver version is reported as 19.1.
