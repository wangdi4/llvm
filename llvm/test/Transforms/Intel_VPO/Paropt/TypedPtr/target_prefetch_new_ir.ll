; RUN: opt -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -S %s 2>&1 | FileCheck %s -check-prefix=OCLPREFETCH
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring)' -switch-to-offload -S %s 2>&1 | FileCheck %s -check-prefix=OCLPREFETCH

; RUN: opt -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-data-prefetch-kind=1 -S %s 2>&1 | FileCheck %s -check-prefix=LSCPREFETCH
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring)' -switch-to-offload -vpo-paropt-data-prefetch-kind=1 -S %s 2>&1 | FileCheck %s -check-prefix=LSCPREFETCH

; Test src:
;
; #include <math.h>
; void foo(int *ptr, int a[], __float128 b[], double c[], int n) {
; #pragma omp target teams distribute parallel for
;   for (int i = 0; i < 1024; i++) {
; #pragma ompx prefetch data(1 : a [i:32]) data(b [i:99]) data(3 : c [i:n]) if (i % 32 == 0)
;     ptr[i] = sinf(a[i]);
;   }
; }

; __float128 b[] should be ignored because the data type is not supported by SPIRV prefetch API.

; Check prefetch code generation that uses OpenCL built-in prefetch API
; OCLPREFETCH: warning: {{.*}} A 'data' clause in the 'prefetch' construct was ignored. SPIRV OpenCL prefetch API doesn't support its element type: fp128.
; OCLPREFETCH: call spir_func void @__builtin_spirv_OpenCL_prefetch_p1i32_i64(i32 addrspace(4)* %arrayidx, i64 %sec.number_of_elements)
; OCLPREFETCH-NEXT: call spir_func void @__builtin_spirv_OpenCL_prefetch_p1f64_i64(double addrspace(4)* %arrayidx8, i64 %sec.number_of_elements13)

