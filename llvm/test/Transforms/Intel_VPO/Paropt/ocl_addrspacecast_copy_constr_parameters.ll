; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:

; class A
; {
; public:
; #pragma omp declare target
;   A();
; #pragma omp end declare target
; };
; A e[100];
; void fn1() {
; #pragma omp target parallel for firstprivate(e)
;   for(int d=0; d<100; d++);
; }

; Check that we are casting the parameters of copy_constr and destr to addrspace(4).
; CHECK: [[PARAM1:%[^ ]+]] = addrspacecast %class.A* %priv.cpy.dest.ptr to %class.A addrspace(4)*
; CHECK: [[PARAM2:%[^ ]+]] = addrspacecast %class.A addrspace(1)* %priv.cpy.src.ptr to %class.A addrspace(4)*
; CHECK: call spir_func void @_ZTS1A.omp.copy_constr(%class.A addrspace(4)* [[PARAM1]], %class.A addrspace(4)* [[PARAM2]])
; CHECK: [[PARAM3:%[^ ]+]] = addrspacecast %class.A* %priv.cpy.dest.ptr26 to %class.A addrspace(4)*
; CHECK: call spir_func void @_ZTS1A.omp.destr(%class.A addrspace(4)* [[PARAM3]])

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%class.A = type { i8 }

@e = external dso_local addrspace(1) global [100 x %class.A], align 1

; Function Attrs: convergent mustprogress noinline nounwind optnone uwtable
define hidden spir_func void @_Z3fn1v() #0 {
entry:
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %d = alloca i32, align 4
  %d.ascast = addrspacecast i32* %d to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 99, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE:NONPOD"([100 x %class.A] addrspace(4)* addrspacecast ([100 x %class.A] addrspace(1)* @e to [100 x %class.A] addrspace(4)*), void (%class.A addrspace(4)*, %class.A addrspace(4)*)* @_ZTS1A.omp.copy_constr, void (%class.A addrspace(4)*)* @_ZTS1A.omp.destr), "QUAL.OMP.MAP.TO"([100 x %class.A] addrspace(4)* addrspacecast ([100 x %class.A] addrspace(1)* @e to [100 x %class.A] addrspace(4)*), [100 x %class.A] addrspace(4)* addrspacecast ([100 x %class.A] addrspace(1)* @e to [100 x %class.A] addrspace(4)*), i64 100, i64 161, i8* null, i8* null), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %d.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE:NONPOD"([100 x %class.A] addrspace(4)* addrspacecast ([100 x %class.A] addrspace(1)* @e to [100 x %class.A] addrspace(4)*), void (%class.A addrspace(4)*, %class.A addrspace(4)*)* @_ZTS1A.omp.copy_constr, void (%class.A addrspace(4)*)* @_ZTS1A.omp.destr), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %d.ascast) ]
  %2 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %2, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %4 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %d.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add1 = add nsw i32 %6, 1
  store i32 %add1, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent noinline nounwind uwtable
declare void @_ZTS1A.omp.copy_constr(%class.A addrspace(4)* %0, %class.A addrspace(4)* %1) #2

; Function Attrs: convergent noinline nounwind uwtable
declare spir_func void @_ZTS1A.omp.destr(%class.A addrspace(4)* %0) #2 section ".text.startup"

attributes #0 = { convergent mustprogress noinline nounwind optnone uwtable "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #1 = { nounwind }
attributes #2 = { convergent noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!6}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}

!0 = !{i32 0, i32 66313, i32 154402824, !"_Z3fn1v", i32 11, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"uwtable", i32 1}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = !{!"clang version 9.0.0"}
