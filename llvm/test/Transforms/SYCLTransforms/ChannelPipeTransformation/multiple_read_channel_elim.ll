; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int bar;
;
; struct Foo {
;   int i;
;   float j;
; };
;
; channel struct Foo star_arr[5][4][3];
;
; __kernel void foo() {
;   int i = read_channel_intel(bar);
;
;   struct Foo stst = read_channel_intel(star_arr[3][2][1]);
;
;   int j = read_channel_intel(bar);
;
;   struct Foo stst1 = read_channel_intel(star_arr[3][2][1]);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL1.2
; ----------------------------------------------------
; RUN: llvm-as %p/../Inputs/fpga-pipes.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck --implicit-check-not read_channel_intel %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

%struct.Foo = type { i32, float }

@bar = addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0
@star_arr = addrspace(1) global [5 x [4 x [3 x target("spirv.Channel")]]] zeroinitializer, align 4, !packet_size !1, !packet_align !0

; CHECK: @[[PIPE_BAR:.*]] = addrspace(1) global ptr addrspace(1)
; CHECK: @[[PIPE_STAR_ARR:.*]] = addrspace(1) global [5 x [4 x [3 x ptr addrspace(1)]]]
;
; All calls to read/write_channel_intel should be replaced by
; corresponding calls to pipe built-ins: check is done by --implicit-check-not
;
; CHECK: %[[LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: call i32 @__read_pipe_2_bl_fpga({{.*}} %[[LOAD_BAR_PIPE]], {{.*}}, i32 4, i32 4)
;
; CHECK: %[[LOAD_STAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_STAR_ARR]]
; CHECK: call i32 @__read_pipe_2_bl_fpga({{.*}} %[[LOAD_STAR_PIPE_ARR]], {{.*}}, i32 8, i32 4)
;
; CHECK: %[[LOAD_BAR_PIPE1:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: call i32 @__read_pipe_2_bl_fpga({{.*}} %[[LOAD_BAR_PIPE1]], {{.*}}, i32 4, i32 4)
;
; CHECK: %[[LOAD_STAR_PIPE_ARR1:.*]] = load {{.*}} @[[PIPE_STAR_ARR]]
; CHECK: call i32 @__read_pipe_2_bl_fpga({{.*}} %[[LOAD_STAR_PIPE_ARR1]], {{.*}}, i32 8, i32 4)

; Function Attrs: convergent norecurse nounwind
define dso_local void @foo() #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !3 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !3 !kernel_arg_host_accessible !3 !kernel_arg_pipe_depth !3 !kernel_arg_pipe_io !3 !kernel_arg_buffer_location !3 !arg_type_null_val !3 {
entry:
  %i = alloca i32, align 4
  %stst = alloca %struct.Foo, align 4
  %j = alloca i32, align 4
  %stst1 = alloca %struct.Foo, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #3
  %0 = load ptr addrspace(1), ptr addrspace(1) @bar, align 4, !tbaa !6
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %0) #4
  store i32 %call, ptr %i, align 4, !tbaa !9
  call void @llvm.lifetime.start.p0(i64 8, ptr %stst) #3
  %1 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([5 x [4 x [3 x target("spirv.Channel")]]], ptr addrspace(1) @star_arr, i64 0, i64 3, i64 2, i64 1), align 4, !tbaa !6
  call void @_Z18read_channel_intel11ocl_channel3Foo(ptr sret(%struct.Foo) align 4 %stst, ptr addrspace(1) %1) #4
  call void @llvm.lifetime.start.p0(i64 4, ptr %j) #3
  %2 = load ptr addrspace(1), ptr addrspace(1) @bar, align 4, !tbaa !6
  %call1 = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %2) #4
  store i32 %call1, ptr %j, align 4, !tbaa !9
  call void @llvm.lifetime.start.p0(i64 8, ptr %stst1) #3
  %3 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([5 x [4 x [3 x target("spirv.Channel")]]], ptr addrspace(1) @star_arr, i64 0, i64 3, i64 2, i64 1), align 4, !tbaa !6
  call void @_Z18read_channel_intel11ocl_channel3Foo(ptr sret(%struct.Foo) align 4 %stst1, ptr addrspace(1) %3) #4
  call void @llvm.lifetime.end.p0(i64 8, ptr %stst1) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr %j) #3
  call void @llvm.lifetime.end.p0(i64 8, ptr %stst) #3
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
declare void @_Z18read_channel_intel11ocl_channel3Foo(ptr sret(%struct.Foo) align 4, ptr addrspace(1)) #2

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { convergent nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #3 = { nounwind }
attributes #4 = { convergent nounwind }

!opencl.ocl.version = !{!2}
!opencl.spir.version = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!sycl.kernels = !{!5}

!0 = !{i32 4}
!1 = !{i32 8}
!2 = !{i32 1, i32 2}
!3 = !{}
!4 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!5 = !{ptr @foo}
!6 = !{!7, !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !7, i64 0}

; DEBUGIFY-NOT: WARNING: Missing line
