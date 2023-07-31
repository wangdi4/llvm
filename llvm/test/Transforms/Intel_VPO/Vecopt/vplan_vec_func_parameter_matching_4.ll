; RUN: opt -S -passes="vplan-vec" < %s | FileCheck %s

; Test that caller/callee parameter matching fails and results in serialized calls. Vector
; argument is not compatible with either uniform or linear parameter.

; CHECK-LABEL: vector.body:
; CHECK: call i64 @foo
; CHECK-NEXT: insertelement
; CHECK-NEXT: call i64 @foo

; CHECK-LABEL: vector.body{{[0-9]+}}:
; CHECK: call i64 @foo2
; CHECK-NEXT: insertelement
; CHECK-NEXT: call i64 @foo2

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define dso_local i64 @foo(i64 %x) local_unnamed_addr #0 {
entry:
  %add = add nsw i64 %x, 1
  ret i64 %add
}

define dso_local i64 @foo2(i64 %x) local_unnamed_addr #1 {
entry:
  %add = add nsw i64 %x, 1
  ret i64 %add
}

define dso_local i32 @main() local_unnamed_addr #2 {
omp.inner.for.body.lr.ph:
  %i.linear.iv36 = alloca i64, align 8
  %i.linear.iv = alloca i64, align 8
  %a = alloca [256 x i64], align 16
  %b = alloca [256 x i64], align 16
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i64 0, i32 1, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %.omp.iv.local.033 = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %add2, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds [256 x i64], ptr %b, i64 0, i64 %.omp.iv.local.033
  %1 = load i64, ptr %arrayidx, align 8
  %call = call i64 @foo(i64 %1) #4
  %arrayidx1 = getelementptr inbounds [256 x i64], ptr %a, i64 0, i64 %.omp.iv.local.033
  store i64 %call, ptr %arrayidx1, align 8
  %add2 = add nuw nsw i64 %.omp.iv.local.033, 1
  %exitcond42 = icmp eq i64 %add2, 256
  br i1 %exitcond42, label %omp.inner.for.body9.lr.ph, label %omp.inner.for.body

omp.inner.for.body9.lr.ph:                        ; preds = %omp.inner.for.body
  store i64 256, ptr %i.linear.iv, align 8
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body9.lr.ph
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv36, i64 0, i32 1, i32 1) ]
  br label %omp.inner.for.body9

omp.inner.for.body9:                              ; preds = %omp.inner.for.body9, %DIR.OMP.END.SIMD.3
  %.omp.iv5.local.035 = phi i64 [ 0, %DIR.OMP.END.SIMD.3 ], [ %add17, %omp.inner.for.body9 ]
  %arrayidx12 = getelementptr inbounds [256 x i64], ptr %a, i64 0, i64 %.omp.iv5.local.035
  %3 = load i64, ptr %arrayidx12, align 8
  %call13 = call i64 @foo2(i64 %3) #4
  %arrayidx14 = getelementptr inbounds [256 x i64], ptr %b, i64 0, i64 %.omp.iv5.local.035
  store i64 %call13, ptr %arrayidx14, align 8
  %add17 = add nuw nsw i64 %.omp.iv5.local.035, 1
  %exitcond = icmp eq i64 %add17, 256
  br i1 %exitcond, label %DIR.OMP.END.SIMD.8, label %omp.inner.for.body9

DIR.OMP.END.SIMD.8:                               ; preds = %omp.inner.for.body9
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.END.SIMD.8
  ret i32 0
}

attributes #0 = { noinline norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbN2u_foo,_ZGVcN4u_foo,_ZGVdN4u_foo,_ZGVeN8u_foo,_ZGVbM2u_foo,_ZGVcM4u_foo,_ZGVdM4u_foo,_ZGVeM8u_foo" }
attributes #1 = { noinline norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbN2l_foo2,_ZGVcN4l_foo2,_ZGVdN4l_foo2,_ZGVeN8l_foo2,_ZGVbM2l_foo2,_ZGVcM4l_foo2,_ZGVdM4l_foo2,_ZGVeM8l_foo2" }
