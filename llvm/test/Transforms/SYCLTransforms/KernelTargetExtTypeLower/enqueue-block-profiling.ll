; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s | FileCheck %s

; Compiled from OpenCL kernel:
; void check_res(size_t tid, const clk_event_t evt, __global ulong *res) {
;   capture_event_profiling_info(evt, CLK_PROFILING_COMMAND_EXEC_TIME,
;                                &res[tid]);
;   release_event(evt);
; }
; kernel void enqueue_block_profiling(__global ulong *res) {
;   size_t tid = get_global_id(0);
;   queue_t def_q = get_default_queue();
;   ndrange_t ndrange = ndrange_1D(1);
;   clk_event_t block_evt1;
;   void (^checkBlock)(void) = ^{
;     check_res(tid, block_evt1, res);
;   };
;   res[0] = enqueue_kernel(def_q, CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange, 1,
;                           &block_evt1, NULL, checkBlock);
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.ndrange_t = type { i32, [3 x i64], [3 x i64], [3 x i64] }

define dso_local spir_func void @check_res(i64 noundef %tid, target("spirv.DeviceEvent") %evt, ptr addrspace(1) noundef %res) #0 {
entry:
  %tid.addr = alloca i64, align 8
  %evt.addr = alloca target("spirv.DeviceEvent"), align 8
  %res.addr = alloca ptr addrspace(1), align 8
  store i64 %tid, ptr %tid.addr, align 8, !tbaa !0
  store target("spirv.DeviceEvent") %evt, ptr %evt.addr, align 8, !tbaa !4
  store ptr addrspace(1) %res, ptr %res.addr, align 8, !tbaa !6
  %0 = load target("spirv.DeviceEvent"), ptr %evt.addr, align 8, !tbaa !4
  %1 = load ptr addrspace(1), ptr %res.addr, align 8, !tbaa !6
  %2 = load i64, ptr %tid.addr, align 8, !tbaa !0
  %arrayidx = getelementptr inbounds i64, ptr addrspace(1) %1, i64 %2
  call spir_func void @_Z28capture_event_profiling_info12ocl_clkeventiPU3AS1v(target("spirv.DeviceEvent") %0, i32 noundef 1, ptr addrspace(1) noundef %arrayidx) #1
  %3 = load target("spirv.DeviceEvent"), ptr %evt.addr, align 8, !tbaa !4
  call spir_func void @_Z13release_event12ocl_clkevent(target("spirv.DeviceEvent") %3) #1
  ret void
}

declare spir_func void @_Z28capture_event_profiling_info12ocl_clkeventiPU3AS1v(target("spirv.DeviceEvent"), i32 noundef, ptr addrspace(1) noundef) #1

declare spir_func void @_Z13release_event12ocl_clkevent(target("spirv.DeviceEvent")) #1

