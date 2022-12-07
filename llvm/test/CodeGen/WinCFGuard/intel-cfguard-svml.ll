
; Check that placeholder SVML functions are not listed as potential targets of indirect calls

; RUN: llc < %s | FileCheck %s
; CHECK-NOT: .section .gfids$y

; The IR is produced with the source and command line:
; icx -O3 test.c  /guard:cf
;
; #include <math.h>
;
; void f(float *in, float *out, int n) {
;   for (int i = 0; i < n; i++)
;     out[i] = log2(in[i]);
; }

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30146"

@llvm.compiler.used = appending global [6 x i8*] [i8* bitcast (<2 x double> (<2 x double>)* @__svml_log22 to i8*), i8* bitcast (<4 x double> (<4 x double>)* @__svml_log24 to i8*), i8* bitcast (<8 x double> (<8 x double>)* @__svml_log28 to i8*), i8* bitcast (<16 x double> (<16 x double>)* @__svml_log216 to i8*), i8* bitcast (<32 x double> (<32 x double>)* @__svml_log232 to i8*), i8* bitcast (<64 x double> (<64 x double>)* @__svml_log264 to i8*)], section "llvm.metadata"
@__guard_dispatch_icall_fptr = external dso_local global void (i8*)*

define dso_local void @f(float* nocapture noundef readonly %in, float* nocapture noundef writeonly %out, i32 noundef %n) local_unnamed_addr #0 {
entry:
  %cmp8 = icmp sgt i32 %n, 0
  br i1 %cmp8, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  %0 = add nsw i64 %wide.trip.count, -1
  %1 = getelementptr inbounds float, float* %in, i64 %0
  %hir.cmp.21 = icmp ult float* %1, %out
  %2 = getelementptr inbounds float, float* %out, i64 %0
  %hir.cmp.22 = icmp ult float* %2, %in
  %hir.cmp.24 = or i1 %hir.cmp.21, %hir.cmp.22
  br i1 %hir.cmp.24, label %then.24, label %loop.16.preheader

loop.16.preheader:                                ; preds = %for.body.preheader
  br label %loop.16

for.cond.cleanup:                                 ; preds = %loop.16, %loop.66, %afterloop.27, %entry
  ret void

then.24:                                          ; preds = %for.body.preheader
  %3 = and i64 %wide.trip.count, 4294967292
  %extract.0.22 = icmp eq i64 %3, 0
  br i1 %extract.0.22, label %loop.66.preheader, label %loop.27.preheader

loop.27.preheader:                                ; preds = %then.24
  br label %loop.27

loop.27:                                          ; preds = %loop.27.preheader, %loop.27
  %i1.i64.0 = phi i64 [ %nextivloop.27, %loop.27 ], [ 0, %loop.27.preheader ]
  %scevgep55 = getelementptr float, float* %in, i64 %i1.i64.0
  %scevgep5556 = bitcast float* %scevgep55 to <4 x float>*
  %gepload = load <4 x float>, <4 x float>* %scevgep5556, align 4
  %4 = fpext <4 x float> %gepload to <4 x double>
  %.part.0.of.2. = shufflevector <4 x double> %4, <4 x double> undef, <2 x i32> <i32 0, i32 1>
  %vcall = call fast svml_cc <2 x double> @__svml_log22(<2 x double> %.part.0.of.2.)
  %.part.1.of.2. = shufflevector <4 x double> %4, <4 x double> undef, <2 x i32> <i32 2, i32 3>
  %vcall59 = call fast svml_cc <2 x double> @__svml_log22(<2 x double> %.part.1.of.2.)
  %shuffle.comb = shufflevector <2 x double> %vcall, <2 x double> %vcall59, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %5 = fptrunc <4 x double> %shuffle.comb to <4 x float>
  %scevgep53 = getelementptr float, float* %out, i64 %i1.i64.0
  %scevgep5354 = bitcast float* %scevgep53 to <4 x float>*
  store <4 x float> %5, <4 x float>* %scevgep5354, align 4
  %nextivloop.27 = add nuw nsw i64 %i1.i64.0, 4
  %condloop.27.not.not = icmp ult i64 %nextivloop.27, %3
  br i1 %condloop.27.not.not, label %loop.27, label %afterloop.27

