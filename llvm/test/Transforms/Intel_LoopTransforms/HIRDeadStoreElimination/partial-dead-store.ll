; RUN: opt -opaque-pointers=0 -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-dead-store-elimination,print<hir>" 2>&1 < %s | FileCheck %s

; Verify that (i32*)(%ptr)[0] is not eliminated because the subsequent single byte store only makes it partially dead.

; CHECK-NOT: modified

; CHECK: (i32*)(%ptr)[0] = 1000000;
; CHECK: (%ptr)[0] = 0;
; CHECK: ret ;

define void @foo(i8* %ptr) {
entry:
  br label %bb

bb:
  %ptr_cast = bitcast i8* %ptr to i32*
  store i32 1000000, i32* %ptr_cast
  store i8 0, i8* %ptr
  ret void
}

