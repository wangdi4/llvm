=========================
Divergence Analysis
=========================

.. contents::
   :local:

Divergence Analysis is the analysis that determines how values change within a
vector. Currently, DA does not take vector length into account, but could be
extended to do so. The terminology used to describe how these values change
is known as "shapes". Generally speaking, there are 3 different type of shapes
that DA computes. They are:

#. Uniform - all values within a vector are identical. E.g., ``%v = <0, 0, 0, 0>``.
#. Strided - all values form a linear sequence with a step value. Stride (step)
   must be a constant, but can be known or unknown at compile time. Some
   examples are:

   * unit-stride - all values within a vector have step 1, e.g.,
     ``%v = <0, 1, 2, 3>``.
   * unknown strided - e.g., ``%v = <%0, %0 + %s, %0 + 2 * %s, %0 + 3 * %s>``,
     where ``%s`` is some unknown uniform value representing the stride. When
     printing shapes, the '?' character is used to indicate unknown stride.

#. Random - all values represent some unknown pattern or there isn't a defined
   relationship between them. For example:

   * ``%v = <%0, %1, %2, %3>``
   * ``%v = <2, 1, 5, 3>``

.. TODO: For now any links to other analyses/transforms go to the section
.. heading in the main directory until that particular documentation is ready.
.. E.g., `SOAAnalysis` points to the directory entry. Later, this should
.. point to the specific SOAAnalysis document.

There are additional shapes defined for :ref:`SOA analysis` that are
separate from the shapes described above, but the concept is similar and based
on the particular data layout for SOA.

