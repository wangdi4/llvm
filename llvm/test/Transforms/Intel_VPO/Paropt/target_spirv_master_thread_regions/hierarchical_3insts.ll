; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; The test has three consecutive Instructions which need to be guarded by the
; 'if (is_master)' test to enable hierarchical parallelism. We need to ensure
; that the barriers are not inserted such as: ' if(master) then __barrier()'

; Test src:
;
; #include <stdio.h>
; #include <omp.h>
;
; int main() {
;   int nt = 0;
; #pragma omp target teams private(nt)
;   {
;     nt = omp_get_num_teams();
;     printf("%d\n", omp_get_num_teams());
;   }
; }
; ModuleID = 'target_teams_call_hierarchical.c'
source_filename = "target_teams_hierarchical_3insts.c"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr addrspace(1) constant [4 x i8] c"%d\0A\00", align 1
; Function Attrs: noinline nounwind optnone uwtable
define hidden i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %nt = alloca i32, align 4
  %nt.ascast = addrspacecast ptr %nt to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %nt.ascast, align 4


  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %nt.ascast, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %nt.ascast, i32 0, i32 1) ]


  %call = call spir_func i32 @omp_get_num_teams() #1
  store i32 %call, ptr addrspace(4) %nt.ascast, align 4
  %call1 = call spir_func i32 @omp_get_num_teams() #1
  %call2 = call spir_func i32 (ptr addrspace(4), ...) @printf(ptr addrspace(4) getelementptr inbounds ([4 x i8], ptr addrspace(4) addrspacecast (ptr addrspace(1) @.str to ptr addrspace(4)), i64 0, i64 0), i32 %call1)


; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
; CHECK: br i1 %is.master.thread, label %[[IF_MASTER_1:[^ ,]+]]
; CHECK: [[IF_MASTER_1]]:
; CHECK: [[NT1:[^ ]+]] = call spir_func i32 @omp_get_num_teams()

; CHECK: store i32 [[NT1]], ptr addrspace(3) @nt.ascast{{.*}}
; CHECK: [[NT2:[^ ]+]] = call spir_func i32 @omp_get_num_teams()

; CHECK: {{[^ ]+}} = call i32 (ptr addrspace(2), ...) @_Z18__spirv_ocl_printfPU3AS2cz(ptr addrspace(2) @.str{{.*}}, i32 [[NT2]])
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)


  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1
; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1
; Function Attrs: nounwind
declare dso_local spir_func i32 @omp_get_num_teams() #2
declare dso_local spir_func i32 @printf(ptr addrspace(4), ...) #3

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2055, i32 156313154, !"main", i32 6, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 10.0.0"}
