
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-dd-analysis>,hir-loop-collapse,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s

; Test checks that we correctly maintain cross-iteration
; dependancy during loop collapsing. In this case distance
; is conservatively set to inner loop TC.

; Before collapsing
;<0>          BEGIN REGION { }
;<24>               + DO i1 = 0, 7, 1   <DO_LOOP> <nounroll>
;<25>               |   + DO i2 = 0, 3, 1   <DO_LOOP> <nounroll>
;<7>                |   |   %3 = (%array)[0][i1][i2];
;<11>               |   |   (%array)[0][i1 + 2][i2] = %3 + 1;
;<25>               |   + END LOOP
;<24>               + END LOOP
;<0>          END REGION

; After collapsing
;<0>          BEGIN REGION { modified }
;<25>               + DO i1 = 0, 31, 1   <DO_LOOP> <nounroll>
;<7>                |   %3 = (%array)[0][0][i1];
;<11>               |   (%array)[0][2][i1] = %3 + 1;
;<25>               + END LOOP
;<0>          END REGION


; Before collapsing:
; CHECK: DD graph for function main:
; CHECK: 11:7 (%array)[0][i1 + 2][i2] --> (%array)[0][i1][i2] FLOW (< =) (2 0)

; After collapsing:
; CHECK: DD graph for function main:
; CHECK: 11:7 (%array)[0][2][i1] --> (%array)[0][0][i1] FLOW (<) (4)


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@n = dso_local local_unnamed_addr global i32 0, align 4

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
  %arrayidx8 = getelementptr inbounds [9 x [4 x i64]], [9 x [4 x i64]]* %array, i64 0, i64 %indvars.iv32, i64 %indvars.iv, !intel-tbaa !11
  %3 = load i64, i64* %arrayidx8, align 8, !tbaa !11
  %add = add i64 %3, 1
  %add2 = add nuw nsw i64 %indvars.iv32, 2
  %arrayidx13 = getelementptr inbounds [9 x [4 x i64]], [9 x [4 x i64]]* %array, i64 0, i64 %add2, i64 %indvars.iv, !intel-tbaa !11
  store i64 %add, i64* %arrayidx13, align 8, !tbaa !11
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
