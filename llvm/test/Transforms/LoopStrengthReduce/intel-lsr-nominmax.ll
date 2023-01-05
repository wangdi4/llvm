; RUN: opt -passes="loop(loop-reduce)" -S < %s | FileCheck %s

; CHECK-NOT: select
; CHECK-NOT: min
; CHECK-NOT: max

; 24864: Make sure LSR+SCEV do not generate new select/min/max insts where
; there were none before.

; The ashr in the IR below is parsed by SCEV to:
; (((-1 * undef) smax undef) /u 8) * (1 smin (-1 smax undef))

; The IV is chained with multiple ashrs, resulting in a fairly large
; expression:
; ((8 * ((-1 * %3) + undef) * (((-1 * undef) smax undef) /u 8) * (1 smin (-1 smax undef))) + (8 * (((-1 * %2) smax %2) /u 8) * (1 smin (-1 smax %2)) * undef) + {(8 + (-8 * undef) + undef),+,8}<%.critedge23>)

; LSR replaces the loop IV with the above sequence, which is clearly worse than
; the original.
; We apply a simple heuristic that ignores SCEVs with 2 or more terms with
; min/max.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$double*$rank3$.0.2.4.10.38.68" = type { double*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }

@upml_mod_mp_bx_khigh_ = external hidden global %"QNCA_a0$double*$rank3$.0.2.4.10.38.68"

define hidden fastcc void @upml_mod_mp_upml_updateh_() unnamed_addr #0 {
  br i1 undef, label %.critedge23, label %.critedge16

.critedge27:                                      ; preds = %.critedge23
  ret void

.critedge16:                                      ; preds = %.critedge16, %0
  br i1 false, label %.critedge23, label %.critedge16

.critedge23:                                      ; preds = %.critedge23, %.critedge16, %0
  %1 = phi i64 [ %11, %.critedge23 ], [ 0, %.critedge16 ], [ 0, %0 ]
  %2 = load i64, i64* getelementptr inbounds (%"QNCA_a0$double*$rank3$.0.2.4.10.38.68", %"QNCA_a0$double*$rank3$.0.2.4.10.38.68"* @upml_mod_mp_bx_khigh_, i64 0, i32 6, i64 1, i32 1), align 16
  %3 = load i64, i64* getelementptr inbounds (%"QNCA_a0$double*$rank3$.0.2.4.10.38.68", %"QNCA_a0$double*$rank3$.0.2.4.10.38.68"* @upml_mod_mp_bx_khigh_, i64 0, i32 6, i64 2, i32 2), align 16
  %4 = ashr exact i64 undef, 3
  %5 = sub nsw i64 undef, %3
  %6 = mul nsw i64 %4, %5
  %7 = ashr exact i64 %2, 3
  %8 = sub nsw i64 undef, 0
  %9 = mul nsw i64 %7, %8
  %10 = getelementptr inbounds double, double* undef, i64 %6
  %11 = add i64 %1, 1
  %12 = sub nsw i64 %11, undef
  %13 = getelementptr inbounds double, double* %10, i64 %9
  %14 = getelementptr inbounds double, double* %13, i64 %12
  br i1 undef, label %.critedge27, label %.critedge23
}

attributes #0 = { "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"LTOPostLink", i32 1}
