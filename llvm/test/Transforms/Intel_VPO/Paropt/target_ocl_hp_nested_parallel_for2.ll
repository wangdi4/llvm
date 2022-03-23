; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; void foo() {
;     #pragma omp parallel for num_threads(1)
;     for (int i = 0; i < 1; ++i) {
;        #pragma omp target teams distribute parallel for
;        for (int j = 0; j < 1000; ++j);
;     }
; }

; Check that the inner parallel-for is partitioned across WGs/WIs.
; The outer parallel-for must not affect the partitioning:
; CHECK: call{{.*}}get_global_id

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind
define hidden spir_func void @foo() #0 {
entry:
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %.omp.lb1 = alloca i32, align 4
  %.omp.lb1.ascast = addrspacecast i32* %.omp.lb1 to i32 addrspace(4)*
  %.omp.ub2 = alloca i32, align 4
  %.omp.ub2.ascast = addrspacecast i32* %.omp.ub2 to i32 addrspace(4)*
  %tmp3 = alloca i32, align 4
  %tmp3.ascast = addrspacecast i32* %tmp3 to i32 addrspace(4)*
  %.omp.iv4 = alloca i32, align 4
  %.omp.iv4.ascast = addrspacecast i32* %.omp.iv4 to i32 addrspace(4)*
  %j = alloca i32, align 4
  %j.ascast = addrspacecast i32* %j to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4, !tbaa !5
  store i32 0, i32 addrspace(4)* %.omp.ub.ascast, align 4, !tbaa !5
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NUM_THREADS"(i32 1), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb1.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub2.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp3.ascast) ]
  %1 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4, !tbaa !5
  store i32 %1, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !5
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc12, %entry
  %2 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !5
  %3 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4, !tbaa !5
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end14

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !5
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4, !tbaa !5
  store i32 0, i32 addrspace(4)* %.omp.lb1.ascast, align 4, !tbaa !5
  store i32 999, i32 addrspace(4)* %.omp.ub2.ascast, align 4, !tbaa !5
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv4.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb1.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub2.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp3.ascast) ]
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv4.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb1.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub2.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp3.ascast) ]
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv4.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb1.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub2.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast) ]
  %8 = load i32, i32 addrspace(4)* %.omp.lb1.ascast, align 4, !tbaa !5
  store i32 %8, i32 addrspace(4)* %.omp.iv4.ascast, align 4, !tbaa !5
  br label %omp.inner.for.cond5

omp.inner.for.cond5:                              ; preds = %omp.inner.for.inc, %omp.inner.for.body
  %9 = load i32, i32 addrspace(4)* %.omp.iv4.ascast, align 4, !tbaa !5
  %10 = load i32, i32 addrspace(4)* %.omp.ub2.ascast, align 4, !tbaa !5
  %cmp6 = icmp sle i32 %9, %10
  br i1 %cmp6, label %omp.inner.for.body7, label %omp.inner.for.end

omp.inner.for.body7:                              ; preds = %omp.inner.for.cond5
  %11 = load i32, i32 addrspace(4)* %.omp.iv4.ascast, align 4, !tbaa !5
  %mul8 = mul nsw i32 %11, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, i32 addrspace(4)* %j.ascast, align 4, !tbaa !5
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body7
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, i32 addrspace(4)* %.omp.iv4.ascast, align 4, !tbaa !5
  %add10 = add nsw i32 %12, 1
  store i32 %add10, i32 addrspace(4)* %.omp.iv4.ascast, align 4, !tbaa !5
  br label %omp.inner.for.cond5

omp.inner.for.end:                                ; preds = %omp.inner.for.cond5
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET"() ]
  br label %omp.body.continue11

omp.body.continue11:                              ; preds = %omp.loop.exit
  br label %omp.inner.for.inc12

omp.inner.for.inc12:                              ; preds = %omp.body.continue11
  %13 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !5
  %add13 = add nsw i32 %13, 1
  store i32 %add13, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !5
  br label %omp.inner.for.cond

omp.inner.for.end14:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit15

omp.loop.exit15:                                  ; preds = %omp.inner.for.end14
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 0, i32 2053, i32 11806120, !"_Z3foo", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{}
!4 = !{!"clang version 10.0.0"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
