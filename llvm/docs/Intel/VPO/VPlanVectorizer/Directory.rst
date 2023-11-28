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

Almost all vectorizer code is found in one directory and its
subdirectories::

   llvm/lib/Transforms/Vectorize/Intel_VPlan          [.../Intel_VPlan]
   llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR      [.../HIR]
   llvm/lib/Transforms/Vectorize/Intel_VPlan/LLVM     [.../LLVM]

The intention of this directory structure is to separate code that is dependent
on the form of the input IR from code that is independent of it.  VPlan is
designed as a separate IR for modeling vectorization opportunities.  Currently
VPlan can be invoked using either LLVM IR or the loop optimizer's HIR representation;
each is converted into the VPlan IR.  (See :doc:`Pipeline` for a full discussion.)
Code in the ``Intel_VPlan`` directory is independent of which IR was chosen.
Code in the ``HIR`` subdirectory is specific to HIR.  Code in the ``LLVM``
subdirectory is specific to LLVM IR.

Work is currently underway to restructure code to meet the above guidelines (and
clean it up in other ways).  Until that work is complete, some files in ``[.../Intel_VPlan]``
will contain IR-dependent code.

A few files are co-located with the community vectorizer's include path in order
to make them available to the pass manager::

   llvm/include/llvm/Transforms/Vectorize             [.../Vectorize]

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

 * `.../Vectorize/IntelVPlanDriverPass.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Transforms/Vectorize/IntelVPlanDriverPass.h>`_
 * `.../Intel_VPlan/IntelVPlanDriver.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanDriver.h>`_
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

* `.../Intel_VPlan/Legality.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/Legality.h>`_
* `.../Intel_VPlan/Legality.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/Legality.cpp>`_
* `.../HIR/LegalityHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/LegalityHIR.h>`_
* `.../HIR/LegalityHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/LegalityHIR.cpp>`_
* `.../LLVM/LegalityLLVM.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/LLVM/LegalityLLVM.h>`_
* `.../LLVM/LegalityLLVM.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/LLVM/LegalityLLVM.cpp>`_

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
* `.../HIR/IntelLoopVectorizationPlannerHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/IntelLoopVectorizationPlannerHIR.h>`_
* `.../HIR/IntelLoopVectorizationPlannerHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/IntelLoopVectorizationPlannerHIR.cpp>`_

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

* `.../Intel_VPlan/IntelVPlanValue.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanValue.h>`_

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
* `.../HIR/IntelVPlanBuilderHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/IntelVPlanBuilderHIR.h>`_

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

The first step in VPlan construction is to create a control flow graph (CFG) containing
VPInstructions that represent the semantics of the input IR.
Constructing the CFG from LLVM IR is fairly straightforward, since the representations
are similar.  The HIR path is more complex, since HIR uses a high-level lexical ordering
representation that must be converted into a CFG in static-single assignment form.  The
code that makes this transformation for HIR is called the ``Decomposer``.

Once the basic CFG has been created, the vectorizer performs loop analysis
on the CFG, producing the ``VPLoopInfo`` structure that models the loop nest.  This phase
also imports loop entities into VPlan from outside the vectorizer.
See `Loop Entities`_.  This process necessarily differs between the LLVM IR and HIR paths.
The two paths use different subclasses of ``VPEntityConverterBase`` to produce a common format
for VPlan to consume.

Control flow graph construction is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanCFGBuilder.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCFGBuilder.h>`_
* `.../Intel_VPlan/IntelVPlanCFGBuilder.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCFGBuilder.cpp>`_
* `.../Intel_VPlan/IntelVPlanHCFGBuilder.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanHCFGBuilder.h>`_
* `.../Intel_VPlan/IntelVPlanHCFGBuilder.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanHCFGBuilder.cpp>`_
* `.../HIR/IntelVPlanDecomposerHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/IntelVPlanDecomposerHIR.h>`_
* `.../HIR/IntelVPlanDecomposerHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/IntelVPlanDecomposerHIR.cpp>`_
* `.../HIR/IntelVPlanHCFGBuilderHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/IntelVPlanHCFGBuilderHIR.h>`_
* `.../HIR/IntelVPlanHCFGBuilderHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/IntelVPlanHCFGBuilderHIR.cpp>`_
* `.../HIR/IntelVPlanInstructionDataHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/IntelVPlanInstructionDataHIR.h>`_
* `.../HIR/IntelVPlanInstructionDataHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/IntelVPlanInstructionDataHIR.cpp>`_

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
forth.  These may be explicit entities identified in the source code (for example,
using ``#pragma omp simd`` clauses), or they may be auto-recognized.  Entities are
imported into VPlan during HCFG construction in two phases.
The first phase operates somewhat differently for LLVM IR and HIR inputs, and
creates entity descriptors in a common form.  The second phase expands the entities
from descriptors into VPlan instructions.

Loop entity management is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanHCFGBuilder.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanHCFGBuilder.h>`_
* `.../Intel_VPlan/IntelVPlanHCFGBuilder.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanHCFGBuilder.cpp>`_
* `.../Intel_VPlan/IntelVPlanLegalityDescr.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanLegalityDescr.h>`_
* `.../Intel_VPlan/IntelVPLoopAnalysis.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPLoopAnalysis.h>`_
* `.../Intel_VPlan/IntelVPLoopAnalysis.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPLoopAnalysis.cpp>`_
* `.../HIR/IntelVPlanHCFGBuilderHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/IntelVPlanHCFGBuilderHIR.h>`_
* `.../HIR/IntelVPlanHCFGBuilderHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/IntelVPlanHCFGBuilderHIR.cpp>`_

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

