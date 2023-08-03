; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination 2>&1 < %s | FileCheck %s

; Verify that i32 stores (%alloc)[0][i1 + 1].0 and (%alloc)[0][i1 + 1].1 are not
; eliminated due to loop-carried dependence with the load (i64*)(%alloc)[0][i1].

; %alloc is identified as a region local alloca.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 1, 1   <DO_LOOP>
; CHECK: |   (%alloc)[0][i1 + 1].0 = 0;
; CHECK: |   (%alloc)[0][i1 + 1].1 = 1;
; CHECK: |   %ld0 = (i64*)(%alloc)[0][i1];
; CHECK: |   %ld1 = (%alloc)[0][i1 + 1].1;
; CHECK: |   %add = %ld0  +  %ld1;
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK-NOT: modified

; CHECK: + DO i1 = 0, 1, 1   <DO_LOOP>
; CHECK: |   (%alloc)[0][i1 + 1].0 = 0;
; CHECK: |   (%alloc)[0][i1 + 1].1 = 1;
; CHECK: |   %ld0 = (i64*)(%alloc)[0][i1];
; CHECK: |   %ld1 = (%alloc)[0][i1 + 1].1;
; CHECK: |   %add = %ld0  +  %ld1;
; CHECK: + END LOOP


%struct.ab = type { i32, i32 }

define i32 @foo() {
entry:
  %alloc = alloca [3 x %struct.ab], align 16
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry ], [ %iv.inc, %loop ]
  %iv.inc = add nsw i64 %iv, 1
  %gep.iv = getelementptr inbounds [3 x %struct.ab], ptr %alloc, i64 0, i64 %iv
  %gep.iv.inc0 = getelementptr inbounds [3 x %struct.ab], ptr %alloc, i64 0, i64 %iv.inc, i32 0
  %gep.iv.inc1 = getelementptr inbounds [3 x %struct.ab], ptr %alloc, i64 0, i64 %iv.inc, i32 1
  store i32 0, ptr %gep.iv.inc0
  store i32 1, ptr %gep.iv.inc1
  %bc = bitcast ptr %gep.iv to ptr
  %ld0 = load i64, ptr %bc
  %trunc = trunc i64 %ld0 to i32
  %ld1 = load i32, ptr %gep.iv.inc1
  %add = add i32 %trunc, %ld1
  %cmp = icmp eq i64 %iv.inc, 2
  br i1 %cmp, label %exit, label %loop

exit:
  %ld.lcssa = phi i32 [ %add, %loop ]
  ret i32 %ld.lcssa
}
  
