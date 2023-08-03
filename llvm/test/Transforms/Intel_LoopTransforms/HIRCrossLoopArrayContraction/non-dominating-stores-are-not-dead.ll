; RUN: opt -intel-libirc-allowed -hir-create-function-level-region -passes="hir-ssa-deconstruction,require<hir-loop-statistics>,hir-cross-loop-array-contraction,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check that DEF (%A)[0][i1][i2][i3][i4][i5] will not be removed.

; CHECK:  if (%n != 0)
; CHECK:  {
; CHECK:     + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:     |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:     |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
; CHECK:     |   |   |   + DO i4 = 0, 99, 1   <DO_LOOP>
; CHECK:     |   |   |   |   + DO i5 = 0, 99, 1   <DO_LOOP>
; CHECK:     |   |   |   |   |   (%A)[0][i1][i2][i3][i4][i5] = i4 + i5;
; CHECK:     |   |   |   |   + END LOOP
; CHECK:     |   |   |   + END LOOP
; CHECK:     |   |   + END LOOP
; CHECK:     |   + END LOOP
; CHECK:     + END LOOP
; CHECK:  }
;
; CHECK:  + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:  |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:  |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
; CHECK:  |   |   |   + DO i4 = 0, 99, 1   <DO_LOOP>
; CHECK:  |   |   |   |   + DO i5 = 0, 99, 1   <DO_LOOP>
; CHECK:  |   |   |   |   |   [[VAR:.*]] = (%A)[0][i1][i2][i3][i5][i4];
; CHECK:  |   |   |   |   |   (%B)[0][i1][i2][i3][i4][i5] = i5 + [[VAR]];
; CHECK:  |   |   |   |   + END LOOP
; CHECK:  |   |   |   + END LOOP
; CHECK:  |   |   + END LOOP
; CHECK:  |   + END LOOP
; CHECK:  + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind readnone uwtable
define dso_local i32 @shell(i32 %n) local_unnamed_addr #0 {
entry:
  %A = alloca [100 x [100 x [100 x [100 x [100 x i32]]]]], align 16
  %B = alloca [100 x [100 x [100 x [100 x [100 x i32]]]]], align 16
  call void @llvm.lifetime.start.p0(i64 40000000000, ptr nonnull %A) #3
  call void @llvm.lifetime.start.p0(i64 40000000000, ptr nonnull %B) #3
  %tobool.not = icmp eq i32 %n, 0
  br i1 %tobool.not, label %if.end, label %for.cond1.preheader.preheader

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.cond.cleanup3
  %indvars.iv176 = phi i64 [ %indvars.iv.next177, %for.cond.cleanup3 ], [ 0, %for.cond1.preheader.preheader ]
  br label %for.cond5.preheader

for.cond5.preheader:                              ; preds = %for.cond1.preheader, %for.cond.cleanup7
  %indvars.iv173 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next174, %for.cond.cleanup7 ]
  br label %for.cond9.preheader

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %indvars.iv.next177 = add nuw nsw i64 %indvars.iv176, 1
  %exitcond178.not = icmp eq i64 %indvars.iv.next177, 100
  br i1 %exitcond178.not, label %if.end.loopexit, label %for.cond1.preheader, !llvm.loop !2

for.cond9.preheader:                              ; preds = %for.cond5.preheader, %for.cond.cleanup11
  %indvars.iv170 = phi i64 [ 0, %for.cond5.preheader ], [ %indvars.iv.next171, %for.cond.cleanup11 ]
  br label %for.cond13.preheader

for.cond.cleanup7:                                ; preds = %for.cond.cleanup11
  %indvars.iv.next174 = add nuw nsw i64 %indvars.iv173, 1
  %exitcond175.not = icmp eq i64 %indvars.iv.next174, 100
  br i1 %exitcond175.not, label %for.cond.cleanup3, label %for.cond5.preheader, !llvm.loop !4

for.cond13.preheader:                             ; preds = %for.cond9.preheader, %for.cond.cleanup15
  %indvars.iv167 = phi i64 [ 0, %for.cond9.preheader ], [ %indvars.iv.next168, %for.cond.cleanup15 ]
  br label %for.body16

for.cond.cleanup11:                               ; preds = %for.cond.cleanup15
  %indvars.iv.next171 = add nuw nsw i64 %indvars.iv170, 1
  %exitcond172.not = icmp eq i64 %indvars.iv.next171, 100
  br i1 %exitcond172.not, label %for.cond.cleanup7, label %for.cond9.preheader, !llvm.loop !5

for.cond.cleanup15:                               ; preds = %for.body16
  %indvars.iv.next168 = add nuw nsw i64 %indvars.iv167, 1
  %exitcond169.not = icmp eq i64 %indvars.iv.next168, 100
  br i1 %exitcond169.not, label %for.cond.cleanup11, label %for.cond13.preheader, !llvm.loop !6

for.body16:                                       ; preds = %for.cond13.preheader, %for.body16
  %indvars.iv163 = phi i64 [ 0, %for.cond13.preheader ], [ %indvars.iv.next164, %for.body16 ]
  %0 = add nuw nsw i64 %indvars.iv163, %indvars.iv167
  %arrayidx24 = getelementptr inbounds [100 x [100 x [100 x [100 x [100 x i32]]]]], ptr %A, i64 0, i64 %indvars.iv176, i64 %indvars.iv173, i64 %indvars.iv170, i64 %indvars.iv167, i64 %indvars.iv163, !intel-tbaa !7
  %1 = trunc i64 %0 to i32
  store i32 %1, ptr %arrayidx24, align 4, !tbaa !7
  %indvars.iv.next164 = add nuw nsw i64 %indvars.iv163, 1
  %exitcond166.not = icmp eq i64 %indvars.iv.next164, 100
  br i1 %exitcond166.not, label %for.cond.cleanup15, label %for.body16, !llvm.loop !16

