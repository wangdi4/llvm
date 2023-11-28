; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels: enable
;
; channel int ch[4];
;
; __attribute__((noinline))
; void sendData1(channel int a) {
;   write_channel_intel(a, 2);
; }
;
; __attribute__((noinline))
; void sendData2(channel int a, channel int b) {
;   write_channel_intel(a, 3);
;   sendData1(b);
; }
;
; __attribute__((noinline))
; void sendData3(channel int a, channel int b, channel int c) {
;   write_channel_intel(a, 4);
;   sendData2(b, c);
; }
;
; __attribute__((noinline))
; void sendData4(channel int a, channel int b, channel int c, channel int d) {
;   write_channel_intel(a, 5);
;   sendData3(b, c, d);
; }
;
; __kernel void foo() {
;   sendData4(ch[0], ch[1], ch[2], ch[3]);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; opt -passes=sycl-kernel-target-ext-type-lower,sycl-kernel-equalizer %s -S
; ----------------------------------------------------

; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@ch = addrspace(1) global [4 x ptr addrspace(1)] zeroinitializer, align 8, !packet_size !0, !packet_align !0

; CHECK: define {{.*}} @foo
; CHECK: %[[PIPE0:.*]] = load {{.*}} @ch,
; CHECK: %[[PIPE1:.*]] = load {{.*}} getelementptr {{.*}} @ch, i64 0, i64 1
; CHECK: %[[PIPE2:.*]] = load {{.*}} getelementptr {{.*}} @ch, i64 0, i64 2
; CHECK: %[[PIPE3:.*]] = load {{.*}} getelementptr {{.*}} @ch, i64 0, i64 3
; CHECK: call void @sendData4({{.*}} %[[PIPE0]], {{.*}} %[[PIPE1]], {{.*}} %[[PIPE2]], {{.*}} %[[PIPE3]])
;
; CHECK: define {{.*}} @sendData1(ptr addrspace(1) %a)
; CHECK: %[[SD1_PIPE_A_ADDR:.*]] = alloca ptr addrspace(1), align 8
; CHECK: store {{.*}} %a, {{.*}} %[[SD1_PIPE_A_ADDR]]
; CHECK: %[[SD1_PIPE:.*]] = load {{.*}} %[[SD1_PIPE_A_ADDR]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[SD1_PIPE]]
;
; CHECK: define {{.*}} @sendData2(ptr addrspace(1) %a, ptr addrspace(1) %b)
; CHECK: %[[SD2_PIPE_A_ADDR:.*]] = alloca ptr addrspace(1), align 8
; CHECK: %[[SD2_PIPE_B_ADDR:.*]] = alloca ptr addrspace(1), align 8
; CHECK: store {{.*}} %a, {{.*}} %[[SD2_PIPE_A_ADDR]]
; CHECK: store {{.*}} %b, {{.*}} %[[SD2_PIPE_B_ADDR]]
; CHECK: %[[SD2_PIPE:.*]] = load {{.*}} %[[SD2_PIPE_A_ADDR]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[SD2_PIPE]]
; CHECK: %[[SD2_PIPE_B:.*]] = load {{.*}} %[[SD2_PIPE_B_ADDR]]
; CHECK: call void @sendData1({{.*}} %[[SD2_PIPE_B]])
;
; CHECK: define {{.*}} @sendData3(ptr addrspace(1) %a, ptr addrspace(1) %b, ptr addrspace(1) %c)
; CHECK: %[[SD3_PIPE_A_ADDR:.*]] = alloca ptr addrspace(1), align 8
; CHECK: %[[SD3_PIPE_B_ADDR:.*]] = alloca ptr addrspace(1), align 8
; CHECK: %[[SD3_PIPE_C_ADDR:.*]] = alloca ptr addrspace(1), align 8
; CHECK: store {{.*}} %a, {{.*}} %[[SD3_PIPE_A_ADDR]]
; CHECK: store {{.*}} %b, {{.*}} %[[SD3_PIPE_B_ADDR]]
; CHECK: store {{.*}} %c, {{.*}} %[[SD3_PIPE_C_ADDR]]
; CHECK: %[[SD3_PIPE:.*]] = load {{.*}} %[[SD3_PIPE_A_ADDR]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[SD3_PIPE]]
; CHECK: %[[SD3_PIPE_B:.*]] = load {{.*}} %[[SD3_PIPE_B_ADDR]]
; CHECK: %[[SD3_PIPE_C:.*]] = load {{.*}} %[[SD3_PIPE_C_ADDR]]
; CHECK: call void @sendData2({{.*}} %[[SD3_PIPE_B]], {{.*}} %[[SD3_PIPE_C]])
;
; CHECK: define {{.*}} @sendData4(ptr addrspace(1) %a, ptr addrspace(1) %b, ptr addrspace(1) %c, ptr addrspace(1) %d)
; CHECK: %[[SD4_PIPE_A_ADDR:.*]] = alloca ptr addrspace(1), align 8
; CHECK: %[[SD4_PIPE_B_ADDR:.*]] = alloca ptr addrspace(1), align 8
; CHECK: %[[SD4_PIPE_C_ADDR:.*]] = alloca ptr addrspace(1), align 8
; CHECK: %[[SD4_PIPE_D_ADDR:.*]] = alloca ptr addrspace(1), align 8
; CHECK: store {{.*}} %a, {{.*}} %[[SD4_PIPE_A_ADDR]]
; CHECK: store {{.*}} %b, {{.*}} %[[SD4_PIPE_B_ADDR]]
; CHECK: store {{.*}} %c, {{.*}} %[[SD4_PIPE_C_ADDR]]
; CHECK: store {{.*}} %d, {{.*}} %[[SD4_PIPE_D_ADDR]]
; CHECK: %[[SD4_PIPE:.*]] = load {{.*}} %[[SD4_PIPE_A_ADDR]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[SD4_PIPE]]
; CHECK: %[[SD4_PIPE_B:.*]] = load {{.*}} %[[SD4_PIPE_B_ADDR]]
; CHECK: %[[SD4_PIPE_C:.*]] = load {{.*}} %[[SD4_PIPE_C_ADDR]]
; CHECK: %[[SD4_PIPE_D:.*]] = load {{.*}} %[[SD4_PIPE_D_ADDR]]
; CHECK: call void @sendData3({{.*}} %[[SD4_PIPE_B]], {{.*}} %[[SD4_PIPE_C]], {{.*}} %[[SD4_PIPE_D]])

; Function Attrs: convergent norecurse nounwind
define dso_local void @foo() #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !arg_type_null_val !2 {
entry:
  %0 = load ptr addrspace(1), ptr addrspace(1) @ch, align 8, !tbaa !5
  %1 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x ptr addrspace(1)], ptr addrspace(1) @ch, i64 0, i64 1), align 8, !tbaa !5
  %2 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x ptr addrspace(1)], ptr addrspace(1) @ch, i64 0, i64 2), align 8, !tbaa !5
  %3 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x ptr addrspace(1)], ptr addrspace(1) @ch, i64 0, i64 3), align 8, !tbaa !5
  call void @sendData4(ptr addrspace(1) %0, ptr addrspace(1) %1, ptr addrspace(1) %2, ptr addrspace(1) %3) #3
  ret void
}

