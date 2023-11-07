=================================================
Interprocedural optimization report via metadata
=================================================

.. contents::
   :local:

Introduction
=============

This document describes the implementation of the interprocedural optimization
report based on metadata.

In order to preserve intermediate results through multiple interprocedural
optimizations and compilation stages, it was decided to use metadata as
the main information storage. Metadata is handled as part of the LLVM IR which
makes it possible to accumulate information during whole compilation process.

Note: The current implementation covers only the function inlining report.

Developer interfaces
====================

Since metadata is attached directly to call instructions and functions,
interface methods can be used to update it on the fly. Currently there are
a number of interfaces that are available outside of inlining phase: methods
from setMDReasonIsInlined() and setMDReasonNotInlined() groups.

Setup pass
==========

Depending on the optimization level and options used for compilation and link,
inliner pass could appear in the pass pipeline more than once. There should be
an opportunity to switch between individual inlining report for each inliner
pass and a composite report for the whole inlining that ever happened for
a particular function. There are two main issues that need to be resolved if
we need to keep inline report consistent between two inlining passes:

* The inlining report should follow IR changes;
* The inlining report nodes should have up-to-date information and be properly
  attached to the IR before inliner pass.

Thus a new inline report setup pass was introduced. It conducts two functions.
First, if there are no inline report metadata in the IR yet, it constructs
basic metadata nodes and attaches them to the IR. The setup and inliner passes
share InlineReportBuilder object so callback vector could be filled up early
and help keep track of instruction/function deletions. Second, if the inline
report metadata is in the IR (like in the case of second inlining pass during
LTO link step), setup pass does verification, updates current inline report
with new callsites which could have appeared, and links all inlining call site
metadata nodes back to the corresponding instructions if they were unlinked
during IR processing. Note, if verification fails for some function, a new
'from scratch' inlining report would be created for it.

Emitter pass
============

Emitter pass walks over module inline report metadata and outputs inline report
in a user-friendly form. The format of report depends on the inline report
level option.

Updating inlining report for functions and call instructions deleted outside of Inliner pass
============================================================================================

After a consistent inlining report is created by Inliner pass, it is possible
that some other optimization could delete call instruction or the whole function.
In order to keep the inlining report consistent we use a callback mechanism.
InlineReportBuilder object is responsible for storing a callback vector for
functions and instructions of interest during optimizations following
the Inliner pass.

Metadata representation
=======================

The root node of the IPO report is a named metadata node called
'intel.module.inlining.report'. This metadata node stores a list of
the metadata IPO reports for each function in the module.

Function IPO report contains metadata tag 'intel.function.inlining.report',
function name, list of initial call site reports, module name, 'is dead' and
'is declaration' flags and linkage character. If function is not dead,
the metadata node is also attached to the function.

Call site report consists of 'intel.callsite.inlining.report' tag, callee name,
list of report for call sites that are result of the inlining of the current
call site(if any), 'is inlined' flag, inlining reason number,
inline, outer inline and early exit inline costs, inline and early exit inline
thresholds, debug location information.

Example
=======

* File test1.c:

.. code-block:: c++

   extern void z();
   extern void b();
   extern void x();

   static void y() {
       z();
   }

   void a() {
       x();
       y();
   }

   int main() {
      a();
      b();
      a();
      return 0;
   }


* After setup pass:

