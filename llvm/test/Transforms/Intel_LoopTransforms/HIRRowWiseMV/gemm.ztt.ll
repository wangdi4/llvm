; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-rowwise-mv -print-before=hir-rowwise-mv -hir-rowwise-mv-skip-dtrans -print-after=hir-rowwise-mv -disable-output 2>&1 < %s | FileCheck %s

; This test checks that the row-wise multiversioning transformation generates
; the expected code for cases where the temp array is needed and ZTTs are
; present.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Print before:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 127, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, %M + -1, 1   <DO_LOOP>
; CHECK:       |   |   + DO i3 = 0, %N + -1, 1   <DO_LOOP>
; CHECK:       |   |   |   + DO i4 = 0, %K + -1, 1   <DO_LOOP>
; CHECK:       |   |   |   |   %AikBkj = (%A)[%K * i3 + i4]  *  (%B)[(%M * %K) * i1 + i2 + %M * i4];
; CHECK:       |   |   |   |   %sum = %sum  +  %AikBkj;
; CHECK:       |   |   |   + END LOOP
; CHECK:       |   |   + END LOOP
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

; Print after:

; CHECK: BEGIN REGION
; CHECK:       %[[ROWCASES:[A-Za-z0-9_.]+]] = alloca %N;

; CHECK:       + DO i1 = 0, %N + -1, 1   <DO_LOOP>
; CHECK:       |   %[[ROWCASE:[A-Za-z0-9_.]+]] = 0;
; CHECK:       |   if (%K > 0)
; CHECK:       |   {
; CHECK:       |      %[[FIRST:[A-Za-z0-9_.]+]] = (%A)[%K * i1];
; CHECK:       |      %[[ALLCLOSE:[A-Za-z0-9_.]+]] = 1;
; CHECK:       |
; CHECK:       |      + DO i2 = 0, %K + -2, 1   <DO_LOOP>
; CHECK:       |      |   %[[NEXT:[A-Za-z0-9_.]+]] = (%A)[%K * i1 + i2 + 1];
; CHECK:       |      |   %[[DIFF:[A-Za-z0-9_.]+]] = %[[NEXT]]  -  %[[FIRST]];
; CHECK:       |      |   %[[ABSDIFF:[A-Za-z0-9_.]+]] = @llvm.fabs.f64(%[[DIFF]]);
; CHECK:       |      |   if (%[[ABSDIFF]] >u 1.000000e-04)
; CHECK:       |      |   {
; CHECK:       |      |      %[[ALLCLOSE]] = 0;
; CHECK:       |      |   }
; CHECK:       |      + END LOOP
; CHECK:       |
; CHECK:       |      if (%[[ALLCLOSE]] != 0)
; CHECK:       |      {
; CHECK:       |         if (%[[FIRST]] == -1.000000e+00)
; CHECK:       |         {
; CHECK:       |            %[[ROWCASE]] = 1;
; CHECK:       |         }
; CHECK:       |         else
; CHECK:       |         {
; CHECK:       |            if (%[[FIRST]] == 0.000000e+00)
; CHECK:       |            {
; CHECK:       |               %[[ROWCASE]] = 2;
; CHECK:       |            }
; CHECK:       |            else
; CHECK:       |            {
; CHECK:       |               if (%[[FIRST]] == 1.000000e+00)
; CHECK:       |               {
; CHECK:       |                  %[[ROWCASE]] = 3;
; CHECK:       |               }
; CHECK:       |            }
; CHECK:       |         }
; CHECK:       |      }
; CHECK:       |   }
; CHECK:       |   (%[[ROWCASES]])[i1] = %[[ROWCASE]]
; CHECK:       + END LOOP

; CHECK:       + DO i1 = 0, 127, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, %M + -1, 1   <DO_LOOP>
; CHECK:       |   |   + DO i3 = 0, %N + -1, 1   <DO_LOOP>
; CHECK:       |   |   |   switch((%[[ROWCASES]])[i3])
; CHECK:       |   |   |   {
; CHECK:       |   |   |   case 1:
; CHECK:       |   |   |      + DO i4 = 0, %K + -1, 1   <DO_LOOP>
; CHECK:       |   |   |      |   %sum = %sum  -  (%B)[(%M * %K) * i1 + i2 + %M * i4];
; CHECK:       |   |   |      + END LOOP
; CHECK:       |   |   |      break;
; CHECK:       |   |   |   case 2:
; CHECK:       |   |   |      break;
; CHECK:       |   |   |   case 3:
; CHECK:       |   |   |      + DO i4 = 0, %K + -1, 1   <DO_LOOP>
; CHECK:       |   |   |      |   %AikBkj = (%B)[(%M * %K) * i1 + i2 + %M * i4];
; CHECK:       |   |   |      |   %sum = %sum  +  %AikBkj;
; CHECK:       |   |   |      + END LOOP
; CHECK:       |   |   |      break;
; CHECK:       |   |   |   default:
; CHECK:       |   |   |      + DO i4 = 0, %K + -1, 1   <DO_LOOP>
; CHECK:       |   |   |      |   %AikBkj = (%A)[%K * i3 + i4]  *  (%B)[(%M * %K) * i1 + i2 + %M * i4];
; CHECK:       |   |   |      |   %sum = %sum  +  %AikBkj;
; CHECK:       |   |   |      + END LOOP
; CHECK:       |   |   |      break;
; CHECK:       |   |   |   }
; CHECK:       |   |   + END LOOP
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

