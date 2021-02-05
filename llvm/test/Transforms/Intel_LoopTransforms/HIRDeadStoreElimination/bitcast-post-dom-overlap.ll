; RUN: opt -hir-create-function-level-region -hir-ssa-deconstruction -hir-dead-store-elimination -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination < %s 2>&1 | FileCheck %s
; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,print<hir>,hir-dead-store-elimination,print<hir>" 2>&1 < %s | FileCheck %s

; Verify that (%ptr)[0] is eliminated even though load and post-dominating store overlap.

; Print Before-

; CHECK: Function: foo

; CHECK: (%ptr)[0] = 100;
; CHECK: %0 = (%ptr)[1];
; CHECK: (i16*)(%ptr)[0] = 5;

; Print After-

; CHECK: Function: foo

; CHECK-NOT: (%ptr)[0] = 100;


define void @foo(i8* %ptr) {
entry:
  br label %bb

bb:
  store i8 100, i8* %ptr
  %ptr1 = getelementptr inbounds i8, i8* %ptr, i64 1
  %0 = load i8, i8* %ptr1
  %ptr_cast = bitcast i8* %ptr to i16*
  store i16 5, i16* %ptr_cast
  ret void
}

