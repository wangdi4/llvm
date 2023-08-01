; RUN: opt < %s -passes='hir-ssa-deconstruction,print<hir>' -hir-create-function-level-region 2>&1 | FileCheck %s

; Verify that we can create a region when a vector field in a struct is accessed
; by a GEP. This doesn't require supporting vector type indexing which the
; framework currently lacks.


; CHECK: BEGIN REGION { }
; CHECK: (%vec.ptr.arr)[0].0 = %val;
; CHECK: ret ;
; CHECK: END REGION


%struct = type { <8 x i64> }


define void @foo(ptr %vec.ptr.arr, <8 x i64> %val) {
entry:
  br label %bb

bb:                                      ; preds = %bb, %entry
  %vec.ptr.gep = getelementptr inbounds %struct, ptr %vec.ptr.arr, i64 0, i32 0
  store <8 x i64> %val, ptr %vec.ptr.gep
  ret void
}

