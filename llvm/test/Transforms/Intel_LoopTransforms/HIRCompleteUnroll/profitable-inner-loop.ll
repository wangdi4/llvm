; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-pre-vec-complete-unroll -print-before=hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll 2>&1 < %s | FileCheck %s

; TODO: change pre-vec pass to post-vec pass after vectorizer improves its cost model.

; Verify that the profitable inner i2 loop gets unrolled.

; CHECK: Dump Before 
; CHECK: DO i1
; CHECK: DO i2


; CHECK: Dump After 
; CHECK-NOT: DO i2


target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"

@Logtable = external local_unnamed_addr global [0 x i8], align 1
@Alogtable = external local_unnamed_addr global [0 x i8], align 1

define void @InvMixColumn([8 x i8]* nocapture %a, i8 zeroext %BC) local_unnamed_addr #4 {
entry:
  %b = alloca [4 x [8 x i8]], align 1
  %0 = getelementptr inbounds [4 x [8 x i8]], [4 x [8 x i8]]* %b, i32 0, i32 0, i32 0
  %conv = zext i8 %BC to i32
  %cmp118 = icmp eq i8 %BC, 0
  br i1 %cmp118, label %for.inc48.3, label %for.cond2.preheader.preheader

for.cond2.preheader.preheader:                    ; preds = %entry
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %for.cond2.preheader.preheader, %for.inc29
  %j.0119 = phi i32 [ %inc30, %for.inc29 ], [ 0, %for.cond2.preheader.preheader ]
  br label %for.body5

for.body5:                                        ; preds = %mul.exit90, %for.cond2.preheader
  %i.0117 = phi i32 [ 0, %for.cond2.preheader ], [ %add, %mul.exit90 ]
  %arrayidx6 = getelementptr inbounds [8 x i8], [8 x i8]* %a, i32 %i.0117, i32 %j.0119
  %1 = load i8, i8* %arrayidx6, align 1
  %tobool2.i = icmp eq i8 %1, 0
  br i1 %tobool2.i, label %mul.exit, label %if.then.i

if.then.i:                                        ; preds = %for.body5
  %conv1.i = zext i8 %1 to i32
  %2 = load i8, i8* getelementptr inbounds ([0 x i8], [0 x i8]* @Logtable, i32 0, i32 14), align 1
  %conv3.i = zext i8 %2 to i32
  %arrayidx5.i = getelementptr inbounds [0 x i8], [0 x i8]* @Logtable, i32 0, i32 %conv1.i
  %3 = load i8, i8* %arrayidx5.i, align 1
  %conv6.i = zext i8 %3 to i32
  %add.i = add nuw nsw i32 %conv6.i, %conv3.i
  %rem.i = urem i32 %add.i, 255
  %arrayidx7.i = getelementptr inbounds [0 x i8], [0 x i8]* @Alogtable, i32 0, i32 %rem.i
  %4 = load i8, i8* %arrayidx7.i, align 1
  br label %mul.exit

mul.exit:                                         ; preds = %for.body5, %if.then.i
  %retval.0.i = phi i8 [ %4, %if.then.i ], [ 0, %for.body5 ]
  %add = add nuw nsw i32 %i.0117, 1
  %5 = icmp eq i32 %add, 4
  %tmp = select i1 %5, i32 0, i32 %add
  %arrayidx9 = getelementptr inbounds [8 x i8], [8 x i8]* %a, i32 %tmp, i32 %j.0119
  %6 = load i8, i8* %arrayidx9, align 1
  %tobool2.i102 = icmp eq i8 %6, 0
  br i1 %tobool2.i102, label %mul.exit112, label %if.then.i110

if.then.i110:                                     ; preds = %mul.exit
  %conv1.i103 = zext i8 %6 to i32
  %7 = load i8, i8* getelementptr inbounds ([0 x i8], [0 x i8]* @Logtable, i32 0, i32 11), align 1
  %conv3.i104 = zext i8 %7 to i32
  %arrayidx5.i105 = getelementptr inbounds [0 x i8], [0 x i8]* @Logtable, i32 0, i32 %conv1.i103
  %8 = load i8, i8* %arrayidx5.i105, align 1
  %conv6.i106 = zext i8 %8 to i32
  %add.i107 = add nuw nsw i32 %conv6.i106, %conv3.i104
  %rem.i108 = urem i32 %add.i107, 255
  %arrayidx7.i109 = getelementptr inbounds [0 x i8], [0 x i8]* @Alogtable, i32 0, i32 %rem.i108
  %9 = load i8, i8* %arrayidx7.i109, align 1
  br label %mul.exit112

