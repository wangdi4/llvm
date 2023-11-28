; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check parsing output for the loop verifying that the index of the store is merged properly.

; CHECK: + DO i1 = 0, 15, 1   <DO_LOOP>
; CHECK: |   %0 = (%pSrc1)[%src1Step * i1];
; CHECK: |   %1 = (%pSrc2)[%src2Step * i1];
; CHECK: |   (i16*)(%pDst)[%dstStep * i1] = sext.i8.i16(%0) + sext.i8.i16(%1);
; CHECK: |   %2 = (%pSrc1)[%src1Step * i1 + 1];
; CHECK: |   %3 = (%pSrc2)[%src2Step * i1 + 1];
; CHECK: |   (%pDst.addr.025)[1] = sext.i8.i16(%2) + sext.i8.i16(%3);
; CHECK: |   %4 = (%pSrc1)[%src1Step * i1 + 2];
; CHECK: |   %5 = (%pSrc2)[%src2Step * i1 + 2];
; CHECK: |   (%pDst.addr.025)[2] = sext.i8.i16(%4) + sext.i8.i16(%5);
; CHECK: |   %6 = (%pSrc1)[%src1Step * i1 + 3];
; CHECK: |   %7 = (%pSrc2)[%src2Step * i1 + 3];
; CHECK: |   (%pDst.addr.025)[3] = sext.i8.i16(%6) + sext.i8.i16(%7);
; CHECK: |   %8 = (%pSrc1)[%src1Step * i1 + 4];
; CHECK: |   %9 = (%pSrc2)[%src2Step * i1 + 4];
; CHECK: |   (%pDst.addr.025)[4] = sext.i8.i16(%8) + sext.i8.i16(%9);
; CHECK: |   %10 = (%pSrc1)[%src1Step * i1 + 5];
; CHECK: |   %11 = (%pSrc2)[%src2Step * i1 + 5];
; CHECK: |   (%pDst.addr.025)[5] = sext.i8.i16(%10) + sext.i8.i16(%11);
; CHECK: |   %12 = (%pSrc1)[%src1Step * i1 + 6];
; CHECK: |   %13 = (%pSrc2)[%src2Step * i1 + 6];
; CHECK: |   (%pDst.addr.025)[6] = sext.i8.i16(%12) + sext.i8.i16(%13);
; CHECK: |   %14 = (%pSrc1)[%src1Step * i1 + 7];
; CHECK: |   %15 = (%pSrc2)[%src2Step * i1 + 7];
; CHECK: |   (%pDst.addr.025)[7] = sext.i8.i16(%14) + sext.i8.i16(%15);
; CHECK: |   %16 = (%pSrc1)[%src1Step * i1 + 8];
; CHECK: |   %17 = (%pSrc2)[%src2Step * i1 + 8];
; CHECK: |   (%pDst.addr.025)[8] = sext.i8.i16(%16) + sext.i8.i16(%17);
; CHECK: |   %18 = (%pSrc1)[%src1Step * i1 + 9];
; CHECK: |   %19 = (%pSrc2)[%src2Step * i1 + 9];
; CHECK: |   (%pDst.addr.025)[9] = sext.i8.i16(%18) + sext.i8.i16(%19);
; CHECK: |   %20 = (%pSrc1)[%src1Step * i1 + 10];
; CHECK: |   %21 = (%pSrc2)[%src2Step * i1 + 10];
; CHECK: |   (%pDst.addr.025)[10] = sext.i8.i16(%20) + sext.i8.i16(%21);
; CHECK: |   %22 = (%pSrc1)[%src1Step * i1 + 11];
; CHECK: |   %23 = (%pSrc2)[%src2Step * i1 + 11];
; CHECK: |   (%pDst.addr.025)[11] = sext.i8.i16(%22) + sext.i8.i16(%23);
; CHECK: |   %24 = (%pSrc1)[%src1Step * i1 + 12];
; CHECK: |   %25 = (%pSrc2)[%src2Step * i1 + 12];
; CHECK: |   (%pDst.addr.025)[12] = sext.i8.i16(%24) + sext.i8.i16(%25);
; CHECK: |   %26 = (%pSrc1)[%src1Step * i1 + 13];
; CHECK: |   %27 = (%pSrc2)[%src2Step * i1 + 13];
; CHECK: |   (%pDst.addr.025)[13] = sext.i8.i16(%26) + sext.i8.i16(%27);
; CHECK: |   %28 = (%pSrc1)[%src1Step * i1 + 14];
; CHECK: |   %29 = (%pSrc2)[%src2Step * i1 + 14];
; CHECK: |   (%pDst.addr.025)[14] = sext.i8.i16(%28) + sext.i8.i16(%29);
; CHECK: |   %30 = (%pSrc1)[%src1Step * i1 + 15];
; CHECK: |   %31 = (%pSrc2)[%src2Step * i1 + 15];
; CHECK: |   (%pDst.addr.025)[15] = sext.i8.i16(%30) + sext.i8.i16(%31);
; CHECK: |   %pDst.addr.025 = &((%pDst)[%dstStep * i1 + %dstStep]);
; CHECK: + END LOOP


