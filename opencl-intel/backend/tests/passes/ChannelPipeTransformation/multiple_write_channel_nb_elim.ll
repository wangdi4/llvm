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
; channel struct Foo star_arr[3][2];
;
; __kernel void foo() {
;   int i = 42;
;   struct Foo st = {0};
;   bool valid;
;
;   valid = write_channel_nb_intel(bar, i);
;   valid = write_channel_nb_intel(star_arr[2][1], st);
;
;   valid = write_channel_nb_intel(bar, i);
;   valid = write_channel_nb_intel(star_arr[2][1], st);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL1.2
; ----------------------------------------------------
; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck --implicit-check-not write_channel_nb_intel %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
%struct.Foo = type { i32 }

@bar = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@star_arr = common addrspace(1) global [3 x [2 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 4, !packet_size !0, !packet_align !0

; CHECK: @[[PIPE_BAR:.*]] = addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK: @[[PIPE_STAR_ARR:.*]] = addrspace(1) global [3 x [2 x %opencl.pipe_t addrspace(1)*]]

; All calls to read/write_channel_nb_intel should be replaced by
; corresponding calls to pipe built-ins: check is done using --implicit-check-not
;
; CHECK: %[[LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[CAST_BAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_PIPE]]
; CHECK: %[[CALL_BAR_PIPE:.*]] = call i32 @__write_pipe_2{{.*}} %[[CAST_BAR_PIPE]]
; CHECK: %[[BOOL_CALL_BAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_BAR_PIPE]], 0
;
; CHECK: %[[LOAD_STAR_ARR_PIPE:.*]] = load {{.*}} @[[PIPE_STAR_ARR]]
; CHECK: %[[CAST_STAR_ARR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_STAR_ARR_PIPE]]
; CHECK: %[[CALL_STAR_ARR_PIPE:.*]] = call i32 @__write_pipe_2{{.*}} %[[CAST_STAR_ARR_PIPE]]
; CHECK: %[[BOOL_CALL_STAR_ARR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_STAR_ARR_PIPE]], 0
;
; CHECK: %[[LOAD_BAR_PIPE1:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[CAST_BAR_PIPE1:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_PIPE1]]
; CHECK: %[[CALL_BAR_PIPE1:.*]] = call i32 @__write_pipe_2{{.*}} %[[CAST_BAR_PIPE1]]
; CHECK: %[[BOOL_CALL_BAR_PIPE1:.*]] = icmp eq {{.*}} %[[CALL_BAR_PIPE1]], 0
;
; CHECK: %[[LOAD_STAR_ARR_PIPE1:.*]] = load {{.*}} @[[PIPE_STAR_ARR]]
; CHECK: %[[CAST_STAR_ARR_PIPE1:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_STAR_ARR_PIPE1]]
; CHECK: %[[CALL_STAR_ARR_PIPE1:.*]] = call i32 @__write_pipe_2{{.*}} %[[CAST_STAR_ARR_PIPE1]]
; CHECK: %[[BOOL_CALL_STAR_ARR_PIPE1:.*]] = icmp eq {{.*}} %[[CALL_STAR_ARR_PIPE1]], 0

; Function Attrs: convergent nounwind
define spir_kernel void @foo() #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !3 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !3 !kernel_arg_host_accessible !3 !kernel_arg_pipe_depth !3 !kernel_arg_pipe_io !3 !kernel_arg_buffer_location !3 {
  %i = alloca i32, align 4
  %st = alloca %struct.Foo, align 4
  %valid = alloca i8, align 1
  %1 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #3
  store i32 42, i32* %i, align 4, !tbaa !5
  %2 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #3
  %3 = bitcast %struct.Foo* %st to i8*
  call void @llvm.memset.p0i8.i64(i8* %3, i8 0, i64 4, i32 4, i1 false)
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %valid) #3
  %4 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !9
  %5 = load i32, i32* %i, align 4, !tbaa !5
  %call = call zeroext i1 @_Z22write_channel_nb_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %4, i32 %5) #4
  %frombool = zext i1 %call to i8
  store i8 %frombool, i8* %valid, align 1, !tbaa !10
  %6 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([3 x [2 x %opencl.channel_t addrspace(1)*]], [3 x [2 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @star_arr, i64 0, i64 2, i64 1), align 4, !tbaa !9
  %call1 = call zeroext i1 @_Z22write_channel_nb_intel11ocl_channel3FooS_(%opencl.channel_t addrspace(1)* %6, %struct.Foo* byval align 4 %st) #4
  %frombool2 = zext i1 %call1 to i8
  store i8 %frombool2, i8* %valid, align 1, !tbaa !10
  %7 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !9
  %8 = load i32, i32* %i, align 4, !tbaa !5
  %call3 = call zeroext i1 @_Z22write_channel_nb_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %7, i32 %8) #4
  %frombool4 = zext i1 %call3 to i8
  store i8 %frombool4, i8* %valid, align 1, !tbaa !10
  %9 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([3 x [2 x %opencl.channel_t addrspace(1)*]], [3 x [2 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @star_arr, i64 0, i64 2, i64 1), align 4, !tbaa !9
  %call5 = call zeroext i1 @_Z22write_channel_nb_intel11ocl_channel3FooS_(%opencl.channel_t addrspace(1)* %9, %struct.Foo* byval align 4 %st) #4
  %frombool6 = zext i1 %call5 to i8
  store i8 %frombool6, i8* %valid, align 1, !tbaa !10
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %valid) #3
  %10 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %10) #3
  %11 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %11) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1) #1

; Function Attrs: convergent
declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #2

; Function Attrs: convergent
declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channel3FooS_(%opencl.channel_t addrspace(1)*, %struct.Foo* byval align 4) #2

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
!9 = !{!7, !7, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"bool", !7, i64 0}

