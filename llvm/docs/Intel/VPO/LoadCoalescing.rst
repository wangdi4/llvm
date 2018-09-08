===============
Load Coalescing
===============


This pass scans the basic block for vector loads that access consecutive memory locations and attempts to replace them with wider vector loads. For example, if we have two 4-wide loads from A[i:i+3] and A[i+4:i+7], it will replace them with a single 8-wide load, A[i:i+7]. If it is legal and dependencies allow, the uses of the loads will get their values through additional shuffles that would be generated as part of the code generation for this pass. An experimental feature allows the pass to coalesce scalar loads in addition to vector loads.

Approach
""""""""
LoadCoalescing is a 'Function' pass. It scans the basic blocks of a function and then tries to coalesce vector-loads within the same basic block into wider vector loads if it is legal and deemed profitable to do so. It does that in the following steps,

    1. **Collect Vector Loads** - We employ the  ``BasicBlockMemRefAnalysis`` class to collect vector loads.  This class scans the basic block for vector loads and put them into different buckets based on the pointers that they are operating on. E.g., if we have ``load(A[i+0]), load(A[i+15]), load(B[i+3]), load(A[i+5])``, the ``Buckets`` would have the following entries,

      ``load (A[i+0]) -> [load (A[i+0]), load (A[i+15]), load (A[i+5])]``

      ``load (B[i+3]) -> [load(B[i+3])]``

      Where ``load (A[i+0]`` and ``load (B[i+3]`` would be the ``bucket headers`` and ``[load (A[i+0]), load (A[i+15]), load (A[i+5])]`` and ``[load(B[i+3])]`` would be bucket members. We also end up adding the ``bucket-header`` to the list of ``bucket-members``. This helps us easily sort the memory references. The current analysis cannot tell whether ``A[i+x]`` and ``A[i+x+1]`` are at a constant offset from one another and ends up putting them in different buckets. This limits the capabilities passes like LoadCoalescing.
      As a next step, we *sort* the buckets based on the *Offset*-from-the-base we computed. This gives us disjoint sets of load/store instructions, such that ``A[i]`` and ``A[i+1]`` are in one set and ``A[i+6]`` and ``A[i+7]`` are in a different set. The greatly simplifies the LoadCoalescing algorithm and makes it more efficient.

    2. **Build chains** - In this step, we build chains of consecutive loads. We have the sorted list of memory-references for each bucket and also disjoint sets of memory reference within each bucket. We use these disjoint sets to form chains. E.g., for ``A -> [load <2 x i32 > A[i+0], load <2 x i32 > A[i+4], load <2 x i32 > A[i+2], load <2 x i32 > A[i+14], load <2 x i32 > A[i+12], load <2 x i32 > A[i+18], load <2 x i32 > A[i+16], ...]``, we form two chains of consecutive loads as follows,

        ``load <2 x i32 > A[i+0] --> load <2 x i32 > A[i+2] --> load <2 x i32 > A[i+4]``

        ``load <2 x i32 > A[i+12] --> load <2 x i32 > A[i+14] --> load <2 x i32 > A[i+16] --> load <2 x i32 > A[i+18]``

       where ``load <2 x i32 > A[i+0]`` and ``load <2 x i32 > A[i+12]`` are the **Heads** and ``load <2 x i32 > A[i+4]`` and ``load <2 x i32 > A[i+18]`` are the **Tails** of the chain. The above functionality is implemented in the *buildLoadsChain* function in ``BasicBlockMemRefAlignmentAnalysis`` class.

    3. **Create groups** - In this step, we create *groups*, i.e., the maximal set of loads from each chain which would ultimately be converted into wider loads. A chain can produce zero or many widened vector loads. We can have left-over (remainder) loads, i.e., loads which cannot be successfully combined with other widened loads. The procedure is as follows,

       In ``buildMaximalGroups``, We iterate through the buckets and starting with **Heads** as the seed instructions for forming groups. Starting with a single instruction, we ``tryInsert`` an instruction into the group, keeping in mind the profitability of the exercise.  We use an iterative approach to build the group either to include all the loads in the chain or till we reach the limit ( target vector-register width or the end of the chain, i.e., we hit the **Tail**).  We also keep track of the next 'seed' instruction as we would want to continue coalescing any more of the loads in the chains.
    4. **Schedule Group** - We do a simple profitability analysis of the group to determine whether generating a wider load would be worth it. Next, we run *trySchedule* method on the group. This method builds a local DAG for the group and schedules the instructions in the group.
    5.  **Generate Code** - We do the code generation in three steps,

        - ApplySchedule - This method applies the schedule that we generated using the *trySchedule* method.
        - Generate the wide load - After we create the maxGroup, we generate code for the group. We get the 'head' from the group and generate a widened load instruction. Within the basic block, we insert the load immediately after the point where the original 'head' instruction existed.
        - Generate Shuffles - We build shuffle instructions for all the uses of the widened load instruction.

Future Work
"""""""""""
Good extensions to the current implementation could be,

    1. Coalesce loads across basic-blocks.

    2. Include a better cost-model when deciding whether coalescing loads would be profitable.
    3. Have LoadCoalescing work with SLP vectorization. Decisions about coalescing loads could be influenced by the needs of the SLP vectorization algorithm.
