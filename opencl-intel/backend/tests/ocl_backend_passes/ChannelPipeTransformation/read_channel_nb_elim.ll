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
; channel float far_arr[5][4];
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

%opencl.channel_t = type opaque
%struct.Foo = type { i32 }

@foo.l_valid = internal addrspace(3) global i8 undef, align 1
@g_valid = common addrspace(1) global i8 0, align 1
@bar = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@far = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@star = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@bar_arr = common addrspace(1) global [5 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4
@far_arr = common addrspace(1) global [5 x [4 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 4

; CHECK:      @[[PIPE_BAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK-NEXT: @[[PIPE_FAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK-NEXT: @[[PIPE_STAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK-NEXT: @[[PIPE_BAR_ARR:.*]] = common addrspace(1) global [5 x %opencl.pipe_t{{.*}} addrspace(1)*] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_FAR_ARR:.*]] = common addrspace(1) global [5 x [4 x %opencl.pipe_t{{.*}} addrspace(1)*]] zeroinitializer, align 4

; All calls to read/write_channel_nb_intel should be replaced by
; corresponding calls to pipe built-ins: check is done using --implicit-check-not
;
; First check was added due to -O0 option passed to clang
; CHECK: %[[TEMP_VALID:.*]] = addrspacecast {{.*}} %valid
; CHECK: %[[VALID:.*]] = addrspacecast {{.*}} %valid
;
; CHECK: %[[LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[CAST_BAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_PIPE]]
; CHECK: %[[CALL_BAR_PIPE:.*]] = call i32 @__read_pipe_2{{.*}} %[[CAST_BAR_PIPE]]
; CHECK: %[[BOOL_CALL_BAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_BAR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_BAR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_BAR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_BAR_PIPE]], {{.*}}* %[[VALID]]
;
; CHECK: %[[LOAD_P_TO_VALID:.*]] = load {{.*}} %p_to_valid
; CHECK: %[[LOAD_FAR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR]]
; CHECK: %[[CAST_FAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_FAR_PIPE]]
; CHECK: %[[CALL_FAR_PIPE:.*]] = call i32 @__read_pipe_2{{.*}} %[[CAST_FAR_PIPE]]
; CHECK: %[[BOOL_CALL_FAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_FAR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_FAR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_FAR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_FAR_PIPE]], {{.*}}* %[[LOAD_P_TO_VALID]]
;
; CHECK: %[[LOAD_P_TO_G_VALID:.*]] = load {{.*}} %p_to_g_valid
; CHECK: %[[P_TO_G_VALID:.*]] = addrspacecast {{.*}} %[[LOAD_P_TO_G_VALID]]
; CHECK: %[[LOAD_STAR_PIPE:.*]] = load {{.*}} @[[PIPE_STAR]]
; CHECK: %[[CAST_STAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_STAR_PIPE]]
; CHECK: %[[CALL_STAR_PIPE:.*]] = call i32 @__read_pipe_2{{.*}} %[[CAST_STAR_PIPE]]
; CHECK: %[[BOOL_CALL_STAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_STAR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_STAR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_STAR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_STAR_PIPE]], {{.*}}* %[[P_TO_G_VALID]]
;
; CHECK: %[[LOAD_P_TO_L_VALID:.*]] = load {{.*}} %p_to_l_valid
; CHECK: %[[P_TO_L_VALID:.*]] = addrspacecast {{.*}} %[[LOAD_P_TO_L_VALID]]
; CHECK: %[[LOAD_BAR_ARR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR_ARR]]
; CHECK: %[[CAST_BAR_ARR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_ARR_PIPE]]
; CHECK: %[[CALL_BAR_ARR_PIPE:.*]] = call i32 @__read_pipe_2{{.*}} %[[CAST_BAR_ARR_PIPE]]
; CHECK: %[[BOOL_CALL_BAR_ARR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_BAR_ARR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_BAR_ARR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_BAR_ARR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_BAR_ARR_PIPE]], {{.*}}* %[[P_TO_L_VALID]]
;
; CHECK: %[[LOAD_P_TO_P_VALID:.*]] = load {{.*}} %p_to_p_valid
; CHECK: %[[P_TO_P_VALID:.*]] = addrspacecast {{.*}} %[[LOAD_P_TO_P_VALID]]
; CHECK: %[[LOAD_FAR_ARR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR_ARR]]
; CHECK: %[[CAST_FAR_ARR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_FAR_ARR_PIPE]]
; CHECK: %[[CALL_FAR_ARR_PIPE:.*]] = call i32 @__read_pipe_2{{.*}} %[[CAST_FAR_ARR_PIPE]]
; CHECK: %[[BOOL_CALL_FAR_ARR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_FAR_ARR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_FAR_ARR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_FAR_ARR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_FAR_ARR_PIPE]], {{.*}}* %[[P_TO_P_VALID]]

; Function Attrs: nounwind
define spir_kernel void @foo() #0 !kernel_arg_addr_space !11 !kernel_arg_access_qual !11 !kernel_arg_type !11 !kernel_arg_base_type !11 !kernel_arg_type_qual !11 {
entry:
  %i = alloca i32, align 4
  %f = alloca float, align 4
  %st = alloca %struct.Foo, align 4
  %valid = alloca i8, align 1
  %p_valid = alloca i8, align 1
  %p_to_valid = alloca i8 addrspace(4)*, align 8
  %p_to_g_valid = alloca i8 addrspace(1)*, align 8
  %p_to_l_valid = alloca i8 addrspace(3)*, align 8
  %p_to_p_valid = alloca i8*, align 8
  %tmp = alloca %struct.Foo, align 4
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  store i32 42, i32* %i, align 4, !tbaa !13
  %1 = bitcast float* %f to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #3
  store float 0x3FDAE147A0000000, float* %f, align 4, !tbaa !17
  %2 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #3
  %3 = bitcast %struct.Foo* %st to i8*
  call void @llvm.memset.p0i8.i64(i8* %3, i8 0, i64 4, i32 4, i1 false)
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %valid) #3
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %p_valid) #3
  %4 = bitcast i8 addrspace(4)** %p_to_valid to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %4) #3
  %5 = addrspacecast i8* %valid to i8 addrspace(4)*
  store i8 addrspace(4)* %5, i8 addrspace(4)** %p_to_valid, align 8, !tbaa !19
  %6 = bitcast i8 addrspace(1)** %p_to_g_valid to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %6) #3
  store i8 addrspace(1)* @g_valid, i8 addrspace(1)** %p_to_g_valid, align 8, !tbaa !19
  %7 = bitcast i8 addrspace(3)** %p_to_l_valid to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %7) #3
  store i8 addrspace(3)* @foo.l_valid, i8 addrspace(3)** %p_to_l_valid, align 8, !tbaa !19
  %8 = bitcast i8** %p_to_p_valid to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %8) #3
  store i8* %p_valid, i8** %p_to_p_valid, align 8, !tbaa !19
  %9 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !21
  %10 = addrspacecast i8* %valid to i8 addrspace(4)*
  %call = call i32 @_Z21read_channel_nb_intel11ocl_channeliPU3AS4b(%opencl.channel_t addrspace(1)* %9, i8 addrspace(4)* %10)
  store i32 %call, i32* %i, align 4, !tbaa !13
  %11 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @far, align 4, !tbaa !21
  %12 = load i8 addrspace(4)*, i8 addrspace(4)** %p_to_valid, align 8, !tbaa !19
  %call1 = call float @_Z21read_channel_nb_intel11ocl_channelfPU3AS4b(%opencl.channel_t addrspace(1)* %11, i8 addrspace(4)* %12)
  store float %call1, float* %f, align 4, !tbaa !17
  %13 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @star, align 4, !tbaa !21
  %14 = load i8 addrspace(1)*, i8 addrspace(1)** %p_to_g_valid, align 8, !tbaa !19
  %15 = addrspacecast i8 addrspace(1)* %14 to i8 addrspace(4)*
  call void @_Z21read_channel_nb_intel11ocl_channel3FooPU3AS4b(%struct.Foo* sret %tmp, %opencl.channel_t addrspace(1)* %13, i8 addrspace(4)* %15)
  %16 = bitcast %struct.Foo* %st to i8*
  %17 = bitcast %struct.Foo* %tmp to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %16, i8* %17, i64 4, i32 4, i1 false), !tbaa.struct !22
  %18 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x %opencl.channel_t addrspace(1)*], [5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, i64 0, i64 3), align 4, !tbaa !21
  %19 = load i8 addrspace(3)*, i8 addrspace(3)** %p_to_l_valid, align 8, !tbaa !19
  %20 = addrspacecast i8 addrspace(3)* %19 to i8 addrspace(4)*
  %call2 = call i32 @_Z21read_channel_nb_intel11ocl_channeliPU3AS4b(%opencl.channel_t addrspace(1)* %18, i8 addrspace(4)* %20)
  store i32 %call2, i32* %i, align 4, !tbaa !13
  %21 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x %opencl.channel_t addrspace(1)*]], [5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @far_arr, i64 0, i64 3, i64 2), align 4, !tbaa !21
  %22 = load i8*, i8** %p_to_p_valid, align 8, !tbaa !19
  %23 = addrspacecast i8* %22 to i8 addrspace(4)*
  %call3 = call float @_Z21read_channel_nb_intel11ocl_channelfPU3AS4b(%opencl.channel_t addrspace(1)* %21, i8 addrspace(4)* %23)
  store float %call3, float* %f, align 4, !tbaa !17
  %24 = bitcast i8** %p_to_p_valid to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %24) #3
  %25 = bitcast i8 addrspace(3)** %p_to_l_valid to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %25) #3
  %26 = bitcast i8 addrspace(1)** %p_to_g_valid to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %26) #3
  %27 = bitcast i8 addrspace(4)** %p_to_valid to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %27) #3
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %p_valid) #3
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %valid) #3
  %28 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %28) #3
  %29 = bitcast float* %f to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %29) #3
  %30 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %30) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1) #1