afterloop.27:                                     ; preds = %loop.27
  %extract.0.2030 = icmp eq i64 %3, %wide.trip.count
  br i1 %extract.0.2030, label %for.cond.cleanup, label %loop.66.preheader

loop.66.preheader:                                ; preds = %afterloop.27, %then.24
  %i1.i64.1.ph = phi i64 [ %3, %afterloop.27 ], [ 0, %then.24 ]
  br label %loop.66

loop.66:                                          ; preds = %loop.66.preheader, %loop.66
  %i1.i64.1 = phi i64 [ %nextivloop.66, %loop.66 ], [ %i1.i64.1.ph, %loop.66.preheader ]
  %scevgep52 = getelementptr float, float* %in, i64 %i1.i64.1
  %gepload31 = load float, float* %scevgep52, align 4
  %6 = fpext float %gepload31 to double
  %.splatinsert32 = insertelement <1 x double> poison, double %6, i64 0
  %vcall60 = call fast svml_cc <1 x double> @__svml_log21(<1 x double> %.splatinsert32)
  %elem34 = extractelement <1 x double> %vcall60, i64 0
  %7 = fptrunc double %elem34 to float
  %scevgep = getelementptr float, float* %out, i64 %i1.i64.1
  store float %7, float* %scevgep, align 4
  %nextivloop.66 = add nuw nsw i64 %i1.i64.1, 1
  %condloop.66.not = icmp eq i64 %wide.trip.count, %nextivloop.66
  br i1 %condloop.66.not, label %for.cond.cleanup, label %loop.66

loop.16:                                          ; preds = %loop.16.preheader, %loop.16
  %i1.i64.2 = phi i64 [ %nextivloop.16, %loop.16 ], [ 0, %loop.16.preheader ]
  %scevgep58 = getelementptr float, float* %in, i64 %i1.i64.2
  %gepload35 = load float, float* %scevgep58, align 4
  %8 = fpext float %gepload35 to double
  %9 = tail call fast double @llvm.log2.f64(double %8) #2
  %10 = fptrunc double %9 to float
  %scevgep57 = getelementptr float, float* %out, i64 %i1.i64.2
  store float %10, float* %scevgep57, align 4
  %nextivloop.16 = add nuw nsw i64 %i1.i64.2, 1
  %condloop.16.not = icmp eq i64 %wide.trip.count, %nextivloop.16
  br i1 %condloop.16.not, label %for.cond.cleanup, label %loop.16
}

declare double @llvm.log2.f64(double) #1

declare <4 x double> @__svml_log24(<4 x double>) #1

declare <1 x double> @__svml_log21(<1 x double>) #1
declare <2 x double> @__svml_log22(<2 x double>) #1
declare <8 x double> @__svml_log28(<8 x double>) #1
declare <16 x double> @__svml_log216(<16 x double>) #1
declare <32 x double> @__svml_log232(<32 x double>) #1
declare <64 x double> @__svml_log264(<64 x double>) #1

attributes #0 = { argmemonly nofree nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { "vector-function-abi-variant"="_ZGV_LLVM_N2v_llvm.log2.f64(__svml_log22),_ZGV_LLVM_N4v_llvm.log2.f64(__svml_log24),_ZGV_LLVM_N8v_llvm.log2.f64(__svml_log28),_ZGV_LLVM_N16v_llvm.log2.f64(__svml_log216),_ZGV_LLVM_N32v_llvm.log2.f64(__svml_log232),_ZGV_LLVM_N64v_llvm.log2.f64(__svml_log264)" }

!llvm.module.flags = !{!6, !7}

!6 = !{i32 2, !"cfguard", i32 2}
!7 = !{i32 1, !"wchar_size", i32 2}
