; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S -debug-only=WRegionUtils %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S -debug-only=WRegionUtils %s 2>&1 | FileCheck %s

; void foo(int c) {
;   #pragma omp parallel for simd if (c > 123)
;   for (int i = 0; i < 2; i++)
;     ;
; }

; Make sure that we capture the "QUAL.OMP.IF" operand %tobool on "DIR.OMP.PARALLEL.LOOP", since it's
; used inside the region., and pass it to the outlined function by reference.

; CHECK: collectNonPointerValuesToBeUsedInOutlinedRegion: Non-pointer values to be passed into the outlined region: 'i1 %tobool '

; CHECK: define internal void @foo.DIR.OMP.PARALLEL.LOOP{{.*}}(ptr %tid, ptr %bid, ptr %tobool.addr, {{.*}})
; CHECK: [[TOBOOL:%tobool[^ ]*]] = load i1, ptr %tobool.addr, align 1
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.IF"(i1 [[TOBOOL]]), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %{{[^, ]*}}, i32 0, i32 1, i32 1) ]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32 noundef %c) {
entry:
  %c.addr = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %c, ptr %c.addr, align 4
  %0 = load i32, ptr %c.addr, align 4
  %cmp = icmp sgt i32 %0, 123
  %conv = zext i1 %cmp to i32
  store i32 %conv, ptr %.capture_expr.0, align 4
  store i32 0, ptr %.omp.lb, align 4
  store i32 1, ptr %.omp.ub, align 4
  %1 = load i32, ptr %.capture_expr.0, align 4
  %tobool = icmp ne i32 %1, 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.IF"(i1 %tobool),
    "QUAL.OMP.SHARED:TYPED"(ptr %.capture_expr.0, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.IF"(i1 %tobool),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i, i32 0, i32 1, i32 1) ]

  %4 = load i32, ptr %.omp.lb, align 4
  store i32 %4, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %5 = load i32, ptr %.omp.iv, align 4
  %6 = load i32, ptr %.omp.ub, align 4
  %cmp1 = icmp sle i32 %5, %6
  br i1 %cmp1, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add3 = add nsw i32 %8, 1
  store i32 %add3, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SIMD"() ]

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  ret void
}

declare token @llvm.directive.region.entry() 

declare void @llvm.directive.region.exit(token)

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
