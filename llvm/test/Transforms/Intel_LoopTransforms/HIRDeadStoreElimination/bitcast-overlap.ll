; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-dead-store-elimination,print<hir>" 2>&1 < %s | FileCheck %s

; Verify that (%ptr)[1] is not eliminated because the intermediate load (%ptr)[0] with type i16 overlaps.

; CHECK-NOT: modified

; CHECK: (%ptr)[1] = 100;
; CHECK: %0 = (%ptr)[0];
; CHECK: %1 = (%ptr)[0];
; CHECK: (%ptr)[1] = 5;
; CHECK: ret ;

define void @foo(ptr %ptr) {
entry:
  br label %bb

bb:
  %ptr1 = getelementptr inbounds i8, ptr %ptr, i64 1
  store i8 100, ptr %ptr1
  %0 = load i8, ptr %ptr
  %1 = load i16, ptr %ptr
  store i8 5, ptr %ptr1
  ret void
}

