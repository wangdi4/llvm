; RUN: %oclopt -sub-group-adaptation -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -sub-group-adaptation -verify -S < %s | FileCheck %s
;;*****************************************************************************
;; This test checks the SubGroupAdaptation pass
;; Four cases:
;;    sub_group_any
;;    sub_group_all
;;    sub_group_barrier
;;*****************************************************************************
; ModuleID = 'Program'
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

; CHECK: @sg_test_all_any
; CHECK: entry
; CHECK-NOT: %call1 = call spir_func i32 @_Z13sub_group_anyi(i32 %1)
; CHECK: %call1 = call spir_func i32 @_Z14work_group_anyi(i32 %1)
; CHECK: store i32 %call1, i32* %b, align 4
; CHECK-NOT: %call2 = call spir_func i32 @_Z13sub_group_alli(i32 %2)
; CHECK: %call2 = call spir_func i32 @_Z14work_group_alli(i32 %2)
; CHECK-NOT: call spir_func void @_Z17sub_group_barrierj(i32 1)
; CHECK: call spir_func void @_Z18work_group_barrierj(i32 1)


; Function Attrs: nounwind
define spir_kernel void @sg_test_all_any(i32 addrspace(1)* %a) #0 {
entry:
  %a.addr = alloca i32 addrspace(1)*, align 8
  %b = alloca i32, align 4
  %done = alloca i32, align 4
  store i32 addrspace(1)* %a, i32 addrspace(1)** %a.addr, align 8
  %call = call spir_func i64 @_Z13get_global_idj(i32 0) #3
  %0 = load i32 addrspace(1)*, i32 addrspace(1)** %a.addr, align 8
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %0, i64 %call
  %1 = load i32, i32 addrspace(1)* %arrayidx, align 4
  %call1 = call spir_func i32 @_Z13sub_group_anyi(i32 %1)
  store i32 %call1, i32* %b, align 4
  store i32 0, i32* %done, align 4
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %2 = load i32, i32* %done, align 4
  %call2 = call spir_func i32 @_Z13sub_group_alli(i32 %2)
  %tobool = icmp ne i32 %call2, 0
  %lnot = xor i1 %tobool, true
  br i1 %lnot, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  store i32 1, i32* %done, align 4
  call spir_func void @_Z17sub_group_barrierj(i32 1)
  br label %while.cond

while.end:                                        ; preds = %while.cond
  ret void
}

declare spir_func i32 @_Z13sub_group_anyi(i32) #1

; Function Attrs: nounwind readnone
declare spir_func i64 @_Z13get_global_idj(i32) #2

declare spir_func i32 @_Z13sub_group_alli(i32) #1

declare spir_func void @_Z17sub_group_barrierj(i32) #1

; CHECK-NOT: declare spir_func i32 @_Z13sub_group_anyi(i32)
; CHECK: declare spir_func i32 @_Z14work_group_anyi(i32)
; CHECK-NOT: declare spir_func i32 @_Z13sub_group_alli(i32)
; CHECK: declare spir_func i32 @_Z14work_group_alli(i32)
; CHECK-NOT: declare spir_func void @_Z17sub_group_barrierj(i32)
; CHECK: declare spir_func void @_Z18work_group_barrierj(i32)

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }

!sycl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = !{void (i32 addrspace(1)*)* @sg_test_all_any, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1}
!2 = !{!"kernel_arg_access_qual", !"none"}
!3 = !{!"kernel_arg_type", !"int*"}
!4 = !{!"kernel_arg_base_type", !"int*"}
!5 = !{!"kernel_arg_type_qual", !""}
!6 = !{i32 1, i32 2}
!7 = !{i32 2, i32 0}
!8 = !{}

;;; --- OpenCL source (compilation options: "-cl-std=CL2.0 -D__OPENCL_C_VERSION__=200"
;;;__attribute__((overloadable)) int sub_group_all(int);
;;;__attribute__((overloadable)) int sub_group_any(int);
;;;__attribute__((overloadable)) void sub_group_barrier(cl_mem_fence_flags);
;;;
;;;__kernel void sg_test_all_any(__global int *a)
;;;{
;;;    int b = sub_group_any(a[get_global_id(0)]);
;;;	int done = 0;
;;;	while(!sub_group_all(done))
;;;	{
;;;	    done = 1;
;;;	    sub_group_barrier(CLK_LOCAL_MEM_FENCE);
;;;	}
;;;} 

; DEBUGIFY-NOT: WARNING
