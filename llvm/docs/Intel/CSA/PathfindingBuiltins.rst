========================
CSA Pathfinding Builtins
========================

**Intel Top Secret**

.. contents::
  :local:

.. toctree::
  :hidden:

Introduction
============

The pathfinding builtins are a set of intrinsics added for the pathfinding
compiler in order to make it possible to expose useful user annotations for CSA
code with minimal intrusion into the frontend and middle-end optimization
passes. When the production compiler eventually ships, they are expected to be
completely supplanted by a combination of proper OpenMP directives with backend
outlining, more standard LLVM metadata, and automatic pattern detection.
However, they are still included in the current build of the compiler because
there is no alternative yet for Fortran (as we don't control the Fortran
frontend) and because some features are not yet supported through other
annotation mechanisms. This page describes each intrinsic in detail and each of
the passes which have been added to manipulate them.

User-Visible Intrinsics
=======================

These intrinsics are available to users as builtins (prefixed with
``__builtin_csa`` for C (and C++) and prefixed with ``builtin_csa`` for
Fortran). Some of them also have x86 no-op variants defined so that they don't
need special marking when used in the compiler offload mode. For the ones that
don't, they need to be wrapped in a ``#ifdef __CSA__``:

.. code-block:: c

   #ifdef __CSA__
   __builtin_csa_pipeline_loop(0);
   #endif // __CSA__

With the exception of `parallel_{region,section}_{entry,exit}`_, these are only
meant for early expansion, aren't generally optimization-safe, and should only
be added by the frontend (or CSAFortranIntrinsics_), not optimization passes.
If you are writing a pass, use `parallel_{region,section}_{entry,exit}`_ and/or
the `internal intrinsics`_ instead.

parallel_loop
-------------

The parallel loop intrinsic is used to mark loops where there are no
dependencies between memory operations across iterations. It is available in C:

.. code-block:: c

   __builtin_csa_parallel_loop();
   for (int i = 0; i < N; ++i) A[i] = B[i];

As well as Fortran:

.. code-block:: fortran

   call builtin_csa_parallel_loop()
   do i=1,n
     a(i) = b(i)
   enddo

The OpenMP equivalent of the parallel_loop intrinsic is parallel for:

.. code-block:: c

   #pragma omp parallel for
   for (int i = 0; i < N; ++i) A[i] = B[i];

As with several other of these intrinsics, the "memory operations" for the
purposes of the parallel_loop intrinsic are any loads and stores that survive
optimization to where the memory ordering pass runs (after instruction
selection). Because of this there isn't exactly a formal definition for what
constitutes a "memory operation" at the source code level. In practice most
cases where there is a loop-carried dependence with something that can be
trivially lowered into an SSA value by mem2reg/SROA fall outside of the domain
of the parallel_loop intrinsic, such as updating a local variable with a simple
type:

.. code-block:: c

   int sum = 0;
   __builtin_csa_parallel_loop();
   for (int i = 0; i < N; ++i) sum = sum + A[i];

Local arrays where the index of each access is known at compile time are also
generally safe:

.. code-block:: c

   int sums[M];
   #pragma unroll M
   for (int j = 0; j < M; ++j) sums[j] = 0;
   __builtin_csa_parallel_loop();
   for (int i = 0; i < N; ++i) {
     #pragma unroll M
     for (int j = 0; j < M; ++j) sums[j] += A[i*M + j];
   }

For these cases the "parallel" in parallel_loop is a slight misnomer because
the iterations still have to happen in order to some degree because of the loop
carried data dependencies. However, they are still valid targets for the
parallel_loop intrinsic if the operations that do come out as actual loads and
stores are parallel across loop iterations and marking such loops can be very
beneficial.

There are also cases where the parallel_loop intrinsic is superfluous because
the memory ordering pass is able to infer that no ordering is needed around the
loop by itself. For now, these cases are ones where each alias set in the loop
contains either only loads or a single store. In the future the number of cases
that the memory ordering pass can reason about by itself should increase
dramatically when it gets access to better loop analysis.

The parallel_loop intrinsic is implemented via expansion into
`parallel_{region,section}_{entry,exit}`_ by the CLIE_ pass.

spmd
----

The spmd intrinsic is used to apply SPMDization [SPMDization link here] to the
marked loop. It is available in C:

.. code-block:: c

   __builtin_csa_spmd(8, 0);
   for (int i = 0; i < N; ++i) A[i] = B[i];

As well as Fortran:

.. code-block:: fortran

   call builtin_csa_spmd(8, 0)
   do i=1,n
     a(i) = b(i)
   enddo

In both cases the first argument is the number of workers and the second is the
chunk size which determines which type of SPMDization to apply.

The OpenMP equivalent of the spmd intrinsic is parallel for with the num_threads
clause (and possibly the schedule clause for blocking/hybrid SPMDization):

.. code-block:: c

   #pragma omp parallel for num_threads(8) schedule(static)
   for (int i = 0; i < N; ++i) A[i] = B[i];

The spmd intrinsic is implemented in two stages: one for the actual worker
generation and one for the implied parallel_loop semantics for each individual
worker. The parallel_loop portion is implemented using the same logic as
parallel_loop_ in CLIE_. The worker generation is handled by the LoopSPMDization
pass but the intrinsic is expanded to `spmdization.{entry,exit}`_ first by CLIE_
to make it more robust to optimizations.

spmdization
-----------

The spmdization intrinsic is an older version of the spmd_ intrinsic that
specifies the type of SPMDization to apply with a string rather than a chunk
size. Unlike with the spmd_ intrinsic, there is no way to specify the chunk size
used for hybrid SPMDization; if "hybrid" is passed for the second argument the
chunk size will be assumed to be 8. The spmdization intrinsic is only available
in C:

.. code-block:: c

   __builtin_csa_spmdization(8, "blocking");
   for (int i = 0; i < N; ++i) A[i] = B[i];

The spmdization intrinsic is implemented by being internally translated to the
equivalent spmd_ intrinsic during CLIE_.

