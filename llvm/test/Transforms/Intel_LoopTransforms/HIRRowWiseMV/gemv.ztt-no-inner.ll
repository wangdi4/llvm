; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-rowwise-mv -print-before=hir-rowwise-mv -hir-rowwise-mv-skip-dtrans -print-after=hir-rowwise-mv -disable-output 2>&1 < %s | FileCheck %s

; This test checks that the basic row-wise multiversioning transformation
; generates the expected code when ZTTs are present, even when there's no ZTT on
; the innermost loop.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Print before:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 63, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, %N + -1, 1   <DO_LOOP>
; CHECK:       |   |   + DO i3 = 0, 127, 1   <DO_LOOP>
; CHECK:       |   |   |   %Aijbj = (%A)[128 * %N * i1 + 128 * i2 + i3]  *  (%b)[i3];
; CHECK:       |   |   |   %sum = %sum  +  %Aijbj;
; CHECK:       |   |   + END LOOP
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

; Print after:

; CHECK: BEGIN REGION
; CHECK:       %[[ROWCASE:[A-Za-z0-9_.]+]] = 0;
; CHECK:       if (%N > 0)
; CHECK:       {
; CHECK:          %[[FIRST:[A-Za-z0-9_.]+]] = (%b)[0];
; CHECK:          %[[ALLCLOSE:[A-Za-z0-9_.]+]] = 1;

; CHECK:          + DO i1 = 0, 126, 1   <DO_LOOP>
; CHECK:          |   %[[NEXT:[A-Za-z0-9_.]+]] = (%b)[i1 + 1];
; CHECK:          |   %[[DIFF:[A-Za-z0-9_.]+]] = %[[NEXT]]  -  %[[FIRST]];
; CHECK:          |   %[[ABSDIFF:[A-Za-z0-9_.]+]] = @llvm.fabs.f64(%[[DIFF]]);
; CHECK:          |   if (%[[ABSDIFF]] >u 1.000000e-04)
; CHECK:          |   {
; CHECK:          |      %[[ALLCLOSE]] = 0;
; CHECK:          |   }
; CHECK:          + END LOOP

; CHECK:          if (%[[ALLCLOSE]] != 0)
; CHECK:          {
; CHECK:             if (%[[FIRST]] == -1.000000e+00)
; CHECK:             {
; CHECK:                %[[ROWCASE]] = 1;
; CHECK:             }
; CHECK:             else
; CHECK:             {
; CHECK:                if (%[[FIRST]] == 0.000000e+00)
; CHECK:                {
; CHECK:                   %[[ROWCASE]] = 2;
; CHECK:                }
; CHECK:                else
; CHECK:                {
; CHECK:                   if (%[[FIRST]] == 1.000000e+00)
; CHECK:                   {
; CHECK:                      %[[ROWCASE]] = 3;
; CHECK:                   }
; CHECK:                }
; CHECK:             }
; CHECK:          }
; CHECK:       }

; CHECK:       + DO i1 = 0, 63, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, %N + -1, 1   <DO_LOOP>
; CHECK:       |   |   switch(%[[ROWCASE]])
; CHECK:       |   |   {
; CHECK:       |   |   case 1:
; CHECK:       |   |      + DO i3 = 0, 127, 1   <DO_LOOP>
; CHECK:       |   |      |   %sum = %sum  - (%A)[128 * %N * i1 + 128 * i2 + i3];
; CHECK:       |   |      + END LOOP
; CHECK:       |   |      break;
; CHECK:       |   |   case 2:
; CHECK:       |   |      break;
; CHECK:       |   |   case 3:
; CHECK:       |   |      + DO i3 = 0, 127, 1   <DO_LOOP>
; CHECK:       |   |      |   %Aijbj = (%A)[128 * %N * i1 + 128 * i2 + i3];
; CHECK:       |   |      |   %sum = %sum  +  %Aijbj;
; CHECK:       |   |      + END LOOP
; CHECK:       |   |      break;
; CHECK:       |   |   default:
; CHECK:       |   |      + DO i3 = 0, 127, 1   <DO_LOOP>
; CHECK:       |   |      |   %Aijbj = (%A)[128 * %N * i1 + 128 * i2 + i3]  *  (%b)[i3];
; CHECK:       |   |      |   %sum = %sum  +  %Aijbj;
; CHECK:       |   |      + END LOOP
; CHECK:       |   |      break;
; CHECK:       |   |   }
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

define double @gemv(double* %A, double* %b, i64 %N) #0 {
entry:
  %L1.ztt = icmp sgt i64 %N, 0
  %N128 = mul nuw nsw i64 %N, 128
  br label %L0

L0:
  %h = phi i64 [ 0, %entry ], [ %h.next, %L0.latch ]
  %sum = phi double [ 0.0, %entry ], [ %sum.final.L1, %L0.latch ]
  %A_mat = mul nuw nsw i64 %h, %N128
  br i1 %L1.ztt, label %L1.pre, label %L0.latch

L1.pre:
  br label %L1

L1:
  %i = phi i64 [ 0, %L1.pre ], [ %i.next, %L1.latch ]
  %sum.L1 = phi double [ %sum, %L1.pre ], [ %sum.next, %L1.latch ]
  %A_row_off = mul nuw nsw i64 %i, 128
  %A_row = add nuw nsw i64 %A_mat, %A_row_off
  br label %L2

L2:
  %j = phi i64 [ 0, %L1 ], [ %j.next, %L2 ]
  %sum.L2 = phi double [ %sum.L1, %L1 ], [ %sum.next, %L2 ]
  %A_ind = add nuw nsw i64 %A_row, %j
  %Aijp = getelementptr inbounds double, double* %A, i64 %A_ind
  %Aij = load double, double* %Aijp
  %bjp = getelementptr inbounds double, double* %b, i64 %j
  %bj = load double, double* %bjp
  %Aijbj = fmul nnan nsz arcp afn reassoc double %Aij, %bj
  %sum.next = fadd double %sum.L2, %Aijbj
  %j.next = add nuw nsw i64 %j, 1
  %L2.cond = icmp eq i64 %j.next, 128
  br i1 %L2.cond, label %L1.latch, label %L2

L1.latch:
  %i.next = add nuw nsw i64 %i, 1
  %L1.cond = icmp eq i64 %i.next, %N
  br i1 %L1.cond, label %L1.exit, label %L1

L1.exit:
  %sum.final.L2.lcssa = phi double [ %sum.next, %L1.latch ]
  br label %L0.latch

L0.latch:
  %sum.final.L1 = phi double [ %sum, %L0 ], [ %sum.final.L2.lcssa, %L1.exit ]
  %h.next = add nuw nsw i64 %h, 1
  %L0.cond = icmp eq i64 %h.next, 64
  br i1 %L0.cond, label %exit, label %L0

exit:
  %sum.final.L1.lcssa = phi double [ %sum.final.L1, %L0.latch ]
  ret double %sum.final.L1.lcssa
}

attributes #0 = { "unsafe-fp-math"="true" }
