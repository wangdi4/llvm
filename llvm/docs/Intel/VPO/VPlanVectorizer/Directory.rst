==========================
VPlan Vectorizer Directory
==========================

.. contents::
   :local:

Introduction
============

The VPlan vectorizer is a large component.  This page provides a brief
description of the vectorizer's subcomponents and where to find the
associated code.

Almost all vectorizer code is found in one directory and its single
subdirectory::

   llvm/lib/Transforms/Vectorize/Intel_VPlan              [.../Intel_VPlan]
   llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR     [.../VPlanHIR]

For historical reasons, a few files are co-located with the community vectorizer::

   llvm/include/llvm/Transforms/Vectorize                 [.../Vectorize]

These should perhaps be moved to the ``Intel_VPlan`` directory at some point.

Overall Framework
=================

Loop Vectorization Driver
-------------------------

The entry point to the vectorizer is the VPlan Driver.  The Driver is invoked
from two different passes, ``VPlanDriverPass`` and ``VPlanDriverHIRPass``.
These passes take input in the form of LLVM IR and HIR, respectively.  The
HIR pass vectorizes both explicit SIMD loops (for example, loops marked with
``#pragma omp simd`` or with ``#pragma vector``) and loops that can
automatically be determined to be legal to vectorize.  The LLVM-IR pass
vectorizes only explicit SIMD loops that were not previously vectorized by
the HIR pass.  The LLVM-IR pass is used most frequently when the loop
optimizer is disabled, for example at low optimization levels for GPU
SIMD kernels that require debuggability.

Because of these differences, the primary Driver routines are templatized
on the kind of loop (``llvm::loop`` or ``HLLoop``) to be optimized.  Most
vectorizer code is shared by both passes, but a significant amount of code
is only used by one pass or the other.  Developers new to the vectorizer
should compare the two implementations of ``processLoop()`` in
``IntelVPlanDriver.cpp`` to get a flavor for these differences.

The Driver calls various other subcomponents to handle each loop, such as:
 * Legalization
 * Converting input IR to VPlan
 * Importing "loop entities" (inductions, reductions, and the like)
 * Predication and linearization
 * Idiom recognition
 * VPlan-to-VPlan transforms
 * Cost modeling
 * Selection and execution of best plan
 * Code generation (producing either LLVM IR or HIR)

The Driver is implemented in the following files:

 * `.../Vectorize/IntelVPlanDriver.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Transforms/Vectorize/IntelVPlanDriver.h>`_
 * `.../Intel_VPlan/IntelVPlanDriver.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanDriver.cpp>`_

Auto-vectorization Candidates
-----------------------------

Along the HIR path, the vectorizer will perform auto-vectorization of
loops for which it's legal and potentially profitable to do so.  However,
the vectorizer does not determine which candidate loops to process.  The
requisite dependence analysis is performed prior to the ``VPlanDriverHIR``
pass.  (The same analysis is also used to find loops whose iterations can
be automatically parallelized.) HIR directives are used to communicate
candidate loops to the vectorizer.

Dependence analysis is implemented in the following files:

* `llvm/include/llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRParVecAnalysis.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRParVecAnalysis.h>`_
* `llvm/lib/Analysis/Intel_LoopAnalysis/Analysis/HIRParVecAnalysis.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Analysis/Intel_LoopAnalysis/Analysis/HIRParVecAnalysis.cpp>`_

Loop Vectorization Legality
---------------------------

As its name suggests, the legality phase determines whether there are any aspects of a loop
that preclude vectorization, other than the dependence analysis already performed.  Legality
imports explicitly declared information from the WRegion description of the loop, including
loop entities (reductions, privates, and linears, the last of which includes inductions), and
checks for any limitations within the implementation that prevents vectorization for now.

Some additional checking is needed along the LLVM IR path that isn't necessary along the HIR path.
For example, the HIR framework ensures the vectorizer only sees loops with specific control
flow patterns, but we need to check for these when consuming LLVM IR.  We must also resolve
any incompatibilities between the community's auto-recognition of inductions/reductions and
VPlan's framework for representing them.

Note that some loops don't even make it to the legality phase, as the VPlan driver also
does some legality testing in its ``isSupported()`` method.

Legality is implemented in the following files:

* `.../Intel_VPlan/IntelLoopVectorizationLegality.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelLoopVectorizationLegality.h>`_
* `.../Intel_VPlan/IntelLoopVectorizationLegality.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelLoopVectorizationLegality.cpp>`_
* `.../VPlanHIR/IntelVPlanHCFGBuilderHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPlanHCFGBuilderHIR.h>`_
* `.../VPlanHIR/IntelVPlanHCFGBuilderHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPlanHCFGBuilderHIR.cpp>`_

The HIR framework (beyond the scope of this document) can be found here:

* `llvm/lib/Analysis/Intel_LoopAnalysis/Framework/ <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Analysis/Intel_LoopAnalysis/Framework/>`_

Loop Vectorization Planner
--------------------------

Once we've determined a loop is legal to vectorize, the work to perform vectorization is
overseen by the VPlan planner.  The planner constructs an initial VPlan from scratch,
populating it with a control flow graph and VPInstructions.  It then performs various
analyses and transforms to optimize the VPlan, employs cost modeling to select the best
vectorization factor and unroll factor, determines whether peel and/or remainder loops
will be generated, generates remarks for the optimization report, and requests the code
generator to create vectorized code in the appropriate IR.

The Planner is implemented in the following files:

* `.../Intel_VPlan/IntelLoopVectorizationPlanner.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelLoopVectorizationPlanner.h>`_
* `.../Intel_VPlan/IntelLoopVectorizationPlanner.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelLoopVectorizationPlanner.cpp>`_
* `.../VPlanHIR/IntelLoopVectorizationPlannerHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelLoopVectorizationPlannerHIR.h>`_
* `.../VPlanHIR/IntelLoopVectorizationPlannerHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelLoopVectorizationPlannerHIR.cpp>`_

Framework APIs
==============

API for VPValue and Derived Types
---------------------------------

The VPlan IR framework follows the structure of the LLVM IR framework closely.  LLVM provides
``Value`` as a base class for ``User``, which is a base class for ``Instruction``.  Similarly
in VPlan IR, ``VPValue`` serves as a base class for ``VPUser``, which is a base class for
``VPInstruction``, and so on.  This portion of the API includes ``VPValue``, ``VPUser``,
``VPProxyUser``, ``VPConstant``, ``VPConstantInt``, ``VPExternalDef``, ``VPExternalUse``,
``VPMetadataAsValue``, ``VPLiveInValue``, and ``VPLiveOutValue``.

The API for VPValue and its derived types is implemented in the following file:

* `.../IntelVPlanValue.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanValue.h>`_

API for VPlan and VPInstruction
-------------------------------

The ``IntelVPlan.h`` header file is quite large, and contains most of the API for defining
and manipulating ``VPlan`` and ``VPInstruction`` objects.  The API can be broken into logical
sections:

* *General instructions:* ``VPInstruction``, ``VPPushVF``, ``VPCmpInst``, ``VPBranchInst``, ``VPBlendInst``, ``VPPHINode``, ``VPGEPInstruction``, ``VPSubscriptInst``, ``VPLoadStoreInst``, ``VPHIRCopyInst``, ``VPCallInstruction``,  ``VPConstStepVector``, ``VPOrigTripCountCalculation``, ``VPVectorTripCountCalculation``, ``VPInvSCEVWrapper``, ``VPActiveLane``, ``VPActiveLaneExtract``, ``VPConvertMaskToInt``, ``VPInsertExtractValue``, ``VPOrigLiveOutImpl``
* *Loop entity instructions:* ``VPPrivateNonPODInstImpl``, ``VPPrivateLastValueNonPODTemplInst``, ``VPInductionInit``, ``VPInductionInitStep``, ``VPInductionFinal``, ``VPReductionInit``, ``VPReductionFinal``, ``VPReductionFinalUDR``, ``VPReductionFinalInscan``, ``VPReductionFinalArrayCmplxImpl``, ``VPRunningUDSBase``, ``VPRunningInclusiveUDS``, ``VPRunningExclusiveUDS``, ``VPRunningInclusiveReduction``, ``VPRunningExclusiveReduction``, ``VPPrivateFinalC``
* *Loop representation instructions:* ``VPScalarLoopBase``, ``VPPeelRemainderImpl``, ``VPPeelRemainder``, ``VPPeelRemainderHIR``, ``VPScalarPeel``, ``VPScalarPeelHIR``, ``VPScalarRemainder``, ``VPScalarRemainderHIR``
* *Idiom instructions:* ``VPCompressExpandInitFinal``, ``VPCompressExpandInit``, ``VPCompressExpandFinal``, ``VPGeneralMemOptConflict``, ``VPTreeConflict``, ``VPConflictInsn``, ``VPPermute``, ``VPCompressExpandIndex``, ``VPCompressExpandIndexInc``
* *Memory allocation instructions:* ``F90DVBufferInit``, ``VPAllocateMemBase``, ``VPAllocateDVBuffer``, ``VPAllocatePrivate``
* *VLS instructions:* ``VPVLSBaseInst``, ``VPVLSLoad``, ``VPVLSStore``, ``VPVLSExtract``, ``VPVLSInsert``
* *VPlan and variants:* ``VPlan``, ``VPlanScalar``, ``VPlanVector``, ``VPlanScalarPeel``, ``VPlanScalarRemainder``, ``VPlanMasked``, ``VPlanNonMasked``
* *Plan adapters:* ``VPlanAdapter``, ``VPlanPeelAdapter``
* *Regions:* ``VPRegion``
* *Library calls:* ``VPTransformLibraryCall``
* *Early exit loop support:* ``VPEarlyExitCond``, ``VPEarlyExitExecMask``
* *Analysis classes:* ``VPAnalysesFactoryBase``, ``VPAnalysesFactory``, ``VPAnalysesFactoryHIR``
* *VPlan state:* ``VPIteration``, ``VPCallback``, ``VPTransformState``
* *Utilities:* ``VPlanPrinter``, ``VPlanUtils``, ``VPlanOptReportBuilder``, ``VPlanDumpControl``

The API for VPlans and VPInstructions is implemented in the following files:

* `.../Intel_VPlan/IntelVPlan.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlan.h>`_
* `.../Intel_VPlan/IntelVPlan.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlan.cpp>`_

VPlan Utilities API
-------------------

The VPlan Utilities API contains a number of standalone utility functions that are not a
part of any class.  Most of them are used to ask questions about a ``VPInstruction`` or
``VPValue``.  There is also an iterator class ``sese_df_iterator`` that provides depth-first
access to blocks of an SESE region.

The VPlan utilities API is implemented in the following file:

* `.../Intel_VPlan/IntelVPlanUtils.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanUtils.h>`_

VPlan Builder
-------------

The VPlan Builder API provides methods for creating VPlan instructions.  The ``VPBuilderHIR``
class also provides support for storing underlying HIR nodes with instructions.

The VPlan Builder API is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanBuilder.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanBuilder.h>`_
* `.../VPlanHIR/IntelVPlanBuilderHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPlanBuilderHIR.h>`_

Externals API
-------------

The externals API provides methods for tracking values that are external to the VPlan being
analyzed.  These include lists of external defs and uses, and lists of live-in and live-out
values.

API classes include ``VPUnlinkedInstructions``, ``ScalarInOutDescr``, ``ScalarInOutDescrHIR``,
``ScalarInOutList``, ``ScalarInOutListHIR``, ``VPExternalValues``, and ``VPLiveInOutCreator``.

The externals API is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanExternals.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanExternals.h>`_
* `.../Intel_VPlan/IntelVPlanExternals.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanExternals.cpp>`_

Basic Block API
---------------

The basic block API defines the ``VPBasicBlock`` class that implements basic blocks within the
VPlan CFG framework.  It includes methods for adding and removing instructions, as well as
iterators over instructions, predecessor blocks, and successor blocks.  The API also includes
the ``VPBlockUtils`` class that provides methods for splitting blocks, updating dominator trees,
and so forth, and the ``GraphTraits`` specialization for VPlan basic blocks.

The basic block API is implemented in the following files:

* `.../Intel_VPlan/IntelVPBasicBlock.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPBasicBlock.h>`_
* `.../Intel_VPlan/IntelVPBasicBlock.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPBasicBlock.cpp>`_

Loop API
--------

Loops in VPlan are represented by the ``VPLoop`` and ``VPLoopInfo`` classes.  ``VPLoop``
specializes and extends the LLVM ``LoopBase`` templated class for the VPlan data structures.
Likewise, ``VPLoopInfo`` specializes the LLVM ``LoopInfoBase`` templated class that identifies
loops in a control flow graph.  The loop ABI also includes ``GraphTraits`` and
``OptReportTraits`` for ``VPLoop`` objects, as well as iterator classes.

The loop API is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanLoopInfo.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanLoopInfo.h>`_
* `.../Intel_VPlan/IntelVPlanLoopInfo.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanLoopInfo.cpp>`_
* `.../Intel_VPlan/IntelVPlanLoopIterator.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanLoopIterator.h>`_

Loop Adapters
-------------

At one time there was an attempt to provide a shared interface between LLVM-IR Loops and
HIR HLLoops.  Some initial loop adapters were created for this purpose, but the idea
appears to have lost traction.

The loop adapters are implemented in the following file:

* `.../Intel_VPlan/IntelVPOLoopAdapters.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPOLoopAdapters.h>`_

Control Flow Graph (CFG)
========================

CFG Construction
----------------

...description...

Along the HIR path, CFG construction includes decomposing HLNodes in HIR into
VPlan instructions, and maintaining underlying data from the HIR representation.

Control flow graph construction is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanCFGBuilder.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCFGBuilder.h>`_
* `.../Intel_VPlan/IntelVPlanCFGBuilder.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCFGBuilder.cpp>`_
* `.../VPlanHIR/IntelVPlanDecomposerHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPlanDecomposerHIR.h>`_
* `.../VPlanHIR/IntelVPlanDecomposerHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPlanDecomposerHIR.cpp>`_
* `.../VPlanHIR/IntelVPlanInstructionDataHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPlanInstructionDataHIR.h>`_
* `.../VPlanHIR/IntelVPlanInstructionDataHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPlanInstructionDataHIR.cpp>`_

Hierarchical CFG Construction
-----------------------------

...description...models the loop nest using VPLoop

HCFG construction also imports loop entities from
outside the vectorizer.  See `Loop Entities`_.

HCFG construction is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanHCFGBuilder.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanHCFGBuilder.h>`_
* `.../Intel_VPlan/IntelVPlanHCFGBuilder.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanHCFGBuilder.cpp>`_
* `.../VPlanHIR/IntelVPlanHCFGBuilderHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPlanHCFGBuilderHIR.h>`_
* `.../VPlanHIR/IntelVPlanHCFGBuilderHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPlanHCFGBuilderHIR.cpp>`_

CFG Merger
----------

The CFG merger is responsible for creating peel and remainder loops and hooking them
in to a single flattened CFG.

The CFG merger is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanCFGMerger.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCFGMerger.h>`_
* `.../Intel_VPlan/IntelVPlanCFGMerger.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCFGMerger.cpp>`_

.. _Loop Entities:

Loop Entities
=============

Loop entities are special constructs that have been analyzed or provided outside
the vectorizer.  Loop entities include inductions, reductions, privates, and so
forth.  These are imported into VPlan during HCFG construction in two phases.
The first phase operates somewhat differently for LLVM IR and HIR inputs, and
creates entity descriptors in a common form.  The second phase expands the entities
from descriptors into VPlan instructions.

Loop entity management is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanHCFGBuilder.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanHCFGBuilder.h>`_
* `.../Intel_VPlan/IntelVPlanHCFGBuilder.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanHCFGBuilder.cpp>`_
* `.../Intel_VPlan/IntelVPlanLegalityDescr.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanLegalityDescr.h>`_
* `.../Intel_VPlan/IntelVPLoopAnalysis.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPLoopAnalysis.h>`_
* `.../Intel_VPlan/IntelVPLoopAnalysis.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPLoopAnalysis.cpp>`_
* `.../VPlanHIR/IntelVPlanHCFGBuilderHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPlanHCFGBuilderHIR.h>`_
* `.../VPlanHIR/IntelVPlanHCFGBuilderHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPlanHCFGBuilderHIR.cpp>`_

Analyses
========

Dominator Analysis
------------------

Dominator analysis calculates dominator and post-dominator relations over the
basic blocks in the CFG.

Dominator analysis is implemented in the following file:

* `.../Intel_VPlan/IntelVPlanDominatorTree.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanDominatorTree.h>`_

Iterated Dominance Frontier Analysis
------------------------------------

Dominance frontier analysis is used during static single assignment (SSA) formation.
The analysis is specialized for the VPlan CFG representation.

Dominance frontier analysis is implemented in the following file:

* `.../Intel_VPlan/IntelVPlanIDF.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanIDF.h>`_

Scalar Evolution Analysis
-------------------------

...description...

Scalar evolution for VPlan is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanScalarEvolution.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanScalarEvolution.h>`_
* `.../Intel_VPlan/IntelVPlanScalarEvolution.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanScalarEvolution.cpp>`_
* `.../VPlanHIR/IntelVPlanScalarEvolutionHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPlanScalarEvolutionHIR.h>`_
* `.../VPlanHIR/IntelVPlanScalarEvolutionHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPlanScalarEvolutionHIR.cpp>`_

Value Tracking Analysis
-----------------------

Value tracking analysis, including assumption analysis, is used to calculate known bits
of VPValues.  Its results are used by Divergence Analysis.

...further description...

Value tracking is implemented in the following files:

* `.../Intel_VPlan/IntelVPAlignAssumeCleanup.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPAlignAssumeCleanup.h>`_
* `.../Intel_VPlan/IntelVPAlignAssumeCleanup.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPAlignAssumeCleanup.cpp>`_
* `.../Intel_VPlan/IntelVPAssumptionCache.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPAssumptionCache.h>`_
* `.../Intel_VPlan/IntelVPAssumptionCache.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPAssumptionCache.cpp>`_
* `.../Intel_VPlan/IntelVPlanValueTracking.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanValueTracking.h>`_
* `.../Intel_VPlan/IntelVPlanValueTracking.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanValueTracking.cpp>`_
* `.../VPlanHIR/IntelVPlanValueTrackingHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPlanValueTrackingHIR.h>`_
* `.../VPlanHIR/IntelVPlanValueTrackingHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPlanValueTrackingHIR.cpp>`_

Divergence Analysis
-------------------

...description...

Divergence analysis includes shape analysis and sync dependence analysis.

Divergence analysis is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanDivergenceAnalysis.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanDivergenceAnalysis.h>`_
* `.../Intel_VPlan/IntelVPlanDivergenceAnalysis.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanDivergenceAnalysis.cpp>`_
* `.../Intel_VPlan/IntelVPlanSyncDependenceAnalysis.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanSyncDependenceAnalysis.cpp>`_
* `.../Intel_VPlan/IntelVPlanVectorShape.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVectorShape.h>`_
* `.../Intel_VPlan/IntelVPlanVectorShape.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVectorShape.cpp>`_

Struct-of-Array (SOA) Analysis
------------------------------

...description...

SOA analysis is implemented in the following files:

* `.../Intel_VPlan/IntelVPSOAAnalysis.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPSOAAnalysis.h>`_
* `.../Intel_VPlan/IntelVPSOAAnalysis.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPSOAAnalysis.cpp>`_

Alignment Analysis for Peeling
------------------------------

...description...

Alignment analysis is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanAlignmentAnalysis.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanAlignmentAnalysis.h>`_
* `.../Intel_VPlan/IntelVPlanAlignmentAnalysis.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanAlignmentAnalysis.cpp>`_

No-Cost Instruction Analysis
----------------------------

...description...

No-cost instruction analysis is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanNoCostInstructionAnalysis.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanNoCostInstructionAnalysis.h>`_
* `.../Intel_VPlan/IntelVPlanNoCostInstructionAnalysis.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanNoCostInstructionAnalysis.cpp>`_

Vector Load/Store Analysis (VLS)
--------------------------------

...description...

Utilizes the general Intel OptVLS analysis.

VLS analysis is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanVLSAnalysis.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVLSAnalysis.h>`_
* `.../Intel_VPlan/IntelVPlanVLSAnalysis.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVLSAnalysis.cpp>`_
* `.../Intel_VPlan/IntelVPlanVLSClient.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVLSClient.h>`_
* `.../Intel_VPlan/IntelVPlanVLSClient.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVLSClient.cpp>`_
* `.../VPlanHIR/IntelVPlanVLSAnalysisHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPlanVLSAnalysisHIR.h>`_
* `.../VPlanHIR/IntelVPlanVLSAnalysisHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPlanVLSAnalysisHIR.cpp>`_
* `.../VPlanHIR/IntelVPlanVLSClientHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPlanVLSClientHIR.h>`_

Function Call Vectorization Analysis
------------------------------------

...description...

Function call vectorization analysis is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanCallVecDecisions.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCallVecDecisions.h>`_
* `.../Intel_VPlan/IntelVPlanCallVecDecisions.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCallVecDecisions.cpp>`_

Scalar/Vector Analysis
----------------------

Analysis to determine which VPInstructions will be scalar/vector at code generation time.

Scalar/vector analysis is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanScalVecAnalysis.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanScalVecAnalysis.h>`_
* `.../Intel_VPlan/IntelVPlanScalVecAnalysis.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanScalVecAnalysis.cpp>`_

Cost Model
==========

Cost modeling is used to compare VPlans with different vectorization factors,
including VF=1 (scalar).  Cost modeling primarily relies on the TTI information
for the target to model instruction costs.  Sometimes additional heuristics are
needed to adjust the TTI costs.  Cost modeling is applied not only to the main
loop but also to peel and remainder loops (IntelVPlanEvaluator).

Cost modeling is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanCostModel.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCostModel.h>`_
* `.../Intel_VPlan/IntelVPlanCostModel.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCostModel.cpp>`_
* `.../Intel_VPlan/IntelVPlanCostModelHeuristics.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCostModelHeuristics.h>`_
* `.../Intel_VPlan/IntelVPlanCostModelHeuristics.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCostModelHeuristics.cpp>`_
* `.../Intel_VPlan/IntelVPlanEvaluator.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanEvaluator.h>`_
* `.../Intel_VPlan/IntelVPlanEvaluator.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanEvaluator.cpp>`_
* `.../Intel_VPlan/IntelVPlanPatternMatch.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanPatternMatch.h>`_

Transforms
==========

Loop-Closed SSA Form (LCSSA) Transformation
----------------------------------------------

Loop-closed SSA form is a variant of SSA form with the property that all
definitions within a loop only have uses that are also within the
loop, unless those uses are PHIs.  To accomplish this, trivial PHIs are added
at loop exits.  A trivial PHI has only one predecessor and is unnecessary
for minimal SSA form.

Conversion from SSA to LCSSA is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanLCSSA.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanLCSSA.h>`_
* `.../Intel_VPlan/IntelVPlanLCSSA.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanLCSSA.cpp>`_

Loop Exit Canonicalization
--------------------------

...description...

Loop exit canonicalization is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanLoopExitCanonicalization.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanLoopExitCanonicalization.h>`_
* `.../Intel_VPlan/IntelVPlanLoopExitCanonicalization.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanLoopExitCanonicalization.cpp>`_

Loop Control Flow Uniformity (CFU) Transform
--------------------------------------------

...description...

The Loop CFU transform is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanLoopCFU.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanLoopCFU.h>`_
* `.../Intel_VPlan/IntelVPlanLoopCFU.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanLoopCFU.cpp>`_

Predication and Linearization
-----------------------------

...description...

Predication and linearization are implemented in the following files:

* `.../Intel_VPlan/IntelVPlanPredicator.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanPredicator.h>`_
* `.../Intel_VPlan/IntelVPlanPredicator.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanPredicator.cpp>`_

All-Zero Bypass Transform
-------------------------

...description...

The All-Zero Bypass Transform is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanAllZeroBypass.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanAllZeroBypass.h>`_
* `.../Intel_VPlan/IntelVPlanAllZeroBypass.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanAllZeroBypass.cpp>`_

Masked Loop Variant Transform
-----------------------------

...description...

The masked loop variant transform is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanMaskedModeLoop.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanMaskedModeLoop.h>`_
* `.../Intel_VPlan/IntelVPlanMaskedModeLoop.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanMaskedModeLoop.cpp>`_

Vector Load-Store Transform
---------------------------

...description...

The VLS transform is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanVLSTransform.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVLSTransform.h>`_
* `.../Intel_VPlan/IntelVPlanVLSTransform.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVLSTransform.cpp>`_

Library Call Transform
----------------------

...description...

The library call transform is implemented in the following files:

* `.../IntelVPTransformLibraryCalls.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPTransformLibraryCalls.h>`_
* `.../IntelVPTransformLibraryCalls.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPTransformLibraryCalls.cpp>`_

OMP SIMD Transforms
-------------------

...description...

The OMP SIMD transforms are implemented in the following files:

* `.../Vectorize/IntelVPlanPragmaOmpOrderedSimdExtract.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Transforms/Vectorize/IntelVPlanPragmaOmpOrderedSimdExtract.h>`_
* `.../Intel_VPlan/IntelVPlanPragmaOmpOrderedSimdExtract.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanPragmaOmpOrderedSimdExtract.cpp>`_
* `.../Vectorize/IntelVPlanPragmaOmpSimdIf.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Transforms/Vectorize/IntelVPlanPragmaOmpSimdIf.h>`_
* `.../Intel_VPlan/IntelVPlanPragmaOmpSimdIf.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanPragmaOmpSimdIf.cpp>`_

Search Loop Idiom Transform
---------------------------

"Search loops" are multi-exit loops of a specific form that can be vectorized.
Currently these are handled on a somewhat ad-hoc basis, rather than transforming
them into canonical forms that the normal VPlan analyses can use.  This code is
temporary and should be removed in 2H23 after proper handling of search loops has
been implemented.

The search loop idiom transform is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanIdioms.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanIdioms.h>`_
* `.../Intel_VPlan/IntelVPlanIdioms.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanIdioms.cpp>`_

Early-Exit Loop Transform
-------------------------

This WIP transform will replace the search loop idiom transform when completed.
It transforms vectorizable multiple-exit loops into equivalent single-exit loops
that can be consumed by the VPlan framework.

The early-exit loop transform is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanTransformEarlyExit.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanTransformEarlyExit.h>`_
* `.../Intel_VPlan/IntelVPlanTransformEarlyExit.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanTransformEarlyExit.cpp>`_

VConflict Transform
-------------------

...description...

The VConflict transform is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanVConflictTransformation.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVConflictTransformation.h>`_
* `.../Intel_VPlan/IntelVPlanVConflictTransformation.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVConflictTransformation.cpp>`_

Loop Unrolling Transform
------------------------

...description...

The loop unrolling transform is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanClone.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanClone.h>`_
* `.../Intel_VPlan/IntelVPlanClone.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanClone.cpp>`_
* `.../Intel_VPlan/IntelVPlanLoopUnroller.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanLoopUnroller.h>`_
* `.../Intel_VPlan/IntelVPlanLoopUnroller.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanLoopUnroller.cpp>`_

Struct-of-Array (SOA) Transform
-------------------------------

...description...

The SOA transform is implemented in the following files:

* `.../Intel_VPlan/IntelVPMemRefTransform.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPMemRefTransform.h>`_
* `.../Intel_VPlan/IntelVPMemRefTransform.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPMemRefTransform.cpp>`_

SSA Deconstruction Transform
----------------------------

Prior to HIR code generation, VPlan must be taken out of SSA form.  This transform inserts
copies to implement the action of PHI nodes, but does not actually remove the PHIs.

The SSA deconstruction transform is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanSSADeconstruction.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanSSADeconstruction.h>`_
* `.../Intel_VPlan/IntelVPlanSSADeconstruction.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanSSADeconstruction.cpp>`_

Code Generation
===============

The final stage of the VPlan vectorizer translates VPlan instructions back into the
input format; that is, into either HIR or LLVM IR.  This "code generation" phase should
not be confused with lowering from LLVM IR into Machine IR, or from Machine IR to final
target code.

VPlan code generation is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanVectorizeIndirectCalls.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVectorizeIndirectCalls.h>`_
* `.../Intel_VPlan/IntelVPlanVectorizeIndirectCalls.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVectorizeIndirectCalls.cpp>`_
* `.../Intel_VPlan/IntelVPOCodeGen.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPOCodeGen.h>`_
* `.../Intel_VPlan/IntelVPOCodeGen.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPOCodeGen.cpp>`_
* `.../VPlanHIR/IntelVPOCodeGenHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPOCodeGenHIR.h>`_
* `.../VPlanHIR/IntelVPOCodeGenHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPOCodeGenHIR.cpp>`_

Optimization Report
===================

...description...

The vectorizer support for the optimization report is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanOptrpt.inc <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanOptrpt.inc>`_
* `.../Intel_VPlan/IntelVPlanOptrpt.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanOptrpt.h>`_
* `.../Intel_VPlan/IntelVPlanOptrpt.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanOptrpt.cpp>`_

The Intel optimization report framework is implemented in the following directories:

* `llvm/include/llvm/Analysis/Intel_OptReport/ <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Analysis/Intel_OptReport/>`_
* `llvm/lib/Analysis/Intel_OptReport/ <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Analysis/Intel_OptReport/>`_

Miscellany
==========

VPlan Verifier
--------------

The VPlan verifier can be called anytime after VPlan formation and before code generation.
It checks the VPlan data structures to be certain everything is well-formed and consistent.

The VPlan verifier is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanVerifier.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVerifier.h>`_
* `.../Intel_VPlan/IntelVPlanVerifier.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVerifier.cpp>`_
* `.../VPlanHIR/IntelVPlanVerifierHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPlanVerifierHIR.h>`_
* `.../VPlanHIR/IntelVPlanVerifierHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR/IntelVPlanVerifierHIR.cpp>`_

Function Vectorizer
-------------------

The function vectorizer is a separate driver that supports generation of
simplified test cases in limited situations.  It has been used particularly
for generating tests for the predicator.  ...further description...

The function vectorizer is implemented in the following files:

* `.../Vectorize/IntelVPlanFunctionVectorizer.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Transforms/Vectorize/IntelVPlanFunctionVectorizer.h>`_
* `.../Intel_VPlan/IntelVPlanFunctionVectorizer.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanFunctionVectorizer.cpp>`_

Minimal OpenCL support
----------------------

...description...

The OpenCL support is implemented in the following file:

* `.../IntelVPlan/IntelVolcanoOpenCL.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVolcanoOpenCL.h>`_

SLP Vectorization
=================

The SLP vectorizer is an LLVM community pass that contains some Intel customizations.  It is
maintained by the VPlan Vectorizer team.

SLP is generally agreed to stand for *superword-level parallelism*, which isn't particularly
descriptive of its function.  SLP vectorization looks for opportunities to replace groups of
scalar instructions with equivalent vector instructions in any region of code where the first
group of scalar instructions dominates all other instructions being vectorized together.
This includes vectorization opportunities within a single loop iteration (but not across
loop iterations) and opportunities outside loops.

SLP vectorization is implemented in the following files:

* `.../Vectorize/SLPVectorizer.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Transforms/Vectorize/SLPVectorizer.h>`_
* `llvm/lib/Transforms/Vectorize/SLPVectorizer.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/SLPVectorizer.cpp>`_

Other Passes
============

There are a number of related passes that are not directly part of the VPlan Vectorizer.

Vector Function Variants (VecClone)
-----------------------------------

...description...

Generation of vector function variants is implemented in the following files:

* `llvm/include/llvm/Transforms/Utils/Intel_VecClone.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Transforms/Utils/Intel_VecClone.h>`_
* `llvm/lib/Transforms/Utils/Intel_VecClone.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Utils/Intel_VecClone.cpp>`_

SYCL Kernel Vector Variants
---------------------------

...description...

Generation of SYCL kernel vector variants is implemented in the following files:

* `llvm/include/llvm/Transforms/SYCLTransforms/Intel_SYCLKernelVecClone.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Transforms/SYCLTransforms/Intel_SYCLKernelVecClone.h>`_
* `llvm/lib/Transforms/SYCLTransforms/Intel_SYCLKernelVecClone.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/SYCLTransforms/Intel_SYCLKernelVecClone.cpp>`_
* `llvm/include/llvm/Transforms/SYCLTransforms/Intel_SYCLPrepareKernelForVecClone.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Transforms/SYCLTransforms/Intel_SYCLPrepareKernelForVecClone.h>`_
* `llvm/lib/Transforms/SYCLTransforms/Intel_SYCLPrepareKernelForVecClone.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/SYCLTransforms/Intel_SYCLPrepareKernelForVecClone.cpp>`_

Vector Function ABI Demangling
------------------------------

...description...

Vector function ABI demangling is implemented in the following files:

* `llvm/lib/Analysis/VFABIDemangling.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Analysis/VFABIDemangling.cpp>`_
* `llvm/include/llvm/Analysis/VectorUtils.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Analysis/VectorUtils.h>`_
* `llvm/lib/Analysis/VectorUtils.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Analysis/VectorUtils.cpp>`_

SVML Mappings
-------------

The SVML (Short Vector Math Library) mappings infrastructure generates maps from scalar math
functions to their target-specific vectorized counterparts in the Intel SVML library.  The mappings
are later consumed by VPlanCallVecDecisions.

The SVML mappings infrastructure is implemented in the following files:

* `llvm/include/llvm/IR/Intel_SVML.td <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/IR/Intel_SVML.td>`_
* `llvm/utils/TableGen/Intel_SVMLEmitter.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/utils/TableGen/Intel_SVMLEmitter.cpp>`_

Math Function Replacement Pass
------------------------------

This pass replaces certain math functions with SVML equivalents.

The math function replacement pass is implemented in the following files:

* `.../Vectorize/IntelMFReplacement.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Transforms/Vectorize/IntelMFReplacement.h>`_
* `llvm/lib/Transforms/Vectorize/IntelMFReplacement.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/IntelMFReplacement.cpp>`_

Load Coalescing Pass
--------------------

...description...

The load coalescing pass is implemented in the following files:

* `.../Vectorize/Intel_LoadCoalescing.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Transforms/Vectorize/Intel_LoadCoalescing.h>`_
* `llvm/lib/Transforms/Vectorize/Intel_LoadCoalescing.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_LoadCoalescing.cpp>`_
