=====
Xmain
=====

.. contents::
   :local:

.. toctree::
   :hidden:

   LoopAnalysis/index
   LoopTransforms/index
   VPO/index

Introduction
============

Xmain is a compiler built on top of LLVM. This section documents the additions
and modifications that separate Xmain from LLVM.

How we propose to document Xmain
================================

Taking advantage of LLVM's in-tree documentation system
-------------------------------------------------------

LLVM already provides an excellent documentation system for itself which can be
built as part of LLVM. There is no need to reinvent any wheels.

Taking advantage of source-code review tools
--------------------------------------------

Since LLVM's documentation system is based on plain-text, the same peer-review
process developers apply for source code can be applied to the documentation.

Taking advantage of the version control system
----------------------------------------------

Placing Xmain's documentation in-tree facilitates applying to documentation the
same principles by which Xmain manages its source code:

* Documentation is written/updated in the relevant sub-project.

* Documentation is promoted as part of the standard promotion process

Major Components
================

:doc:`LoopAnalysis <LoopAnalysis/index>`
   Describes the High-Level-IR and the various analyses based on it.

:doc:`LoopTransforms <LoopTransforms/index>`
   Describes the loop transformations based on High-Level-IR analyses.

:doc:`VPO <VPO/index>`
   Describes the VPO vectorizer.

