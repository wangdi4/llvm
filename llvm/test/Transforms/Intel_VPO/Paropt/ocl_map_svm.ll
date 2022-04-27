; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; void foo(int *arg) {
; #pragma omp target map(arg[22:77])
;   arg[33] = 55;
; }

; Verify that the kernel has only one argument:
; CHECK: define weak dso_local spir_kernel void @__omp_offloading_804_{{.*}}_foo_l2(i32 addrspace(4)* addrspace(1)*{{[^,]*}})

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @foo(i32 addrspace(4)* %arg) #0 {
entry:
  %arg.addr = alloca i32 addrspace(4)*, align 8
  %0 = addrspacecast i32 addrspace(4)** %arg.addr to i32 addrspace(4)* addrspace(4)*
  store i32 addrspace(4)* %arg, i32 addrspace(4)* addrspace(4)* %0, align 8
  %1 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %arrayidx = getelementptr inbounds i32, i32 addrspace(4)* %1, i64 22
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(i32 addrspace(4)* addrspace(4)* %0, i32 addrspace(4)* addrspace(4)* %0, i64 8), "QUAL.OMP.MAP.TOFROM:AGGR"(i32 addrspace(4)* addrspace(4)* %0, i32 addrspace(4)* %arrayidx, i64 308) ]
  %3 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %0, align 8
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(4)* %3, i64 33
  store i32 55, i32 addrspace(4)* %arrayidx1, align 4
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2052, i32 85987294, !"foo", i32 2, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 8.0.0"}
