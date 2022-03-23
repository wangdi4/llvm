; REQUIRES: asserts

; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S -disable-output -debug-only=vpo-paropt-utils %s 2>&1 | FileCheck %s -check-prefix=DEBUG
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S -disable-output -debug-only=vpo-paropt-utils %s 2>&1 | FileCheck %s -check-prefix=DEBUG

; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=IR
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=IR

; Test src:

; #include <omp.h>
; #include <stdio.h>
;
; #pragma omp declare target
; int result = 0;
;
; int f1() { return omp_get_num_threads(); }
; void f2() {
; #pragma omp target
;   {
;     printf("%d\n", f1());
; #pragma omp parallel
; #pragma omp master
;     printf("%d\n", f1());
;   }
; }
; #pragma omp end declare target
;
; int main() {
;
;   printf("case 1\n");
;   f2(); // 1, 8
;
;   printf("case 2\n");
; #pragma omp target
;   f2(); // 1, 1
;
;   printf("case 3\n");
; #pragma omp target parallel
; #pragma omp master
;   f2(); // 8, 8
;
;   return 0;
; }

; DEBUG: deleteCallsInFunctionTo: Deleting call '  call spir_func void @__kmpc_begin_spmd_target()' from function '@f2'.
; DEBUG: deleteCallsInFunctionTo: Deleting call '  call spir_func void @__kmpc_end_spmd_target()' from function '@f2'.
; DEBUG: deleteCallsInFunctionTo: Deleting call '  call spir_func void @__kmpc_begin_spmd_parallel()' from function '@f2'.
; DEBUG: deleteCallsInFunctionTo: Deleting call '  call spir_func void @__kmpc_end_spmd_parallel()' from function '@f2'.

; Check that the function f2 doesn't have calls to __kmpc_begin/end_spmd_target.
; IR-LABEL: define{{.*}} spir_func void @f2()
; IR-NOT:   call spir_func void @__kmpc_begin_spmd_target()
; IR-NOT:   call spir_func void @__kmpc_begin_spmd_parallel()
; IR-NOT:   call spir_func void @__kmpc_end_spmd_parallel()
; IR-NOT:   call spir_func void @__kmpc_end_spmd_target()

; IR-LABEL: define{{.*}} spir_kernel void @__omp_offloading_{{.*}}main{{.*}}()
; IR:       call spir_func void @__kmpc_begin_spmd_target()
; IR:       call spir_func void @__kmpc_end_spmd_target()

; IR:       call spir_func void @__kmpc_begin_spmd_target()
; IR:       call spir_func void @__kmpc_begin_spmd_parallel()
; IR:       call spir_func void @__kmpc_end_spmd_parallel()
; IR:       call spir_func void @__kmpc_end_spmd_target()

; Check that the outlined function for the target region in f2 has calls to  __kmpc_begin/end_spmd_target.
; IR-LABEL: define{{.*}} spir_kernel void @__omp_offloading_{{.*}}f2{{.*}}()
; IR:       call spir_func void @__kmpc_begin_spmd_target()
; IR:       call spir_func void @__kmpc_begin_spmd_parallel()
; IR:       call spir_func void @__kmpc_end_spmd_parallel()
; IR:       call spir_func void @__kmpc_end_spmd_target()

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@result = target_declare addrspace(1) global i32 0, align 4
@.str = private unnamed_addr addrspace(1) constant [4 x i8] c"%d\0A\00", align 1
@.str.1 = private unnamed_addr addrspace(1) constant [8 x i8] c"case 1\0A\00", align 1
@.str.2 = private unnamed_addr addrspace(1) constant [8 x i8] c"case 2\0A\00", align 1
@.str.3 = private unnamed_addr addrspace(1) constant [8 x i8] c"case 3\0A\00", align 1

; Function Attrs: convergent noinline nounwind optnone
define hidden spir_func i32 @f1() #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %call = call spir_func i32 @omp_get_num_threads() #6
  ret i32 %call
}

; Function Attrs: convergent noinline nounwind optnone
define hidden spir_func void @f2() #2 {
entry:

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1) ]

  %call = call spir_func i32 @f1() #6
  %call1 = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* noundef getelementptr inbounds ([4 x i8], [4 x i8] addrspace(4)* addrspacecast ([4 x i8] addrspace(1)* @.str to [4 x i8] addrspace(4)*), i64 0, i64 0), i32 noundef %call) #6

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASTER"() ]
  fence acquire
  %call2 = call spir_func i32 @f1() #6
  %call3 = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* noundef getelementptr inbounds ([4 x i8], [4 x i8] addrspace(4)* addrspacecast ([4 x i8] addrspace(1)* @.str to [4 x i8] addrspace(4)*), i64 0, i64 0), i32 noundef %call2) #6
  fence release
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.MASTER"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: convergent noinline nounwind optnone
define hidden i32 @main() #5 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  %call = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* noundef getelementptr inbounds ([8 x i8], [8 x i8] addrspace(4)* addrspacecast ([8 x i8] addrspace(1)* @.str.1 to [8 x i8] addrspace(4)*), i64 0, i64 0)) #7
  call spir_func void @f2() #7
  %call1 = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* noundef getelementptr inbounds ([8 x i8], [8 x i8] addrspace(4)* addrspacecast ([8 x i8] addrspace(1)* @.str.2 to [8 x i8] addrspace(4)*), i64 0, i64 0)) #7

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2) ]
  call spir_func void @f2() #6
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  %call2 = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* noundef getelementptr inbounds ([8 x i8], [8 x i8] addrspace(4)* addrspacecast ([8 x i8] addrspace(1)* @.str.3 to [8 x i8] addrspace(4)*), i64 0, i64 0)) #7

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 3) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASTER"() ]
  fence acquire
  call spir_func void @f2() #6
  fence release
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.MASTER"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: convergent
declare spir_func i32 @printf(i8 addrspace(4)* noundef, ...) #4

; Function Attrs: convergent nounwind
declare spir_func i32 @omp_get_num_threads() #1

attributes #0 = { convergent noinline nounwind optnone "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" }
attributes #1 = { convergent nounwind "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" }
attributes #2 = { convergent noinline nounwind optnone "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" }
attributes #3 = { nounwind }
attributes #4 = { convergent "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" }
attributes #5 = { convergent noinline nounwind optnone "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #6 = { convergent nounwind }
attributes #7 = { convergent }

!omp_offload.info = !{!0, !1, !2, !3}
!llvm.module.flags = !{!4, !5, !6, !7, !8}

!0 = !{i32 0, i32 66313, i32 47073819, !"_Z4main", i32 25, i32 2, i32 0}
!1 = !{i32 0, i32 66313, i32 47073819, !"_Z4main", i32 29, i32 3, i32 0}
!2 = !{i32 0, i32 66313, i32 47073819, !"_Z2f2", i32 9, i32 1, i32 0}
!3 = !{i32 1, !"_Z6result", i32 0, i32 0, i32 addrspace(1)* @result}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"openmp", i32 50}
!6 = !{i32 7, !"openmp-device", i32 50}
!7 = !{i32 7, !"PIC Level", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 2}
