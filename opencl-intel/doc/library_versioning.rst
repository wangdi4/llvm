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

Versioning is set as 'yy.major.mm.minor' via SOVERSION, where 'major' equals
'LLVM_VERSION_MAJOR - 1' (latest LLVM release version), 'yy', 'mm' - monthly
release date and 'minor' is internally agreed digit (for now it's '0'). All the
deliverable libraries must follow this rule. SOVERSION mechanism creates a
symbolic link to a library as well with the same name but versioning postfix
dropped.

Driver version reported follows the same rule.


Example
=======

Lets look as an example to upcoming April 2019 release.

                     |  Library version    |  Driver version
--------------------------------------------------------------
Apr 17 release       |  2019.8.4.0         |  2019.8.4.0
--------------------------------------------------------------

So in library directory we see:

libintelocl.so -> libintelocl.so.2019.8.4.0
libintelocl.so.2019.8.4.0
