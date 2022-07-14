; Check to see that we properly promote boolean (i1) parameters to <VF x i8>
; when generating a call to a vectorized variant function.

; RUN: opt -vec-clone -vplan-vec -S < %s | FileCheck %s
; RUN: opt -passes='vec-clone,vplan-vec' -S < %s | FileCheck %s

; CHECK-LABEL: @bar(i1 noundef zeroext %i)

; CHECK:   [[ZEXT2:%.*]] = zext <16 x i1> [[VAL2:%.*]] to <16 x i8>
; CHECK:   [[RESULT2:%.*]] = call <16 x i32> @_ZGVbN16v_foo(<16 x i8> [[ZEXT2]])

; CHECK:   [[ZEXT:%.*]] = zext <16 x i1> [[VAL:%.*]] to <16 x i8>
; CHECK:   [[RESULT:%.*]] = call <16 x i32> @_ZGVbM16v_foo(<16 x i8> [[ZEXT]], <16 x i32> [[MASK:%.*]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare i32 @foo(i1) #0

define i32 @bar(i1 noundef zeroext %i) #1 {
entry:
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %entry
  %.omp.iv.02 = phi i32 [ 0, %DIR.OMP.SIMD.1 ], [ %add9, %omp.inner.for.inc ]
  %result = call i32 @foo(i1 %i)
  %cond = call i1 @condition()
  br i1 %cond, label %cond.true, label %omp.inner.for.inc

cond.true:                                        ; preds = %omp.inner.for.body
  %result2 = call i32 @foo(i1 %i)
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %cond.true, %omp.inner.for.body
  %add9 = add i32 %.omp.iv.02, 1
  %exitcond.not = icmp eq i32 %.omp.iv.02, undef
  br i1 %exitcond.not, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.inc
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:                                             ; preds = %omp.loop.exit
  ret i32 0
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare i1 @condition()

attributes #0 = { "vector-variants"="_ZGVbN16v_foo,_ZGVbM16v_foo" }
attributes #1 = { "prefer-vector-width"="512" "target-cpu"="skylake-avx512" }
