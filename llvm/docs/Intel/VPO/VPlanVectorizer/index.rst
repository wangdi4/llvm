================
VPlan Vectorizer
================

.. contents::
   :local:

.. toctree::
   :hidden:

   Directory
   Pipeline
   DivergenceAnalysis

Introduction
============

Welcome to the documentation hub for the Intel VPlan Vectorizer!  This site is intended to
provide an overall picture of the Vectorizer design, along with more detailed descriptions
of its subcomponents.

For an overall description of the VPlan Vectorizer pipeline design, see :doc:`Pipeline`.

For a "road atlas" of the VPlan Vectorizer files, see :doc:`Directory`.

Detailed Documentation
======================

Additional documentation for individual features is available below.

:doc:`DivergenceAnalysis`

Useful VPlan development flags
==============================

``VPlan`` has multiple useful flags enabling and disabling various features within
compiler. The list is not static and new flags can be added or removed by developers based
on their needs. Flags are added using the CommandLine library. Example definition of a flag
may look like this:

.. code-block:: c++

  static cl::opt<bool> EnableNestedBlobVec(
      "enable-nested-blob-vec", cl::init(true), cl::Hidden,
      cl::desc("Enable vectorization of loops with nested blobs"));

Complete documentation for CommandLine library can be found in `CommandLine library manual`_.

To use custom VPlan flags during compilation with Xmain, the flag should be prefixed with
``-mllvm`` or, when you use -lto in the compilation, ``-mllvm-lto``. For example:

.. code-block:: c++

  icx [additional options] -mllvm -enable-nested-blob-vec source.cpp

Placement of the ``-mllvm -flag`` pair is not important within the icx command line.
The ``-mllvm`` prefix is not required when running ``opt`` utility.

Complete list of existing flags can be obtained in following way:

.. code-block:: c++

  opt -help-hidden | grep vplan

Once the required flag is found in ``opt -help-hidden`` it can be grepped in the VPlan
sources without the leading hyphen.

.. _CommandLine library manual: https://llvm.org/docs/CommandLine.html
