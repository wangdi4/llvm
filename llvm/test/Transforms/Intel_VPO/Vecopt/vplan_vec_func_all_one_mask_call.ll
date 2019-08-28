; RUN: opt -VPlanDriver -use-i1-mask-for-simd-funcs -S < %s | FileCheck %s
; RUN: opt -VPlanDriver -use-i1-mask-for-simd-funcs -enable-vp-value-codegen -S < %s | FileCheck %s
; Check that we can vectorize vector functions with masked variants only in
; uniform CFG.
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare i32 @foo(i32 %i) #0
declare <4 x i32> @_ZGVbM4v_foo(<4 x i32> , <4 x i1>) #1

; Function Attrs: noinline nounwind uwtable
define i32 @main() local_unnamed_addr #1 {
entry:
  %a = alloca [1000 x i32], align 16
  %0 = bitcast [1000 x i32]* %a to i8*
  call void @llvm.lifetime.start(i64 4000, i8* nonnull %0) #5
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %1 = trunc i64 %indvars.iv to i32
; CHECK-LABEL: main
; CHECK-NOT: call i32 @foo
; CHECK:   call <4 x i32> @_ZGVbM4v_foo(<4 x i32> [[VECTOR_ARG:%.*]], <4 x i1> <i1 true, i1 true, i1 true, i1 true>)
; CHECK-NOT: call i32 @foo
; CHECK: omp.inner.for.body
  %call = tail call i32 @foo(i32 %1)
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* %a, i64 0, i64 %indvars.iv
  store i32 %call, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %omp.loop.exit
  %arrayidx2 = getelementptr inbounds [1000 x i32], [1000 x i32]* %a, i64 0, i64 0
  call void @llvm.lifetime.end(i64 4000, i8* nonnull %0) #5
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbM4v_foo" }

attributes #1 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind }
