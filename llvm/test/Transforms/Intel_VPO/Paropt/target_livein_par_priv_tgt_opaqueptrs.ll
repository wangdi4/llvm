; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -aa-pipeline=basic-aa -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:

; int main()
; {
;   int x[2];
; #pragma omp target
; #pragma omp parallel private(x)
;   ;
; }

; Make sure x is not a kernel parameter for the target region.
; CHECK: define weak dso_local spir_kernel void @__omp_offloading{{.*}}Z4main_l4()
; Check that we allocate a private copy of x for the parallel region.
; CHECK:   %x.ascast.priv = alloca [2 x i32], align 4

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind optnone
define protected i32 @main() #0 {
entry:
  %x = alloca [2 x i32], align 4
  %x.ascast = addrspacecast ptr %x to ptr addrspace(4)

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
 "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
 "QUAL.OMP.LIVEIN"(ptr addrspace(4) %x.ascast) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
 "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %x.ascast, i32 0, i64 2) ]

  %array.begin = getelementptr inbounds [2 x i32], ptr addrspace(4) %x.ascast, i32 0, i32 0

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}

!0 = !{i32 0, i32 58, i32 -684467853, !"_Z4main", i32 4, i32 0, i32 0}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"openmp-device", i32 50}
