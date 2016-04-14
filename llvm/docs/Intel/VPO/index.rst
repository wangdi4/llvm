==============
VPO Vectorizer
==============

.. contents::
   :local:

.. toctree::
   :hidden:

   SIMDLaneEvolution/index
   Predication/index

Introduction
============

VPO Vectorizer is Xmain's loop/function, explicit/auto vectorizer.

Vectorizer Skeleton
-------------------

The skeleton below reflects dependences of passes (on other
passes, or on a specific context -- such as loop-level)
and the inputs/outputs the passes consume/produce. 


The general flow of the Innermost vectorizer is as follows:

For each candidate innermost loop:

.. code-block:: c
   :linenos:

        WI = WIAnalysis ( AVR );                   //VF insensitive SLEV (see phase ordering note below)
                                                   //to process/propagate uniform/linear argument directives
        ALIGNInfo = AlignmentAnalysis ();
        Mrfs = MemoryAccessPatternAnalysis ( AVR, WI, ALIGNInfo ); 
                                                   //Gather all memref RegDDRefs in Loop, and analyze their
                                                   //access in Loop (strided/indexed…). Used to create/populate 
                                                   //the OVLSMemrefs for VLS grouping, and the LocTable.
        AVR = OptimizeAbstractLayer ( AVR );       //Apply idioms and other avr optimizations.
        LocTable = SetupLocations ( AVR, Mrfs );   //Create the location table, 
                                                   //manage reductions, privates, ordered, etc.
        VFs = ComputeCandidateVFs( AVR );
        VFRanges = FindVFRangesForVLSGrouping ( VFs, DDG, Mrfs ); 

        For VFRange in VFRanges: 
             
            VFsInRange = getNextVFRange ( VFs, DDG ); //Invalidate VLS Grouping
            For VF in VFsInRange:

                VF = getNextVF ( VFsInRange ); 
                AVR = TypeSelectionAnalysis ( AVR, VF );
                S = SLEVAnalysis ( VF , AVR );
                Mrfs = RefineMemrefsAnalysis ( S , Mrfs );
                VLSGroups = VLSGroupsAnalysis ( VFRange, DDG, Mrfs ); //If Mrfs (following SLEV) haven't
                                                                      //changed, no need to rerun VLSGroup
                AVR = Predication ( S, AVR );
                CurrCost = CostAnalysis ( AVR, Mrfs, VLSGroups );

                BestAVR,BestVF,BestCost = min (BestCost, CurrCost); 

        Packetize/CodeGen ( BestAVR, BestVF );


Note: In the above, the fact that some analysis
pass appears inside a loop that iterates over candidate VFs, such as SLEV analysis 
for example, only implies that SLEV analysis is sensitive to the VF; But it doesn’t 
mean that in the actual implementation SLEV analysis has to be invoked inside that loop.
We could compute SLEV for all loops and all VFs up front and record that information, 
and then in the VF-loop just call a SLEV utility that asks for SLEV information for 
a specific instance (loop-level and VF). 

Vectorizer Passes
-------------------

**Setup Locations**
  TODO.

**SLEV Analysis**
   Describes the SIMDLaneEvolution analysis: :doc:`SIMDLaneEvolution <SIMDLaneEvolution/index>`

**Predication**
   Describes the Predication transformation: :doc:`Predication <Predication/index>`

**Memory Access Pattern Analysis and its dependence on SLEV**
  Memory reference analysis pass will perform the necessary setting of access 
  patterns as unit, non-unit, strided, indexed, etc. This analysis also needs 
  information about which variables are simd linear/uniform, which in turn requires
  processing the simd pragma operands that may be incoming, and propagating 
  this information.   
  Since SLEV analysis processes and propagates linear/uniform/random information through control-flow,
  Memory Access Pattern Analysis depends on SLEV. 
  Here's an example why SLEV Analysis affects Memory Access Pattern Analysis:
  Say we have this loop and we are vectorizing the i-loop:

