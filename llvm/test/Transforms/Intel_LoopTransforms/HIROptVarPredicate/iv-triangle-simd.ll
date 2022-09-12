; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-opt-var-predicate -hir-opt-var-predicate-bypass-simd=0 -disable-hir-opt-var-predicate-cost-model -disable-output -print-before=hir-opt-var-predicate -print-after=hir-opt-var-predicate %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-var-predicate" -hir-opt-var-predicate-bypass-simd=0 -disable-hir-opt-var-predicate-cost-model -disable-output -print-before=hir-opt-var-predicate -print-after=hir-opt-var-predicate %s 2>&1 | FileCheck %s

; Verify that reduction-related insts are cloned together with simd intrinsics when opt-var-predicate hanppened.

; CHECK: Function: _Z3fooPKfPfll
;
; CHECK:          BEGIN REGION { }
; CHECK:                + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK:                |   (%a1.red)[0] = 0.000000e+00;
; CHECK:                |   %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.REDUCTION.ADD(&((%a1.red)[0])),  QUAL.OMP.LINEAR:IV(&((%j.linear.iv)[0])1),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null),  QUAL.OMP.LIVEIN(null),  QUAL.OMP.LIVEIN:F90_DV(null) ]
; CHECK:                |   %1 = (%x)[i1];
; CHECK:                |   %conv14 = fpext.float.double(%1);
; CHECK:                |   %mul12 = %conv14  *  3.140000e+00;
; CHECK:                |   %2 = (%a1.red)[0];
;                       |
; CHECK:                |   + DO i2 = 0, %n + -1, 1   <DO_LOOP> <simd> <ivdep>
; CHECK:                |   |   %sub7 = %1  -  (%x)[i2];
; CHECK:                |   |   %storemerge = 0.000000e+00;
; CHECK:                |   |   if (i2 != i1 + %k)
; CHECK:                |   |   {
; CHECK:                |   |      %mul8 = %sub7  *  %sub7;
; CHECK:                |   |      %add9 = %mul8  +  %sub7;
; CHECK:                |   |      %4 = @llvm.sqrt.f32(%add9);
; CHECK:                |   |      %conv = fpext.float.double(%4);
; CHECK:                |   |      %mul15 = %mul12  *  %conv;
; CHECK:                |   |      %conv16 = fptrunc.double.float(%mul15);
; CHECK:                |   |      %storemerge = %conv16;
; CHECK:                |   |   }
; CHECK:                |   |   %mul17 = %storemerge  *  %sub7;
; CHECK:                |   |   %2 = %2  +  %mul17;
; CHECK:                |   + END LOOP
;                       |
; CHECK:                |   (%j.linear.iv)[0] = %n;
; CHECK:                |   (%a1.red)[0] = %2;
; CHECK:                |   @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; CHECK:                |   %5 = %2  +  0.000000e+00;
; CHECK:                |   (%y)[i1] = %5;
; CHECK:                + END LOOP
; CHECK:          END REGION

;CHECK:  Function: _Z3fooPKfPfll
;
;CHECK:          BEGIN REGION { modified }
;CHECK:                + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;CHECK:                |   (%a1.red)[0] = 0.000000e+00;
;CHECK:                |   %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.REDUCTION.ADD(&((%a1.red)[0])),  QUAL.OMP.LINEAR:IV(&((%j.linear.iv)[0])1),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null),  QUAL.OMP.LIVEIN(null),  QUAL.OMP.LIVEIN:F90_DV(null) ]
;CHECK:                |   %1 = (%x)[i1];
;CHECK:                |   %conv14 = fpext.float.double(%1);
;CHECK:                |   %mul12 = %conv14  *  3.140000e+00;
;CHECK:                |   %2 = (%a1.red)[0];
;CHECK:                |   %ivcopy = i1 + %k;
;                      |
;CHECK:                |   + DO i2 = 0, smin((-1 + %n), (-1 + %ivcopy)), 1   <DO_LOOP> <simd> <ivdep>
;CHECK:                |   |   %sub7 = %1  -  (%x)[i2];
;CHECK:                |   |   %storemerge = 0.000000e+00;
;CHECK:                |   |   %mul8 = %sub7  *  %sub7;
;CHECK:                |   |   %add9 = %mul8  +  %sub7;
;CHECK:                |   |   %4 = @llvm.sqrt.f32(%add9);
;CHECK:                |   |   %conv = fpext.float.double(%4);
;CHECK:                |   |   %mul15 = %mul12  *  %conv;
;CHECK:                |   |   %conv16 = fptrunc.double.float(%mul15);
;CHECK:                |   |   %storemerge = %conv16;
;CHECK:                |   |   %mul17 = %storemerge  *  %sub7;
;CHECK:                |   |   %2 = %2  +  %mul17;
;CHECK:                |   + END LOOP
;     :                |
;CHECK:                |   (%a1.red)[0] = %2;
;CHECK:                |   @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]

