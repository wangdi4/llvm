==================================
Optimization Reports in Xmain
==================================

.. contents::
   :local:

Introduction
============

This document describes the optimization report feature available in
xmain. The main goal of optimization report framework is to report all
transformations and optimizations that got triggered on loop nests in
a user friendly form.

The user-friendly form of opt reports means that we try hard to:

* Report loop nests in hierarchical manner in lexical order.

* Try to preserve optimized away loops (e.g. completely unrolled)
  in the loop hierarchy.

Other goals that were followed:

* HIR framework is the main phase with loop transformations, but
  it is not the only one. We must be able to add the relevant
  information into optimization reports outside HIR phase (before and after).

* Optimization report API for transformation developers must be
  as simple as possible.

The documentation is essentially split into three pieces: command line options
interface, API for transformation developers and the implementation details.

Get started with the command line interface
===========================================

For enabling optimization reports:

.. code-block:: console

   $ icx -qopt-report=[1, 2, 3]

This is the user-facing driver option for enabling optimization reports; it will
set the appropriate combination of the internal flags below for typical usage,
along with options to enable debug locations and the inlining/register
allocation reports. The argument is a numerical version of the verbosity below:
1 is low, 2 is medium, and 3 is high. The user-facing documentation for this
option is `here <https://www.intel.com/content/www/us/en/docs/dpcpp-cpp-compiler/developer-guide-reference/2023-2/qopt-report-qopt-report.html>`_.

For enabling optimization reports using internal options:

.. code-block:: console

   $ icx -mllvm -intel-opt-report=[none, low, medium, high]

* This will enable generation of optimization reports in the compiler. The
  value provided to the option is the verbosity of the opt reports.

  + **none**   -  Optimization reports are disabled.

  + **low**    -  Only emit positive remarks. In other words
    report only transformations that actually happened.

  + **medium** -  'low' plus emit negative remarks with a reason
    why transformation didn't happen.

  + **high**   -  'medium' plus all extra details.

* This option will not enable any user output. For that you need to use the
  option described below.

For specifying what kind of an output the user wants to have for loop
opt reports:

.. code-block:: console

   $ icx <...> -mllvm -intel-opt-report-emitter=[none, ir, hir, mir]

* **none** is useful when e.g. we would like to just have opt reports
  into LLVM IR text form after "-emit-llvm -S" and we don't want to
  have anything printed.

* **ir** is the standard emitter - ``"intel-ir-optreport-emitter"``.
  This pass is scheduled after post-LoopOpt VPO passes but before many of the
  community passes. If later passes would like to contribute information to opt
  reports it would be their responsibility to push the pass further down the
  pass manager pipeline. In most cases the user is expected to use this pass
  (though, see description of MIR emitter below).

* **mir** is another emitter - ``"intel-mir-optreport-emitter"``.
  This pass is scheduled at the end of MIR processing. We plan that this pass
  will eventually replace the IR emitter, but, first, we need to make sure that
  passes running between HIR CodeGen and binary/assembly emission preserve the
  optimization reports.

* **hir** output is implemented in a pass called ``"hir-optreport-emitter"``.
  This pass will print only loops that are in the HIR form and is located right
  before HIRCodeGen. HIR emitter is not expected to be hooked up from the driver
  and will serve developers purposes only. Since this pass is developer-only and
  not used by users, it's not recommended for testing use and LoopOpt tests
  should use ``intel-ir-optreport-emitter`` instead.

For specifying where optimization reports should be written (equivalent to the
external `-qopt-report-file <https://www.intel.com/content/www/us/en/docs/dpcpp-cpp-compiler/developer-guide-reference/2023-2/qopt-report-file-qopt-report-file.html>`_
option):

.. code-block:: console

   $ icx <...> -mllvm -intel-opt-report-file=[<filename>, stdout, stderr]

* **<filename>** writes optimization reports to the specified file.

* **stdout** writes optimization reports to stdout.

* **stderr** writes optimization reports to stderr. This is the default.

.. Note: vtune-compatible binary output should go there.

Let's take a look at the example of Opt Report.