pipeline_loop
-------------

The pipeline_loop intrinsic is used to apply ILPL [ILPL link here] to the marked
loop. It can be used in C:

.. code-block:: c

   __builtin_csa_parallel_loop();
   for (int i = 0; i < N; ++i) {
     __builtin_csa_pipeline_loop(0);
     for (int j = 0; j < M; ++j) A[i] += B[i*M + j];
   }

And in Fortran:

.. code-block:: fortran

   call builtin_csa_parallel_loop()
   do i=1,n
     call builtin_csa_pipeline_loop(0)
     do j=0,m
       a(i) = a(i) + b(i*m + j)
     enddo
   enddo

The single argument is the maximum number of concurrent iterations to allow in
the loop at a given time or 0 for the compiler to choose a number automatically.

There is no OpenMP equivalent for the pipeline_loop intrinsic but almost all
cases where the intrinsic would be useful can be inferred automatically by the
compiler if the -csa-ilpl-selection=automatic flag is passed.

The pipeline_loop intrinsic is expanded to `pipeline.loop.{entry,exit}`_ by
CLIE_ for consumption by the CSAInnerLoopPrep pass.

parallel_{region,section}_{entry,exit}
--------------------------------------

The parallel_{region,section}_{entry,exit} intrinsics are a set of four
low-level quasi-internal intrinsics that provide a powerful interface to the
memory ordering pass in order to make assertions about groups of memory
operations that are known to be independent. They are used to implement the
parallel_loop intrinsic, are used extensively by SPMDization, are currently also
used to implement the OpenMP directives supported in CSA code, and are exposed
to users so that they can be used to mimic the omp sections directive or for
other more specialized cases where the other intrinsics might not work.
Naturally these are the most complicated of the intrinsics listed here, so this
section will be more in-depth than the others.

Syntax
~~~~~~

::

  declare i32 @llvm.csa.parallel.region.entry(i32 <region_id>)
  declare void @llvm.csa.parallel.region.exit(i32 <entry>)
  declare i32 @llvm.csa.parallel.section.entry(i32 <region_entry>)
  declare void @llvm.csa.parallel.section.exit(i32 <entry>)

Arguments
~~~~~~~~~

The arguments to these four intrinsics are mostly used to connect calls to them
together. The single argument to both exit intrinsics should be connected to the
corresponding entry intrinsics. The single argument to the section entry
intrinsic should be connected to a corresponding region entry. The single
argument to the region entry intrinsic is a `region ID`_.

Background
~~~~~~~~~~

Originally, parallel_loop_ was the only intrinsic available and it passed all
of the way through the compilation process until it was detected in the backend
and converted into a pseudo-instruction marking an individual ordering backedge
to be removed by dataflow optimization. There were two main issues with this
implementation:

1. No matter how many side-effects the intrinsic is claimed to have, there was
   no way to stop LLVM from shifting loops around it as long as all of the
   actual operations still happen in the same order. This is bad news for the
   intrinsic because the loops that end up showing up after it can be different
   in the backend from what they were in the frontend. One particularly
   egregious example that we encountered looked like this:

   .. code-block:: c

      for (...) {
        __builtin_csa_parallel_loop();
        for (...) {
          ...
        }
      }
      __builtin_csa_parallel_loop();
      for (...) {
        ...
      }

      // v transformed to v

      __builtin_csa_parallel_loop();
      for (...) {
        for (...) {
          ...
        }
        __builtin_csa_parallel_loop();
      }
      for (...) {
        ...
      }

2. Information about independence of loop iterations was lost during unrolling
   because the loop itself was the only thing marked and there was no way to
   discern unrolled copies of the loop body. This doesn't necessarily affect
   throughput since the unrolled loop bodies are executed in a pipeline, but it
   does make the pipeline take significantly longer to fill compared to an
   implementation where the unrolled loop bodies are fully parallel. In
   practice, this would look like this:

   .. code-block:: c

      __builtin_csa_parallel_loop();
      #pragma unroll 2
      for (int i = 0; i < N; ++i) {
        A[i] = B[i]; // <- all of these are known to be parallel
      }

      // v unrolling (roughly; assuming N%2 == 0 for clarity) v

      __builtin_csa_parallel_loop();
      for (int i = 0; i < N; i += 2) {
        A[i]   = B[i];   // <- these can start in parallel
        A[i+1] = B[i+1]; // <- this has to wait for A[i] = B[i] to finish
      }

The parallel_{region,section}_{entry,exit} intrinsics were introduced to solve
both of these problems: the first one by introducing more intrinsics at key
points in the loop to make the markings more robust against optimizations and
the second by marking the loop body in such a way that the markings are
automatically replicated when the loop body is.

Usage
~~~~~

At a high level, these intrinsics allow users and passes to specify
*parallel regions* with parallel_region_{entry,exit} and *parallel sections*
with parallel_section_{entry,exit}. All ordering edges between sections of the
same region are allowed to be ignored by the memory ordering pass. This is best
illustrated with the canonical unrolling example:

.. code-block:: c

   int reg = __builtin_csa_parallel_region_entry(0);
   for (int i = 0; i < N; i += 2) {
     int sec0 = __builtin_csa_parallel_section_entry(reg);
     A[i] = B[i];
     __builtin_csa_parallel_section_exit(sec0);
     int sec1 = __builtin_csa_parallel_section_entry(reg);
     A[i+1] = B[i+1];
     __builtin_csa_parallel_section_exit(sec1);
   }
   __builtin_csa_parallel_region_exit(reg);

As before, both unrolled loop bodies (``A[i] = B[i]`` and ``A[i+1] = B[i+1]``)
are parallel with respect to other iterations of the loop because the backedge
is outside of both sections. Unlike before, the loop bodies are independent
relative to each other within iterations because they are in different sections.
The 0 that is passed into ``__builtin_csa_parallel_region_entry`` here is a
`region ID`_.

.. _`region ID`: `Region IDs`_

In practice, most uses are compositions of two basic patterns:

