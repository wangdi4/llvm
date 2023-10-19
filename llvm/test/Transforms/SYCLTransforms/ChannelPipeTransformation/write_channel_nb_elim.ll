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
;
; __global bool g_valid;
;
; __kernel void foo() {
;   int i = 42;
;   char f = 10;
;   struct Foo st = {0};
;
;   bool valid;
;   __private bool p_valid;
;   __local bool l_valid;
;
;   valid = write_channel_nb_intel(bar, i);
;   g_valid = write_channel_nb_intel(far, f);
;   p_valid = write_channel_nb_intel(star, st);
;   l_valid = write_channel_nb_intel(bar_arr[3], i);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------

; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck --implicit-check-not write_channel_nb_intel %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

%struct.Foo = type { i32, i64 }

@foo.l_valid = internal addrspace(3) global i8 undef, align 1
@bar = addrspace(1) global ptr addrspace(1) null, align 4, !packet_size !0, !packet_align !0, !depth !1
@far = addrspace(1) global ptr addrspace(1) null, align 1, !packet_size !2, !packet_align !2, !depth !3
@g_valid = addrspace(1) global i8 0, align 1
@star = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !4, !packet_align !5
@bar_arr = addrspace(1) global [5 x ptr addrspace(1)] zeroinitializer, align 4, !packet_size !0, !packet_align !0

; CHECK: @[[PIPE_BAR:.*]] = addrspace(1) global ptr addrspace(1)
; CHECK: @[[PIPE_FAR:.*]] = addrspace(1) global ptr addrspace(1)
; CHECK: @[[PIPE_STAR:.*]] = addrspace(1) global ptr addrspace(1)
; CHECK: @[[PIPE_BAR_ARR:.*]] = addrspace(1) global [5 x ptr addrspace(1)]

; All calls to read/write_channel_nb_intel should be replaced by
; corresponding calls to pipe built-ins: check is done using --implicit-check-not
;
; CHECK: %[[LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[CALL_BAR_PIPE:.*]] = call i32 @__write_pipe_2_fpga({{.*}} %[[LOAD_BAR_PIPE]], {{.*}}, i32 4, i32 4)
; CHECK: %[[BOOL_CALL_BAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_BAR_PIPE]], 0
;
; CHECK: %[[LOAD_FAR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR]]
; CHECK: %[[CALL_FAR_PIPE:.*]] = call i32 @__write_pipe_2_fpga({{.*}} %[[LOAD_FAR_PIPE]], {{.*}}, i32 1, i32 1)
; CHECK: %[[BOOL_CALL_FAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_FAR_PIPE]], 0
;
; CHECK: %[[LOAD_STAR_PIPE:.*]] = load {{.*}} @[[PIPE_STAR]]
; CHECK: %[[CALL_STAR_PIPE:.*]] = call i32 @__write_pipe_2_fpga({{.*}} %[[LOAD_STAR_PIPE]], {{.*}}, i32 16, i32 8)
; CHECK: %[[BOOL_CALL_STAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_STAR_PIPE]], 0
;
; CHECK: %[[LOAD_BAR_ARR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR_ARR]]
; CHECK: %[[CALL_BAR_ARR_PIPE:.*]] = call i32 @__write_pipe_2_fpga({{.*}} %[[LOAD_BAR_ARR_PIPE]], {{.*}}, i32 4, i32 4)
; CHECK: %[[BOOL_CALL_BAR_ARR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_BAR_ARR_PIPE]], 0

; Function Attrs: convergent norecurse nounwind
define dso_local void @foo() #0 !kernel_arg_addr_space !7 !kernel_arg_access_qual !7 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !7 !kernel_arg_host_accessible !7 !kernel_arg_pipe_depth !7 !kernel_arg_pipe_io !7 !kernel_arg_buffer_location !7 !arg_type_null_val !7 {
entry:
  %i = alloca i32, align 4
  %f = alloca i8, align 1
  %st = alloca %struct.Foo, align 8
  %valid = alloca i8, align 1
  %p_valid = alloca i8, align 1
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #4
  store i32 42, ptr %i, align 4, !tbaa !10
  call void @llvm.lifetime.start.p0(i64 1, ptr %f) #4
  store i8 10, ptr %f, align 1, !tbaa !14
  call void @llvm.lifetime.start.p0(i64 16, ptr %st) #4
  call void @llvm.memset.p0.i64(ptr align 8 %st, i8 0, i64 16, i1 false)
  call void @llvm.lifetime.start.p0(i64 1, ptr %valid) #4
  call void @llvm.lifetime.start.p0(i64 1, ptr %p_valid) #4
  %0 = load ptr addrspace(1), ptr addrspace(1) @bar, align 4, !tbaa !14
  %1 = load i32, ptr %i, align 4, !tbaa !10
  %call = call zeroext i1 @_Z22write_channel_nb_intel11ocl_channelii(ptr addrspace(1) %0, i32 noundef %1) #5
  %frombool = zext i1 %call to i8
  store i8 %frombool, ptr %valid, align 1, !tbaa !15
  %2 = load ptr addrspace(1), ptr addrspace(1) @far, align 1, !tbaa !14
  %3 = load i8, ptr %f, align 1, !tbaa !14
  %call1 = call zeroext i1 @_Z22write_channel_nb_intel11ocl_channelcc(ptr addrspace(1) %2, i8 noundef signext %3) #5
  %frombool2 = zext i1 %call1 to i8
  store i8 %frombool2, ptr addrspace(1) @g_valid, align 1, !tbaa !15
  %4 = load ptr addrspace(1), ptr addrspace(1) @star, align 8, !tbaa !14
  %call3 = call zeroext i1 @_Z22write_channel_nb_intel11ocl_channel3FooS_(ptr addrspace(1) %4, ptr noundef byval(%struct.Foo) align 8 %st) #5
  %frombool4 = zext i1 %call3 to i8
  store i8 %frombool4, ptr %p_valid, align 1, !tbaa !15
  %5 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([5 x ptr addrspace(1)], ptr addrspace(1) @bar_arr, i64 0, i64 3), align 4, !tbaa !14
  %6 = load i32, ptr %i, align 4, !tbaa !10
  %call5 = call zeroext i1 @_Z22write_channel_nb_intel11ocl_channelii(ptr addrspace(1) %5, i32 noundef %6) #5
  %frombool6 = zext i1 %call5 to i8
  store i8 %frombool6, ptr addrspace(3) @foo.l_valid, align 1, !tbaa !15
  call void @llvm.lifetime.end.p0(i64 1, ptr %p_valid) #4
  call void @llvm.lifetime.end.p0(i64 1, ptr %valid) #4
  call void @llvm.lifetime.end.p0(i64 16, ptr %st) #4
  call void @llvm.lifetime.end.p0(i64 1, ptr %f) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #4
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: convergent nounwind
declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channelii(ptr addrspace(1), i32 noundef) #3

; Function Attrs: convergent nounwind
declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channelcc(ptr addrspace(1), i8 noundef signext) #3

; Function Attrs: convergent nounwind
declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channel3FooS_(ptr addrspace(1), ptr noundef byval(%struct.Foo) align 8) #3

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nocallback nofree nounwind willreturn memory(argmem: write) }
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
!16 = !{!"bool", !12, i64 0}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-15: WARNING: Instruction with empty DebugLoc in function
; DEBUGIFY-NOT: WARNING