declare i32 @_Z21read_channel_nb_intel11ocl_channeliPU3AS4b(%opencl.channel_t addrspace(1)*, i8 addrspace(4)*) #2

declare float @_Z21read_channel_nb_intel11ocl_channelfPU3AS4b(%opencl.channel_t addrspace(1)*, i8 addrspace(4)*) #2

declare void @_Z21read_channel_nb_intel11ocl_channel3FooPU3AS4b(%struct.Foo* sret, %opencl.channel_t addrspace(1)*, i8 addrspace(4)*) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!opencl.channels = !{!0, !4, !6, !7, !8}
!llvm.module.flags = !{!9}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!10}
!opencl.spir.version = !{!10}
!opencl.used.extensions = !{!11}
!opencl.used.optional.core.features = !{!11}
!opencl.compiler.options = !{!11}
!llvm.ident = !{!12}

!0 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @bar, !1, !2, !3}
!1 = !{!"packet_size", i32 4}
!2 = !{!"packet_align", i32 4}
!3 = !{!"depth", i32 0}
!4 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @far, !1, !2, !5}
!5 = !{!"depth", i32 3}
!6 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @star, !1, !2}
!7 = !{[5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, !1, !2}
!8 = !{[5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @far_arr, !1, !2}
!9 = !{i32 1, !"wchar_size", i32 4}
!10 = !{i32 2, i32 0}
!11 = !{}
!12 = !{!"clang version 5.0.0 "}
!13 = !{!14, !14, i64 0}
!14 = !{!"int", !15, i64 0}
!15 = !{!"omnipotent char", !16, i64 0}
!16 = !{!"Simple C/C++ TBAA"}
!17 = !{!18, !18, i64 0}
!18 = !{!"float", !15, i64 0}
!19 = !{!20, !20, i64 0}
!20 = !{!"any pointer", !15, i64 0}
!21 = !{!15, !15, i64 0}
!22 = !{i64 0, i64 4, !13}
