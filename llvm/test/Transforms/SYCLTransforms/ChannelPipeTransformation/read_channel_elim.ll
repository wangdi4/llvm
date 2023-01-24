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
; channel int bar_arr[5] __attribute__((depth(0)));
; channel char far_arr[5][4] __attribute__((depth(3)));
; channel struct Foo star_arr[5][4][3];
;
; __kernel void foo() {
;   int i = read_channel_intel(bar);
;   char f = read_channel_intel(far);
;   struct Foo st = read_channel_intel(star);
;
;   int ii = read_channel_intel(bar_arr[3]);
;   char ff = read_channel_intel(far_arr[3][2]);
;   struct Foo stst = read_channel_intel(star_arr[3][2][1]);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl
; ----------------------------------------------------
; RUN: llvm-as %p/../Inputs/fpga-pipes.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck --implicit-check-not read_channel_intel %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
%struct.Foo = type { i32, i64 }

@bar = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0, !depth !1
@far = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 1, !packet_size !2, !packet_align !2, !depth !3
@star = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 8, !packet_size !4, !packet_align !5
@bar_arr = common addrspace(1) global [5 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4, !packet_size !0, !packet_align !0, !depth !1
@far_arr = common addrspace(1) global [5 x [4 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 1, !packet_size !2, !packet_align !2, !depth !3
@star_arr = common addrspace(1) global [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] zeroinitializer, align 8, !packet_size !4, !packet_align !5

; CHECK: @[[PIPE_BAR:.*]] = addrspace(1) global %opencl.pipe_rw_t{{.*}} addrspace(1)*
; CHECK: @[[PIPE_FAR:.*]] = addrspace(1) global %opencl.pipe_rw_t{{.*}} addrspace(1)*
; CHECK: @[[PIPE_STAR:.*]] = addrspace(1) global %opencl.pipe_rw_t{{.*}} addrspace(1)*
; CHECK: @[[PIPE_BAR_ARR:.*]] = addrspace(1) global [5 x %opencl.pipe_rw_t{{.*}} addrspace(1)*]
; CHECK: @[[PIPE_FAR_ARR:.*]] = addrspace(1) global [5 x [4 x %opencl.pipe_rw_t{{.*}} addrspace(1)*]]
; CHECK: @[[PIPE_STAR_ARR:.*]] = addrspace(1) global [5 x [4 x [3 x %opencl.pipe_rw_t{{.*}} addrspace(1)*]]]

; All calls to read/write_channel_intel should be replaced by
; corresponding calls to pipe built-ins: check is done by --implicit-check-not
;
; CHECK: %[[LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[CAST_BAR_PIPE:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_BAR_PIPE]] to %opencl.pipe_ro_t
; CHECK: call i32 @__read_pipe_2_bl_fpga({{.*}} %[[CAST_BAR_PIPE]], {{.*}}, i32 4, i32 4)
;
; CHECK: %[[LOAD_FAR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR]]
; CHECK: %[[CAST_FAR_PIPE:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_FAR_PIPE]] to %opencl.pipe_ro_t
; CHECK: call i32 @__read_pipe_2_bl_fpga({{.*}} %[[CAST_FAR_PIPE]], {{.*}}, i32 1, i32 1)
;
; CHECK: %[[LOAD_STAR_PIPE:.*]] = load {{.*}} @[[PIPE_STAR]]
; CHECK: %[[CAST_STAR_PIPE:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_STAR_PIPE]] to %opencl.pipe_ro_t
; CHECK: call i32 @__read_pipe_2_bl_fpga({{.*}} %[[CAST_STAR_PIPE]], {{.*}}, i32 16, i32 8)
;
; CHECK: %[[LOAD_BAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_BAR_ARR]]
; CHECK: %[[CAST_BAR_PIPE_ARR:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_BAR_PIPE_ARR]] to %opencl.pipe_ro_t
; CHECK: call i32 @__read_pipe_2_bl_fpga({{.*}} %[[CAST_BAR_PIPE_ARR]], {{.*}}, i32 4, i32 4)
;
; CHECK: %[[LOAD_FAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_FAR_ARR]]
; CHECK: %[[CAST_FAR_PIPE_ARR:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_FAR_PIPE_ARR]] to %opencl.pipe_ro_t
; CHECK: call i32 @__read_pipe_2_bl_fpga({{.*}} %[[CAST_FAR_PIPE_ARR]], {{.*}}, i32 1, i32 1)
;
; CHECK: %[[LOAD_STAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_STAR_ARR]]
; CHECK: %[[CAST_STAR_PIPE_ARR:.*]] = bitcast %opencl.pipe_rw_t{{.*}} %[[LOAD_STAR_PIPE_ARR]] to %opencl.pipe_ro_t
; CHECK: call i32 @__read_pipe_2_bl_fpga({{.*}} %[[CAST_STAR_PIPE_ARR]], {{.*}}, i32 16, i32 8)

; Function Attrs: convergent nounwind
define spir_kernel void @foo() #0 !kernel_arg_addr_space !9 !kernel_arg_access_qual !9 !kernel_arg_type !9 !kernel_arg_base_type !9 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !9 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 {
entry:
  %i = alloca i32, align 4
  %f = alloca i8, align 1
  %st = alloca %struct.Foo, align 8
  %ii = alloca i32, align 4
  %ff = alloca i8, align 1
  %stst = alloca %struct.Foo, align 8
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !11
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)* %1) #4
  store i32 %call, i32* %i, align 4, !tbaa !14
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %f) #3
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @far, align 1, !tbaa !11
  %call1 = call signext i8 @_Z18read_channel_intel11ocl_channelc(%opencl.channel_t addrspace(1)* %2) #4
  store i8 %call1, i8* %f, align 1, !tbaa !11
  %3 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* %3) #3
  %4 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @star, align 8, !tbaa !11
  call void @_Z18read_channel_intel11ocl_channel3Foo(%struct.Foo* sret(%struct.Foo) %st, %opencl.channel_t addrspace(1)* %4) #4
  %5 = bitcast i32* %ii to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #3
  %6 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x %opencl.channel_t addrspace(1)*], [5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, i64 0, i64 3), align 4, !tbaa !11
  %call2 = call i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)* %6) #4
  store i32 %call2, i32* %ii, align 4, !tbaa !14
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %ff) #3
  %7 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x %opencl.channel_t addrspace(1)*]], [5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @far_arr, i64 0, i64 3, i64 2), align 1, !tbaa !11
  %call3 = call signext i8 @_Z18read_channel_intel11ocl_channelc(%opencl.channel_t addrspace(1)* %7) #4
  store i8 %call3, i8* %ff, align 1, !tbaa !11
  %8 = bitcast %struct.Foo* %stst to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* %8) #3
  %9 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]], [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* @star_arr, i64 0, i64 3, i64 2, i64 1), align 8, !tbaa !11
  call void @_Z18read_channel_intel11ocl_channel3Foo(%struct.Foo* sret(%struct.Foo) %stst, %opencl.channel_t addrspace(1)* %9) #4
  %10 = bitcast %struct.Foo* %stst to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %10) #3
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %ff) #3
  %11 = bitcast i32* %ii to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %11) #3
  %12 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %12) #3
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %f) #3
  %13 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %13) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: convergent
declare i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)*) #2

; Function Attrs: convergent
declare signext i8 @_Z18read_channel_intel11ocl_channelc(%opencl.channel_t addrspace(1)*) #2

; Function Attrs: convergent
declare void @_Z18read_channel_intel11ocl_channel3Foo(%struct.Foo* sret(%struct.Foo), %opencl.channel_t addrspace(1)*) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { convergent }

!llvm.module.flags = !{!6}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!7}
!opencl.spir.version = !{!8}
!opencl.used.extensions = !{!9}
!opencl.used.optional.core.features = !{!9}
!opencl.compiler.options = !{!9}
!llvm.ident = !{!10}

!0 = !{i32 4}
!1 = !{i32 0}
!2 = !{i32 1}
!3 = !{i32 3}
!4 = !{i32 16}
!5 = !{i32 8}
!6 = !{i32 1, !"wchar_size", i32 4}
!7 = !{i32 1, i32 0}
!8 = !{i32 1, i32 2}
!9 = !{}
!10 = !{!"clang version 7.0.0 "}
!11 = !{!12, !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
!14 = !{!15, !15, i64 0}
!15 = !{!"int", !12, i64 0}

; DEBUGIFY-NOT: WARNING: Missing line
