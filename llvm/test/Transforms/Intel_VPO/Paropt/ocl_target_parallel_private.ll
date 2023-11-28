; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Test src:
;
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
; CHECK-DAG: store i32 222, ptr addrspace(1) [[X_PRIV_TGT]], align 4

; Check for the private copy of @x for the parallel construct
; CHECK-DAG: [[X_PRIV_PAR:%x.*]] = alloca i32, align 4
; CHECK-DAG: store i32 333, ptr [[X_PRIV_PAR]], align 4

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@x = external addrspace(1) global i32, align 4

; Function Attrs: convergent noinline nounwind optnone
define protected spir_func void @foo() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @x to ptr addrspace(4)), i32 0, i32 1) ]
  store i32 222, ptr addrspace(4) addrspacecast (ptr addrspace(1) @x to ptr addrspace(4)), align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @x to ptr addrspace(4)), i32 0, i32 1) ]
  store i32 333, ptr addrspace(4) addrspacecast (ptr addrspace(1) @x to ptr addrspace(4)), align 4
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 53, i32 -1923506143, !"_Z3foo", i32 6, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
