================================================
Scalar/Vector Nature Analysis for VPInstructions
================================================

Purpose of Analysis
===================

The analysis is supposed to identify possible cases for using scalar
instructions instead of vector ones inside vectorized context. Usually, vector
instructions are "heavier" and thus less effective than scalar ones. Especially
when vector value can not be used in an instruction directly and we need to
extract scalar value from the vector (e.g. index or pointer operand in a
unit-stride memory access). Also, calculations of some uniform values that are
used as vectors very often can be done in a chain of scalar instructions with
only broadcasting the last value.
The analysis is not supposed to replace DA and is built on top of it.


Terms in Analysis
=================

1. ``Vector`` - A value that is used as vector in vectorized loop.

2. ``FirstScalar`` - A value from the first active vector lane that is used as
   a scalar inside vectorized loop. For example, index or pointer operand of
   unit-stride accesses, uniform/linear parameter of a call, and possibly any
   value that is used to calculate another scalar or uniform value.

3. ``LastScalar`` - A value from the last active vector lane that is used as a
   scalar inside vectorized loop. For example, divergent value being stored to
   a uniform memory location.


.. note:: In general, the terms ``FirstScalar`` and ``LastScalar`` do not imply
   uniformity (e.g. loop IV) but rather the scenario that the value should
   be kept as a scalar (unwidened) variable and calculated using scalar
   instructions.

.. note:: ``Scalar`` is very often confused with ``Serialized``. ``Serialized``
   refers to a part of vector value (that might be divergent) that is extracted
   to use during serialization of some instructions (e.g. calls of functions
   w/o vector variants, masked inserts/extracts etc).

.. note:: A VPInstruction can be both ``{First|Last}Scalar`` and ``Vector``. For
   example, a pointer can be used in a unit-stride store as the store address
   and as the stored value.


Results Representation
======================

In general SVA is meant to determine how a given instruction in incoming scalar
code evolves in outgoing vector code, hence the analysis primarily works on
VPInstructions. In order to provide more accurate results about this, SVA
records the natures at both defintion and use points of a VPInstruction. In
general, this is how a VPInstruction would be marked by SVA -

[RetBits, InstBits] LHS = Opcode [OpBits] Operand1, [OpBits] Operand2

``Instruction-level SVABits``
  This represents the overall required nature of the instruction in outgoing
  code.

``Operand-level SVABits``
  This represents the required nature of an operand being used in a given
  instruction.

``Return-value SVABits``
  This represents the required nature of return value of an instruction.

The additional operand level mapping introduces the concept of edges between
instructions and their users across the VPlan CFG. These edges in-turn help us
to determine implicit patterns like broadcasts and extracts. For example,

1. An instruction that is ``FirstScalar`` at definition, having a use marked as
   ``Vector`` implies that it needs to be broadcasted.
2. An instruction that is ``Vector`` at definition, having a use marked as
   ``FirstScalar`` or ``LastScalar`` implies that it needs to be extracted
   from.

.. note:: We cannot represent broadcast/extract sequences explicitly in VPlan
   CFG since the results of SVA analysis are VF dependent and we would need
   different VPlan CFGs for different VFs. Furthermore we don't have explicit
   vector type in VPlan to accurately model these sequences.

.. note:: In most cases return value's nature would match the instruction's
   nature. Exceptions to this include special intrinsics that return scalar
   uniform value after vectorization, for example subgroup reduce intrinsics,
   LLVM vector reduce intrinsics.

.. note:: When an instruction has multiple bits set it implies that the given
   instruction will be lowered into multiple versions in the outgoing vector
   code. For example -

   [FScalVec] %gep = getelementptr [FScalVec] %addr, [FScalVec] %iv
        [Vec] %load = load [FScal] %gep
        [Vec] store [Vec] %gep, [Vec] %ptr

   This will be lowered in vector code as -
   %scal.gep = getelementptr %addr, %scal.iv
   %vec.gep = getelementptr %addr.bcast, %vec.iv
   %wide.load = load %scal.gep
   scatter(%vec.gep, %vec.ptr)


Analysis Results Usage
======================

The results of this analysis should not be used instead of DA results.
The primary user of analysis results is code generator. If a VPInstruction is
marked as ``FirstScalar`` or ``LastScalar`` then code generator should generate
scalar code, and if a VPInstruction is marked as ``Vector`` then code generator
generates vector code. Cost model should accurately model these situations.
The optimal use of generated vector/scalar values in code generator is not
subject of this analysis.

