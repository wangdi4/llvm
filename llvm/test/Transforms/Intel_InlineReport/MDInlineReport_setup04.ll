; RUN: opt -passes=inlinereportsetup -inline-report=0x180 < %s -S 2>&1 | FileCheck %s

; This test checks that verification passes if all callsites and functions have metadata attached even if callsites are shuffled.

; CHECK: define dso_local void @main() local_unnamed_addr !intel.function.inlining.report [[MAIN_FIR:![0-9]+]] {
; CHECK:   tail call void (...) @c(), !intel.callsite.inlining.report [[C_MAIN_CS:![0-9]+]]
; CHECK:   tail call void (...) @b(), !intel.callsite.inlining.report [[B_MAIN_CS:![0-9]+]]
; CHECK:   tail call void (...) @a(), !intel.callsite.inlining.report [[A_MAIN_CS:![0-9]+]]

; CHECK: declare !intel.function.inlining.report [[A_FIR:![0-9]+]] dso_local void @a(...) local_unnamed_addr

; CHECK: declare !intel.function.inlining.report [[B_FIR:![0-9]+]] dso_local void @b(...) local_unnamed_addr

; CHECK: declare !intel.function.inlining.report [[C_FIR:![0-9]+]] dso_local void @c(...) local_unnamed_addr

; CHECK: !intel.module.inlining.report = !{[[MAIN_FIR]], [[A_FIR]], [[B_FIR]], [[C_FIR]]}
; CHECK: [[MAIN_FIR]] = distinct !{!"intel.function.inlining.report", [[MAIN_NAME:![0-9]+]], [[MAIN_CSs:![0-9]+]], {{.*}}
; CHECK: [[MAIN_NAME]] = !{!"name: main"}
; CHECK: [[MAIN_CSs]] = distinct !{!"intel.callsites.inlining.report", [[A_MAIN_CS]], [[B_MAIN_CS]], [[C_MAIN_CS]]}
; CHECK: [[A_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[A_NAME:![0-9]+]], null, [[IS_INL_0:![0-9]+]], {{.*}}
; CHECK: [[A_NAME]] = !{!"name: a"}
; CHECK: [[IS_INL_0]] = !{!"isInlined: 0"}
; CHECK: [[B_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[B_NAME:![0-9]+]], null, [[IS_INL_0]], {{.*}}
; CHECK: [[B_NAME]] = !{!"name: b"}
; CHECK: [[C_MAIN_CS]] = distinct !{!"intel.callsite.inlining.report", [[C_NAME:![0-9]+]], null, [[IS_INL_0]], {{.*}}
; CHECK: [[C_NAME]] = !{!"name: c"}
; CHECK: [[A_FIR]] = distinct !{!"intel.function.inlining.report", [[A_NAME]], null, {{.*}}
; CHECK: [[B_FIR]] = distinct !{!"intel.function.inlining.report", [[B_NAME]], null, {{.*}}
; CHECK: [[C_FIR]] = distinct !{!"intel.function.inlining.report", [[C_NAME]], null, {{.*}}


; Inline report call sites and IR call sites go in a different order.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @main() local_unnamed_addr !intel.function.inlining.report !2 {
entry:
  tail call void (...) @c(), !intel.callsite.inlining.report !17
  tail call void (...) @b(), !intel.callsite.inlining.report !15
  tail call void (...) @a(), !intel.callsite.inlining.report !5
  ret void
}

declare !intel.function.inlining.report !22 dso_local void @a(...) local_unnamed_addr

declare !intel.function.inlining.report !24 dso_local void @b(...) local_unnamed_addr

declare !intel.function.inlining.report !25 dso_local void @c(...) local_unnamed_addr

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}
!intel.module.inlining.report = !{!2, !22, !24, !25}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = distinct !{!"intel.function.inlining.report", !3, !4, !14, !19, !20, !21}
!3 = !{!"name: main"}
!4 = distinct !{!"intel.callsites.inlining.report", !5, !15, !17}
!5 = distinct !{!"intel.callsite.inlining.report", !6, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!6 = !{!"name: a"}
!7 = !{!"isInlined: 0"}
!8 = !{!"reason: 33"}
!9 = !{!"inlineCost: -1"}
!10 = !{!"outerInlineCost: -1"}
!11 = !{!"inlineThreshold: -1"}
!12 = !{!"earlyExitCost: 2147483647"}
!13 = !{!"earlyExitThreshold: 2147483647"}
!14 = !{!"moduleName: test4.c"}
!15 = distinct !{!"intel.callsite.inlining.report", !16, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!16 = !{!"name: b"}
!17 = distinct !{!"intel.callsite.inlining.report", !18, null, !7, !8, !9, !10, !11, !12, !13, !"line: 0 col: 0", !14}
!18 = !{!"name: c"}
!19 = !{!"isDead: 0"}
!20 = !{!"isDeclaration: 0"}
!21 = !{!"linkage: A"}
!22 = distinct !{!"intel.function.inlining.report", !6, null, !14, !19, !23, !21}
!23 = !{!"isDeclaration: 1"}
!24 = distinct !{!"intel.function.inlining.report", !16, null, !14, !19, !23, !21}
!25 = distinct !{!"intel.function.inlining.report", !18, null, !14, !19, !23, !21}

