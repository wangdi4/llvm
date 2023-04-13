; RUN: opt -opaque-pointers=0 -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-dead-store-elimination,print<hir>" 2>&1 < %s | FileCheck %s

; Verify that (%ptr)[1] is not eliminated because the intermediate bitcast load (i16*)(%ptr)[0] overlaps.

; CHECK-NOT: modified

; CHECK: (%ptr)[1] = 100;
; CHECK: %0 = (%ptr)[0];
; CHECK: %1 = (i16*)(%ptr)[0];
; CHECK: (%ptr)[1] = 5;
; CHECK: ret ;

define void @foo(i8* %ptr) {
entry:
  br label %bb

bb:
  %ptr1 = getelementptr inbounds i8, i8* %ptr, i64 1
  store i8 100, i8* %ptr1
  %0 = load i8, i8* %ptr
  %ptr_cast = bitcast i8* %ptr to i16*
  %1 = load i16, i16* %ptr_cast
  store i8 5, i8* %ptr1
  ret void
}

