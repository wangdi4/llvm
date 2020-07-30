; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-cond-ldst-motion -print-before=hir-cond-ldst-motion -print-after=hir-cond-ldst-motion -disable-output 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-cond-ldst-motion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test checks that HIRConditionalLoadStoreMotion does not hoist/sink
; aliasing loads/stores from an HLIf inside a nested loop where the accesses are
; dependent with direction vector (= =).

; Print before:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 30, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, 63, 1   <DO_LOOP>
; CHECK:       |   |   if (%which != 0)
; CHECK:       |   |   {
; CHECK:       |   |      %stval.then = %what  +  1.000000e+00;
; CHECK:       |   |      (%A)[i1][i2] = %stval.then;
; CHECK:       |   |      %retval = (%A)[i1][i2];
; CHECK:       |   |   }
; CHECK:       |   |   else
; CHECK:       |   |   {
; CHECK:       |   |      (%A)[i1][i2] = %what;
; CHECK:       |   |      %retval.else = (%A)[i1][i2]  +  1.000000e+00;
; CHECK:       |   |      %retval = %retval.else;
; CHECK:       |   |   }
; CHECK:       |   |   %sum = %sum  +  %retval;
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

; Print after:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 30, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, 63, 1   <DO_LOOP>
; CHECK:       |   |   if (%which != 0)
; CHECK:       |   |   {
; CHECK:       |   |      %stval.then = %what  +  1.000000e+00;
; CHECK:       |   |      (%A)[i1][i2] = %stval.then;
; CHECK:       |   |      %retval = (%A)[i1][i2];
; CHECK:       |   |   }
; CHECK:       |   |   else
; CHECK:       |   |   {
; CHECK:       |   |      (%A)[i1][i2] = %what;
; CHECK:       |   |      %retval.else = (%A)[i1][i2]  +  1.000000e+00;
; CHECK:       |   |      %retval = %retval.else;
; CHECK:       |   |   }
; CHECK:       |   |   %sum = %sum  +  %retval;
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define double @nestedeq([64 x double]* %A, double %what, i1 %which) {

entry:
  br label %L1

L1:
  %i = phi i64 [ 0, %entry ], [ %i.next, %L1.latch ]
  %sum = phi double [ 0.0, %entry ], [ %sum.L2.final, %L1.latch ]
  %i.next = add nuw nsw i64 %i, 1
  br label %L2

L2:
  %j = phi i64 [ 0, %L1 ], [ %j.next, %L2.latch ]
  %sum.L2 = phi double [ %sum, %L1 ], [ %sum.L2.next, %L2.latch ]
  br i1 %which, label %then, label %else

then:
  %stval.then = fadd double %what, 1.0
  %stptr.then = getelementptr inbounds [64 x double], [64 x double]* %A, i64 %i, i64 %j
  store double %stval.then, double* %stptr.then, align 8
  %ldptr.then = getelementptr inbounds [64 x double], [64 x double]* %A, i64 %i, i64 %j
  %ldval.then = load double, double* %ldptr.then, align 8
  br label %L2.latch

else:
  %stptr.else = getelementptr inbounds [64 x double], [64 x double]* %A, i64 %i, i64 %j
  store double %what, double* %stptr.else, align 8
  %ldptr.else = getelementptr inbounds [64 x double], [64 x double]* %A, i64 %i, i64 %j
  %ldval.else = load double, double* %ldptr.else, align 8
  %retval.else = fadd double %ldval.else, 1.0
  br label %L2.latch

L2.latch:
  %retval = phi double [ %ldval.then, %then ], [ %retval.else, %else ]
  %sum.L2.next = fadd double %sum.L2, %retval
  %j.next = add nuw nsw i64 %j, 1
  %cond.L2 = icmp ne i64 %j.next, 64
  br i1 %cond.L2, label %L2, label %L1.latch

L1.latch:
  %sum.L2.final = phi double [ %sum.L2.next, %L2.latch ]
  %cond.L1 = icmp ne i64 %i.next, 31
  br i1 %cond.L1, label %L1, label %exit

exit:
  %sum.L1.final = phi double [ %sum.L2.final, %L1.latch ]
  ret double %sum.L1.final
}

