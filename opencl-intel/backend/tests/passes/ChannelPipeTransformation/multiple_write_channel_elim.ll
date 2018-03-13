; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int bar;
;
; struct Foo {
;   int i;
; };
;
; channel struct Foo star_arr[5][4][3];
;
; __kernel void foo() {
;   int i = 42;
;   float f = 0.42f;
;   struct Foo st = {0};
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
; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck --implicit-check-not write_channel_intel %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
%struct.Foo = type { i32 }

@bar = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@star_arr = common addrspace(1) global [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] zeroinitializer, align 4, !packet_size !0, !packet_align !0

; CHECK: @[[PIPE_BAR:.*]] = addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK: @[[PIPE_STAR_ARR:.*]] = addrspace(1) global [5 x [4 x [3 x %opencl.pipe_t{{.*}} addrspace(1)*]]]
;
; All calls to read/write_channel_intel should be replaced by
; corresponding calls to pipe built-ins: check is done by --implicit-check-not
;
; CHECK: %[[LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[CAST_BAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_PIPE]]
; CHECK: call i32 @__write_pipe_2_bl{{.*}} %[[CAST_BAR_PIPE]]
;
; CHECK: %[[LOAD_STAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_STAR_ARR]]
; CHECK: %[[CAST_STAR_PIPE_ARR:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_STAR_PIPE_ARR]]
; CHECK: call i32 @__write_pipe_2_bl{{.*}} %[[CAST_STAR_PIPE_ARR]]
;
; CHECK: %[[LOAD_BAR_PIPE1:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[CAST_BAR_PIPE1:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_PIPE1]]
; CHECK: call i32 @__write_pipe_2_bl{{.*}} %[[CAST_BAR_PIPE1]]
;
; CHECK: %[[LOAD_STAR_PIPE_ARR1:.*]] = load {{.*}} @[[PIPE_STAR_ARR]]
; CHECK: %[[CAST_STAR_PIPE_ARR1:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_STAR_PIPE_ARR1]]
; CHECK: call i32 @__write_pipe_2_bl{{.*}} %[[CAST_STAR_PIPE_ARR1]]

; Function Attrs: convergent nounwind
define spir_kernel void @foo() #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !3 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !3 !kernel_arg_host_accessible !3 !kernel_arg_pipe_depth !3 !kernel_arg_pipe_io !3 !kernel_arg_buffer_location !3 {
  %i = alloca i32, align 4
  %f = alloca float, align 4
  %st = alloca %struct.Foo, align 4
  %1 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #3
  store i32 42, i32* %i, align 4, !tbaa !5
  %2 = bitcast float* %f to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #3
  store float 0x3FDAE147A0000000, float* %f, align 4, !tbaa !9
  %3 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #3
  %4 = bitcast %struct.Foo* %st to i8*
  call void @llvm.memset.p0i8.i64(i8* %4, i8 0, i64 4, i32 4, i1 false)
  %5 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !11
  %6 = load i32, i32* %i, align 4, !tbaa !5
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %5, i32 %6) #4
  %7 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]], [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* @star_arr, i64 0, i64 3, i64 2, i64 1), align 4, !tbaa !11
  call void @_Z19write_channel_intel11ocl_channel3FooS_(%opencl.channel_t addrspace(1)* %7, %struct.Foo* byval align 4 %st) #4
  %8 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !11
  %9 = load i32, i32* %i, align 4, !tbaa !5
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %8, i32 %9) #4
  %10 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]], [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* @star_arr, i64 0, i64 3, i64 2, i64 1), align 4, !tbaa !11
  call void @_Z19write_channel_intel11ocl_channel3FooS_(%opencl.channel_t addrspace(1)* %10, %struct.Foo* byval align 4 %st) #4
  %11 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %11) #3
  %12 = bitcast float* %f to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %12) #3
  %13 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %13) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1) #1

; Function Attrs: convergent
declare void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #2

; Function Attrs: convergent
declare void @_Z19write_channel_intel11ocl_channel3FooS_(%opencl.channel_t addrspace(1)*, %struct.Foo* byval align 4) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { convergent }

!llvm.module.flags = !{!1}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!2}
!opencl.spir.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 4}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, i32 2}
!3 = !{}
!4 = !{!"clang version 6.0.0 "}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!10, !10, i64 0}
!10 = !{!"float", !7, i64 0}
!11 = !{!7, !7, i64 0}