*Scalar evolution* is a standard term for compiler analysis of chains of recurrences within a loop.
Within the vectorizer we are most interested in linear recurrences such as ``{0,+,1}``, describing
a variable that starts at 0 and is incremented by 1 during each iteration of a loop.  VPlan
implements its own versions of scalar evolution.  VPlan's LLVM IR version is relatively simple
and can interoperate with LLVM's community SCEV analysis.  The HIR SCEV analysis is based on
``CanonExpr`` and ``RegDDRef`` operands from the HIR framework.

Scalar evolution for VPlan is implemented in the following files:

* `.../Intel_VPlan/ScalarEvolution.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/ScalarEvolution.h>`_
* `.../Intel_VPlan/ScalarEvolution.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/ScalarEvolution.cpp>`_
* `.../HIR/ScalarEvolutionHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/ScalarEvolutionHIR.h>`_
* `.../HIR/ScalarEvolutionHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/ScalarEvolutionHIR.cpp>`_
* `.../LLVM/ScalarEvolutionLLVM.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/LLVM/ScalarEvolutionLLVM.h>`_
* `.../LLVM/ScalarEvolutionLLVM.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/LLVM/ScalarEvolutionLLVM.cpp>`_

.. _ValueTracking-label:

Value Tracking Analysis
-----------------------

Value tracking analysis, including assumption analysis, is used to calculate known bits
of VPValues.  Its results are used by Divergence Analysis.

Value tracking is implemented in the following files:

* `.../Intel_VPlan/IntelVPAlignAssumeCleanup.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPAlignAssumeCleanup.h>`_
* `.../Intel_VPlan/IntelVPAlignAssumeCleanup.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPAlignAssumeCleanup.cpp>`_
* `.../Intel_VPlan/IntelVPAssumptionCache.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPAssumptionCache.h>`_
* `.../Intel_VPlan/IntelVPAssumptionCache.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPAssumptionCache.cpp>`_
* `.../Intel_VPlan/IntelVPlanValueTracking.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanValueTracking.h>`_
* `.../Intel_VPlan/IntelVPlanValueTracking.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanValueTracking.cpp>`_
* `.../HIR/IntelVPlanValueTrackingHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/IntelVPlanValueTrackingHIR.h>`_
* `.../HIR/IntelVPlanValueTrackingHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/IntelVPlanValueTrackingHIR.cpp>`_

Divergence Analysis
-------------------

:doc:`DivergenceAnalysis`

Divergence Analysis (DA) analyzes the behavior of values within a vector.  These behaviors are
referred to as `shapes`, which include `uniform` (identical in all lanes of a vector),
`strided` (increasing or decreasing by a fixed amount), and `random` (unknown shape),
along with various other shapes associated with SOA analysis.  DA is used throughout the
vectorizer, in such phases as All-Zero Bypass, Loop CFU, Code Generation, and many more.

