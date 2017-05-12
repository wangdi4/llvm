; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup -disable-output -hir-details < %s 2>&1 | FileCheck %s

; Verify that the loop liveouts of i1 and i3 loop are correctly updated after redundant liveout copies are removed.

; CHECK: Dump Before HIR Temp Cleanup
; CHECK: LiveOut symbases: 6
; CHECK: LiveOut symbases: 4
; CHECK: LiveOut symbases: 45

; CHECK: Dump After HIR Temp Cleanup
; CHECK: LiveOut symbases: 4 
; CHECK: LiveOut symbases: 4
; CHECK: LiveOut symbases: 3


define i32 @foo(i32 %js.promoted427) {
entry:
  %jc = alloca i32, align 4
  %k7 = alloca i32, align 4
  %js = alloca i32, align 4
  %jb = alloca i32, align 4
  %ck2 = alloca [100 x i32], align 16
  %s = alloca [100 x i32], align 16
  %c = alloca [100 x i32], align 16
  %k = alloca [100 x i32], align 16
  %g3 = alloca [100 x i32], align 16
  %h = alloca [100 x i32], align 16
  %t1 = alloca [100 x [100 x i32]], align 16
  %dr5 = alloca [100 x [100 x [100 x i32]]], align 16
  %qz4 = alloca [100 x [100 x i32]], align 16
  %n = alloca [100 x [100 x i32]], align 16
  %b = alloca [100 x i32], align 16
  br label %for.cond20.preheader

for.cond20.preheader:                             ; preds = %entry, %for.inc94
  %indvars.iv487 = phi i64 [ 30, %entry ], [ %indvars.iv.next488, %for.inc94 ]
  %.lcssa423428 = phi i32 [ %js.promoted427, %entry ], [ %.lcssa535, %for.inc94 ]
  %0 = add nuw nsw i64 %indvars.iv487, 1
  %indvars.iv.next488 = add nsw i64 %indvars.iv487, -1
  br label %for.body22

for.body22:                                       ; preds = %for.cond20.preheader, %for.inc91
  %indvars.iv481 = phi i64 [ 1, %for.cond20.preheader ], [ %indvars.iv.next482, %for.inc91 ]
  %indvars.iv476 = phi i32 [ -1, %for.cond20.preheader ], [ %indvars.iv.next477, %for.inc91 ]
  %1 = phi i32 [ %.lcssa423428, %for.cond20.preheader ], [ %18, %for.inc91 ]
  %2 = add i32 %indvars.iv476, 2
  %3 = add nsw i64 %indvars.iv481, -1
  %arrayidx24 = getelementptr inbounds [100 x i32], [100 x i32]* %k, i64 0, i64 %3
  %4 = load i32, i32* %arrayidx24, align 4
  %5 = sub nuw nsw i64 4294967188, %indvars.iv481
  %6 = trunc i64 %5 to i32
  %sub26 = add i32 %6, %4
  store i32 %sub26, i32* %arrayidx24, align 4
  %arrayidx32 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %t1, i64 0, i64 %0, i64 %3
  %7 = load i32, i32* %arrayidx32, align 4
  %add33 = add i32 %1, %7
  %cmp35420 = icmp ugt i64 %indvars.iv481, 1
  br i1 %cmp35420, label %for.body36.preheader, label %for.inc91

for.body36.preheader:                             ; preds = %for.body22
  %8 = add nuw nsw i64 %indvars.iv481, 1
  %arrayidx53 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %qz4, i64 0, i64 %3, i64 %indvars.iv481
  br label %for.body36

for.body36:                                       ; preds = %for.body36.preheader, %for.cond34.backedge
  %indvars.iv472 = phi i64 [ %indvars.iv.next473, %for.cond34.backedge ], [ 1, %for.body36.preheader ]
  %9 = phi i32 [ %15, %for.cond34.backedge ], [ %add33, %for.body36.preheader ]
  %10 = sub nsw i64 %indvars.iv472, %indvars.iv487
  %11 = add nsw i64 %indvars.iv472, -1
  %arrayidx46 = getelementptr inbounds [100 x [100 x [100 x i32]]], [100 x [100 x [100 x i32]]]* %dr5, i64 0, i64 %11, i64 %indvars.iv.next488, i64 %8
  %12 = load i32, i32* %arrayidx46, align 4
  %13 = trunc i64 %indvars.iv472 to i32
  %mul47 = mul i32 %12, %13
  %14 = trunc i64 %10 to i32
  %cmp48 = icmp ugt i32 %14, %mul47
  %indvars.iv.next473 = add nuw nsw i64 %indvars.iv472, 1
  br i1 %cmp48, label %for.cond34.backedge, label %if.then

for.cond34.backedge:                              ; preds = %for.body36, %if.then
  %15 = phi i32 [ %9, %for.body36 ], [ %16, %if.then ]
  %exitcond480 = icmp eq i64 %indvars.iv.next473, %indvars.iv481
  br i1 %exitcond480, label %for.inc91.loopexit, label %for.body36

if.then:                                          ; preds = %for.body36
  %16 = load i32, i32* %arrayidx53, align 4
  %arrayidx65 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %n, i64 0, i64 %8, i64 %indvars.iv.next473
  %17 = load i32, i32* %arrayidx65, align 4
  %mul66 = mul i32 %17, %14
  %arrayidx81 = getelementptr inbounds [100 x [100 x [100 x i32]]], [100 x [100 x [100 x i32]]]* %dr5, i64 0, i64 %11, i64 %indvars.iv.next488, i64 %indvars.iv481
  store i32 %mul66, i32* %arrayidx81, align 4
  br label %for.cond34.backedge

for.inc91.loopexit:                               ; preds = %for.cond34.backedge
  %.lcssa = phi i32 [ %15, %for.cond34.backedge ]
  br label %for.inc91

for.inc91:                                        ; preds = %for.inc91.loopexit, %for.body22
  %add63.lcssa424 = phi i32 [ 1, %for.body22 ], [ %2, %for.inc91.loopexit ]
  %18 = phi i32 [ %add33, %for.body22 ], [ %.lcssa, %for.inc91.loopexit ]
  %indvars.iv.next482 = add nuw nsw i64 %indvars.iv481, 4
  %cmp21 = icmp ult i64 %indvars.iv.next482, 41
  %indvars.iv.next477 = add nsw i32 %indvars.iv476, 4
  br i1 %cmp21, label %for.body22, label %for.inc94

for.inc94:                                        ; preds = %for.inc91
  %add63.lcssa424.lcssa = phi i32 [ %add63.lcssa424, %for.inc91 ]
  %.lcssa535 = phi i32 [ %18, %for.inc91 ]
  %cmp = icmp ugt i64 %indvars.iv.next488, 1
  br i1 %cmp, label %for.cond20.preheader, label %exit

exit:
  ret i32 %.lcssa535
}


