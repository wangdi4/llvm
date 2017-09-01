===========================================
Xmain Opt Level Pass
===========================================

.. contents::
      :local:

Introduction
============

The opt level available in PassManagerBuilder can only be passed to
transformation and immutable passes which are added explicitly to the pass
pipeline. It is unavailable to analysis passes which are invoked implicitly by
the Pass Manager. This pass stores the opt level to make it available to all
LLVM IR passes. This requirement was driven by LoopOpt framework's need to
adjust its cost model (to save compile time) based on opt level information.

