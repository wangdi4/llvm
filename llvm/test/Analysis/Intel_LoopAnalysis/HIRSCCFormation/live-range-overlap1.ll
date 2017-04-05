; RUN: opt < %s -analyze -hir-scc-formation | FileCheck %s

; Check that only one SCCs with %mul is formed for this loop. The SCC (%sub13 -> %0 -> %add7189 -> %sub18) should have been invalidated because %sub13 is used in the merge phi %add7189 despite %sub18 from the same SCC being defined later in the same bblock causing live range overlap.

; CHECK: SCC1
; CHECK-SAME: %mul
; CHECK-NOT: SCC2


define i32 @foo(i32 %r.promoted, i32 %add) {
entry:
  %l0 = alloca [100 x i32], align 16
  %d = alloca [100 x i32], align 16
  %a = alloca [100 x i32], align 16
  %e = alloca [100 x i32], align 16
  %x = alloca [100 x i32], align 16

  br label %for.body

for.body:                                         ; preds = %entry, %if.end
  %indvars.iv208 = phi i64 [ 51, %entry ], [ %indvars.iv.next209, %if.end ]
  %mul190 = phi i32 [ %r.promoted, %entry ], [ %mul, %if.end ]
  %0 = phi i32 [ %add, %entry ], [ %add7189, %if.end ]
  %1 = trunc i64 %indvars.iv208 to i32
  %add7 = sub i32 %1, %0
  %2 = add nuw nsw i64 %indvars.iv208, 1
  %arrayidx9 = getelementptr inbounds [100 x i32], [100 x i32]* %l0, i64 0, i64 %2
  %3 = load i32, i32* %arrayidx9, align 4
  %mul = mul i32 %mul190, %3
  %sub13 = sub i32 %add7, %3
  %mul14 = mul i32 %sub13, %mul
  %indvars.iv.next209 = add nsw i64 %indvars.iv208, -1
  %arrayidx17 = getelementptr inbounds [100 x i32], [100 x i32]* %d, i64 0, i64 %indvars.iv.next209
  %4 = load i32, i32* %arrayidx17, align 4
  %sub18 = sub i32 %4, %mul14
  store i32 %sub18, i32* %arrayidx17, align 4
  %cmp23 = icmp eq i64 %indvars.iv208, 24
  br i1 %cmp23, label %if.end, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx20 = getelementptr inbounds [100 x i32], [100 x i32]* %d, i64 0, i64 %indvars.iv208
  %5 = load i32, i32* %arrayidx20, align 4
  %arrayidx25 = getelementptr inbounds [100 x i32], [100 x i32]* %e, i64 0, i64 %indvars.iv208
  %6 = load i32, i32* %arrayidx25, align 4
  %arrayidx28 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %indvars.iv208
  %7 = load i32, i32* %arrayidx28, align 4
  %sub29 = add i32 %7, %6
  store i32 %sub29, i32* %arrayidx28, align 4
  %sub32 = add i32 %3, %1
  %add36 = sub i32 %sub32, %5
  store i32 %add36, i32* %arrayidx9, align 4
  br label %if.end

if.end:                                           ; preds = %for.body, %if.then
  %add7189 = phi i32 [ %sub13, %if.then ], [ %sub18, %for.body ]
  %arrayidx42 = getelementptr inbounds [100 x i32], [100 x i32]* %x, i64 0, i64 %indvars.iv.next209
  %8 = load i32, i32* %arrayidx42, align 4
  %arrayidx45 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %indvars.iv.next209
  store i32 %8, i32* %arrayidx45, align 4
  %cmp = icmp ugt i64 %indvars.iv.next209, 1
  br i1 %cmp, label %for.body, label %for.end

for.end:
  %res = add i32 %mul, %add7189
  ret i32 %res
}
