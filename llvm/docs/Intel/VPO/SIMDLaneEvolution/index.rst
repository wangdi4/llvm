===========================
SIMD Lane Evolution for AVR
===========================

.. contents::
   :local:

--------------------------
High level design document
--------------------------

:Author: Gil Rapaport, Ayal Zaks
:Date: 2015-12

Overview
========

WIAnalysis (WIA for short) is a data & control flow analysis developed
for the OpenCL vectorizer whole-function vectorizer that determines
the relative behavior of the lanes in the SIMD execution of a program.
This document transfers this technology into the Xmain vectorizer.

Definitions
===========

We assume the code for analyzing is composed of a set of ``Instructions``,
where each Instruction represents a primitive operation (e.g., an addition,
a store) with an optional LHS ``Def`` and optional RHS ``Use`` operands.

Given an instruction I, the term ``Influence Region of I`` refers to the set of
all instructions that can be executed after instruction I and before the
immediate post-dominator instruction of I.

Motivation
==========

The analysis' goal is twofold:

1. SIMD execution of a program depends on the ability to discover
   control divergence. Diverging control flow, where not all current
   lanes agree on the
   next instruction to execute, is supported on CPU by sequentially executing
   every alternative path (until the diverging paths converge) where each
   instruction is predicated to only execute the lanes that opted for it.

   a. If divergent control flow is wrongfully considered non-divergent,
      program correctness is compromised.

   b. If non-divergent control flow is conservatively considered divergent,
      program performance may suffer.

2. Support further characterization of data divergence to enable optimization,
   e.g., characterizing consecutive (and strided) memory accesses can help fold
   gathers and scatters into wide vector loads and stores (and shuffles).

Data Divergence
===============

Data divergence in SIMD code occurs when a Def may set distinct values for the
current lanes executing the Instruction.
We call such a Def 'divergent', as opposed to 'uniform'.

**Divergence origin**: 
  Some Defs may be predefined as divergent.

  Typical sources of predefined divergent Defs include:

  * the induction variable of a vectorized loop
  * function calls pre-defined as returning divergent data (as in OpenCL)
  * function arguments defined as being divergent (as in OpenMP)

  The divergence in SIMD code stems from divergence origin in a data-flow manner.
  Hence, if no Defs are predefined as being divergent, all Defs are uniform.

A Def is divergent if it is predefined as such, or if it
belongs to an instruction that has
a divergent Use. A Use is divergent if it is fed by a divergent Def, or if it
is affected by divergent control flow. In the latter case, the Use may be
fed by two distinct Defs, or by one Def coming from different loop iterations.
Note that in this latter case, the Def(s) feeding the Use may be non-divergent.
A divergent Use is said to be "tainted" by a divergent Def or by the
Divergent Instruction of the control flow (defined below).

Control Divergence
==================

Control divergence occurs when the current
SIMD lanes executing an Instruction with
multiple successors disagree on the next Instruction to execute. This
happens whenever selecting the next Instruction depends on divergent data.
In such cases the Instruction is called a ``Divergent Instruction``.

Control divergence may in turn cause further data divergence of a Use,
by feeding it with different Defs reaching it from distinct
divergent control paths. We will use the following definitions to define those
cases:

Given some Divergent Instruction :math:`A`, let:

  :math:`T, F \in \text{succ}(A)` be two distinct successors of :math:`A`

  :math:`D \in \text{IR}(A)` be a Def in :math:`A`'s Influence Region.

  :math:`U \in \text{uses}(D)` be a Use of :math:`D`

Divergence due to 2 Defs
------------------------

A Divergent Instruction can generate data
divergence when a Use mixes a Def from inside its Influence Region with
a second reaching Def. The second Def can either also belong to the
Influence Region, or not. This holds whether the Divergent Instruction
belongs to a loop or not. In fact, if the Divergent Instruction does not belong
to any loop, this is the only way for it to cause divergence of a Use.

The following figure shows a few common cases of control divergence in DAGs
(nodes are Instructions, Divergent Instructions are marked in purple 
and their Influence Regions
are marked in red).

.. graphviz:: divergence.dot

[Divergent control flow with tainted and non-tainted Uses. Note the Defs of
``x`` reaching its Use in each case: a Def occuring within the Influence
Region will taint the Use (I, III) unless it is the only reaching Def (II)
or the immediate dominator of all reaching Defs is strictly dominated by
the divergent Instruction (IV).]

.. Divergent control flow with tainted and non-tainted Uses. Note the Defs of
  ``x`` reaching its Use in each case: a Def occuring within the Influence
  Region will taint the Use (I, III) unless it is the only reaching Def (II)
  or the immediate dominator of all reaching Defs is strictly dominated by
  the divergent Instruction (IV).

