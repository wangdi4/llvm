; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-cond-ldst-motion,print<hir>" -aa-pipeline="basic-aa"  < %s -disable-output 2>&1 | FileCheck %s

; This test checks that HIRConditionalLoadStoreMotion uses a common temp when
; hoisting and sinking identical references.

; Print before:

; CHECK:      BEGIN REGION
; CHECK-NEXT:       + DO i1 = 0, 62, 1   <DO_LOOP>
; CHECK-NEXT:       |   if (%which != 0)
; CHECK-NEXT:       |   {
; CHECK-NEXT:       |      %t.then = (%A)[i1] + 1.000000e+00;
; CHECK-NEXT:       |      (%A)[i1] = %t.then;
; CHECK-NEXT:       |   }
; CHECK-NEXT:       |   else
; CHECK-NEXT:       |   {
; CHECK-NEXT:       |      (%A)[i1] = (%A)[i1];
; CHECK-NEXT:       |   }
; CHECK-NEXT:       + END LOOP
; CHECK-NEXT: END REGION

; Print after:

; CHECK:      BEGIN REGION
; CHECK-NEXT:       + DO i1 = 0, 62, 1   <DO_LOOP>
; CHECK-NEXT:       |   %[[MOTIONED:[A-Za-z0-9_.]+]] = (%A)[i1];
; CHECK-NEXT:       |   if (%which != 0)
; CHECK-NEXT:       |   {
; CHECK-NEXT:       |      %t.then = %[[MOTIONED]] + 1.000000e+00;
; CHECK-NEXT:       |      %[[MOTIONED]] = %t.then;
; CHECK-NEXT:       |   }
; CHECK-NEXT:       |   else
; CHECK-NEXT:       |   {
; CHECK-NEXT:       |      %[[MOTIONED]] = %[[MOTIONED]];
; CHECK-NEXT:       |   }
; CHECK-NEXT:       |   (%A)[i1] = %[[MOTIONED]];
; CHECK-NEXT:       + END LOOP
; CHECK-NEXT: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @sametemp(ptr %A, i1 %which) {

entry:
  br label %L1

L1:
  %i = phi i64 [ 0, %entry ], [ %i.next, %L1.latch ]
  %Ai = getelementptr inbounds double, ptr %A, i64 %i
  br i1 %which, label %then, label %else

then:
  %ldval.then = load double, ptr %Ai, align 8
  %t.then = fadd double %ldval.then, 1.0
  store double %t.then, ptr %Ai, align 8
  br label %L1.latch

else:
  %t.else = load double, ptr %Ai, align 8
  store double %t.else, ptr %Ai, align 8
  br label %L1.latch

L1.latch:
  %i.next = add nuw nsw i64 %i, 1
  %cond = icmp ne i64 %i.next, 63
  br i1 %cond, label %L1, label %exit

exit:
  ret void
}