define dso_local spir_kernel void @enqueue_block_profiling(ptr addrspace(1) noundef align 8 %res) #0 !kernel_arg_addr_space !8 !kernel_arg_access_qual !9 !kernel_arg_type !10 !kernel_arg_base_type !10 !kernel_arg_type_qual !11 !kernel_arg_name !12 !kernel_arg_host_accessible !13 !kernel_arg_pipe_depth !14 !kernel_arg_pipe_io !11 !kernel_arg_buffer_location !11 {
entry:
; CHECK-LABEL: define dso_local spir_kernel void @enqueue_block_profiling(

  %res.addr = alloca ptr addrspace(1), align 8
  %tid = alloca i64, align 8
; CHECK: %def_q = alloca ptr, align 8
  %def_q = alloca target("spirv.Queue"), align 8
  %ndrange = alloca %struct.ndrange_t, align 8
; CHECK: %block_evt1 = alloca ptr, align 8
  %block_evt1 = alloca target("spirv.DeviceEvent"), align 8
  %checkBlock = alloca ptr addrspace(4), align 8
; CHECK: %block = alloca <{ i32, i32, ptr addrspace(4), i64, ptr, ptr addrspace(1) }>, align 8
  %block = alloca <{ i32, i32, ptr addrspace(4), i64, target("spirv.DeviceEvent"), ptr addrspace(1) }>, align 8
  %tmp = alloca %struct.ndrange_t, align 8
  store ptr addrspace(1) %res, ptr %res.addr, align 8, !tbaa !6
  %call = call spir_func i64 @_Z13get_global_idj(i32 noundef 0) #2
  store i64 %call, ptr %tid, align 8, !tbaa !0
; CHECK: %call1 = call spir_func ptr @_Z17get_default_queuev()
; CHECK: store ptr %call1, ptr %def_q, align 8, !tbaa
  %call1 = call spir_func target("spirv.Queue") @_Z17get_default_queuev() #1
  store target("spirv.Queue") %call1, ptr %def_q, align 8, !tbaa !15
  call spir_func void @_Z10ndrange_1Dm(ptr sret(%struct.ndrange_t) align 8 %ndrange, i64 noundef 1) #1
; CHECK: %block.size = getelementptr inbounds <{ i32, i32, ptr addrspace(4), i64, ptr, ptr addrspace(1) }>, ptr %block, i32 0, i32 0
  %block.size = getelementptr inbounds <{ i32, i32, ptr addrspace(4), i64, target("spirv.DeviceEvent"), ptr addrspace(1) }>, ptr %block, i32 0, i32 0
  store i32 40, ptr %block.size, align 8
; CHECK: %block.align = getelementptr inbounds <{ i32, i32, ptr addrspace(4), i64, ptr, ptr addrspace(1) }>, ptr %block, i32 0, i32 1
  %block.align = getelementptr inbounds <{ i32, i32, ptr addrspace(4), i64, target("spirv.DeviceEvent"), ptr addrspace(1) }>, ptr %block, i32 0, i32 1
  store i32 8, ptr %block.align, align 4
; CHECK: %block.invoke = getelementptr inbounds <{ i32, i32, ptr addrspace(4), i64, ptr, ptr addrspace(1) }>, ptr %block, i32 0, i32 2
  %block.invoke = getelementptr inbounds <{ i32, i32, ptr addrspace(4), i64, target("spirv.DeviceEvent"), ptr addrspace(1) }>, ptr %block, i32 0, i32 2
  store ptr addrspace(4) addrspacecast (ptr @__enqueue_block_profiling_block_invoke to ptr addrspace(4)), ptr %block.invoke, align 8
; CHECK: %block.captured = getelementptr inbounds <{ i32, i32, ptr addrspace(4), i64, ptr, ptr addrspace(1) }>, ptr %block, i32 0, i32 3
  %block.captured = getelementptr inbounds <{ i32, i32, ptr addrspace(4), i64, target("spirv.DeviceEvent"), ptr addrspace(1) }>, ptr %block, i32 0, i32 3
  %0 = load i64, ptr %tid, align 8, !tbaa !0
  store i64 %0, ptr %block.captured, align 8, !tbaa !0
; CHECK: %block.captured2 = getelementptr inbounds <{ i32, i32, ptr addrspace(4), i64, ptr, ptr addrspace(1) }>, ptr %block, i32 0, i32 4
; CHECK: load ptr, ptr %block_evt1, align 8, !tbaa
; CHECK: store ptr {{.*}}, ptr %block.captured2, align 8, !tbaa
; CHECK: %block.captured3 = getelementptr inbounds <{ i32, i32, ptr addrspace(4), i64, ptr, ptr addrspace(1) }>, ptr %block, i32 0, i32 5
  %block.captured2 = getelementptr inbounds <{ i32, i32, ptr addrspace(4), i64, target("spirv.DeviceEvent"), ptr addrspace(1) }>, ptr %block, i32 0, i32 4
  %1 = load target("spirv.DeviceEvent"), ptr %block_evt1, align 8, !tbaa !4
  store target("spirv.DeviceEvent") %1, ptr %block.captured2, align 8, !tbaa !4
  %block.captured3 = getelementptr inbounds <{ i32, i32, ptr addrspace(4), i64, target("spirv.DeviceEvent"), ptr addrspace(1) }>, ptr %block, i32 0, i32 5
  %2 = load ptr addrspace(1), ptr %res.addr, align 8, !tbaa !6
  store ptr addrspace(1) %2, ptr %block.captured3, align 8, !tbaa !6
  %block.ascast = addrspacecast ptr %block to ptr addrspace(4)
  store ptr addrspace(4) %block.ascast, ptr %checkBlock, align 8, !tbaa !17
; CHECK: load ptr, ptr %def_q, align 8, !tbaa
  %3 = load target("spirv.Queue"), ptr %def_q, align 8, !tbaa !15
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %tmp, ptr align 8 %ndrange, i64 80, i1 false), !tbaa.struct !18
  %4 = addrspacecast ptr %block_evt1 to ptr addrspace(4)
  %5 = load ptr addrspace(4), ptr %checkBlock, align 8, !tbaa !17
  %6 = addrspacecast ptr %block to ptr addrspace(4)
; CHECK: call spir_func i32 @__enqueue_kernel_basic_events(ptr {{%[0-9]+}},
  %7 = call spir_func i32 @__enqueue_kernel_basic_events(target("spirv.Queue") %3, i32 0, ptr %tmp, i32 1, ptr addrspace(4) %4, ptr addrspace(4) null, ptr addrspace(4) addrspacecast (ptr @__enqueue_block_profiling_block_invoke_kernel to ptr addrspace(4)), ptr addrspace(4) %6)
  %conv = sext i32 %7 to i64
  %8 = load ptr addrspace(1), ptr %res.addr, align 8, !tbaa !6
  %arrayidx = getelementptr inbounds i64, ptr addrspace(1) %8, i64 0
  store i64 %conv, ptr addrspace(1) %arrayidx, align 8, !tbaa !0
  ret void
}

