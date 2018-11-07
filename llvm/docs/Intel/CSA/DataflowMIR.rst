========================================
CSA Dataflow Machine Code Representation
========================================

**Intel Top Secret**

.. contents::
  :local:

.. toctree::
  :hidden:

Introduction
============

The CSA backend uses normal control flow based Machine IR for its early passes,
but control flow Machine IR is not ideal for representing the results of
dataflow conversion or for use in dataflow-specific optimizations that run
afterwards. Because of this, the CSA backend uses special Machine IR with
dataflow semantics rather than control flow ones starting with :doc:`the
dataflow conversion pass <DataflowConversion>`. This page describes that variant
of Machine IR and the CSADataflowVerifier_ pass that checks that it is used
correctly.

Structure
=========

Operations
----------

CSA operations are represented by MachineInstrs as they are in "normal" control
flow Machine IR. Unlike in control flow, there is no real instruction ordering
besides ordering implied by operands. The CFG is generally merged into one large
basic block that contains all of the instructions. We generally try to keep defs
before uses where possible, but this is just for readability.

Currently, each MachineInstr has a NonSequential flag which determines whether
it is treated as a dataflow instruction (if the flag is set) or as a control
flow one (if it is not). This is mostly a holdover from the early days of the
compiler before the SXU was defeatured in order to facilitate moving hot code
incrementally off of the SXU onto dataflow units. Now, sequential instructions
are mostly only used for SXU-based function calls before conversion to dataflow
calls in CSAProcedureCalls. In the near future, this flag is expected to be
removed in favor of a subtarget feature that can control dataflow/sequential
compilation at the function level rather than the instruction one.

Dataflow Machine IR currently sometimes claims to be "SSA" even though it
clearly isn't in order to make register allocation happy. In the near future
this will not be necessary because register allocation will no longer need to be
run on dataflow code.

LICs
----

LICs are represented as virtual registers that are treated as channels rather
than actual registers. Their semantics are based on CSA assembly:

* LICs must have exactly one producer (non-INIT def): lics with multiple
  producers are not well-defined and lics with no producers are effectively
  equivalent to %na (and %na should just be used for this purpose instead).
* At some point there may be an additional requirement that every lic's width
  must match the width of its producer in order to simplify the late tools. This
  is not yet a requirement of CSA assembly or in the compiler, but may be in the
  future. A preliminary option ``-csa-check-def-widths`` is available to see
  where this condition is violated.
* LICs may have multiple consumers (uses): each use will see a copy of every
  value produced at the def. LICs with no uses are not allowed because they
  can't be used for anything besides eventually stalling the graph when they
  fill up.
* Initial values for lics are currently represented as special INIT instructions
  in Machine IR which are emitted as .value assembly directives. These
  instructions don't count as real producers and there can be any number of them
  added for a lic. The order of initial lic values is determined based on the
  order of INIT instructions, which will be preserved because the INIT
  instruction is marked as having side-effects. We might be investigating more
  natural ways of representing initial values in the future, though.

  The depth of each lic generally needs to be explicitly specified to be deep
  enough to hold all of the initial values for the lic, but CSADataflowVerifier_
  automatically increases the depth if it finds a lic that is not deep enough
  for its initial values.

There are several properties of lics that do not generally exist for virtual
registers, including depth and lic attributes. These are kept in tables in
CSAMachineFunctionInfo instead and can be queried/updated using methods of that
class.

.. _CSADataflowVerifier:

CSADataflowVerifier
===================

The CSADataflowVerifier pass serves as a dataflow analog of MachineVerifierPass.
Currently, it just checks the lic conditions above and adjusts the lic depth if
needed to hold initial values.