Verification of results is done using the following rules:

.. note:: Call VPInstructions are verified in a special manner by accounting
   for vector variants and knowledge from TLI, intrinsics properties and
   function attributes. Also loop entity init/final instructions are verified
   in a special manner. Below rules are used for all other VPInstructions.


+----------------------+-------------+---------------+---------------+
|   InstBits\UseBits   |    Vector   |  FirstScalar  |   LastScalar  |
+----------------------+-------------+---------------+---------------+
|        Vector        |  Use vector | Needs extract | Needs extract |
+----------------------+-------------+---------------+---------------+
|      FirstScalar     | Needs bcast |   Use scalar  |  Not allowed  |
+----------------------+-------------+---------------+---------------+
|      LastScalar      | Needs bcast |  Not allowed  |   Use scalar  |
+----------------------+-------------+---------------+---------------+

.. note:: When an instruction has multiple bits set then we expect all its
   operands to have the same number of bits set since any mismatch or
   discrepancy means that instruction cannot be lowered into multiple versions
   as indicated by the bits.

Data Structure and Interfaces
=============================

The scalar/vector attributes of a VPInstruction is tracked via a table that
resides in the analysis. This table maps a VPInstruction to its set of last
computed attributes.

Bits in SVAKind
---------------

.. note:: ``SVAKind`` is used internally in the analysis to represent the
   attributes of a VPValue. It is not publicly visible or accessible.

1. ``SVAKind::Vector`` - Bit to represent vector attribute.
2. ``SVAKind::FirstScalar`` - Bit to represent scalar attribute for first
   active lane.
3. ``SVAKind::LastScalar`` - Bit to represent scalar attribute for last active
   lane.

Getter interfaces
-----------------

``instNeedsVectorCode(VPInstruction *)``
  Returns true if VPInstruction is known to be vector.

``instNeedsFirstScalarCode(VPInstruction *)``
  Returns true if VPInstruction is known to be first scalar.

``instNeedsLastScalarCode(VPInstruction *)``
  Returns true if VPInstruction is known to be last scalar.

``operandNeedsVectorCode(VPInstruction *, unsigned)``
  Returns true if operand (by index) of instruction is known to be vector.

``operandNeedsFirstScalarCode(VPInstruction *, unsigned)``
  Returns true if operand (by index) of instruction is known to be first
  scalar.

``operandNeedsLastScalarCode(VPInstruction *, unsigned)``
  Returns true if operand (by index) of instruction is known to be last scalar.

``instNeedsBroadcast(VPInstruction *)``
  Returns true if VPInstruction needs to be broadcasted. This is determined by
  examining all use-site operand bits for this instruction. If instruction has
  ``SVAKind::Vector`` then it will never be broadcasted.

``instNeedsExtractFromFirstActiveLane(VPInstruction *)``
  Returns true if VPInstruction needs an extract from first active lane. This
  is determined by examining all use-site operand bits for this instruction. If
  instruction has ``SVAKind::FirstScalar`` then it will not need an extract.

``instNeedsExtractFromLastActiveLane(VPInstruction *)``
  Returns true if VPInstruction needs an extract from last active lane. This is
  determined by examining all use-site operand bits for this instruction. If
  instruction has ``SVAKind::LastScalar`` then it will not need an extract.


Setter interfaces
-----------------

.. note:: Setter interfaces are currently not implemented since computation
   is local to analysis. They will be introduced in future if needed.


Other interfaces
----------------

``compute()``
  Central interface to run ScalVec analysis and compute scalar/vector
  attributes of VPInstructions in given VPlan.

``clear()``
  Clear the last computed results of analysis.


High-level Algorithm Description
================================

The analysis' computation algorithm performs a post-order traversal on the CFG
to visit all VPInstructions in reverse order. It then propagates the ScalVec
attributes of VPInstruction from their uses to the definitions (use->def
chains), using DA results and knowledge about instruction nature. For example,
special processing is performed for PHINodes, call instructions, memory
accesses, and certain loop entities related instructions. Note that propagation
is not done recursively (unless needed for specific PHIs), and hence the
algorithm is known to terminate in approximately linear time.

