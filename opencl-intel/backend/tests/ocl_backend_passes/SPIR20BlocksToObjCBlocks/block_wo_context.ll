;; You can find CL source file in the same directory where this file is located
;;
;;**************************************************************************************
;; Check SPIR 2.0 to Objective-C translation of a block with empty context is working OK
;;**************************************************************************************
;;
;; RUN: opt -S -spir20-to-objc-blocks --verify < %s | FileCheck %s

; ModuleID = 'block_wo_context.cl'
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknonw-unknown"

%struct.ndrange_t = type { i32, [3 x i64], [3 x i64], [3 x i64] }
%opencl.queue_t = type opaque
%opencl.block = type opaque

@glbRes = addrspace(1) global i32 0, align 4

; Function Attrs: nounwind
define spir_func void @block_fn(i32 %arg, i32 addrspace(1)* nocapture %res) #0 {
entry:
  store i32 %arg, i32 addrspace(1)* %res, align 4, !tbaa !10
  ret void
}

; Function Attrs: nounwind
define internal spir_func void @kernelBlockNoCtx_block_invoke(i8* nocapture readnone %.block_descriptor) #0 {
entry:
  store i32 1, i32 addrspace(1)* @glbRes, align 4, !tbaa !10
  ret void
}

; Function Attrs: nounwind
define spir_kernel void @enqueue_block_wo_context(i32 addrspace(1)* nocapture readnone %res) #0 {
entry:
  %ndrange = alloca %struct.ndrange_t, align 8
  %call = tail call spir_func %opencl.queue_t* @get_default_queue() #2
  %0 = bitcast %struct.ndrange_t* %ndrange to i8*
  call void @llvm.lifetime.start(i64 80, i8* %0) #2
  call spir_func void @_Z10ndrange_1Dmm(%struct.ndrange_t* sret %ndrange, i64 1, i64 1) #2
; CHECK-NOT: spir_block_bind
; CHECK:     [[BLKCAST:%[a-z.]+]] = bitcast {{.*}} to %opencl.block*
  %1 = call %opencl.block* @spir_block_bind(i8* bitcast (void (i8*)* @kernelBlockNoCtx_block_invoke to i8*), i32 0, i32 0, i8* null) #2
; CHECK:     @_Z14enqueue_kernel9ocl_queuei9ndrange_tU13block_pointerFvvE({{.*}}, %opencl.block* [[BLKCAST]])
  %call1 = call spir_func i32 @_Z14enqueue_kernel9ocl_queuei9ndrange_tU13block_pointerFvvE(%opencl.queue_t* %call, i32 241, %struct.ndrange_t* byval %ndrange, %opencl.block* %1) #2
  call void @llvm.lifetime.end(i64 80, i8* %0) #2
  ret void
}

declare spir_func %opencl.queue_t* @get_default_queue() #1

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #2

declare spir_func void @_Z10ndrange_1Dmm(%struct.ndrange_t* sret, i64, i64) #1

declare spir_func i32 @_Z14enqueue_kernel9ocl_queuei9ndrange_tU13block_pointerFvvE(%opencl.queue_t*, i32, %struct.ndrange_t* byval, %opencl.block*) #1

declare %opencl.block* @spir_block_bind(i8*, i32, i32, i8*)

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

!0 = !{void (i32 addrspace(1)*)* @enqueue_block_wo_context, !1, !2, !3, !4, !5}
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