define double @gemm(double* %A, double* %B, i64 %N, i64 %M, i64 %K) #0 {
entry:
  %L1.ztt = icmp sgt i64 %M, 0
  %L2.ztt = icmp sgt i64 %N, 0
  %L3.ztt = icmp sgt i64 %K, 0
  %KM = mul nuw nsw i64 %K, %M
  br label %L0

L0:
  %h = phi i64 [ 0, %entry ], [ %h.next, %L0.latch ]
  %sum = phi double [ 0.0, %entry ], [%sum.final.L1, %L0.latch ]
  %B_mat = mul nuw nsw i64 %h, %KM
  br i1 %L1.ztt, label %L1.pre, label %L0.latch

L1.pre:
  br label %L1

L1:
  %j = phi i64 [ 0, %L1.pre ], [ %j.next, %L1.latch ]
  %sum.L1 = phi double [ %sum, %L1.pre ], [ %sum.final.L2, %L1.latch ]
  br i1 %L2.ztt, label %L2.pre, label %L1.latch

L2.pre:
  br label %L2

L2:
  %i = phi i64 [ 0, %L2.pre ], [ %i.next, %L2.latch ]
  %sum.L2 = phi double [ %sum.L1, %L2.pre ], [ %sum.final.L3, %L2.latch ]
  %A_row = mul nuw nsw i64 %i, %K
  br i1 %L3.ztt, label %L3.pre, label %L2.latch

L3.pre:
  br label %L3

L3:
  %k = phi i64 [ 0, %L3.pre ], [ %k.next, %L3 ]
  %sum.L3 = phi double [ %sum.L2, %L3.pre ], [ %sum.next, %L3 ]
  %A_ind = add nuw nsw i64 %A_row, %k
  %Aikp = getelementptr inbounds double, double* %A, i64 %A_ind
  %Aik = load double, double* %Aikp
  %B_row_off = mul nuw nsw i64 %k, %M
  %B_row = add nuw nsw i64 %B_mat, %B_row_off
  %B_ind = add nuw nsw i64 %B_row, %j
  %Bkjp = getelementptr inbounds double, double* %B, i64 %B_ind
  %Bkj = load double, double* %Bkjp
  %AikBkj = fmul nnan nsz arcp afn reassoc double %Aik, %Bkj
  %sum.next = fadd double %sum.L3, %AikBkj
  %k.next = add nuw nsw i64 %k, 1
  %L3.cond = icmp eq i64 %k.next, %K
  br i1 %L3.cond, label %L3.exit, label %L3

L3.exit:
  %sum.next.lcssa = phi double [ %sum.next, %L3 ]
  br label %L2.latch

L2.latch:
  %sum.final.L3 = phi double [ %sum.L2, %L2 ], [ %sum.next.lcssa, %L3.exit ]
  %i.next = add nuw nsw i64 %i, 1
  %L2.cond = icmp eq i64 %i.next, %N
  br i1 %L2.cond, label %L2.exit, label %L2

L2.exit:
  %sum.final.L3.lcssa = phi double [ %sum.final.L3, %L2.latch ]
  br label %L1.latch

L1.latch:
  %sum.final.L2 = phi double [ %sum.L1, %L1 ], [ %sum.final.L3.lcssa, %L2.exit ]
  %j.next = add nuw nsw i64 %j, 1
  %L1.cond = icmp eq i64 %j.next, %M
  br i1 %L1.cond, label %L1.exit, label %L1

L1.exit:
  %sum.final.L2.lcssa = phi double [ %sum.final.L2, %L1.latch ]
  br label %L0.latch

L0.latch:
  %sum.final.L1 = phi double [ %sum, %L0 ], [ %sum.final.L2.lcssa, %L1.exit ]
  %h.next = add nuw nsw i64 %h, 1
  %L0.cond = icmp eq i64 %h.next, 128
  br i1 %L0.cond, label %exit, label %L0

exit:
  %sum.final.L1.lcssa = phi double [ %sum.final.L1, %L0.latch ]
  ret double %sum.final.L1.lcssa
}

attributes #0 = { "unsafe-fp-math"="true" }
