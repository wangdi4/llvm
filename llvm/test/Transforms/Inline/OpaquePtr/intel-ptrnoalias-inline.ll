; RUN: opt -opaque-pointers -passes=inline < %s -S -o - | FileCheck %s

; Check that "ptrnoalias" on arguments will be converted into
; alias.scope/noalias metadata after inlining.

; CHECK: %"p[i].i" = getelementptr inbounds float, ptr %p.i, i64 %i.i
; CHECK: %"*p[i].i" = load float, ptr %"p[i].i", align 4, !alias.scope ![[S1:.*]], !noalias ![[S2:.*]]
; CHECK: %"q[i].i" = getelementptr inbounds float, ptr %q.i, i64 %i.i
; CHECK: store float %"*p[i].i", ptr %"q[i].i", align 4, !alias.scope ![[S2]], !noalias ![[S1]]

%struct.dv = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

define void @foo(ptr "ptrnoalias" %a, ptr "ptrnoalias" %b) {
entry:
  %a_ptr = getelementptr inbounds %struct.dv, ptr %a, i64 0, i32 0
  %b_ptr = getelementptr inbounds %struct.dv, ptr %b, i64 0, i32 0
  %q = load ptr, ptr %a_ptr, align 8
  %p = load ptr, ptr %b_ptr, align 8
  br label %loop_header

loop_header:                                      ; preds = %entry
  br label %loop

loop:                                             ; preds = %loop, %loop_header
  %i = phi i64 [ %ip1, %loop ], [ 0, %loop_header ]
  %"p[i]" = getelementptr inbounds float, ptr %p, i64 %i
  %"*p[i]" = load float, ptr %"p[i]", align 4
  %"q[i]" = getelementptr inbounds float, ptr %q, i64 %i
  store float %"*p[i]", ptr %"q[i]", align 4
  %ip1 = add nuw nsw i64 %i, 1
  %cmp = icmp eq i64 %ip1, 100
  br i1 %cmp, label %loop_exit, label %loop

loop_exit:                                        ; preds = %loop
  br label %exit

exit:                                             ; preds = %loop_exit
  ret void
}

define void @bar(ptr %a, ptr %b) {
bb:
  call void @foo(ptr %a, ptr %b)
  ret void
}
