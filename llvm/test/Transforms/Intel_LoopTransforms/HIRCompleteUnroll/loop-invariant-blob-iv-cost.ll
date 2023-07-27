; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-post-vec-complete-unroll,print<hir>" -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s

; Incoming HIR-
; + DO i1 = 0, 7, 1   <DO_LOOP>
; |   + DO i2 = 0, 7, 1   <DO_LOOP>
; |   |   %ld = (%struct.ptr)[0].1[%t][8 * i1 + i2];
; |   |   (%char.ptr)[(8 + sext.i32.i64((-8 + %s))) * i1 + i2] = (%glob.ptr)[sext.i16.i64(%ld) + 128];
; |   + END LOOP
; + END LOOP

; Verify that the cost of of this invariant IV blob-
; (8 + sext.i32.i64((-8 + %s))) * i1 + i2

; Is calculated to be:
; ((OuterTripCount - 1) * InnerTripCount) = 7 * 8 = 56

; i1 is zero in the first loop iteration so (1 * InnerTripCount) is considered
; as savings for the folded multiplication.

; CHECK: ScaledCost: 56

; There is one extra saving for the folded addition of i1 = 0 and i2 = 0.
; CHECK: ScaledSavings: 9

; CHECK: modified

; Verify that inner loop was unrolled but the entire loopnest wasn't.
; CHECK: DO i1
; CHECK-NOT: DO i2

%struct.layer_data = type { i8, [10 x [10 x i16]] }

define void @foo(ptr %char.ptr, ptr %glob.ptr, ptr %struct.ptr, i64 %t, i32 %s) {
entry:
  %add1 = add nsw i32 %s, -8
  %sext1 = sext i32 %add1 to i64
  %i16.ptr = getelementptr inbounds %struct.layer_data, ptr %struct.ptr, i64 0, i32 1, i64 %t, i64 0
  br label %loop

loop:                                             ; preds = %entry, %latch
  %i16.ptr.phi = phi ptr [ %i16.ptr.phi.inc, %latch ], [ %i16.ptr, %entry ]
  %char.ptr.phi = phi ptr [ %char.ptr.phi.inc, %latch ], [ %char.ptr, %entry ]
  %iv = phi i32 [ %iv.inc, %latch ], [ 0, %entry ]
  br label %inner.loop

inner.loop:                                             ; preds = %inner.loop, %loop
  %i16.ptr.phi.inner = phi ptr [ %i16.ptr.phi, %loop ], [ %i16.ptr.phi.inner.inc, %inner.loop ]
  %char.ptr.phi.inner = phi ptr [ %char.ptr.phi, %loop ], [ %char.ptr.phi.inner.inc, %inner.loop ]
  %inner.iv = phi i32 [ 0, %loop ], [ %inner.iv.inc, %inner.loop ]
  %i16.ptr.phi.inner.inc = getelementptr inbounds i16, ptr %i16.ptr.phi.inner, i64 1
  %ld = load i16, ptr %i16.ptr.phi.inner, align 2
  %sext = sext i16 %ld to i64
  %add = add nsw i64 %sext, 128
  %gep = getelementptr inbounds i8, ptr %glob.ptr, i64 %add
  %glob.ld = load i8, ptr %gep, align 1
  %char.ptr.phi.inner.inc = getelementptr inbounds i8, ptr %char.ptr.phi.inner, i64 1
  store i8 %glob.ld, ptr %char.ptr.phi.inner, align 1
  %inner.iv.inc = add nuw nsw i32 %inner.iv, 1
  %cmp = icmp eq i32 %inner.iv.inc, 8
  br i1 %cmp, label %latch, label %inner.loop

latch:                                             ; preds = %inner.loop
  %i16.ptr.phi.inc = getelementptr i16, ptr %i16.ptr.phi, i64 8
  %t2276 = getelementptr i8, ptr %char.ptr.phi, i64 8
  %char.ptr.phi.inc = getelementptr inbounds i8, ptr %t2276, i64 %sext1
  %iv.inc = add nuw nsw i32 %iv, 1
  %cmp1 = icmp eq i32 %iv.inc, 8
  br i1 %cmp1, label %exit, label %loop

exit:
  ret void
}
