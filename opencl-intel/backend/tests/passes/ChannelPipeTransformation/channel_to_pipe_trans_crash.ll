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
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -instcombine -channel-pipe-transformation -verify %s -S | FileCheck %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
; CHECK: %opencl.pipe_rw_t = type opaque
; CHECK: %opencl.pipe_ro_t = type opaque

@chanin = common addrspace(1) global [8 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4, !packet_size !0, !packet_align !0, !depth !1
; CHECK: @chanin.pipe = {{.*}} global [8 x %opencl.pipe_rw_t {{.*}}

; Function Attrs: convergent norecurse nounwind
define spir_kernel void @test() #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !4 !kernel_arg_pipe_depth !4 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 {
entry:
  %res = alloca float, align 4
  %0 = bitcast float* %res to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  ; CHECK: %[[PIPE_RW:[0-9]+]] = load %opencl.pipe_rw_t {{[^*]*}}*, %opencl.pipe_rw_t {{.*}} getelementptr inbounds ([8 x %opencl.pipe_rw_t {{.*}} @chanin.pipe, i64 0, i64 0)
  ; CHECK: %[[PIPE_RO:[0-9]+]] = bitcast %opencl.pipe_rw_t {{[^*]*}}* %[[PIPE_RW]] to %opencl.pipe_ro_t {{[^*]*}}*
  ; CHECK: %{{[0-9a-z]+}} = call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t {{[^*]*}}* %[[PIPE_RO]]
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([8 x %opencl.channel_t addrspace(1)*], [8 x %opencl.channel_t addrspace(1)*] addrspace(1)* @chanin, i64 0, i64 0), align 4, !tbaa !6
  %call = call float @_Z18read_channel_intel11ocl_channelf(%opencl.channel_t addrspace(1)* %1) #4
  store float %call, float* %res, align 4, !tbaa !9
  %2 = bitcast float* %res to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %2) #3
  ret void
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: convergent
declare float @_Z18read_channel_intel11ocl_channelf(%opencl.channel_t addrspace(1)*) #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { convergent norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { convergent }

!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!2}
!opencl.spir.version = !{!3}
!opencl.used.extensions = !{!4}
!opencl.used.optional.core.features = !{!4}
!opencl.compiler.options = !{!4}
!llvm.ident = !{!5}

!0 = !{i32 4}
!1 = !{i32 8}
!2 = !{i32 1, i32 0}
!3 = !{i32 1, i32 2}
!4 = !{}
!5 = !{!"clang version 11.0.0"}
!6 = !{!7, !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!10, !10, i64 0}
!10 = !{!"float", !7, i64 0}
