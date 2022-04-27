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

   $ icx -mllvm -intel-opt-report=[none, low, medium, high]

* This will enable generation of optimization reports in the compiler. The
  value provided to the option is the verbosity of the opt reports.

  + **none**   -  Optimization reports are disabled.

  + **low**    -  Only emit positive remarks. In other words
    report only transformations that actually happened.

  + **medium** -  'low' plus emit negative remarks with a reason
    why transformation didn't happen.

  + **high**   -  'medium' plus all extra details.

* This option will not enable any user output (in stderr). For that you need to
  use the option described below.

For specifying what kind of an output the user wants to have for loop
opt reports:

.. code-block:: console

   $ icx <...> -mllvm -intel-opt-report-emitter=[none, ir, hir, mir]

* **none** is useful when e.g. we would like to just have opt reports
  into LLVM IR text form after "-emit-llvm -S" and we don't want to
  have anything printed.

* **ir** is the standard emitter - ``"intel-ir-optreport-emitter"``.
  It prints information to stderr. This pass is scheduled right after
  HIR CodeGen. If later passes would like to contribute information to
  opt reports it would be their responsibility to push the pass further
  down the pass manager pipeline. In most cases the user expected to use
  this pass (though, see description of MIR emitter below).

* **mir** is another emitter - ``"intel-mir-optreport-emitter"``.
  It prints information to stderr. This pass is scheduled at the end
  of MIR processing. We plan that this pass will eventually replace
  the IR emitter, but, first, we need to make sure that passes
  running between HIR CodeGen and binary/assembly emission preserve
  the optimization reports.

* **hir** output implemented in a pass called ``"hir-optreport-emitter"``,
  it will print (in stderr) only loops that are in the HIR form and is
  located right before HIRCodeGen. HIR emitter is not expected to be
  hooked up from the driver and will serve developers purposes only.

Note: vtune-compatible binary output should go there.

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

   icx ~/foo.c -mllvm -intel-opt-report=low -mllvm -intel-opt-report-emitter=ir -std=c99 -emit-llvm -S -g -O3

::

   Global optimization report for : foo

   LOOP BEGIN at /user/aivchenk/foo.c (3, 3)
       Remark #XXXXX: Loop has been unswitched via cmp230

       LOOP BEGIN at /user/aivchenk/foo.c (4, 5)
           Remark #XXXXX: Loop has been vectorized with vector 4 factor
       LOOP END

       LOOP BEGIN at /user/aivchenk/foo.c (4, 5)
           <Remainder loop for vectorization>
       LOOP END

       LOOP BEGIN at /user/aivchenk/foo.c (8, 5)
           Remark #XXXXX: Loop completely unrolled
       LOOP END
   LOOP END


How to instrument your own transformation pass
==============================================

Instrumenting your pass starts with adding headers:

.. code-block:: c++

   +#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h" // INTEL
   +#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"     // INTEL

The first include declares immutable pass called ``OptReportOptionsPass``.
That is an important concept that allows your pass to know whether opt reports
are enabled or not and if yes, what is the current level of verbosity. Hence,
you should add this pass as a required one in getAnalysisUsage

.. code-block:: c++

   AU.addRequired<OptReportOptionsPass>(); // INTEL

   // and don't forget to initialize the dependency:

   INITIALIZE_PASS_DEPENDENCY(OptReportOptionsPass) // INTEL

The second include declares ``OptReportBuilder`` class, which is a bread and
butter of optimization report framework. In essence, it allows you to add
remarks about your transformation to IR loops, HIR loops, OpenMP regions or
functions. For details about this class go the corresponding section in the
`Implementation Details`_, but for now let's concentrate on the API.

You first initialize the builder, usually it happens in runOn[Function,Loop]:

.. code-block:: c++

  auto &OROP = getAnalysis<OptReportOptionsPass>();
  ORBuilder.setup(F->getContext(), OROP.getOptReportVerbosity());
  // Where ORBuilder is of type OptReportBuilder and usually declared as a
  // pass class member.

