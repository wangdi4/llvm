

; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-dd-analysis>,hir-loop-collapse,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s

; Test checks that we correctly maintain cross-iteration
; dependancy during loop collapsing.

;*** IR Dump After HIRTempCleanupPass ***
;<0>          BEGIN REGION { }
;<23>               + DO i1 = 0, 7, 1   <DO_LOOP> <nounroll>
;<24>               |   + DO i2 = 0, 3, 1   <DO_LOOP> <nounroll>
;<7>                |   |   %3 = (%array)[0][i1 + 1][i2];
;<10>               |   |   (%array)[0][i1][i2] = %3 + 1;
;<24>               |   + END LOOP
;<23>               + END LOOP
;<0>          END REGION

;*** IR Dump After HIRLoopCollapsePass ***
;<0>          BEGIN REGION { modified }
;<24>               + DO i1 = 0, 31, 1   <DO_LOOP> <nounroll>
;<7>                |   %3 = (%array)[0][1][i1];
;<10>               |   (%array)[0][0][i1] = %3 + 1;
;<24>               + END LOOP
;<0>          END REGION


; Before collapsing:
; CHECK: DD graph for function main:
; CHECK: 7:10 (%array)[0][i1 + 1][i2] --> (%array)[0][i1][i2] ANTI (< =) (1 0)

; After collapsing:
; CHECK: DD graph for function main:
; CHECK: 7:10 (%array)[0][1][i1] --> (%array)[0][0][i1] ANTI (=) (0)


source_filename = "test4.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@n = dso_local local_unnamed_addr global i32 0, align 4

; Function Attrs: argmemonly nofree noinline norecurse nosync nounwind writeonly uwtable
define dso_local void @initialize_2d_array(i64* nocapture noundef writeonly %data, i32 noundef %dim0, i32 noundef %dim1) local_unnamed_addr #0 {
entry:
  %cmp23 = icmp sgt i32 %dim0, 0
  br i1 %cmp23, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp221 = icmp sgt i32 %dim1, 0
  %0 = sext i32 %dim1 to i64
  %wide.trip.count30 = zext i32 %dim0 to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.cond.cleanup3
  %indvars.iv27 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next28, %for.cond.cleanup3 ]
  br i1 %cmp221, label %for.body4.lr.ph, label %for.cond.cleanup3

for.body4.lr.ph:                                  ; preds = %for.cond1.preheader
  %1 = mul nsw i64 %indvars.iv27, %0
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.cond.cleanup3.loopexit:                       ; preds = %for.body4
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3.loopexit, %for.cond1.preheader
  %indvars.iv.next28 = add nuw nsw i64 %indvars.iv27, 1
  %exitcond31.not = icmp eq i64 %indvars.iv.next28, %wide.trip.count30
  br i1 %exitcond31.not, label %for.cond.cleanup.loopexit, label %for.cond1.preheader, !llvm.loop !3

for.body4:                                        ; preds = %for.body4.lr.ph, %for.body4
  %indvars.iv = phi i64 [ 0, %for.body4.lr.ph ], [ %indvars.iv.next, %for.body4 ]
  %2 = add nsw i64 %indvars.iv, %1
  %3 = add nsw i64 %2, 1
  %arrayidx = getelementptr inbounds i64, i64* %data, i64 %2
  store i64 %3, i64* %arrayidx, align 8, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %0
  br i1 %exitcond.not, label %for.cond.cleanup3.loopexit, label %for.body4, !llvm.loop !10
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nofree nosync nounwind readonly uwtable
define dso_local i32 @main() local_unnamed_addr #2 {
entry:
  %array = alloca [9 x [4 x i64]], align 16
  %0 = bitcast [9 x [4 x i64]]* %array to i8*
  call void @llvm.lifetime.start.p0i8(i64 288, i8* nonnull %0) #3
  %arrayidx1 = getelementptr inbounds [9 x [4 x i64]], [9 x [4 x i64]]* %array, i64 0, i64 0, i64 0, !intel-tbaa !11
  call void @initialize_2d_array(i64* noundef nonnull %arrayidx1, i32 noundef 9, i32 noundef 4)
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %entry, %for.cond.cleanup4
  %indvars.iv32 = phi i64 [ 0, %entry ], [ %indvars.iv.next33, %for.cond.cleanup4 ]
  %indvars.iv.next33 = add nuw nsw i64 %indvars.iv32, 1
  br label %for.body5

for.cond.cleanup:                                 ; preds = %for.cond.cleanup4
  %1 = load i32, i32* @n, align 4, !tbaa !14
  %idxprom17 = sext i32 %1 to i64
  %arrayidx20 = getelementptr inbounds [9 x [4 x i64]], [9 x [4 x i64]]* %array, i64 0, i64 %idxprom17, i64 %idxprom17, !intel-tbaa !11
  %2 = load i64, i64* %arrayidx20, align 8, !tbaa !11
  %conv = trunc i64 %2 to i32
  call void @llvm.lifetime.end.p0i8(i64 288, i8* nonnull %0) #3
  ret i32 %conv

for.cond.cleanup4:                                ; preds = %for.body5
  %exitcond34.not = icmp eq i64 %indvars.iv.next33, 8
  br i1 %exitcond34.not, label %for.cond.cleanup, label %for.cond2.preheader, !llvm.loop !16

for.body5:                                        ; preds = %for.cond2.preheader, %for.body5
  %indvars.iv = phi i64 [ 0, %for.cond2.preheader ], [ %indvars.iv.next, %for.body5 ]
  %arrayidx8 = getelementptr inbounds [9 x [4 x i64]], [9 x [4 x i64]]* %array, i64 0, i64 %indvars.iv.next33, i64 %indvars.iv, !intel-tbaa !11
  %3 = load i64, i64* %arrayidx8, align 8, !tbaa !11
  %add9 = add i64 %3, 1
  %arrayidx13 = getelementptr inbounds [9 x [4 x i64]], [9 x [4 x i64]]* %array, i64 0, i64 %indvars.iv32, i64 %indvars.iv, !intel-tbaa !11
  store i64 %add9, i64* %arrayidx13, align 8, !tbaa !11
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond.not, label %for.cond.cleanup4, label %for.body5, !llvm.loop !17
}

attributes #0 = { argmemonly nofree noinline norecurse nosync nounwind writeonly uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly mustprogress nocallback nofree nosync nounwind willreturn }
attributes #2 = { nofree nosync nounwind readonly uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!3 = distinct !{!3, !4, !5}
!4 = !{!"llvm.loop.mustprogress"}
!5 = !{!"llvm.loop.unroll.disable"}
!6 = !{!7, !7, i64 0}
!7 = !{!"long", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = distinct !{!10, !4, !5}
!11 = !{!12, !7, i64 0}
!12 = !{!"array@_ZTSA9_A4_m", !13, i64 0}
!13 = !{!"array@_ZTSA4_m", !7, i64 0}
!14 = !{!15, !15, i64 0}
!15 = !{!"int", !8, i64 0}
!16 = distinct !{!16, !4, !5}
!17 = distinct !{!17, !4, !5}
