; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck %s

; Compiled from OpenCL kernel:
; #pragma OPENCL EXTENSION cl_intel_channels : enable
; channel uchar EXTERNAL_STREAM_B_ready[3];
; channel uchar EXTERNAL_STREAM_B_processed[2];
; channel uint2 EXTERNAL_STREAM_B[4];
; void stream_reader(global uint2 *in_stream, channel uchar ready,
;                    channel uint2 stream, channel uchar processed) {
;   read_channel_intel(ready);
;   write_channel_intel(stream, in_stream[get_local_id(0)]);
;   write_channel_intel(processed, (uchar)1);
; }
; kernel void stream_reader_B0(global uint2 *in_stream) {
;   stream_reader(in_stream, EXTERNAL_STREAM_B_ready[0], EXTERNAL_STREAM_B[0],
;                 EXTERNAL_STREAM_B_processed[0]);
; }


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: @EXTERNAL_STREAM_B_ready = internal
; CHECK: @EXTERNAL_STREAM_B = internal
; CHECK: @EXTERNAL_STREAM_B_processed = internal
; CHECK: @llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_ctor, ptr null }]
; CHECK: @EXTERNAL_STREAM_B_ready.pipe = addrspace(1) global [3 x ptr addrspace(1)] zeroinitializer, align 16, !packet_size {{.*}}, !packet_align
; CHECK: @EXTERNAL_STREAM_B_ready.pipe.bs = addrspace(1) global [1350 x i8] zeroinitializer, align 1
; CHECK: @EXTERNAL_STREAM_B.pipe = addrspace(1) global [4 x ptr addrspace(1)] zeroinitializer, align 16, !packet_size {{.*}}, !packet_align
; CHECK: @EXTERNAL_STREAM_B.pipe.bs = addrspace(1) global [1856 x i8] zeroinitializer, align 8
; CHECK: @EXTERNAL_STREAM_B_processed.pipe = addrspace(1) global [2 x ptr addrspace(1)] zeroinitializer, align 8, !packet_size {{.*}}, !packet_align
; CHECK: @EXTERNAL_STREAM_B_processed.pipe.bs = addrspace(1) global [900 x i8] zeroinitializer, align 1

@EXTERNAL_STREAM_B_ready = addrspace(1) global [3 x target("spirv.Channel")] zeroinitializer, align 1, !packet_size !0, !packet_align !0
@EXTERNAL_STREAM_B = addrspace(1) global [4 x target("spirv.Channel")] zeroinitializer, align 8, !packet_size !1, !packet_align !1
@EXTERNAL_STREAM_B_processed = addrspace(1) global [2 x target("spirv.Channel")] zeroinitializer, align 1, !packet_size !0, !packet_align !0

declare i64 @_Z12get_local_idj(i32 noundef) #0

define dso_local void @stream_reader_B0(ptr addrspace(1) noundef align 8 %in_stream) #1 !kernel_arg_addr_space !0 !kernel_arg_access_qual !3 !kernel_arg_type !4 !kernel_arg_base_type !5 !kernel_arg_type_qual !6 !kernel_arg_name !7 !kernel_arg_host_accessible !8 !kernel_arg_pipe_depth !9 !kernel_arg_pipe_io !6 !kernel_arg_buffer_location !6 !arg_type_null_val !10 {
entry:
; CHECK-LABEL: define dso_local void @stream_reader_B0(
; CHECK: [[LI0:%[0-9]+]] = load ptr addrspace(1), ptr addrspace(1) @EXTERNAL_STREAM_B_ready.pipe, align 8, !tbaa
; CHECK: [[LI1:%[0-9]+]] = load ptr addrspace(1), ptr addrspace(1) @EXTERNAL_STREAM_B.pipe, align 8, !tbaa
; CHECK: [[LI2:%[0-9]+]] = load ptr addrspace(1), ptr addrspace(1) @EXTERNAL_STREAM_B_processed.pipe, align 8, !tbaa
; CHECK: call void @stream_reader(ptr addrspace(1) noundef %in_stream, ptr addrspace(1) [[LI0]], ptr addrspace(1) [[LI1]], ptr addrspace(1) [[LI2]])

  %0 = load ptr addrspace(1), ptr addrspace(1) @EXTERNAL_STREAM_B_ready, align 1, !tbaa !11
  %1 = load ptr addrspace(1), ptr addrspace(1) @EXTERNAL_STREAM_B, align 8, !tbaa !11
  %2 = load ptr addrspace(1), ptr addrspace(1) @EXTERNAL_STREAM_B_processed, align 1, !tbaa !11
  call void @stream_reader(ptr addrspace(1) noundef %in_stream, ptr addrspace(1) %0, ptr addrspace(1) %1, ptr addrspace(1) %2) #2
  ret void
}

define internal void @stream_reader(ptr addrspace(1) noundef %in_stream, ptr addrspace(1) %ready, ptr addrspace(1) %stream, ptr addrspace(1) %processed) #1 !arg_type_null_val !14 {
entry:
; CHECK-LABEL: define internal void @stream_reader(
; CHECK: call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %ready, ptr addrspace(4) {{.*}}, i32 1, i32 1)
; CHECK: call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %stream, ptr addrspace(4) {{.*}}, i32 8, i32 8)
; CHECK: call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %processed, ptr addrspace(4) {{.*}}, i32 1, i32 1)

  %call = call zeroext i8 @_Z18read_channel_intel11ocl_channelh(ptr addrspace(1) %ready) #2
  %call1 = call i64 @_Z12get_local_idj(i32 noundef 0) #0
  %arrayidx = getelementptr inbounds <2 x i32>, ptr addrspace(1) %in_stream, i64 %call1
  %0 = load <2 x i32>, ptr addrspace(1) %arrayidx, align 8, !tbaa !11
  call void @_Z19write_channel_intel11ocl_channelDv2_jS_(ptr addrspace(1) %stream, <2 x i32> noundef %0) #2
  call void @_Z19write_channel_intel11ocl_channelhh(ptr addrspace(1) %processed, i8 noundef zeroext 1) #2
  ret void
}

declare zeroext i8 @_Z18read_channel_intel11ocl_channelh(ptr addrspace(1)) #2

declare void @_Z19write_channel_intel11ocl_channelDv2_jS_(ptr addrspace(1), <2 x i32> noundef) #2

declare void @_Z19write_channel_intel11ocl_channelhh(ptr addrspace(1), i8 noundef zeroext) #2

attributes #0 = { convergent nounwind willreturn memory(none) }
attributes #1 = { convergent norecurse nounwind }
attributes #2 = { convergent nounwind }

!sycl.kernels = !{!2}

!0 = !{i32 1}
!1 = !{i32 8}
!2 = !{ptr @stream_reader_B0}
!3 = !{!"none"}
!4 = !{!"uint2*"}
!5 = !{!"uint __attribute__((ext_vector_type(2)))*"}
!6 = !{!""}
!7 = !{!"in_stream"}
!8 = !{i1 false}
!9 = !{i32 0}
!10 = !{ptr addrspace(1) null}
!11 = !{!12, !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
!14 = !{ptr addrspace(1) null, target("spirv.Channel") zeroinitializer, target("spirv.Channel") zeroinitializer, target("spirv.Channel") zeroinitializer}

; DEBUGIFY-COUNT-12: WARNING: Instruction with empty DebugLoc in function __pipe_global_ctor
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function stream_reader --  %write.{{.*}} = alloca <2 x i32>, align 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function stream_reader --  %write.{{.*}} = alloca i8, align 1
; DEBUGIFY-NOT: WARNING
