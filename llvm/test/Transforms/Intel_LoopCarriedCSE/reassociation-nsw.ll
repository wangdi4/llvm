; Removing nsw and nuw flags in the instruction chains when reassociation happens
;
; RUN: opt < %s -loop-carried-cse -S 2>&1 | FileCheck %s
; RUN: opt -passes="loop-carried-cse" -S 2>&1 < %s | FileCheck %s
;
; CHECK: %1 = add i32 %gepload, %gepload56
; CHECK: %t46.0 = phi i32
; CHECK: %t44.0.lccse = phi i32 [ %1, %for.body.preheader ], [ %6, %loop.34 ]
; CHECK: %2 = add i32 %t44.0.lccse, %gepload59
; CHECK: %3 = add i32 %2, %gepload57
;
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@E = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@F = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@m = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@D = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @sub(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp43 = icmp sgt i32 %n, 0
  br i1 %cmp43, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  %gepload = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @B, i64 0, i64 0), align 4, !tbaa !2
  %gepload56 = load i32, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @C, i64 0, i64 0), align 4, !tbaa !2
  %0 = add i64 %wide.trip.count, -1
  br label %loop.34

for.end:                                          ; preds = %loop.34, %entry
  ret void

loop.34:                                          ; preds = %loop.34, %for.body.preheader
  %i1.i64.0 = phi i64 [ 0, %for.body.preheader ], [ %4, %loop.34 ]
  %t46.0 = phi i32 [ %gepload56, %for.body.preheader ], [ %gepload64, %loop.34 ]
  %t44.0 = phi i32 [ %gepload, %for.body.preheader ], [ %gepload62, %loop.34 ]
  %arrayIdx = getelementptr inbounds [1000 x i32], [1000 x i32]* @E, i64 0, i64 %i1.i64.0
  %gepload57 = load i32, i32* %arrayIdx, align 4, !tbaa !2
  %arrayIdx58 = getelementptr inbounds [1000 x i32], [1000 x i32]* @F, i64 0, i64 %i1.i64.0
  %gepload59 = load i32, i32* %arrayIdx58, align 4, !tbaa !2
  %arrayIdx60 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %i1.i64.0
  %1 = add nsw i32 %t44.0, %gepload57
  %2 = add nsw i32 %1, %gepload59
  %3 = add nuw i32 %2, %t46.0
  store i32 %3, i32* %arrayIdx60, align 4, !tbaa !2
  %4 = add i64 %i1.i64.0, 1
  %arrayIdx61 = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %4
  %gepload62 = load i32, i32* %arrayIdx61, align 4, !tbaa !2
  %arrayIdx63 = getelementptr inbounds [1000 x i32], [1000 x i32]* @C, i64 0, i64 %4
  %gepload64 = load i32, i32* %arrayIdx63, align 4, !tbaa !2
  %arrayIdx65 = getelementptr inbounds [1000 x i32], [1000 x i32]* @m, i64 0, i64 %i1.i64.0
  %gepload66 = load i32, i32* %arrayIdx65, align 4, !tbaa !2
  %5 = sext i32 %gepload66 to i64
  %arrayIdx67 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %5
  %6 = add nsw i32 %gepload62, %gepload64
  store i32 %6, i32* %arrayIdx67, align 4, !tbaa !2
  %arrayIdx68 = getelementptr inbounds [1000 x i32], [1000 x i32]* @D, i64 0, i64 %i1.i64.0
  %7 = add i32 %t46.0, %gepload64
  store i32 %7, i32* %arrayIdx68, align 4, !tbaa !2
  %condloop.34 = icmp sle i64 %4, %0
  br i1 %condloop.34, label %loop.34, label %for.end, !llvm.loop !7
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) 2019.8.2.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8, !9}
!8 = !{!"llvm.loop.vectorize.width", i32 1}
!9 = !{!"llvm.loop.unroll.disable"}

