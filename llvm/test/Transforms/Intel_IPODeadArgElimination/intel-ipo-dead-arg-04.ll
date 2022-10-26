; Inline report
; RUN: opt -passes='cgscc(inline)',intel-ipo-dead-arg-elimination -inline-report=0xe807   < %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-IR

; Inline report with metadata
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s  -S | opt -passes='cgscc(inline)',intel-ipo-dead-arg-elimination -inline-report=0xe886  -S | opt -passes='inlinereportemitter' -inline-report=0xe886  -S 2>&1 | FileCheck %s -check-prefix=CHECK-IR-MD

; This test case checks that the inlining report is preserved after simplified
; dead arguments elimination. Function @foo wasn't inlined intentionally to
; generate the IR needed for dead arg elimination.

; Verify that the traditional inline report was produced correctly.

; CHECK-IR: Function Attrs: noinline
; CHECK-IR-NEXT: define internal float @foo(float* %0, i64 %1, i64 %2) #1 {
; CHECK-IR-NEXT:   %4 = load float, float* %0, align 4
; CHECK-IR-NEXT:   ret float %4
; CHECK-IR-NEXT: }

; Make sure that dead arg elimination produced the IR we want
; CHECK-IR: define float @bas(float* %0, float %1, i64 %2, i64 %3)
; CHECK-IR-NEXT:   %5 = call float @foo(float* %0, i64 %2, i64 %3)
; CHECK-IR-NEXT:   %6 = fadd float %1, %5
; CHECK-IR-NEXT:   ret float %6

; CHECK-IR: attributes #1 = { noinline }

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
; CHECK-IR-MD: define internal float @foo(float* %0, i64 %1, i64 %2) #1 !intel.function.inlining.report !8 {

; CHECK-IR-MD: define float @bas(float* %0, float %1, i64 %2, i64 %3) !intel.function.inlining.report !24 {
; CHECK-IR-MD:   %5 = call float @foo(float* %0, i64 %2, i64 %3), !intel.callsite.inlining.report !29

; CHECK-IR-MD: !intel.module.inlining.report = !{!0, !8, !12, !24}

; CHECK-IR-MD: !0 = distinct !{!"intel.function.inlining.report", !1, null, !2, !3, !4, !5, !6, !7}
; CHECK-IR-MD: !1 = !{!"name: llvm.intel.subscript.p0f32.i64.i64.p0f32.i64"}
; CHECK-IR-MD: !2 = !{!"moduleName: <stdin>"}
; CHECK-IR-MD: !3 = !{!"isDead: 0"}
; CHECK-IR-MD: !4 = !{!"isDeclaration: 1"}
; CHECK-IR-MD: !5 = !{!"linkage: A"}
; CHECK-IR-MD: !6 = !{!"language: C"}
; CHECK-IR-MD: !7 = !{!"isSuppressPrint: 0"}
; CHECK-IR-MD: !8 = distinct !{!"intel.function.inlining.report", !9, null, !2, !3, !10, !11, !6, !7}
; CHECK-IR-MD: !9 = !{!"name: foo"}
; CHECK-IR-MD: !10 = !{!"isDeclaration: 0"}
; CHECK-IR-MD: !11 = !{!"linkage: L"}
; CHECK-IR-MD: !12 = distinct !{!"intel.function.inlining.report", !13, !14, !2, !23, !10, !11, !6, !7}
; CHECK-IR-MD: !13 = !{!"name: bar"}
; CHECK-IR-MD: !14 = distinct !{!"intel.callsites.inlining.report", !15}
; CHECK-IR-MD: !15 = distinct !{!"intel.callsite.inlining.report", !9, null, !16, !17, !18, !19, !20, !21, !22, !"line: 0 col: 0", !2, !7}
; CHECK-IR-MD: !16 = !{!"isInlined: 0"}
; CHECK-IR-MD: !17 = !{!"reason: 41"}
; CHECK-IR-MD: !18 = !{!"inlineCost: -1"}
; CHECK-IR-MD: !19 = !{!"outerInlineCost: -1"}
; CHECK-IR-MD: !20 = !{!"inlineThreshold: -1"}
; CHECK-IR-MD: !21 = !{!"earlyExitCost: 2147483647"}
; CHECK-IR-MD: !22 = !{!"earlyExitThreshold: 2147483647"}
; CHECK-IR-MD: !23 = !{!"isDead: 1"}
; CHECK-IR-MD: !24 = distinct !{!"intel.function.inlining.report", !25, !26, !2, !3, !10, !5, !6, !7}
; CHECK-IR-MD: !25 = !{!"name: bas"}
; CHECK-IR-MD: !26 = distinct !{!"intel.callsites.inlining.report", !27}
; CHECK-IR-MD: !27 = distinct !{!"intel.callsite.inlining.report", !13, !28, !31, !32, !33, !19, !34, !21, !22, !"line: 0 col: 0", !2, !7}
; CHECK-IR-MD: !28 = distinct !{!"intel.callsites.inlining.report", !29}
; CHECK-IR-MD: !29 = distinct !{!"intel.callsite.inlining.report", !9, null, !16, !30, !18, !19, !20, !21, !22, !"line: 0 col: 0", !2, !7}
; CHECK-IR-MD: !30 = !{!"reason: 60"}
; CHECK-IR-MD: !31 = !{!"isInlined: 1"}
; CHECK-IR-MD: !32 = !{!"reason: 10"}
; CHECK-IR-MD: !33 = !{!"inlineCost: -15000"}
; CHECK-IR-MD: !34 = !{!"inlineThreshold: 337"}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 %0, i64 %1, i64 %2, float* elementtype(float) %3, i64 %4)

; Function Attrs: noinline
define internal float @foo(float *%0, float *%1, i64 %2, i64 %3) #0 {
   %5 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %2, i64 %3, float* elementtype(float) nonnull %0, i64 %2)
   %6 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %2, i64 4, float* elementtype(float) nonnull %5, i64 %2)
   %7 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %2, i64 %3, float* elementtype(float) nonnull %6, i64 %2)
   %8 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %2, i64 %3, float* elementtype(float) nonnull %7, i64 %2)
   %9 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %2, i64 4, float* elementtype(float) nonnull %8, i64 %2)
   %10 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %2, i64 %3, float* elementtype(float) nonnull %9, i64 %2)
   store float 0.000000e+00, float* %10
   %11 = load float, float* %1
   ret float %11
}

define internal float @bar(float *%0, float *%1, i64 %2, i64 %3) {
  %5 = call float @foo(float *%0, float *%1, i64 %2, i64 %3)
  ret float %5
}

define float @bas(float *%0, float %1, i64 %2, i64 %3) {
  %5 = alloca float, i64 %3
  %6 = call float @bar(float *%5, float *%0, i64 %2, i64 %3)
  %7 = fadd float %1, %6
  ret float %7
}

attributes #0 = { noinline }
