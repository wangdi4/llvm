
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-dd-analysis>,hir-loop-collapse,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s

; Test checks that we correctly maintain cross-iteration
; dependency during loop collapsing.

;*** IR Dump After HIRTempCleanupPass ***
;<0>          BEGIN REGION { }
;<26>               + DO i1 = 0, 7, 1   <DO_LOOP> <nounroll>
;<27>               |   + DO i2 = 0, 3, 1   <DO_LOOP> <nounroll>
;<9>                |   |   (%array)[0][i1 + 1][i2] = (%b)[0][i1][i2];
;<11>               |   |   %5 = (%array)[0][i1][i2];
;<13>               |   |   (%b)[0][i1][i2] = %5 + 1;
;<27>               |   + END LOOP
;<26>               + END LOOP
;<0>          END REGION

;*** IR Dump After HIRLoopCollapsePass ***
;<0>          BEGIN REGION { modified }
;<27>               + DO i1 = 0, 31, 1   <DO_LOOP> <nounroll>
;<9>                |   (%array)[0][1][i1] = (%b)[0][0][i1];
;<11>               |   %5 = (%array)[0][0][i1];
;<13>               |   (%b)[0][0][i1] = %5 + 1;
;<27>               + END LOOP
;<0>          END REGION


; Before collapsing:
; CHECK-LABEL: DD graph for function main:
; CHECK-DAG: 7:13 (%b)[0][i1][i2] --> (%b)[0][i1][i2] ANTI (= =) (0 0)
; CHECK-DAG: 9:11 (%array)[0][i1 + 1][i2] --> (%array)[0][i1][i2] FLOW (< =) (1 0)

; After collapsing:
; CHECK-LABEL: DD graph for function main:
; CHECK-DAG: 7:13 (%b)[0][0][i1] --> (%b)[0][0][i1] ANTI (=) (0)
; CHECK-DAG: 9:11 (%array)[0][1][i1] --> (%array)[0][0][i1] FLOW (=) (0)


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@n = dso_local local_unnamed_addr global i32 0, align 4

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nofree nosync nounwind readonly uwtable
define dso_local i32 @main() local_unnamed_addr #2 {
entry:
  %array = alloca [9 x [4 x i64]], align 16
  %b = alloca [9 x [4 x i64]], align 16
  %0 = bitcast ptr %array to ptr
  call void @llvm.lifetime.start.p0(i64 288, ptr nonnull %0) #3
  %1 = bitcast ptr %b to ptr
  call void @llvm.lifetime.start.p0(i64 288, ptr nonnull %1) #3
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %entry, %for.cond.cleanup4
  %indvars.iv44 = phi i64 [ 0, %entry ], [ %indvars.iv.next45, %for.cond.cleanup4 ]
  %indvars.iv.next45 = add nuw nsw i64 %indvars.iv44, 1
  br label %for.body5

for.cond.cleanup:                                 ; preds = %for.cond.cleanup4
  %2 = load i32, ptr @n, align 4, !tbaa !14
  %idxprom25 = sext i32 %2 to i64
  %arrayidx28 = getelementptr inbounds [9 x [4 x i64]], ptr %array, i64 0, i64 %idxprom25, i64 %idxprom25, !intel-tbaa !11
  %3 = load i64, ptr %arrayidx28, align 8, !tbaa !11
  %conv = trunc i64 %3 to i32
  call void @llvm.lifetime.end.p0(i64 288, ptr nonnull %1) #3
  call void @llvm.lifetime.end.p0(i64 288, ptr nonnull %0) #3
  ret i32 %conv

for.cond.cleanup4:                                ; preds = %for.body5
  %exitcond46.not = icmp eq i64 %indvars.iv.next45, 8
  br i1 %exitcond46.not, label %for.cond.cleanup, label %for.cond2.preheader, !llvm.loop !16

for.body5:                                        ; preds = %for.cond2.preheader, %for.body5
  %indvars.iv = phi i64 [ 0, %for.cond2.preheader ], [ %indvars.iv.next, %for.body5 ]
  %arrayidx8 = getelementptr inbounds [9 x [4 x i64]], ptr %b, i64 0, i64 %indvars.iv44, i64 %indvars.iv, !intel-tbaa !11
  %4 = load i64, ptr %arrayidx8, align 8, !tbaa !11
  %arrayidx12 = getelementptr inbounds [9 x [4 x i64]], ptr %array, i64 0, i64 %indvars.iv.next45, i64 %indvars.iv, !intel-tbaa !11
  store i64 %4, ptr %arrayidx12, align 8, !tbaa !11
  %arrayidx16 = getelementptr inbounds [9 x [4 x i64]], ptr %array, i64 0, i64 %indvars.iv44, i64 %indvars.iv, !intel-tbaa !11
  %5 = load i64, ptr %arrayidx16, align 8, !tbaa !11
  %add17 = add i64 %5, 1
  store i64 %add17, ptr %arrayidx8, align 8, !tbaa !11
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
