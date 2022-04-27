; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; #include <omp.h>
; #include <stdio.h>
;   int x = 111;
;
; void foo() {
; #pragma omp target private(x)
;   {
; #if TEAMS
; #pragma omp teams private(x)
; #endif
;     {
;       x = 222;
; //      int team_idx = omp_get_team_num();
; #if DEBUG
;       if (team_idx == 0)
;         printf("Before parallel: x = %d\n", x);
; #endif
; #pragma omp parallel private(x)
;       {
; //        int thread_idx = omp_get_thread_num();
;         x = 333;
; #if DEBUG
;         if (thread_idx == 0 && team_idx == 0) {
;           int NThreads = omp_get_num_threads();
;           int NTeams = omp_get_num_teams();
;           printf("teams = %d, threads = %d, x = %d\n", NTeams, NThreads, x);
;         }
; #endif
;       }
; //      if (team_idx == 0) {
; //#if DEBUG
; //        printf("After parallel: x = %d\n", x);
; //#endif
; //        if (x != 222)
; //          printf("failed. x = %d. Expected 222.\n", x);
; //        else
; //          printf("passed\n");
; //      }
;     }
;   }
; }
;
; //int main() { foo(); }

; Check for the private copy of @x for the target construct
; CHECK: [[X_PRIV_TGT:@x.*__global]] = internal addrspace(1) global i32 0, align 4
; CHECK-DAG: store i32 222, i32 addrspace(1)* [[X_PRIV_TGT]], align 4

; Check for the private copy of @x for the parallel construct
; CHECK-DAG: [[X_PRIV_PAR:%x.*]] = alloca i32, align 4
; CHECK-DAG: store i32 333, i32* [[X_PRIV_PAR]], align 4

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@x = external dso_local addrspace(1) global i32, align 4

; Function Attrs: noinline nounwind optnone uwtable
define hidden spir_func void @_Z3foov() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @x to i32 addrspace(4)*)) ]

  store i32 222, i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @x to i32 addrspace(4)*), align 4

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @x to i32 addrspace(4)*)) ]

  store i32 333, i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @x to i32 addrspace(4)*), align 4

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}

!0 = !{i32 0, i32 2055, i32 150403263, !"_Z3foov", i32 6, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