1. Loops, marking each iteration as independent even in the event of unrolling
   (as in the example above):

   .. code-block:: c

      int reg = __builtin_csa_parallel_region_entry(0);
      for (int i = 0; i < N; ++i) {
        int sec = __builtin_csa_parallel_section_entry(reg);
        A[i] = B[i];
        __builtin_csa_parallel_section_exit(sec);
      }
      __builtin_csa_parallel_region_exit(reg);

2. Sections, marking parts of straight-line code as independent:

   .. code-block:: c

      int reg = __builtin_csa_parallel_region_entry(0);
      int sec0 = __builtin_csa_parallel_section_entry(reg);
      *a = *b;
      __builtin_csa_parallel_section_exit(sec0);
      int sec1 = __builtin_csa_parallel_section_entry(reg);
      *c = *d;
      __builtin_csa_parallel_section_exit(sec1);
      __builtin_csa_parallel_region_exit(reg);

There are other ways of using these intrinsics but those aren't as useful as
these two patterns and don't show up as frequently in user code.

The parallel_{region,section}_{entry,exit} intrinsics are not supported directly
in Fortran mainly due to the extra complexity of handling return "values." They
are also too low-level to have an exact equivalent in OpenMP, but
``omp sections`` and ``omp section`` are a close (but not yet implemented) match
for simpler use cases.

Semantics
~~~~~~~~~

As originally described, semantics for the
parallel_{region,section}_{entry,exit} intrinsics are based on dominance and
post-dominance. A memory operation is said to be in a region/section if
dominated by the parallel_{region,section}_entry and post-dominated by the
parallel_{region,section}_exit. If the dominance/post-dominance conditions are
held it is relatively simple to calculate each memory operation's membership in
each region/section and to directly remove edges between sections with that
information. However, though passes inserting these intrinsics stick to the
dominance/post-dominance conditions, the conditions aren't always sufficient for
use in optimization because there are several transforms that leave regions and
sections without a unique pair of entry/exit intrinsics and possibly even
multiple equivalent entry/exit intrinsics on different paths that don't
dominate/post-dominate the memory operations that they are supposed to.

To get around this, the implementation of these intrinsics in the memory
ordering pass uses a slightly looser version of the semantics based more
directly on control flow paths rather than dominance/post-dominance
relationships. The basic rules are these:

- Regions are identified only by `region IDs`_ which are passed as the argument
  to parallel_region_entry intrinsics. All intrinsics which depend on region
  entries with the same region ID belong to the same region. Any intrinsic that
  depends on multiple region entries where the region entries don't all have the
  same region ID is an error.

  .. code-block:: llvm

     bb1:
       %0 = call i32 @llvm.csa.parallel.region.entry(i32 0) ; Entry for region 0
       br label %bb3

     bb2:
       %1 = call i32 @llvm.csa.parallel.region.entry(i32 0) ; Entry for region 0
       %2 = call i32 @llvm.csa.parallel.region.entry(i32 1) ; Entry for region 1
       br label %bb3

     bb3:
       %p1 = phi i32 [ %0, %bb1 ], [ %1, %bb2 ]
       %p2 = phi i32 [ %0, %bb1 ], [ %2, %bb2 ]
       %3 = call i32 @llvm.csa.parallel.section.entry(i32 %p1)
         ; ^ Okay: part of region 0
       %4 = call i32 @llvm.csa.parallel.section.entry(i32 %p2)
         ; ^ ERROR: region ID ambiguous!

- Regions are processed entirely independently and there are no constraints
  between intrinsics belonging to different regions. The set of ordering edges
  that can be ignored is the union of the sets from individual regions.
- A memory operation is inside of a region if the last region entry/exit of that
  region along every path to that memory operation is an entry and the next
  region entry/exit of that region along every path from that memory operation
  is an exit. Every point in the function must either be inside or outside of
  every individual region: points are not allowed to be inside a region along
  one path and outside along another.

  .. code-block:: llvm

     bb1:
       %0 = call i32 @llvm.csa.parallel.region.entry(i32 0)
       ; Okay: inside region 0
       br i1 undef, label %bb2, label %bb3

     bb2:
       ; Okay: inside region 0
       br label %bb4

     bb3:
       ; Okay: inside region 0
       call void @llvm.csa.parallel.region.exit(i32 %0)
       ; Okay: outside region 0
       br label %bb4

     bb4:
       ; ERROR: membership in region 0 ambiguous!

- The same rules apply to determine if a memory operation is inside of a section
  of a region. *Which* section a memory operation is inside of is irrelevant for
  this analysis: the only important information is whether a memory operation is
  in any section of the region or not.

  .. code-block:: llvm

     bb1:
       %0 = call i32 @llvm.csa.parallel.region.entry(i32 0)
       ; Okay: outside of section of region 0
       %1 = call i32 @llvm.csa.parallel.section.entry(i32 %0)
       ; Okay: inside of section of region 0
       br i1 undef, label %bb2, label %bb3

     bb2:
       ; Okay: inside of section of region 0
       call void @llvm.csa.parallel.section.exit(i32 %1)
       ; Okay: outside of section of region 0
       %2 = call i32 @llvm.csa.parallel.section.entry(i32 %0)
       ; Okay: inside of section of region 0
       br label %bb3

     bb3:
       %p1 = phi i32 [ %1, %bb1 ], [ %2, %bb2 ]
       ; Okay: inside of section of region 0
       call void @llvm.csa.parallel.section.exit(i32 %p1)
       ; Okay: outside of section of region 0
       %3 = call i32 @llvm.csa.parallel.section.entry(i32 %0)
       ; Okay: inside of section of region 0
       br i1 undef, label %bb4, label %bb5

     bb4:
       ; Okay: inside of section of region 0
       call void @llvm.csa.parallel.section.exit(i32 %3)
       ; Okay: outside of section of region 0
       br label %bb5

     bb5:
       ; ERROR: section membership for region 0 ambiguous!

