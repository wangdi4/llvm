; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; #pragma omp declare target
; double coef_bnd;
; void initialize() {
; #pragma omp barrier
; #pragma omp single
;   coef_bnd = 123.0;
; }
; #pragma omp end declare target

; 'omp single' caused generation of __kmpc_barrier with host signature.
; Check that the correct SPIR-V target signature is used:
; CHECK: call spir_func void @__kmpc_barrier()
; CHECK: call spir_func void @__kmpc_barrier()
; CHECK: declare spir_func void @__kmpc_barrier()

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@coef_bnd = hidden target_declare addrspace(1) global double 0.000000e+00, align 8

define hidden spir_func void @_Z10initializev() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.BARRIER"() ]
  fence acq_rel
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.BARRIER"() ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]
  fence acquire
  store double 1.230000e+02, double addrspace(4)* addrspacecast (double addrspace(1)* @coef_bnd to double addrspace(4)*), align 8
  fence release
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SINGLE"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { "openmp-target-declare"="true" }

!omp_offload.info = !{!0}
!0 = !{i32 1, !"_Z8coef_bnd", i32 0, i32 0, double addrspace(1)* @coef_bnd}
