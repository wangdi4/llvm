;
; Test that functionality to restrict vector calls for large VFs
; works corrrectly.
;
; RUN: opt -S -passes="vplan-vec" < %s | FileCheck %s --check-prefixes=CHECK,RESTRICT
; RUN: opt -S -passes="vplan-vec" -vplan-cm-prohibit-zmm-low-pumping=0 < %s | FileCheck %s --check-prefixes=CHECK,NORM
; RUN: opt -S -passes="vplan-vec" -vplan-force-vf=16 < %s | FileCheck %s --check-prefixes=CHECK,FORCE16
;
; CHECK-LABEL: define dso_local i32 @func1
; RESTRICT-NOT: call <16 x i32> @_ZGVeN16vvv_foo
; NORM:    call <16 x i32> @_ZGVeN16vvv_foo
; FORCE16: call <16 x i32> @_ZGVeN16vvv_foo

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

declare i32 @foo(i32, i16, i32) local_unnamed_addr #0

define dso_local i32 @func1() local_unnamed_addr #1 {
DIR.OMP.SIMD.2:
  %i.linear.iv = alloca i32, align 4
  %a = alloca [256 x i32], align 16
  %b = alloca [256 x i32], align 16
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1)]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %DIR.OMP.SIMD.1 ]
  %arrayidx = getelementptr inbounds [256 x i32], ptr %b, i64 0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx, align 4
  %2 = trunc i64 %indvars.iv to i16
  %call = call i32 @foo(i32 256, i16 %2, i32 %1)
  %arrayidx2 = getelementptr inbounds [256 x i32], ptr %a, i64 0, i64 %indvars.iv
  store i32 %call, ptr %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 256
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.4
  ret i32 0
}

; CHECK-LABEL: define dso_local i32 @func2
; RESTRICT-NOT: call void @_ZGVeN16vvv_foo2
; NORM:    call void @_ZGVeN16vvv_foo2
; FORCE16: call void @_ZGVeN16vvv_foo2

declare void @foo2(i8, i16, i32) local_unnamed_addr #2

define dso_local i32 @func2() local_unnamed_addr #1 {
DIR.OMP.SIMD.2:
  %i.linear.iv = alloca i32, align 4
  %a = alloca [256 x i32], align 16
  %b = alloca [256 x i32], align 16
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1)]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %DIR.OMP.SIMD.1 ]
  %arrayidx = getelementptr inbounds [256 x i32], ptr %b, i64 0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx, align 4
  %2 = trunc i64 %indvars.iv to i16
  call void  @foo2(i8 256, i16 %2, i32 %1)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 256
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.4
  ret i32 0
}


attributes #0 = { "vector-variants"="_ZGVdN8vvv_foo,_ZGVeN16vvv_foo,_ZGVdM8vvv_foo,_ZGVeM16vvv_foo" noinline norecurse nounwind readnone uwtable }
attributes #1 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { "vector-variants"="_ZGVdN8vvv_foo2,_ZGVeN16vvv_foo2,_ZGVdM8vvv_foo2,_ZGVeM16vvv_foo2" noinline norecurse nounwind readnone uwtable }

