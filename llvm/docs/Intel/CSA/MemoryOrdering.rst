=============================
CSA Memory Operation Ordering
=============================

**Intel Top Secret**

.. contents::
   :local:

.. toctree::
   :hidden:

Introduction
============

Unlike on "normal" architectures, instruction ordering on CSA is irrelevant and
every instruction runs all of the time whenever it has inputs. This feature is a
boon to performance, but there are many programs that do require that certain
operations happen in a certain order for correctness. CSA includes functionality
to support the enforcement of these ordering dependencies in the form of Spatial
Dependency Flows (SDF), which represents the dependencies via normal dataflow
channels ("ordering edges") connected to special operands on memory operations
that are used to exchange dependency tokens. For example, a portion of CSA
assembly containing a store that must be run after a load (so that it doesn't
interfere with the loaded value) may look like this::

  .lic .i0 load_done
  ld64 val0, addr0, load_done, %ign
  st64 addr1, val1, %ign, load_done

The ``load_done`` channel carries dependency tokens from the load to the store
so that each invocation of the store must wait for a token from the
corresponding invocation of the load to start.

In order to support programs with meaningful through-memory dependencies, the
compiler must be able to leverage SDF to express those dependencies in the
generated code. The way that it does that is with the Memory Operation Ordering
pass (often shortened to Memop Ordering or Memory Ordering), which runs just
before instruction selection to add ordering edges to memory operations that
need them so that the dependencies will be preserved through dataflow conversion
and the rest of the backend. This page describes Memory Operation Ordering and
the related infrastructure for supporting SDF in the compiler.

Representation of SDF Ordering Edges
====================================

In MachineIR
------------

Explicit Ordering Operands
~~~~~~~~~~~~~~~~~~~~~~~~~~

CSA machine instructions that need to be ordered are generally defined with
explicit ordering operands matching those from the architecture. By convention
(and a shortcut in the older MIR memory ordering pass) these are generally the
last use operand and the last def operand for each instruction. This might
change in the future so that the operand ordering in MIR more closely matches
the ordering on the actual assembly instructions.

SXU MOV0
~~~~~~~~

Though the SXU has been removed in v1, the initial call lowering is still SXU
based and so there are still places where synchronization between dataflow units
and the SXU are needed. The way that this is done is with MOV instructions to or
from an SXU register: MOV instructions *to* an SXU register block SXU control
flow until a value arrives on the incoming lic, and MOV instructions *from* an
SXU register emit a value to a lic every time that they are reached by SXU
control flow. The pre-dataflow SXU calling convention leverages these SXU MOVs
in order to ensure that function calls are ordered. Though any register can be
used for this purpose, there are specific registers that are used in order to
avoid issues with optimizations:

- Edges to dataflow units after function entries and continues are represented
  using a MOV from RA. RA is used here because it is known to be clobbered by
  function calls and so if a MOV from RA is put after a call in a loop LLVM
  won't try to hoist it out of the loop.
- Edges from dataflow units ahead of function calls are represented using a MOV
  to R59. R59 is used for this because it is not used to pass values and will
  generally not be used by functions, which is important because the MOV zeros
  it out every time that it runs. R59 is also added as an explicit operand to
  JSR instructions in order to make sure that the MOV isn't DCE'd.

In the near future the lowering for function calls will change so that the
dataflow calling convention will be used from the beginning; when that happens,
these SXU MOVs and the register assignments described here will no longer be
needed because the ordering operands on the dataflow entry/return/call/continue
instructions can be used directly instead. This early lowering is already
implemented for returns.

In IR
-----

Dependency tokens are represented in LLVM IR after the memory ordering pass runs
as ``i1`` values. Since LLVM doesn't natively represent ordering edges on its
instructions, there is a special set of intrinsics that are used to construct
ordering graphs.

``llvm.csa.inord`` and ``llvm.csa.outord``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Syntax:
^^^^^^^

.. code-block:: llvm

   declare void @llvm.csa.inord(i1)
   declare i1 @llvm.csa.outord()

Overview:
^^^^^^^^^

The inord and outord intrinsics are used to attach memory ordering edges to LLVM
IR instructions.

Arguments:
^^^^^^^^^^

The input to the inord intrinsic specifies the input ordering dependency token
that should be consumed by the instruction being ordered. The return value of
the outord intrinsic is the output ordering dependency token produced by the
instruction being ordered.

