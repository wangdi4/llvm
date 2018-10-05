; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int ch1;
; channel int ch_arr[4];
;
; __kernel void foo(__global int* iters) {
;   for (int i = 0; i < *iters; ++i) {
;     write_channel_intel(ch1, 42);
;     // implicit work-group barrier here
;   }
; }
;
; __kernel void boo() {
;   #pragma unroll
;   for (int i = 0; i < 4; ++i) {
;     write_channel_intel(ch_arr[i], 42);
;   }
; }
; ----------------------------------------------------
; Compile options:
;   clang -cc1 -x cl -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -finclude-default-header -cl-std=CL1.2 -emit-llvm
; Optimizer options:
;   opt -runtimelib=%p/../../vectorizer/Full/runtime.bc -demangle-fpga-pipes -spir-materializer -channel-pipe-transformation -verify %s -S
; ----------------------------------------------------
; RUN: %oclopt -pipe-ordering -verify %s -S | FileCheck %s
; REQUIRES: fpga-emulator

; CHECK:   define void @foo
; CHECK:     call void @_Z7barrierj(i32 1)
; CHECK:   define void @boo
; CHECK-NOT: call void @_Z7barrierj(i32 1)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
%opencl.pipe_rw_t = type opaque
%opencl.pipe_wo_t = type opaque
%struct.__pipe_t = type { i32, i32, i32, i32, i32, i32, [0 x i8] }

@ch1 = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@ch_arr = common addrspace(1) global [4 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4, !packet_size !0, !packet_align !0
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @__pipe_global_ctor, i8* null }]
@ch1.pipe = addrspace(1) global %opencl.pipe_rw_t addrspace(1)* null, align 8, !packet_size !0, !packet_align !0
@ch1.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4
@ch_arr.pipe = addrspace(1) global [4 x %opencl.pipe_rw_t addrspace(1)*] zeroinitializer, align 16, !packet_size !0, !packet_align !0
@ch_arr.pipe.bs = addrspace(1) global [1312 x i8] zeroinitializer, align 4

; Function Attrs: convergent nounwind
define void @foo(i32 addrspace(1)* %iters) #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 {
entry:
  %write.src = alloca i32
  %iters.addr = alloca i32 addrspace(1)*, align 8
  %i = alloca i32, align 4
  store i32 addrspace(1)* %iters, i32 addrspace(1)** %iters.addr, align 8, !tbaa !12
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  store i32 0, i32* %i, align 4, !tbaa !16
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, i32* %i, align 4, !tbaa !16
  %2 = load i32 addrspace(1)*, i32 addrspace(1)** %iters.addr, align 8, !tbaa !12
  %3 = load i32, i32 addrspace(1)* %2, align 4, !tbaa !16
  %cmp = icmp slt i32 %1, %3
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %4 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %4) #3
  br label %for.end

for.body:                                         ; preds = %for.cond
  %5 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch1.pipe
  %6 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch1, align 4, !tbaa !18
  store i32 42, i32* %write.src
  %7 = bitcast %opencl.pipe_rw_t addrspace(1)* %5 to %opencl.pipe_wo_t addrspace(1)*
  %8 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %9 = call i32 @__write_pipe_2_bl(%opencl.pipe_wo_t addrspace(1)* %7, i8 addrspace(4)* %8, i32 4, i32 4)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %10 = load i32, i32* %i, align 4, !tbaa !16
  %inc = add nsw i32 %10, 1
  store i32 %inc, i32* %i, align 4, !tbaa !16
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: convergent nounwind
define void @boo() #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !3 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !3 !kernel_arg_host_accessible !3 !kernel_arg_pipe_depth !3 !kernel_arg_pipe_io !3 !kernel_arg_buffer_location !3 {
entry:
  %write.src = alloca i32
  %i = alloca i32, align 4
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  store i32 0, i32* %i, align 4, !tbaa !16
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, i32* %i, align 4, !tbaa !16
  %cmp = icmp slt i32 %1, 4
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %2 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %2) #3
  br label %for.end