Divergence analysis is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanDivergenceAnalysis.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanDivergenceAnalysis.h>`_
* `.../Intel_VPlan/IntelVPlanDivergenceAnalysis.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanDivergenceAnalysis.cpp>`_
* `.../Intel_VPlan/IntelVPlanSyncDependenceAnalysis.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanSyncDependenceAnalysis.cpp>`_
* `.../Intel_VPlan/IntelVPlanVectorShape.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVectorShape.h>`_
* `.../Intel_VPlan/IntelVPlanVectorShape.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVectorShape.cpp>`_

.. _SOA analysis:

Struct-of-Array (SOA) Analysis
------------------------------

Struct-of-Array (SOA) analysis determines whether it is legal and profitable to change the
data layout of loop privates.  A common programming idiom is to create arrays whose elements
are structures of more than one scalar field.  When algorithms access individual structure
fields across such an array, the access patterns are strided by the size of the structure,
which may be a large value.  Typically vectorizing such accesses requires gather loads and
scatter stores.

Loop privates are copies of data that are not externally visible.  When creating loop privates,
the vectorizer has the freedom to represent the data differently.  Specifically, we can convert
an array of structs into a struct of arrays, where there is a separate array for each element
of the struct.  Strided accesses to these more compact arrays are much more efficient when
the common access pattern is accessing the same struct field in each array element.

SOA analysis is implemented in the following files:

* `.../Intel_VPlan/IntelVPSOAAnalysis.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPSOAAnalysis.h>`_
* `.../Intel_VPlan/IntelVPSOAAnalysis.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPSOAAnalysis.cpp>`_

Alignment Analysis for Peeling
------------------------------

It is often beneficial to peel a loop by some number of scalar iterations in order to
align memory references on an efficient boundary for vector loads and stores.  Alignment
analysis considers the memory references in a loop to determine which reference(s) are
most beneficial to align by peeling.

Alignment analysis is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanAlignmentAnalysis.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanAlignmentAnalysis.h>`_
* `.../Intel_VPlan/IntelVPlanAlignmentAnalysis.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanAlignmentAnalysis.cpp>`_

No-Cost Instruction Analysis
----------------------------

This analysis determines which instructions should be ignored during cost modeling.  There are
two flavors of no-cost instruction analysis.  One version identifies instructions that only
exist to calculate values in ``@llvm.assume`` directives; such instructions should always be
ignored.  The other version analyzes a masked-mode VPlan to identify loop normalization
instructions; these should be ignored when the VPlan is peeled for alignment.

No-cost instruction analysis is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanNoCostInstructionAnalysis.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanNoCostInstructionAnalysis.h>`_
* `.../Intel_VPlan/IntelVPlanNoCostInstructionAnalysis.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanNoCostInstructionAnalysis.cpp>`_

Vector Load/Store Analysis (VLS)
--------------------------------

The Vector Load/Store (VLS) analysis performs legality and profitability testing for the
`VLS transform`_.  It makes use of the general Intel ``OptVLS`` infrastructure, which is
IR-agnostic for use by several clients (vectorizer, HIR loop optimizer, and ``OptVLSPass``).

VLS analysis is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanVLSAnalysis.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVLSAnalysis.h>`_
* `.../Intel_VPlan/IntelVPlanVLSAnalysis.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVLSAnalysis.cpp>`_
* `.../Intel_VPlan/IntelVPlanVLSClient.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVLSClient.h>`_
* `.../Intel_VPlan/IntelVPlanVLSClient.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVLSClient.cpp>`_
* `.../HIR/IntelVPlanVLSAnalysisHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/IntelVPlanVLSAnalysisHIR.h>`_
* `.../HIR/IntelVPlanVLSAnalysisHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/IntelVPlanVLSAnalysisHIR.cpp>`_
* `.../HIR/IntelVPlanVLSClientHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/IntelVPlanVLSClientHIR.h>`_

The Intel ``OptVLS`` infrastructure is implemented in the following files:

* `llvm/include/llvm/Analysis/Intel_OptVLS.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Analysis/Intel_OptVLS.h>`_
* `llvm/lib/Analysis/Intel_OptVLS.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Analysis/Intel_OptVLS.cpp>`_
* `llvm/lib/Analysis/Intel_OptVLSClientUtils.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Analysis/Intel_OptVLSClientUtils.cpp>`_

.. _CallVecDecisions-label:

Function Call Vectorization Analysis
------------------------------------

Function call vectorization analysis considers a specific vectorization factor and the
available vector function variants that correspond to a scalar function call, determining
the best call or sequence of calls to use for that vectorization factor.