;Module Before HIR; ModuleID = 'cq153724.c'
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @cq153724(ptr noalias nocapture readonly %pSrc1, i32 %src1Step, ptr noalias nocapture readonly %pSrc2, i32 %src2Step, ptr noalias nocapture %pDst, i32 %dstStep) {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.cond.1.preheader, %entry
  %i.027 = phi i32 [ 0, %entry ], [ %inc11, %for.cond.1.preheader ]
  %pSrc1.addr.026 = phi ptr [ %pSrc1, %entry ], [ %add.ptr, %for.cond.1.preheader ]
  %pDst.addr.025 = phi ptr [ %pDst, %entry ], [ %add.ptr9, %for.cond.1.preheader ]
  %pSrc2.addr.024 = phi ptr [ %pSrc2, %entry ], [ %add.ptr8, %for.cond.1.preheader ]
  %0 = load i8, ptr %pSrc1.addr.026, align 1
  %conv = sext i8 %0 to i16
  %1 = load i8, ptr %pSrc2.addr.024, align 1
  %conv5 = sext i8 %1 to i16
  %add = add nsw i16 %conv5, %conv
  store i16 %add, ptr %pDst.addr.025, align 2
  %arrayidx.1 = getelementptr inbounds i8, ptr %pSrc1.addr.026, i32 1
  %2 = load i8, ptr %arrayidx.1, align 1
  %conv.1 = sext i8 %2 to i16
  %arrayidx4.1 = getelementptr inbounds i8, ptr %pSrc2.addr.024, i32 1
  %3 = load i8, ptr %arrayidx4.1, align 1
  %conv5.1 = sext i8 %3 to i16
  %add.1 = add nsw i16 %conv5.1, %conv.1
  %arrayidx7.1 = getelementptr inbounds i16, ptr %pDst.addr.025, i32 1
  store i16 %add.1, ptr %arrayidx7.1, align 2
  %arrayidx.2 = getelementptr inbounds i8, ptr %pSrc1.addr.026, i32 2
  %4 = load i8, ptr %arrayidx.2, align 1
  %conv.2 = sext i8 %4 to i16
  %arrayidx4.2 = getelementptr inbounds i8, ptr %pSrc2.addr.024, i32 2
  %5 = load i8, ptr %arrayidx4.2, align 1
  %conv5.2 = sext i8 %5 to i16
  %add.2 = add nsw i16 %conv5.2, %conv.2
  %arrayidx7.2 = getelementptr inbounds i16, ptr %pDst.addr.025, i32 2
  store i16 %add.2, ptr %arrayidx7.2, align 2
  %arrayidx.3 = getelementptr inbounds i8, ptr %pSrc1.addr.026, i32 3
  %6 = load i8, ptr %arrayidx.3, align 1
  %conv.3 = sext i8 %6 to i16
  %arrayidx4.3 = getelementptr inbounds i8, ptr %pSrc2.addr.024, i32 3
  %7 = load i8, ptr %arrayidx4.3, align 1
  %conv5.3 = sext i8 %7 to i16
  %add.3 = add nsw i16 %conv5.3, %conv.3
  %arrayidx7.3 = getelementptr inbounds i16, ptr %pDst.addr.025, i32 3
  store i16 %add.3, ptr %arrayidx7.3, align 2
  %arrayidx.4 = getelementptr inbounds i8, ptr %pSrc1.addr.026, i32 4
  %8 = load i8, ptr %arrayidx.4, align 1
  %conv.4 = sext i8 %8 to i16
  %arrayidx4.4 = getelementptr inbounds i8, ptr %pSrc2.addr.024, i32 4
  %9 = load i8, ptr %arrayidx4.4, align 1
  %conv5.4 = sext i8 %9 to i16
  %add.4 = add nsw i16 %conv5.4, %conv.4
  %arrayidx7.4 = getelementptr inbounds i16, ptr %pDst.addr.025, i32 4
  store i16 %add.4, ptr %arrayidx7.4, align 2
  %arrayidx.5 = getelementptr inbounds i8, ptr %pSrc1.addr.026, i32 5
  %10 = load i8, ptr %arrayidx.5, align 1
  %conv.5 = sext i8 %10 to i16
  %arrayidx4.5 = getelementptr inbounds i8, ptr %pSrc2.addr.024, i32 5
  %11 = load i8, ptr %arrayidx4.5, align 1
  %conv5.5 = sext i8 %11 to i16
  %add.5 = add nsw i16 %conv5.5, %conv.5
  %arrayidx7.5 = getelementptr inbounds i16, ptr %pDst.addr.025, i32 5
  store i16 %add.5, ptr %arrayidx7.5, align 2
  %arrayidx.6 = getelementptr inbounds i8, ptr %pSrc1.addr.026, i32 6
  %12 = load i8, ptr %arrayidx.6, align 1
  %conv.6 = sext i8 %12 to i16
  %arrayidx4.6 = getelementptr inbounds i8, ptr %pSrc2.addr.024, i32 6
  %13 = load i8, ptr %arrayidx4.6, align 1
  %conv5.6 = sext i8 %13 to i16
  %add.6 = add nsw i16 %conv5.6, %conv.6
  %arrayidx7.6 = getelementptr inbounds i16, ptr %pDst.addr.025, i32 6
  store i16 %add.6, ptr %arrayidx7.6, align 2
  %arrayidx.7 = getelementptr inbounds i8, ptr %pSrc1.addr.026, i32 7
  %14 = load i8, ptr %arrayidx.7, align 1
  %conv.7 = sext i8 %14 to i16
  %arrayidx4.7 = getelementptr inbounds i8, ptr %pSrc2.addr.024, i32 7
  %15 = load i8, ptr %arrayidx4.7, align 1
  %conv5.7 = sext i8 %15 to i16
  %add.7 = add nsw i16 %conv5.7, %conv.7
  %arrayidx7.7 = getelementptr inbounds i16, ptr %pDst.addr.025, i32 7
  store i16 %add.7, ptr %arrayidx7.7, align 2
  %arrayidx.8 = getelementptr inbounds i8, ptr %pSrc1.addr.026, i32 8
  %16 = load i8, ptr %arrayidx.8, align 1
  %conv.8 = sext i8 %16 to i16
  %arrayidx4.8 = getelementptr inbounds i8, ptr %pSrc2.addr.024, i32 8
  %17 = load i8, ptr %arrayidx4.8, align 1
  %conv5.8 = sext i8 %17 to i16
  %add.8 = add nsw i16 %conv5.8, %conv.8
  %arrayidx7.8 = getelementptr inbounds i16, ptr %pDst.addr.025, i32 8
  store i16 %add.8, ptr %arrayidx7.8, align 2
  %arrayidx.9 = getelementptr inbounds i8, ptr %pSrc1.addr.026, i32 9
  %18 = load i8, ptr %arrayidx.9, align 1
  %conv.9 = sext i8 %18 to i16
  %arrayidx4.9 = getelementptr inbounds i8, ptr %pSrc2.addr.024, i32 9
  %19 = load i8, ptr %arrayidx4.9, align 1
  %conv5.9 = sext i8 %19 to i16
  %add.9 = add nsw i16 %conv5.9, %conv.9
  %arrayidx7.9 = getelementptr inbounds i16, ptr %pDst.addr.025, i32 9
  store i16 %add.9, ptr %arrayidx7.9, align 2
  %arrayidx.10 = getelementptr inbounds i8, ptr %pSrc1.addr.026, i32 10
  %20 = load i8, ptr %arrayidx.10, align 1
  %conv.10 = sext i8 %20 to i16
  %arrayidx4.10 = getelementptr inbounds i8, ptr %pSrc2.addr.024, i32 10
  %21 = load i8, ptr %arrayidx4.10, align 1
  %conv5.10 = sext i8 %21 to i16
  %add.10 = add nsw i16 %conv5.10, %conv.10
  %arrayidx7.10 = getelementptr inbounds i16, ptr %pDst.addr.025, i32 10
  store i16 %add.10, ptr %arrayidx7.10, align 2
  %arrayidx.11 = getelementptr inbounds i8, ptr %pSrc1.addr.026, i32 11
  %22 = load i8, ptr %arrayidx.11, align 1
  %conv.11 = sext i8 %22 to i16
  %arrayidx4.11 = getelementptr inbounds i8, ptr %pSrc2.addr.024, i32 11
  %23 = load i8, ptr %arrayidx4.11, align 1
  %conv5.11 = sext i8 %23 to i16
  %add.11 = add nsw i16 %conv5.11, %conv.11
  %arrayidx7.11 = getelementptr inbounds i16, ptr %pDst.addr.025, i32 11
  store i16 %add.11, ptr %arrayidx7.11, align 2
  %arrayidx.12 = getelementptr inbounds i8, ptr %pSrc1.addr.026, i32 12
  %24 = load i8, ptr %arrayidx.12, align 1
  %conv.12 = sext i8 %24 to i16
  %arrayidx4.12 = getelementptr inbounds i8, ptr %pSrc2.addr.024, i32 12
  %25 = load i8, ptr %arrayidx4.12, align 1
  %conv5.12 = sext i8 %25 to i16
  %add.12 = add nsw i16 %conv5.12, %conv.12
  %arrayidx7.12 = getelementptr inbounds i16, ptr %pDst.addr.025, i32 12
  store i16 %add.12, ptr %arrayidx7.12, align 2
  %arrayidx.13 = getelementptr inbounds i8, ptr %pSrc1.addr.026, i32 13
  %26 = load i8, ptr %arrayidx.13, align 1
  %conv.13 = sext i8 %26 to i16
  %arrayidx4.13 = getelementptr inbounds i8, ptr %pSrc2.addr.024, i32 13
  %27 = load i8, ptr %arrayidx4.13, align 1
  %conv5.13 = sext i8 %27 to i16
  %add.13 = add nsw i16 %conv5.13, %conv.13
  %arrayidx7.13 = getelementptr inbounds i16, ptr %pDst.addr.025, i32 13
  store i16 %add.13, ptr %arrayidx7.13, align 2
  %arrayidx.14 = getelementptr inbounds i8, ptr %pSrc1.addr.026, i32 14
  %28 = load i8, ptr %arrayidx.14, align 1
  %conv.14 = sext i8 %28 to i16
  %arrayidx4.14 = getelementptr inbounds i8, ptr %pSrc2.addr.024, i32 14
  %29 = load i8, ptr %arrayidx4.14, align 1
  %conv5.14 = sext i8 %29 to i16
  %add.14 = add nsw i16 %conv5.14, %conv.14
  %arrayidx7.14 = getelementptr inbounds i16, ptr %pDst.addr.025, i32 14
  store i16 %add.14, ptr %arrayidx7.14, align 2
  %arrayidx.15 = getelementptr inbounds i8, ptr %pSrc1.addr.026, i32 15
  %30 = load i8, ptr %arrayidx.15, align 1
  %conv.15 = sext i8 %30 to i16
  %arrayidx4.15 = getelementptr inbounds i8, ptr %pSrc2.addr.024, i32 15
  %31 = load i8, ptr %arrayidx4.15, align 1
  %conv5.15 = sext i8 %31 to i16
  %add.15 = add nsw i16 %conv5.15, %conv.15
  %arrayidx7.15 = getelementptr inbounds i16, ptr %pDst.addr.025, i32 15
  store i16 %add.15, ptr %arrayidx7.15, align 2
  %add.ptr = getelementptr inbounds i8, ptr %pSrc1.addr.026, i32 %src1Step
  %add.ptr8 = getelementptr inbounds i8, ptr %pSrc2.addr.024, i32 %src2Step
  %add.ptr9 = getelementptr inbounds i8, ptr %pDst.addr.025, i32 %dstStep
  %inc11 = add nuw nsw i32 %i.027, 1
  %exitcond28 = icmp eq i32 %inc11, 16
  br i1 %exitcond28, label %for.end.12, label %for.cond.1.preheader

for.end.12:                                       ; preds = %for.cond.1.preheader
  ret void
}

