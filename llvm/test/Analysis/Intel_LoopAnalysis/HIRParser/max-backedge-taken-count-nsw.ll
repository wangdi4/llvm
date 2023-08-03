; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" -hir-framework-debug=parser -hir-details 2>&1 | FileCheck %s

; Verify that the range of %iv and %iv.dec is refined to start from 0 and 1 respectively.

; CHECK: + HasSignedIV: Yes
; CHECK: + DO i32 i1 = 0, zext.i8.i32(%arg) + -1 * smin(1, zext.i8.i32(%arg)), 1   <DO_LOOP>  <MAX_TC_EST = 255>
; CHECK: |   %ld = (%tmp221)[i1];
; CHECK: |   %add.iv = %add.iv  +  %ld;
; CHECK: + END LOOP

define void @foo(i8 %arg, ptr %ptr) {
entry:
  %tmp220 = zext i8 %arg to i32
  %tmp221 = bitcast ptr %ptr to ptr
  br label %loop

loop:                                            ; preds = %loop, %entry
  %ptr.iv = phi ptr [ %gep, %loop ], [ %tmp221, %entry ]
  %add.iv = phi i64 [ %add, %loop ], [ 0, %entry ]
  %iv = phi i32 [ %iv.dec, %loop ], [ %tmp220, %entry ]
  %ld = load i16, ptr %ptr.iv, align 2
  %zxt = zext i16 %ld to i64
  %add = add i64 %add.iv, %zxt
  %iv.dec = add nsw i32 %iv, -1
  %gep = getelementptr inbounds i16, ptr %ptr.iv, i64 1
  %cmp = icmp sgt i32 %iv, 1
  br i1 %cmp, label %loop, label %exit

exit:                                            ; preds = %loop
  %add.lcssa = phi i64 [ %add, %loop ]
  ret void
}
