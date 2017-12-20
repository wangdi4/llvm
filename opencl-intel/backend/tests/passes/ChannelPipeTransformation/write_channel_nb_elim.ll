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
; channel int bar_arr[5];
;
; __global bool g_valid;
;
; __kernel void foo() {
;   int i = 42;
;   float f = 0.42f;
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
; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck --implicit-check-not write_channel_nb_intel %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
%struct.Foo = type { i32 }

@foo.l_valid = internal addrspace(3) global i8 undef, align 1
@bar = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@far = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@g_valid = common addrspace(1) global i8 0, align 1
@star = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@bar_arr = common addrspace(1) global [5 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4

; CHECK:      @[[PIPE_BAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK-NEXT: @[[PIPE_FAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK-NEXT: @[[PIPE_STAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK-NEXT: @[[PIPE_BAR_ARR:.*]] = common addrspace(1) global [5 x %opencl.pipe_t{{.*}} addrspace(1)*] zeroinitializer, align 4

; All calls to read/write_channel_nb_intel should be replaced by
; corresponding calls to pipe built-ins: check is done using --implicit-check-not
;
; CHECK: %[[LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[CAST_BAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_PIPE]]
; CHECK: %[[CALL_BAR_PIPE:.*]] = call i32 @__write_pipe_2{{.*}} %[[CAST_BAR_PIPE]]
; CHECK: %[[BOOL_CALL_BAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_BAR_PIPE]], 0
;
; CHECK: %[[LOAD_FAR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR]]
; CHECK: %[[CAST_FAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_FAR_PIPE]]
; CHECK: %[[CALL_FAR_PIPE:.*]] = call i32 @__write_pipe_2{{.*}} %[[CAST_FAR_PIPE]]
; CHECK: %[[BOOL_CALL_FAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_FAR_PIPE]], 0
;
; CHECK: %[[LOAD_STAR_PIPE:.*]] = load {{.*}} @[[PIPE_STAR]]
; CHECK: %[[CAST_STAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_STAR_PIPE]]
; CHECK: %[[CALL_STAR_PIPE:.*]] = call i32 @__write_pipe_2{{.*}} %[[CAST_STAR_PIPE]]
; CHECK: %[[BOOL_CALL_STAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_STAR_PIPE]], 0
;
; CHECK: %[[LOAD_BAR_ARR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR_ARR]]
; CHECK: %[[CAST_BAR_ARR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_ARR_PIPE]]
; CHECK: %[[CALL_BAR_ARR_PIPE:.*]] = call i32 @__write_pipe_2{{.*}} %[[CAST_BAR_ARR_PIPE]]
; CHECK: %[[BOOL_CALL_BAR_ARR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_BAR_ARR_PIPE]], 0

; Function Attrs: nounwind
define spir_kernel void @foo() #0 !kernel_arg_addr_space !10 !kernel_arg_access_qual !10 !kernel_arg_type !10 !kernel_arg_base_type !10 !kernel_arg_type_qual !10 {
entry:
  %i = alloca i32, align 4
  %f = alloca float, align 4
  %st = alloca %struct.Foo, align 4
  %valid = alloca i8, align 1
  %p_valid = alloca i8, align 1
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  store i32 42, i32* %i, align 4, !tbaa !12
  %1 = bitcast float* %f to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #3
  store float 0x3FDAE147A0000000, float* %f, align 4, !tbaa !16
  %2 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #3
  %3 = bitcast %struct.Foo* %st to i8*
  call void @llvm.memset.p0i8.i64(i8* %3, i8 0, i64 4, i32 4, i1 false)
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %valid) #3
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %p_valid) #3
  %4 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !18
  %5 = load i32, i32* %i, align 4, !tbaa !12
  %call = call zeroext i1 @_Z22write_channel_nb_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %4, i32 %5)
  %frombool = zext i1 %call to i8
  store i8 %frombool, i8* %valid, align 1, !tbaa !19
  %6 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @far, align 4, !tbaa !18
  %7 = load float, float* %f, align 4, !tbaa !16
  %call1 = call zeroext i1 @_Z22write_channel_nb_intel11ocl_channelff(%opencl.channel_t addrspace(1)* %6, float %7)
  %frombool2 = zext i1 %call1 to i8
  store i8 %frombool2, i8 addrspace(1)* @g_valid, align 1, !tbaa !19
  %8 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @star, align 4, !tbaa !18
  %call3 = call zeroext i1 @_Z22write_channel_nb_intel11ocl_channel3FooS_(%opencl.channel_t addrspace(1)* %8, %struct.Foo* byval align 4 %st)
  %frombool4 = zext i1 %call3 to i8
  store i8 %frombool4, i8* %p_valid, align 1, !tbaa !19
  %9 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x %opencl.channel_t addrspace(1)*], [5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, i64 0, i64 3), align 4, !tbaa !18
  %10 = load i32, i32* %i, align 4, !tbaa !12
  %call5 = call zeroext i1 @_Z22write_channel_nb_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %9, i32 %10)
  %frombool6 = zext i1 %call5 to i8
  store i8 %frombool6, i8 addrspace(3)* @foo.l_valid, align 1, !tbaa !19
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %p_valid) #3
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %valid) #3
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

declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #2

declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channelff(%opencl.channel_t addrspace(1)*, float) #2

declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channel3FooS_(%opencl.channel_t addrspace(1)*, %struct.Foo* byval align 4) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!opencl.channels = !{!0, !4, !6, !7}
!llvm.module.flags = !{!8}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!9}
!opencl.spir.version = !{!9}
!opencl.used.extensions = !{!10}
!opencl.used.optional.core.features = !{!10}
!opencl.compiler.options = !{!10}
!llvm.ident = !{!11}

!0 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @bar, !1, !2, !3}
!1 = !{!"packet_size", i32 4}
!2 = !{!"packet_align", i32 4}
!3 = !{!"depth", i32 0}
!4 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @far, !1, !2, !5}
!5 = !{!"depth", i32 3}
!6 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @star, !1, !2}
!7 = !{[5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, !1, !2}
!8 = !{i32 1, !"wchar_size", i32 4}
!9 = !{i32 2, i32 0}
!10 = !{}
!11 = !{!"clang version 5.0.0 "}
!12 = !{!13, !13, i64 0}
!13 = !{!"int", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C/C++ TBAA"}
!16 = !{!17, !17, i64 0}
!17 = !{!"float", !14, i64 0}
!18 = !{!14, !14, i64 0}
!19 = !{!20, !20, i64 0}
!20 = !{!"bool", !14, i64 0}
