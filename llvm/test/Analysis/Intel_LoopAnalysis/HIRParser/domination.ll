; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that %mul is reverse engineered correctly into %0 * %1.
; CHECK: + DO i1 = 0, 42, 1   <DO_LOOP>
; CHECK: |   (%kk)[0][-1 * i1 + 45] = (%1 * %0);
; CHECK: |   (%kk)[0][-1 * i1 + 44] = i1 + %nd6.promoted + 18;
; CHECK: |   %1 = i1 + %nd6.promoted + 1;
; CHECK: |   %0 = i1 + %nd6.promoted + 18;
; CHECK: + END LOOP


define i32 @foo(i32 %.pre, i32 %nd6.promoted) {
entry:
  %kk = alloca [100 x i32], align 16
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %0 = phi i32 [ %.pre, %entry ], [ %add4, %for.body ]
  %indvars.iv = phi i64 [ 44, %entry ], [ %indvars.iv.next, %for.body ]
  %1 = phi i32 [ %nd6.promoted, %entry ], [ %inc, %for.body ]
  %inc = add i32 %1, 1
  %2 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* %kk, i64 0, i64 %2
  %mul = mul i32 %0, %1
  store i32 %mul, i32* %arrayidx, align 4
  %add4 = add i32 %1, 18
  %arrayidx6 = getelementptr inbounds [100 x i32], [100 x i32]* %kk, i64 0, i64 %indvars.iv
  store i32 %add4, i32* %arrayidx6, align 4
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  %cmp = icmp ugt i64 %indvars.iv.next, 1
  br i1 %cmp, label %for.body, label %exit

exit:
  ret i32 0
}
