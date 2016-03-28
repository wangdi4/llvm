===============
AVR Predication
===============

--------------------------
High level design document
--------------------------

:Author: Gil Rapaport, Ayal Zaks
:Date: 2015-12

Overview
========

This document describes the predication process of AVR code as part of
the vectorization process.

Definitions
===========

We will use the terminology defined in the document
``SIMD Lane Evolution for AVR``.

Motivation
==========

Predication is the process of turning scalar code with diverging control flow
into SIMD code with uniform (or no) control flow. This process is required for
CPUs to run SIMD code, because their control flow remains scalar.
In contrast to GPUs that check divergence in HW at runtime and typically
reconverge at immediate post-dominators, for CPUs we handle divergence in SW
at compile time and reconverge at every merge point (aka SSA Phi. See
``SIMD Re-Convergence At Thread Frontiers``, Micro 2011).
The predication process consists of two principal transformations:

* **Masking**

  Since control flow may not diverge, the CPU must execute all instructions
  reached by at least one SIMD lane. When an instruction is relevant to only a
  subset of SIMD lanes, the other lanes should either not
  execute the instruction or execute it as a ``NOP``.
  To that end, the masking process embeds masks in the SIMD code which are
  used for three purposes:

  - Preventing execution of irrelevant lanes by instructions with side effects.
  - Blending together diverging Defs that feed a Use.
  - Turning all diverging control flow into uniform control flow
    (Linearization).

* **Linearization**

  Removing divergence from the control flow is done in one of two ways:

  (i) Replacing the branching control flow with a linear (or no) control flow that
      includes, informally, all instructions lying between the diverging
      instruction and its immediate post-dominator.
  (ii) Replacing the non-uniform condition controlling the flow with a uniform
       condition that applies if all lanes agree.

  Divergent ``while`` loops must be handled by (ii).
  This approach is also usable as an optional
  optimization to DAGs (see ``Bypasses`` below).

Dependencies
============

* **SIMD Lane Evolution (SLEV)**

Masking
=======
 
Conceptually, every Instruction in the AVR code being vectorized is executed
under a mask. We refer to these masks as *Instruction Masks*. In addition,
each edge in the CFG is associated with a mask representing the SIMD lanes
opting to flow along that edge. We refer to such masks as *Edge Masks*.

We denote the Instruction Mask of :math:`I` as :math:`\mu_I` and the Edge Mask
:math:`I \rightarrow J` as :math:`\mu_{I \rightarrow J}`. For a given
Instruction :math:`I` let

* :math:`P = \{P_1, \ldots, P_n\}` be the set of :math:`I`'s predecessors,
* :math:`S = \{S_1, \ldots, S_n\}` be the set of :math:`I`'s successors, and
* :math:`C = \{C_1, \ldots, C_n\}` be the respective set of (mutually
  exclusive) conditions for :math:`S`;

then the corresponding Instruction and Edge masks are given by:

.. math::

  \mu_I =
    \begin{cases}
      1 & \text{if } |P| = 0\\
      \mu_{P_1 \rightarrow I} \lor ... \lor \mu_{P_n \rightarrow I} &
        \text{ otherwise}
    \end{cases}

  \mu_{I \rightarrow S_k} = \mu_I \land C_k

Note that:

* in the common case of a structured if-then-else hammock,
  each mask is the bitwise complement of the other.

* Two Instructions :math:`I` and :math:`J` will have an identical Instruction
  Mask (denoted :math:`\mu_I \equiv \mu_J`) iff :math:`I` 
  dominates :math:`J` and :math:`J` postdominates
  :math:`I` (in other words, :math:`I` and :math:`J` form a
  Single-Entry-Single-Exit region in the CFG).

* All lanes are expected to be active at the entry block.
  If this is not the case, an entry mask is to be provided.


Both Instruction and Edge masks are dynamically computed at runtime whenever
control
reaches the relevant execution points. Masks are implemented as AVR
variables. They are defined by AVR computations in terms of ``AVR-EXPR``'s
and other masks.

We maintain in each AVR node :math:`N`
a pointer to the AVR node that is the mask
associated with :math:`N`.
Distinct AVR nodes can point to the same mask iff their
mask is identical by the definition above.

An AVR Instruction may have multiple (CFG) successors if it is the condition
on which control flow depends. Its condition semantics depends on the AVR
control structure applying it:

* For an ``AVR-IF`` the condition must be boolean.
* For an ``AVR-SWITCH`` the condition must be an integral number.

In the former case we can create the Edge Masks based on the condition. In
the latter, we need to create the explicit boolean expression capturing each
alternative successor.

