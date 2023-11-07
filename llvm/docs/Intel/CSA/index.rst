===
CSA
===

**Intel Top Secret**

.. contents::
   :local:

.. toctree::
   :hidden:

   CSALowerLoopIdioms
   DataflowConversion
   DataflowMIR
   MemoryOrdering
   PathfindingBuiltins
   MIRMatcher

Introduction
============

The CSA backend and OpenMP target plugin allow users to compile C, C++, and
Fortran code to run on CSA. Usage of CSA compiler features is documented
elsewhere; these pages cover the implementation of those features and how they
fit together to allow compilation for CSA.

Pass Overview
=============

The following is an overview of CSA-specific passes and where they fit into the
general compilation flow:

- Frontend
- :doc:`CSA Builtin Lowering <PathfindingBuiltins>`
  (:ref:`CSAFortranIntrinsics`, :ref:`CSALoopIntrinsicExpander`) - Lowers
  user-facing pathfinding builtins into common/internal forms.
- Generic IR optimizations + LoopSPMDization
- Late CSA IR passes (CSAIRReductionOpt, CSAInnerLoopPrep,
  :ref:`CSAIntrinsicCleaner`,
  :doc:`CSALowerLoopIdioms`) - These perform extra
  CSA-advantaged optimizations and prepare the code for instruction selection.
- :doc:`CSA Memory Operation Ordering (CSAMemopOrdering) <MemoryOrdering>` -
  Inserts ordering edges between memory operations as needed to enforce memory
  ordering constraints in the original program.
- CSAStreamingMemoryConversion - This makes streaming loads and stores out of
  affine loop expressions.
- Instruction Selection + CSAExpandInlineAsm
- :doc:`CSA Dataflow Conversion (CSACvtCFDFPass) <DataflowConversion>` -
  Converts the code from SSA form into dataflow by converting branches/phis
  into picks and switches.
- CSA Dataflow Optimizations:

  - CSAOptDFPass - Optimizes dataflow loops using CSA sequence instructions.
  - CSADataflowCanonicalization - Simplifies dataflow code into canonical forms.
  - CSAStreamingMemoryConversion - Converts strided memory accesses into CSA
    streaming memory operations.
  - CSAMultiSeq - Adds redundant sequence operations to avoid backpressure
    issues.
  - CSARedundantMovElim, CSADeadInstructionElim - Dataflow-aware DCE and simple
    value propagation.
  - CSAReassocReduc - If enabled, expands reduction operations to increase
    throughput via reassociation.

- Register Allocation + CSANormalizeDebugPass - Register allocation is still
  needed for SXU registers if dataflow calls are disabled, but is a no-op for
  lics. CSANormalizeDebugPass is a small pass that runs beforehand to make sure
  that there aren't disconnected lics hanging around as a result of debug
  values.
- :ref:`CSADataflowVerifier` - Verifies dataflow machine code
- CSAProcedureCalls - Converts functions from their initial SXU-based calling
  conventions to dataflow calling conventions.
- Assembly Printing

Passes before instruction selection operate on standard LLVM IR and many of the
CSA-specific ones exist mostly to manipulate intrinsics used by backend passes
later on. Memory ordering runs at the very end of the IR phase and produces
memory ordering intrinsics meant to be directly consumed by instruction
selection. Passes starting with dataflow conversion operate on Machine IR
interpreted as dataflow code. See :doc:`CSA Dataflow Machine Code Representation
<DataflowMIR>` for more information on this format and how to use it.

Usage Models
============

There are several different ways of compiling for CSA:

Manual Offload / Separate Compilation
  C and C++ code can be compiled directly for CSA with ``-target csa`` and run
  directly using the simulator API defined in csa_invoke.h. This is the simplest
  way of invoking the compiler and often the most useful for compiler work.

Compiler Offload / OpenMP Offload
  C and C++ code can be compiled with ``-fopenmp-targets=csa`` and regions
  marked with ``#pragma omp target`` will automatically be run through the
  compilation flow above at compile time and then automatically invoked using
  the simulator at runtime. This is the easiest way to run existing code on CSA
  and will be the primary usage model in the long term.

Fortran
  Until the Intel Fortran frontend is mature enough for external use, we are
  using a temporary workaround where code is compiled into IR using gfortran
  with the DragonEgg plugin and then run through the rest of the standard
  compilation flow. This is not as stable or well-tested as the other usage
  models, but customers have had moderate success using it with a couple of
  Fortran workloads.

[ TODO: Compiler Offload and DragonEgg may warrant separate pages describing all
of their details - add links in this section when those are available. ]

Topics
======

The following is a full list of CSA compilation topics documented here:

.. NOTE: These should be in alphabetical order.

:doc:`CSALowerLoopIdioms`
  A pass that lowers memcpy/memmove/memset intrinsics to CSA-friendly loops.
:doc:`Dataflow Conversion <DataflowConversion>`
  The dataflow conversion pass of CSA.
:doc:`Dataflow Machine Code Representation <DataflowMIR>`
  Structure and rules of CSA dataflow machine code.
:doc:`Memory Operation Ordering <MemoryOrdering>`
  The CSAMemopOrdering pass and other information related to memory ordering.
:doc:`Pathfinding Builtins <PathfindingBuiltins>`
  The set of intrinsics added to annotate code in the pathfinding compiler and
  related passes.