Semantics:
^^^^^^^^^^

The inord intrinsic indicates than an input ordering edge should be attached to
the following instruction and the outord intrinsic indicates that an output edge
should be attached to the previous instruction. They are almost always used in
pairs:

.. code-block:: llvm

   call void @llvm.csa.inord(i1 %inord)
   %val = load i64, i64* %addr
   %outord = call i1 @llvm.csa.outord()

``ret`` is the exception to this rule: since the return instruction only has an
input ordering edge, only the inord intrinsic is used when ordering returns:

.. code-block:: llvm

   call void @llvm.csa.inord(i1 %inord)
   ret i32 %result

CSA instruction selection folds inord and outord intrinsics into the
instructions that they are added around, so the first example above will look
like this in MIR after instruction selection has finished:

::

  <%val>:i64, <%outord>:i0 = LD64 <%addr>:i64, 3, <%inord>:i0

If something ends up going wrong with this inord/outord folding mechanism,
instructions that are missing their ordering edges may or may not consistently
produce noticeable errors in program code and can be very difficult to detect.
As a failsafe to avoid these kinds of situations, inord and outord intrinsics
are **mandatory** for orderable operations.  Instruction selection asserts if an
instruction that it thinks should have inord/outord intrinsics appears without
them, and any leftover inord/outord intrinsics that couldn't be paired with an
instruction also result in a fatal error because they are not selectable on
their own.

Besides call/return instructions, the set of instructions that have ordering
edges and therefore need inord/outord intrinsics is enumerated in a table in
CSATargetLowering::LowerMemop. There isn't a direct mapping from this to IR, so
the definitive source for this information in IR is
CSAMemopOrderingBase::needsOrderingEdges. The current set includes normal memory
operations (loads, stores, and atomics) except for loads from constant memory,
returns, and all calls except for calls to any intrinsic that is not the
prefetch intrinsic.

Even though inord and outord intrinsics must be added for these instructions,
there are sometimes cases where a particular ordering edge is not actually
needed. If this is the case for an output edge, just leave the outord
intrinsic's result unused and various DCE passes in the backend will eventually
replace the output operand with ``%ign``. If an input edge is not needed, it can
be set to a constant:

.. code-block:: llvm

   call void @llvm.csa.inord(i1 0)
   %val = load i64, i64* %addr
   %outord = call i1 @llvm.csa.outord()

This is automatically replaced with a semantically identical but more idiomatic
use of ``%ign`` during instruction selection::

  <%val>:i64, <%outord>:i0 = LD64 <%addr>:i64, 3, $ign

