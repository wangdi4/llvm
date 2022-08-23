; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; RUN: opt -whole-program-assume -hir-create-function-level-region -dtransanalysis -hir-ssa-deconstruction -hir-temp-cleanup -hir-rowwise-mv -print-before=hir-rowwise-mv -print-after=hir-rowwise-mv -disable-output 2>&1 < %s | FileCheck %s

; RUN: opt -whole-program-assume -hir-create-function-level-region -passes='require<dtransanalysis>,function(hir-ssa-deconstruction,hir-temp-cleanup,hir-rowwise-mv)' -print-before=hir-rowwise-mv -print-after=hir-rowwise-mv -disable-output 2>&1 < %s | FileCheck %s


; This test checks that unknown loops don't interfere with the DTrans checks.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Print before:

; CHECK:      BEGIN REGION
; CHECK-NEXT:       %b = (%BI)[0].1;
; CHECK-NEXT:       %sum = 0.000000e+00;
;
; CHECK:            + UNKNOWN LOOP i1
; CHECK-NEXT:       |   <i1 = 0>
; CHECK-NEXT:       |   L0:
; CHECK-NEXT:       |
; CHECK-NEXT:       |   + DO i2 = 0, 63, 1   <DO_LOOP>
; CHECK-NEXT:       |   |   + DO i3 = 0, 127, 1   <DO_LOOP>
; CHECK-NEXT:       |   |   |   %Aijbj = (%A)[128 * i2 + i3]  *  (%b)[i3];
; CHECK-NEXT:       |   |   |   %sum = %sum  +  %Aijbj;
; CHECK-NEXT:       |   |   + END LOOP
; CHECK-NEXT:       |   + END LOOP
; CHECK-NEXT:       |
; CHECK-NEXT:       |   if ((%b)[i1] !=u 0.000000e+00)
; CHECK-NEXT:       |   {
; CHECK-NEXT:       |      <i1 = i1 + 1>
; CHECK-NEXT:       |      goto L0;
; CHECK-NEXT:       |   }
; CHECK-NEXT:       + END LOOP
;
; CHECK:            ret %sum;
; CHECK-NEXT: END REGION

; Print after:

; (1.0 is not expected here because there are no assignments of 1.0 to the array
; in structinit below)

