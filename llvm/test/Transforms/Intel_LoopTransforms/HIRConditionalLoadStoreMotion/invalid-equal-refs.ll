; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-cond-ldst-motion" -print-before=hir-cond-ldst-motion -print-after=hir-cond-ldst-motion -hir-details < %s -disable-output 2>&1 | FileCheck %s

; Verify that we do not sink (%A)[i1 + 1] out of if-else because the refs are
; not really equal. The ref in if case has a zext() in the subscript which can
; result in wraparound whereas the ref in else case does not have any cast.

; CHECK: Dump Before

; CHECK: + DO i64 i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   if (%cond != 0)
; CHECK: |   {
; CHECK: |      %stval.then = %init  +  1.000000e+00;
; CHECK: |      (%A)[i1 + 1] = %stval.then;
; CHECK: |         <LVAL-REG> {al:8}(LINEAR ptr %A)[LINEAR zext.i32.i64(i1 + 1)]
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      (%A)[i1 + 1] = %init;
; CHECK: |         <LVAL-REG> {al:8}(LINEAR ptr %A)[LINEAR i64 i1 + 1]
; CHECK: |   }
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK: + DO i64 i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   if (%cond != 0)
; CHECK: |   {
; CHECK: |      %stval.then = %init  +  1.000000e+00;
; CHECK: |      (%A)[i1 + 1] = %stval.then;
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      (%A)[i1 + 1] = %init;
; CHECK: |   }
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr %A, double %init, i1 %cond, i64 %n) {

entry:
  br label %L1

L1:
  %i = phi i64 [ 0, %entry ], [ %i.next, %L1.latch ]
  %i.next = add nuw nsw i64 %i, 1
  br i1 %cond, label %then, label %else

then:
  %stval.then = fadd double %init, 1.0
  %and = and i64 %i.next, 4294967295
  %stptr.then = getelementptr inbounds double, ptr %A, i64 %and
  store double %stval.then, ptr %stptr.then, align 8
  br label %L1.latch

else:
  %stptr.else = getelementptr inbounds double, ptr %A, i64 %i.next
  store double %init, ptr %stptr.else, align 8
  br label %L1.latch

L1.latch:
  %cmp = icmp ne i64 %i.next, %n
  br i1 %cmp, label %L1, label %exit

exit:
  ret void
}
