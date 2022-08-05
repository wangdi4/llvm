; RUN: opt -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -S %s 2>&1 | FileCheck %s -check-prefix=OCLPREFETCH
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring)' -switch-to-offload -S %s 2>&1 | FileCheck %s -check-prefix=OCLPREFETCH

; RUN: opt -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-data-prefetch-kind=1 -S %s 2>&1 | FileCheck %s -check-prefix=LSCPREFETCH
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring)' -switch-to-offload -vpo-paropt-data-prefetch-kind=1 -S %s 2>&1 | FileCheck %s -check-prefix=LSCPREFETCH

; Test src:
;
; #include <math.h>
; void foo(int *ptr, int a[], __float128 b[], double c[], int n, int cond) {
; #pragma omp target teams distribute parallel for
;   for (int i = 0; i < 1024; i++) {
; // New syntax: #pragma ompx prefetch data(1 : a [i:32]) data(2 : b [i:99]) data(3 : c [i:n]) if (i % 32 == 0)
; #pragma omp prefetch data(&a[i] : 1 : 32) data(&b[i] : 2 : 99) data(&c[i] : 3 : 20) if (i % 32 == 0)
;     ptr[i] = sinf(a[i]);
;   }
; }

; The IR is a hand-modified version from the old syntax.
; FE hasn't supported the new syntax of DATA clause shown in the source comment.
; __float128 b[] should be ignored because the data type is not supported by SPIRV prefetch API.

; Check prefetch code generation that uses OpenCL built-in prefetch API
; OCLPREFETCH: warning: {{.*}} A 'data' clause in the 'prefetch' construct was ignored. SPIRV OpenCL prefetch API doesn't support its element type: fp128.
; OCLPREFETCH: call spir_func void @__builtin_spirv_OpenCL_prefetch_p1i32_i64(ptr addrspace(4) %.tmp.prefetch.ascast, i64 32)
; OCLPREFETCH-NEXT: [[NUM_ELEMENTS:%[A-Za-z0-9_.]+]] = sext i32 %n to i64
; OCLPREFETCH-NEXT: call spir_func void @__builtin_spirv_OpenCL_prefetch_p1f64_i64(ptr addrspace(4) %.tmp.prefetch4.ascast, i64 [[NUM_ELEMENTS]])