``llvm.csa.mementry```
~~~~~~~~~~~~~~~~~~~~~~

Syntax:
^^^^^^^

.. code-block:: llvm

   declare i1 @llvm.csa.mementry()

Overview:
^^^^^^^^^

The mementry intrinsic represents the function entry's ordering edge.

Semantics:
^^^^^^^^^^

The semantics of mementry are very similar to inord and outord except that
mementry attaches an output ordering edge to the function entry rather than to
any particular IR instruction. This is currently lowered to a copy from RA
during instruction selection, but will eventually be combined into the entry
instruction when the changes to enable early entry lowering are ready.

Earlier proposals for this set of intrinsics also included a similar memexit
intrinsic that attaches to a function exit. This was replaced by inord
intrinsics attached to returns.

``llvm.csa.all0``
~~~~~~~~~~~~~~~~~

Syntax:
^^^^^^^

.. code-block:: llvm

   declare i1 @llvm.csa.all0(...)

Overview:
^^^^^^^^^

The all0 intrinsic implements an N-ary dependency token merge.

Arguments:
^^^^^^^^^^

The all0 intrinsic accepts any number of dependency token (``i1``) inputs to
merge.

Semantics:
^^^^^^^^^^

The all0 intrinsic is the IR equivalent to the CSA all0 instruction and can be
used to specify a dependency on multiple input dependency tokens. For instance,
the all0 in this example ensures that the store is not run until both loads have
finished:

.. code-block:: llvm

   call void @llvm.csa.inord(i1 %inord)
   %val0 = load i64, i64* %addr0
   %l0 = call i1 @llvm.csa.outord()
   call void @llvm.csa.inord(i1 %inord)
   %val1 = load i64, i64* %addr1
   %l1 = call i1 @llvm.csa.outord()

   %loads_done = call i1 (...) @llvm.csa.all0(i1 %l0, i1 %l1)

   call void @llvm.csa.inord(i1 %loads_done)
   store i64 %val2, i64* %addr2
   %s0 = call i1 @llvm.csa.outord()

all0 intrinsics are lowered to trees of all0 instructions during instruction
selection.

External Ordering Edges
-----------------------

External ordering edges are not supported yet because it is not clear how they
interact with the calling conventions and because there aren't very many useful
things that can be done with them without true multi-tile modelling. When they
are supported, they will likely take the form of extra operands on MIR
instructions following the architecture definitions and a new @llvm.csa.xordout
intrinsic.

Common Memory Operation Ordering Code and Alternative Passes
============================================================

Besides the main memory ordering pass, there are a couple of more trivial memory
ordering passes available that can be used for testing/experimentation and as a
fallback for cases that the main pass can't handle. This section covers those
passes and the common code that those passes and the main memory ordering pass
share.

CSAMemopOrderingBase
--------------------

CSAMemopOrderingBase is a base class for memory ordering passes which implements
common memory ordering logic. This includes important operations that need to be
performed during memory ordering such as adding ordering edges to instructions
(``createOrderingEdges``) along with a skeleton implementation of the main logic
of the pass which carries out things that always need to be done during memory
ordering such as adding a mementry intrinsic and removing parallel
region/section intrinsics. Memory ordering passes that derive from this just
need to override the ``order`` virtual function to implement their particular
flavor of ordering and a couple of other FunctionPass virtual functions to
expose their pass name and get access to extra analyses.

This structure may change when we transition to the new pass manager because the
passes will no longer need to derive from FunctionPass. The memory ordering
passes will likely still derive from CSAMemopOrderingBase so that they can keep
sharing common memory ordering code.

CSALinearMemopOrdering
----------------------

CSALinearMemopOrdering is likely the most trivial ordering pass possible: it
orders every memory operation as strictly as possible so that each one is not
allowed to start until the previous one has finished. This gives terrible
performance, but it is still useful as a comparison point and fallback because
it is obviously correct. It can be run by passing
``-csa-memop-ordering-mode=linear``.

The way that this pass is implemented is very straightforward and accomplishes
the ordering in two steps:

1. Memory operations are linearly chained together within each basic block
   starting at an empty phi node.
2. The phi node inputs of each block are connected to the chain ends of the
   block's predecessors.

There is also a third step that cleans up phi nodes where the inputs from each
predecessor are the same; this used to be necessary to avoid a bug in dataflow
conversion, but now it is mostly done just to make the output of the pass less
ugly.

CSARaceModeMemopOrdering
------------------------

CSARaceModeMemopOrdering is the opposite of CSALinearMemopOrdering: instead of
ordering every operation as strictly as possible, it orders them as loosely as
it could possibly make sense to. Earlier versions of the compiler included a
flag which completely disabled the memory ordering pass and left both ordering
operands of each memory operation unconnected. This was sometimes helpful for
tracking down compiler crashes, but the produced code was often useless because
the function would exit after a handful of cycles since it did not wait for any
stores to complete. Race mode ordering implements something that is one step
short of this extreme lack of ordering by ordering memory operations only with
the start and end of the function. This keeps memory operations completely
unordered within the function but ensures that none of them starts before the
function does and that the function will not exit until they are all complete.
It can be run by passing ``-csa-memop-ordering-mode=race``.

Race mode ordering is not correct in general and often lives up to its name by
producing incorrect code that is prone to data races. As such, it is not useful
as a fallback pass and mostly exists for performing certain kinds of experiments
evaluating the performance impact of enforcing ordering dependencies.

The implementation of CSARaceModeMemopOrdering is similar to
CSALinearMemopOrdering but slightly more involved. It also has two main steps:

1. All of the non-return memory operations in each basic block have their input
   ordering edges attached to the mementry intrinsic at the beginning of the
   function and have their output ordering edges merged together with an all0.
   Return instructions are collected for the next step.
2. Each return instruction is ordered with every memory operation in the
   function (including the function entry). This has to be done along every path
   that the return instruction is reachable from, but there are a couple of
   properties that can be taken advantage of here:

   - Memops which dominate a point in the CFG can be connected directly there
     and will be accounted for on every path through that point.
   - Dataflow channels have an important property where they keep values in
     order and so can't produce a token for an iteration of a loop until they
     have already produced one for all of the previous iterations. This means
     that any basic blocks that are connected at one point along a path are
     automatically accounted for earlier on that path and can be ignored if they
     are encountered again.

   With these things in mind, this step is implemented as a recursive backwards
   traversal through the function. Starting with the return's basic block,
   memops from basic blocks that dominate the current basic block are connected
   directly and memops in predecessors to those basic blocks that don't dominate
   them are connected by recursing into the predecessors. During each recursive
   call, dominators are marked so that they won't be traversed again down the
   call stack. When each recursive call finishes the results are connected into
   the caller's basic block with phi nodes, and then when the initial call
   completes its result is connected directly to the return instruction.

CSAMemopOrdering
================

[ A section on the real memory ordering pass, to be written later ]

Ordering Rules
--------------

[ This is a section detailing ordering rules for each type of instruction, to be
filled out later ]

Prefetches
~~~~~~~~~~

Prefetches are not normal memory operations, and so they don't follow the normal
ordering rules that other operations use. While other operations are ordered to
ensure correct behavior, prefetches are mainly ordered to keep them from
outpacing their corresponding memory operations and kicking current data out of
the cache. Therefore, prefetch ordering follows five main rules:

1. Prefetches are not ordered ahead of any operation, and the output ordering
   edge is always unused as a result. This ensures that the prefetches won't
   explicitly throttle the performance of a loop; we still don't want the actual
   memory ops to get ahead of the prefetches, but that shouldn't happen in most
   cases since the prefetches are issued at a lower frequency than the memory
   ops.
2. Read prefetches (with <rw>=0) are only ordered with memory ops that may load
   and write prefetches (with <rw>=1) only with memory ops that may load or
   may store. This helps to better associate prefetches with their corresponding
   ops: it's generally not useful to use read prefetches with operations that
   perform stores since the store will end up going through the memory system
   anyway to get ownership of the line, but write prefetches may be used in
   loops with both loads and stores and so it makes sense to order them with
   loads as well. Read prefetches are also ordered with function calls even if
   they may store, because the store could be unrelated to the prefetch and
   there could be relevant loads. Neither type of prefetch is ordered with other
   prefetches, as a result of rule (1).
3. Alias analysis queries involving prefetch ops always use an arbitrarily large
   access size, even in straight-line code. This ensures that prefetches are
   ordered with the corresponding memory ops in the common case that the
   prefetch address has some offset from the memory op one so that they don't
   actually alias.
4. Prefetch ops are only ordered within the context of their deepest containing
   loop that also contains ops corresponding to their prefetch type
   (see rule (2)). If there is no such loop, prefetches are ordered within the
   context of the function (which does not include the mementry and return).
   Prefetches are generally used in such a way that the only relevant ops are
   ones in the same loop, and this rule helps capture this specificity. However,
   it is also sometimes useful to use a loop containing only prefetches to
   prefetch an entire block of memory, and that use case is also supported by
   this rule. Note that this rule will produce entirely unordered prefetches in
   cases where the relevant loop contains ops of the corresponding type that do
   not alias the prefetches according to rule (3); this is a trade-off for not
   having to define the relevant loop depending on what the results of alias
   analysis are and is expected to be a very rare case.
5. Parallel region/section intrinsics and loop metadata are ignored when
   ordering prefetch intrinsics. This enables prefetches to be throttled using
   ordering edges from ops in previous loop iterations even in loops that are
   marked as parallel where such loop-carried edges would normally be dropped,
   though it generally can't un-parallelize the loop since rule (1) ensures that
   prefetch ops can't appear in a dataflow cycle.

These rules produce the expected ordering edges for several common code
patterns. For instance:

.. code-block:: c

   #pragma omp parallel for num_threads(4)
   for (int i = 0; i < N; ++i) {
     if (!(i & 0x7)) __builtin_prefetch(A + i + 1024);
     const double x = A[i];
     // ...
   }

Rule (5) ensures that there is still an ordering edge throttling the prefetch
from the previous iteration's load, which would otherwise be removed because of
the parallel for directive. However, the prefetches are still not connected to
loads from the other SPMD workers thanks to rule (4). Note that the ordering
edge here from individual loads from A blocks streaming load generation; for a
strategy which is compatible with streaming loads, `Data-Gated Prefetches`_
should be used instead.

It's also possible to put the load ahead of the prefetch:

.. code-block:: c

   #pragma omp parallel for num_threads(4)
   for (int i = 0; i < N; ++i) {
     const double x = A[i];
     if (!(i & 0x7)) __builtin_prefetch(A + i + 1024);
     // ...
   }

This is possibly more unusual, but rule (3) ensures that the prefetch is
throttled by the load in the same loop iteration as people would expect rather
than the previous iteration.

These rules also support patterns where prefetch loops are used to prefetch an
entire block of memory:

.. code-block:: c

   #pragma omp parallel for
   for (int i = 0; i < N; i += M) {
     for (int j = i; j < i + M; j += 8) __builtin_prefetch(B + j, 1);
     // ...
     for (int k = i; k < i + M; ++k) B[k] = f(x);
   }

The important feature that enables this is the definition of the relevant loop
in rule (4) as the deepest loop containing both the prefetch and a relevant
memory op, not just the deepest loop containing the prefetch.

Data-Gated Prefetches
=====================

Along with normal prefetch operations, the compiler provides data-gated
prefetches that are triggered on the availability of particular data values
rather than ordering tokens in order to support element-wise prefetching for
streaming memory operations. This section describes the intrinsic used to
represent these.

``llvm.csa.gated.prefetch``
---------------------------

Syntax:
~~~~~~~

.. code-block:: llvm

   declare void @llvm.csa.gated.prefetch.<type>(<type> <gate>, i8* <address>, i32 <rw>, i32 <locality>)

Overview:
~~~~~~~~~

The data gated prefetch intrinsic is used to specify a prefetch operation
triggered by the availability of particular data values.

Arguments:
~~~~~~~~~~

The arguments of ``llvm.csa.gated.prefetch`` are identical to those of
``llvm.prefetch`` except that:

- The ``gate`` parameter has been added to the front of the list to specify the
  values used to control when the prefetch triggers.
- The ``cache type`` parameter has been removed since CSA lacks traditional
  instruction prefetching. This operation will always perform a data prefetch.

Semantics:
~~~~~~~~~~

This intrinsic issues a prefetch when the value of ``gate`` is available. It's
implemented via a semi-trivial expansion at the end of the memory ordering pass
that replaces this intrinsic with ``llvm.prefetch`` and attaches ``gate`` to its
input ordering edge by adapting it to ``i1``. For most types this is just done
with a combination of truncs, bitcasts, and ptrtoints:

.. code-block:: llvm

   call void @llvm.csa.gated.prefetch.v2f32(<2 x float> %gate, i8* %addr, i32 0, i32 3)

   ; v expands to v

   %cast = bitcast <2 x float> %gate to i64
   %trunc = trunc i64 %cast to i1
   call void @llvm.csa.inord(i1 %trunc)
   call void @llvm.prefetch(i8* %addr, i32 0, i32 3, i32 1)
   %outord = call i1 @llvm.csa.outord()

   ; v generates as v

   ; prefetch addr, %ign, gate, MEMLEVEL_T0

For aggregate types (arrays and structs), the elements are converted to ``i1``
individually and combined together with ``all0`` so that the prefetch is issued
when all of the elements are available:

.. code-block:: llvm

   call void @llvm.csa.gated.prefetch.sl_i1i32s({i1, i32} %gate, i8* %addr, i32 0, i32 3)

   ; v expands to v

   %v0 = extractvalue {i1, i32} %gate, 0
   %v1 = extractvalue {i1, i32} %gate, 1
   %trunc = trunc i32 %v1 to i1
   %all = call i1 (...) @llvm.csa.all0(i1 %v0, i1 %trunc)
   call void @llvm.csa.inord(i1 %all)
   call void @llvm.prefetch(i8* %addr, i32 0, i32 3, i32 1)
   %outord = call i1 @llvm.csa.outord()

   ; v generates as v

   ; .lic .i0 gate
   ; all0 gate, gate.0, gate.1
   ; prefetch addr, %ign, gate, MEMLEVEL_T0
