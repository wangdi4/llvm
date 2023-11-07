=====
Xmain
=====

.. INTEL_FEATURE_CSA

**Intel Top Secret** - **may be removed from this document, when CSA becomes public**

.. end INTEL_FEATURE_CSA

.. contents::
   :local:

.. toctree::
   :hidden:

   IRSuppression
   IRVersionCompatibility
   OptReport
   TraceBack
   InlineReportViaMetadata
   LoopAnalysis/index
   LoopTransforms/index
   VPO/index
   Analysis/index
   CodeGen/index
   Clang/index
   GapAnalysis/index
   Tools/index
   MLPGO/index
   NoAlias
   Subscript

.. INTEL_FEATURE_MARKERCOUNT

.. toctree::
   :hidden:

   MarkerCount/index

.. end INTEL_FEATURE_MARKERCOUNT

.. INTEL_FEATURE_SW_DTRANS

.. toctree::
   :hidden:

   DTrans/index

.. end INTEL_FEATURE_SW_DTRANS

.. INTEL_FEATURE_CSA

.. toctree::
   :hidden:

   CSA/index

.. end INTEL_FEATURE_CSA

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

The LLVM development processes are governed by the `CE DevOps
<mailto:CE.DevOps@intel.com>`_. Please direct process related questions or
process change requests to this team.

Homepage for Xmain Processes documentation:
https://wiki.ith.intel.com/display/ITSCompilersDevOps/Xmain

:doc:`IR Suppression in Product Executables <IRSuppression>`
   Describes the rationale behind suppressing IR output in released products
   and the implementation used to do so.

:doc:`IR Version Compatibility <IRVersionCompatibility>`
   Describes the design of Intel-specific versioning in bitcode files and
   how backward compatibility will be handled if the bitcode changes in an
   incompatible way.

:doc:`Optimization Reports <OptReport>`

:doc:`TraceBack <TraceBack>`

:doc:`Interprocedural Optimization Reports <InlineReportViaMetadata>`
   Describes the usage and implementation details of optimization reports
   in xmain.

:doc:`Explicit Memory Aliasing Representation in LLVM IR <NoAlias>`

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

.. INTEL_FEATURE_SW_DTRANS

:doc:`DTrans <DTrans/index>`
   Describes data layout transformations and analysis.

.. end INTEL_FEATURE_SW_DTRANS

:doc:`GapAnalysis <GapAnalysis/index>`
   Documents xmain performance and functional gaps for replacing ICC.

:doc:`Tools <Tools/index>`
   Container for Tools documents.

.. INTEL_FEATURE_CSA

:doc:`CSA <CSA/index>`
   Describes the CSA backend and related topics.

.. end INTEL_FEATURE_CSA
