; RUN: opt -passes=sycl-kernel-vec-clone -sycl-vector-variant-isa-encoding-override=AVX512Core -sycl-vect-info=%p/../Inputs/VectInfo64.gen %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-vec-clone,vplan-vec -sycl-vector-variant-isa-encoding-override=AVX512Core -sycl-vect-info=%p/../Inputs/VectInfo64.gen %s -S -o - | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "x86_64-unknown-linux-gnu"

define void @test(ptr addrspace(1) %ints, ptr addrspace(1) %coords) !recommended_vector_length !3 !kernel_arg_base_type !4 !arg_type_null_val !5 {
  %gid = tail call i64 @_Z13get_global_idj(i32 0)
  %ints.i = getelementptr inbounds i32, ptr addrspace(1) %ints, i64 %gid
  %coords.i = getelementptr inbounds <2 x i32>, ptr addrspace(1) %coords, i64 %gid

  %int = load i32, ptr addrspace(1) %ints.i
  %long = zext i32 %int to i64
  %short = trunc i32 %int to i16
  %char = trunc i32 %int to i8

  %coord = load <2 x i32>, ptr addrspace(1) %coords.i
  %pint = inttoptr i64 %long to ptr addrspace(1)
  %img_ro = inttoptr i64 %long to ptr addrspace(1)
  %img_wo = inttoptr i64 %long to ptr addrspace(1)
  %img_rw = inttoptr i64 %long to ptr addrspace(1)

  ; sub_group_broadcast
  %call1 = tail call i32 @_Z19sub_group_broadcastij(i32 %int, i32 %int)
  %call2 = tail call i64 @_Z19sub_group_broadcastlj(i64 %long, i32 %int)
  %call3 = tail call i16 @_Z25intel_sub_group_broadcastsj(i16 %short, i32 %int)
  %call4 = tail call i8 @_Z25intel_sub_group_broadcastcj(i8 %char, i32 %int)
; CHECK: call <16 x i32> @_Z19sub_group_broadcastDv16_iDv16_jS0_
; CHECK: call <16 x i64> @_Z19sub_group_broadcastDv16_lDv16_jS0_
; CHECK: call <16 x i16> @_Z25intel_sub_group_broadcastDv16_sDv16_jS0_
; CHECK: call <16 x i8> @_Z25intel_sub_group_broadcastDv16_cDv16_jS0_

  ; sub_group_block_read
  %call5 = tail call i32 @_Z26intel_sub_group_block_readPU3AS1Kj(ptr addrspace(1) %pint)
  %call6 = tail call <8 x i32> @_Z27intel_sub_group_block_read8PU3AS1Kj(ptr addrspace(1) %pint)
; CHECK: call <16 x i32> @_Z30intel_sub_group_block_read1_16Dv16_PU3AS1KjDv16_j
; CHECK: call <128 x i32> @_Z30intel_sub_group_block_read8_16Dv16_PU3AS1KjDv16_j
  %call7 = tail call i32 @_Z26intel_sub_group_block_read14ocl_image2d_roDv2_i(ptr addrspace(1) %img_ro, <2 x i32> %coord)
  %call8 = tail call i32 @_Z26intel_sub_group_block_read14ocl_image2d_rwDv2_i(ptr addrspace(1) %img_rw, <2 x i32> %coord)
; CHECK: call <16 x i32> @_Z30intel_sub_group_block_read1_16Dv16_14ocl_image2d_roDv32_iDv16_j
; CHECK: call <16 x i32> @_Z30intel_sub_group_block_read1_16Dv16_14ocl_image2d_rwDv32_iDv16_j
  %call9 = tail call <8 x i32> @_Z27intel_sub_group_block_read814ocl_image2d_roDv2_i(ptr addrspace(1) %img_ro, <2 x i32> %coord)
  %call10 = tail call <8 x i32> @_Z27intel_sub_group_block_read814ocl_image2d_rwDv2_i(ptr addrspace(1) %img_rw, <2 x i32> %coord)
; CHECK: call <128 x i32> @_Z30intel_sub_group_block_read8_16Dv16_14ocl_image2d_roDv32_iDv16_j
; CHECK: call <128 x i32> @_Z30intel_sub_group_block_read8_16Dv16_14ocl_image2d_rwDv32_iDv16_j

  ; sub_group_block_write

  tail call void @_Z27intel_sub_group_block_writePU3AS1jj(ptr addrspace(1) %pint, i32 %int)
  tail call void @_Z28intel_sub_group_block_write8PU3AS1jDv8_j(ptr addrspace(1) %pint, <8 x i32> undef)
; CHECK: call void @_Z31intel_sub_group_block_write1_16Dv16_PU3AS1jDv16_jS2_
; CHECK: call void @_Z31intel_sub_group_block_write8_16Dv16_PU3AS1jDv128_jDv16_j
  tail call void @_Z27intel_sub_group_block_write14ocl_image2d_woDv2_ij(ptr addrspace(1) %img_wo, <2 x i32> %coord, i32 %int)
  tail call void @_Z27intel_sub_group_block_write14ocl_image2d_rwDv2_ij(ptr addrspace(1) %img_rw, <2 x i32> %coord, i32 %int)
; CHECK: call void @_Z31intel_sub_group_block_write1_16Dv16_14ocl_image2d_woDv32_iDv16_jS2_
; CHECK: call void @_Z31intel_sub_group_block_write1_16Dv16_14ocl_image2d_rwDv32_iDv16_jS2_
  tail call void @_Z28intel_sub_group_block_write814ocl_image2d_woDv2_iDv8_j(ptr addrspace(1) %img_wo, <2 x i32> %coord, <8 x i32> undef)
  tail call void @_Z28intel_sub_group_block_write814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img_rw, <2 x i32> %coord, <8 x i32> undef)
