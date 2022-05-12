; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int bar;
;
; struct Foo {
;   int i;
;   float f;
; };
;
; channel struct Foo star_arr[5][4][3];
;
; __kernel void foo() {
;   int i = 42;
;   struct Foo st = {0, 1};
;
;   write_channel_intel(bar, i);
;   write_channel_intel(star_arr[3][2][1], st);
;
;   write_channel_intel(bar, i);
;   write_channel_intel(star_arr[3][2][1], st);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL1.2
; ----------------------------------------------------
; RUN: llvm-as %p/../Inputs/fpga-pipes.rtl -o %t.rtl.bc
; RUN: opt -enable-new-pm=0 -dpcpp-kernel-builtin-lib=%t.rtl.bc -dpcpp-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -enable-new-pm=0 -dpcpp-kernel-builtin-lib=%t.rtl.bc -dpcpp-kernel-channel-pipe-transformation -verify %s -S | FileCheck --implicit-check-not write_channel_intel %s
; RUN: opt -dpcpp-kernel-builtin-lib=%t.rtl.bc -passes=dpcpp-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-builtin-lib=%t.rtl.bc -passes=dpcpp-kernel-channel-pipe-transformation %s -S | FileCheck --implicit-check-not write_channel_intel %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%struct.Foo = type { i32, float }
%opencl.channel_t = type opaque

@foo.st = private unnamed_addr addrspace(2) constant %struct.Foo { i32 0, float 1.000000e+00 }, align 4
@bar = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@star_arr = common addrspace(1) global [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] zeroinitializer, align 4, !packet_size !1, !packet_align !0

; CHECK: @[[PIPE_BAR:.*]] = addrspace(1) global %opencl.pipe_rw_t{{.*}} addrspace(1)*
; CHECK: @[[PIPE_STAR_ARR:.*]] = addrspace(1) global [5 x [4 x [3 x %opencl.pipe_rw_t{{.*}} addrspace(1)*]]]
;
; All calls to read/write_channel_intel should be replaced by
; corresponding calls to pipe built-ins: check is done by --implicit-check-not
;
; CHECK: %[[LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[CAST_BAR_PIPE:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_BAR_PIPE]] to %opencl.pipe_wo_t
; CHECK: call i32 @__write_pipe_2_bl_fpga({{.*}} %[[CAST_BAR_PIPE]], {{.*}}, i32 4, i32 4)
;
; CHECK: %[[LOAD_STAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_STAR_ARR]]
; CHECK: %[[CAST_STAR_PIPE_ARR:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_STAR_PIPE_ARR]] to %opencl.pipe_wo_t
; CHECK: call i32 @__write_pipe_2_bl_fpga({{.*}} %[[CAST_STAR_PIPE_ARR]], {{.*}}, i32 8, i32 4)
;
; CHECK: %[[LOAD_BAR_PIPE1:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[CAST_BAR_PIPE1:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_BAR_PIPE1]] to %opencl.pipe_wo_t
; CHECK: call i32 @__write_pipe_2_bl_fpga({{.*}} %[[CAST_BAR_PIPE1]], {{.*}}, i32 4, i32 4)
;
; CHECK: %[[LOAD_STAR_PIPE_ARR1:.*]] = load {{.*}} @[[PIPE_STAR_ARR]]
; CHECK: %[[CAST_STAR_PIPE_ARR1:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_STAR_PIPE_ARR1]] to %opencl.pipe_wo_t
; CHECK: call i32 @__write_pipe_2_bl_fpga({{.*}} %[[CAST_STAR_PIPE_ARR1]], {{.*}}, i32 8, i32 4)

; Function Attrs: convergent nounwind
define spir_kernel void @foo() #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !4 !kernel_arg_pipe_depth !4 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 {
entry:
  %i = alloca i32, align 4
  %st = alloca %struct.Foo, align 4
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  store i32 42, i32* %i, align 4, !tbaa !6
  %1 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %1) #3
  %2 = bitcast %struct.Foo* %st to i8*
  call void @llvm.memcpy.p0i8.p2i8.i64(i8* align 4 %2, i8 addrspace(2)* align 4 bitcast (%struct.Foo addrspace(2)* @foo.st to i8 addrspace(2)*), i64 8, i1 false)
  %3 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !10
  %4 = load i32, i32* %i, align 4, !tbaa !6
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %3, i32 %4) #4
  %5 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]], [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* @star_arr, i64 0, i64 3, i64 2, i64 1), align 4, !tbaa !10
  call void @_Z19write_channel_intel11ocl_channel3FooS_(%opencl.channel_t addrspace(1)* %5, %struct.Foo* byval(%struct.Foo) align 4 %st) #4
  %6 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !10
  %7 = load i32, i32* %i, align 4, !tbaa !6
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %6, i32 %7) #4
  %8 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]], [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* @star_arr, i64 0, i64 3, i64 2, i64 1), align 4, !tbaa !10
  call void @_Z19write_channel_intel11ocl_channel3FooS_(%opencl.channel_t addrspace(1)* %8, %struct.Foo* byval(%struct.Foo) align 4 %st) #4
  %9 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %9) #3
  %10 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %10) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p2i8.i64(i8* nocapture writeonly, i8 addrspace(2)* nocapture readonly, i64, i1) #1

; Function Attrs: convergent
declare void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #2

; Function Attrs: convergent
declare void @_Z19write_channel_intel11ocl_channel3FooS_(%opencl.channel_t addrspace(1)*, %struct.Foo* byval(%struct.Foo) align 4) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { convergent }

!llvm.module.flags = !{!2}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!3}
!opencl.spir.version = !{!3}
!opencl.used.extensions = !{!4}
!opencl.used.optional.core.features = !{!4}
!opencl.compiler.options = !{!4}
!llvm.ident = !{!5}

!0 = !{i32 4}
!1 = !{i32 8}
!2 = !{i32 1, !"wchar_size", i32 4}
!3 = !{i32 1, i32 2}
!4 = !{}
!5 = !{!"clang version 7.0.0 "}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!8, !8, i64 0}

; DEBUGIFY-NOT: WARNING: Missing line
