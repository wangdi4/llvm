; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that we create a liveout copy for %add11 in the SCC (%add11327 -> %add11 -> %add11328) to avoid live range violation.

; CHECK: + DO i1 = 0, 49, 1   <DO_LOOP>
; CHECK: |   %1 = (%pb)[0][-1 * i1 + 53];
; CHECK: |   %add11328 = %add11328  +  %1;
; CHECK: |   %add11328.out = %add11328;
; CHECK: |   %2 = (%jg7)[0][-1 * i1 + 53];
; CHECK: |   %3 = (%fp)[0][-1 * i1 + 53];
; CHECK: |   (%fp)[0][-1 * i1 + 53] = %3 + -1 * %add11328.out;
; CHECK: |   if (-1 * i1 + 52 == 9)
; CHECK: |   {
; CHECK: |      (%b9)[0][6] = 89;
; CHECK: |      %add11328 = 0;
; CHECK: |   }
; CHECK: |   %4 = (%nx)[0][-1 * i1 + 52];
; CHECK: |   (%fp)[0][-1 * i1 + 53] = %3 + -1 * %add11328.out + %4;
; CHECK: |   %5 = (%m3)[0][-1 * i1 + 53];
; CHECK: |   (%m3)[0][-1 * i1 + 53] = %5 + %add11328 + -94;
; CHECK: + END LOOP


define void @foo(i32 %am.promoted329, i32 %ql.promoted) {
entry:
  %jg7 = alloca [100 x i32], align 16
  %pb = alloca [100 x i32], align 16
  %fp = alloca [100 x i32], align 16
  %nx = alloca [100 x i32], align 16
  %m3 = alloca [100 x i32], align 16
  %b9 = alloca [100 x i32], align 16
  %arrayidx48 = getelementptr inbounds [100 x i32], [100 x i32]* %b9, i64 0, i64 6
  br label %for.body

for.body:                                         ; preds = %if.end, %entry
  %indvars.iv372 = phi i64 [ 52, %entry ], [ %indvars.iv.next373, %if.end ]
  %indvars.iv368 = phi i32 [ -48, %entry ], [ %indvars.iv.next369, %if.end ]
  %add20330 = phi i32 [ %am.promoted329, %entry ], [ %add20, %if.end ]
  %add11328 = phi i32 [ %ql.promoted, %entry ], [ %add11327, %if.end ]
  %0 = add nuw nsw i64 %indvars.iv372, 1
  %arrayidx10 = getelementptr inbounds [100 x i32], [100 x i32]* %pb, i64 0, i64 %0
  %1 = load i32, i32* %arrayidx10, align 4
  %add11 = add i32 %add11328, %1
  %arrayidx17 = getelementptr inbounds [100 x i32], [100 x i32]* %jg7, i64 0, i64 %0
  %2 = load i32, i32* %arrayidx17, align 4
  %add19 = sub i32 %add11, %2
  %add20 = add i32 %add19, %add20330
  %arrayidx23 = getelementptr inbounds [100 x i32], [100 x i32]* %fp, i64 0, i64 %0
  %3 = load i32, i32* %arrayidx23, align 4
  %sub24 = sub i32 %3, %add11
  store i32 %sub24, i32* %arrayidx23, align 4
  %cmp25 = icmp eq i64 %indvars.iv372, 9
  br i1 %cmp25, label %for.cond44.preheader, label %if.end

for.cond44.preheader:                             ; preds = %for.body
  store i32 89, i32* %arrayidx48, align 8
  br label %if.end

if.end:                                           ; preds = %for.cond44.preheader, %for.body
  %add11327 = phi i32 [ 0, %for.cond44.preheader ], [ %add11, %for.body ]
  %arrayidx54 = getelementptr inbounds [100 x i32], [100 x i32]* %nx, i64 0, i64 %indvars.iv372
  %4 = load i32, i32* %arrayidx54, align 4
  %add58 = add i32 %sub24, %4
  store i32 %add58, i32* %arrayidx23, align 4
  %sub60 = add i32 %add11327, -94
  %arrayidx63 = getelementptr inbounds [100 x i32], [100 x i32]* %m3, i64 0, i64 %0
  %5 = load i32, i32* %arrayidx63, align 4
  %add64 = add i32 %sub60, %5
  store i32 %add64, i32* %arrayidx63, align 4
  %indvars.iv.next373 = add nsw i64 %indvars.iv372, -1
  %indvars.iv.next369 = add nsw i32 %indvars.iv368, 1
  %exitcond378 = icmp eq i32 %indvars.iv.next369, 2
  br i1 %exitcond378, label %for.exit, label %for.body

for.exit:
  ret void
}
