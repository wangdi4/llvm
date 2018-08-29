=====
Xmain
=====

**Intel Top Secret**

.. contents::
   :local:

.. toctree::
   :hidden:

   OpenSourceGuidelines
   OpenSourceProcesses
   XmainProcesses
   PulldownProcess
   JIRA
   XmainAlloyGerrit
   XmainGatekeeperDuties
   IRSuppression
   LoopOptReport
   LoopAnalysis/index
   LoopTransforms/index
   VPO/index
   Analysis/index
   CodeGen/index
   Clang/index
   DTrans/index
   GapAnalysis/index
   Tools/index
   CSA/index

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

Intel LLVM Development Process Documentation
============================================

The Intel LLVM development process documentation is currently under review. If
anything looks wrong to you, please contact `David Kreitzer
<mailto:david.l.kreitzer@intel.com>`_.

The LLVM development processes are governed by the `ICL Process Management Team
<mailto:ICLProcessTeam@intel.com>`_. Please direct process related questions or
process change requests to them.

:doc:`Open Source Guidelines <OpenSourceGuidelines>`
   Describes the decision making process for determining what technology will
   be contributed to open source and what technology will be Intel
   customizations in xmain.

:doc:`Open Source Development Processes <OpenSourceProcesses>`
   Describes the processes for open source (llvm.org) development.

:doc:`Xmain Development Processes <XmainProcesses>`
   Describes the processes for xmain development.

:doc:`Xmain llorg Pulldown Process <PulldownProcess>`
   Describes the processes for syncing xmain with open source LLVM.

:doc:`LLVM Specific JIRA Usage <JIRA>`
   Describes JIRA usage policies specific to LLVM development.

:doc:`IR Suppression in Product Executables <IRSuppression>`
   Describes the rationale behind suppressing IR output in released products
   and the implementation used to do so.

:doc:`Loop Optimization Reports <LoopOptReport>`
   Describes the usage and implementation details of optimization reports
   in xmain.

Major Components
================
:doc:`LoopAnalysis <LoopAnalysis/index>`
   Describes the High-Level-IR and the various analyses based on it.

:doc:`LoopTransforms <LoopTransforms/index>`
   Describes the loop transformations based on High-Level-IR analyses.

:doc:`VPO <VPO/index>`
   Describes the VPO vectorizer.

:doc:`Analysis <Analysis/index>`
   Describes the xmain projects under Analysis.

:doc:`CodeGen <CodeGen/index>`
   Container for CodeGen documents.

:doc:`Clang <Clang/index>`
   Container for Clang documents.

:doc:`DTrans <DTrans/index>`
   Describes data layout transformations and analysis.

:doc:`GapAnalysis <GapAnalysis/index>`
   Documents xmain performance and functional gaps for replacing ICC.

:doc:`Tools <Tools/index>`
   Container for Tools documents.

:doc:`CSA <CSA/index>`
   Describes the CSA backend and related topics.