.. code-block:: console

   $ cat foo.c

.. code-block:: c++

   void foo(int *restrict A, int *restrict B, int N, int *G) {
     for (int j = 0; j < N; ++j) {
       for (int i = 0; i < N; ++i) {
         B[i] = j;
       }
       *G += j;
       for (int i = 0; i < 3; ++i) {
         A[i] = j;
       }
     }
   }

.. code-block:: console

   $ icx -O3 -xCORE-AVX512 -qopt-report=1 -c foo.c

::

   Global optimization report for : foo

   LOOP BEGIN at foo.c (2, 2)

       LOOP BEGIN at foo.c (3, 4)
           remark #15300: LOOP WAS VECTORIZED
           remark #15305: vectorization support: vector length 8
       LOOP END

       LOOP BEGIN at foo.c (3, 4)
       <Remainder loop for vectorization>
       LOOP END

       LOOP BEGIN at foo.c (7, 4)
           remark #25436: Loop completely unrolled by 3
       LOOP END
   LOOP END

How to instrument your own transformation pass
==============================================

Setup
-----

Instrumenting your pass starts with adding headers:

.. code-block:: c++

   #include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h" // INTEL
   #include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"     // INTEL

The first include declares an immutable pass called ``OptReportOptionsPass``.
That is an important concept that allows your pass to know whether opt reports
are enabled or not and if yes, what is the current level of verbosity.

The second include declares the ``OptReportBuilder`` class, which is the bread
and butter of the optimization report framework. In essence, it allows you to
add remarks about your transformation to IR loops, HIR loops, OpenMP regions or
functions. For details about this class go the corresponding section in the
`Implementation Details`_, but for now let's concentrate on the API.

You first initialize the builder, usually it happens in ``run``:

