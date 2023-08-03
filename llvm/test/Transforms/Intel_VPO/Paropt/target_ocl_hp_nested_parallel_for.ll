; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; #if 0
; #include <stdio.h>
; #include <stdlib.h>
; #endif
;
; int main() {
;   int a[100] = {0};
; #pragma omp target parallel for map(from: a)
;   for (int i = 0; i < 100; ++i) {
;     int lp;
; #pragma omp parallel for collapse(2) lastprivate(lp)
;     for (int j = 0; j < 100; ++j)
;       for (int k = 0; k < 100; ++k)
;         lp = j + k;
;
;     a[i] = lp;
;   }
; #if 0
;   for (int i = 0; i < 100; ++i)
;     if (a[i] != 198) {
;       printf("a[%d] == %d != 198\n", i, a[i]);
;       exit(1);
;     }
; #endif
;   return 0;
; }

; Check that the inner parallel-for is not partitioned across
; WGs/WIs:
; CHECK: call{{.*}}get_global_id
; CHECK-NOT: get_local_id
; CHECK-NOT: get_local_size

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define hidden i32 @main() {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %a = alloca [100 x i32], align 4
  %a.ascast = addrspacecast ptr %a to ptr addrspace(4)
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i = alloca i32, align 4
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %lp = alloca i32, align 4
  %lp.ascast = addrspacecast ptr %lp to ptr addrspace(4)
  %tmp1 = alloca i32, align 4
  %tmp1.ascast = addrspacecast ptr %tmp1 to ptr addrspace(4)
  %tmp2 = alloca i32, align 4
  %tmp2.ascast = addrspacecast ptr %tmp2 to ptr addrspace(4)
  %.omp.uncollapsed.iv = alloca i32, align 4
  %.omp.uncollapsed.iv.ascast = addrspacecast ptr %.omp.uncollapsed.iv to ptr addrspace(4)
  %.omp.uncollapsed.iv3 = alloca i32, align 4
  %.omp.uncollapsed.iv3.ascast = addrspacecast ptr %.omp.uncollapsed.iv3 to ptr addrspace(4)
  %.omp.uncollapsed.lb = alloca i32, align 4
  %.omp.uncollapsed.lb.ascast = addrspacecast ptr %.omp.uncollapsed.lb to ptr addrspace(4)
  %.omp.uncollapsed.ub = alloca i32, align 4
  %.omp.uncollapsed.ub.ascast = addrspacecast ptr %.omp.uncollapsed.ub to ptr addrspace(4)
  %.omp.uncollapsed.lb4 = alloca i32, align 4
  %.omp.uncollapsed.lb4.ascast = addrspacecast ptr %.omp.uncollapsed.lb4 to ptr addrspace(4)
  %.omp.uncollapsed.ub5 = alloca i32, align 4
  %.omp.uncollapsed.ub5.ascast = addrspacecast ptr %.omp.uncollapsed.ub5 to ptr addrspace(4)
  %j = alloca i32, align 4
  %j.ascast = addrspacecast ptr %j to ptr addrspace(4)
  %k = alloca i32, align 4
  %k.ascast = addrspacecast ptr %k to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  call void @llvm.memset.p4.i64(ptr addrspace(4) align 4 %a.ascast, i8 0, i64 400, i1 false)
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 99, ptr addrspace(4) %.omp.ub.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.FROM"(ptr addrspace(4) %a.ascast, ptr addrspace(4) %a.ascast, i64 400, i64 34),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %lp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp1.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i32 100),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %lp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp1.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1) ]

  %2 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %2, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %4 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.uncollapsed.lb.ascast, align 4
  store i32 99, ptr addrspace(4) %.omp.uncollapsed.ub.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.uncollapsed.lb4.ascast, align 4
  store i32 99, ptr addrspace(4) %.omp.uncollapsed.ub5.ascast, align 4
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.COLLAPSE"(i32 2),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr addrspace(4) %lp.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i32 0, ptr addrspace(4) %.omp.uncollapsed.iv3.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i32 0, ptr addrspace(4) %.omp.uncollapsed.ub5.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1) ]

  %7 = load i32, ptr addrspace(4) %.omp.uncollapsed.lb.ascast, align 4
  store i32 %7, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc16, %omp.inner.for.body
  %8 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  %9 = load i32, ptr addrspace(4) %.omp.uncollapsed.ub.ascast, align 4
  %cmp6 = icmp sle i32 %8, %9
  br i1 %cmp6, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end18

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %10 = load i32, ptr addrspace(4) %.omp.uncollapsed.lb4.ascast, align 4
  store i32 %10, ptr addrspace(4) %.omp.uncollapsed.iv3.ascast, align 4
  br label %omp.uncollapsed.loop.cond7

omp.uncollapsed.loop.cond7:                       ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body
  %11 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv3.ascast, align 4
  %12 = load i32, ptr addrspace(4) %.omp.uncollapsed.ub5.ascast, align 4
  %cmp8 = icmp sle i32 %11, %12
  br i1 %cmp8, label %omp.uncollapsed.loop.body9, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body9:                       ; preds = %omp.uncollapsed.loop.cond7
  %13 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  %mul10 = mul nsw i32 %13, 1
  %add11 = add nsw i32 0, %mul10
  store i32 %add11, ptr addrspace(4) %j.ascast, align 4
  %14 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv3.ascast, align 4
  %mul12 = mul nsw i32 %14, 1
  %add13 = add nsw i32 0, %mul12
  store i32 %add13, ptr addrspace(4) %k.ascast, align 4
  %15 = load i32, ptr addrspace(4) %j.ascast, align 4
  %16 = load i32, ptr addrspace(4) %k.ascast, align 4
  %add14 = add nsw i32 %15, %16
  store i32 %add14, ptr addrspace(4) %lp.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.uncollapsed.loop.body9
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.body.continue
  %17 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv3.ascast, align 4
  %add15 = add nsw i32 %17, 1
  store i32 %add15, ptr addrspace(4) %.omp.uncollapsed.iv3.ascast, align 4
  br label %omp.uncollapsed.loop.cond7

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond7
  br label %omp.uncollapsed.loop.inc16

omp.uncollapsed.loop.inc16:                       ; preds = %omp.uncollapsed.loop.end
  %18 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  %add17 = add nsw i32 %18, 1
  store i32 %add17, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end18:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %19 = load i32, ptr addrspace(4) %lp.ascast, align 4
  %20 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom = sext i32 %20 to i64
  %arrayidx = getelementptr inbounds [100 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 %idxprom
  store i32 %19, ptr addrspace(4) %arrayidx, align 4
  br label %omp.body.continue19

omp.body.continue19:                              ; preds = %omp.uncollapsed.loop.end18
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue19
  %21 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add20 = add nsw i32 %21, 1
  store i32 %add20, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}

declare void @llvm.memset.p4.i64(ptr addrspace(4) nocapture writeonly, i8, i64, i1 immarg)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2054, i32 1856647, !"_Z4main", i32 8, i32 0, i32 0}