- Memory ordering edges can be ignored along a path between two memory
  operations if all of these hold for any region:

  - Both memory operations are inside of sections of that region.
  - The path contains a section exit and entry (a section crossing)
    corresponding to the region.
  - The path does not contain a region exit and entry (a region crossing) for
    that region - ordering must be preserved outside of the parallel region.

  .. code-block:: llvm

     bb1:
       store i32 undef, i32* undef ; <- st1, outside of region 0
       ; no memory operations precede st1
       br label %bb2

     bb2: ; <- header of loop lp1
       %0 = call i32 @llvm.csa.parallel.region.entry(i32 0)
       br label %bb3

     bb3: ; <- header of loop lp2
       %1 = call i32 @llvm.csa.parallel.section.entry(i32 %0)
       store i32 undef, i32* undef ; <- st2, inside of section of region 0
       ; st2 <- st1 preserved (because st1 is not in a section of region 0)
       ; st2 <- lp2 <- st3 ignored (section crossing around lp2)
       ; st2 <- lp1 <- st3 preserved (section and region crossing around lp1)
       store i32 undef, i32* undef ; <- st3, inside of section of region 0
       ; st3 <- st1 preserved (because st1 is not in a section of region 0)
       ; st3 <- st2 preserved (no section crossing on the straight-line path)
       ; st3 <- lp2 <- st2 ignored (section crossing around lp2)
       ; st3 <- lp1 <- st2 preserved (section and region crossing around lp1)
       call void @llvm.csa.parallel.section.exit(i32 %1)
       br i1 undef, label %bb3, label %bb4

     bb4:
       call void @llvm.csa.parallel.region.exit(i32 %0)
       br i1 undef, label %bb2, label %bb5

- For every region, any two paths between the same memory operations which
  traverse the same set of loop backedges should produce the same result when
  analyzed to determine whether memory ordering edges can be ignored.

  .. code-block:: llvm

     bb1:
       %0 = call i32 @llvm.csa.parallel.region.entry(i32 0)
       %1 = call i32 @llvm.csa.parallel.section.entry(i32 %0)
       store i32 undef, i32* undef ; <- st1
       br i1 undef, label %bb2, label %bb3

     bb2:
       call void @llvm.csa.parallel.section.exit(i32 %1)
       %2 = call i32 @llvm.csa.parallel.section.entry(i32 %0)
       br label %bb3

     bb3:
       store i32 undef, i32* undef; <- st2
       ; ERROR: st2 <- st1 preserved on bb1->bb3, ignored on bb1->bb2->bb3!

Some of these rules are assumed by the memory ordering pass but some are checked
explicitly. In the rare case that one of the explicitly-checked rules is broken,
the memory ordering pass will emit this warning::

  !! WARNING: BAD PARALLEL SECTION INTRINSICS !!

After emitting this warning, the memory ordering pass will back off and retry
ordering without parallel region/section intrinsics. Since parallel
region/section intrinsics are rarely used directly by users, this warning is
most often the result of an internal compiler issue either from a bug in one
of the passes that handles these intrinsics directly or from an unrelated
transform breaking one of the rules above. In this situation, the
``-csa-view-pre-ordering-memop-cfg`` option can be used to show the memop CFG
with all of the region/section intrinsics intact which is very useful for
identifying which region/section intrinsics are causing problems.

It should be noted that one corner case for these intrinsics is memory
operations which appear inside of a region but outside of any section of that
region:

.. code-block:: c

   int reg = __builtin_csa_parallel_region_entry(0);
   *a = 2;
   int sec = __builtin_csa_parallel_section_entry(reg);
   *b = 3;
   __builtin_csa_parallel_section_exit(sec);
   __builtin_csa_parallel_region_exit(reg);

According to the rules above, normal ordering is preserved between the store to
``a`` and the store to ``b`` in this example, as if the store to ``a`` was
outside of the region.

Region IDs
~~~~~~~~~~

Region IDs are unique integers used to identify parallel regions, which are
passed as the argument to parallel_region_entry intrinsics. Two regions in the
same function cannot use the same region ID because that would make them
logically the same region, but at the same time it is incorrect to arbitrarily
assign unique region IDs to each region entry intrinsic since there are cases
where we could want a region to have multiple region entry intrinsics.

Region IDs have surprisingly proven to be the most common source of headaches
with these intrinsics since "generate a unique integer" turns out to be easier
said than done. These are the rules that we have established for dealing with
them correctly:

- When users use ``__builtin_csa_parallel_region_entry`` in their code directly
  they are responsible for choosing region IDs and it is on them to ensure
  uniqueness. Region IDs below 1000 are reserved for users and will not be used
  by the compiler when inserting region entry intrinsics.
- When the compiler is putting in parallel regions it has to assign unique
  region IDs as well. In order to ensure that region IDs are unique across
  passes, there's a special procedure that needs to be followed to generate
  them:

  1. The first step is to create a region entry intrinsic call with a dummy
     region ID parameter. It doesn't matter what gets put there since it'll be
     overwritten in a later step.
  2. As the value is created, LLVM will automatically assign it a unique name
     based on the one that we provide for it. This is what we use as a baseline
     for the region ID that is generated later in order to ensure that it is
     unique within the function.
  3. This is not enough to ensure uniqueness across passes because there are
     cases where a name gets used by one pass, an optimization renames the value
     which frees the name, and then a later pass re-uses the same name and ends
     up with a region ID conflict. To avoid this, we ensure that each pass has a
     unique prefix which it uses in the base names that it provides to LLVM. In
     the example below, this is ``clie`` for CSALoopIntrinsicExpander_.
  4. Since the region ID needs to be an integer, we use getMDKindID in order to
     establish a mapping from the region entry call's value name string to an
     integer.
  5. 1000 is added to that value to reserve space for users.
  6. Finally, the operand is updated with the new region ID that has been
     generated.

  In code, this looks like this:

  .. code-block:: c++

     CallInst *const region_entry
       = IRBuilder<>{preheader_terminator}.CreateCall(
         Intrinsic::getDeclaration(
           module, Intrinsic::csa_parallel_region_entry
         ),
         ConstantInt::get(IntegerType::get(context, 32), 0),
         "clie_pre"
       );
     const int region_token
       = context.getMDKindID(region_entry->getName()) + 1000;
     region_entry->setOperand(
       0, ConstantInt::get(IntegerType::get(context, 32), region_token)
     );

