; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-rowwise-mv -print-before=hir-rowwise-mv -hir-rowwise-mv-skip-dtrans -print-after=hir-rowwise-mv -disable-output 2>&1 < %s | FileCheck %s

; This test checks that the basic row-wise multiversioning transformation
; generates the expected code.

; This test also checks the opt report functionality for this pass, but this is
; only enabled for internal builds.
; REQUIRES: intel_internal_build

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Print before:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 63, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, 127, 1   <DO_LOOP>
; CHECK:       |   |   %Aijbj = (%A)[128 * i1 + i2]  *  (%b)[i2];
; CHECK:       |   |   %sum = %sum  +  %Aijbj;
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

; Print after:

; CHECK: BEGIN REGION
; CHECK:       %[[FIRST:[A-Za-z0-9_.]+]] = (%b)[0];
; CHECK:       %[[ALLCLOSE:[A-Za-z0-9_.]+]] = 1;

; CHECK:       + DO i1 = 0, 126, 1   <DO_LOOP>
; CHECK:       |   %[[NEXT:[A-Za-z0-9_.]+]] = (%b)[i1 + 1];
; CHECK:       |   %[[DIFF:[A-Za-z0-9_.]+]] = %[[NEXT]]  -  %[[FIRST]];
; CHECK:       |   %[[ABSDIFF:[A-Za-z0-9_.]+]] = @llvm.fabs.f64(%[[DIFF]]);
; CHECK:       |   if (%[[ABSDIFF]] >u 1.000000e-04)
; CHECK:       |   {
; CHECK:       |      %[[ALLCLOSE]] = 0;
; CHECK:       |   }
; CHECK:       + END LOOP

; CHECK:       %[[ROWCASE:[A-Za-z0-9_.]+]] = 0;
; CHECK:       if (%[[ALLCLOSE]] != 0)
; CHECK:       {
; CHECK:          if (%[[FIRST]] == -1.000000e+00)
; CHECK:          {
; CHECK:             %[[ROWCASE]] = 1;
; CHECK:          }
; CHECK:          else
; CHECK:          {
; CHECK:             if (%[[FIRST]] == 0.000000e+00)
; CHECK:             {
; CHECK:                %[[ROWCASE]] = 2;
; CHECK:             }
; CHECK:             else
; CHECK:             {
; CHECK:                if (%[[FIRST]] == 1.000000e+00)
; CHECK:                {
; CHECK:                   %[[ROWCASE]] = 3;
; CHECK:                }
; CHECK:             }
; CHECK:          }
; CHECK:       }

; CHECK:       + DO i1 = 0, 63, 1   <DO_LOOP>
; CHECK:       |   switch(%[[ROWCASE]])
; CHECK:       |   {
; CHECK:       |   case 1:
; CHECK:       |      + DO i2 = 0, 127, 1   <DO_LOOP>
; CHECK:       |      |   %sum = %sum  - (%A)[128 * i1 + i2];
; CHECK:       |      + END LOOP
; CHECK:       |      break;
; CHECK:       |   case 2:
; CHECK-NEXT:  |      break;
; CHECK:       |   case 3:
; CHECK:       |      + DO i2 = 0, 127, 1   <DO_LOOP>
; CHECK:       |      |   %Aijbj = (%A)[128 * i1 + i2];
; CHECK:       |      |   %sum = %sum  +  %Aijbj;
; CHECK:       |      + END LOOP
; CHECK:       |      break;
; CHECK:       |   default:
; CHECK:       |      + DO i2 = 0, 127, 1   <DO_LOOP>
; CHECK:       |      |   %Aijbj = (%A)[128 * i1 + i2]  *  (%b)[i2];
; CHECK:       |      |   %sum = %sum  +  %Aijbj;
; CHECK:       |      + END LOOP
; CHECK:       |      break;
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK: END REGION

; Opt report:
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-rowwise-mv -hir-cg -simplifycfg -intel-ir-optreport-emitter -hir-rowwise-mv-skip-dtrans -force-hir-cg -intel-opt-report=low -disable-output 2>&1 < %s | FileCheck %s -check-prefix=OPTREPORT

; OPTREPORT: LOOP BEGIN
; OPTREPORT: <Probe loop for row-wise multiversioning>
; OPTREPORT: LOOP END

; OPTREPORT: LOOP BEGIN

; OPTREPORT:     LOOP BEGIN
; OPTREPORT:     <Row-wise multiversioned loop for value 1.000000e+00>
; OPTREPORT:     LOOP END

; OPTREPORT:     LOOP BEGIN
; OPTREPORT:     <Row-wise multiversioned loop for value -1.000000e+00>
; OPTREPORT:     LOOP END

; OPTREPORT:     LOOP BEGIN
; OPTREPORT:     <Row-wise multiversioned loop for value 0.000000e+00>
; OPTREPORT:     LOOP END

; OPTREPORT:     LOOP BEGIN
; OPTREPORT:         remark #25581: Loop has been row-wise multiversioned
; OPTREPORT:     LOOP END
; OPTREPORT: LOOP END

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
