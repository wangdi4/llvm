; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel char bar;
; channel double16 far;
;
; __kernel void foo(char c, __global double16 *d) {
;   write_channel_intel(bar, c);
;   *d = read_channel_intel(far);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL1.2
; ----------------------------------------------------
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -run-twice -verify %s -S | FileCheck %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque

@bar = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 1, !packet_size !0, !packet_align !0
@far = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 128, !packet_size !1, !packet_align !1

; CHECK: define {{.*}} void @foo
; CHECK: %write.src = alloca i8
; CHECK: %read.dst = alloca <16 x double>

; Function Attrs: convergent nounwind
define spir_kernel void @foo(i8 signext %c, <16 x double> addrspace(1)* %d) #0 !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_base_type !10 !kernel_arg_type_qual !11 !kernel_arg_host_accessible !12 !kernel_arg_pipe_depth !13 !kernel_arg_pipe_io !11 !kernel_arg_buffer_location !11 {
entry:
  %c.addr = alloca i8, align 1
  %d.addr = alloca <16 x double> addrspace(1)*, align 8
  store i8 %c, i8* %c.addr, align 1, !tbaa !14
  store <16 x double> addrspace(1)* %d, <16 x double> addrspace(1)** %d.addr, align 8, !tbaa !17
  %0 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 1, !tbaa !14
  %1 = load i8, i8* %c.addr, align 1, !tbaa !14
  call void @_Z19write_channel_intel11ocl_channelcc(%opencl.channel_t addrspace(1)* %0, i8 signext %1) #2
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @far, align 128, !tbaa !14
  %call = call <16 x double> @_Z18read_channel_intel11ocl_channelDv16_d(%opencl.channel_t addrspace(1)* %2) #2
  %3 = load <16 x double> addrspace(1)*, <16 x double> addrspace(1)** %d.addr, align 8, !tbaa !17
  store <16 x double> %call, <16 x double> addrspace(1)* %3, align 128, !tbaa !14
  ret void
}

; Function Attrs: convergent
declare void @_Z19write_channel_intel11ocl_channelcc(%opencl.channel_t addrspace(1)*, i8 signext) #1

; Function Attrs: convergent
declare <16 x double> @_Z18read_channel_intel11ocl_channelDv16_d(%opencl.channel_t addrspace(1)*) #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent }

!llvm.module.flags = !{!2}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!3}
!opencl.spir.version = !{!3}
!opencl.used.extensions = !{!4}
!opencl.used.optional.core.features = !{!5}
!opencl.compiler.options = !{!4}
!llvm.ident = !{!6}

!0 = !{i32 1}
!1 = !{i32 128}
!2 = !{i32 1, !"wchar_size", i32 4}
!3 = !{i32 1, i32 2}
!4 = !{}
!5 = !{!"cl_doubles"}
!6 = !{!"clang version 6.0.0 "}
!7 = !{i32 0, i32 1}
!8 = !{!"none", !"none"}
!9 = !{!"char", !"double16*"}
!10 = !{!"char", !"double __attribute__((ext_vector_type(16)))*"}
!11 = !{!"", !""}
!12 = !{i1 false, i1 false}
!13 = !{i32 0, i32 0}
!14 = !{!15, !15, i64 0}
!15 = !{!"omnipotent char", !16, i64 0}
!16 = !{!"Simple C/C++ TBAA"}
!17 = !{!18, !18, i64 0}
!18 = !{!"any pointer", !15, i64 0}
