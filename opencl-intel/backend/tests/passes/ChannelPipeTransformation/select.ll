; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int ch1;
; channel int ch2;
;
; __kernel void k(__global int *cond, __global int *res) {
;   *res = (cond) ? read_channel_intel(ch1) : read_channel_intel(ch2);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -x cl -cl-std=CL1.2 -finclude-default-header
; ----------------------------------------------------
; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s

; CHECK: @[[PIPE1:.*]] = addrspace(1) global %opencl.pipe_rw_t addrspace(1)*
; CHECK: @[[PIPE2:.*]] = addrspace(1) global %opencl.pipe_rw_t addrspace(1)*

; CHECK: %[[PIPE2VAL:.*]] = load %opencl.pipe_rw_t addrspace(1)*{{.*}} @[[PIPE2]]
; CHECK: %[[PIPE1VAL:.*]] = load %opencl.pipe_rw_t addrspace(1)*{{.*}} @[[PIPE1]]
; CHECK: %[[PIPESEL:.*]] = select i1 %tobool, %opencl.pipe_rw_t addrspace(1)* %[[PIPE2VAL]], %opencl.pipe_rw_t addrspace(1)* %[[PIPE1VAL]]
; CHECK: %[[PIPECAST:.*]] = bitcast %opencl.pipe_rw_t addrspace(1)* %[[PIPESEL]] to %opencl.pipe_ro_t addrspace(1)*
; CHECK: call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)* %[[PIPECAST]]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque

@ch1 = common local_unnamed_addr addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@ch2 = common local_unnamed_addr addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0

; Function Attrs: convergent nounwind
define spir_kernel void @k(i32 addrspace(1)* readnone %cond, i32 addrspace(1)* nocapture %res) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 {
entry:
  %tobool = icmp eq i32 addrspace(1)* %cond, null
  %ch2.val = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch2, align 4
  %ch1.val = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch1, align 4
  %0 = select i1 %tobool, %opencl.channel_t addrspace(1)* %ch2.val, %opencl.channel_t addrspace(1)* %ch1.val
  %call1 = tail call i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)* %0) #2
  store i32 %call1, i32 addrspace(1)* %res, align 4, !tbaa !11
  ret void
}

; Function Attrs: convergent
declare i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)*) local_unnamed_addr #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent nounwind }

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
!4 = !{!"clang version 7.0.0 "}
!5 = !{i32 1, i32 1}
!6 = !{!"none", !"none"}
!7 = !{!"int*", !"int*"}
!8 = !{!"", !""}
!9 = !{i1 false, i1 false}
!10 = !{i32 0, i32 0}
!11 = !{!12, !12, i64 0}
!12 = !{!"int", !13, i64 0}
!13 = !{!"omnipotent char", !14, i64 0}
!14 = !{!"Simple C/C++ TBAA"}
