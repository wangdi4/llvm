; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir>" -disable-output  2>&1 | FileCheck %s

; Verify that we create a liveout copy for %tphi and that is the one which is
; used in the i1 loop. %tphi is propagated outside the i2 loop using the SCEV
; form of %tout. Since %tphi is deconstructed, using the livein copy 
; %tphi = %ld; in the i2 loop, if we don't create a liveout copy for %tphi it
; will result in live range violation.


; CHECK: + DO i1 = 0, 15, 1   <DO_LOOP>
; CHECK: |   %tphi = %tphi.in;
; CHECK: |
; CHECK: |   + DO i2 = 0, 15, 1   <DO_LOOP>
; CHECK: |   |   %tphi.out = %tphi;
; CHECK: |   |   %ld = (%src)[i2];
; CHECK: |   |   %tphi = %ld;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   (%dst)[i1] = (%tadd + %tphi.out + 8)/u16;
; CHECK: + END LOOP


define void @func(i32 %tphi.in, i32 %tadd, ptr %dst, ptr %src) {
entry:
  br label %loop.outer

loop.outer:
  %iv.outer = phi i32 [ %iv.outer.inc, %latch.outer ], [ 0, %entry ]
  br label %loop.inner

loop.inner:                                               ; preds = %loop.inner, %loop.outer
  %iv.inner = phi i32 [ %iv.inner.inc, %loop.inner ], [ 0, %loop.outer ]
  %tphi = phi i32 [ %ld, %loop.inner ], [ %tphi.in, %loop.outer ]
  %add1 = add nuw nsw i32 %tadd, %tphi
  %add2 = add nuw nsw i32 %add1, 8
  %tout = lshr i32 %add2, 4
  %gep = getelementptr inbounds i32, ptr %src, i32 %iv.inner
  %ld = load i32, ptr %gep
  %iv.inner.inc = add nsw i32 %iv.inner, 1
  %cmp1 = icmp eq i32 %iv.inner.inc, 16
  br i1 %cmp1, label %latch.outer, label %loop.inner

latch.outer:                                              ; preds = %loop.inner
  %tout.lcssa = phi i32 [ %tout, %loop.inner ]
  %iv.outer.inc = add i32 %iv.outer, 1
  %gep1 = getelementptr inbounds i32, ptr %dst, i32 %iv.outer
  store i32 %tout.lcssa, ptr %gep1
  %cmp2 = icmp eq i32 %iv.outer.inc, 16
  br i1 %cmp2, label %exit, label %loop.outer

exit:
  ret void
}
  
