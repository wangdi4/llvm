; RUN: opt -passes=inline < %s -S -o - | FileCheck %s

; Check that "ptrnoalias" on arguments will be converted into
; alias.scope/noalias metadata after inlining.

; CHECK: %"p[i].i" = getelementptr inbounds float, float* %p.i, i64 %i.i
; CHECK: %"*p[i].i" = load float, float* %"p[i].i", align 4, !alias.scope ![[S1:.*]], !noalias ![[S2:.*]]
; CHECK: %"q[i].i" = getelementptr inbounds float, float* %q.i, i64 %i.i
; CHECK: store float %"*p[i].i", float* %"q[i].i", align 4, !alias.scope ![[S2]], !noalias ![[S1]]

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
  ret void
}

define void @bar(%struct.dv* %a, %struct.dv* %b) {
  call void @foo(%struct.dv* %a, %struct.dv* %b)
  ret void
}