- The above two cases are enough for keeping region IDs unique within functions,
  but during inlining parallel regions from different functions can get
  imported to a function and conflict with the existing regions in that
  function. This was especially problematic when users were trying to wrap
  parallel loops to use them with C++ lambdas:

  .. code-block:: c++

     template <typename T, typename F>
     void csa_parfor(T* base, int stride, int end, F&& fun) {
       __builtin_csa_parallel_loop();
       for (T* cur = base; cur < base + end; cur += stride) fun(cur);
     }

     void inc_mat(int* A) {
       csa_parfor(A, M, M*N, [](int* row) {
         csa_parfor(row, 1, M, [](int* el) {
           ++*el;
         });
       });
     }

  The parallel_loop intrinsic is lowered *before* inlining, which means that the
  IDs for the regions that will eventually end up in ``inc_mat`` are generated
  while their code is still in the two instantiations of ``csa_parfor``. These
  region IDs aren't necessarily unique because they're in different functions
  - if they're the same (which is actually quite likely), they will cause a
  region ID conflict during inlining.

  We get around this issue with a special patch to the inliner (mostly in
  reassignCSAParallelRegionId in lib/Transforms/Utils/InlineFunction.cpp) which
  detects parallel region entries during inlining and automatically assigns them
  new region ids using a procedure similar to the one from the previous bullet
  to make sure that the inlined region IDs are distinct from the ones already in
  the function.

Using unique integers this way to distinguish parallel regions is pretty ugly
and is ripe for bugs when passes aren't extremely careful about the way that
they're generated. We've been looking into better ways of doing this off and on
and one promising alternative approach is to identify parallel regions using the
addresses of dummy local variables: before register allocation LLVM generally
does a very good job of making sure that local variables don't end up at the
same address, so they have roughly the same behavior that we want for region
IDs. On the other hand, the integer solution is working and we're planning on
getting rid of these intrinsics before product release, so there isn't much
benefit to changing them now.

Intrinsic Properties and Optimization Interaction
-------------------------------------------------

The memory operations that these intrinsics apply to are identified only by
their control flow relationship to the intrinsics, which means that we need to
lock down the intrinsics so that memory operations aren't allowed to be moved
around them. On the other hand, there are some important optimizations that rely
on being able to move memory operations around the intrinsics, which means that
we need to make sure that they aren't locked down so that those optimizations
will continue to work. These are a couple examples of movement that is
acceptable and movement that isn't:

.. code-block:: c

   // The initial code.
   int reg = __builtin_csa_parallel_region_entry(0);
   for (int i = 0; i < N; ++i) {
     ... = A[i];
     int sec = __builtin_csa_parallel_section_entry(reg);
     ... = *B;
     C[i] = ...
     ...
     __builtin_csa_parallel_section_exit(sec);
   }
   __builtin_csa_parallel_region_exit(reg);

   // A valid movement across the section entry/exit and a very valuable
   // optimization.
   int reg = __builtin_csa_parallel_region_entry(0);
   ... = *B;
   for (int i = 0; i < N; ++i) {
     ... = A[i];
     int sec = __builtin_csa_parallel_section_entry(reg);
     C[i] = ...
     ...
     __builtin_csa_parallel_section_exit(sec);
   }
   __builtin_csa_parallel_region_exit(reg);

   // Another valid movement across the section entry but one that isn't very
   // useful.
   int reg = __builtin_csa_parallel_region_entry(0);
   for (int i = 0; i < N; ++i) {
     ... = A[i];
     ... = *B;
     int sec = __builtin_csa_parallel_section_entry(reg);
     C[i] = ...
     ...
     __builtin_csa_parallel_section_exit(sec);
   }
   __builtin_csa_parallel_region_exit(reg);

   // An *invalid* movement across the section entry. This code has memory
   // ordering edges ignored that were not ignored in the original code, and is
   // therefore incorrect.
   int reg = __builtin_csa_parallel_region_entry(0);
   for (int i = 0; i < N; ++i) {
     int sec = __builtin_csa_parallel_section_entry(reg);
     ... = A[i];
     ... = *B;
     C[i] = ...
     ...
     __builtin_csa_parallel_section_exit(sec);
   }
   __builtin_csa_parallel_region_exit(reg);

In general, it is safe to move memory operations *out of* parallel sections but
not *into* parallel sections because moving things into parallel sections is
what relaxes ordering constraints. LLVM does not support such a distinction out
of the box and the amount of intrusive changes that we would have to make to
common transforms in order to specifically support these intrinsics is more than
we are comfortable with, so our choices with the basic controls that LLVM has
are to either make the intrinsics too constrained and inhibit optimizations or
to make them not constrained enough and risk incorrect code.

These intrinsics are currently marked ``IntrInaccessibleMemOrArgMemOnly``, which
leans to the side of not being constrained enough and is technically too loose
to stop memory operations from being moved into parallel sections. However, this
isn't a problem in practice since problematic code movement like in the last
example above isn't beneficial for optimization. But even with the overly loose
intrinsic properties we still have some cases where useful optimizations are
blocked by the presence of these intrinsics.

The real problem here is that we're trying to identify groups of memory
operations based on where they appear in control flow and control flow is too
fluid to be reliably used this way without overly constraining the program. What
we really want is mechanisms to mark individual memory accesses which will
follow them around during code motions and which is standard enough to already
be handled correctly by LLVM's standard optimizations. One very early experiment
with this is the CSALowerParallelIntrinsics pass (enabled with the
``-csa-lower-parallel-intrinsics`` flag) which attempts to re-express parallel
regions/sections using ``llvm.loop.csa_parallel`` loop metadata but which can't
handle every case due to a combination of the loop metadata being less powerful
than parallel regions/sections and the pass' incomplete handling of the
regions/sections themselves.

