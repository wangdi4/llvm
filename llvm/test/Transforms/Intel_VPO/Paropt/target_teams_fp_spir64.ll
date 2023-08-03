; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-handle-firstprivate-on-teams=false -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=IGNOREFP
; RUN: opt -vpo-paropt-handle-firstprivate-on-teams=false -switch-to-offload -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=IGNOREFP

; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-handle-firstprivate-on-teams=true -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=HANDLEFP
; RUN: opt -vpo-paropt-handle-firstprivate-on-teams=true -switch-to-offload -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=HANDLEFP

; Test src:

; #include <stdio.h>
;
; int x = 0;
; void foo() {
; #pragma omp target firstprivate(x)
; #pragma omp teams firstprivate(x)
;   printf("%p\n", &x);
; }

; With firstprivate handling on teams construct disabled, the clause is only
; handled for the WILOCAL firstprivate on the outer target.

; IGNOREFP: define{{.*}} spir_kernel void @__omp_offloading{{.*}}foo{{.*}}(i64 [[X_VAL:%[^ ]+]])
; IGNOREFP:   [[X_TGTFP:%.*.fpriv]] = alloca i32, align 4
; IGNOREFP:   [[X_CAST:%[^ ]+]] = addrspacecast ptr [[X_TGTFP]] to ptr addrspace(4)
; IGNOREFP:   br i1 %is.master.thread, label %master.thread.code, label %master.thread.fallthru
; IGNOREFP: master.thread.code:
; IGNOREFP:   [[X_VAL_CAST:%[^ ]+]] = trunc i64 [[X_VAL]] to i32
; IGNOREFP:   store i32 [[X_VAL_CAST]], ptr [[X_TGTFP]], align 4
;
; IGNOREFP:   call {{.*}} @_Z18__spirv_ocl_printfPU3AS2cz({{.*}}, ptr addrspace(4) [[X_CAST]])


; With firstprivate handling on teams construct enabled, the clause is only
; handled for both teams as well as the outer target constructs.

; HANDLEFP: [[X_TEAMFP:@x.ascast.fpriv.__local]] = internal addrspace(3) global i32 0
; HANDLEFP: define{{.*}} spir_kernel void @__omp_offloading{{.*}}foo{{.*}}(i64 [[X_VAL:%[^ ]+]])
; HANDLEFP:   [[X_TGTFP:%.*.fpriv]] = alloca i32, align 4
; HANDLEFP:   br i1 %is.master.thread, label %master.thread.code, label %master.thread.fallthru
; HANDLEFP: master.thread.code:
; HANDLEFP:   [[X_VAL_CAST:%[^ ]+]] = trunc i64 [[X_VAL]] to i32
; HANDLEFP:   store i32 [[X_VAL_CAST]], ptr [[X_TGTFP]], align 4
; HANDLEFP:   [[X_TGTFP_VAL:%[^ ]+]] = load i32, ptr [[X_TGTFP]], align 4
; HANDLEFP:   [[X_TGTFP_VAL_CAST:%[^ ]+]] = zext i32 [[X_TGTFP_VAL]] to i64
; HANDLEFP:   [[X_TGTFP_VAL_CAST1:%[^ ]+]] = trunc i64 [[X_TGTFP_VAL_CAST]] to i32
;
; HANDLEFP:   store i32 [[X_TGTFP_VAL_CAST1]], ptr addrspace(3) [[X_TEAMFP]], align 4
;
; HANDLEFP:   call {{.*}} @_Z18__spirv_ocl_printfPU3AS2cz({{.*}}, ptr addrspace(4) addrspacecast (ptr addrspace(3) [[X_TEAMFP]] to ptr addrspace(4)))

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr addrspace(1) constant [4 x i8] c"%p\0A\00", align 1

; Function Attrs: convergent noinline nounwind optnone
define protected spir_func void @foo() #0 {
entry:
  %x = alloca i32, align 4
  %x.ascast = addrspacecast ptr %x to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %x.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:WILOCAL.TYPED"(ptr addrspace(4) %x.ascast, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %x.ascast, i32 0, i32 1) ]

  %call = call spir_func i32 (ptr addrspace(4), ...) @printf(ptr addrspace(4) noundef addrspacecast (ptr addrspace(1) @.str to ptr addrspace(4)), ptr addrspace(4) noundef %x.ascast) #3

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent
declare spir_func i32 @printf(ptr addrspace(4) noundef, ...) #2

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { convergent "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #3 = { convergent nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 66313, i32 117992196, !"_Z3foo", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
