; RUN: opt < %s -passes='print<hir-region-identification>' 2>&1 | FileCheck %s

; Verify that we can create a region when pointer to vector is
; being accessed in a GEP. This doesn't require supporting vector type
; indexing which the framework currently lacks.


; CHECK: Region 1


define void @foo([10 x <4 x float>*]* %vec.ptr.arr, <4 x float>*** %vec.ptr.ptr) {
entry:
  br label %for.body

for.body:                                      ; preds = %for.body, %entry
  %i = phi i64 [ 0, %entry ], [ %i.next, %for.body ]
  %vec.elem = getelementptr inbounds [10 x <4 x float>*], [10 x <4 x float>*]* %vec.ptr.arr, i64 0, i64 %i
  store <4 x float>** %vec.elem, <4 x float>*** %vec.ptr.ptr
  %i.next = add nsw i64 %i, 1
  %cmp = icmp eq i64 %i.next, 10
  br i1 %cmp, label %exit, label %for.body

exit:
  ret void
}