if.end.loopexit:                                  ; preds = %for.cond.cleanup3
  br label %if.end

if.end:                                           ; preds = %if.end.loopexit, %entry
  br label %for.cond43.preheader

for.cond43.preheader:                             ; preds = %if.end, %for.cond.cleanup45
  %indvars.iv160 = phi i64 [ 0, %if.end ], [ %indvars.iv.next161, %for.cond.cleanup45 ]
  br label %for.cond48.preheader

for.cond.cleanup40:                               ; preds = %for.cond.cleanup45
  %arrayidx102 = getelementptr inbounds [100 x [100 x [100 x [100 x [100 x i32]]]]], ptr %B, i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, !intel-tbaa !7
  %2 = load i32, ptr %arrayidx102, align 4, !tbaa !7
  call void @llvm.lifetime.end.p0(i64 40000000000, ptr nonnull %B) #3
  call void @llvm.lifetime.end.p0(i64 40000000000, ptr nonnull %A) #3
  ret i32 %2

for.cond48.preheader:                             ; preds = %for.cond43.preheader, %for.cond.cleanup50
  %indvars.iv157 = phi i64 [ 0, %for.cond43.preheader ], [ %indvars.iv.next158, %for.cond.cleanup50 ]
  br label %for.cond53.preheader

for.cond.cleanup45:                               ; preds = %for.cond.cleanup50
  %indvars.iv.next161 = add nuw nsw i64 %indvars.iv160, 1
  %exitcond162.not = icmp eq i64 %indvars.iv.next161, 100
  br i1 %exitcond162.not, label %for.cond.cleanup40, label %for.cond43.preheader, !llvm.loop !17

for.cond53.preheader:                             ; preds = %for.cond48.preheader, %for.cond.cleanup55
  %indvars.iv154 = phi i64 [ 0, %for.cond48.preheader ], [ %indvars.iv.next155, %for.cond.cleanup55 ]
  br label %for.cond58.preheader

for.cond.cleanup50:                               ; preds = %for.cond.cleanup55
  %indvars.iv.next158 = add nuw nsw i64 %indvars.iv157, 1
  %exitcond159.not = icmp eq i64 %indvars.iv.next158, 100
  br i1 %exitcond159.not, label %for.cond.cleanup45, label %for.cond48.preheader, !llvm.loop !18

for.cond58.preheader:                             ; preds = %for.cond53.preheader, %for.cond.cleanup60
  %indvars.iv151 = phi i64 [ 0, %for.cond53.preheader ], [ %indvars.iv.next152, %for.cond.cleanup60 ]
  br label %for.body61

for.cond.cleanup55:                               ; preds = %for.cond.cleanup60
  %indvars.iv.next155 = add nuw nsw i64 %indvars.iv154, 1
  %exitcond156.not = icmp eq i64 %indvars.iv.next155, 100
  br i1 %exitcond156.not, label %for.cond.cleanup50, label %for.cond53.preheader, !llvm.loop !19

for.cond.cleanup60:                               ; preds = %for.body61
  %indvars.iv.next152 = add nuw nsw i64 %indvars.iv151, 1
  %exitcond153.not = icmp eq i64 %indvars.iv.next152, 100
  br i1 %exitcond153.not, label %for.cond.cleanup55, label %for.cond58.preheader, !llvm.loop !20

for.body61:                                       ; preds = %for.cond58.preheader, %for.body61
  %indvars.iv = phi i64 [ 0, %for.cond58.preheader ], [ %indvars.iv.next, %for.body61 ]
  %arrayidx71 = getelementptr inbounds [100 x [100 x [100 x [100 x [100 x i32]]]]], ptr %A, i64 0, i64 %indvars.iv160, i64 %indvars.iv157, i64 %indvars.iv154, i64 %indvars.iv, i64 %indvars.iv151, !intel-tbaa !7
  %3 = load i32, ptr %arrayidx71, align 4, !tbaa !7
  %4 = trunc i64 %indvars.iv to i32
  %add72 = add nsw i32 %3, %4
  %arrayidx82 = getelementptr inbounds [100 x [100 x [100 x [100 x [100 x i32]]]]], ptr %B, i64 0, i64 %indvars.iv160, i64 %indvars.iv157, i64 %indvars.iv154, i64 %indvars.iv151, i64 %indvars.iv, !intel-tbaa !7
  store i32 %add72, ptr %arrayidx82, align 4, !tbaa !7
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond.cleanup60, label %for.body61, !llvm.loop !21
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind readnone uwtable
define dso_local i32 @main() local_unnamed_addr #2 {
entry:
  %call = tail call i32 @shell(i32 1)
  ret i32 %call
}

attributes #0 = { noinline nounwind readnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind readnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.mustprogress"}
!4 = distinct !{!4, !3}
!5 = distinct !{!5, !3}
!6 = distinct !{!6, !3}
!7 = !{!8, !13, i64 0}
!8 = !{!"array@_ZTSA100_A100_A100_A100_A100_i", !9, i64 0}
!9 = !{!"array@_ZTSA100_A100_A100_A100_i", !10, i64 0}
!10 = !{!"array@_ZTSA100_A100_A100_i", !11, i64 0}
!11 = !{!"array@_ZTSA100_A100_i", !12, i64 0}
!12 = !{!"array@_ZTSA100_i", !13, i64 0}
!13 = !{!"int", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C/C++ TBAA"}
!16 = distinct !{!16, !3}
!17 = distinct !{!17, !3}
!18 = distinct !{!18, !3}
!19 = distinct !{!19, !3}
!20 = distinct !{!20, !3}
!21 = distinct !{!21, !3}