define internal spir_func void @__enqueue_block_profiling_block_invoke(ptr addrspace(4) noundef %.block_descriptor) #1 {
entry:
; CHECK-LABEL: define internal spir_func void @__enqueue_block_profiling_block_invoke(
  %.block_descriptor.addr = alloca ptr addrspace(4), align 8
  store ptr addrspace(4) %.block_descriptor, ptr %.block_descriptor.addr, align 8
; CHECK: %block.capture.addr = getelementptr inbounds <{ i32, i32, ptr addrspace(4), i64, ptr, ptr addrspace(1) }>, ptr addrspace(4) %.block_descriptor, i32 0, i32 3
  %block.capture.addr = getelementptr inbounds <{ i32, i32, ptr addrspace(4), i64, target("spirv.DeviceEvent"), ptr addrspace(1) }>, ptr addrspace(4) %.block_descriptor, i32 0, i32 3
  %0 = load i64, ptr addrspace(4) %block.capture.addr, align 8, !tbaa !0
; CHECK: %block.capture.addr1 = getelementptr inbounds <{ i32, i32, ptr addrspace(4), i64, ptr, ptr addrspace(1) }>, ptr addrspace(4) %.block_descriptor, i32 0, i32 4
; CHECK: load ptr, ptr addrspace(4) %block.capture.addr1, align 8, !tbaa
; CHECK: %block.capture.addr2 = getelementptr inbounds <{ i32, i32, ptr addrspace(4), i64, ptr, ptr addrspace(1) }>, ptr addrspace(4) %.block_descriptor, i32 0, i32 5
  %block.capture.addr1 = getelementptr inbounds <{ i32, i32, ptr addrspace(4), i64, target("spirv.DeviceEvent"), ptr addrspace(1) }>, ptr addrspace(4) %.block_descriptor, i32 0, i32 4
  %1 = load target("spirv.DeviceEvent"), ptr addrspace(4) %block.capture.addr1, align 8, !tbaa !4
  %block.capture.addr2 = getelementptr inbounds <{ i32, i32, ptr addrspace(4), i64, target("spirv.DeviceEvent"), ptr addrspace(1) }>, ptr addrspace(4) %.block_descriptor, i32 0, i32 5
  %2 = load ptr addrspace(1), ptr addrspace(4) %block.capture.addr2, align 8, !tbaa !6
  call spir_func void @check_res(i64 noundef %0, target("spirv.DeviceEvent") %1, ptr addrspace(1) noundef %2) #1
  ret void
}

define spir_kernel void @__enqueue_block_profiling_block_invoke_kernel(ptr addrspace(4) %0) #1 {
entry:
  call spir_func void @__enqueue_block_profiling_block_invoke(ptr addrspace(4) %0)
  ret void
}

declare spir_func i64 @_Z13get_global_idj(i32 noundef) #2

declare spir_func target("spirv.Queue") @_Z17get_default_queuev() #1

declare spir_func void @_Z10ndrange_1Dm(ptr sret(%struct.ndrange_t) align 8, i64 noundef) #1

declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #3

declare spir_func i32 @__enqueue_kernel_basic_events(target("spirv.Queue"), i32, ptr, i32, ptr addrspace(4), ptr addrspace(4), ptr addrspace(4), ptr addrspace(4))

attributes #0 = { convergent norecurse nounwind }
attributes #1 = { convergent nounwind }
attributes #2 = { convergent nounwind willreturn memory(none) }
attributes #3 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }

!0 = !{!1, !1, i64 0}
!1 = !{!"long", !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C/C++ TBAA"}
!4 = !{!5, !5, i64 0}
!5 = !{!"clk_event_t", !2, i64 0}
!6 = !{!7, !7, i64 0}
!7 = !{!"any pointer", !2, i64 0}
!8 = !{i32 1}
!9 = !{!"none"}
!10 = !{!"ulong*"}
!11 = !{!""}
!12 = !{!"res"}
!13 = !{i1 false}
!14 = !{i32 0}
!15 = !{!16, !16, i64 0}
!16 = !{!"queue_t", !2, i64 0}
!17 = !{!2, !2, i64 0}
!18 = !{i64 0, i64 4, !19, i64 8, i64 24, !17, i64 32, i64 24, !17, i64 56, i64 24, !17}
!19 = !{!20, !20, i64 0}
!20 = !{!"int", !2, i64 0}

; DEBUGIFY-NOT: WARNING