; Function Attrs: convergent noinline norecurse nounwind
define dso_local void @sendData1(ptr addrspace(1) %a) #1 !arg_type_null_val !8 {
entry:
  %a.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %a, ptr %a.addr, align 8, !tbaa !5
  %0 = load ptr addrspace(1), ptr %a.addr, align 8, !tbaa !5
  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %0, i32 noundef 2) #3
  ret void
}

; Function Attrs: convergent nounwind
declare void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1), i32 noundef) #2

; Function Attrs: convergent noinline norecurse nounwind
define dso_local void @sendData2(ptr addrspace(1) %a, ptr addrspace(1) %b) #1 !arg_type_null_val !9 {
entry:
  %a.addr = alloca ptr addrspace(1), align 8
  %b.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %a, ptr %a.addr, align 8, !tbaa !5
  store ptr addrspace(1) %b, ptr %b.addr, align 8, !tbaa !5
  %0 = load ptr addrspace(1), ptr %a.addr, align 8, !tbaa !5
  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %0, i32 noundef 3) #3
  %1 = load ptr addrspace(1), ptr %b.addr, align 8, !tbaa !5
  call void @sendData1(ptr addrspace(1) %1) #3
  ret void
}

; Function Attrs: convergent noinline norecurse nounwind
define dso_local void @sendData3(ptr addrspace(1) %a, ptr addrspace(1) %b, ptr addrspace(1) %c) #1 !arg_type_null_val !10 {
entry:
  %a.addr = alloca ptr addrspace(1), align 8
  %b.addr = alloca ptr addrspace(1), align 8
  %c.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %a, ptr %a.addr, align 8, !tbaa !5
  store ptr addrspace(1) %b, ptr %b.addr, align 8, !tbaa !5
  store ptr addrspace(1) %c, ptr %c.addr, align 8, !tbaa !5
  %0 = load ptr addrspace(1), ptr %a.addr, align 8, !tbaa !5
  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %0, i32 noundef 4) #3
  %1 = load ptr addrspace(1), ptr %b.addr, align 8, !tbaa !5
  %2 = load ptr addrspace(1), ptr %c.addr, align 8, !tbaa !5
  call void @sendData2(ptr addrspace(1) %1, ptr addrspace(1) %2) #3
  ret void
}

