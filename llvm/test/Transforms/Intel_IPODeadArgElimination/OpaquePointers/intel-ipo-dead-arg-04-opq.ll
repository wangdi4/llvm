; Inline report
; RUN: opt -opaque-pointers -passes='cgscc(inline)',intel-ipo-dead-arg-elimination -inline-report=0xe807   < %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-IR

; Inline report with metadata
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s  -S | opt -opaque-pointers -passes='cgscc(inline)',intel-ipo-dead-arg-elimination -inline-report=0xe886  -S | opt -opaque-pointers -passes='inlinereportemitter' -inline-report=0xe886  -S 2>&1 | FileCheck %s -check-prefix=CHECK-IR-MD

; This test case checks that the inlining report is preserved after simplified
; dead arguments elimination. Function @foo wasn't inlined intentionally to
; generate the IR needed for dead arg elimination. This test case is the same
; as intel-ipo-dead-arg-04.ll but it checks the IR with opaque pointers.

; Verify that the traditional inline report was produced correctly.

; CHECK-IR: Function Attrs: noinline
; CHECK-IR-NEXT: define internal float @foo(ptr %0, i64 %1, i64 %2) #0 {
; CHECK-IR-NEXT:   %4 = load float, ptr %0, align 4
; CHECK-IR-NEXT:   ret float %4
; CHECK-IR-NEXT: }

; Make sure that dead arg elimination produced the IR we want
; CHECK-IR: define float @bas(ptr %0, float %1, i64 %2, i64 %3)
; CHECK-IR-NEXT:   %5 = call float @foo(ptr %0, i64 %2, i64 %3)
; CHECK-IR-NEXT:   %6 = fadd float %1, %5
; CHECK-IR-NEXT:   ret float %6

; CHECK-IR: attributes #0 = { noinline }

; Check that function @foo is shown as "noinline" in the report
; CHECK-IR: DEAD STATIC FUNC: bar

; CHECK-IR: COMPILE FUNC: bas
; CHECK-IR:    INLINE: bar {{.*}}Callee has single callsite and local linkage
; CHECK-IR:       foo{{.*}}Callee has noinline attribute

; CHECK-IR: COMPILE FUNC: foo

; Check that the inline report with metadata was produced correctly.
; CHECK-IR-MD: COMPILE FUNC: bas
; CHECK-IR-MD:    INLINE: bar {{.*}}Callee has single callsite and local linkage
; CHECK-IR-MD:       foo{{.*}}Callee has noinline attribute

; Verify the IR
; CHECK-IR-MD: ; Function Attrs: noinline
; CHECK-IR-MD: define internal float @foo(ptr %0, i64 %1, i64 %2) #0 !intel.function.inlining.report !0 {

