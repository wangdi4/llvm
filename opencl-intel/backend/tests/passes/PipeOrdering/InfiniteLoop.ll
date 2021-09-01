; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int ch;
;
; __kernel void foo() {
;   // Loop only in for header.
;   while (true) {
;     write_channel_intel(ch, 42);
;   }
; }
; ----------------------------------------------------
; Compile options:
;   clang -cc1 -x cl -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -finclude-default-header -cl-std=CL1.2 -emit-llvm
; Optimizer options:
;   opt -runtimelib=%p/../../vectorizer/Full/runtime.bc -demangle-fpga-pipes -llvm-equalizer -channel-pipe-transformation -verify %s -S
; ----------------------------------------------------
; RUN: %oclopt -pipe-ordering %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -pipe-ordering -verify %s -S | FileCheck %s

; CHECK-LABEL: while.body:
; CHECK: call i32 @__write_pipe_2_bl
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK: br label %while.body

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
define void @foo() #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !3 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !3 !kernel_arg_host_accessible !3 !kernel_arg_pipe_depth !3 !kernel_arg_pipe_io !3 !kernel_arg_buffer_location !3 {
entry:
  %write.src = alloca i32
  br label %while.body

while.body:                                       ; preds = %while.body, %entry
  %0 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch.pipe
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch, align 4, !tbaa !6
  store i32 42, i32* %write.src
  %2 = bitcast %opencl.pipe_rw_t addrspace(1)* %0 to %opencl.pipe_wo_t addrspace(1)*
  %3 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %4 = call i32 @__write_pipe_2_bl(%opencl.pipe_wo_t addrspace(1)* %2, i8 addrspace(4)* %3, i32 4, i32 4)
  br label %while.body

return:                                           ; No predecessors!
  ret void
}

define void @__pipe_global_ctor() {
entry:
  call void @__pipe_init_intel(%struct.__pipe_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @ch.pipe.bs to %struct.__pipe_t addrspace(1)*), i32 4, i32 0, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @ch.pipe.bs to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch.pipe
  ret void
}

; Function Attrs: nounwind readnone
declare void @__pipe_init_intel(%struct.__pipe_t addrspace(1)*, i32, i32, i32) #1

; Function Attrs: nounwind readnone
declare i32 @__write_pipe_2(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(4)* nocapture readonly, i32, i32) #1

declare i32 @__write_pipe_2_bl(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(4)*, i32, i32)

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!1}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!2}
!opencl.spir.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!sycl.kernels = !{!5}

!0 = !{i32 4}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, i32 2}
!3 = !{}
!4 = !{!"clang version 7.0.0 "}
!5 = !{void ()* @foo}
!6 = !{!7, !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}

; DEBUGIFY-NOT: WARNING
