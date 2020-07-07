; Test to check the functionality of vectorization opt-report for LLVM-IR based vectorizer.

; RUN: opt -VPlanDriver -vector-library=SVML -vplan-force-vf=4 -intel-loop-optreport=high -intel-ir-optreport-emitter < %s -disable-output 2>&1 | FileCheck %s --strict-whitespace -check-prefixes=LLVM,CHECK

; RUN: opt -hir-ssa-deconstruction -hir-framework -VPlanDriverHIR -vector-library=SVML -vplan-force-vf=4 -intel-loop-optreport=high -hir-optreport-emitter < %s -disable-output 2>&1 | FileCheck %s --strict-whitespace -check-prefixes=HIR,CHECK

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @serial_call() nounwind

define void @test_serialized(i32* nocapture %arr) local_unnamed_addr {
; LLVM-LABEL: Global loop optimization report for : test_serialized
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:      Remark: LOOP WAS VECTORIZED
; LLVM-NEXT:      Remark: vectorization support: vector length 4
; LLVM-NEXT:      Remark: --- begin vector loop cost summary ---
; LLVM-NEXT:      Remark: vectorized math library calls: 0
; LLVM-NEXT:      Remark: vector function calls: 0
; LLVM-NEXT:      Remark: serialized function calls: 2
; LLVM-NEXT:      Remark: --- end vector loop cost summary ---
; LLVM-NEXT:  LOOP END
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:  LOOP END
; LLVM-NEXT:  =================================================================

; FIXME: Call serialization isn't supported for HIR yet.
; HIR-LABEL: Report from: HIR Loop optimizations framework for : test_serialized
; HIR-EMPTY:
; HIR-NEXT:  LOOP BEGIN
; HIR-NEXT:      Remark: loop was not vectorized:
; HIR-NEXT:  LOOP END
; HIR-NEXT:  =================================================================

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

define void @test_vector_variant(i32* nocapture %arr) local_unnamed_addr {
; LLVM-LABEL: Global loop optimization report for : test_vector_variant
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:      Remark: LOOP WAS VECTORIZED
; LLVM-NEXT:      Remark: vectorization support: vector length 4
; LLVM-NEXT:      Remark: --- begin vector loop cost summary ---
; LLVM-NEXT:      Remark: vectorized math library calls: 0
; LLVM-NEXT:      Remark: vector function calls: 1
; LLVM-NEXT:      Remark: serialized function calls: 0
; LLVM-NEXT:      Remark: --- end vector loop cost summary ---
; LLVM-NEXT:  LOOP END
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:  LOOP END
; LLVM-NEXT:  =================================================================

; FIXME: Vector-variant isn't supported for HIR yet.
; HIR-LABEL: Report from: HIR Loop optimizations framework for : test_vector_variant
; HIR-EMPTY:
; HIR-NEXT:  LOOP BEGIN
; HIR-NEXT:      Remark: loop was not vectorized:
; HIR-NEXT:  LOOP END
; HIR-NEXT:  =================================================================
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %header ]
  call void @vec_func(i64 %iv)
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 300
  br i1 %exitcond, label %loop.exit, label %header

loop.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare double @llvm.sqrt.f64(double %val) #1
declare double @_Z4sqrtd(double %val) #1
declare double @sqrt(double %val) #1
define void @test_sqrt(i32* nocapture %arr) local_unnamed_addr #1 {
; LLVM-LABEL: Global loop optimization report for : test_sqrt
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:      Remark: LOOP WAS VECTORIZED
; LLVM-NEXT:      Remark: vectorization support: vector length 4
; LLVM-NEXT:      Remark: --- begin vector loop cost summary ---
; LLVM-NEXT:      Remark: vectorized math library calls: 3
; LLVM-NEXT:      Remark: vector function calls: 0
; LLVM-NEXT:      Remark: serialized function calls: 0
; LLVM-NEXT:      Remark: --- end vector loop cost summary ---
; LLVM-NEXT:  LOOP END
; LLVM-EMPTY:
; LLVM-NEXT:  LOOP BEGIN
; LLVM-NEXT:  LOOP END
; LLVM-NEXT:  =================================================================

; HIR-LABEL: Report from: HIR Loop optimizations framework for : test_sqrt
; HIR-EMPTY:
; HIR-NEXT:  LOOP BEGIN
; HIR-NEXT:      Remark: LOOP WAS VECTORIZED
; HIR-NEXT:      Remark: vectorization support: vector length 4
; HIR-NEXT:      Remark: --- begin vector loop cost summary ---
; HIR-NEXT:      Remark: vectorized math library calls: 3
; HIR-NEXT:      Remark: vector function calls: 0
; HIR-NEXT:      Remark: serialized function calls: 0
; HIR-NEXT:      Remark: --- end vector loop cost summary ---
; HIR-NEXT:  LOOP END
; HIR-NEXT:  =================================================================
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %header ]
  %d = sitofp i64 %iv to double
  ; Intrinsic
  %sqrt = call double @llvm.sqrt.f64(double %d)
  ; Vectorizable, but vector function isn't prefixed with __svml
  %sqrt2 = call double @_Z4sqrtd(double %d)
  ; Vector function is prefixed with __svml
  %sqrt3 = call double @sqrt(double %d)
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 300
  br i1 %exitcond, label %loop.exit, label %header

loop.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { nounwind "vector-variants"="_ZGVbN4v_vec_func" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87,+avx,+avx2" }
attributes #1 = { nounwind "target-features"="+fxsr,+mmx,+sse,+sse2,+x87,+avx,+avx2" }
