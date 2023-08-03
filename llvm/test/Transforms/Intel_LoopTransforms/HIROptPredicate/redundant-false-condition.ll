; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check that false condition was removed during the transformation.

; BEGIN REGION { }
;       + DO i1 = 0, 100, 1   <DO_LOOP>
;       |   if (%n == 100)
;       |   {
;       |      %v = (%q)[0];
;       |      if (%v == 0 && undef false undef)
;       |      {
;       |         (%p)[0] = 5;
;       |      }
;       |      else
;       |      {
;       |         (%p)[0] = 1;
;       |      }
;       |   }
;       + END LOOP
; END REGION

; CHECK: modified
; CHECK: + DO i1 = 0, 100, 1
; CHECK-NOT: false
; CHECK-NOT: (%p)[0] = 5
; CHECK: + END LOOP

define void @foo(ptr %p, ptr %q, i64 %n) {
entry:
  br label %loop

loop:
  %i = phi i64 [0, %entry], [%ip, %loop_inc]

  %cx = icmp eq i64 %n, 100
  br i1 %cx, label %first, label %loop_inc

first:
  %v = load i64, ptr %q
  %c1 = icmp eq i64 %v, 0
  %c2 = and i1 %c1, false
  br i1 %c2, label %st1, label %st2

st1:
  store i64 5, ptr %p
  br label %loop_inc

st2:
  store i64 1, ptr %p
  br label %loop_inc

loop_inc:
  %ip = add nsw i64 %i, 1
  %cmp = icmp slt i64 %i, 100
  br i1 %cmp, label %loop, label %exit

exit:
  ret void
}