; Check prefetch code generation that uses LSC built-in prefetch API
; LSCPREFETCH: warning: {{.*}} A 'data' clause in the 'prefetch' construct was ignored. SPIRV LSC prefetch API doesn't support its element type: fp128.
; LSCPREFETCH: call spir_func void @__builtin_IB_lsc_prefetch_global_uint(ptr addrspace(4) %.tmp.prefetch.ascast, i32 0, i32 1)
; LSCPREFETCH-NEXT: call spir_func void @__builtin_IB_lsc_prefetch_global_ulong(ptr addrspace(4) %.tmp.prefetch4.ascast, i32 0, i32 3)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind
define protected spir_func void @foo(ptr addrspace(4) noundef %ptr, ptr addrspace(4) noundef %a, ptr addrspace(4) noundef %b, ptr addrspace(4) noundef %c, i32 noundef %n, i32 noundef %cond) #0 {
entry:
  %ptr.addr = alloca ptr addrspace(4), align 8
  %a.addr = alloca ptr addrspace(4), align 8
  %b.addr = alloca ptr addrspace(4), align 8
  %c.addr = alloca ptr addrspace(4), align 8
  %n.addr = alloca i32, align 4
  %cond.addr = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %a.map.ptr.tmp = alloca ptr addrspace(4), align 8
  %b.map.ptr.tmp = alloca ptr addrspace(4), align 8
  %c.map.ptr.tmp = alloca ptr addrspace(4), align 8
  %ptr.map.ptr.tmp = alloca ptr addrspace(4), align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %.tmp.prefetch = alloca ptr addrspace(4), align 8
  %.tmp.prefetch1 = alloca ptr addrspace(4), align 8
  %.tmp.prefetch4 = alloca ptr addrspace(4), align 8
  %ptr.addr.ascast = addrspacecast ptr %ptr.addr to ptr addrspace(4)
  %a.addr.ascast = addrspacecast ptr %a.addr to ptr addrspace(4)
  %b.addr.ascast = addrspacecast ptr %b.addr to ptr addrspace(4)
  %c.addr.ascast = addrspacecast ptr %c.addr to ptr addrspace(4)
  %n.addr.ascast = addrspacecast ptr %n.addr to ptr addrspace(4)
  %cond.addr.ascast = addrspacecast ptr %cond.addr to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %a.map.ptr.tmp.ascast = addrspacecast ptr %a.map.ptr.tmp to ptr addrspace(4)
  %b.map.ptr.tmp.ascast = addrspacecast ptr %b.map.ptr.tmp to ptr addrspace(4)
  %c.map.ptr.tmp.ascast = addrspacecast ptr %c.map.ptr.tmp to ptr addrspace(4)
  %ptr.map.ptr.tmp.ascast = addrspacecast ptr %ptr.map.ptr.tmp to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %.tmp.prefetch.ascast = addrspacecast ptr %.tmp.prefetch to ptr addrspace(4)
  %.tmp.prefetch1.ascast = addrspacecast ptr %.tmp.prefetch1 to ptr addrspace(4)
  %.tmp.prefetch4.ascast = addrspacecast ptr %.tmp.prefetch4 to ptr addrspace(4)
  store ptr addrspace(4) %ptr, ptr addrspace(4) %ptr.addr.ascast, align 8, !tbaa !8
  store ptr addrspace(4) %a, ptr addrspace(4) %a.addr.ascast, align 8, !tbaa !8
  store ptr addrspace(4) %b, ptr addrspace(4) %b.addr.ascast, align 8, !tbaa !12
  store ptr addrspace(4) %c, ptr addrspace(4) %c.addr.ascast, align 8, !tbaa !14
  store i32 %n, ptr addrspace(4) %n.addr.ascast, align 4, !tbaa !16
  store i32 %cond, ptr addrspace(4) %cond.addr.ascast, align 4, !tbaa !16
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4, !tbaa !16
  store i32 1023, ptr addrspace(4) %.omp.ub.ascast, align 4, !tbaa !16
  %0 = load ptr addrspace(4), ptr addrspace(4) %a.addr.ascast, align 8, !tbaa !8
  %1 = load ptr addrspace(4), ptr addrspace(4) %b.addr.ascast, align 8, !tbaa !12
  %2 = load ptr addrspace(4), ptr addrspace(4) %c.addr.ascast, align 8, !tbaa !14
  %3 = load ptr addrspace(4), ptr addrspace(4) %ptr.addr.ascast, align 8, !tbaa !8
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %0, ptr addrspace(4) %0, i64 0, i64 544, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %1, ptr addrspace(4) %1, i64 0, i64 544, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %2, ptr addrspace(4) %2, i64 0, i64 544, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %3, ptr addrspace(4) %3, i64 0, i64 544, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %a.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %b.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %c.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %ptr.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.tmp.prefetch.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.tmp.prefetch1.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.tmp.prefetch4.ascast, ptr addrspace(4) null, i32 1) ]
  store ptr addrspace(4) %0, ptr addrspace(4) %a.map.ptr.tmp.ascast, align 8
  store ptr addrspace(4) %1, ptr addrspace(4) %b.map.ptr.tmp.ascast, align 8
  store ptr addrspace(4) %2, ptr addrspace(4) %c.map.ptr.tmp.ascast, align 8
  store ptr addrspace(4) %3, ptr addrspace(4) %ptr.map.ptr.tmp.ascast, align 8
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %ptr.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %c.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.tmp.prefetch.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.tmp.prefetch1.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.tmp.prefetch4.ascast, ptr addrspace(4) null, i32 1) ]
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %ptr.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %c.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.tmp.prefetch.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.tmp.prefetch1.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.tmp.prefetch4.ascast, ptr addrspace(4) null, i32 1) ]
  %7 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4, !tbaa !16
  store i32 %7, ptr addrspace(4) %.omp.iv.ascast, align 4, !tbaa !16
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %8 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4, !tbaa !16
  %9 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4, !tbaa !16
  %cmp = icmp sle i32 %8, %9
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4, !tbaa !16
  %mul = mul nsw i32 %10, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4, !tbaa !16
  %11 = load ptr addrspace(4), ptr addrspace(4) %a.map.ptr.tmp.ascast, align 8, !tbaa !8
  %12 = load i32, ptr addrspace(4) %i.ascast, align 4, !tbaa !16
  %idxprom = sext i32 %12 to i64
  %arrayidx = getelementptr inbounds i32, ptr addrspace(4) %11, i64 %idxprom
  store ptr addrspace(4) %arrayidx, ptr addrspace(4) %.tmp.prefetch.ascast, align 8, !tbaa !8
  %13 = load ptr addrspace(4), ptr addrspace(4) %b.map.ptr.tmp.ascast, align 8, !tbaa !12
  %14 = load i32, ptr addrspace(4) %i.ascast, align 4, !tbaa !16
  %idxprom2 = sext i32 %14 to i64
  %arrayidx3 = getelementptr inbounds fp128, ptr addrspace(4) %13, i64 %idxprom2
  store ptr addrspace(4) %arrayidx3, ptr addrspace(4) %.tmp.prefetch1.ascast, align 8, !tbaa !12
  %15 = load ptr addrspace(4), ptr addrspace(4) %c.map.ptr.tmp.ascast, align 8, !tbaa !14
  %16 = load i32, ptr addrspace(4) %i.ascast, align 4, !tbaa !16
  %idxprom5 = sext i32 %16 to i64
  %arrayidx6 = getelementptr inbounds double, ptr addrspace(4) %15, i64 %idxprom5
  store ptr addrspace(4) %arrayidx6, ptr addrspace(4) %.tmp.prefetch4.ascast, align 8, !tbaa !14
  %17 = load i32, ptr addrspace(4) %i.ascast, align 4, !tbaa !16
  %rem = srem i32 %17, 32
  %cmp7 = icmp eq i32 %rem, 0
  %18 = call token @llvm.directive.region.entry() [ "DIR.OMP.PREFETCH"(),
    "QUAL.OMP.DATA"(ptr addrspace(4) %.tmp.prefetch.ascast, i32 0, i64 32, i32 1),
    "QUAL.OMP.DATA"(ptr addrspace(4) %.tmp.prefetch1.ascast, fp128 0xL00000000000000000000000000000000, i64 99, i32 2),
    "QUAL.OMP.DATA"(ptr addrspace(4) %.tmp.prefetch4.ascast, double 0.000000e+00, i32 %n, i32 3),
    "QUAL.OMP.IF"(i1 %cmp7) ]
  call void @llvm.directive.region.exit(token %18) [ "DIR.OMP.END.PREFETCH"() ]
  %19 = load ptr addrspace(4), ptr addrspace(4) %a.map.ptr.tmp.ascast, align 8, !tbaa !8
  %20 = load i32, ptr addrspace(4) %i.ascast, align 4, !tbaa !16
  %idxprom8 = sext i32 %20 to i64
  %arrayidx9 = getelementptr inbounds i32, ptr addrspace(4) %19, i64 %idxprom8
  %21 = load i32, ptr addrspace(4) %arrayidx9, align 4, !tbaa !16
  %conv = sitofp i32 %21 to float
  %22 = call fast float @llvm.sin.f32(float %conv) #3
  %conv10 = fptosi float %22 to i32
  %23 = load ptr addrspace(4), ptr addrspace(4) %ptr.map.ptr.tmp.ascast, align 8, !tbaa !8
  %24 = load i32, ptr addrspace(4) %i.ascast, align 4, !tbaa !16
  %idxprom11 = sext i32 %24 to i64
  %arrayidx12 = getelementptr inbounds i32, ptr addrspace(4) %23, i64 %idxprom11
  store i32 %conv10, ptr addrspace(4) %arrayidx12, align 4, !tbaa !16
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %25 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4, !tbaa !16
  %add13 = add nsw i32 %25, 1
  store i32 %add13, ptr addrspace(4) %.omp.iv.ascast, align 4, !tbaa !16
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

attributes #0 = { convergent noinline nounwind "approx-func-fp-math"="true" "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { convergent }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}

!0 = !{i32 0, i32 53, i32 -1936817302, !"_Z3foo", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!8 = !{!9, !9, i64 0}
!9 = !{!"pointer@_ZTSPi", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
!12 = !{!13, !13, i64 0}
!13 = !{!"pointer@_ZTSPg", !10, i64 0}
!14 = !{!15, !15, i64 0}
!15 = !{!"pointer@_ZTSPd", !10, i64 0}
!16 = !{!17, !17, i64 0}
!17 = !{!"int", !10, i64 0}
