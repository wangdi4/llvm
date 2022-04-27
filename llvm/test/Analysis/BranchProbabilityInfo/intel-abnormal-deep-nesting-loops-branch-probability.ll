;RUN: opt < %s -analyze -enable-new-pm=0 -branch-prob -abnormal-loop-depth-threshold=3 -enable-intel-advanced-opts | FileCheck %s --check-prefixes=CHECK

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [1 x [9 x i32]] zeroinitializer, align 16
@soln = dso_local local_unnamed_addr global [1 x i32] zeroinitializer, align 4

; CHECK: Printing analysis 'Branch Probability Analysis' for function 'abnormal_deep_nesting_loops_test':
; CHECK:   edge entry -> for.body probability is 0x80000000 / 0x80000000 = 100.00% [HOT edge]
; CHECK:   edge for.body -> for.inc37 probability is 0x66666666 / 0x80000000 = 80.00%
; CHECK:   edge for.body -> if.end probability is 0x1999999a / 0x80000000 = 20.00%
; CHECK:   edge if.end -> for.body5 probability is 0x80000000 / 0x80000000 = 100.00% [HOT edge]
; CHECK:   edge for.body5 -> for.inc32 probability is 0x66666666 / 0x80000000 = 80.00%
; CHECK:   edge for.body5 -> if.end9 probability is 0x1999999a / 0x80000000 = 20.00%
; CHECK:   edge if.end9 -> cleanup probability is 0x04000000 / 0x80000000 = 3.12%
; CHECK:   edge if.end9 -> for.body18 probability is 0x7c000000 / 0x80000000 = 96.88% [HOT edge]
; CHECK:   edge for.body18 -> for.inc probability is 0x30000000 / 0x80000000 = 37.50%
; CHECK:   edge for.body18 -> if.end22 probability is 0x50000000 / 0x80000000 = 62.50%
; CHECK:   edge if.end22 -> cleanup probability is 0x04000000 / 0x80000000 = 3.12%
; CHECK:   edge if.end22 -> if.end28 probability is 0x7c000000 / 0x80000000 = 96.88% [HOT edge]
; CHECK:   edge if.end28 -> for.inc probability is 0x80000000 / 0x80000000 = 100.00% [HOT edge]
; CHECK:   edge for.inc -> for.end probability is 0x0ea0ea0f / 0x80000000 = 11.43%
; CHECK:   edge for.inc -> for.body18 probability is 0x715f15f1 / 0x80000000 = 88.57% [HOT edge]
; CHECK:   edge for.end -> for.inc32 probability is 0x80000000 / 0x80000000 = 100.00% [HOT edge]
; CHECK:   edge for.inc32 -> for.end34 probability is 0x0ea0ea0f / 0x80000000 = 11.43%
; CHECK:   edge for.inc32 -> for.body5 probability is 0x715f15f1 / 0x80000000 = 88.57% [HOT edge]
; CHECK:   edge for.end34 -> for.inc37 probability is 0x80000000 / 0x80000000 = 100.00% [HOT edge]
; CHECK:   edge for.inc37 -> cleanup probability is 0x0ea0ea0f / 0x80000000 = 11.43%
; CHECK:   edge for.inc37 -> for.body probability is 0x715f15f1 / 0x80000000 = 88.57% [HOT edge]

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @abnormal_deep_nesting_loops_test(i64 %u1, i64 %u2, i64 %u3) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc37
  %i1.071 = phi i64 [ 0, %entry ], [ %inc38, %for.inc37 ]
  %arrayidx = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 1, i64 0, i64 %i1.071
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp slt i32 %0, 1
  br i1 %cmp1, label %for.inc37, label %if.end

if.end:                                           ; preds = %for.body
  %sub = add nsw i32 %0, -11
  store i32 %sub, i32* %arrayidx, align 4, !tbaa !2
  br label %for.body5

for.body5:                                        ; preds = %if.end, %for.inc32
  %i2.070 = phi i64 [ 0, %if.end ], [ %inc33, %for.inc32 ]
  %arrayidx6 = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 2, i64 0, i64 %i2.070
  %1 = load i32, i32* %arrayidx6, align 4, !tbaa !2
  %cmp7 = icmp slt i32 %1, 1
  br i1 %cmp7, label %for.inc32, label %if.end9

if.end9:                                          ; preds = %for.body5
  %sub11 = add nsw i32 %1, -12
  store i32 %sub11, i32* %arrayidx6, align 4, !tbaa !2
  %arrayidx12 = getelementptr inbounds [1 x i32], [1 x i32]* @soln, i64 0, i64 %i2.070
  %2 = load i32, i32* %arrayidx12, align 4, !tbaa !2
  %cmp13 = icmp sgt i32 %2, 1
  br i1 %cmp13, label %cleanup, label %for.body18

for.body18:                                       ; preds = %if.end9, %for.inc
  %i3.069 = phi i64 [ %inc, %for.inc ], [ 0, %if.end9 ]
  %arrayidx19 = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 3, i64 0, i64 %i3.069
  %3 = load i32, i32* %arrayidx19, align 4, !tbaa !2
  %cmp20 = icmp slt i32 %3, 1
  br i1 %cmp20, label %for.inc, label %if.end22

if.end22:                                         ; preds = %for.body18
  %sub24 = add nsw i32 %3, -13
  store i32 %sub24, i32* %arrayidx19, align 4, !tbaa !2
  %arrayidx25 = getelementptr inbounds [1 x i32], [1 x i32]* @soln, i64 0, i64 %i3.069
  %4 = load i32, i32* %arrayidx25, align 4, !tbaa !2
  %cmp26 = icmp sgt i32 %4, 1
  br i1 %cmp26, label %cleanup, label %if.end28

if.end28:                                         ; preds = %if.end22
  store i32 %3, i32* %arrayidx19, align 4, !tbaa !2
  br label %for.inc

for.inc:                                          ; preds = %for.body18, %if.end28
  %inc = add i64 %i3.069, 1
  %cmp17 = icmp ugt i64 %inc, %u3
  br i1 %cmp17, label %for.end, label %for.body18

for.end:                                          ; preds = %for.inc
  %5 = load i32, i32* %arrayidx6, align 4, !tbaa !2
  %add31 = add nsw i32 %5, 12
  store i32 %add31, i32* %arrayidx6, align 4, !tbaa !2
  br label %for.inc32

for.inc32:                                        ; preds = %for.body5, %for.end
  %inc33 = add i64 %i2.070, 1
  %cmp4 = icmp ugt i64 %inc33, %u2
  br i1 %cmp4, label %for.end34, label %for.body5

for.end34:                                        ; preds = %for.inc32
  %6 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %add36 = add nsw i32 %6, 11
  store i32 %add36, i32* %arrayidx, align 4, !tbaa !2
  br label %for.inc37

for.inc37:                                        ; preds = %for.body, %for.end34
  %inc38 = add i64 %i1.071, 1
  %cmp = icmp ugt i64 %inc38, %u1
  br i1 %cmp, label %cleanup, label %for.body

cleanup:                                          ; preds = %for.inc37, %if.end9, %if.end22
  ret void
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
