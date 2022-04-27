; RUN: opt < %s -basic-aa -aa-eval -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s

; Check that "ptrnoalias" on arguments implies "noalias" for loaded pointers %p and %q.

; CHECK-DAG: NoAlias:      float* %p, float* %q
; CHECK-DAG: NoAlias:      float* %"p[i]", float* %q
; CHECK-DAG: NoAlias:      float* %"q[i]", float* %p
; CHECK-DAG: NoAlias:      float* %"p[i]", float* %"q[i]"

%struct.dv = type { float*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

define void @foo(%struct.dv* "ptrnoalias" %a, %struct.dv* "ptrnoalias" %b) {
entry:
  %a_ptr = getelementptr inbounds %struct.dv, %struct.dv* %a, i64 0, i32 0
  %b_ptr = getelementptr inbounds %struct.dv, %struct.dv* %b, i64 0, i32 0
  %q = load float*, float** %a_ptr
  %p = load float*, float** %b_ptr
  br label %loop_header

loop_header:
  br label %loop

loop:
  %i = phi i64 [ %ip1, %loop ], [ 0, %loop_header ]
  %"p[i]" = getelementptr inbounds float, float* %p, i64 %i
  %"*p[i]" = load float, float* %"p[i]"
  %"q[i]" = getelementptr inbounds float, float* %q, i64 %i
  store float %"*p[i]", float* %"q[i]"
  %ip1 = add nuw nsw i64 %i, 1
  %cmp = icmp eq i64 %ip1, 100
  br i1 %cmp, label %loop_exit, label %loop

loop_exit:
  br label %exit

exit:
; dead loads, needed to get aa-eval to trigger
  %ld.p = load float, float* %p, align 8
  %ld.q = load float, float* %q, align 8

  ret void
}

