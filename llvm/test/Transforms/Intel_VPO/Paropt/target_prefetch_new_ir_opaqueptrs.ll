; RUN: opt -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -S %s | FileCheck %s -check-prefix=OCLPREFETCH
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring)' -switch-to-offload -S %s | FileCheck %s -check-prefix=OCLPREFETCH

; RUN: opt -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-data-prefetch-kind=1 -S %s | FileCheck %s -check-prefix=LSCPREFETCH
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring)' -switch-to-offload -vpo-paropt-data-prefetch-kind=1 -S %s | FileCheck %s -check-prefix=LSCPREFETCH

; Test src:
;
; #include <math.h>
; void foo(int *ptr, int a[], int b[], int n, int cond) {
; #pragma omp target teams distribute parallel for
;   for (int i = 0; i < 1024; i++) {
; #pragma ompx prefetch data(1 : a [i:8]) data(2 : b [i:n]) if (i % 32 == 0)
;     ptr[i] = sinf(a[i]);
;   }
; }

; The IR is a hand-modified version because FE hasn't support the new syntax of DATA clause.

; Check prefetch code generation that uses OpenCL built-in prefetch API
; OCLPREFETCH: call spir_func void @__builtin_spirv_OpenCL_prefetch_p1i32_i64(ptr addrspace(4) %.tmp.prefetch.ascast, i64 32)
; OCLPREFETCH: [[NUM_ELEMENTS:%[A-Za-z0-9_.]+]] = sext i32 %n to i64
; OCLPREFETCH-NEXT: call spir_func void @__builtin_spirv_OpenCL_prefetch_p1i32_i64(ptr addrspace(4) %.tmp.prefetch1.ascast, i64 [[NUM_ELEMENTS]])