Internal Intrinsics
===================

These intrinsics are not user-visible and are added by CLIE_ to mark loops of
interest for later passes. They come in pairs in order to be more robust to
optimizations and avoid issues that the user-visible loop intrinsics have when
their loops are moved out from under them. One thing to keep in mind with these
intrinsics is that even though they always enclose a single loop when they are
originally added, that won't necessarily be the case when optimizations get to
the loop.

- Intrinsics with no enclosed loop commonly arise from full unrolling:

  .. code-block:: c

     int spmd = spmdization_entry(...);
     for (int i = 0; i < 3; ++i) A[i] = 2;
     spmdization_exit(spmd);

     // v unrolling v

     int spmd = spmdization_entry(...);
     A[0] = 2;
     A[1] = 2;
     A[2] = 2;
     spmdization_exit(spmd);

  Because of this, it is not sufficient to just look for an entry intrinsic in
  the preheader of the loop and an exit intrinsic in the exit: if the loop
  happens to be preceded and succeeded by fully-unrolled loops it would
  incorrectly pick up one of the fully-unrolled loop's markings. The fix for
  this is to search more extensively through the preheader and exit blocks to
  find an entry/exit pair where the exit uses the value of the entry.
  LoopSPMDization::detectSPMDIntrinsic is possibly the best place to look for an
  example of how to implement this.

- Intrinsics with multiple enclosed loops may arise from some oddball (and
  often transient) multiversioning transforms:

  .. code-block:: c

     int spmd = spmdization_entry(...);
     for (int i = 0; i < N; ++i) {
       for (int j = 0; j < m; ++j) A[i*m + j] = 2;
       ...
     }
     spmdization_exit(spmd);

     // v optimizations v

     int spmd = spmdization_entry(...);
     if (m <= 0) {
       for (int i = 0; i < N; ++i) {
         ...
       }
     } else {
       for (int i = 0; i < N; ++i) {
         for (int j = 0; j < m; ++j) A[i*m + j] = 2;
         ...
       }
     }
     spmdization_exit(spmd);

  It is tempting to clean up the intrinsics as they are handled for each loop,
  but for cases like this one that would result in only one of the loops being
  transformed. In the most vexing cases the loop that was transformed gets
  completely optimized out later, leaving behind only one loop which
  inexplicably didn't get transformed despite its markings. The solution to this
  problem is to leave the intrinsics in place between invocations of loop passes
  in case there are other loops that will use them.

In both cases, the passes that handle these intrinsics should not be deleting
them. Instead, CSAIntrinsicCleaner_ was created to make sure that all of the
leftover intrinsics are removed before instruction selection.

spmdization.{entry,exit}
------------------------

Syntax:
~~~~~~~

::

  declare i32 @llvm.csa.spmdization.entry(i32 <worker_count>, i32 <chunk_size>)
  declare void @llvm.csa.spmdization.exit(i32 <entry>)

Overview:
~~~~~~~~~

The spmdization.{entry,exit} intrinsics mark loops that should be SPMDized by
the SPMDization pass.

Arguments:
~~~~~~~~~~

As in the spmd_ intrinsic, the first argument to spmdization.entry is the number
of SPMDization workers and the second is the chunk size. The argument to
spmdization.exit is the "value" returned by the paired spmdization.entry.

Semantics:
~~~~~~~~~~

The spmdization.entry intrinsic should be added to the preheader of the loop
to be SPMDized and the spmdization.exit intrinsic should be added to its exit
block:

.. code-block:: llvm

   entry:
     %spmdization_entry = call i32 @llvm.csa.spmdization.entry(i32 2, i32 1)
     br label %for.cond

   for.cond:                                         ; preds = %for.body, %entry
     %prod.0 = phi i32 [ 1, %entry ], [ %mul, %for.body ]
     %i.0 = phi i32 [ 2, %entry ], [ %inc, %for.body ]
     %cmp = icmp sle i32 %i.0, %n
     br i1 %cmp, label %for.body, label %for.cond.cleanup

   for.body:                                         ; preds = %for.cond
     %mul = mul nsw i32 %prod.0, %i.0
     %inc = add nsw i32 %i.0, 1
     br label %for.cond

   for.cond.cleanup:                                 ; preds = %for.cond
     call void @llvm.csa.spmdization.exit(i32 %spmdization_entry)
     ret i32 %prod.0

Loops that are enclosed by the spmdization.{entry,exit} intrinsics will be
duplicated/bounds-adjusted when the SPMDization pass runs.

pipeline.loop.{entry,exit}
--------------------------

Syntax:
~~~~~~~

::

  declare i32 @llvm.csa.pipeline.loop.entry(i32 <depth>)
  declare void @llvm.csa.pipeline.loop.exit(i32 <entry>)

Overview:
~~~~~~~~~

The pipeline.loop.{entry,exit} intrinsics mark loops that should be ILPL'd for
further processing by the CSAInnerLoopPrep pass.

Arguments:
~~~~~~~~~~

As in the pipeline_loop_ intrinsic, the argument to pipeline.loop.entry is the
maximum number of concurrent iterations to allow in the loop (or 0 for the
compiler default). The argument to pipeline.loop.exit is the "value" returned by
the paired pipeline.loop.entry.

Semantics:
~~~~~~~~~~

The pipeline.loop.entry intrinsic should be added to the preheader of the loop
to be ILPL'd and the pipeline.loop.exit intrinsic should be added to its exit
block:

.. code-block:: llvm

   for.body:                                     ; preds = %for.cond
     %clie_pse = call i32 @llvm.csa.parallel.section.entry(i32 %clie_pre)
     %mul = mul nsw i32 %block, %i.0
     %ilpl_entry = call i32 @llvm.csa.pipeline.loop.entry(i32 0)
     br label %for.cond1

   for.cond1:                                    ; preds = %for.body4, %for.body
     %sum.0 = phi double [ 0.000000e+00, %for.body ], [ %add5, %for.body4 ]
     %j.0 = phi i32 [ 0, %for.body ], [ %inc, %for.body4 ]
     %cmp2 = icmp slt i32 %j.0, %block
     br i1 %cmp2, label %for.body4, label %for.cond.cleanup3

   for.body4:                                    ; preds = %for.cond1
     %add = add nsw i32 %mul, %j.0
     %idxprom = sext i32 %add to i64
     %arrayidx = getelementptr inbounds double, double* %in, i64 %idxprom
     %0 = load double, double* %arrayidx, align 8, !tbaa !2
     %add5 = fadd double %sum.0, %0
     %inc = add nsw i32 %j.0, 1
     br label %for.cond1

   for.cond.cleanup3:                            ; preds = %for.cond1
     call void @llvm.csa.pipeline.loop.exit(i32 %ilpl_entry)
     %idxprom6 = sext i32 %i.0 to i64
     %arrayidx7 = getelementptr inbounds double, double* %out, i64 %idxprom6
     store double %sum.0, double* %arrayidx7, align 8, !tbaa !2
     call void @llvm.csa.parallel.section.exit(i32 %clie_pse)
     %inc9 = add nsw i32 %i.0, 1
     br label %for.cond

Loops that are enclosed by the pipeline.loop.{entry,exit} intrinsics will be
adjusted by the CSAInnerLoopPrep pass and then fully ILPL'd during dataflow
conversion.

Passes
======

The following passes are the ones that are responsible for maintaining these
intrinsics:

.. _CSAFortranIntrinsics:

CSAFortranIntrinsics
--------------------

Since the Intel Fortran frontend isn't ready yet, we are using GFortran with
DragonEgg to compile Fortran programs. However, we don't want to make any
modifications to GFortran for licensing reasons. We still need these intrinsics
to be usable in Fortran, though, so the workaround that we have come up with is
to express them as calls to external procedures and to convert those calls into
intrinsics in the middle-end before the linker gets to them.
CSAFortranIntrinsics is the pass that does this conversion.

CSAFortranIntrinsics must run before CLIE_ so that the intrinsics will get
picked up in that pass, so it is usually the first pass that runs after the
frontend. The pass itself is very simple: it scans each function for
Fortran-style calls and when it finds one it tries to convert it to an intrinsic
based on a small table defined at the top of the file:

.. code-block:: cpp

   //>>>>>> THE INTRINSIC MAPPING TABLE <<<<<<
   // Add entries for any more intrinsics that need to be converted:
   //
   //  intrinsic_table[i].first:  The generated name of the Fortran function to
   //                             convert.
   //  intrinsic_table[i].second: The ID of the intrinsic to convert it to.
   //
   // Note the underscore at the end of the Fortran function names - that seems
   // to be added by the compiler, so if this table has an entry for
   // "builtin_thing_" it will correspond to a call that looks like this in
   // Fortran:
   //
   //  call builtin_thing()
   //
   constexpr std::pair<const char *, Intrinsic::ID> intrinsic_table[] = {
     {"builtin_csa_parallel_loop_", Intrinsic::csa_parallel_loop},
     {"builtin_csa_spmd_", Intrinsic::csa_spmd},
     {"builtin_csa_pipeline_loop_", Intrinsic::csa_pipeline_loop}};

When it is converting one of the calls from this table, it will automatically
translate parameters into forms that are more similar to how parameters are
passed in C. This works for integral constants and possibly floating-point
constants (though none of our builtins use those). Variable inputs and string
constants are not supported by this translation because they aren't needed by
the intrinsics that this pass supports.

.. _CLIE: CSALoopIntrinsicExpander_
.. _CSALoopIntrinsicExpander:

CSALoopIntrinsicExpander
------------------------

CSALoopIntrinsicExpander (CLIE) is the most involved of the passes described
here. It expands single intrinsics marking loops into internal forms that are
more resistant to optimization.

Pass Scheduling
~~~~~~~~~~~~~~~

Since the single intrinsics are easily separated from their loops, CLIE has to
run very early in the compilation flow before nearly all of the optimization
passes. Currently, it is scheduled just after the frontend and after this small
set of passes:

- CSAFortranIntrinsics_
- SROA
- LoopSimplify
- LICM

These passes reduce the number of spurious memory accesses that are seen by CLIE
and ensure that loops appear in normalized form.

Loop Intrinsic Identification
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CLIE is a function pass, but most of its functionality operates on a per-loop
basis. CLIE iterates each loop in a function and starts by looking for loop
intrinsics to be expanded. These intrinsics are added ahead of the loop, so the
search for each loop starts at the preheader and traverses basic blocks
backwards as long as there is a single predecessor and that predecessor is at
the same loop level. The intrinsics that CLIE looks for are:

- parallel_loop_
- spmd_
- spmdization_
- pipeline_loop_

The search stops after it encounters any *one* of these intrinsics - there is no
reason to use multiple intrinsics on a single loop because each implies/is
implied by (spmd_ and parallel_loop_) or cannot be combined with (pipeline_loop_
and spmd_) the others.

Parallel Section Determination
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Once CLIE has identified an intrinsic for the loop, the expansion starts. For
parallel_loop_ and spmd_ (and therefore spmdization_), one component of this
expansion is the addition of `parallel_{region,section}_{entry,exit}`_
intrinsics to mark the loop as having no through-memory dependencies on the
backedge. The general form for these intrinsics is the loop pattern from the
Usage_ sub-section for those intrinsics:

.. code-block:: c

   int reg = __builtin_csa_parallel_region_entry(0);
   for (int i = 0; i < N; ++i) {
     int sec = __builtin_csa_parallel_section_entry(reg);
     A[i] = B[i];
     __builtin_csa_parallel_section_exit(sec);
   }
   __builtin_csa_parallel_region_exit(reg);