Eager Def blending
------------------

Uses with multiple reaching Defs must have these Defs correctly blended 
per lane.

In SSA form, by definition, there is a :math:`\varphi` node wherever two or
more Defs conflict. In Def-Use terms :math:`\varphi` nodes are Uses that force
conflicting Defs to meet as soon as possible, and are therefore the only Uses
with multiple reaching Defs.

SSA form thus enables a straightforward approach to blending Defs at the Use,
where
:math:`\varphi` nodes (doing predecessor-based selection) are replaced by
``BLEND`` Instructions (doing predecessor-mask-based selection). Blending at
the Use facilitates keeping uniform Defs and Uses scalar, by limiting blending
to non-uniform Uses. This may help relieve vector register pressure, at the
expense of raising scalar register pressure.

In (non-SSA) Def-Use form,
however, such artificial Uses are not inserted at the first
locations where two or more Defs reach.  In other words, a Use is not
necessarily located where :math:`\varphi`'s are due. There is
therefore no direct relation between a Use's incoming Edge Masks and the
Instruction Masks of its reaching Defs, so blending based on the incoming Edge
Masks would be incorrect, as can be seen in the following figure:

.. graphviz:: ssa_vs_du.dot

[Example execution of four lanes through two diverging Instructions. In SSA
form (I) the :math:`\varphi` nodes are placed where the correct Def can be
selected at runtime according to the incoming edge. In Def-Use form (II)
the incoming edge alone may not carry sufficient information to do so (e.g.,
for lanes ``2`` and ``4``, both reaching through ``F`` but carrying
different values).]

Instead of blending at the Use, Def-Use variables can blend at each Def: since
each Def (being an Instruction) is executed under an Instruction mask, applying
this mask on the Def (e.g., an ``AVR-ASSIGN``) naturally means redefining the
active lanes with the new value while maintaining the existing value for the
inactive lanes.

.. graphviz:: eager_blending.dot

[Same Def-Use execution example masked under four possible linearizations. The
current value of variable ``X`` is noted on the edges.
Undef values are denoted by
:math:`\phi`.]

Note that blending at the Def naturally handles Def aggregation across loop
iterations such that each SIMD lane maintains the value defined by the last
iteration to go through the Def.

Algorithm
---------

The predication process proceeds as follows:

1) **Canonicalize divergent loops**

   A SIMD loop must execute until every lane takes one of its exits.
   In addition, the next SIMD iteration cannot begin until all lanes
   complete the current iteration (i.e., until every Instruction with at least
   one bit set in its Instruction Mask executes). This follows the general
   model of reconverging on every merge point - including loop headers.

   Uniform SIMD loops (and, trivially, scalar loops) possess several properties
   that may not hold for divergent SIMD loops:

   * All lanes execute the same number of iterations.
   * All lanes exit through the same exit. I.e., if a lane decides to leave an
     iteration through some exit, all lanes do so.
   * No lane begins iteration :math:`i` before all lanes complete
     iteration :math:`i-1`.

   These properties are crucial for executing SIMD loops on CPUs, which execute
   all lanes on a single execution thread. Divergent loops must therefore be
   transformed into uniform loops that maintain the correct Instruction Mask for
   each lane inside the loop, as well as outside the loop - 
   until all lanes reach the loop header's immediate
   post-dominator. In other words, given an Instruction `I` that causes a loop
   to be divergent, the transformation applies to the Influence
   Region of `I` that contains the entire loop and possibly additional
   instructions following it until reaching the immediate post-dominator of `I`,
   which is also the immediate post-dominator of the loop header.

   This canonicalization step that transforms divergent loops into uniform loops
   can be done either as (A) an integral part of the masking process, which
   then takes
   care of all divergence cases - loops and non-loops, or (B) as a separate
   preparatory step before the masking process.
   In case (A), Edge Masks corresponding to edges that exit the loop will behave
   as an aggregation of multiple Edge Masks, corresponding to multiple
   iterations that may exit the loop through this edge.
   In case (B), the
   subsequent masking process is simplified as it handles only non-loop
   divergence; in particular, all Edge Masks behave the same - simply as a
   conjunction of their source Instruction and its condition.

   .. Note: the immediate post-dominator of every instruction belongs to the
      same loop.

   The separate canonicalization step is done as follow, given a divergent loop
   with potentially multiple latches and/or exits:

   * The loop is transformed to have a new single latch :math:`l` which
     coincides with a single exit. The successors of :math:`l` are the header
     of the loop and :math:`f` - the single successor of the loop.

     * The latch :math:`l` is based on a new
       boolean condition of the form ``all-zeros(v)`` where ``v`` is a newly
       introduced variable recording the currently iterating lanes, having
       the following Defs:

       * **at loop pre-header:** :math:`v = 1` under the pre-header's
         Instruction Mask.
         In AVR this instruction is introduced as a new
         predecessor of the ``AVR-LOOP`` construct, and inherits the
         Instruction Mask of the previous predecessor.

       * **at each loop exit**, :math:`v = 0` under the Edge Mask exiting the
         loop.
         This instruction executes on the path that remains inside the loop.

     * A new variable :math:`exit\_to` is created and a new `switch(exit\_to)`
       Instruction is introduced into :math:`f`.
       Note that if the original loop had a single exit or two exits, this
       `switch` Instruction can be optimized.

     * All edges from original latches to the header are redirected to
       :math:`l`. Any lane that wishes to continue to next iteration must do
       so together with other lanes having same wish.

     * All edges from original exits that leave the loop are redirected to
       :math:`l`. Any lane that wishes to exit the loop must wait until all
       other lanes have same wish.

   * Foreach exit :math:`j` from Instruction :math:`e_j` exiting the loop to
     Instruction :math:`b_j`:

     1. keep track of whether or not exit :math:`j`
        was to be taken, by introducing a :math:`exit\_to = j` Def at the loop
        exit.
        The Def executes on the path that remains inside the loop,
        under the Edge Mask exiting the loop.

     #. Introduce Instruction :math:`b_j` as the :math:`j`'th successor of the
        new `switch(exit\_to)` Instruction at :math:`f`, corresponding to
        `exit\_to == j` case.

     #. redirect the exit edge to the new single latch:
        replace :math:`b_j` with :math:`l` as a successor of :math:`e_j`.

   The following figure demonstrates this transformation:

   .. graphviz:: loop_canonicalization.dot

   [Loop canonicalization example: (I) a loop (denoted by the blue edges) with
   two exits (denoted by the green and purple paths) canonicalized to a
   single-latch-single-exit loop (II): original latch, ``break`` and
   ``continue`` rewired to the single latch with variables recording the
   original exit paths to be applied after the loop exits uniformly.]

