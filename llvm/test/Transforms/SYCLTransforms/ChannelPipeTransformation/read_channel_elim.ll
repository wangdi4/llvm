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
;   int i = read_channel_intel(bar);
;   char f = read_channel_intel(far);
;   struct Foo st = read_channel_intel(star);
;
;   int ii = read_channel_intel(bar_arr[3]);
;   char ff = read_channel_intel(far_arr[3][2]);
;   struct Foo stst = read_channel_intel(star_arr[3][2][1]);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl
; ----------------------------------------------------

; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck --implicit-check-not read_channel_intel %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

%struct.Foo = type { i32, i64 }

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
; CHECK: call i32 @__read_pipe_2_bl_fpga({{.*}} %[[LOAD_BAR_PIPE]], {{.*}}, i32 4, i32 4)
;
; CHECK: %[[LOAD_FAR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR]]
; CHECK: call i32 @__read_pipe_2_bl_fpga({{.*}} %[[LOAD_FAR_PIPE]], {{.*}}, i32 1, i32 1)
;
; CHECK: %[[LOAD_STAR_PIPE:.*]] = load {{.*}} @[[PIPE_STAR]]
; CHECK: call i32 @__read_pipe_2_bl_fpga({{.*}} %[[LOAD_STAR_PIPE]], {{.*}}, i32 16, i32 8)
;
; CHECK: %[[LOAD_BAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_BAR_ARR]]
; CHECK: call i32 @__read_pipe_2_bl_fpga({{.*}} %[[LOAD_BAR_PIPE_ARR]], {{.*}}, i32 4, i32 4)
;
; CHECK: %[[LOAD_FAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_FAR_ARR]]
; CHECK: call i32 @__read_pipe_2_bl_fpga({{.*}} %[[LOAD_FAR_PIPE_ARR]], {{.*}}, i32 1, i32 1)
;
; CHECK: %[[LOAD_STAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_STAR_ARR]]
; CHECK: call i32 @__read_pipe_2_bl_fpga({{.*}} %[[LOAD_STAR_PIPE_ARR]], {{.*}}, i32 16, i32 8)

; Function Attrs: convergent norecurse nounwind
define dso_local void @foo() #0 !kernel_arg_addr_space !7 !kernel_arg_access_qual !7 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !7 !kernel_arg_host_accessible !7 !kernel_arg_pipe_depth !7 !kernel_arg_pipe_io !7 !kernel_arg_buffer_location !7 !arg_type_null_val !7 {
entry:
  %i = alloca i32, align 4
  %f = alloca i8, align 1
  %st = alloca %struct.Foo, align 8
  %ii = alloca i32, align 4
  %ff = alloca i8, align 1
  %stst = alloca %struct.Foo, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #3
  %0 = load ptr addrspace(1), ptr addrspace(1) @bar, align 4, !tbaa !10
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %0) #4
  store i32 %call, ptr %i, align 4, !tbaa !13
  call void @llvm.lifetime.start.p0(i64 1, ptr %f) #3
  %1 = load ptr addrspace(1), ptr addrspace(1) @far, align 1, !tbaa !10
  %call1 = call signext i8 @_Z18read_channel_intel11ocl_channelc(ptr addrspace(1) %1) #4
  store i8 %call1, ptr %f, align 1, !tbaa !10
  call void @llvm.lifetime.start.p0(i64 16, ptr %st) #3
  %2 = load ptr addrspace(1), ptr addrspace(1) @star, align 8, !tbaa !10
  call void @_Z18read_channel_intel11ocl_channel3Foo(ptr sret(%struct.Foo) align 8 %st, ptr addrspace(1) %2) #4
  call void @llvm.lifetime.start.p0(i64 4, ptr %ii) #3
  %3 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([5 x ptr addrspace(1)], ptr addrspace(1) @bar_arr, i64 0, i64 3), align 4, !tbaa !10
  %call2 = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %3) #4
  store i32 %call2, ptr %ii, align 4, !tbaa !13
  call void @llvm.lifetime.start.p0(i64 1, ptr %ff) #3
  %4 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([5 x [4 x ptr addrspace(1)]], ptr addrspace(1) @far_arr, i64 0, i64 3, i64 2), align 1, !tbaa !10
  %call3 = call signext i8 @_Z18read_channel_intel11ocl_channelc(ptr addrspace(1) %4) #4
  store i8 %call3, ptr %ff, align 1, !tbaa !10
  call void @llvm.lifetime.start.p0(i64 16, ptr %stst) #3
  %5 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([5 x [4 x [3 x ptr addrspace(1)]]], ptr addrspace(1) @star_arr, i64 0, i64 3, i64 2, i64 1), align 8, !tbaa !10
  call void @_Z18read_channel_intel11ocl_channel3Foo(ptr sret(%struct.Foo) align 8 %stst, ptr addrspace(1) %5) #4
  call void @llvm.lifetime.end.p0(i64 16, ptr %stst) #3
  call void @llvm.lifetime.end.p0(i64 1, ptr %ff) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr %ii) #3
  call void @llvm.lifetime.end.p0(i64 16, ptr %st) #3
  call void @llvm.lifetime.end.p0(i64 1, ptr %f) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #3
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: convergent nounwind
declare i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1)) #2

; Function Attrs: convergent nounwind
declare signext i8 @_Z18read_channel_intel11ocl_channelc(ptr addrspace(1)) #2

; Function Attrs: convergent nounwind
declare void @_Z18read_channel_intel11ocl_channel3Foo(ptr sret(%struct.Foo) align 8, ptr addrspace(1)) #2

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { convergent nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #3 = { nounwind }
attributes #4 = { convergent nounwind }

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
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
!13 = !{!14, !14, i64 0}
!14 = !{!"int", !11, i64 0}

; DEBUGIFY-NOT: WARNING: Missing line
