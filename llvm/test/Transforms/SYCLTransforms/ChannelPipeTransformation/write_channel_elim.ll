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
; channel int bar_arr[5] __attribute__((depth(0)));
; channel char far_arr[5][4] __attribute__((depth(3)));
; channel struct Foo star_arr[5][4][3];
;
; __kernel void foo() {
;   int i = 42;
;   char f = 10;
;   struct Foo st = {0, 1};
;
;   write_channel_intel(bar, i);
;   write_channel_intel(far, f);
;   write_channel_intel(star, st);
;
;   write_channel_intel(bar_arr[3], i);
;   write_channel_intel(far_arr[3][2], f);
;   write_channel_intel(star_arr[3][2][1], st);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl
; ----------------------------------------------------

; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck --implicit-check-not write_channel_intel %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

%struct.Foo = type { i32, i64 }

@__const.foo.st = private unnamed_addr addrspace(2) constant %struct.Foo { i32 0, i64 1 }, align 8
@bar = addrspace(1) global ptr addrspace(1) null, align 4, !packet_size !0, !packet_align !0, !depth !1
@far = addrspace(1) global ptr addrspace(1) null, align 1, !packet_size !2, !packet_align !2, !depth !3
@star = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !4, !packet_align !5
@bar_arr = addrspace(1) global [5 x ptr addrspace(1)] zeroinitializer, align 4, !packet_size !0, !packet_align !0, !depth !1
@far_arr = addrspace(1) global [5 x [4 x ptr addrspace(1)]] zeroinitializer, align 1, !packet_size !2, !packet_align !2, !depth !3
@star_arr = addrspace(1) global [5 x [4 x [3 x ptr addrspace(1)]]] zeroinitializer, align 8, !packet_size !4, !packet_align !5

; CHECK: @[[PIPE_BAR:.*]] = addrspace(1) global ptr addrspace(1)
; CHECK: @[[PIPE_FAR:.*]] = addrspace(1) global ptr addrspace(1)
; CHECK: @[[PIPE_STAR:.*]] = addrspace(1) global ptr addrspace(1)
; CHECK: @[[PIPE_BAR_ARR:.*]] = addrspace(1) global [5 x ptr addrspace(1)]
; CHECK: @[[PIPE_FAR_ARR:.*]] = addrspace(1) global [5 x [4 x ptr addrspace(1)]]
; CHECK: @[[PIPE_STAR_ARR:.*]] = addrspace(1) global [5 x [4 x [3 x ptr addrspace(1)]]]

; All calls to read/write_channel_intel should be replaced by
; corresponding calls to pipe built-ins: check is done by --implicit-check-not
;
; CHECK: %[[LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: call i32 @__write_pipe_2_bl_fpga({{.*}} %[[LOAD_BAR_PIPE]], {{.*}}, i32 4, i32 4)
;
; CHECK: %[[LOAD_FAR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR]]
; CHECK: call i32 @__write_pipe_2_bl_fpga({{.*}} %[[LOAD_FAR_PIPE]], {{.*}}, i32 1, i32 1)
;
; CHECK: %[[LOAD_STAR_PIPE:.*]] = load {{.*}} @[[PIPE_STAR]]
; CHECK: call i32 @__write_pipe_2_bl_fpga({{.*}} %[[LOAD_STAR_PIPE]], {{.*}}, i32 16, i32 8)
;
; CHECK: %[[LOAD_BAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_BAR_ARR]]
; CHECK: call i32 @__write_pipe_2_bl_fpga({{.*}} %[[LOAD_BAR_PIPE_ARR]], {{.*}}, i32 4, i32 4)
;
; CHECK: %[[LOAD_FAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_FAR_ARR]]
; CHECK: call i32 @__write_pipe_2_bl_fpga({{.*}} %[[LOAD_FAR_PIPE_ARR]], {{.*}}, i32 1, i32 1)
;
; CHECK: %[[LOAD_STAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_STAR_ARR]]
; CHECK: call i32 @__write_pipe_2_bl_fpga({{.*}} %[[LOAD_STAR_PIPE_ARR]], {{.*}}, i32 16, i32 8)

; Function Attrs: convergent norecurse nounwind
define dso_local void @foo() #0 !kernel_arg_addr_space !7 !kernel_arg_access_qual !7 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !7 !kernel_arg_host_accessible !7 !kernel_arg_pipe_depth !7 !kernel_arg_pipe_io !7 !kernel_arg_buffer_location !7 !arg_type_null_val !7 {
entry:
  %i = alloca i32, align 4
  %f = alloca i8, align 1
  %st = alloca %struct.Foo, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #4
  store i32 42, ptr %i, align 4, !tbaa !10
  call void @llvm.lifetime.start.p0(i64 1, ptr %f) #4
  store i8 10, ptr %f, align 1, !tbaa !14
  call void @llvm.lifetime.start.p0(i64 16, ptr %st) #4
  call void @llvm.memcpy.p0.p2.i64(ptr align 8 %st, ptr addrspace(2) align 8 @__const.foo.st, i64 16, i1 false)
  %0 = load ptr addrspace(1), ptr addrspace(1) @bar, align 4, !tbaa !14
  %1 = load i32, ptr %i, align 4, !tbaa !10
  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %0, i32 noundef %1) #5
  %2 = load ptr addrspace(1), ptr addrspace(1) @far, align 1, !tbaa !14
  %3 = load i8, ptr %f, align 1, !tbaa !14
  call void @_Z19write_channel_intel11ocl_channelcc(ptr addrspace(1) %2, i8 noundef signext %3) #5
  %4 = load ptr addrspace(1), ptr addrspace(1) @star, align 8, !tbaa !14
  call void @_Z19write_channel_intel11ocl_channel3FooS_(ptr addrspace(1) %4, ptr noundef byval(%struct.Foo) align 8 %st) #5
  %5 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([5 x ptr addrspace(1)], ptr addrspace(1) @bar_arr, i64 0, i64 3), align 4, !tbaa !14
  %6 = load i32, ptr %i, align 4, !tbaa !10
  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %5, i32 noundef %6) #5
  %7 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([5 x [4 x ptr addrspace(1)]], ptr addrspace(1) @far_arr, i64 0, i64 3, i64 2), align 1, !tbaa !14
  %8 = load i8, ptr %f, align 1, !tbaa !14
  call void @_Z19write_channel_intel11ocl_channelcc(ptr addrspace(1) %7, i8 noundef signext %8) #5
  %9 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([5 x [4 x [3 x ptr addrspace(1)]]], ptr addrspace(1) @star_arr, i64 0, i64 3, i64 2, i64 1), align 8, !tbaa !14
  call void @_Z19write_channel_intel11ocl_channel3FooS_(ptr addrspace(1) %9, ptr noundef byval(%struct.Foo) align 8 %st) #5
  call void @llvm.lifetime.end.p0(i64 16, ptr %st) #4
  call void @llvm.lifetime.end.p0(i64 1, ptr %f) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #4
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p2.i64(ptr noalias nocapture writeonly, ptr addrspace(2) noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: convergent nounwind
declare void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1), i32 noundef) #3

; Function Attrs: convergent nounwind
declare void @_Z19write_channel_intel11ocl_channelcc(ptr addrspace(1), i8 noundef signext) #3

; Function Attrs: convergent nounwind
declare void @_Z19write_channel_intel11ocl_channel3FooS_(ptr addrspace(1), ptr noundef byval(%struct.Foo) align 8) #3

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" }
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
!6 = !{i32 1, i32 2}
!7 = !{}
!8 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!9 = !{ptr @foo}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
!14 = !{!12, !12, i64 0}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %write.src1 = alloca i32, align 4
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- %write.src = alloca i8, align 1
; DEBUGIFY-COUNT-42: WARNING: Instruction with empty DebugLoc in function __pipe_global_ctor
; DEBUGIFY-NOT: WARNING
