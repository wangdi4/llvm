;; You can find CL source file in the same directory where this file is located
;;
;;***********************************************************************************
;; Check SPIR 2.0 to Objective-C translation of a block with multiple spir_block_bind
;; is handled correctly. I.e.
;; 1. Single ObjectiveC block is created
;; 2. No calls to spir_block_bind are left as well as the function itself got deleted
;; 3. The final LLVM IR is correct
;;***********************************************************************************
;;
;; RUN: opt -S -spir20-to-objc-blocks --verify < %s | FileCheck %s

; ModuleID = 'multiple_bind_block.cl'
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknonw-unknown"

%struct.ndrange_t = type { i32, [3 x i64], [3 x i64], [3 x i64] }
%opencl.block = type opaque
%opencl.queue_t = type opaque

; Function Attrs: nounwind
define spir_func void @block_fn(i32 %arg, i32 addrspace(1)* nocapture %res) #0 {
entry:
  store i32 %arg, i32 addrspace(1)* %res, align 4, !tbaa !10
  ret void
}

; CHECK-NOT:   spir_block_bind
; CHECK-LABEL: @multiple_bind_block
; CHECK:       [[BLKCAST:%[a-z.]+]] = bitcast {{.*}} to %opencl.block*
; CHECK:       @_Z26get_kernel_work_group_size{{.*}}(%opencl.block* [[BLKCAST]])
; CHECK:       @_Z45get_kernel_preferred_work_group_size_multiple{{.*}}(%opencl.block* [[BLKCAST]])
; CHECK:       @_Z14enqueue_kernel{{.*}}%opencl.block* [[BLKCAST]])

; Function Attrs: nounwind
define spir_kernel void @multiple_bind_block(i32 addrspace(1)* %res) #0 {
entry:
  %captured = alloca <{ i32 addrspace(1)* }>, align 8
  %ndrange = alloca %struct.ndrange_t, align 8
  %block.captured = getelementptr inbounds <{ i32 addrspace(1)* }>, <{ i32 addrspace(1)* }>* %captured, i64 0, i32 0
  store i32 addrspace(1)* %res, i32 addrspace(1)** %block.captured, align 8, !tbaa !14
  %0 = bitcast <{ i32 addrspace(1)* }>* %captured to i8*
  %1 = call %opencl.block* @spir_block_bind(i8* bitcast (void (i8*)* @__multiple_bind_block_block_invoke to i8*), i32 8, i32 8, i8* %0) #2
  %call = call spir_func i32 @_Z26get_kernel_work_group_sizeU13block_pointerFvvE(%opencl.block* %1) #2
  %block_bind2 = call %opencl.block* @spir_block_bind(i8* bitcast (void (i8*)* @__multiple_bind_block_block_invoke to i8*), i32 8, i32 8, i8* %0) #2
%call1 = call spir_func i32 @_Z45get_kernel_preferred_work_group_size_multipleU13block_pointerFvvE(%opencl.block* %block_bind2) #2
  %div = udiv i32 %call, %call1
  %call2 = call spir_func %opencl.queue_t* @get_default_queue() #2
  %2 = bitcast %struct.ndrange_t* %ndrange to i8*
  call void @llvm.lifetime.start(i64 80, i8* %2) #2
  %conv = zext i32 %div to i64
  %conv3 = zext i32 %call to i64
  call spir_func void @_Z10ndrange_1Dmm(%struct.ndrange_t* sret %ndrange, i64 %conv, i64 %conv3) #2
  %block_bind3 = call %opencl.block* @spir_block_bind(i8* bitcast (void (i8*)* @__multiple_bind_block_block_invoke to i8*), i32 8, i32 8, i8* %0) #2
  %call4 = call spir_func i32 @_Z14enqueue_kernel9ocl_queuei9ndrange_tU13block_pointerFvvE(%opencl.queue_t* %call2, i32 241, %struct.ndrange_t* byval %ndrange, %opencl.block* %block_bind3) #2
  call void @llvm.lifetime.end(i64 80, i8* %2) #2
  ret void
}

; Function Attrs: nounwind
define internal spir_func void @__multiple_bind_block_block_invoke(i8* nocapture readonly %.block_descriptor) #0 {
entry:
  %block = bitcast i8* %.block_descriptor to <{ i32 addrspace(1)* }>*
  %block.capture.addr = getelementptr inbounds <{ i32 addrspace(1)* }>, <{ i32 addrspace(1)* }>* %block, i32 0, i32 0
  %0 = load i32 addrspace(1)** %block.capture.addr, align 8, !tbaa !14
  store i32 2, i32 addrspace(1)* %0, align 4, !tbaa !10
  ret void
}

declare %opencl.block* @spir_block_bind(i8*, i32, i32, i8*)

declare spir_func i32 @_Z26get_kernel_work_group_sizeU13block_pointerFvvE(%opencl.block*) #1

declare spir_func i32 @_Z45get_kernel_preferred_work_group_size_multipleU13block_pointerFvvE(%opencl.block*) #1

declare spir_func %opencl.queue_t* @get_default_queue() #1

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #2

declare spir_func void @_Z10ndrange_1Dmm(%struct.ndrange_t* sret, i64, i64) #1

declare spir_func i32 @_Z14enqueue_kernel9ocl_queuei9ndrange_tU13block_pointerFvvE(%opencl.queue_t*, i32, %struct.ndrange_t* byval, %opencl.block*) #1

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #2

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}
!llvm.ident = !{!9}

!0 = !{void (i32 addrspace(1)*)* @multiple_bind_block, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1}
!2 = !{!"kernel_arg_access_qual", !"none"}
!3 = !{!"kernel_arg_type", !"int*"}
!4 = !{!"kernel_arg_base_type", !"int*"}
!5 = !{!"kernel_arg_type_qual", !""}
!6 = !{i32 1, i32 2}
!7 = !{i32 2, i32 0}
!8 = !{}
!9 = !{!"clang version 3.6.1 (https://github.com/KhronosGroup/SPIR.git 49a8b4a760d227b12116a79b2f7b2e34ef2e6879) (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-llvm d9b98710f905089caec167209da23af2e4f72bf0)"}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
!14 = !{!15, !15, i64 0}
!15 = !{!"any pointer", !12, i64 0}
