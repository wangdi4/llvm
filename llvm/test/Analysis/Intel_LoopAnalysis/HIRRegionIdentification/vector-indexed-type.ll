; REQUIRES: asserts

; RUN: opt < %s -passes='print<hir-region-identification>' -hir-create-function-level-region -debug-only=hir-region-identification 2>&1 | FileCheck %s

; Verify that we give up on GEP which is accessing index of a vector.
; Framework currently does not support vector indexing.


; CHECK: Bailing out on instruction
; CHECK: %vec.elem.gep = getelementptr inbounds


%struct = type { <8 x i64> }


define void @foo(ptr %vec.ptr.arr, i64 %val) {
entry:
  br label %bb

bb:                                      ; preds = %bb, %entry
  %vec.elem.gep = getelementptr inbounds %struct, ptr %vec.ptr.arr, i64 0, i32 0, i32 4
  store i64 %val, ptr %vec.elem.gep
  ret void
}

