; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
;  channel int ch;
;
;  __kernel void foo(__global int* iters) {
;    for (int i = 0; i < *iters; ++i) {
;      for (int j = 0; j < i*i+1; ++j) {
;        write_channel_intel(ch, 42);
;        // implicit work-group barrier here
;      }
;      for (int j = 0; j < i*i*i+1; ++j) {
;        write_channel_intel(ch, 42);
;        // implicit work-group barrier here
;      }
;    }
;  }
; ----------------------------------------------------
; Compile options:
;   clang -cc1 -x cl -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -finclude-default-header -cl-std=CL1.2 -emit-llvm
; Optimizer options:
;   opt -runtimelib=%p/../../vectorizer/Full/runtime.bc -demangle-fpga-pipes -spir-materializer -channel-pipe-transformation -verify %s -S
; ----------------------------------------------------
; RUN: %oclopt -pipe-ordering -verify %s -S | FileCheck %s
; REQUIRES: fpga-emulator

; CHECK-LABEL: for.cond1:
; CHECK: call void @_Z7barrierj(i32 1)
; CHECK: br i1 %cmp2, label %for.body4, label %for.cond.cleanup3
; CHECK-LABEL: for.cond6:
; CHECK: call void @_Z7barrierj(i32 1)
; CHECK: br i1 %cmp10, label %for.body12, label %for.cond.cleanup11

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
%opencl.pipe_rw_t = type opaque
%opencl.pipe_wo_t = type opaque
%struct.__pipe_t = type { i32, i32, i32, i32, i32, i32, [0 x i8] }

@ch = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @__pipe_global_ctor, i8* null }]
@ch.pipe = addrspace(1) global %opencl.pipe_rw_t addrspace(1)* null, align 8, !packet_size !0, !packet_align !0
@ch.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4

; Function Attrs: convergent nounwind
define void @foo(i32 addrspace(1)* %iters) #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 {
entry:
  %write.src = alloca i32
  %iters.addr = alloca i32 addrspace(1)*, align 8
  %i = alloca i32, align 4
  %cleanup.dest.slot = alloca i32, align 4
  %j = alloca i32, align 4
  %j5 = alloca i32, align 4
  store i32 addrspace(1)* %iters, i32 addrspace(1)** %iters.addr, align 8, !tbaa !12
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  store i32 0, i32* %i, align 4, !tbaa !16
  br label %for.cond

for.cond:                                         ; preds = %for.inc16, %entry
  %1 = load i32, i32* %i, align 4, !tbaa !16
  %2 = load i32 addrspace(1)*, i32 addrspace(1)** %iters.addr, align 8, !tbaa !12
  %3 = load i32, i32 addrspace(1)* %2, align 4, !tbaa !16
  %cmp = icmp slt i32 %1, %3
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  store i32 2, i32* %cleanup.dest.slot, align 4
  %4 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %4) #3
  br label %for.end18

for.body:                                         ; preds = %for.cond
  %5 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #3
  store i32 0, i32* %j, align 4, !tbaa !16
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc, %for.body
  %6 = load i32, i32* %j, align 4, !tbaa !16
  %7 = load i32, i32* %i, align 4, !tbaa !16
  %8 = load i32, i32* %i, align 4, !tbaa !16
  %mul = mul nsw i32 %7, %8
  %add = add nsw i32 %mul, 1
  %cmp2 = icmp slt i32 %6, %add
  br i1 %cmp2, label %for.body4, label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond1
  store i32 5, i32* %cleanup.dest.slot, align 4
  %9 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %9) #3
  br label %for.end

for.body4:                                        ; preds = %for.cond1
  %10 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch.pipe
  %11 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch, align 4, !tbaa !18
  store i32 42, i32* %write.src
  %12 = bitcast %opencl.pipe_rw_t addrspace(1)* %10 to %opencl.pipe_wo_t addrspace(1)*
  %13 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %14 = call i32 @__write_pipe_2_bl(%opencl.pipe_wo_t addrspace(1)* %12, i8 addrspace(4)* %13, i32 4, i32 4)
  br label %for.inc

for.inc:                                          ; preds = %for.body4
  %15 = load i32, i32* %j, align 4, !tbaa !16
  %inc = add nsw i32 %15, 1
  store i32 %inc, i32* %j, align 4, !tbaa !16
  br label %for.cond1

for.end:                                          ; preds = %for.cond.cleanup3
  %16 = bitcast i32* %j5 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %16) #3
  store i32 0, i32* %j5, align 4, !tbaa !16
  br label %for.cond6

for.cond6:                                        ; preds = %for.inc13, %for.end
  %17 = load i32, i32* %j5, align 4, !tbaa !16
  %18 = load i32, i32* %i, align 4, !tbaa !16
  %19 = load i32, i32* %i, align 4, !tbaa !16
  %mul7 = mul nsw i32 %18, %19
  %20 = load i32, i32* %i, align 4, !tbaa !16
  %mul8 = mul nsw i32 %mul7, %20
  %add9 = add nsw i32 %mul8, 1
  %cmp10 = icmp slt i32 %17, %add9
  br i1 %cmp10, label %for.body12, label %for.cond.cleanup11

for.cond.cleanup11:                               ; preds = %for.cond6
  store i32 8, i32* %cleanup.dest.slot, align 4
  %21 = bitcast i32* %j5 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %21) #3
  br label %for.end15

for.body12:                                       ; preds = %for.cond6
  %22 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch.pipe
  %23 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch, align 4, !tbaa !18
  store i32 42, i32* %write.src
  %24 = bitcast %opencl.pipe_rw_t addrspace(1)* %22 to %opencl.pipe_wo_t addrspace(1)*
  %25 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %26 = call i32 @__write_pipe_2_bl(%opencl.pipe_wo_t addrspace(1)* %24, i8 addrspace(4)* %25, i32 4, i32 4)
  br label %for.inc13

for.inc13:                                        ; preds = %for.body12
  %27 = load i32, i32* %j5, align 4, !tbaa !16
  %inc14 = add nsw i32 %27, 1
  store i32 %inc14, i32* %j5, align 4, !tbaa !16
  br label %for.cond6

for.end15:                                        ; preds = %for.cond.cleanup11
  br label %for.inc16

for.inc16:                                        ; preds = %for.end15
  %28 = load i32, i32* %i, align 4, !tbaa !16
  %inc17 = add nsw i32 %28, 1
  store i32 %inc17, i32* %i, align 4, !tbaa !16
  br label %for.cond

for.end18:                                        ; preds = %for.cond.cleanup
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

define void @__pipe_global_ctor() {
entry:
  call void @__pipe_init_intel(%struct.__pipe_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @ch.pipe.bs to %struct.__pipe_t addrspace(1)*), i32 4, i32 0, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @ch.pipe.bs to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch.pipe
  ret void
}

; Function Attrs: nounwind readnone
declare void @__pipe_init_intel(%struct.__pipe_t addrspace(1)*, i32, i32, i32) #2

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
!5 = !{void (i32 addrspace(1)*)* @foo}
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