Formally, :math:`D` taints :math:`U` if there exist :math:`D^\prime \neq D`,
:math:`D^\prime \in \text{defs}(U)` a second Def reaching :math:`U` and
two paths :math:`P` and :math:`P^\prime` reaching :math:`U` diverging through
:math:`A` such that
:math:`P \cap P^\prime = \{A, U\}`,
:math:`P = (A, T, \ldots, D, \ldots, U)`,
and either
:math:`P^\prime = (A, F, \ldots, D^\prime, \ldots, U)`
or
:math:`P^\prime = (D^\prime, \ldots, A, F, \ldots, U)`.

Divergence due to 1 Def
-----------------------

SIMD loops can generate data divergence even with a single, uniform Def, and
can do so in two different manners:

1) **Leaking iterations**

   When a SIMD loop allows different lanes to exit at different iterations,
   a single Def may apply at iteration ``i`` to fewer lanes than it did on
   iterations ``0..i-1``. If Def sets a different value on different iterations
   (even if each of those values is itself uniform) it overrides the old
   value only for the subset of current lanes. This scenario can only taint
   Uses outside the loop, because Uses inside the loop operate on current lanes
   only.

   Formally, a Def :math:`D` in the Influence Region of a divergent Instruction
   :math:`A` taints :math:`U` due to loop :math:`L` if all following conditions
   hold:

   * :math:`A, D \in L`
   * :math:`U \notin L`
   * there exists a simple path :math:`P = (T, \ldots, A, F, \ldots, U)`

   The following figure shows loops that 'leak iterations', i.e. allow
   iterations exit the loop at different iterations and their potential effect
   on Uses outside the loop.

   .. graphviz:: leaking_iterations.dot

   [Loops with a single def and leaking iterations (with tainting path, if one
   exists): (I) Use tainted outside IR; (II) Use tainted inside IR, outside
   loop; (III) Use not tainted since no leaking iteration can reach it.]

2) **Partially killing Defs**

   When a Def inside a loop lies within the Influence Region of a divergent
   Instruction,
   each lane can choose whether to execute the Def in each
   iteration or not, resulting in a partial kill of the Def by a previous
   iteration. This scenario can taint Uses inside and outside the loop.

   Formally, a Def :math:`D` in the Influence Region of a divergent Instruction
   :math:`A` taints :math:`U` due to loop :math:`L` if all following conditions

   * :math:`A, D \in L`

   * :math:`U` is not dominated by :math:`D`

   * there exists a simple path :math:`P = (F, \ldots, A, T, D)`

   The following figure shows loops that allow an iteration to override a
   Def by a previous iteration and their potential effect on Uses inside the
   loop.

   .. graphviz:: partially_killing_defs.dot

   [Loops with a partially killing single Def: (I) diverging Instruction is on
   the loop; (II) diverging instruction and IR encapsulated inside the loop;
   (III) Use dominated by Def is not tainted by its partially killed
   definition.]

The common effect of both scenarios is that a SIMD loop may execute
a Def for a subset of the lanes, keeping other lanes with an old value set by
Def at an earlier iteration.

Note that when the control flow within a SIMD loop is completely uniform it
behaves just like a scalar loop, i.e. each iteration either kills or keeps any
Def executed by previous iterations for all lanes reaching the loop.

Handling Undefs
-----------------

We say a Use :math:`U` *has a reaching Undef* when there exists a simple path
:math:`P` from the entry node :math:`E` to :math:`U` that does not contain any
Def of :math:`U`.

Reaching Undefs are the root cause of Uses tainted by a single-Def. Since those
effects are already taken into account by the cases mentioned above we can
safely ignore Reaching Undefs by treating them as usual "don't-care" values:
any lane reaching the Use through no reaching Def at runtime may take any value
and specifically the value of one of the other lanes (which is the value of all
other lanes in case of a uniform Def). In other words, except for the cases we
handled above a Reaching Undef does not cause further divergence on a Use.

Blending of the same value
--------------------------

Blending of Defs may be of no consequence to the affected Use if it so happens
that the blending Defs were of the same value. While we expect many such
cases to be eliminated by optimizations running prior to vectorization,
this is not a guarantee. These optimizations involve
code hoisting and sinking, e.g., LLVM's
LICM, SimplifyCFG, Combine redundant instructions.

Tainting any Use can have a profound effect on performance, so we would like
to refrain from needlessly tainting such Uses.
One particular such case is a Use tainted by a uniform
single Def in a loop (either by leaking-iterations or a partially-killing-def
scenario). Recall that the uniformity of the Def does not prevent the Use from
being
fed different values since each iteration can Def a different uniform value;
However, if all iterations do infact Def the same value, each lane would end up
with that value (or be Undef), and thereby remain uniform.

For example, a Def that records its current iteration may taint a Use outside its
loop, or even some Uses inside its loop. However, a loop-invariant Def cannot taint
any Use. Two Defs that are effectively the same (could be combined), cannot taint a
Use even if they feed it via two disjoint divergent paths.