; CHECK:      BEGIN REGION
; CHECK-NEXT:       %b = (%BI)[0].1;
; CHECK-NEXT:       %sum = 0.000000e+00;
;
; CHECK:               %[[NEEDCHECK:[A-Za-z0-9_.]+]] = 1;
; CHECK-NEXT:          %[[ROWCASE:[A-Za-z0-9_.]+]] = 0;
; CHECK-NEXT:       + UNKNOWN LOOP i1
; CHECK-NEXT:       |   <i1 = 0>
; CHECK-NEXT:       |   L0:
; CHECK-NEXT:       |   if (%[[NEEDCHECK]] != 0)
; CHECK-NEXT:       |   {
; CHECK-NEXT:       |      %[[FIRST:[A-Za-z0-9_.]+]] = (%b)[0];
; CHECK-NEXT:       |      %[[ALLCLOSE:[A-Za-z0-9_.]+]] = 1;
; CHECK-NEXT:       |
; CHECK-NEXT:       |      + DO i2 = 0, 126, 1   <DO_LOOP>
; CHECK-NEXT:       |      |   %[[NEXT:[A-Za-z0-9_.]+]] = (%b)[i2 + 1];
; CHECK-NEXT:       |      |   %[[DIFF:[A-Za-z0-9_.]+]] = %[[NEXT]]  -  %[[FIRST]];
; CHECK-NEXT:       |      |   %[[ABSDIFF:[A-Za-z0-9_.]+]] = @llvm.fabs.f64(%[[DIFF]]);
; CHECK-NEXT:       |      |   if (%[[ABSDIFF]] >u 1.000000e-04)
; CHECK-NEXT:       |      |   {
; CHECK-NEXT:       |      |      %[[ALLCLOSE]] = 0;
; CHECK-NEXT:       |      |   }
; CHECK-NEXT:       |      + END LOOP
; CHECK-NEXT:       |
; CHECK-NEXT:       |      if (%[[ALLCLOSE]] != 0)
; CHECK-NEXT:       |      {
; CHECK-NEXT:       |         if (%[[FIRST]] == -1.000000e+00)
; CHECK-NEXT:       |         {
; CHECK-NEXT:       |            %[[ROWCASE]] = 1;
; CHECK-NEXT:       |         }
; CHECK-NEXT:       |         else
; CHECK-NEXT:       |         {
; CHECK-NEXT:       |            if (%[[FIRST]] == 0.000000e+00)
; CHECK-NEXT:       |            {
; CHECK-NEXT:       |               %[[ROWCASE]] = 2;
; CHECK-NEXT:       |            }
; CHECK-NEXT:       |         }
; CHECK-NEXT:       |      }
; CHECK-NEXT:       |      %[[NEEDCHECK]] = 0;
; CHECK-NEXT:       |   }
; CHECK-NEXT:       |
; CHECK-NEXT:       |   + DO i2 = 0, 63, 1   <DO_LOOP>
; CHECK-NEXT:       |   |   switch(%[[ROWCASE]])
; CHECK-NEXT:       |   |   {
; CHECK-NEXT:       |   |   case 1:
; CHECK-NEXT:       |   |      + DO i3 = 0, 127, 1   <DO_LOOP>
; CHECK-NEXT:       |   |      |   %sum = %sum  - (%A)[128 * i2 + i3];
; CHECK-NEXT:       |   |      + END LOOP
; CHECK-NEXT:       |   |      break;
; CHECK-NEXT:       |   |   case 2:
; CHECK-NEXT:       |   |      break;
; CHECK-NEXT:       |   |   default:
; CHECK-NEXT:       |   |      + DO i3 = 0, 127, 1   <DO_LOOP>
; CHECK-NEXT:       |   |      |   %Aijbj = (%A)[128 * i2 + i3]  *  (%b)[i3];
; CHECK-NEXT:       |   |      |   %sum = %sum  +  %Aijbj;
; CHECK-NEXT:       |   |      + END LOOP
; CHECK-NEXT:       |   |      break;
; CHECK-NEXT:       |   |   }
; CHECK-NEXT:       |   + END LOOP
; CHECK-NEXT:       |
; CHECK-NEXT:       |   if ((%b)[i1] !=u 0.000000e+00)
; CHECK-NEXT:       |   {
; CHECK-NEXT:       |      <i1 = i1 + 1>
; CHECK-NEXT:       |      goto L0;
; CHECK-NEXT:       |   }
; CHECK-NEXT:       + END LOOP
;
; CHECK:            ret %sum;
; CHECK-NEXT: END REGION

%struct.BInfo = type { i32, double* }

declare noalias i8* @malloc(i64)

define double @gemv(double* %A, %struct.BInfo* %BI) #0 {
entry:
  br label %L0.pre

L0.pre:
  %bptr = getelementptr inbounds %struct.BInfo, %struct.BInfo* %BI, i32 0, i32 1
  %b = load double*, double** %bptr
  br label %L0

L0:
  %h = phi i32 [ 0, %L0.pre ], [ %h.next, %L1.exit ]
  %sum = phi double [ 0.0, %L0.pre ], [ %sum.next.lcssa.lcssa, %L1.exit ]
  br label %L1

L1:
  %i = phi i32 [ 0, %L0 ], [ %i.next, %L2.exit ]
  %sum.L1 = phi double [ %sum, %L0 ], [ %sum.next.lcssa, %L2.exit ]
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
  %h.next = add nuw nsw i32 %h, 1
  %bhp = getelementptr inbounds double, double* %b, i32 %h
  %bh = load double, double* %bhp
  %L0.cond = fcmp oeq double %bh, 0.0
  br i1 %L0.cond, label %L0.exit, label %L0

L0.exit:
  %sum.final = phi double [ %sum.next.lcssa.lcssa, %L1.exit ]
  ret double %sum.final
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