The divergence analysis terminology is derived from an open source project
worked on by Saarland University (https://github.com/cdl-saarland/rv). It can
really be thought of as a kind of uniformity analysis or linearity analysis.
The most fundamental purpose of DA is to propagate shapes to a value's users,
some of which can be memory references, which provides valuable information
such as whether or not loads/stores are unit-stride. As such, this information
is crucial for application performance since it can prevent the generation of
gather/scatter intrinsics. DA results are used by :ref:`SVA-label` (a.k.a. SVA)
to help determine the context in which the value is used. For instance, if a
value is known to be uniform and users of that instruction are scalar, SVA can
mark the values in a way that tells the code generators that a broadcast to a
vector is not needed, thus improving the quality of code generated from the
vectorizer. For similar reasons, the cost model uses DA to get a more accurate
assessment of what types of instructions will be generated in the code
generators (e.g., unit-stride load stores). Therefore, DA helps the cost model
become more accurate and match what the code generators output. In addition, DA
results are used by a handful of other VPlan analyses and transformations,
including:

#. :ref:`AZB-label` (a.k.a, AZB), which finds divergent block-predicates to
   know where to insert bypasses.
#. :ref:`CallVecDecisions-label`, which uses shapes to match caller side with
   callee side simd function signatures.
#. :ref:`LoopCFU-label`, which checks to see if an innermost loop exit
   condition is divergent in outerloop vectorization scenarios so that it can
   transform it into a uniform (masked) loop.
#. :ref:`VConflictTransform-label`, which makes use of operand shapes of the
   reduction-like operation that is part of the idiom to know how to
   differentiate between idioms. For example, the histogram idiom has a uniform
   operand, whereas tree conflict has a divergent operand.

The VPlan implementation no longer follows the existing open source
implementation as closely since many VPlan-specific optimizations have now been
applied to the algorithm. One main difference, made for stability purposes, is
the assumption that all values are undefined by default and that all shapes must
be defined before continued processing of the VPlan. The community version,
called UniformityAnalysis, propagates divergence properties through the CFG and
begins by assuming that all values are uniform. Another difference is the use
of a legacy version of ``SyncDependenceAnalysis`` (SDA) that has been dropped
by the community. The main difference is that the community version supports
irreducible control flow. This is not necessary for vectorization since we rely
on single entry/single exit code regions, and the original algorithm is still
known to be correct.  However, if a reason is found where it can benefit VPlan,
there should not be any limiting factors to adopting it. SDA is described
further below. In addition, the SOA shape propagation is specific to VPlan, as
are the recent updates to DA to take overflow and :ref:`ValueTracking-label`
results into consideration for more accurate (albeit conservative) results.
There have been many other smaller bug fixes/improvements made based on the
VPlan framework, as well as verification that all values have shapes after the
analysis has run. We have an HIR-specific optimization for memory references
that helps determine unit-strideness of loads/stores and was necessary for
performance reasons. Eventually, the goal is to remove such code from DA as it
is meant to be IR-independent.

Divergence Analysis is invoked in 3 ways:

1. During construction of a VPlan, the compute() method is called.
2. Recomputed from other analyses/transformations that wish to do a full update
   (e.g., :ref:`MaskedModeLoopTransform-label`) through the recomputeShapes()
   method.
3. Transformations that introduce new instructions can compute DA shapes for
   that specific value and its users through the updateVectorShape() interface.

For a full compute or recompute, all VPlan instructions are pushed to a
worklist in reverse post-order due to the fact that in most cases we want to be
able to compute the shape of a definition before its use. In the case of a
partial recomputation, the entire "seed" list is pushed to DA's work list, and
the shapes are computed for all of the "seeds" and their users. For special
instructions like a recurrent phi, we delay the shape computation and push its
users to the worklist so that their shapes will fully be made available for the
phi.  Eventually, the update to the recurrence will be encountered and the phi
shape will be "back-propagated". Non-recurrent phi shapes are propagated by
using SDA. SDA computes all disjoint paths from the block containing the
divergent condition to the block where these paths merge to determine those
phis that become divergent. As an example, if we have an if/else statement
where the results of the condition causes the execution for some lanes to go
down the if part and other lanes to go down the else part, then the phi
encountered at the "join point" will be marked as divergent and recorded as
such. In the example in Figure 1, DA is able to compute that ``%vp14270`` is a
divergent comparison (e.g., ``a[i] < N-10``) and then computes the join point
as ``BB6``. ``%vp14994`` and all of its users will be marked as divergent. SDA
also has the capability of computing disjoint paths out of loops, but an example
of that was excluded for brevity.

Figure 1. Simple VPlan loop containing divergent if condition
::

 External Defs Start:
   %vp57680 = {%N + -10}
 External Defs End:

 BB3: # preds: BB2, BB6
  i64 %vp9010 = phi  [ i64 %vp35880, BB2 ],  [ i64 %vp13562, BB6 ]
  ptr %vp12074 = subscript inbounds ptr %i.linear.iv
  ptr %vp14206 = subscript inbounds ptr %a i64 %vp9010
  i32 %vp14330 = load ptr %vp14206
  i1 %vp14270 = icmp slt i32 %vp14330 i32 %vp57680
  br i1 %vp14270, BB4, BB5

  BB5: # preds: BB3
   ptr %vp14800 = subscript inbounds ptr %b i64 %vp9010
   i32 %vp9414 = load ptr %vp14800
   i32 %vp13594 = sdiv i32 %vp9414 i32 %N
   br BB6

  BB4: # preds: BB3
   i32 %vp13722 = add i32 %N i32 %vp14330
   br BB6

 BB6: # preds: BB4, BB5
  i32 %vp14994 = phi  [ i32 %vp13722, BB4 ],  [ i32 %vp13594, BB5 ]
  ptr %vp15508 = subscript inbounds ptr %c i64 %vp9010
  store i32 %vp14994 ptr %vp15508
  ptr %vp15628 = subscript inbounds ptr %i.linear.iv
  i64 %vp13562 = add i64 %vp9010 i64 %vp16462
  i1 %vp15906 = icmp slt i64 %vp13562 i64 %vp13536
  br i1 %vp15906, BB3, BB7

The results computed by DA can be thought of in two levels of resolution. The
first level is that shapes can simply be thought of as uniform or divergent.
The second level is the refinement of shapes for divergent values. For instance,
we can say that a value is divergent but has stride of 1, or that a value is
divergent but has a random set of values. In some cases, the first level of
information is enough for analyses and transformations to do their job.
AZB, for example, only needs to know that a condition was divergent to insert
bypasses. From the SDA point of view this is sufficient since it is up to the
main DA shape propagation algorithm to further refine the shapes of divergent
values. Figure 2 shows an example of the worklist-based shape propagation order
of processing.

Figure 2. (DA processing example for Figure 1.)
::

 Computing shape for[DA: [Shape: Undef]] i64 %vp9010 = phi  [ i64 %vp35880, BB2 ],  [ i64 %vp13562, BB6 ] Result:[Shape: Unit Stride, Stride: i64 1]
 Computing shape for[DA: [Shape: Undef]] ptr %vp12074 = subscript inbounds ptr %i.linear.iv Result:[Shape: Uniform]
 Computing shape for[DA: [Shape: Undef]] ptr %vp14206 = subscript inbounds ptr %a i64 %vp9010 Result:[Shape: Strided, Stride: i64 4]
 Computing shape for[DA: [Shape: Undef]] i32 %vp14330 = load ptr %vp14206 Result:[Shape: Random]
 Computing shape for[DA: [Shape: Undef]] i1 %vp14270 = icmp slt i32 %vp14330 i32 %vp57680 Result:[Shape: Random]
 Computing shape for[DA: [Shape: Undef]] br i1 %vp14270, BB4, BB5 Result:[Shape: Random]
 *propBranchDiv BB3
 Computing shape for[DA: [Shape: Undef]] ptr %vp14800 = subscript inbounds ptr %b i64 %vp9010 Result:[Shape: Strided, Stride: i64 4]
 Computing shape for[DA: [Shape: Undef]] i32 %vp9414 = load ptr %vp14800 Result:[Shape: Random]
 Computing shape for[DA: [Shape: Undef]] i32 %vp13594 = sdiv i32 %vp9414 i32 %N Result:[Shape: Random]
 Computing shape for[DA: [Shape: Undef]] br BB6 Result:[Shape: Uniform]
 Computing shape for[DA: [Shape: Undef]] i32 %vp13722 = add i32 %N i32 %vp14330 Result:[Shape: Random]
 Computing shape for[DA: [Shape: Undef]] br BB6 Result:[Shape: Uniform]
 Computing shape for[DA: [Shape: Undef]] i32 %vp14994 = phi  [ i32 %vp14548, BB4 ],  [ i32 %vp14854, BB5 ] Result:[Shape: Random]
 Computing shape for[DA: [Shape: Undef]] ptr %vp15508 = subscript inbounds ptr %c i64 %vp9010 Result:[Shape: Strided, Stride: i64 4]
 Computing shape for[DA: [Shape: Undef]] store i32 %vp14994 ptr %vp15508 Result:[Shape: Strided, Stride: i64 4]
 Computing shape for[DA: [Shape: Undef]] ptr %vp15628 = subscript inbounds ptr %i.linear.iv Result:[Shape: Uniform]
 Computing shape for[DA: [Shape: Undef]] i64 %vp13562 = add i64 %vp9010 i64 %vp16462 Result:[Shape: Unit Stride, Stride: i64 1]
 Computing shape for[DA: [Shape: Undef]] i1 %vp15906 = icmp slt i64 %vp13562 i64 %vp16604 Result:[Shape: Uniform]
 Computing shape for[DA: [Shape: Undef]] br i1 %vp15906, BB3, BB7 Result:[Shape: Uniform]
 **Computing shape for[DA: [Shape: Unit Stride, Stride: i64 1]] i64 %vp9010 = phi  [ i64 %vp35880, BB2 ],  [ i64 %vp13562, BB6 ] Result:[Shape: Unit Stride, Stride: i64 1]

 * propBranchDiv BB3 is the point at which SDA has computed that disjoint paths
   exist from BB3 to BB6.

 ** Note that for the recurrent phi %vp9010 the initial shape computed is unit
    stride due to one incoming shape being unit stride. However, it's not until
    %vp13562 is visited that the final shape is computed.
