; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -S %s | FileCheck %s -check-prefix=OCLPREFETCH
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring)' -switch-to-offload -S %s | FileCheck %s -check-prefix=OCLPREFETCH

; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-data-prefetch-kind=1 -S %s | FileCheck %s -check-prefix=LSCPREFETCH
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring)' -switch-to-offload -vpo-paropt-data-prefetch-kind=1 -S %s | FileCheck %s -check-prefix=LSCPREFETCH

;
; It tests data prefetch generation for PVC
;
; #include<math.h>
; void foo(int *ptr, int a[], int cond) {
;   #pragma omp target teams distribute parallel for
;   for (int i = 0; i < 1024; i++) {
;      #pragma omp prefetch data(&a[i]:1:32) if(i%32==0)
;      ptr[i] = sinf(a[i]);
;   }
; }
;

;
; Check prefetch code generation that uses OpenCL built-in prefetch API
; OCLPREFETCH: call spir_func void @__builtin_spirv_OpenCL_prefetch_p1i32_i64(i32 addrspace(4)* addrspace(4)* [[PTR:%[A-Za-z0-9_.]+]], i64 32)
;
; Check prefetch code generation that uses LSC built-in prefetch API
; LSCPREFETCH: call spir_func void @__builtin_IB_lsc_prefetch_global_uint(i32 addrspace(4)* addrspace(4)* %.tmp.prefetch.ascast, i32 32, i32 1)
;

; ModuleID = 'target_prefetch.cpp'
source_filename = "target_prefetch.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@"@tid.addr" = external global i32

; Function Attrs: convergent mustprogress noinline nounwind
define protected spir_func void @_Z3fooPiS_i(i32 addrspace(4)* noundef %ptr, i32 addrspace(4)* noundef %a, i32 noundef %cond) #0 {
entry:
  %ptr.addr = alloca i32 addrspace(4)*, align 8
  %a.addr = alloca i32 addrspace(4)*, align 8
  %cond.addr = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %a.map.ptr.tmp = alloca i32 addrspace(4)*, align 8
  %ptr.map.ptr.tmp = alloca i32 addrspace(4)*, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %.tmp.prefetch = alloca i32 addrspace(4)*, align 8
  %ptr.addr.ascast = addrspacecast i32 addrspace(4)** %ptr.addr to i32 addrspace(4)* addrspace(4)*
  %a.addr.ascast = addrspacecast i32 addrspace(4)** %a.addr to i32 addrspace(4)* addrspace(4)*
  %cond.addr.ascast = addrspacecast i32* %cond.addr to i32 addrspace(4)*
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %a.map.ptr.tmp.ascast = addrspacecast i32 addrspace(4)** %a.map.ptr.tmp to i32 addrspace(4)* addrspace(4)*
  %ptr.map.ptr.tmp.ascast = addrspacecast i32 addrspace(4)** %ptr.map.ptr.tmp to i32 addrspace(4)* addrspace(4)*
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %.tmp.prefetch.ascast = addrspacecast i32 addrspace(4)** %.tmp.prefetch to i32 addrspace(4)* addrspace(4)*
  store i32 addrspace(4)* %ptr, i32 addrspace(4)* addrspace(4)* %ptr.addr.ascast, align 8, !tbaa !8
  store i32 addrspace(4)* %a, i32 addrspace(4)* addrspace(4)* %a.addr.ascast, align 8, !tbaa !8
  store i32 %cond, i32 addrspace(4)* %cond.addr.ascast, align 4, !tbaa !12
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4, !tbaa !12
  store i32 1023, i32 addrspace(4)* %.omp.ub.ascast, align 4, !tbaa !12
  %0 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %a.addr.ascast, align 8, !tbaa !8
  %1 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %ptr.addr.ascast, align 8, !tbaa !8
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %0, i32 addrspace(4)* %0, i64 0, i64 544, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %1, i32 addrspace(4)* %1, i64 0, i64 544, i8* null, i8* null), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspace(4)* %a.map.ptr.tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspace(4)* %ptr.map.ptr.tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspace(4)* %.tmp.prefetch.ascast) ]
  store i32 addrspace(4)* %0, i32 addrspace(4)* addrspace(4)* %a.map.ptr.tmp.ascast, align 8
  store i32 addrspace(4)* %1, i32 addrspace(4)* addrspace(4)* %ptr.map.ptr.tmp.ascast, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspace(4)* %ptr.map.ptr.tmp.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspace(4)* %a.map.ptr.tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspace(4)* %.tmp.prefetch.ascast) ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspace(4)* %ptr.map.ptr.tmp.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspace(4)* %a.map.ptr.tmp.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspace(4)* %.tmp.prefetch.ascast) ]
  %5 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4, !tbaa !12
  store i32 %5, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !12
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !12
  %7 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4, !tbaa !12
  %cmp = icmp sle i32 %6, %7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !12
  %mul = mul nsw i32 %8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4, !tbaa !12
  %9 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %a.map.ptr.tmp.ascast, align 8, !tbaa !8
  %10 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !12
  %idxprom = sext i32 %10 to i64
  %arrayidx = getelementptr inbounds i32, i32 addrspace(4)* %9, i64 %idxprom
  store i32 addrspace(4)* %arrayidx, i32 addrspace(4)* addrspace(4)* %.tmp.prefetch.ascast, align 8, !tbaa !8
  %11 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !12
  %rem = srem i32 %11, 32
  %cmp1 = icmp eq i32 %rem, 0
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.PREFETCH"(), "QUAL.OMP.DATA"(i32 addrspace(4)* addrspace(4)* %.tmp.prefetch.ascast, i32 1, i64 32), "QUAL.OMP.IF"(i1 %cmp1) ]
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.PREFETCH"() ]
  %13 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %a.map.ptr.tmp.ascast, align 8, !tbaa !8
  %14 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !12
  %idxprom2 = sext i32 %14 to i64
  %arrayidx3 = getelementptr inbounds i32, i32 addrspace(4)* %13, i64 %idxprom2
  %15 = load i32, i32 addrspace(4)* %arrayidx3, align 4, !tbaa !12
  %conv = sitofp i32 %15 to float
  %16 = call fast float @llvm.sin.f32(float %conv) #3
  %conv4 = fptosi float %16 to i32
  %17 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %ptr.map.ptr.tmp.ascast, align 8, !tbaa !8
  %18 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !12
  %idxprom5 = sext i32 %18 to i64
  %arrayidx6 = getelementptr inbounds i32, i32 addrspace(4)* %17, i64 %idxprom5
  store i32 %conv4, i32 addrspace(4)* %arrayidx6, align 4, !tbaa !12
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %19 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !12
  %add7 = add nsw i32 %19, 1
  store i32 %add7, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !12
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare float @llvm.sin.f32(float) #2

declare spir_func void @__builtin_spirv_OpenCL_prefetch_p1i32_i64(i32 addrspace(4)* addrspace(4)*, i64)

attributes #0 = { convergent mustprogress noinline nounwind "approx-func-fp-math"="true" "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { convergent }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}

!0 = !{i32 0, i32 64770, i32 13507765, !"_Z3fooPiS_i", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = !{!"clang version 14.0.0"}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C++ TBAA"}
!12 = !{!13, !13, i64 0}
!13 = !{!"pointer@_ZTSPi", !10, i64 0}
