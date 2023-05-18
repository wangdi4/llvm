; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s | FileCheck %s

; Compiled from OpenCL kernel:
; void block_fn() {
; }
; kernel void test() {
;   void (^kernelBlock)(void) = ^{
;     block_fn();
;   };
;   queue_t def_q = get_default_queue();
;   ndrange_t ndrange = ndrange_1D(1);
;   enqueue_kernel(def_q, CLK_ENQUEUE_FLAGS_WAIT_KERNEL, ndrange,
;                                kernelBlock);
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.ndrange_t = type { i32, [3 x i64], [3 x i64], [3 x i64] }

@__block_literal_global = internal addrspace(1) constant { i32, i32, ptr addrspace(4) } { i32 16, i32 8, ptr addrspace(4) addrspacecast (ptr @__test_block_invoke to ptr addrspace(4)) }, align 8 #0

define dso_local spir_func void @block_fn() #1 {
entry:
; CHECk-LABEL: define dso_local spir_func void @block_fn(
; CHECK-NOT: !arg_type_null_val

  ret void
}

define dso_local spir_kernel void @test() #1 !kernel_arg_addr_space !1 !kernel_arg_access_qual !1 !kernel_arg_type !1 !kernel_arg_base_type !1 !kernel_arg_type_qual !1 !kernel_arg_name !1 !kernel_arg_host_accessible !1 !kernel_arg_pipe_depth !1 !kernel_arg_pipe_io !1 !kernel_arg_buffer_location !1 {
entry:
; CHECK-LABEL: define dso_local spir_kernel void @test()
; CHECK-SAME: !arg_type_null_val [[MD_ARG_TY_NULL:![0-9]+]]
; CHECK: %def_q = alloca ptr, align 8
; CHECK: %call = call spir_func ptr @_Z17get_default_queuev()
; CHECK: store ptr %call, ptr %def_q, align 8
; CHECK: load ptr, ptr %def_q, align 8
; CHECK: call spir_func i32 @__enqueue_kernel_basic(ptr

  %kernelBlock = alloca ptr addrspace(4), align 8
  %def_q = alloca target("spirv.Queue"), align 8
  %ndrange = alloca %struct.ndrange_t, align 8
  %tmp = alloca %struct.ndrange_t, align 8
  store ptr addrspace(4) addrspacecast (ptr addrspace(1) @__block_literal_global to ptr addrspace(4)), ptr %kernelBlock, align 8, !tbaa !3
  %call = call spir_func target("spirv.Queue") @_Z17get_default_queuev() #3
  store target("spirv.Queue") %call, ptr %def_q, align 8, !tbaa !6
  call spir_func void @_Z10ndrange_1Dm(ptr sret(%struct.ndrange_t) align 8 %ndrange, i64 noundef 1) #3
  %0 = load target("spirv.Queue"), ptr %def_q, align 8, !tbaa !6
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %tmp, ptr align 8 %ndrange, i64 80, i1 false), !tbaa.struct !8
  %1 = call spir_func i32 @__enqueue_kernel_basic(target("spirv.Queue") %0, i32 1, ptr byval(%struct.ndrange_t) %tmp, ptr addrspace(4) addrspacecast (ptr @__test_block_invoke_kernel to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @__block_literal_global to ptr addrspace(4)))
  ret void
}

define internal spir_func void @__test_block_invoke(ptr addrspace(4) noundef %.block_descriptor) #3 {
entry:
; CHECK-LABEL: define internal spir_func void @__test_block_invoke(
; CHECK-NOT: !arg_type_null_val

  %.block_descriptor.addr = alloca ptr addrspace(4), align 8
  store ptr addrspace(4) %.block_descriptor, ptr %.block_descriptor.addr, align 8
  call spir_func void @block_fn() #3
  ret void
}

define spir_kernel void @__test_block_invoke_kernel(ptr addrspace(4) %0) #3 {
entry:
; CHECK-LABEL: define spir_kernel void @__test_block_invoke_kernel(
; CHECK-SAME: !arg_type_null_val

  call spir_func void @__test_block_invoke(ptr addrspace(4) %0)
  ret void
}

; CHECK: declare spir_func ptr @_Z17get_default_queuev()

; CHECK: declare spir_func i32 @__enqueue_kernel_basic(ptr, i32, ptr, ptr addrspace(4), ptr addrspace(4))

declare spir_func target("spirv.Queue") @_Z17get_default_queuev() #3

declare spir_func void @_Z10ndrange_1Dm(ptr sret(%struct.ndrange_t) align 8, i64 noundef) #3

declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #4

declare spir_func i32 @__enqueue_kernel_basic(target("spirv.Queue"), i32, ptr, ptr addrspace(4), ptr addrspace(4))

attributes #0 = { "objc_arc_inert" }
attributes #1 = { convergent norecurse nounwind }
attributes #2 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { convergent nounwind }
attributes #4 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }

!llvm.linker.options = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.compiler.options = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 3, i32 0}
!1 = !{}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"queue_t", !4, i64 0}
!8 = !{i64 0, i64 4, !9, i64 8, i64 24, !3, i64 32, i64 24, !3, i64 56, i64 24, !3}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !4, i64 0}

; DEBUGIFY-NOT: WARNING
