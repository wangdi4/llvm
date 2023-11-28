; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck %s

; Compiled from OpenCL kernel:
; #pragma OPENCL EXTENSION cl_intel_channels : enable
; typedef unsigned char uchar;
; typedef unsigned int uint;
; typedef uint uint2 __attribute__((ext_vector_type(2)));
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

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@EXTERNAL_STREAM_B_ready = addrspace(1) global [3 x ptr addrspace(1)] zeroinitializer, align 8, !packet_size !0, !packet_align !0
@EXTERNAL_STREAM_B = addrspace(1) global [4 x ptr addrspace(1)] zeroinitializer, align 8, !packet_size !1, !packet_align !1
@EXTERNAL_STREAM_B_processed = addrspace(1) global [2 x ptr addrspace(1)] zeroinitializer, align 8, !packet_size !0, !packet_align !0

; CHECK: @EXTERNAL_STREAM_B_ready = addrspace(1) global [3 x ptr addrspace(1)] zeroinitializer, align 8, !packet_size {{.*}}, !packet_align
; CHECK: @EXTERNAL_STREAM_B_ready.bs = addrspace(1) global [1350 x i8] zeroinitializer, align 1
; CHECK: @EXTERNAL_STREAM_B = addrspace(1) global [4 x ptr addrspace(1)] zeroinitializer, align 8, !packet_size {{.*}}, !packet_align
; CHECK: @EXTERNAL_STREAM_B.bs = addrspace(1) global [1856 x i8] zeroinitializer, align 8
; CHECK: @EXTERNAL_STREAM_B_processed = addrspace(1) global [2 x ptr addrspace(1)] zeroinitializer, align 8, !packet_size {{.*}}, !packet_align
; CHECK: @EXTERNAL_STREAM_B_processed.bs = addrspace(1) global [900 x i8] zeroinitializer, align 1
; CHECK: @llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_ctor, ptr null }]

; Function Attrs: convergent nounwind willreturn memory(none)
declare i64 @_Z12get_local_idj(i32 noundef) #0

; Function Attrs: convergent norecurse nounwind
define dso_local void @stream_reader_B0(ptr addrspace(1) noundef align 8 %in_stream) #1 !kernel_arg_addr_space !0 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !arg_type_null_val !12 {
entry:
; CHECK-LABEL: define dso_local void @stream_reader_B0(
; CHECK: [[LI0:%[0-9]+]] = load ptr addrspace(1), ptr addrspace(1) @EXTERNAL_STREAM_B_ready, align 8, !tbaa
; CHECK: [[LI1:%[0-9]+]] = load ptr addrspace(1), ptr addrspace(1) @EXTERNAL_STREAM_B, align 8, !tbaa
; CHECK: [[LI2:%[0-9]+]] = load ptr addrspace(1), ptr addrspace(1) @EXTERNAL_STREAM_B_processed, align 8, !tbaa
; CHECK: call void @stream_reader(ptr addrspace(1) noundef %0, ptr addrspace(1) [[LI0]], ptr addrspace(1) [[LI1]], ptr addrspace(1) [[LI2]])
  %in_stream.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %in_stream, ptr %in_stream.addr, align 8, !tbaa !13
  %0 = load ptr addrspace(1), ptr %in_stream.addr, align 8, !tbaa !13
  %1 = load ptr addrspace(1), ptr addrspace(1) @EXTERNAL_STREAM_B_ready, align 8, !tbaa !17
  %2 = load ptr addrspace(1), ptr addrspace(1) @EXTERNAL_STREAM_B, align 8, !tbaa !17
  %3 = load ptr addrspace(1), ptr addrspace(1) @EXTERNAL_STREAM_B_processed, align 8, !tbaa !17
  call void @stream_reader(ptr addrspace(1) noundef %0, ptr addrspace(1) %1, ptr addrspace(1) %2, ptr addrspace(1) %3) #4
  ret void
}

