; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_altera_channels : enable
; channel int bar __attribute__((depth(0)));
; channel float far __attribute__((depth(3)));
;
; struct Foo {
;   int i;
; };
; channel struct Foo star;
;
; channel int bar_arr[5] __attribute__((depth(0)));
; channel float far_arr[5][4] __attribute__((depth(3)));
; channel struct Foo star_arr[5][4][3];
;
; __kernel void foo() {
;   int i = 42;
;   float f = 0.42f;
;   struct Foo st = {0};
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
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------
; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck --implicit-check-not write_channel_intel %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
%struct.Foo = type { i32 }

@bar = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@far = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@star = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@bar_arr = common addrspace(1) global [5 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4
@far_arr = common addrspace(1) global [5 x [4 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 4
@star_arr = common addrspace(1) global [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] zeroinitializer, align 4

; CHECK:      @[[PIPE_BAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK-NEXT: @[[PIPE_FAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK-NEXT: @[[PIPE_STAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK:      @[[PIPE_BAR_ARR:.*]] = common addrspace(1) global [5 x %opencl.pipe_t{{.*}} addrspace(1)*] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_FAR_ARR:.*]] = common addrspace(1) global [5 x [4 x %opencl.pipe_t{{.*}} addrspace(1)*]] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_STAR_ARR:.*]] = common addrspace(1) global [5 x [4 x [3 x %opencl.pipe_t{{.*}} addrspace(1)*]]] zeroinitializer, align 4

; All calls to read/write_channel_intel should be replaced by
; corresponding calls to pipe built-ins: check is done by --implicit-check-not
;
; CHECK: %[[LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[CAST_BAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_PIPE]]
; CHECK: call i32 @__write_pipe_2_bl{{.*}} %[[CAST_BAR_PIPE]]
;
; CHECK: %[[LOAD_FAR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR]]
; CHECK: %[[CAST_FAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_FAR_PIPE]]
; CHECK: call i32 @__write_pipe_2_bl{{.*}} %[[CAST_FAR_PIPE]]
;
; CHECK: %[[LOAD_STAR_PIPE:.*]] = load {{.*}} @[[PIPE_STAR]]
; CHECK: %[[CAST_STAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_STAR_PIPE]]
; CHECK: call i32 @__write_pipe_2_bl{{.*}} %[[CAST_STAR_PIPE]]
;
; CHECK: %[[LOAD_BAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_BAR_ARR]]
; CHECK: %[[CAST_BAR_PIPE_ARR:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_PIPE_ARR]]
; CHECK: call i32 @__write_pipe_2_bl{{.*}} %[[CAST_BAR_PIPE_ARR]]
;
; CHECK: %[[LOAD_FAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_FAR_ARR]]
; CHECK: %[[CAST_FAR_PIPE_ARR:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_FAR_PIPE_ARR]]
; CHECK: call i32 @__write_pipe_2_bl{{.*}} %[[CAST_FAR_PIPE_ARR]]
;
; CHECK: %[[LOAD_STAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_STAR_ARR]]
; CHECK: %[[CAST_STAR_PIPE_ARR:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_STAR_PIPE_ARR]]
; CHECK: call i32 @__write_pipe_2_bl{{.*}} %[[CAST_STAR_PIPE_ARR]]

; Function Attrs: nounwind
define spir_kernel void @foo() #0 !kernel_arg_addr_space !12 !kernel_arg_access_qual !12 !kernel_arg_type !12 !kernel_arg_base_type !12 !kernel_arg_type_qual !12 {
entry:
  %i = alloca i32, align 4
  %f = alloca float, align 4
  %st = alloca %struct.Foo, align 4
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  store i32 42, i32* %i, align 4, !tbaa !14
  %1 = bitcast float* %f to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #3
  store float 0x3FDAE147A0000000, float* %f, align 4, !tbaa !18
  %2 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #3
  %3 = bitcast %struct.Foo* %st to i8*
  call void @llvm.memset.p0i8.i64(i8* %3, i8 0, i64 4, i32 4, i1 false)
  %4 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !20
  %5 = load i32, i32* %i, align 4, !tbaa !14
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %4, i32 %5)
  %6 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @far, align 4, !tbaa !20
  %7 = load float, float* %f, align 4, !tbaa !18
  call void @_Z19write_channel_intel11ocl_channelff(%opencl.channel_t addrspace(1)* %6, float %7)
  %8 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @star, align 4, !tbaa !20
  call void @_Z19write_channel_intel11ocl_channel3FooS_(%opencl.channel_t addrspace(1)* %8, %struct.Foo* byval align 4 %st)
  %9 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x %opencl.channel_t addrspace(1)*], [5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, i64 0, i64 3), align 4, !tbaa !20
  %10 = load i32, i32* %i, align 4, !tbaa !14
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %9, i32 %10)
  %11 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x %opencl.channel_t addrspace(1)*]], [5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @far_arr, i64 0, i64 3, i64 2), align 4, !tbaa !20
  %12 = load float, float* %f, align 4, !tbaa !18
  call void @_Z19write_channel_intel11ocl_channelff(%opencl.channel_t addrspace(1)* %11, float %12)
  %13 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]], [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* @star_arr, i64 0, i64 3, i64 2, i64 1), align 4, !tbaa !20
  call void @_Z19write_channel_intel11ocl_channel3FooS_(%opencl.channel_t addrspace(1)* %13, %struct.Foo* byval align 4 %st)
  %14 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %14) #3
  %15 = bitcast float* %f to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %15) #3
  %16 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %16) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1) #1

declare void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #2

declare void @_Z19write_channel_intel11ocl_channelff(%opencl.channel_t addrspace(1)*, float) #2

declare void @_Z19write_channel_intel11ocl_channel3FooS_(%opencl.channel_t addrspace(1)*, %struct.Foo* byval align 4) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!opencl.channels = !{!0, !4, !6, !7, !8, !9}
!llvm.module.flags = !{!10}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!11}
!opencl.spir.version = !{!11}
!opencl.used.extensions = !{!12}
!opencl.used.optional.core.features = !{!12}
!opencl.compiler.options = !{!12}
!llvm.ident = !{!13}

!0 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @bar, !1, !2, !3}
!1 = !{!"packet_size", i32 4}
!2 = !{!"packet_align", i32 4}
!3 = !{!"depth", i32 0}
!4 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @far, !1, !2, !5}
!5 = !{!"depth", i32 3}
!6 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @star, !1, !2}
!7 = !{[5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, !1, !2, !3}
!8 = !{[5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @far_arr, !1, !2, !5}
!9 = !{[5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* @star_arr, !1, !2}
!10 = !{i32 1, !"wchar_size", i32 4}
!11 = !{i32 2, i32 0}
!12 = !{}
!13 = !{!"clang version 5.0.0 "}
!14 = !{!15, !15, i64 0}
!15 = !{!"int", !16, i64 0}
!16 = !{!"omnipotent char", !17, i64 0}
!17 = !{!"Simple C/C++ TBAA"}
!18 = !{!19, !19, i64 0}
!19 = !{!"float", !16, i64 0}
!20 = !{!16, !16, i64 0}