Function call vectorization analysis is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanCallVecDecisions.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCallVecDecisions.h>`_
* `.../Intel_VPlan/IntelVPlanCallVecDecisions.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanCallVecDecisions.cpp>`_

.. _SVA-label:

Scalar/Vector Analysis
----------------------

This analysis determines which VPInstructions will be vectorized during code generation, and
which will be left as scalar instructions.

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

Loop exit canonicalization comprises two utility functions that place loops into a canonical
form suitable for vectorization.  The first converts single-exit ``while`` loops (where
the loop exit is in the loop header) into single-exit ``do until`` loops (where the loop
exit is in the loop latch).  The second converts inner loops with more than one exit into
equivalent loops with a single exit at the loop latch.

Loop exit canonicalization is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanLoopExitCanonicalization.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanLoopExitCanonicalization.h>`_
* `.../Intel_VPlan/IntelVPlanLoopExitCanonicalization.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanLoopExitCanonicalization.cpp>`_

.. _LoopCFU-label:

Loop Control Flow Uniformity (CFU) Transform
--------------------------------------------

The Loop CFU transform ensures that all back edges for an innermost loop have uniform
control flow.  This is done by replacing any divergent condition in a latch block with an
all-zero-check instruction and introducing a mask controlling execution of the original
loop body.

The Loop CFU transform is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanLoopCFU.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanLoopCFU.h>`_
* `.../Intel_VPlan/IntelVPlanLoopCFU.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanLoopCFU.cpp>`_

Predication and Linearization
-----------------------------

The purpose of the predication transform is to remove divergent control flow inside candidate loops
and replace it with predicate masks that control execution of instructions in each basic
block.  For example, a simple ``if-then-else`` sequence with a single block in the ``then``
and ``else`` clauses, with control flow controlled by a predicate `P`, is converted into a
sequence of two blocks that are always executed, the first under a mask encoding those
iterations on which `P` is true, and the second under the inversion of that mask.  The
"flattening" of the CFG is referred to as `linearization`, and the introduction of masks
is known as `predication`.

Predication and linearization are implemented in the following files:

* `.../Intel_VPlan/IntelVPlanPredicator.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanPredicator.h>`_
* `.../Intel_VPlan/IntelVPlanPredicator.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanPredicator.cpp>`_

.. _AZB-label:

All-Zero Bypass Transform
-------------------------

The All-Zero Bypass transformation looks for single-entry/single-exit regions within the VPlan
CFG that are executed under a divergent mask; that is, under a mask that may be different for
different vector lanes.  The all-zero bypass can be introduced around a region to avoid
execution when the mask has all bits equal to zero.

The All-Zero Bypass Transform is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanAllZeroBypass.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanAllZeroBypass.h>`_
* `.../Intel_VPlan/IntelVPlanAllZeroBypass.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanAllZeroBypass.cpp>`_

.. _MaskedModeLoopTransform-label:

Masked Loop Variant Transform
-----------------------------

The Masked Loop Variant transform is used to create variants of loop bodies that operate
under an iteration mask.  The primary reason for this is to create masked, primarily
single-iteration, loops in cases where we know that we will have no more than VF scalar
iterations; these are remainder loops and loops with known trip counts.  For example, if
it's known at compile time that the loop consists of 5 iterations, we can vectorize it with
VF=8.  The masked variant is created by wrapping the loop body in an ``if`` condition comparing
the loop index with the upper bound.  The predicator will then create the necessary masked
code.

The masked loop variant transform is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanMaskedModeLoop.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanMaskedModeLoop.h>`_
* `.../Intel_VPlan/IntelVPlanMaskedModeLoop.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanMaskedModeLoop.cpp>`_

.. _VLS transform:

Vector Load-Store Transform
---------------------------

The Vector Load-Store (VLS) transform looks for memory references that can be combined
into wider loads or stores for a given vectorization factor.  The memory references need not
be uniform in size or alignment, and need not all be adjacent.  The selected group of memory
references is replaced by a single load or store, with insert/extract operations used to
access the individual scalars.

The VLS transform is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanVLSTransform.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVLSTransform.h>`_
* `.../Intel_VPlan/IntelVPlanVLSTransform.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVLSTransform.cpp>`_

Library Call Transform
----------------------

The Library Call transform replaces library calls whose scalar signature does not match their
vectorized signature (e.g., ``sincos``) with a special transformed library call instruction
that records this mismatch.  It also inserts any post-processing instructions necessary to
handle the signature mismatch after vectorization occurs.

The library call transform is implemented in the following files:

* `.../IntelVPTransformLibraryCalls.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPTransformLibraryCalls.h>`_
* `.../IntelVPTransformLibraryCalls.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPTransformLibraryCalls.cpp>`_

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

.. _VConflictTransform-label:

VConflict Idiom Transform
-------------------------

