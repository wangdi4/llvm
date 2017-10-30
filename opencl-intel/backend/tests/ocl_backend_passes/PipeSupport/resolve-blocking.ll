; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int cin;
; channel int cout;
;
; __kernel void k1() {
;   int i = read_channel_intel(cin);
;   write_channel_intel(cout, i);
; }
; ----------------------------------------------------
; Compilation command:
;   clang -cc1 -triple spir64-unknown-unknown-intelfpga -emit-llvm -cl-std=CL2.0
;   oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation
; ----------------------------------------------------
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -pipe-support -verify %s -S | FileCheck %s
; REQUIRES: fpga-emulator

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.pipe_t = type opaque
%struct.__pipe_t = type { i32, i32, i32, i32, i32, i32, [0 x i8] }

@pipe.cin = common addrspace(1) global %opencl.pipe_t addrspace(1)* null, align 4
@pipe.cout = common addrspace(1) global %opencl.pipe_t addrspace(1)* null, align 4

; Function Attrs: nounwind
define spir_kernel void @k1() #0 {
entry:
  %write.src = alloca i32
  %read.dst = alloca i32
  %0 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)* addrspace(1)* @pipe.cin
  %1 = bitcast %opencl.pipe_t addrspace(1)* %0 to %struct.__pipe_t addrspace(1)*
  %2 = addrspacecast i32* %read.dst to i8 addrspace(4)*
  %3 = call i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %1, i8 addrspace(4)* %2)
  ; CHECK: [[body0:pipe_bl.body.*]]:
  ; CHECK:   %[[ret0:[0-9]+]] = call i32 @__read_pipe_2_intel
  ; CHECK:   %[[failed0:pipe.failed.*]] = icmp ne i32 %[[ret0]], 0
  ; CHECK:   br i1 %[[failed0]], label %[[body0]], label %[[exit0:pipe_bl.exit.*]]
  ; CHECK: [[exit0]]:

  %4 = load i32, i32* %read.dst
  store i32 %4, i32* %write.src
  %5 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)* addrspace(1)* @pipe.cout
  %6 = bitcast %opencl.pipe_t addrspace(1)* %5 to %struct.__pipe_t addrspace(1)*
  %7 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %8 = call i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %6, i8 addrspace(4)* %7)
  ; CHECK: [[body1:pipe_bl.body.*]]:
  ; CHECK:   %[[ret1:[0-9]+]] = call i32 @__write_pipe_2_intel
  ; CHECK:   %[[failed1:pipe.failed.*]] = icmp ne i32 %[[ret1]], 0
  ; CHECK:   br i1 %[[failed1]], label %[[body1]], label %[[exit1:pipe_bl.exit.*]]
  ; CHECK: [[exit1]]:

  ret void
}

; Function Attrs: alwaysinline nounwind
declare i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)*) #3

; Function Attrs: alwaysinline nounwind
declare i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)*) #3

; Function Attrs: nounwind readnone
declare i32 @__read_pipe_2_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)* nocapture) #2

; Function Attrs: nounwind readnone
declare i32 @__write_pipe_2_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)* nocapture readonly) #2

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!11}
!opencl.spir.version = !{!11}
!opencl.used.extensions = !{!12}
!opencl.used.optional.core.features = !{!12}
!opencl.compiler.options = !{!12}
!llvm.ident = !{!13}

!0 = !{void ()* @k1, !1, !2, !3, !4, !5, !6}
!1 = !{!"kernel_arg_addr_space"}
!2 = !{!"kernel_arg_access_qual"}
!3 = !{!"kernel_arg_type"}
!4 = !{!"kernel_arg_base_type"}
!5 = !{!"kernel_arg_type_qual"}
!6 = !{!"kernel_arg_host_accessible"}
!11 = !{i32 2, i32 0}
!12 = !{}
!13 = !{!"clang version 3.8.1"}
!14 = !{!15, !15, i64 0}
!15 = !{!"omnipotent char", !16, i64 0}
!16 = !{!"Simple C/C++ TBAA"}
