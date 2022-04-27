; REQUIRES: asserts

; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-paropt-simulate-get-num-threads-frugally=true -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s -check-prefixes=DEFAULT,ALL
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -vpo-paropt-simulate-get-num-threads-frugally=true -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s -check-prefixes=DEFAULT,ALL

; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-paropt-simulate-get-num-threads-frugally=false -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s -check-prefixes=NOFRUGAL,ALL
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -vpo-paropt-simulate-get-num-threads-frugally=false -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s -check-prefixes=NOFRUGAL,ALL

; Test src:

; #include <omp.h>
; #include <stdio.h>
;
; int main() {
; #pragma omp target
;   {
;     int nt = omp_get_num_threads();
;
; #pragma omp parallel
; #pragma omp master
;     printf("%d\n", nt); // 1
;   }
;
;   return 0;
; }

; Check that we emit begin/end_spmd calls for the target region, but not the
; parallel region.

; DEFAULT:  collectOmpNumThreadsCallerInfo: @main may call omp_get_num_threads.

; DEFAULT:  mayCallOmpGetNumThreads: Ignoring the call to library function '__kmpc_masked' from the region.
; DEFAULT:  mayCallOmpGetNumThreads: Ignoring the call to library function 'printf' from the region.
; DEFAULT:  mayCallOmpGetNumThreads: Ignoring the call to library function '__kmpc_end_masked' from the region.
; DEFAULT:  mayCallOmpGetNumThreads: Didn't find any potential caller of omp_get_num_threads in the region.
; DEFAULT:  mayCallOmpGetNumThreads: Region #2 (parallel) may call omp_get_num_threads: No.
; NOFRUGAL: mayCallOmpGetNumThreads: Region #2 (parallel) may call omp_get_num_threads: Yes.

; DEFAULT:  mayCallOmpGetNumThreads: The region calls omp_get_num_threads directly.
; ALL:      mayCallOmpGetNumThreads: Region #1 (target) may call omp_get_num_threads: Yes.

; ALL:         call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(){{.*}}]
; ALL:         call spir_func void @__kmpc_begin_spmd_target()

; DEFAULT-NOT: call spir_func void @__kmpc_begin_spmd_parallel()
; NOFRUGAL:    call spir_func void @__kmpc_begin_spmd_parallel()
; ALL:         call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(){{.*}}]

; ALL:         call void @llvm.directive.region.exit({{.*}}) [ "DIR.OMP.END.PARALLEL"() ]
; DEFAULT-NOT: call spir_func void @__kmpc_end_spmd_parallel()
; NOFRUGAL:    call spir_func void @__kmpc_end_spmd_parallel()

; ALL:         call spir_func void @__kmpc_end_spmd_target()
; ALL:         call void @llvm.directive.region.exit({{.*}}) [ "DIR.OMP.END.TARGET"() ]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr addrspace(1) constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: convergent noinline nounwind optnone
define hidden i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %nt = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %nt.ascast = addrspacecast i32* %nt to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %nt.ascast) ]

  %call = call spir_func i32 @omp_get_num_threads() #4
  store i32 %call, i32 addrspace(4)* %nt.ascast, align 4

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(i32 addrspace(4)* %nt.ascast) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASTER"() ]
  fence acquire
  %3 = load i32, i32 addrspace(4)* %nt.ascast, align 4
  %call1 = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([4 x i8], [4 x i8] addrspace(4)* addrspacecast ([4 x i8] addrspace(1)* @.str to [4 x i8] addrspace(4)*), i64 0, i64 0), i32 %3) #4
  fence release
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.MASTER"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent nounwind
declare spir_func i32 @omp_get_num_threads() #2

; Function Attrs: convergent
declare spir_func i32 @printf(i8 addrspace(4)*, ...) #3

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { convergent nounwind "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #3 = { convergent "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #4 = { convergent nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!6}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 66313, i32 47064489, !"_Z4main", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
