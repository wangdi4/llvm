; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-post-vec-complete-unroll" -print-before=hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll -hir-details 2>&1 < %s | FileCheck %s

; Verify that the vector cast zext.<16 x i32>.<16 x i64>(i1) is correctly
; simplified when substituting IV with constant in complete unroll.

; Before the fix, src type was incorrectly being set to scalar i64 type during
; simplification.

; Incoming vectorized HIR-
; + DO i1 = 0, 15, 1   <DO_LOOP>
; |   %3 = (@a)[0];
; |   %.vec = bitcast.<16 x i32*>.<16 x i8*>(%e.linear.iv);
; |   %.unifload = (i32*)(%c)[0][i1][0];
; |   (<16 x i32>*)(%c)[0][<i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>][i1] = %.unifload;
; |   %.vec2 = bitcast.<16 x i32*>.<16 x i8*>(%e.linear.iv);
; |   %4 = extractelement %.unifload,  15;
; |   (%3)[0] = %4;
; + END LOOP

; CHECK: Dump Before

; CHECK: (<16 x i32>*)(LINEAR ptr %c)[<16 x i64> 0][zext.<16 x i32>.<16 x i64>(<i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>)][LINEAR zext.<16 x i32>.<16 x i64>(i1)]

; CHECK: Dump After

; CHECK: (<16 x i32>*)(LINEAR ptr %c)[<16 x i64> 0][zext.<16 x i32>.<16 x i64>(<i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>)][<16 x i64> 0]


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global ptr null, align 8

; Function Attrs: nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  %e.linear.iv = alloca i32, align 4
  %c = alloca [16 x [16 x i32]], align 16
  call void @llvm.lifetime.start.p0(i64 1024, ptr nonnull %c) #2
  br label %DIR.OMP.SIMD.124

for.cond.cleanup:                                 ; preds = %DIR.OMP.END.SIMD.4
  call void @llvm.lifetime.end.p0(i64 1024, ptr nonnull %c) #2
  ret void

DIR.OMP.SIMD.124:                                 ; preds = %entry, %DIR.OMP.END.SIMD.4
  %d.022 = phi i32 [ 0, %entry ], [ %inc, %DIR.OMP.END.SIMD.4 ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.124
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %e.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %idxprom = zext i32 %d.022 to i64
  %arrayidx2 = getelementptr inbounds [16 x [16 x i32]], ptr %c, i64 0, i64 %idxprom, i64 0, !intel-tbaa !2
  %1 = load ptr, ptr @a, align 8, !tbaa !8, !llvm.access.group !10
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %.omp.iv.local.018 = phi i32 [ 0, %DIR.OMP.SIMD.2 ], [ %add7, %omp.inner.for.body ]
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %e.linear.iv) #2, !llvm.access.group !10
  %2 = load i32, ptr %arrayidx2, align 16, !tbaa !2, !llvm.access.group !10
  %idxprom3 = zext i32 %.omp.iv.local.018 to i64
  %arrayidx6 = getelementptr inbounds [16 x [16 x i32]], ptr %c, i64 0, i64 %idxprom3, i64 %idxprom, !intel-tbaa !2
  store i32 %2, ptr %arrayidx6, align 4, !tbaa !2, !llvm.access.group !10
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %e.linear.iv) #2, !llvm.access.group !10
  %add7 = add nuw nsw i32 %.omp.iv.local.018, 1
  %exitcond.not = icmp eq i32 %add7, 16
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body, !llvm.loop !11

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  %.lcssa = phi i32 [ %2, %omp.inner.for.body ]
  store i32 %.lcssa, ptr %1, align 4, !tbaa !14
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.END.SIMD.3
  %inc = add nuw nsw i32 %d.022, 1
  %exitcond23.not = icmp eq i32 %inc, 16
  br i1 %exitcond23.not, label %for.cond.cleanup, label %DIR.OMP.SIMD.124, !llvm.loop !15
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA16_A16_i", !4, i64 0}
!4 = !{!"array@_ZTSA16_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !9, i64 0}
!9 = !{!"pointer@_ZTSPi", !6, i64 0}
!10 = distinct !{}
!11 = distinct !{!11, !12, !13}
!12 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
!13 = !{!"llvm.loop.parallel_accesses", !10}
!14 = !{!5, !5, i64 0}
!15 = distinct !{!15, !16}
!16 = !{!"llvm.loop.mustprogress"}