Dependencies
============

The operations we need to perform on Instructions are:

* **Def-Use chains (DU)**

  Given a Def, iterate over all the Uses it reaches.
  This is needed in order to propagate divergence properties (e.g., if a Def
  is Random so are all its Uses).

  Given a Use, iterate over all the Defs that reach it.
  This is needed in order to identify tainted Uses, that are reached from a
  Def in an Influence Region.

* **Dominance information (DOM)**

  Given an Instruction I, find the Instruction that immediately
  post-dominates I. This is needed for Instructions that 
  are found to be non-uniform.
  
  Given an Instruction I and a set of Defs, find the immediate common dominator
  D of the Defs, and check if D is strictly dominated by I.
  This is needed in order to check if a possibly tainted Use is
  shielded from the divergent control-flow source.

* **Control flow graph (CFG)**

  Given an Instruction, iterate over the subsequent instructions (successors).
  In particular, an instruction having more than one successor corresponds to
  a conditional branch in LLVM-IR, or an ``AVR-EXPR`` feeding an IF or LOOP.
  This is needed in order to identify all Defs that reside inside an
  Influence Region.

  Note: alternatively, one can recognize all Defs that reside inside the
  influence region of instruction I, (by elimination,) by checking for Defs
  that are dominated by I and post-dominated by the immediate post-dominator of
  I, provided I dominates its post-dominator (or side-entries are excluded when
  computing dominance).

  Note: one can conservatively (and iteratively)
  take the dominator of I's immediate
  post-dominator as a substitute of I, etc., to effectively turn the influence
  region into a single-entry-single-exit region. Thereby dominance information
  alone will suffice to represent the influence region.

Algorithm
=========

.. code-block:: python

  SIMDLaneEvolution = {} # Instruction -> UNIFORM/STRIDED/RANDOM
  SIMDLaneStride = {} # Instruction -> int
  CFG = getControlFlowGraph() # instruction-level CFG interface
  DU = getDefUse() # def-use information interface
  DOM = getDominance() # dominance information interface

  # Predefine the SLEV of each def occuring outside of and used
  # within the analyzed region.
  def predefineExternalDefs(evolution, stride):
    SIMDLaneEvolution = evolution
    SIMDLaneStride = stride

  # Analyze a single-entry-single-exit region designated by entry
  # instruction E.
  def analyzeSIMDLaneEvolution(E):
    Worklist = [] # Remaining instructions to classify
    for I in CFG.iterate(E):
      push(Worklist, I)
    while not empty(Worklist):
      I = pop(Worklist)
      (C, S) = calcSIMDLaneEvolution(I)
      setSIMDLaneEvolution(I, C, S)
      if len(CFG.successors(I)) > 1 and SIMDLaneEvolution[I] != UNIFORM:
        handleControlDivergence(I)

  # Calculate current state of evolution based on this
  # instructions's semantics (opcode and operands, which
  # are uses)
  def calcSIMDLaneEvolution(I):
    C = BOTTOM
    S = None
    # ... instruction specific code, propagating from I's operands ...
    return (C, S)

  # Set SIMD lane evolution C with stride S to def D
  def setSIMDLaneEvolution(D, C, S = None):
    if (D not in SIMDLaneEvolution or
        C != SIMDLaneEvolution[D] or
        C == STRIDED and S != SIMDLaneStride[D]):
      SIMDLaneEvolution[D] = C
      if C == STRIDED:
        SIMDLaneStride[D] = S
      push(Worklist, uses(D))

  # Compute the set of instructions influenced by a
  # diverging instruction I.
  def influenceRegion(I):
    IPD = DOM.immediatePostdominator(I)
    return [X | for X in CFG.iterate(I)
                if CFG.simplePathExists([D, X, IPD]) or
                   CFG.simplePathExists([D, X, D])]

  # Transform control divergence of a diverging instruction
  # I to data divergence by tainting every use U of any def
  # D residing in I's influence region, unless the immediate
  # dominator of all defs reaching U is dominated by I.
  def handleControlDivergence(I):
      for D in influenceRegion(I):
        for U in DU.uses(D):
          RDS = DU.reachingDefinitions(U)
          if len(RDS) > 1:
            # Do the (multiple) reaching Defs taint U together?
            IdomRDS = DOM.immediateDominator(RDS)
            if not DOM.strictlyDominates(I, IdomRDS):
              setSIMDLaneEvolution(U, RANDOM)
              return
          # Does any of the reaching Defs taint U single-handedly?
          for L in CFG.getLoops():
            if I in L and D in L:
              # Check if L leaks iterations en route to U
              if U not in L:
                if CFG.simplePathExists([F, D, I, T, U]):
                  setSIMDLaneEvolution(U, RANDOM)
                  return
              # Check if D is a partially-killing Def affecting U
              if not (U in L and DOM.Dominates(D, U)):
                if CFG.simplePathExists([F, I, T, D]):
                  setSIMDLaneEvolution(U, RANDOM)
                  return

