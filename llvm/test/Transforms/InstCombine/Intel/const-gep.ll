; CMPLRLLVM-24000
; Transform in InstCombine is expecting a non-constant GEP.
; RUN: opt -passes="instcombine" < %s -S | FileCheck %s
;
; IC should be able to remove everything in the loop as dead.
; CHECK-LABEL: loop.18:
; CHECK-NEXT: br

source_filename = "jr11837_00_basis.i"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@cbsdata_ = external unnamed_addr global [3855472 x i8], align 32

define void @bas_nprim_cn_max_(ptr %"bas_nprim_cn_max_$BASISIN") local_unnamed_addr #0 {
alloca_40:
  %"bas_nprim_cn_max_$BASISIN_fetch" = load i64, ptr %"bas_nprim_cn_max_$BASISIN", align 1
  br i1 undef, label %bb6346, label %bb6345.preheader

bb6345.preheader:                                 ; preds = %alloca_40
  br label %loop.18

bb6346:                                           ; preds = %alloca_40
  ret void

loop.18:                                          ; preds = %loop.18, %bb6345.preheader
  %t17.0 = phi <4 x i64> [ zeroinitializer, %bb6345.preheader ], [ %4, %loop.18 ]
  %0 = add i64 %"bas_nprim_cn_max_$BASISIN_fetch", -566
  %.splatinsert9 = insertelement <4 x i64> undef, i64 %0, i32 0
  %1 = mul nsw <4 x i64> %.splatinsert9, <i64 80008, i64 undef, i64 undef, i64 undef>
  %2 = shufflevector <4 x i64> %1, <4 x i64> undef, <4 x i32> zeroinitializer
  %arrayIdx = getelementptr inbounds i64, ptr getelementptr inbounds ([3855472 x i8], ptr @cbsdata_, i64 0, i64 13928), <4 x i64> %2
  %arrayIdx13 = getelementptr inbounds i64, <4 x ptr> %arrayIdx, <4 x i64> <i64 1, i64 1, i64 1, i64 1>
  %arrayIdx14 = getelementptr inbounds i64, <4 x ptr> %arrayIdx13, <4 x i64> undef
  %3 = call <4 x i64> @llvm.masked.gather.v4i64.v4p0(<4 x ptr> %arrayIdx14, i32 1, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i64> undef)
  %4 = select <4 x i1> undef, <4 x i64> %3, <4 x i64> %t17.0
  br label %loop.18
}

; Function Attrs: nounwind readonly willreturn
declare <4 x i64> @llvm.masked.gather.v4i64.v4p0(<4 x ptr>, i32 immarg, <4 x i1>, <4 x i64>) #1

attributes #0 = { "target-features"="+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" }
attributes #1 = { nounwind readonly willreturn }

!omp_offload.info = !{}
