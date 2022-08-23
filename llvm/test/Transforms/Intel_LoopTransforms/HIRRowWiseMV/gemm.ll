; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-rowwise-mv -print-before=hir-rowwise-mv -hir-rowwise-mv-skip-dtrans -print-after=hir-rowwise-mv -disable-output 2>&1 < %s | FileCheck %s
;
; RUN: opt -opaque-pointers -hir-ssa-deconstruction -hir-temp-cleanup -hir-rowwise-mv -print-before=hir-rowwise-mv -hir-rowwise-mv-skip-dtrans -print-after=hir-rowwise-mv -disable-output 2>&1 < %s | FileCheck %s
;
; This test checks that the row-wise multiversioning transformation generates
; the expected code for cases where the temp array is needed.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Print before:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 31, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, 63, 1   <DO_LOOP>
; CHECK:       |   |   + DO i3 = 0, 127, 1   <DO_LOOP>
; CHECK:       |   |   |   %AikBkj = (%A)[128 * i2 + i3]  *  (%B)[i1 + 32 * i3];
; CHECK:       |   |   |   %sum = %sum  +  %AikBkj;
; CHECK:       |   |   + END LOOP
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

; Print after:

; CHECK: BEGIN REGION
; CHECK:       %[[ROWCASES:[A-Za-z0-9_.]+]] = alloca 64;

; CHECK:       + DO i1 = 0, 63, 1   <DO_LOOP>
; CHECK:       |   %[[FIRST:[A-Za-z0-9_.]+]] = (%A)[128 * i1];
; CHECK:       |   %[[ALLCLOSE:[A-Za-z0-9_.]+]] = 1;
; CHECK:       |
; CHECK:       |   + DO i2 = 0, 126, 1   <DO_LOOP>
; CHECK:       |   |   %[[NEXT:[A-Za-z0-9_.]+]] = (%A)[128 * i1 + i2 + 1];
; CHECK:       |   |   %[[DIFF:[A-Za-z0-9_.]+]] = %[[NEXT]]  -  %[[FIRST]];
; CHECK:       |   |   %[[ABSDIFF:[A-Za-z0-9_.]+]] = @llvm.fabs.f64(%[[DIFF]]);
; CHECK:       |   |   if (%[[ABSDIFF]] >u 1.000000e-04)
; CHECK:       |   |   {
; CHECK:       |   |      %[[ALLCLOSE]] = 0;
; CHECK:       |   |   }
; CHECK:       |   + END LOOP
; CHECK:       |
; CHECK:       |   %[[ROWCASE:[A-Za-z0-9_.]+]] = 0;
; CHECK:       |   if (%[[ALLCLOSE]] != 0)
; CHECK:       |   {
; CHECK:       |      if (%[[FIRST]] == -1.000000e+00)
; CHECK:       |      {
; CHECK:       |         %[[ROWCASE]] = 1;
; CHECK:       |      }
; CHECK:       |      else
; CHECK:       |      {
; CHECK:       |         if (%[[FIRST]] == 0.000000e+00)
; CHECK:       |         {
; CHECK:       |            %[[ROWCASE]] = 2;
; CHECK:       |         }
; CHECK:       |         else
; CHECK:       |         {
; CHECK:       |            if (%[[FIRST]] == 1.000000e+00)
; CHECK:       |            {
; CHECK:       |               %[[ROWCASE]] = 3;
; CHECK:       |            }
; CHECK:       |         }
; CHECK:       |      }
; CHECK:       |   }
; CHECK:       |   (%[[ROWCASES]])[i1] = %[[ROWCASE]]
; CHECK:       + END LOOP

; CHECK:       + DO i1 = 0, 31, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, 63, 1   <DO_LOOP>
; CHECK:       |   |   switch((%[[ROWCASES]])[i2])
; CHECK:       |   |   {
; CHECK:       |   |   case 1:
; CHECK:       |   |      + DO i3 = 0, 127, 1   <DO_LOOP>
; CHECK:       |   |      |   %sum = %sum  - (%B)[i1 + 32 * i3];
; CHECK:       |   |      + END LOOP
; CHECK:       |   |      break;
; CHECK:       |   |   case 2:
; CHECK-NEXT:  |   |      break;
; CHECK:       |   |   case 3:
; CHECK:       |   |      + DO i3 = 0, 127, 1   <DO_LOOP>
; CHECK:       |   |      |   %AikBkj = (%B)[i1 + 32 * i3];
; CHECK:       |   |      |   %sum = %sum  +  %AikBkj;
; CHECK:       |   |      + END LOOP
; CHECK:       |   |      break;
; CHECK:       |   |   default:
; CHECK:       |   |      + DO i3 = 0, 127, 1   <DO_LOOP>
; CHECK:       |   |      |   %AikBkj = (%A)[128 * i2 + i3]  *  (%B)[i1 + 32 * i3];
; CHECK:       |   |      |   %sum = %sum  +  %AikBkj;
; CHECK:       |   |      + END LOOP
; CHECK:       |   |      break;
; CHECK:       |   |   }
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

define double @gemm(double* %A, double* %B) #0 {
entry:
  br label %L1

L1:
  %j = phi i32 [ 0, %entry ], [ %j.next, %L2.exit ]
  %sum = phi double [ 0.0, %entry ], [ %sum.next.lcssa.lcssa, %L2.exit ]
  br label %L2

L2:
  %i = phi i32 [ 0, %L1 ], [ %i.next, %L3.exit ]
  %sum.L2 = phi double [ %sum, %L1 ], [ %sum.next.lcssa, %L3.exit ]
  %A_row = mul nuw nsw i32 %i, 128
  br label %L3

L3:
  %k = phi i32 [ 0, %L2 ], [ %k.next, %L3 ]
  %sum.L3 = phi double [ %sum.L2, %L2 ], [ %sum.next, %L3 ]
  %A_ind = add nuw nsw i32 %A_row, %k
  %Aikp = getelementptr inbounds double, double* %A, i32 %A_ind
  %Aik = load double, double* %Aikp
  %B_row = mul nuw nsw i32 %k, 32
  %B_ind = add nuw nsw i32 %B_row, %j
  %Bkjp = getelementptr inbounds double, double* %B, i32 %B_ind
  %Bkj = load double, double* %Bkjp
  %AikBkj = fmul nnan nsz arcp afn reassoc double %Aik, %Bkj
  %sum.next = fadd double %sum.L3, %AikBkj
  %k.next = add nuw nsw i32 %k, 1
  %L3.cond = icmp eq i32 %k.next, 128
  br i1 %L3.cond, label %L3.exit, label %L3

L3.exit:
  %sum.next.lcssa = phi double [ %sum.next, %L3 ]
  %i.next = add nuw nsw i32 %i, 1
  %L2.cond = icmp eq i32 %i.next, 64
  br i1 %L2.cond, label %L2.exit, label %L2

L2.exit:
  %sum.next.lcssa.lcssa = phi double [ %sum.next.lcssa, %L3.exit ]
  %j.next = add nuw nsw i32 %j, 1
  %L1.cond = icmp eq i32 %j.next, 32
  br i1 %L1.cond, label %L1.exit, label %L1

L1.exit:
  %sum.next.lcssa.lcssa.lcssa = phi double [ %sum.next.lcssa.lcssa, %L2.exit ]
  ret double %sum.next.lcssa.lcssa.lcssa
}

attributes #0 = { "unsafe-fp-math"="true" }