Implementing for AVR
====================

Requirements
------------

An AVR program is a tree of nodes representing both instructions and control
structures/flow. There is no basic block concept. Control flow is either
implied by the node type (e.g. ``AVR-IF``) or expressed explicitly with an
``AVR-BRANCH``/``AVR-LABEL`` pair of statements.

The higher-level AVR constructs (if-then-else and loop) support unstructured
control flow. Short-circuit conditions, for example, translate into a nested
if-then-else construct with a forward-branch from the inner else to the outer
one. Furthermore, the higher-level structures may not even get constructed for
some LLVM IR programs.

[add C code]

::

  AVR_IF: if (%a > 7)
    AVR_IF: if (%b > 3)
      AVR_ASSIGN:<12>   | %div = %a / 7;
      AVR_ASSIGN:<13>   | %sum.02 = %sum.02 + %div;
    }
    ELSE {
      AVR_FBRANCH:<10>  | goto if.else;
    }
  }
  ELSE {
    AVR_LABEL:<23>      | if.else:
    AVR_ASSIGN:<26>     | %sum.02 = %sum.02 + ((-3 + %b) * %a);
  }

The analysis is therefore still required to support unstructured control
flow in general.

Mapping the algorithm to AVR
----------------------------

In ``AVR``, an ``Instruction`` is any ``AVR-EXPR``, ``AVR-ASSIGN``,
``AVR-PHI``, ``AVR-CALL`` and ``AVR-COMPARE``. The CFG edges reflect the
implicit and explicit control flow induced by ``AVR-IF``, ``AVR-LOOP`` and
``AVR-FBRANCH/AVR-LABEL``.
For instance, an ``AVR-IF`` node is seen by the CFG
as an instruction (the condition) succeeded by the first ``AVR`` node in the
'then' block and by the first ``AVR`` node in the 'else' block.

This is illustrated in the following figure showing the CFG of the ``AVR``
short-circuit if-then-else from the previous section.

.. graphviz:: short_circuit.dot

[Short-circuit if-then-else as a CFG. The edges replace both the AVR control
instructions (``AVR-FBRANCH``, ``AVR-LABEL``) and the AVR control structures
(``AVR-IF``, ``AVR-LOOP`` etc.).]

SSA vs. Variables
-----------------

AVR trees may be based on SSA (if built from LLVM IR) or on SCC variables (if
built from HIR). Since the algorithm relies on def-use chains it supports both
cases (where an ``AVR-PHI`` node is a Def and a Use).

API
---

The analysis provides the following public API:

.. code-block:: python

  def getSLEV(I):
    if D not in SIMDLaneEvolution:
      return BOTTOM
    return SIMDLaneEvolution[I]

  def getStride(I):
    assert(getSLEV(I) in [UNIFORM, STRIDED])
    return SIMDLaneStride[I]

  def isDivergent(I):
    return getSLEV(I) != UNIFORM and CFG.getSuccessorsNumber() > 1

APPENDIX
========

* A def reaching a use from outside influence region should taint it if
  it can pass through I, but need not taint it if it cannot pass through
  I. In the latter case, undef may reach the use. We are conservative in
  this case. Note that a use can have undef's reaching it, where we will
  state that it is not divergent; e.g., if it has real defs reaching it
  from within influence region that are shielded by an internal common
  dominator.

  .. graphviz:: esoteric_cases.dot

  [A few esoteric cases (diverging instructions and tainted uses are red):
  (I) ``PHI`` nodes ``x3``, ``x4`` within IR are not tainted (``x3`` does not
  depend on the diverging instruction - all lanes reaching it will be
  uniformly ``x1`` or ``x2``);
  (II) A partial Def is considered uniform since all other lanes are
  ``undef``;
  (III) A Def within the IR taints Use by partly overriding another
  reaching Def;
  (IV) A Def in IR does not taint Uses since it does not override the Def
  outside the IR.]

* An irreducible loop may result in an influence region having multiple
  entries, all of which can look like "side entries". That is, the
  divergent branch itself will not be an entry block.

  .. graphviz:: esoteric_cases.dot

  [Divergent instructions that are not entry blocks of their Influence Region:
  (I) a divergent latch of a multi-exit loop; (II) an irreducible loop with a
  divergent instruction.]

* Revisit superrel compound conditions.

* Revisit "am I being bypassed?" - resurrect uniform branches that have
  been included in SESE IR, by generating zero-bypasses, noting that
  their "else" clause is all-ones.

  Usecase: optimize masked vector load with uniform address.

  Usecase: optimize masked vector store with uniform address and uniform value.

