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
; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck --implicit-check-not write_channel_nb_intel %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
%struct.Foo = type { i32, i64 }

@foo.l_valid = internal addrspace(3) global i8 undef, align 1
@bar = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0, !depth !1
@far = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 1, !packet_size !2, !packet_align !2, !depth !3
@g_valid = common addrspace(1) global i8 0, align 1
@star = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 8, !packet_size !4, !packet_align !5
@bar_arr = common addrspace(1) global [5 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4, !packet_size !0, !packet_align !0

; CHECK: @[[PIPE_BAR:.*]] = addrspace(1) global %opencl.pipe_rw_t{{.*}} addrspace(1)*
; CHECK: @[[PIPE_FAR:.*]] = addrspace(1) global %opencl.pipe_rw_t{{.*}} addrspace(1)*
; CHECK: @[[PIPE_STAR:.*]] = addrspace(1) global %opencl.pipe_rw_t{{.*}} addrspace(1)*
; CHECK: @[[PIPE_BAR_ARR:.*]] = addrspace(1) global [5 x %opencl.pipe_rw_t{{.*}} addrspace(1)*]

; All calls to read/write_channel_nb_intel should be replaced by
; corresponding calls to pipe built-ins: check is done using --implicit-check-not
;
; CHECK: %[[LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[CAST_BAR_PIPE:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_BAR_PIPE]] to %opencl.pipe_wo_t
; CHECK: %[[CALL_BAR_PIPE:.*]] = call i32 @__write_pipe_2_intel({{.*}} %[[CAST_BAR_PIPE]], {{.*}}, i32 4, i32 4)
; CHECK: %[[BOOL_CALL_BAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_BAR_PIPE]], 0
;
; CHECK: %[[LOAD_FAR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR]]
; CHECK: %[[CAST_FAR_PIPE:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_FAR_PIPE]] to %opencl.pipe_wo_t
; CHECK: %[[CALL_FAR_PIPE:.*]] = call i32 @__write_pipe_2_intel({{.*}} %[[CAST_FAR_PIPE]], {{.*}}, i32 1, i32 1)
; CHECK: %[[BOOL_CALL_FAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_FAR_PIPE]], 0
;
; CHECK: %[[LOAD_STAR_PIPE:.*]] = load {{.*}} @[[PIPE_STAR]]
; CHECK: %[[CAST_STAR_PIPE:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_STAR_PIPE]] to %opencl.pipe_wo_t
; CHECK: %[[CALL_STAR_PIPE:.*]] = call i32 @__write_pipe_2_intel({{.*}} %[[CAST_STAR_PIPE]], {{.*}}, i32 16, i32 8)
; CHECK: %[[BOOL_CALL_STAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_STAR_PIPE]], 0
;
; CHECK: %[[LOAD_BAR_ARR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR_ARR]]
; CHECK: %[[CAST_BAR_ARR_PIPE:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_BAR_ARR_PIPE]] to %opencl.pipe_wo_t
; CHECK: %[[CALL_BAR_ARR_PIPE:.*]] = call i32 @__write_pipe_2_intel({{.*}} %[[CAST_BAR_ARR_PIPE]], {{.*}}, i32 4, i32 4)
; CHECK: %[[BOOL_CALL_BAR_ARR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_BAR_ARR_PIPE]], 0

; Function Attrs: convergent nounwind
define spir_kernel void @foo() #0 !kernel_arg_addr_space !8 !kernel_arg_access_qual !8 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !8 !kernel_arg_pipe_depth !8 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 {
entry:
  %i = alloca i32, align 4
  %f = alloca i8, align 1
  %st = alloca %struct.Foo, align 8
  %valid = alloca i8, align 1
  %p_valid = alloca i8, align 1
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  store i32 42, i32* %i, align 4, !tbaa !10
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %f) #3
  store i8 10, i8* %f, align 1, !tbaa !14
  %1 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* %1) #3
  %2 = bitcast %struct.Foo* %st to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %2, i8 0, i64 16, i1 false)
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %valid) #3
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %p_valid) #3
  %3 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !14
  %4 = load i32, i32* %i, align 4, !tbaa !10
  %call = call zeroext i1 @_Z22write_channel_nb_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %3, i32 %4) #4
  %frombool = zext i1 %call to i8
  store i8 %frombool, i8* %valid, align 1, !tbaa !15
  %5 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @far, align 1, !tbaa !14
  %6 = load i8, i8* %f, align 1, !tbaa !14
  %call1 = call zeroext i1 @_Z22write_channel_nb_intel11ocl_channelcc(%opencl.channel_t addrspace(1)* %5, i8 signext %6) #4
  %frombool2 = zext i1 %call1 to i8
  store i8 %frombool2, i8 addrspace(1)* @g_valid, align 1, !tbaa !15
  %7 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @star, align 8, !tbaa !14
  %call3 = call zeroext i1 @_Z22write_channel_nb_intel11ocl_channel3FooS_(%opencl.channel_t addrspace(1)* %7, %struct.Foo* byval align 8 %st) #4
  %frombool4 = zext i1 %call3 to i8
  store i8 %frombool4, i8* %p_valid, align 1, !tbaa !15
  %8 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x %opencl.channel_t addrspace(1)*], [5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, i64 0, i64 3), align 4, !tbaa !14
  %9 = load i32, i32* %i, align 4, !tbaa !10
  %call5 = call zeroext i1 @_Z22write_channel_nb_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %8, i32 %9) #4
  %frombool6 = zext i1 %call5 to i8
  store i8 %frombool6, i8 addrspace(3)* @foo.l_valid, align 1, !tbaa !15
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %p_valid) #3
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %valid) #3
  %10 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %10) #3
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %f) #3
  %11 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %11) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1) #1

; Function Attrs: convergent
declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #2

; Function Attrs: convergent
declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channelcc(%opencl.channel_t addrspace(1)*, i8 signext) #2

; Function Attrs: convergent
declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channel3FooS_(%opencl.channel_t addrspace(1)*, %struct.Foo* byval align 8) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { convergent }

!llvm.module.flags = !{!6}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!7}
!opencl.spir.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}
!llvm.ident = !{!9}

!0 = !{i32 4}
!1 = !{i32 0}
!2 = !{i32 1}
!3 = !{i32 3}
!4 = !{i32 16}
!5 = !{i32 8}
!6 = !{i32 1, !"wchar_size", i32 4}
!7 = !{i32 2, i32 0}
!8 = !{}
!9 = !{!"clang version 7.0.0 "}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
!14 = !{!12, !12, i64 0}
!15 = !{!16, !16, i64 0}
!16 = !{!"bool", !12, i64 0}
