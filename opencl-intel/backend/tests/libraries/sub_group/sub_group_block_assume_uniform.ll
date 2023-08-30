; RUN: llvm-extract %libdir/../clbltfnshared.rtl -rfunc intel_sub_group_block -S -o - | opt -S -o - | FileCheck %s

; CHECK-DAG: define <4 x i32> @_Z29intel_sub_group_block_read1_4Dv4_PU3AS1KjDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask){{.*}} [[ATTR_RO_128:#[0-9]+]]
; CHECK-DAG: call <4 x i32> @_Z29intel_sub_group_block_read1_4PU3AS1KjDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <8 x i32> @_Z29intel_sub_group_block_read1_8Dv8_PU3AS1KjDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <8 x i32> @_Z29intel_sub_group_block_read1_8PU3AS1KjDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <16 x i32> @_Z30intel_sub_group_block_read1_16Dv16_PU3AS1KjDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <16 x i32> @_Z30intel_sub_group_block_read1_16PU3AS1KjDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z30intel_sub_group_block_read1_32Dv32_PU3AS1KjDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z30intel_sub_group_block_read1_32PU3AS1KjDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z30intel_sub_group_block_read1_64Dv64_PU3AS1KjDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z30intel_sub_group_block_read1_64PU3AS1KjDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <8 x i32> @_Z29intel_sub_group_block_read2_4Dv4_PU3AS1KjDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call <8 x i32> @_Z29intel_sub_group_block_read2_4PU3AS1KjDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <16 x i32> @_Z29intel_sub_group_block_read2_8Dv8_PU3AS1KjDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <16 x i32> @_Z29intel_sub_group_block_read2_8PU3AS1KjDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z30intel_sub_group_block_read2_16Dv16_PU3AS1KjDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z30intel_sub_group_block_read2_16PU3AS1KjDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z30intel_sub_group_block_read2_32Dv32_PU3AS1KjDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z30intel_sub_group_block_read2_32PU3AS1KjDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <128 x i32> @_Z30intel_sub_group_block_read2_64Dv64_PU3AS1KjDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <128 x i32> @_Z30intel_sub_group_block_read2_64PU3AS1KjDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <16 x i32> @_Z29intel_sub_group_block_read4_4Dv4_PU3AS1KjDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call <16 x i32> @_Z29intel_sub_group_block_read4_4PU3AS1KjDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z29intel_sub_group_block_read4_8Dv8_PU3AS1KjDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z29intel_sub_group_block_read4_8PU3AS1KjDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z30intel_sub_group_block_read4_16Dv16_PU3AS1KjDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z30intel_sub_group_block_read4_16PU3AS1KjDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <128 x i32> @_Z30intel_sub_group_block_read4_32Dv32_PU3AS1KjDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <128 x i32> @_Z30intel_sub_group_block_read4_32PU3AS1KjDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <256 x i32> @_Z30intel_sub_group_block_read4_64Dv64_PU3AS1KjDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <256 x i32> @_Z30intel_sub_group_block_read4_64PU3AS1KjDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z29intel_sub_group_block_read8_4Dv4_PU3AS1KjDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z29intel_sub_group_block_read8_4PU3AS1KjDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z29intel_sub_group_block_read8_8Dv8_PU3AS1KjDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z29intel_sub_group_block_read8_8PU3AS1KjDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <128 x i32> @_Z30intel_sub_group_block_read8_16Dv16_PU3AS1KjDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <128 x i32> @_Z30intel_sub_group_block_read8_16PU3AS1KjDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <256 x i32> @_Z30intel_sub_group_block_read8_32Dv32_PU3AS1KjDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <256 x i32> @_Z30intel_sub_group_block_read8_32PU3AS1KjDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <512 x i32> @_Z30intel_sub_group_block_read8_64Dv64_PU3AS1KjDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <512 x i32> @_Z30intel_sub_group_block_read8_64PU3AS1KjDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write1_4Dv4_PU3AS1jDv4_jS2_(<4 x ptr addrspace(1)> %data, <4 x i32> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write1_4PU3AS1jDv4_jS1_(ptr addrspace(1) %vecext, <4 x i32> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write1_8Dv8_PU3AS1jDv8_jS2_(<8 x ptr addrspace(1)> %data, <8 x i32> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write1_8PU3AS1jDv8_jS1_(ptr addrspace(1) %vecext, <8 x i32> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write1_16Dv16_PU3AS1jDv16_jS2_(<16 x ptr addrspace(1)> %data, <16 x i32> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write1_16PU3AS1jDv16_jS1_(ptr addrspace(1) %vecext, <16 x i32> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write1_32Dv32_PU3AS1jDv32_jS2_(<32 x ptr addrspace(1)> %data, <32 x i32> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write1_32PU3AS1jDv32_jS1_(ptr addrspace(1) %vecext, <32 x i32> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write1_64Dv64_PU3AS1jDv64_jS2_(<64 x ptr addrspace(1)> %data, <64 x i32> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write1_64PU3AS1jDv64_jS1_(ptr addrspace(1) %vecext, <64 x i32> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write2_4Dv4_PU3AS1jDv8_jDv4_j(<4 x ptr addrspace(1)> %data, <8 x i32> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write2_4PU3AS1jDv8_jDv4_j(ptr addrspace(1) %vecext, <8 x i32> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write2_8Dv8_PU3AS1jDv16_jDv8_j(<8 x ptr addrspace(1)> %data, <16 x i32> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write2_8PU3AS1jDv16_jDv8_j(ptr addrspace(1) %vecext, <16 x i32> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write2_16Dv16_PU3AS1jDv32_jDv16_j(<16 x ptr addrspace(1)> %data, <32 x i32> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write2_16PU3AS1jDv32_jDv16_j(ptr addrspace(1) %vecext, <32 x i32> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write2_32Dv32_PU3AS1jDv64_jDv32_j(<32 x ptr addrspace(1)> %data, <64 x i32> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write2_32PU3AS1jDv64_jDv32_j(ptr addrspace(1) %vecext, <64 x i32> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write2_64Dv64_PU3AS1jDv128_jDv64_j(<64 x ptr addrspace(1)> %data, <128 x i32> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write2_64PU3AS1jDv128_jDv64_j(ptr addrspace(1) %vecext, <128 x i32> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write4_4Dv4_PU3AS1jDv16_jDv4_j(<4 x ptr addrspace(1)> %data, <16 x i32> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write4_4PU3AS1jDv16_jDv4_j(ptr addrspace(1) %vecext, <16 x i32> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write4_8Dv8_PU3AS1jDv32_jDv8_j(<8 x ptr addrspace(1)> %data, <32 x i32> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write4_8PU3AS1jDv32_jDv8_j(ptr addrspace(1) %vecext, <32 x i32> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write4_16Dv16_PU3AS1jDv64_jDv16_j(<16 x ptr addrspace(1)> %data, <64 x i32> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write4_16PU3AS1jDv64_jDv16_j(ptr addrspace(1) %vecext, <64 x i32> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write4_32Dv32_PU3AS1jDv128_jDv32_j(<32 x ptr addrspace(1)> %data, <128 x i32> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write4_32PU3AS1jDv128_jDv32_j(ptr addrspace(1) %vecext, <128 x i32> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write4_64Dv64_PU3AS1jDv256_jDv64_j(<64 x ptr addrspace(1)> %data, <256 x i32> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write4_64PU3AS1jDv256_jDv64_j(ptr addrspace(1) %vecext, <256 x i32> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write8_4Dv4_PU3AS1jDv32_jDv4_j(<4 x ptr addrspace(1)> %data, <32 x i32> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write8_4PU3AS1jDv32_jDv4_j(ptr addrspace(1) %vecext, <32 x i32> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write8_8Dv8_PU3AS1jDv64_jDv8_j(<8 x ptr addrspace(1)> %data, <64 x i32> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write8_8PU3AS1jDv64_jDv8_j(ptr addrspace(1) %vecext, <64 x i32> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write8_16Dv16_PU3AS1jDv128_jDv16_j(<16 x ptr addrspace(1)> %data, <128 x i32> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write8_16PU3AS1jDv128_jDv16_j(ptr addrspace(1) %vecext, <128 x i32> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write8_32Dv32_PU3AS1jDv256_jDv32_j(<32 x ptr addrspace(1)> %data, <256 x i32> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write8_32PU3AS1jDv256_jDv32_j(ptr addrspace(1) %vecext, <256 x i32> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write8_64Dv64_PU3AS1jDv512_jDv64_j(<64 x ptr addrspace(1)> %data, <512 x i32> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write8_64PU3AS1jDv512_jDv64_j(ptr addrspace(1) %vecext, <512 x i32> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define <4 x i32> @_Z29intel_sub_group_block_read1_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <4 x i32> @_Z29intel_sub_group_block_read1_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <8 x i32> @_Z29intel_sub_group_block_read1_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <8 x i32> @_Z29intel_sub_group_block_read1_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <16 x i32> @_Z30intel_sub_group_block_read1_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <16 x i32> @_Z30intel_sub_group_block_read1_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z30intel_sub_group_block_read1_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z30intel_sub_group_block_read1_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z30intel_sub_group_block_read1_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z30intel_sub_group_block_read1_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <8 x i32> @_Z29intel_sub_group_block_read2_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <8 x i32> @_Z29intel_sub_group_block_read2_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <16 x i32> @_Z29intel_sub_group_block_read2_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <16 x i32> @_Z29intel_sub_group_block_read2_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z30intel_sub_group_block_read2_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z30intel_sub_group_block_read2_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z30intel_sub_group_block_read2_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z30intel_sub_group_block_read2_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <128 x i32> @_Z30intel_sub_group_block_read2_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <128 x i32> @_Z30intel_sub_group_block_read2_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <16 x i32> @_Z29intel_sub_group_block_read4_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <16 x i32> @_Z29intel_sub_group_block_read4_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z29intel_sub_group_block_read4_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z29intel_sub_group_block_read4_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z30intel_sub_group_block_read4_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z30intel_sub_group_block_read4_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <128 x i32> @_Z30intel_sub_group_block_read4_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <128 x i32> @_Z30intel_sub_group_block_read4_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <256 x i32> @_Z30intel_sub_group_block_read4_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <256 x i32> @_Z30intel_sub_group_block_read4_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z29intel_sub_group_block_read8_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z29intel_sub_group_block_read8_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z29intel_sub_group_block_read8_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z29intel_sub_group_block_read8_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <128 x i32> @_Z30intel_sub_group_block_read8_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <128 x i32> @_Z30intel_sub_group_block_read8_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <256 x i32> @_Z30intel_sub_group_block_read8_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <256 x i32> @_Z30intel_sub_group_block_read8_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <512 x i32> @_Z30intel_sub_group_block_read8_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <512 x i32> @_Z30intel_sub_group_block_read8_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <4 x i32> @_Z29intel_sub_group_block_read1_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <4 x i32> @_Z29intel_sub_group_block_read1_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <8 x i32> @_Z29intel_sub_group_block_read1_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <8 x i32> @_Z29intel_sub_group_block_read1_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <16 x i32> @_Z30intel_sub_group_block_read1_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <16 x i32> @_Z30intel_sub_group_block_read1_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z30intel_sub_group_block_read1_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z30intel_sub_group_block_read1_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z30intel_sub_group_block_read1_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z30intel_sub_group_block_read1_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <8 x i32> @_Z29intel_sub_group_block_read2_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <8 x i32> @_Z29intel_sub_group_block_read2_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <16 x i32> @_Z29intel_sub_group_block_read2_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <16 x i32> @_Z29intel_sub_group_block_read2_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z30intel_sub_group_block_read2_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z30intel_sub_group_block_read2_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z30intel_sub_group_block_read2_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z30intel_sub_group_block_read2_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <128 x i32> @_Z30intel_sub_group_block_read2_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <128 x i32> @_Z30intel_sub_group_block_read2_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <16 x i32> @_Z29intel_sub_group_block_read4_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <16 x i32> @_Z29intel_sub_group_block_read4_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z29intel_sub_group_block_read4_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z29intel_sub_group_block_read4_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z30intel_sub_group_block_read4_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z30intel_sub_group_block_read4_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <128 x i32> @_Z30intel_sub_group_block_read4_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <128 x i32> @_Z30intel_sub_group_block_read4_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <256 x i32> @_Z30intel_sub_group_block_read4_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <256 x i32> @_Z30intel_sub_group_block_read4_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z29intel_sub_group_block_read8_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z29intel_sub_group_block_read8_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z29intel_sub_group_block_read8_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z29intel_sub_group_block_read8_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <128 x i32> @_Z30intel_sub_group_block_read8_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <128 x i32> @_Z30intel_sub_group_block_read8_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <256 x i32> @_Z30intel_sub_group_block_read8_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <256 x i32> @_Z30intel_sub_group_block_read8_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <512 x i32> @_Z30intel_sub_group_block_read8_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <512 x i32> @_Z30intel_sub_group_block_read8_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write1_4Dv4_14ocl_image2d_woDv8_iDv4_jS2_(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write1_414ocl_image2d_woDv2_iDv4_jS1_(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write1_8Dv8_14ocl_image2d_woDv16_iDv8_jS2_(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write1_814ocl_image2d_woDv2_iDv8_jS1_(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write1_16Dv16_14ocl_image2d_woDv32_iDv16_jS2_(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write1_1614ocl_image2d_woDv2_iDv16_jS1_(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write1_32Dv32_14ocl_image2d_woDv64_iDv32_jS2_(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write1_3214ocl_image2d_woDv2_iDv32_jS1_(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write1_64Dv64_14ocl_image2d_woDv128_iDv64_jS2_(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write1_6414ocl_image2d_woDv2_iDv64_jS1_(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write2_4Dv4_14ocl_image2d_woDv8_iDv8_jDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <8 x i32> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write2_414ocl_image2d_woDv2_iDv8_jDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write2_8Dv8_14ocl_image2d_woDv16_iDv16_jDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <16 x i32> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write2_814ocl_image2d_woDv2_iDv16_jDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write2_16Dv16_14ocl_image2d_woDv32_iDv32_jDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <32 x i32> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write2_1614ocl_image2d_woDv2_iDv32_jDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write2_32Dv32_14ocl_image2d_woDv64_iDv64_jDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <64 x i32> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write2_3214ocl_image2d_woDv2_iDv64_jDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write2_64Dv64_14ocl_image2d_woDv128_iDv128_jDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <128 x i32> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write2_6414ocl_image2d_woDv2_iDv128_jDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i32> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write4_4Dv4_14ocl_image2d_woDv8_iDv16_jDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <16 x i32> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write4_414ocl_image2d_woDv2_iDv16_jDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write4_8Dv8_14ocl_image2d_woDv16_iDv32_jDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <32 x i32> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write4_814ocl_image2d_woDv2_iDv32_jDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write4_16Dv16_14ocl_image2d_woDv32_iDv64_jDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <64 x i32> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write4_1614ocl_image2d_woDv2_iDv64_jDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write4_32Dv32_14ocl_image2d_woDv64_iDv128_jDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <128 x i32> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write4_3214ocl_image2d_woDv2_iDv128_jDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i32> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write4_64Dv64_14ocl_image2d_woDv128_iDv256_jDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <256 x i32> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write4_6414ocl_image2d_woDv2_iDv256_jDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i32> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write8_4Dv4_14ocl_image2d_woDv8_iDv32_jDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <32 x i32> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write8_414ocl_image2d_woDv2_iDv32_jDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write8_8Dv8_14ocl_image2d_woDv16_iDv64_jDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <64 x i32> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write8_814ocl_image2d_woDv2_iDv64_jDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write8_16Dv16_14ocl_image2d_woDv32_iDv128_jDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <128 x i32> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write8_1614ocl_image2d_woDv2_iDv128_jDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i32> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write8_32Dv32_14ocl_image2d_woDv64_iDv256_jDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <256 x i32> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write8_3214ocl_image2d_woDv2_iDv256_jDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i32> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write8_64Dv64_14ocl_image2d_woDv128_iDv512_jDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <512 x i32> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write8_6414ocl_image2d_woDv2_iDv512_jDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <512 x i32> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write1_4Dv4_14ocl_image2d_rwDv8_iDv4_jS2_(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write1_414ocl_image2d_rwDv2_iDv4_jS1_(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write1_8Dv8_14ocl_image2d_rwDv16_iDv8_jS2_(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write1_814ocl_image2d_rwDv2_iDv8_jS1_(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write1_16Dv16_14ocl_image2d_rwDv32_iDv16_jS2_(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write1_1614ocl_image2d_rwDv2_iDv16_jS1_(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write1_32Dv32_14ocl_image2d_rwDv64_iDv32_jS2_(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write1_3214ocl_image2d_rwDv2_iDv32_jS1_(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write1_64Dv64_14ocl_image2d_rwDv128_iDv64_jS2_(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write1_6414ocl_image2d_rwDv2_iDv64_jS1_(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write2_4Dv4_14ocl_image2d_rwDv8_iDv8_jDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <8 x i32> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write2_414ocl_image2d_rwDv2_iDv8_jDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write2_8Dv8_14ocl_image2d_rwDv16_iDv16_jDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <16 x i32> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write2_814ocl_image2d_rwDv2_iDv16_jDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write2_16Dv16_14ocl_image2d_rwDv32_iDv32_jDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <32 x i32> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write2_1614ocl_image2d_rwDv2_iDv32_jDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write2_32Dv32_14ocl_image2d_rwDv64_iDv64_jDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <64 x i32> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write2_3214ocl_image2d_rwDv2_iDv64_jDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write2_64Dv64_14ocl_image2d_rwDv128_iDv128_jDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <128 x i32> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write2_6414ocl_image2d_rwDv2_iDv128_jDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i32> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write4_4Dv4_14ocl_image2d_rwDv8_iDv16_jDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <16 x i32> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write4_414ocl_image2d_rwDv2_iDv16_jDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write4_8Dv8_14ocl_image2d_rwDv16_iDv32_jDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <32 x i32> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write4_814ocl_image2d_rwDv2_iDv32_jDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write4_16Dv16_14ocl_image2d_rwDv32_iDv64_jDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <64 x i32> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write4_1614ocl_image2d_rwDv2_iDv64_jDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write4_32Dv32_14ocl_image2d_rwDv64_iDv128_jDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <128 x i32> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write4_3214ocl_image2d_rwDv2_iDv128_jDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i32> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write4_64Dv64_14ocl_image2d_rwDv128_iDv256_jDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <256 x i32> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write4_6414ocl_image2d_rwDv2_iDv256_jDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i32> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write8_4Dv4_14ocl_image2d_rwDv8_iDv32_jDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <32 x i32> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write8_414ocl_image2d_rwDv2_iDv32_jDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z30intel_sub_group_block_write8_8Dv8_14ocl_image2d_rwDv16_iDv64_jDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <64 x i32> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z30intel_sub_group_block_write8_814ocl_image2d_rwDv2_iDv64_jDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write8_16Dv16_14ocl_image2d_rwDv32_iDv128_jDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <128 x i32> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write8_1614ocl_image2d_rwDv2_iDv128_jDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i32> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write8_32Dv32_14ocl_image2d_rwDv64_iDv256_jDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <256 x i32> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write8_3214ocl_image2d_rwDv2_iDv256_jDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i32> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z31intel_sub_group_block_write8_64Dv64_14ocl_image2d_rwDv128_iDv512_jDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <512 x i32> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z31intel_sub_group_block_write8_6414ocl_image2d_rwDv2_iDv512_jDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <512 x i32> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define <4 x i8> @_Z32intel_sub_group_block_read_uc1_4Dv4_PU3AS1KhDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call <4 x i8> @_Z32intel_sub_group_block_read_uc1_4PU3AS1KhDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <8 x i8> @_Z32intel_sub_group_block_read_uc1_8Dv8_PU3AS1KhDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <8 x i8> @_Z32intel_sub_group_block_read_uc1_8PU3AS1KhDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <16 x i8> @_Z33intel_sub_group_block_read_uc1_16Dv16_PU3AS1KhDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <16 x i8> @_Z33intel_sub_group_block_read_uc1_16PU3AS1KhDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <32 x i8> @_Z33intel_sub_group_block_read_uc1_32Dv32_PU3AS1KhDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <32 x i8> @_Z33intel_sub_group_block_read_uc1_32PU3AS1KhDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <64 x i8> @_Z33intel_sub_group_block_read_uc1_64Dv64_PU3AS1KhDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <64 x i8> @_Z33intel_sub_group_block_read_uc1_64PU3AS1KhDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <8 x i8> @_Z32intel_sub_group_block_read_uc2_4Dv4_PU3AS1KhDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call <8 x i8> @_Z32intel_sub_group_block_read_uc2_4PU3AS1KhDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <16 x i8> @_Z32intel_sub_group_block_read_uc2_8Dv8_PU3AS1KhDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <16 x i8> @_Z32intel_sub_group_block_read_uc2_8PU3AS1KhDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <32 x i8> @_Z33intel_sub_group_block_read_uc2_16Dv16_PU3AS1KhDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <32 x i8> @_Z33intel_sub_group_block_read_uc2_16PU3AS1KhDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <64 x i8> @_Z33intel_sub_group_block_read_uc2_32Dv32_PU3AS1KhDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <64 x i8> @_Z33intel_sub_group_block_read_uc2_32PU3AS1KhDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <128 x i8> @_Z33intel_sub_group_block_read_uc2_64Dv64_PU3AS1KhDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <128 x i8> @_Z33intel_sub_group_block_read_uc2_64PU3AS1KhDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <16 x i8> @_Z32intel_sub_group_block_read_uc4_4Dv4_PU3AS1KhDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call <16 x i8> @_Z32intel_sub_group_block_read_uc4_4PU3AS1KhDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <32 x i8> @_Z32intel_sub_group_block_read_uc4_8Dv8_PU3AS1KhDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <32 x i8> @_Z32intel_sub_group_block_read_uc4_8PU3AS1KhDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <64 x i8> @_Z33intel_sub_group_block_read_uc4_16Dv16_PU3AS1KhDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <64 x i8> @_Z33intel_sub_group_block_read_uc4_16PU3AS1KhDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <128 x i8> @_Z33intel_sub_group_block_read_uc4_32Dv32_PU3AS1KhDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <128 x i8> @_Z33intel_sub_group_block_read_uc4_32PU3AS1KhDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <256 x i8> @_Z33intel_sub_group_block_read_uc4_64Dv64_PU3AS1KhDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <256 x i8> @_Z33intel_sub_group_block_read_uc4_64PU3AS1KhDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <32 x i8> @_Z32intel_sub_group_block_read_uc8_4Dv4_PU3AS1KhDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call <32 x i8> @_Z32intel_sub_group_block_read_uc8_4PU3AS1KhDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <64 x i8> @_Z32intel_sub_group_block_read_uc8_8Dv8_PU3AS1KhDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <64 x i8> @_Z32intel_sub_group_block_read_uc8_8PU3AS1KhDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <128 x i8> @_Z33intel_sub_group_block_read_uc8_16Dv16_PU3AS1KhDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <128 x i8> @_Z33intel_sub_group_block_read_uc8_16PU3AS1KhDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <256 x i8> @_Z33intel_sub_group_block_read_uc8_32Dv32_PU3AS1KhDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <256 x i8> @_Z33intel_sub_group_block_read_uc8_32PU3AS1KhDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <512 x i8> @_Z33intel_sub_group_block_read_uc8_64Dv64_PU3AS1KhDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <512 x i8> @_Z33intel_sub_group_block_read_uc8_64PU3AS1KhDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <64 x i8> @_Z33intel_sub_group_block_read_uc16_4Dv4_PU3AS1KhDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call <64 x i8> @_Z33intel_sub_group_block_read_uc16_4PU3AS1KhDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <128 x i8> @_Z33intel_sub_group_block_read_uc16_8Dv8_PU3AS1KhDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <128 x i8> @_Z33intel_sub_group_block_read_uc16_8PU3AS1KhDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <256 x i8> @_Z34intel_sub_group_block_read_uc16_16Dv16_PU3AS1KhDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <256 x i8> @_Z34intel_sub_group_block_read_uc16_16PU3AS1KhDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <512 x i8> @_Z34intel_sub_group_block_read_uc16_32Dv32_PU3AS1KhDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <512 x i8> @_Z34intel_sub_group_block_read_uc16_32PU3AS1KhDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <1024 x i8> @_Z34intel_sub_group_block_read_uc16_64Dv64_PU3AS1KhDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <1024 x i8> @_Z34intel_sub_group_block_read_uc16_64PU3AS1KhDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc1_4Dv4_PU3AS1hDv4_hDv4_j(<4 x ptr addrspace(1)> %data, <4 x i8> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc1_4PU3AS1hDv4_hDv4_j(ptr addrspace(1) %vecext, <4 x i8> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc1_8Dv8_PU3AS1hDv8_hDv8_j(<8 x ptr addrspace(1)> %data, <8 x i8> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc1_8PU3AS1hDv8_hDv8_j(ptr addrspace(1) %vecext, <8 x i8> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc1_16Dv16_PU3AS1hDv16_hDv16_j(<16 x ptr addrspace(1)> %data, <16 x i8> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc1_16PU3AS1hDv16_hDv16_j(ptr addrspace(1) %vecext, <16 x i8> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc1_32Dv32_PU3AS1hDv32_hDv32_j(<32 x ptr addrspace(1)> %data, <32 x i8> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc1_32PU3AS1hDv32_hDv32_j(ptr addrspace(1) %vecext, <32 x i8> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc1_64Dv64_PU3AS1hDv64_hDv64_j(<64 x ptr addrspace(1)> %data, <64 x i8> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc1_64PU3AS1hDv64_hDv64_j(ptr addrspace(1) %vecext, <64 x i8> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc2_4Dv4_PU3AS1hDv8_hDv4_j(<4 x ptr addrspace(1)> %data, <8 x i8> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc2_4PU3AS1hDv8_hDv4_j(ptr addrspace(1) %vecext, <8 x i8> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc2_8Dv8_PU3AS1hDv16_hDv8_j(<8 x ptr addrspace(1)> %data, <16 x i8> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc2_8PU3AS1hDv16_hDv8_j(ptr addrspace(1) %vecext, <16 x i8> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc2_16Dv16_PU3AS1hDv32_hDv16_j(<16 x ptr addrspace(1)> %data, <32 x i8> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc2_16PU3AS1hDv32_hDv16_j(ptr addrspace(1) %vecext, <32 x i8> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc2_32Dv32_PU3AS1hDv64_hDv32_j(<32 x ptr addrspace(1)> %data, <64 x i8> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc2_32PU3AS1hDv64_hDv32_j(ptr addrspace(1) %vecext, <64 x i8> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc2_64Dv64_PU3AS1hDv128_hDv64_j(<64 x ptr addrspace(1)> %data, <128 x i8> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc2_64PU3AS1hDv128_hDv64_j(ptr addrspace(1) %vecext, <128 x i8> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc4_4Dv4_PU3AS1hDv16_hDv4_j(<4 x ptr addrspace(1)> %data, <16 x i8> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc4_4PU3AS1hDv16_hDv4_j(ptr addrspace(1) %vecext, <16 x i8> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc4_8Dv8_PU3AS1hDv32_hDv8_j(<8 x ptr addrspace(1)> %data, <32 x i8> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc4_8PU3AS1hDv32_hDv8_j(ptr addrspace(1) %vecext, <32 x i8> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc4_16Dv16_PU3AS1hDv64_hDv16_j(<16 x ptr addrspace(1)> %data, <64 x i8> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc4_16PU3AS1hDv64_hDv16_j(ptr addrspace(1) %vecext, <64 x i8> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc4_32Dv32_PU3AS1hDv128_hDv32_j(<32 x ptr addrspace(1)> %data, <128 x i8> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc4_32PU3AS1hDv128_hDv32_j(ptr addrspace(1) %vecext, <128 x i8> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc4_64Dv64_PU3AS1hDv256_hDv64_j(<64 x ptr addrspace(1)> %data, <256 x i8> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc4_64PU3AS1hDv256_hDv64_j(ptr addrspace(1) %vecext, <256 x i8> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc8_4Dv4_PU3AS1hDv32_hDv4_j(<4 x ptr addrspace(1)> %data, <32 x i8> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc8_4PU3AS1hDv32_hDv4_j(ptr addrspace(1) %vecext, <32 x i8> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc8_8Dv8_PU3AS1hDv64_hDv8_j(<8 x ptr addrspace(1)> %data, <64 x i8> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc8_8PU3AS1hDv64_hDv8_j(ptr addrspace(1) %vecext, <64 x i8> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc8_16Dv16_PU3AS1hDv128_hDv16_j(<16 x ptr addrspace(1)> %data, <128 x i8> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc8_16PU3AS1hDv128_hDv16_j(ptr addrspace(1) %vecext, <128 x i8> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc8_32Dv32_PU3AS1hDv256_hDv32_j(<32 x ptr addrspace(1)> %data, <256 x i8> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc8_32PU3AS1hDv256_hDv32_j(ptr addrspace(1) %vecext, <256 x i8> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc8_64Dv64_PU3AS1hDv512_hDv64_j(<64 x ptr addrspace(1)> %data, <512 x i8> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc8_64PU3AS1hDv512_hDv64_j(ptr addrspace(1) %vecext, <512 x i8> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc16_4Dv4_PU3AS1hDv64_hDv4_j(<4 x ptr addrspace(1)> %data, <64 x i8> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc16_4PU3AS1hDv64_hDv4_j(ptr addrspace(1) %vecext, <64 x i8> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc16_8Dv8_PU3AS1hDv128_hDv8_j(<8 x ptr addrspace(1)> %data, <128 x i8> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc16_8PU3AS1hDv128_hDv8_j(ptr addrspace(1) %vecext, <128 x i8> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z35intel_sub_group_block_write_uc16_16Dv16_PU3AS1hDv256_hDv16_j(<16 x ptr addrspace(1)> %data, <256 x i8> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z35intel_sub_group_block_write_uc16_16PU3AS1hDv256_hDv16_j(ptr addrspace(1) %vecext, <256 x i8> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z35intel_sub_group_block_write_uc16_32Dv32_PU3AS1hDv512_hDv32_j(<32 x ptr addrspace(1)> %data, <512 x i8> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z35intel_sub_group_block_write_uc16_32PU3AS1hDv512_hDv32_j(ptr addrspace(1) %vecext, <512 x i8> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z35intel_sub_group_block_write_uc16_64Dv64_PU3AS1hDv1024_hDv64_j(<64 x ptr addrspace(1)> %data, <1024 x i8> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z35intel_sub_group_block_write_uc16_64PU3AS1hDv1024_hDv64_j(ptr addrspace(1) %vecext, <1024 x i8> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define <4 x i8> @_Z32intel_sub_group_block_read_uc1_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <4 x i8> @_Z32intel_sub_group_block_read_uc1_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <8 x i8> @_Z32intel_sub_group_block_read_uc1_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <8 x i8> @_Z32intel_sub_group_block_read_uc1_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <16 x i8> @_Z33intel_sub_group_block_read_uc1_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <16 x i8> @_Z33intel_sub_group_block_read_uc1_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <32 x i8> @_Z33intel_sub_group_block_read_uc1_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <32 x i8> @_Z33intel_sub_group_block_read_uc1_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <64 x i8> @_Z33intel_sub_group_block_read_uc1_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <64 x i8> @_Z33intel_sub_group_block_read_uc1_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <8 x i8> @_Z32intel_sub_group_block_read_uc2_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <8 x i8> @_Z32intel_sub_group_block_read_uc2_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <16 x i8> @_Z32intel_sub_group_block_read_uc2_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <16 x i8> @_Z32intel_sub_group_block_read_uc2_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <32 x i8> @_Z33intel_sub_group_block_read_uc2_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <32 x i8> @_Z33intel_sub_group_block_read_uc2_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <64 x i8> @_Z33intel_sub_group_block_read_uc2_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <64 x i8> @_Z33intel_sub_group_block_read_uc2_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <128 x i8> @_Z33intel_sub_group_block_read_uc2_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <128 x i8> @_Z33intel_sub_group_block_read_uc2_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <16 x i8> @_Z32intel_sub_group_block_read_uc4_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <16 x i8> @_Z32intel_sub_group_block_read_uc4_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <32 x i8> @_Z32intel_sub_group_block_read_uc4_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <32 x i8> @_Z32intel_sub_group_block_read_uc4_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <64 x i8> @_Z33intel_sub_group_block_read_uc4_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <64 x i8> @_Z33intel_sub_group_block_read_uc4_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <128 x i8> @_Z33intel_sub_group_block_read_uc4_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <128 x i8> @_Z33intel_sub_group_block_read_uc4_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <256 x i8> @_Z33intel_sub_group_block_read_uc4_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <256 x i8> @_Z33intel_sub_group_block_read_uc4_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <32 x i8> @_Z32intel_sub_group_block_read_uc8_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <32 x i8> @_Z32intel_sub_group_block_read_uc8_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <64 x i8> @_Z32intel_sub_group_block_read_uc8_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <64 x i8> @_Z32intel_sub_group_block_read_uc8_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <128 x i8> @_Z33intel_sub_group_block_read_uc8_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <128 x i8> @_Z33intel_sub_group_block_read_uc8_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <256 x i8> @_Z33intel_sub_group_block_read_uc8_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <256 x i8> @_Z33intel_sub_group_block_read_uc8_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <512 x i8> @_Z33intel_sub_group_block_read_uc8_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <512 x i8> @_Z33intel_sub_group_block_read_uc8_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <64 x i8> @_Z33intel_sub_group_block_read_uc16_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <64 x i8> @_Z33intel_sub_group_block_read_uc16_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <128 x i8> @_Z33intel_sub_group_block_read_uc16_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <128 x i8> @_Z33intel_sub_group_block_read_uc16_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <256 x i8> @_Z34intel_sub_group_block_read_uc16_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <256 x i8> @_Z34intel_sub_group_block_read_uc16_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <512 x i8> @_Z34intel_sub_group_block_read_uc16_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <512 x i8> @_Z34intel_sub_group_block_read_uc16_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <1024 x i8> @_Z34intel_sub_group_block_read_uc16_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <1024 x i8> @_Z34intel_sub_group_block_read_uc16_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <4 x i8> @_Z32intel_sub_group_block_read_uc1_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <4 x i8> @_Z32intel_sub_group_block_read_uc1_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <8 x i8> @_Z32intel_sub_group_block_read_uc1_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <8 x i8> @_Z32intel_sub_group_block_read_uc1_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <16 x i8> @_Z33intel_sub_group_block_read_uc1_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <16 x i8> @_Z33intel_sub_group_block_read_uc1_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <32 x i8> @_Z33intel_sub_group_block_read_uc1_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <32 x i8> @_Z33intel_sub_group_block_read_uc1_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <64 x i8> @_Z33intel_sub_group_block_read_uc1_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <64 x i8> @_Z33intel_sub_group_block_read_uc1_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <8 x i8> @_Z32intel_sub_group_block_read_uc2_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <8 x i8> @_Z32intel_sub_group_block_read_uc2_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <16 x i8> @_Z32intel_sub_group_block_read_uc2_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <16 x i8> @_Z32intel_sub_group_block_read_uc2_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <32 x i8> @_Z33intel_sub_group_block_read_uc2_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <32 x i8> @_Z33intel_sub_group_block_read_uc2_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <64 x i8> @_Z33intel_sub_group_block_read_uc2_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <64 x i8> @_Z33intel_sub_group_block_read_uc2_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <128 x i8> @_Z33intel_sub_group_block_read_uc2_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <128 x i8> @_Z33intel_sub_group_block_read_uc2_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <16 x i8> @_Z32intel_sub_group_block_read_uc4_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <16 x i8> @_Z32intel_sub_group_block_read_uc4_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <32 x i8> @_Z32intel_sub_group_block_read_uc4_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <32 x i8> @_Z32intel_sub_group_block_read_uc4_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <64 x i8> @_Z33intel_sub_group_block_read_uc4_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <64 x i8> @_Z33intel_sub_group_block_read_uc4_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <128 x i8> @_Z33intel_sub_group_block_read_uc4_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <128 x i8> @_Z33intel_sub_group_block_read_uc4_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <256 x i8> @_Z33intel_sub_group_block_read_uc4_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <256 x i8> @_Z33intel_sub_group_block_read_uc4_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <32 x i8> @_Z32intel_sub_group_block_read_uc8_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <32 x i8> @_Z32intel_sub_group_block_read_uc8_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <64 x i8> @_Z32intel_sub_group_block_read_uc8_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <64 x i8> @_Z32intel_sub_group_block_read_uc8_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <128 x i8> @_Z33intel_sub_group_block_read_uc8_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <128 x i8> @_Z33intel_sub_group_block_read_uc8_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <256 x i8> @_Z33intel_sub_group_block_read_uc8_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <256 x i8> @_Z33intel_sub_group_block_read_uc8_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <512 x i8> @_Z33intel_sub_group_block_read_uc8_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <512 x i8> @_Z33intel_sub_group_block_read_uc8_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <64 x i8> @_Z33intel_sub_group_block_read_uc16_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <64 x i8> @_Z33intel_sub_group_block_read_uc16_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <128 x i8> @_Z33intel_sub_group_block_read_uc16_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <128 x i8> @_Z33intel_sub_group_block_read_uc16_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <256 x i8> @_Z34intel_sub_group_block_read_uc16_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <256 x i8> @_Z34intel_sub_group_block_read_uc16_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <512 x i8> @_Z34intel_sub_group_block_read_uc16_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <512 x i8> @_Z34intel_sub_group_block_read_uc16_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <1024 x i8> @_Z34intel_sub_group_block_read_uc16_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <1024 x i8> @_Z34intel_sub_group_block_read_uc16_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc1_4Dv4_14ocl_image2d_woDv8_iDv4_hDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i8> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc1_414ocl_image2d_woDv2_iDv4_hDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i8> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc1_8Dv8_14ocl_image2d_woDv16_iDv8_hDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i8> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc1_814ocl_image2d_woDv2_iDv8_hDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i8> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc1_16Dv16_14ocl_image2d_woDv32_iDv16_hDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i8> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc1_1614ocl_image2d_woDv2_iDv16_hDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i8> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc1_32Dv32_14ocl_image2d_woDv64_iDv32_hDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i8> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc1_3214ocl_image2d_woDv2_iDv32_hDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i8> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc1_64Dv64_14ocl_image2d_woDv128_iDv64_hDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i8> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc1_6414ocl_image2d_woDv2_iDv64_hDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i8> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc2_4Dv4_14ocl_image2d_woDv8_iDv8_hDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <8 x i8> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc2_414ocl_image2d_woDv2_iDv8_hDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i8> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc2_8Dv8_14ocl_image2d_woDv16_iDv16_hDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <16 x i8> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc2_814ocl_image2d_woDv2_iDv16_hDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i8> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc2_16Dv16_14ocl_image2d_woDv32_iDv32_hDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <32 x i8> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc2_1614ocl_image2d_woDv2_iDv32_hDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i8> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc2_32Dv32_14ocl_image2d_woDv64_iDv64_hDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <64 x i8> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc2_3214ocl_image2d_woDv2_iDv64_hDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i8> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc2_64Dv64_14ocl_image2d_woDv128_iDv128_hDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <128 x i8> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc2_6414ocl_image2d_woDv2_iDv128_hDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i8> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc4_4Dv4_14ocl_image2d_woDv8_iDv16_hDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <16 x i8> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc4_414ocl_image2d_woDv2_iDv16_hDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i8> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc4_8Dv8_14ocl_image2d_woDv16_iDv32_hDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <32 x i8> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc4_814ocl_image2d_woDv2_iDv32_hDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i8> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc4_16Dv16_14ocl_image2d_woDv32_iDv64_hDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <64 x i8> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc4_1614ocl_image2d_woDv2_iDv64_hDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i8> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc4_32Dv32_14ocl_image2d_woDv64_iDv128_hDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <128 x i8> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc4_3214ocl_image2d_woDv2_iDv128_hDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i8> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc4_64Dv64_14ocl_image2d_woDv128_iDv256_hDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <256 x i8> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc4_6414ocl_image2d_woDv2_iDv256_hDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i8> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc8_4Dv4_14ocl_image2d_woDv8_iDv32_hDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <32 x i8> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc8_414ocl_image2d_woDv2_iDv32_hDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i8> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc8_8Dv8_14ocl_image2d_woDv16_iDv64_hDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <64 x i8> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc8_814ocl_image2d_woDv2_iDv64_hDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i8> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc8_16Dv16_14ocl_image2d_woDv32_iDv128_hDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <128 x i8> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc8_1614ocl_image2d_woDv2_iDv128_hDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i8> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc8_32Dv32_14ocl_image2d_woDv64_iDv256_hDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <256 x i8> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc8_3214ocl_image2d_woDv2_iDv256_hDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i8> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc8_64Dv64_14ocl_image2d_woDv128_iDv512_hDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <512 x i8> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc8_6414ocl_image2d_woDv2_iDv512_hDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <512 x i8> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc16_4Dv4_14ocl_image2d_woDv8_iDv64_hDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <64 x i8> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc16_414ocl_image2d_woDv2_iDv64_hDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i8> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc16_8Dv8_14ocl_image2d_woDv16_iDv128_hDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <128 x i8> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc16_814ocl_image2d_woDv2_iDv128_hDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i8> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z35intel_sub_group_block_write_uc16_16Dv16_14ocl_image2d_woDv32_iDv256_hDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <256 x i8> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z35intel_sub_group_block_write_uc16_1614ocl_image2d_woDv2_iDv256_hDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i8> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z35intel_sub_group_block_write_uc16_32Dv32_14ocl_image2d_woDv64_iDv512_hDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <512 x i8> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z35intel_sub_group_block_write_uc16_3214ocl_image2d_woDv2_iDv512_hDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <512 x i8> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z35intel_sub_group_block_write_uc16_64Dv64_14ocl_image2d_woDv128_iDv1024_hDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <1024 x i8> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z35intel_sub_group_block_write_uc16_6414ocl_image2d_woDv2_iDv1024_hDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <1024 x i8> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc1_4Dv4_14ocl_image2d_rwDv8_iDv4_hDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i8> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc1_414ocl_image2d_rwDv2_iDv4_hDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i8> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc1_8Dv8_14ocl_image2d_rwDv16_iDv8_hDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i8> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc1_814ocl_image2d_rwDv2_iDv8_hDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i8> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc1_16Dv16_14ocl_image2d_rwDv32_iDv16_hDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i8> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc1_1614ocl_image2d_rwDv2_iDv16_hDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i8> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc1_32Dv32_14ocl_image2d_rwDv64_iDv32_hDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i8> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc1_3214ocl_image2d_rwDv2_iDv32_hDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i8> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc1_64Dv64_14ocl_image2d_rwDv128_iDv64_hDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i8> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc1_6414ocl_image2d_rwDv2_iDv64_hDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i8> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc2_4Dv4_14ocl_image2d_rwDv8_iDv8_hDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <8 x i8> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc2_414ocl_image2d_rwDv2_iDv8_hDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i8> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc2_8Dv8_14ocl_image2d_rwDv16_iDv16_hDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <16 x i8> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc2_814ocl_image2d_rwDv2_iDv16_hDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i8> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc2_16Dv16_14ocl_image2d_rwDv32_iDv32_hDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <32 x i8> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc2_1614ocl_image2d_rwDv2_iDv32_hDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i8> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc2_32Dv32_14ocl_image2d_rwDv64_iDv64_hDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <64 x i8> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc2_3214ocl_image2d_rwDv2_iDv64_hDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i8> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc2_64Dv64_14ocl_image2d_rwDv128_iDv128_hDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <128 x i8> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc2_6414ocl_image2d_rwDv2_iDv128_hDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i8> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc4_4Dv4_14ocl_image2d_rwDv8_iDv16_hDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <16 x i8> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc4_414ocl_image2d_rwDv2_iDv16_hDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i8> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc4_8Dv8_14ocl_image2d_rwDv16_iDv32_hDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <32 x i8> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc4_814ocl_image2d_rwDv2_iDv32_hDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i8> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc4_16Dv16_14ocl_image2d_rwDv32_iDv64_hDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <64 x i8> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc4_1614ocl_image2d_rwDv2_iDv64_hDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i8> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc4_32Dv32_14ocl_image2d_rwDv64_iDv128_hDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <128 x i8> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc4_3214ocl_image2d_rwDv2_iDv128_hDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i8> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc4_64Dv64_14ocl_image2d_rwDv128_iDv256_hDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <256 x i8> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc4_6414ocl_image2d_rwDv2_iDv256_hDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i8> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc8_4Dv4_14ocl_image2d_rwDv8_iDv32_hDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <32 x i8> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc8_414ocl_image2d_rwDv2_iDv32_hDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i8> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_uc8_8Dv8_14ocl_image2d_rwDv16_iDv64_hDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <64 x i8> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_uc8_814ocl_image2d_rwDv2_iDv64_hDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i8> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc8_16Dv16_14ocl_image2d_rwDv32_iDv128_hDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <128 x i8> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc8_1614ocl_image2d_rwDv2_iDv128_hDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i8> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc8_32Dv32_14ocl_image2d_rwDv64_iDv256_hDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <256 x i8> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc8_3214ocl_image2d_rwDv2_iDv256_hDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i8> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc8_64Dv64_14ocl_image2d_rwDv128_iDv512_hDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <512 x i8> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc8_6414ocl_image2d_rwDv2_iDv512_hDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <512 x i8> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc16_4Dv4_14ocl_image2d_rwDv8_iDv64_hDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <64 x i8> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc16_414ocl_image2d_rwDv2_iDv64_hDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i8> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_uc16_8Dv8_14ocl_image2d_rwDv16_iDv128_hDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <128 x i8> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_uc16_814ocl_image2d_rwDv2_iDv128_hDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i8> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z35intel_sub_group_block_write_uc16_16Dv16_14ocl_image2d_rwDv32_iDv256_hDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <256 x i8> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z35intel_sub_group_block_write_uc16_1614ocl_image2d_rwDv2_iDv256_hDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i8> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z35intel_sub_group_block_write_uc16_32Dv32_14ocl_image2d_rwDv64_iDv512_hDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <512 x i8> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z35intel_sub_group_block_write_uc16_3214ocl_image2d_rwDv2_iDv512_hDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <512 x i8> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z35intel_sub_group_block_write_uc16_64Dv64_14ocl_image2d_rwDv128_iDv1024_hDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <1024 x i8> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z35intel_sub_group_block_write_uc16_6414ocl_image2d_rwDv2_iDv1024_hDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <1024 x i8> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define <4 x i16> @_Z32intel_sub_group_block_read_us1_4Dv4_PU3AS1KtDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call <4 x i16> @_Z32intel_sub_group_block_read_us1_4PU3AS1KtDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <8 x i16> @_Z32intel_sub_group_block_read_us1_8Dv8_PU3AS1KtDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <8 x i16> @_Z32intel_sub_group_block_read_us1_8PU3AS1KtDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <16 x i16> @_Z33intel_sub_group_block_read_us1_16Dv16_PU3AS1KtDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <16 x i16> @_Z33intel_sub_group_block_read_us1_16PU3AS1KtDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <32 x i16> @_Z33intel_sub_group_block_read_us1_32Dv32_PU3AS1KtDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <32 x i16> @_Z33intel_sub_group_block_read_us1_32PU3AS1KtDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <64 x i16> @_Z33intel_sub_group_block_read_us1_64Dv64_PU3AS1KtDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <64 x i16> @_Z33intel_sub_group_block_read_us1_64PU3AS1KtDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <8 x i16> @_Z32intel_sub_group_block_read_us2_4Dv4_PU3AS1KtDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call <8 x i16> @_Z32intel_sub_group_block_read_us2_4PU3AS1KtDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <16 x i16> @_Z32intel_sub_group_block_read_us2_8Dv8_PU3AS1KtDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <16 x i16> @_Z32intel_sub_group_block_read_us2_8PU3AS1KtDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <32 x i16> @_Z33intel_sub_group_block_read_us2_16Dv16_PU3AS1KtDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <32 x i16> @_Z33intel_sub_group_block_read_us2_16PU3AS1KtDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <64 x i16> @_Z33intel_sub_group_block_read_us2_32Dv32_PU3AS1KtDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <64 x i16> @_Z33intel_sub_group_block_read_us2_32PU3AS1KtDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <128 x i16> @_Z33intel_sub_group_block_read_us2_64Dv64_PU3AS1KtDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <128 x i16> @_Z33intel_sub_group_block_read_us2_64PU3AS1KtDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <16 x i16> @_Z32intel_sub_group_block_read_us4_4Dv4_PU3AS1KtDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call <16 x i16> @_Z32intel_sub_group_block_read_us4_4PU3AS1KtDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <32 x i16> @_Z32intel_sub_group_block_read_us4_8Dv8_PU3AS1KtDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <32 x i16> @_Z32intel_sub_group_block_read_us4_8PU3AS1KtDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <64 x i16> @_Z33intel_sub_group_block_read_us4_16Dv16_PU3AS1KtDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <64 x i16> @_Z33intel_sub_group_block_read_us4_16PU3AS1KtDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <128 x i16> @_Z33intel_sub_group_block_read_us4_32Dv32_PU3AS1KtDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <128 x i16> @_Z33intel_sub_group_block_read_us4_32PU3AS1KtDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <256 x i16> @_Z33intel_sub_group_block_read_us4_64Dv64_PU3AS1KtDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <256 x i16> @_Z33intel_sub_group_block_read_us4_64PU3AS1KtDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <32 x i16> @_Z32intel_sub_group_block_read_us8_4Dv4_PU3AS1KtDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call <32 x i16> @_Z32intel_sub_group_block_read_us8_4PU3AS1KtDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <64 x i16> @_Z32intel_sub_group_block_read_us8_8Dv8_PU3AS1KtDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <64 x i16> @_Z32intel_sub_group_block_read_us8_8PU3AS1KtDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <128 x i16> @_Z33intel_sub_group_block_read_us8_16Dv16_PU3AS1KtDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <128 x i16> @_Z33intel_sub_group_block_read_us8_16PU3AS1KtDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <256 x i16> @_Z33intel_sub_group_block_read_us8_32Dv32_PU3AS1KtDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <256 x i16> @_Z33intel_sub_group_block_read_us8_32PU3AS1KtDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <512 x i16> @_Z33intel_sub_group_block_read_us8_64Dv64_PU3AS1KtDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <512 x i16> @_Z33intel_sub_group_block_read_us8_64PU3AS1KtDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us1_4Dv4_PU3AS1tDv4_tDv4_j(<4 x ptr addrspace(1)> %data, <4 x i16> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us1_4PU3AS1tDv4_tDv4_j(ptr addrspace(1) %vecext, <4 x i16> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us1_8Dv8_PU3AS1tDv8_tDv8_j(<8 x ptr addrspace(1)> %data, <8 x i16> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us1_8PU3AS1tDv8_tDv8_j(ptr addrspace(1) %vecext, <8 x i16> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us1_16Dv16_PU3AS1tDv16_tDv16_j(<16 x ptr addrspace(1)> %data, <16 x i16> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us1_16PU3AS1tDv16_tDv16_j(ptr addrspace(1) %vecext, <16 x i16> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us1_32Dv32_PU3AS1tDv32_tDv32_j(<32 x ptr addrspace(1)> %data, <32 x i16> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us1_32PU3AS1tDv32_tDv32_j(ptr addrspace(1) %vecext, <32 x i16> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us1_64Dv64_PU3AS1tDv64_tDv64_j(<64 x ptr addrspace(1)> %data, <64 x i16> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us1_64PU3AS1tDv64_tDv64_j(ptr addrspace(1) %vecext, <64 x i16> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us2_4Dv4_PU3AS1tDv8_tDv4_j(<4 x ptr addrspace(1)> %data, <8 x i16> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us2_4PU3AS1tDv8_tDv4_j(ptr addrspace(1) %vecext, <8 x i16> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us2_8Dv8_PU3AS1tDv16_tDv8_j(<8 x ptr addrspace(1)> %data, <16 x i16> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us2_8PU3AS1tDv16_tDv8_j(ptr addrspace(1) %vecext, <16 x i16> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us2_16Dv16_PU3AS1tDv32_tDv16_j(<16 x ptr addrspace(1)> %data, <32 x i16> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us2_16PU3AS1tDv32_tDv16_j(ptr addrspace(1) %vecext, <32 x i16> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us2_32Dv32_PU3AS1tDv64_tDv32_j(<32 x ptr addrspace(1)> %data, <64 x i16> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us2_32PU3AS1tDv64_tDv32_j(ptr addrspace(1) %vecext, <64 x i16> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us2_64Dv64_PU3AS1tDv128_tDv64_j(<64 x ptr addrspace(1)> %data, <128 x i16> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us2_64PU3AS1tDv128_tDv64_j(ptr addrspace(1) %vecext, <128 x i16> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us4_4Dv4_PU3AS1tDv16_tDv4_j(<4 x ptr addrspace(1)> %data, <16 x i16> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us4_4PU3AS1tDv16_tDv4_j(ptr addrspace(1) %vecext, <16 x i16> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us4_8Dv8_PU3AS1tDv32_tDv8_j(<8 x ptr addrspace(1)> %data, <32 x i16> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us4_8PU3AS1tDv32_tDv8_j(ptr addrspace(1) %vecext, <32 x i16> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us4_16Dv16_PU3AS1tDv64_tDv16_j(<16 x ptr addrspace(1)> %data, <64 x i16> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us4_16PU3AS1tDv64_tDv16_j(ptr addrspace(1) %vecext, <64 x i16> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us4_32Dv32_PU3AS1tDv128_tDv32_j(<32 x ptr addrspace(1)> %data, <128 x i16> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us4_32PU3AS1tDv128_tDv32_j(ptr addrspace(1) %vecext, <128 x i16> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us4_64Dv64_PU3AS1tDv256_tDv64_j(<64 x ptr addrspace(1)> %data, <256 x i16> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us4_64PU3AS1tDv256_tDv64_j(ptr addrspace(1) %vecext, <256 x i16> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us8_4Dv4_PU3AS1tDv32_tDv4_j(<4 x ptr addrspace(1)> %data, <32 x i16> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us8_4PU3AS1tDv32_tDv4_j(ptr addrspace(1) %vecext, <32 x i16> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us8_8Dv8_PU3AS1tDv64_tDv8_j(<8 x ptr addrspace(1)> %data, <64 x i16> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us8_8PU3AS1tDv64_tDv8_j(ptr addrspace(1) %vecext, <64 x i16> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us8_16Dv16_PU3AS1tDv128_tDv16_j(<16 x ptr addrspace(1)> %data, <128 x i16> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us8_16PU3AS1tDv128_tDv16_j(ptr addrspace(1) %vecext, <128 x i16> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us8_32Dv32_PU3AS1tDv256_tDv32_j(<32 x ptr addrspace(1)> %data, <256 x i16> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us8_32PU3AS1tDv256_tDv32_j(ptr addrspace(1) %vecext, <256 x i16> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us8_64Dv64_PU3AS1tDv512_tDv64_j(<64 x ptr addrspace(1)> %data, <512 x i16> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us8_64PU3AS1tDv512_tDv64_j(ptr addrspace(1) %vecext, <512 x i16> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define <4 x i16> @_Z32intel_sub_group_block_read_us1_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <4 x i16> @_Z32intel_sub_group_block_read_us1_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <8 x i16> @_Z32intel_sub_group_block_read_us1_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <8 x i16> @_Z32intel_sub_group_block_read_us1_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <16 x i16> @_Z33intel_sub_group_block_read_us1_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <16 x i16> @_Z33intel_sub_group_block_read_us1_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <32 x i16> @_Z33intel_sub_group_block_read_us1_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <32 x i16> @_Z33intel_sub_group_block_read_us1_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <64 x i16> @_Z33intel_sub_group_block_read_us1_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <64 x i16> @_Z33intel_sub_group_block_read_us1_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <8 x i16> @_Z32intel_sub_group_block_read_us2_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <8 x i16> @_Z32intel_sub_group_block_read_us2_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <16 x i16> @_Z32intel_sub_group_block_read_us2_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <16 x i16> @_Z32intel_sub_group_block_read_us2_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <32 x i16> @_Z33intel_sub_group_block_read_us2_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <32 x i16> @_Z33intel_sub_group_block_read_us2_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <64 x i16> @_Z33intel_sub_group_block_read_us2_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <64 x i16> @_Z33intel_sub_group_block_read_us2_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <128 x i16> @_Z33intel_sub_group_block_read_us2_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <128 x i16> @_Z33intel_sub_group_block_read_us2_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <16 x i16> @_Z32intel_sub_group_block_read_us4_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <16 x i16> @_Z32intel_sub_group_block_read_us4_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <32 x i16> @_Z32intel_sub_group_block_read_us4_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <32 x i16> @_Z32intel_sub_group_block_read_us4_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <64 x i16> @_Z33intel_sub_group_block_read_us4_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <64 x i16> @_Z33intel_sub_group_block_read_us4_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <128 x i16> @_Z33intel_sub_group_block_read_us4_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <128 x i16> @_Z33intel_sub_group_block_read_us4_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <256 x i16> @_Z33intel_sub_group_block_read_us4_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <256 x i16> @_Z33intel_sub_group_block_read_us4_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <32 x i16> @_Z32intel_sub_group_block_read_us8_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <32 x i16> @_Z32intel_sub_group_block_read_us8_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <64 x i16> @_Z32intel_sub_group_block_read_us8_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <64 x i16> @_Z32intel_sub_group_block_read_us8_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <128 x i16> @_Z33intel_sub_group_block_read_us8_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <128 x i16> @_Z33intel_sub_group_block_read_us8_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <256 x i16> @_Z33intel_sub_group_block_read_us8_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <256 x i16> @_Z33intel_sub_group_block_read_us8_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <512 x i16> @_Z33intel_sub_group_block_read_us8_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <512 x i16> @_Z33intel_sub_group_block_read_us8_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <4 x i16> @_Z32intel_sub_group_block_read_us1_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <4 x i16> @_Z32intel_sub_group_block_read_us1_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <8 x i16> @_Z32intel_sub_group_block_read_us1_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <8 x i16> @_Z32intel_sub_group_block_read_us1_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <16 x i16> @_Z33intel_sub_group_block_read_us1_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <16 x i16> @_Z33intel_sub_group_block_read_us1_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <32 x i16> @_Z33intel_sub_group_block_read_us1_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <32 x i16> @_Z33intel_sub_group_block_read_us1_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <64 x i16> @_Z33intel_sub_group_block_read_us1_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <64 x i16> @_Z33intel_sub_group_block_read_us1_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <8 x i16> @_Z32intel_sub_group_block_read_us2_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <8 x i16> @_Z32intel_sub_group_block_read_us2_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <16 x i16> @_Z32intel_sub_group_block_read_us2_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <16 x i16> @_Z32intel_sub_group_block_read_us2_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <32 x i16> @_Z33intel_sub_group_block_read_us2_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <32 x i16> @_Z33intel_sub_group_block_read_us2_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <64 x i16> @_Z33intel_sub_group_block_read_us2_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <64 x i16> @_Z33intel_sub_group_block_read_us2_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <128 x i16> @_Z33intel_sub_group_block_read_us2_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <128 x i16> @_Z33intel_sub_group_block_read_us2_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <16 x i16> @_Z32intel_sub_group_block_read_us4_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <16 x i16> @_Z32intel_sub_group_block_read_us4_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <32 x i16> @_Z32intel_sub_group_block_read_us4_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <32 x i16> @_Z32intel_sub_group_block_read_us4_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <64 x i16> @_Z33intel_sub_group_block_read_us4_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <64 x i16> @_Z33intel_sub_group_block_read_us4_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <128 x i16> @_Z33intel_sub_group_block_read_us4_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <128 x i16> @_Z33intel_sub_group_block_read_us4_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <256 x i16> @_Z33intel_sub_group_block_read_us4_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <256 x i16> @_Z33intel_sub_group_block_read_us4_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <32 x i16> @_Z32intel_sub_group_block_read_us8_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <32 x i16> @_Z32intel_sub_group_block_read_us8_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <64 x i16> @_Z32intel_sub_group_block_read_us8_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <64 x i16> @_Z32intel_sub_group_block_read_us8_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <128 x i16> @_Z33intel_sub_group_block_read_us8_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <128 x i16> @_Z33intel_sub_group_block_read_us8_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <256 x i16> @_Z33intel_sub_group_block_read_us8_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <256 x i16> @_Z33intel_sub_group_block_read_us8_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <512 x i16> @_Z33intel_sub_group_block_read_us8_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <512 x i16> @_Z33intel_sub_group_block_read_us8_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us1_4Dv4_14ocl_image2d_woDv8_iDv4_tDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i16> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us1_414ocl_image2d_woDv2_iDv4_tDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i16> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us1_8Dv8_14ocl_image2d_woDv16_iDv8_tDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i16> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us1_814ocl_image2d_woDv2_iDv8_tDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i16> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us1_16Dv16_14ocl_image2d_woDv32_iDv16_tDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i16> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us1_1614ocl_image2d_woDv2_iDv16_tDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i16> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us1_32Dv32_14ocl_image2d_woDv64_iDv32_tDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i16> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us1_3214ocl_image2d_woDv2_iDv32_tDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i16> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us1_64Dv64_14ocl_image2d_woDv128_iDv64_tDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i16> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us1_6414ocl_image2d_woDv2_iDv64_tDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i16> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us2_4Dv4_14ocl_image2d_woDv8_iDv8_tDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <8 x i16> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us2_414ocl_image2d_woDv2_iDv8_tDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i16> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us2_8Dv8_14ocl_image2d_woDv16_iDv16_tDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <16 x i16> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us2_814ocl_image2d_woDv2_iDv16_tDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i16> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us2_16Dv16_14ocl_image2d_woDv32_iDv32_tDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <32 x i16> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us2_1614ocl_image2d_woDv2_iDv32_tDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i16> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us2_32Dv32_14ocl_image2d_woDv64_iDv64_tDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <64 x i16> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us2_3214ocl_image2d_woDv2_iDv64_tDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i16> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us2_64Dv64_14ocl_image2d_woDv128_iDv128_tDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <128 x i16> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us2_6414ocl_image2d_woDv2_iDv128_tDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i16> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us4_4Dv4_14ocl_image2d_woDv8_iDv16_tDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <16 x i16> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us4_414ocl_image2d_woDv2_iDv16_tDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i16> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us4_8Dv8_14ocl_image2d_woDv16_iDv32_tDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <32 x i16> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us4_814ocl_image2d_woDv2_iDv32_tDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i16> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us4_16Dv16_14ocl_image2d_woDv32_iDv64_tDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <64 x i16> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us4_1614ocl_image2d_woDv2_iDv64_tDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i16> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us4_32Dv32_14ocl_image2d_woDv64_iDv128_tDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <128 x i16> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us4_3214ocl_image2d_woDv2_iDv128_tDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i16> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us4_64Dv64_14ocl_image2d_woDv128_iDv256_tDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <256 x i16> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us4_6414ocl_image2d_woDv2_iDv256_tDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i16> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us8_4Dv4_14ocl_image2d_woDv8_iDv32_tDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <32 x i16> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us8_414ocl_image2d_woDv2_iDv32_tDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i16> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us8_8Dv8_14ocl_image2d_woDv16_iDv64_tDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <64 x i16> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us8_814ocl_image2d_woDv2_iDv64_tDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i16> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us8_16Dv16_14ocl_image2d_woDv32_iDv128_tDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <128 x i16> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us8_1614ocl_image2d_woDv2_iDv128_tDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i16> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us8_32Dv32_14ocl_image2d_woDv64_iDv256_tDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <256 x i16> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us8_3214ocl_image2d_woDv2_iDv256_tDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i16> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us8_64Dv64_14ocl_image2d_woDv128_iDv512_tDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <512 x i16> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us8_6414ocl_image2d_woDv2_iDv512_tDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <512 x i16> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us1_4Dv4_14ocl_image2d_rwDv8_iDv4_tDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i16> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us1_414ocl_image2d_rwDv2_iDv4_tDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i16> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us1_8Dv8_14ocl_image2d_rwDv16_iDv8_tDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i16> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us1_814ocl_image2d_rwDv2_iDv8_tDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i16> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us1_16Dv16_14ocl_image2d_rwDv32_iDv16_tDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i16> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us1_1614ocl_image2d_rwDv2_iDv16_tDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i16> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us1_32Dv32_14ocl_image2d_rwDv64_iDv32_tDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i16> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us1_3214ocl_image2d_rwDv2_iDv32_tDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i16> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us1_64Dv64_14ocl_image2d_rwDv128_iDv64_tDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i16> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us1_6414ocl_image2d_rwDv2_iDv64_tDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i16> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us2_4Dv4_14ocl_image2d_rwDv8_iDv8_tDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <8 x i16> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us2_414ocl_image2d_rwDv2_iDv8_tDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i16> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us2_8Dv8_14ocl_image2d_rwDv16_iDv16_tDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <16 x i16> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us2_814ocl_image2d_rwDv2_iDv16_tDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i16> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us2_16Dv16_14ocl_image2d_rwDv32_iDv32_tDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <32 x i16> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us2_1614ocl_image2d_rwDv2_iDv32_tDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i16> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us2_32Dv32_14ocl_image2d_rwDv64_iDv64_tDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <64 x i16> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us2_3214ocl_image2d_rwDv2_iDv64_tDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i16> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us2_64Dv64_14ocl_image2d_rwDv128_iDv128_tDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <128 x i16> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us2_6414ocl_image2d_rwDv2_iDv128_tDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i16> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us4_4Dv4_14ocl_image2d_rwDv8_iDv16_tDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <16 x i16> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us4_414ocl_image2d_rwDv2_iDv16_tDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i16> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us4_8Dv8_14ocl_image2d_rwDv16_iDv32_tDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <32 x i16> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us4_814ocl_image2d_rwDv2_iDv32_tDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i16> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us4_16Dv16_14ocl_image2d_rwDv32_iDv64_tDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <64 x i16> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us4_1614ocl_image2d_rwDv2_iDv64_tDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i16> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us4_32Dv32_14ocl_image2d_rwDv64_iDv128_tDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <128 x i16> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us4_3214ocl_image2d_rwDv2_iDv128_tDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i16> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us4_64Dv64_14ocl_image2d_rwDv128_iDv256_tDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <256 x i16> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us4_6414ocl_image2d_rwDv2_iDv256_tDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i16> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us8_4Dv4_14ocl_image2d_rwDv8_iDv32_tDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <32 x i16> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us8_414ocl_image2d_rwDv2_iDv32_tDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i16> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_us8_8Dv8_14ocl_image2d_rwDv16_iDv64_tDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <64 x i16> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_us8_814ocl_image2d_rwDv2_iDv64_tDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i16> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us8_16Dv16_14ocl_image2d_rwDv32_iDv128_tDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <128 x i16> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us8_1614ocl_image2d_rwDv2_iDv128_tDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i16> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us8_32Dv32_14ocl_image2d_rwDv64_iDv256_tDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <256 x i16> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us8_3214ocl_image2d_rwDv2_iDv256_tDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i16> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_us8_64Dv64_14ocl_image2d_rwDv128_iDv512_tDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <512 x i16> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_us8_6414ocl_image2d_rwDv2_iDv512_tDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <512 x i16> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define <4 x i32> @_Z32intel_sub_group_block_read_ui1_4Dv4_PU3AS1KjDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call <4 x i32> @_Z32intel_sub_group_block_read_ui1_4PU3AS1KjDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <8 x i32> @_Z32intel_sub_group_block_read_ui1_8Dv8_PU3AS1KjDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <8 x i32> @_Z32intel_sub_group_block_read_ui1_8PU3AS1KjDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <16 x i32> @_Z33intel_sub_group_block_read_ui1_16Dv16_PU3AS1KjDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <16 x i32> @_Z33intel_sub_group_block_read_ui1_16PU3AS1KjDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z33intel_sub_group_block_read_ui1_32Dv32_PU3AS1KjDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z33intel_sub_group_block_read_ui1_32PU3AS1KjDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z33intel_sub_group_block_read_ui1_64Dv64_PU3AS1KjDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z33intel_sub_group_block_read_ui1_64PU3AS1KjDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <8 x i32> @_Z32intel_sub_group_block_read_ui2_4Dv4_PU3AS1KjDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call <8 x i32> @_Z32intel_sub_group_block_read_ui2_4PU3AS1KjDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <16 x i32> @_Z32intel_sub_group_block_read_ui2_8Dv8_PU3AS1KjDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <16 x i32> @_Z32intel_sub_group_block_read_ui2_8PU3AS1KjDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z33intel_sub_group_block_read_ui2_16Dv16_PU3AS1KjDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z33intel_sub_group_block_read_ui2_16PU3AS1KjDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z33intel_sub_group_block_read_ui2_32Dv32_PU3AS1KjDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z33intel_sub_group_block_read_ui2_32PU3AS1KjDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <128 x i32> @_Z33intel_sub_group_block_read_ui2_64Dv64_PU3AS1KjDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <128 x i32> @_Z33intel_sub_group_block_read_ui2_64PU3AS1KjDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <16 x i32> @_Z32intel_sub_group_block_read_ui4_4Dv4_PU3AS1KjDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call <16 x i32> @_Z32intel_sub_group_block_read_ui4_4PU3AS1KjDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z32intel_sub_group_block_read_ui4_8Dv8_PU3AS1KjDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z32intel_sub_group_block_read_ui4_8PU3AS1KjDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z33intel_sub_group_block_read_ui4_16Dv16_PU3AS1KjDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z33intel_sub_group_block_read_ui4_16PU3AS1KjDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <128 x i32> @_Z33intel_sub_group_block_read_ui4_32Dv32_PU3AS1KjDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <128 x i32> @_Z33intel_sub_group_block_read_ui4_32PU3AS1KjDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <256 x i32> @_Z33intel_sub_group_block_read_ui4_64Dv64_PU3AS1KjDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <256 x i32> @_Z33intel_sub_group_block_read_ui4_64PU3AS1KjDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z32intel_sub_group_block_read_ui8_4Dv4_PU3AS1KjDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z32intel_sub_group_block_read_ui8_4PU3AS1KjDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z32intel_sub_group_block_read_ui8_8Dv8_PU3AS1KjDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z32intel_sub_group_block_read_ui8_8PU3AS1KjDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <128 x i32> @_Z33intel_sub_group_block_read_ui8_16Dv16_PU3AS1KjDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <128 x i32> @_Z33intel_sub_group_block_read_ui8_16PU3AS1KjDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <256 x i32> @_Z33intel_sub_group_block_read_ui8_32Dv32_PU3AS1KjDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <256 x i32> @_Z33intel_sub_group_block_read_ui8_32PU3AS1KjDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <512 x i32> @_Z33intel_sub_group_block_read_ui8_64Dv64_PU3AS1KjDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <512 x i32> @_Z33intel_sub_group_block_read_ui8_64PU3AS1KjDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui1_4Dv4_PU3AS1jDv4_jS2_(<4 x ptr addrspace(1)> %data, <4 x i32> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui1_4PU3AS1jDv4_jS1_(ptr addrspace(1) %vecext, <4 x i32> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui1_8Dv8_PU3AS1jDv8_jS2_(<8 x ptr addrspace(1)> %data, <8 x i32> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui1_8PU3AS1jDv8_jS1_(ptr addrspace(1) %vecext, <8 x i32> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui1_16Dv16_PU3AS1jDv16_jS2_(<16 x ptr addrspace(1)> %data, <16 x i32> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui1_16PU3AS1jDv16_jS1_(ptr addrspace(1) %vecext, <16 x i32> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui1_32Dv32_PU3AS1jDv32_jS2_(<32 x ptr addrspace(1)> %data, <32 x i32> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui1_32PU3AS1jDv32_jS1_(ptr addrspace(1) %vecext, <32 x i32> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui1_64Dv64_PU3AS1jDv64_jS2_(<64 x ptr addrspace(1)> %data, <64 x i32> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui1_64PU3AS1jDv64_jS1_(ptr addrspace(1) %vecext, <64 x i32> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui2_4Dv4_PU3AS1jDv8_jDv4_j(<4 x ptr addrspace(1)> %data, <8 x i32> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui2_4PU3AS1jDv8_jDv4_j(ptr addrspace(1) %vecext, <8 x i32> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui2_8Dv8_PU3AS1jDv16_jDv8_j(<8 x ptr addrspace(1)> %data, <16 x i32> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui2_8PU3AS1jDv16_jDv8_j(ptr addrspace(1) %vecext, <16 x i32> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui2_16Dv16_PU3AS1jDv32_jDv16_j(<16 x ptr addrspace(1)> %data, <32 x i32> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui2_16PU3AS1jDv32_jDv16_j(ptr addrspace(1) %vecext, <32 x i32> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui2_32Dv32_PU3AS1jDv64_jDv32_j(<32 x ptr addrspace(1)> %data, <64 x i32> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui2_32PU3AS1jDv64_jDv32_j(ptr addrspace(1) %vecext, <64 x i32> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui2_64Dv64_PU3AS1jDv128_jDv64_j(<64 x ptr addrspace(1)> %data, <128 x i32> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui2_64PU3AS1jDv128_jDv64_j(ptr addrspace(1) %vecext, <128 x i32> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui4_4Dv4_PU3AS1jDv16_jDv4_j(<4 x ptr addrspace(1)> %data, <16 x i32> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui4_4PU3AS1jDv16_jDv4_j(ptr addrspace(1) %vecext, <16 x i32> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui4_8Dv8_PU3AS1jDv32_jDv8_j(<8 x ptr addrspace(1)> %data, <32 x i32> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui4_8PU3AS1jDv32_jDv8_j(ptr addrspace(1) %vecext, <32 x i32> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui4_16Dv16_PU3AS1jDv64_jDv16_j(<16 x ptr addrspace(1)> %data, <64 x i32> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui4_16PU3AS1jDv64_jDv16_j(ptr addrspace(1) %vecext, <64 x i32> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui4_32Dv32_PU3AS1jDv128_jDv32_j(<32 x ptr addrspace(1)> %data, <128 x i32> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui4_32PU3AS1jDv128_jDv32_j(ptr addrspace(1) %vecext, <128 x i32> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui4_64Dv64_PU3AS1jDv256_jDv64_j(<64 x ptr addrspace(1)> %data, <256 x i32> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui4_64PU3AS1jDv256_jDv64_j(ptr addrspace(1) %vecext, <256 x i32> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui8_4Dv4_PU3AS1jDv32_jDv4_j(<4 x ptr addrspace(1)> %data, <32 x i32> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui8_4PU3AS1jDv32_jDv4_j(ptr addrspace(1) %vecext, <32 x i32> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui8_8Dv8_PU3AS1jDv64_jDv8_j(<8 x ptr addrspace(1)> %data, <64 x i32> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui8_8PU3AS1jDv64_jDv8_j(ptr addrspace(1) %vecext, <64 x i32> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui8_16Dv16_PU3AS1jDv128_jDv16_j(<16 x ptr addrspace(1)> %data, <128 x i32> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui8_16PU3AS1jDv128_jDv16_j(ptr addrspace(1) %vecext, <128 x i32> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui8_32Dv32_PU3AS1jDv256_jDv32_j(<32 x ptr addrspace(1)> %data, <256 x i32> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui8_32PU3AS1jDv256_jDv32_j(ptr addrspace(1) %vecext, <256 x i32> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui8_64Dv64_PU3AS1jDv512_jDv64_j(<64 x ptr addrspace(1)> %data, <512 x i32> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui8_64PU3AS1jDv512_jDv64_j(ptr addrspace(1) %vecext, <512 x i32> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define <4 x i32> @_Z32intel_sub_group_block_read_ui1_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <4 x i32> @_Z32intel_sub_group_block_read_ui1_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <8 x i32> @_Z32intel_sub_group_block_read_ui1_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <8 x i32> @_Z32intel_sub_group_block_read_ui1_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <16 x i32> @_Z33intel_sub_group_block_read_ui1_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <16 x i32> @_Z33intel_sub_group_block_read_ui1_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z33intel_sub_group_block_read_ui1_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z33intel_sub_group_block_read_ui1_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z33intel_sub_group_block_read_ui1_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z33intel_sub_group_block_read_ui1_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <8 x i32> @_Z32intel_sub_group_block_read_ui2_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <8 x i32> @_Z32intel_sub_group_block_read_ui2_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <16 x i32> @_Z32intel_sub_group_block_read_ui2_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <16 x i32> @_Z32intel_sub_group_block_read_ui2_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z33intel_sub_group_block_read_ui2_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z33intel_sub_group_block_read_ui2_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z33intel_sub_group_block_read_ui2_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z33intel_sub_group_block_read_ui2_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <128 x i32> @_Z33intel_sub_group_block_read_ui2_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <128 x i32> @_Z33intel_sub_group_block_read_ui2_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <16 x i32> @_Z32intel_sub_group_block_read_ui4_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <16 x i32> @_Z32intel_sub_group_block_read_ui4_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z32intel_sub_group_block_read_ui4_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z32intel_sub_group_block_read_ui4_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z33intel_sub_group_block_read_ui4_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z33intel_sub_group_block_read_ui4_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <128 x i32> @_Z33intel_sub_group_block_read_ui4_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <128 x i32> @_Z33intel_sub_group_block_read_ui4_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <256 x i32> @_Z33intel_sub_group_block_read_ui4_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <256 x i32> @_Z33intel_sub_group_block_read_ui4_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z32intel_sub_group_block_read_ui8_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z32intel_sub_group_block_read_ui8_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z32intel_sub_group_block_read_ui8_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z32intel_sub_group_block_read_ui8_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <128 x i32> @_Z33intel_sub_group_block_read_ui8_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <128 x i32> @_Z33intel_sub_group_block_read_ui8_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <256 x i32> @_Z33intel_sub_group_block_read_ui8_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <256 x i32> @_Z33intel_sub_group_block_read_ui8_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <512 x i32> @_Z33intel_sub_group_block_read_ui8_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <512 x i32> @_Z33intel_sub_group_block_read_ui8_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <4 x i32> @_Z32intel_sub_group_block_read_ui1_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <4 x i32> @_Z32intel_sub_group_block_read_ui1_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <8 x i32> @_Z32intel_sub_group_block_read_ui1_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <8 x i32> @_Z32intel_sub_group_block_read_ui1_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <16 x i32> @_Z33intel_sub_group_block_read_ui1_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <16 x i32> @_Z33intel_sub_group_block_read_ui1_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z33intel_sub_group_block_read_ui1_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z33intel_sub_group_block_read_ui1_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z33intel_sub_group_block_read_ui1_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z33intel_sub_group_block_read_ui1_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <8 x i32> @_Z32intel_sub_group_block_read_ui2_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <8 x i32> @_Z32intel_sub_group_block_read_ui2_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <16 x i32> @_Z32intel_sub_group_block_read_ui2_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <16 x i32> @_Z32intel_sub_group_block_read_ui2_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z33intel_sub_group_block_read_ui2_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z33intel_sub_group_block_read_ui2_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z33intel_sub_group_block_read_ui2_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z33intel_sub_group_block_read_ui2_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <128 x i32> @_Z33intel_sub_group_block_read_ui2_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <128 x i32> @_Z33intel_sub_group_block_read_ui2_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <16 x i32> @_Z32intel_sub_group_block_read_ui4_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <16 x i32> @_Z32intel_sub_group_block_read_ui4_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z32intel_sub_group_block_read_ui4_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z32intel_sub_group_block_read_ui4_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z33intel_sub_group_block_read_ui4_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z33intel_sub_group_block_read_ui4_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <128 x i32> @_Z33intel_sub_group_block_read_ui4_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <128 x i32> @_Z33intel_sub_group_block_read_ui4_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <256 x i32> @_Z33intel_sub_group_block_read_ui4_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <256 x i32> @_Z33intel_sub_group_block_read_ui4_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <32 x i32> @_Z32intel_sub_group_block_read_ui8_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <32 x i32> @_Z32intel_sub_group_block_read_ui8_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <64 x i32> @_Z32intel_sub_group_block_read_ui8_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <64 x i32> @_Z32intel_sub_group_block_read_ui8_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <128 x i32> @_Z33intel_sub_group_block_read_ui8_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <128 x i32> @_Z33intel_sub_group_block_read_ui8_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <256 x i32> @_Z33intel_sub_group_block_read_ui8_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <256 x i32> @_Z33intel_sub_group_block_read_ui8_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <512 x i32> @_Z33intel_sub_group_block_read_ui8_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <512 x i32> @_Z33intel_sub_group_block_read_ui8_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui1_4Dv4_14ocl_image2d_woDv8_iDv4_jS2_(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui1_414ocl_image2d_woDv2_iDv4_jS1_(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui1_8Dv8_14ocl_image2d_woDv16_iDv8_jS2_(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui1_814ocl_image2d_woDv2_iDv8_jS1_(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui1_16Dv16_14ocl_image2d_woDv32_iDv16_jS2_(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui1_1614ocl_image2d_woDv2_iDv16_jS1_(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui1_32Dv32_14ocl_image2d_woDv64_iDv32_jS2_(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui1_3214ocl_image2d_woDv2_iDv32_jS1_(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui1_64Dv64_14ocl_image2d_woDv128_iDv64_jS2_(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui1_6414ocl_image2d_woDv2_iDv64_jS1_(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui2_4Dv4_14ocl_image2d_woDv8_iDv8_jDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <8 x i32> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui2_414ocl_image2d_woDv2_iDv8_jDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui2_8Dv8_14ocl_image2d_woDv16_iDv16_jDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <16 x i32> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui2_814ocl_image2d_woDv2_iDv16_jDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui2_16Dv16_14ocl_image2d_woDv32_iDv32_jDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <32 x i32> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui2_1614ocl_image2d_woDv2_iDv32_jDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui2_32Dv32_14ocl_image2d_woDv64_iDv64_jDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <64 x i32> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui2_3214ocl_image2d_woDv2_iDv64_jDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui2_64Dv64_14ocl_image2d_woDv128_iDv128_jDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <128 x i32> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui2_6414ocl_image2d_woDv2_iDv128_jDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i32> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui4_4Dv4_14ocl_image2d_woDv8_iDv16_jDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <16 x i32> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui4_414ocl_image2d_woDv2_iDv16_jDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui4_8Dv8_14ocl_image2d_woDv16_iDv32_jDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <32 x i32> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui4_814ocl_image2d_woDv2_iDv32_jDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui4_16Dv16_14ocl_image2d_woDv32_iDv64_jDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <64 x i32> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui4_1614ocl_image2d_woDv2_iDv64_jDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui4_32Dv32_14ocl_image2d_woDv64_iDv128_jDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <128 x i32> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui4_3214ocl_image2d_woDv2_iDv128_jDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i32> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui4_64Dv64_14ocl_image2d_woDv128_iDv256_jDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <256 x i32> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui4_6414ocl_image2d_woDv2_iDv256_jDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i32> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui8_4Dv4_14ocl_image2d_woDv8_iDv32_jDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <32 x i32> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui8_414ocl_image2d_woDv2_iDv32_jDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui8_8Dv8_14ocl_image2d_woDv16_iDv64_jDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <64 x i32> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui8_814ocl_image2d_woDv2_iDv64_jDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui8_16Dv16_14ocl_image2d_woDv32_iDv128_jDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <128 x i32> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui8_1614ocl_image2d_woDv2_iDv128_jDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i32> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui8_32Dv32_14ocl_image2d_woDv64_iDv256_jDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <256 x i32> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui8_3214ocl_image2d_woDv2_iDv256_jDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i32> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui8_64Dv64_14ocl_image2d_woDv128_iDv512_jDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <512 x i32> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui8_6414ocl_image2d_woDv2_iDv512_jDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <512 x i32> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui1_4Dv4_14ocl_image2d_rwDv8_iDv4_jS2_(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui1_414ocl_image2d_rwDv2_iDv4_jS1_(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui1_8Dv8_14ocl_image2d_rwDv16_iDv8_jS2_(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui1_814ocl_image2d_rwDv2_iDv8_jS1_(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui1_16Dv16_14ocl_image2d_rwDv32_iDv16_jS2_(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui1_1614ocl_image2d_rwDv2_iDv16_jS1_(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui1_32Dv32_14ocl_image2d_rwDv64_iDv32_jS2_(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui1_3214ocl_image2d_rwDv2_iDv32_jS1_(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui1_64Dv64_14ocl_image2d_rwDv128_iDv64_jS2_(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui1_6414ocl_image2d_rwDv2_iDv64_jS1_(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui2_4Dv4_14ocl_image2d_rwDv8_iDv8_jDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <8 x i32> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui2_414ocl_image2d_rwDv2_iDv8_jDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui2_8Dv8_14ocl_image2d_rwDv16_iDv16_jDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <16 x i32> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui2_814ocl_image2d_rwDv2_iDv16_jDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui2_16Dv16_14ocl_image2d_rwDv32_iDv32_jDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <32 x i32> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui2_1614ocl_image2d_rwDv2_iDv32_jDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui2_32Dv32_14ocl_image2d_rwDv64_iDv64_jDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <64 x i32> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui2_3214ocl_image2d_rwDv2_iDv64_jDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui2_64Dv64_14ocl_image2d_rwDv128_iDv128_jDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <128 x i32> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui2_6414ocl_image2d_rwDv2_iDv128_jDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i32> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui4_4Dv4_14ocl_image2d_rwDv8_iDv16_jDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <16 x i32> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui4_414ocl_image2d_rwDv2_iDv16_jDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui4_8Dv8_14ocl_image2d_rwDv16_iDv32_jDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <32 x i32> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui4_814ocl_image2d_rwDv2_iDv32_jDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui4_16Dv16_14ocl_image2d_rwDv32_iDv64_jDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <64 x i32> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui4_1614ocl_image2d_rwDv2_iDv64_jDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui4_32Dv32_14ocl_image2d_rwDv64_iDv128_jDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <128 x i32> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui4_3214ocl_image2d_rwDv2_iDv128_jDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i32> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui4_64Dv64_14ocl_image2d_rwDv128_iDv256_jDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <256 x i32> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui4_6414ocl_image2d_rwDv2_iDv256_jDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i32> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui8_4Dv4_14ocl_image2d_rwDv8_iDv32_jDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <32 x i32> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui8_414ocl_image2d_rwDv2_iDv32_jDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ui8_8Dv8_14ocl_image2d_rwDv16_iDv64_jDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <64 x i32> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ui8_814ocl_image2d_rwDv2_iDv64_jDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui8_16Dv16_14ocl_image2d_rwDv32_iDv128_jDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <128 x i32> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui8_1614ocl_image2d_rwDv2_iDv128_jDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i32> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui8_32Dv32_14ocl_image2d_rwDv64_iDv256_jDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <256 x i32> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui8_3214ocl_image2d_rwDv2_iDv256_jDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i32> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ui8_64Dv64_14ocl_image2d_rwDv128_iDv512_jDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <512 x i32> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ui8_6414ocl_image2d_rwDv2_iDv512_jDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <512 x i32> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define <4 x i64> @_Z32intel_sub_group_block_read_ul1_4Dv4_PU3AS1KmDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call <4 x i64> @_Z32intel_sub_group_block_read_ul1_4PU3AS1KmDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <8 x i64> @_Z32intel_sub_group_block_read_ul1_8Dv8_PU3AS1KmDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <8 x i64> @_Z32intel_sub_group_block_read_ul1_8PU3AS1KmDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <16 x i64> @_Z33intel_sub_group_block_read_ul1_16Dv16_PU3AS1KmDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <16 x i64> @_Z33intel_sub_group_block_read_ul1_16PU3AS1KmDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <32 x i64> @_Z33intel_sub_group_block_read_ul1_32Dv32_PU3AS1KmDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <32 x i64> @_Z33intel_sub_group_block_read_ul1_32PU3AS1KmDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <64 x i64> @_Z33intel_sub_group_block_read_ul1_64Dv64_PU3AS1KmDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <64 x i64> @_Z33intel_sub_group_block_read_ul1_64PU3AS1KmDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <8 x i64> @_Z32intel_sub_group_block_read_ul2_4Dv4_PU3AS1KmDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call <8 x i64> @_Z32intel_sub_group_block_read_ul2_4PU3AS1KmDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <16 x i64> @_Z32intel_sub_group_block_read_ul2_8Dv8_PU3AS1KmDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <16 x i64> @_Z32intel_sub_group_block_read_ul2_8PU3AS1KmDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <32 x i64> @_Z33intel_sub_group_block_read_ul2_16Dv16_PU3AS1KmDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <32 x i64> @_Z33intel_sub_group_block_read_ul2_16PU3AS1KmDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <64 x i64> @_Z33intel_sub_group_block_read_ul2_32Dv32_PU3AS1KmDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <64 x i64> @_Z33intel_sub_group_block_read_ul2_32PU3AS1KmDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <128 x i64> @_Z33intel_sub_group_block_read_ul2_64Dv64_PU3AS1KmDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <128 x i64> @_Z33intel_sub_group_block_read_ul2_64PU3AS1KmDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <16 x i64> @_Z32intel_sub_group_block_read_ul4_4Dv4_PU3AS1KmDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call <16 x i64> @_Z32intel_sub_group_block_read_ul4_4PU3AS1KmDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <32 x i64> @_Z32intel_sub_group_block_read_ul4_8Dv8_PU3AS1KmDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <32 x i64> @_Z32intel_sub_group_block_read_ul4_8PU3AS1KmDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <64 x i64> @_Z33intel_sub_group_block_read_ul4_16Dv16_PU3AS1KmDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <64 x i64> @_Z33intel_sub_group_block_read_ul4_16PU3AS1KmDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <128 x i64> @_Z33intel_sub_group_block_read_ul4_32Dv32_PU3AS1KmDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <128 x i64> @_Z33intel_sub_group_block_read_ul4_32PU3AS1KmDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <256 x i64> @_Z33intel_sub_group_block_read_ul4_64Dv64_PU3AS1KmDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <256 x i64> @_Z33intel_sub_group_block_read_ul4_64PU3AS1KmDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <32 x i64> @_Z32intel_sub_group_block_read_ul8_4Dv4_PU3AS1KmDv4_j(<4 x ptr addrspace(1)> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call <32 x i64> @_Z32intel_sub_group_block_read_ul8_4PU3AS1KmDv4_j(ptr addrspace(1) %vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <64 x i64> @_Z32intel_sub_group_block_read_ul8_8Dv8_PU3AS1KmDv8_j(<8 x ptr addrspace(1)> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call <64 x i64> @_Z32intel_sub_group_block_read_ul8_8PU3AS1KmDv8_j(ptr addrspace(1) %vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <128 x i64> @_Z33intel_sub_group_block_read_ul8_16Dv16_PU3AS1KmDv16_j(<16 x ptr addrspace(1)> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call <128 x i64> @_Z33intel_sub_group_block_read_ul8_16PU3AS1KmDv16_j(ptr addrspace(1) %vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <256 x i64> @_Z33intel_sub_group_block_read_ul8_32Dv32_PU3AS1KmDv32_j(<32 x ptr addrspace(1)> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call <256 x i64> @_Z33intel_sub_group_block_read_ul8_32PU3AS1KmDv32_j(ptr addrspace(1) %vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <512 x i64> @_Z33intel_sub_group_block_read_ul8_64Dv64_PU3AS1KmDv64_j(<64 x ptr addrspace(1)> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call <512 x i64> @_Z33intel_sub_group_block_read_ul8_64PU3AS1KmDv64_j(ptr addrspace(1) %vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul1_4Dv4_PU3AS1mDv4_mDv4_j(<4 x ptr addrspace(1)> %data, <4 x i64> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul1_4PU3AS1mDv4_mDv4_j(ptr addrspace(1) %vecext, <4 x i64> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul1_8Dv8_PU3AS1mDv8_mDv8_j(<8 x ptr addrspace(1)> %data, <8 x i64> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul1_8PU3AS1mDv8_mDv8_j(ptr addrspace(1) %vecext, <8 x i64> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul1_16Dv16_PU3AS1mDv16_mDv16_j(<16 x ptr addrspace(1)> %data, <16 x i64> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul1_16PU3AS1mDv16_mDv16_j(ptr addrspace(1) %vecext, <16 x i64> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul1_32Dv32_PU3AS1mDv32_mDv32_j(<32 x ptr addrspace(1)> %data, <32 x i64> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul1_32PU3AS1mDv32_mDv32_j(ptr addrspace(1) %vecext, <32 x i64> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul1_64Dv64_PU3AS1mDv64_mDv64_j(<64 x ptr addrspace(1)> %data, <64 x i64> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul1_64PU3AS1mDv64_mDv64_j(ptr addrspace(1) %vecext, <64 x i64> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul2_4Dv4_PU3AS1mDv8_mDv4_j(<4 x ptr addrspace(1)> %data, <8 x i64> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul2_4PU3AS1mDv8_mDv4_j(ptr addrspace(1) %vecext, <8 x i64> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul2_8Dv8_PU3AS1mDv16_mDv8_j(<8 x ptr addrspace(1)> %data, <16 x i64> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul2_8PU3AS1mDv16_mDv8_j(ptr addrspace(1) %vecext, <16 x i64> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul2_16Dv16_PU3AS1mDv32_mDv16_j(<16 x ptr addrspace(1)> %data, <32 x i64> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul2_16PU3AS1mDv32_mDv16_j(ptr addrspace(1) %vecext, <32 x i64> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul2_32Dv32_PU3AS1mDv64_mDv32_j(<32 x ptr addrspace(1)> %data, <64 x i64> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul2_32PU3AS1mDv64_mDv32_j(ptr addrspace(1) %vecext, <64 x i64> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul2_64Dv64_PU3AS1mDv128_mDv64_j(<64 x ptr addrspace(1)> %data, <128 x i64> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul2_64PU3AS1mDv128_mDv64_j(ptr addrspace(1) %vecext, <128 x i64> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul4_4Dv4_PU3AS1mDv16_mDv4_j(<4 x ptr addrspace(1)> %data, <16 x i64> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul4_4PU3AS1mDv16_mDv4_j(ptr addrspace(1) %vecext, <16 x i64> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul4_8Dv8_PU3AS1mDv32_mDv8_j(<8 x ptr addrspace(1)> %data, <32 x i64> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul4_8PU3AS1mDv32_mDv8_j(ptr addrspace(1) %vecext, <32 x i64> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul4_16Dv16_PU3AS1mDv64_mDv16_j(<16 x ptr addrspace(1)> %data, <64 x i64> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul4_16PU3AS1mDv64_mDv16_j(ptr addrspace(1) %vecext, <64 x i64> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul4_32Dv32_PU3AS1mDv128_mDv32_j(<32 x ptr addrspace(1)> %data, <128 x i64> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul4_32PU3AS1mDv128_mDv32_j(ptr addrspace(1) %vecext, <128 x i64> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul4_64Dv64_PU3AS1mDv256_mDv64_j(<64 x ptr addrspace(1)> %data, <256 x i64> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul4_64PU3AS1mDv256_mDv64_j(ptr addrspace(1) %vecext, <256 x i64> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul8_4Dv4_PU3AS1mDv32_mDv4_j(<4 x ptr addrspace(1)> %data, <32 x i64> %src, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul8_4PU3AS1mDv32_mDv4_j(ptr addrspace(1) %vecext, <32 x i64> %src, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul8_8Dv8_PU3AS1mDv64_mDv8_j(<8 x ptr addrspace(1)> %data, <64 x i64> %src, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul8_8PU3AS1mDv64_mDv8_j(ptr addrspace(1) %vecext, <64 x i64> %src, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul8_16Dv16_PU3AS1mDv128_mDv16_j(<16 x ptr addrspace(1)> %data, <128 x i64> %src, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul8_16PU3AS1mDv128_mDv16_j(ptr addrspace(1) %vecext, <128 x i64> %src, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul8_32Dv32_PU3AS1mDv256_mDv32_j(<32 x ptr addrspace(1)> %data, <256 x i64> %src, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul8_32PU3AS1mDv256_mDv32_j(ptr addrspace(1) %vecext, <256 x i64> %src, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul8_64Dv64_PU3AS1mDv512_mDv64_j(<64 x ptr addrspace(1)> %data, <512 x i64> %src, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul8_64PU3AS1mDv512_mDv64_j(ptr addrspace(1) %vecext, <512 x i64> %src, <64 x i32> %vec_mask)

; CHECK-DAG: define <4 x i64> @_Z32intel_sub_group_block_read_ul1_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <4 x i64> @_Z32intel_sub_group_block_read_ul1_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <8 x i64> @_Z32intel_sub_group_block_read_ul1_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <8 x i64> @_Z32intel_sub_group_block_read_ul1_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <16 x i64> @_Z33intel_sub_group_block_read_ul1_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <16 x i64> @_Z33intel_sub_group_block_read_ul1_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <32 x i64> @_Z33intel_sub_group_block_read_ul1_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <32 x i64> @_Z33intel_sub_group_block_read_ul1_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <64 x i64> @_Z33intel_sub_group_block_read_ul1_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <64 x i64> @_Z33intel_sub_group_block_read_ul1_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <8 x i64> @_Z32intel_sub_group_block_read_ul2_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <8 x i64> @_Z32intel_sub_group_block_read_ul2_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <16 x i64> @_Z32intel_sub_group_block_read_ul2_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <16 x i64> @_Z32intel_sub_group_block_read_ul2_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <32 x i64> @_Z33intel_sub_group_block_read_ul2_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <32 x i64> @_Z33intel_sub_group_block_read_ul2_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <64 x i64> @_Z33intel_sub_group_block_read_ul2_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <64 x i64> @_Z33intel_sub_group_block_read_ul2_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <128 x i64> @_Z33intel_sub_group_block_read_ul2_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <128 x i64> @_Z33intel_sub_group_block_read_ul2_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <16 x i64> @_Z32intel_sub_group_block_read_ul4_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <16 x i64> @_Z32intel_sub_group_block_read_ul4_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <32 x i64> @_Z32intel_sub_group_block_read_ul4_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <32 x i64> @_Z32intel_sub_group_block_read_ul4_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <64 x i64> @_Z33intel_sub_group_block_read_ul4_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <64 x i64> @_Z33intel_sub_group_block_read_ul4_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <128 x i64> @_Z33intel_sub_group_block_read_ul4_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <128 x i64> @_Z33intel_sub_group_block_read_ul4_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <256 x i64> @_Z33intel_sub_group_block_read_ul4_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <256 x i64> @_Z33intel_sub_group_block_read_ul4_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <32 x i64> @_Z32intel_sub_group_block_read_ul8_4Dv4_14ocl_image2d_roDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <32 x i64> @_Z32intel_sub_group_block_read_ul8_414ocl_image2d_roDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <64 x i64> @_Z32intel_sub_group_block_read_ul8_8Dv8_14ocl_image2d_roDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <64 x i64> @_Z32intel_sub_group_block_read_ul8_814ocl_image2d_roDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <128 x i64> @_Z33intel_sub_group_block_read_ul8_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <128 x i64> @_Z33intel_sub_group_block_read_ul8_1614ocl_image2d_roDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <256 x i64> @_Z33intel_sub_group_block_read_ul8_32Dv32_14ocl_image2d_roDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <256 x i64> @_Z33intel_sub_group_block_read_ul8_3214ocl_image2d_roDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <512 x i64> @_Z33intel_sub_group_block_read_ul8_64Dv64_14ocl_image2d_roDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <512 x i64> @_Z33intel_sub_group_block_read_ul8_6414ocl_image2d_roDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <4 x i64> @_Z32intel_sub_group_block_read_ul1_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <4 x i64> @_Z32intel_sub_group_block_read_ul1_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <8 x i64> @_Z32intel_sub_group_block_read_ul1_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <8 x i64> @_Z32intel_sub_group_block_read_ul1_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <16 x i64> @_Z33intel_sub_group_block_read_ul1_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <16 x i64> @_Z33intel_sub_group_block_read_ul1_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <32 x i64> @_Z33intel_sub_group_block_read_ul1_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <32 x i64> @_Z33intel_sub_group_block_read_ul1_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <64 x i64> @_Z33intel_sub_group_block_read_ul1_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <64 x i64> @_Z33intel_sub_group_block_read_ul1_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <8 x i64> @_Z32intel_sub_group_block_read_ul2_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <8 x i64> @_Z32intel_sub_group_block_read_ul2_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <16 x i64> @_Z32intel_sub_group_block_read_ul2_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <16 x i64> @_Z32intel_sub_group_block_read_ul2_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <32 x i64> @_Z33intel_sub_group_block_read_ul2_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <32 x i64> @_Z33intel_sub_group_block_read_ul2_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <64 x i64> @_Z33intel_sub_group_block_read_ul2_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <64 x i64> @_Z33intel_sub_group_block_read_ul2_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <128 x i64> @_Z33intel_sub_group_block_read_ul2_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <128 x i64> @_Z33intel_sub_group_block_read_ul2_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <16 x i64> @_Z32intel_sub_group_block_read_ul4_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <16 x i64> @_Z32intel_sub_group_block_read_ul4_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <32 x i64> @_Z32intel_sub_group_block_read_ul4_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <32 x i64> @_Z32intel_sub_group_block_read_ul4_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <64 x i64> @_Z33intel_sub_group_block_read_ul4_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <64 x i64> @_Z33intel_sub_group_block_read_ul4_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <128 x i64> @_Z33intel_sub_group_block_read_ul4_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <128 x i64> @_Z33intel_sub_group_block_read_ul4_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <256 x i64> @_Z33intel_sub_group_block_read_ul4_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <256 x i64> @_Z33intel_sub_group_block_read_ul4_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define <32 x i64> @_Z32intel_sub_group_block_read_ul8_4Dv4_14ocl_image2d_rwDv8_iDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i32> %vec_mask)
; CHECK-DAG: call <32 x i64> @_Z32intel_sub_group_block_read_ul8_414ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i32> %vec_mask)

; CHECK-DAG: define <64 x i64> @_Z32intel_sub_group_block_read_ul8_8Dv8_14ocl_image2d_rwDv16_iDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i32> %vec_mask)
; CHECK-DAG: call <64 x i64> @_Z32intel_sub_group_block_read_ul8_814ocl_image2d_rwDv2_iDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i32> %vec_mask)

; CHECK-DAG: define <128 x i64> @_Z33intel_sub_group_block_read_ul8_16Dv16_14ocl_image2d_rwDv32_iDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i32> %vec_mask)
; CHECK-DAG: call <128 x i64> @_Z33intel_sub_group_block_read_ul8_1614ocl_image2d_rwDv2_iDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)

; CHECK-DAG: define <256 x i64> @_Z33intel_sub_group_block_read_ul8_32Dv32_14ocl_image2d_rwDv64_iDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i32> %vec_mask)
; CHECK-DAG: call <256 x i64> @_Z33intel_sub_group_block_read_ul8_3214ocl_image2d_rwDv2_iDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i32> %vec_mask)

; CHECK-DAG: define <512 x i64> @_Z33intel_sub_group_block_read_ul8_64Dv64_14ocl_image2d_rwDv128_iDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i32> %vec_mask)
; CHECK-DAG: call <512 x i64> @_Z33intel_sub_group_block_read_ul8_6414ocl_image2d_rwDv2_iDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul1_4Dv4_14ocl_image2d_woDv8_iDv4_mDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i64> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul1_414ocl_image2d_woDv2_iDv4_mDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i64> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul1_8Dv8_14ocl_image2d_woDv16_iDv8_mDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i64> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul1_814ocl_image2d_woDv2_iDv8_mDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i64> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul1_16Dv16_14ocl_image2d_woDv32_iDv16_mDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i64> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul1_1614ocl_image2d_woDv2_iDv16_mDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i64> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul1_32Dv32_14ocl_image2d_woDv64_iDv32_mDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i64> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul1_3214ocl_image2d_woDv2_iDv32_mDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i64> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul1_64Dv64_14ocl_image2d_woDv128_iDv64_mDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i64> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul1_6414ocl_image2d_woDv2_iDv64_mDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i64> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul2_4Dv4_14ocl_image2d_woDv8_iDv8_mDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <8 x i64> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul2_414ocl_image2d_woDv2_iDv8_mDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i64> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul2_8Dv8_14ocl_image2d_woDv16_iDv16_mDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <16 x i64> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul2_814ocl_image2d_woDv2_iDv16_mDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i64> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul2_16Dv16_14ocl_image2d_woDv32_iDv32_mDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <32 x i64> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul2_1614ocl_image2d_woDv2_iDv32_mDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i64> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul2_32Dv32_14ocl_image2d_woDv64_iDv64_mDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <64 x i64> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul2_3214ocl_image2d_woDv2_iDv64_mDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i64> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul2_64Dv64_14ocl_image2d_woDv128_iDv128_mDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <128 x i64> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul2_6414ocl_image2d_woDv2_iDv128_mDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i64> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul4_4Dv4_14ocl_image2d_woDv8_iDv16_mDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <16 x i64> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul4_414ocl_image2d_woDv2_iDv16_mDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i64> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul4_8Dv8_14ocl_image2d_woDv16_iDv32_mDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <32 x i64> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul4_814ocl_image2d_woDv2_iDv32_mDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i64> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul4_16Dv16_14ocl_image2d_woDv32_iDv64_mDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <64 x i64> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul4_1614ocl_image2d_woDv2_iDv64_mDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i64> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul4_32Dv32_14ocl_image2d_woDv64_iDv128_mDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <128 x i64> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul4_3214ocl_image2d_woDv2_iDv128_mDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i64> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul4_64Dv64_14ocl_image2d_woDv128_iDv256_mDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <256 x i64> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul4_6414ocl_image2d_woDv2_iDv256_mDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i64> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul8_4Dv4_14ocl_image2d_woDv8_iDv32_mDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <32 x i64> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul8_414ocl_image2d_woDv2_iDv32_mDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i64> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul8_8Dv8_14ocl_image2d_woDv16_iDv64_mDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <64 x i64> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul8_814ocl_image2d_woDv2_iDv64_mDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i64> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul8_16Dv16_14ocl_image2d_woDv32_iDv128_mDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <128 x i64> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul8_1614ocl_image2d_woDv2_iDv128_mDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i64> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul8_32Dv32_14ocl_image2d_woDv64_iDv256_mDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <256 x i64> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul8_3214ocl_image2d_woDv2_iDv256_mDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i64> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul8_64Dv64_14ocl_image2d_woDv128_iDv512_mDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <512 x i64> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul8_6414ocl_image2d_woDv2_iDv512_mDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <512 x i64> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul1_4Dv4_14ocl_image2d_rwDv8_iDv4_mDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <4 x i64> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul1_414ocl_image2d_rwDv2_iDv4_mDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <4 x i64> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul1_8Dv8_14ocl_image2d_rwDv16_iDv8_mDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <8 x i64> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul1_814ocl_image2d_rwDv2_iDv8_mDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i64> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul1_16Dv16_14ocl_image2d_rwDv32_iDv16_mDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <16 x i64> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul1_1614ocl_image2d_rwDv2_iDv16_mDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i64> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul1_32Dv32_14ocl_image2d_rwDv64_iDv32_mDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <32 x i64> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul1_3214ocl_image2d_rwDv2_iDv32_mDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i64> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul1_64Dv64_14ocl_image2d_rwDv128_iDv64_mDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <64 x i64> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul1_6414ocl_image2d_rwDv2_iDv64_mDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i64> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul2_4Dv4_14ocl_image2d_rwDv8_iDv8_mDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <8 x i64> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul2_414ocl_image2d_rwDv2_iDv8_mDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <8 x i64> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul2_8Dv8_14ocl_image2d_rwDv16_iDv16_mDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <16 x i64> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul2_814ocl_image2d_rwDv2_iDv16_mDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i64> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul2_16Dv16_14ocl_image2d_rwDv32_iDv32_mDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <32 x i64> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul2_1614ocl_image2d_rwDv2_iDv32_mDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i64> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul2_32Dv32_14ocl_image2d_rwDv64_iDv64_mDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <64 x i64> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul2_3214ocl_image2d_rwDv2_iDv64_mDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i64> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul2_64Dv64_14ocl_image2d_rwDv128_iDv128_mDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <128 x i64> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul2_6414ocl_image2d_rwDv2_iDv128_mDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i64> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul4_4Dv4_14ocl_image2d_rwDv8_iDv16_mDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <16 x i64> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul4_414ocl_image2d_rwDv2_iDv16_mDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <16 x i64> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul4_8Dv8_14ocl_image2d_rwDv16_iDv32_mDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <32 x i64> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul4_814ocl_image2d_rwDv2_iDv32_mDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i64> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul4_16Dv16_14ocl_image2d_rwDv32_iDv64_mDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <64 x i64> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul4_1614ocl_image2d_rwDv2_iDv64_mDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i64> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul4_32Dv32_14ocl_image2d_rwDv64_iDv128_mDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <128 x i64> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul4_3214ocl_image2d_rwDv2_iDv128_mDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i64> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul4_64Dv64_14ocl_image2d_rwDv128_iDv256_mDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <256 x i64> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul4_6414ocl_image2d_rwDv2_iDv256_mDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i64> %data, <64 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul8_4Dv4_14ocl_image2d_rwDv8_iDv32_mDv4_j(<4 x ptr addrspace(1)> %img, <8 x i32> %coord, <32 x i64> %data, <4 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul8_414ocl_image2d_rwDv2_iDv32_mDv4_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <32 x i64> %data, <4 x i32> %vec_mask)

; CHECK-DAG: define void @_Z33intel_sub_group_block_write_ul8_8Dv8_14ocl_image2d_rwDv16_iDv64_mDv8_j(<8 x ptr addrspace(1)> %img, <16 x i32> %coord, <64 x i64> %data, <8 x i32> %vec_mask)
; CHECK-DAG: call void @_Z33intel_sub_group_block_write_ul8_814ocl_image2d_rwDv2_iDv64_mDv8_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <64 x i64> %data, <8 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul8_16Dv16_14ocl_image2d_rwDv32_iDv128_mDv16_j(<16 x ptr addrspace(1)> %img, <32 x i32> %coord, <128 x i64> %data, <16 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul8_1614ocl_image2d_rwDv2_iDv128_mDv16_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <128 x i64> %data, <16 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul8_32Dv32_14ocl_image2d_rwDv64_iDv256_mDv32_j(<32 x ptr addrspace(1)> %img, <64 x i32> %coord, <256 x i64> %data, <32 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul8_3214ocl_image2d_rwDv2_iDv256_mDv32_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <256 x i64> %data, <32 x i32> %vec_mask)

; CHECK-DAG: define void @_Z34intel_sub_group_block_write_ul8_64Dv64_14ocl_image2d_rwDv128_iDv512_mDv64_j(<64 x ptr addrspace(1)> %img, <128 x i32> %coord, <512 x i64> %data, <64 x i32> %vec_mask)
; CHECK-DAG: call void @_Z34intel_sub_group_block_write_ul8_6414ocl_image2d_rwDv2_iDv512_mDv64_j(ptr addrspace(1) %img.vecext, <2 x i32> %coord.vecext, <512 x i64> %data, <64 x i32> %vec_mask)

; CHECK-DAG: attributes [[ATTR_RO_128]] = { nofree norecurse nounwind memory(read) "denormal-fp-math"="dynamic,dynamic" "min-legal-vector-width"="128" }
