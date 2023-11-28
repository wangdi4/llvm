; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
; channel int bar __attribute__((depth(0)));
; channel char far __attribute__((depth(3)));
;
; struct Foo {
;   int i;
;   long j;
; };
; channel struct Foo star;
;
; channel int bar_arr[5];
; channel char far_arr[5][4];
;
; __global bool g_valid;
;
; __kernel void foo() {
;   int i = 42;
;   char f = 10;
;   struct Foo st = {0, 1};
;
;   bool valid;
;   __private bool p_valid;
;   __local bool l_valid;
;
;   bool * p_to_valid = &valid;
;   __global bool *p_to_g_valid = &g_valid;
;   __local bool *p_to_l_valid = &l_valid;
;   __private bool *p_to_p_valid = &p_valid;
;
;   i = read_channel_nb_intel(bar, &valid);
;   f = read_channel_nb_intel(far, p_to_valid);
;   st = read_channel_nb_intel(star, p_to_g_valid);
;   i = read_channel_nb_intel(bar_arr[3], p_to_l_valid);
;   f = read_channel_nb_intel(far_arr[3][2], p_to_p_valid);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------

; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck --implicit-check-not read_channel_nb_intel %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

%struct.Foo = type { i32, i64 }

@__const.foo.st = private unnamed_addr addrspace(2) constant %struct.Foo { i32 0, i64 1 }, align 8
@foo.l_valid = internal addrspace(3) global i8 undef, align 1
@g_valid = addrspace(1) global i8 0, align 1
@bar = addrspace(1) global ptr addrspace(1) null, align 4, !packet_size !0, !packet_align !0, !depth !1
@far = addrspace(1) global ptr addrspace(1) null, align 1, !packet_size !2, !packet_align !2, !depth !3
@star = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !4, !packet_align !5
@bar_arr = addrspace(1) global [5 x ptr addrspace(1)] zeroinitializer, align 4, !packet_size !0, !packet_align !0
@far_arr = addrspace(1) global [5 x [4 x ptr addrspace(1)]] zeroinitializer, align 1, !packet_size !2, !packet_align !2

; CHECK: @[[PIPE_BAR:.*]] = addrspace(1) global ptr addrspace(1)
; CHECK: @[[PIPE_FAR:.*]] = addrspace(1) global ptr addrspace(1)
; CHECK: @[[PIPE_STAR:.*]] = addrspace(1) global ptr addrspace(1)
; CHECK: @[[PIPE_BAR_ARR:.*]] = addrspace(1) global [5 x ptr addrspace(1)]
; CHECK: @[[PIPE_FAR_ARR:.*]] = addrspace(1) global [5 x [4 x ptr addrspace(1)]]

; All calls to read/write_channel_nb_intel should be replaced by
; corresponding calls to pipe built-ins: check is done using --implicit-check-not
;
; CHECK: %[[LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[VALID:.*]] = addrspacecast {{.*}} %valid
; CHECK: %[[CALL_BAR_PIPE:.*]] = call i32 @__read_pipe_2_fpga({{.*}} %[[LOAD_BAR_PIPE]], {{.*}}, i32 4, i32 4)
; CHECK: %[[BOOL_CALL_BAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_BAR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_BAR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_BAR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_BAR_PIPE]], ptr addrspace(4) %[[VALID]]
;

; CHECK: %[[LOAD_FAR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR]]
; CHECK: %[[LOAD_P_TO_VALID:.*]] = load {{.*}} %p_to_valid
; CHECK: %[[CALL_FAR_PIPE:.*]] = call i32 @__read_pipe_2_fpga({{.*}} %[[LOAD_FAR_PIPE]], {{.*}}, i32 1, i32 1)
; CHECK: %[[BOOL_CALL_FAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_FAR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_FAR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_FAR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_FAR_PIPE]], ptr addrspace(4) %[[LOAD_P_TO_VALID]]
;
; CHECK: %[[LOAD_STAR_PIPE:.*]] = load {{.*}} @[[PIPE_STAR]]
; CHECK: %[[LOAD_P_TO_G_VALID:.*]] = load {{.*}} %p_to_g_valid
; CHECK: %[[P_TO_G_VALID:.*]] = addrspacecast {{.*}} %[[LOAD_P_TO_G_VALID]]
; CHECK: %[[CALL_STAR_PIPE:.*]] = call i32 @__read_pipe_2_fpga({{.*}} %[[LOAD_STAR_PIPE]], {{.*}}, i32 16, i32 8)
; CHECK: %[[BOOL_CALL_STAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_STAR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_STAR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_STAR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_STAR_PIPE]], ptr addrspace(4) %[[P_TO_G_VALID]]
;
; CHECK: %[[LOAD_BAR_ARR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR_ARR]]
; CHECK: %[[LOAD_P_TO_L_VALID:.*]] = load {{.*}} %p_to_l_valid
; CHECK: %[[P_TO_L_VALID:.*]] = addrspacecast {{.*}} %[[LOAD_P_TO_L_VALID]]
; CHECK: %[[CALL_BAR_ARR_PIPE:.*]] = call i32 @__read_pipe_2_fpga({{.*}} %[[LOAD_BAR_ARR_PIPE]], {{.*}}, i32 4, i32 4)
; CHECK: %[[BOOL_CALL_BAR_ARR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_BAR_ARR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_BAR_ARR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_BAR_ARR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_BAR_ARR_PIPE]], ptr addrspace(4) %[[P_TO_L_VALID]]
;
; CHECK: %[[LOAD_FAR_ARR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR_ARR]]
; CHECK: %[[LOAD_P_TO_P_VALID:.*]] = load {{.*}} %p_to_p_valid
; CHECK: %[[P_TO_P_VALID:.*]] = addrspacecast {{.*}} %[[LOAD_P_TO_P_VALID]]
; CHECK: %[[CALL_FAR_ARR_PIPE:.*]] = call i32 @__read_pipe_2_fpga({{.*}} %[[LOAD_FAR_ARR_PIPE]], {{.*}}, i32 1, i32 1)
; CHECK: %[[BOOL_CALL_FAR_ARR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_FAR_ARR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_FAR_ARR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_FAR_ARR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_FAR_ARR_PIPE]], ptr addrspace(4) %[[P_TO_P_VALID]]

; Function Attrs: convergent norecurse nounwind
define dso_local void @foo() #0 !kernel_arg_addr_space !7 !kernel_arg_access_qual !7 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !7 !kernel_arg_host_accessible !7 !kernel_arg_pipe_depth !7 !kernel_arg_pipe_io !7 !kernel_arg_buffer_location !7 !arg_type_null_val !7 {
entry:
  %i = alloca i32, align 4
  %f = alloca i8, align 1
  %st = alloca %struct.Foo, align 8
  %valid = alloca i8, align 1
  %p_valid = alloca i8, align 1
  %p_to_valid = alloca ptr addrspace(4), align 8
  %p_to_g_valid = alloca ptr addrspace(1), align 8
  %p_to_l_valid = alloca ptr addrspace(3), align 8
  %p_to_p_valid = alloca ptr, align 8
  %tmp = alloca %struct.Foo, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #4
  store i32 42, ptr %i, align 4, !tbaa !10
  call void @llvm.lifetime.start.p0(i64 1, ptr %f) #4
  store i8 10, ptr %f, align 1, !tbaa !14
  call void @llvm.lifetime.start.p0(i64 16, ptr %st) #4
  call void @llvm.memcpy.p0.p2.i64(ptr align 8 %st, ptr addrspace(2) align 8 @__const.foo.st, i64 16, i1 false)
  call void @llvm.lifetime.start.p0(i64 1, ptr %valid) #4
  call void @llvm.lifetime.start.p0(i64 1, ptr %p_valid) #4
  call void @llvm.lifetime.start.p0(i64 8, ptr %p_to_valid) #4
  %valid.ascast = addrspacecast ptr %valid to ptr addrspace(4)
  store ptr addrspace(4) %valid.ascast, ptr %p_to_valid, align 8, !tbaa !15
  call void @llvm.lifetime.start.p0(i64 8, ptr %p_to_g_valid) #4
  store ptr addrspace(1) @g_valid, ptr %p_to_g_valid, align 8, !tbaa !15
  call void @llvm.lifetime.start.p0(i64 8, ptr %p_to_l_valid) #4
  store ptr addrspace(3) @foo.l_valid, ptr %p_to_l_valid, align 8, !tbaa !15
  call void @llvm.lifetime.start.p0(i64 8, ptr %p_to_p_valid) #4
  store ptr %p_valid, ptr %p_to_p_valid, align 8, !tbaa !15
  %0 = load ptr addrspace(1), ptr addrspace(1) @bar, align 4, !tbaa !14
  %valid.ascast1 = addrspacecast ptr %valid to ptr addrspace(4)
  %call = call i32 @_Z21read_channel_nb_intel11ocl_channeliPU3AS4b(ptr addrspace(1) %0, ptr addrspace(4) noundef %valid.ascast1) #5
  store i32 %call, ptr %i, align 4, !tbaa !10
  %1 = load ptr addrspace(1), ptr addrspace(1) @far, align 1, !tbaa !14
  %2 = load ptr addrspace(4), ptr %p_to_valid, align 8, !tbaa !15
  %call2 = call signext i8 @_Z21read_channel_nb_intel11ocl_channelcPU3AS4b(ptr addrspace(1) %1, ptr addrspace(4) noundef %2) #5
  store i8 %call2, ptr %f, align 1, !tbaa !14
  call void @llvm.lifetime.start.p0(i64 16, ptr %tmp) #4
  %3 = load ptr addrspace(1), ptr addrspace(1) @star, align 8, !tbaa !14
  %4 = load ptr addrspace(1), ptr %p_to_g_valid, align 8, !tbaa !15
  %5 = addrspacecast ptr addrspace(1) %4 to ptr addrspace(4)
  call void @_Z21read_channel_nb_intel11ocl_channel3FooPU3AS4b(ptr sret(%struct.Foo) align 8 %tmp, ptr addrspace(1) %3, ptr addrspace(4) noundef %5) #5
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %st, ptr align 8 %tmp, i64 16, i1 false), !tbaa.struct !17
  call void @llvm.lifetime.end.p0(i64 16, ptr %tmp) #4
  %6 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([5 x ptr addrspace(1)], ptr addrspace(1) @bar_arr, i64 0, i64 3), align 4, !tbaa !14
  %7 = load ptr addrspace(3), ptr %p_to_l_valid, align 8, !tbaa !15
  %8 = addrspacecast ptr addrspace(3) %7 to ptr addrspace(4)
  %call3 = call i32 @_Z21read_channel_nb_intel11ocl_channeliPU3AS4b(ptr addrspace(1) %6, ptr addrspace(4) noundef %8) #5
  store i32 %call3, ptr %i, align 4, !tbaa !10
  %9 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([5 x [4 x ptr addrspace(1)]], ptr addrspace(1) @far_arr, i64 0, i64 3, i64 2), align 1, !tbaa !14
  %10 = load ptr, ptr %p_to_p_valid, align 8, !tbaa !15
  %11 = addrspacecast ptr %10 to ptr addrspace(4)
  %call4 = call signext i8 @_Z21read_channel_nb_intel11ocl_channelcPU3AS4b(ptr addrspace(1) %9, ptr addrspace(4) noundef %11) #5
  store i8 %call4, ptr %f, align 1, !tbaa !14
  call void @llvm.lifetime.end.p0(i64 8, ptr %p_to_p_valid) #4
  call void @llvm.lifetime.end.p0(i64 8, ptr %p_to_l_valid) #4
  call void @llvm.lifetime.end.p0(i64 8, ptr %p_to_g_valid) #4
  call void @llvm.lifetime.end.p0(i64 8, ptr %p_to_valid) #4
  call void @llvm.lifetime.end.p0(i64 1, ptr %p_valid) #4
  call void @llvm.lifetime.end.p0(i64 1, ptr %valid) #4
  call void @llvm.lifetime.end.p0(i64 16, ptr %st) #4
  call void @llvm.lifetime.end.p0(i64 1, ptr %f) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #4
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p2.i64(ptr noalias nocapture writeonly, ptr addrspace(2) noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: convergent nounwind
declare i32 @_Z21read_channel_nb_intel11ocl_channeliPU3AS4b(ptr addrspace(1), ptr addrspace(4) noundef) #3

; Function Attrs: convergent nounwind
declare signext i8 @_Z21read_channel_nb_intel11ocl_channelcPU3AS4b(ptr addrspace(1), ptr addrspace(4) noundef) #3

; Function Attrs: convergent nounwind
declare void @_Z21read_channel_nb_intel11ocl_channel3FooPU3AS4b(ptr sret(%struct.Foo) align 8, ptr addrspace(1), ptr addrspace(4) noundef) #3

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { convergent nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #4 = { nounwind }
attributes #5 = { convergent nounwind }

!opencl.ocl.version = !{!6}
!opencl.spir.version = !{!6}
!opencl.compiler.options = !{!7}
!llvm.ident = !{!8}
!sycl.kernels = !{!9}

!0 = !{i32 4}
!1 = !{i32 0}
!2 = !{i32 1}
!3 = !{i32 3}
!4 = !{i32 16}
!5 = !{i32 8}
!6 = !{i32 2, i32 0}
!7 = !{}
!8 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!9 = !{ptr @foo}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
!14 = !{!12, !12, i64 0}
!15 = !{!16, !16, i64 0}
!16 = !{!"any pointer", !12, i64 0}
!17 = !{i64 0, i64 4, !10, i64 8, i64 8, !18}
!18 = !{!19, !19, i64 0}
!19 = !{!"long", !12, i64 0}

; DEBUGIFY-NOT: WARNING: Missing line