; Check prefetch code generation that uses LSC built-in prefetch API
; LSCPREFETCH: call spir_func void @__builtin_IB_lsc_prefetch_global_uint(ptr addrspace(4) %.tmp.prefetch.ascast, i32 32, i32 1)
; LSCPREFETCH: call spir_func void @__builtin_IB_lsc_prefetch_global_uint(ptr addrspace(4) %.tmp.prefetch1.ascast, i32 %n, i32 2)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind optnone
define protected spir_func void @foo(ptr addrspace(4) noundef %ptr, ptr addrspace(4) noundef %a, ptr addrspace(4) noundef %b, i32 noundef %n, i32 noundef %cond) #0 {
entry:
  %ptr.addr = alloca ptr addrspace(4), align 8
  %a.addr = alloca ptr addrspace(4), align 8
  %b.addr = alloca ptr addrspace(4), align 8
  %n.addr = alloca i32, align 4
  %cond.addr = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %a.map.ptr.tmp = alloca ptr addrspace(4), align 8
  %b.map.ptr.tmp = alloca ptr addrspace(4), align 8
  %ptr.map.ptr.tmp = alloca ptr addrspace(4), align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %.tmp.prefetch = alloca ptr addrspace(4), align 8
  %.tmp.prefetch1 = alloca ptr addrspace(4), align 8
  %ptr.addr.ascast = addrspacecast ptr %ptr.addr to ptr addrspace(4)
  %a.addr.ascast = addrspacecast ptr %a.addr to ptr addrspace(4)
  %b.addr.ascast = addrspacecast ptr %b.addr to ptr addrspace(4)
  %n.addr.ascast = addrspacecast ptr %n.addr to ptr addrspace(4)
  %cond.addr.ascast = addrspacecast ptr %cond.addr to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %a.map.ptr.tmp.ascast = addrspacecast ptr %a.map.ptr.tmp to ptr addrspace(4)
  %b.map.ptr.tmp.ascast = addrspacecast ptr %b.map.ptr.tmp to ptr addrspace(4)
  %ptr.map.ptr.tmp.ascast = addrspacecast ptr %ptr.map.ptr.tmp to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %.tmp.prefetch.ascast = addrspacecast ptr %.tmp.prefetch to ptr addrspace(4)
  %.tmp.prefetch1.ascast = addrspacecast ptr %.tmp.prefetch1 to ptr addrspace(4)
  store ptr addrspace(4) %ptr, ptr addrspace(4) %ptr.addr.ascast, align 8
  store ptr addrspace(4) %a, ptr addrspace(4) %a.addr.ascast, align 8
  store ptr addrspace(4) %b, ptr addrspace(4) %b.addr.ascast, align 8
  store i32 %n, ptr addrspace(4) %n.addr.ascast, align 4
  store i32 %cond, ptr addrspace(4) %cond.addr.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 1023, ptr addrspace(4) %.omp.ub.ascast, align 4
  %0 = load ptr addrspace(4), ptr addrspace(4) %a.addr.ascast, align 8
  %1 = load ptr addrspace(4), ptr addrspace(4) %b.addr.ascast, align 8
  %2 = load ptr addrspace(4), ptr addrspace(4) %ptr.addr.ascast, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %0, ptr addrspace(4) %0, i64 0, i64 544, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %1, ptr addrspace(4) %1, i64 0, i64 544, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %2, ptr addrspace(4) %2, i64 0, i64 544, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %a.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %b.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %ptr.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.tmp.prefetch.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.tmp.prefetch1.ascast, ptr addrspace(4) null, i32 1) ]
  store ptr addrspace(4) %0, ptr addrspace(4) %a.map.ptr.tmp.ascast, align 8
  store ptr addrspace(4) %1, ptr addrspace(4) %b.map.ptr.tmp.ascast, align 8
  store ptr addrspace(4) %2, ptr addrspace(4) %ptr.map.ptr.tmp.ascast, align 8
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %ptr.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.tmp.prefetch.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.tmp.prefetch1.ascast, ptr addrspace(4) null, i32 1) ]
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %ptr.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.tmp.prefetch.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.tmp.prefetch1.ascast, ptr addrspace(4) null, i32 1) ]
  %6 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %6, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %7 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %8 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %7, %8
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  %10 = load ptr addrspace(4), ptr addrspace(4) %a.map.ptr.tmp.ascast, align 8
  %11 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom = sext i32 %11 to i64
  %arrayidx = getelementptr inbounds i32, ptr addrspace(4) %10, i64 %idxprom
  store ptr addrspace(4) %arrayidx, ptr addrspace(4) %.tmp.prefetch.ascast, align 8
  %12 = load ptr addrspace(4), ptr addrspace(4) %b.map.ptr.tmp.ascast, align 8
  %13 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom2 = sext i32 %13 to i64
  %arrayidx3 = getelementptr inbounds i32, ptr addrspace(4) %12, i64 %idxprom2
  store ptr addrspace(4) %arrayidx3, ptr addrspace(4) %.tmp.prefetch1.ascast, align 8
  %14 = load i32, ptr addrspace(4) %i.ascast, align 4
  %rem = srem i32 %14, 32
  %cmp4 = icmp eq i32 %rem, 0
  %15 = call token @llvm.directive.region.entry() [ "DIR.OMP.PREFETCH"(),
    "QUAL.OMP.DATA"(ptr addrspace(4) %.tmp.prefetch.ascast, i32 0, i32 1, i64 32),
    "QUAL.OMP.DATA"(ptr addrspace(4) %.tmp.prefetch1.ascast, i32 0, i32 2, i32 %n),
    "QUAL.OMP.IF"(i1 %cmp4) ]
  call void @llvm.directive.region.exit(token %15) [ "DIR.OMP.END.PREFETCH"() ]
  %16 = load ptr addrspace(4), ptr addrspace(4) %a.map.ptr.tmp.ascast, align 8
  %17 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom5 = sext i32 %17 to i64
  %arrayidx6 = getelementptr inbounds i32, ptr addrspace(4) %16, i64 %idxprom5
  %18 = load i32, ptr addrspace(4) %arrayidx6, align 4
  %conv = sitofp i32 %18 to float
  %19 = call fast float @llvm.sin.f32(float %conv) #3
  %conv7 = fptosi float %19 to i32
  %20 = load ptr addrspace(4), ptr addrspace(4) %ptr.map.ptr.tmp.ascast, align 8
  %21 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom8 = sext i32 %21 to i64
  %arrayidx9 = getelementptr inbounds i32, ptr addrspace(4) %20, i64 %idxprom8
  store i32 %conv7, ptr addrspace(4) %arrayidx9, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %22 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add10 = add nsw i32 %22, 1
  store i32 %add10, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare float @llvm.sin.f32(float) #2

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { convergent }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}

!0 = !{i32 0, i32 53, i32 -1925405096, !"_Z3foo", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