; Function Attrs: convergent noinline norecurse nounwind
define dso_local void @sendData4(ptr addrspace(1) %a, ptr addrspace(1) %b, ptr addrspace(1) %c, ptr addrspace(1) %d) #1 !arg_type_null_val !11 {
entry:
  %a.addr = alloca ptr addrspace(1), align 8
  %b.addr = alloca ptr addrspace(1), align 8
  %c.addr = alloca ptr addrspace(1), align 8
  %d.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %a, ptr %a.addr, align 8, !tbaa !5
  store ptr addrspace(1) %b, ptr %b.addr, align 8, !tbaa !5
  store ptr addrspace(1) %c, ptr %c.addr, align 8, !tbaa !5
  store ptr addrspace(1) %d, ptr %d.addr, align 8, !tbaa !5
  %0 = load ptr addrspace(1), ptr %a.addr, align 8, !tbaa !5
  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %0, i32 noundef 5) #3
  %1 = load ptr addrspace(1), ptr %b.addr, align 8, !tbaa !5
  %2 = load ptr addrspace(1), ptr %c.addr, align 8, !tbaa !5
  %3 = load ptr addrspace(1), ptr %d.addr, align 8, !tbaa !5
  call void @sendData3(ptr addrspace(1) %1, ptr addrspace(1) %2, ptr addrspace(1) %3) #3
  ret void
}

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" }
attributes #1 = { convergent noinline norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #2 = { convergent nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #3 = { convergent nounwind }

!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!sycl.kernels = !{!4}

!0 = !{i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!4 = !{ptr @foo}
!5 = !{!6, !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{target("spirv.Channel") zeroinitializer}
!9 = !{target("spirv.Channel") zeroinitializer, target("spirv.Channel") zeroinitializer}
!10 = !{target("spirv.Channel") zeroinitializer, target("spirv.Channel") zeroinitializer, target("spirv.Channel") zeroinitializer}
!11 = !{target("spirv.Channel") zeroinitializer, target("spirv.Channel") zeroinitializer, target("spirv.Channel") zeroinitializer, target("spirv.Channel") zeroinitializer}

; DEBUGIFY-NOT: WARNING: Missing line
