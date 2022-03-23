; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; int X;
; int foo() {
; #pragma omp target private(X)
; #pragma omp teams private(X)
;   *((char *)&X + 3) = 10;
;   return 0;
; }

; Verify that 'X' is privatized for teams as addrspace(3) variable,
; and that the uses relinking adjusts the original addrspace(1) correctly:
; CHECK-LABEL: @__omp_offloading_806_1c0cbe__Z3foov_l3
; CHECK: [[PTR:%[a-zA-Z._0-9]+]] = bitcast i32 addrspace(3)* @X.priv.__local to i8 addrspace(3)*
; CHECK: [[GEP:%[a-zA-Z._0-9]+]] = getelementptr inbounds i8, i8 addrspace(3)* [[PTR]], i64 3
; CHECK: store i8 10, i8 addrspace(3)* [[GEP]]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@X = external dso_local addrspace(1) global i32, align 4

; Function Attrs: noinline nounwind optnone uwtable
define hidden spir_func i32 @_Z3foov() #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @X to i32 addrspace(4)*)) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @X to i32 addrspace(4)*)) ]
  store i8 10, i8 addrspace(4)* getelementptr inbounds (i8, i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast (i32 addrspace(1)* @X to i8 addrspace(1)*) to i8 addrspace(4)*), i64 3), align 1
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2054, i32 1838270, !"_Z3foov", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}
