; RUN: opt -vpo-paropt-target-non-wilocal-fp-alloc-mode=rtl -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=RTL
; RUN: opt -vpo-paropt-target-non-wilocal-fp-alloc-mode=rtl -switch-to-offload -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=RTL

; RUN: opt -vpo-paropt-target-non-wilocal-fp-alloc-mode=module -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=MODULE
; RUN: opt -vpo-paropt-target-non-wilocal-fp-alloc-mode=module -switch-to-offload -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=MODULE

; RUN: opt -vpo-paropt-target-non-wilocal-fp-alloc-mode=wilocal -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=LOCAL
; RUN: opt -vpo-paropt-target-non-wilocal-fp-alloc-mode=wilocal -switch-to-offload -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=LOCAL

; Test src:
; #include <stdio.h>
;
; int x = 0;
; void foo() {
; #pragma omp target firstprivate(x)
;   printf("%p\n", &x);
; }

; With "rtl" allocation mode, no extra copy of "x" is created. The allocation
; and initialization of "x" is handled by libomptarget.

; RTL:    define{{.*}} spir_kernel void @__omp_offloading{{.*}}foo{{.*}}(i32 addrspace(1)* [[X_PARM:%[^ ]+]])
; RTL:      [[X_CAST:%[^ ]+]] = addrspacecast i32 addrspace(1)* [[X_PARM]] to i32 addrspace(4)*
;
; RTL:      br i1 %is.master.thread, label %master.thread.code, label %master.thread.fallthru
; RTL:    master.thread.code:
; RTL:      call {{.*}} @_Z18__spirv_ocl_printfPU3AS2ci({{.*}}, i32 addrspace(4)* [[X_CAST]])


; With "module" allocation mode, the private copy of "x" is allocated in
; addrspace 1 at the module level. Its initial value is passed in as a
; parameter, and the initialization is guarded by a master-thread check.

; MODULE: [[X_FPRIV:@.*.fpriv.__global]] = internal addrspace(1) global i32 0, align 1
; MODULE: define{{.*}} spir_kernel void @__omp_offloading{{.*}}foo{{.*}}(i64 [[X_VAL:%[^ ]+]])
; MODULE:   [[X_VAL_CAST:%[^ ]+]] = trunc i64 [[X_VAL]] to i32
;
; MODULE:   br i1 %is.master.thread, label %master.thread.code, label %master.thread.fallthru
; MODULE; master.thread.code:
; MODULE:   store i32 [[X_VAL_CAST]], i32 addrspace(1)* [[X_FPRIV]], align 4
;
; MODULE:   br i1 %is.master.thread, label %master.thread.code1, label %master.thread.fallthru{{.*}}
; MODULE: master.thread.code1:
; MODULE:   call {{.*}} @_Z18__spirv_ocl_printfPU3AS2ci({{.*}}, i32 addrspace(4)* addrspacecast (i32 addrspace(1)* [[X_FPRIV]] to i32 addrspace(4)*))

; With "wilocal" allocation mode, the firstprivate clause is handled as if the
; WILOCAL modifier was present for it.

; LOCAL:  define{{.*}} spir_kernel void @__omp_offloading{{.*}}foo{{.*}}(i64 [[X_VAL:%[^ ]+]])
; LOCAL:    [[X_FPRIV:%.*.fpriv]] = alloca i32, align 1
; LOCAL:    [[X_CAST:%[^ ]+]] = addrspacecast i32* [[X_FPRIV]] to i32 addrspace(4)*
; LOCAL:    [[X_VAL_CAST:%[^ ]+]] = trunc i64 [[X_VAL]] to i32
; LOCAL:    store i32 [[X_VAL_CAST]], i32* [[X_FPRIV]], align 4
;
; LOCAL:    br i1 %is.master.thread, label %master.thread.code, label %master.thread.fallthru
; LOCAL:  master.thread.code:
; LOCAL:    call {{.*}} @_Z18__spirv_ocl_printfPU3AS2ci({{.*}}, i32 addrspace(4)* [[X_CAST]])

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@x = external addrspace(1) global i32, align 4
@.str = private unnamed_addr addrspace(1) constant [4 x i8] c"%p\0A\00", align 1

; Function Attrs: convergent noinline nounwind optnone
define protected spir_func void @foo() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @x to i32 addrspace(4)*)) ]

  %call = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* noundef getelementptr inbounds ([4 x i8], [4 x i8] addrspace(4)* addrspacecast ([4 x i8] addrspace(1)* @.str to [4 x i8] addrspace(4)*), i64 0, i64 0), i32 addrspace(4)* noundef addrspacecast (i32 addrspace(1)* @x to i32 addrspace(4)*)) #3

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent
declare spir_func i32 @printf(i8 addrspace(4)* noundef, ...) #2

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { convergent "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #3 = { convergent nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 66313, i32 117992173, !"_Z3foo", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
