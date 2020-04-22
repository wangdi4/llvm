; RUN: opt -S -VPlanDriver < %s | FileCheck %s
; RUN: opt -S -passes="vplan-driver" < %s | FileCheck %s
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare i32 @foo(i32 %i)
declare <4 x i32> @_ZGVbN4l_foo(i32 %i)

define i32 @main() local_unnamed_addr {
entry:
  %a = alloca [1000 x i32], align 16
  %0 = bitcast [1000 x i32]* %a to i8*
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %1 = trunc i64 %indvars.iv to i32
  %call = tail call i32 @foo(i32 %1) #0
; CHECK: call <4 x i32> @_ZGVbN4l_foo(i32 [[ARG:%.*]]){{$}}
  %call2 = tail call i32 @foo(i32 %1) #1
; CHECK: call <4 x i32> @_ZGVbN4l_foo(i32 [[ARG]]) [[SECOND_CALL_ATTR:#[0-9]*]]
;
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* %a, i64 0, i64 %indvars.iv
  store i32 %call, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret i32 0
}
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { "vector-variants"="_ZGVbN4l_foo,_ZGVcN8l_foo,_ZGVdN8l_foo,_ZGVeN16l_foo,_ZGVbM4l_foo,_ZGVcM8l_foo,_ZGVdM8l_foo,_ZGVeM16l_foo" }
attributes #1 = { noinline "vector-variants"="_ZGVbN4l_foo,_ZGVcN8l_foo,_ZGVdN8l_foo,_ZGVeN16l_foo,_ZGVbM4l_foo,_ZGVcM8l_foo,_ZGVdM8l_foo,_ZGVeM16l_foo" }

; CHECK: attributes [[SECOND_CALL_ATTR]] = { noinline }
