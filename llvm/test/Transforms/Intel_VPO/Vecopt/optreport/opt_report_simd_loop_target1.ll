; Test to check the functionality of target check of vectorization opt-report for LLVM-IR based vectorizer.
; Test should pass if remarks #15569 and 15300 apear for code with #pragma omp simd
; RUN: opt -vplan-vec -vplan-force-vf=4 -intel-opt-report=high -intel-ir-optreport-emitter -enable-intel-advanced-opts -mcpu=skylake-avx512 < %s -disable-output 2>&1 | FileCheck %s --strict-whitespace -check-prefixes=AVX512
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @serial_call() nounwind

define void @test_serialized(i32* nocapture %arr) local_unnamed_addr {
; AVX512-LABEL: Global optimization report for : test_serialized
; AVX512-EMPTY:
; AVX512-NEXT:  LOOP BEGIN
; AVX512-NEXT:      remark #15569: Compiler has chosen to target XMM/YMM vector. Try using -mprefer-vector-width=512 to override.
; AVX512-NEXT:      remark #15301: SIMD LOOP WAS VECTORIZED
; AVX512:       LOOP END

entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %header ]
  call void @serial_call()
  %iv.next = add nuw nsw i64 %iv, 1
  call void @serial_call()
  %exitcond = icmp eq i64 %iv.next, 300
  br i1 %exitcond, label %loop.exit, label %header

loop.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare void @vec_func(i64) #0
declare void @_ZGVBN4v_vec_func(i64)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { nounwind "vector-variants"="_ZGVbN4l_vec_func" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87,+avx,+avx2" }
attributes #1 = { nounwind readnone "target-features"="+fxsr,+mmx,+sse,+sse2,+x87,+avx,+avx2" }