; CHECK: call void @_Z31intel_sub_group_block_write8_16Dv16_14ocl_image2d_woDv32_iDv128_jDv16_j
; CHECK: call void @_Z31intel_sub_group_block_write8_16Dv16_14ocl_image2d_rwDv32_iDv128_jDv16_j
  ret void
}

; sub_group_broadcast
declare i32 @_Z19sub_group_broadcastij(i32, i32)
declare i64 @_Z19sub_group_broadcastlj(i64, i32)
declare i16 @_Z25intel_sub_group_broadcastsj(i16, i32)
declare i8 @_Z25intel_sub_group_broadcastcj(i8, i32)

; sub_group_block_read
declare i32 @_Z26intel_sub_group_block_readPU3AS1Kj(ptr addrspace(1))
declare <8 x i32> @_Z27intel_sub_group_block_read8PU3AS1Kj(ptr addrspace(1))
declare i32 @_Z26intel_sub_group_block_read14ocl_image2d_roDv2_i(ptr addrspace(1), <2 x i32>)
declare i32 @_Z26intel_sub_group_block_read14ocl_image2d_rwDv2_i(ptr addrspace(1), <2 x i32>)
declare <8 x i32> @_Z27intel_sub_group_block_read814ocl_image2d_roDv2_i(ptr addrspace(1), <2 x i32>)
declare <8 x i32> @_Z27intel_sub_group_block_read814ocl_image2d_rwDv2_i(ptr addrspace(1), <2 x i32>)

; sub_group_block_write
declare void @_Z27intel_sub_group_block_writePU3AS1jj(ptr addrspace(1), i32)
declare void @_Z28intel_sub_group_block_write8PU3AS1jDv8_j(ptr addrspace(1), <8 x i32>)
declare void @_Z27intel_sub_group_block_write14ocl_image2d_woDv2_ij(ptr addrspace(1), <2 x i32>, i32)
declare void @_Z27intel_sub_group_block_write14ocl_image2d_rwDv2_ij(ptr addrspace(1), <2 x i32>, i32)
declare void @_Z28intel_sub_group_block_write814ocl_image2d_woDv2_iDv8_j(ptr addrspace(1), <2 x i32>, <8 x i32>)
declare void @_Z28intel_sub_group_block_write814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1), <2 x i32>, <8 x i32>)

declare i64 @_Z13get_global_idj(i32)

!sycl.kernels = !{!2}

!0 = !{i32 2, i32 0}
!1 = !{}
!2 = !{ptr @test}
!3 = !{i32 16}
!4 = !{!"int*", !"int2*"}
!5 = !{ptr addrspace(1) null, ptr addrspace(1) null}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu_test {{.*}} br
; DEBUGIFY-NOT: WARNING