This pattern is simple enough at the source level: parallel_region_{entry,exit}
intrinsics are added around the entire loop and parallel_section_{entry,exit}
intrinsics are added around the source-level "body." However, this expansion is
happening at the IR level rather than in the frontend, so there is no general
way to tell which part of the loop was the source-level body (``A[i] = B[i]``
above) and which part was the source-level header (``int i = 0; i < N; ++i``).

The OpenMP expansion has the benefit of some frontend integration that marks the
boundaries of the source-level loop body with special intrinsics, so it is able
to do this expansion correctly; CLIE is not so fortunate and so it has to guess
where the source-level body of the loop was instead. Since
parallel_section_{entry,exit} intrinsics should be added marking a single-entry,
single-exit region of code, CLIE guesses where they should go by calculating the
smallest such region that contains every subloop and every instruction which may
touch memory. The algorithm for this is very simple:

1. Temporarily re-route loop backedges to a dummy basic block and recalculate
   the dominator/post-dominator trees.
2. Start with a region containing a single subloop preheader/instruction.
3. For every subloop preheader or exit block and every block containing an
   instruction that may access memory in the loop, update the region start to
   the lowest common dominator of that block and the old region start and update
   the region end to the highest common post-dominator of that block and the old
   region end.
4. With the region start/end blocks for the section determined, the CFG can be
   restored and the dominator/post-dominator trees reset.
5. The parallel_section_entry goes ahead of the first memory-accessing
   instruction in the start block for the region (or terminator if there is
   none) and the parallel_section_exit goes after the last memory-accessing
   instruction in the end block for the region (or after the last phi if there
   is none).

There are some cases where this algorithm fails to find a good place to put the
parallel section intrinsics, generally because two points that need to be
contained don't have a common post-dominator or have one which is outside of the
loop. In this case, CLIE will skip the loop and emit a warning that looks like
this::

  !! WARNING: COULD NOT PARALLELIZE LOOP !!

Before LICM was added to the list of passes run before CLIE, this warning most
commonly appeared because of spurious loads of loop bounds in the loop header.
Now that those are moved out of the loop in most cases, this warning is much
rarer.

Additional Expansion Logic
~~~~~~~~~~~~~~~~~~~~~~~~~~

After the parallel section location has been determined, the expansion logic is
slightly different for each intrinsic:

parallel_loop_
  is expanded by adding a parallel_region_entry to the preheader and a
  parallel_region_exit to the exit block of the loop and adding
  parallel_section_{entry,exit} instructions where indicated by the procedure
  above. If there are no subloops or memory-accessing instructions in the loop,
  a warning will be emitted as parallel_loop_ has no effects on loops with no
  memory accesses.

spmd_
  is expanded in the same way as parallel_loop_ but with no warning emitted if
  there are no memory accesses in the loop (as SPMDization is still useful for
  cloning non-memory loops). `spmdization.{entry,exit}`_ intrinsics are also
  added to the loop's preheader and exit block.

spmdization_
  uses the same logic as spmd_ but with a step to translate the SPMDization type
  string into a chunk size number first.

pipeline_loop_
  completely disregards the section location and just adds
  `pipeline.loop.{entry,exit}`_ intrinsics to the preheader and exit block of
  the loop.

Cleanup
~~~~~~~

After CLIE has finished looking at each loop, it removes every loop intrinsic in
the function. This includes intrinsics which weren't used for expanding loops,
since none of the later passes are going to handle them and we don't want them
sticking around until instruction selection.

.. _CSAIntrinsicCleaner:

CSAIntrinsicCleaner
-------------------

CSAIntrinsicCleaner is a simple pass that runs after all of the optimizations
just before instruction selection and that performs two major tasks: cleaning up
leftover internal intrinsics and checking for iteration-local storage.

Intrinsic Removal
~~~~~~~~~~~~~~~~~

CSAIntrinsicCleaner removes all of the `spmdization.{entry,exit}`_ and
`pipeline.loop.{entry,exit}`_ intrinsics added by CLIE_ or the OpenMP expansion
by locating entry intrinsics and recursively removing all of their users. This
method ensures that there are no dangling phi nodes left if any ended up being
inserted by other transformations.

Iteration-Local Storage Check
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We currently do not have any functionality for dealing with storage allocated
in an iteration-local scope in parallel loops that is not optimized out of
memory into lics, so what currently happens for those cases is that the same
allocation at the function level is used for every loop iteration and the loop
iterations step on each other and produce incorrect results. This is obviously
not an ideal failure mode and when this happens it isn't always obvious what is
going on either, so CSAIntrinsicCleaner mitigates this problem by checking for
loops that contain a lifetime_start intrinsic and also a parallel_section_entry
that is not part of a subloop. If such a loop is found, it emits this warning::

  !! WARNING: ITERATION-LOCAL STORAGE DETECTED IN A PARALLELIZED LOOP !!

This warning doesn't have any effect on code generation and can be disabled with
the ``-csa-disable-loop-storage-check`` flag.

Originally, this check was done in CLIE_ and would block the expansion of
parallel_loop_/spmdization_ intrinsics; it turns out that there are many useful
cases where iteration-local storage could be optimized out by the time that it
reached the backend, though, and this check was preventing users from taking
advantage of those. Because of this, the check was moved much later to where it
is now and was made optional. However, since it has been moved to a later pass
the false positive rate is much lower and the code produced when this warning is
emitted almost certainly suffers from the improper iteration-local storage
sharing issue.

The message that comes along with this warning suggests either moving the
storage allocation outside of the loop and manually ensuring that the iterations
don't touch the same storage or removing the parallel markings from the loop.
The warning also commonly arises in cases where the iteration-local storage was
expected to be implemented via lics rather than memory but wasn't because of
some deficiency in pass ordering. In those cases, double-compilation might be
an effective way of circumventing the issue or the issue might be resolvable by
dialing back the amount of iteration-local storage and loop unroll factors.
There are still other cases where lics can't be used and where allocating
outside of the loop is infeasible, and being able to parallelize these cases is
not possible yet but is an active area of work.
