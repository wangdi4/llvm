; RUN: opt -passes='default<O2>' -loopopt -S -paropt=31 -pre-loopopt-vpo-passes=false -print-after=hir-temp-cleanup < %s 2>&1 | FileCheck %s
; Note: -paropt=31 is an equivalent of -fiopenmp for the driver.

; Check that the loop will be recognized and vectorized inside loopopt without failures.

; void foo(int *a) {
;   int i;
; #pragma omp simd
;   for (i = 32; i > 1; i--) {
;     if (a[4]!=20) {
;       a[i] = i;
;     }
;   }
; }

; CHECK-LABEL: BEGIN REGION
; CHECK:       @llvm.directive.region.entry
; CHECK-SAME:  DIR.OMP.SIMD
; CHECK:       @llvm.directive.region.exit
; CHECK-SAME:  DIR.OMP.END.SIMD

; CHECK-LABEL: define
; CHECK-SAME:  @foo
; CHECK:       <{{[0-9]+}} x i{{[0-9]+}}>

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(ptr %a) #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #2
  store volatile i32 30, ptr %.omp.ub, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0) ]
  store volatile i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.SIMD.1
  %1 = load volatile i32, ptr %.omp.iv, align 4
  %2 = load volatile i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %1, %2
  br i1 %cmp, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %3 = load volatile i32, ptr %.omp.iv, align 4
  %sub = sub nsw i32 32, %3
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 4
  %4 = load i32, ptr %arrayidx, align 4
  %cmp1 = icmp ne i32 %4, 20
  br i1 %cmp1, label %if.then, label %omp.inner.for.inc

if.then:                                          ; preds = %omp.inner.for.body
  %idxprom = sext i32 %sub to i64
  %arrayidx2 = getelementptr inbounds i32, ptr %a, i64 %idxprom
  store i32 %sub, ptr %arrayidx2, align 4
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %if.then, %omp.inner.for.body
  %5 = load volatile i32, ptr %.omp.iv, align 4
  %add = add nsw i32 %5, 1
  store volatile i32 %add, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64, ptr nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64, ptr nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" "loopopt-pipeline"="full" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