;CHECK:                |   if (smax(0, %ivcopy) < smin((-1 + %n), %ivcopy) + 1)
;CHECK:                |    {
;CHECK:                |       %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.REDUCTION.ADD(&((%a1.red)[0])),  QUAL.OMP.LINEAR:IV(&((%j.linear.iv)[0])1),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null), QUAL.OMP.LIVEIN(null), QUAL.OMP.LIVEIN:F90_DV(null) ]
;CHECK:                |       %2 = (%a1.red)[0];

;CHECK:                |       %sub7 = %1  -  (%x)[smax(0, %ivcopy)];
;CHECK:                |       %storemerge = 0.000000e+00;
;CHECK:                |       %mul17 = %storemerge  *  %sub7;
;CHECK:                |       %2 = %2  +  %mul17;

;CHECK:                |       (%a1.red)[0] = %2;
;CHECK:                |       @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
;CHECK:                |   }

;CHECK:                |   %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.REDUCTION.ADD(&((%a1.red)[0])),  QUAL.OMP.LINEAR:IV(&((%j.linear.iv)[0])1),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null), QUAL.OMP.LIVEIN(null), QUAL.OMP.LIVEIN:F90_DV(null) ]
;CHECK:                |   %2 = (%a1.red)[0];
;     :                |
;CHECK:                |   + DO i2 = 0, %n + -1 * smax(0, (1 + %ivcopy)) + -1, 1   <DO_LOOP> <simd> <ivdep>
;CHECK:                |   |   %sub7 = %1  -  (%x)[i2 + smax(0, (1 + %ivcopy))];
;CHECK:                |   |   %storemerge = 0.000000e+00;
;CHECK:                |   |   %mul8 = %sub7  *  %sub7;
;CHECK:                |   |   %add9 = %mul8  +  %sub7;
;CHECK:                |   |   %4 = @llvm.sqrt.f32(%add9);
;CHECK:                |   |   %conv = fpext.float.double(%4);
;CHECK:                |   |   %mul15 = %mul12  *  %conv;
;CHECK:                |   |   %conv16 = fptrunc.double.float(%mul15);
;CHECK:                |   |   %storemerge = %conv16;
;CHECK:                |   |   %mul17 = %storemerge  *  %sub7;
;CHECK:                |   |   %2 = %2  +  %mul17;
;CHECK:                |   + END LOOP
;     :                |
;CHECK:                |   (%j.linear.iv)[0] = %n;
;CHECK:                |   (%a1.red)[0] = %2;
;CHECK:                |   @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
;CHECK:                |   %5 = %2  +  0.000000e+00;
;CHECK:                |   (%y)[i1] = %5;
;CHECK:                + END LOOP
;CHECK:          END REGION

;Module Before HIR
; ModuleID = 'test.cpp'
source_filename = "test.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z3fooPKfPfll(float* noalias nocapture readonly %x, float* noalias nocapture writeonly %y, i64 %n, i64 %k) local_unnamed_addr #0 {
entry:
  %a1.red = alloca float, align 4
  %j.linear.iv = alloca i64, align 8
  %cmp3 = icmp sgt i64 %n, 0
  br i1 %cmp3, label %DIR.OMP.SIMD.1.preheader, label %for.end

