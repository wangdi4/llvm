; RUN: opt -passes="vpo-paropt,function(loop-mssa(licm))" -aa-pipeline="basic-aa,scoped-noalias-aa" -S %s 2>&1 | FileCheck %s

; After paropt, the load of c0 should be hoisted by LICM as invariant.
; VPO outlines the for loop and generates noalias sets for the load and store.
; VPO must recognize that c0 is not captured by the OMP directive.
; This nocapture is tested directly by test "vpo-shared-nocapture.ll".

; void const_load(float c0, float **A) {
;   int i;
;   #pragma omp parallel for
;   for(i=1;i<100;i++) {
;     (*A)[i] = c0+i; // c0 is a parameter with a non-captured address
;   }
; }

;  outlined function definition
; CHECK: define{{.*}}split

;  c0 should be loaded in the pre-header of the loop
; CHECK: .ph:
; CHECK: load{{.*}}%c0.addr

;  c0 should not be loaded in the body
; CHECK: for.body{{.*}}:
; CHECK-NOT: load{{.*}}%c0.addr

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @const_load(float noundef nofpclass(nan inf) %c0, ptr noundef %A) {
entry:
  %c0.addr = alloca float, align 4
  %A.addr = alloca ptr, align 8
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store float %c0, ptr %c0.addr, align 4
  store ptr %A, ptr %A.addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 98, ptr %.omp.ub, align 4
  br label %DIR.OMP.PARALLEL.LOOP.1

DIR.OMP.PARALLEL.LOOP.1:                          ; preds = %entry
  br label %DIR.OMP.PARALLEL.LOOP.2

DIR.OMP.PARALLEL.LOOP.2:                          ; preds = %DIR.OMP.PARALLEL.LOOP.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %c0.addr, float 0.000000e+00, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %A.addr, ptr null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0) ]

  br label %DIR.OMP.PARALLEL.LOOP.3

DIR.OMP.PARALLEL.LOOP.3:                          ; preds = %DIR.OMP.PARALLEL.LOOP.2
  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.PARALLEL.LOOP.3
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 1, %mul
  store i32 %add, ptr %i, align 4
  %5 = load float, ptr %c0.addr, align 4
  %6 = load i32, ptr %i, align 4
  %conv = sitofp i32 %6 to float
  %add1 = fadd fast float %5, %conv
  %7 = load ptr, ptr %A.addr, align 8
  %8 = load ptr, ptr %7, align 8
  %9 = load i32, ptr %i, align 4
  %idxprom = sext i32 %9 to i64
  %arrayidx = getelementptr inbounds float, ptr %8, i64 %idxprom
  store float %add1, ptr %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, ptr %.omp.iv, align 4
  %add2 = add nsw i32 %10, 1
  store i32 %add2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:                      ; preds = %omp.loop.exit
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
