; RUN: opt -O3 -paropt=31 -S %s | FileCheck %s

; Test src:
;
; void test_fence(float *A, int N) {
; #pragma omp parallel for
;   for (int i = 0; i < N; ++i)
; #pragma omp critical
;     *A += 5.0;
; }

; This test checks that presense of fence intructions inside the parallel region
; does not prevent privatization of a shared item that is not modified inside
; the region.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @test_fence(ptr noundef %A, i32 noundef %N) #0 {
entry:
  %A.addr = alloca ptr, align 8
  %N.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %A, ptr %A.addr, align 8
  store i32 %N, ptr %N.addr, align 4
  %0 = load i32, ptr %N.addr, align 4
  store i32 %0, ptr %.capture_expr.0, align 4
  %1 = load i32, ptr %.capture_expr.0, align 4
  %sub = sub nsw i32 %1, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.1, align 4
  %2 = load i32, ptr %.capture_expr.0, align 4
  %cmp = icmp slt i32 0, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, ptr %.omp.lb, align 4
  %3 = load i32, ptr %.capture_expr.1, align 4
  store i32 %3, ptr %.omp.ub, align 4

; CHECK: call {{.*}} @__kmpc_fork_call({{.*}}, i32 3, ptr nonnull [[OUTLINED_TEST_FENCE:@.+]], ptr %A,

  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %A.addr, ptr null, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]
  %5 = load i32, ptr %.omp.lb, align 4
  store i32 %5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %6 = load i32, ptr %.omp.iv, align 4
  %7 = load i32, ptr %.omp.ub, align 4
  %cmp3 = icmp sle i32 %6, %7
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %8, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr %i, align 4
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.CRITICAL"() ]
  fence acquire
  %10 = load ptr, ptr %A.addr, align 8
  %11 = load float, ptr %10, align 4
  %conv = fpext float %11 to double
  %add5 = fadd fast double %conv, 5.000000e+00
  %conv6 = fptrunc double %add5 to float
  store float %conv6, ptr %10, align 4
  fence release
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.CRITICAL"() ]
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, ptr %.omp.iv, align 4
  %add7 = add nsw i32 %12, 1
  store i32 %add7, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}

; CHECK: define internal void [[OUTLINED_TEST_FENCE]](ptr nocapture readonly %tid, ptr nocapture readnone %bid, ptr nocapture [[AADDR:%.+]], i64 %{{.+}}, i64 %{{.+}}) #{{[0-9]+}} {
; CHECK: omp.inner.for.body:
; CHECK:   [[AVAL:%.+]] = load float, ptr [[AADDR]]
; CHECK:   [[ADD:%.+]] = fadd fast float [[AVAL]], 5.000000e+00
; CHECK:   store float [[ADD]], ptr [[AADDR]]
; CHECK: }