; Check prefetch code generation that uses LSC built-in prefetch API
; LSCPREFETCH: warning: {{.*}} A 'data' clause in the 'prefetch' construct was ignored. SPIRV LSC prefetch API doesn't support its element type: fp128.
; LSCPREFETCH: call spir_func void @__builtin_IB_lsc_prefetch_global_uint(i32 addrspace(4)* %arrayidx, i32 0, i32 1)
; LSCPREFETCH-NEXT: [[LSC_PTR:%[A-Za-z0-9_.]+]] = bitcast double addrspace(4)* %arrayidx8 to i64 addrspace(4)*
; LSCPREFETCH-NEXT: call spir_func void @__builtin_IB_lsc_prefetch_global_ulong(i64 addrspace(4)* [[LSC_PTR]], i32 0, i32 3)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind optnone
define protected spir_func void @foo(i32 addrspace(4)* noundef %ptr, i32 addrspace(4)* noundef %a, fp128 addrspace(4)* noundef %b, double addrspace(4)* noundef %c, i32 noundef %n) #0 {
entry:
  %ptr.addr = alloca i32 addrspace(4)*, align 8
  %a.addr = alloca i32 addrspace(4)*, align 8
  %b.addr = alloca fp128 addrspace(4)*, align 8
  %c.addr = alloca double addrspace(4)*, align 8
  %n.addr = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %a.map.ptr.tmp = alloca i32 addrspace(4)*, align 8
  %b.map.ptr.tmp = alloca fp128 addrspace(4)*, align 8
  %c.map.ptr.tmp = alloca double addrspace(4)*, align 8
  %ptr.map.ptr.tmp = alloca i32 addrspace(4)*, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %ptr.addr.ascast = addrspacecast i32 addrspace(4)** %ptr.addr to i32 addrspace(4)* addrspace(4)*
  %a.addr.ascast = addrspacecast i32 addrspace(4)** %a.addr to i32 addrspace(4)* addrspace(4)*
  %b.addr.ascast = addrspacecast fp128 addrspace(4)** %b.addr to fp128 addrspace(4)* addrspace(4)*
  %c.addr.ascast = addrspacecast double addrspace(4)** %c.addr to double addrspace(4)* addrspace(4)*
  %n.addr.ascast = addrspacecast i32* %n.addr to i32 addrspace(4)*
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %a.map.ptr.tmp.ascast = addrspacecast i32 addrspace(4)** %a.map.ptr.tmp to i32 addrspace(4)* addrspace(4)*
  %b.map.ptr.tmp.ascast = addrspacecast fp128 addrspace(4)** %b.map.ptr.tmp to fp128 addrspace(4)* addrspace(4)*
  %c.map.ptr.tmp.ascast = addrspacecast double addrspace(4)** %c.map.ptr.tmp to double addrspace(4)* addrspace(4)*
  %ptr.map.ptr.tmp.ascast = addrspacecast i32 addrspace(4)** %ptr.map.ptr.tmp to i32 addrspace(4)* addrspace(4)*
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  store i32 addrspace(4)* %ptr, i32 addrspace(4)* addrspace(4)* %ptr.addr.ascast, align 8
  store i32 addrspace(4)* %a, i32 addrspace(4)* addrspace(4)* %a.addr.ascast, align 8
  store fp128 addrspace(4)* %b, fp128 addrspace(4)* addrspace(4)* %b.addr.ascast, align 8
  store double addrspace(4)* %c, double addrspace(4)* addrspace(4)* %c.addr.ascast, align 8
  store i32 %n, i32 addrspace(4)* %n.addr.ascast, align 4
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 1023, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %0 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %a.addr.ascast, align 8
  %1 = load fp128 addrspace(4)*, fp128 addrspace(4)* addrspace(4)* %b.addr.ascast, align 8
  %2 = load double addrspace(4)*, double addrspace(4)* addrspace(4)* %c.addr.ascast, align 8
  %3 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %ptr.addr.ascast, align 8
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %0, i32 addrspace(4)* %0, i64 0, i64 544, i8* null, i8* null),
    "QUAL.OMP.MAP.TOFROM"(fp128 addrspace(4)* %1, fp128 addrspace(4)* %1, i64 0, i64 544, i8* null, i8* null),
    "QUAL.OMP.MAP.TOFROM"(double addrspace(4)* %2, double addrspace(4)* %2, i64 0, i64 544, i8* null, i8* null),
    "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %3, i32 addrspace(4)* %3, i64 0, i64 544, i8* null, i8* null),
    "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %n.addr.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast),
    "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast),
    "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspace(4)* %a.map.ptr.tmp.ascast),
    "QUAL.OMP.PRIVATE"(fp128 addrspace(4)* addrspace(4)* %b.map.ptr.tmp.ascast),
    "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspace(4)* %c.map.ptr.tmp.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspace(4)* %ptr.map.ptr.tmp.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  store i32 addrspace(4)* %0, i32 addrspace(4)* addrspace(4)* %a.map.ptr.tmp.ascast, align 8
  store fp128 addrspace(4)* %1, fp128 addrspace(4)* addrspace(4)* %b.map.ptr.tmp.ascast, align 8
  store double addrspace(4)* %2, double addrspace(4)* addrspace(4)* %c.map.ptr.tmp.ascast, align 8
  store i32 addrspace(4)* %3, i32 addrspace(4)* addrspace(4)* %ptr.map.ptr.tmp.ascast, align 8
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspace(4)* %ptr.map.ptr.tmp.ascast),
    "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspace(4)* %a.map.ptr.tmp.ascast),
    "QUAL.OMP.SHARED"(fp128 addrspace(4)* addrspace(4)* %b.map.ptr.tmp.ascast),
    "QUAL.OMP.SHARED"(double addrspace(4)* addrspace(4)* %c.map.ptr.tmp.ascast),
    "QUAL.OMP.SHARED"(i32 addrspace(4)* %n.addr.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast),
    "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast),
    "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspace(4)* %ptr.map.ptr.tmp.ascast),
    "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspace(4)* %a.map.ptr.tmp.ascast),
    "QUAL.OMP.SHARED"(fp128 addrspace(4)* addrspace(4)* %b.map.ptr.tmp.ascast),
    "QUAL.OMP.SHARED"(double addrspace(4)* addrspace(4)* %c.map.ptr.tmp.ascast),
    "QUAL.OMP.SHARED"(i32 addrspace(4)* %n.addr.ascast),
    "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast),
    "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast),
    "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast) ]
  %7 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %7, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %8 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %9 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %8, %9
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %10, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  %11 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %12 = sext i32 %11 to i64
  %13 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %a.map.ptr.tmp.ascast, align 8
  %arrayidx = getelementptr inbounds i32, i32 addrspace(4)* %13, i64 %12
  %sec.lower.cast = ptrtoint i32 addrspace(4)* %arrayidx to i64
  %14 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %15 = sext i32 %14 to i64
  %lb_add_len = add nsw i64 %15, 31
  %16 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %a.map.ptr.tmp.ascast, align 8
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(4)* %16, i64 %lb_add_len
  %sec.upper.cast = ptrtoint i32 addrspace(4)* %arrayidx1 to i64
  %17 = sub i64 %sec.upper.cast, %sec.lower.cast
  %18 = sdiv exact i64 %17, 8
  %sec.number_of_elements = add i64 %18, 1
  %19 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %20 = sext i32 %19 to i64
  %21 = load fp128 addrspace(4)*, fp128 addrspace(4)* addrspace(4)* %b.map.ptr.tmp.ascast, align 8
  %arrayidx2 = getelementptr inbounds fp128, fp128 addrspace(4)* %21, i64 %20
  %sec.lower.cast3 = ptrtoint fp128 addrspace(4)* %arrayidx2 to i64
  %22 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %23 = sext i32 %22 to i64
  %lb_add_len4 = add nsw i64 %23, 98
  %24 = load fp128 addrspace(4)*, fp128 addrspace(4)* addrspace(4)* %b.map.ptr.tmp.ascast, align 8
  %arrayidx5 = getelementptr inbounds fp128, fp128 addrspace(4)* %24, i64 %lb_add_len4
  %sec.upper.cast6 = ptrtoint fp128 addrspace(4)* %arrayidx5 to i64
  %25 = sub i64 %sec.upper.cast6, %sec.lower.cast3
  %26 = sdiv exact i64 %25, 8
  %sec.number_of_elements7 = add i64 %26, 1
  %27 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %28 = sext i32 %27 to i64
  %29 = load double addrspace(4)*, double addrspace(4)* addrspace(4)* %c.map.ptr.tmp.ascast, align 8
  %arrayidx8 = getelementptr inbounds double, double addrspace(4)* %29, i64 %28
  %sec.lower.cast9 = ptrtoint double addrspace(4)* %arrayidx8 to i64
  %30 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %31 = sext i32 %30 to i64
  %32 = load i32, i32 addrspace(4)* %n.addr.ascast, align 4
  %33 = sext i32 %32 to i64
  %lb_add_len10 = add nsw i64 %31, %33
  %idx_sub_1 = sub nsw i64 %lb_add_len10, 1
  %34 = load double addrspace(4)*, double addrspace(4)* addrspace(4)* %c.map.ptr.tmp.ascast, align 8
  %arrayidx11 = getelementptr inbounds double, double addrspace(4)* %34, i64 %idx_sub_1
  %sec.upper.cast12 = ptrtoint double addrspace(4)* %arrayidx11 to i64
  %35 = sub i64 %sec.upper.cast12, %sec.lower.cast9
  %36 = sdiv exact i64 %35, 8
  %sec.number_of_elements13 = add i64 %36, 1
  %37 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %rem = srem i32 %37, 32
  %cmp14 = icmp eq i32 %rem, 0
  %38 = call token @llvm.directive.region.entry() [ "DIR.OMP.PREFETCH"(),
    "QUAL.OMP.DATA"(i32 addrspace(4)* %arrayidx, i32 0, i64 %sec.number_of_elements, i32 1),
    "QUAL.OMP.DATA"(fp128 addrspace(4)* %arrayidx2, fp128 0xL00000000000000000000000000000000, i64 %sec.number_of_elements7, i32 0),
    "QUAL.OMP.DATA"(double addrspace(4)* %arrayidx8, double 0.000000e+00, i64 %sec.number_of_elements13, i32 3),
    "QUAL.OMP.IF"(i1 %cmp14) ]
  call void @llvm.directive.region.exit(token %38) [ "DIR.OMP.END.PREFETCH"() ]
  %39 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %a.map.ptr.tmp.ascast, align 8
  %40 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom = sext i32 %40 to i64
  %arrayidx15 = getelementptr inbounds i32, i32 addrspace(4)* %39, i64 %idxprom
  %41 = load i32, i32 addrspace(4)* %arrayidx15, align 4
  %conv = sitofp i32 %41 to float
  %42 = call fast float @llvm.sin.f32(float %conv) #3
  %conv16 = fptosi float %42 to i32
  %43 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %ptr.map.ptr.tmp.ascast, align 8
  %44 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom17 = sext i32 %44 to i64
  %arrayidx18 = getelementptr inbounds i32, i32 addrspace(4)* %43, i64 %idxprom17
  store i32 %conv16, i32 addrspace(4)* %arrayidx18, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %45 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add19 = add nsw i32 %45, 1
  store i32 %add19, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET"() ]
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
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 53, i32 -1928024417, !"_Z3foo", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
