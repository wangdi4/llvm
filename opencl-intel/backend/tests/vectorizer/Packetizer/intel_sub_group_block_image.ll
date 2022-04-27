; RUN: %oclopt -runtimelib %p/../Full/runtime.bc -scalarize -predicate -packetize -packet-size=4 -verify %s -S -o - | FileCheck %s
; ModuleID = 'intel_sub_group_block_image.cl'
source_filename = "intel_sub_group_block_image.cl"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

%opencl.image2d_rw_t = type opaque
; Function Attrs: convergent nounwind
define spir_kernel void @test(%opencl.image2d_rw_t addrspace(1)* %image) local_unnamed_addr #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 !kernel_arg_host_accessible !8 !kernel_arg_pipe_depth !9 !kernel_arg_pipe_io !7 !kernel_arg_buffer_location !7 {
entry:
  %call = tail call spir_func <2 x i32> @_Z27intel_sub_group_block_read214ocl_image2d_rwDv2_i(%opencl.image2d_rw_t addrspace(1)* %image, <2 x i32> <i32 1, i32 1>) #2
; CHECK: = call <8 x i32> @_Z29intel_sub_group_block_read2_414ocl_image2d_rwDv2_iDv4_j(%opencl.image2d_rw_t addrspace(1)* {{%.*}}, <2 x i32> {{%.*}}, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
  tail call spir_func void @_Z28intel_sub_group_block_write214ocl_image2d_rwDv2_iDv2_j(%opencl.image2d_rw_t addrspace(1)* %image, <2 x i32> <i32 1, i32 1>, <2 x i32> %call) #2
; CHECK: call void @_Z30intel_sub_group_block_write2_414ocl_image2d_rwDv2_iDv8_jDv4_j(%opencl.image2d_rw_t addrspace(1)* {{%.*}}, <2 x i32> {{%.*}}, <8 x i32> {{%.*}}, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
 %call1 = tail call spir_func <2 x i16> @_Z30intel_sub_group_block_read_us214ocl_image2d_rwDv2_i(%opencl.image2d_rw_t addrspace(1)* %image, <2 x i32> <i32 1, i32 1>) #2
; CHECK: = call <8 x i16> @_Z32intel_sub_group_block_read_us2_414ocl_image2d_rwDv2_iDv4_j(%opencl.image2d_rw_t addrspace(1)* {{%.*}}, <2 x i32> {{%.*}}, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
  tail call spir_func void @_Z31intel_sub_group_block_write_us214ocl_image2d_rwDv2_iDv2_t(%opencl.image2d_rw_t addrspace(1)* %image, <2 x i32> <i32 1, i32 1>, <2 x i16> %call1) #2
; CHECK: call void @_Z33intel_sub_group_block_write_us2_414ocl_image2d_rwDv2_iDv8_tDv4_j(%opencl.image2d_rw_t addrspace(1)* {{%.*}}, <2 x i32> {{%.*}}, <8 x i16> {{%.*}}, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
  ret void
}
; Function Attrs: convergent
declare spir_func <2 x i32> @_Z27intel_sub_group_block_read214ocl_image2d_rwDv2_i(%opencl.image2d_rw_t addrspace(1)*, <2 x i32>) local_unnamed_addr #1
; Function Attrs: convergent
declare spir_func void @_Z28intel_sub_group_block_write214ocl_image2d_rwDv2_iDv2_j(%opencl.image2d_rw_t addrspace(1)*, <2 x i32>, <2 x i32>) local_unnamed_addr #1
; Function Attrs: convergent
declare spir_func <2 x i16> @_Z30intel_sub_group_block_read_us214ocl_image2d_rwDv2_i(%opencl.image2d_rw_t addrspace(1)*, <2 x i32>) local_unnamed_addr #1
; Function Attrs: convergent
declare spir_func void @_Z31intel_sub_group_block_write_us214ocl_image2d_rwDv2_iDv2_t(%opencl.image2d_rw_t addrspace(1)*, <2 x i32>, <2 x i16>) local_unnamed_addr #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="64" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent nounwind }

!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!1}
!llvm.ident = !{!3}

!0 = !{i32 2, i32 0}
!1 = !{}
!2 = !{!"cl_images"}
!3 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!4 = !{i32 1}
!5 = !{!"read_write"}
!6 = !{!"image2d_t"}
!7 = !{!""}
!8 = !{i1 false}
!9 = !{i32 0}
