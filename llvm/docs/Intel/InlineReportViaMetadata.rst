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


* After inlining:

.. code-block:: llvm

 ; ModuleID = 'test1.c'

 define dso_local i32 @main() local_unnamed_addr #0 !dbg !40 !intel.function.inlining.report !8 {
 entry:
   tail call void (...) @x() #2, !dbg !44, !intel.callsite.inlining.report !12
   tail call void (...) @z() #2, !dbg !49, !intel.callsite.inlining.report !16
   tail call void (...) @b() #2, !dbg !52, !intel.callsite.inlining.report !18
   tail call void @a() #3, !dbg !53, !intel.callsite.inlining.report !20
   ret i32 0, !dbg !54
 }

 declare !intel.function.inlining.report !22 dso_local void @b(...) local_unnamed_addr #1

 define dso_local void @a() local_unnamed_addr #0 !dbg !45 !intel.function.inlining.report !24 {
 entry:
   tail call void (...) @x() #2, !dbg !55, !intel.callsite.inlining.report !26
   tail call void (...) @z() #2, !dbg !56, !intel.callsite.inlining.report !30
   ret void, !dbg !58
 }

 declare !intel.function.inlining.report !32 dso_local void @x(...) local_unnamed_addr #1

 declare !intel.function.inlining.report !38 dso_local void @z(...) local_unnamed_addr #1

 !intel.module.inlining.report = !{!8, !22, !24, !32, !34, !38}
 !8 = distinct !{!"intel.function.inlining.report", !"name: main", !9, !"moduleName: test1.c", !"isDead: 0", !"isDeclaration: 0", !"linkage: A"}
 !9 = distinct !{!"intel.callsites.inlining.report", !10, !18, !20}
 !10 = distinct !{!"intel.callsite.inlining.report", !"name: a", !11, !"isInlined: 1", !"reason: 7", !"inlineCost: 30", !"outerInlineCost: -1",
                  !"inlineThreshold: 337", !"earlyExitCost: 2147483647", !"earlyExitThreshold: 2147483647", !"line: 11 col: 3", !"moduleName: test1.c"}
 !11 = distinct !{!"intel.callsites.inlining.report", !12, !14}
 !12 = distinct !{!"intel.callsite.inlining.report", !"name: x", !13, !"isInlined: 0", !"reason: 29", !"inlineCost: -1", !"outerInlineCost: -1",
                  !"inlineThreshold: -1", !"earlyExitCost: 2147483647", !"earlyExitThreshold: 2147483647", !"line: 18 col: 3", !"moduleName: test1.c"}
 !13 = distinct !{!"intel.callsites.inlining.report"}
 !14 = distinct !{!"intel.callsite.inlining.report", !"name: y", !15, !"isInlined: 1", !"reason: 6", !"inlineCost: -15000", !"outerInlineCost: -1",
                  !"inlineThreshold: 337", !"earlyExitCost: 2147483647", !"earlyExitThreshold: 2147483647", !"line: 19 col: 3", !"moduleName: test1.c"}
 !15 = distinct !{!"intel.callsites.inlining.report", !16}
 !16 = distinct !{!"intel.callsite.inlining.report", !"name: z", !17, !"isInlined: 0", !"reason: 29", !"inlineCost: -1", !"outerInlineCost: -1",
                  !"inlineThreshold: -1", !"earlyExitCost: 2147483647", !"earlyExitThreshold: 2147483647", !"line: 23 col: 3", !"moduleName: test1.c"}
 !17 = distinct !{!"intel.callsites.inlining.report"}
 !18 = distinct !{!"intel.callsite.inlining.report", !"name: b", !19, !"isInlined: 0", !"reason: 29", !"inlineCost: -1", !"outerInlineCost: -1",
                  !"inlineThreshold: -1", !"earlyExitCost: 2147483647", !"earlyExitThreshold: 2147483647", !"line: 12 col: 3", !"moduleName: test1.c"}
 !19 = distinct !{!"intel.callsites.inlining.report"}
 !20 = distinct !{!"intel.callsite.inlining.report", !"name: a", !21, !"isInlined: 0", !"reason: 45", !"inlineCost: -1", !"outerInlineCost: -1",
                  !"inlineThreshold: -1", !"earlyExitCost: 2147483647", !"earlyExitThreshold: 2147483647", !"line: 13 col: 3", !"moduleName: test1.c"}
 !21 = distinct !{!"intel.callsites.inlining.report"}
 !22 = distinct !{!"intel.function.inlining.report", !"name: b", !23, !"moduleName: test1.c", !"isDead: 0", !"isDeclaration: 1", !"linkage: A"}
 !23 = distinct !{!"intel.callsites.inlining.report"}
 !24 = distinct !{!"intel.function.inlining.report", !"name: a", !25, !"moduleName: test1.c", !"isDead: 0", !"isDeclaration: 0", !"linkage: A"}
 !25 = distinct !{!"intel.callsites.inlining.report", !26, !28}
 !26 = distinct !{!"intel.callsite.inlining.report", !"name: x", !27, !"isInlined: 0", !"reason: 29", !"inlineCost: -1", !"outerInlineCost: -1",
                  !"inlineThreshold: -1", !"earlyExitCost: 2147483647", !"earlyExitThreshold: 2147483647", !"line: 18 col: 3", !"moduleName: test1.c"}
 !27 = distinct !{!"intel.callsites.inlining.report"}
 !28 = distinct !{!"intel.callsite.inlining.report", !"name: y", !29, !"isInlined: 1", !"reason: 6", !"inlineCost: -15000", !"outerInlineCost: -1",
                  !"inlineThreshold: 337", !"earlyExitCost: 2147483647", !"earlyExitThreshold: 2147483647", !"line: 19 col: 3", !"moduleName: test1.c"}
 !29 = distinct !{!"intel.callsites.inlining.report", !30}
 !30 = distinct !{!"intel.callsite.inlining.report", !"name: z", !31, !"isInlined: 0", !"reason: 29", !"inlineCost: -1", !"outerInlineCost: -1",
                  !"inlineThreshold: -1", !"earlyExitCost: 2147483647", !"earlyExitThreshold: 2147483647", !"line: 23 col: 3", !"moduleName: test1.c"}
 !31 = distinct !{!"intel.callsites.inlining.report"}
 !32 = distinct !{!"intel.function.inlining.report", !"name: x", !33, !"moduleName: test1.c", !"isDead: 0", !"isDeclaration: 1", !"linkage: A"}
 !33 = distinct !{!"intel.callsites.inlining.report"}
 !34 = distinct !{!"intel.function.inlining.report", !"name: y", !35, !"moduleName: test1.c", !"isDead: 1", !"isDeclaration: 0", !"linkage: L"}
 !35 = distinct !{!"intel.callsites.inlining.report", !36}
 !36 = distinct !{!"intel.callsite.inlining.report", !"name: z", !37, !"isInlined: 0", !"reason: 29", !"inlineCost: -1", !"outerInlineCost: -1",
                  !"inlineThreshold: -1", !"earlyExitCost: 2147483647", !"earlyExitThreshold: 2147483647", !"line: 23 col: 3", !"moduleName: test1.c"}
 !37 = distinct !{!"intel.callsites.inlining.report"}
 !38 = distinct !{!"intel.function.inlining.report", !"name: z", !39, !"moduleName: test1.c", !"isDead: 0", !"isDeclaration: 1", !"linkage: A"}
 !39 = distinct !{!"intel.callsites.inlining.report"}

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