for.body:                                         ; preds = %for.cond
  %3 = load i32, i32* %i, align 4, !tbaa !16
  %idxprom = sext i32 %3 to i64
  %arrayidx1 = getelementptr [4 x %opencl.pipe_rw_t addrspace(1)*], [4 x %opencl.pipe_rw_t addrspace(1)*] addrspace(1)* @ch_arr.pipe, i64 0, i64 %idxprom
  %arrayidx = getelementptr inbounds [4 x %opencl.channel_t addrspace(1)*], [4 x %opencl.channel_t addrspace(1)*] addrspace(1)* @ch_arr, i64 0, i64 %idxprom
  %4 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* %arrayidx1
  %5 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* %arrayidx, align 4, !tbaa !18
  store i32 42, i32* %write.src
  %6 = bitcast %opencl.pipe_rw_t addrspace(1)* %4 to %opencl.pipe_wo_t addrspace(1)*
  %7 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %8 = call i32 @__write_pipe_2_bl(%opencl.pipe_wo_t addrspace(1)* %6, i8 addrspace(4)* %7, i32 4, i32 4)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %9 = load i32, i32* %i, align 4, !tbaa !16
  %inc = add nsw i32 %9, 1
  store i32 %inc, i32* %i, align 4, !tbaa !16
  br label %for.cond, !llvm.loop !19

for.end:                                          ; preds = %for.cond.cleanup
  ret void
}

define void @__pipe_global_ctor() {
entry:
  call void @__pipe_init_intel(%struct.__pipe_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @ch1.pipe.bs to %struct.__pipe_t addrspace(1)*), i32 4, i32 0, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @ch1.pipe.bs to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch1.pipe
  store %opencl.pipe_rw_t addrspace(1)* bitcast ([1312 x i8] addrspace(1)* @ch_arr.pipe.bs to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x %opencl.pipe_rw_t addrspace(1)*], [4 x %opencl.pipe_rw_t addrspace(1)*] addrspace(1)* @ch_arr.pipe, i32 0, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([1312 x i8], [1312 x i8] addrspace(1)* @ch_arr.pipe.bs, i64 0, i64 328) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x %opencl.pipe_rw_t addrspace(1)*], [4 x %opencl.pipe_rw_t addrspace(1)*] addrspace(1)* @ch_arr.pipe, i32 0, i32 1)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([1312 x i8], [1312 x i8] addrspace(1)* @ch_arr.pipe.bs, i64 0, i64 656) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x %opencl.pipe_rw_t addrspace(1)*], [4 x %opencl.pipe_rw_t addrspace(1)*] addrspace(1)* @ch_arr.pipe, i32 0, i32 2)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([1312 x i8], [1312 x i8] addrspace(1)* @ch_arr.pipe.bs, i64 0, i64 984) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x %opencl.pipe_rw_t addrspace(1)*], [4 x %opencl.pipe_rw_t addrspace(1)*] addrspace(1)* @ch_arr.pipe, i32 0, i32 3)
  call void @__pipe_init_array_intel(%struct.__pipe_t addrspace(1)* addrspace(1)* bitcast ([4 x %opencl.pipe_rw_t addrspace(1)*] addrspace(1)* @ch_arr.pipe to %struct.__pipe_t addrspace(1)* addrspace(1)*), i32 4, i32 4, i32 0, i32 0)
  ret void
}

; Function Attrs: nounwind readnone
declare void @__pipe_init_intel(%struct.__pipe_t addrspace(1)*, i32, i32, i32) #2

; Function Attrs: nounwind readnone
declare void @__pipe_init_array_intel(%struct.__pipe_t addrspace(1)* addrspace(1)*, i32, i32, i32, i32) #2

; Function Attrs: nounwind readnone
declare i32 @__write_pipe_2(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(4)* nocapture readonly, i32, i32) #2

declare i32 @__write_pipe_2_bl(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(4)*, i32, i32)

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind readnone }
attributes #3 = { nounwind }

!llvm.module.flags = !{!1}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!2}
!opencl.spir.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!opencl.kernels = !{!5}

!0 = !{i32 4}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, i32 2}
!3 = !{}
!4 = !{!"clang version 7.0.0 "}
!5 = !{void (i32 addrspace(1)*)* @foo, void ()* @boo}
!6 = !{i32 1}
!7 = !{!"none"}
!8 = !{!"int*"}
!9 = !{!""}
!10 = !{i1 false}
!11 = !{i32 0}
!12 = !{!13, !13, i64 0}
!13 = !{!"any pointer", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C/C++ TBAA"}
!16 = !{!17, !17, i64 0}
!17 = !{!"int", !14, i64 0}
!18 = !{!14, !14, i64 0}
!19 = distinct !{!19, !20}
!20 = !{!"llvm.loop.unroll.enable"}
