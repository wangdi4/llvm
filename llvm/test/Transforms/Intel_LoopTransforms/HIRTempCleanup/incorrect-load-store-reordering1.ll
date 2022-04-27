; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup" -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup 2>&1 | FileCheck %s

; Verify that load (%t115 = (%arr)[i1 + 2]) is not substituted because the use
; inst (%sum.0 = %sum.0  +  %t115) is the rval def inst of (%sum.0.out = %sum.0)
; and is moved past the corresponding store with reordering.

; CHECK: Before

; CHECK: + DO i1 = 0, 255, 1   <DO_LOOP>
; CHECK: |   %sum.0.out2 = %sum.0;
; CHECK: |   %t113 = (%arr)[i1];
; CHECK: |   %t114 = (%arr)[i1 + 1];
; CHECK: |   %t115 = (%arr)[i1 + 2];
; CHECK: |   %t116 = (%arr)[i1 + 3];
; CHECK: |   %sum.0 = %t113  +  %sum.0;
; CHECK: |   %sum.0.out1 = %sum.0;
; CHECK: |   %sum.0 = %sum.0  +  %t114;
; CHECK: |   %sum.0.out = %sum.0;
; CHECK: |   %sum.0 = %sum.0  +  %t115;
; CHECK: |   (%arr)[i1] = %sum.0.out2;
; CHECK: |   (%arr)[i1 + 1] = %sum.0.out1;
; CHECK: |   (%arr)[i1 + 2] = %sum.0.out;
; CHECK: |   (%arr)[i1 + 3] = %sum.0;
; CHECK: |   %sum.0 = %sum.0  +  %t116;
; CHECK: + END LOOP
; CHECK: END REGION


; CHECK: After

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, 255, 1   <DO_LOOP>
; CHECK: |   %sum.0.out2 = %sum.0;
; CHECK: |   %t115 = (%arr)[i1 + 2];
; CHECK: |   %t116 = (%arr)[i1 + 3];
; CHECK: |   %sum.0 = (%arr)[i1]  +  %sum.0;
; CHECK: |   %sum.0.out1 = %sum.0;
; CHECK: |   %sum.0 = %sum.0  +  (%arr)[i1 + 1];
; CHECK: |   (%arr)[i1] = %sum.0.out2;
; CHECK: |   (%arr)[i1 + 1] = %sum.0.out1;
; CHECK: |   (%arr)[i1 + 2] = %sum.0;
; CHECK: |   %sum.0 = %sum.0  +  %t115;
; CHECK: |   (%arr)[i1 + 3] = %sum.0;
; CHECK: |   %sum.0 = %sum.0  +  %t116;
; CHECK: + END LOOP


define void @foo(i32* %arr) {
entry:
  br label %do.body

do.body:                                    ; preds = %do.body, %entry
  %sum.0 = phi i32 [ 0, %entry ], [ %add10, %do.body ]
  %arr.idx = phi i64 [ 0, %entry ], [ %arr.add, %do.body ]
  %arr.ptr = getelementptr inbounds i32, i32* %arr, i64 %arr.idx
  %t113 = load i32, i32* %arr.ptr, align 4
  %arrayidx1 = getelementptr inbounds i32, i32* %arr.ptr, i64 1
  %t114 = load i32, i32* %arrayidx1, align 4
  %arrayidx2 = getelementptr inbounds i32, i32* %arr.ptr, i64 2
  %t115 = load i32, i32* %arrayidx2, align 4
  %arrayidx3 = getelementptr inbounds i32, i32* %arr.ptr, i64 3
  %t116 = load i32, i32* %arrayidx3, align 4
  %add = add i32 %t113, %sum.0
  %add4 = add i32 %add, %t114
  %add5 = add i32 %add4, %t115
  store i32 %sum.0, i32* %arr.ptr, align 4
  store i32 %add, i32* %arrayidx1, align 4
  store i32 %add4, i32* %arrayidx2, align 4
  store i32 %add5, i32* %arrayidx3, align 4
  %add10 = add i32 %add5, %t116
  %arr.add = add nuw nsw i64 %arr.idx, 1
  %cmp.not = icmp eq i64 %arr.add, 256
  br i1 %cmp.not, label %do.cond.loopexit, label %do.body

do.cond.loopexit:
  ret void
}