mul.exit112:                                      ; preds = %mul.exit, %if.then.i110
  %retval.0.i111 = phi i8 [ %9, %if.then.i110 ], [ 0, %mul.exit ]
  %xor77 = xor i8 %retval.0.i111, %retval.0.i
  %add12 = add nuw nsw i32 %i.0117, 2
  %rem13 = srem i32 %add12, 4
  %arrayidx15 = getelementptr inbounds [8 x i8], [8 x i8]* %a, i32 %rem13, i32 %j.0119
  %10 = load i8, i8* %arrayidx15, align 1
  %tobool2.i91 = icmp eq i8 %10, 0
  br i1 %tobool2.i91, label %mul.exit101, label %if.then.i99

if.then.i99:                                      ; preds = %mul.exit112
  %conv1.i92 = zext i8 %10 to i32
  %11 = load i8, i8* getelementptr inbounds ([0 x i8], [0 x i8]* @Logtable, i32 0, i32 13), align 1
  %conv3.i93 = zext i8 %11 to i32
  %arrayidx5.i94 = getelementptr inbounds [0 x i8], [0 x i8]* @Logtable, i32 0, i32 %conv1.i92
  %12 = load i8, i8* %arrayidx5.i94, align 1
  %conv6.i95 = zext i8 %12 to i32
  %add.i96 = add nuw nsw i32 %conv6.i95, %conv3.i93
  %rem.i97 = urem i32 %add.i96, 255
  %arrayidx7.i98 = getelementptr inbounds [0 x i8], [0 x i8]* @Alogtable, i32 0, i32 %rem.i97
  %13 = load i8, i8* %arrayidx7.i98, align 1
  br label %mul.exit101

mul.exit101:                                      ; preds = %mul.exit112, %if.then.i99
  %retval.0.i100 = phi i8 [ %13, %if.then.i99 ], [ 0, %mul.exit112 ]
  %xor1878 = xor i8 %xor77, %retval.0.i100
  %add19 = add nuw nsw i32 %i.0117, 3
  %rem20 = srem i32 %add19, 4
  %arrayidx22 = getelementptr inbounds [8 x i8], [8 x i8]* %a, i32 %rem20, i32 %j.0119
  %14 = load i8, i8* %arrayidx22, align 1
  %tobool2.i80 = icmp eq i8 %14, 0
  br i1 %tobool2.i80, label %mul.exit90, label %if.then.i88

if.then.i88:                                      ; preds = %mul.exit101
  %conv1.i81 = zext i8 %14 to i32
  %15 = load i8, i8* getelementptr inbounds ([0 x i8], [0 x i8]* @Logtable, i32 0, i32 9), align 1
  %conv3.i82 = zext i8 %15 to i32
  %arrayidx5.i83 = getelementptr inbounds [0 x i8], [0 x i8]* @Logtable, i32 0, i32 %conv1.i81
  %16 = load i8, i8* %arrayidx5.i83, align 1
  %conv6.i84 = zext i8 %16 to i32
  %add.i85 = add nuw nsw i32 %conv6.i84, %conv3.i82
  %rem.i86 = urem i32 %add.i85, 255
  %arrayidx7.i87 = getelementptr inbounds [0 x i8], [0 x i8]* @Alogtable, i32 0, i32 %rem.i86
  %17 = load i8, i8* %arrayidx7.i87, align 1
  br label %mul.exit90

mul.exit90:                                       ; preds = %mul.exit101, %if.then.i88
  %retval.0.i89 = phi i8 [ %17, %if.then.i88 ], [ 0, %mul.exit101 ]
  %xor2579 = xor i8 %xor1878, %retval.0.i89
  %arrayidx28 = getelementptr inbounds [4 x [8 x i8]], [4 x [8 x i8]]* %b, i32 0, i32 %i.0117, i32 %j.0119
  store i8 %xor2579, i8* %arrayidx28, align 1
  %exitcond = icmp eq i32 %add, 4
  br i1 %exitcond, label %for.inc29, label %for.body5

for.inc29:                                        ; preds = %mul.exit90
  %inc30 = add nuw nsw i32 %j.0119, 1
  %exitc.nd121 = icmp eq i32 %inc30, %conv
  br i1 %exitc.nd121, label %for.cond32.preheader, label %for.cond2.preheader

for.cond32.preheader:                             ; preds = %for.inc29
  br i1 %cmp118, label %for.inc48.3, label %for.inc48.3

for.inc48.3:                                      ; preds = %entry, %for.cond32.preheader, %for.inc48.3
  ret void
}