DIR.OMP.SIMD.1.preheader:                         ; preds = %entry
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.1.preheader, %DIR.OMP.END.SIMD.4
  %i.058 = phi i64 [ %inc, %DIR.OMP.END.SIMD.4 ], [ 0, %DIR.OMP.SIMD.1.preheader ]
  store float 0.000000e+00, float* %a1.red, align 4
  br label %DIR.OMP.SIMD.162

DIR.OMP.SIMD.162:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD"(float* %a1.red), "QUAL.OMP.LINEAR:IV"(i64* %j.linear.iv, i32 1), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LIVEIN"(i8* null), "QUAL.OMP.LIVEIN:F90_DV"(i8* null) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.162
  %arrayidx = getelementptr inbounds float, float* %x, i64 %i.058
  %1 = load float, float* %arrayidx, align 4, !tbaa !4, !llvm.access.group !8
  %add10 = add nsw i64 %i.058, %k
  %conv14 = fpext float %1 to double
  %mul12 = fmul fast double %conv14, 3.140000e+00
  %a1.red.promoted = load float, float* %a1.red, align 4, !tbaa !4
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %if.end
  %2 = phi float [ %a1.red.promoted, %DIR.OMP.SIMD.2 ], [ %add18, %if.end ]
  %.omp.iv.local.044 = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %add19, %if.end ]
  %arrayidx6 = getelementptr inbounds float, float* %x, i64 %.omp.iv.local.044
  %3 = load float, float* %arrayidx6, align 4, !tbaa !4, !llvm.access.group !8
  %sub7 = fsub fast float %1, %3
  %cmp11.not = icmp eq i64 %.omp.iv.local.044, %add10
  br i1 %cmp11.not, label %if.end, label %if.then

if.then:                                          ; preds = %omp.inner.for.body
  %mul8 = fmul fast float %sub7, %sub7
  %add9 = fadd fast float %mul8, %sub7
  %4 = call fast float @llvm.sqrt.f32(float %add9) #1
  %conv = fpext float %4 to double
  %mul15 = fmul fast double %mul12, %conv
  %conv16 = fptrunc double %mul15 to float
  br label %if.end

if.end:                                           ; preds = %omp.inner.for.body, %if.then
  %storemerge = phi float [ %conv16, %if.then ], [ 0.000000e+00, %omp.inner.for.body ]
  %mul17 = fmul fast float %storemerge, %sub7
  %add18 = fadd fast float %2, %mul17
  %add19 = add nuw nsw i64 %.omp.iv.local.044, 1
  %exitcond.not = icmp eq i64 %add19, %n
  br i1 %exitcond.not, label %omp.precond.end, label %omp.inner.for.body, !llvm.loop !9

omp.precond.end:                                  ; preds = %if.end
  %add18.lcssa = phi float [ %add18, %if.end ]
  store i64 %n, i64* %j.linear.iv, align 8, !tbaa !13
  store float %add18.lcssa, float* %a1.red, align 4, !tbaa !4
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %omp.precond.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.END.SIMD.3
  %5 = fadd float %add18.lcssa, 0.000000e+00
  %arrayidx21 = getelementptr inbounds float, float* %y, i64 %i.058
  store float %5, float* %arrayidx21, align 4, !tbaa !4
  %inc = add nuw nsw i64 %i.058, 1
  %exitcond60.not = icmp eq i64 %inc, %n
  br i1 %exitcond60.not, label %for.end.loopexit, label %DIR.OMP.SIMD.1, !llvm.loop !15

for.end.loopexit:                                 ; preds = %DIR.OMP.END.SIMD.4
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: mustprogress nofree nosync nounwind readnone speculatable willreturn
declare float @llvm.sqrt.f32(float) #2

attributes #0 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { mustprogress nofree nosync nounwind readnone speculatable willreturn }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}
!nvvm.annotations = !{}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!4 = !{!5, !5, i64 0}
!5 = !{!"float", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = distinct !{}
!9 = distinct !{!9, !11, !12}
!11 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
!12 = !{!"llvm.loop.parallel_accesses", !8}
!13 = !{!14, !14, i64 0}
!14 = !{!"long", !6, i64 0}
!15 = distinct !{!15, !16}
!16 = !{!"llvm.loop.mustprogress"}