; Function Attrs: convergent norecurse nounwind
define dso_local void @stream_reader(ptr addrspace(1) noundef %in_stream, ptr addrspace(1) %ready, ptr addrspace(1) %stream, ptr addrspace(1) %processed) #2 !arg_type_null_val !18 {
entry:
; CHECK-LABEL: define dso_local void @stream_reader(
; CHECK: [[PIPE_READY:%.*]] = load ptr addrspace(1), ptr %ready.addr,
; CHECK: call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) [[PIPE_READY]], ptr addrspace(4) {{.*}}, i32 1, i32 1)
; CHECK: [[PIPE_STREAM:%.*]] = load ptr addrspace(1), ptr %stream.addr,
; CHECK: call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) [[PIPE_STREAM]], ptr addrspace(4) {{.*}}, i32 8, i32 8)
; CHECK: [[PIPE_PROCESSED:%.*]] = load ptr addrspace(1), ptr %processed.addr,
; CHECK: call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) [[PIPE_PROCESSED]], ptr addrspace(4) {{.*}}, i32 1, i32 1)
  %in_stream.addr = alloca ptr addrspace(1), align 8
  %ready.addr = alloca ptr addrspace(1), align 8
  %stream.addr = alloca ptr addrspace(1), align 8
  %processed.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %in_stream, ptr %in_stream.addr, align 8, !tbaa !13
  store ptr addrspace(1) %ready, ptr %ready.addr, align 8, !tbaa !17
  store ptr addrspace(1) %stream, ptr %stream.addr, align 8, !tbaa !17
  store ptr addrspace(1) %processed, ptr %processed.addr, align 8, !tbaa !17
  %0 = load ptr addrspace(1), ptr %ready.addr, align 8, !tbaa !17
  %call = call zeroext i8 @_Z18read_channel_intel11ocl_channelh(ptr addrspace(1) %0) #4
  %1 = load ptr addrspace(1), ptr %stream.addr, align 8, !tbaa !17
  %2 = load ptr addrspace(1), ptr %in_stream.addr, align 8, !tbaa !13
  %call1 = call i64 @_Z12get_local_idj(i32 noundef 0) #5
  %arrayidx = getelementptr inbounds <2 x i32>, ptr addrspace(1) %2, i64 %call1
  %3 = load <2 x i32>, ptr addrspace(1) %arrayidx, align 8, !tbaa !17
  call void @_Z19write_channel_intel11ocl_channelDv2_jS_(ptr addrspace(1) %1, <2 x i32> noundef %3) #4
  %4 = load ptr addrspace(1), ptr %processed.addr, align 8, !tbaa !17
  call void @_Z19write_channel_intel11ocl_channelhh(ptr addrspace(1) %4, i8 noundef zeroext 1) #4
  ret void
}

; Function Attrs: convergent nounwind
declare zeroext i8 @_Z18read_channel_intel11ocl_channelh(ptr addrspace(1)) #3

; Function Attrs: convergent nounwind
declare void @_Z19write_channel_intel11ocl_channelDv2_jS_(ptr addrspace(1), <2 x i32> noundef) #3

; Function Attrs: convergent nounwind
declare void @_Z19write_channel_intel11ocl_channelhh(ptr addrspace(1), i8 noundef zeroext) #3

attributes #0 = { convergent nounwind willreturn memory(none) "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #1 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" }
attributes #2 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #3 = { convergent nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #4 = { convergent nounwind }
attributes #5 = { convergent nounwind willreturn memory(none) }

!opencl.ocl.version = !{!2}
!opencl.spir.version = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!sycl.kernels = !{!5}

!0 = !{i32 1}
!1 = !{i32 8}
!2 = !{i32 2, i32 0}
!3 = !{}
!4 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!5 = !{ptr @stream_reader_B0}
!6 = !{!"none"}
!7 = !{!"uint2*"}
!8 = !{!"uint __attribute__((ext_vector_type(2)))*"}
!9 = !{!""}
!10 = !{i1 false}
!11 = !{i32 0}
!12 = !{ptr addrspace(1) null}
!13 = !{!14, !14, i64 0}
!14 = !{!"any pointer", !15, i64 0}
!15 = !{!"omnipotent char", !16, i64 0}
!16 = !{!"Simple C/C++ TBAA"}
!17 = !{!15, !15, i64 0}
!18 = !{ptr addrspace(1) null, target("spirv.Channel") zeroinitializer, target("spirv.Channel") zeroinitializer, target("spirv.Channel") zeroinitializer}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function stream_reader --  %write.{{.*}} = alloca <2 x i32>, align 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function stream_reader --  %write.{{.*}} = alloca i8, align 1
; DEBUGIFY-COUNT-13: WARNING: Instruction with empty DebugLoc in function __pipe_global_ctor
; DEBUGIFY-NOT: WARNING
