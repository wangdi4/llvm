; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Verify that near perfect loopnest can be multiversioned.

; Input HIR-
; + DO i1 = 0, %t72 + -1, 1   <DO_LOOP>
; |   + DO i2 = 0, %t72 + -1, 1   <DO_LOOP>
; |   |   %t97 = (%ptr3)[%t72 * i1 + i2];
; |   |   %t80 = %t97;
; |   |
; |   |   + DO i3 = 0, %t72 + -1, 1   <DO_LOOP>
; |   |   |   %t84 = (%ptr1)[%t72 * i1 + i3];
; |   |   |   %t88 = (%ptr2)[i2 + %t72 * i3];
; |   |   |   %t89 = %t88  *  %t84;
; |   |   |   %t80 = %t80  -  %t89;
; |   |   |   (%ptr3)[%t72 * i1 + i2] = %t80;
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP


; CHECK: = &((%ptr3)[(%t72 * %t72) + -1]) >=u &((%ptr1)[0])
 
; CHECK: if (%mv
; CHECK:   DO i1
; CHECK:     DO i2
; CHECK:       DO i3

; CHECK: else
; CHECK:   DO i1 {{.*}} <nounroll> <novectorize>
; CHECK:     DO i2 {{.*}} <nounroll> <novectorize>
; CHECK:       %t97 = (%ptr3)[%t72 * i1 + i2];
; CHECK:       %t80 = %t97;

; CHECK:       DO i3 {{.*}} <nounroll> <novectorize>


define void @foo(float* %ptr1, float* %ptr2, float* %ptr3, i64 %t72) {
entry:
  br label %outer.loop

outer.loop:                                               ; preds = %outer.latch, %entry
  %t74 = phi i64 [ %t99, %outer.latch ], [ 0, %entry ]
  %t75 = mul nuw nsw i64 %t74, %t72
  br label %middle.loop

middle.loop:                                               ; preds = %middle.latch, %outer.loop
  %t94 = phi i64 [ 0, %outer.loop ], [ %t77, %middle.latch ]
  %t95 = add nuw nsw i64 %t94, %t75
  %t96 = getelementptr inbounds float, float* %ptr3, i64 %t95
  %t97 = load float, float* %t96, align 4
  br label %inner.loop

inner.loop:                                               ; preds = %middle.loop, %inner.loop
  %t80 = phi float [ %t97, %middle.loop ], [ %t90, %inner.loop ]
  %t81 = phi i64 [ 0, %middle.loop ], [ %t91, %inner.loop ]
  %t82 = add nuw nsw i64 %t81, %t75
  %t83 = getelementptr inbounds float, float* %ptr1, i64 %t82
  %t84 = load float, float* %t83, align 4
  %t85 = mul nuw nsw i64 %t81, %t72
  %t86 = add nuw nsw i64 %t85, %t94
  %t87 = getelementptr inbounds float, float* %ptr2, i64 %t86
  %t88 = load float, float* %t87, align 4
  %t89 = fmul fast float %t88, %t84
  %t90 = fsub fast float %t80, %t89
  store float %t90, float* %t96, align 4
  %t91 = add nuw nsw i64 %t81, 1
  %t92 = icmp eq i64 %t91, %t72
  br i1 %t92, label %middle.latch, label %inner.loop

middle.latch:                                               ; preds = %inner.loop
  %t77 = add nuw nsw i64 %t94, 1
  %t78 = icmp eq i64 %t77, %t72
  br i1 %t78, label %outer.latch, label %middle.loop

outer.latch:                                               ; preds = %middle.latch
  %t99 = add nuw nsw i64 %t74, 1
  %t100 = icmp eq i64 %t99, %t72
  br i1 %t100, label %exit, label %outer.loop

exit:
  ret void
}
