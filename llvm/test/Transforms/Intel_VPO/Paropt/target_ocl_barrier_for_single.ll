; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

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

; Function Attrs: noinline nounwind optnone
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

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!4}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!5}

!0 = !{i32 1, !"_Z8coef_bnd", i32 0, i32 0, double addrspace(1)* @coef_bnd}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{}
!4 = !{!"cl_doubles"}
!5 = !{!"clang version 10.0.0"}
