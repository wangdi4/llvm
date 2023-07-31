; This test case is turned off since we disabled partial hoisting due to
; nested Ifs (CMPLRLLVM-44207).

; XFAIL: *
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-cond-ldst-motion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test checks that hoisting a legal subset of an initial set of loads works
; as intended, only performing the legal hoists.

; Print before:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 62, 1   <DO_LOOP>
; CHECK:       |   if (%which != 0)
; CHECK:       |   {
; CHECK:       |      (%B)[i1] = (%A)[i1];
; CHECK:       |      %retval = (%A)[i1];
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      %retval.else = (%A)[i1]  +  1.000000e+00;
; CHECK:       |      %retval = %retval.else;
; CHECK:       |   }
; CHECK:       |   %sum = %sum  +  %retval;
; CHECK:       + END LOOP
; CHECK: END REGION

; Print after:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 62, 1   <DO_LOOP>
; CHECK:       |   %[[HOISTED:[A-Za-z0-9_.]+]] = (%A)[i1];
; CHECK:       |   if (%which != 0)
; CHECK:       |   {
; CHECK:       |      (%B)[i1] = %[[HOISTED]];
; CHECK:       |      %retval = (%A)[i1];
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      %retval.else = %[[HOISTED]]  +  1.000000e+00;
; CHECK:       |      %retval = %retval.else;
; CHECK:       |   }
; CHECK:       |   %sum = %sum  +  %retval;
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define double @partial_hoist(ptr %A, ptr %B, i1 %which) {

entry:
  br label %L1

L1:
  %i = phi i64 [ 0, %entry ], [ %i.next, %L1.latch ]
  %sum = phi double [ 0.0, %entry ], [ %sum.next, %L1.latch ]
  %i.next = add nuw nsw i64 %i, 1
  br i1 %which, label %then, label %else

then:
  %ldptr.then = getelementptr inbounds double, ptr %A, i64 %i
  %ldval.then = load double, ptr %ldptr.then, align 8
  %stptr = getelementptr inbounds double, ptr %B, i64 %i
  store double %ldval.then, ptr %stptr, align 8
  %retval.then = load double, ptr %ldptr.then, align 8
  br label %L1.latch

else:
  %ldptr.else = getelementptr inbounds double, ptr %A, i64 %i
  %ldval.else = load double, ptr %ldptr.else, align 8
  %retval.else = fadd double %ldval.else, 1.0
  br label %L1.latch

L1.latch:
  %retval = phi double [ %retval.then, %then ], [ %retval.else, %else ]
  %sum.next = fadd double %sum, %retval
  %cond = icmp ne i64 %i.next, 63
  br i1 %cond, label %L1, label %exit

exit:
  %sum.next.lcssa = phi double [ %sum.next, %L1.latch ]
  ret double %sum.next.lcssa
}
