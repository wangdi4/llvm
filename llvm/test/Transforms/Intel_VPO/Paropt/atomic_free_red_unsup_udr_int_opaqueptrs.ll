; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck %s

; This a modified version of udr_add_int.ll checking that reduction items,
; that are supported by atomic-free reduction and which are not, coexist peacefully
; and not trigger a compiler error.
; The order of clauses is inverse: UDR, ADD.

;
; int a[1000];
; int main()
; {
;   int i;
;   int sum=0,sum1;
;   int sum_udr=0,sum_udr1;
;
;   // Init array
;   for (i=0; i<1000; i++) {
;     a[i] = i%10;
;   }
;
;   #pragma omp declare reduction(myadd : int : omp_out += omp_in) initializer (omp_priv = 0)
;
;   #pragma omp parallel for reduction(myadd:sum_udr) reduction(+:sum) reduction(myadd:sum_udr1) reduction(+:sum1)
;   for (i=0; i<1000; i++) {
;     sum_udr = sum_udr+a[i];
;     sum += a[i];
;     sum_udr1 = sum_udr+a[i];
;     sum1 += a[i];
;   }
;   return 0;
; }

; CHECK: __kmpc_critical
; CHECK-NOT: atomic.free.red.local.update.update.header
; CHECK: __kmpc_end_critical
; CHECK: __kmpc_critical
; CHECK-NOT: atomic.free.red.local.update.update.header
; CHECK: __kmpc_end_critical
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@a = external addrspace(1) global [1000 x i32], align 4

; Function Attrs: convergent noinline nounwind optnone
define protected i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca i32, align 4
  %sum1 = alloca i32, align 4
  %sum_udr = alloca i32, align 4
  %sum_udr1 = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %sum.ascast = addrspacecast ptr %sum to ptr addrspace(4)
  %sum1.ascast = addrspacecast ptr %sum1 to ptr addrspace(4)
  %sum_udr.ascast = addrspacecast ptr %sum_udr to ptr addrspace(4)
  %sum_udr1.ascast = addrspacecast ptr %sum_udr1 to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store i32 0, ptr addrspace(4) %sum.ascast, align 4
  store i32 0, ptr addrspace(4) %sum1.ascast, align 4
  store i32 0, ptr addrspace(4) %sum_udr.ascast, align 4
  store i32 0, ptr addrspace(4) %sum_udr1.ascast, align 4
  store i32 0, ptr addrspace(4) %i.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr addrspace(4) %i.ascast, align 4
  %cmp = icmp slt i32 %0, 1000
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, ptr addrspace(4) %i.ascast, align 4
  %rem = srem i32 %1, 10
  %2 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], ptr addrspace(4) addrspacecast (ptr addrspace(1) @a to ptr addrspace(4)), i64 0, i64 %idxprom
  store i32 %rem, ptr addrspace(4) %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %3 = load i32, ptr addrspace(4) %i.ascast, align 4
  %inc = add nsw i32 %3, 1
  store i32 %inc, ptr addrspace(4) %i.ascast, align 4
  br label %for.cond, !llvm.loop !7

