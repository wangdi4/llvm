; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=vpo-paropt-transform -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=vpo-paropt-transform -S %s 2>&1 | FileCheck %s

; Check that paropt pass recognizes the rotated loops by frontend and doesn't call the loop rotation utility. 
; Also, check that paropt pass generates the proper reduction code.

; Test Src:

; int main() {
;   int a1, a2, threads, j;
;   for (threads = min_threads; threads <= max_threads; threads++) {
;     omp_set_num_threads(threads);
;     a1 = 67;
;     a2 = 67;
; #pragma omp parallel shared(a1, a2)
;     {
; #pragma omp for reduction(* : a1) reduction(- : a2)
;       for (j = 0; j < 11; j++) {
;         a1 *= 2;
;         a2--;
;       }
;     }
;   }
;   return 0;
; }

; CHECK-NOT: Loop is not rotated, calling LoopRotation
; CHECK: define internal void @main_tree_reduce{{.*}}(ptr %dst, ptr %src)
; CHECK: define internal void @main.DIR.OMP.PARALLEL{{.*}}
; CHECK: call i32 @__kmpc_reduce(ptr {{.*}}, i32 {{.*}}, i32 {{.*}}, i32 {{.*}}, ptr {{.*}}, ptr @main_tree_reduce{{.*}}, ptr {{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %a1 = alloca i32, align 4
  %a2 = alloca i32, align 4
  %threads = alloca i32, align 4
  %j = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i32 1, ptr %threads, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %threads, align 4
  %cmp = icmp sle i32 %0, 4
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, ptr %threads, align 4
  call void @omp_set_num_threads(i32 noundef %1)
  store i32 67, ptr %a1, align 4
  store i32 67, ptr %a2, align 4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %a1, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %a2, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]

  store i32 0, ptr %.omp.lb, align 4
  store i32 10, ptr %.omp.ub, align 4
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.REDUCTION.MUL:TYPED"(ptr %a1, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.SUB:TYPED"(ptr %a2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0) ]

  %4 = load i32, ptr %.omp.lb, align 4
  store i32 %4, ptr %.omp.iv, align 4
  %5 = load i32, ptr %.omp.iv, align 4
  %6 = load i32, ptr %.omp.ub, align 4
  %cmp1 = icmp sle i32 %5, %6
  br i1 %cmp1, label %omp.inner.for.body.lh, label %omp.inner.for.end

omp.inner.for.body.lh:                            ; preds = %for.body
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %omp.inner.for.body.lh
  %7 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %j, align 4
  %8 = load i32, ptr %a1, align 4
  %mul2 = mul nsw i32 %8, 2
  store i32 %mul2, ptr %a1, align 4
  %9 = load i32, ptr %a2, align 4
  %dec = add nsw i32 %9, -1
  store i32 %dec, ptr %a2, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, ptr %.omp.iv, align 4
  %add3 = add nsw i32 %10, 1
  store i32 %add3, ptr %.omp.iv, align 4
  %11 = load i32, ptr %.omp.iv, align 4
  %12 = load i32, ptr %.omp.ub, align 4
  %cmp4 = icmp sle i32 %11, %12
  br i1 %cmp4, label %omp.inner.for.body, label %omp.inner.for.end_crit_edge

omp.inner.for.end_crit_edge:                      ; preds = %omp.inner.for.inc
  br label %omp.inner.for.end

omp.inner.for.end:                                ; preds = %omp.inner.for.end_crit_edge, %for.body
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.LOOP"() ]

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]

  br label %for.inc

for.inc:                                          ; preds = %omp.loop.exit
  %13 = load i32, ptr %threads, align 4
  %inc = add nsw i32 %13, 1
  store i32 %inc, ptr %threads, align 4
  br label %for.cond, !llvm.loop !5

for.end:                                          ; preds = %for.cond
  ret i32 0
}

declare dso_local void @omp_set_num_threads(i32 noundef)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
