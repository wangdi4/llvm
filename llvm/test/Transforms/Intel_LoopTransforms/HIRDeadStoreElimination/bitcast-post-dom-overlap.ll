; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,print<hir>,hir-dead-store-elimination,print<hir>" -disable-output 2>&1 < %s | FileCheck %s

; Verify that (%ptr)[0] is eliminated even though load and post-dominating store overlap.

; Print Before-

; CHECK: Function: foo

; CHECK: (%ptr)[0] = 100;
; CHECK: %0 = (%ptr)[1];
; CHECK: (%ptr)[0] = 5;

; Print After-

; CHECK: Function: foo

; CHECK-NOT: (%ptr)[0] = 100;


define void @foo(ptr %ptr) {
entry:
  br label %bb

bb:
  store i8 100, ptr %ptr
  %ptr1 = getelementptr inbounds i8, ptr %ptr, i64 1
  %0 = load i8, ptr %ptr1
  store i16 5, ptr %ptr
  ret void
}

