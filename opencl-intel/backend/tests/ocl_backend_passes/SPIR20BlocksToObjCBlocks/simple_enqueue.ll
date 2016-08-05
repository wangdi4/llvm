;; You can find CL source file in the same directory where this file is located
;;
;;****************************************************************************************
;; 1. The test checks SPIR 2.0 to Objective-C block translation with all necessary for CPU
;;    BE Compiler information is stored in it.
;; 2. It also checks BIImport correctly maps %opencl.block opaque type while importing
;;    the enqueue_kernel built-in.
;;***************************************************************************************
;;
; FIXME: https://jira01.devtools.intel.com/browse/CORC-1357
; XFAIL: *

;; RUN: opt -S -spir20-to-objc-blocks -runtimelib=%S/extended_execution_functions.rtl -builtin-import --verify < %s | FileCheck %s

;; ModuleID = './simple_enqueue.cl'
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

%struct.ndrange_t = type { i32, [3 x i64], [3 x i64], [3 x i64] }
%opencl.queue_t = type opaque
%opencl.block = type opaque

; Function Attrs: nounwind
define spir_func void @device_kernel(float addrspace(1)* %inout) #0 {
  %1 = load float, float addrspace(1)* %inout, align 4
  %2 = call spir_func float @_Z3cosf(float %1)
  store float %2, float addrspace(1)* %inout, align 4
  ret void
}

declare spir_func float @_Z3cosf(float) #1
;;*************************************
;; SPIR 2.0 -> ObjectiveC block mapping
;;*************************************
; CHECK: [[BLOCKDESC:.*]] = internal constant [[BLOCKDESCTY:.*]] { i64 0, i64 40 }
; CHECK-LABEL: define spir_func void @host_kernel

; CHECK-NEXT: [[BLOCKPTR: .*]] = alloca <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, float addrspace(1)* }>
; CHECK-NEXT: [[INVOKEGEP:.*]] = getelementptr {{<.*>}}, [[BLOCKPTRTY:.*]][[BLOCKPTR]], i32 0, i32 3
; CHECK-NEXT: store i8* bitcast {{.*}} @__host_kernel_block_invoke {{.*}}[[INVOKEGEP]]
; CHECK-NEXT: [[DESCRGEP:.*]] = getelementptr {{.*}} [[BLOCKPTRTY]][[BLOCKPTR]], i32 0, i32 4
; CHECK-NEXT: store [[BLOCKDESCTY]]* [[BLOCKDESC]], [[BLOCKDESCTY]]**[[DESCRGEP]]
; CHECK-NEXT: [[TOSPIRBLOCK:.*]] = bitcast [[BLOCKPTRTY]][[BLOCKPTR]] to %opencl.block*

; CHECK: [[CAPTGEP:.*]] = getelementptr {{.*}} [[BLOCKPTRTY]][[BLOCKPTR]], i32 0, i32 5
; CHECK: store float addrspace(1)* %inout, float addrspace(1)**[[CAPTGEP]]

; CHECK: call spir_func i32 @_Z14enqueue_kernel{{.*}}, %opencl.block*[[TOSPIRBLOCK]])

;;*******************************
;; enqueue_kernel built-in import
;;*******************************
; CHECK-LABEL: define {{.*}} i32 @_Z14enqueue_kernel{{.*}}, %opencl.block* %block)

; Function Attrs: nounwind
define spir_func void @host_kernel(float addrspace(1)* %inout) #0 {
  %captured = alloca <{ float addrspace(1)* }>, align 8
  %captured.gep = getelementptr inbounds <{ float addrspace(1)* }>, <{ float addrspace(1)* }>* %captured, i32 0, i32 0
  store float addrspace(1)* %inout, float addrspace(1)** %captured.gep
  %1 = alloca %struct.ndrange_t, align 8
  %2 = call spir_func %opencl.queue_t* @get_default_queue()
  call spir_func void @_Z10ndrange_1Dm(%struct.ndrange_t* sret %1, i64 1)
  %3 = bitcast <{ float addrspace(1)* }>* %captured to i8*
  %4 = call %opencl.block* @spir_block_bind(i8* bitcast (void (i8*)* @__host_kernel_block_invoke to i8*), i32 8, i32 8, i8* %3)
  %5 = call spir_func i32 @_Z14enqueue_kernel9ocl_queuei9ndrange_tU13block_pointerFvvE(%opencl.queue_t* %2, i32 0, %struct.ndrange_t* byval %1, %opencl.block* %4)
  ret void
}

declare spir_func i32 @_Z14enqueue_kernel9ocl_queuei9ndrange_tU13block_pointerFvvE(%opencl.queue_t*, i32, %struct.ndrange_t* byval, %opencl.block*) #1

declare spir_func %opencl.queue_t* @get_default_queue() #1

declare spir_func void @_Z10ndrange_1Dm(%struct.ndrange_t* sret, i64) #1

;;**********************************
;; Correction of the invoke function
;;**********************************
; CHECK: define internal spir_func void @__host_kernel_block_invoke(i8* [[INVKARG:.*]])
; CHECK: [[TOOBJCBLOCK:.*]] = bitcast i8* [[INVKARG]] to [[BLOCKPTRTY]]
; CHECK: [[CAPTURED:.*]] = getelementptr [[BLOCKPTRTY]][[TOOBJCBLOCK]], i32 0, i32 5
; CHECK: load float addrspace(1)*, float addrspace(1)**[[CAPTURED]]

; Function Attrs: nounwind
define internal spir_func void @__host_kernel_block_invoke(i8* %.block_descriptor) #0 {
  %1 = bitcast i8* %.block_descriptor to <{ float addrspace(1)* }>*
  %2 = getelementptr inbounds <{ float addrspace(1)* }>, <{ float addrspace(1)* }>* %1, i32 0, i32 0
  %3 = load float addrspace(1)*, float addrspace(1)** %2, align 8
  call spir_func void @device_kernel(float addrspace(1)* %3)
  ret void
}

declare %opencl.block* @spir_block_bind(i8*, i32, i32, i8*)

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.kernels = !{!0, !6}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}
!llvm.ident = !{!9}

!0 = !{void (float addrspace(1)*)* @device_kernel, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1}
!2 = !{!"kernel_arg_access_qual", !"none"}
!3 = !{!"kernel_arg_type", !"float*"}
!4 = !{!"kernel_arg_type_qual", !""}
!5 = !{!"kernel_arg_base_type", !"float*"}
!6 = !{void (float addrspace(1)*)* @host_kernel, !1, !2, !3, !4, !5}
!7 = !{i32 2, i32 0}
!8 = !{}
!9 = !{!"clang version 3.4 "}
