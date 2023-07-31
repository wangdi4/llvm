; RUN: opt -S -passes="vplan-vec" < %s | FileCheck %s
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare i64 @foo(i64 %i)
declare <2 x i64> @_ZGVbN2l_foo(i64 %i)
declare <2 x i64> @_ZGVbN2u_foo(i64 %u)

define i32 @main() local_unnamed_addr {
entry:
  %a = alloca [1000 x i64], align 16
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %call = tail call i64 @foo(i64 %indvars.iv) #0
; CHECK: call <2 x i64> @_ZGVbN2l_foo(i64 [[ARG:%.*]]){{$}}
  %call2 = tail call i64 @foo(i64 3) #0
; CHECK: call <2 x i64> @_ZGVbN2u_foo(i64 3)
;
  %arrayidx = getelementptr inbounds [1000 x i64], ptr %a, i64 0, i64 %indvars.iv
  store i64 %call, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret i32 0
}
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { "vector-variants"="_ZGVbN2l_foo,_ZGVbN2u_foo" }
