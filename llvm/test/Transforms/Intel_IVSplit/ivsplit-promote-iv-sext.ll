; RUN: opt < %s -iv-split -iv-split-loop-depth-threshold=2 -iv-sext-promote-scale-threshold=8 -S | FileCheck %s
; This test case checks optimization which transforms following sequence:
;
; %6155 = load i32, i32* getelementptr ...
; %6694 = trunc i64 %3999 to i32
; %3996 = trunc i64 %3970 to i32
; %3967 = trunc i64 %3938 to i32
; %3935 = trunc i64 %3903 to i32
; %3900 = trunc i64 %3865 to i32
; %3862 = trunc i64 %3824 to i32
; %3821 = trunc i64 %3780 to i32
; %6700 = add i32 %6155, %6694
; %6701 = add i32 %6700, %3996
; %6702 = add i32 %6701, %3967
; %6703 = add i32 %6702, %3935
; %6704 = add i32 %6703, %3900
; %6705 = add i32 %6704, %3862
; %6706 = add i32 %6705, %3821
; %6707 = sub i32 45, %6706
; %6708 = sext i32 %6707 to i64
;
; to following sequence (with less trunc):
;
; %6155 = load i32, i32* getelementptr ...
; %6156 = sext i32 %6155 to i64
; %6702 = add i64 %6156, %3999
; %6704 = add i64 %6702, %3970
; %6706 = add i64 %6704, %3938
; %6708 = add i64 %6706, %3903
; %6710 = add i64 %6708, %3865
; %6712 = add i64 %6710, %3824
; %6714 = add i64 %6712, %3780
; %6716 = bitcast i64 193273528320 to i64 # 193273528320 = 45<<32
; %6717 = shl i64 %6714, 32
; %6718 = sub i64 %6716, %6717
; %6719 = ashr i64 %6718, 32

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a1 = external dso_local local_unnamed_addr global [0 x i32], align 4
@a2 = external dso_local local_unnamed_addr global [0 x i32], align 4
@a3 = external dso_local local_unnamed_addr global [0 x i32], align 4
@b = external dso_local local_unnamed_addr global [0 x i32], align 4
@a9 = external dso_local local_unnamed_addr global i32*, align 8

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @ivsplit_promote_iv_sext_test(i64 %u1, i64 %u2, i64 %u3) local_unnamed_addr #0 {
; CHECK-LABEL: @ivsplit_promote_iv_sext_test(
; CHECK: [[CONST:%.*]] = bitcast i64 193273528320 to i64
; CHECK-NEXT: [[SHL:%.*]] = shl i64 [[ADD:%.*]], 32
; CHECK-NEXT: [[SUB:%.*]] = sub i64 [[CONST]], [[SHL]]
; CHECK-NEXT: [[ASHR:%.*]] = ashr i64 [[SUB]], 32
entry:
  %cmp83 = icmp slt i64 %u1, 0
  br i1 %cmp83, label %for.end54, label %for.body

for.body:                                         ; preds = %entry, %for.inc52
  %i1.084 = phi i64 [ %.pre89, %for.inc52 ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds [0 x i32], [0 x i32]* @a1, i64 0, i64 %i1.084
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp slt i32 %0, 1
  %.pre89 = add nuw i64 %i1.084, 1
  br i1 %cmp1, label %for.inc52, label %if.end

if.end:                                           ; preds = %for.body
  %arrayidx2 = getelementptr inbounds [0 x i32], [0 x i32]* @a1, i64 0, i64 %.pre89
  %1 = load i32, i32* %arrayidx2, align 4, !tbaa !2
  %sub = add nsw i32 %1, -10
  store i32 %sub, i32* %arrayidx2, align 4, !tbaa !2
  %2 = or i64 %u2, %u3
  %3 = icmp slt i64 %2, 0
  br i1 %3, label %for.end48, label %for.body5

for.body5:                                        ; preds = %if.end, %for.inc46
  %i2.082 = phi i64 [ %.pre90, %for.inc46 ], [ 0, %if.end ]
  %arrayidx6 = getelementptr inbounds [0 x i32], [0 x i32]* @a2, i64 0, i64 %i2.082
  %4 = load i32, i32* %arrayidx6, align 4, !tbaa !2
  %cmp7 = icmp slt i32 %4, 1
  %.pre90 = add nuw i64 %i2.082, 1
  br i1 %cmp7, label %for.inc46, label %if.end9

if.end9:                                          ; preds = %for.body5
  %arrayidx11 = getelementptr inbounds [0 x i32], [0 x i32]* @a2, i64 0, i64 %.pre90
  %5 = load i32, i32* %arrayidx11, align 4, !tbaa !2
  %sub12 = add nsw i32 %5, -10
  store i32 %sub12, i32* %arrayidx11, align 4, !tbaa !2
  br label %for.body15

for.body15:                                       ; preds = %for.inc, %if.end9
  %i3.080 = phi i64 [ 0, %if.end9 ], [ %.pre91, %for.inc ]
  %arrayidx16 = getelementptr inbounds [0 x i32], [0 x i32]* @a3, i64 0, i64 %i3.080
  %6 = load i32, i32* %arrayidx16, align 4, !tbaa !2
  %cmp17 = icmp slt i32 %6, 1
  %.pre91 = add nuw i64 %i3.080, 1
  br i1 %cmp17, label %for.inc, label %if.end19

if.end19:                                         ; preds = %for.body15
  %arrayidx21 = getelementptr inbounds [0 x i32], [0 x i32]* @a3, i64 0, i64 %.pre91
  %7 = load i32, i32* %arrayidx21, align 4, !tbaa !2
  %sub22 = add nsw i32 %7, -10
  store i32 %sub22, i32* %arrayidx21, align 4, !tbaa !2
  %conv = trunc i64 %i1.084 to i32
  %conv23 = trunc i64 %i2.082 to i32
  %add24 = add i32 %conv23, %conv
  %conv25 = trunc i64 %i3.080 to i32
  %add26 = add i32 %add24, %conv25
  %add27 = add nuw nsw i64 %i3.080, 2
  %arrayidx28 = getelementptr inbounds [0 x i32], [0 x i32]* @b, i64 0, i64 %add27
  %8 = load i32, i32* %arrayidx28, align 4, !tbaa !2
  %add29 = add i32 %add26, %8
  %sub30 = sub i32 45, %add29
  %9 = load i32*, i32** @a9, align 8, !tbaa !6
  %idxprom = sext i32 %sub30 to i64
  %arrayidx31 = getelementptr inbounds i32, i32* %9, i64 %idxprom
  %10 = load i32, i32* %arrayidx31, align 4, !tbaa !2
  %cmp32 = icmp sgt i32 %10, 0
  br i1 %cmp32, label %if.then34, label %if.end39

if.then34:                                        ; preds = %if.end19
  %add35 = sub i32 46, %8
  %idxprom36 = sext i32 %add35 to i64
  %arrayidx37 = getelementptr inbounds i32, i32* %9, i64 %idxprom36
  %11 = load i32, i32* %arrayidx37, align 4, !tbaa !2
  %sub38 = add nsw i32 %11, -9
  store i32 %sub38, i32* %arrayidx37, align 4, !tbaa !2
  %.pre = load i32, i32* %arrayidx21, align 4, !tbaa !2
  br label %if.end39

if.end39:                                         ; preds = %if.then34, %if.end19
  %12 = phi i32 [ %.pre, %if.then34 ], [ %sub22, %if.end19 ]
  %add42 = add nsw i32 %12, 10
  store i32 %add42, i32* %arrayidx21, align 4, !tbaa !2
  br label %for.inc

for.inc:                                          ; preds = %for.body15, %if.end39
  %exitcond = icmp eq i64 %i3.080, %u3
  br i1 %exitcond, label %for.end, label %for.body15

for.end:                                          ; preds = %for.inc
  %13 = load i32, i32* %arrayidx11, align 4, !tbaa !2
  %add45 = add nsw i32 %13, 10
  store i32 %add45, i32* %arrayidx11, align 4, !tbaa !2
  br label %for.inc46

for.inc46:                                        ; preds = %for.body5, %for.end
  %exitcond86 = icmp eq i64 %i2.082, %u2
  br i1 %exitcond86, label %for.end48.loopexit85, label %for.body5

for.end48.loopexit85:                             ; preds = %for.inc46
  %.pre88 = load i32, i32* %arrayidx2, align 4, !tbaa !2
  br label %for.end48

for.end48:                                        ; preds = %if.end, %for.end48.loopexit85
  %14 = phi i32 [ %.pre88, %for.end48.loopexit85 ], [ %sub, %if.end ]
  %add51 = add nsw i32 %14, 10
  store i32 %add51, i32* %arrayidx2, align 4, !tbaa !2
  br label %for.inc52

for.inc52:                                        ; preds = %for.body, %for.end48
  %exitcond87 = icmp eq i64 %i1.084, %u1
  br i1 %exitcond87, label %for.end54, label %for.body

for.end54:                                        ; preds = %for.inc52, %entry
  ret void
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" "contains-rec-pro-clone" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"any pointer", !4, i64 0}
