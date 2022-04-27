; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:

; #include <stdio.h>
;
; int a;
; void foo() {
; #pragma omp target map(to:a)
; #pragma omp teams private(a)
;   {
;     a = 10;
;   }
; }

; Check that the local copy of the teams private version of @a is allocated.
; CHECK: @a.priv.__local = internal addrspace(3) global i32 0

; ModuleID = 'target_teams_global_priv.c'
source_filename = "target_teams_global_priv.c"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@a = external dso_local addrspace(1) global i32, align 4

; Function Attrs: noinline nounwind optnone uwtable
define hidden spir_func void @foo() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TO"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @a to i32 addrspace(4)*), i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @a to i32 addrspace(4)*), i64 4, i64 1, i8* null, i8* null) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @a to i32 addrspace(4)*)) ]

  store i32 10, i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @a to i32 addrspace(4)*), align 4
; Check that @a is replaced with @a.priv.__local inside the teams region.
; CHECK: store i32 10, i32 addrspace(3)* @a.priv.__local

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
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

!0 = !{i32 0, i32 2055, i32 150351988, !"foo", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 10.0.0"}

