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
; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck --implicit-check-not read_channel_nb_intel %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%struct.Foo = type { i32, i64 }
%opencl.channel_t = type opaque

@foo.st = private unnamed_addr addrspace(2) constant %struct.Foo { i32 0, i64 1 }, align 8
@foo.l_valid = internal addrspace(3) global i8 undef, align 1
@g_valid = common addrspace(1) global i8 0, align 1
@bar = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0, !depth !1
@far = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 1, !packet_size !2, !packet_align !2, !depth !3
@star = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 8, !packet_size !4, !packet_align !5
@bar_arr = common addrspace(1) global [5 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4, !packet_size !0, !packet_align !0
@far_arr = common addrspace(1) global [5 x [4 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 1, !packet_size !2, !packet_align !2

; CHECK: @[[PIPE_BAR:.*]] = addrspace(1) global %opencl.pipe_rw_t{{.*}} addrspace(1)*
; CHECK: @[[PIPE_FAR:.*]] = addrspace(1) global %opencl.pipe_rw_t{{.*}} addrspace(1)*
; CHECK: @[[PIPE_STAR:.*]] = addrspace(1) global %opencl.pipe_rw_t{{.*}} addrspace(1)*
; CHECK: @[[PIPE_BAR_ARR:.*]] = addrspace(1) global [5 x %opencl.pipe_rw_t{{.*}} addrspace(1)*]
; CHECK: @[[PIPE_FAR_ARR:.*]] = addrspace(1) global [5 x [4 x %opencl.pipe_rw_t{{.*}} addrspace(1)*]]

; All calls to read/write_channel_nb_intel should be replaced by
; corresponding calls to pipe built-ins: check is done using --implicit-check-not
;
; CHECK: %[[LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[VALID:.*]] = addrspacecast {{.*}} %valid
; CHECK: %[[CAST_BAR_PIPE:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_BAR_PIPE]] to %opencl.pipe_ro_t
; CHECK: %[[CALL_BAR_PIPE:.*]] = call i32 @__read_pipe_2_fpga({{.*}} %[[CAST_BAR_PIPE]], {{.*}}, i32 4, i32 4)
; CHECK: %[[BOOL_CALL_BAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_BAR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_BAR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_BAR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_BAR_PIPE]], {{.*}}* %[[VALID]]
;

; CHECK: %[[LOAD_FAR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR]]
; CHECK: %[[LOAD_P_TO_VALID:.*]] = load {{.*}} %p_to_valid
; CHECK: %[[CAST_FAR_PIPE:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_FAR_PIPE]] to %opencl.pipe_ro_t
; CHECK: %[[CALL_FAR_PIPE:.*]] = call i32 @__read_pipe_2_fpga({{.*}} %[[CAST_FAR_PIPE]], {{.*}}, i32 1, i32 1)
; CHECK: %[[BOOL_CALL_FAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_FAR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_FAR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_FAR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_FAR_PIPE]], {{.*}}* %[[LOAD_P_TO_VALID]]
;
; CHECK: %[[LOAD_STAR_PIPE:.*]] = load {{.*}} @[[PIPE_STAR]]
; CHECK: %[[LOAD_P_TO_G_VALID:.*]] = load {{.*}} %p_to_g_valid
; CHECK: %[[P_TO_G_VALID:.*]] = addrspacecast {{.*}} %[[LOAD_P_TO_G_VALID]]
; CHECK: %[[CAST_STAR_PIPE:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_STAR_PIPE]] to %opencl.pipe_ro_t
; CHECK: %[[CALL_STAR_PIPE:.*]] = call i32 @__read_pipe_2_fpga({{.*}} %[[CAST_STAR_PIPE]], {{.*}}, i32 16, i32 8)
; CHECK: %[[BOOL_CALL_STAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_STAR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_STAR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_STAR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_STAR_PIPE]], {{.*}}* %[[P_TO_G_VALID]]
;
; CHECK: %[[LOAD_BAR_ARR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR_ARR]]
; CHECK: %[[LOAD_P_TO_L_VALID:.*]] = load {{.*}} %p_to_l_valid
; CHECK: %[[P_TO_L_VALID:.*]] = addrspacecast {{.*}} %[[LOAD_P_TO_L_VALID]]
; CHECK: %[[CAST_BAR_ARR_PIPE:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_BAR_ARR_PIPE]] to %opencl.pipe_ro_t
; CHECK: %[[CALL_BAR_ARR_PIPE:.*]] = call i32 @__read_pipe_2_fpga({{.*}} %[[CAST_BAR_ARR_PIPE]], {{.*}}, i32 4, i32 4)
; CHECK: %[[BOOL_CALL_BAR_ARR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_BAR_ARR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_BAR_ARR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_BAR_ARR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_BAR_ARR_PIPE]], {{.*}}* %[[P_TO_L_VALID]]
;
; CHECK: %[[LOAD_FAR_ARR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR_ARR]]
; CHECK: %[[LOAD_P_TO_P_VALID:.*]] = load {{.*}} %p_to_p_valid
; CHECK: %[[P_TO_P_VALID:.*]] = addrspacecast {{.*}} %[[LOAD_P_TO_P_VALID]]
; CHECK: %[[CAST_FAR_ARR_PIPE:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_FAR_ARR_PIPE]] to %opencl.pipe_ro_t
; CHECK: %[[CALL_FAR_ARR_PIPE:.*]] = call i32 @__read_pipe_2_fpga({{.*}} %[[CAST_FAR_ARR_PIPE]], {{.*}}, i32 1, i32 1)
; CHECK: %[[BOOL_CALL_FAR_ARR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_FAR_ARR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_FAR_ARR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_FAR_ARR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_FAR_ARR_PIPE]], {{.*}}* %[[P_TO_P_VALID]]

; Function Attrs: convergent nounwind
define spir_kernel void @foo() #0 !kernel_arg_addr_space !8 !kernel_arg_access_qual !8 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !8 !kernel_arg_pipe_depth !8 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 {
entry:
  %i = alloca i32, align 4
  %f = alloca i8, align 1
  %st = alloca %struct.Foo, align 8
  %valid = alloca i8, align 1
  %p_valid = alloca i8, align 1
  %p_to_valid = alloca i8 addrspace(4)*, align 8
  %p_to_g_valid = alloca i8 addrspace(1)*, align 8
  %p_to_l_valid = alloca i8 addrspace(3)*, align 8
  %p_to_p_valid = alloca i8*, align 8
  %tmp = alloca %struct.Foo, align 8
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  store i32 42, i32* %i, align 4, !tbaa !10
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %f) #3
  store i8 10, i8* %f, align 1, !tbaa !14
  %1 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* %1) #3
  %2 = bitcast %struct.Foo* %st to i8*
  call void @llvm.memcpy.p0i8.p2i8.i64(i8* align 8 %2, i8 addrspace(2)* align 8 bitcast (%struct.Foo addrspace(2)* @foo.st to i8 addrspace(2)*), i64 16, i1 false)
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %valid) #3
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %p_valid) #3
  %3 = bitcast i8 addrspace(4)** %p_to_valid to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %3) #3
  %4 = addrspacecast i8* %valid to i8 addrspace(4)*
  store i8 addrspace(4)* %4, i8 addrspace(4)** %p_to_valid, align 8, !tbaa !15
  %5 = bitcast i8 addrspace(1)** %p_to_g_valid to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %5) #3
  store i8 addrspace(1)* @g_valid, i8 addrspace(1)** %p_to_g_valid, align 8, !tbaa !15
  %6 = bitcast i8 addrspace(3)** %p_to_l_valid to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %6) #3
  store i8 addrspace(3)* @foo.l_valid, i8 addrspace(3)** %p_to_l_valid, align 8, !tbaa !15
  %7 = bitcast i8** %p_to_p_valid to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %7) #3
  store i8* %p_valid, i8** %p_to_p_valid, align 8, !tbaa !15
  %8 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !14
  %9 = addrspacecast i8* %valid to i8 addrspace(4)*
  %call = call i32 @_Z21read_channel_nb_intel11ocl_channeliPU3AS4b(%opencl.channel_t addrspace(1)* %8, i8 addrspace(4)* %9) #4
  store i32 %call, i32* %i, align 4, !tbaa !10
  %10 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @far, align 1, !tbaa !14
  %11 = load i8 addrspace(4)*, i8 addrspace(4)** %p_to_valid, align 8, !tbaa !15
  %call1 = call signext i8 @_Z21read_channel_nb_intel11ocl_channelcPU3AS4b(%opencl.channel_t addrspace(1)* %10, i8 addrspace(4)* %11) #4
  store i8 %call1, i8* %f, align 1, !tbaa !14
  %12 = bitcast %struct.Foo* %tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* %12) #3
  %13 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @star, align 8, !tbaa !14
  %14 = load i8 addrspace(1)*, i8 addrspace(1)** %p_to_g_valid, align 8, !tbaa !15
  %15 = addrspacecast i8 addrspace(1)* %14 to i8 addrspace(4)*
  call void @_Z21read_channel_nb_intel11ocl_channel3FooPU3AS4b(%struct.Foo* sret %tmp, %opencl.channel_t addrspace(1)* %13, i8 addrspace(4)* %15) #4
  %16 = bitcast %struct.Foo* %st to i8*
  %17 = bitcast %struct.Foo* %tmp to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %16, i8* align 8 %17, i64 16, i1 false), !tbaa.struct !17
  %18 = bitcast %struct.Foo* %tmp to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %18) #3
  %19 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x %opencl.channel_t addrspace(1)*], [5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, i64 0, i64 3), align 4, !tbaa !14
  %20 = load i8 addrspace(3)*, i8 addrspace(3)** %p_to_l_valid, align 8, !tbaa !15
  %21 = addrspacecast i8 addrspace(3)* %20 to i8 addrspace(4)*
  %call2 = call i32 @_Z21read_channel_nb_intel11ocl_channeliPU3AS4b(%opencl.channel_t addrspace(1)* %19, i8 addrspace(4)* %21) #4
  store i32 %call2, i32* %i, align 4, !tbaa !10
  %22 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x %opencl.channel_t addrspace(1)*]], [5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @far_arr, i64 0, i64 3, i64 2), align 1, !tbaa !14
  %23 = load i8*, i8** %p_to_p_valid, align 8, !tbaa !15
  %24 = addrspacecast i8* %23 to i8 addrspace(4)*
  %call3 = call signext i8 @_Z21read_channel_nb_intel11ocl_channelcPU3AS4b(%opencl.channel_t addrspace(1)* %22, i8 addrspace(4)* %24) #4
  store i8 %call3, i8* %f, align 1, !tbaa !14
  %25 = bitcast i8** %p_to_p_valid to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %25) #3
  %26 = bitcast i8 addrspace(3)** %p_to_l_valid to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %26) #3
  %27 = bitcast i8 addrspace(1)** %p_to_g_valid to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %27) #3
  %28 = bitcast i8 addrspace(4)** %p_to_valid to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %28) #3
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %p_valid) #3
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %valid) #3
  %29 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %29) #3
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %f) #3
  %30 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %30) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p2i8.i64(i8* nocapture writeonly, i8 addrspace(2)* nocapture readonly, i64, i1) #1

; Function Attrs: convergent
declare i32 @_Z21read_channel_nb_intel11ocl_channeliPU3AS4b(%opencl.channel_t addrspace(1)*, i8 addrspace(4)*) #2

; Function Attrs: convergent
declare signext i8 @_Z21read_channel_nb_intel11ocl_channelcPU3AS4b(%opencl.channel_t addrspace(1)*, i8 addrspace(4)*) #2

; Function Attrs: convergent
declare void @_Z21read_channel_nb_intel11ocl_channel3FooPU3AS4b(%struct.Foo* sret, %opencl.channel_t addrspace(1)*, i8 addrspace(4)*) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1) #1

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
!16 = !{!"any pointer", !12, i64 0}
!17 = !{i64 0, i64 4, !10, i64 8, i64 8, !18}
!18 = !{!19, !19, i64 0}
!19 = !{!"long", !12, i64 0}
