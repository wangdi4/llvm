; RUN: not opt -passes='verify' -disable-output < %s 2>&1 | FileCheck %s

; This test checks that the verifier is able to gracefully handle code with
; subscript intrinsics that don't have the elementtype attribute set.

; CHECK: llvm.intel.subscript requires elementtype attribute

source_filename = "iota.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nofree nosync nounwind writeonly uwtable
define void @iota_(ptr noalias nocapture writeonly dereferenceable(8) %"iota_$A") local_unnamed_addr #0 !llfort.type_idx !0 {
alloca_0:
  br label %bb2

bb2:                                              ; preds = %bb2, %alloca_0
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb2 ], [ 1, %alloca_0 ]
  %0 = trunc i64 %indvars.iv to i32
  %"(double)iota_$I_fetch.1$" = sitofp i32 %0 to double, !llfort.type_idx !1
  %"iota_$A_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull %"iota_$A", i64 %indvars.iv), !llfort.type_idx !2, !ifx.array_extent !3
  store double %"(double)iota_$I_fetch.1$", ptr %"iota_$A_entry[]", align 1, !tbaa !4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1025
  br i1 %exitcond.not, label %bb5, label %bb2

bb5:                                              ; preds = %bb2
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { argmemonly nofree nosync nounwind writeonly uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}

!0 = !{i64 25}
!1 = !{i64 6}
!2 = !{i64 28}
!3 = !{i64 1024}
!4 = !{!5, !5, i64 0}
!5 = !{!"ifx$unique_sym$2", !6, i64 0}
!6 = !{!"Fortran Data Symbol", !7, i64 0}
!7 = !{!"Generic Fortran Symbol", !8, i64 0}
!8 = !{!"ifx$root$1$iota_"}
