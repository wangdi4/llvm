; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -sroa -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,loop-simplify,sroa,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s

; Verify that the reference to '@a' is seen during the outlining
; of the target region, when device_triples contain SPIRV,
; so that extraction for host and SPIRV follows the same pattern.
; If the reference to '@a' is not present after PARALLEL.LOOP
; outlining on the host, then it will not be represented
; as an argument to the target outline function.  At the same time,
; the reference will be seen during target outlining in SPIRV
; compilation, and will have the corresponding outlined function
; argument.  This causes interface mismatch for the target outline
; function on the host and SPIRV target.
; Original code:
; int a[100];
;
; void foo() {
;   int i;
; #pragma omp target map(a)
; #pragma omp parallel for
;   for (i = 0; i < 100; ++i) {
;     a[i] = 0;
;   }
; }

; CHECK-DAG: define internal void @__omp_offloading_804_52009c5_foo_l5(ptr %a, ptr %i)
; In addition, check that the outlined function for PARALLEL.LOOP does not
; take 'a' by a pointer argument, because it may access it as a global variable.
; CHECK-DAG: define internal void @foo.DIR.OMP.PARALLEL.LOOP{{.*}}(ptr %tid, ptr %bid, ptr %a, ptr %.omp.lb, ptr %.omp.ub)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@a = common dso_local global [100 x i32] zeroinitializer, align 16

define dso_local void @foo() {
entry:
  %i = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %i)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr @a, ptr @a, i64 400, i64 35, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1) ]

; %.omp.iv was manually added to PRIVATE clause above.  This will be done automatically in FE,
; when omp.iv privatization for the inner region is enabled in Paropt.
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv)
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb)
  store i32 0, ptr %.omp.lb, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub)
  store i32 99, ptr %.omp.ub, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @a, i32 0, i32 100) ]

  %2 = load i32, ptr %.omp.lb, align 4
  store i32 %2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, ptr %.omp.iv, align 4
  %4 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %6 = load i32, ptr %i, align 4
  %idxprom = sext i32 %6 to i64
  %arrayidx = getelementptr inbounds [100 x i32], ptr @a, i64 0, i64 %idxprom
  store i32 0, ptr %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %7, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub)
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.lb)
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv)
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %i)
  ret void
}

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2052, i32 85985733, !"foo", i32 5, i32 0, i32 0}
