==============================
VPlan Hierarchical CFG Builder
==============================

--------------------------
High level design document
--------------------------

:Author: Diego Caballero, ...
:Date: 2017-07

Overview
========

This document describes the process of building a hierarchical CFG for VPlan
using VPBasicBlock's, VPRegionBlock's and VPLoopRegion's.

Definitions
===========

We will use the terminology defined in the document ``VPlan``.

Motivation
==========

Dependencies
============

Main Algorithm
==============

VPlan Hierarchical CFG Builder builds the initial Hierarchical CFG (HCFG) for a
VPlan, given its incomming LLVM IR. VPBasicBlock's in the resulting HCFG may
not have a one-to-one correspondence with BasicBlock's from the incomming
IR. The algorithm proceeds as follows:

1) **Build Plain CFG**

   It builds a plain CFG from the incomming IR by creating
   VPBasicBlock's with VPOneByOneRecipe's (only VPVectorizeOneByOneRecipe's by
   now) and VPConditionBitRecipes. A VPRegionBlock, denoted Top Region, is
   created to enclose all the VPBasicBlock's of the plain CFG. Two dummy
   VPBasicBlock's are used as Top Region's Entry and Exit.

   **TODOs**
      * If incoming outermost loop has multiple exits, current implementation
        creates a dummy VPBasicBlock as landing pad of all loop exits. However,
        including in the CFG all basic blocks up to the common postdominator of
        all loop exits can be a better approach.
      
#) **Simplify Plain CFG**

   It applies transformations on plain CFG to make it suitable for construction
   of VPRegionBlock's (next step of the algorithm). These transformations
   include, in this particular order:

  1. **Loops pre-header massaging**: splits loop pre-header VPBasicBlock's when
     one of the following conditions is met:

     * Pre-header has multiple predecessors.
     * Pre-header is header of another loop.

  #. **Multiple-exit-to-single-exit loop massaging**: This transformation
     removes the side exits of the inner loop by doing the following:

     * For each exit block, a new intermediate basic block is created. The
       exiting block is disconnected from the exit block and is connected as
       predecessor to the new intermediate block.
     * All the intermediate blocks and the original loop latch are landing to a
       new loop latch. In the new loop latch, the back edge is taken if the new
       loop latch is reached from the original loop latch and the old back-edge
       condition is true. Otherwise, the control is transferred to the loop
       exit.
     * To preserve the old control flow, several cascaded if blocks are emitted
       after the new loop latch. Each cascaded if block redirects the control
       flow to the right exit block.

  #. **Inner loop non-uniform control flow massaging**: Matt's TODO. Please,
     add reference if documentation is in another file.

  #. **Loop single exit massaging**: splits loops single exit VPBasicBlock when
     one of the following conditions is met:

     * Loop exit has multiple successors.
     * Loop exit is preheader of another loop.

  #. **Non-loop region Entry and Exit massaging**: splits VPBasicBlocks that are
     potential Entry and/or Exit VPBasicBlock's of VPRegionBlock's but currently
     have multiple incoming and outgoing edges. These VPBasicBlock's meet all of
     the following conditions:
 
     * VPBasicBlock has more than one successor.
     * VPBasicBlock has more than one predecessors.

     These conditions are too generic and need refinement but they cover the
     following cases that will prevent the formation of a VPRegionBlock:
   
     * Loop header with two successors. It cannot be a VPRegionBlock Entry
       because it also has two predecessors. 
     * VPRegionBlock's Exit that is another VPRegionBlock's Entry. A
       VPBasicBlock cannot be part of multiple VPRegionBlock's.
     * VPBasicBlock with multiple predecessors that is also a Loop latch and a
       Loop exiting VPBasicBlock. It cannot be a VPRegionBlock Exit because it
       also has multiple successors.

   **TODOs**
      * Add splitting rules to enable the construction of VPRegionBlock's that
        are currently not built because they share the same Exit VPBasicBlock's
        with other VPRegionBlock's. This transformation would require to
        introduce new recipes to split original phi instructions in the
        problematic basic blocks.
      * Massaging's should be subject to uniformity analysis. E.g., if an
        innerloop is uniform, let it continue to have as many exit/ing blocks
        as it likes.
 
#) **Build Hierarchical CFG**

   It builds VPLoopRegion's and non-loop VPRegionBlock's on the plain CFG using
   an outer-to-inner approach. It carries out two detection steps for each
   VPBasicBlock:

   1. **VPLoopRegion detection**: creates new VPLoopRegion's for each VPLoop in
      VPLoopInfo analysis and inserts them into the CFG. Loop preheader and
      loop single exit VPBasicBlock's are used as Entry and Exit of the region,
      respectively.

   #. **VPRegionBlock (non-loop region) detection**: traverses the plain CFG
      and, for each VPBasicBlock, checks whether a VPRegionBlock can be formed
      using such VPBasicBlock as Entry. If so, a new VPRegionBlock is created
      and inserted into the CFG. The new VPRegionBlock is then recursively
      processed following the same approach. The criteria to form a region is
      the following: 

      * Entry and Exit must be VPBasicBlock.
      * Entry must have more that one successors and a single predecessor.
      * Exit must have more that one predecessors and a single successor.
      * Entry/Exit cannot be the Entry/Exit of parent region, respectively.
      * A VPRegionBlock immediately nested inside a VPLoopRegion cannot contain
        a graph cycle (i.e., loop latch/loop header cannot be reachable from
        region's Entry to region's Exit).

     **TODOs**
       * Improve efficiency of regionIsBackEdgeCompliant. Use Dominance
         Frontiers or any other analysis that provides information about graph
         cycles.