; CHECK-IR-MD: define float @bas(ptr %0, float %1, i64 %2, i64 %3) !intel.function.inlining.report !20 {
; CHECK-IR-MD:   %5 = call float @foo(ptr %0, i64 %2, i64 %3), !intel.callsite.inlining.report !25

; CHECK-IR-MD: !intel.module.inlining.report = !{!0, !8, !20, !32}

; CHECK-IR-MD: !0 = distinct !{!"intel.function.inlining.report", !1, null, !2, !3, !4, !5, !6, !7}
; CHECK-IR-MD: !1 = !{!"name: foo"}
; CHECK-IR-MD: !2 = !{!"moduleName: <stdin>"}
; CHECK-IR-MD: !3 = !{!"isDead: 0"}
; CHECK-IR-MD: !4 = !{!"isDeclaration: 0"}
; CHECK-IR-MD: !5 = !{!"linkage: L"}
; CHECK-IR-MD: !6 = !{!"language: C"}
; CHECK-IR-MD: !7 = !{!"isSuppressPrint: 0"}
; CHECK-IR-MD: !8 = distinct !{!"intel.function.inlining.report", !9, !10, !2, !19, !4, !5, !6, !7}
; CHECK-IR-MD: !9 = !{!"name: bar"}
; CHECK-IR-MD: !10 = distinct !{!"intel.callsites.inlining.report", !11}
; CHECK-IR-MD: !11 = distinct !{!"intel.callsite.inlining.report", !1, null, !12, !13, !14, !15, !16, !17, !18, !"line: 0 col: 0", !2, !7}
; CHECK-IR-MD: !12 = !{!"isInlined: 0"}
; CHECK-IR-MD: !13 = !{!"reason: 41"}
; CHECK-IR-MD: !14 = !{!"inlineCost: -1"}
; CHECK-IR-MD: !15 = !{!"outerInlineCost: -1"}
; CHECK-IR-MD: !16 = !{!"inlineThreshold: -1"}
; CHECK-IR-MD: !17 = !{!"earlyExitCost: 2147483647"}
; CHECK-IR-MD: !18 = !{!"earlyExitThreshold: 2147483647"}
; CHECK-IR-MD: !19 = !{!"isDead: 1"}
; CHECK-IR-MD: !20 = distinct !{!"intel.function.inlining.report", !21, !22, !2, !3, !4, !31, !6, !7}
; CHECK-IR-MD: !21 = !{!"name: bas"}
; CHECK-IR-MD: !22 = distinct !{!"intel.callsites.inlining.report", !23}
; CHECK-IR-MD: !23 = distinct !{!"intel.callsite.inlining.report", !9, !24, !27, !28, !29, !15, !30, !17, !18, !"line: 0 col: 0", !2, !7}
; CHECK-IR-MD: !24 = distinct !{!"intel.callsites.inlining.report", !25}
; CHECK-IR-MD: !25 = distinct !{!"intel.callsite.inlining.report", !1, null, !12, !26, !14, !15, !16, !17, !18, !"line: 0 col: 0", !2, !7}
; CHECK-IR-MD: !26 = !{!"reason: 60"}
; CHECK-IR-MD: !27 = !{!"isInlined: 1"}
; CHECK-IR-MD: !28 = !{!"reason: 10"}
; CHECK-IR-MD: !29 = !{!"inlineCost: -15000"}
; CHECK-IR-MD: !30 = !{!"inlineThreshold: 337"}
; CHECK-IR-MD: !31 = !{!"linkage: A"}
; CHECK-IR-MD: !32 = distinct !{!"intel.function.inlining.report", !33, null, !2, !3, !34, !31, !6, !7}
; CHECK-IR-MD: !33 = !{!"name: llvm.intel.subscript.p0.i64.i64.p0.i64"}
; CHECK-IR-MD: !34 = !{!"isDeclaration: 1"}

; ModuleID = 'intel-ipo-dead-arg-04.ll'
source_filename = "intel-ipo-dead-arg-04.ll"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline
define internal float @foo(ptr %0, ptr %1, i64 %2, i64 %3) #0 {
  %5 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %2, i64 %3, ptr nonnull elementtype(float) %0, i64 %2)
  %6 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %2, i64 4, ptr nonnull elementtype(float) %5, i64 %2)
  %7 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %2, i64 %3, ptr nonnull elementtype(float) %6, i64 %2)
  %8 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %2, i64 %3, ptr nonnull elementtype(float) %7, i64 %2)
  %9 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %2, i64 4, ptr nonnull elementtype(float) %8, i64 %2)
  %10 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %2, i64 %3, ptr nonnull elementtype(float) %9, i64 %2)
  store float 0.000000e+00, ptr %10, align 4
  %11 = load float, ptr %1, align 4
  ret float %11
}

define internal float @bar(ptr %0, ptr %1, i64 %2, i64 %3) {
  %5 = call float @foo(ptr %0, ptr %1, i64 %2, i64 %3)
  ret float %5
}

define float @bas(ptr %0, float %1, i64 %2, i64 %3) {
  %5 = alloca float, i64 %3, align 4
  %6 = call float @bar(ptr %5, ptr %0, i64 %2, i64 %3)
  %7 = fadd float %1, %6
  ret float %7
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { noinline }
attributes #1 = { nounwind readnone speculatable }
