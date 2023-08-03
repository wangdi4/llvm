; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s | FileCheck %s

; Compiled from OpenCL kernel and only host_kernel is extracted:
; kernel void device_kernel(__global int * inout, clk_event_t event) {
;   *inout = is_valid_event(event);
;   release_event(event);
; }
; kernel void host_kernel(__global int * inout) {
;   clk_event_t event = create_user_event();
;   enqueue_kernel(get_default_queue(), CLK_ENQUEUE_FLAGS_NO_WAIT,
;                  ndrange_1D(1),
;                  ^{ device_kernel(inout, event); });
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.ndrange_t = type { i32, [3 x i64], [3 x i64], [3 x i64] }

define dso_local spir_kernel void @host_kernel(ptr addrspace(1) noundef align 4 %inout) #0 !kernel_arg_addr_space !0 !kernel_arg_access_qual !1 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !3 !kernel_arg_name !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !3 !kernel_arg_buffer_location !3 {
entry:
  %inout.addr = alloca ptr addrspace(1), align 8
; CHECK: %event = alloca ptr, align 8
  %event = alloca target("spirv.DeviceEvent"), align 8
  %tmp = alloca %struct.ndrange_t, align 8
; CHECK: %block = alloca <{ i32, i32, ptr addrspace(4), ptr addrspace(1), ptr }>, align 8
  %block = alloca <{ i32, i32, ptr addrspace(4), ptr addrspace(1), target("spirv.DeviceEvent") }>, align 8
  store ptr addrspace(1) %inout, ptr %inout.addr, align 8, !tbaa !7
; CHECK: %call = call spir_func ptr @_Z17create_user_eventv() #
; CHECK-NEXT: store ptr %call, ptr %event, align 8, !tbaa
; CHECK-NEXT: %call1 = call spir_func ptr @_Z17get_default_queuev() #
  %call = call spir_func target("spirv.DeviceEvent") @_Z17create_user_eventv() #1
  store target("spirv.DeviceEvent") %call, ptr %event, align 8, !tbaa !11
  %call1 = call spir_func target("spirv.Queue") @_Z17get_default_queuev() #1
  call spir_func void @_Z10ndrange_1Dm(ptr sret(%struct.ndrange_t) align 8 %tmp, i64 noundef 1) #1
; CHECK: getelementptr inbounds <{ i32, i32, ptr addrspace(4), ptr addrspace(1), ptr }>, ptr %block, i32 0, i32 0
  %block.size = getelementptr inbounds <{ i32, i32, ptr addrspace(4), ptr addrspace(1), target("spirv.DeviceEvent") }>, ptr %block, i32 0, i32 0
  store i32 32, ptr %block.size, align 8
; CHECK: getelementptr inbounds <{ i32, i32, ptr addrspace(4), ptr addrspace(1), ptr }>, ptr %block, i32 0, i32 1
  %block.align = getelementptr inbounds <{ i32, i32, ptr addrspace(4), ptr addrspace(1), target("spirv.DeviceEvent") }>, ptr %block, i32 0, i32 1
  store i32 8, ptr %block.align, align 4
; CHECK: getelementptr inbounds <{ i32, i32, ptr addrspace(4), ptr addrspace(1), ptr }>, ptr %block, i32 0, i32 2
  %block.invoke = getelementptr inbounds <{ i32, i32, ptr addrspace(4), ptr addrspace(1), target("spirv.DeviceEvent") }>, ptr %block, i32 0, i32 2
  store ptr addrspace(4) addrspacecast (ptr @__host_kernel_block_invoke to ptr addrspace(4)), ptr %block.invoke, align 8
; CHECK: getelementptr inbounds <{ i32, i32, ptr addrspace(4), ptr addrspace(1), ptr }>, ptr %block, i32 0, i32 3
  %block.captured = getelementptr inbounds <{ i32, i32, ptr addrspace(4), ptr addrspace(1), target("spirv.DeviceEvent") }>, ptr %block, i32 0, i32 3
  %0 = load ptr addrspace(1), ptr %inout.addr, align 8, !tbaa !7
  store ptr addrspace(1) %0, ptr %block.captured, align 8, !tbaa !7
; CHECK: getelementptr inbounds <{ i32, i32, ptr addrspace(4), ptr addrspace(1), ptr }>, ptr %block, i32 0, i32 4
; CHECK-NEXT: load ptr, ptr %event, align 8, !tbaa
; CHECK-NEXT: store ptr %1, ptr %block.captured2, align 8, !tbaa
  %block.captured2 = getelementptr inbounds <{ i32, i32, ptr addrspace(4), ptr addrspace(1), target("spirv.DeviceEvent") }>, ptr %block, i32 0, i32 4
  %1 = load target("spirv.DeviceEvent"), ptr %event, align 8, !tbaa !11
  store target("spirv.DeviceEvent") %1, ptr %block.captured2, align 8, !tbaa !11
  %2 = addrspacecast ptr %block to ptr addrspace(4)
; CHECK: call spir_func i32 @__enqueue_kernel_basic(ptr %call1,
  %3 = call spir_func i32 @__enqueue_kernel_basic(target("spirv.Queue") %call1, i32 0, ptr byval(%struct.ndrange_t) %tmp, ptr addrspace(4) addrspacecast (ptr @__host_kernel_block_invoke_kernel to ptr addrspace(4)), ptr addrspace(4) %2)
  ret void
}

; CHECK-DAG: declare spir_func ptr @_Z17create_user_eventv() #
; CHECK-DAG: declare spir_func ptr @_Z17get_default_queuev() #
; CHECK-DAG: declare spir_func i32 @__enqueue_kernel_basic(ptr, i32, ptr, ptr addrspace(4), ptr addrspace(4))

declare spir_func target("spirv.DeviceEvent") @_Z17create_user_eventv() #1

declare spir_func target("spirv.Queue") @_Z17get_default_queuev() #1

declare spir_func void @_Z10ndrange_1Dm(ptr sret(%struct.ndrange_t) align 8, i64 noundef) #1

declare hidden spir_func void @__host_kernel_block_invoke(ptr addrspace(4) noundef) #1

declare spir_kernel void @__host_kernel_block_invoke_kernel(ptr addrspace(4)) #1

declare spir_func i32 @__enqueue_kernel_basic(target("spirv.Queue"), i32, ptr, ptr addrspace(4), ptr addrspace(4))

attributes #0 = { convergent norecurse nounwind }
attributes #1 = { convergent nounwind }

!0 = !{i32 1}
!1 = !{!"none"}
!2 = !{!"int*"}
!3 = !{!""}
!4 = !{!"inout"}
!5 = !{i1 false}
!6 = !{i32 0}
!7 = !{!8, !8, i64 0}
!8 = !{!"any pointer", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = !{!12, !12, i64 0}
!12 = !{!"clk_event_t", !9, i64 0}

; DEBUGIFY-NOT: WARNING