The VConflict Idiom transform makes it possible to vectorize loops containing updates to
array elements that may conflict during multiple iterations.  For example, consider a typical
histogram loop::

  for (int i = 0; i < N; i++) {
    index = B[i];
    A[index] += K;
  }

If this loop is vectorized, an element of A may be updated by more than one lane of B.
A straightforward implementation of a gather load, increment by broadcast K, and scatter
store is not correct if this occurs.  The VConflict idiom transform generates special
VPlan instructions to ensure correct code is generated.

The VConflict idiom transform is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanVConflictTransformation.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVConflictTransformation.h>`_
* `.../Intel_VPlan/IntelVPlanVConflictTransformation.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanVConflictTransformation.cpp>`_

Loop Unrolling Transform
------------------------

After VPlan cost modeling is complete, we have determined the most profitable vectorization
factor (``VF``) and unroll factor (``UF``) to apply to the VPlan loop.  The Loop Unrolling
transform duplicates the VPlan loop body ``UF`` times.  The unroller also performs partial-sum
reduction on the unrolled loop body to increase the amount of instruction-level parallelism
and reduce bottlenecks in reductions.

The loop unrolling transform is implemented in the following files:

* `.../Intel_VPlan/IntelVPlanClone.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanClone.h>`_
* `.../Intel_VPlan/IntelVPlanClone.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanClone.cpp>`_
* `.../Intel_VPlan/IntelVPlanLoopUnroller.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanLoopUnroller.h>`_
* `.../Intel_VPlan/IntelVPlanLoopUnroller.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanLoopUnroller.cpp>`_

Struct-of-Array (SOA) Transform
-------------------------------

The Struct-of-Array (SOA) transform performs the data layout transformation found to be legal and
profitable by `SOA analysis`_.

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
* `.../Intel_VPlan/LLVM/CodeGenLLVM.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/LLVM/CodeGenLLVM.h>`_
* `.../Intel_VPlan/LLVM/CodeGenLLVM.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/LLVM/CodeGenLLVM.cpp>`_
* `.../HIR/IntelVPOCodeGenHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/IntelVPOCodeGenHIR.h>`_
* `.../HIR/IntelVPOCodeGenHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/IntelVPOCodeGenHIR.cpp>`_

Optimization Report
===================

Users may choose to generate an Intel Optimization Report that describes optimizations
that occurred and optimizations that were prevented.  The vectorization report is a
substantial part of the Intel optimization report.  We provide our information in the
form of metadata that is anchored by the ``!llvm.loop`` metadata attached to each loop's
latch branch.  In addition to specifying which loops were and were not vectorized, along
with reasons that vectorization was skipped, we also provide statistics about memory
references, information about peel and remainder loops, and so forth.

The vectorizer support for the optimization report is primarily implemented in the following
files:

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
* `.../HIR/IntelVPlanVerifierHIR.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/IntelVPlanVerifierHIR.h>`_
* `.../HIR/IntelVPlanVerifierHIR.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/HIR/IntelVPlanVerifierHIR.cpp>`_

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

This file contains a single array of mangled names for ``select`` functions for OpenCL.
This is used when necessary by Code Generation.

The OpenCL support is implemented in the following file:

* `.../IntelVPlan/IntelVolcanoOpenCL.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVolcanoOpenCL.h>`_

SLP Vectorization
=================

The SLP vectorizer is an LLVM community pass that contains some Intel customizations.  It is
maintained by the VPlan Vectorizer team.

SLP is generally agreed to stand for *superword-level parallelism*, which isn't particularly
descriptive of its function.  SLP vectorization looks for opportunities to replace groups of
scalar instructions with equivalent vector instructions in any region of code where the first
group of scalar instructions dominates all other instructions being vectorized together
(i.e., in an `extended basic block`).
This includes vectorization opportunities within a single loop iteration (but not across
loop iterations) and opportunities outside loops.

SLP vectorization is implemented in the following files:

* `.../Vectorize/SLPVectorizer.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Transforms/Vectorize/SLPVectorizer.h>`_
* `llvm/lib/Transforms/Vectorize/SLPVectorizer.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/SLPVectorizer.cpp>`_

Other Passes
============

There are a number of related passes that are not directly part of the VPlan Vectorizer.

.. _Vector Function Variants:

Vector Function Variants (VecClone)
-----------------------------------

The ``VecClone`` pass runs very early in the pass pipeline.  For user functions that have
been declared with ``#pragma omp declare simd``, ``VecClone`` creates the necessary versions
of the functions.  The variant specifications are created by the front end(s), and the vector
functions have mangled names according to the `Intel Vector Function ABI
<https://www.intel.com/content/dam/develop/external/us/en/documents/intel-vector-function-abi.pdf>`_.
``VecClone`` works by cloning the function and creating a scalar loop around the function body
with a trip count that matches the vector factor defined for the variant, and then relying on
the VPlan vectorizer to vectorize the function body.