.. code-block:: c++

   PreservedAnalyses MyPass::run(Function &F, FunctionAnalysisManager &AM) {

   #if INTEL_CUSTOMIZATION
     const OptReportOptions &OROP = AM.getResult<OptReportOptionsAnalysis>(F);
     ORBuilder.setup(F->getContext(), OROP.getOptReportVerbosity());
     // Where ORBuilder is of type OptReportBuilder and usually declared as a
     // pass class member.
   #endif  // INTEL_CUSTOMIZATION

Adding remarks
--------------

Now you are ready to go. The simplest way you can use it is to add a remark to
a loop:

.. code-block:: c++

   ORBuilder(*loop).addRemark(OptReportVerbosity::Low, OptRemarkID::LoopCompletelyUnrolled);

Note that as a transformation developer, your duty is to follow the convention
on remark verbosity levels:

* OptReportVerbosity::Low is the basic verbosity level. It only should
  include positive remarks about the transformations that actually got
  triggered. E.g. "loop got unrolled", "loop has been vectorized", etc.

* OptReportVerbosity::Medium includes negative remarks that explain
  why some transformation did not happen. E.g. "loop has not been
  vectorized because of the dependency".

* OptReportVerbosity::High includes the most verbose remarks, which are
  really extra details and usually the user does not have to know them.
  An example of such remark would be listing the dependency with src/dst
  expressions, which prevents vectorization.

``addRemark`` method explicitly takes the verbosity level as a first argument
as we would like to really make sure the developer understands with what
verbosity level to use the method.

If current verbosity level is lower than the remark verbosity, then this
remark will not be added to the report. Sometimes though creating and composing
proper remark messages is expensive and you would like to avoid that at all.
Then you can call the ``OptReportOptionsPass`` method
``isOptReportOn()`` to guard the code with remarks.

.. code-block:: c++

   if (OROP.isOptReportOn()) {
     // Code to generate remark messages
     ORBuilder.addRemark(...);
   }
   // Or it can be checked in a more precise way:
   if (OROP.getOptReportVerbosity() > OptReportVerbosity::Low) {
     // Code to generate remark messages
     ORBuilder.addRemark(...);
   }

Adding origins
--------------

There are special remarks that are called origins. They serve to tell
the user where the loop came from or what is the purpose of this loop.

.. code-block:: c++

   ORBuilder(*Loop).addOrigin(OptRemarkID::VectorizerRemainderLoop);

The method currently does not have verbosity checks.

You can stack multiple origins using addOrigin method.

Report preservation
-------------------

If after some transformation the loop is optimized away (e.g. completely
unrolled) and you want the user to still see the information about it in
the report, you should use this method:

.. code-block:: c++

   ORBuilder(*Loop).preserveLostOptReport();

You should call this method while the loop is not deleted, but after you
have already put all the necessary remarks into that.

**Important notice**: transformation developers have to be aware, that this
method removes the opt report from the loop and relocates it to a proper
place. This proper place is one of those, in this order:

1. The opt report of the previous sibling loop, if previous sibling loop
   exists. "Previous" means the loop that has the same nesting level and is
   located lexically before the current one. In this case the
   current opt report will be attached as "next sibling" of the
   previous loop.

2. The opt report of the parent loop, if parent loop exists. In this
   case the current opt report will be attached as "first child" of
   this parent loop.

3. Parent HIR region or function. Just as for previous case, the current
   opt report will be attached as "first child" of the region. Note that
   all reports attached to a region will be reattached to the corresponding
   function after HIRCodeGen.

Sometimes you would like to move opt reports from one entity to another.
E.g. you are vectorizing the loop and it is more convenient to create a
new loop from scratch to do that. After you created this new vectorized
loop, it makes sense to move all the opt report information from the
old loop to the new one:

.. code-block:: c++

  ORBuilder(*OrigLoop).moveOptReportTo(*NewLoop);

Let's say now that you would like to transform the original loop into the
vectorization remainder. Keep in mind that opt report information that
you previously moved with ``moveOptReportTo`` method may also contain
reports from optimized away loops, which can be stored as "next_sibling".
Since you don't want any reports be printed between main vectorized loop
and the remainder, you need to move all next siblings from main loop back
to the now remainder.

.. code-block:: c++

  ORBuilder(*NewLoop).moveSiblingsTo(*OrigLoop);


The syntactic sugar allows you to 'stack' methods:

.. code-block:: c++

  ORBuilder(*Loop).addRemark(OptReportVerbosity::Low, OptRemarkID::LoopCompletelyUnrolled)
                  .preserveLostOptReport();

Adding new messages to the message catalog
------------------------------------------

Optimization report messages are stored in a message catalog defined in
``llvm/include/llvm/Analysis/Intel_OptReport/Diag.h`` and
``llvm/lib/Analysis/Intel_OptReport/Diag.cpp``. To add a new message, first
assign it a remark id by adding it to the ``OptRemarkID`` enum in ``Diag.h``:

.. code-block:: c++

  enum class OptRemarkID {

    LoopCompletelyUnrolled = 25508,

Remark ids are organized into blocks based on component; new messages are
usually added at the end of their component's block, but if there are related
messages that have numbering gaps it can make sense to add messages there
instead. The most important thing is to ensure that the remark id numbers are
unique in the enum.

The next step of adding a new message is to add a format string for it in the
``OptReportDiag::Diags`` table in ``Diag.cpp``:

.. code-block:: c++

  const DenseMap<DiagTableKey, const char *> OptReportDiag::Diags = {

      {OptRemarkID::LoopCompletelyUnrolled, "Loop completely unrolled"},

This table lists formatting strings for every message that's in use (which are
all represented in ``OptRemarkID``) as well as messages that were part of the
classic icc message catalog which are identified by plain numbers if they
haven't been reused in xmain yet:

.. code-block:: c++

      {15302, "routine skipped: vectorization disabled due to -mno-sse, "
              "-mno-sse2, -mia32, and/or -no-vec flag"},

If one of these messages is equivalent to the one you're looking to add, it can
be reused by assigning its number in ``OptRemarkID`` and updating the table to
use the ``OptRemarkID`` value instead of the raw number. This is preferable to
adding a new message to the catalog because it keeps the remark ids more
consistent with icc.

Format strings
--------------

Optimization report messages support limited printf-style formatting:

.. code-block:: c++

    {OptRemarkID::OpenMPConstructUserIgnored,
     "Construct %d (%s) ignored at user's direction"},

Two ``%`` format specifiers are supported:

* ``%d`` is substituted with an integer value using decimal notation.
* ``%s`` is substituted with a string value.

As in printf, ``%%`` can be used to add a literal ``%`` to the message. Any
other character following ``%`` is invalid and will trigger an assert.

The values substituted for these specifiers are passed as extra arguments to
``addRemark`` or ``addOrigin``:

.. code-block:: c++

  const unsigned Number = W->getNumber();
  const StringRef Name = W->getName();
  ORBuilder(*W, WRegionList).addRemark(OptReportVerbosity::Medium,
    OptRemarkID::OpenMPConstructUserIgnored, Number, Name
  );

In the final optimization report, the printed message will look like this:

::

  OMP CRITICAL BEGIN
      remark #30009: Construct 4 (critical) ignored at user's direction
  OMP CRITICAL END

``AtLine``
^^^^^^^^^^

``AtLine`` is a special wrapper that can be used to add the substring
``" at line <N>"`` to messages as a substitution for ``%s``:

.. code-block:: c++

  // Diag.cpp:
  {OptRemarkID::InvariantConditionHoisted,
   "Invariant Condition%s hoisted out of this loop"},

  // In the transform:
  const unsigned Line = getOptReportLine(Cond);
  ORBuilder(*L, *LI).addRemark(OptReportVerbosity::Low,
                               OptRemarkID::InvariantConditionHoisted,
                               AtLine(Line));

  // Printed like this in the report:
  // remark #25422: Invariant Condition at line 61 hoisted out of this loop

AuxDiag
^^^^^^^

Some messages include parts that can be substituted with different phrases.
Instead of hard-coding these phrases, we have a second AuxDiag message catalog
to better support translation. Similar to the main message catalog, the AuxDiag
message catalog has substrings identified by ``AuxRemarkID`` and defined in
``Diag.h`` and ``Diag.cpp``:

.. code-block:: c++

  // Diag.h:
  enum class AuxRemarkID {

    Loop,
    SimdLoop,

  // Diag.cpp:
  const DenseMap<AuxRemarkID, const char *> OptReportAuxDiag::AuxDiags = {

      {AuxRemarkID::Loop, "loop"},
      {AuxRemarkID::SimdLoop, "simd loop"},

``AuxRemarkID`` values can be passed directly into ``addRemark`` or
``addOrigin`` to substitute for ``%s`` in a message format string:

.. code-block:: c++

  // Diag.cpp:
  {OptRemarkID::VecFailBadlyFormedMultiExitLoop,
   "%s was not vectorized: loop with multiple exits cannot be "
   "vectorized unless it meets search loop idiom criteria"},

  // In the transform:
  const AuxRemarkID LoopOrSimd = WRLp && WRLp->isOmpSIMDLoop()
    ? AuxRemarkID::SimdLoop : AuxRemarkID::Loop;
  ORBuilder(*Lp, *LI).addRemark(OptReportVerbosity::Medium,
    OptRemarkID::VecFailBadlyFormedMultiExitLoop, LoopOrSimd
  );

  // Printed like this in the report:
  // remark #15520: simd loop was not vectorized: loop with multiple exits cannot be vectorized unless it meets search loop idiom criteria

Unit testing
------------

Every transform that adds optimization report remarks should have checks in
at least one of its unit tests to make sure the remarks are being emitted as
expected. In most cases these checks can be added to existing unit tests that
are already checking the results of the transform. There are many ways to add
these checks; this is an example of best practices demonstrated with a test for
the LLVM LoopUnroll remarks:

.. code-block:: llvm

  ; RUN: opt < %s -S -passes=loop-unroll -pass-remarks=loop-unroll -unroll-count=16 2>&1 | FileCheck -check-prefix=COMPLETE-UNROLL %s
  ; RUN: opt < %s -S -passes=loop-unroll -pass-remarks=loop-unroll -unroll-count=4 2>&1 | FileCheck -check-prefix=PARTIAL-UNROLL %s
  ; RUN: opt < %s -S -passes=loop-unroll -pass-remarks=loop-unroll -unroll-count=4 -unroll-runtime=true -unroll-remainder 2>&1 | FileCheck %s --check-prefix=RUNTIME-UNROLL

  ; INTEL_CUSTOMIZATION
  ; RUN: opt -passes='loop-unroll,intel-ir-optreport-emitter' -unroll-count=16 -intel-opt-report=low -disable-output -intel-opt-report-file=stdout < %s | FileCheck -check-prefixes=OPTREPORT,OPTREPORT-COMPLETE-UNROLL %s
  ; RUN: opt -passes='loop-unroll,intel-ir-optreport-emitter' -unroll-count=4 -intel-opt-report=low -disable-output -intel-opt-report-file=stdout < %s | FileCheck -check-prefixes=OPTREPORT,OPTREPORT-PARTIAL-UNROLL %s
  ; end INTEL_CUSTOMIZATION

  ; COMPLETE-UNROLL: remark: {{.*}}: completely unrolled loop with 16 iterations
  ; PARTIAL-UNROLL: remark: {{.*}}: unrolled loop by a factor of 4
  ; RUNTIME-UNROLL: remark: {{.*}}: unrolled loop by a factor of 4

  ; INTEL_CUSTOMIZATION
  ; OPTREPORT: Global optimization report for : sum

  ; OPTREPORT-COMPLETE-UNROLL: LOOP BEGIN
  ; OPTREPORT-COMPLETE-UNROLL:     remark #25603: Loop has been completely unrolled by LLVM LoopUnroll
  ; OPTREPORT-COMPLETE-UNROLL: LOOP END

  ; OPTREPORT-PARTIAL-UNROLL: LOOP BEGIN
  ; OPTREPORT-PARTIAL-UNROLL:     remark #25604: Loop has been partially unrolled with factor 4 by LLVM LoopUnroll
  ; OPTREPORT-PARTIAL-UNROLL: LOOP END
  ; end INTEL_CUSTOMIZATION

The important options to set in the new RUN line are:

* ``intel-ir-optreport-emitter`` needs to be added to the end of the ``-passes``
  option to print the report. In LoopOpt, ``hir-cg`` and ``simplifycfg`` should
  also be added ahead of ``intel-ir-optreport-emitter`` to generate the HIR
  loops in IR and to remove the dead original loops so they don't show up in the
  report. ``hir-optreport-emitter`` is not recommended for testing because it's
  not user-visible.
* ``-intel-opt-report`` needs to be set to enable adding the remarks. The
  example above uses ``-intel-opt-report=low`` to ensure that the added
  low-verbosity remarks are printed at low verbosity; if the remarks to check
  are medium or high verbosity, ``-intel-opt-report=medium`` or
  ``-intel-opt-report=high`` should be used instead.
* ``-disable-output -intel-opt-report-file=stdout`` disables IR output and
  writes optimization report output to stdout so it can be piped directly into
  FileCheck. This could also be done by writing the output to a temporary file
  or by writing it to stderr and redirecting it to stdout to pipe to FileCheck;
  writing to stdout is just a cleaner way of doing this.
* ``-check-prefix=OPTREPORT`` sets an opt-report specific check prefix so that
  the opt-report checks don't interfere with the existing checks in the unit
  test.

Implementation details
======================

The structure of optimization reports
-------------------------------------

The optimization reports should be considered as an opaque metadata attached to
loops, OpenMP work regions and functions. This metadata gets incrementally
updated as optimizations are run. After all the interesting optimizations an
emitter pass is scheduled. It traverses the code and prints found optimization
reports in hierarchical order. That is, generally, the reports are not linked,
and one needs to traverse the code to find them. However, this doesn't work well
for reporting information about loops that got optimized away.
To support such cases, two additional concepts are added to optimization
reports: children and siblings. When a loop is optimized away, we attach its
report either to the previous sibling loop or to the parent
loop/region/function. When printing reports, all children reports are printed as
nested loops, and all sibling loops are printed right after the current loop at
the same nesting level.

It's the responsibility of a transformation developer to do his best to produce
optimization reports that will look reasonable even if the optimized loop has
children or sibling optimization reports.

The format of metadata
----------------------

Optimization reports are stored within LLVM Metadata using
special convention. Here is its format:

::

  OPTREPORT_NODE := <!"intel.optreport">, (DEBUG_LOC_NODE), (ORIGIN_NODE), (REMARKS_NODE), (FIRST_CHILD_NODE), (NEXT_SIBLING_NODE)
  DEBUG_LOC_NODE := <!"intel.optreport.debug_location">, <*DILocation>
  TITLE_NODE := <!"intel.optreport.title">, <string>
  ORIGIN_NODE := <!"intel.optreport.origin">, <REMARK>, (REMARK), ..., (REMARK)
  REMARKS_NODE := <!"intel.optreport.remarks">, <REMARK>, (REMARK), ..., (REMARK)
  FIRST_CHILD_NODE := <!"intel.optreport.first_child">, <PROXY_OPTREPORT_NODE>
  NEXT_SIBLING_NODE := <!"intel.optreport.next_sibling">, <PROXY_OPTREPORT_NODE>
  REMARK := <!"intel.optreport.remark">, <remark-id>, (arg0), ..., (argN)


* Mandatory fields are denoted in angle brackets '<', '>'.

* Optional operands are denoted in parenthesis '(', ')'.

* All nodes are represented as MDTuple.

* ``OPTREPORT_NODE`` is always distinct, and all other nodes are uniqued.

Here is the loop metadata for first inner loop from example in the intro:

.. code-block:: llvm

  !20 = !DILocation(line: 3, column: 4, scope: !6)
  !21 = distinct !{!21, !20, !22, !23, !24, !25, !26, !27}
  !22 = !DILocation(line: 5, column: 4, scope: !6)
  !23 = !{!"llvm.loop.mustprogress"}
  !24 = !{!"llvm.loop.vectorize.width", i32 1}
  !25 = !{!"llvm.loop.interleave.count", i32 1}
  !26 = !{!"llvm.loop.unroll.disable"}
  !27 = distinct !{!"intel.optreport", !28, !35}      <== OPT_REPORT_NODE
  !28 = !{!"intel.optreport.debug_location", !20}     <== DEBUG_LOC_NODE
  !35 = !{!"intel.optreport.remarks", !36, !37}       <== REMARKS_NODE
  !37 = !{!"intel.optreport.remark", i32 15300}       <== REMARK_NODE
  !38 = !{!"intel.optreport.remark", i32 15305, !"8"} <== REMARK_NODE

``OptReportBuilder``
------------------------

``OptReportBuilder`` is the main entry point for generating optimization
reports, and at first it is the only class visible to user. However, under the
hood it uses several other classes. First of all, ``OptReportBuilder`` itself
doesn't provide any methods to manipulate optimization reports. Instead, its
``operator()`` returns a transient instance of template ``OptReportThunk<T>``
class, which provides access to optimization report of a specific loop or OpenMP
work region, and it has an extensive set of supported operations for that.
``OptReportThunk`` is mostly implemented in a generic (type-agnostic) way.
However, obviously, not all operations can be expressed in a generic way. The
minimal set of such type-specific operations is incapsulated into template
``OptReportTraits<T>`` class. It doesn't have a default implementation, and each
supported class should provide its own specialization of this template class.
At the moment of writing, the specializations are provided for the following
classes:

* ``llvm::Opt::HLLoop``
* ``llvm::Opt::HLRegion``
* ``llvm::Loop``
* ``llvm::Function``
* ``llvm::vpo::WRegionNode``

``OptReport``
-----------------

``OptReport`` class, obviously, represents an optimization report and is
intended to hide details of how optimization reports are represented in
metadata.

``OptReport`` is a lightweight (pass it by value) wrapper for a pointer to
actual metadata representation. It can be initialized with a pointer (possibly,
with ``nullptr``) and it can be explicitly converted to ``bool``. All the
necessary functionality to manipulate optreport metadata is exposed through
``OptReport`` API, and a user shouldn't fiddle with metadata himself.
