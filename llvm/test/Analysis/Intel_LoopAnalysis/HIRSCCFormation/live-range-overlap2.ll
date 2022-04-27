; RUN: opt < %s -enable-new-pm=0 -analyze -hir-scc-formation | FileCheck %s

; We use the --allow-empty flag with FileCheck for the new-format opt because:
;
; - new-format opt output is empty for this test, (old-format opt emits just one
;       line: Printing analysis 'HIR SCC Formation' for function...).
; - The check consists of 'CHECK-NOT' only, and has no 'CHECK' lines.
;
; TODO: If the lit-test is modified, and new-format opt is no longer empty,
;     please make sure to remove the --allow-empty flag, and this comment.
;
; RUN: opt %s -passes="print<hir-scc-formation>" -disable-output 2>&1 | FileCheck --allow-empty %s

; Check that no SCCs are formed for this loop. The SCC (%add31 -> %add -> %4 -> %0) should have been invalidated because %0 is used in the merge phi %4 despite %add from the same SCC being defined later in the same bblock causing live range overlap.

; CHECK-NOT: SCC1

define i32 @foo() {
entry:
  %s = alloca i32, align 4
  %ju3 = alloca i32, align 4
  %w = alloca i32, align 4
  %d1 = alloca i32, align 4
  %n8 = alloca i32, align 4
  %lh = alloca [100 x i32], align 16
  %xa = alloca [100 x i32], align 16
  %t5 = alloca [100 x i32], align 16
  %m8 = alloca [100 x i32], align 16
  %z5 = alloca [100 x i32], align 16
  %y = alloca [100 x i32], align 16
  br label %for.body

for.body:                                         ; preds = %for.cond.backedge, %entry
  %0 = phi i32 [ 0, %entry ], [ %4, %for.cond.backedge ]
  %indvars.iv257 = phi i64 [ 1, %entry ], [ %indvars.iv.next258, %for.cond.backedge ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* %lh, i64 0, i64 %indvars.iv257
  store i32 0, i32* %arrayidx, align 4
  %1 = add nsw i64 %indvars.iv257, -1
  %arrayidx10 = getelementptr inbounds [100 x i32], [100 x i32]* %xa, i64 0, i64 %1
  %2 = load i32, i32* %arrayidx10, align 4
  %3 = trunc i64 %indvars.iv257 to i32
  %sub11 = sub i32 %2, %3
  %add = add i32 %0, %3
  %cmp12 = icmp ult i32 %sub11, %add
  %indvars.iv.next258 = add nuw nsw i64 %indvars.iv257, 1
  br i1 %cmp12, label %if.then, label %for.cond.backedge

for.cond.backedge:                                ; preds = %for.body, %if.then40, %if.then
  %4 = phi i32 [ %0, %for.body ], [ %add31, %if.then40 ], [ %add31, %if.then ]
  %exitcond260 = icmp eq i64 %indvars.iv.next258, 91
  br i1 %exitcond260, label %for.end, label %for.body

if.then:                                          ; preds = %for.body
  %arrayidx18 = getelementptr inbounds [100 x i32], [100 x i32]* %t5, i64 0, i64 %1
  %5 = load i32, i32* %arrayidx18, align 4
  %sub19 = sub i32 %5, %2
  store i32 %sub19, i32* %arrayidx18, align 4
  store i32 %sub11, i32* %arrayidx, align 4
  %arrayidx29 = getelementptr inbounds [100 x i32], [100 x i32]* %t5, i64 0, i64 %indvars.iv.next258
  %6 = load i32, i32* %arrayidx29, align 4
  %add31 = add i32 %add, %6
  store i32 %add31, i32* %s, align 4
  %arrayidx34 = getelementptr inbounds [100 x i32], [100 x i32]* %m8, i64 0, i64 %indvars.iv.next258
  %7 = load i32, i32* %arrayidx34, align 4
  %8 = load i32, i32* %w, align 4
  %add35 = add i32 %8, %7
  store i32 %add35, i32* %w, align 4
  %arrayidx38 = getelementptr inbounds [100 x i32], [100 x i32]* %z5, i64 0, i64 %1
  %9 = load i32, i32* %arrayidx38, align 4
  %mul = mul i32 %add31, %3
  %cmp39 = icmp ugt i32 %9, %mul
  br i1 %cmp39, label %if.then40, label %for.cond.backedge

if.then40:                                        ; preds = %if.then
  %sub41 = add i32 %add31, -85
  %10 = load i32, i32* %d1, align 4
  %mul42 = mul i32 %10, %sub41
  store i32 %mul42, i32* %d1, align 4
  %arrayidx44 = getelementptr inbounds [100 x i32], [100 x i32]* %z5, i64 0, i64 %indvars.iv257
  %11 = load i32, i32* %arrayidx44, align 4
  %sub48 = sub i32 %sub19, %11
  store i32 %sub48, i32* %arrayidx18, align 4
  %add52 = add i32 %add35, %9
  store i32 %add52, i32* %w, align 4
  %arrayidx54 = getelementptr inbounds [100 x i32], [100 x i32]* %m8, i64 0, i64 %indvars.iv257
  %12 = load i32, i32* %arrayidx54, align 4
  %arrayidx57 = getelementptr inbounds [100 x i32], [100 x i32]* %y, i64 0, i64 %indvars.iv.next258
  store i32 %12, i32* %arrayidx57, align 4
  %13 = load i32, i32* %n8, align 4
  %sub60 = sub i32 %13, %11
  store i32 %sub60, i32* %n8, align 4
  br label %for.cond.backedge

for.end:
  ret i32 %4
}
