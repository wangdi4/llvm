; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; RUN: opt -whole-program-assume -dtransanalysis -hir-ssa-deconstruction -hir-temp-cleanup -hir-rowwise-mv -print-before=hir-rowwise-mv -print-after=hir-rowwise-mv -disable-output 2>&1 < %s | FileCheck %s

; RUN: opt -whole-program-assume -passes='require<dtransanalysis>,function(hir-ssa-deconstruction,hir-temp-cleanup,hir-rowwise-mv)' -print-before=hir-rowwise-mv -print-after=hir-rowwise-mv -disable-output 2>&1 < %s | FileCheck %s

; This test checks that the basic row-wise multiversioning transformation
; generates the expected code when using DTrans analysis to determine likely
; values.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Print before:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 31, 1   <DO_LOOP>
; CHECK:       |   %b = (%BI)[0].1;
; CHECK:       |   if ((%b)[0] !=u 0.000000e+00)
; CHECK:       |   {
; CHECK:       |      + DO i2 = 0, 63, 1   <DO_LOOP>
; CHECK:       |      |   + DO i3 = 0, 127, 1   <DO_LOOP>
; CHECK:       |      |   |   %Aijbj = (%A)[128 * i2 + i3]  *  (%b)[i3];
; CHECK:       |      |   |   %sum = %sum  +  %Aijbj;
; CHECK:       |      |   + END LOOP
; CHECK:       |      + END LOOP
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK: END REGION

; Print after:

; (1.0 is not expected here because there are no assignments of 1.0 to the array
; in structinit below)

; CHECK: BEGIN REGION
; CHECK:          %[[NEEDCHECK:[A-Za-z0-9_.]+]] = 1;
; CHECK:          %[[ROWCASE:[A-Za-z0-9_.]+]] = 0;
; CHECK:       + DO i1 = 0, 31, 1   <DO_LOOP>
; CHECK:       |   %b = (%BI)[0].1;
; CHECK:       |   if ((%b)[0] !=u 0.000000e+00)
; CHECK:       |   {
; CHECK:       |      if (%[[NEEDCHECK]] != 0)
; CHECK:       |      {
; CHECK:       |         %[[FIRST:[A-Za-z0-9_.]+]] = (%b)[0];
; CHECK:       |         %[[ALLCLOSE:[A-Za-z0-9_.]+]] = 1;
; CHECK:       |
; CHECK:       |         + DO i2 = 0, 126, 1   <DO_LOOP>
; CHECK:       |         |   %[[NEXT:[A-Za-z0-9_.]+]] = (%b)[i2 + 1];
; CHECK:       |         |   %[[DIFF:[A-Za-z0-9_.]+]] = %[[NEXT]]  -  %[[FIRST]];
; CHECK:       |         |   %[[ABSDIFF:[A-Za-z0-9_.]+]] = @llvm.fabs.f64(%[[DIFF]]);
; CHECK:       |         |   if (%[[ABSDIFF]] >u 1.000000e-04)
; CHECK:       |         |   {
; CHECK:       |         |      %[[ALLCLOSE]] = 0;
; CHECK:       |         |   }
; CHECK:       |         + END LOOP
; CHECK:       |
; CHECK:       |         if (%[[ALLCLOSE]] != 0)
; CHECK:       |         {
; CHECK:       |            if (%[[FIRST]] == -1.000000e+00)
; CHECK:       |            {
; CHECK:       |               %[[ROWCASE]] = 1;
; CHECK:       |            }
; CHECK:       |            else
; CHECK:       |            {
; CHECK:       |               if (%[[FIRST]] == 0.000000e+00)
; CHECK:       |               {
; CHECK:       |                  %[[ROWCASE]] = 2;
; CHECK:       |               }
; CHECK-NEXT:  |            }
; CHECK:       |         }
; CHECK:       |         %[[NEEDCHECK]] = 0;
; CHECK:       |      }
; CHECK:       |
; CHECK:       |      + DO i2 = 0, 63, 1   <DO_LOOP>
; CHECK:       |      |   switch(%[[ROWCASE]])
; CHECK:       |      |   {
; CHECK:       |      |   case 1:
; CHECK:       |      |      + DO i3 = 0, 127, 1   <DO_LOOP>
; CHECK:       |      |      |   %sum = %sum  - (%A)[128 * i2 + i3];
; CHECK:       |      |      + END LOOP
; CHECK:       |      |      break;
; CHECK:       |      |   case 2:
; CHECK-NEXT:  |      |      break;
; CHECK-NEXT:  |      |   default:
; CHECK:       |      |      + DO i3 = 0, 127, 1   <DO_LOOP>
; CHECK:       |      |      |   %Aijbj = (%A)[128 * i2 + i3]  *  (%b)[i3];
; CHECK:       |      |      |   %sum = %sum  +  %Aijbj;
; CHECK:       |      |      + END LOOP
; CHECK:       |      |      break;
; CHECK:       |      |   }
; CHECK:       |      + END LOOP
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK: END REGION

%struct.BInfo = type { i32, double* }

declare noalias i8* @malloc(i64)

define double @gemv(double* %A, %struct.BInfo* %BI) #0 {
entry:
  br label %L0

L0:
  %h = phi i32 [ 0, %entry ], [ %h.next, %L0.latch ]
  %sum = phi double [ 0.0, %entry ], [ %sum.final, %L0.latch ]
  %bptr = getelementptr inbounds %struct.BInfo, %struct.BInfo* %BI, i32 0, i32 1
  %b = load double*, double** %bptr
  %bfirst = load double, double* %b
  %bcheck = fcmp une double %bfirst, 0.0
  br i1 %bcheck, label %L1.pre, label %L0.latch

L1.pre:
  br label %L1

L1:
  %i = phi i32 [ 0, %L1.pre ], [ %i.next, %L2.exit ]
  %sum.L1 = phi double [ %sum, %L1.pre ], [ %sum.next.lcssa, %L2.exit ]
  %A_row = mul nuw nsw i32 %i, 128
  br label %L2

L2:
  %j = phi i32 [ 0, %L1 ], [ %j.next, %L2 ]
  %sum.L2 = phi double [ %sum.L1, %L1 ], [ %sum.next, %L2 ]
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
  br label %L0.latch

L0.latch:
  %sum.final = phi double [ %sum, %L0 ], [ %sum.next.lcssa.lcssa, %L1.exit ]
  %h.next = add nuw nsw i32 %h, 1
  %L0.cond = icmp eq i32 %h.next, 32
  br i1 %L0.cond, label %L0.exit, label %L0

L0.exit:
  %sum.final.lcssa = phi double [ %sum.final, %L0.latch ]
  ret double %sum.final.lcssa
}

@GBI = global %struct.BInfo zeroinitializer, align 8

define void @structinit() {
  %b8 = tail call noalias dereferenceable_or_null(1024) i8* @malloc(i64 1024)
  %b = bitcast i8* %b8 to double*
  store double* %b, double** getelementptr inbounds (%struct.BInfo, %struct.BInfo* @GBI, i64 0, i32 1), align 8
  %b2 = getelementptr inbounds double, double* %b, i64 2
  store double -1.0, double* %b2, align 8
  %b3 = getelementptr inbounds double, double* %b, i64 3
  store double 0.0, double* %b3, align 8
  ret void
}

attributes #0 = { "unsafe-fp-math"="true" }

; end INTEL_FEATURE_SW_DTRANS