.. code-block:: c

    for (i=0; i<M; i++) { 
      k=0; 
      for (j=0; j<N; j++) {
        if (C) { k++;}
        a[i+k];
      }
    }

In the above, the vectorized `a[i+k]` starts the j-loop as unit-strided (`a[0,1,2,3]`); 
however, between consecutive iterations of the j-loop:

 - If the condition is uniform: all lanes either advance by 1, or all lanes don’t advance; 
   so the access remains a unit-stride load (although we don’t know exactly how it advances between j-iterations).
 - If the condition is random: some lanes advance by 1 and some don’t, so `a[i+k]` is random.

So based on SLEV analysis, refineMemRefAnalysis (which may or may not be part of SLEV analysis itself) should be able to deduce that:

 - If the condition is (piecewise) uniform: `a[i+k]` is unit-stride on the i-loop. We can generate a regular vector load;  
 - If the condition is random: We have to generate a gather in the loop. 

SLEV analysis is sensitive to both the loop-level vectorized and the VF, as it 
may be able to identify that some branch is piecewise-uniform for a 
certain VF but not for some other VF. 
FOr example, In the example code above, if the condition `C` is `(j > 7)`,
SLEV can tell that for VF=2/VF=4/VF=8 the branch is uniform,
but for VF=16 and more this doesn’t hold.

.. note:: **Phase Ordering Issue:** The refinement of memref analysis is sensitive to 
   SLEV, which in turn is sensitive to both the loop-level and VF;
   However when the location table is created, this information should already be consumed.
   So refining memref access analysis based on SLEV 
   may be too late for that.
   One option is to start off with a rough mem-access analysis that doesn’t use SLEV information (ignores simd pragma operands classification), 
   and refines later per vectorization context; 
   Alternatively, the approach taken in the current skeleton, is to do some very simplified propagation of the linear/uniform simd pragma operands -- 
   some kind of simplified SLEV insensitive to VF, such as in the WI-Analysis in the openCL vectorizer. 
   
**MemrefAnalysisRefinement**
   Refine memory access information based on VF-sensitive SLEV analysis.
   Refinement doesn’t necessarily mean complete recalculation of entire memref analysis from one VF to another. 
   We can probably identify when VF matters (when piecewise analysis matters), which memrefs are sensitive to that if any, etc.  
   in order to minimize the repeated computation of memref analysis.

**Find VF-ranges** 
  VF affects grouping only when there are dependences that limit us in moving 
  memrefs around (so that they could reside in the same VLS group), where the 
  distance of those dependences is less than VF. For example, if there’s a 
  dependence with distance 4 that prevents moving a load past a store, then 
  for VFs=2,4 we don’t care about this dependence and we’ll compute the same 
  Groups in both cases. However for VFs=8,16,… we’ll be more limited and may 
  compute different groups. So in short, VF-ranges determine when we need to 
  recalculate VLS-grouping. If the ranges are {2,4}, {8, maxVF} we only need 
  to calculate grouping twice for the Loop. (This is just a compile time 
  optimization…  We can start with recalculating the VLS-groups for each VF; 
  or alternatively we can assume that the minimum dependence distance is 
  always large enough and calculate grouping only once). 

**VLSGroupsAnalysis**
  The VLS getGroups utility takes as input 1) a vector of OVLSMemrefs:
  an IR independent representations of the Mrfs, and 2) GroupSize, which limits 
  how many memrefs it will try to place in the same group. The GroupSize 
  “basically represents the size of underlying vector register length, unless 
  there is a special requirement by the client, for example the groupsize will 
  be different for accesses in a remainder loop, than the accesses in the main 
  loop.” (Farhana). (**Chekme**: clarify why the getGroups utility has to be 
  limited to use only one vector register). Currently the MaxGroupSize is set to 64 bytes; Hopefully 
  when a VLS client (such as the vectorizer) uses MaxGroupSize when calling the 
  getGroups utility it is guaranteed to get the best shuffle sequence (so that 
  it is never necessary to try out different group sizes). (**Checkme**: confirm
  this last point).