Generation of vector function variants is implemented in the following files:

* `llvm/include/llvm/Transforms/Utils/Intel_VecClone.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Transforms/Utils/Intel_VecClone.h>`_
* `llvm/lib/Transforms/Utils/Intel_VecClone.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Utils/Intel_VecClone.cpp>`_

SYCL Kernel Vector Variants
---------------------------

SYCL kernel vector variants are generated in an analogous fashion to `Vector Function Variants`_.

Generation of SYCL kernel vector variants is implemented in the following files:

* `llvm/include/llvm/Transforms/SYCLTransforms/Intel_SYCLKernelVecClone.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Transforms/SYCLTransforms/Intel_SYCLKernelVecClone.h>`_
* `llvm/lib/Transforms/SYCLTransforms/Intel_SYCLKernelVecClone.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/SYCLTransforms/Intel_SYCLKernelVecClone.cpp>`_
* `llvm/include/llvm/Transforms/SYCLTransforms/Intel_SYCLPrepareKernelForVecClone.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Transforms/SYCLTransforms/Intel_SYCLPrepareKernelForVecClone.h>`_
* `llvm/lib/Transforms/SYCLTransforms/Intel_SYCLPrepareKernelForVecClone.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/SYCLTransforms/Intel_SYCLPrepareKernelForVecClone.cpp>`_

OMP SIMD Transforms
-------------------

The OMP SIMD transforms currently comprise two transformations to support specific clauses
of ``#pragma omp simd``.  They operate on LLVM IR.  Along the HIR path, they execute in the
loop optimization pipeline prior to the HIR SSA Deconstruction pass.  Along the LLVM IR path,
they execute just prior to the vectorizer.

The first of these transforms handles ``#pragma omp simd ordered``, which specifies a region
within a SIMD loop that should be executed sequentially.  This is implemented by outlining
the region into a function, and replacing the region with the call of that function.  Then
during vectorization the call is scalarized.  The function has the always-inline attribute,
and thus is inlined after vectorization, implementing the required semantics of ``#pragma
omp simd ordered``.  This is considered to be a temporary solution until the necessary
processing is implemented within VPlan.

Second, ``#pragma omp simd if`` implements the semantics required by the OpenMP 5.0
standard.  The region guarded by the pragma is duplicated, with one copy using ``#pragma omp
simd`` and the other using ``#pragma omp simd simdlen(1)``.  If the ``if`` condition is true,
the copy using ``#pragma omp simd`` is executed; otherwise the scalarized copy is executed.

The OMP SIMD transforms are implemented in the following files:

* `.../Vectorize/IntelVPlanPragmaOmpOrderedSimdExtract.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Transforms/Vectorize/IntelVPlanPragmaOmpOrderedSimdExtract.h>`_
* `.../Intel_VPlan/IntelVPlanPragmaOmpOrderedSimdExtract.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanPragmaOmpOrderedSimdExtract.cpp>`_
* `.../Vectorize/IntelVPlanPragmaOmpSimdIf.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Transforms/Vectorize/IntelVPlanPragmaOmpSimdIf.h>`_
* `.../Intel_VPlan/IntelVPlanPragmaOmpSimdIf.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanPragmaOmpSimdIf.cpp>`_

Vector Function ABI Demangling
------------------------------

There are utilities for demangling function names according to the `Intel Vector Function ABI
<https://www.intel.com/content/dam/develop/external/us/en/documents/intel-vector-function-abi.pdf>`_.
This is needed when performing argument matching for vector function variants.

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

The Load Coalescing pass is a cleanup pass that runs following the SLP vectorizer.  It looks
for cases where multiple vector loads can be replaced by a wider vector load, with subsequent
shuffles to produce the loaded values for each original load.

The load coalescing pass is implemented in the following files:

* `.../Vectorize/Intel_LoadCoalescing.h <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/include/llvm/Transforms/Vectorize/Intel_LoadCoalescing.h>`_
* `llvm/lib/Transforms/Vectorize/Intel_LoadCoalescing.cpp <https://github.com/intel-restricted/applications.compilers.llvm-project/blob/xmain/llvm/lib/Transforms/Vectorize/Intel_LoadCoalescing.cpp>`_
