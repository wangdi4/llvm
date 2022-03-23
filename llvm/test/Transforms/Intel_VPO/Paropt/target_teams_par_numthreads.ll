; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=DEFAULT
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=DEFAULT

; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-paropt-simulate-get-num-threads-in-target=false -S %s | FileCheck %s -check-prefix=DISABLED
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -vpo-paropt-simulate-get-num-threads-in-target=false -S %s | FileCheck %s -check-prefix=DISABLED

; Test src:
;
; int main(int argv, char **argc) {
; #pragma omp target
;   {
; #pragma omp teams
;     {
;       printf("threads in teams: %d\n", omp_get_num_threads()); // 1
; #pragma omp parallel
; #pragma omp master
;       printf("threads in parallel: %d\n", omp_get_num_threads()); // 4096
;     }
;   }
;   return 0;
; }

; DEFAULT: [[TARGET:%[^ ]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(){{.*}}]
; DEFAULT: call spir_func void @__kmpc_begin_spmd_target()
; DEFAULT: [[TEAMS:%[^ ]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(){{.*}}]
; DEFAULT: call spir_func void @__kmpc_begin_spmd_parallel()
; DEFAULT: [[PAR:%[^ ]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(){{.*}}]
; DEFAULT: call void @llvm.directive.region.exit(token [[PAR]]) [ "DIR.OMP.END.PARALLEL"() ]
; DEFAULT: call spir_func void @__kmpc_end_spmd_parallel()
; DEFAULT: call void @llvm.directive.region.exit(token [[TEAMS]]) [ "DIR.OMP.END.TEAMS"() ]
; DEFAULT: call spir_func void @__kmpc_end_spmd_target()
; DEFAULT: call void @llvm.directive.region.exit(token [[TARGET]]) [ "DIR.OMP.END.TARGET"() ]

; Check that these calls are not emitted with -vpo-paropt-simulate-get-num-threads-in-target=false.
; DISABLED-NOT: call spir_func void @__kmpc_begin_spmd{{.*}}()
; DISABLED-NOT: call spir_func void @__kmpc_end_spmd{{.*}}()
; DISABLED: [[TARGET:%[^ ]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(){{.*}}]
; DISABLED: [[TEAMS:%[^ ]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(){{.*}}]
; DISABLED: [[PAR:%[^ ]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(){{.*}}]
; DISABLED: call void @llvm.directive.region.exit(token [[PAR]]) [ "DIR.OMP.END.PARALLEL"() ]
; DISABLED: call void @llvm.directive.region.exit(token [[TEAMS]]) [ "DIR.OMP.END.TEAMS"() ]
; DISABLED: call void @llvm.directive.region.exit(token [[TARGET]]) [ "DIR.OMP.END.TARGET"() ]


; ModuleID = 'target_teams_par_numthreads.c'
source_filename = "target_teams_par_numthreads.c"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr addrspace(1) constant [22 x i8] c"threads in teams: %d\0A\00", align 1
@.str.1 = private unnamed_addr addrspace(1) constant [25 x i8] c"threads in parallel: %d\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define hidden i32 @main(i32 %argv, i8 addrspace(4)* addrspace(4)* %argc) #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %argv.addr = alloca i32, align 4
  %argv.addr.ascast = addrspacecast i32* %argv.addr to i32 addrspace(4)*
  %argc.addr = alloca i8 addrspace(4)* addrspace(4)*, align 8
  %argc.addr.ascast = addrspacecast i8 addrspace(4)* addrspace(4)** %argc.addr to i8 addrspace(4)* addrspace(4)* addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  store i32 %argv, i32 addrspace(4)* %argv.addr.ascast, align 4
  store i8 addrspace(4)* addrspace(4)* %argc, i8 addrspace(4)* addrspace(4)* addrspace(4)* %argc.addr.ascast, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"() ]
  %call = call spir_func i32 @omp_get_num_threads() #1
  %call1 = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([22 x i8], [22 x i8] addrspace(4)* addrspacecast ([22 x i8] addrspace(1)* @.str to [22 x i8] addrspace(4)*), i64 0, i64 0), i32 %call) #1
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASTER"() ]
  fence acquire
  %call2 = call spir_func i32 @omp_get_num_threads() #1
  %call3 = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([25 x i8], [25 x i8] addrspace(4)* addrspacecast ([25 x i8] addrspace(1)* @.str.1 to [25 x i8] addrspace(4)*), i64 0, i64 0), i32 %call2) #1
  fence release
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.MASTER"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local spir_func i32 @printf(i8 addrspace(4)*, ...) #2

; Function Attrs: nounwind
declare dso_local spir_func i32 @omp_get_num_threads() #3

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2055, i32 151743790, !"_Z4main", i32 10, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 10.0.0"}
