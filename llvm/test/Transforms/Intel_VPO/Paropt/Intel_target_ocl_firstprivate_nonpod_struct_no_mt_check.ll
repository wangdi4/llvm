; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
;struct A {
;  int a[100];
;  A(int x) {a[0] = x;}
;};
;void foo(struct A a) {
;#pragma omp target teams distribute parallel for firstprivate(a)
;  for (int i = 0; i < 100; ++i)
;    a.a[i] = i;
;}

; Verify that the pass-as-kernel-argument is applied and that
; the copy constructors for private versions of the 'a' are not guarded
; with a master-thread check. That would be incorrect, because the copies
; must be initialized in all threads.
; CHECK: define{{.*}}spir_kernel void @__omp_offloading_805_be217a__Z3foo1A_l6(<{ [50 x i64] }>* byval(<{ [50 x i64] }>)
; Master-thread check will generate get_local_id calls:
; CHECK-NOT: get_local_id
; CHECK-DAG: call spir_func void @_ZTS1A.omp.copy_constr({{.*}}[[ATTR1:#[0-9]+]]
; CHECK-DAG: call spir_func void @_ZTS1A.omp.destr({{.*}}[[ATTR2:#[0-9]+]]
; CHECK-DAG: [[ATTR1]] = {{{.*}}"openmp-privatization-copyconstructor"{{.*}}}
; CHECK-DAG: [[ATTR2]] = {{{.*}}"openmp-privatization-destructor"{{.*}}}


target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%struct.A = type { [100 x i32] }

; Function Attrs: convergent mustprogress noinline nounwind
define hidden spir_func void @_Z3foo1A(%struct.A addrspace(4)* byval(%struct.A) align 4 %a) #0 {
entry:
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4, !tbaa !8
  store i32 99, i32 addrspace(4)* %.omp.ub.ascast, align 4, !tbaa !8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE:NONPOD"(%struct.A addrspace(4)* %a, void (%struct.A addrspace(4)*, %struct.A addrspace(4)*)* @_ZTS1A.omp.copy_constr, void (%struct.A addrspace(4)*)* @_ZTS1A.omp.destr), "QUAL.OMP.MAP.TO"(%struct.A addrspace(4)* %a, %struct.A addrspace(4)* %a, i64 400, i64 161, i8* null, i8* null), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.FIRSTPRIVATE:NONPOD"(%struct.A addrspace(4)* %a, void (%struct.A addrspace(4)*, %struct.A addrspace(4)*)* @_ZTS1A.omp.copy_constr, void (%struct.A addrspace(4)*)* @_ZTS1A.omp.destr), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.FIRSTPRIVATE:NONPOD"(%struct.A addrspace(4)* %a, void (%struct.A addrspace(4)*, %struct.A addrspace(4)*)* @_ZTS1A.omp.copy_constr, void (%struct.A addrspace(4)*)* @_ZTS1A.omp.destr), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast) ]
  %3 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4, !tbaa !8
  store i32 %3, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !8
  %5 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4, !tbaa !8
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !8
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4, !tbaa !8
  %7 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !8
  %a1 = getelementptr inbounds %struct.A, %struct.A addrspace(4)* %a, i32 0, i32 0, !intel-tbaa !12
  %8 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !8
  %idxprom = sext i32 %8 to i64
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32] addrspace(4)* %a1, i64 0, i64 %idxprom, !intel-tbaa !15
  store i32 %7, i32 addrspace(4)* %arrayidx, align 4, !tbaa !16
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !8
  %add2 = add nsw i32 %9, 1
  store i32 %add2, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !8
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent nounwind
define internal void @_ZTS1A.omp.copy_constr(%struct.A addrspace(4)* %0, %struct.A addrspace(4)* %1) #2 {
entry:
  %.addr = alloca %struct.A addrspace(4)*, align 8
  %.addr.ascast = addrspacecast %struct.A addrspace(4)** %.addr to %struct.A addrspace(4)* addrspace(4)*
  %.addr1 = alloca %struct.A addrspace(4)*, align 8
  %.addr1.ascast = addrspacecast %struct.A addrspace(4)** %.addr1 to %struct.A addrspace(4)* addrspace(4)*
  store %struct.A addrspace(4)* %0, %struct.A addrspace(4)* addrspace(4)* %.addr.ascast, align 8, !tbaa !17
  store %struct.A addrspace(4)* %1, %struct.A addrspace(4)* addrspace(4)* %.addr1.ascast, align 8, !tbaa !17
  %2 = load %struct.A addrspace(4)*, %struct.A addrspace(4)* addrspace(4)* %.addr.ascast, align 8
  %3 = load %struct.A addrspace(4)*, %struct.A addrspace(4)* addrspace(4)* %.addr1.ascast, align 8, !tbaa !17
  %4 = bitcast %struct.A addrspace(4)* %2 to i8 addrspace(4)*
  %5 = bitcast %struct.A addrspace(4)* %3 to i8 addrspace(4)*
  call void @llvm.memcpy.p4i8.p4i8.i64(i8 addrspace(4)* align 4 %4, i8 addrspace(4)* align 4 %5, i64 400, i1 false), !tbaa.struct !19
  ret void
}

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p4i8.p4i8.i64(i8 addrspace(4)* noalias nocapture writeonly, i8 addrspace(4)* noalias nocapture readonly, i64, i1 immarg) #3

; Function Attrs: convergent nounwind
define internal spir_func void @_ZTS1A.omp.destr(%struct.A addrspace(4)* %0) #2 {
entry:
  %.addr = alloca %struct.A addrspace(4)*, align 8
  %.addr.ascast = addrspacecast %struct.A addrspace(4)** %.addr to %struct.A addrspace(4)* addrspace(4)*
  store %struct.A addrspace(4)* %0, %struct.A addrspace(4)* addrspace(4)* %.addr.ascast, align 8, !tbaa !17
  ret void
}

attributes #0 = { convergent mustprogress noinline nounwind "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { convergent nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #3 = { argmemonly nofree nounwind willreturn }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!6}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}

!0 = !{i32 0, i32 2053, i32 12460410, !"_Z3foo1A", i32 6, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = !{!"clang version 13.0.0"}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C++ TBAA"}
!12 = !{!13, !14, i64 0}
!13 = !{!"struct@_ZTS1A", !14, i64 0}
!14 = !{!"array@_ZTSA100_i", !9, i64 0}
!15 = !{!14, !9, i64 0}
!16 = !{!13, !9, i64 0}
!17 = !{!18, !18, i64 0}
!18 = !{!"pointer@_ZTSP1A", !10, i64 0}
!19 = !{i64 0, i64 400, !20}
!20 = !{!14, !14, i64 0}
