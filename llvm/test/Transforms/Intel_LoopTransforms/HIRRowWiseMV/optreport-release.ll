; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-rowwise-mv -hir-cg -simplifycfg -intel-ir-optreport-emitter -hir-rowwise-mv-skip-dtrans -force-hir-cg -intel-opt-report=low -disable-output 2>&1 < %s | FileCheck --implicit-check-not multiversioned %s

; This test checks that the OptReport generation in HIRRowWiseMultiversioning
; does not reveal information about the transform in release builds. Based on my
; understanding, lit tests are not currently run for such configurations so this
; test is likely not actively being run yet, but I have included it anyway for
; completeness and in case this configuration starts being tested in the future.

; UNSUPPORTED: intel_internal_build

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK:      LOOP BEGIN
; CHECK-NEXT: LOOP END

; CHECK:      LOOP BEGIN

; CHECK:          LOOP BEGIN
; CHECK-NEXT:     LOOP END

; CHECK:          LOOP BEGIN
; CHECK-NEXT:     LOOP END

; CHECK:          LOOP BEGIN
; CHECK-NEXT:     LOOP END

; CHECK:          LOOP BEGIN
; CHECK-NEXT:     LOOP END
; CHECK:      LOOP END

define double @gemv(double* %A, double* %b) #0 {
entry:
  br label %L1

L1:
  %i = phi i32 [ 0, %entry ], [ %i.next, %L2.exit ]
  %sum = phi double [ 0.0, %entry ], [ %sum.next.lcssa, %L2.exit ]
  %A_row = mul nuw nsw i32 %i, 128
  br label %L2

L2:
  %j = phi i32 [ 0, %L1 ], [ %j.next, %L2 ]
  %sum.L2 = phi double [ %sum, %L1 ], [ %sum.next, %L2 ]
  %A_ind = add nuw nsw i32 %A_row, %j
  %Aijp = getelementptr inbounds double, double* %A, i32 %A_ind
  %Aij = load double, double* %Aijp
  %bjp = getelementptr inbounds double, double* %b, i32 %j
  %bj = load double, double* %bjp
  %Aijbj = fmul nnan nsz arcp afn reassoc double %Aij, %bj
  %sum.next = fadd double %sum.L2, %Aijbj
  %j.next = add nuw nsw i32 %j, 1
  %L2.cond = icmp eq i32 %j.next, 128
  br i1 %L2.cond, label %L2.exit, label %L2

L2.exit:
  %sum.next.lcssa = phi double [ %sum.next, %L2 ]
  %i.next = add nuw nsw i32 %i, 1
  %L1.cond = icmp eq i32 %i.next, 64
  br i1 %L1.cond, label %L1.exit, label %L1

L1.exit:
  %sum.next.lcssa.lcssa = phi double [ %sum.next.lcssa, %L2.exit ]
  ret double %sum.next.lcssa.lcssa
}

attributes #0 = { "unsafe-fp-math"="true" }