.. code-block:: llvm

   ; ModuleID = 'test1.c'

   define dso_local void @a() !intel.function.inlining.report !8 {
   entry:
     call void (...) @x(), !intel.callsite.inlining.report !11
     call void @y(), !intel.callsite.inlining.report !21
     ret void
   }

   declare !intel.function.inlining.report !27 dso_local void @x(...)

   define dso_local i32 @main() !intel.function.inlining.report !29 {
   entry:
     call void @a(), !intel.callsite.inlining.report !32
     call void (...) @b(), !intel.callsite.inlining.report !33
     call void @a(), !intel.callsite.inlining.report !35
     ret i32 0
   }

   declare !intel.function.inlining.report !36 dso_local void @b(...)

   define internal void @y() !intel.function.inlining.report !37 {
   entry:
     call void (...) @z(), !intel.callsite.inlining.report !39
     ret void
   }

   declare !intel.function.inlining.report !42 dso_local void @z(...)

   !intel.module.inlining.report = !{!8, !27, !29, !36, !37, !42}

   !8 = distinct !{!"intel.function.inlining.report", !9, !10, !20, !24, !25, !26}
   !9 = !{!"name: a"}
   !10 = distinct !{!"intel.callsites.inlining.report", !11, !21}
   !11 = distinct !{!"intel.callsite.inlining.report", !12, null, !13, !14, !15, !16, !17, !18, !19, !"line: 8 col: 3", !20}
   !12 = !{!"name: x"}
   !13 = !{!"isInlined: 0"}
   !14 = !{!"reason: 31"}
   !15 = !{!"inlineCost: -1"}
   !16 = !{!"outerInlineCost: -1"}
   !17 = !{!"inlineThreshold: -1"}
   !18 = !{!"earlyExitCost: 2147483647"}
   !19 = !{!"earlyExitThreshold: 2147483647"}
   !20 = !{!"moduleName: test4.c"}
   !21 = distinct !{!"intel.callsite.inlining.report", !22, null, !13, !23, !15, !16, !17, !18, !19, !"line: 9 col: 3", !20}
   !22 = !{!"name: y"}
   !23 = !{!"reason: 25"}
   !24 = !{!"isDead: 0"}
   !25 = !{!"isDeclaration: 0"}
   !26 = !{!"linkage: A"}
   !27 = distinct !{!"intel.function.inlining.report", !12, null, !20, !24, !28, !26}
   !28 = !{!"isDeclaration: 1"}
   !29 = distinct !{!"intel.function.inlining.report", !30, !31, !20, !24, !25, !26}
   !30 = !{!"name: main"}
   !31 = distinct !{!"intel.callsites.inlining.report", !32, !33, !35}
   !32 = distinct !{!"intel.callsite.inlining.report", !9, null, !13, !23, !15, !16, !17, !18, !19, !"line: 13 col: 3", !20}
   !33 = distinct !{!"intel.callsite.inlining.report", !34, null, !13, !14, !15, !16, !17, !18, !19, !"line: 14 col: 3", !20}
   !34 = !{!"name: b"}
   !35 = distinct !{!"intel.callsite.inlining.report", !9, null, !13, !23, !15, !16, !17, !18, !19, !"line: 15 col: 3", !20}
   !36 = distinct !{!"intel.function.inlining.report", !34, null, !20, !24, !28, !26}
   !37 = distinct !{!"intel.function.inlining.report", !22, !38, !20, !24, !25, !41}
   !38 = distinct !{!"intel.callsites.inlining.report", !39}
   !39 = distinct !{!"intel.callsite.inlining.report", !40, null, !13, !14, !15, !16, !17, !18, !19, !"line: 5 col: 19", !20}
   !40 = !{!"name: z"}
   !41 = !{!"linkage: L"}
   !42 = distinct !{!"intel.function.inlining.report", !40, null, !20, !24, !28, !26}

* After inlining:

.. code-block:: llvm

   ; ModuleID = 'test1.c'

   define dso_local void @a() local_unnamed_addr !intel.function.inlining.report !8 {
   entry:
     tail call void (...) @x(), !intel.callsite.inlining.report !11
     tail call void (...) @z(), !intel.callsite.inlining.report !24
     ret void, !dbg !65
   }

   declare !intel.function.inlining.report !33 dso_local void @x(...) local_unnamed_addr

   define dso_local i32 @main() local_unnamed_addr !intel.function.inlining.report !35 {
   entry:
     tail call void (...) @x(), !intel.callsite.inlining.report !40
     tail call void (...) @z(), !intel.callsite.inlining.report !43
     tail call void (...) @b(), !intel.callsite.inlining.report !46
     tail call void @a(), !intel.callsite.inlining.report !48
     ret i32 0
   }

   declare !intel.function.inlining.report !50 dso_local void @b(...) local_unnamed_addr

   declare !intel.function.inlining.report !57 dso_local void @z(...) local_unnamed_addr

   !intel.module.inlining.report = !{!8, !33, !35, !50, !51, !57}

   !8 = distinct !{!"intel.function.inlining.report", !9, !10, !20, !30, !31, !32}
   !9 = !{!"name: a"}
   !10 = distinct !{!"intel.callsites.inlining.report", !11, !21}
   !11 = distinct !{!"intel.callsite.inlining.report", !12, null, !13, !14, !15, !16, !17, !18, !19, !"line: 8 col: 3", !20}
   !12 = !{!"name: x"}
   !13 = !{!"isInlined: 0"}
   !14 = !{!"reason: 31"}
   !15 = !{!"inlineCost: -1"}
   !16 = !{!"outerInlineCost: -1"}
   !17 = !{!"inlineThreshold: -1"}
   !18 = !{!"earlyExitCost: 2147483647"}
   !19 = !{!"earlyExitThreshold: 2147483647"}
   !20 = !{!"moduleName: test4.c"}
   !21 = distinct !{!"intel.callsite.inlining.report", !22, !23, !26, !27, !28, !16, !29, !18, !19, !"line: 9 col: 3", !20}
   !22 = !{!"name: y"}
   !23 = distinct !{!"intel.callsites.inlining.report", !24}
   !24 = distinct !{!"intel.callsite.inlining.report", !25, null, !13, !14, !15, !16, !17, !18, !19, !"line: 5 col: 19", !20}
   !25 = !{!"name: z"}
   !26 = !{!"isInlined: 1"}
   !27 = !{!"reason: 7"}
   !28 = !{!"inlineCost: -15000"}
   !29 = !{!"inlineThreshold: 337"}
   !30 = !{!"isDead: 0"}
   !31 = !{!"isDeclaration: 0"}
   !32 = !{!"linkage: A"}
   !33 = distinct !{!"intel.function.inlining.report", !12, null, !20, !30, !34, !32}
   !34 = !{!"isDeclaration: 1"}
   !35 = distinct !{!"intel.function.inlining.report", !36, !37, !20, !30, !31, !32}
   !36 = !{!"name: main"}
   !37 = distinct !{!"intel.callsites.inlining.report", !38, !46, !48}
   !38 = distinct !{!"intel.callsite.inlining.report", !9, !39, !26, !44, !45, !16, !29, !18, !19, !"line: 13 col: 3", !20}
   !39 = distinct !{!"intel.callsites.inlining.report", !40, !41}
   !40 = distinct !{!"intel.callsite.inlining.report", !12, null, !13, !14, !15, !16, !17, !18, !19, !"line: 8 col: 3", !20}
   !41 = distinct !{!"intel.callsite.inlining.report", !22, !42, !26, !27, !28, !16, !29, !18, !19, !"line: 9 col: 3", !20}
   !42 = distinct !{!"intel.callsites.inlining.report", !43}
   !43 = distinct !{!"intel.callsite.inlining.report", !25, null, !13, !14, !15, !16, !17, !18, !19, !"line: 5 col: 19", !20}
   !44 = !{!"reason: 8"}
   !45 = !{!"inlineCost: 30"}
   !46 = distinct !{!"intel.callsite.inlining.report", !47, null, !13, !14, !15, !16, !17, !18, !19, !"line: 14 col: 3", !20}
   !47 = !{!"name: b"}
   !48 = distinct !{!"intel.callsite.inlining.report", !9, null, !13, !49, !15, !16, !17, !18, !19, !"line: 15 col: 3", !20}
   !49 = !{!"reason: 47"}
   !50 = distinct !{!"intel.function.inlining.report", !47, null, !20, !30, !34, !32}
   !51 = distinct !{!"intel.function.inlining.report", !22, !52, !20, !55, !31, !56}
   !52 = distinct !{!"intel.callsites.inlining.report", !53}
   !53 = distinct !{!"intel.callsite.inlining.report", !25, null, !13, !54, !15, !16, !17, !18, !19, !"line: 5 col: 19", !20}
   !54 = !{!"reason: 28"}
   !55 = !{!"isDead: 1"}
   !56 = !{!"linkage: L"}
   !57 = distinct !{!"intel.function.inlining.report", !25, null, !20, !30, !34, !32}

.. code-block:: none

    ---- Begin Inlining Report ---- (via metadata)
    COMPILE FUNC: A main
       -> INLINE: A a test1.c (11, 3) (30<=337) <<Callee is single basic block>>
          -> EXTERN: A x test1.c (18, 3)
          -> INLINE: L y test1.c (19, 3) (-15000<=337) <<Callee has single callsite and local linkage>>
             -> EXTERN: A z test1.c (23, 3)
       -> EXTERN: A b test1.c (12, 3)
       -> A a test1.c (13, 3) [[Callsite is noinline]]

    COMPILE FUNC: A a
       -> EXTERN: A x test1.c (18, 3)
       -> INLINE: L y test1.c (19, 3) (-15000<=337) <<Callee has single callsite and local linkage>>
          -> EXTERN: A z test1.c (23, 3)

    DEAD STATIC FUNC: L y

    ---- End Inlining Report ------ (via metadata)

