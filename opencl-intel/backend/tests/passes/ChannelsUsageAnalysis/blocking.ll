; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int a;
;
; __kernel void writer() {
;   write_channel_intel(a, 10);
; }
;
; __kernel void reader(__global int *data) {
;   *data = read_channel_intel(a);
; }
; ----------------------------------------------------
; Compile options:
;   -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -O2 -disable-llvm-passes -x cl
;   oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S
; ----------------------------------------------------
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -analyze -channels-usage-analysis %s -S | FileCheck %s

; CHECK: digraph G {
; CHECK: edge [arrowtail=box,arrowhead=normal,dir=both,label="a"];
; CHECK-NEXT: "writer" -> "reader"
; CHECK-NEXT: }

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
%opencl.pipe_rw_t = type opaque
%opencl.pipe_wo_t = type opaque
%opencl.pipe_ro_t = type opaque
%struct.__pipe_t = type { i32, i32, i32, i32, i32, i32, [0 x i8] }

@a = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @__pipe_global_ctor, i8* null }]
@a.pipe = addrspace(1) global %opencl.pipe_rw_t addrspace(1)* null, align 8, !packet_size !0, !packet_align !0
@a.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4

; Function Attrs: convergent nounwind
define spir_kernel void @writer() #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !4 !kernel_arg_pipe_depth !4 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 {
entry:
  %write.src = alloca i32
  %0 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @a.pipe
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @a, align 4, !tbaa !6
  store i32 10, i32* %write.src
  %2 = bitcast %opencl.pipe_rw_t addrspace(1)* %0 to %opencl.pipe_wo_t addrspace(1)*
  %3 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %4 = call i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t addrspace(1)* %2, i8 addrspace(4)* %3, i32 4, i32 4)
  ret void
}

; Function Attrs: convergent nounwind
define spir_kernel void @reader(i32 addrspace(1)* %data) #0 !kernel_arg_addr_space !9 !kernel_arg_access_qual !10 !kernel_arg_type !11 !kernel_arg_base_type !11 !kernel_arg_type_qual !12 !kernel_arg_host_accessible !13 !kernel_arg_pipe_depth !14 !kernel_arg_pipe_io !12 !kernel_arg_buffer_location !12 {
entry:
  %read.dst = alloca i32
  %data.addr = alloca i32 addrspace(1)*, align 8
  store i32 addrspace(1)* %data, i32 addrspace(1)** %data.addr, align 8, !tbaa !15
  %0 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @a.pipe
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @a, align 4, !tbaa !6
  %2 = bitcast %opencl.pipe_rw_t addrspace(1)* %0 to %opencl.pipe_ro_t addrspace(1)*
  %3 = addrspacecast i32* %read.dst to i8 addrspace(4)*
  %call1 = call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)* %2, i8 addrspace(4)* %3, i32 4, i32 4)
  %4 = load i32, i32* %read.dst
  %5 = load i32 addrspace(1)*, i32 addrspace(1)** %data.addr, align 8, !tbaa !15
  store i32 %4, i32 addrspace(1)* %5, align 4, !tbaa !17
  ret void
}

define void @__pipe_global_ctor() {
entry:
  call void @__pipe_init_fpga(%struct.__pipe_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @a.pipe.bs to %struct.__pipe_t addrspace(1)*), i32 4, i32 0, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @a.pipe.bs to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @a.pipe
  ret void
}

; Function Attrs: nounwind readnone
declare void @__pipe_init_fpga(%struct.__pipe_t addrspace(1)*, i32, i32, i32) #1

; Function Attrs: nounwind readnone
declare i32 @__write_pipe_2_fpga(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(4)* nocapture readonly, i32, i32) #1

declare i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(4)*, i32, i32)

; Function Attrs: nounwind readnone
declare i32 @__read_pipe_2_fpga(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(4)* nocapture, i32, i32) #1

declare i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(4)*, i32, i32)

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!1}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!2}
!opencl.spir.version = !{!3}
!opencl.used.extensions = !{!4}
!opencl.used.optional.core.features = !{!4}
!opencl.compiler.options = !{!4}
!llvm.ident = !{!5}

!0 = !{i32 4}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, i32 0}
!3 = !{i32 1, i32 2}
!4 = !{}
!5 = !{!"clang version 8.0.0 "}
!6 = !{!7, !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{i32 1}
!10 = !{!"none"}
!11 = !{!"int*"}
!12 = !{!""}
!13 = !{i1 false}
!14 = !{i32 0}
!15 = !{!16, !16, i64 0}
!16 = !{!"any pointer", !7, i64 0}
!17 = !{!18, !18, i64 0}
!18 = !{!"int", !7, i64 0}