#) **Canonicalize divergent control structures**

   Since CPUs do not support divergent control flow, we will need to remove
   such control flow from the AVR program. The final removal of such control
   flow will be done during linearization, yet at this point we replace any
   divergent AVR control structure with ``AVR-FBRANCH``/``AVR-LABEL`` pairs.
   This simplifies the masking process by expressing the control flow
   explicitly and uniformly.

   Consider for example ``AVR-SWITCH`` with its implicit equality cases *(and
   fallthrough behavior?)*: there are no ``AVR-EXPR``'s holding the Boolean
   comparison result for each case to serve the masking process *(and no
   ``AVR-FBRANCH``'s expressing break/fallthrough behavior?)*. This phase will
   create those boolean ``AVR-EXPR`` and implement that ``AVR-SWITCH`` using
   ``AVR-FBRANCH``/``AVR-LABEL`` pairs.

   After this phase, all divergent Instructions are Boolean and have 2
   successors.

#) **Mask creation and management**

   We begin by creating Masks wherever the control flow may diverge (i.e., at
   Instructions having multiple successors that are 
   declared non-uniform by ``SLEV``) in the following manner:

   1. Foreach non-uniform Instruction :math:`I` with multiple successors:

      1. Create :math:`\mu_I = 1` as an ``AVR-ASSIGN`` of 1 to a new variable
         and insert it as :math:`I`'s new predecessor.

      #. For successor :math:`I_T` create
         :math:`\mu_{I \rightarrow I_T} = \mu_I \wedge I` as an
         ``AVR-ASSIGN`` to a new variable and insert it as :math:`I`'s new
         successor.

      #. For successor :math:`I_F` create
         :math:`\mu_{I \rightarrow I_F} = \mu_I \wedge \lnot I` as an
         ``AVR-ASSIGN`` to a new variable and insert it as :math:`I`'s new
         successor.

   1. For each non-uniform Instruction :math:`I` with multiple predecessors
      :math:`{P_1, \ldots, P_n}`:

      1. Replace the RHS of :math:`\mu_I = 1` with a new ``AVR-EXPR`` to
         produce
         :math:`\mu_I = \mu_{P_1 \rightarrow I} \vee \mu_{P_2 \rightarrow I}
         \ldots \vee \mu_{P_n \rightarrow I}`

#) **Associate Instructions with dominating masks**

   1. For each Instruction :math:`I` with a single predecessor :math:`P`:

      1. If :math:`\mu_P` exists then :math:`\mu_I \equiv \mu_P`. This will
         be implemented by :math:`I`'s AVR node pointing to the same mask
         variable as :math:`P`.

Linearization
=============

Linearization can be done at different levels of granularity, ranging from
maximal linearization
removing all control flow (excluding loops) to minimal linearization striving
to retain control flow as much as possible
(as in Karrenberg's partial linearization via Rewire Targets analysis).

All levels produce correct code; the accuracy of the
process (i.e., avoiding linearization where possible) is a matter of
optimization. Since Xmain is required to vectorize at ``-O0`` configurations
the implementation should take debuggability and compile-time into account.

Uniform Single-Entry-Single-Exit regions
----------------------------------------

Note that even inside the Influence Region of a divergent Instruction, where all
instructions operate under masks, there may be
uniform control flow we could preserve.
We say that *An SESE region is uniform* if no Instruction in it excluding
possibly the exit Instruction is divergent.

In a uniform SESE region all Instructions share the same mask, i.e., each
Instruction in it is either executed by all lanes entering it or by none.
This allows us to maintain the control flow within this region and treat it
w.r.t. enclosing Influence Regions as a single Instruction.

Divergent Single-Entry-Single-Exit regions
------------------------------------------

We say that *An SESE region is divergent* if its entry Instruction is divergent.
We will call a divergent SESE region *simple* if the only divergent
Instruction in it is its entry Instruction, excluding possibly its exit
Instruction.

Once linearized, a divergent SESE region becomes a uniform chain of
Instructions. It can then be treated as a single Instruction w.r.t. an enclosing
uniform SESE region.

Alternating collapse of SESE regions
------------------------------------

A simple approach to maintaining the control flow of uniform SESE regions is
to iteratively collapse (maximal) SESE regions, as demonstrated in the
following figure:

.. graphviz:: alternating_seses.dot

[Alternating collapse of SESE regions: (I) The original CFG with the SESE
regions marked (divergent Instructions are red); (II) After collapsing the
innermost divergent regions; (III) After collapsing the outermost divegent
region.]

Algorithm
---------

The linearization algorithm is based on the alternating collapse approach
discussed above.

.. code-block:: python

  class SESERegion:
    def __init__(self, entry, uniform):
      self.entry = entry
      self.subRegions = []
      self.isUniform = uniform
      self.isSimple = True

  # Find the wrapping SESE region for a give Instruction
  # by iteratively jumping to the current postdom and
  # its idom until the reaching an Instruction J such that
  # J = idom(pdom(J)).
  def getContainingSESERegionEntry(I):
    J = CFG.getIDom(CFG.getPDom(I))
    while I != J:
      I = J
      J = CFG.getIDom(CFG.getPDom(I))
    return I

  # Identify all SESE regions in the CFG (including its
  # entry/exit pair) and construct a tree of SESE regions.
  # Mark a region uniform if (a) its entry is not divergent
  # and (b) any contained divergent Instruction is wrapped
  # by a sub SESE region.
  def computeSESERegions():
    regions = {}
    current = None
    for I in DFS(CFG):
      isUniform = not SLEV.isDivergent(I)
      successorsNum = len(CFG.getSuccessors())
      if current is not None:
        if not isUniform and I != current.exit:
          current.isSimple = False
      SESEEntry = getContainingSESERegion(I)
      if successorsNum > 1 and I == SESEEntry:
        # New SESE region identified
        newRegion = SESERegion(I, isUniform)
        current.subRegions.push(newRegion)
        current = newRegion
        regions[I] = newRegion
      elif not isUniform:
        # A non-SESE Influence Region detected. Find the
        # containing SESE region (we should have identified it
        # by now) and mark it (if it wasn't already) non-uniform.
        entry = getContainingSESERegionEntry()
        region = regions[entry]
        region.isUniform = False
    return regions

  def handleSESERegion(region)
    if region.isUniform:
      # Just handle all subregions recursively
      for subRegion in region.subRegions:
        handleSESERegion(subRegion)
    else:
      # First, handle all uniform subregions
      for subRegion in region.subRegions:
        if subRegion.isUniform:
          handleSESERegion(subRegion)
      # Now, linearize this region
      linearizeSESERegion(region.entry)

  # Linearize a SESE region which contains complete Influence
  # regions of at least one divergent Instruction (that is not
  # its exit Instruction). Treat contained uniform SESE regions
  # as a single uniform Instruction.
  def linearizeSESERegion(I):
    # TODO

  def linearize():
    SESERegions = computeSESERegions()
    handleSESERegion(SESERegions[CFG.getEntry()])

Future possible extensions
--------------------------

One alternative is to loosen the SESE requirement by considering 'side-entries'
into the Influence Regions of divergent Instructions. Since conceptually every
Instruction carries an execution mask an Instruction can continue into the
middle of a divergent region as long as all Uses of Edge Masks reachable
from the side entry are properly initialized. This would lead into linearized
Instruction chains connected by uniform control flow. It would also allow
taking advantage of disjoint masks by using :math:`\varphi` nodes to select
between them instead of blending operations.

.. graphviz:: side_entry.dot

[Example of an Influence Region with a side entry (I) with its enclosing SESE
region linearized (II) and with a side-entry into the linearized Influence
Region (III). Note the use of :math:`\varphi` nodes in (III) instead of the
blending ``OR`` in (II) compensating for possibly-Undef masks.]

Another alternative is to implement Karrenberg's Rewire Targets approach which
aims to provide highly-accurate linearization at the Instruction level. The
algorithm requires more study and adaptation to AVR.

Both approaches require more study in order to evaluate their development
effort, compile-time and run-time pros and cons compared to the current
approach.

Optimizations
=============

Masked uniform execution
------------------------

Uniform vector computations (where all operands are uniform) can be executed by
scalar registers and operations. This is also true for masked uniform values,
although care must taken to avoid side-effects if the mask is empty.

The benefit in performing uniform computations on scalar registers is program
dependant. In order to support it, an optimization pass will scan the
vectorized AVR in search of masked uniform computations that add up to regions
worth scalarizing. Such regions may have to be zero-bypassed in order to avoid
side-effects, so this analysis pass should be combined into the zero-bypass
placement heuristics.

To actually scalarize such a computation one must do the following for each
variable involved:

* extract the value from (one of) the active lanes into a new variable
  (which is of scalar type, as opposed to the vector variable).

* maintain any partial vector Def that has Uses outside the region being
  scalarized.

This process can be delayed until after bypasses are in place.

Unmasking Instructions with complementing masks
-----------------------------------------------

A set of Instructions :math:`\{I_1, \ldots, I_n\}` with a set of respective
disjoint Instruction Masks :math:`\{\mu_{I_1}, \ldots, \mu_{I_n}\}` can be
replaced by a single Instruction :math:`I` with an Instruction Mask
:math:`\mu_I = \mu_{I_1} \lor \ldots \lor \mu_{I_n}` if
:math:`I_1 = \ldots = I_n`.

Such sets of Instructions often exist in if-then-else constructs where an
identical value is computed in both the then and else blocks (and therefor with
complementing masks).

Combining such Instructions into a single one can replace multiple expensive
machine instructions with cheaper mask combining instructions. When the
combined mask already exists (as in the if-then-else case) there is no mask
combining cost at all. In particular, when the combined mask is known to be full
the mask can be dropped altogether from the Instruction.

Installing bypasses
-------------------

All-zeros bypasses
~~~~~~~~~~~~~~~~~~

This kind of bypass is a no-op runtime check that branches over a chain of
Instructions if no lane reaching the bypass requires their execution. All-zero
bypasses are placed where Instruction masks are generated (in other words,
where the execution mask is reduced to fewer lanes). These points in the
program correspond to divergent Instructions that were linearized.

All-ones bypasses
~~~~~~~~~~~~~~~~~

This kind of bypass is a form of code versioning. A runtime check of the mask
selects between executing a chain of masked Instructions or a parallel chain of
the same Instructions, unmasked.

Lowering masking information
============================

Masking information is currently used for preventing inactive lanes from
producing side-effects. Future platforms, however, are also expected to take
advantage of masking information to reduce power consumption. This, however,
requires masking to be applied to all predicated instructions, not just the
few that may have side-effects.

LLVM does not provide a way of attaching a mask to instructions. The
existing support for masks includes intrinsic functions for masked load/store
and (and gather/scatter).

Our goal is to propagate masking information from AVR to LLVM IR such that

a) it reaches code generation phase

b) it is generic; i.e., does not require an intrinsic function per LLVM
   instruction

b) it is safe w.r.t standard LLVM optimizations; i.e., prevents mask-crossing
   operations yet facilitates same-mask optimizations

We will encode masking information in the LLVM IR by passing their arguments
through a dedicated instrinsic function ``llvm.masked`` (we can skip this
marking for any argument whose Def was executed under the same mask). This new
instrinsic is defined polymorphically for any type ``T`` and natural number
``N`` as:

``<N x T> llvm.masked(<N x T>, <N x i1>)``

.. note:: Add reference to ``applymask`` 2008 discussion
