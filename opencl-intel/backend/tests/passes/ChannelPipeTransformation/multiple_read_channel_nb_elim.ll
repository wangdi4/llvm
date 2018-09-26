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
; channel struct Foo star_arr[3][2];
;
; __kernel void foo() {
;   bool valid;
;
;   int i = read_channel_nb_intel(bar, &valid);
;   struct Foo stst = read_channel_nb_intel(star_arr[2][1], &valid);
;
;   int j = read_channel_nb_intel(bar, &valid);
;   struct Foo stst1 = read_channel_nb_intel(star_arr[2][1], &valid);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL1.2
; ----------------------------------------------------
; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck --implicit-check-not read_channel_nb_intel %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
%struct.Foo = type { i32, float }

@bar = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@star_arr = common addrspace(1) global [3 x [2 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 4, !packet_size !1, !packet_align !0

; CHECK: @[[PIPE_BAR:.*]] = addrspace(1) global %opencl.pipe_rw_t{{.*}} addrspace(1)*
; CHECK: @[[PIPE_STAR_ARR:.*]] = addrspace(1) global [3 x [2 x %opencl.pipe_rw_t{{.*}} addrspace(1)*]]
;
; All calls to read/write_channel_nb_intel should be replaced by
; corresponding calls to pipe built-ins: check is done using --implicit-check-not
;
; CHECK: %[[VALID:.*]] = alloca i8, align 1
;
; CHECK: %[[LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[CAST_BAR_PIPE:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_BAR_PIPE]] to %opencl.pipe_ro_t
; CHECK: %[[CALL_BAR_PIPE:.*]] = call i32 @__read_pipe_2_intel({{.*}} %[[CAST_BAR_PIPE]], {{.*}}, i32 4, i32 4)
; CHECK: %[[BOOL_CALL_BAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_BAR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_BAR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_BAR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_BAR_PIPE]], {{.*}}* %[[VALID]]
;
; CHECK: %[[LOAD_STAR_ARR_PIPE:.*]] = load {{.*}} @[[PIPE_STAR_ARR]]
; CHECK: %[[CAST_STAR_ARR_PIPE:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_STAR_ARR_PIPE]] to %opencl.pipe_ro_t
; CHECK: %[[CALL_STAR_ARR_PIPE:.*]] = call i32 @__read_pipe_2_intel({{.*}} %[[CAST_STAR_ARR_PIPE]], {{.*}}, i32 8, i32 4)
; CHECK: %[[BOOL_CALL_STAR_ARR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_STAR_ARR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_STAR_ARR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_STAR_ARR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_STAR_ARR_PIPE]], {{.*}}* %[[VALID]]
;
; CHECK: %[[LOAD_BAR_PIPE1:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[CAST_BAR_PIPE1:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_BAR_PIPE1]] to %opencl.pipe_ro_t
; CHECK: %[[CALL_BAR_PIPE1:.*]] = call i32 @__read_pipe_2_intel({{.*}} %[[CAST_BAR_PIPE1]], {{.*}}, i32 4, i32 4)
; CHECK: %[[BOOL_CALL_BAR_PIPE1:.*]] = icmp eq {{.*}} %[[CALL_BAR_PIPE1]], 0
; CHECK: %[[ZEXT_BOOL_CALL_BAR_PIPE1:.*]] = zext {{.*}} %[[BOOL_CALL_BAR_PIPE1]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_BAR_PIPE1]], {{.*}}* %[[VALID]]
;
; CHECK: %[[LOAD_STAR_ARR_PIPE1:.*]] = load {{.*}} @[[PIPE_STAR_ARR]]
; CHECK: %[[CAST_STAR_ARR_PIPE1:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_STAR_ARR_PIPE1]] to %opencl.pipe_ro_t
; CHECK: %[[CALL_STAR_ARR_PIPE1:.*]] = call i32 @__read_pipe_2_intel({{.*}} %[[CAST_STAR_ARR_PIPE1]], {{.*}}, i32 8, i32 4)
; CHECK: %[[BOOL_CALL_STAR_ARR_PIPE1:.*]] = icmp eq {{.*}} %[[CALL_STAR_ARR_PIPE1]], 0
; CHECK: %[[ZEXT_BOOL_CALL_STAR_ARR_PIPE1:.*]] = zext {{.*}} %[[BOOL_CALL_STAR_ARR_PIPE1]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_STAR_ARR_PIPE1]], {{.*}}* %[[VALID]]

; Function Attrs: convergent nounwind
define spir_kernel void @foo() #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !4 !kernel_arg_pipe_depth !4 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 {
entry:
  %valid = alloca i8, align 1
  %i = alloca i32, align 4
  %stst = alloca %struct.Foo, align 4
  %j = alloca i32, align 4
  %stst1 = alloca %struct.Foo, align 4
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %valid) #3
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !6
  %call = call i32 @_Z21read_channel_nb_intel11ocl_channeliPb(%opencl.channel_t addrspace(1)* %1, i8* %valid) #4
  store i32 %call, i32* %i, align 4, !tbaa !9
  %2 = bitcast %struct.Foo* %stst to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %2) #3
  %3 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([3 x [2 x %opencl.channel_t addrspace(1)*]], [3 x [2 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @star_arr, i64 0, i64 2, i64 1), align 4, !tbaa !6
  call void @_Z21read_channel_nb_intel11ocl_channel3FooPb(%struct.Foo* sret %stst, %opencl.channel_t addrspace(1)* %3, i8* %valid) #4
  %4 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #3
  %5 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !6
  %call1 = call i32 @_Z21read_channel_nb_intel11ocl_channeliPb(%opencl.channel_t addrspace(1)* %5, i8* %valid) #4
  store i32 %call1, i32* %j, align 4, !tbaa !9
  %6 = bitcast %struct.Foo* %stst1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %6) #3
  %7 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([3 x [2 x %opencl.channel_t addrspace(1)*]], [3 x [2 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @star_arr, i64 0, i64 2, i64 1), align 4, !tbaa !6
  call void @_Z21read_channel_nb_intel11ocl_channel3FooPb(%struct.Foo* sret %stst1, %opencl.channel_t addrspace(1)* %7, i8* %valid) #4
  %8 = bitcast %struct.Foo* %stst1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %8) #3
  %9 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %9) #3
  %10 = bitcast %struct.Foo* %stst to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %10) #3
  %11 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %11) #3
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %valid) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: convergent
declare i32 @_Z21read_channel_nb_intel11ocl_channeliPb(%opencl.channel_t addrspace(1)*, i8*) #2

; Function Attrs: convergent
declare void @_Z21read_channel_nb_intel11ocl_channel3FooPb(%struct.Foo* sret, %opencl.channel_t addrspace(1)*, i8*) #2

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
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !7, i64 0}
