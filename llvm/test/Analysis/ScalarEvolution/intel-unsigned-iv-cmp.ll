; CMPLRLLVM-25541
; -indvars is removing the compare/branch of the IV d.013, in for.body11.
; SCEV is incorrectly "proving" that the compare of e.020 in for.body3,
; implies that d.013 is always < 4.
; This logic is used:
; If X > 0 and -B >= X, then B < 0.
; This logic doesn't work for unsigned compares, as -B may convert a small
; unsigned number into a large one, which may be trivially > X.

; RUN: opt -indvars -S %s | FileCheck %s
; RUN: opt -passes=indvars -S %s | FileCheck %s
; INTEL
; CHECK: %cmp10 = icmp ult i64 %indvars.iv1, 4
; CHECK: br i1 %cmp10, label %for.body11, label %for.cond8.for.end_crit_edge

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@k = dso_local local_unnamed_addr global i8 0, align 1
@q = dso_local local_unnamed_addr global i64 0, align 8
@p = dso_local local_unnamed_addr global [20 x i32] zeroinitializer, align 16
@i = dso_local local_unnamed_addr global [20 x i8] zeroinitializer, align 16
@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.end21
  %storemerge21 = phi i64 [ 0, %entry ], [ %add23, %for.end21 ]
  %conv = trunc i64 %storemerge21 to i32
  %conv516 = and i64 %storemerge21, 4294967295
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.end19
  %e.020 = phi i64 [ 0, %for.cond1.preheader ], [ %inc, %for.end19 ]
  %cmp6.not17 = icmp ult i64 %e.020, %conv516
  br i1 %cmp6.not17, label %for.end19, label %for.cond8.preheader.preheader

for.cond8.preheader.preheader:                    ; preds = %for.body3
  br label %for.cond8.preheader

for.cond8.preheader:                              ; preds = %for.cond8.preheader.preheader, %for.end
  %z.018 = phi i32 [ %add18, %for.end ], [ %conv, %for.cond8.preheader.preheader ]
  %cmp1012 = icmp ult i32 %z.018, 9
  br i1 %cmp1012, label %for.body11.lr.ph, label %for.end

for.body11.lr.ph:                                 ; preds = %for.cond8.preheader
  %k.promoted = load i8, i8* @k, align 1, !tbaa !3
  %.promoted = load i8, i8* getelementptr inbounds ([20 x i8], [20 x i8]* @i, i64 0, i64 0), align 16, !tbaa !6
  br label %for.body11

for.body11:                                       ; preds = %for.body11.lr.ph, %for.body11
  %add15 = phi i8 [ %.promoted, %for.body11.lr.ph ], [ %add, %for.body11 ]
  %conv1314 = phi i8 [ %k.promoted, %for.body11.lr.ph ], [ %conv13, %for.body11 ]
  %d.013 = phi i32 [ %z.018, %for.body11.lr.ph ], [ %add16, %for.body11 ]
  %conv9 = zext i32 %d.013 to i64
  %arrayidx = getelementptr inbounds [20 x i32], [20 x i32]* @p, i64 0, i64 %conv9, !intel-tbaa !8
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !8
  %1 = trunc i32 %0 to i8
  %conv13 = or i8 %conv1314, %1
  %add = add i8 %add15, 3
  %add16 = add nuw nsw i32 %d.013, 5
  %cmp10 = icmp ult i32 %d.013, 4
  br i1 %cmp10, label %for.body11, label %for.cond8.for.end_crit_edge, !llvm.loop !11

for.cond8.for.end_crit_edge:                      ; preds = %for.body11
  %conv13.lcssa = phi i8 [ %conv13, %for.body11 ]
  %add.lcssa = phi i8 [ %add, %for.body11 ]
  store i8 %conv13.lcssa, i8* @k, align 1, !tbaa !3
  store i8 %add.lcssa, i8* getelementptr inbounds ([20 x i8], [20 x i8]* @i, i64 0, i64 0), align 16, !tbaa !6
  br label %for.end

for.end:                                          ; preds = %for.cond8.for.end_crit_edge, %for.cond8.preheader
  %add18 = add i32 %z.018, 4
  %conv5 = zext i32 %add18 to i64
  %cmp6.not = icmp ult i64 %e.020, %conv5
  br i1 %cmp6.not, label %for.end19.loopexit, label %for.cond8.preheader, !llvm.loop !13

for.end19.loopexit:                               ; preds = %for.end
  br label %for.end19

for.end19:                                        ; preds = %for.end19.loopexit, %for.body3
  %inc = add nuw i64 %e.020, 1
  %cmp2 = icmp ult i64 %e.020, 5
  br i1 %cmp2, label %for.body3, label %for.end21, !llvm.loop !14

for.end21:                                        ; preds = %for.end19
  %add23 = add i64 %storemerge21, 9
  %cmp = icmp eq i64 %add23, 0
  br i1 %cmp, label %for.cond1.preheader, label %for.end24, !llvm.loop !15

for.end24:                                        ; preds = %for.end21
  %add23.lcssa = phi i64 [ %add23, %for.end21 ]
  store i64 %add23.lcssa, i64* @q, align 8, !tbaa !16
  %2 = load i8, i8* getelementptr inbounds ([20 x i8], [20 x i8]* @i, i64 0, i64 0), align 16, !tbaa !6
  %conv25 = zext i8 %2 to i32
  %call = tail call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %conv25)
  ret i32 0
}

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @printf(i8* nocapture noundef readonly, ...) local_unnamed_addr #1

attributes #0 = { nofree norecurse nounwind uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nofree nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !4, i64 0}
!7 = !{!"array@_ZTSA20_h", !4, i64 0}
!8 = !{!9, !10, i64 0}
!9 = !{!"array@_ZTSA20_j", !10, i64 0}
!10 = !{!"int", !4, i64 0}
!11 = distinct !{!11, !12}
!12 = !{!"llvm.loop.mustprogress"}
!13 = distinct !{!13, !12}
!14 = distinct !{!14, !12}
!15 = distinct !{!15, !12}
!16 = !{!17, !17, i64 0}
!17 = !{!"long", !4, i64 0}