Now you are ready to go. The simplest way you can use it is to add a remark to
a loop:


.. code-block:: c++

   ORBuilder(*loop).addRemark(OptReportVerbosity::Low, "Loop completely unrolled");

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
     ORBuilder.addRemark(RemarkMsg);
   }
   // Or it can be checked in a more precise way:
   if (OROP.getOptReportVerbosity() > OptReportVerbosity::Low) {
     // Code to generate remark messages
     ORBuilder.addRemark(RemarkMsg);
   }


There are special remarks that are called origins. They serve to tell
the user where the loop came from or what is the purpose of this loop.

.. code-block:: c++

   ORBuilder(*Loop).addOrigin("Remainder loop for vectorization");

The method currently does not have verbosity checks.

You can stack multiple origins using addOrigin method.


If after some transformation the loop is optimized away (e.g. completely
unrolled) and you want the user to still see the information about it in
the report, you should use this method:

.. code-block:: c++

   ORBuilder(*Loop).preserveLostOptReport();

You should call this method while the loop is not deleted, but after you
have already put all the necessary remarks into that.

Note: The preserveLostOptReport method is only implemented for HLLoops as
of now.

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

  ORBuilder(*Loop).addRemark(OptReportVerbosity::Low,
                             "Loop completely unrolled")
                   .preserveLostOptReport();


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

  ROOT_NODE := <!"intel.optreport.rootnode">, <PROXY_OPTREPORT_NODE>
  PROXY_OPTREPORT_NODE := <!"intel.optreport">, (DEBUG_LOC_NODE), (ORIGIN_NODE), (REMARKS_NODE), (FIRST_CHILD_NODE), (NEXT_SIBLING_NODE)
  DEBUG_LOC_NODE := <!"intel.optreport.debug_location">, <*DILocation>
  TITLE_NODE := <!"intel.optreport.title">, <string>
  ORIGIN_NODE := <!"intel.optreport.origin">, <REMARK>, (REMARK), ..., (REMARK)
  REMARKS_NODE := <!"intel.optreport.remarks">, <REMARK>, (REMARK), ..., (REMARK)
  FIRST_CHILD_NODE := <!"intel.optreport.first_child">, <PROXY_OPTREPORT_NODE>
  NEXT_SIBLING_NODE := <!"intel.optreport.next_sibling">, <PROXY_OPTREPORT_NODE>
  REMARK := <!"intel.optreport.remark">, <remark-id>, <formatted string>, (arg0), ..., (argN)


* Mandatory fields are denoted in angle brackets '<', '>'.

* Optional operands are denoted in parenthesis '(', ')'.

* All nodes are represented as MDTuple.

* ``ROOT_NODE`` is always distinct, as we need that to be unique for
  each node to allow safe replacement of the proxy node in it.

* ``PROXY_OPTREPORT_NODE`` is needed so the root node is never
  invalidated (`OptReport`_ class description contains more
  details on that).

* ``PROXY_OPTREPORT_NODE`` is distinct only if it has any optional operands.

 Here is the loop metadata for first inner loop from example in the intro:

.. code-block:: llvm

  !51 = distinct !{!51, !52, !53}
  !52 = !{!"llvm.loop.unroll.disable"}
  !53 = distinct !{!"intel.optreport.rootnode", !54}  <== ROOT_NODE
  !54 = distinct !{!"intel.optreport", !55, !56}      <== PROXY_OPT_REPORT_NODE
  !55 = !{!"intel.optreport.debug_location", !50}     <== DEBUG_LOC_NODE
  !56 = !{!"intel.optreport.remarks", !57}            <== REMARKS_NODE
  !57 = !{!"intel.optreport.remark", i32 XXXXX, !"*vectorized with vect. %d fact.", i32 4}
                                                      ^== REMARK_NODE

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