for.end:                                          ; preds = %for.cond
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 999, ptr addrspace(4) %.omp.ub.ascast, align 4
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum_udr.ascast, ptr addrspace(4) %sum_udr.ascast, i64 4, i64 547, ptr null, ptr null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @a to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @a to ptr addrspace(4)), i64 4000, i64 547, ptr null, ptr null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum.ascast, ptr addrspace(4) %sum.ascast, i64 4, i64 547, ptr null, ptr null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum_udr1.ascast, ptr addrspace(4) %sum_udr1.ascast, i64 4, i64 547, ptr null, ptr null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum1.ascast, ptr addrspace(4) %sum1.ascast, i64 4, i64 547, ptr null, ptr null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.UDR:TYPED"(ptr addrspace(4) %sum_udr.ascast, i32 0, i32 1, ptr null, ptr null, ptr @.omp_combiner., ptr @.omp_initializer.),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum.ascast, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.UDR:TYPED"(ptr addrspace(4) %sum_udr1.ascast, i32 0, i32 1, ptr null, ptr null, ptr @.omp_combiner., ptr @.omp_initializer.),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum1.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @a to ptr addrspace(4)), i32 0, i64 1000),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0) ]

  %6 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %6, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %for.end
  %7 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %8 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp1 = icmp sle i32 %7, %8
  br i1 %cmp1, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  %10 = load i32, ptr addrspace(4) %sum_udr.ascast, align 4
  %11 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom2 = sext i32 %11 to i64
  %arrayidx3 = getelementptr inbounds [1000 x i32], ptr addrspace(4) addrspacecast (ptr addrspace(1) @a to ptr addrspace(4)), i64 0, i64 %idxprom2
  %12 = load i32, ptr addrspace(4) %arrayidx3, align 4
  %add4 = add nsw i32 %10, %12
  store i32 %add4, ptr addrspace(4) %sum_udr.ascast, align 4
  %13 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom5 = sext i32 %13 to i64
  %arrayidx6 = getelementptr inbounds [1000 x i32], ptr addrspace(4) addrspacecast (ptr addrspace(1) @a to ptr addrspace(4)), i64 0, i64 %idxprom5
  %14 = load i32, ptr addrspace(4) %arrayidx6, align 4
  %15 = load i32, ptr addrspace(4) %sum.ascast, align 4
  %add7 = add nsw i32 %15, %14
  store i32 %add7, ptr addrspace(4) %sum.ascast, align 4
  %16 = load i32, ptr addrspace(4) %sum_udr1.ascast, align 4
  %17 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom8 = sext i32 %17 to i64
  %arrayidx9 = getelementptr inbounds [1000 x i32], ptr addrspace(4) addrspacecast (ptr addrspace(1) @a to ptr addrspace(4)), i64 0, i64 %idxprom8
  %18 = load i32, ptr addrspace(4) %arrayidx9, align 4
  %add10 = add nsw i32 %16, %18
  store i32 %add10, ptr addrspace(4) %sum_udr1.ascast, align 4
  %19 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom11 = sext i32 %19 to i64
  %arrayidx12 = getelementptr inbounds [1000 x i32], ptr addrspace(4) addrspacecast (ptr addrspace(1) @a to ptr addrspace(4)), i64 0, i64 %idxprom11
  %20 = load i32, ptr addrspace(4) %arrayidx12, align 4
  %21 = load i32, ptr addrspace(4) %sum1.ascast, align 4
  %add13 = add nsw i32 %21, %20
  store i32 %add13, ptr addrspace(4) %sum1.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %22 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add14 = add nsw i32 %22, 1
  store i32 %add14, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent noinline nounwind
define internal void @.omp_combiner.(ptr addrspace(4) noalias noundef %0, ptr addrspace(4) noalias noundef %1) #2 {
entry:
  %.addr = alloca ptr addrspace(4), align 8
  %.addr1 = alloca ptr addrspace(4), align 8
  %.addr.ascast = addrspacecast ptr %.addr to ptr addrspace(4)
  %.addr1.ascast = addrspacecast ptr %.addr1 to ptr addrspace(4)
  store ptr addrspace(4) %0, ptr addrspace(4) %.addr.ascast, align 8
  store ptr addrspace(4) %1, ptr addrspace(4) %.addr1.ascast, align 8
  %2 = load ptr addrspace(4), ptr addrspace(4) %.addr1.ascast, align 8
  %3 = load ptr addrspace(4), ptr addrspace(4) %.addr.ascast, align 8
  %4 = load i32, ptr addrspace(4) %2, align 4
  %5 = load i32, ptr addrspace(4) %3, align 4
  %add = add nsw i32 %5, %4
  store i32 %add, ptr addrspace(4) %3, align 4
  ret void
}

; Function Attrs: convergent noinline nounwind
define internal void @.omp_initializer.(ptr addrspace(4) noalias noundef %0, ptr addrspace(4) noalias noundef %1) #2 {
entry:
  %.addr = alloca ptr addrspace(4), align 8
  %.addr1 = alloca ptr addrspace(4), align 8
  %.addr.ascast = addrspacecast ptr %.addr to ptr addrspace(4)
  %.addr1.ascast = addrspacecast ptr %.addr1 to ptr addrspace(4)
  store ptr addrspace(4) %0, ptr addrspace(4) %.addr.ascast, align 8
  store ptr addrspace(4) %1, ptr addrspace(4) %.addr1.ascast, align 8
  %2 = load ptr addrspace(4), ptr addrspace(4) %.addr1.ascast, align 8
  %3 = load ptr addrspace(4), ptr addrspace(4) %.addr.ascast, align 8
  store i32 0, ptr addrspace(4) %3, align 4
  ret void
}

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { convergent noinline nounwind "approx-func-fp-math"="true" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 64773, i32 10359099, !"_Z4main", i32 17, i32 0, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
