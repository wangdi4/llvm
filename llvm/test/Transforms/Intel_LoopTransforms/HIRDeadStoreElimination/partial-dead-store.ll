; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-dead-store-elimination,print<hir>" 2>&1 < %s | FileCheck %s

; Verify that first store to (%ptr)[0] is not eliminated because the subsequent single byte store only makes it partially dead.

; CHECK-NOT: modified

; CHECK: (%ptr)[0] = 1000000;
; CHECK: (%ptr)[0] = 0;
; CHECK: ret ;

define void @foo(ptr %ptr) {
entry:
  br label %bb

bb:
  store i32 1000000, ptr %ptr
  store i8 0, ptr %ptr
  ret void
}

