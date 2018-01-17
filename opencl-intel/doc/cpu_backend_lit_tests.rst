==================================
OpenCL CPU Backend LIT tests guide
==================================

Overview
========

The goal of this document is to describe how OpenCL CPU Backend is tested using
LIT infrastructure. This guide aims to answer the following questions:

* How can I launch LIT tests for OpenCL CPU Backend?
* How can I re-configure/tune some LIT tests for OpenCL CPU Backend?

How to run OpenCL CPU Backend LIT tests
=======================================

OCL CPU BE LIT tests available as cmake targets. There are two basic options to
launch LIT tests:

* One target to launch them all!
* A bunch of auto-generated targets to run LIT tests from each subdirectory
  separately.

So, to launch OCL CPU BE LIT tests you need to execute something like
``ics libbuild <target-to-launch-lit-tests> -j8``.

Available LIT tests targets
---------------------------

Targets for LIT tests are generated based on the directories structure: you can
launch LIT tests from each subdirectory separately __(and it is really cool!)__.

To launch **all** OCL CPU BE LIT tests you need to specify target
``check-ocl-backend``.

To launch LIT tests placed in certain directory you need to generate target
using path to selected directory using the following rules:

For example, you want to launch tests from
``opencl/backend/tests/barrier/DataPerBarrier`` subdirectory.

1. Take relative path to selected subdirectory based from
   ``opencl/backend/tests``. For our example it is ``barrier/DataPerBarrier``

2. Lowercase path: ``barrier/dataperbarrier``

3. Replace directory seperators with dashes: ``barrier-dataperbarrier``

4. Prefix with ``check-ocl-backend-``:
   ``check-ocl-backend-barrier-dataperbarrier``

5. Launch produced target. Profit!

Test discovery
--------------

LIT searches for files with certain extensions and treat ones as tests. List of
this extensions is specified in ``lit.local.cfg`` files in subdirectories of
``opencl/backend/tests``.

For example in ``opencl/backend/tests/barrier`` directory all files with ``.ll``
extension are treated as tests.

In case when a new test folder is added during development cmake won't
re-run automatically and new test target will not be created.
A solution is to re-run cmake by either performing a clean build or
clearing cmake cache.

LIT tests configuration and internal details
============================================

Main config files are ``opencl/backend/tests/lit.site.cfg.in`` and
``opencl/backend/tests/lit.cfg``

``lit.site.cfg.in`` is used to generate ``lit.site.cfg`` for further usage. In
turn, the last one is indended to catch some variables from cmake scripts, user
defined params and pass it into ``lit.cfg``.

``lit.cfg`` does real work: it configures substitutions, available features,
test discovery process, etc.

Some subdirectories contains ``lit.local.cfg`` - it is a local config which
can be used to specify some directory-specific settings. It is actually used
to specify file extensions which will be used to search files that should be
treated as tests.

How it works
------------

First step: on build time cmake configures ``lit.site.cfg.in``.  It is done by
``configure_lit_site_cfg`` function from ``AddLLVM.cmake`` (which uses
``configure_file`` cmake command).

Configuration of file means copying it to another location and modification of
its contents.

Modification means replacing all ``@VAR`` strings with values of corresponding
cmake variables.

You can read more about configuration of files in `cmake documentation`_.

.. _`cmake documentation`: https://cmake.org/cmake/help/v3.4/command/configure_file.html

Resulting ``lit.site.cfg`` is placed somewhere in build directory.

Second step depends on target you launched.

check-ocl-backend target
^^^^^^^^^^^^^^^^^^^^^^^^

``check-ocl-backend`` target launches LIT and pass to it path to
generated ``lit.site.cfg``.

LIT executes ``lit.site.cfg`` which defines required variables, loads
``lit.cfg`` and executes it.

This scenario is simple and straightforward.

check-ocl-backend-path-to-subdir target
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This scenario is a little bit tricky.

Any other target for a certain subdirectory launches LIT and pass to it path to
corresponding subdirectory.

LIT starts to search upwards from the input path until it finds ``lit.site.cfg``
or ``lit.cfg`` and it finds top-level ``lit.cfg`` in ``backend/tests``
directory.

LIT starts executing ``lit.cfg``. The problem is that requried variables that
should come from cmake scripts via ``lit.site.cfg`` is not defined.

You can see that LIT was launched with additional command line argument:
``--param be_lit_site_config=path/to/lit.site.cfg`` and ``lit.cfg`` contains
special ``if`` statement for this situation.

In such situation ``lit.cfg`` uses ``be_lit_site_config`` command line argument
to find and load ``lit.site.cfg``.

Summarizing, it works in the following manner:

* LIT searches upward from the input path until it finds ``lit.site.cfg`` or
  ``lit.cfg``.

* LIT finds ``lit.cfg`` from ``backend/tests`` and executes it.

* ``lit.cfg`` detects that ``lit.site.cfg`` was not loaded and loaded it using
  additional command line argument ``be_lit_site_config``.

* ``lit.site.cfg`` defines required variables and loads ``lit.cfg`` again.

Yes, it is a really tricky, but such workflow is used for LLVM and Clang
testing.

Useful links
=============

* `lit - LLVM Integrated Tester <https://llvm.org/docs/CommandGuide/lit.html>`_
