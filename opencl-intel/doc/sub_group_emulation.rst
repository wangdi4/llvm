==========================================
Implementation DOC for sub-group emulation
==========================================

.. contents::
   : local:

ResolveVariableTIDCall
----------------------
Eliminate all TID Calls with variable argument.
e.g.
.. code-block:: c++

  y = get_local_id(x);

should be transformed to:
.. code-block:: c++

  if (x == 0)
    y = get_local_id(0);
  else if (x == 1)
    y = get_local_id(1);
  else if (x == 2)
    y = get_local_id(2);
  else
    y = 0; // OpenCL SPEC says it should be 0


SetVectorizationFactor
-------------
Check whether current ``VF`` (From env var, ``intel_reqd_sub_group_size``,
``intel_vec_len_hint``, ``InstCounter`` Pass) can be satisfied.
1. Check if the VF is multi-constrained: It's unclear which constaint should
   be chosen if more than one constraint is specified.
2. Record inital VF according to given constraints. Initial means this VF may
   be falled back. Currently only intel_vec_len_hint can be falled back.
3. Check if there is any pattern can't be vectorized:
   Kernels with Sync calls OR TID calls in non-inlined functions
   OR Unsupported VecTypeHint can't be vectorized ATM.
4. Check horizontal operations: For sub-group and work-group built-ins, they
   can only work with some special VFs.
   For sub-group (1 (if emulation enabled), 4 ,8 ,16, 32, 64)
   For work-group (1, 4, 8, 16)
5. Check sub-group semantics: sub-group calls should always be vectorized,
   So if the parant function of sub-group call can't be vectorized for some
   reasons, then sub-group semantics is broken. In such cases, if sub-group
   emulation is enabled, we will record the SG Emulation Size.
6. Set up ``recommended_vector_length`` for VPO; Set up ``sg_emu_size``
   for sub-group emulation.

GroupBuiltin
------------
Run this pass first to insert work-group barriers for work-group built-ins.
Please refer to barrier doc.

BarrierInFunction
-----------------
Run this pass to propagate work-group barriers.
Please refer to barrier doc.

RemoveDuplicateBarrier
----------------------
Run this pass to remove duplicate work-group barriers.
Please refer to barrier doc.

SGCallLinearize
---------------
**Out Of Scope**
Since OpenCL allows non-uniform sub-group calls, the calls may be in divergent
path. In such cases, we can't just handle it as uniform calls because we
currenly treat all sub-group calls as implicit ``sub_group_barrier``, it's
illegal to put barrier calls into divergent path. If we want to do this later,
we may have to introduce some code-linearize logic OR sth like loop massaging
to encode the control flow to data flow.

SGBuiltin
---------
For all functions directly / indirectly called by kernel with ``sg_emu_size``
metadata, visit all instructions and Do:
1. Add **vector-variants** attribute for all sub-group calls (exclude those
   handled in ResolveSubGroupWICall Pass). This phase looks like what we do in
   OCLVecClone, but the main difference is the attribute is attached to function
   site (not call site), by this way, we can unify the widening logic for
   sub-group calls and other functions to be emulated.
2. Insert ``sub_group_barrier`` before and ``dummy_sg_barrier`` after sub-group
   calls.
3. Insert ``sub_group_barrier`` before and ``dummy_sg_barrier`` after work-group
   barrier calls; Insert ``dummy_sg_barrier`` after dummybarrier calls.

SGBarrierPropagate
------------------
For all functions calling ``dummy_sg_barrier`` (Now we can identify which
function should be emulated by checking whether it calls ``dummy_sg_barrier``
and later passes always operate on these functions)
Do:
1. Insert ``dummy_sg_barrier / sub_group_barrier`` at the begin / end of
   functions calling sub_group_barrier.
2. Insert ``dummy_sg_barrier / sub_group_barrier`` after / before calls to
   functions handled in step1.
Note: this procedure is executed iteratively.

SGBarrierSimplify
-----------------
1. Remove redundant ``sub_group_barrier / dummy_sg_barrier`` calls to avoid
   creating empty sub-group loop introduced by adjacent barrier calls.
2. Make sync inst become the first instruction of a BasicBlock. This can
   simplify later analysis, such as cross-barrier analysis.

SGValueWiden
------------
1. Widen the prototype for functions to be emulated except for kernels. This
   makes the function looks like being vectorized. All sub-group built-in
   declarations are also processed in this phase. This phase looks like
   VecClone, the main difference is we don't create the SIMD Loop here and
   just widen the parameters / return value and then update their uses / def.
2. Alloca an array / vector for non-uniform value crossed by sub_group_barrier.
3. Alloca a scalar counterpart for uniform value crossed by sub_group_barrier.
4  Replace original use with the value loaded from corresponding alloca
   instruction; Store the def to corresponding alloca instruction.
5. For widened functions, replace their orignal scalar calls with widened calls.

TODO List:
1. Complete the logic handling atrributes for vector paramters.
   Note: VecClone has many issues in this phase, it should be improved.
2. Improve WIRelatedAnalysis pass to make it suitable for sub-group emulation.
3. Improve the logic handling <VF x iN> while N is not power of 2.
4. Improve / Fix the logic handling <C x Ty> while C is not power of 2.
5. Fix debug info for parameters, llvm.dbg.value intrinsics.

SGLoopConstruct
---------------

1. Create loop from each ``sub_group_barrier`` to all ``sub_group_barrier``
   and ``dummy_sg_barrier`` calls which can **immediately** reach it.
   (**immediately** means there can't be any other ``sub_group_barrier`` or
   ``dummy_sg_barrier`` in the path and the ``sub_group_barrier`` call itself
   should not be taken into consideration either, just like idom)
2. Update TID calls.
.. code-block:: c++

  get_local(global)_id(EmuDim)

should be transformed to:

.. code-block:: c++

   get_local(global)_id(EmuDim) + get_sub_group_local_id()

3. For ``get_sub_group_local_id`` call in non-emulated functions, we have to
   hoist them to its ancestor emulated function and pass the call result to its
   original calling function.
4. Replace ``get_sub_group_local_id`` call with sub-group loop control variable.
5. Set the **vectorized_width** as **sg_emu_size**. Then the kernel pretends
   to be a vectorized kernel.

BarrierPass
-----------
Create barrier loop, calculate offest in special buffer and update the def and
use for values crossed by work-group barrier.
Please refer to barrier doc.
