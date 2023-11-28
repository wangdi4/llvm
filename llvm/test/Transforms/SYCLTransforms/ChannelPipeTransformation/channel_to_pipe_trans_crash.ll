; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
; channel float chanin[8] __attribute__((depth(8)));
;
; kernel void test() {
;   float res = read_channel_intel(chanin[0]);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl
; ----------------------------------------------------

; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes="function(instcombine),sycl-kernel-channel-pipe-transformation" %s -S | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@chanin = addrspace(1) global [8 x ptr addrspace(1)] zeroinitializer, align 8, !packet_size !0, !packet_align !0, !depth !1
; CHECK: @chanin = addrspace(1) global [8 x ptr addrspace(1)] zeroinitializer

; Function Attrs: convergent norecurse nounwind
define dso_local void @test() #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !3 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !3 !kernel_arg_host_accessible !3 !kernel_arg_pipe_depth !3 !kernel_arg_pipe_io !3 !kernel_arg_buffer_location !3 !arg_type_null_val !3 {
entry:
  %res = alloca float, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %res) #3
  %0 = load ptr addrspace(1), ptr addrspace(1) @chanin, align 4, !tbaa !6
  %call = call float @_Z18read_channel_intel11ocl_channelf(ptr addrspace(1) %0) #4
  store float %call, ptr %res, align 4, !tbaa !9
  call void @llvm.lifetime.end.p0(i64 4, ptr %res) #3
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: convergent nounwind
declare float @_Z18read_channel_intel11ocl_channelf(ptr addrspace(1)) #2

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { convergent nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #3 = { nounwind }
attributes #4 = { convergent nounwind }

!opencl.ocl.version = !{!2}
!opencl.spir.version = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!sycl.kernels = !{!5}

!0 = !{i32 4}
!1 = !{i32 8}
!2 = !{i32 1, i32 2}
!3 = !{}
!4 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!5 = !{ptr @test}
!6 = !{!7, !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!10, !10, i64 0}
!10 = !{!"float", !7, i64 0}
