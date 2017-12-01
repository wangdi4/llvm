; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_altera_channels : enable
;
; channel int ch1;
; channel int ch2;
;
; __kernel void k(__global int *cond, __global int *res) {
;   *res = (cond) ? read_channel_intel(ch1) : read_channel_intel(ch2);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------
; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s

source_filename = "select.cl"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque

@ch1 = common local_unnamed_addr addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@ch2 = common local_unnamed_addr addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4

; CHECK: @[[PIPE1:.*]] = addrspace(1) global %opencl.pipe_t addrspace(1)*
; CHECK: @[[PIPE2:.*]] = addrspace(1) global %opencl.pipe_t addrspace(1)*

; CHECK: %[[PIPE2VAL:.*]] = load %opencl.pipe_t addrspace(1)*{{.*}} @[[PIPE2]]
; CHECK: %[[PIPE1VAL:.*]] = load %opencl.pipe_t addrspace(1)*{{.*}} @[[PIPE1]]
; CHECK: %[[PIPESEL:.*]] = select i1 %tobool, %opencl.pipe_t addrspace(1)* %[[PIPE2VAL]], %opencl.pipe_t addrspace(1)* %[[PIPE1VAL]]
; CHECK: %[[PIPECAST:.*]] = bitcast %opencl.pipe_t addrspace(1)* %[[PIPESEL]] to %struct.__pipe_t addrspace(1)*
; CHECK: call i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %[[PIPECAST]]

; Function Attrs: nounwind
define spir_kernel void @k(i32 addrspace(1)* readnone %cond, i32 addrspace(1)* nocapture %res) local_unnamed_addr #0 !kernel_arg_addr_space !8 !kernel_arg_access_qual !9 !kernel_arg_type !10 !kernel_arg_base_type !10 !kernel_arg_type_qual !11 !kernel_arg_host_accessible !12 {
entry:
  %tobool = icmp eq i32 addrspace(1)* %cond, null
  %ch2.val = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch2, align 4
  %ch1.val = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch1, align 4
  %0 = select i1 %tobool, %opencl.channel_t addrspace(1)* %ch2.val, %opencl.channel_t addrspace(1)* %ch1.val
  %call1 = tail call i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)* %0) #2
  store i32 %call1, i32 addrspace(1)* %res, align 4, !tbaa !13
  ret void
}

declare i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)*) local_unnamed_addr #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!opencl.channels = !{!0, !3}
!llvm.module.flags = !{!4}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!5}
!opencl.spir.version = !{!5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!6}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}

!0 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @ch1, !1, !2}
!1 = !{!"packet_size", i32 4}
!2 = !{!"packet_align", i32 4}
!3 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @ch2, !1, !2}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 2, i32 0}
!6 = !{}
!7 = !{!"clang version 6.0.0"}
!8 = !{i32 1, i32 1}
!9 = !{!"none", !"none"}
!10 = !{!"int*", !"int*"}
!11 = !{!"", !""}
!12 = !{i1 false, i1 false}
!13 = !{!14, !14, i64 0}
!14 = !{!"int", !15, i64 0}
!15 = !{!"omnipotent char", !16, i64 0}
!16 = !{!"Simple C/C++ TBAA"}
