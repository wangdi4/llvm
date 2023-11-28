; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
; #pragma OPENCL EXTENSION cl_khr_fp64 : enable
;
; channel char bar;
; typedef double double16 __attribute__((ext_vector_type(16)));
; channel double16 far;
;
; __kernel void foo(char c, __global double16 *d) {
;   write_channel_intel(bar, c);
;   *d = read_channel_intel(far);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL1.2
; opt -passes=sycl-kernel-target-ext-type-lower,sycl-kernel-equalizer %s -S
; ----------------------------------------------------

; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation -run-twice %s -S | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@bar = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@far = addrspace(1) global ptr addrspace(1) null, align 128, !packet_size !1, !packet_align !1

; CHECK: define {{.*}} void @foo
; CHECK: %write.src = alloca i8
; CHECK: %read.dst = alloca <16 x double>

; Function Attrs: convergent norecurse nounwind
define dso_local void @foo(i8 noundef signext %c, ptr addrspace(1) noundef align 128 %d) #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !9 !kernel_arg_type_qual !10 !kernel_arg_host_accessible !11 !kernel_arg_pipe_depth !12 !kernel_arg_pipe_io !10 !kernel_arg_buffer_location !10 !arg_type_null_val !13 {
entry:
  %c.addr = alloca i8, align 1
  %d.addr = alloca ptr addrspace(1), align 8
  store i8 %c, ptr %c.addr, align 1, !tbaa !14
  store ptr addrspace(1) %d, ptr %d.addr, align 8, !tbaa !17
  %0 = load ptr addrspace(1), ptr addrspace(1) @bar, align 1, !tbaa !14
  %1 = load i8, ptr %c.addr, align 1, !tbaa !14
  call void @_Z19write_channel_intel11ocl_channelcc(ptr addrspace(1) %0, i8 noundef signext %1) #2
  %2 = load ptr addrspace(1), ptr addrspace(1) @far, align 128, !tbaa !14
  %call = call <16 x double> @_Z18read_channel_intel11ocl_channelDv16_d(ptr addrspace(1) %2) #2
  %3 = load ptr addrspace(1), ptr %d.addr, align 8, !tbaa !17
  store <16 x double> %call, ptr addrspace(1) %3, align 128, !tbaa !14
  ret void
}

; Function Attrs: convergent nounwind
declare void @_Z19write_channel_intel11ocl_channelcc(ptr addrspace(1), i8 noundef signext) #1

; Function Attrs: convergent nounwind
declare <16 x double> @_Z18read_channel_intel11ocl_channelDv16_d(ptr addrspace(1)) #1

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" }
attributes #1 = { convergent nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #2 = { convergent nounwind }

!opencl.ocl.version = !{!2}
!opencl.spir.version = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!sycl.kernels = !{!5}

!0 = !{i32 1}
!1 = !{i32 128}
!2 = !{i32 1, i32 2}
!3 = !{}
!4 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!5 = !{ptr @foo}
!6 = !{i32 0, i32 1}
!7 = !{!"none", !"none"}
!8 = !{!"char", !"double16*"}
!9 = !{!"char", !"double __attribute__((ext_vector_type(16)))*"}
!10 = !{!"", !""}
!11 = !{i1 false, i1 false}
!12 = !{i32 0, i32 0}
!13 = !{i8 0, ptr addrspace(1) null}
!14 = !{!15, !15, i64 0}
!15 = !{!"omnipotent char", !16, i64 0}
!16 = !{!"Simple C/C++ TBAA"}
!17 = !{!18, !18, i64 0}
!18 = !{!"any pointer", !15, i64 0}

; DEBUGIFY-NOT: WARNING: Missing line
