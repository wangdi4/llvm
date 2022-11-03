; RUN: opt -S -vplan-vec < %s | FileCheck %s
; RUN: opt -S -passes="vplan-vec" < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Test to ensure that calling convention is transferred to the call to
; the vector function. Here, fastcc should be copied to the call.

; CHECK: call fastcc noundef <8 x i64> @_ZGVbN8ul__Z3fooll

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind readnone willreturn uwtable
declare fastcc i64 @_Z3fooll(i64 , i64 ) #0 

; Function Attrs: mustprogress noinline norecurse nounwind uwtable
define dso_local noundef i32 @main(i32 noundef %argc, i8** nocapture noundef readnone %argv) local_unnamed_addr #1 {
DIR.OMP.SIMD.1:
  %a = alloca [128 x i32], align 16
  br label %DIR.OMP.SIMD.131

DIR.OMP.SIMD.131:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.131
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %.omp.iv.local.026 = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %add7, %omp.inner.for.body ]
  %call = call fastcc noundef i64 @_Z3fooll(i64 noundef 1, i64 noundef %.omp.iv.local.026)
  %conv = trunc i64 %call to i32
  %arrayidx6 = getelementptr inbounds [128 x i32], [128 x i32]* %a, i64 0, i64 %.omp.iv.local.026
  store i32 %conv, i32* %arrayidx6, align 4
  %add7 = add nuw nsw i64 %.omp.iv.local.026, 1
  %exitcond.not = icmp eq i64 %add7, 128
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { mustprogress nofree noinline norecurse nosync nounwind readnone willreturn uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "vector-variants"="_ZGVbN8ul__Z3fooll" }
