; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-cond-ldst-motion,print<hir>" -aa-pipeline="basic-aa"  < %s -disable-output 2>&1 | FileCheck %s

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-cond-ldst-motion" -print-changed < %s -disable-output 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED


; This test checks that HIRConditionalLoadStoreMotion will *not* hoist/sink
; simple possibly-aliased loads and stores from an HLIf.

; Print before:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 63, 1   <DO_LOOP>
; CHECK:       |   if (%which != 0)
; CHECK:       |   {
; CHECK:       |      %stval.then = %what  +  1.000000e+00;
; CHECK:       |      (%A)[%where] = %stval.then;
; CHECK:       |      %retval = (%A)[i1];
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      (%A)[%where] = %what;
; CHECK:       |      %retval.else = (%A)[i1]  +  1.000000e+00;
; CHECK:       |      %retval = %retval.else;
; CHECK:       |   }
; CHECK:       |   %sum = %sum  +  %retval;
; CHECK:       + END LOOP
; CHECK: END REGION

; Print after: (no changes expected)

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 63, 1   <DO_LOOP>
; CHECK:       |   if (%which != 0)
; CHECK:       |   {
; CHECK:       |      %stval.then = %what  +  1.000000e+00;
; CHECK:       |      (%A)[%where] = %stval.then;
; CHECK:       |      %retval = (%A)[i1];
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      (%A)[%where] = %what;
; CHECK:       |      %retval.else = (%A)[i1]  +  1.000000e+00;
; CHECK:       |      %retval = %retval.else;
; CHECK:       |   }
; CHECK:       |   %sum = %sum  +  %retval;
; CHECK:       + END LOOP
; CHECK: END REGION


; Verify that pass is not dumped with print-changed if it bails out.

; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED-NOT: Dump After HIRConditionalLoadStoreMotion


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define double @nohoist(double* %A, double %what, i64 %where, i1 %which) {

entry:
  br label %L1

L1:
  %i = phi i64 [ 0, %entry ], [ %i.next, %L1.latch ]
  %sum = phi double [ 0.0, %entry ], [ %sum.next, %L1.latch ]
  br i1 %which, label %then, label %else

then:
  %stval.then = fadd double %what, 1.0
  %stptr.then = getelementptr inbounds double, double* %A, i64 %where
  store double %stval.then, double* %stptr.then, align 8
  %ldptr.then = getelementptr inbounds double, double* %A, i64 %i
  %ldval.then = load double, double* %ldptr.then, align 8
  br label %L1.latch

else:
  %stptr.else = getelementptr inbounds double, double* %A, i64 %where
  store double %what, double* %stptr.else, align 8
  %ldptr.else = getelementptr inbounds double, double* %A, i64 %i
  %ldval.else = load double, double* %ldptr.else, align 8
  %retval.else = fadd double %ldval.else, 1.0
  br label %L1.latch

L1.latch:
  %retval = phi double [ %ldval.then, %then ], [ %retval.else, %else ]
  %sum.next = fadd double %sum, %retval
  %i.next = add nuw nsw i64 %i, 1
  %cond = icmp ne i64 %i.next, 64
  br i1 %cond, label %L1, label %exit

exit:
  %sum.next.lcssa = phi double [ %sum.next, %L1.latch ]
  ret double %sum.next.lcssa
}
