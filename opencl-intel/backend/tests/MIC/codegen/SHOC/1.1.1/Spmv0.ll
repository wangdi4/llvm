; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@spmv_csr_vector_kernel.partialSums = internal addrspace(3) global [128 x float] zeroinitializer, align 16

declare void @__spmv_csr_scalar_kernel_original(float addrspace(1)* noalias nocapture, float addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32, float addrspace(1)* noalias nocapture) nounwind

declare i64 @get_global_id(i32) nounwind readnone

declare void @__spmv_csr_vector_kernel_original(float addrspace(1)* noalias nocapture, float addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32, float addrspace(1)* noalias nocapture) nounwind

declare i64 @get_local_id(i32) nounwind readnone

declare i64 @get_local_size(i32) nounwind readnone

declare i64 @get_group_id(i32) nounwind readnone

declare void @barrier(i64)

declare void @__spmv_ellpackr_kernel_original(float addrspace(1)* noalias nocapture, float addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32, float addrspace(1)* noalias nocapture) nounwind

declare [7 x i64] @__WG.boundaries.spmv_csr_scalar_kernel_original(float addrspace(1)*, float addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, float addrspace(1)*)

declare i64 @get_base_global_id.(i32)

declare [7 x i64] @__WG.boundaries.spmv_ellpackr_kernel_original(float addrspace(1)*, float addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, float addrspace(1)*)

declare void @____Vectorized_.spmv_csr_vector_kernel_original(float addrspace(1)* noalias nocapture, float addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32, float addrspace(1)* noalias nocapture) nounwind

declare i1 @allOne(i1)

declare i1 @allZero(i1)

declare i32 @masked_load_align4_0(i1, i32 addrspace(1)*)

declare float @masked_load_align4_1(i1, float addrspace(1)*)

declare float @masked_load_align4_2(i1, float addrspace(1)*)

declare i1 @allZero_v16(<16 x i1>)

declare i1 @allOne_v16(<16 x i1>)

declare i32 @masked_load_align4_6(i1, i32 addrspace(1)*)

declare i32 @masked_load_align4_7(i1, i32 addrspace(1)*)

declare i32 @masked_load_align4_8(i1, i32 addrspace(1)*)

declare float @masked_load_align4_9(i1, float addrspace(1)*)

declare float @masked_load_align4_10(i1, float addrspace(1)*)

declare void @masked_store_align4_0(i1, float, float addrspace(3)*)

declare void @maskedf_0_barrier(i1, i64)

declare float @masked_load_align4_11(i1, float addrspace(3)*)

declare float @masked_load_align4_12(i1, float addrspace(3)*)

declare void @masked_store_align4_1(i1, float, float addrspace(3)*)

declare void @maskedf_1_barrier(i1, i64)

declare float @masked_load_align4_13(i1, float addrspace(3)*)

declare float @masked_load_align4_14(i1, float addrspace(3)*)

declare void @masked_store_align4_2(i1, float, float addrspace(3)*)

declare void @maskedf_2_barrier(i1, i64)

declare float @masked_load_align4_15(i1, float addrspace(3)*)

declare float @masked_load_align4_16(i1, float addrspace(3)*)

declare void @masked_store_align4_3(i1, float, float addrspace(3)*)

declare void @maskedf_3_barrier(i1, i64)

declare float @masked_load_align4_17(i1, float addrspace(3)*)

declare float @masked_load_align4_18(i1, float addrspace(3)*)

declare void @masked_store_align4_4(i1, float, float addrspace(3)*)

declare void @maskedf_4_barrier(i1, i64)

declare float @masked_load_align4_19(i1, float addrspace(3)*)

declare float @masked_load_align4_20(i1, float addrspace(3)*)

declare void @masked_store_align4_5(i1, float, float addrspace(3)*)

declare void @maskedf_5_barrier(i1, i64)

declare float @masked_load_align4_21(i1, float addrspace(3)*)

declare void @masked_store_align4_6(i1, float, float addrspace(1)*)

declare void @masked_store_align4_7(<16 x i1>, <16 x float>, <16 x float> addrspace(3)*)

declare <16 x float> @masked_load_align4_27(<16 x i1>, <16 x float> addrspace(3)*)

declare <16 x float> @masked_load_align4_28(<16 x i1>, <16 x float> addrspace(3)*)

declare void @masked_store_align4_8(<16 x i1>, <16 x float>, <16 x float> addrspace(3)*)

declare <16 x float> @masked_load_align4_29(<16 x i1>, <16 x float> addrspace(3)*)

declare <16 x float> @masked_load_align4_30(<16 x i1>, <16 x float> addrspace(3)*)

declare void @masked_store_align4_9(<16 x i1>, <16 x float>, <16 x float> addrspace(3)*)

declare <16 x float> @masked_load_align4_31(<16 x i1>, <16 x float> addrspace(3)*)

declare <16 x float> @masked_load_align4_32(<16 x i1>, <16 x float> addrspace(3)*)

declare void @masked_store_align4_10(<16 x i1>, <16 x float>, <16 x float> addrspace(3)*)

declare <16 x float> @masked_load_align4_33(<16 x i1>, <16 x float> addrspace(3)*)

declare <16 x float> @masked_load_align4_34(<16 x i1>, <16 x float> addrspace(3)*)

declare void @masked_store_align4_11(<16 x i1>, <16 x float>, <16 x float> addrspace(3)*)

declare <16 x float> @masked_load_align4_35(<16 x i1>, <16 x float> addrspace(3)*)

declare <16 x float> @masked_load_align4_36(<16 x i1>, <16 x float> addrspace(3)*)

declare void @masked_store_align4_12(<16 x i1>, <16 x float>, <16 x float> addrspace(3)*)

declare <16 x float> @masked_load_align4_37(<16 x i1>, <16 x float> addrspace(3)*)

declare float @masked_load_align4_38(i1, float addrspace(1)*)

declare i32 @masked_load_align4_39(i1, i32 addrspace(1)*)

declare float @masked_load_align4_40(i1, float addrspace(1)*)

declare <16 x float> @masked_load_align4_41(<16 x i1>, <16 x float> addrspace(1)*)

declare <16 x i32> @masked_load_align4_42(<16 x i1>, <16 x i32> addrspace(1)*)

declare void @dummybarrier.()

declare i8* @get_special_buffer.() nounwind readnone

declare i64 @get_iter_count.() nounwind readnone

declare i64 @get_new_local_id.(i32, i64) nounwind readnone

declare void @__spmv_csr_scalar_kernel_separated_args(float addrspace(1)* noalias nocapture, float addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32, float addrspace(1)* noalias nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare void @__spmv_csr_vector_kernel_separated_args(float addrspace(1)* noalias nocapture, float addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32, float addrspace(1)* noalias nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare void @__spmv_ellpackr_kernel_separated_args(float addrspace(1)* noalias nocapture, float addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32, float addrspace(1)* noalias nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare [7 x i64] @WG.boundaries.spmv_csr_scalar_kernel(float addrspace(1)*, float addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, float addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)

declare [7 x i64] @WG.boundaries.spmv_ellpackr_kernel(float addrspace(1)*, float addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, float addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)

declare void @____Vectorized_.spmv_csr_vector_kernel_separated_args(float addrspace(1)* noalias nocapture, float addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32, float addrspace(1)* noalias nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare i32 @llvm.x86.mic.kortestc(i16, i16) nounwind readnone

declare i32 @llvm.x86.mic.kortestz(i16, i16) nounwind readnone

define void @spmv_ellpackr_kernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32 addrspace(1)**
  %7 = load i32 addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32 addrspace(1)**
  %10 = load i32 addrspace(1)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to float addrspace(1)**
  %16 = load float addrspace(1)** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 56
  %18 = bitcast i8* %17 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %19 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 72
  %21 = bitcast i8* %20 to <{ [4 x i64] }>**
  %22 = load <{ [4 x i64] }>** %21, align 8
  %23 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %24 = load i64* %23, align 8
  %25 = getelementptr <{ [4 x i64] }>* %22, i64 0, i32 0, i64 0
  %26 = load i64* %25, align 8
  %27 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 1
  %28 = load i64* %27, align 8
  %29 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 2
  %30 = load i64* %29, align 8
  %31 = sext i32 %13 to i64
  %32 = add i64 %24, %26
  %33 = icmp slt i64 %32, %31
  %34 = select i1 %33, i64 %32, i64 %31
  %35 = sub i64 %34, %26
  %36 = icmp sgt i64 %35, 0
  br i1 %36, label %WGLoopsEntry.i, label %__spmv_ellpackr_kernel_separated_args.exit

WGLoopsEntry.i:                                   ; preds = %entry
  %vector.size.i = ashr i64 %35, 4
  %num.vector.wi.i = and i64 %35, -16
  %max.vector.gid.i = add i64 %num.vector.wi.i, %26
  %scalar.size.i = sub i64 %35, %num.vector.wi.i
  %37 = icmp eq i64 %vector.size.i, 0
  br i1 %37, label %scalarIf.i, label %dim_1_vector_pre_head.i

dim_1_vector_pre_head.i:                          ; preds = %dim_1_vector_exit.i, %WGLoopsEntry.i
  %dim_2_vector_ind_var.i = phi i64 [ %dim_2_vector_inc_ind_var.i, %dim_1_vector_exit.i ], [ 0, %WGLoopsEntry.i ]
  br label %dim_0_vector_pre_head.i

dim_0_vector_pre_head.i:                          ; preds = %dim_0_vector_exit.i, %dim_1_vector_pre_head.i
  %dim_1_vector_ind_var.i = phi i64 [ 0, %dim_1_vector_pre_head.i ], [ %dim_1_vector_inc_ind_var.i, %dim_0_vector_exit.i ]
  br label %entryvector_func.i

entryvector_func.i:                               ; preds = %for.endvector_func.i, %dim_0_vector_pre_head.i
  %dim_0_vector_ind_var.i = phi i64 [ 0, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_ind_var.i, %for.endvector_func.i ]
  %dim_0_vector_tid.i = phi i64 [ %26, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_tid.i, %for.endvector_func.i ]
  %broadcast1vector_func.i = insertelement <16 x i64> undef, i64 %dim_0_vector_tid.i, i32 0
  %broadcast2vector_func.i = shufflevector <16 x i64> %broadcast1vector_func.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %38 = add <16 x i64> %broadcast2vector_func.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %conv8vector_func.i = trunc <16 x i64> %38 to <16 x i32>
  %idxprom9vector_func.i = sext <16 x i32> %conv8vector_func.i to <16 x i64>
  %extractvector_func.i = extractelement <16 x i64> %idxprom9vector_func.i, i32 0
  %39 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extractvector_func.i
  %ptrTypeCastvector_func.i = bitcast i32 addrspace(1)* %39 to <16 x i32> addrspace(1)*
  %40 = load <16 x i32> addrspace(1)* %ptrTypeCastvector_func.i, align 4
  %cmp21vector_func.i = icmp sgt <16 x i32> %40, zeroinitializer
  %ipred.i1.i = bitcast <16 x i1> %cmp21vector_func.i to i16
  %val.i2.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i1.i, i16 %ipred.i1.i) nounwind
  %41 = and i32 %val.i2.i, 1
  %res.i3.i = icmp eq i32 %41, 0
  br i1 %res.i3.i, label %for.bodyvector_func.preheader.i, label %for.endvector_func.i

for.bodyvector_func.preheader.i:                  ; preds = %entryvector_func.i
  %negIncomingLoopMask28vector_func.i = xor <16 x i1> %cmp21vector_func.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  br label %for.bodyvector_func.i

for.bodyvector_func.i:                            ; preds = %postload294vector_func.i, %for.bodyvector_func.preheader.i
  %vectorPHIvector_func.i = phi <16 x i1> [ %loop_mask1109vector_func.i, %postload294vector_func.i ], [ %negIncomingLoopMask28vector_func.i, %for.bodyvector_func.preheader.i ]
  %vectorPHI30vector_func.i = phi <16 x float> [ %out_sel103vector_func.i, %postload294vector_func.i ], [ undef, %for.bodyvector_func.preheader.i ]
  %vectorPHI31vector_func.i = phi <16 x i1> [ %local_edge111vector_func.i, %postload294vector_func.i ], [ %cmp21vector_func.i, %for.bodyvector_func.preheader.i ]
  %indvars.ivvector_func.i = phi i64 [ %indvars.iv.nextvector_func.i, %postload294vector_func.i ], [ 0, %for.bodyvector_func.preheader.i ]
  %vectorPHI32vector_func.i = phi <16 x float> [ %add11102vector_func.i, %postload294vector_func.i ], [ zeroinitializer, %for.bodyvector_func.preheader.i ]
  %extract70vector_func.i = extractelement <16 x i1> %vectorPHI31vector_func.i, i32 0
  %extract71vector_func.i = extractelement <16 x i1> %vectorPHI31vector_func.i, i32 1
  %extract72vector_func.i = extractelement <16 x i1> %vectorPHI31vector_func.i, i32 2
  %extract73vector_func.i = extractelement <16 x i1> %vectorPHI31vector_func.i, i32 3
  %extract74vector_func.i = extractelement <16 x i1> %vectorPHI31vector_func.i, i32 4
  %extract75vector_func.i = extractelement <16 x i1> %vectorPHI31vector_func.i, i32 5
  %extract76vector_func.i = extractelement <16 x i1> %vectorPHI31vector_func.i, i32 6
  %extract77vector_func.i = extractelement <16 x i1> %vectorPHI31vector_func.i, i32 7
  %extract78vector_func.i = extractelement <16 x i1> %vectorPHI31vector_func.i, i32 8
  %extract79vector_func.i = extractelement <16 x i1> %vectorPHI31vector_func.i, i32 9
  %extract80vector_func.i = extractelement <16 x i1> %vectorPHI31vector_func.i, i32 10
  %extract81vector_func.i = extractelement <16 x i1> %vectorPHI31vector_func.i, i32 11
  %extract82vector_func.i = extractelement <16 x i1> %vectorPHI31vector_func.i, i32 12
  %extract83vector_func.i = extractelement <16 x i1> %vectorPHI31vector_func.i, i32 13
  %extract84vector_func.i = extractelement <16 x i1> %vectorPHI31vector_func.i, i32 14
  %extract85vector_func.i = extractelement <16 x i1> %vectorPHI31vector_func.i, i32 15
  %42 = trunc i64 %indvars.ivvector_func.i to i32
  %mulvector_func.i = mul nsw i32 %42, %13
  %tempvector_func.i = insertelement <16 x i32> undef, i32 %mulvector_func.i, i32 0
  %vectorvector_func.i = shufflevector <16 x i32> %tempvector_func.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %add33vector_func.i = add nsw <16 x i32> %vectorvector_func.i, %conv8vector_func.i
  %idxprom434vector_func.i = sext <16 x i32> %add33vector_func.i to <16 x i64>
  %extract35vector_func.i = extractelement <16 x i64> %idxprom434vector_func.i, i32 0
  br i1 %extract70vector_func.i, label %preload272vector_func.i, label %postload273vector_func.i

preload272vector_func.i:                          ; preds = %for.bodyvector_func.i
  %43 = getelementptr inbounds float addrspace(1)* %1, i64 %extract35vector_func.i
  %vload116vector_func.i = load float addrspace(1)* %43, align 4
  br label %postload273vector_func.i

postload273vector_func.i:                         ; preds = %preload272vector_func.i, %for.bodyvector_func.i
  %phi274vector_func.i = phi float [ undef, %for.bodyvector_func.i ], [ %vload116vector_func.i, %preload272vector_func.i ]
  %vpackvector_func.i = insertelement <16 x float> undef, float %phi274vector_func.i, i32 0
  br i1 %extract71vector_func.i, label %preloadvector_func.i, label %postloadvector_func.i

preloadvector_func.i:                             ; preds = %postload273vector_func.i
  %.sum426vector_func.i = add i64 %extract35vector_func.i, 1
  %44 = getelementptr float addrspace(1)* %1, i64 %.sum426vector_func.i
  %vload119vector_func.i = load float addrspace(1)* %44, align 4
  br label %postloadvector_func.i

postloadvector_func.i:                            ; preds = %preloadvector_func.i, %postload273vector_func.i
  %phivector_func.i = phi float [ undef, %postload273vector_func.i ], [ %vload119vector_func.i, %preloadvector_func.i ]
  %vpack120vector_func.i = insertelement <16 x float> %vpackvector_func.i, float %phivector_func.i, i32 1
  br i1 %extract72vector_func.i, label %preload335vector_func.i, label %postload336vector_func.i

preload335vector_func.i:                          ; preds = %postloadvector_func.i
  %.sum425vector_func.i = add i64 %extract35vector_func.i, 2
  %45 = getelementptr float addrspace(1)* %1, i64 %.sum425vector_func.i
  %vload123vector_func.i = load float addrspace(1)* %45, align 4
  br label %postload336vector_func.i

postload336vector_func.i:                         ; preds = %preload335vector_func.i, %postloadvector_func.i
  %phi337vector_func.i = phi float [ undef, %postloadvector_func.i ], [ %vload123vector_func.i, %preload335vector_func.i ]
  %vpack124vector_func.i = insertelement <16 x float> %vpack120vector_func.i, float %phi337vector_func.i, i32 2
  br i1 %extract73vector_func.i, label %preload302vector_func.i, label %postload303vector_func.i

preload302vector_func.i:                          ; preds = %postload336vector_func.i
  %.sum424vector_func.i = add i64 %extract35vector_func.i, 3
  %46 = getelementptr float addrspace(1)* %1, i64 %.sum424vector_func.i
  %vload127vector_func.i = load float addrspace(1)* %46, align 4
  br label %postload303vector_func.i

postload303vector_func.i:                         ; preds = %preload302vector_func.i, %postload336vector_func.i
  %phi304vector_func.i = phi float [ undef, %postload336vector_func.i ], [ %vload127vector_func.i, %preload302vector_func.i ]
  %vpack128vector_func.i = insertelement <16 x float> %vpack124vector_func.i, float %phi304vector_func.i, i32 3
  br i1 %extract74vector_func.i, label %preload257vector_func.i, label %postload258vector_func.i

preload257vector_func.i:                          ; preds = %postload303vector_func.i
  %.sum423vector_func.i = add i64 %extract35vector_func.i, 4
  %47 = getelementptr float addrspace(1)* %1, i64 %.sum423vector_func.i
  %vload131vector_func.i = load float addrspace(1)* %47, align 4
  br label %postload258vector_func.i

postload258vector_func.i:                         ; preds = %preload257vector_func.i, %postload303vector_func.i
  %phi259vector_func.i = phi float [ undef, %postload303vector_func.i ], [ %vload131vector_func.i, %preload257vector_func.i ]
  %vpack132vector_func.i = insertelement <16 x float> %vpack128vector_func.i, float %phi259vector_func.i, i32 4
  br i1 %extract75vector_func.i, label %preload329vector_func.i, label %postload330vector_func.i

preload329vector_func.i:                          ; preds = %postload258vector_func.i
  %.sum422vector_func.i = add i64 %extract35vector_func.i, 5
  %48 = getelementptr float addrspace(1)* %1, i64 %.sum422vector_func.i
  %vload135vector_func.i = load float addrspace(1)* %48, align 4
  br label %postload330vector_func.i

postload330vector_func.i:                         ; preds = %preload329vector_func.i, %postload258vector_func.i
  %phi331vector_func.i = phi float [ undef, %postload258vector_func.i ], [ %vload135vector_func.i, %preload329vector_func.i ]
  %vpack136vector_func.i = insertelement <16 x float> %vpack132vector_func.i, float %phi331vector_func.i, i32 5
  br i1 %extract76vector_func.i, label %preload377vector_func.i, label %postload378vector_func.i

preload377vector_func.i:                          ; preds = %postload330vector_func.i
  %.sum421vector_func.i = add i64 %extract35vector_func.i, 6
  %49 = getelementptr float addrspace(1)* %1, i64 %.sum421vector_func.i
  %vload139vector_func.i = load float addrspace(1)* %49, align 4
  br label %postload378vector_func.i

postload378vector_func.i:                         ; preds = %preload377vector_func.i, %postload330vector_func.i
  %phi379vector_func.i = phi float [ undef, %postload330vector_func.i ], [ %vload139vector_func.i, %preload377vector_func.i ]
  %vpack140vector_func.i = insertelement <16 x float> %vpack136vector_func.i, float %phi379vector_func.i, i32 6
  br i1 %extract77vector_func.i, label %preload392vector_func.i, label %postload393vector_func.i

preload392vector_func.i:                          ; preds = %postload378vector_func.i
  %.sum420vector_func.i = add i64 %extract35vector_func.i, 7
  %50 = getelementptr float addrspace(1)* %1, i64 %.sum420vector_func.i
  %vload143vector_func.i = load float addrspace(1)* %50, align 4
  br label %postload393vector_func.i

postload393vector_func.i:                         ; preds = %preload392vector_func.i, %postload378vector_func.i
  %phi394vector_func.i = phi float [ undef, %postload378vector_func.i ], [ %vload143vector_func.i, %preload392vector_func.i ]
  %vpack144vector_func.i = insertelement <16 x float> %vpack140vector_func.i, float %phi394vector_func.i, i32 7
  br i1 %extract78vector_func.i, label %preload341vector_func.i, label %postload342vector_func.i

preload341vector_func.i:                          ; preds = %postload393vector_func.i
  %.sum419vector_func.i = add i64 %extract35vector_func.i, 8
  %51 = getelementptr float addrspace(1)* %1, i64 %.sum419vector_func.i
  %vload147vector_func.i = load float addrspace(1)* %51, align 4
  br label %postload342vector_func.i

postload342vector_func.i:                         ; preds = %preload341vector_func.i, %postload393vector_func.i
  %phi343vector_func.i = phi float [ undef, %postload393vector_func.i ], [ %vload147vector_func.i, %preload341vector_func.i ]
  %vpack148vector_func.i = insertelement <16 x float> %vpack144vector_func.i, float %phi343vector_func.i, i32 8
  br i1 %extract79vector_func.i, label %preload299vector_func.i, label %postload300vector_func.i

preload299vector_func.i:                          ; preds = %postload342vector_func.i
  %.sum418vector_func.i = add i64 %extract35vector_func.i, 9
  %52 = getelementptr float addrspace(1)* %1, i64 %.sum418vector_func.i
  %vload151vector_func.i = load float addrspace(1)* %52, align 4
  br label %postload300vector_func.i

postload300vector_func.i:                         ; preds = %preload299vector_func.i, %postload342vector_func.i
  %phi301vector_func.i = phi float [ undef, %postload342vector_func.i ], [ %vload151vector_func.i, %preload299vector_func.i ]
  %vpack152vector_func.i = insertelement <16 x float> %vpack148vector_func.i, float %phi301vector_func.i, i32 9
  br i1 %extract80vector_func.i, label %preload260vector_func.i, label %postload261vector_func.i

preload260vector_func.i:                          ; preds = %postload300vector_func.i
  %.sum417vector_func.i = add i64 %extract35vector_func.i, 10
  %53 = getelementptr float addrspace(1)* %1, i64 %.sum417vector_func.i
  %vload155vector_func.i = load float addrspace(1)* %53, align 4
  br label %postload261vector_func.i

postload261vector_func.i:                         ; preds = %preload260vector_func.i, %postload300vector_func.i
  %phi262vector_func.i = phi float [ undef, %postload300vector_func.i ], [ %vload155vector_func.i, %preload260vector_func.i ]
  %vpack156vector_func.i = insertelement <16 x float> %vpack152vector_func.i, float %phi262vector_func.i, i32 10
  br i1 %extract81vector_func.i, label %preload332vector_func.i, label %postload333vector_func.i

preload332vector_func.i:                          ; preds = %postload261vector_func.i
  %.sum416vector_func.i = add i64 %extract35vector_func.i, 11
  %54 = getelementptr float addrspace(1)* %1, i64 %.sum416vector_func.i
  %vload159vector_func.i = load float addrspace(1)* %54, align 4
  br label %postload333vector_func.i

postload333vector_func.i:                         ; preds = %preload332vector_func.i, %postload261vector_func.i
  %phi334vector_func.i = phi float [ undef, %postload261vector_func.i ], [ %vload159vector_func.i, %preload332vector_func.i ]
  %vpack160vector_func.i = insertelement <16 x float> %vpack156vector_func.i, float %phi334vector_func.i, i32 11
  br i1 %extract82vector_func.i, label %preload326vector_func.i, label %postload327vector_func.i

preload326vector_func.i:                          ; preds = %postload333vector_func.i
  %.sum415vector_func.i = add i64 %extract35vector_func.i, 12
  %55 = getelementptr float addrspace(1)* %1, i64 %.sum415vector_func.i
  %vload163vector_func.i = load float addrspace(1)* %55, align 4
  br label %postload327vector_func.i

postload327vector_func.i:                         ; preds = %preload326vector_func.i, %postload333vector_func.i
  %phi328vector_func.i = phi float [ undef, %postload333vector_func.i ], [ %vload163vector_func.i, %preload326vector_func.i ]
  %vpack164vector_func.i = insertelement <16 x float> %vpack160vector_func.i, float %phi328vector_func.i, i32 12
  br i1 %extract83vector_func.i, label %preload266vector_func.i, label %postload267vector_func.i

preload266vector_func.i:                          ; preds = %postload327vector_func.i
  %.sum414vector_func.i = add i64 %extract35vector_func.i, 13
  %56 = getelementptr float addrspace(1)* %1, i64 %.sum414vector_func.i
  %vload167vector_func.i = load float addrspace(1)* %56, align 4
  br label %postload267vector_func.i

postload267vector_func.i:                         ; preds = %preload266vector_func.i, %postload327vector_func.i
  %phi268vector_func.i = phi float [ undef, %postload327vector_func.i ], [ %vload167vector_func.i, %preload266vector_func.i ]
  %vpack168vector_func.i = insertelement <16 x float> %vpack164vector_func.i, float %phi268vector_func.i, i32 13
  br i1 %extract84vector_func.i, label %preload350vector_func.i, label %postload351vector_func.i

preload350vector_func.i:                          ; preds = %postload267vector_func.i
  %.sum413vector_func.i = add i64 %extract35vector_func.i, 14
  %57 = getelementptr float addrspace(1)* %1, i64 %.sum413vector_func.i
  %vload171vector_func.i = load float addrspace(1)* %57, align 4
  br label %postload351vector_func.i

postload351vector_func.i:                         ; preds = %preload350vector_func.i, %postload267vector_func.i
  %phi352vector_func.i = phi float [ undef, %postload267vector_func.i ], [ %vload171vector_func.i, %preload350vector_func.i ]
  %vpack172vector_func.i = insertelement <16 x float> %vpack168vector_func.i, float %phi352vector_func.i, i32 14
  br i1 %extract85vector_func.i, label %preload371vector_func.i, label %postload372vector_func.i

preload371vector_func.i:                          ; preds = %postload351vector_func.i
  %.sum412vector_func.i = add i64 %extract35vector_func.i, 15
  %58 = getelementptr float addrspace(1)* %1, i64 %.sum412vector_func.i
  %vload175vector_func.i = load float addrspace(1)* %58, align 4
  br label %postload372vector_func.i

postload372vector_func.i:                         ; preds = %preload371vector_func.i, %postload351vector_func.i
  %phi373vector_func.i = phi float [ undef, %postload351vector_func.i ], [ %vload175vector_func.i, %preload371vector_func.i ]
  %vpack176vector_func.i = insertelement <16 x float> %vpack172vector_func.i, float %phi373vector_func.i, i32 15
  br i1 %extract70vector_func.i, label %preload344vector_func.i, label %postload345vector_func.i

preload344vector_func.i:                          ; preds = %postload372vector_func.i
  %59 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract35vector_func.i
  %vload180vector_func.i = load i32 addrspace(1)* %59, align 4
  br label %postload345vector_func.i

postload345vector_func.i:                         ; preds = %preload344vector_func.i, %postload372vector_func.i
  %phi346vector_func.i = phi i32 [ undef, %postload372vector_func.i ], [ %vload180vector_func.i, %preload344vector_func.i ]
  %vpack181vector_func.i = insertelement <16 x i32> undef, i32 %phi346vector_func.i, i32 0
  br i1 %extract71vector_func.i, label %preload347vector_func.i, label %postload348vector_func.i

preload347vector_func.i:                          ; preds = %postload345vector_func.i
  %.sum411vector_func.i = add i64 %extract35vector_func.i, 1
  %60 = getelementptr i32 addrspace(1)* %7, i64 %.sum411vector_func.i
  %vload184vector_func.i = load i32 addrspace(1)* %60, align 4
  br label %postload348vector_func.i

postload348vector_func.i:                         ; preds = %preload347vector_func.i, %postload345vector_func.i
  %phi349vector_func.i = phi i32 [ undef, %postload345vector_func.i ], [ %vload184vector_func.i, %preload347vector_func.i ]
  %vpack185vector_func.i = insertelement <16 x i32> %vpack181vector_func.i, i32 %phi349vector_func.i, i32 1
  br i1 %extract72vector_func.i, label %preload353vector_func.i, label %postload354vector_func.i

preload353vector_func.i:                          ; preds = %postload348vector_func.i
  %.sum410vector_func.i = add i64 %extract35vector_func.i, 2
  %61 = getelementptr i32 addrspace(1)* %7, i64 %.sum410vector_func.i
  %vload188vector_func.i = load i32 addrspace(1)* %61, align 4
  br label %postload354vector_func.i

postload354vector_func.i:                         ; preds = %preload353vector_func.i, %postload348vector_func.i
  %phi355vector_func.i = phi i32 [ undef, %postload348vector_func.i ], [ %vload188vector_func.i, %preload353vector_func.i ]
  %vpack189vector_func.i = insertelement <16 x i32> %vpack185vector_func.i, i32 %phi355vector_func.i, i32 2
  br i1 %extract73vector_func.i, label %preload356vector_func.i, label %postload357vector_func.i

preload356vector_func.i:                          ; preds = %postload354vector_func.i
  %.sum409vector_func.i = add i64 %extract35vector_func.i, 3
  %62 = getelementptr i32 addrspace(1)* %7, i64 %.sum409vector_func.i
  %vload192vector_func.i = load i32 addrspace(1)* %62, align 4
  br label %postload357vector_func.i

postload357vector_func.i:                         ; preds = %preload356vector_func.i, %postload354vector_func.i
  %phi358vector_func.i = phi i32 [ undef, %postload354vector_func.i ], [ %vload192vector_func.i, %preload356vector_func.i ]
  %vpack193vector_func.i = insertelement <16 x i32> %vpack189vector_func.i, i32 %phi358vector_func.i, i32 3
  br i1 %extract74vector_func.i, label %preload362vector_func.i, label %postload363vector_func.i

preload362vector_func.i:                          ; preds = %postload357vector_func.i
  %.sum408vector_func.i = add i64 %extract35vector_func.i, 4
  %63 = getelementptr i32 addrspace(1)* %7, i64 %.sum408vector_func.i
  %vload196vector_func.i = load i32 addrspace(1)* %63, align 4
  br label %postload363vector_func.i

postload363vector_func.i:                         ; preds = %preload362vector_func.i, %postload357vector_func.i
  %phi364vector_func.i = phi i32 [ undef, %postload357vector_func.i ], [ %vload196vector_func.i, %preload362vector_func.i ]
  %vpack197vector_func.i = insertelement <16 x i32> %vpack193vector_func.i, i32 %phi364vector_func.i, i32 4
  br i1 %extract75vector_func.i, label %preload380vector_func.i, label %postload381vector_func.i

preload380vector_func.i:                          ; preds = %postload363vector_func.i
  %.sum407vector_func.i = add i64 %extract35vector_func.i, 5
  %64 = getelementptr i32 addrspace(1)* %7, i64 %.sum407vector_func.i
  %vload200vector_func.i = load i32 addrspace(1)* %64, align 4
  br label %postload381vector_func.i

postload381vector_func.i:                         ; preds = %preload380vector_func.i, %postload363vector_func.i
  %phi382vector_func.i = phi i32 [ undef, %postload363vector_func.i ], [ %vload200vector_func.i, %preload380vector_func.i ]
  %vpack201vector_func.i = insertelement <16 x i32> %vpack197vector_func.i, i32 %phi382vector_func.i, i32 5
  br i1 %extract76vector_func.i, label %preload374vector_func.i, label %postload375vector_func.i

preload374vector_func.i:                          ; preds = %postload381vector_func.i
  %.sum406vector_func.i = add i64 %extract35vector_func.i, 6
  %65 = getelementptr i32 addrspace(1)* %7, i64 %.sum406vector_func.i
  %vload204vector_func.i = load i32 addrspace(1)* %65, align 4
  br label %postload375vector_func.i

postload375vector_func.i:                         ; preds = %preload374vector_func.i, %postload381vector_func.i
  %phi376vector_func.i = phi i32 [ undef, %postload381vector_func.i ], [ %vload204vector_func.i, %preload374vector_func.i ]
  %vpack205vector_func.i = insertelement <16 x i32> %vpack201vector_func.i, i32 %phi376vector_func.i, i32 6
  br i1 %extract77vector_func.i, label %preload368vector_func.i, label %postload369vector_func.i

preload368vector_func.i:                          ; preds = %postload375vector_func.i
  %.sum405vector_func.i = add i64 %extract35vector_func.i, 7
  %66 = getelementptr i32 addrspace(1)* %7, i64 %.sum405vector_func.i
  %vload208vector_func.i = load i32 addrspace(1)* %66, align 4
  br label %postload369vector_func.i

postload369vector_func.i:                         ; preds = %preload368vector_func.i, %postload375vector_func.i
  %phi370vector_func.i = phi i32 [ undef, %postload375vector_func.i ], [ %vload208vector_func.i, %preload368vector_func.i ]
  %vpack209vector_func.i = insertelement <16 x i32> %vpack205vector_func.i, i32 %phi370vector_func.i, i32 7
  br i1 %extract78vector_func.i, label %preload386vector_func.i, label %postload387vector_func.i

preload386vector_func.i:                          ; preds = %postload369vector_func.i
  %.sum404vector_func.i = add i64 %extract35vector_func.i, 8
  %67 = getelementptr i32 addrspace(1)* %7, i64 %.sum404vector_func.i
  %vload212vector_func.i = load i32 addrspace(1)* %67, align 4
  br label %postload387vector_func.i

postload387vector_func.i:                         ; preds = %preload386vector_func.i, %postload369vector_func.i
  %phi388vector_func.i = phi i32 [ undef, %postload369vector_func.i ], [ %vload212vector_func.i, %preload386vector_func.i ]
  %vpack213vector_func.i = insertelement <16 x i32> %vpack209vector_func.i, i32 %phi388vector_func.i, i32 8
  br i1 %extract79vector_func.i, label %preload365vector_func.i, label %postload366vector_func.i

preload365vector_func.i:                          ; preds = %postload387vector_func.i
  %.sum403vector_func.i = add i64 %extract35vector_func.i, 9
  %68 = getelementptr i32 addrspace(1)* %7, i64 %.sum403vector_func.i
  %vload216vector_func.i = load i32 addrspace(1)* %68, align 4
  br label %postload366vector_func.i

postload366vector_func.i:                         ; preds = %preload365vector_func.i, %postload387vector_func.i
  %phi367vector_func.i = phi i32 [ undef, %postload387vector_func.i ], [ %vload216vector_func.i, %preload365vector_func.i ]
  %vpack217vector_func.i = insertelement <16 x i32> %vpack213vector_func.i, i32 %phi367vector_func.i, i32 9
  br i1 %extract80vector_func.i, label %preload338vector_func.i, label %postload339vector_func.i

preload338vector_func.i:                          ; preds = %postload366vector_func.i
  %.sum402vector_func.i = add i64 %extract35vector_func.i, 10
  %69 = getelementptr i32 addrspace(1)* %7, i64 %.sum402vector_func.i
  %vload220vector_func.i = load i32 addrspace(1)* %69, align 4
  br label %postload339vector_func.i

postload339vector_func.i:                         ; preds = %preload338vector_func.i, %postload366vector_func.i
  %phi340vector_func.i = phi i32 [ undef, %postload366vector_func.i ], [ %vload220vector_func.i, %preload338vector_func.i ]
  %vpack221vector_func.i = insertelement <16 x i32> %vpack217vector_func.i, i32 %phi340vector_func.i, i32 10
  br i1 %extract81vector_func.i, label %preload383vector_func.i, label %postload384vector_func.i

preload383vector_func.i:                          ; preds = %postload339vector_func.i
  %.sum401vector_func.i = add i64 %extract35vector_func.i, 11
  %70 = getelementptr i32 addrspace(1)* %7, i64 %.sum401vector_func.i
  %vload224vector_func.i = load i32 addrspace(1)* %70, align 4
  br label %postload384vector_func.i

postload384vector_func.i:                         ; preds = %preload383vector_func.i, %postload339vector_func.i
  %phi385vector_func.i = phi i32 [ undef, %postload339vector_func.i ], [ %vload224vector_func.i, %preload383vector_func.i ]
  %vpack225vector_func.i = insertelement <16 x i32> %vpack221vector_func.i, i32 %phi385vector_func.i, i32 11
  br i1 %extract82vector_func.i, label %preload263vector_func.i, label %postload264vector_func.i

preload263vector_func.i:                          ; preds = %postload384vector_func.i
  %.sum400vector_func.i = add i64 %extract35vector_func.i, 12
  %71 = getelementptr i32 addrspace(1)* %7, i64 %.sum400vector_func.i
  %vload228vector_func.i = load i32 addrspace(1)* %71, align 4
  br label %postload264vector_func.i

postload264vector_func.i:                         ; preds = %preload263vector_func.i, %postload384vector_func.i
  %phi265vector_func.i = phi i32 [ undef, %postload384vector_func.i ], [ %vload228vector_func.i, %preload263vector_func.i ]
  %vpack229vector_func.i = insertelement <16 x i32> %vpack225vector_func.i, i32 %phi265vector_func.i, i32 12
  br i1 %extract83vector_func.i, label %preload359vector_func.i, label %postload360vector_func.i

preload359vector_func.i:                          ; preds = %postload264vector_func.i
  %.sum399vector_func.i = add i64 %extract35vector_func.i, 13
  %72 = getelementptr i32 addrspace(1)* %7, i64 %.sum399vector_func.i
  %vload232vector_func.i = load i32 addrspace(1)* %72, align 4
  br label %postload360vector_func.i

postload360vector_func.i:                         ; preds = %preload359vector_func.i, %postload264vector_func.i
  %phi361vector_func.i = phi i32 [ undef, %postload264vector_func.i ], [ %vload232vector_func.i, %preload359vector_func.i ]
  %vpack233vector_func.i = insertelement <16 x i32> %vpack229vector_func.i, i32 %phi361vector_func.i, i32 13
  br i1 %extract84vector_func.i, label %preload296vector_func.i, label %postload297vector_func.i

preload296vector_func.i:                          ; preds = %postload360vector_func.i
  %.sum398vector_func.i = add i64 %extract35vector_func.i, 14
  %73 = getelementptr i32 addrspace(1)* %7, i64 %.sum398vector_func.i
  %vload236vector_func.i = load i32 addrspace(1)* %73, align 4
  br label %postload297vector_func.i

postload297vector_func.i:                         ; preds = %preload296vector_func.i, %postload360vector_func.i
  %phi298vector_func.i = phi i32 [ undef, %postload360vector_func.i ], [ %vload236vector_func.i, %preload296vector_func.i ]
  %vpack237vector_func.i = insertelement <16 x i32> %vpack233vector_func.i, i32 %phi298vector_func.i, i32 14
  br i1 %extract85vector_func.i, label %preload269vector_func.i, label %postload270vector_func.i

preload269vector_func.i:                          ; preds = %postload297vector_func.i
  %.sumvector_func.i = add i64 %extract35vector_func.i, 15
  %74 = getelementptr i32 addrspace(1)* %7, i64 %.sumvector_func.i
  %vload240vector_func.i = load i32 addrspace(1)* %74, align 4
  br label %postload270vector_func.i

postload270vector_func.i:                         ; preds = %preload269vector_func.i, %postload297vector_func.i
  %phi271vector_func.i = phi i32 [ undef, %postload297vector_func.i ], [ %vload240vector_func.i, %preload269vector_func.i ]
  %vpack241vector_func.i = insertelement <16 x i32> %vpack237vector_func.i, i32 %phi271vector_func.i, i32 15
  %idxprom853vector_func.i = sext <16 x i32> %vpack241vector_func.i to <16 x i64>
  %extract55vector_func.i = extractelement <16 x i64> %idxprom853vector_func.i, i32 1
  %extract56vector_func.i = extractelement <16 x i64> %idxprom853vector_func.i, i32 2
  %extract57vector_func.i = extractelement <16 x i64> %idxprom853vector_func.i, i32 3
  %extract58vector_func.i = extractelement <16 x i64> %idxprom853vector_func.i, i32 4
  %extract59vector_func.i = extractelement <16 x i64> %idxprom853vector_func.i, i32 5
  %extract60vector_func.i = extractelement <16 x i64> %idxprom853vector_func.i, i32 6
  %extract61vector_func.i = extractelement <16 x i64> %idxprom853vector_func.i, i32 7
  %extract62vector_func.i = extractelement <16 x i64> %idxprom853vector_func.i, i32 8
  %extract63vector_func.i = extractelement <16 x i64> %idxprom853vector_func.i, i32 9
  %extract64vector_func.i = extractelement <16 x i64> %idxprom853vector_func.i, i32 10
  %extract65vector_func.i = extractelement <16 x i64> %idxprom853vector_func.i, i32 11
  %extract66vector_func.i = extractelement <16 x i64> %idxprom853vector_func.i, i32 12
  %extract67vector_func.i = extractelement <16 x i64> %idxprom853vector_func.i, i32 13
  %extract68vector_func.i = extractelement <16 x i64> %idxprom853vector_func.i, i32 14
  %extract69vector_func.i = extractelement <16 x i64> %idxprom853vector_func.i, i32 15
  %75 = getelementptr inbounds float addrspace(1)* %4, i64 %extract55vector_func.i
  %76 = getelementptr inbounds float addrspace(1)* %4, i64 %extract56vector_func.i
  %77 = getelementptr inbounds float addrspace(1)* %4, i64 %extract57vector_func.i
  %78 = getelementptr inbounds float addrspace(1)* %4, i64 %extract58vector_func.i
  %79 = getelementptr inbounds float addrspace(1)* %4, i64 %extract59vector_func.i
  %80 = getelementptr inbounds float addrspace(1)* %4, i64 %extract60vector_func.i
  %81 = getelementptr inbounds float addrspace(1)* %4, i64 %extract61vector_func.i
  %82 = getelementptr inbounds float addrspace(1)* %4, i64 %extract62vector_func.i
  %83 = getelementptr inbounds float addrspace(1)* %4, i64 %extract63vector_func.i
  %84 = getelementptr inbounds float addrspace(1)* %4, i64 %extract64vector_func.i
  %85 = getelementptr inbounds float addrspace(1)* %4, i64 %extract65vector_func.i
  %86 = getelementptr inbounds float addrspace(1)* %4, i64 %extract66vector_func.i
  %87 = getelementptr inbounds float addrspace(1)* %4, i64 %extract67vector_func.i
  %88 = getelementptr inbounds float addrspace(1)* %4, i64 %extract68vector_func.i
  %89 = getelementptr inbounds float addrspace(1)* %4, i64 %extract69vector_func.i
  br i1 %extract70vector_func.i, label %preload395vector_func.i, label %postload396vector_func.i

preload395vector_func.i:                          ; preds = %postload270vector_func.i
  %extract54vector_func.i = extractelement <16 x i64> %idxprom853vector_func.i, i32 0
  %90 = getelementptr inbounds float addrspace(1)* %4, i64 %extract54vector_func.i
  %masked_loadvector_func.i = load float addrspace(1)* %90, align 4
  br label %postload396vector_func.i

postload396vector_func.i:                         ; preds = %preload395vector_func.i, %postload270vector_func.i
  %phi397vector_func.i = phi float [ undef, %postload270vector_func.i ], [ %masked_loadvector_func.i, %preload395vector_func.i ]
  br i1 %extract71vector_func.i, label %preload389vector_func.i, label %postload390vector_func.i

preload389vector_func.i:                          ; preds = %postload396vector_func.i
  %masked_load242vector_func.i = load float addrspace(1)* %75, align 4
  br label %postload390vector_func.i

postload390vector_func.i:                         ; preds = %preload389vector_func.i, %postload396vector_func.i
  %phi391vector_func.i = phi float [ undef, %postload396vector_func.i ], [ %masked_load242vector_func.i, %preload389vector_func.i ]
  br i1 %extract72vector_func.i, label %preload305vector_func.i, label %postload306vector_func.i

preload305vector_func.i:                          ; preds = %postload390vector_func.i
  %masked_load243vector_func.i = load float addrspace(1)* %76, align 4
  br label %postload306vector_func.i

postload306vector_func.i:                         ; preds = %preload305vector_func.i, %postload390vector_func.i
  %phi307vector_func.i = phi float [ undef, %postload390vector_func.i ], [ %masked_load243vector_func.i, %preload305vector_func.i ]
  br i1 %extract73vector_func.i, label %preload308vector_func.i, label %postload309vector_func.i

preload308vector_func.i:                          ; preds = %postload306vector_func.i
  %masked_load244vector_func.i = load float addrspace(1)* %77, align 4
  br label %postload309vector_func.i

postload309vector_func.i:                         ; preds = %preload308vector_func.i, %postload306vector_func.i
  %phi310vector_func.i = phi float [ undef, %postload306vector_func.i ], [ %masked_load244vector_func.i, %preload308vector_func.i ]
  br i1 %extract74vector_func.i, label %preload311vector_func.i, label %postload312vector_func.i

preload311vector_func.i:                          ; preds = %postload309vector_func.i
  %masked_load245vector_func.i = load float addrspace(1)* %78, align 4
  br label %postload312vector_func.i

postload312vector_func.i:                         ; preds = %preload311vector_func.i, %postload309vector_func.i
  %phi313vector_func.i = phi float [ undef, %postload309vector_func.i ], [ %masked_load245vector_func.i, %preload311vector_func.i ]
  br i1 %extract75vector_func.i, label %preload314vector_func.i, label %postload315vector_func.i

preload314vector_func.i:                          ; preds = %postload312vector_func.i
  %masked_load246vector_func.i = load float addrspace(1)* %79, align 4
  br label %postload315vector_func.i

postload315vector_func.i:                         ; preds = %preload314vector_func.i, %postload312vector_func.i
  %phi316vector_func.i = phi float [ undef, %postload312vector_func.i ], [ %masked_load246vector_func.i, %preload314vector_func.i ]
  br i1 %extract76vector_func.i, label %preload317vector_func.i, label %postload318vector_func.i

preload317vector_func.i:                          ; preds = %postload315vector_func.i
  %masked_load247vector_func.i = load float addrspace(1)* %80, align 4
  br label %postload318vector_func.i

postload318vector_func.i:                         ; preds = %preload317vector_func.i, %postload315vector_func.i
  %phi319vector_func.i = phi float [ undef, %postload315vector_func.i ], [ %masked_load247vector_func.i, %preload317vector_func.i ]
  br i1 %extract77vector_func.i, label %preload320vector_func.i, label %postload321vector_func.i

preload320vector_func.i:                          ; preds = %postload318vector_func.i
  %masked_load248vector_func.i = load float addrspace(1)* %81, align 4
  br label %postload321vector_func.i

postload321vector_func.i:                         ; preds = %preload320vector_func.i, %postload318vector_func.i
  %phi322vector_func.i = phi float [ undef, %postload318vector_func.i ], [ %masked_load248vector_func.i, %preload320vector_func.i ]
  br i1 %extract78vector_func.i, label %preload323vector_func.i, label %postload324vector_func.i

preload323vector_func.i:                          ; preds = %postload321vector_func.i
  %masked_load249vector_func.i = load float addrspace(1)* %82, align 4
  br label %postload324vector_func.i

postload324vector_func.i:                         ; preds = %preload323vector_func.i, %postload321vector_func.i
  %phi325vector_func.i = phi float [ undef, %postload321vector_func.i ], [ %masked_load249vector_func.i, %preload323vector_func.i ]
  br i1 %extract79vector_func.i, label %preload275vector_func.i, label %postload276vector_func.i

preload275vector_func.i:                          ; preds = %postload324vector_func.i
  %masked_load250vector_func.i = load float addrspace(1)* %83, align 4
  br label %postload276vector_func.i

postload276vector_func.i:                         ; preds = %preload275vector_func.i, %postload324vector_func.i
  %phi277vector_func.i = phi float [ undef, %postload324vector_func.i ], [ %masked_load250vector_func.i, %preload275vector_func.i ]
  br i1 %extract80vector_func.i, label %preload278vector_func.i, label %postload279vector_func.i

preload278vector_func.i:                          ; preds = %postload276vector_func.i
  %masked_load251vector_func.i = load float addrspace(1)* %84, align 4
  br label %postload279vector_func.i

postload279vector_func.i:                         ; preds = %preload278vector_func.i, %postload276vector_func.i
  %phi280vector_func.i = phi float [ undef, %postload276vector_func.i ], [ %masked_load251vector_func.i, %preload278vector_func.i ]
  br i1 %extract81vector_func.i, label %preload281vector_func.i, label %postload282vector_func.i

preload281vector_func.i:                          ; preds = %postload279vector_func.i
  %masked_load252vector_func.i = load float addrspace(1)* %85, align 4
  br label %postload282vector_func.i

postload282vector_func.i:                         ; preds = %preload281vector_func.i, %postload279vector_func.i
  %phi283vector_func.i = phi float [ undef, %postload279vector_func.i ], [ %masked_load252vector_func.i, %preload281vector_func.i ]
  br i1 %extract82vector_func.i, label %preload284vector_func.i, label %postload285vector_func.i

preload284vector_func.i:                          ; preds = %postload282vector_func.i
  %masked_load253vector_func.i = load float addrspace(1)* %86, align 4
  br label %postload285vector_func.i

postload285vector_func.i:                         ; preds = %preload284vector_func.i, %postload282vector_func.i
  %phi286vector_func.i = phi float [ undef, %postload282vector_func.i ], [ %masked_load253vector_func.i, %preload284vector_func.i ]
  br i1 %extract83vector_func.i, label %preload287vector_func.i, label %postload288vector_func.i

preload287vector_func.i:                          ; preds = %postload285vector_func.i
  %masked_load254vector_func.i = load float addrspace(1)* %87, align 4
  br label %postload288vector_func.i

postload288vector_func.i:                         ; preds = %preload287vector_func.i, %postload285vector_func.i
  %phi289vector_func.i = phi float [ undef, %postload285vector_func.i ], [ %masked_load254vector_func.i, %preload287vector_func.i ]
  br i1 %extract84vector_func.i, label %preload290vector_func.i, label %postload291vector_func.i

preload290vector_func.i:                          ; preds = %postload288vector_func.i
  %masked_load255vector_func.i = load float addrspace(1)* %88, align 4
  br label %postload291vector_func.i

postload291vector_func.i:                         ; preds = %preload290vector_func.i, %postload288vector_func.i
  %phi292vector_func.i = phi float [ undef, %postload288vector_func.i ], [ %masked_load255vector_func.i, %preload290vector_func.i ]
  br i1 %extract85vector_func.i, label %preload293vector_func.i, label %postload294vector_func.i

preload293vector_func.i:                          ; preds = %postload291vector_func.i
  %masked_load256vector_func.i = load float addrspace(1)* %89, align 4
  br label %postload294vector_func.i

postload294vector_func.i:                         ; preds = %preload293vector_func.i, %postload291vector_func.i
  %phi295vector_func.i = phi float [ undef, %postload291vector_func.i ], [ %masked_load256vector_func.i, %preload293vector_func.i ]
  %temp.vectvector_func.i = insertelement <16 x float> undef, float %phi397vector_func.i, i32 0
  %temp.vect86vector_func.i = insertelement <16 x float> %temp.vectvector_func.i, float %phi391vector_func.i, i32 1
  %temp.vect87vector_func.i = insertelement <16 x float> %temp.vect86vector_func.i, float %phi307vector_func.i, i32 2
  %temp.vect88vector_func.i = insertelement <16 x float> %temp.vect87vector_func.i, float %phi310vector_func.i, i32 3
  %temp.vect89vector_func.i = insertelement <16 x float> %temp.vect88vector_func.i, float %phi313vector_func.i, i32 4
  %temp.vect90vector_func.i = insertelement <16 x float> %temp.vect89vector_func.i, float %phi316vector_func.i, i32 5
  %temp.vect91vector_func.i = insertelement <16 x float> %temp.vect90vector_func.i, float %phi319vector_func.i, i32 6
  %temp.vect92vector_func.i = insertelement <16 x float> %temp.vect91vector_func.i, float %phi322vector_func.i, i32 7
  %temp.vect93vector_func.i = insertelement <16 x float> %temp.vect92vector_func.i, float %phi325vector_func.i, i32 8
  %temp.vect94vector_func.i = insertelement <16 x float> %temp.vect93vector_func.i, float %phi277vector_func.i, i32 9
  %temp.vect95vector_func.i = insertelement <16 x float> %temp.vect94vector_func.i, float %phi280vector_func.i, i32 10
  %temp.vect96vector_func.i = insertelement <16 x float> %temp.vect95vector_func.i, float %phi283vector_func.i, i32 11
  %temp.vect97vector_func.i = insertelement <16 x float> %temp.vect96vector_func.i, float %phi286vector_func.i, i32 12
  %temp.vect98vector_func.i = insertelement <16 x float> %temp.vect97vector_func.i, float %phi289vector_func.i, i32 13
  %temp.vect99vector_func.i = insertelement <16 x float> %temp.vect98vector_func.i, float %phi292vector_func.i, i32 14
  %temp.vect100vector_func.i = insertelement <16 x float> %temp.vect99vector_func.i, float %phi295vector_func.i, i32 15
  %mul10101vector_func.i = fmul <16 x float> %vpack176vector_func.i, %temp.vect100vector_func.i
  %add11102vector_func.i = fadd <16 x float> %vectorPHI32vector_func.i, %mul10101vector_func.i
  %out_sel103vector_func.i = select <16 x i1> %vectorPHI31vector_func.i, <16 x float> %add11102vector_func.i, <16 x float> %vectorPHI30vector_func.i
  %indvars.iv.nextvector_func.i = add i64 %indvars.ivvector_func.i, 1
  %lftr.wideivvector_func.i = trunc i64 %indvars.iv.nextvector_func.i to i32
  %temp104vector_func.i = insertelement <16 x i32> undef, i32 %lftr.wideivvector_func.i, i32 0
  %vector105vector_func.i = shufflevector <16 x i32> %temp104vector_func.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %exitcondvector_func.i = icmp eq <16 x i32> %vector105vector_func.i, %40
  %notCond106vector_func.i = xor <16 x i1> %exitcondvector_func.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %who_left_tr107vector_func.i = and <16 x i1> %vectorPHI31vector_func.i, %exitcondvector_func.i
  %loop_mask1109vector_func.i = or <16 x i1> %vectorPHIvector_func.i, %who_left_tr107vector_func.i
  %ipred.i.i = bitcast <16 x i1> %loop_mask1109vector_func.i to i16
  %val.i.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i.i, i16 %ipred.i.i) nounwind
  %91 = and i32 %val.i.i, 1
  %res.i.i = icmp eq i32 %91, 0
  %local_edge111vector_func.i = and <16 x i1> %vectorPHI31vector_func.i, %notCond106vector_func.i
  br i1 %res.i.i, label %for.bodyvector_func.i, label %for.endvector_func.i

for.endvector_func.i:                             ; preds = %postload294vector_func.i, %entryvector_func.i
  %vectorPHI112vector_func.i = phi <16 x float> [ undef, %entryvector_func.i ], [ %out_sel103vector_func.i, %postload294vector_func.i ]
  %merge113vector_func.i = select <16 x i1> %cmp21vector_func.i, <16 x float> %vectorPHI112vector_func.i, <16 x float> zeroinitializer
  %92 = getelementptr inbounds float addrspace(1)* %16, i64 %extractvector_func.i
  %ptrTypeCast114vector_func.i = bitcast float addrspace(1)* %92 to <16 x float> addrspace(1)*
  store <16 x float> %merge113vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast114vector_func.i, align 4
  %dim_0_vector_inc_ind_var.i = add i64 %dim_0_vector_ind_var.i, 1
  %dim_0_vector_cmp.to.max.i = icmp eq i64 %dim_0_vector_inc_ind_var.i, %vector.size.i
  %dim_0_vector_inc_tid.i = add i64 %dim_0_vector_tid.i, 16
  br i1 %dim_0_vector_cmp.to.max.i, label %dim_0_vector_exit.i, label %entryvector_func.i

dim_0_vector_exit.i:                              ; preds = %for.endvector_func.i
  %dim_1_vector_inc_ind_var.i = add i64 %dim_1_vector_ind_var.i, 1
  %dim_1_vector_cmp.to.max.i = icmp eq i64 %dim_1_vector_inc_ind_var.i, %28
  br i1 %dim_1_vector_cmp.to.max.i, label %dim_1_vector_exit.i, label %dim_0_vector_pre_head.i

dim_1_vector_exit.i:                              ; preds = %dim_0_vector_exit.i
  %dim_2_vector_inc_ind_var.i = add i64 %dim_2_vector_ind_var.i, 1
  %dim_2_vector_cmp.to.max.i = icmp eq i64 %dim_2_vector_inc_ind_var.i, %30
  br i1 %dim_2_vector_cmp.to.max.i, label %scalarIf.i, label %dim_1_vector_pre_head.i

scalarIf.i:                                       ; preds = %dim_1_vector_exit.i, %WGLoopsEntry.i
  %93 = icmp eq i64 %35, %num.vector.wi.i
  br i1 %93, label %__spmv_ellpackr_kernel_separated_args.exit, label %dim_1_pre_head.i

dim_1_pre_head.i:                                 ; preds = %dim_1_exit.i, %scalarIf.i
  %dim_2_ind_var.i = phi i64 [ %dim_2_inc_ind_var.i, %dim_1_exit.i ], [ 0, %scalarIf.i ]
  br label %dim_0_pre_head.i

dim_0_pre_head.i:                                 ; preds = %dim_0_exit.i, %dim_1_pre_head.i
  %dim_1_ind_var.i = phi i64 [ 0, %dim_1_pre_head.i ], [ %dim_1_inc_ind_var.i, %dim_0_exit.i ]
  br label %scalar_kernel_entry.i

scalar_kernel_entry.i:                            ; preds = %for.end.i, %dim_0_pre_head.i
  %dim_0_ind_var.i = phi i64 [ 0, %dim_0_pre_head.i ], [ %dim_0_inc_ind_var.i, %for.end.i ]
  %dim_0_tid.i = phi i64 [ %max.vector.gid.i, %dim_0_pre_head.i ], [ %dim_0_inc_tid.i, %for.end.i ]
  %conv.i = trunc i64 %dim_0_tid.i to i32
  %idxprom.i = sext i32 %conv.i to i64
  %arrayidx.i = getelementptr inbounds i32 addrspace(1)* %10, i64 %idxprom.i
  %94 = load i32 addrspace(1)* %arrayidx.i, align 4
  %cmp21.i = icmp sgt i32 %94, 0
  br i1 %cmp21.i, label %for.body.i, label %for.end.i

for.body.i:                                       ; preds = %for.body.i, %scalar_kernel_entry.i
  %indvars.iv.i = phi i64 [ %indvars.iv.next.i, %for.body.i ], [ 0, %scalar_kernel_entry.i ]
  %result.02.i = phi float [ %add11.i, %for.body.i ], [ 0.000000e+00, %scalar_kernel_entry.i ]
  %95 = trunc i64 %indvars.iv.i to i32
  %mul.i = mul nsw i32 %95, %13
  %add.i = add nsw i32 %mul.i, %conv.i
  %idxprom4.i = sext i32 %add.i to i64
  %arrayidx5.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom4.i
  %96 = load float addrspace(1)* %arrayidx5.i, align 4
  %arrayidx7.i = getelementptr inbounds i32 addrspace(1)* %7, i64 %idxprom4.i
  %97 = load i32 addrspace(1)* %arrayidx7.i, align 4
  %idxprom8.i = sext i32 %97 to i64
  %arrayidx9.i = getelementptr inbounds float addrspace(1)* %4, i64 %idxprom8.i
  %98 = load float addrspace(1)* %arrayidx9.i, align 4
  %mul10.i = fmul float %96, %98
  %add11.i = fadd float %result.02.i, %mul10.i
  %indvars.iv.next.i = add i64 %indvars.iv.i, 1
  %lftr.wideiv.i = trunc i64 %indvars.iv.next.i to i32
  %exitcond.i = icmp eq i32 %lftr.wideiv.i, %94
  br i1 %exitcond.i, label %for.end.i, label %for.body.i

for.end.i:                                        ; preds = %for.body.i, %scalar_kernel_entry.i
  %result.0.lcssa.i = phi float [ 0.000000e+00, %scalar_kernel_entry.i ], [ %add11.i, %for.body.i ]
  %arrayidx13.i = getelementptr inbounds float addrspace(1)* %16, i64 %idxprom.i
  store float %result.0.lcssa.i, float addrspace(1)* %arrayidx13.i, align 4
  %dim_0_inc_ind_var.i = add i64 %dim_0_ind_var.i, 1
  %dim_0_cmp.to.max.i = icmp eq i64 %dim_0_inc_ind_var.i, %scalar.size.i
  %dim_0_inc_tid.i = add i64 %dim_0_tid.i, 1
  br i1 %dim_0_cmp.to.max.i, label %dim_0_exit.i, label %scalar_kernel_entry.i

dim_0_exit.i:                                     ; preds = %for.end.i
  %dim_1_inc_ind_var.i = add i64 %dim_1_ind_var.i, 1
  %dim_1_cmp.to.max.i = icmp eq i64 %dim_1_inc_ind_var.i, %28
  br i1 %dim_1_cmp.to.max.i, label %dim_1_exit.i, label %dim_0_pre_head.i

dim_1_exit.i:                                     ; preds = %dim_0_exit.i
  %dim_2_inc_ind_var.i = add i64 %dim_2_ind_var.i, 1
  %dim_2_cmp.to.max.i = icmp eq i64 %dim_2_inc_ind_var.i, %30
  br i1 %dim_2_cmp.to.max.i, label %__spmv_ellpackr_kernel_separated_args.exit, label %dim_1_pre_head.i

__spmv_ellpackr_kernel_separated_args.exit:       ; preds = %entry, %scalarIf.i, %dim_1_exit.i
  ret void
}

define void @spmv_csr_scalar_kernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32 addrspace(1)**
  %7 = load i32 addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32 addrspace(1)**
  %10 = load i32 addrspace(1)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to float addrspace(1)**
  %16 = load float addrspace(1)** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 56
  %18 = bitcast i8* %17 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %19 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 72
  %21 = bitcast i8* %20 to <{ [4 x i64] }>**
  %22 = load <{ [4 x i64] }>** %21, align 8
  %23 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %24 = load i64* %23, align 8
  %25 = getelementptr <{ [4 x i64] }>* %22, i64 0, i32 0, i64 0
  %26 = load i64* %25, align 8
  %27 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 1
  %28 = load i64* %27, align 8
  %29 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 2
  %30 = load i64* %29, align 8
  %31 = sext i32 %13 to i64
  %32 = add i64 %24, %26
  %33 = icmp slt i64 %32, %31
  %34 = select i1 %33, i64 %32, i64 %31
  %35 = sub i64 %34, %26
  %36 = icmp sgt i64 %35, 0
  br i1 %36, label %WGLoopsEntry.i, label %__spmv_csr_scalar_kernel_separated_args.exit

WGLoopsEntry.i:                                   ; preds = %entry
  %vector.size.i = ashr i64 %35, 4
  %num.vector.wi.i = and i64 %35, -16
  %max.vector.gid.i = add i64 %num.vector.wi.i, %26
  %scalar.size.i = sub i64 %35, %num.vector.wi.i
  %37 = icmp eq i64 %vector.size.i, 0
  br i1 %37, label %scalarIf.i, label %dim_1_vector_pre_head.i

dim_1_vector_pre_head.i:                          ; preds = %dim_1_vector_exit.i, %WGLoopsEntry.i
  %dim_2_vector_ind_var.i = phi i64 [ %dim_2_vector_inc_ind_var.i, %dim_1_vector_exit.i ], [ 0, %WGLoopsEntry.i ]
  br label %dim_0_vector_pre_head.i

dim_0_vector_pre_head.i:                          ; preds = %dim_0_vector_exit.i, %dim_1_vector_pre_head.i
  %dim_1_vector_ind_var.i = phi i64 [ 0, %dim_1_vector_pre_head.i ], [ %dim_1_vector_inc_ind_var.i, %dim_0_vector_exit.i ]
  br label %entryvector_func.i

entryvector_func.i:                               ; preds = %for.endvector_func.i, %dim_0_vector_pre_head.i
  %dim_0_vector_ind_var.i = phi i64 [ 0, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_ind_var.i, %for.endvector_func.i ]
  %dim_0_vector_tid.i = phi i64 [ %26, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_tid.i, %for.endvector_func.i ]
  %broadcast1vector_func.i = insertelement <16 x i64> undef, i64 %dim_0_vector_tid.i, i32 0
  %broadcast2vector_func.i = shufflevector <16 x i64> %broadcast1vector_func.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %38 = add <16 x i64> %broadcast2vector_func.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %conv8vector_func.i = trunc <16 x i64> %38 to <16 x i32>
  %idxprom9vector_func.i = sext <16 x i32> %conv8vector_func.i to <16 x i64>
  %extractvector_func.i = extractelement <16 x i64> %idxprom9vector_func.i, i32 0
  %39 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extractvector_func.i
  %ptrTypeCastvector_func.i = bitcast i32 addrspace(1)* %39 to <16 x i32> addrspace(1)*
  %40 = load <16 x i32> addrspace(1)* %ptrTypeCastvector_func.i, align 4
  %.lhsvector_func.i = extractelement <16 x i32> %conv8vector_func.i, i32 0
  %41 = add i32 %.lhsvector_func.i, 1
  %extract27vector_func.i = sext i32 %41 to i64
  %42 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract27vector_func.i
  %ptrTypeCast43vector_func.i = bitcast i32 addrspace(1)* %42 to <16 x i32> addrspace(1)*
  %43 = load <16 x i32> addrspace(1)* %ptrTypeCast43vector_func.i, align 4
  %cmp41vector_func.i = icmp slt <16 x i32> %40, %43
  %ipred.i1.i = bitcast <16 x i1> %cmp41vector_func.i to i16
  %val.i2.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i1.i, i16 %ipred.i1.i) nounwind
  %44 = and i32 %val.i2.i, 1
  %res.i3.i = icmp eq i32 %44, 0
  br i1 %res.i3.i, label %for.bodyvector_func.preheader.i, label %for.endvector_func.i

for.bodyvector_func.preheader.i:                  ; preds = %entryvector_func.i
  %45 = sext <16 x i32> %40 to <16 x i64>
  %negIncomingLoopMask47vector_func.i = xor <16 x i1> %cmp41vector_func.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  br label %for.bodyvector_func.i

for.bodyvector_func.i:                            ; preds = %postload349vector_func.i, %for.bodyvector_func.preheader.i
  %vectorPHIvector_func.i = phi <16 x i1> [ %loop_mask1157vector_func.i, %postload349vector_func.i ], [ %negIncomingLoopMask47vector_func.i, %for.bodyvector_func.preheader.i ]
  %vectorPHI49vector_func.i = phi <16 x float> [ %out_sel151vector_func.i, %postload349vector_func.i ], [ undef, %for.bodyvector_func.preheader.i ]
  %vectorPHI50vector_func.i = phi <16 x i1> [ %local_edge159vector_func.i, %postload349vector_func.i ], [ %cmp41vector_func.i, %for.bodyvector_func.preheader.i ]
  %vectorPHI51vector_func.i = phi <16 x i64> [ %indvars.iv.next152vector_func.i, %postload349vector_func.i ], [ %45, %for.bodyvector_func.preheader.i ]
  %vectorPHI52vector_func.i = phi <16 x float> [ %add12150vector_func.i, %postload349vector_func.i ], [ zeroinitializer, %for.bodyvector_func.preheader.i ]
  %extract69vector_func.i = extractelement <16 x i1> %vectorPHI50vector_func.i, i32 0
  %extract70vector_func.i = extractelement <16 x i1> %vectorPHI50vector_func.i, i32 1
  %extract71vector_func.i = extractelement <16 x i1> %vectorPHI50vector_func.i, i32 2
  %extract72vector_func.i = extractelement <16 x i1> %vectorPHI50vector_func.i, i32 3
  %extract73vector_func.i = extractelement <16 x i1> %vectorPHI50vector_func.i, i32 4
  %extract74vector_func.i = extractelement <16 x i1> %vectorPHI50vector_func.i, i32 5
  %extract75vector_func.i = extractelement <16 x i1> %vectorPHI50vector_func.i, i32 6
  %extract76vector_func.i = extractelement <16 x i1> %vectorPHI50vector_func.i, i32 7
  %extract77vector_func.i = extractelement <16 x i1> %vectorPHI50vector_func.i, i32 8
  %extract78vector_func.i = extractelement <16 x i1> %vectorPHI50vector_func.i, i32 9
  %extract79vector_func.i = extractelement <16 x i1> %vectorPHI50vector_func.i, i32 10
  %extract80vector_func.i = extractelement <16 x i1> %vectorPHI50vector_func.i, i32 11
  %extract81vector_func.i = extractelement <16 x i1> %vectorPHI50vector_func.i, i32 12
  %extract82vector_func.i = extractelement <16 x i1> %vectorPHI50vector_func.i, i32 13
  %extract83vector_func.i = extractelement <16 x i1> %vectorPHI50vector_func.i, i32 14
  %extract84vector_func.i = extractelement <16 x i1> %vectorPHI50vector_func.i, i32 15
  %extract53vector_func.i = extractelement <16 x i64> %vectorPHI51vector_func.i, i32 0
  %extract54vector_func.i = extractelement <16 x i64> %vectorPHI51vector_func.i, i32 1
  %extract55vector_func.i = extractelement <16 x i64> %vectorPHI51vector_func.i, i32 2
  %extract56vector_func.i = extractelement <16 x i64> %vectorPHI51vector_func.i, i32 3
  %extract57vector_func.i = extractelement <16 x i64> %vectorPHI51vector_func.i, i32 4
  %extract58vector_func.i = extractelement <16 x i64> %vectorPHI51vector_func.i, i32 5
  %extract59vector_func.i = extractelement <16 x i64> %vectorPHI51vector_func.i, i32 6
  %extract60vector_func.i = extractelement <16 x i64> %vectorPHI51vector_func.i, i32 7
  %extract61vector_func.i = extractelement <16 x i64> %vectorPHI51vector_func.i, i32 8
  %extract62vector_func.i = extractelement <16 x i64> %vectorPHI51vector_func.i, i32 9
  %extract63vector_func.i = extractelement <16 x i64> %vectorPHI51vector_func.i, i32 10
  %extract64vector_func.i = extractelement <16 x i64> %vectorPHI51vector_func.i, i32 11
  %extract65vector_func.i = extractelement <16 x i64> %vectorPHI51vector_func.i, i32 12
  %extract66vector_func.i = extractelement <16 x i64> %vectorPHI51vector_func.i, i32 13
  %extract67vector_func.i = extractelement <16 x i64> %vectorPHI51vector_func.i, i32 14
  %extract68vector_func.i = extractelement <16 x i64> %vectorPHI51vector_func.i, i32 15
  %46 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract54vector_func.i
  %47 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract55vector_func.i
  %48 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract56vector_func.i
  %49 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract57vector_func.i
  %50 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract58vector_func.i
  %51 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract59vector_func.i
  %52 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract60vector_func.i
  %53 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract61vector_func.i
  %54 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract62vector_func.i
  %55 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract63vector_func.i
  %56 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract64vector_func.i
  %57 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract65vector_func.i
  %58 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract66vector_func.i
  %59 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract67vector_func.i
  %60 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract68vector_func.i
  br i1 %extract69vector_func.i, label %preloadvector_func.i, label %postloadvector_func.i

preloadvector_func.i:                             ; preds = %for.bodyvector_func.i
  %61 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract53vector_func.i
  %masked_loadvector_func.i = load i32 addrspace(1)* %61, align 4
  br label %postloadvector_func.i

postloadvector_func.i:                            ; preds = %preloadvector_func.i, %for.bodyvector_func.i
  %phivector_func.i = phi i32 [ undef, %for.bodyvector_func.i ], [ %masked_loadvector_func.i, %preloadvector_func.i ]
  br i1 %extract70vector_func.i, label %preload216vector_func.i, label %postload217vector_func.i

preload216vector_func.i:                          ; preds = %postloadvector_func.i
  %masked_load163vector_func.i = load i32 addrspace(1)* %46, align 4
  br label %postload217vector_func.i

postload217vector_func.i:                         ; preds = %preload216vector_func.i, %postloadvector_func.i
  %phi218vector_func.i = phi i32 [ undef, %postloadvector_func.i ], [ %masked_load163vector_func.i, %preload216vector_func.i ]
  br i1 %extract71vector_func.i, label %preload225vector_func.i, label %postload226vector_func.i

preload225vector_func.i:                          ; preds = %postload217vector_func.i
  %masked_load164vector_func.i = load i32 addrspace(1)* %47, align 4
  br label %postload226vector_func.i

postload226vector_func.i:                         ; preds = %preload225vector_func.i, %postload217vector_func.i
  %phi227vector_func.i = phi i32 [ undef, %postload217vector_func.i ], [ %masked_load164vector_func.i, %preload225vector_func.i ]
  br i1 %extract72vector_func.i, label %preload234vector_func.i, label %postload235vector_func.i

preload234vector_func.i:                          ; preds = %postload226vector_func.i
  %masked_load165vector_func.i = load i32 addrspace(1)* %48, align 4
  br label %postload235vector_func.i

postload235vector_func.i:                         ; preds = %preload234vector_func.i, %postload226vector_func.i
  %phi236vector_func.i = phi i32 [ undef, %postload226vector_func.i ], [ %masked_load165vector_func.i, %preload234vector_func.i ]
  br i1 %extract73vector_func.i, label %preload243vector_func.i, label %postload244vector_func.i

preload243vector_func.i:                          ; preds = %postload235vector_func.i
  %masked_load166vector_func.i = load i32 addrspace(1)* %49, align 4
  br label %postload244vector_func.i

postload244vector_func.i:                         ; preds = %preload243vector_func.i, %postload235vector_func.i
  %phi245vector_func.i = phi i32 [ undef, %postload235vector_func.i ], [ %masked_load166vector_func.i, %preload243vector_func.i ]
  br i1 %extract74vector_func.i, label %preload252vector_func.i, label %postload253vector_func.i

preload252vector_func.i:                          ; preds = %postload244vector_func.i
  %masked_load167vector_func.i = load i32 addrspace(1)* %50, align 4
  br label %postload253vector_func.i

postload253vector_func.i:                         ; preds = %preload252vector_func.i, %postload244vector_func.i
  %phi254vector_func.i = phi i32 [ undef, %postload244vector_func.i ], [ %masked_load167vector_func.i, %preload252vector_func.i ]
  br i1 %extract75vector_func.i, label %preload261vector_func.i, label %postload262vector_func.i

preload261vector_func.i:                          ; preds = %postload253vector_func.i
  %masked_load168vector_func.i = load i32 addrspace(1)* %51, align 4
  br label %postload262vector_func.i

postload262vector_func.i:                         ; preds = %preload261vector_func.i, %postload253vector_func.i
  %phi263vector_func.i = phi i32 [ undef, %postload253vector_func.i ], [ %masked_load168vector_func.i, %preload261vector_func.i ]
  br i1 %extract76vector_func.i, label %preload270vector_func.i, label %postload271vector_func.i

preload270vector_func.i:                          ; preds = %postload262vector_func.i
  %masked_load169vector_func.i = load i32 addrspace(1)* %52, align 4
  br label %postload271vector_func.i

postload271vector_func.i:                         ; preds = %preload270vector_func.i, %postload262vector_func.i
  %phi272vector_func.i = phi i32 [ undef, %postload262vector_func.i ], [ %masked_load169vector_func.i, %preload270vector_func.i ]
  br i1 %extract77vector_func.i, label %preload279vector_func.i, label %postload280vector_func.i

preload279vector_func.i:                          ; preds = %postload271vector_func.i
  %masked_load170vector_func.i = load i32 addrspace(1)* %53, align 4
  br label %postload280vector_func.i

postload280vector_func.i:                         ; preds = %preload279vector_func.i, %postload271vector_func.i
  %phi281vector_func.i = phi i32 [ undef, %postload271vector_func.i ], [ %masked_load170vector_func.i, %preload279vector_func.i ]
  br i1 %extract78vector_func.i, label %preload288vector_func.i, label %postload289vector_func.i

preload288vector_func.i:                          ; preds = %postload280vector_func.i
  %masked_load171vector_func.i = load i32 addrspace(1)* %54, align 4
  br label %postload289vector_func.i

postload289vector_func.i:                         ; preds = %preload288vector_func.i, %postload280vector_func.i
  %phi290vector_func.i = phi i32 [ undef, %postload280vector_func.i ], [ %masked_load171vector_func.i, %preload288vector_func.i ]
  br i1 %extract79vector_func.i, label %preload297vector_func.i, label %postload298vector_func.i

preload297vector_func.i:                          ; preds = %postload289vector_func.i
  %masked_load172vector_func.i = load i32 addrspace(1)* %55, align 4
  br label %postload298vector_func.i

postload298vector_func.i:                         ; preds = %preload297vector_func.i, %postload289vector_func.i
  %phi299vector_func.i = phi i32 [ undef, %postload289vector_func.i ], [ %masked_load172vector_func.i, %preload297vector_func.i ]
  br i1 %extract80vector_func.i, label %preload306vector_func.i, label %postload307vector_func.i

preload306vector_func.i:                          ; preds = %postload298vector_func.i
  %masked_load173vector_func.i = load i32 addrspace(1)* %56, align 4
  br label %postload307vector_func.i

postload307vector_func.i:                         ; preds = %preload306vector_func.i, %postload298vector_func.i
  %phi308vector_func.i = phi i32 [ undef, %postload298vector_func.i ], [ %masked_load173vector_func.i, %preload306vector_func.i ]
  br i1 %extract81vector_func.i, label %preload315vector_func.i, label %postload316vector_func.i

preload315vector_func.i:                          ; preds = %postload307vector_func.i
  %masked_load174vector_func.i = load i32 addrspace(1)* %57, align 4
  br label %postload316vector_func.i

postload316vector_func.i:                         ; preds = %preload315vector_func.i, %postload307vector_func.i
  %phi317vector_func.i = phi i32 [ undef, %postload307vector_func.i ], [ %masked_load174vector_func.i, %preload315vector_func.i ]
  br i1 %extract82vector_func.i, label %preload324vector_func.i, label %postload325vector_func.i

preload324vector_func.i:                          ; preds = %postload316vector_func.i
  %masked_load175vector_func.i = load i32 addrspace(1)* %58, align 4
  br label %postload325vector_func.i

postload325vector_func.i:                         ; preds = %preload324vector_func.i, %postload316vector_func.i
  %phi326vector_func.i = phi i32 [ undef, %postload316vector_func.i ], [ %masked_load175vector_func.i, %preload324vector_func.i ]
  br i1 %extract83vector_func.i, label %preload333vector_func.i, label %postload334vector_func.i

preload333vector_func.i:                          ; preds = %postload325vector_func.i
  %masked_load176vector_func.i = load i32 addrspace(1)* %59, align 4
  br label %postload334vector_func.i

postload334vector_func.i:                         ; preds = %preload333vector_func.i, %postload325vector_func.i
  %phi335vector_func.i = phi i32 [ undef, %postload325vector_func.i ], [ %masked_load176vector_func.i, %preload333vector_func.i ]
  br i1 %extract84vector_func.i, label %preload342vector_func.i, label %postload343vector_func.i

preload342vector_func.i:                          ; preds = %postload334vector_func.i
  %masked_load177vector_func.i = load i32 addrspace(1)* %60, align 4
  br label %postload343vector_func.i

postload343vector_func.i:                         ; preds = %preload342vector_func.i, %postload334vector_func.i
  %phi344vector_func.i = phi i32 [ undef, %postload334vector_func.i ], [ %masked_load177vector_func.i, %preload342vector_func.i ]
  %temp.vectvector_func.i = insertelement <16 x i32> undef, i32 %phivector_func.i, i32 0
  %temp.vect85vector_func.i = insertelement <16 x i32> %temp.vectvector_func.i, i32 %phi218vector_func.i, i32 1
  %temp.vect86vector_func.i = insertelement <16 x i32> %temp.vect85vector_func.i, i32 %phi227vector_func.i, i32 2
  %temp.vect87vector_func.i = insertelement <16 x i32> %temp.vect86vector_func.i, i32 %phi236vector_func.i, i32 3
  %temp.vect88vector_func.i = insertelement <16 x i32> %temp.vect87vector_func.i, i32 %phi245vector_func.i, i32 4
  %temp.vect89vector_func.i = insertelement <16 x i32> %temp.vect88vector_func.i, i32 %phi254vector_func.i, i32 5
  %temp.vect90vector_func.i = insertelement <16 x i32> %temp.vect89vector_func.i, i32 %phi263vector_func.i, i32 6
  %temp.vect91vector_func.i = insertelement <16 x i32> %temp.vect90vector_func.i, i32 %phi272vector_func.i, i32 7
  %temp.vect92vector_func.i = insertelement <16 x i32> %temp.vect91vector_func.i, i32 %phi281vector_func.i, i32 8
  %temp.vect93vector_func.i = insertelement <16 x i32> %temp.vect92vector_func.i, i32 %phi290vector_func.i, i32 9
  %temp.vect94vector_func.i = insertelement <16 x i32> %temp.vect93vector_func.i, i32 %phi299vector_func.i, i32 10
  %temp.vect95vector_func.i = insertelement <16 x i32> %temp.vect94vector_func.i, i32 %phi308vector_func.i, i32 11
  %temp.vect96vector_func.i = insertelement <16 x i32> %temp.vect95vector_func.i, i32 %phi317vector_func.i, i32 12
  %temp.vect97vector_func.i = insertelement <16 x i32> %temp.vect96vector_func.i, i32 %phi326vector_func.i, i32 13
  %temp.vect98vector_func.i = insertelement <16 x i32> %temp.vect97vector_func.i, i32 %phi335vector_func.i, i32 14
  %temp.vect99vector_func.i = insertelement <16 x i32> %temp.vect98vector_func.i, i32 %phi344vector_func.i, i32 15
  %62 = getelementptr inbounds float addrspace(1)* %1, i64 %extract54vector_func.i
  %63 = getelementptr inbounds float addrspace(1)* %1, i64 %extract55vector_func.i
  %64 = getelementptr inbounds float addrspace(1)* %1, i64 %extract56vector_func.i
  %65 = getelementptr inbounds float addrspace(1)* %1, i64 %extract57vector_func.i
  %66 = getelementptr inbounds float addrspace(1)* %1, i64 %extract58vector_func.i
  %67 = getelementptr inbounds float addrspace(1)* %1, i64 %extract59vector_func.i
  %68 = getelementptr inbounds float addrspace(1)* %1, i64 %extract60vector_func.i
  %69 = getelementptr inbounds float addrspace(1)* %1, i64 %extract61vector_func.i
  %70 = getelementptr inbounds float addrspace(1)* %1, i64 %extract62vector_func.i
  %71 = getelementptr inbounds float addrspace(1)* %1, i64 %extract63vector_func.i
  %72 = getelementptr inbounds float addrspace(1)* %1, i64 %extract64vector_func.i
  %73 = getelementptr inbounds float addrspace(1)* %1, i64 %extract65vector_func.i
  %74 = getelementptr inbounds float addrspace(1)* %1, i64 %extract66vector_func.i
  %75 = getelementptr inbounds float addrspace(1)* %1, i64 %extract67vector_func.i
  %76 = getelementptr inbounds float addrspace(1)* %1, i64 %extract68vector_func.i
  br i1 %extract69vector_func.i, label %preload210vector_func.i, label %postload211vector_func.i

preload210vector_func.i:                          ; preds = %postload343vector_func.i
  %77 = getelementptr inbounds float addrspace(1)* %1, i64 %extract53vector_func.i
  %masked_load178vector_func.i = load float addrspace(1)* %77, align 4
  br label %postload211vector_func.i

postload211vector_func.i:                         ; preds = %preload210vector_func.i, %postload343vector_func.i
  %phi212vector_func.i = phi float [ undef, %postload343vector_func.i ], [ %masked_load178vector_func.i, %preload210vector_func.i ]
  br i1 %extract70vector_func.i, label %preload219vector_func.i, label %postload220vector_func.i

preload219vector_func.i:                          ; preds = %postload211vector_func.i
  %masked_load179vector_func.i = load float addrspace(1)* %62, align 4
  br label %postload220vector_func.i

postload220vector_func.i:                         ; preds = %preload219vector_func.i, %postload211vector_func.i
  %phi221vector_func.i = phi float [ undef, %postload211vector_func.i ], [ %masked_load179vector_func.i, %preload219vector_func.i ]
  br i1 %extract71vector_func.i, label %preload228vector_func.i, label %postload229vector_func.i

preload228vector_func.i:                          ; preds = %postload220vector_func.i
  %masked_load180vector_func.i = load float addrspace(1)* %63, align 4
  br label %postload229vector_func.i

postload229vector_func.i:                         ; preds = %preload228vector_func.i, %postload220vector_func.i
  %phi230vector_func.i = phi float [ undef, %postload220vector_func.i ], [ %masked_load180vector_func.i, %preload228vector_func.i ]
  br i1 %extract72vector_func.i, label %preload237vector_func.i, label %postload238vector_func.i

preload237vector_func.i:                          ; preds = %postload229vector_func.i
  %masked_load181vector_func.i = load float addrspace(1)* %64, align 4
  br label %postload238vector_func.i

postload238vector_func.i:                         ; preds = %preload237vector_func.i, %postload229vector_func.i
  %phi239vector_func.i = phi float [ undef, %postload229vector_func.i ], [ %masked_load181vector_func.i, %preload237vector_func.i ]
  br i1 %extract73vector_func.i, label %preload246vector_func.i, label %postload247vector_func.i

preload246vector_func.i:                          ; preds = %postload238vector_func.i
  %masked_load182vector_func.i = load float addrspace(1)* %65, align 4
  br label %postload247vector_func.i

postload247vector_func.i:                         ; preds = %preload246vector_func.i, %postload238vector_func.i
  %phi248vector_func.i = phi float [ undef, %postload238vector_func.i ], [ %masked_load182vector_func.i, %preload246vector_func.i ]
  br i1 %extract74vector_func.i, label %preload255vector_func.i, label %postload256vector_func.i

preload255vector_func.i:                          ; preds = %postload247vector_func.i
  %masked_load183vector_func.i = load float addrspace(1)* %66, align 4
  br label %postload256vector_func.i

postload256vector_func.i:                         ; preds = %preload255vector_func.i, %postload247vector_func.i
  %phi257vector_func.i = phi float [ undef, %postload247vector_func.i ], [ %masked_load183vector_func.i, %preload255vector_func.i ]
  br i1 %extract75vector_func.i, label %preload264vector_func.i, label %postload265vector_func.i

preload264vector_func.i:                          ; preds = %postload256vector_func.i
  %masked_load184vector_func.i = load float addrspace(1)* %67, align 4
  br label %postload265vector_func.i

postload265vector_func.i:                         ; preds = %preload264vector_func.i, %postload256vector_func.i
  %phi266vector_func.i = phi float [ undef, %postload256vector_func.i ], [ %masked_load184vector_func.i, %preload264vector_func.i ]
  br i1 %extract76vector_func.i, label %preload273vector_func.i, label %postload274vector_func.i

preload273vector_func.i:                          ; preds = %postload265vector_func.i
  %masked_load185vector_func.i = load float addrspace(1)* %68, align 4
  br label %postload274vector_func.i

postload274vector_func.i:                         ; preds = %preload273vector_func.i, %postload265vector_func.i
  %phi275vector_func.i = phi float [ undef, %postload265vector_func.i ], [ %masked_load185vector_func.i, %preload273vector_func.i ]
  br i1 %extract77vector_func.i, label %preload282vector_func.i, label %postload283vector_func.i

preload282vector_func.i:                          ; preds = %postload274vector_func.i
  %masked_load186vector_func.i = load float addrspace(1)* %69, align 4
  br label %postload283vector_func.i

postload283vector_func.i:                         ; preds = %preload282vector_func.i, %postload274vector_func.i
  %phi284vector_func.i = phi float [ undef, %postload274vector_func.i ], [ %masked_load186vector_func.i, %preload282vector_func.i ]
  br i1 %extract78vector_func.i, label %preload291vector_func.i, label %postload292vector_func.i

preload291vector_func.i:                          ; preds = %postload283vector_func.i
  %masked_load187vector_func.i = load float addrspace(1)* %70, align 4
  br label %postload292vector_func.i

postload292vector_func.i:                         ; preds = %preload291vector_func.i, %postload283vector_func.i
  %phi293vector_func.i = phi float [ undef, %postload283vector_func.i ], [ %masked_load187vector_func.i, %preload291vector_func.i ]
  br i1 %extract79vector_func.i, label %preload300vector_func.i, label %postload301vector_func.i

preload300vector_func.i:                          ; preds = %postload292vector_func.i
  %masked_load188vector_func.i = load float addrspace(1)* %71, align 4
  br label %postload301vector_func.i

postload301vector_func.i:                         ; preds = %preload300vector_func.i, %postload292vector_func.i
  %phi302vector_func.i = phi float [ undef, %postload292vector_func.i ], [ %masked_load188vector_func.i, %preload300vector_func.i ]
  br i1 %extract80vector_func.i, label %preload309vector_func.i, label %postload310vector_func.i

preload309vector_func.i:                          ; preds = %postload301vector_func.i
  %masked_load189vector_func.i = load float addrspace(1)* %72, align 4
  br label %postload310vector_func.i

postload310vector_func.i:                         ; preds = %preload309vector_func.i, %postload301vector_func.i
  %phi311vector_func.i = phi float [ undef, %postload301vector_func.i ], [ %masked_load189vector_func.i, %preload309vector_func.i ]
  br i1 %extract81vector_func.i, label %preload318vector_func.i, label %postload319vector_func.i

preload318vector_func.i:                          ; preds = %postload310vector_func.i
  %masked_load190vector_func.i = load float addrspace(1)* %73, align 4
  br label %postload319vector_func.i

postload319vector_func.i:                         ; preds = %preload318vector_func.i, %postload310vector_func.i
  %phi320vector_func.i = phi float [ undef, %postload310vector_func.i ], [ %masked_load190vector_func.i, %preload318vector_func.i ]
  br i1 %extract82vector_func.i, label %preload327vector_func.i, label %postload328vector_func.i

preload327vector_func.i:                          ; preds = %postload319vector_func.i
  %masked_load191vector_func.i = load float addrspace(1)* %74, align 4
  br label %postload328vector_func.i

postload328vector_func.i:                         ; preds = %preload327vector_func.i, %postload319vector_func.i
  %phi329vector_func.i = phi float [ undef, %postload319vector_func.i ], [ %masked_load191vector_func.i, %preload327vector_func.i ]
  br i1 %extract83vector_func.i, label %preload336vector_func.i, label %postload337vector_func.i

preload336vector_func.i:                          ; preds = %postload328vector_func.i
  %masked_load192vector_func.i = load float addrspace(1)* %75, align 4
  br label %postload337vector_func.i

postload337vector_func.i:                         ; preds = %preload336vector_func.i, %postload328vector_func.i
  %phi338vector_func.i = phi float [ undef, %postload328vector_func.i ], [ %masked_load192vector_func.i, %preload336vector_func.i ]
  br i1 %extract84vector_func.i, label %preload345vector_func.i, label %postload346vector_func.i

preload345vector_func.i:                          ; preds = %postload337vector_func.i
  %masked_load193vector_func.i = load float addrspace(1)* %76, align 4
  br label %postload346vector_func.i

postload346vector_func.i:                         ; preds = %preload345vector_func.i, %postload337vector_func.i
  %phi347vector_func.i = phi float [ undef, %postload337vector_func.i ], [ %masked_load193vector_func.i, %preload345vector_func.i ]
  %temp.vect117vector_func.i = insertelement <16 x float> undef, float %phi212vector_func.i, i32 0
  %temp.vect118vector_func.i = insertelement <16 x float> %temp.vect117vector_func.i, float %phi221vector_func.i, i32 1
  %temp.vect119vector_func.i = insertelement <16 x float> %temp.vect118vector_func.i, float %phi230vector_func.i, i32 2
  %temp.vect120vector_func.i = insertelement <16 x float> %temp.vect119vector_func.i, float %phi239vector_func.i, i32 3
  %temp.vect121vector_func.i = insertelement <16 x float> %temp.vect120vector_func.i, float %phi248vector_func.i, i32 4
  %temp.vect122vector_func.i = insertelement <16 x float> %temp.vect121vector_func.i, float %phi257vector_func.i, i32 5
  %temp.vect123vector_func.i = insertelement <16 x float> %temp.vect122vector_func.i, float %phi266vector_func.i, i32 6
  %temp.vect124vector_func.i = insertelement <16 x float> %temp.vect123vector_func.i, float %phi275vector_func.i, i32 7
  %temp.vect125vector_func.i = insertelement <16 x float> %temp.vect124vector_func.i, float %phi284vector_func.i, i32 8
  %temp.vect126vector_func.i = insertelement <16 x float> %temp.vect125vector_func.i, float %phi293vector_func.i, i32 9
  %temp.vect127vector_func.i = insertelement <16 x float> %temp.vect126vector_func.i, float %phi302vector_func.i, i32 10
  %temp.vect128vector_func.i = insertelement <16 x float> %temp.vect127vector_func.i, float %phi311vector_func.i, i32 11
  %temp.vect129vector_func.i = insertelement <16 x float> %temp.vect128vector_func.i, float %phi320vector_func.i, i32 12
  %temp.vect130vector_func.i = insertelement <16 x float> %temp.vect129vector_func.i, float %phi329vector_func.i, i32 13
  %temp.vect131vector_func.i = insertelement <16 x float> %temp.vect130vector_func.i, float %phi338vector_func.i, i32 14
  %temp.vect132vector_func.i = insertelement <16 x float> %temp.vect131vector_func.i, float %phi347vector_func.i, i32 15
  %idxprom10100vector_func.i = sext <16 x i32> %temp.vect99vector_func.i to <16 x i64>
  %extract102vector_func.i = extractelement <16 x i64> %idxprom10100vector_func.i, i32 1
  %extract103vector_func.i = extractelement <16 x i64> %idxprom10100vector_func.i, i32 2
  %extract104vector_func.i = extractelement <16 x i64> %idxprom10100vector_func.i, i32 3
  %extract105vector_func.i = extractelement <16 x i64> %idxprom10100vector_func.i, i32 4
  %extract106vector_func.i = extractelement <16 x i64> %idxprom10100vector_func.i, i32 5
  %extract107vector_func.i = extractelement <16 x i64> %idxprom10100vector_func.i, i32 6
  %extract108vector_func.i = extractelement <16 x i64> %idxprom10100vector_func.i, i32 7
  %extract109vector_func.i = extractelement <16 x i64> %idxprom10100vector_func.i, i32 8
  %extract110vector_func.i = extractelement <16 x i64> %idxprom10100vector_func.i, i32 9
  %extract111vector_func.i = extractelement <16 x i64> %idxprom10100vector_func.i, i32 10
  %extract112vector_func.i = extractelement <16 x i64> %idxprom10100vector_func.i, i32 11
  %extract113vector_func.i = extractelement <16 x i64> %idxprom10100vector_func.i, i32 12
  %extract114vector_func.i = extractelement <16 x i64> %idxprom10100vector_func.i, i32 13
  %extract115vector_func.i = extractelement <16 x i64> %idxprom10100vector_func.i, i32 14
  %extract116vector_func.i = extractelement <16 x i64> %idxprom10100vector_func.i, i32 15
  %78 = getelementptr inbounds float addrspace(1)* %4, i64 %extract102vector_func.i
  %79 = getelementptr inbounds float addrspace(1)* %4, i64 %extract103vector_func.i
  %80 = getelementptr inbounds float addrspace(1)* %4, i64 %extract104vector_func.i
  %81 = getelementptr inbounds float addrspace(1)* %4, i64 %extract105vector_func.i
  %82 = getelementptr inbounds float addrspace(1)* %4, i64 %extract106vector_func.i
  %83 = getelementptr inbounds float addrspace(1)* %4, i64 %extract107vector_func.i
  %84 = getelementptr inbounds float addrspace(1)* %4, i64 %extract108vector_func.i
  %85 = getelementptr inbounds float addrspace(1)* %4, i64 %extract109vector_func.i
  %86 = getelementptr inbounds float addrspace(1)* %4, i64 %extract110vector_func.i
  %87 = getelementptr inbounds float addrspace(1)* %4, i64 %extract111vector_func.i
  %88 = getelementptr inbounds float addrspace(1)* %4, i64 %extract112vector_func.i
  %89 = getelementptr inbounds float addrspace(1)* %4, i64 %extract113vector_func.i
  %90 = getelementptr inbounds float addrspace(1)* %4, i64 %extract114vector_func.i
  %91 = getelementptr inbounds float addrspace(1)* %4, i64 %extract115vector_func.i
  %92 = getelementptr inbounds float addrspace(1)* %4, i64 %extract116vector_func.i
  br i1 %extract69vector_func.i, label %preload213vector_func.i, label %postload214vector_func.i

preload213vector_func.i:                          ; preds = %postload346vector_func.i
  %extract101vector_func.i = extractelement <16 x i64> %idxprom10100vector_func.i, i32 0
  %93 = getelementptr inbounds float addrspace(1)* %4, i64 %extract101vector_func.i
  %masked_load194vector_func.i = load float addrspace(1)* %93, align 4
  br label %postload214vector_func.i

postload214vector_func.i:                         ; preds = %preload213vector_func.i, %postload346vector_func.i
  %phi215vector_func.i = phi float [ undef, %postload346vector_func.i ], [ %masked_load194vector_func.i, %preload213vector_func.i ]
  br i1 %extract70vector_func.i, label %preload222vector_func.i, label %postload223vector_func.i

preload222vector_func.i:                          ; preds = %postload214vector_func.i
  %masked_load195vector_func.i = load float addrspace(1)* %78, align 4
  br label %postload223vector_func.i

postload223vector_func.i:                         ; preds = %preload222vector_func.i, %postload214vector_func.i
  %phi224vector_func.i = phi float [ undef, %postload214vector_func.i ], [ %masked_load195vector_func.i, %preload222vector_func.i ]
  br i1 %extract71vector_func.i, label %preload231vector_func.i, label %postload232vector_func.i

preload231vector_func.i:                          ; preds = %postload223vector_func.i
  %masked_load196vector_func.i = load float addrspace(1)* %79, align 4
  br label %postload232vector_func.i

postload232vector_func.i:                         ; preds = %preload231vector_func.i, %postload223vector_func.i
  %phi233vector_func.i = phi float [ undef, %postload223vector_func.i ], [ %masked_load196vector_func.i, %preload231vector_func.i ]
  br i1 %extract72vector_func.i, label %preload240vector_func.i, label %postload241vector_func.i

preload240vector_func.i:                          ; preds = %postload232vector_func.i
  %masked_load197vector_func.i = load float addrspace(1)* %80, align 4
  br label %postload241vector_func.i

postload241vector_func.i:                         ; preds = %preload240vector_func.i, %postload232vector_func.i
  %phi242vector_func.i = phi float [ undef, %postload232vector_func.i ], [ %masked_load197vector_func.i, %preload240vector_func.i ]
  br i1 %extract73vector_func.i, label %preload249vector_func.i, label %postload250vector_func.i

preload249vector_func.i:                          ; preds = %postload241vector_func.i
  %masked_load198vector_func.i = load float addrspace(1)* %81, align 4
  br label %postload250vector_func.i

postload250vector_func.i:                         ; preds = %preload249vector_func.i, %postload241vector_func.i
  %phi251vector_func.i = phi float [ undef, %postload241vector_func.i ], [ %masked_load198vector_func.i, %preload249vector_func.i ]
  br i1 %extract74vector_func.i, label %preload258vector_func.i, label %postload259vector_func.i

preload258vector_func.i:                          ; preds = %postload250vector_func.i
  %masked_load199vector_func.i = load float addrspace(1)* %82, align 4
  br label %postload259vector_func.i

postload259vector_func.i:                         ; preds = %preload258vector_func.i, %postload250vector_func.i
  %phi260vector_func.i = phi float [ undef, %postload250vector_func.i ], [ %masked_load199vector_func.i, %preload258vector_func.i ]
  br i1 %extract75vector_func.i, label %preload267vector_func.i, label %postload268vector_func.i

preload267vector_func.i:                          ; preds = %postload259vector_func.i
  %masked_load200vector_func.i = load float addrspace(1)* %83, align 4
  br label %postload268vector_func.i

postload268vector_func.i:                         ; preds = %preload267vector_func.i, %postload259vector_func.i
  %phi269vector_func.i = phi float [ undef, %postload259vector_func.i ], [ %masked_load200vector_func.i, %preload267vector_func.i ]
  br i1 %extract76vector_func.i, label %preload276vector_func.i, label %postload277vector_func.i

preload276vector_func.i:                          ; preds = %postload268vector_func.i
  %masked_load201vector_func.i = load float addrspace(1)* %84, align 4
  br label %postload277vector_func.i

postload277vector_func.i:                         ; preds = %preload276vector_func.i, %postload268vector_func.i
  %phi278vector_func.i = phi float [ undef, %postload268vector_func.i ], [ %masked_load201vector_func.i, %preload276vector_func.i ]
  br i1 %extract77vector_func.i, label %preload285vector_func.i, label %postload286vector_func.i

preload285vector_func.i:                          ; preds = %postload277vector_func.i
  %masked_load202vector_func.i = load float addrspace(1)* %85, align 4
  br label %postload286vector_func.i

postload286vector_func.i:                         ; preds = %preload285vector_func.i, %postload277vector_func.i
  %phi287vector_func.i = phi float [ undef, %postload277vector_func.i ], [ %masked_load202vector_func.i, %preload285vector_func.i ]
  br i1 %extract78vector_func.i, label %preload294vector_func.i, label %postload295vector_func.i

preload294vector_func.i:                          ; preds = %postload286vector_func.i
  %masked_load203vector_func.i = load float addrspace(1)* %86, align 4
  br label %postload295vector_func.i

postload295vector_func.i:                         ; preds = %preload294vector_func.i, %postload286vector_func.i
  %phi296vector_func.i = phi float [ undef, %postload286vector_func.i ], [ %masked_load203vector_func.i, %preload294vector_func.i ]
  br i1 %extract79vector_func.i, label %preload303vector_func.i, label %postload304vector_func.i

preload303vector_func.i:                          ; preds = %postload295vector_func.i
  %masked_load204vector_func.i = load float addrspace(1)* %87, align 4
  br label %postload304vector_func.i

postload304vector_func.i:                         ; preds = %preload303vector_func.i, %postload295vector_func.i
  %phi305vector_func.i = phi float [ undef, %postload295vector_func.i ], [ %masked_load204vector_func.i, %preload303vector_func.i ]
  br i1 %extract80vector_func.i, label %preload312vector_func.i, label %postload313vector_func.i

preload312vector_func.i:                          ; preds = %postload304vector_func.i
  %masked_load205vector_func.i = load float addrspace(1)* %88, align 4
  br label %postload313vector_func.i

postload313vector_func.i:                         ; preds = %preload312vector_func.i, %postload304vector_func.i
  %phi314vector_func.i = phi float [ undef, %postload304vector_func.i ], [ %masked_load205vector_func.i, %preload312vector_func.i ]
  br i1 %extract81vector_func.i, label %preload321vector_func.i, label %postload322vector_func.i

preload321vector_func.i:                          ; preds = %postload313vector_func.i
  %masked_load206vector_func.i = load float addrspace(1)* %89, align 4
  br label %postload322vector_func.i

postload322vector_func.i:                         ; preds = %preload321vector_func.i, %postload313vector_func.i
  %phi323vector_func.i = phi float [ undef, %postload313vector_func.i ], [ %masked_load206vector_func.i, %preload321vector_func.i ]
  br i1 %extract82vector_func.i, label %preload330vector_func.i, label %postload331vector_func.i

preload330vector_func.i:                          ; preds = %postload322vector_func.i
  %masked_load207vector_func.i = load float addrspace(1)* %90, align 4
  br label %postload331vector_func.i

postload331vector_func.i:                         ; preds = %preload330vector_func.i, %postload322vector_func.i
  %phi332vector_func.i = phi float [ undef, %postload322vector_func.i ], [ %masked_load207vector_func.i, %preload330vector_func.i ]
  br i1 %extract83vector_func.i, label %preload339vector_func.i, label %postload340vector_func.i

preload339vector_func.i:                          ; preds = %postload331vector_func.i
  %masked_load208vector_func.i = load float addrspace(1)* %91, align 4
  br label %postload340vector_func.i

postload340vector_func.i:                         ; preds = %preload339vector_func.i, %postload331vector_func.i
  %phi341vector_func.i = phi float [ undef, %postload331vector_func.i ], [ %masked_load208vector_func.i, %preload339vector_func.i ]
  br i1 %extract84vector_func.i, label %preload348vector_func.i, label %postload349vector_func.i

preload348vector_func.i:                          ; preds = %postload340vector_func.i
  %masked_load209vector_func.i = load float addrspace(1)* %92, align 4
  br label %postload349vector_func.i

postload349vector_func.i:                         ; preds = %preload348vector_func.i, %postload340vector_func.i
  %phi350vector_func.i = phi float [ undef, %postload340vector_func.i ], [ %masked_load209vector_func.i, %preload348vector_func.i ]
  %temp.vect133vector_func.i = insertelement <16 x float> undef, float %phi215vector_func.i, i32 0
  %temp.vect134vector_func.i = insertelement <16 x float> %temp.vect133vector_func.i, float %phi224vector_func.i, i32 1
  %temp.vect135vector_func.i = insertelement <16 x float> %temp.vect134vector_func.i, float %phi233vector_func.i, i32 2
  %temp.vect136vector_func.i = insertelement <16 x float> %temp.vect135vector_func.i, float %phi242vector_func.i, i32 3
  %temp.vect137vector_func.i = insertelement <16 x float> %temp.vect136vector_func.i, float %phi251vector_func.i, i32 4
  %temp.vect138vector_func.i = insertelement <16 x float> %temp.vect137vector_func.i, float %phi260vector_func.i, i32 5
  %temp.vect139vector_func.i = insertelement <16 x float> %temp.vect138vector_func.i, float %phi269vector_func.i, i32 6
  %temp.vect140vector_func.i = insertelement <16 x float> %temp.vect139vector_func.i, float %phi278vector_func.i, i32 7
  %temp.vect141vector_func.i = insertelement <16 x float> %temp.vect140vector_func.i, float %phi287vector_func.i, i32 8
  %temp.vect142vector_func.i = insertelement <16 x float> %temp.vect141vector_func.i, float %phi296vector_func.i, i32 9
  %temp.vect143vector_func.i = insertelement <16 x float> %temp.vect142vector_func.i, float %phi305vector_func.i, i32 10
  %temp.vect144vector_func.i = insertelement <16 x float> %temp.vect143vector_func.i, float %phi314vector_func.i, i32 11
  %temp.vect145vector_func.i = insertelement <16 x float> %temp.vect144vector_func.i, float %phi323vector_func.i, i32 12
  %temp.vect146vector_func.i = insertelement <16 x float> %temp.vect145vector_func.i, float %phi332vector_func.i, i32 13
  %temp.vect147vector_func.i = insertelement <16 x float> %temp.vect146vector_func.i, float %phi341vector_func.i, i32 14
  %temp.vect148vector_func.i = insertelement <16 x float> %temp.vect147vector_func.i, float %phi350vector_func.i, i32 15
  %mul149vector_func.i = fmul <16 x float> %temp.vect132vector_func.i, %temp.vect148vector_func.i
  %add12150vector_func.i = fadd <16 x float> %vectorPHI52vector_func.i, %mul149vector_func.i
  %out_sel151vector_func.i = select <16 x i1> %vectorPHI50vector_func.i, <16 x float> %add12150vector_func.i, <16 x float> %vectorPHI49vector_func.i
  %indvars.iv.next152vector_func.i = add <16 x i64> %vectorPHI51vector_func.i, <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %lftr.wideiv153vector_func.i = trunc <16 x i64> %indvars.iv.next152vector_func.i to <16 x i32>
  %exitcondvector_func.i = icmp eq <16 x i32> %lftr.wideiv153vector_func.i, %43
  %notCond154vector_func.i = xor <16 x i1> %exitcondvector_func.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %who_left_tr155vector_func.i = and <16 x i1> %vectorPHI50vector_func.i, %exitcondvector_func.i
  %loop_mask1157vector_func.i = or <16 x i1> %vectorPHIvector_func.i, %who_left_tr155vector_func.i
  %ipred.i.i = bitcast <16 x i1> %loop_mask1157vector_func.i to i16
  %val.i.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i.i, i16 %ipred.i.i) nounwind
  %94 = and i32 %val.i.i, 1
  %res.i.i = icmp eq i32 %94, 0
  %local_edge159vector_func.i = and <16 x i1> %vectorPHI50vector_func.i, %notCond154vector_func.i
  br i1 %res.i.i, label %for.bodyvector_func.i, label %for.endvector_func.i

for.endvector_func.i:                             ; preds = %postload349vector_func.i, %entryvector_func.i
  %vectorPHI160vector_func.i = phi <16 x float> [ undef, %entryvector_func.i ], [ %out_sel151vector_func.i, %postload349vector_func.i ]
  %merge161vector_func.i = select <16 x i1> %cmp41vector_func.i, <16 x float> %vectorPHI160vector_func.i, <16 x float> zeroinitializer
  %95 = getelementptr inbounds float addrspace(1)* %16, i64 %extractvector_func.i
  %ptrTypeCast162vector_func.i = bitcast float addrspace(1)* %95 to <16 x float> addrspace(1)*
  store <16 x float> %merge161vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast162vector_func.i, align 4
  %dim_0_vector_inc_ind_var.i = add i64 %dim_0_vector_ind_var.i, 1
  %dim_0_vector_cmp.to.max.i = icmp eq i64 %dim_0_vector_inc_ind_var.i, %vector.size.i
  %dim_0_vector_inc_tid.i = add i64 %dim_0_vector_tid.i, 16
  br i1 %dim_0_vector_cmp.to.max.i, label %dim_0_vector_exit.i, label %entryvector_func.i

dim_0_vector_exit.i:                              ; preds = %for.endvector_func.i
  %dim_1_vector_inc_ind_var.i = add i64 %dim_1_vector_ind_var.i, 1
  %dim_1_vector_cmp.to.max.i = icmp eq i64 %dim_1_vector_inc_ind_var.i, %28
  br i1 %dim_1_vector_cmp.to.max.i, label %dim_1_vector_exit.i, label %dim_0_vector_pre_head.i

dim_1_vector_exit.i:                              ; preds = %dim_0_vector_exit.i
  %dim_2_vector_inc_ind_var.i = add i64 %dim_2_vector_ind_var.i, 1
  %dim_2_vector_cmp.to.max.i = icmp eq i64 %dim_2_vector_inc_ind_var.i, %30
  br i1 %dim_2_vector_cmp.to.max.i, label %scalarIf.i, label %dim_1_vector_pre_head.i

scalarIf.i:                                       ; preds = %dim_1_vector_exit.i, %WGLoopsEntry.i
  %96 = icmp eq i64 %35, %num.vector.wi.i
  br i1 %96, label %__spmv_csr_scalar_kernel_separated_args.exit, label %dim_1_pre_head.i

dim_1_pre_head.i:                                 ; preds = %dim_1_exit.i, %scalarIf.i
  %dim_2_ind_var.i = phi i64 [ %dim_2_inc_ind_var.i, %dim_1_exit.i ], [ 0, %scalarIf.i ]
  br label %dim_0_pre_head.i

dim_0_pre_head.i:                                 ; preds = %dim_0_exit.i, %dim_1_pre_head.i
  %dim_1_ind_var.i = phi i64 [ 0, %dim_1_pre_head.i ], [ %dim_1_inc_ind_var.i, %dim_0_exit.i ]
  br label %scalar_kernel_entry.i

scalar_kernel_entry.i:                            ; preds = %for.end.i, %dim_0_pre_head.i
  %dim_0_ind_var.i = phi i64 [ 0, %dim_0_pre_head.i ], [ %dim_0_inc_ind_var.i, %for.end.i ]
  %dim_0_tid.i = phi i64 [ %max.vector.gid.i, %dim_0_pre_head.i ], [ %dim_0_inc_tid.i, %for.end.i ]
  %conv.i = trunc i64 %dim_0_tid.i to i32
  %idxprom.i = sext i32 %conv.i to i64
  %arrayidx.i = getelementptr inbounds i32 addrspace(1)* %10, i64 %idxprom.i
  %97 = load i32 addrspace(1)* %arrayidx.i, align 4
  %add.i = add nsw i32 %conv.i, 1
  %idxprom2.i = sext i32 %add.i to i64
  %arrayidx3.i = getelementptr inbounds i32 addrspace(1)* %10, i64 %idxprom2.i
  %98 = load i32 addrspace(1)* %arrayidx3.i, align 4
  %cmp41.i = icmp slt i32 %97, %98
  br i1 %cmp41.i, label %for.body.lr.ph.i, label %for.end.i

for.body.lr.ph.i:                                 ; preds = %scalar_kernel_entry.i
  %99 = sext i32 %97 to i64
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %for.body.lr.ph.i
  %indvars.iv.i = phi i64 [ %99, %for.body.lr.ph.i ], [ %indvars.iv.next.i, %for.body.i ]
  %t.02.i = phi float [ 0.000000e+00, %for.body.lr.ph.i ], [ %add12.i, %for.body.i ]
  %arrayidx7.i = getelementptr inbounds i32 addrspace(1)* %7, i64 %indvars.iv.i
  %100 = load i32 addrspace(1)* %arrayidx7.i, align 4
  %arrayidx9.i = getelementptr inbounds float addrspace(1)* %1, i64 %indvars.iv.i
  %101 = load float addrspace(1)* %arrayidx9.i, align 4
  %idxprom10.i = sext i32 %100 to i64
  %arrayidx11.i = getelementptr inbounds float addrspace(1)* %4, i64 %idxprom10.i
  %102 = load float addrspace(1)* %arrayidx11.i, align 4
  %mul.i = fmul float %101, %102
  %add12.i = fadd float %t.02.i, %mul.i
  %indvars.iv.next.i = add i64 %indvars.iv.i, 1
  %lftr.wideiv.i = trunc i64 %indvars.iv.next.i to i32
  %exitcond.i = icmp eq i32 %lftr.wideiv.i, %98
  br i1 %exitcond.i, label %for.end.i, label %for.body.i

for.end.i:                                        ; preds = %for.body.i, %scalar_kernel_entry.i
  %t.0.lcssa.i = phi float [ 0.000000e+00, %scalar_kernel_entry.i ], [ %add12.i, %for.body.i ]
  %arrayidx14.i = getelementptr inbounds float addrspace(1)* %16, i64 %idxprom.i
  store float %t.0.lcssa.i, float addrspace(1)* %arrayidx14.i, align 4
  %dim_0_inc_ind_var.i = add i64 %dim_0_ind_var.i, 1
  %dim_0_cmp.to.max.i = icmp eq i64 %dim_0_inc_ind_var.i, %scalar.size.i
  %dim_0_inc_tid.i = add i64 %dim_0_tid.i, 1
  br i1 %dim_0_cmp.to.max.i, label %dim_0_exit.i, label %scalar_kernel_entry.i

dim_0_exit.i:                                     ; preds = %for.end.i
  %dim_1_inc_ind_var.i = add i64 %dim_1_ind_var.i, 1
  %dim_1_cmp.to.max.i = icmp eq i64 %dim_1_inc_ind_var.i, %28
  br i1 %dim_1_cmp.to.max.i, label %dim_1_exit.i, label %dim_0_pre_head.i

dim_1_exit.i:                                     ; preds = %dim_0_exit.i
  %dim_2_inc_ind_var.i = add i64 %dim_2_ind_var.i, 1
  %dim_2_cmp.to.max.i = icmp eq i64 %dim_2_inc_ind_var.i, %30
  br i1 %dim_2_cmp.to.max.i, label %__spmv_csr_scalar_kernel_separated_args.exit, label %dim_1_pre_head.i

__spmv_csr_scalar_kernel_separated_args.exit:     ; preds = %entry, %scalarIf.i, %dim_1_exit.i
  ret void
}

define void @spmv_csr_vector_kernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32 addrspace(1)**
  %7 = load i32 addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32 addrspace(1)**
  %10 = load i32 addrspace(1)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to float addrspace(1)**
  %16 = load float addrspace(1)** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 48
  %18 = bitcast i8* %17 to i8 addrspace(3)**
  %19 = load i8 addrspace(3)** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 56
  %21 = bitcast i8* %20 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %22 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 64
  %24 = bitcast i8* %23 to i64**
  %25 = load i64** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 80
  %27 = bitcast i8* %26 to <{ [4 x i64] }>**
  %28 = load <{ [4 x i64] }>** %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 96
  %30 = bitcast i8* %29 to i64*
  %31 = load i64* %30, align 8
  %32 = getelementptr i8* %pBuffer, i64 104
  %33 = bitcast i8* %32 to i8**
  %34 = load i8** %33, align 8
  %35 = bitcast i8 addrspace(3)* %19 to [128 x float] addrspace(3)*
  br label %SyncBB150.outer.i

SyncBB150.outer.i:                                ; preds = %thenBB188.i, %entry
  %CurrWI..0.ph.i = phi i64 [ 0, %entry ], [ %"CurrWI++192.i", %thenBB188.i ]
  %CurrSBIndex..0.ph.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride194.i", %thenBB188.i ]
  %currBarrier.0.ph.i = phi i32 [ 15, %entry ], [ %currBarrier.2.i, %thenBB188.i ]
  br label %SyncBB150.i

SyncBB150.i:                                      ; preds = %thenBB160.i, %SyncBB150.outer.i
  %CurrWI..0.i = phi i64 [ %"CurrWI++164.i", %thenBB160.i ], [ %CurrWI..0.ph.i, %SyncBB150.outer.i ]
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride166.i", %thenBB160.i ], [ %CurrSBIndex..0.ph.i, %SyncBB150.outer.i ]
  %36 = getelementptr <{ [4 x i64] }>* %28, i64 %CurrWI..0.i, i32 0, i64 0
  %37 = load i64* %36, align 8
  %conv.i = trunc i64 %37 to i32
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %conv.i, i32* %CastToValueType.i, align 4
  %and.i = and i32 %conv.i, 31
  %"&(pSB[currWI].offset)30.i" = add nuw i64 %CurrSBIndex..0.i, 4
  %"&pSB[currWI].offset31.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)30.i"
  %CastToValueType32.i = bitcast i8* %"&pSB[currWI].offset31.i" to i32*
  store i32 %and.i, i32* %CastToValueType32.i, align 4
  %38 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %22, i64 0, i32 3, i64 0
  %39 = load i64* %38, align 8
  %40 = load i64* %25, align 8
  %41 = shl i64 %39, 27
  %sext.i = ashr i64 %41, 32
  %mul.i = mul i64 %sext.i, %40
  %div5.i = sdiv i32 %conv.i, 32
  %conv61.i = zext i32 %div5.i to i64
  %add.i = add i64 %mul.i, %conv61.i
  %conv7.i = trunc i64 %add.i to i32
  %idxprom.i = sext i32 %conv.i to i64
  %arrayidx.i = getelementptr inbounds [128 x float] addrspace(3)* %35, i64 0, i64 %idxprom.i
  %"&(pSB[currWI].offset)64.i" = add nuw i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset65.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)64.i"
  %CastToValueType66.i = bitcast i8* %"&pSB[currWI].offset65.i" to float addrspace(3)**
  store float addrspace(3)* %arrayidx.i, float addrspace(3)** %CastToValueType66.i, align 8
  store volatile float 0.000000e+00, float addrspace(3)* %arrayidx.i, align 4
  %cmp.i = icmp slt i32 %conv7.i, %13
  br i1 %cmp.i, label %if.then.i, label %if.end85.i

if.then.i:                                        ; preds = %SyncBB150.i
  %idxprom9.i = sext i32 %conv7.i to i64
  %"&(pSB[currWI].offset)128.i" = add nuw i64 %CurrSBIndex..0.i, 16
  %"&pSB[currWI].offset129.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)128.i"
  %CastToValueType130.i = bitcast i8* %"&pSB[currWI].offset129.i" to i64*
  store i64 %idxprom9.i, i64* %CastToValueType130.i, align 8
  %arrayidx10.i = getelementptr inbounds i32 addrspace(1)* %10, i64 %idxprom9.i
  %42 = load i32 addrspace(1)* %arrayidx10.i, align 4
  %add11.i = add nsw i32 %conv7.i, 1
  %idxprom12.i = sext i32 %add11.i to i64
  %arrayidx13.i = getelementptr inbounds i32 addrspace(1)* %10, i64 %idxprom12.i
  %43 = load i32 addrspace(1)* %arrayidx13.i, align 4
  %loadedValue37.i = load i32* %CastToValueType32.i, align 4
  %add14.i = add nsw i32 %42, %loadedValue37.i
  %cmp152.i = icmp slt i32 %add14.i, %43
  br i1 %cmp152.i, label %for.body.lr.ph.i, label %for.end.i

for.body.lr.ph.i:                                 ; preds = %if.then.i
  %44 = sext i32 %add14.i to i64
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %for.body.lr.ph.i
  %indvars.iv.i = phi i64 [ %44, %for.body.lr.ph.i ], [ %indvars.iv.next.i, %for.body.i ]
  %mySum.03.i = phi float [ 0.000000e+00, %for.body.lr.ph.i ], [ %add24.i, %for.body.i ]
  %arrayidx18.i = getelementptr inbounds i32 addrspace(1)* %7, i64 %indvars.iv.i
  %45 = load i32 addrspace(1)* %arrayidx18.i, align 4
  %arrayidx20.i = getelementptr inbounds float addrspace(1)* %1, i64 %indvars.iv.i
  %46 = load float addrspace(1)* %arrayidx20.i, align 4
  %idxprom21.i = sext i32 %45 to i64
  %arrayidx22.i = getelementptr inbounds float addrspace(1)* %4, i64 %idxprom21.i
  %47 = load float addrspace(1)* %arrayidx22.i, align 4
  %mul23.i = fmul float %46, %47
  %add24.i = fadd float %mySum.03.i, %mul23.i
  %indvars.iv.next.i = add i64 %indvars.iv.i, 32
  %48 = trunc i64 %indvars.iv.next.i to i32
  %cmp15.i = icmp slt i32 %48, %43
  br i1 %cmp15.i, label %for.body.i, label %for.end.i

for.end.i:                                        ; preds = %for.body.i, %if.then.i
  %mySum.0.lcssa.i = phi float [ 0.000000e+00, %if.then.i ], [ %add24.i, %for.body.i ]
  %loadedValue126.i = load float addrspace(3)** %CastToValueType66.i, align 8
  store volatile float %mySum.0.lcssa.i, float addrspace(3)* %loadedValue126.i, align 4
  %check.WI.iter163.i = icmp ult i64 %CurrWI..0.i, %31
  br i1 %check.WI.iter163.i, label %thenBB160.i, label %SyncBB146.i

thenBB160.i:                                      ; preds = %for.end.i
  %"CurrWI++164.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride166.i" = add nuw i64 %CurrSBIndex..0.i, 32
  br label %SyncBB150.i

SyncBB146.i:                                      ; preds = %thenBB167.i, %for.end.i
  %CurrWI..1.i = phi i64 [ %"CurrWI++171.i", %thenBB167.i ], [ 0, %for.end.i ]
  %CurrSBIndex..1.i = phi i64 [ %"loadedCurrSB+Stride173.i", %thenBB167.i ], [ 0, %for.end.i ]
  %"&(pSB[currWI].offset)591.i" = or i64 %CurrSBIndex..1.i, 4
  %"&pSB[currWI].offset60.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)591.i"
  %CastToValueType61.i = bitcast i8* %"&pSB[currWI].offset60.i" to i32*
  %loadedValue62.i = load i32* %CastToValueType61.i, align 4
  %cmp28.i = icmp ult i32 %loadedValue62.i, 16
  br i1 %cmp28.i, label %if.then30.i, label %if.end.i

if.then30.i:                                      ; preds = %SyncBB146.i
  %"&pSB[currWI].offset26.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..1.i
  %CastToValueType27.i = bitcast i8* %"&pSB[currWI].offset26.i" to i32*
  %loadedValue28.i = load i32* %CastToValueType27.i, align 4
  %add31.i = add nsw i32 %loadedValue28.i, 16
  %idxprom32.i = sext i32 %add31.i to i64
  %arrayidx33.i = getelementptr inbounds [128 x float] addrspace(3)* %35, i64 0, i64 %idxprom32.i
  %49 = load volatile float addrspace(3)* %arrayidx33.i, align 4
  %"&(pSB[currWI].offset)11315.i" = or i64 %CurrSBIndex..1.i, 8
  %"&pSB[currWI].offset114.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)11315.i"
  %CastToValueType115.i = bitcast i8* %"&pSB[currWI].offset114.i" to float addrspace(3)**
  %loadedValue116.i = load float addrspace(3)** %CastToValueType115.i, align 8
  %50 = load volatile float addrspace(3)* %loadedValue116.i, align 4
  %add36.i = fadd float %50, %49
  store volatile float %add36.i, float addrspace(3)* %loadedValue116.i, align 4
  br label %if.end.i

if.end.i:                                         ; preds = %if.then30.i, %SyncBB146.i
  %check.WI.iter170.i = icmp ult i64 %CurrWI..1.i, %31
  br i1 %check.WI.iter170.i, label %thenBB167.i, label %SyncBB147.i

thenBB167.i:                                      ; preds = %if.end.i
  %"CurrWI++171.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride173.i" = add nuw i64 %CurrSBIndex..1.i, 32
  br label %SyncBB146.i

SyncBB147.i:                                      ; preds = %thenBB153.i, %if.end.i
  %CurrWI..2.i = phi i64 [ %"CurrWI++157.i", %thenBB153.i ], [ 0, %if.end.i ]
  %CurrSBIndex..2.i = phi i64 [ %"loadedCurrSB+Stride159.i", %thenBB153.i ], [ 0, %if.end.i ]
  %"&(pSB[currWI].offset)542.i" = or i64 %CurrSBIndex..2.i, 4
  %"&pSB[currWI].offset55.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)542.i"
  %CastToValueType56.i = bitcast i8* %"&pSB[currWI].offset55.i" to i32*
  %loadedValue57.i = load i32* %CastToValueType56.i, align 4
  %cmp37.i = icmp ult i32 %loadedValue57.i, 8
  br i1 %cmp37.i, label %if.then39.i, label %if.end46.i

if.then39.i:                                      ; preds = %SyncBB147.i
  %"&pSB[currWI].offset21.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..2.i
  %CastToValueType22.i = bitcast i8* %"&pSB[currWI].offset21.i" to i32*
  %loadedValue23.i = load i32* %CastToValueType22.i, align 4
  %add40.i = add nsw i32 %loadedValue23.i, 8
  %idxprom41.i = sext i32 %add40.i to i64
  %arrayidx42.i = getelementptr inbounds [128 x float] addrspace(3)* %35, i64 0, i64 %idxprom41.i
  %51 = load volatile float addrspace(3)* %arrayidx42.i, align 4
  %"&(pSB[currWI].offset)10313.i" = or i64 %CurrSBIndex..2.i, 8
  %"&pSB[currWI].offset104.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)10313.i"
  %CastToValueType105.i = bitcast i8* %"&pSB[currWI].offset104.i" to float addrspace(3)**
  %loadedValue106.i = load float addrspace(3)** %CastToValueType105.i, align 8
  %52 = load volatile float addrspace(3)* %loadedValue106.i, align 4
  %add45.i = fadd float %52, %51
  store volatile float %add45.i, float addrspace(3)* %loadedValue106.i, align 4
  br label %if.end46.i

if.end46.i:                                       ; preds = %if.then39.i, %SyncBB147.i
  %check.WI.iter156.i = icmp ult i64 %CurrWI..2.i, %31
  br i1 %check.WI.iter156.i, label %thenBB153.i, label %SyncBB145.i

thenBB153.i:                                      ; preds = %if.end46.i
  %"CurrWI++157.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride159.i" = add nuw i64 %CurrSBIndex..2.i, 32
  br label %SyncBB147.i

SyncBB145.i:                                      ; preds = %thenBB.i, %if.end46.i
  %CurrWI..3.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %if.end46.i ]
  %CurrSBIndex..3.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %if.end46.i ]
  %"&(pSB[currWI].offset)493.i" = or i64 %CurrSBIndex..3.i, 4
  %"&pSB[currWI].offset50.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)493.i"
  %CastToValueType51.i = bitcast i8* %"&pSB[currWI].offset50.i" to i32*
  %loadedValue52.i = load i32* %CastToValueType51.i, align 4
  %cmp47.i = icmp ult i32 %loadedValue52.i, 4
  br i1 %cmp47.i, label %if.then49.i, label %if.end56.i

if.then49.i:                                      ; preds = %SyncBB145.i
  %"&pSB[currWI].offset16.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..3.i
  %CastToValueType17.i = bitcast i8* %"&pSB[currWI].offset16.i" to i32*
  %loadedValue18.i = load i32* %CastToValueType17.i, align 4
  %add50.i = add nsw i32 %loadedValue18.i, 4
  %idxprom51.i = sext i32 %add50.i to i64
  %arrayidx52.i = getelementptr inbounds [128 x float] addrspace(3)* %35, i64 0, i64 %idxprom51.i
  %53 = load volatile float addrspace(3)* %arrayidx52.i, align 4
  %"&(pSB[currWI].offset)9311.i" = or i64 %CurrSBIndex..3.i, 8
  %"&pSB[currWI].offset94.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)9311.i"
  %CastToValueType95.i = bitcast i8* %"&pSB[currWI].offset94.i" to float addrspace(3)**
  %loadedValue96.i = load float addrspace(3)** %CastToValueType95.i, align 8
  %54 = load volatile float addrspace(3)* %loadedValue96.i, align 4
  %add55.i = fadd float %54, %53
  store volatile float %add55.i, float addrspace(3)* %loadedValue96.i, align 4
  br label %if.end56.i

if.end56.i:                                       ; preds = %if.then49.i, %SyncBB145.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..3.i, %31
  br i1 %check.WI.iter.i, label %thenBB.i, label %SyncBB.i

thenBB.i:                                         ; preds = %if.end56.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..3.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..3.i, 32
  br label %SyncBB145.i

SyncBB.i:                                         ; preds = %thenBB174.i, %if.end56.i
  %CurrWI..4.i = phi i64 [ %"CurrWI++178.i", %thenBB174.i ], [ 0, %if.end56.i ]
  %CurrSBIndex..4.i = phi i64 [ %"loadedCurrSB+Stride180.i", %thenBB174.i ], [ 0, %if.end56.i ]
  %"&(pSB[currWI].offset)444.i" = or i64 %CurrSBIndex..4.i, 4
  %"&pSB[currWI].offset45.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)444.i"
  %CastToValueType46.i = bitcast i8* %"&pSB[currWI].offset45.i" to i32*
  %loadedValue47.i = load i32* %CastToValueType46.i, align 4
  %cmp57.i = icmp ult i32 %loadedValue47.i, 2
  br i1 %cmp57.i, label %if.then59.i, label %if.end66.i

if.then59.i:                                      ; preds = %SyncBB.i
  %"&pSB[currWI].offset11.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..4.i
  %CastToValueType12.i = bitcast i8* %"&pSB[currWI].offset11.i" to i32*
  %loadedValue13.i = load i32* %CastToValueType12.i, align 4
  %add60.i = add nsw i32 %loadedValue13.i, 2
  %idxprom61.i = sext i32 %add60.i to i64
  %arrayidx62.i = getelementptr inbounds [128 x float] addrspace(3)* %35, i64 0, i64 %idxprom61.i
  %55 = load volatile float addrspace(3)* %arrayidx62.i, align 4
  %"&(pSB[currWI].offset)839.i" = or i64 %CurrSBIndex..4.i, 8
  %"&pSB[currWI].offset84.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)839.i"
  %CastToValueType85.i = bitcast i8* %"&pSB[currWI].offset84.i" to float addrspace(3)**
  %loadedValue86.i = load float addrspace(3)** %CastToValueType85.i, align 8
  %56 = load volatile float addrspace(3)* %loadedValue86.i, align 4
  %add65.i = fadd float %56, %55
  store volatile float %add65.i, float addrspace(3)* %loadedValue86.i, align 4
  br label %if.end66.i

if.end66.i:                                       ; preds = %if.then59.i, %SyncBB.i
  %check.WI.iter177.i = icmp ult i64 %CurrWI..4.i, %31
  br i1 %check.WI.iter177.i, label %thenBB174.i, label %SyncBB148.i

thenBB174.i:                                      ; preds = %if.end66.i
  %"CurrWI++178.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride180.i" = add nuw i64 %CurrSBIndex..4.i, 32
  br label %SyncBB.i

SyncBB148.i:                                      ; preds = %thenBB181.i, %if.end66.i
  %CurrWI..5.i = phi i64 [ %"CurrWI++185.i", %thenBB181.i ], [ 0, %if.end66.i ]
  %CurrSBIndex..5.i = phi i64 [ %"loadedCurrSB+Stride187.i", %thenBB181.i ], [ 0, %if.end66.i ]
  %"&(pSB[currWI].offset)395.i" = or i64 %CurrSBIndex..5.i, 4
  %"&pSB[currWI].offset40.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)395.i"
  %CastToValueType41.i = bitcast i8* %"&pSB[currWI].offset40.i" to i32*
  %loadedValue42.i = load i32* %CastToValueType41.i, align 4
  %cmp67.i = icmp eq i32 %loadedValue42.i, 0
  %"&(pSB[currWI].offset)1376.i" = or i64 %CurrSBIndex..5.i, 24
  %"&pSB[currWI].offset138.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)1376.i"
  %CastToValueType139.i = bitcast i8* %"&pSB[currWI].offset138.i" to i1*
  store i1 %cmp67.i, i1* %CastToValueType139.i, align 1
  br i1 %cmp67.i, label %if.then69.i, label %if.end76.i

if.then69.i:                                      ; preds = %SyncBB148.i
  %"&pSB[currWI].offset7.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..5.i
  %CastToValueType8.i = bitcast i8* %"&pSB[currWI].offset7.i" to i32*
  %loadedValue.i = load i32* %CastToValueType8.i, align 4
  %add70.i = add nsw i32 %loadedValue.i, 1
  %idxprom71.i = sext i32 %add70.i to i64
  %arrayidx72.i = getelementptr inbounds [128 x float] addrspace(3)* %35, i64 0, i64 %idxprom71.i
  %57 = load volatile float addrspace(3)* %arrayidx72.i, align 4
  %"&(pSB[currWI].offset)737.i" = or i64 %CurrSBIndex..5.i, 8
  %"&pSB[currWI].offset74.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)737.i"
  %CastToValueType75.i = bitcast i8* %"&pSB[currWI].offset74.i" to float addrspace(3)**
  %loadedValue76.i = load float addrspace(3)** %CastToValueType75.i, align 8
  %58 = load volatile float addrspace(3)* %loadedValue76.i, align 4
  %add75.i = fadd float %58, %57
  store volatile float %add75.i, float addrspace(3)* %loadedValue76.i, align 4
  br label %if.end76.i

if.end76.i:                                       ; preds = %if.then69.i, %SyncBB148.i
  %check.WI.iter184.i = icmp ult i64 %CurrWI..5.i, %31
  br i1 %check.WI.iter184.i, label %thenBB181.i, label %SyncBB149.i

thenBB181.i:                                      ; preds = %if.end76.i
  %"CurrWI++185.i" = add nuw i64 %CurrWI..5.i, 1
  %"loadedCurrSB+Stride187.i" = add nuw i64 %CurrSBIndex..5.i, 32
  br label %SyncBB148.i

SyncBB149.i:                                      ; preds = %thenBB188.i, %if.end76.i
  %CurrWI..6.i = phi i64 [ %"CurrWI++192.i", %thenBB188.i ], [ 0, %if.end76.i ]
  %CurrSBIndex..6.i = phi i64 [ %"loadedCurrSB+Stride194.i", %thenBB188.i ], [ 0, %if.end76.i ]
  %currBarrier.1.i = phi i32 [ %currBarrier.2.i, %thenBB188.i ], [ 5, %if.end76.i ]
  %"&(pSB[currWI].offset)141.i" = add nuw i64 %CurrSBIndex..6.i, 24
  %"&pSB[currWI].offset142.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)141.i"
  %CastToValueType143.i = bitcast i8* %"&pSB[currWI].offset142.i" to i1*
  %loadedValue144.i = load i1* %CastToValueType143.i, align 1
  br i1 %loadedValue144.i, label %if.then79.i, label %if.end85.i

if.then79.i:                                      ; preds = %SyncBB149.i
  %"&(pSB[currWI].offset)68.i" = add nuw i64 %CurrSBIndex..6.i, 8
  %"&pSB[currWI].offset69.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)68.i"
  %CastToValueType70.i = bitcast i8* %"&pSB[currWI].offset69.i" to float addrspace(3)**
  %loadedValue71.i = load float addrspace(3)** %CastToValueType70.i, align 8
  %59 = load volatile float addrspace(3)* %loadedValue71.i, align 4
  %"&(pSB[currWI].offset)132.i" = add nuw i64 %CurrSBIndex..6.i, 16
  %"&pSB[currWI].offset133.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)132.i"
  %CastToValueType134.i = bitcast i8* %"&pSB[currWI].offset133.i" to i64*
  %loadedValue135.i = load i64* %CastToValueType134.i, align 8
  %arrayidx83.i = getelementptr inbounds float addrspace(1)* %16, i64 %loadedValue135.i
  store float %59, float addrspace(1)* %arrayidx83.i, align 4
  br label %if.end85.i

if.end85.i:                                       ; preds = %if.then79.i, %SyncBB149.i, %SyncBB150.i
  %CurrWI..7.i = phi i64 [ %CurrWI..6.i, %if.then79.i ], [ %CurrWI..6.i, %SyncBB149.i ], [ %CurrWI..0.i, %SyncBB150.i ]
  %CurrSBIndex..7.i = phi i64 [ %CurrSBIndex..6.i, %if.then79.i ], [ %CurrSBIndex..6.i, %SyncBB149.i ], [ %CurrSBIndex..0.i, %SyncBB150.i ]
  %currBarrier.2.i = phi i32 [ %currBarrier.1.i, %if.then79.i ], [ %currBarrier.1.i, %SyncBB149.i ], [ %currBarrier.0.ph.i, %SyncBB150.i ]
  %check.WI.iter191.i = icmp ult i64 %CurrWI..7.i, %31
  br i1 %check.WI.iter191.i, label %thenBB188.i, label %__spmv_csr_vector_kernel_separated_args.exit

thenBB188.i:                                      ; preds = %if.end85.i
  %"CurrWI++192.i" = add nuw i64 %CurrWI..7.i, 1
  %"loadedCurrSB+Stride194.i" = add nuw i64 %CurrSBIndex..7.i, 32
  %cond.i = icmp eq i32 %currBarrier.2.i, 5
  br i1 %cond.i, label %SyncBB149.i, label %SyncBB150.outer.i

__spmv_csr_vector_kernel_separated_args.exit:     ; preds = %if.end85.i
  ret void
}

define void @__Vectorized_.spmv_csr_vector_kernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32 addrspace(1)**
  %7 = load i32 addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32 addrspace(1)**
  %10 = load i32 addrspace(1)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to float addrspace(1)**
  %16 = load float addrspace(1)** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 48
  %18 = bitcast i8* %17 to i8 addrspace(3)**
  %19 = load i8 addrspace(3)** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 56
  %21 = bitcast i8* %20 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %22 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 64
  %24 = bitcast i8* %23 to i64**
  %25 = load i64** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 80
  %27 = bitcast i8* %26 to <{ [4 x i64] }>**
  %28 = load <{ [4 x i64] }>** %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 96
  %30 = bitcast i8* %29 to i64*
  %31 = load i64* %30, align 8
  %32 = getelementptr i8* %pBuffer, i64 104
  %33 = bitcast i8* %32 to i8**
  %34 = load i8** %33, align 8
  %35 = bitcast i8 addrspace(3)* %19 to [128 x float] addrspace(3)*
  %temp97.i = insertelement <16 x i32> undef, i32 %13, i32 0
  %vector98.i = shufflevector <16 x i32> %temp97.i, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB4569.outer.i

SyncBB4569.outer.i:                               ; preds = %thenBB4611.i, %thenBB4603.i, %thenBB4595.i, %thenBB4587.i, %thenBB4579.i, %thenBB4572.i, %entry
  %CurrWI..0.ph.i = phi i64 [ 0, %entry ], [ %"CurrWI++4576.i", %thenBB4572.i ], [ %"CurrWI++4583.i", %thenBB4579.i ], [ %"CurrWI++4591.i", %thenBB4587.i ], [ %"CurrWI++4599.i", %thenBB4595.i ], [ %"CurrWI++4607.i", %thenBB4603.i ], [ %"CurrWI++4615.i", %thenBB4611.i ]
  %CurrSBIndex..0.ph.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride4578.i", %thenBB4572.i ], [ %"loadedCurrSB+Stride4585.i", %thenBB4579.i ], [ %"loadedCurrSB+Stride4593.i", %thenBB4587.i ], [ %"loadedCurrSB+Stride4601.i", %thenBB4595.i ], [ %"loadedCurrSB+Stride4609.i", %thenBB4603.i ], [ %"loadedCurrSB+Stride4617.i", %thenBB4611.i ]
  %currBarrier.0.ph.i = phi i32 [ 14, %entry ], [ %currBarrier.2.i, %thenBB4572.i ], [ %currBarrier.4.i, %thenBB4579.i ], [ %currBarrier.6.i, %thenBB4587.i ], [ %currBarrier.8.i, %thenBB4595.i ], [ %currBarrier.10.i, %thenBB4603.i ], [ %currBarrier.12.i, %thenBB4611.i ]
  br label %SyncBB4569.i

SyncBB4569.i:                                     ; preds = %thenBB.i, %SyncBB4569.outer.i
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ %CurrWI..0.ph.i, %SyncBB4569.outer.i ]
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ %CurrSBIndex..0.ph.i, %SyncBB4569.outer.i ]
  %36 = getelementptr <{ [4 x i64] }>* %28, i64 %CurrWI..0.i, i32 0, i64 0
  %37 = load i64* %36, align 8
  %broadcast1.i = insertelement <16 x i64> undef, i64 %37, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %38 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %conv75.i = trunc <16 x i64> %38 to <16 x i32>
  %and76.i = and <16 x i32> %conv75.i, <i32 31, i32 31, i32 31, i32 31, i32 31, i32 31, i32 31, i32 31, i32 31, i32 31, i32 31, i32 31, i32 31, i32 31, i32 31, i32 31>
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to <16 x i32>*
  store <16 x i32> %and76.i, <16 x i32>* %CastToValueType.i, align 64
  %39 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %22, i64 0, i32 3, i64 0
  %40 = load i64* %39, align 8
  %41 = load i64* %25, align 8
  %42 = shl i64 %40, 27
  %sext.i = ashr i64 %42, 32
  %mul.i = mul i64 %sext.i, %41
  %temp.i = insertelement <16 x i64> undef, i64 %mul.i, i32 0
  %vector.i = shufflevector <16 x i64> %temp.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %div577.i = sdiv <16 x i32> %conv75.i, <i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32>
  %conv6178.i = zext <16 x i32> %div577.i to <16 x i64>
  %add79.i = add <16 x i64> %vector.i, %conv6178.i
  %conv780.i = trunc <16 x i64> %add79.i to <16 x i32>
  %43 = extractelement <16 x i32> %conv75.i, i32 0
  %"&(pSB[currWI].offset)2952.i" = add nuw i64 %CurrSBIndex..0.i, 64
  %"&pSB[currWI].offset2953.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)2952.i"
  %CastToValueType2954.i = bitcast i8* %"&pSB[currWI].offset2953.i" to i32*
  store i32 %43, i32* %CastToValueType2954.i, align 4
  %extract.i = sext i32 %43 to i64
  %"&(pSB[currWI].offset)2981.i" = add nuw i64 %CurrSBIndex..0.i, 72
  %"&pSB[currWI].offset2982.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)2981.i"
  %CastToValueType2983.i = bitcast i8* %"&pSB[currWI].offset2982.i" to i64*
  store i64 %extract.i, i64* %CastToValueType2983.i, align 8
  %44 = getelementptr inbounds [128 x float] addrspace(3)* %35, i64 0, i64 %extract.i
  %"&(pSB[currWI].offset)3885.i" = add nuw i64 %CurrSBIndex..0.i, 80
  %"&pSB[currWI].offset3886.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3885.i"
  %CastToValueType3887.i = bitcast i8* %"&pSB[currWI].offset3886.i" to float addrspace(3)**
  store float addrspace(3)* %44, float addrspace(3)** %CastToValueType3887.i, align 8
  %ptrTypeCast.i = bitcast float addrspace(3)* %44 to <16 x float> addrspace(3)*
  store <16 x float> zeroinitializer, <16 x float> addrspace(3)* %ptrTypeCast.i, align 4
  %cmp.i = icmp slt <16 x i32> %conv780.i, %vector98.i
  %"&(pSB[currWI].offset)3949.i" = add nuw i64 %CurrSBIndex..0.i, 96
  %"&pSB[currWI].offset3950.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3949.i"
  %CastToValueType3951.i = bitcast i8* %"&pSB[currWI].offset3950.i" to <16 x i1>*
  store <16 x i1> %cmp.i, <16 x i1>* %CastToValueType3951.i, align 16
  %extract117.i = extractelement <16 x i1> %cmp.i, i32 0
  %"&(pSB[currWI].offset)3988.i" = add nuw i64 %CurrSBIndex..0.i, 98
  %"&pSB[currWI].offset3989.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3988.i"
  %CastToValueType3990.i = bitcast i8* %"&pSB[currWI].offset3989.i" to i1*
  store i1 %extract117.i, i1* %CastToValueType3990.i, align 1
  %extract118.i = extractelement <16 x i1> %cmp.i, i32 1
  %extract119.i = extractelement <16 x i1> %cmp.i, i32 2
  %extract120.i = extractelement <16 x i1> %cmp.i, i32 3
  %extract121.i = extractelement <16 x i1> %cmp.i, i32 4
  %extract122.i = extractelement <16 x i1> %cmp.i, i32 5
  %extract123.i = extractelement <16 x i1> %cmp.i, i32 6
  %extract124.i = extractelement <16 x i1> %cmp.i, i32 7
  %extract125.i = extractelement <16 x i1> %cmp.i, i32 8
  %extract126.i = extractelement <16 x i1> %cmp.i, i32 9
  %extract127.i = extractelement <16 x i1> %cmp.i, i32 10
  %extract128.i = extractelement <16 x i1> %cmp.i, i32 11
  %extract129.i = extractelement <16 x i1> %cmp.i, i32 12
  %extract130.i = extractelement <16 x i1> %cmp.i, i32 13
  %extract131.i = extractelement <16 x i1> %cmp.i, i32 14
  %extract132.i = extractelement <16 x i1> %cmp.i, i32 15
  %idxprom9100.i = sext <16 x i32> %conv780.i to <16 x i64>
  %extract101.i = extractelement <16 x i64> %idxprom9100.i, i32 0
  %"&(pSB[currWI].offset)4032.i" = add nuw i64 %CurrSBIndex..0.i, 104
  %"&pSB[currWI].offset4033.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4032.i"
  %CastToValueType4034.i = bitcast i8* %"&pSB[currWI].offset4033.i" to i64*
  store i64 %extract101.i, i64* %CastToValueType4034.i, align 8
  %extract102.i = extractelement <16 x i64> %idxprom9100.i, i32 1
  %"&(pSB[currWI].offset)4046.i" = add nuw i64 %CurrSBIndex..0.i, 112
  %"&pSB[currWI].offset4047.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4046.i"
  %CastToValueType4048.i = bitcast i8* %"&pSB[currWI].offset4047.i" to i64*
  store i64 %extract102.i, i64* %CastToValueType4048.i, align 8
  %extract103.i = extractelement <16 x i64> %idxprom9100.i, i32 2
  %"&(pSB[currWI].offset)4055.i" = add nuw i64 %CurrSBIndex..0.i, 120
  %"&pSB[currWI].offset4056.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4055.i"
  %CastToValueType4057.i = bitcast i8* %"&pSB[currWI].offset4056.i" to i64*
  store i64 %extract103.i, i64* %CastToValueType4057.i, align 8
  %extract104.i = extractelement <16 x i64> %idxprom9100.i, i32 3
  %"&(pSB[currWI].offset)4064.i" = add nuw i64 %CurrSBIndex..0.i, 128
  %"&pSB[currWI].offset4065.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4064.i"
  %CastToValueType4066.i = bitcast i8* %"&pSB[currWI].offset4065.i" to i64*
  store i64 %extract104.i, i64* %CastToValueType4066.i, align 8
  %extract105.i = extractelement <16 x i64> %idxprom9100.i, i32 4
  %"&(pSB[currWI].offset)4073.i" = add nuw i64 %CurrSBIndex..0.i, 136
  %"&pSB[currWI].offset4074.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4073.i"
  %CastToValueType4075.i = bitcast i8* %"&pSB[currWI].offset4074.i" to i64*
  store i64 %extract105.i, i64* %CastToValueType4075.i, align 8
  %extract106.i = extractelement <16 x i64> %idxprom9100.i, i32 5
  %"&(pSB[currWI].offset)4082.i" = add nuw i64 %CurrSBIndex..0.i, 144
  %"&pSB[currWI].offset4083.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4082.i"
  %CastToValueType4084.i = bitcast i8* %"&pSB[currWI].offset4083.i" to i64*
  store i64 %extract106.i, i64* %CastToValueType4084.i, align 8
  %extract107.i = extractelement <16 x i64> %idxprom9100.i, i32 6
  %"&(pSB[currWI].offset)4091.i" = add nuw i64 %CurrSBIndex..0.i, 152
  %"&pSB[currWI].offset4092.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4091.i"
  %CastToValueType4093.i = bitcast i8* %"&pSB[currWI].offset4092.i" to i64*
  store i64 %extract107.i, i64* %CastToValueType4093.i, align 8
  %extract108.i = extractelement <16 x i64> %idxprom9100.i, i32 7
  %"&(pSB[currWI].offset)4100.i" = add nuw i64 %CurrSBIndex..0.i, 160
  %"&pSB[currWI].offset4101.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4100.i"
  %CastToValueType4102.i = bitcast i8* %"&pSB[currWI].offset4101.i" to i64*
  store i64 %extract108.i, i64* %CastToValueType4102.i, align 8
  %extract109.i = extractelement <16 x i64> %idxprom9100.i, i32 8
  %"&(pSB[currWI].offset)4109.i" = add nuw i64 %CurrSBIndex..0.i, 168
  %"&pSB[currWI].offset4110.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4109.i"
  %CastToValueType4111.i = bitcast i8* %"&pSB[currWI].offset4110.i" to i64*
  store i64 %extract109.i, i64* %CastToValueType4111.i, align 8
  %extract110.i = extractelement <16 x i64> %idxprom9100.i, i32 9
  %"&(pSB[currWI].offset)4118.i" = add nuw i64 %CurrSBIndex..0.i, 176
  %"&pSB[currWI].offset4119.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4118.i"
  %CastToValueType4120.i = bitcast i8* %"&pSB[currWI].offset4119.i" to i64*
  store i64 %extract110.i, i64* %CastToValueType4120.i, align 8
  %extract111.i = extractelement <16 x i64> %idxprom9100.i, i32 10
  %"&(pSB[currWI].offset)4127.i" = add nuw i64 %CurrSBIndex..0.i, 184
  %"&pSB[currWI].offset4128.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4127.i"
  %CastToValueType4129.i = bitcast i8* %"&pSB[currWI].offset4128.i" to i64*
  store i64 %extract111.i, i64* %CastToValueType4129.i, align 8
  %extract112.i = extractelement <16 x i64> %idxprom9100.i, i32 11
  %"&(pSB[currWI].offset)4136.i" = add nuw i64 %CurrSBIndex..0.i, 192
  %"&pSB[currWI].offset4137.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4136.i"
  %CastToValueType4138.i = bitcast i8* %"&pSB[currWI].offset4137.i" to i64*
  store i64 %extract112.i, i64* %CastToValueType4138.i, align 8
  %extract113.i = extractelement <16 x i64> %idxprom9100.i, i32 12
  %"&(pSB[currWI].offset)4145.i" = add nuw i64 %CurrSBIndex..0.i, 200
  %"&pSB[currWI].offset4146.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4145.i"
  %CastToValueType4147.i = bitcast i8* %"&pSB[currWI].offset4146.i" to i64*
  store i64 %extract113.i, i64* %CastToValueType4147.i, align 8
  %extract114.i = extractelement <16 x i64> %idxprom9100.i, i32 13
  %"&(pSB[currWI].offset)4154.i" = add nuw i64 %CurrSBIndex..0.i, 208
  %"&pSB[currWI].offset4155.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4154.i"
  %CastToValueType4156.i = bitcast i8* %"&pSB[currWI].offset4155.i" to i64*
  store i64 %extract114.i, i64* %CastToValueType4156.i, align 8
  %extract115.i = extractelement <16 x i64> %idxprom9100.i, i32 14
  %"&(pSB[currWI].offset)4163.i" = add nuw i64 %CurrSBIndex..0.i, 216
  %"&pSB[currWI].offset4164.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4163.i"
  %CastToValueType4165.i = bitcast i8* %"&pSB[currWI].offset4164.i" to i64*
  store i64 %extract115.i, i64* %CastToValueType4165.i, align 8
  %extract116.i = extractelement <16 x i64> %idxprom9100.i, i32 15
  %"&(pSB[currWI].offset)4172.i" = add nuw i64 %CurrSBIndex..0.i, 224
  %"&pSB[currWI].offset4173.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4172.i"
  %CastToValueType4174.i = bitcast i8* %"&pSB[currWI].offset4173.i" to i64*
  store i64 %extract116.i, i64* %CastToValueType4174.i, align 8
  %45 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract102.i
  %46 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract103.i
  %47 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract104.i
  %48 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract105.i
  %49 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract106.i
  %50 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract107.i
  %51 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract108.i
  %52 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract109.i
  %53 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract110.i
  %54 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract111.i
  %55 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract112.i
  %56 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract113.i
  %57 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract114.i
  %58 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract115.i
  %59 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract116.i
  br i1 %extract117.i, label %preload1669.i, label %postload1670.i

preload1669.i:                                    ; preds = %SyncBB4569.i
  %60 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract101.i
  %masked_load.i = load i32 addrspace(1)* %60, align 4
  br label %postload1670.i

postload1670.i:                                   ; preds = %preload1669.i, %SyncBB4569.i
  %phi1671.i = phi i32 [ undef, %SyncBB4569.i ], [ %masked_load.i, %preload1669.i ]
  br i1 %extract118.i, label %preload1687.i, label %postload1688.i

preload1687.i:                                    ; preds = %postload1670.i
  %masked_load484.i = load i32 addrspace(1)* %45, align 4
  br label %postload1688.i

postload1688.i:                                   ; preds = %preload1687.i, %postload1670.i
  %phi1689.i = phi i32 [ undef, %postload1670.i ], [ %masked_load484.i, %preload1687.i ]
  br i1 %extract119.i, label %preload1693.i, label %postload1694.i

preload1693.i:                                    ; preds = %postload1688.i
  %masked_load485.i = load i32 addrspace(1)* %46, align 4
  br label %postload1694.i

postload1694.i:                                   ; preds = %preload1693.i, %postload1688.i
  %phi1695.i = phi i32 [ undef, %postload1688.i ], [ %masked_load485.i, %preload1693.i ]
  br i1 %extract120.i, label %preload1699.i, label %postload1700.i

preload1699.i:                                    ; preds = %postload1694.i
  %masked_load486.i = load i32 addrspace(1)* %47, align 4
  br label %postload1700.i

postload1700.i:                                   ; preds = %preload1699.i, %postload1694.i
  %phi1701.i = phi i32 [ undef, %postload1694.i ], [ %masked_load486.i, %preload1699.i ]
  br i1 %extract121.i, label %preload1618.i, label %postload1619.i

preload1618.i:                                    ; preds = %postload1700.i
  %masked_load487.i = load i32 addrspace(1)* %48, align 4
  br label %postload1619.i

postload1619.i:                                   ; preds = %preload1618.i, %postload1700.i
  %phi1620.i = phi i32 [ undef, %postload1700.i ], [ %masked_load487.i, %preload1618.i ]
  br i1 %extract122.i, label %preload1624.i, label %postload1625.i

preload1624.i:                                    ; preds = %postload1619.i
  %masked_load488.i = load i32 addrspace(1)* %49, align 4
  br label %postload1625.i

postload1625.i:                                   ; preds = %preload1624.i, %postload1619.i
  %phi1626.i = phi i32 [ undef, %postload1619.i ], [ %masked_load488.i, %preload1624.i ]
  br i1 %extract123.i, label %preload1630.i, label %postload1631.i

preload1630.i:                                    ; preds = %postload1625.i
  %masked_load489.i = load i32 addrspace(1)* %50, align 4
  br label %postload1631.i

postload1631.i:                                   ; preds = %preload1630.i, %postload1625.i
  %phi1632.i = phi i32 [ undef, %postload1625.i ], [ %masked_load489.i, %preload1630.i ]
  br i1 %extract124.i, label %preload1636.i, label %postload1637.i

preload1636.i:                                    ; preds = %postload1631.i
  %masked_load490.i = load i32 addrspace(1)* %51, align 4
  br label %postload1637.i

postload1637.i:                                   ; preds = %preload1636.i, %postload1631.i
  %phi1638.i = phi i32 [ undef, %postload1631.i ], [ %masked_load490.i, %preload1636.i ]
  br i1 %extract125.i, label %preload1642.i, label %postload1643.i

preload1642.i:                                    ; preds = %postload1637.i
  %masked_load491.i = load i32 addrspace(1)* %52, align 4
  br label %postload1643.i

postload1643.i:                                   ; preds = %preload1642.i, %postload1637.i
  %phi1644.i = phi i32 [ undef, %postload1637.i ], [ %masked_load491.i, %preload1642.i ]
  br i1 %extract126.i, label %preload1648.i, label %postload1649.i

preload1648.i:                                    ; preds = %postload1643.i
  %masked_load492.i = load i32 addrspace(1)* %53, align 4
  br label %postload1649.i

postload1649.i:                                   ; preds = %preload1648.i, %postload1643.i
  %phi1650.i = phi i32 [ undef, %postload1643.i ], [ %masked_load492.i, %preload1648.i ]
  br i1 %extract127.i, label %preload1654.i, label %postload1655.i

preload1654.i:                                    ; preds = %postload1649.i
  %masked_load493.i = load i32 addrspace(1)* %54, align 4
  br label %postload1655.i

postload1655.i:                                   ; preds = %preload1654.i, %postload1649.i
  %phi1656.i = phi i32 [ undef, %postload1649.i ], [ %masked_load493.i, %preload1654.i ]
  br i1 %extract128.i, label %preload1660.i, label %postload1661.i

preload1660.i:                                    ; preds = %postload1655.i
  %masked_load494.i = load i32 addrspace(1)* %55, align 4
  br label %postload1661.i

postload1661.i:                                   ; preds = %preload1660.i, %postload1655.i
  %phi1662.i = phi i32 [ undef, %postload1655.i ], [ %masked_load494.i, %preload1660.i ]
  br i1 %extract129.i, label %preload1741.i, label %postload1742.i

preload1741.i:                                    ; preds = %postload1661.i
  %masked_load495.i = load i32 addrspace(1)* %56, align 4
  br label %postload1742.i

postload1742.i:                                   ; preds = %preload1741.i, %postload1661.i
  %phi1743.i = phi i32 [ undef, %postload1661.i ], [ %masked_load495.i, %preload1741.i ]
  br i1 %extract130.i, label %preload1747.i, label %postload1748.i

preload1747.i:                                    ; preds = %postload1742.i
  %masked_load496.i = load i32 addrspace(1)* %57, align 4
  br label %postload1748.i

postload1748.i:                                   ; preds = %preload1747.i, %postload1742.i
  %phi1749.i = phi i32 [ undef, %postload1742.i ], [ %masked_load496.i, %preload1747.i ]
  br i1 %extract131.i, label %preload1753.i, label %postload1754.i

preload1753.i:                                    ; preds = %postload1748.i
  %masked_load497.i = load i32 addrspace(1)* %58, align 4
  br label %postload1754.i

postload1754.i:                                   ; preds = %preload1753.i, %postload1748.i
  %phi1755.i = phi i32 [ undef, %postload1748.i ], [ %masked_load497.i, %preload1753.i ]
  br i1 %extract132.i, label %preload1759.i, label %postload1760.i

preload1759.i:                                    ; preds = %postload1754.i
  %masked_load498.i = load i32 addrspace(1)* %59, align 4
  br label %postload1760.i

postload1760.i:                                   ; preds = %preload1759.i, %postload1754.i
  %phi1761.i = phi i32 [ undef, %postload1754.i ], [ %masked_load498.i, %preload1759.i ]
  %temp.vect.i = insertelement <16 x i32> undef, i32 %phi1671.i, i32 0
  %temp.vect151.i = insertelement <16 x i32> %temp.vect.i, i32 %phi1689.i, i32 1
  %temp.vect152.i = insertelement <16 x i32> %temp.vect151.i, i32 %phi1695.i, i32 2
  %temp.vect153.i = insertelement <16 x i32> %temp.vect152.i, i32 %phi1701.i, i32 3
  %temp.vect154.i = insertelement <16 x i32> %temp.vect153.i, i32 %phi1620.i, i32 4
  %temp.vect155.i = insertelement <16 x i32> %temp.vect154.i, i32 %phi1626.i, i32 5
  %temp.vect156.i = insertelement <16 x i32> %temp.vect155.i, i32 %phi1632.i, i32 6
  %temp.vect157.i = insertelement <16 x i32> %temp.vect156.i, i32 %phi1638.i, i32 7
  %temp.vect158.i = insertelement <16 x i32> %temp.vect157.i, i32 %phi1644.i, i32 8
  %temp.vect159.i = insertelement <16 x i32> %temp.vect158.i, i32 %phi1650.i, i32 9
  %temp.vect160.i = insertelement <16 x i32> %temp.vect159.i, i32 %phi1656.i, i32 10
  %temp.vect161.i = insertelement <16 x i32> %temp.vect160.i, i32 %phi1662.i, i32 11
  %temp.vect162.i = insertelement <16 x i32> %temp.vect161.i, i32 %phi1743.i, i32 12
  %temp.vect163.i = insertelement <16 x i32> %temp.vect162.i, i32 %phi1749.i, i32 13
  %temp.vect164.i = insertelement <16 x i32> %temp.vect163.i, i32 %phi1755.i, i32 14
  %temp.vect165.i = insertelement <16 x i32> %temp.vect164.i, i32 %phi1761.i, i32 15
  %add11133.i = add nsw <16 x i32> %conv780.i, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %idxprom12134.i = sext <16 x i32> %add11133.i to <16 x i64>
  %extract136.i = extractelement <16 x i64> %idxprom12134.i, i32 1
  %extract137.i = extractelement <16 x i64> %idxprom12134.i, i32 2
  %extract138.i = extractelement <16 x i64> %idxprom12134.i, i32 3
  %extract139.i = extractelement <16 x i64> %idxprom12134.i, i32 4
  %extract140.i = extractelement <16 x i64> %idxprom12134.i, i32 5
  %extract141.i = extractelement <16 x i64> %idxprom12134.i, i32 6
  %extract142.i = extractelement <16 x i64> %idxprom12134.i, i32 7
  %extract143.i = extractelement <16 x i64> %idxprom12134.i, i32 8
  %extract144.i = extractelement <16 x i64> %idxprom12134.i, i32 9
  %extract145.i = extractelement <16 x i64> %idxprom12134.i, i32 10
  %extract146.i = extractelement <16 x i64> %idxprom12134.i, i32 11
  %extract147.i = extractelement <16 x i64> %idxprom12134.i, i32 12
  %extract148.i = extractelement <16 x i64> %idxprom12134.i, i32 13
  %extract149.i = extractelement <16 x i64> %idxprom12134.i, i32 14
  %extract150.i = extractelement <16 x i64> %idxprom12134.i, i32 15
  %61 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract136.i
  %62 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract137.i
  %63 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract138.i
  %64 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract139.i
  %65 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract140.i
  %66 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract141.i
  %67 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract142.i
  %68 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract143.i
  %69 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract144.i
  %70 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract145.i
  %71 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract146.i
  %72 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract147.i
  %73 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract148.i
  %74 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract149.i
  %75 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract150.i
  br i1 %extract117.i, label %preload1672.i, label %postload1673.i

preload1672.i:                                    ; preds = %postload1760.i
  %extract135.i = extractelement <16 x i64> %idxprom12134.i, i32 0
  %76 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract135.i
  %masked_load499.i = load i32 addrspace(1)* %76, align 4
  br label %postload1673.i

postload1673.i:                                   ; preds = %preload1672.i, %postload1760.i
  %phi1674.i = phi i32 [ undef, %postload1760.i ], [ %masked_load499.i, %preload1672.i ]
  br i1 %extract118.i, label %preload1690.i, label %postload1691.i

preload1690.i:                                    ; preds = %postload1673.i
  %masked_load500.i = load i32 addrspace(1)* %61, align 4
  br label %postload1691.i

postload1691.i:                                   ; preds = %preload1690.i, %postload1673.i
  %phi1692.i = phi i32 [ undef, %postload1673.i ], [ %masked_load500.i, %preload1690.i ]
  br i1 %extract119.i, label %preload1696.i, label %postload1697.i

preload1696.i:                                    ; preds = %postload1691.i
  %masked_load501.i = load i32 addrspace(1)* %62, align 4
  br label %postload1697.i

postload1697.i:                                   ; preds = %preload1696.i, %postload1691.i
  %phi1698.i = phi i32 [ undef, %postload1691.i ], [ %masked_load501.i, %preload1696.i ]
  br i1 %extract120.i, label %preload1702.i, label %postload1703.i

preload1702.i:                                    ; preds = %postload1697.i
  %masked_load502.i = load i32 addrspace(1)* %63, align 4
  br label %postload1703.i

postload1703.i:                                   ; preds = %preload1702.i, %postload1697.i
  %phi1704.i = phi i32 [ undef, %postload1697.i ], [ %masked_load502.i, %preload1702.i ]
  br i1 %extract121.i, label %preload1621.i, label %postload1622.i

preload1621.i:                                    ; preds = %postload1703.i
  %masked_load503.i = load i32 addrspace(1)* %64, align 4
  br label %postload1622.i

postload1622.i:                                   ; preds = %preload1621.i, %postload1703.i
  %phi1623.i = phi i32 [ undef, %postload1703.i ], [ %masked_load503.i, %preload1621.i ]
  br i1 %extract122.i, label %preload1627.i, label %postload1628.i

preload1627.i:                                    ; preds = %postload1622.i
  %masked_load504.i = load i32 addrspace(1)* %65, align 4
  br label %postload1628.i

postload1628.i:                                   ; preds = %preload1627.i, %postload1622.i
  %phi1629.i = phi i32 [ undef, %postload1622.i ], [ %masked_load504.i, %preload1627.i ]
  br i1 %extract123.i, label %preload1633.i, label %postload1634.i

preload1633.i:                                    ; preds = %postload1628.i
  %masked_load505.i = load i32 addrspace(1)* %66, align 4
  br label %postload1634.i

postload1634.i:                                   ; preds = %preload1633.i, %postload1628.i
  %phi1635.i = phi i32 [ undef, %postload1628.i ], [ %masked_load505.i, %preload1633.i ]
  br i1 %extract124.i, label %preload1639.i, label %postload1640.i

preload1639.i:                                    ; preds = %postload1634.i
  %masked_load506.i = load i32 addrspace(1)* %67, align 4
  br label %postload1640.i

postload1640.i:                                   ; preds = %preload1639.i, %postload1634.i
  %phi1641.i = phi i32 [ undef, %postload1634.i ], [ %masked_load506.i, %preload1639.i ]
  br i1 %extract125.i, label %preload1645.i, label %postload1646.i

preload1645.i:                                    ; preds = %postload1640.i
  %masked_load507.i = load i32 addrspace(1)* %68, align 4
  br label %postload1646.i

postload1646.i:                                   ; preds = %preload1645.i, %postload1640.i
  %phi1647.i = phi i32 [ undef, %postload1640.i ], [ %masked_load507.i, %preload1645.i ]
  br i1 %extract126.i, label %preload1651.i, label %postload1652.i

preload1651.i:                                    ; preds = %postload1646.i
  %masked_load508.i = load i32 addrspace(1)* %69, align 4
  br label %postload1652.i

postload1652.i:                                   ; preds = %preload1651.i, %postload1646.i
  %phi1653.i = phi i32 [ undef, %postload1646.i ], [ %masked_load508.i, %preload1651.i ]
  br i1 %extract127.i, label %preload1657.i, label %postload1658.i

preload1657.i:                                    ; preds = %postload1652.i
  %masked_load509.i = load i32 addrspace(1)* %70, align 4
  br label %postload1658.i

postload1658.i:                                   ; preds = %preload1657.i, %postload1652.i
  %phi1659.i = phi i32 [ undef, %postload1652.i ], [ %masked_load509.i, %preload1657.i ]
  br i1 %extract128.i, label %preload1663.i, label %postload1664.i

preload1663.i:                                    ; preds = %postload1658.i
  %masked_load510.i = load i32 addrspace(1)* %71, align 4
  br label %postload1664.i

postload1664.i:                                   ; preds = %preload1663.i, %postload1658.i
  %phi1665.i = phi i32 [ undef, %postload1658.i ], [ %masked_load510.i, %preload1663.i ]
  br i1 %extract129.i, label %preload1744.i, label %postload1745.i

preload1744.i:                                    ; preds = %postload1664.i
  %masked_load511.i = load i32 addrspace(1)* %72, align 4
  br label %postload1745.i

postload1745.i:                                   ; preds = %preload1744.i, %postload1664.i
  %phi1746.i = phi i32 [ undef, %postload1664.i ], [ %masked_load511.i, %preload1744.i ]
  br i1 %extract130.i, label %preload1750.i, label %postload1751.i

preload1750.i:                                    ; preds = %postload1745.i
  %masked_load512.i = load i32 addrspace(1)* %73, align 4
  br label %postload1751.i

postload1751.i:                                   ; preds = %preload1750.i, %postload1745.i
  %phi1752.i = phi i32 [ undef, %postload1745.i ], [ %masked_load512.i, %preload1750.i ]
  br i1 %extract131.i, label %preload1756.i, label %postload1757.i

preload1756.i:                                    ; preds = %postload1751.i
  %masked_load513.i = load i32 addrspace(1)* %74, align 4
  br label %postload1757.i

postload1757.i:                                   ; preds = %preload1756.i, %postload1751.i
  %phi1758.i = phi i32 [ undef, %postload1751.i ], [ %masked_load513.i, %preload1756.i ]
  br i1 %extract132.i, label %preload1762.i, label %postload1763.i

preload1762.i:                                    ; preds = %postload1757.i
  %masked_load514.i = load i32 addrspace(1)* %75, align 4
  br label %postload1763.i

postload1763.i:                                   ; preds = %preload1762.i, %postload1757.i
  %phi1764.i = phi i32 [ undef, %postload1757.i ], [ %masked_load514.i, %preload1762.i ]
  %temp.vect167.i = insertelement <16 x i32> undef, i32 %phi1674.i, i32 0
  %temp.vect168.i = insertelement <16 x i32> %temp.vect167.i, i32 %phi1692.i, i32 1
  %temp.vect169.i = insertelement <16 x i32> %temp.vect168.i, i32 %phi1698.i, i32 2
  %temp.vect170.i = insertelement <16 x i32> %temp.vect169.i, i32 %phi1704.i, i32 3
  %temp.vect171.i = insertelement <16 x i32> %temp.vect170.i, i32 %phi1623.i, i32 4
  %temp.vect172.i = insertelement <16 x i32> %temp.vect171.i, i32 %phi1629.i, i32 5
  %temp.vect173.i = insertelement <16 x i32> %temp.vect172.i, i32 %phi1635.i, i32 6
  %temp.vect174.i = insertelement <16 x i32> %temp.vect173.i, i32 %phi1641.i, i32 7
  %temp.vect175.i = insertelement <16 x i32> %temp.vect174.i, i32 %phi1647.i, i32 8
  %temp.vect176.i = insertelement <16 x i32> %temp.vect175.i, i32 %phi1653.i, i32 9
  %temp.vect177.i = insertelement <16 x i32> %temp.vect176.i, i32 %phi1659.i, i32 10
  %temp.vect178.i = insertelement <16 x i32> %temp.vect177.i, i32 %phi1665.i, i32 11
  %temp.vect179.i = insertelement <16 x i32> %temp.vect178.i, i32 %phi1746.i, i32 12
  %temp.vect180.i = insertelement <16 x i32> %temp.vect179.i, i32 %phi1752.i, i32 13
  %temp.vect181.i = insertelement <16 x i32> %temp.vect180.i, i32 %phi1758.i, i32 14
  %temp.vect182.i = insertelement <16 x i32> %temp.vect181.i, i32 %phi1764.i, i32 15
  %loadedValue2950.i = load <16 x i32>* %CastToValueType.i, align 64
  %add14166.i = add nsw <16 x i32> %temp.vect165.i, %loadedValue2950.i
  %cmp152.i = icmp slt <16 x i32> %add14166.i, %temp.vect182.i
  %Mneg2183.i = xor <16 x i1> %cmp152.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %if.then_to_for.end184.i = and <16 x i1> %cmp.i, %Mneg2183.i
  %if.then_to_for.body.lr.ph185.i = and <16 x i1> %cmp.i, %cmp152.i
  %ipred.i.i = bitcast <16 x i1> %if.then_to_for.body.lr.ph185.i to i16
  %val.i.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i.i, i16 %ipred.i.i) nounwind
  %77 = and i32 %val.i.i, 1
  %res.i.i = icmp eq i32 %77, 0
  br i1 %res.i.i, label %for.body.preheader.i, label %for.end.i

for.body.preheader.i:                             ; preds = %postload1763.i
  %78 = sext <16 x i32> %add14166.i to <16 x i64>
  %negIncomingLoopMask186.i = xor <16 x i1> %if.then_to_for.body.lr.ph185.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  br label %for.body.i

for.body.i:                                       ; preds = %postload1916.i, %for.body.preheader.i
  %vectorPHI.i = phi <16 x i1> [ %loop_mask3296.i, %postload1916.i ], [ %negIncomingLoopMask186.i, %for.body.preheader.i ]
  %vectorPHI188.i = phi <16 x float> [ %out_sel291.i, %postload1916.i ], [ undef, %for.body.preheader.i ]
  %vectorPHI189.i = phi <16 x i1> [ %local_edge315.i, %postload1916.i ], [ %if.then_to_for.body.lr.ph185.i, %for.body.preheader.i ]
  %vectorPHI190.i = phi <16 x i64> [ %indvars.iv.next292.i, %postload1916.i ], [ %78, %for.body.preheader.i ]
  %vectorPHI191.i = phi <16 x float> [ %add24290.i, %postload1916.i ], [ zeroinitializer, %for.body.preheader.i ]
  %extract208.i = extractelement <16 x i1> %vectorPHI189.i, i32 0
  %extract209.i = extractelement <16 x i1> %vectorPHI189.i, i32 1
  %extract210.i = extractelement <16 x i1> %vectorPHI189.i, i32 2
  %extract211.i = extractelement <16 x i1> %vectorPHI189.i, i32 3
  %extract212.i = extractelement <16 x i1> %vectorPHI189.i, i32 4
  %extract213.i = extractelement <16 x i1> %vectorPHI189.i, i32 5
  %extract214.i = extractelement <16 x i1> %vectorPHI189.i, i32 6
  %extract215.i = extractelement <16 x i1> %vectorPHI189.i, i32 7
  %extract216.i = extractelement <16 x i1> %vectorPHI189.i, i32 8
  %extract217.i = extractelement <16 x i1> %vectorPHI189.i, i32 9
  %extract218.i = extractelement <16 x i1> %vectorPHI189.i, i32 10
  %extract219.i = extractelement <16 x i1> %vectorPHI189.i, i32 11
  %extract220.i = extractelement <16 x i1> %vectorPHI189.i, i32 12
  %extract221.i = extractelement <16 x i1> %vectorPHI189.i, i32 13
  %extract222.i = extractelement <16 x i1> %vectorPHI189.i, i32 14
  %extract223.i = extractelement <16 x i1> %vectorPHI189.i, i32 15
  %extract192.i = extractelement <16 x i64> %vectorPHI190.i, i32 0
  %extract193.i = extractelement <16 x i64> %vectorPHI190.i, i32 1
  %extract194.i = extractelement <16 x i64> %vectorPHI190.i, i32 2
  %extract195.i = extractelement <16 x i64> %vectorPHI190.i, i32 3
  %extract196.i = extractelement <16 x i64> %vectorPHI190.i, i32 4
  %extract197.i = extractelement <16 x i64> %vectorPHI190.i, i32 5
  %extract198.i = extractelement <16 x i64> %vectorPHI190.i, i32 6
  %extract199.i = extractelement <16 x i64> %vectorPHI190.i, i32 7
  %extract200.i = extractelement <16 x i64> %vectorPHI190.i, i32 8
  %extract201.i = extractelement <16 x i64> %vectorPHI190.i, i32 9
  %extract202.i = extractelement <16 x i64> %vectorPHI190.i, i32 10
  %extract203.i = extractelement <16 x i64> %vectorPHI190.i, i32 11
  %extract204.i = extractelement <16 x i64> %vectorPHI190.i, i32 12
  %extract205.i = extractelement <16 x i64> %vectorPHI190.i, i32 13
  %extract206.i = extractelement <16 x i64> %vectorPHI190.i, i32 14
  %extract207.i = extractelement <16 x i64> %vectorPHI190.i, i32 15
  %79 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract193.i
  %80 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract194.i
  %81 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract195.i
  %82 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract196.i
  %83 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract197.i
  %84 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract198.i
  %85 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract199.i
  %86 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract200.i
  %87 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract201.i
  %88 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract202.i
  %89 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract203.i
  %90 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract204.i
  %91 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract205.i
  %92 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract206.i
  %93 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract207.i
  br i1 %extract208.i, label %preload1774.i, label %postload1775.i

preload1774.i:                                    ; preds = %for.body.i
  %94 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract192.i
  %masked_load515.i = load i32 addrspace(1)* %94, align 4
  br label %postload1775.i

postload1775.i:                                   ; preds = %preload1774.i, %for.body.i
  %phi1776.i = phi i32 [ undef, %for.body.i ], [ %masked_load515.i, %preload1774.i ]
  br i1 %extract209.i, label %preload1783.i, label %postload1784.i

preload1783.i:                                    ; preds = %postload1775.i
  %masked_load516.i = load i32 addrspace(1)* %79, align 4
  br label %postload1784.i

postload1784.i:                                   ; preds = %preload1783.i, %postload1775.i
  %phi1785.i = phi i32 [ undef, %postload1775.i ], [ %masked_load516.i, %preload1783.i ]
  br i1 %extract210.i, label %preload1792.i, label %postload1793.i

preload1792.i:                                    ; preds = %postload1784.i
  %masked_load517.i = load i32 addrspace(1)* %80, align 4
  br label %postload1793.i

postload1793.i:                                   ; preds = %preload1792.i, %postload1784.i
  %phi1794.i = phi i32 [ undef, %postload1784.i ], [ %masked_load517.i, %preload1792.i ]
  br i1 %extract211.i, label %preload1801.i, label %postload1802.i

preload1801.i:                                    ; preds = %postload1793.i
  %masked_load518.i = load i32 addrspace(1)* %81, align 4
  br label %postload1802.i

postload1802.i:                                   ; preds = %preload1801.i, %postload1793.i
  %phi1803.i = phi i32 [ undef, %postload1793.i ], [ %masked_load518.i, %preload1801.i ]
  br i1 %extract212.i, label %preload1810.i, label %postload1811.i

preload1810.i:                                    ; preds = %postload1802.i
  %masked_load519.i = load i32 addrspace(1)* %82, align 4
  br label %postload1811.i

postload1811.i:                                   ; preds = %preload1810.i, %postload1802.i
  %phi1812.i = phi i32 [ undef, %postload1802.i ], [ %masked_load519.i, %preload1810.i ]
  br i1 %extract213.i, label %preload1819.i, label %postload1820.i

preload1819.i:                                    ; preds = %postload1811.i
  %masked_load520.i = load i32 addrspace(1)* %83, align 4
  br label %postload1820.i

postload1820.i:                                   ; preds = %preload1819.i, %postload1811.i
  %phi1821.i = phi i32 [ undef, %postload1811.i ], [ %masked_load520.i, %preload1819.i ]
  br i1 %extract214.i, label %preload1828.i, label %postload1829.i

preload1828.i:                                    ; preds = %postload1820.i
  %masked_load521.i = load i32 addrspace(1)* %84, align 4
  br label %postload1829.i

postload1829.i:                                   ; preds = %preload1828.i, %postload1820.i
  %phi1830.i = phi i32 [ undef, %postload1820.i ], [ %masked_load521.i, %preload1828.i ]
  br i1 %extract215.i, label %preload1837.i, label %postload1838.i

preload1837.i:                                    ; preds = %postload1829.i
  %masked_load522.i = load i32 addrspace(1)* %85, align 4
  br label %postload1838.i

postload1838.i:                                   ; preds = %preload1837.i, %postload1829.i
  %phi1839.i = phi i32 [ undef, %postload1829.i ], [ %masked_load522.i, %preload1837.i ]
  br i1 %extract216.i, label %preload1846.i, label %postload1847.i

preload1846.i:                                    ; preds = %postload1838.i
  %masked_load523.i = load i32 addrspace(1)* %86, align 4
  br label %postload1847.i

postload1847.i:                                   ; preds = %preload1846.i, %postload1838.i
  %phi1848.i = phi i32 [ undef, %postload1838.i ], [ %masked_load523.i, %preload1846.i ]
  br i1 %extract217.i, label %preload1855.i, label %postload1856.i

preload1855.i:                                    ; preds = %postload1847.i
  %masked_load524.i = load i32 addrspace(1)* %87, align 4
  br label %postload1856.i

postload1856.i:                                   ; preds = %preload1855.i, %postload1847.i
  %phi1857.i = phi i32 [ undef, %postload1847.i ], [ %masked_load524.i, %preload1855.i ]
  br i1 %extract218.i, label %preload1864.i, label %postload1865.i

preload1864.i:                                    ; preds = %postload1856.i
  %masked_load525.i = load i32 addrspace(1)* %88, align 4
  br label %postload1865.i

postload1865.i:                                   ; preds = %preload1864.i, %postload1856.i
  %phi1866.i = phi i32 [ undef, %postload1856.i ], [ %masked_load525.i, %preload1864.i ]
  br i1 %extract219.i, label %preload1873.i, label %postload1874.i

preload1873.i:                                    ; preds = %postload1865.i
  %masked_load526.i = load i32 addrspace(1)* %89, align 4
  br label %postload1874.i

postload1874.i:                                   ; preds = %preload1873.i, %postload1865.i
  %phi1875.i = phi i32 [ undef, %postload1865.i ], [ %masked_load526.i, %preload1873.i ]
  br i1 %extract220.i, label %preload1882.i, label %postload1883.i

preload1882.i:                                    ; preds = %postload1874.i
  %masked_load527.i = load i32 addrspace(1)* %90, align 4
  br label %postload1883.i

postload1883.i:                                   ; preds = %preload1882.i, %postload1874.i
  %phi1884.i = phi i32 [ undef, %postload1874.i ], [ %masked_load527.i, %preload1882.i ]
  br i1 %extract221.i, label %preload1891.i, label %postload1892.i

preload1891.i:                                    ; preds = %postload1883.i
  %masked_load528.i = load i32 addrspace(1)* %91, align 4
  br label %postload1892.i

postload1892.i:                                   ; preds = %preload1891.i, %postload1883.i
  %phi1893.i = phi i32 [ undef, %postload1883.i ], [ %masked_load528.i, %preload1891.i ]
  br i1 %extract222.i, label %preload1900.i, label %postload1901.i

preload1900.i:                                    ; preds = %postload1892.i
  %masked_load529.i = load i32 addrspace(1)* %92, align 4
  br label %postload1901.i

postload1901.i:                                   ; preds = %preload1900.i, %postload1892.i
  %phi1902.i = phi i32 [ undef, %postload1892.i ], [ %masked_load529.i, %preload1900.i ]
  br i1 %extract223.i, label %preload1909.i, label %postload1910.i

preload1909.i:                                    ; preds = %postload1901.i
  %masked_load530.i = load i32 addrspace(1)* %93, align 4
  br label %postload1910.i

postload1910.i:                                   ; preds = %preload1909.i, %postload1901.i
  %phi1911.i = phi i32 [ undef, %postload1901.i ], [ %masked_load530.i, %preload1909.i ]
  %temp.vect224.i = insertelement <16 x i32> undef, i32 %phi1776.i, i32 0
  %temp.vect225.i = insertelement <16 x i32> %temp.vect224.i, i32 %phi1785.i, i32 1
  %temp.vect226.i = insertelement <16 x i32> %temp.vect225.i, i32 %phi1794.i, i32 2
  %temp.vect227.i = insertelement <16 x i32> %temp.vect226.i, i32 %phi1803.i, i32 3
  %temp.vect228.i = insertelement <16 x i32> %temp.vect227.i, i32 %phi1812.i, i32 4
  %temp.vect229.i = insertelement <16 x i32> %temp.vect228.i, i32 %phi1821.i, i32 5
  %temp.vect230.i = insertelement <16 x i32> %temp.vect229.i, i32 %phi1830.i, i32 6
  %temp.vect231.i = insertelement <16 x i32> %temp.vect230.i, i32 %phi1839.i, i32 7
  %temp.vect232.i = insertelement <16 x i32> %temp.vect231.i, i32 %phi1848.i, i32 8
  %temp.vect233.i = insertelement <16 x i32> %temp.vect232.i, i32 %phi1857.i, i32 9
  %temp.vect234.i = insertelement <16 x i32> %temp.vect233.i, i32 %phi1866.i, i32 10
  %temp.vect235.i = insertelement <16 x i32> %temp.vect234.i, i32 %phi1875.i, i32 11
  %temp.vect236.i = insertelement <16 x i32> %temp.vect235.i, i32 %phi1884.i, i32 12
  %temp.vect237.i = insertelement <16 x i32> %temp.vect236.i, i32 %phi1893.i, i32 13
  %temp.vect238.i = insertelement <16 x i32> %temp.vect237.i, i32 %phi1902.i, i32 14
  %temp.vect239.i = insertelement <16 x i32> %temp.vect238.i, i32 %phi1911.i, i32 15
  %95 = getelementptr inbounds float addrspace(1)* %1, i64 %extract193.i
  %96 = getelementptr inbounds float addrspace(1)* %1, i64 %extract194.i
  %97 = getelementptr inbounds float addrspace(1)* %1, i64 %extract195.i
  %98 = getelementptr inbounds float addrspace(1)* %1, i64 %extract196.i
  %99 = getelementptr inbounds float addrspace(1)* %1, i64 %extract197.i
  %100 = getelementptr inbounds float addrspace(1)* %1, i64 %extract198.i
  %101 = getelementptr inbounds float addrspace(1)* %1, i64 %extract199.i
  %102 = getelementptr inbounds float addrspace(1)* %1, i64 %extract200.i
  %103 = getelementptr inbounds float addrspace(1)* %1, i64 %extract201.i
  %104 = getelementptr inbounds float addrspace(1)* %1, i64 %extract202.i
  %105 = getelementptr inbounds float addrspace(1)* %1, i64 %extract203.i
  %106 = getelementptr inbounds float addrspace(1)* %1, i64 %extract204.i
  %107 = getelementptr inbounds float addrspace(1)* %1, i64 %extract205.i
  %108 = getelementptr inbounds float addrspace(1)* %1, i64 %extract206.i
  %109 = getelementptr inbounds float addrspace(1)* %1, i64 %extract207.i
  br i1 %extract208.i, label %preload1777.i, label %postload1778.i

preload1777.i:                                    ; preds = %postload1910.i
  %110 = getelementptr inbounds float addrspace(1)* %1, i64 %extract192.i
  %masked_load531.i = load float addrspace(1)* %110, align 4
  br label %postload1778.i

postload1778.i:                                   ; preds = %preload1777.i, %postload1910.i
  %phi1779.i = phi float [ undef, %postload1910.i ], [ %masked_load531.i, %preload1777.i ]
  br i1 %extract209.i, label %preload1786.i, label %postload1787.i

preload1786.i:                                    ; preds = %postload1778.i
  %masked_load532.i = load float addrspace(1)* %95, align 4
  br label %postload1787.i

postload1787.i:                                   ; preds = %preload1786.i, %postload1778.i
  %phi1788.i = phi float [ undef, %postload1778.i ], [ %masked_load532.i, %preload1786.i ]
  br i1 %extract210.i, label %preload1795.i, label %postload1796.i

preload1795.i:                                    ; preds = %postload1787.i
  %masked_load533.i = load float addrspace(1)* %96, align 4
  br label %postload1796.i

postload1796.i:                                   ; preds = %preload1795.i, %postload1787.i
  %phi1797.i = phi float [ undef, %postload1787.i ], [ %masked_load533.i, %preload1795.i ]
  br i1 %extract211.i, label %preload1804.i, label %postload1805.i

preload1804.i:                                    ; preds = %postload1796.i
  %masked_load534.i = load float addrspace(1)* %97, align 4
  br label %postload1805.i

postload1805.i:                                   ; preds = %preload1804.i, %postload1796.i
  %phi1806.i = phi float [ undef, %postload1796.i ], [ %masked_load534.i, %preload1804.i ]
  br i1 %extract212.i, label %preload1813.i, label %postload1814.i

preload1813.i:                                    ; preds = %postload1805.i
  %masked_load535.i = load float addrspace(1)* %98, align 4
  br label %postload1814.i

postload1814.i:                                   ; preds = %preload1813.i, %postload1805.i
  %phi1815.i = phi float [ undef, %postload1805.i ], [ %masked_load535.i, %preload1813.i ]
  br i1 %extract213.i, label %preload1822.i, label %postload1823.i

preload1822.i:                                    ; preds = %postload1814.i
  %masked_load536.i = load float addrspace(1)* %99, align 4
  br label %postload1823.i

postload1823.i:                                   ; preds = %preload1822.i, %postload1814.i
  %phi1824.i = phi float [ undef, %postload1814.i ], [ %masked_load536.i, %preload1822.i ]
  br i1 %extract214.i, label %preload1831.i, label %postload1832.i

preload1831.i:                                    ; preds = %postload1823.i
  %masked_load537.i = load float addrspace(1)* %100, align 4
  br label %postload1832.i

postload1832.i:                                   ; preds = %preload1831.i, %postload1823.i
  %phi1833.i = phi float [ undef, %postload1823.i ], [ %masked_load537.i, %preload1831.i ]
  br i1 %extract215.i, label %preload1840.i, label %postload1841.i

preload1840.i:                                    ; preds = %postload1832.i
  %masked_load538.i = load float addrspace(1)* %101, align 4
  br label %postload1841.i

postload1841.i:                                   ; preds = %preload1840.i, %postload1832.i
  %phi1842.i = phi float [ undef, %postload1832.i ], [ %masked_load538.i, %preload1840.i ]
  br i1 %extract216.i, label %preload1849.i, label %postload1850.i

preload1849.i:                                    ; preds = %postload1841.i
  %masked_load539.i = load float addrspace(1)* %102, align 4
  br label %postload1850.i

postload1850.i:                                   ; preds = %preload1849.i, %postload1841.i
  %phi1851.i = phi float [ undef, %postload1841.i ], [ %masked_load539.i, %preload1849.i ]
  br i1 %extract217.i, label %preload1858.i, label %postload1859.i

preload1858.i:                                    ; preds = %postload1850.i
  %masked_load540.i = load float addrspace(1)* %103, align 4
  br label %postload1859.i

postload1859.i:                                   ; preds = %preload1858.i, %postload1850.i
  %phi1860.i = phi float [ undef, %postload1850.i ], [ %masked_load540.i, %preload1858.i ]
  br i1 %extract218.i, label %preload1867.i, label %postload1868.i

preload1867.i:                                    ; preds = %postload1859.i
  %masked_load541.i = load float addrspace(1)* %104, align 4
  br label %postload1868.i

postload1868.i:                                   ; preds = %preload1867.i, %postload1859.i
  %phi1869.i = phi float [ undef, %postload1859.i ], [ %masked_load541.i, %preload1867.i ]
  br i1 %extract219.i, label %preload1876.i, label %postload1877.i

preload1876.i:                                    ; preds = %postload1868.i
  %masked_load542.i = load float addrspace(1)* %105, align 4
  br label %postload1877.i

postload1877.i:                                   ; preds = %preload1876.i, %postload1868.i
  %phi1878.i = phi float [ undef, %postload1868.i ], [ %masked_load542.i, %preload1876.i ]
  br i1 %extract220.i, label %preload1885.i, label %postload1886.i

preload1885.i:                                    ; preds = %postload1877.i
  %masked_load543.i = load float addrspace(1)* %106, align 4
  br label %postload1886.i

postload1886.i:                                   ; preds = %preload1885.i, %postload1877.i
  %phi1887.i = phi float [ undef, %postload1877.i ], [ %masked_load543.i, %preload1885.i ]
  br i1 %extract221.i, label %preload1894.i, label %postload1895.i

preload1894.i:                                    ; preds = %postload1886.i
  %masked_load544.i = load float addrspace(1)* %107, align 4
  br label %postload1895.i

postload1895.i:                                   ; preds = %preload1894.i, %postload1886.i
  %phi1896.i = phi float [ undef, %postload1886.i ], [ %masked_load544.i, %preload1894.i ]
  br i1 %extract222.i, label %preload1903.i, label %postload1904.i

preload1903.i:                                    ; preds = %postload1895.i
  %masked_load545.i = load float addrspace(1)* %108, align 4
  br label %postload1904.i

postload1904.i:                                   ; preds = %preload1903.i, %postload1895.i
  %phi1905.i = phi float [ undef, %postload1895.i ], [ %masked_load545.i, %preload1903.i ]
  br i1 %extract223.i, label %preload1912.i, label %postload1913.i

preload1912.i:                                    ; preds = %postload1904.i
  %masked_load546.i = load float addrspace(1)* %109, align 4
  br label %postload1913.i

postload1913.i:                                   ; preds = %preload1912.i, %postload1904.i
  %phi1914.i = phi float [ undef, %postload1904.i ], [ %masked_load546.i, %preload1912.i ]
  %temp.vect257.i = insertelement <16 x float> undef, float %phi1779.i, i32 0
  %temp.vect258.i = insertelement <16 x float> %temp.vect257.i, float %phi1788.i, i32 1
  %temp.vect259.i = insertelement <16 x float> %temp.vect258.i, float %phi1797.i, i32 2
  %temp.vect260.i = insertelement <16 x float> %temp.vect259.i, float %phi1806.i, i32 3
  %temp.vect261.i = insertelement <16 x float> %temp.vect260.i, float %phi1815.i, i32 4
  %temp.vect262.i = insertelement <16 x float> %temp.vect261.i, float %phi1824.i, i32 5
  %temp.vect263.i = insertelement <16 x float> %temp.vect262.i, float %phi1833.i, i32 6
  %temp.vect264.i = insertelement <16 x float> %temp.vect263.i, float %phi1842.i, i32 7
  %temp.vect265.i = insertelement <16 x float> %temp.vect264.i, float %phi1851.i, i32 8
  %temp.vect266.i = insertelement <16 x float> %temp.vect265.i, float %phi1860.i, i32 9
  %temp.vect267.i = insertelement <16 x float> %temp.vect266.i, float %phi1869.i, i32 10
  %temp.vect268.i = insertelement <16 x float> %temp.vect267.i, float %phi1878.i, i32 11
  %temp.vect269.i = insertelement <16 x float> %temp.vect268.i, float %phi1887.i, i32 12
  %temp.vect270.i = insertelement <16 x float> %temp.vect269.i, float %phi1896.i, i32 13
  %temp.vect271.i = insertelement <16 x float> %temp.vect270.i, float %phi1905.i, i32 14
  %temp.vect272.i = insertelement <16 x float> %temp.vect271.i, float %phi1914.i, i32 15
  %idxprom21240.i = sext <16 x i32> %temp.vect239.i to <16 x i64>
  %extract242.i = extractelement <16 x i64> %idxprom21240.i, i32 1
  %extract243.i = extractelement <16 x i64> %idxprom21240.i, i32 2
  %extract244.i = extractelement <16 x i64> %idxprom21240.i, i32 3
  %extract245.i = extractelement <16 x i64> %idxprom21240.i, i32 4
  %extract246.i = extractelement <16 x i64> %idxprom21240.i, i32 5
  %extract247.i = extractelement <16 x i64> %idxprom21240.i, i32 6
  %extract248.i = extractelement <16 x i64> %idxprom21240.i, i32 7
  %extract249.i = extractelement <16 x i64> %idxprom21240.i, i32 8
  %extract250.i = extractelement <16 x i64> %idxprom21240.i, i32 9
  %extract251.i = extractelement <16 x i64> %idxprom21240.i, i32 10
  %extract252.i = extractelement <16 x i64> %idxprom21240.i, i32 11
  %extract253.i = extractelement <16 x i64> %idxprom21240.i, i32 12
  %extract254.i = extractelement <16 x i64> %idxprom21240.i, i32 13
  %extract255.i = extractelement <16 x i64> %idxprom21240.i, i32 14
  %extract256.i = extractelement <16 x i64> %idxprom21240.i, i32 15
  %111 = getelementptr inbounds float addrspace(1)* %4, i64 %extract242.i
  %112 = getelementptr inbounds float addrspace(1)* %4, i64 %extract243.i
  %113 = getelementptr inbounds float addrspace(1)* %4, i64 %extract244.i
  %114 = getelementptr inbounds float addrspace(1)* %4, i64 %extract245.i
  %115 = getelementptr inbounds float addrspace(1)* %4, i64 %extract246.i
  %116 = getelementptr inbounds float addrspace(1)* %4, i64 %extract247.i
  %117 = getelementptr inbounds float addrspace(1)* %4, i64 %extract248.i
  %118 = getelementptr inbounds float addrspace(1)* %4, i64 %extract249.i
  %119 = getelementptr inbounds float addrspace(1)* %4, i64 %extract250.i
  %120 = getelementptr inbounds float addrspace(1)* %4, i64 %extract251.i
  %121 = getelementptr inbounds float addrspace(1)* %4, i64 %extract252.i
  %122 = getelementptr inbounds float addrspace(1)* %4, i64 %extract253.i
  %123 = getelementptr inbounds float addrspace(1)* %4, i64 %extract254.i
  %124 = getelementptr inbounds float addrspace(1)* %4, i64 %extract255.i
  %125 = getelementptr inbounds float addrspace(1)* %4, i64 %extract256.i
  br i1 %extract208.i, label %preload1780.i, label %postload1781.i

preload1780.i:                                    ; preds = %postload1913.i
  %extract241.i = extractelement <16 x i64> %idxprom21240.i, i32 0
  %126 = getelementptr inbounds float addrspace(1)* %4, i64 %extract241.i
  %masked_load547.i = load float addrspace(1)* %126, align 4
  br label %postload1781.i

postload1781.i:                                   ; preds = %preload1780.i, %postload1913.i
  %phi1782.i = phi float [ undef, %postload1913.i ], [ %masked_load547.i, %preload1780.i ]
  br i1 %extract209.i, label %preload1789.i, label %postload1790.i

preload1789.i:                                    ; preds = %postload1781.i
  %masked_load548.i = load float addrspace(1)* %111, align 4
  br label %postload1790.i

postload1790.i:                                   ; preds = %preload1789.i, %postload1781.i
  %phi1791.i = phi float [ undef, %postload1781.i ], [ %masked_load548.i, %preload1789.i ]
  br i1 %extract210.i, label %preload1798.i, label %postload1799.i

preload1798.i:                                    ; preds = %postload1790.i
  %masked_load549.i = load float addrspace(1)* %112, align 4
  br label %postload1799.i

postload1799.i:                                   ; preds = %preload1798.i, %postload1790.i
  %phi1800.i = phi float [ undef, %postload1790.i ], [ %masked_load549.i, %preload1798.i ]
  br i1 %extract211.i, label %preload1807.i, label %postload1808.i

preload1807.i:                                    ; preds = %postload1799.i
  %masked_load550.i = load float addrspace(1)* %113, align 4
  br label %postload1808.i

postload1808.i:                                   ; preds = %preload1807.i, %postload1799.i
  %phi1809.i = phi float [ undef, %postload1799.i ], [ %masked_load550.i, %preload1807.i ]
  br i1 %extract212.i, label %preload1816.i, label %postload1817.i

preload1816.i:                                    ; preds = %postload1808.i
  %masked_load551.i = load float addrspace(1)* %114, align 4
  br label %postload1817.i

postload1817.i:                                   ; preds = %preload1816.i, %postload1808.i
  %phi1818.i = phi float [ undef, %postload1808.i ], [ %masked_load551.i, %preload1816.i ]
  br i1 %extract213.i, label %preload1825.i, label %postload1826.i

preload1825.i:                                    ; preds = %postload1817.i
  %masked_load552.i = load float addrspace(1)* %115, align 4
  br label %postload1826.i

postload1826.i:                                   ; preds = %preload1825.i, %postload1817.i
  %phi1827.i = phi float [ undef, %postload1817.i ], [ %masked_load552.i, %preload1825.i ]
  br i1 %extract214.i, label %preload1834.i, label %postload1835.i

preload1834.i:                                    ; preds = %postload1826.i
  %masked_load553.i = load float addrspace(1)* %116, align 4
  br label %postload1835.i

postload1835.i:                                   ; preds = %preload1834.i, %postload1826.i
  %phi1836.i = phi float [ undef, %postload1826.i ], [ %masked_load553.i, %preload1834.i ]
  br i1 %extract215.i, label %preload1843.i, label %postload1844.i

preload1843.i:                                    ; preds = %postload1835.i
  %masked_load554.i = load float addrspace(1)* %117, align 4
  br label %postload1844.i

postload1844.i:                                   ; preds = %preload1843.i, %postload1835.i
  %phi1845.i = phi float [ undef, %postload1835.i ], [ %masked_load554.i, %preload1843.i ]
  br i1 %extract216.i, label %preload1852.i, label %postload1853.i

preload1852.i:                                    ; preds = %postload1844.i
  %masked_load555.i = load float addrspace(1)* %118, align 4
  br label %postload1853.i

postload1853.i:                                   ; preds = %preload1852.i, %postload1844.i
  %phi1854.i = phi float [ undef, %postload1844.i ], [ %masked_load555.i, %preload1852.i ]
  br i1 %extract217.i, label %preload1861.i, label %postload1862.i

preload1861.i:                                    ; preds = %postload1853.i
  %masked_load556.i = load float addrspace(1)* %119, align 4
  br label %postload1862.i

postload1862.i:                                   ; preds = %preload1861.i, %postload1853.i
  %phi1863.i = phi float [ undef, %postload1853.i ], [ %masked_load556.i, %preload1861.i ]
  br i1 %extract218.i, label %preload1870.i, label %postload1871.i

preload1870.i:                                    ; preds = %postload1862.i
  %masked_load557.i = load float addrspace(1)* %120, align 4
  br label %postload1871.i

postload1871.i:                                   ; preds = %preload1870.i, %postload1862.i
  %phi1872.i = phi float [ undef, %postload1862.i ], [ %masked_load557.i, %preload1870.i ]
  br i1 %extract219.i, label %preload1879.i, label %postload1880.i

preload1879.i:                                    ; preds = %postload1871.i
  %masked_load558.i = load float addrspace(1)* %121, align 4
  br label %postload1880.i

postload1880.i:                                   ; preds = %preload1879.i, %postload1871.i
  %phi1881.i = phi float [ undef, %postload1871.i ], [ %masked_load558.i, %preload1879.i ]
  br i1 %extract220.i, label %preload1888.i, label %postload1889.i

preload1888.i:                                    ; preds = %postload1880.i
  %masked_load559.i = load float addrspace(1)* %122, align 4
  br label %postload1889.i

postload1889.i:                                   ; preds = %preload1888.i, %postload1880.i
  %phi1890.i = phi float [ undef, %postload1880.i ], [ %masked_load559.i, %preload1888.i ]
  br i1 %extract221.i, label %preload1897.i, label %postload1898.i

preload1897.i:                                    ; preds = %postload1889.i
  %masked_load560.i = load float addrspace(1)* %123, align 4
  br label %postload1898.i

postload1898.i:                                   ; preds = %preload1897.i, %postload1889.i
  %phi1899.i = phi float [ undef, %postload1889.i ], [ %masked_load560.i, %preload1897.i ]
  br i1 %extract222.i, label %preload1906.i, label %postload1907.i

preload1906.i:                                    ; preds = %postload1898.i
  %masked_load561.i = load float addrspace(1)* %124, align 4
  br label %postload1907.i

postload1907.i:                                   ; preds = %preload1906.i, %postload1898.i
  %phi1908.i = phi float [ undef, %postload1898.i ], [ %masked_load561.i, %preload1906.i ]
  br i1 %extract223.i, label %preload1915.i, label %postload1916.i

preload1915.i:                                    ; preds = %postload1907.i
  %masked_load562.i = load float addrspace(1)* %125, align 4
  br label %postload1916.i

postload1916.i:                                   ; preds = %preload1915.i, %postload1907.i
  %phi1917.i = phi float [ undef, %postload1907.i ], [ %masked_load562.i, %preload1915.i ]
  %temp.vect273.i = insertelement <16 x float> undef, float %phi1782.i, i32 0
  %temp.vect274.i = insertelement <16 x float> %temp.vect273.i, float %phi1791.i, i32 1
  %temp.vect275.i = insertelement <16 x float> %temp.vect274.i, float %phi1800.i, i32 2
  %temp.vect276.i = insertelement <16 x float> %temp.vect275.i, float %phi1809.i, i32 3
  %temp.vect277.i = insertelement <16 x float> %temp.vect276.i, float %phi1818.i, i32 4
  %temp.vect278.i = insertelement <16 x float> %temp.vect277.i, float %phi1827.i, i32 5
  %temp.vect279.i = insertelement <16 x float> %temp.vect278.i, float %phi1836.i, i32 6
  %temp.vect280.i = insertelement <16 x float> %temp.vect279.i, float %phi1845.i, i32 7
  %temp.vect281.i = insertelement <16 x float> %temp.vect280.i, float %phi1854.i, i32 8
  %temp.vect282.i = insertelement <16 x float> %temp.vect281.i, float %phi1863.i, i32 9
  %temp.vect283.i = insertelement <16 x float> %temp.vect282.i, float %phi1872.i, i32 10
  %temp.vect284.i = insertelement <16 x float> %temp.vect283.i, float %phi1881.i, i32 11
  %temp.vect285.i = insertelement <16 x float> %temp.vect284.i, float %phi1890.i, i32 12
  %temp.vect286.i = insertelement <16 x float> %temp.vect285.i, float %phi1899.i, i32 13
  %temp.vect287.i = insertelement <16 x float> %temp.vect286.i, float %phi1908.i, i32 14
  %temp.vect288.i = insertelement <16 x float> %temp.vect287.i, float %phi1917.i, i32 15
  %mul23289.i = fmul <16 x float> %temp.vect272.i, %temp.vect288.i
  %add24290.i = fadd <16 x float> %vectorPHI191.i, %mul23289.i
  %out_sel291.i = select <16 x i1> %vectorPHI189.i, <16 x float> %add24290.i, <16 x float> %vectorPHI188.i
  %indvars.iv.next292.i = add <16 x i64> %vectorPHI190.i, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %127 = trunc <16 x i64> %indvars.iv.next292.i to <16 x i32>
  %cmp15.i = icmp slt <16 x i32> %127, %temp.vect182.i
  %notCond293.i = xor <16 x i1> %cmp15.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %who_left_tr294.i = and <16 x i1> %vectorPHI189.i, %notCond293.i
  %loop_mask3296.i = or <16 x i1> %vectorPHI.i, %who_left_tr294.i
  %ipred.i1.i = bitcast <16 x i1> %loop_mask3296.i to i16
  %val.i2.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i1.i, i16 %ipred.i1.i) nounwind
  %128 = and i32 %val.i2.i, 1
  %res.i3.i = icmp eq i32 %128, 0
  %local_edge315.i = and <16 x i1> %vectorPHI189.i, %cmp15.i
  br i1 %res.i3.i, label %for.body.i, label %for.end.i

for.end.i:                                        ; preds = %postload1916.i, %postload1763.i
  %vectorPHI332.i = phi <16 x float> [ undef, %postload1763.i ], [ %out_sel291.i, %postload1916.i ]
  %merge333.i = select <16 x i1> %if.then_to_for.end184.i, <16 x float> zeroinitializer, <16 x float> %vectorPHI332.i
  br i1 %extract117.i, label %preload1573.i, label %postload1574.i

preload1573.i:                                    ; preds = %for.end.i
  %exData.i = extractelement <16 x float> %merge333.i, i32 0
  %loadedValue3892.i = load float addrspace(3)** %CastToValueType3887.i, align 8
  store float %exData.i, float addrspace(3)* %loadedValue3892.i, align 4
  br label %postload1574.i

postload1574.i:                                   ; preds = %preload1573.i, %for.end.i
  br i1 %extract118.i, label %preload1591.i, label %postload1592.i

preload1591.i:                                    ; preds = %postload1574.i
  %loadedValue2988.i = load i64* %CastToValueType2983.i, align 8
  %.sum2921.i = add i64 %loadedValue2988.i, 1
  %129 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2921.i
  %exData566.i = extractelement <16 x float> %merge333.i, i32 1
  store float %exData566.i, float addrspace(3)* %129, align 4
  br label %postload1592.i

postload1592.i:                                   ; preds = %preload1591.i, %postload1574.i
  br i1 %extract119.i, label %preload1570.i, label %postload1571.i

preload1570.i:                                    ; preds = %postload1592.i
  %loadedValue2993.i = load i64* %CastToValueType2983.i, align 8
  %.sum2920.i = add i64 %loadedValue2993.i, 2
  %130 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2920.i
  %exData569.i = extractelement <16 x float> %merge333.i, i32 2
  store float %exData569.i, float addrspace(3)* %130, align 4
  br label %postload1571.i

postload1571.i:                                   ; preds = %preload1570.i, %postload1592.i
  br i1 %extract120.i, label %preload1600.i, label %postload1601.i

preload1600.i:                                    ; preds = %postload1571.i
  %loadedValue2998.i = load i64* %CastToValueType2983.i, align 8
  %.sum2919.i = add i64 %loadedValue2998.i, 3
  %131 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2919.i
  %exData572.i = extractelement <16 x float> %merge333.i, i32 3
  store float %exData572.i, float addrspace(3)* %131, align 4
  br label %postload1601.i

postload1601.i:                                   ; preds = %preload1600.i, %postload1571.i
  br i1 %extract121.i, label %preload.i, label %postload.i

preload.i:                                        ; preds = %postload1601.i
  %loadedValue3003.i = load i64* %CastToValueType2983.i, align 8
  %.sum2918.i = add i64 %loadedValue3003.i, 4
  %132 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2918.i
  %exData575.i = extractelement <16 x float> %merge333.i, i32 4
  store float %exData575.i, float addrspace(3)* %132, align 4
  br label %postload.i

postload.i:                                       ; preds = %preload.i, %postload1601.i
  br i1 %extract122.i, label %preload1732.i, label %postload1733.i

preload1732.i:                                    ; preds = %postload.i
  %loadedValue3008.i = load i64* %CastToValueType2983.i, align 8
  %.sum2917.i = add i64 %loadedValue3008.i, 5
  %133 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2917.i
  %exData578.i = extractelement <16 x float> %merge333.i, i32 5
  store float %exData578.i, float addrspace(3)* %133, align 4
  br label %postload1733.i

postload1733.i:                                   ; preds = %preload1732.i, %postload.i
  br i1 %extract123.i, label %preload1585.i, label %postload1586.i

preload1585.i:                                    ; preds = %postload1733.i
  %loadedValue3013.i = load i64* %CastToValueType2983.i, align 8
  %.sum2916.i = add i64 %loadedValue3013.i, 6
  %134 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2916.i
  %exData581.i = extractelement <16 x float> %merge333.i, i32 6
  store float %exData581.i, float addrspace(3)* %134, align 4
  br label %postload1586.i

postload1586.i:                                   ; preds = %preload1585.i, %postload1733.i
  br i1 %extract124.i, label %preload1582.i, label %postload1583.i

preload1582.i:                                    ; preds = %postload1586.i
  %loadedValue3018.i = load i64* %CastToValueType2983.i, align 8
  %.sum2915.i = add i64 %loadedValue3018.i, 7
  %135 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2915.i
  %exData584.i = extractelement <16 x float> %merge333.i, i32 7
  store float %exData584.i, float addrspace(3)* %135, align 4
  br label %postload1583.i

postload1583.i:                                   ; preds = %preload1582.i, %postload1586.i
  br i1 %extract125.i, label %preload1588.i, label %postload1589.i

preload1588.i:                                    ; preds = %postload1583.i
  %loadedValue3023.i = load i64* %CastToValueType2983.i, align 8
  %.sum2914.i = add i64 %loadedValue3023.i, 8
  %136 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2914.i
  %exData587.i = extractelement <16 x float> %merge333.i, i32 8
  store float %exData587.i, float addrspace(3)* %136, align 4
  br label %postload1589.i

postload1589.i:                                   ; preds = %preload1588.i, %postload1583.i
  br i1 %extract126.i, label %preload1666.i, label %postload1667.i

preload1666.i:                                    ; preds = %postload1589.i
  %loadedValue3028.i = load i64* %CastToValueType2983.i, align 8
  %.sum2913.i = add i64 %loadedValue3028.i, 9
  %137 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2913.i
  %exData590.i = extractelement <16 x float> %merge333.i, i32 9
  store float %exData590.i, float addrspace(3)* %137, align 4
  br label %postload1667.i

postload1667.i:                                   ; preds = %preload1666.i, %postload1589.i
  br i1 %extract127.i, label %preload1567.i, label %postload1568.i

preload1567.i:                                    ; preds = %postload1667.i
  %loadedValue3033.i = load i64* %CastToValueType2983.i, align 8
  %.sum2912.i = add i64 %loadedValue3033.i, 10
  %138 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2912.i
  %exData593.i = extractelement <16 x float> %merge333.i, i32 10
  store float %exData593.i, float addrspace(3)* %138, align 4
  br label %postload1568.i

postload1568.i:                                   ; preds = %preload1567.i, %postload1667.i
  br i1 %extract128.i, label %preload1579.i, label %postload1580.i

preload1579.i:                                    ; preds = %postload1568.i
  %loadedValue3038.i = load i64* %CastToValueType2983.i, align 8
  %.sum2911.i = add i64 %loadedValue3038.i, 11
  %139 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2911.i
  %exData596.i = extractelement <16 x float> %merge333.i, i32 11
  store float %exData596.i, float addrspace(3)* %139, align 4
  br label %postload1580.i

postload1580.i:                                   ; preds = %preload1579.i, %postload1568.i
  br i1 %extract129.i, label %preload1711.i, label %postload1712.i

preload1711.i:                                    ; preds = %postload1580.i
  %loadedValue3043.i = load i64* %CastToValueType2983.i, align 8
  %.sum2910.i = add i64 %loadedValue3043.i, 12
  %140 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2910.i
  %exData599.i = extractelement <16 x float> %merge333.i, i32 12
  store float %exData599.i, float addrspace(3)* %140, align 4
  br label %postload1712.i

postload1712.i:                                   ; preds = %preload1711.i, %postload1580.i
  br i1 %extract130.i, label %preload1576.i, label %postload1577.i

preload1576.i:                                    ; preds = %postload1712.i
  %loadedValue3048.i = load i64* %CastToValueType2983.i, align 8
  %.sum2909.i = add i64 %loadedValue3048.i, 13
  %141 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2909.i
  %exData602.i = extractelement <16 x float> %merge333.i, i32 13
  store float %exData602.i, float addrspace(3)* %141, align 4
  br label %postload1577.i

postload1577.i:                                   ; preds = %preload1576.i, %postload1712.i
  br i1 %extract131.i, label %preload2005.i, label %postload2006.i

preload2005.i:                                    ; preds = %postload1577.i
  %loadedValue3053.i = load i64* %CastToValueType2983.i, align 8
  %.sum2908.i = add i64 %loadedValue3053.i, 14
  %142 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2908.i
  %exData605.i = extractelement <16 x float> %merge333.i, i32 14
  store float %exData605.i, float addrspace(3)* %142, align 4
  br label %postload2006.i

postload2006.i:                                   ; preds = %preload2005.i, %postload1577.i
  br i1 %extract132.i, label %preload1705.i, label %postload2006.i.postload1706.i_crit_edge

postload2006.i.postload1706.i_crit_edge:          ; preds = %postload2006.i
  br label %postload1706.i

preload1705.i:                                    ; preds = %postload2006.i
  %loadedValue3058.i = load i64* %CastToValueType2983.i, align 8
  %.sum2907.i = add i64 %loadedValue3058.i, 15
  %143 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2907.i
  %exData608.i = extractelement <16 x float> %merge333.i, i32 15
  store float %exData608.i, float addrspace(3)* %143, align 4
  br label %postload1706.i

postload1706.i:                                   ; preds = %postload2006.i.postload1706.i_crit_edge, %preload1705.i
  %loadedValue4025.i = load i1* %CastToValueType3990.i, align 1
  br i1 %loadedValue4025.i, label %preload1675.i, label %postload1706.i.postload1676.i_crit_edge

postload1706.i.postload1676.i_crit_edge:          ; preds = %postload1706.i
  br label %postload1676.i

preload1675.i:                                    ; preds = %postload1706.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %31
  br i1 %check.WI.iter.i, label %thenBB.i, label %preload1675.i.postload1676.i_crit_edge

preload1675.i.postload1676.i_crit_edge:           ; preds = %preload1675.i
  br label %postload1676.i

thenBB.i:                                         ; preds = %preload1675.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 256
  br label %SyncBB4569.i

postload1676.i:                                   ; preds = %thenBB4611.i.postload1676.i_crit_edge, %thenBB4603.i.postload1676.i_crit_edge, %thenBB4595.i.postload1676.i_crit_edge, %thenBB4587.i.postload1676.i_crit_edge, %thenBB4579.i.postload1676.i_crit_edge, %thenBB4572.i.postload1676.i_crit_edge, %preload1675.i.postload1676.i_crit_edge, %postload1706.i.postload1676.i_crit_edge
  %CurrWI..2.i = phi i64 [ %CurrWI..0.i, %postload1706.i.postload1676.i_crit_edge ], [ 0, %preload1675.i.postload1676.i_crit_edge ], [ %"CurrWI++4576.i", %thenBB4572.i.postload1676.i_crit_edge ], [ %"CurrWI++4583.i", %thenBB4579.i.postload1676.i_crit_edge ], [ %"CurrWI++4591.i", %thenBB4587.i.postload1676.i_crit_edge ], [ %"CurrWI++4599.i", %thenBB4595.i.postload1676.i_crit_edge ], [ %"CurrWI++4607.i", %thenBB4603.i.postload1676.i_crit_edge ], [ %"CurrWI++4615.i", %thenBB4611.i.postload1676.i_crit_edge ]
  %CurrSBIndex..2.i = phi i64 [ %CurrSBIndex..0.i, %postload1706.i.postload1676.i_crit_edge ], [ 0, %preload1675.i.postload1676.i_crit_edge ], [ %"loadedCurrSB+Stride4578.i", %thenBB4572.i.postload1676.i_crit_edge ], [ %"loadedCurrSB+Stride4585.i", %thenBB4579.i.postload1676.i_crit_edge ], [ %"loadedCurrSB+Stride4593.i", %thenBB4587.i.postload1676.i_crit_edge ], [ %"loadedCurrSB+Stride4601.i", %thenBB4595.i.postload1676.i_crit_edge ], [ %"loadedCurrSB+Stride4609.i", %thenBB4603.i.postload1676.i_crit_edge ], [ %"loadedCurrSB+Stride4617.i", %thenBB4611.i.postload1676.i_crit_edge ]
  %currBarrier.2.i = phi i32 [ %currBarrier.0.ph.i, %postload1706.i.postload1676.i_crit_edge ], [ 6, %preload1675.i.postload1676.i_crit_edge ], [ %currBarrier.2.i, %thenBB4572.i.postload1676.i_crit_edge ], [ %currBarrier.4.i, %thenBB4579.i.postload1676.i_crit_edge ], [ %currBarrier.6.i, %thenBB4587.i.postload1676.i_crit_edge ], [ %currBarrier.8.i, %thenBB4595.i.postload1676.i_crit_edge ], [ %currBarrier.10.i, %thenBB4603.i.postload1676.i_crit_edge ], [ %currBarrier.12.i, %thenBB4611.i.postload1676.i_crit_edge ]
  %"&pSB[currWI].offset2943.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..2.i
  %CastToValueType2944.i = bitcast i8* %"&pSB[currWI].offset2943.i" to <16 x i32>*
  %loadedValue2945.i = load <16 x i32>* %CastToValueType2944.i, align 64
  %cmp28.i = icmp ult <16 x i32> %loadedValue2945.i, <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>
  %"&(pSB[currWI].offset)3963.i" = add nuw i64 %CurrSBIndex..2.i, 96
  %"&pSB[currWI].offset3964.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3963.i"
  %CastToValueType3965.i = bitcast i8* %"&pSB[currWI].offset3964.i" to <16 x i1>*
  %loadedValue3966.i = load <16 x i1>* %CastToValueType3965.i, align 16
  %for.end_to_if.then30335.i = and <16 x i1> %loadedValue3966.i, %cmp28.i
  %"&(pSB[currWI].offset)2976.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset2977.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)2976.i"
  %CastToValueType2978.i = bitcast i8* %"&pSB[currWI].offset2977.i" to i32*
  %loadedValue2979.i = load i32* %CastToValueType2978.i, align 4
  %144 = add i32 %loadedValue2979.i, 16
  %extract338.i = sext i32 %144 to i64
  %exmask610.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 0
  br i1 %exmask610.i, label %preload1708.i, label %postload1709.i

preload1708.i:                                    ; preds = %postload1676.i
  %145 = getelementptr inbounds [128 x float] addrspace(3)* %35, i64 0, i64 %extract338.i
  %vload611.i = load float addrspace(3)* %145, align 4
  br label %postload1709.i

postload1709.i:                                   ; preds = %preload1708.i, %postload1676.i
  %phi1710.i = phi float [ undef, %postload1676.i ], [ %vload611.i, %preload1708.i ]
  %vpack.i = insertelement <16 x float> undef, float %phi1710.i, i32 0
  %exmask613.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 1
  br i1 %exmask613.i, label %preload1729.i, label %postload1730.i

preload1729.i:                                    ; preds = %postload1709.i
  %.sum2906.i = add i64 %extract338.i, 1
  %146 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2906.i
  %vload614.i = load float addrspace(3)* %146, align 4
  br label %postload1730.i

postload1730.i:                                   ; preds = %preload1729.i, %postload1709.i
  %phi1731.i = phi float [ undef, %postload1709.i ], [ %vload614.i, %preload1729.i ]
  %vpack615.i = insertelement <16 x float> %vpack.i, float %phi1731.i, i32 1
  %exmask617.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 2
  br i1 %exmask617.i, label %preload1714.i, label %postload1715.i

preload1714.i:                                    ; preds = %postload1730.i
  %.sum2905.i = add i64 %extract338.i, 2
  %147 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2905.i
  %vload618.i = load float addrspace(3)* %147, align 4
  br label %postload1715.i

postload1715.i:                                   ; preds = %preload1714.i, %postload1730.i
  %phi1716.i = phi float [ undef, %postload1730.i ], [ %vload618.i, %preload1714.i ]
  %vpack619.i = insertelement <16 x float> %vpack615.i, float %phi1716.i, i32 2
  %exmask621.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 3
  br i1 %exmask621.i, label %preload1717.i, label %postload1718.i

preload1717.i:                                    ; preds = %postload1715.i
  %.sum2904.i = add i64 %extract338.i, 3
  %148 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2904.i
  %vload622.i = load float addrspace(3)* %148, align 4
  br label %postload1718.i

postload1718.i:                                   ; preds = %preload1717.i, %postload1715.i
  %phi1719.i = phi float [ undef, %postload1715.i ], [ %vload622.i, %preload1717.i ]
  %vpack623.i = insertelement <16 x float> %vpack619.i, float %phi1719.i, i32 3
  %exmask625.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 4
  br i1 %exmask625.i, label %preload1918.i, label %postload1919.i

preload1918.i:                                    ; preds = %postload1718.i
  %.sum2903.i = add i64 %extract338.i, 4
  %149 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2903.i
  %vload626.i = load float addrspace(3)* %149, align 4
  br label %postload1919.i

postload1919.i:                                   ; preds = %preload1918.i, %postload1718.i
  %phi1920.i = phi float [ undef, %postload1718.i ], [ %vload626.i, %preload1918.i ]
  %vpack627.i = insertelement <16 x float> %vpack623.i, float %phi1920.i, i32 4
  %exmask629.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 5
  br i1 %exmask629.i, label %preload1612.i, label %postload1613.i

preload1612.i:                                    ; preds = %postload1919.i
  %.sum2902.i = add i64 %extract338.i, 5
  %150 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2902.i
  %vload630.i = load float addrspace(3)* %150, align 4
  br label %postload1613.i

postload1613.i:                                   ; preds = %preload1612.i, %postload1919.i
  %phi1614.i = phi float [ undef, %postload1919.i ], [ %vload630.i, %preload1612.i ]
  %vpack631.i = insertelement <16 x float> %vpack627.i, float %phi1614.i, i32 5
  %exmask633.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 6
  br i1 %exmask633.i, label %preload1615.i, label %postload1616.i

preload1615.i:                                    ; preds = %postload1613.i
  %.sum2901.i = add i64 %extract338.i, 6
  %151 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2901.i
  %vload634.i = load float addrspace(3)* %151, align 4
  br label %postload1616.i

postload1616.i:                                   ; preds = %preload1615.i, %postload1613.i
  %phi1617.i = phi float [ undef, %postload1613.i ], [ %vload634.i, %preload1615.i ]
  %vpack635.i = insertelement <16 x float> %vpack631.i, float %phi1617.i, i32 6
  %exmask637.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 7
  br i1 %exmask637.i, label %preload1594.i, label %postload1595.i

preload1594.i:                                    ; preds = %postload1616.i
  %.sum2900.i = add i64 %extract338.i, 7
  %152 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2900.i
  %vload638.i = load float addrspace(3)* %152, align 4
  br label %postload1595.i

postload1595.i:                                   ; preds = %preload1594.i, %postload1616.i
  %phi1596.i = phi float [ undef, %postload1616.i ], [ %vload638.i, %preload1594.i ]
  %vpack639.i = insertelement <16 x float> %vpack635.i, float %phi1596.i, i32 7
  %exmask641.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 8
  br i1 %exmask641.i, label %preload1597.i, label %postload1598.i

preload1597.i:                                    ; preds = %postload1595.i
  %.sum2899.i = add i64 %extract338.i, 8
  %153 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2899.i
  %vload642.i = load float addrspace(3)* %153, align 4
  br label %postload1598.i

postload1598.i:                                   ; preds = %preload1597.i, %postload1595.i
  %phi1599.i = phi float [ undef, %postload1595.i ], [ %vload642.i, %preload1597.i ]
  %vpack643.i = insertelement <16 x float> %vpack639.i, float %phi1599.i, i32 8
  %exmask645.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 9
  br i1 %exmask645.i, label %preload1978.i, label %postload1979.i

preload1978.i:                                    ; preds = %postload1598.i
  %.sum2898.i = add i64 %extract338.i, 9
  %154 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2898.i
  %vload646.i = load float addrspace(3)* %154, align 4
  br label %postload1979.i

postload1979.i:                                   ; preds = %preload1978.i, %postload1598.i
  %phi1980.i = phi float [ undef, %postload1598.i ], [ %vload646.i, %preload1978.i ]
  %vpack647.i = insertelement <16 x float> %vpack643.i, float %phi1980.i, i32 9
  %exmask649.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 10
  br i1 %exmask649.i, label %preload1981.i, label %postload1982.i

preload1981.i:                                    ; preds = %postload1979.i
  %.sum2897.i = add i64 %extract338.i, 10
  %155 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2897.i
  %vload650.i = load float addrspace(3)* %155, align 4
  br label %postload1982.i

postload1982.i:                                   ; preds = %preload1981.i, %postload1979.i
  %phi1983.i = phi float [ undef, %postload1979.i ], [ %vload650.i, %preload1981.i ]
  %vpack651.i = insertelement <16 x float> %vpack647.i, float %phi1983.i, i32 10
  %exmask653.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 11
  br i1 %exmask653.i, label %preload1984.i, label %postload1985.i

preload1984.i:                                    ; preds = %postload1982.i
  %.sum2896.i = add i64 %extract338.i, 11
  %156 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2896.i
  %vload654.i = load float addrspace(3)* %156, align 4
  br label %postload1985.i

postload1985.i:                                   ; preds = %preload1984.i, %postload1982.i
  %phi1986.i = phi float [ undef, %postload1982.i ], [ %vload654.i, %preload1984.i ]
  %vpack655.i = insertelement <16 x float> %vpack651.i, float %phi1986.i, i32 11
  %exmask657.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 12
  br i1 %exmask657.i, label %preload1735.i, label %postload1736.i

preload1735.i:                                    ; preds = %postload1985.i
  %.sum2895.i = add i64 %extract338.i, 12
  %157 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2895.i
  %vload658.i = load float addrspace(3)* %157, align 4
  br label %postload1736.i

postload1736.i:                                   ; preds = %preload1735.i, %postload1985.i
  %phi1737.i = phi float [ undef, %postload1985.i ], [ %vload658.i, %preload1735.i ]
  %vpack659.i = insertelement <16 x float> %vpack655.i, float %phi1737.i, i32 12
  %exmask661.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 13
  br i1 %exmask661.i, label %preload1738.i, label %postload1739.i

preload1738.i:                                    ; preds = %postload1736.i
  %.sum2894.i = add i64 %extract338.i, 13
  %158 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2894.i
  %vload662.i = load float addrspace(3)* %158, align 4
  br label %postload1739.i

postload1739.i:                                   ; preds = %preload1738.i, %postload1736.i
  %phi1740.i = phi float [ undef, %postload1736.i ], [ %vload662.i, %preload1738.i ]
  %vpack663.i = insertelement <16 x float> %vpack659.i, float %phi1740.i, i32 13
  %exmask665.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 14
  br i1 %exmask665.i, label %preload1987.i, label %postload1988.i

preload1987.i:                                    ; preds = %postload1739.i
  %.sum2893.i = add i64 %extract338.i, 14
  %159 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2893.i
  %vload666.i = load float addrspace(3)* %159, align 4
  br label %postload1988.i

postload1988.i:                                   ; preds = %preload1987.i, %postload1739.i
  %phi1989.i = phi float [ undef, %postload1739.i ], [ %vload666.i, %preload1987.i ]
  %vpack667.i = insertelement <16 x float> %vpack663.i, float %phi1989.i, i32 14
  %exmask669.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 15
  br i1 %exmask669.i, label %preload1990.i, label %postload1991.i

preload1990.i:                                    ; preds = %postload1988.i
  %.sum2892.i = add i64 %extract338.i, 15
  %160 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2892.i
  %vload670.i = load float addrspace(3)* %160, align 4
  br label %postload1991.i

postload1991.i:                                   ; preds = %preload1990.i, %postload1988.i
  %phi1992.i = phi float [ undef, %postload1988.i ], [ %vload670.i, %preload1990.i ]
  %vpack671.i = insertelement <16 x float> %vpack667.i, float %phi1992.i, i32 15
  br i1 %exmask610.i, label %preload1993.i, label %postload1994.i

preload1993.i:                                    ; preds = %postload1991.i
  %"&(pSB[currWI].offset)3894.i" = add nuw i64 %CurrSBIndex..2.i, 80
  %"&pSB[currWI].offset3895.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3894.i"
  %CastToValueType3896.i = bitcast i8* %"&pSB[currWI].offset3895.i" to float addrspace(3)**
  %loadedValue3897.i = load float addrspace(3)** %CastToValueType3896.i, align 8
  %vload675.i = load float addrspace(3)* %loadedValue3897.i, align 4
  br label %postload1994.i

postload1994.i:                                   ; preds = %preload1993.i, %postload1991.i
  %phi1995.i = phi float [ undef, %postload1991.i ], [ %vload675.i, %preload1993.i ]
  %vpack676.i = insertelement <16 x float> undef, float %phi1995.i, i32 0
  br i1 %exmask613.i, label %preload1720.i, label %postload1721.i

preload1720.i:                                    ; preds = %postload1994.i
  %"&(pSB[currWI].offset)3060.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3061.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3060.i"
  %CastToValueType3062.i = bitcast i8* %"&pSB[currWI].offset3061.i" to i64*
  %loadedValue3063.i = load i64* %CastToValueType3062.i, align 8
  %.sum2891.i = add i64 %loadedValue3063.i, 1
  %161 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2891.i
  %vload679.i = load float addrspace(3)* %161, align 4
  br label %postload1721.i

postload1721.i:                                   ; preds = %preload1720.i, %postload1994.i
  %phi1722.i = phi float [ undef, %postload1994.i ], [ %vload679.i, %preload1720.i ]
  %vpack680.i = insertelement <16 x float> %vpack676.i, float %phi1722.i, i32 1
  br i1 %exmask617.i, label %preload1723.i, label %postload1724.i

preload1723.i:                                    ; preds = %postload1721.i
  %"&(pSB[currWI].offset)3065.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3066.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3065.i"
  %CastToValueType3067.i = bitcast i8* %"&pSB[currWI].offset3066.i" to i64*
  %loadedValue3068.i = load i64* %CastToValueType3067.i, align 8
  %.sum2890.i = add i64 %loadedValue3068.i, 2
  %162 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2890.i
  %vload683.i = load float addrspace(3)* %162, align 4
  br label %postload1724.i

postload1724.i:                                   ; preds = %preload1723.i, %postload1721.i
  %phi1725.i = phi float [ undef, %postload1721.i ], [ %vload683.i, %preload1723.i ]
  %vpack684.i = insertelement <16 x float> %vpack680.i, float %phi1725.i, i32 2
  br i1 %exmask621.i, label %preload1726.i, label %postload1727.i

preload1726.i:                                    ; preds = %postload1724.i
  %"&(pSB[currWI].offset)3070.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3071.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3070.i"
  %CastToValueType3072.i = bitcast i8* %"&pSB[currWI].offset3071.i" to i64*
  %loadedValue3073.i = load i64* %CastToValueType3072.i, align 8
  %.sum2889.i = add i64 %loadedValue3073.i, 3
  %163 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2889.i
  %vload687.i = load float addrspace(3)* %163, align 4
  br label %postload1727.i

postload1727.i:                                   ; preds = %preload1726.i, %postload1724.i
  %phi1728.i = phi float [ undef, %postload1724.i ], [ %vload687.i, %preload1726.i ]
  %vpack688.i = insertelement <16 x float> %vpack684.i, float %phi1728.i, i32 3
  br i1 %exmask625.i, label %preload1765.i, label %postload1766.i

preload1765.i:                                    ; preds = %postload1727.i
  %"&(pSB[currWI].offset)3075.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3076.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3075.i"
  %CastToValueType3077.i = bitcast i8* %"&pSB[currWI].offset3076.i" to i64*
  %loadedValue3078.i = load i64* %CastToValueType3077.i, align 8
  %.sum2888.i = add i64 %loadedValue3078.i, 4
  %164 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2888.i
  %vload691.i = load float addrspace(3)* %164, align 4
  br label %postload1766.i

postload1766.i:                                   ; preds = %preload1765.i, %postload1727.i
  %phi1767.i = phi float [ undef, %postload1727.i ], [ %vload691.i, %preload1765.i ]
  %vpack692.i = insertelement <16 x float> %vpack688.i, float %phi1767.i, i32 4
  br i1 %exmask629.i, label %preload1768.i, label %postload1769.i

preload1768.i:                                    ; preds = %postload1766.i
  %"&(pSB[currWI].offset)3080.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3081.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3080.i"
  %CastToValueType3082.i = bitcast i8* %"&pSB[currWI].offset3081.i" to i64*
  %loadedValue3083.i = load i64* %CastToValueType3082.i, align 8
  %.sum2887.i = add i64 %loadedValue3083.i, 5
  %165 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2887.i
  %vload695.i = load float addrspace(3)* %165, align 4
  br label %postload1769.i

postload1769.i:                                   ; preds = %preload1768.i, %postload1766.i
  %phi1770.i = phi float [ undef, %postload1766.i ], [ %vload695.i, %preload1768.i ]
  %vpack696.i = insertelement <16 x float> %vpack692.i, float %phi1770.i, i32 5
  br i1 %exmask633.i, label %preload1771.i, label %postload1772.i

preload1771.i:                                    ; preds = %postload1769.i
  %"&(pSB[currWI].offset)3085.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3086.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3085.i"
  %CastToValueType3087.i = bitcast i8* %"&pSB[currWI].offset3086.i" to i64*
  %loadedValue3088.i = load i64* %CastToValueType3087.i, align 8
  %.sum2886.i = add i64 %loadedValue3088.i, 6
  %166 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2886.i
  %vload699.i = load float addrspace(3)* %166, align 4
  br label %postload1772.i

postload1772.i:                                   ; preds = %preload1771.i, %postload1769.i
  %phi1773.i = phi float [ undef, %postload1769.i ], [ %vload699.i, %preload1771.i ]
  %vpack700.i = insertelement <16 x float> %vpack696.i, float %phi1773.i, i32 6
  br i1 %exmask637.i, label %preload2038.i, label %postload2039.i

preload2038.i:                                    ; preds = %postload1772.i
  %"&(pSB[currWI].offset)3090.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3091.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3090.i"
  %CastToValueType3092.i = bitcast i8* %"&pSB[currWI].offset3091.i" to i64*
  %loadedValue3093.i = load i64* %CastToValueType3092.i, align 8
  %.sum2885.i = add i64 %loadedValue3093.i, 7
  %167 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2885.i
  %vload703.i = load float addrspace(3)* %167, align 4
  br label %postload2039.i

postload2039.i:                                   ; preds = %preload2038.i, %postload1772.i
  %phi2040.i = phi float [ undef, %postload1772.i ], [ %vload703.i, %preload2038.i ]
  %vpack704.i = insertelement <16 x float> %vpack700.i, float %phi2040.i, i32 7
  br i1 %exmask641.i, label %preload2041.i, label %postload2042.i

preload2041.i:                                    ; preds = %postload2039.i
  %"&(pSB[currWI].offset)3095.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3096.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3095.i"
  %CastToValueType3097.i = bitcast i8* %"&pSB[currWI].offset3096.i" to i64*
  %loadedValue3098.i = load i64* %CastToValueType3097.i, align 8
  %.sum2884.i = add i64 %loadedValue3098.i, 8
  %168 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2884.i
  %vload707.i = load float addrspace(3)* %168, align 4
  br label %postload2042.i

postload2042.i:                                   ; preds = %preload2041.i, %postload2039.i
  %phi2043.i = phi float [ undef, %postload2039.i ], [ %vload707.i, %preload2041.i ]
  %vpack708.i = insertelement <16 x float> %vpack704.i, float %phi2043.i, i32 8
  br i1 %exmask645.i, label %preload2044.i, label %postload2045.i

preload2044.i:                                    ; preds = %postload2042.i
  %"&(pSB[currWI].offset)3100.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3101.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3100.i"
  %CastToValueType3102.i = bitcast i8* %"&pSB[currWI].offset3101.i" to i64*
  %loadedValue3103.i = load i64* %CastToValueType3102.i, align 8
  %.sum2883.i = add i64 %loadedValue3103.i, 9
  %169 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2883.i
  %vload711.i = load float addrspace(3)* %169, align 4
  br label %postload2045.i

postload2045.i:                                   ; preds = %preload2044.i, %postload2042.i
  %phi2046.i = phi float [ undef, %postload2042.i ], [ %vload711.i, %preload2044.i ]
  %vpack712.i = insertelement <16 x float> %vpack708.i, float %phi2046.i, i32 9
  br i1 %exmask649.i, label %preload1996.i, label %postload1997.i

preload1996.i:                                    ; preds = %postload2045.i
  %"&(pSB[currWI].offset)3105.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3106.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3105.i"
  %CastToValueType3107.i = bitcast i8* %"&pSB[currWI].offset3106.i" to i64*
  %loadedValue3108.i = load i64* %CastToValueType3107.i, align 8
  %.sum2882.i = add i64 %loadedValue3108.i, 10
  %170 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2882.i
  %vload715.i = load float addrspace(3)* %170, align 4
  br label %postload1997.i

postload1997.i:                                   ; preds = %preload1996.i, %postload2045.i
  %phi1998.i = phi float [ undef, %postload2045.i ], [ %vload715.i, %preload1996.i ]
  %vpack716.i = insertelement <16 x float> %vpack712.i, float %phi1998.i, i32 10
  br i1 %exmask653.i, label %preload1999.i, label %postload2000.i

preload1999.i:                                    ; preds = %postload1997.i
  %"&(pSB[currWI].offset)3110.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3111.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3110.i"
  %CastToValueType3112.i = bitcast i8* %"&pSB[currWI].offset3111.i" to i64*
  %loadedValue3113.i = load i64* %CastToValueType3112.i, align 8
  %.sum2881.i = add i64 %loadedValue3113.i, 11
  %171 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2881.i
  %vload719.i = load float addrspace(3)* %171, align 4
  br label %postload2000.i

postload2000.i:                                   ; preds = %preload1999.i, %postload1997.i
  %phi2001.i = phi float [ undef, %postload1997.i ], [ %vload719.i, %preload1999.i ]
  %vpack720.i = insertelement <16 x float> %vpack716.i, float %phi2001.i, i32 11
  br i1 %exmask657.i, label %preload2002.i, label %postload2003.i

preload2002.i:                                    ; preds = %postload2000.i
  %"&(pSB[currWI].offset)3115.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3116.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3115.i"
  %CastToValueType3117.i = bitcast i8* %"&pSB[currWI].offset3116.i" to i64*
  %loadedValue3118.i = load i64* %CastToValueType3117.i, align 8
  %.sum2880.i = add i64 %loadedValue3118.i, 12
  %172 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2880.i
  %vload723.i = load float addrspace(3)* %172, align 4
  br label %postload2003.i

postload2003.i:                                   ; preds = %preload2002.i, %postload2000.i
  %phi2004.i = phi float [ undef, %postload2000.i ], [ %vload723.i, %preload2002.i ]
  %vpack724.i = insertelement <16 x float> %vpack720.i, float %phi2004.i, i32 12
  br i1 %exmask661.i, label %preload2008.i, label %postload2009.i

preload2008.i:                                    ; preds = %postload2003.i
  %"&(pSB[currWI].offset)3120.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3121.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3120.i"
  %CastToValueType3122.i = bitcast i8* %"&pSB[currWI].offset3121.i" to i64*
  %loadedValue3123.i = load i64* %CastToValueType3122.i, align 8
  %.sum2879.i = add i64 %loadedValue3123.i, 13
  %173 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2879.i
  %vload727.i = load float addrspace(3)* %173, align 4
  br label %postload2009.i

postload2009.i:                                   ; preds = %preload2008.i, %postload2003.i
  %phi2010.i = phi float [ undef, %postload2003.i ], [ %vload727.i, %preload2008.i ]
  %vpack728.i = insertelement <16 x float> %vpack724.i, float %phi2010.i, i32 13
  br i1 %exmask665.i, label %preload2011.i, label %postload2012.i

preload2011.i:                                    ; preds = %postload2009.i
  %"&(pSB[currWI].offset)3125.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3126.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3125.i"
  %CastToValueType3127.i = bitcast i8* %"&pSB[currWI].offset3126.i" to i64*
  %loadedValue3128.i = load i64* %CastToValueType3127.i, align 8
  %.sum2878.i = add i64 %loadedValue3128.i, 14
  %174 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2878.i
  %vload731.i = load float addrspace(3)* %174, align 4
  br label %postload2012.i

postload2012.i:                                   ; preds = %preload2011.i, %postload2009.i
  %phi2013.i = phi float [ undef, %postload2009.i ], [ %vload731.i, %preload2011.i ]
  %vpack732.i = insertelement <16 x float> %vpack728.i, float %phi2013.i, i32 14
  br i1 %exmask669.i, label %preload2014.i, label %postload2015.i

preload2014.i:                                    ; preds = %postload2012.i
  %"&(pSB[currWI].offset)3130.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3131.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3130.i"
  %CastToValueType3132.i = bitcast i8* %"&pSB[currWI].offset3131.i" to i64*
  %loadedValue3133.i = load i64* %CastToValueType3132.i, align 8
  %.sum2877.i = add i64 %loadedValue3133.i, 15
  %175 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2877.i
  %vload735.i = load float addrspace(3)* %175, align 4
  br label %postload2015.i

postload2015.i:                                   ; preds = %preload2014.i, %postload2012.i
  %phi2016.i = phi float [ undef, %postload2012.i ], [ %vload735.i, %preload2014.i ]
  %vpack736.i = insertelement <16 x float> %vpack732.i, float %phi2016.i, i32 15
  %add36356.i = fadd <16 x float> %vpack736.i, %vpack671.i
  br i1 %exmask610.i, label %preload2047.i, label %postload2048.i

preload2047.i:                                    ; preds = %postload2015.i
  %exData740.i = extractelement <16 x float> %add36356.i, i32 0
  %"&(pSB[currWI].offset)3899.i" = add nuw i64 %CurrSBIndex..2.i, 80
  %"&pSB[currWI].offset3900.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3899.i"
  %CastToValueType3901.i = bitcast i8* %"&pSB[currWI].offset3900.i" to float addrspace(3)**
  %loadedValue3902.i = load float addrspace(3)** %CastToValueType3901.i, align 8
  store float %exData740.i, float addrspace(3)* %loadedValue3902.i, align 4
  br label %postload2048.i

postload2048.i:                                   ; preds = %preload2047.i, %postload2015.i
  br i1 %exmask613.i, label %preload2050.i, label %postload2051.i

preload2050.i:                                    ; preds = %postload2048.i
  %"&(pSB[currWI].offset)3135.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3136.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3135.i"
  %CastToValueType3137.i = bitcast i8* %"&pSB[currWI].offset3136.i" to i64*
  %loadedValue3138.i = load i64* %CastToValueType3137.i, align 8
  %.sum2876.i = add i64 %loadedValue3138.i, 1
  %176 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2876.i
  %exData743.i = extractelement <16 x float> %add36356.i, i32 1
  store float %exData743.i, float addrspace(3)* %176, align 4
  br label %postload2051.i

postload2051.i:                                   ; preds = %preload2050.i, %postload2048.i
  br i1 %exmask617.i, label %preload2053.i, label %postload2054.i

preload2053.i:                                    ; preds = %postload2051.i
  %"&(pSB[currWI].offset)3140.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3141.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3140.i"
  %CastToValueType3142.i = bitcast i8* %"&pSB[currWI].offset3141.i" to i64*
  %loadedValue3143.i = load i64* %CastToValueType3142.i, align 8
  %.sum2875.i = add i64 %loadedValue3143.i, 2
  %177 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2875.i
  %exData746.i = extractelement <16 x float> %add36356.i, i32 2
  store float %exData746.i, float addrspace(3)* %177, align 4
  br label %postload2054.i

postload2054.i:                                   ; preds = %preload2053.i, %postload2051.i
  br i1 %exmask621.i, label %preload2056.i, label %postload2057.i

preload2056.i:                                    ; preds = %postload2054.i
  %"&(pSB[currWI].offset)3145.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3146.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3145.i"
  %CastToValueType3147.i = bitcast i8* %"&pSB[currWI].offset3146.i" to i64*
  %loadedValue3148.i = load i64* %CastToValueType3147.i, align 8
  %.sum2874.i = add i64 %loadedValue3148.i, 3
  %178 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2874.i
  %exData749.i = extractelement <16 x float> %add36356.i, i32 3
  store float %exData749.i, float addrspace(3)* %178, align 4
  br label %postload2057.i

postload2057.i:                                   ; preds = %preload2056.i, %postload2054.i
  br i1 %exmask625.i, label %preload1603.i, label %postload1604.i

preload1603.i:                                    ; preds = %postload2057.i
  %"&(pSB[currWI].offset)3150.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3151.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3150.i"
  %CastToValueType3152.i = bitcast i8* %"&pSB[currWI].offset3151.i" to i64*
  %loadedValue3153.i = load i64* %CastToValueType3152.i, align 8
  %.sum2873.i = add i64 %loadedValue3153.i, 4
  %179 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2873.i
  %exData752.i = extractelement <16 x float> %add36356.i, i32 4
  store float %exData752.i, float addrspace(3)* %179, align 4
  br label %postload1604.i

postload1604.i:                                   ; preds = %preload1603.i, %postload2057.i
  br i1 %exmask629.i, label %preload1606.i, label %postload1607.i

preload1606.i:                                    ; preds = %postload1604.i
  %"&(pSB[currWI].offset)3155.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3156.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3155.i"
  %CastToValueType3157.i = bitcast i8* %"&pSB[currWI].offset3156.i" to i64*
  %loadedValue3158.i = load i64* %CastToValueType3157.i, align 8
  %.sum2872.i = add i64 %loadedValue3158.i, 5
  %180 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2872.i
  %exData755.i = extractelement <16 x float> %add36356.i, i32 5
  store float %exData755.i, float addrspace(3)* %180, align 4
  br label %postload1607.i

postload1607.i:                                   ; preds = %preload1606.i, %postload1604.i
  br i1 %exmask633.i, label %preload1609.i, label %postload1610.i

preload1609.i:                                    ; preds = %postload1607.i
  %"&(pSB[currWI].offset)3160.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3161.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3160.i"
  %CastToValueType3162.i = bitcast i8* %"&pSB[currWI].offset3161.i" to i64*
  %loadedValue3163.i = load i64* %CastToValueType3162.i, align 8
  %.sum2871.i = add i64 %loadedValue3163.i, 6
  %181 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2871.i
  %exData758.i = extractelement <16 x float> %add36356.i, i32 6
  store float %exData758.i, float addrspace(3)* %181, align 4
  br label %postload1610.i

postload1610.i:                                   ; preds = %preload1609.i, %postload1607.i
  br i1 %exmask637.i, label %preload2017.i, label %postload2018.i

preload2017.i:                                    ; preds = %postload1610.i
  %"&(pSB[currWI].offset)3165.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3166.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3165.i"
  %CastToValueType3167.i = bitcast i8* %"&pSB[currWI].offset3166.i" to i64*
  %loadedValue3168.i = load i64* %CastToValueType3167.i, align 8
  %.sum2870.i = add i64 %loadedValue3168.i, 7
  %182 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2870.i
  %exData761.i = extractelement <16 x float> %add36356.i, i32 7
  store float %exData761.i, float addrspace(3)* %182, align 4
  br label %postload2018.i

postload2018.i:                                   ; preds = %preload2017.i, %postload1610.i
  br i1 %exmask641.i, label %preload2020.i, label %postload2021.i

preload2020.i:                                    ; preds = %postload2018.i
  %"&(pSB[currWI].offset)3170.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3171.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3170.i"
  %CastToValueType3172.i = bitcast i8* %"&pSB[currWI].offset3171.i" to i64*
  %loadedValue3173.i = load i64* %CastToValueType3172.i, align 8
  %.sum2869.i = add i64 %loadedValue3173.i, 8
  %183 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2869.i
  %exData764.i = extractelement <16 x float> %add36356.i, i32 8
  store float %exData764.i, float addrspace(3)* %183, align 4
  br label %postload2021.i

postload2021.i:                                   ; preds = %preload2020.i, %postload2018.i
  br i1 %exmask645.i, label %preload2023.i, label %postload2024.i

preload2023.i:                                    ; preds = %postload2021.i
  %"&(pSB[currWI].offset)3175.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3176.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3175.i"
  %CastToValueType3177.i = bitcast i8* %"&pSB[currWI].offset3176.i" to i64*
  %loadedValue3178.i = load i64* %CastToValueType3177.i, align 8
  %.sum2868.i = add i64 %loadedValue3178.i, 9
  %184 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2868.i
  %exData767.i = extractelement <16 x float> %add36356.i, i32 9
  store float %exData767.i, float addrspace(3)* %184, align 4
  br label %postload2024.i

postload2024.i:                                   ; preds = %preload2023.i, %postload2021.i
  br i1 %exmask649.i, label %preload2026.i, label %postload2027.i

preload2026.i:                                    ; preds = %postload2024.i
  %"&(pSB[currWI].offset)3180.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3181.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3180.i"
  %CastToValueType3182.i = bitcast i8* %"&pSB[currWI].offset3181.i" to i64*
  %loadedValue3183.i = load i64* %CastToValueType3182.i, align 8
  %.sum2867.i = add i64 %loadedValue3183.i, 10
  %185 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2867.i
  %exData770.i = extractelement <16 x float> %add36356.i, i32 10
  store float %exData770.i, float addrspace(3)* %185, align 4
  br label %postload2027.i

postload2027.i:                                   ; preds = %preload2026.i, %postload2024.i
  br i1 %exmask653.i, label %preload2029.i, label %postload2030.i

preload2029.i:                                    ; preds = %postload2027.i
  %"&(pSB[currWI].offset)3185.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3186.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3185.i"
  %CastToValueType3187.i = bitcast i8* %"&pSB[currWI].offset3186.i" to i64*
  %loadedValue3188.i = load i64* %CastToValueType3187.i, align 8
  %.sum2866.i = add i64 %loadedValue3188.i, 11
  %186 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2866.i
  %exData773.i = extractelement <16 x float> %add36356.i, i32 11
  store float %exData773.i, float addrspace(3)* %186, align 4
  br label %postload2030.i

postload2030.i:                                   ; preds = %preload2029.i, %postload2027.i
  br i1 %exmask657.i, label %preload2032.i, label %postload2033.i

preload2032.i:                                    ; preds = %postload2030.i
  %"&(pSB[currWI].offset)3190.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3191.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3190.i"
  %CastToValueType3192.i = bitcast i8* %"&pSB[currWI].offset3191.i" to i64*
  %loadedValue3193.i = load i64* %CastToValueType3192.i, align 8
  %.sum2865.i = add i64 %loadedValue3193.i, 12
  %187 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2865.i
  %exData776.i = extractelement <16 x float> %add36356.i, i32 12
  store float %exData776.i, float addrspace(3)* %187, align 4
  br label %postload2033.i

postload2033.i:                                   ; preds = %preload2032.i, %postload2030.i
  br i1 %exmask661.i, label %preload2035.i, label %postload2036.i

preload2035.i:                                    ; preds = %postload2033.i
  %"&(pSB[currWI].offset)3195.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3196.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3195.i"
  %CastToValueType3197.i = bitcast i8* %"&pSB[currWI].offset3196.i" to i64*
  %loadedValue3198.i = load i64* %CastToValueType3197.i, align 8
  %.sum2864.i = add i64 %loadedValue3198.i, 13
  %188 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2864.i
  %exData779.i = extractelement <16 x float> %add36356.i, i32 13
  store float %exData779.i, float addrspace(3)* %188, align 4
  br label %postload2036.i

postload2036.i:                                   ; preds = %preload2035.i, %postload2033.i
  br i1 %exmask665.i, label %preload2091.i, label %postload2092.i

preload2091.i:                                    ; preds = %postload2036.i
  %"&(pSB[currWI].offset)3200.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3201.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3200.i"
  %CastToValueType3202.i = bitcast i8* %"&pSB[currWI].offset3201.i" to i64*
  %loadedValue3203.i = load i64* %CastToValueType3202.i, align 8
  %.sum2863.i = add i64 %loadedValue3203.i, 14
  %189 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2863.i
  %exData782.i = extractelement <16 x float> %add36356.i, i32 14
  store float %exData782.i, float addrspace(3)* %189, align 4
  br label %postload2092.i

postload2092.i:                                   ; preds = %preload2091.i, %postload2036.i
  br i1 %exmask669.i, label %preload2094.i, label %postload2092.i.if.end.i_crit_edge

postload2092.i.if.end.i_crit_edge:                ; preds = %postload2092.i
  br label %if.end.i

preload2094.i:                                    ; preds = %postload2092.i
  %"&(pSB[currWI].offset)3205.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3206.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3205.i"
  %CastToValueType3207.i = bitcast i8* %"&pSB[currWI].offset3206.i" to i64*
  %loadedValue3208.i = load i64* %CastToValueType3207.i, align 8
  %.sum2862.i = add i64 %loadedValue3208.i, 15
  %190 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2862.i
  %exData785.i = extractelement <16 x float> %add36356.i, i32 15
  store float %exData785.i, float addrspace(3)* %190, align 4
  br label %if.end.i

if.end.i:                                         ; preds = %postload2092.i.if.end.i_crit_edge, %preload2094.i
  %"&(pSB[currWI].offset)4017.i" = add nuw i64 %CurrSBIndex..2.i, 98
  %"&pSB[currWI].offset4018.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4017.i"
  %CastToValueType4019.i = bitcast i8* %"&pSB[currWI].offset4018.i" to i1*
  %loadedValue4020.i = load i1* %CastToValueType4019.i, align 1
  br i1 %loadedValue4020.i, label %preload1677.i, label %if.end.i.postload1678.i_crit_edge

if.end.i.postload1678.i_crit_edge:                ; preds = %if.end.i
  br label %postload1678.i

preload1677.i:                                    ; preds = %if.end.i
  %check.WI.iter4575.i = icmp ult i64 %CurrWI..2.i, %31
  br i1 %check.WI.iter4575.i, label %thenBB4572.i, label %preload1677.i.postload1678.i_crit_edge

preload1677.i.postload1678.i_crit_edge:           ; preds = %preload1677.i
  br label %postload1678.i

thenBB4572.i:                                     ; preds = %preload1677.i
  %"CurrWI++4576.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride4578.i" = add nuw i64 %CurrSBIndex..2.i, 256
  %cond.i = icmp eq i32 %currBarrier.2.i, 14
  br i1 %cond.i, label %SyncBB4569.outer.i, label %thenBB4572.i.postload1676.i_crit_edge

thenBB4572.i.postload1676.i_crit_edge:            ; preds = %thenBB4572.i
  br label %postload1676.i

postload1678.i:                                   ; preds = %thenBB4611.i.postload1678.i_crit_edge, %thenBB4603.i.postload1678.i_crit_edge, %thenBB4595.i.postload1678.i_crit_edge, %thenBB4587.i.postload1678.i_crit_edge, %thenBB4579.i.postload1678.i_crit_edge, %preload1677.i.postload1678.i_crit_edge, %if.end.i.postload1678.i_crit_edge
  %CurrWI..4.i = phi i64 [ %CurrWI..2.i, %if.end.i.postload1678.i_crit_edge ], [ 0, %preload1677.i.postload1678.i_crit_edge ], [ %"CurrWI++4583.i", %thenBB4579.i.postload1678.i_crit_edge ], [ %"CurrWI++4591.i", %thenBB4587.i.postload1678.i_crit_edge ], [ %"CurrWI++4599.i", %thenBB4595.i.postload1678.i_crit_edge ], [ %"CurrWI++4607.i", %thenBB4603.i.postload1678.i_crit_edge ], [ %"CurrWI++4615.i", %thenBB4611.i.postload1678.i_crit_edge ]
  %CurrSBIndex..4.i = phi i64 [ %CurrSBIndex..2.i, %if.end.i.postload1678.i_crit_edge ], [ 0, %preload1677.i.postload1678.i_crit_edge ], [ %"loadedCurrSB+Stride4585.i", %thenBB4579.i.postload1678.i_crit_edge ], [ %"loadedCurrSB+Stride4593.i", %thenBB4587.i.postload1678.i_crit_edge ], [ %"loadedCurrSB+Stride4601.i", %thenBB4595.i.postload1678.i_crit_edge ], [ %"loadedCurrSB+Stride4609.i", %thenBB4603.i.postload1678.i_crit_edge ], [ %"loadedCurrSB+Stride4617.i", %thenBB4611.i.postload1678.i_crit_edge ]
  %currBarrier.4.i = phi i32 [ %currBarrier.2.i, %if.end.i.postload1678.i_crit_edge ], [ 7, %preload1677.i.postload1678.i_crit_edge ], [ %currBarrier.4.i, %thenBB4579.i.postload1678.i_crit_edge ], [ %currBarrier.6.i, %thenBB4587.i.postload1678.i_crit_edge ], [ %currBarrier.8.i, %thenBB4595.i.postload1678.i_crit_edge ], [ %currBarrier.10.i, %thenBB4603.i.postload1678.i_crit_edge ], [ %currBarrier.12.i, %thenBB4611.i.postload1678.i_crit_edge ]
  %"&pSB[currWI].offset2938.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..4.i
  %CastToValueType2939.i = bitcast i8* %"&pSB[currWI].offset2938.i" to <16 x i32>*
  %loadedValue2940.i = load <16 x i32>* %CastToValueType2939.i, align 64
  %cmp37.i = icmp ult <16 x i32> %loadedValue2940.i, <i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8>
  %"&(pSB[currWI].offset)3968.i" = add nuw i64 %CurrSBIndex..4.i, 96
  %"&pSB[currWI].offset3969.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3968.i"
  %CastToValueType3970.i = bitcast i8* %"&pSB[currWI].offset3969.i" to <16 x i1>*
  %loadedValue3971.i = load <16 x i1>* %CastToValueType3970.i, align 16
  %if.end_to_if.then39358.i = and <16 x i1> %loadedValue3971.i, %cmp37.i
  %"&(pSB[currWI].offset)2971.i" = add nuw i64 %CurrSBIndex..4.i, 64
  %"&pSB[currWI].offset2972.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)2971.i"
  %CastToValueType2973.i = bitcast i8* %"&pSB[currWI].offset2972.i" to i32*
  %loadedValue2974.i = load i32* %CastToValueType2973.i, align 4
  %191 = add i32 %loadedValue2974.i, 8
  %extract361.i = sext i32 %191 to i64
  %exmask788.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 0
  br i1 %exmask788.i, label %preload2097.i, label %postload2098.i

preload2097.i:                                    ; preds = %postload1678.i
  %192 = getelementptr inbounds [128 x float] addrspace(3)* %35, i64 0, i64 %extract361.i
  %vload789.i = load float addrspace(3)* %192, align 4
  br label %postload2098.i

postload2098.i:                                   ; preds = %preload2097.i, %postload1678.i
  %phi2099.i = phi float [ undef, %postload1678.i ], [ %vload789.i, %preload2097.i ]
  %vpack790.i = insertelement <16 x float> undef, float %phi2099.i, i32 0
  %exmask792.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 1
  br i1 %exmask792.i, label %preload2100.i, label %postload2101.i

preload2100.i:                                    ; preds = %postload2098.i
  %.sum2861.i = add i64 %extract361.i, 1
  %193 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2861.i
  %vload793.i = load float addrspace(3)* %193, align 4
  br label %postload2101.i

postload2101.i:                                   ; preds = %preload2100.i, %postload2098.i
  %phi2102.i = phi float [ undef, %postload2098.i ], [ %vload793.i, %preload2100.i ]
  %vpack794.i = insertelement <16 x float> %vpack790.i, float %phi2102.i, i32 1
  %exmask796.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 2
  br i1 %exmask796.i, label %preload2103.i, label %postload2104.i

preload2103.i:                                    ; preds = %postload2101.i
  %.sum2860.i = add i64 %extract361.i, 2
  %194 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2860.i
  %vload797.i = load float addrspace(3)* %194, align 4
  br label %postload2104.i

postload2104.i:                                   ; preds = %preload2103.i, %postload2101.i
  %phi2105.i = phi float [ undef, %postload2101.i ], [ %vload797.i, %preload2103.i ]
  %vpack798.i = insertelement <16 x float> %vpack794.i, float %phi2105.i, i32 2
  %exmask800.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 3
  br i1 %exmask800.i, label %preload2106.i, label %postload2107.i

preload2106.i:                                    ; preds = %postload2104.i
  %.sum2859.i = add i64 %extract361.i, 3
  %195 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2859.i
  %vload801.i = load float addrspace(3)* %195, align 4
  br label %postload2107.i

postload2107.i:                                   ; preds = %preload2106.i, %postload2104.i
  %phi2108.i = phi float [ undef, %postload2104.i ], [ %vload801.i, %preload2106.i ]
  %vpack802.i = insertelement <16 x float> %vpack798.i, float %phi2108.i, i32 3
  %exmask804.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 4
  br i1 %exmask804.i, label %preload2109.i, label %postload2110.i

preload2109.i:                                    ; preds = %postload2107.i
  %.sum2858.i = add i64 %extract361.i, 4
  %196 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2858.i
  %vload805.i = load float addrspace(3)* %196, align 4
  br label %postload2110.i

postload2110.i:                                   ; preds = %preload2109.i, %postload2107.i
  %phi2111.i = phi float [ undef, %postload2107.i ], [ %vload805.i, %preload2109.i ]
  %vpack806.i = insertelement <16 x float> %vpack802.i, float %phi2111.i, i32 4
  %exmask808.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 5
  br i1 %exmask808.i, label %preload2112.i, label %postload2113.i

preload2112.i:                                    ; preds = %postload2110.i
  %.sum2857.i = add i64 %extract361.i, 5
  %197 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2857.i
  %vload809.i = load float addrspace(3)* %197, align 4
  br label %postload2113.i

postload2113.i:                                   ; preds = %preload2112.i, %postload2110.i
  %phi2114.i = phi float [ undef, %postload2110.i ], [ %vload809.i, %preload2112.i ]
  %vpack810.i = insertelement <16 x float> %vpack806.i, float %phi2114.i, i32 5
  %exmask812.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 6
  br i1 %exmask812.i, label %preload2115.i, label %postload2116.i

preload2115.i:                                    ; preds = %postload2113.i
  %.sum2856.i = add i64 %extract361.i, 6
  %198 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2856.i
  %vload813.i = load float addrspace(3)* %198, align 4
  br label %postload2116.i

postload2116.i:                                   ; preds = %preload2115.i, %postload2113.i
  %phi2117.i = phi float [ undef, %postload2113.i ], [ %vload813.i, %preload2115.i ]
  %vpack814.i = insertelement <16 x float> %vpack810.i, float %phi2117.i, i32 6
  %exmask816.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 7
  br i1 %exmask816.i, label %preload2118.i, label %postload2119.i

preload2118.i:                                    ; preds = %postload2116.i
  %.sum2855.i = add i64 %extract361.i, 7
  %199 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2855.i
  %vload817.i = load float addrspace(3)* %199, align 4
  br label %postload2119.i

postload2119.i:                                   ; preds = %preload2118.i, %postload2116.i
  %phi2120.i = phi float [ undef, %postload2116.i ], [ %vload817.i, %preload2118.i ]
  %vpack818.i = insertelement <16 x float> %vpack814.i, float %phi2120.i, i32 7
  %exmask820.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 8
  br i1 %exmask820.i, label %preload2121.i, label %postload2122.i

preload2121.i:                                    ; preds = %postload2119.i
  %.sum2854.i = add i64 %extract361.i, 8
  %200 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2854.i
  %vload821.i = load float addrspace(3)* %200, align 4
  br label %postload2122.i

postload2122.i:                                   ; preds = %preload2121.i, %postload2119.i
  %phi2123.i = phi float [ undef, %postload2119.i ], [ %vload821.i, %preload2121.i ]
  %vpack822.i = insertelement <16 x float> %vpack818.i, float %phi2123.i, i32 8
  %exmask824.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 9
  br i1 %exmask824.i, label %preload2124.i, label %postload2125.i

preload2124.i:                                    ; preds = %postload2122.i
  %.sum2853.i = add i64 %extract361.i, 9
  %201 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2853.i
  %vload825.i = load float addrspace(3)* %201, align 4
  br label %postload2125.i

postload2125.i:                                   ; preds = %preload2124.i, %postload2122.i
  %phi2126.i = phi float [ undef, %postload2122.i ], [ %vload825.i, %preload2124.i ]
  %vpack826.i = insertelement <16 x float> %vpack822.i, float %phi2126.i, i32 9
  %exmask828.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 10
  br i1 %exmask828.i, label %preload2127.i, label %postload2128.i

preload2127.i:                                    ; preds = %postload2125.i
  %.sum2852.i = add i64 %extract361.i, 10
  %202 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2852.i
  %vload829.i = load float addrspace(3)* %202, align 4
  br label %postload2128.i

postload2128.i:                                   ; preds = %preload2127.i, %postload2125.i
  %phi2129.i = phi float [ undef, %postload2125.i ], [ %vload829.i, %preload2127.i ]
  %vpack830.i = insertelement <16 x float> %vpack826.i, float %phi2129.i, i32 10
  %exmask832.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 11
  br i1 %exmask832.i, label %preload2130.i, label %postload2131.i

preload2130.i:                                    ; preds = %postload2128.i
  %.sum2851.i = add i64 %extract361.i, 11
  %203 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2851.i
  %vload833.i = load float addrspace(3)* %203, align 4
  br label %postload2131.i

postload2131.i:                                   ; preds = %preload2130.i, %postload2128.i
  %phi2132.i = phi float [ undef, %postload2128.i ], [ %vload833.i, %preload2130.i ]
  %vpack834.i = insertelement <16 x float> %vpack830.i, float %phi2132.i, i32 11
  %exmask836.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 12
  br i1 %exmask836.i, label %preload2133.i, label %postload2134.i

preload2133.i:                                    ; preds = %postload2131.i
  %.sum2850.i = add i64 %extract361.i, 12
  %204 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2850.i
  %vload837.i = load float addrspace(3)* %204, align 4
  br label %postload2134.i

postload2134.i:                                   ; preds = %preload2133.i, %postload2131.i
  %phi2135.i = phi float [ undef, %postload2131.i ], [ %vload837.i, %preload2133.i ]
  %vpack838.i = insertelement <16 x float> %vpack834.i, float %phi2135.i, i32 12
  %exmask840.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 13
  br i1 %exmask840.i, label %preload2136.i, label %postload2137.i

preload2136.i:                                    ; preds = %postload2134.i
  %.sum2849.i = add i64 %extract361.i, 13
  %205 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2849.i
  %vload841.i = load float addrspace(3)* %205, align 4
  br label %postload2137.i

postload2137.i:                                   ; preds = %preload2136.i, %postload2134.i
  %phi2138.i = phi float [ undef, %postload2134.i ], [ %vload841.i, %preload2136.i ]
  %vpack842.i = insertelement <16 x float> %vpack838.i, float %phi2138.i, i32 13
  %exmask844.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 14
  br i1 %exmask844.i, label %preload2139.i, label %postload2140.i

preload2139.i:                                    ; preds = %postload2137.i
  %.sum2848.i = add i64 %extract361.i, 14
  %206 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2848.i
  %vload845.i = load float addrspace(3)* %206, align 4
  br label %postload2140.i

postload2140.i:                                   ; preds = %preload2139.i, %postload2137.i
  %phi2141.i = phi float [ undef, %postload2137.i ], [ %vload845.i, %preload2139.i ]
  %vpack846.i = insertelement <16 x float> %vpack842.i, float %phi2141.i, i32 14
  %exmask848.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 15
  br i1 %exmask848.i, label %preload2142.i, label %postload2143.i

preload2142.i:                                    ; preds = %postload2140.i
  %.sum2847.i = add i64 %extract361.i, 15
  %207 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2847.i
  %vload849.i = load float addrspace(3)* %207, align 4
  br label %postload2143.i

postload2143.i:                                   ; preds = %preload2142.i, %postload2140.i
  %phi2144.i = phi float [ undef, %postload2140.i ], [ %vload849.i, %preload2142.i ]
  %vpack850.i = insertelement <16 x float> %vpack846.i, float %phi2144.i, i32 15
  br i1 %exmask788.i, label %preload2145.i, label %postload2146.i

preload2145.i:                                    ; preds = %postload2143.i
  %"&(pSB[currWI].offset)3904.i" = add nuw i64 %CurrSBIndex..4.i, 80
  %"&pSB[currWI].offset3905.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3904.i"
  %CastToValueType3906.i = bitcast i8* %"&pSB[currWI].offset3905.i" to float addrspace(3)**
  %loadedValue3907.i = load float addrspace(3)** %CastToValueType3906.i, align 8
  %vload854.i = load float addrspace(3)* %loadedValue3907.i, align 4
  br label %postload2146.i

postload2146.i:                                   ; preds = %preload2145.i, %postload2143.i
  %phi2147.i = phi float [ undef, %postload2143.i ], [ %vload854.i, %preload2145.i ]
  %vpack855.i = insertelement <16 x float> undef, float %phi2147.i, i32 0
  br i1 %exmask792.i, label %preload2148.i, label %postload2149.i

preload2148.i:                                    ; preds = %postload2146.i
  %"&(pSB[currWI].offset)3210.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3211.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3210.i"
  %CastToValueType3212.i = bitcast i8* %"&pSB[currWI].offset3211.i" to i64*
  %loadedValue3213.i = load i64* %CastToValueType3212.i, align 8
  %.sum2846.i = add i64 %loadedValue3213.i, 1
  %208 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2846.i
  %vload858.i = load float addrspace(3)* %208, align 4
  br label %postload2149.i

postload2149.i:                                   ; preds = %preload2148.i, %postload2146.i
  %phi2150.i = phi float [ undef, %postload2146.i ], [ %vload858.i, %preload2148.i ]
  %vpack859.i = insertelement <16 x float> %vpack855.i, float %phi2150.i, i32 1
  br i1 %exmask796.i, label %preload2151.i, label %postload2152.i

preload2151.i:                                    ; preds = %postload2149.i
  %"&(pSB[currWI].offset)3215.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3216.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3215.i"
  %CastToValueType3217.i = bitcast i8* %"&pSB[currWI].offset3216.i" to i64*
  %loadedValue3218.i = load i64* %CastToValueType3217.i, align 8
  %.sum2845.i = add i64 %loadedValue3218.i, 2
  %209 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2845.i
  %vload862.i = load float addrspace(3)* %209, align 4
  br label %postload2152.i

postload2152.i:                                   ; preds = %preload2151.i, %postload2149.i
  %phi2153.i = phi float [ undef, %postload2149.i ], [ %vload862.i, %preload2151.i ]
  %vpack863.i = insertelement <16 x float> %vpack859.i, float %phi2153.i, i32 2
  br i1 %exmask800.i, label %preload2154.i, label %postload2155.i

preload2154.i:                                    ; preds = %postload2152.i
  %"&(pSB[currWI].offset)3220.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3221.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3220.i"
  %CastToValueType3222.i = bitcast i8* %"&pSB[currWI].offset3221.i" to i64*
  %loadedValue3223.i = load i64* %CastToValueType3222.i, align 8
  %.sum2844.i = add i64 %loadedValue3223.i, 3
  %210 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2844.i
  %vload866.i = load float addrspace(3)* %210, align 4
  br label %postload2155.i

postload2155.i:                                   ; preds = %preload2154.i, %postload2152.i
  %phi2156.i = phi float [ undef, %postload2152.i ], [ %vload866.i, %preload2154.i ]
  %vpack867.i = insertelement <16 x float> %vpack863.i, float %phi2156.i, i32 3
  br i1 %exmask804.i, label %preload2157.i, label %postload2158.i

preload2157.i:                                    ; preds = %postload2155.i
  %"&(pSB[currWI].offset)3225.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3226.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3225.i"
  %CastToValueType3227.i = bitcast i8* %"&pSB[currWI].offset3226.i" to i64*
  %loadedValue3228.i = load i64* %CastToValueType3227.i, align 8
  %.sum2843.i = add i64 %loadedValue3228.i, 4
  %211 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2843.i
  %vload870.i = load float addrspace(3)* %211, align 4
  br label %postload2158.i

postload2158.i:                                   ; preds = %preload2157.i, %postload2155.i
  %phi2159.i = phi float [ undef, %postload2155.i ], [ %vload870.i, %preload2157.i ]
  %vpack871.i = insertelement <16 x float> %vpack867.i, float %phi2159.i, i32 4
  br i1 %exmask808.i, label %preload2160.i, label %postload2161.i

preload2160.i:                                    ; preds = %postload2158.i
  %"&(pSB[currWI].offset)3230.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3231.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3230.i"
  %CastToValueType3232.i = bitcast i8* %"&pSB[currWI].offset3231.i" to i64*
  %loadedValue3233.i = load i64* %CastToValueType3232.i, align 8
  %.sum2842.i = add i64 %loadedValue3233.i, 5
  %212 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2842.i
  %vload874.i = load float addrspace(3)* %212, align 4
  br label %postload2161.i

postload2161.i:                                   ; preds = %preload2160.i, %postload2158.i
  %phi2162.i = phi float [ undef, %postload2158.i ], [ %vload874.i, %preload2160.i ]
  %vpack875.i = insertelement <16 x float> %vpack871.i, float %phi2162.i, i32 5
  br i1 %exmask812.i, label %preload2163.i, label %postload2164.i

preload2163.i:                                    ; preds = %postload2161.i
  %"&(pSB[currWI].offset)3235.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3236.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3235.i"
  %CastToValueType3237.i = bitcast i8* %"&pSB[currWI].offset3236.i" to i64*
  %loadedValue3238.i = load i64* %CastToValueType3237.i, align 8
  %.sum2841.i = add i64 %loadedValue3238.i, 6
  %213 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2841.i
  %vload878.i = load float addrspace(3)* %213, align 4
  br label %postload2164.i

postload2164.i:                                   ; preds = %preload2163.i, %postload2161.i
  %phi2165.i = phi float [ undef, %postload2161.i ], [ %vload878.i, %preload2163.i ]
  %vpack879.i = insertelement <16 x float> %vpack875.i, float %phi2165.i, i32 6
  br i1 %exmask816.i, label %preload2166.i, label %postload2167.i

preload2166.i:                                    ; preds = %postload2164.i
  %"&(pSB[currWI].offset)3240.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3241.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3240.i"
  %CastToValueType3242.i = bitcast i8* %"&pSB[currWI].offset3241.i" to i64*
  %loadedValue3243.i = load i64* %CastToValueType3242.i, align 8
  %.sum2840.i = add i64 %loadedValue3243.i, 7
  %214 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2840.i
  %vload882.i = load float addrspace(3)* %214, align 4
  br label %postload2167.i

postload2167.i:                                   ; preds = %preload2166.i, %postload2164.i
  %phi2168.i = phi float [ undef, %postload2164.i ], [ %vload882.i, %preload2166.i ]
  %vpack883.i = insertelement <16 x float> %vpack879.i, float %phi2168.i, i32 7
  br i1 %exmask820.i, label %preload2169.i, label %postload2170.i

preload2169.i:                                    ; preds = %postload2167.i
  %"&(pSB[currWI].offset)3245.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3246.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3245.i"
  %CastToValueType3247.i = bitcast i8* %"&pSB[currWI].offset3246.i" to i64*
  %loadedValue3248.i = load i64* %CastToValueType3247.i, align 8
  %.sum2839.i = add i64 %loadedValue3248.i, 8
  %215 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2839.i
  %vload886.i = load float addrspace(3)* %215, align 4
  br label %postload2170.i

postload2170.i:                                   ; preds = %preload2169.i, %postload2167.i
  %phi2171.i = phi float [ undef, %postload2167.i ], [ %vload886.i, %preload2169.i ]
  %vpack887.i = insertelement <16 x float> %vpack883.i, float %phi2171.i, i32 8
  br i1 %exmask824.i, label %preload2172.i, label %postload2173.i

preload2172.i:                                    ; preds = %postload2170.i
  %"&(pSB[currWI].offset)3250.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3251.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3250.i"
  %CastToValueType3252.i = bitcast i8* %"&pSB[currWI].offset3251.i" to i64*
  %loadedValue3253.i = load i64* %CastToValueType3252.i, align 8
  %.sum2838.i = add i64 %loadedValue3253.i, 9
  %216 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2838.i
  %vload890.i = load float addrspace(3)* %216, align 4
  br label %postload2173.i

postload2173.i:                                   ; preds = %preload2172.i, %postload2170.i
  %phi2174.i = phi float [ undef, %postload2170.i ], [ %vload890.i, %preload2172.i ]
  %vpack891.i = insertelement <16 x float> %vpack887.i, float %phi2174.i, i32 9
  br i1 %exmask828.i, label %preload2175.i, label %postload2176.i

preload2175.i:                                    ; preds = %postload2173.i
  %"&(pSB[currWI].offset)3255.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3256.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3255.i"
  %CastToValueType3257.i = bitcast i8* %"&pSB[currWI].offset3256.i" to i64*
  %loadedValue3258.i = load i64* %CastToValueType3257.i, align 8
  %.sum2837.i = add i64 %loadedValue3258.i, 10
  %217 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2837.i
  %vload894.i = load float addrspace(3)* %217, align 4
  br label %postload2176.i

postload2176.i:                                   ; preds = %preload2175.i, %postload2173.i
  %phi2177.i = phi float [ undef, %postload2173.i ], [ %vload894.i, %preload2175.i ]
  %vpack895.i = insertelement <16 x float> %vpack891.i, float %phi2177.i, i32 10
  br i1 %exmask832.i, label %preload1921.i, label %postload1922.i

preload1921.i:                                    ; preds = %postload2176.i
  %"&(pSB[currWI].offset)3260.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3261.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3260.i"
  %CastToValueType3262.i = bitcast i8* %"&pSB[currWI].offset3261.i" to i64*
  %loadedValue3263.i = load i64* %CastToValueType3262.i, align 8
  %.sum2836.i = add i64 %loadedValue3263.i, 11
  %218 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2836.i
  %vload898.i = load float addrspace(3)* %218, align 4
  br label %postload1922.i

postload1922.i:                                   ; preds = %preload1921.i, %postload2176.i
  %phi1923.i = phi float [ undef, %postload2176.i ], [ %vload898.i, %preload1921.i ]
  %vpack899.i = insertelement <16 x float> %vpack895.i, float %phi1923.i, i32 11
  br i1 %exmask836.i, label %preload1924.i, label %postload1925.i

preload1924.i:                                    ; preds = %postload1922.i
  %"&(pSB[currWI].offset)3265.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3266.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3265.i"
  %CastToValueType3267.i = bitcast i8* %"&pSB[currWI].offset3266.i" to i64*
  %loadedValue3268.i = load i64* %CastToValueType3267.i, align 8
  %.sum2835.i = add i64 %loadedValue3268.i, 12
  %219 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2835.i
  %vload902.i = load float addrspace(3)* %219, align 4
  br label %postload1925.i

postload1925.i:                                   ; preds = %preload1924.i, %postload1922.i
  %phi1926.i = phi float [ undef, %postload1922.i ], [ %vload902.i, %preload1924.i ]
  %vpack903.i = insertelement <16 x float> %vpack899.i, float %phi1926.i, i32 12
  br i1 %exmask840.i, label %preload1927.i, label %postload1928.i

preload1927.i:                                    ; preds = %postload1925.i
  %"&(pSB[currWI].offset)3270.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3271.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3270.i"
  %CastToValueType3272.i = bitcast i8* %"&pSB[currWI].offset3271.i" to i64*
  %loadedValue3273.i = load i64* %CastToValueType3272.i, align 8
  %.sum2834.i = add i64 %loadedValue3273.i, 13
  %220 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2834.i
  %vload906.i = load float addrspace(3)* %220, align 4
  br label %postload1928.i

postload1928.i:                                   ; preds = %preload1927.i, %postload1925.i
  %phi1929.i = phi float [ undef, %postload1925.i ], [ %vload906.i, %preload1927.i ]
  %vpack907.i = insertelement <16 x float> %vpack903.i, float %phi1929.i, i32 13
  br i1 %exmask844.i, label %preload1930.i, label %postload1931.i

preload1930.i:                                    ; preds = %postload1928.i
  %"&(pSB[currWI].offset)3275.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3276.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3275.i"
  %CastToValueType3277.i = bitcast i8* %"&pSB[currWI].offset3276.i" to i64*
  %loadedValue3278.i = load i64* %CastToValueType3277.i, align 8
  %.sum2833.i = add i64 %loadedValue3278.i, 14
  %221 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2833.i
  %vload910.i = load float addrspace(3)* %221, align 4
  br label %postload1931.i

postload1931.i:                                   ; preds = %preload1930.i, %postload1928.i
  %phi1932.i = phi float [ undef, %postload1928.i ], [ %vload910.i, %preload1930.i ]
  %vpack911.i = insertelement <16 x float> %vpack907.i, float %phi1932.i, i32 14
  br i1 %exmask848.i, label %preload1933.i, label %postload1934.i

preload1933.i:                                    ; preds = %postload1931.i
  %"&(pSB[currWI].offset)3280.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3281.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3280.i"
  %CastToValueType3282.i = bitcast i8* %"&pSB[currWI].offset3281.i" to i64*
  %loadedValue3283.i = load i64* %CastToValueType3282.i, align 8
  %.sum2832.i = add i64 %loadedValue3283.i, 15
  %222 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2832.i
  %vload914.i = load float addrspace(3)* %222, align 4
  br label %postload1934.i

postload1934.i:                                   ; preds = %preload1933.i, %postload1931.i
  %phi1935.i = phi float [ undef, %postload1931.i ], [ %vload914.i, %preload1933.i ]
  %vpack915.i = insertelement <16 x float> %vpack911.i, float %phi1935.i, i32 15
  %add45379.i = fadd <16 x float> %vpack915.i, %vpack850.i
  br i1 %exmask788.i, label %preload1936.i, label %postload1937.i

preload1936.i:                                    ; preds = %postload1934.i
  %exData919.i = extractelement <16 x float> %add45379.i, i32 0
  %"&(pSB[currWI].offset)3909.i" = add nuw i64 %CurrSBIndex..4.i, 80
  %"&pSB[currWI].offset3910.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3909.i"
  %CastToValueType3911.i = bitcast i8* %"&pSB[currWI].offset3910.i" to float addrspace(3)**
  %loadedValue3912.i = load float addrspace(3)** %CastToValueType3911.i, align 8
  store float %exData919.i, float addrspace(3)* %loadedValue3912.i, align 4
  br label %postload1937.i

postload1937.i:                                   ; preds = %preload1936.i, %postload1934.i
  br i1 %exmask792.i, label %preload1939.i, label %postload1940.i

preload1939.i:                                    ; preds = %postload1937.i
  %"&(pSB[currWI].offset)3285.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3286.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3285.i"
  %CastToValueType3287.i = bitcast i8* %"&pSB[currWI].offset3286.i" to i64*
  %loadedValue3288.i = load i64* %CastToValueType3287.i, align 8
  %.sum2831.i = add i64 %loadedValue3288.i, 1
  %223 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2831.i
  %exData922.i = extractelement <16 x float> %add45379.i, i32 1
  store float %exData922.i, float addrspace(3)* %223, align 4
  br label %postload1940.i

postload1940.i:                                   ; preds = %preload1939.i, %postload1937.i
  br i1 %exmask796.i, label %preload1942.i, label %postload1943.i

preload1942.i:                                    ; preds = %postload1940.i
  %"&(pSB[currWI].offset)3290.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3291.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3290.i"
  %CastToValueType3292.i = bitcast i8* %"&pSB[currWI].offset3291.i" to i64*
  %loadedValue3293.i = load i64* %CastToValueType3292.i, align 8
  %.sum2830.i = add i64 %loadedValue3293.i, 2
  %224 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2830.i
  %exData925.i = extractelement <16 x float> %add45379.i, i32 2
  store float %exData925.i, float addrspace(3)* %224, align 4
  br label %postload1943.i

postload1943.i:                                   ; preds = %preload1942.i, %postload1940.i
  br i1 %exmask800.i, label %preload1945.i, label %postload1946.i

preload1945.i:                                    ; preds = %postload1943.i
  %"&(pSB[currWI].offset)3295.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3296.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3295.i"
  %CastToValueType3297.i = bitcast i8* %"&pSB[currWI].offset3296.i" to i64*
  %loadedValue3298.i = load i64* %CastToValueType3297.i, align 8
  %.sum2829.i = add i64 %loadedValue3298.i, 3
  %225 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2829.i
  %exData928.i = extractelement <16 x float> %add45379.i, i32 3
  store float %exData928.i, float addrspace(3)* %225, align 4
  br label %postload1946.i

postload1946.i:                                   ; preds = %preload1945.i, %postload1943.i
  br i1 %exmask804.i, label %preload1948.i, label %postload1949.i

preload1948.i:                                    ; preds = %postload1946.i
  %"&(pSB[currWI].offset)3300.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3301.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3300.i"
  %CastToValueType3302.i = bitcast i8* %"&pSB[currWI].offset3301.i" to i64*
  %loadedValue3303.i = load i64* %CastToValueType3302.i, align 8
  %.sum2828.i = add i64 %loadedValue3303.i, 4
  %226 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2828.i
  %exData931.i = extractelement <16 x float> %add45379.i, i32 4
  store float %exData931.i, float addrspace(3)* %226, align 4
  br label %postload1949.i

postload1949.i:                                   ; preds = %preload1948.i, %postload1946.i
  br i1 %exmask808.i, label %preload1951.i, label %postload1952.i

preload1951.i:                                    ; preds = %postload1949.i
  %"&(pSB[currWI].offset)3305.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3306.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3305.i"
  %CastToValueType3307.i = bitcast i8* %"&pSB[currWI].offset3306.i" to i64*
  %loadedValue3308.i = load i64* %CastToValueType3307.i, align 8
  %.sum2827.i = add i64 %loadedValue3308.i, 5
  %227 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2827.i
  %exData934.i = extractelement <16 x float> %add45379.i, i32 5
  store float %exData934.i, float addrspace(3)* %227, align 4
  br label %postload1952.i

postload1952.i:                                   ; preds = %preload1951.i, %postload1949.i
  br i1 %exmask812.i, label %preload1954.i, label %postload1955.i

preload1954.i:                                    ; preds = %postload1952.i
  %"&(pSB[currWI].offset)3310.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3311.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3310.i"
  %CastToValueType3312.i = bitcast i8* %"&pSB[currWI].offset3311.i" to i64*
  %loadedValue3313.i = load i64* %CastToValueType3312.i, align 8
  %.sum2826.i = add i64 %loadedValue3313.i, 6
  %228 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2826.i
  %exData937.i = extractelement <16 x float> %add45379.i, i32 6
  store float %exData937.i, float addrspace(3)* %228, align 4
  br label %postload1955.i

postload1955.i:                                   ; preds = %preload1954.i, %postload1952.i
  br i1 %exmask816.i, label %preload1957.i, label %postload1958.i

preload1957.i:                                    ; preds = %postload1955.i
  %"&(pSB[currWI].offset)3315.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3316.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3315.i"
  %CastToValueType3317.i = bitcast i8* %"&pSB[currWI].offset3316.i" to i64*
  %loadedValue3318.i = load i64* %CastToValueType3317.i, align 8
  %.sum2825.i = add i64 %loadedValue3318.i, 7
  %229 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2825.i
  %exData940.i = extractelement <16 x float> %add45379.i, i32 7
  store float %exData940.i, float addrspace(3)* %229, align 4
  br label %postload1958.i

postload1958.i:                                   ; preds = %preload1957.i, %postload1955.i
  br i1 %exmask820.i, label %preload1960.i, label %postload1961.i

preload1960.i:                                    ; preds = %postload1958.i
  %"&(pSB[currWI].offset)3320.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3321.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3320.i"
  %CastToValueType3322.i = bitcast i8* %"&pSB[currWI].offset3321.i" to i64*
  %loadedValue3323.i = load i64* %CastToValueType3322.i, align 8
  %.sum2824.i = add i64 %loadedValue3323.i, 8
  %230 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2824.i
  %exData943.i = extractelement <16 x float> %add45379.i, i32 8
  store float %exData943.i, float addrspace(3)* %230, align 4
  br label %postload1961.i

postload1961.i:                                   ; preds = %preload1960.i, %postload1958.i
  br i1 %exmask824.i, label %preload1963.i, label %postload1964.i

preload1963.i:                                    ; preds = %postload1961.i
  %"&(pSB[currWI].offset)3325.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3326.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3325.i"
  %CastToValueType3327.i = bitcast i8* %"&pSB[currWI].offset3326.i" to i64*
  %loadedValue3328.i = load i64* %CastToValueType3327.i, align 8
  %.sum2823.i = add i64 %loadedValue3328.i, 9
  %231 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2823.i
  %exData946.i = extractelement <16 x float> %add45379.i, i32 9
  store float %exData946.i, float addrspace(3)* %231, align 4
  br label %postload1964.i

postload1964.i:                                   ; preds = %preload1963.i, %postload1961.i
  br i1 %exmask828.i, label %preload1966.i, label %postload1967.i

preload1966.i:                                    ; preds = %postload1964.i
  %"&(pSB[currWI].offset)3330.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3331.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3330.i"
  %CastToValueType3332.i = bitcast i8* %"&pSB[currWI].offset3331.i" to i64*
  %loadedValue3333.i = load i64* %CastToValueType3332.i, align 8
  %.sum2822.i = add i64 %loadedValue3333.i, 10
  %232 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2822.i
  %exData949.i = extractelement <16 x float> %add45379.i, i32 10
  store float %exData949.i, float addrspace(3)* %232, align 4
  br label %postload1967.i

postload1967.i:                                   ; preds = %preload1966.i, %postload1964.i
  br i1 %exmask832.i, label %preload1969.i, label %postload1970.i

preload1969.i:                                    ; preds = %postload1967.i
  %"&(pSB[currWI].offset)3335.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3336.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3335.i"
  %CastToValueType3337.i = bitcast i8* %"&pSB[currWI].offset3336.i" to i64*
  %loadedValue3338.i = load i64* %CastToValueType3337.i, align 8
  %.sum2821.i = add i64 %loadedValue3338.i, 11
  %233 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2821.i
  %exData952.i = extractelement <16 x float> %add45379.i, i32 11
  store float %exData952.i, float addrspace(3)* %233, align 4
  br label %postload1970.i

postload1970.i:                                   ; preds = %preload1969.i, %postload1967.i
  br i1 %exmask836.i, label %preload1972.i, label %postload1973.i

preload1972.i:                                    ; preds = %postload1970.i
  %"&(pSB[currWI].offset)3340.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3341.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3340.i"
  %CastToValueType3342.i = bitcast i8* %"&pSB[currWI].offset3341.i" to i64*
  %loadedValue3343.i = load i64* %CastToValueType3342.i, align 8
  %.sum2820.i = add i64 %loadedValue3343.i, 12
  %234 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2820.i
  %exData955.i = extractelement <16 x float> %add45379.i, i32 12
  store float %exData955.i, float addrspace(3)* %234, align 4
  br label %postload1973.i

postload1973.i:                                   ; preds = %preload1972.i, %postload1970.i
  br i1 %exmask840.i, label %preload1975.i, label %postload1976.i

preload1975.i:                                    ; preds = %postload1973.i
  %"&(pSB[currWI].offset)3345.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3346.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3345.i"
  %CastToValueType3347.i = bitcast i8* %"&pSB[currWI].offset3346.i" to i64*
  %loadedValue3348.i = load i64* %CastToValueType3347.i, align 8
  %.sum2819.i = add i64 %loadedValue3348.i, 13
  %235 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2819.i
  %exData958.i = extractelement <16 x float> %add45379.i, i32 13
  store float %exData958.i, float addrspace(3)* %235, align 4
  br label %postload1976.i

postload1976.i:                                   ; preds = %preload1975.i, %postload1973.i
  br i1 %exmask844.i, label %preload2178.i, label %postload2179.i

preload2178.i:                                    ; preds = %postload1976.i
  %"&(pSB[currWI].offset)3350.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3351.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3350.i"
  %CastToValueType3352.i = bitcast i8* %"&pSB[currWI].offset3351.i" to i64*
  %loadedValue3353.i = load i64* %CastToValueType3352.i, align 8
  %.sum2818.i = add i64 %loadedValue3353.i, 14
  %236 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2818.i
  %exData961.i = extractelement <16 x float> %add45379.i, i32 14
  store float %exData961.i, float addrspace(3)* %236, align 4
  br label %postload2179.i

postload2179.i:                                   ; preds = %preload2178.i, %postload1976.i
  br i1 %exmask848.i, label %preload2181.i, label %postload2179.i.if.end46.i_crit_edge

postload2179.i.if.end46.i_crit_edge:              ; preds = %postload2179.i
  br label %if.end46.i

preload2181.i:                                    ; preds = %postload2179.i
  %"&(pSB[currWI].offset)3355.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3356.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3355.i"
  %CastToValueType3357.i = bitcast i8* %"&pSB[currWI].offset3356.i" to i64*
  %loadedValue3358.i = load i64* %CastToValueType3357.i, align 8
  %.sum2817.i = add i64 %loadedValue3358.i, 15
  %237 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2817.i
  %exData964.i = extractelement <16 x float> %add45379.i, i32 15
  store float %exData964.i, float addrspace(3)* %237, align 4
  br label %if.end46.i

if.end46.i:                                       ; preds = %postload2179.i.if.end46.i_crit_edge, %preload2181.i
  %"&(pSB[currWI].offset)4012.i" = add nuw i64 %CurrSBIndex..4.i, 98
  %"&pSB[currWI].offset4013.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4012.i"
  %CastToValueType4014.i = bitcast i8* %"&pSB[currWI].offset4013.i" to i1*
  %loadedValue4015.i = load i1* %CastToValueType4014.i, align 1
  br i1 %loadedValue4015.i, label %preload1679.i, label %if.end46.i.postload1680.i_crit_edge

if.end46.i.postload1680.i_crit_edge:              ; preds = %if.end46.i
  br label %postload1680.i

preload1679.i:                                    ; preds = %if.end46.i
  %check.WI.iter4582.i = icmp ult i64 %CurrWI..4.i, %31
  br i1 %check.WI.iter4582.i, label %thenBB4579.i, label %preload1679.i.postload1680.i_crit_edge

preload1679.i.postload1680.i_crit_edge:           ; preds = %preload1679.i
  br label %postload1680.i

thenBB4579.i:                                     ; preds = %preload1679.i
  %"CurrWI++4583.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride4585.i" = add nuw i64 %CurrSBIndex..4.i, 256
  switch i32 %currBarrier.4.i, label %thenBB4579.i.postload1678.i_crit_edge [
    i32 6, label %thenBB4579.i.postload1676.i_crit_edge
    i32 14, label %SyncBB4569.outer.i
  ]

thenBB4579.i.postload1676.i_crit_edge:            ; preds = %thenBB4579.i
  br label %postload1676.i

thenBB4579.i.postload1678.i_crit_edge:            ; preds = %thenBB4579.i
  br label %postload1678.i

postload1680.i:                                   ; preds = %thenBB4611.i.postload1680.i_crit_edge, %thenBB4603.i.postload1680.i_crit_edge, %thenBB4595.i.postload1680.i_crit_edge, %thenBB4587.i.postload1680.i_crit_edge, %preload1679.i.postload1680.i_crit_edge, %if.end46.i.postload1680.i_crit_edge
  %CurrWI..6.i = phi i64 [ %CurrWI..4.i, %if.end46.i.postload1680.i_crit_edge ], [ 0, %preload1679.i.postload1680.i_crit_edge ], [ %"CurrWI++4591.i", %thenBB4587.i.postload1680.i_crit_edge ], [ %"CurrWI++4599.i", %thenBB4595.i.postload1680.i_crit_edge ], [ %"CurrWI++4607.i", %thenBB4603.i.postload1680.i_crit_edge ], [ %"CurrWI++4615.i", %thenBB4611.i.postload1680.i_crit_edge ]
  %CurrSBIndex..6.i = phi i64 [ %CurrSBIndex..4.i, %if.end46.i.postload1680.i_crit_edge ], [ 0, %preload1679.i.postload1680.i_crit_edge ], [ %"loadedCurrSB+Stride4593.i", %thenBB4587.i.postload1680.i_crit_edge ], [ %"loadedCurrSB+Stride4601.i", %thenBB4595.i.postload1680.i_crit_edge ], [ %"loadedCurrSB+Stride4609.i", %thenBB4603.i.postload1680.i_crit_edge ], [ %"loadedCurrSB+Stride4617.i", %thenBB4611.i.postload1680.i_crit_edge ]
  %currBarrier.6.i = phi i32 [ %currBarrier.4.i, %if.end46.i.postload1680.i_crit_edge ], [ 8, %preload1679.i.postload1680.i_crit_edge ], [ %currBarrier.6.i, %thenBB4587.i.postload1680.i_crit_edge ], [ %currBarrier.8.i, %thenBB4595.i.postload1680.i_crit_edge ], [ %currBarrier.10.i, %thenBB4603.i.postload1680.i_crit_edge ], [ %currBarrier.12.i, %thenBB4611.i.postload1680.i_crit_edge ]
  %"&pSB[currWI].offset2933.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..6.i
  %CastToValueType2934.i = bitcast i8* %"&pSB[currWI].offset2933.i" to <16 x i32>*
  %loadedValue2935.i = load <16 x i32>* %CastToValueType2934.i, align 64
  %cmp47.i = icmp ult <16 x i32> %loadedValue2935.i, <i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4>
  %"&(pSB[currWI].offset)3973.i" = add nuw i64 %CurrSBIndex..6.i, 96
  %"&pSB[currWI].offset3974.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3973.i"
  %CastToValueType3975.i = bitcast i8* %"&pSB[currWI].offset3974.i" to <16 x i1>*
  %loadedValue3976.i = load <16 x i1>* %CastToValueType3975.i, align 16
  %if.end46_to_if.then49381.i = and <16 x i1> %loadedValue3976.i, %cmp47.i
  %"&(pSB[currWI].offset)2966.i" = add nuw i64 %CurrSBIndex..6.i, 64
  %"&pSB[currWI].offset2967.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)2966.i"
  %CastToValueType2968.i = bitcast i8* %"&pSB[currWI].offset2967.i" to i32*
  %loadedValue2969.i = load i32* %CastToValueType2968.i, align 4
  %238 = add i32 %loadedValue2969.i, 4
  %extract384.i = sext i32 %238 to i64
  %exmask967.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 0
  br i1 %exmask967.i, label %preload2184.i, label %postload2185.i

preload2184.i:                                    ; preds = %postload1680.i
  %239 = getelementptr inbounds [128 x float] addrspace(3)* %35, i64 0, i64 %extract384.i
  %vload968.i = load float addrspace(3)* %239, align 4
  br label %postload2185.i

postload2185.i:                                   ; preds = %preload2184.i, %postload1680.i
  %phi2186.i = phi float [ undef, %postload1680.i ], [ %vload968.i, %preload2184.i ]
  %vpack969.i = insertelement <16 x float> undef, float %phi2186.i, i32 0
  %exmask971.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 1
  br i1 %exmask971.i, label %preload2187.i, label %postload2188.i

preload2187.i:                                    ; preds = %postload2185.i
  %.sum2816.i = add i64 %extract384.i, 1
  %240 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2816.i
  %vload972.i = load float addrspace(3)* %240, align 4
  br label %postload2188.i

postload2188.i:                                   ; preds = %preload2187.i, %postload2185.i
  %phi2189.i = phi float [ undef, %postload2185.i ], [ %vload972.i, %preload2187.i ]
  %vpack973.i = insertelement <16 x float> %vpack969.i, float %phi2189.i, i32 1
  %exmask975.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 2
  br i1 %exmask975.i, label %preload2190.i, label %postload2191.i

preload2190.i:                                    ; preds = %postload2188.i
  %.sum2815.i = add i64 %extract384.i, 2
  %241 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2815.i
  %vload976.i = load float addrspace(3)* %241, align 4
  br label %postload2191.i

postload2191.i:                                   ; preds = %preload2190.i, %postload2188.i
  %phi2192.i = phi float [ undef, %postload2188.i ], [ %vload976.i, %preload2190.i ]
  %vpack977.i = insertelement <16 x float> %vpack973.i, float %phi2192.i, i32 2
  %exmask979.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 3
  br i1 %exmask979.i, label %preload2193.i, label %postload2194.i

preload2193.i:                                    ; preds = %postload2191.i
  %.sum2814.i = add i64 %extract384.i, 3
  %242 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2814.i
  %vload980.i = load float addrspace(3)* %242, align 4
  br label %postload2194.i

postload2194.i:                                   ; preds = %preload2193.i, %postload2191.i
  %phi2195.i = phi float [ undef, %postload2191.i ], [ %vload980.i, %preload2193.i ]
  %vpack981.i = insertelement <16 x float> %vpack977.i, float %phi2195.i, i32 3
  %exmask983.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 4
  br i1 %exmask983.i, label %preload2196.i, label %postload2197.i

preload2196.i:                                    ; preds = %postload2194.i
  %.sum2813.i = add i64 %extract384.i, 4
  %243 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2813.i
  %vload984.i = load float addrspace(3)* %243, align 4
  br label %postload2197.i

postload2197.i:                                   ; preds = %preload2196.i, %postload2194.i
  %phi2198.i = phi float [ undef, %postload2194.i ], [ %vload984.i, %preload2196.i ]
  %vpack985.i = insertelement <16 x float> %vpack981.i, float %phi2198.i, i32 4
  %exmask987.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 5
  br i1 %exmask987.i, label %preload2199.i, label %postload2200.i

preload2199.i:                                    ; preds = %postload2197.i
  %.sum2812.i = add i64 %extract384.i, 5
  %244 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2812.i
  %vload988.i = load float addrspace(3)* %244, align 4
  br label %postload2200.i

postload2200.i:                                   ; preds = %preload2199.i, %postload2197.i
  %phi2201.i = phi float [ undef, %postload2197.i ], [ %vload988.i, %preload2199.i ]
  %vpack989.i = insertelement <16 x float> %vpack985.i, float %phi2201.i, i32 5
  %exmask991.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 6
  br i1 %exmask991.i, label %preload2202.i, label %postload2203.i

preload2202.i:                                    ; preds = %postload2200.i
  %.sum2811.i = add i64 %extract384.i, 6
  %245 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2811.i
  %vload992.i = load float addrspace(3)* %245, align 4
  br label %postload2203.i

postload2203.i:                                   ; preds = %preload2202.i, %postload2200.i
  %phi2204.i = phi float [ undef, %postload2200.i ], [ %vload992.i, %preload2202.i ]
  %vpack993.i = insertelement <16 x float> %vpack989.i, float %phi2204.i, i32 6
  %exmask995.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 7
  br i1 %exmask995.i, label %preload2205.i, label %postload2206.i

preload2205.i:                                    ; preds = %postload2203.i
  %.sum2810.i = add i64 %extract384.i, 7
  %246 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2810.i
  %vload996.i = load float addrspace(3)* %246, align 4
  br label %postload2206.i

postload2206.i:                                   ; preds = %preload2205.i, %postload2203.i
  %phi2207.i = phi float [ undef, %postload2203.i ], [ %vload996.i, %preload2205.i ]
  %vpack997.i = insertelement <16 x float> %vpack993.i, float %phi2207.i, i32 7
  %exmask999.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 8
  br i1 %exmask999.i, label %preload2208.i, label %postload2209.i

preload2208.i:                                    ; preds = %postload2206.i
  %.sum2809.i = add i64 %extract384.i, 8
  %247 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2809.i
  %vload1000.i = load float addrspace(3)* %247, align 4
  br label %postload2209.i

postload2209.i:                                   ; preds = %preload2208.i, %postload2206.i
  %phi2210.i = phi float [ undef, %postload2206.i ], [ %vload1000.i, %preload2208.i ]
  %vpack1001.i = insertelement <16 x float> %vpack997.i, float %phi2210.i, i32 8
  %exmask1003.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 9
  br i1 %exmask1003.i, label %preload2211.i, label %postload2212.i

preload2211.i:                                    ; preds = %postload2209.i
  %.sum2808.i = add i64 %extract384.i, 9
  %248 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2808.i
  %vload1004.i = load float addrspace(3)* %248, align 4
  br label %postload2212.i

postload2212.i:                                   ; preds = %preload2211.i, %postload2209.i
  %phi2213.i = phi float [ undef, %postload2209.i ], [ %vload1004.i, %preload2211.i ]
  %vpack1005.i = insertelement <16 x float> %vpack1001.i, float %phi2213.i, i32 9
  %exmask1007.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 10
  br i1 %exmask1007.i, label %preload2214.i, label %postload2215.i

preload2214.i:                                    ; preds = %postload2212.i
  %.sum2807.i = add i64 %extract384.i, 10
  %249 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2807.i
  %vload1008.i = load float addrspace(3)* %249, align 4
  br label %postload2215.i

postload2215.i:                                   ; preds = %preload2214.i, %postload2212.i
  %phi2216.i = phi float [ undef, %postload2212.i ], [ %vload1008.i, %preload2214.i ]
  %vpack1009.i = insertelement <16 x float> %vpack1005.i, float %phi2216.i, i32 10
  %exmask1011.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 11
  br i1 %exmask1011.i, label %preload2217.i, label %postload2218.i

preload2217.i:                                    ; preds = %postload2215.i
  %.sum2806.i = add i64 %extract384.i, 11
  %250 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2806.i
  %vload1012.i = load float addrspace(3)* %250, align 4
  br label %postload2218.i

postload2218.i:                                   ; preds = %preload2217.i, %postload2215.i
  %phi2219.i = phi float [ undef, %postload2215.i ], [ %vload1012.i, %preload2217.i ]
  %vpack1013.i = insertelement <16 x float> %vpack1009.i, float %phi2219.i, i32 11
  %exmask1015.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 12
  br i1 %exmask1015.i, label %preload2220.i, label %postload2221.i

preload2220.i:                                    ; preds = %postload2218.i
  %.sum2805.i = add i64 %extract384.i, 12
  %251 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2805.i
  %vload1016.i = load float addrspace(3)* %251, align 4
  br label %postload2221.i

postload2221.i:                                   ; preds = %preload2220.i, %postload2218.i
  %phi2222.i = phi float [ undef, %postload2218.i ], [ %vload1016.i, %preload2220.i ]
  %vpack1017.i = insertelement <16 x float> %vpack1013.i, float %phi2222.i, i32 12
  %exmask1019.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 13
  br i1 %exmask1019.i, label %preload2223.i, label %postload2224.i

preload2223.i:                                    ; preds = %postload2221.i
  %.sum2804.i = add i64 %extract384.i, 13
  %252 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2804.i
  %vload1020.i = load float addrspace(3)* %252, align 4
  br label %postload2224.i

postload2224.i:                                   ; preds = %preload2223.i, %postload2221.i
  %phi2225.i = phi float [ undef, %postload2221.i ], [ %vload1020.i, %preload2223.i ]
  %vpack1021.i = insertelement <16 x float> %vpack1017.i, float %phi2225.i, i32 13
  %exmask1023.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 14
  br i1 %exmask1023.i, label %preload2226.i, label %postload2227.i

preload2226.i:                                    ; preds = %postload2224.i
  %.sum2803.i = add i64 %extract384.i, 14
  %253 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2803.i
  %vload1024.i = load float addrspace(3)* %253, align 4
  br label %postload2227.i

postload2227.i:                                   ; preds = %preload2226.i, %postload2224.i
  %phi2228.i = phi float [ undef, %postload2224.i ], [ %vload1024.i, %preload2226.i ]
  %vpack1025.i = insertelement <16 x float> %vpack1021.i, float %phi2228.i, i32 14
  %exmask1027.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 15
  br i1 %exmask1027.i, label %preload2229.i, label %postload2230.i

preload2229.i:                                    ; preds = %postload2227.i
  %.sum2802.i = add i64 %extract384.i, 15
  %254 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2802.i
  %vload1028.i = load float addrspace(3)* %254, align 4
  br label %postload2230.i

postload2230.i:                                   ; preds = %preload2229.i, %postload2227.i
  %phi2231.i = phi float [ undef, %postload2227.i ], [ %vload1028.i, %preload2229.i ]
  %vpack1029.i = insertelement <16 x float> %vpack1025.i, float %phi2231.i, i32 15
  br i1 %exmask967.i, label %preload2232.i, label %postload2233.i

preload2232.i:                                    ; preds = %postload2230.i
  %"&(pSB[currWI].offset)3914.i" = add nuw i64 %CurrSBIndex..6.i, 80
  %"&pSB[currWI].offset3915.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3914.i"
  %CastToValueType3916.i = bitcast i8* %"&pSB[currWI].offset3915.i" to float addrspace(3)**
  %loadedValue3917.i = load float addrspace(3)** %CastToValueType3916.i, align 8
  %vload1033.i = load float addrspace(3)* %loadedValue3917.i, align 4
  br label %postload2233.i

postload2233.i:                                   ; preds = %preload2232.i, %postload2230.i
  %phi2234.i = phi float [ undef, %postload2230.i ], [ %vload1033.i, %preload2232.i ]
  %vpack1034.i = insertelement <16 x float> undef, float %phi2234.i, i32 0
  br i1 %exmask971.i, label %preload2235.i, label %postload2236.i

preload2235.i:                                    ; preds = %postload2233.i
  %"&(pSB[currWI].offset)3360.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3361.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3360.i"
  %CastToValueType3362.i = bitcast i8* %"&pSB[currWI].offset3361.i" to i64*
  %loadedValue3363.i = load i64* %CastToValueType3362.i, align 8
  %.sum2801.i = add i64 %loadedValue3363.i, 1
  %255 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2801.i
  %vload1037.i = load float addrspace(3)* %255, align 4
  br label %postload2236.i

postload2236.i:                                   ; preds = %preload2235.i, %postload2233.i
  %phi2237.i = phi float [ undef, %postload2233.i ], [ %vload1037.i, %preload2235.i ]
  %vpack1038.i = insertelement <16 x float> %vpack1034.i, float %phi2237.i, i32 1
  br i1 %exmask975.i, label %preload2238.i, label %postload2239.i

preload2238.i:                                    ; preds = %postload2236.i
  %"&(pSB[currWI].offset)3365.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3366.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3365.i"
  %CastToValueType3367.i = bitcast i8* %"&pSB[currWI].offset3366.i" to i64*
  %loadedValue3368.i = load i64* %CastToValueType3367.i, align 8
  %.sum2800.i = add i64 %loadedValue3368.i, 2
  %256 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2800.i
  %vload1041.i = load float addrspace(3)* %256, align 4
  br label %postload2239.i

postload2239.i:                                   ; preds = %preload2238.i, %postload2236.i
  %phi2240.i = phi float [ undef, %postload2236.i ], [ %vload1041.i, %preload2238.i ]
  %vpack1042.i = insertelement <16 x float> %vpack1038.i, float %phi2240.i, i32 2
  br i1 %exmask979.i, label %preload2241.i, label %postload2242.i

preload2241.i:                                    ; preds = %postload2239.i
  %"&(pSB[currWI].offset)3370.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3371.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3370.i"
  %CastToValueType3372.i = bitcast i8* %"&pSB[currWI].offset3371.i" to i64*
  %loadedValue3373.i = load i64* %CastToValueType3372.i, align 8
  %.sum2799.i = add i64 %loadedValue3373.i, 3
  %257 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2799.i
  %vload1045.i = load float addrspace(3)* %257, align 4
  br label %postload2242.i

postload2242.i:                                   ; preds = %preload2241.i, %postload2239.i
  %phi2243.i = phi float [ undef, %postload2239.i ], [ %vload1045.i, %preload2241.i ]
  %vpack1046.i = insertelement <16 x float> %vpack1042.i, float %phi2243.i, i32 3
  br i1 %exmask983.i, label %preload2244.i, label %postload2245.i

preload2244.i:                                    ; preds = %postload2242.i
  %"&(pSB[currWI].offset)3375.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3376.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3375.i"
  %CastToValueType3377.i = bitcast i8* %"&pSB[currWI].offset3376.i" to i64*
  %loadedValue3378.i = load i64* %CastToValueType3377.i, align 8
  %.sum2798.i = add i64 %loadedValue3378.i, 4
  %258 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2798.i
  %vload1049.i = load float addrspace(3)* %258, align 4
  br label %postload2245.i

postload2245.i:                                   ; preds = %preload2244.i, %postload2242.i
  %phi2246.i = phi float [ undef, %postload2242.i ], [ %vload1049.i, %preload2244.i ]
  %vpack1050.i = insertelement <16 x float> %vpack1046.i, float %phi2246.i, i32 4
  br i1 %exmask987.i, label %preload2247.i, label %postload2248.i

preload2247.i:                                    ; preds = %postload2245.i
  %"&(pSB[currWI].offset)3380.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3381.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3380.i"
  %CastToValueType3382.i = bitcast i8* %"&pSB[currWI].offset3381.i" to i64*
  %loadedValue3383.i = load i64* %CastToValueType3382.i, align 8
  %.sum2797.i = add i64 %loadedValue3383.i, 5
  %259 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2797.i
  %vload1053.i = load float addrspace(3)* %259, align 4
  br label %postload2248.i

postload2248.i:                                   ; preds = %preload2247.i, %postload2245.i
  %phi2249.i = phi float [ undef, %postload2245.i ], [ %vload1053.i, %preload2247.i ]
  %vpack1054.i = insertelement <16 x float> %vpack1050.i, float %phi2249.i, i32 5
  br i1 %exmask991.i, label %preload2250.i, label %postload2251.i

preload2250.i:                                    ; preds = %postload2248.i
  %"&(pSB[currWI].offset)3385.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3386.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3385.i"
  %CastToValueType3387.i = bitcast i8* %"&pSB[currWI].offset3386.i" to i64*
  %loadedValue3388.i = load i64* %CastToValueType3387.i, align 8
  %.sum2796.i = add i64 %loadedValue3388.i, 6
  %260 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2796.i
  %vload1057.i = load float addrspace(3)* %260, align 4
  br label %postload2251.i

postload2251.i:                                   ; preds = %preload2250.i, %postload2248.i
  %phi2252.i = phi float [ undef, %postload2248.i ], [ %vload1057.i, %preload2250.i ]
  %vpack1058.i = insertelement <16 x float> %vpack1054.i, float %phi2252.i, i32 6
  br i1 %exmask995.i, label %preload2253.i, label %postload2254.i

preload2253.i:                                    ; preds = %postload2251.i
  %"&(pSB[currWI].offset)3390.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3391.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3390.i"
  %CastToValueType3392.i = bitcast i8* %"&pSB[currWI].offset3391.i" to i64*
  %loadedValue3393.i = load i64* %CastToValueType3392.i, align 8
  %.sum2795.i = add i64 %loadedValue3393.i, 7
  %261 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2795.i
  %vload1061.i = load float addrspace(3)* %261, align 4
  br label %postload2254.i

postload2254.i:                                   ; preds = %preload2253.i, %postload2251.i
  %phi2255.i = phi float [ undef, %postload2251.i ], [ %vload1061.i, %preload2253.i ]
  %vpack1062.i = insertelement <16 x float> %vpack1058.i, float %phi2255.i, i32 7
  br i1 %exmask999.i, label %preload2256.i, label %postload2257.i

preload2256.i:                                    ; preds = %postload2254.i
  %"&(pSB[currWI].offset)3395.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3396.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3395.i"
  %CastToValueType3397.i = bitcast i8* %"&pSB[currWI].offset3396.i" to i64*
  %loadedValue3398.i = load i64* %CastToValueType3397.i, align 8
  %.sum2794.i = add i64 %loadedValue3398.i, 8
  %262 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2794.i
  %vload1065.i = load float addrspace(3)* %262, align 4
  br label %postload2257.i

postload2257.i:                                   ; preds = %preload2256.i, %postload2254.i
  %phi2258.i = phi float [ undef, %postload2254.i ], [ %vload1065.i, %preload2256.i ]
  %vpack1066.i = insertelement <16 x float> %vpack1062.i, float %phi2258.i, i32 8
  br i1 %exmask1003.i, label %preload2259.i, label %postload2260.i

preload2259.i:                                    ; preds = %postload2257.i
  %"&(pSB[currWI].offset)3400.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3401.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3400.i"
  %CastToValueType3402.i = bitcast i8* %"&pSB[currWI].offset3401.i" to i64*
  %loadedValue3403.i = load i64* %CastToValueType3402.i, align 8
  %.sum2793.i = add i64 %loadedValue3403.i, 9
  %263 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2793.i
  %vload1069.i = load float addrspace(3)* %263, align 4
  br label %postload2260.i

postload2260.i:                                   ; preds = %preload2259.i, %postload2257.i
  %phi2261.i = phi float [ undef, %postload2257.i ], [ %vload1069.i, %preload2259.i ]
  %vpack1070.i = insertelement <16 x float> %vpack1066.i, float %phi2261.i, i32 9
  br i1 %exmask1007.i, label %preload2262.i, label %postload2263.i

preload2262.i:                                    ; preds = %postload2260.i
  %"&(pSB[currWI].offset)3405.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3406.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3405.i"
  %CastToValueType3407.i = bitcast i8* %"&pSB[currWI].offset3406.i" to i64*
  %loadedValue3408.i = load i64* %CastToValueType3407.i, align 8
  %.sum2792.i = add i64 %loadedValue3408.i, 10
  %264 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2792.i
  %vload1073.i = load float addrspace(3)* %264, align 4
  br label %postload2263.i

postload2263.i:                                   ; preds = %preload2262.i, %postload2260.i
  %phi2264.i = phi float [ undef, %postload2260.i ], [ %vload1073.i, %preload2262.i ]
  %vpack1074.i = insertelement <16 x float> %vpack1070.i, float %phi2264.i, i32 10
  br i1 %exmask1011.i, label %preload2265.i, label %postload2266.i

preload2265.i:                                    ; preds = %postload2263.i
  %"&(pSB[currWI].offset)3410.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3411.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3410.i"
  %CastToValueType3412.i = bitcast i8* %"&pSB[currWI].offset3411.i" to i64*
  %loadedValue3413.i = load i64* %CastToValueType3412.i, align 8
  %.sum2791.i = add i64 %loadedValue3413.i, 11
  %265 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2791.i
  %vload1077.i = load float addrspace(3)* %265, align 4
  br label %postload2266.i

postload2266.i:                                   ; preds = %preload2265.i, %postload2263.i
  %phi2267.i = phi float [ undef, %postload2263.i ], [ %vload1077.i, %preload2265.i ]
  %vpack1078.i = insertelement <16 x float> %vpack1074.i, float %phi2267.i, i32 11
  br i1 %exmask1015.i, label %preload2268.i, label %postload2269.i

preload2268.i:                                    ; preds = %postload2266.i
  %"&(pSB[currWI].offset)3415.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3416.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3415.i"
  %CastToValueType3417.i = bitcast i8* %"&pSB[currWI].offset3416.i" to i64*
  %loadedValue3418.i = load i64* %CastToValueType3417.i, align 8
  %.sum2790.i = add i64 %loadedValue3418.i, 12
  %266 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2790.i
  %vload1081.i = load float addrspace(3)* %266, align 4
  br label %postload2269.i

postload2269.i:                                   ; preds = %preload2268.i, %postload2266.i
  %phi2270.i = phi float [ undef, %postload2266.i ], [ %vload1081.i, %preload2268.i ]
  %vpack1082.i = insertelement <16 x float> %vpack1078.i, float %phi2270.i, i32 12
  br i1 %exmask1019.i, label %preload2271.i, label %postload2272.i

preload2271.i:                                    ; preds = %postload2269.i
  %"&(pSB[currWI].offset)3420.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3421.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3420.i"
  %CastToValueType3422.i = bitcast i8* %"&pSB[currWI].offset3421.i" to i64*
  %loadedValue3423.i = load i64* %CastToValueType3422.i, align 8
  %.sum2789.i = add i64 %loadedValue3423.i, 13
  %267 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2789.i
  %vload1085.i = load float addrspace(3)* %267, align 4
  br label %postload2272.i

postload2272.i:                                   ; preds = %preload2271.i, %postload2269.i
  %phi2273.i = phi float [ undef, %postload2269.i ], [ %vload1085.i, %preload2271.i ]
  %vpack1086.i = insertelement <16 x float> %vpack1082.i, float %phi2273.i, i32 13
  br i1 %exmask1023.i, label %preload2274.i, label %postload2275.i

preload2274.i:                                    ; preds = %postload2272.i
  %"&(pSB[currWI].offset)3425.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3426.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3425.i"
  %CastToValueType3427.i = bitcast i8* %"&pSB[currWI].offset3426.i" to i64*
  %loadedValue3428.i = load i64* %CastToValueType3427.i, align 8
  %.sum2788.i = add i64 %loadedValue3428.i, 14
  %268 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2788.i
  %vload1089.i = load float addrspace(3)* %268, align 4
  br label %postload2275.i

postload2275.i:                                   ; preds = %preload2274.i, %postload2272.i
  %phi2276.i = phi float [ undef, %postload2272.i ], [ %vload1089.i, %preload2274.i ]
  %vpack1090.i = insertelement <16 x float> %vpack1086.i, float %phi2276.i, i32 14
  br i1 %exmask1027.i, label %preload2277.i, label %postload2278.i

preload2277.i:                                    ; preds = %postload2275.i
  %"&(pSB[currWI].offset)3430.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3431.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3430.i"
  %CastToValueType3432.i = bitcast i8* %"&pSB[currWI].offset3431.i" to i64*
  %loadedValue3433.i = load i64* %CastToValueType3432.i, align 8
  %.sum2787.i = add i64 %loadedValue3433.i, 15
  %269 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2787.i
  %vload1093.i = load float addrspace(3)* %269, align 4
  br label %postload2278.i

postload2278.i:                                   ; preds = %preload2277.i, %postload2275.i
  %phi2279.i = phi float [ undef, %postload2275.i ], [ %vload1093.i, %preload2277.i ]
  %vpack1094.i = insertelement <16 x float> %vpack1090.i, float %phi2279.i, i32 15
  %add55402.i = fadd <16 x float> %vpack1094.i, %vpack1029.i
  br i1 %exmask967.i, label %preload2280.i, label %postload2281.i

preload2280.i:                                    ; preds = %postload2278.i
  %exData1098.i = extractelement <16 x float> %add55402.i, i32 0
  %"&(pSB[currWI].offset)3919.i" = add nuw i64 %CurrSBIndex..6.i, 80
  %"&pSB[currWI].offset3920.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3919.i"
  %CastToValueType3921.i = bitcast i8* %"&pSB[currWI].offset3920.i" to float addrspace(3)**
  %loadedValue3922.i = load float addrspace(3)** %CastToValueType3921.i, align 8
  store float %exData1098.i, float addrspace(3)* %loadedValue3922.i, align 4
  br label %postload2281.i

postload2281.i:                                   ; preds = %preload2280.i, %postload2278.i
  br i1 %exmask971.i, label %preload2283.i, label %postload2284.i

preload2283.i:                                    ; preds = %postload2281.i
  %"&(pSB[currWI].offset)3435.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3436.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3435.i"
  %CastToValueType3437.i = bitcast i8* %"&pSB[currWI].offset3436.i" to i64*
  %loadedValue3438.i = load i64* %CastToValueType3437.i, align 8
  %.sum2786.i = add i64 %loadedValue3438.i, 1
  %270 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2786.i
  %exData1101.i = extractelement <16 x float> %add55402.i, i32 1
  store float %exData1101.i, float addrspace(3)* %270, align 4
  br label %postload2284.i

postload2284.i:                                   ; preds = %preload2283.i, %postload2281.i
  br i1 %exmask975.i, label %preload2286.i, label %postload2287.i

preload2286.i:                                    ; preds = %postload2284.i
  %"&(pSB[currWI].offset)3440.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3441.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3440.i"
  %CastToValueType3442.i = bitcast i8* %"&pSB[currWI].offset3441.i" to i64*
  %loadedValue3443.i = load i64* %CastToValueType3442.i, align 8
  %.sum2785.i = add i64 %loadedValue3443.i, 2
  %271 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2785.i
  %exData1104.i = extractelement <16 x float> %add55402.i, i32 2
  store float %exData1104.i, float addrspace(3)* %271, align 4
  br label %postload2287.i

postload2287.i:                                   ; preds = %preload2286.i, %postload2284.i
  br i1 %exmask979.i, label %preload2289.i, label %postload2290.i

preload2289.i:                                    ; preds = %postload2287.i
  %"&(pSB[currWI].offset)3445.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3446.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3445.i"
  %CastToValueType3447.i = bitcast i8* %"&pSB[currWI].offset3446.i" to i64*
  %loadedValue3448.i = load i64* %CastToValueType3447.i, align 8
  %.sum2784.i = add i64 %loadedValue3448.i, 3
  %272 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2784.i
  %exData1107.i = extractelement <16 x float> %add55402.i, i32 3
  store float %exData1107.i, float addrspace(3)* %272, align 4
  br label %postload2290.i

postload2290.i:                                   ; preds = %preload2289.i, %postload2287.i
  br i1 %exmask983.i, label %preload2292.i, label %postload2293.i

preload2292.i:                                    ; preds = %postload2290.i
  %"&(pSB[currWI].offset)3450.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3451.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3450.i"
  %CastToValueType3452.i = bitcast i8* %"&pSB[currWI].offset3451.i" to i64*
  %loadedValue3453.i = load i64* %CastToValueType3452.i, align 8
  %.sum2783.i = add i64 %loadedValue3453.i, 4
  %273 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2783.i
  %exData1110.i = extractelement <16 x float> %add55402.i, i32 4
  store float %exData1110.i, float addrspace(3)* %273, align 4
  br label %postload2293.i

postload2293.i:                                   ; preds = %preload2292.i, %postload2290.i
  br i1 %exmask987.i, label %preload2295.i, label %postload2296.i

preload2295.i:                                    ; preds = %postload2293.i
  %"&(pSB[currWI].offset)3455.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3456.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3455.i"
  %CastToValueType3457.i = bitcast i8* %"&pSB[currWI].offset3456.i" to i64*
  %loadedValue3458.i = load i64* %CastToValueType3457.i, align 8
  %.sum2782.i = add i64 %loadedValue3458.i, 5
  %274 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2782.i
  %exData1113.i = extractelement <16 x float> %add55402.i, i32 5
  store float %exData1113.i, float addrspace(3)* %274, align 4
  br label %postload2296.i

postload2296.i:                                   ; preds = %preload2295.i, %postload2293.i
  br i1 %exmask991.i, label %preload2298.i, label %postload2299.i

preload2298.i:                                    ; preds = %postload2296.i
  %"&(pSB[currWI].offset)3460.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3461.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3460.i"
  %CastToValueType3462.i = bitcast i8* %"&pSB[currWI].offset3461.i" to i64*
  %loadedValue3463.i = load i64* %CastToValueType3462.i, align 8
  %.sum2781.i = add i64 %loadedValue3463.i, 6
  %275 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2781.i
  %exData1116.i = extractelement <16 x float> %add55402.i, i32 6
  store float %exData1116.i, float addrspace(3)* %275, align 4
  br label %postload2299.i

postload2299.i:                                   ; preds = %preload2298.i, %postload2296.i
  br i1 %exmask995.i, label %preload2301.i, label %postload2302.i

preload2301.i:                                    ; preds = %postload2299.i
  %"&(pSB[currWI].offset)3465.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3466.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3465.i"
  %CastToValueType3467.i = bitcast i8* %"&pSB[currWI].offset3466.i" to i64*
  %loadedValue3468.i = load i64* %CastToValueType3467.i, align 8
  %.sum2780.i = add i64 %loadedValue3468.i, 7
  %276 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2780.i
  %exData1119.i = extractelement <16 x float> %add55402.i, i32 7
  store float %exData1119.i, float addrspace(3)* %276, align 4
  br label %postload2302.i

postload2302.i:                                   ; preds = %preload2301.i, %postload2299.i
  br i1 %exmask999.i, label %preload2304.i, label %postload2305.i

preload2304.i:                                    ; preds = %postload2302.i
  %"&(pSB[currWI].offset)3470.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3471.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3470.i"
  %CastToValueType3472.i = bitcast i8* %"&pSB[currWI].offset3471.i" to i64*
  %loadedValue3473.i = load i64* %CastToValueType3472.i, align 8
  %.sum2779.i = add i64 %loadedValue3473.i, 8
  %277 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2779.i
  %exData1122.i = extractelement <16 x float> %add55402.i, i32 8
  store float %exData1122.i, float addrspace(3)* %277, align 4
  br label %postload2305.i

postload2305.i:                                   ; preds = %preload2304.i, %postload2302.i
  br i1 %exmask1003.i, label %preload2307.i, label %postload2308.i

preload2307.i:                                    ; preds = %postload2305.i
  %"&(pSB[currWI].offset)3475.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3476.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3475.i"
  %CastToValueType3477.i = bitcast i8* %"&pSB[currWI].offset3476.i" to i64*
  %loadedValue3478.i = load i64* %CastToValueType3477.i, align 8
  %.sum2778.i = add i64 %loadedValue3478.i, 9
  %278 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2778.i
  %exData1125.i = extractelement <16 x float> %add55402.i, i32 9
  store float %exData1125.i, float addrspace(3)* %278, align 4
  br label %postload2308.i

postload2308.i:                                   ; preds = %preload2307.i, %postload2305.i
  br i1 %exmask1007.i, label %preload2310.i, label %postload2311.i

preload2310.i:                                    ; preds = %postload2308.i
  %"&(pSB[currWI].offset)3480.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3481.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3480.i"
  %CastToValueType3482.i = bitcast i8* %"&pSB[currWI].offset3481.i" to i64*
  %loadedValue3483.i = load i64* %CastToValueType3482.i, align 8
  %.sum2777.i = add i64 %loadedValue3483.i, 10
  %279 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2777.i
  %exData1128.i = extractelement <16 x float> %add55402.i, i32 10
  store float %exData1128.i, float addrspace(3)* %279, align 4
  br label %postload2311.i

postload2311.i:                                   ; preds = %preload2310.i, %postload2308.i
  br i1 %exmask1011.i, label %preload2313.i, label %postload2314.i

preload2313.i:                                    ; preds = %postload2311.i
  %"&(pSB[currWI].offset)3485.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3486.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3485.i"
  %CastToValueType3487.i = bitcast i8* %"&pSB[currWI].offset3486.i" to i64*
  %loadedValue3488.i = load i64* %CastToValueType3487.i, align 8
  %.sum2776.i = add i64 %loadedValue3488.i, 11
  %280 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2776.i
  %exData1131.i = extractelement <16 x float> %add55402.i, i32 11
  store float %exData1131.i, float addrspace(3)* %280, align 4
  br label %postload2314.i

postload2314.i:                                   ; preds = %preload2313.i, %postload2311.i
  br i1 %exmask1015.i, label %preload2316.i, label %postload2317.i

preload2316.i:                                    ; preds = %postload2314.i
  %"&(pSB[currWI].offset)3490.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3491.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3490.i"
  %CastToValueType3492.i = bitcast i8* %"&pSB[currWI].offset3491.i" to i64*
  %loadedValue3493.i = load i64* %CastToValueType3492.i, align 8
  %.sum2775.i = add i64 %loadedValue3493.i, 12
  %281 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2775.i
  %exData1134.i = extractelement <16 x float> %add55402.i, i32 12
  store float %exData1134.i, float addrspace(3)* %281, align 4
  br label %postload2317.i

postload2317.i:                                   ; preds = %preload2316.i, %postload2314.i
  br i1 %exmask1019.i, label %preload2319.i, label %postload2320.i

preload2319.i:                                    ; preds = %postload2317.i
  %"&(pSB[currWI].offset)3495.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3496.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3495.i"
  %CastToValueType3497.i = bitcast i8* %"&pSB[currWI].offset3496.i" to i64*
  %loadedValue3498.i = load i64* %CastToValueType3497.i, align 8
  %.sum2774.i = add i64 %loadedValue3498.i, 13
  %282 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2774.i
  %exData1137.i = extractelement <16 x float> %add55402.i, i32 13
  store float %exData1137.i, float addrspace(3)* %282, align 4
  br label %postload2320.i

postload2320.i:                                   ; preds = %preload2319.i, %postload2317.i
  br i1 %exmask1023.i, label %preload2322.i, label %postload2323.i

preload2322.i:                                    ; preds = %postload2320.i
  %"&(pSB[currWI].offset)3500.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3501.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3500.i"
  %CastToValueType3502.i = bitcast i8* %"&pSB[currWI].offset3501.i" to i64*
  %loadedValue3503.i = load i64* %CastToValueType3502.i, align 8
  %.sum2773.i = add i64 %loadedValue3503.i, 14
  %283 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2773.i
  %exData1140.i = extractelement <16 x float> %add55402.i, i32 14
  store float %exData1140.i, float addrspace(3)* %283, align 4
  br label %postload2323.i

postload2323.i:                                   ; preds = %preload2322.i, %postload2320.i
  br i1 %exmask1027.i, label %preload2325.i, label %postload2323.i.if.end56.i_crit_edge

postload2323.i.if.end56.i_crit_edge:              ; preds = %postload2323.i
  br label %if.end56.i

preload2325.i:                                    ; preds = %postload2323.i
  %"&(pSB[currWI].offset)3505.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3506.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3505.i"
  %CastToValueType3507.i = bitcast i8* %"&pSB[currWI].offset3506.i" to i64*
  %loadedValue3508.i = load i64* %CastToValueType3507.i, align 8
  %.sum2772.i = add i64 %loadedValue3508.i, 15
  %284 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2772.i
  %exData1143.i = extractelement <16 x float> %add55402.i, i32 15
  store float %exData1143.i, float addrspace(3)* %284, align 4
  br label %if.end56.i

if.end56.i:                                       ; preds = %postload2323.i.if.end56.i_crit_edge, %preload2325.i
  %"&(pSB[currWI].offset)4007.i" = add nuw i64 %CurrSBIndex..6.i, 98
  %"&pSB[currWI].offset4008.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4007.i"
  %CastToValueType4009.i = bitcast i8* %"&pSB[currWI].offset4008.i" to i1*
  %loadedValue4010.i = load i1* %CastToValueType4009.i, align 1
  br i1 %loadedValue4010.i, label %preload1681.i, label %if.end56.i.postload1682.i_crit_edge

if.end56.i.postload1682.i_crit_edge:              ; preds = %if.end56.i
  br label %postload1682.i

preload1681.i:                                    ; preds = %if.end56.i
  %check.WI.iter4590.i = icmp ult i64 %CurrWI..6.i, %31
  br i1 %check.WI.iter4590.i, label %thenBB4587.i, label %preload1681.i.postload1682.i_crit_edge

preload1681.i.postload1682.i_crit_edge:           ; preds = %preload1681.i
  br label %postload1682.i

thenBB4587.i:                                     ; preds = %preload1681.i
  %"CurrWI++4591.i" = add nuw i64 %CurrWI..6.i, 1
  %"loadedCurrSB+Stride4593.i" = add nuw i64 %CurrSBIndex..6.i, 256
  switch i32 %currBarrier.6.i, label %thenBB4587.i.postload1680.i_crit_edge [
    i32 7, label %thenBB4587.i.postload1678.i_crit_edge
    i32 6, label %thenBB4587.i.postload1676.i_crit_edge
    i32 14, label %SyncBB4569.outer.i
  ]

thenBB4587.i.postload1676.i_crit_edge:            ; preds = %thenBB4587.i
  br label %postload1676.i

thenBB4587.i.postload1678.i_crit_edge:            ; preds = %thenBB4587.i
  br label %postload1678.i

thenBB4587.i.postload1680.i_crit_edge:            ; preds = %thenBB4587.i
  br label %postload1680.i

postload1682.i:                                   ; preds = %thenBB4611.i.postload1682.i_crit_edge, %thenBB4603.i.postload1682.i_crit_edge, %thenBB4595.i.postload1682.i_crit_edge, %preload1681.i.postload1682.i_crit_edge, %if.end56.i.postload1682.i_crit_edge
  %CurrWI..8.i = phi i64 [ %CurrWI..6.i, %if.end56.i.postload1682.i_crit_edge ], [ 0, %preload1681.i.postload1682.i_crit_edge ], [ %"CurrWI++4599.i", %thenBB4595.i.postload1682.i_crit_edge ], [ %"CurrWI++4607.i", %thenBB4603.i.postload1682.i_crit_edge ], [ %"CurrWI++4615.i", %thenBB4611.i.postload1682.i_crit_edge ]
  %CurrSBIndex..8.i = phi i64 [ %CurrSBIndex..6.i, %if.end56.i.postload1682.i_crit_edge ], [ 0, %preload1681.i.postload1682.i_crit_edge ], [ %"loadedCurrSB+Stride4601.i", %thenBB4595.i.postload1682.i_crit_edge ], [ %"loadedCurrSB+Stride4609.i", %thenBB4603.i.postload1682.i_crit_edge ], [ %"loadedCurrSB+Stride4617.i", %thenBB4611.i.postload1682.i_crit_edge ]
  %currBarrier.8.i = phi i32 [ %currBarrier.6.i, %if.end56.i.postload1682.i_crit_edge ], [ 9, %preload1681.i.postload1682.i_crit_edge ], [ %currBarrier.8.i, %thenBB4595.i.postload1682.i_crit_edge ], [ %currBarrier.10.i, %thenBB4603.i.postload1682.i_crit_edge ], [ %currBarrier.12.i, %thenBB4611.i.postload1682.i_crit_edge ]
  %"&pSB[currWI].offset2928.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..8.i
  %CastToValueType2929.i = bitcast i8* %"&pSB[currWI].offset2928.i" to <16 x i32>*
  %loadedValue2930.i = load <16 x i32>* %CastToValueType2929.i, align 64
  %cmp57.i = icmp ult <16 x i32> %loadedValue2930.i, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %"&(pSB[currWI].offset)3978.i" = add nuw i64 %CurrSBIndex..8.i, 96
  %"&pSB[currWI].offset3979.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3978.i"
  %CastToValueType3980.i = bitcast i8* %"&pSB[currWI].offset3979.i" to <16 x i1>*
  %loadedValue3981.i = load <16 x i1>* %CastToValueType3980.i, align 16
  %if.end56_to_if.then59404.i = and <16 x i1> %loadedValue3981.i, %cmp57.i
  %"&(pSB[currWI].offset)2961.i" = add nuw i64 %CurrSBIndex..8.i, 64
  %"&pSB[currWI].offset2962.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)2961.i"
  %CastToValueType2963.i = bitcast i8* %"&pSB[currWI].offset2962.i" to i32*
  %loadedValue2964.i = load i32* %CastToValueType2963.i, align 4
  %285 = add i32 %loadedValue2964.i, 2
  %extract407.i = sext i32 %285 to i64
  %exmask1146.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 0
  br i1 %exmask1146.i, label %preload2328.i, label %postload2329.i

preload2328.i:                                    ; preds = %postload1682.i
  %286 = getelementptr inbounds [128 x float] addrspace(3)* %35, i64 0, i64 %extract407.i
  %vload1147.i = load float addrspace(3)* %286, align 4
  br label %postload2329.i

postload2329.i:                                   ; preds = %preload2328.i, %postload1682.i
  %phi2330.i = phi float [ undef, %postload1682.i ], [ %vload1147.i, %preload2328.i ]
  %vpack1148.i = insertelement <16 x float> undef, float %phi2330.i, i32 0
  %exmask1150.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 1
  br i1 %exmask1150.i, label %preload2331.i, label %postload2332.i

preload2331.i:                                    ; preds = %postload2329.i
  %.sum2771.i = add i64 %extract407.i, 1
  %287 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2771.i
  %vload1151.i = load float addrspace(3)* %287, align 4
  br label %postload2332.i

postload2332.i:                                   ; preds = %preload2331.i, %postload2329.i
  %phi2333.i = phi float [ undef, %postload2329.i ], [ %vload1151.i, %preload2331.i ]
  %vpack1152.i = insertelement <16 x float> %vpack1148.i, float %phi2333.i, i32 1
  %exmask1154.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 2
  br i1 %exmask1154.i, label %preload2334.i, label %postload2335.i

preload2334.i:                                    ; preds = %postload2332.i
  %.sum2770.i = add i64 %extract407.i, 2
  %288 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2770.i
  %vload1155.i = load float addrspace(3)* %288, align 4
  br label %postload2335.i

postload2335.i:                                   ; preds = %preload2334.i, %postload2332.i
  %phi2336.i = phi float [ undef, %postload2332.i ], [ %vload1155.i, %preload2334.i ]
  %vpack1156.i = insertelement <16 x float> %vpack1152.i, float %phi2336.i, i32 2
  %exmask1158.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 3
  br i1 %exmask1158.i, label %preload2337.i, label %postload2338.i

preload2337.i:                                    ; preds = %postload2335.i
  %.sum2769.i = add i64 %extract407.i, 3
  %289 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2769.i
  %vload1159.i = load float addrspace(3)* %289, align 4
  br label %postload2338.i

postload2338.i:                                   ; preds = %preload2337.i, %postload2335.i
  %phi2339.i = phi float [ undef, %postload2335.i ], [ %vload1159.i, %preload2337.i ]
  %vpack1160.i = insertelement <16 x float> %vpack1156.i, float %phi2339.i, i32 3
  %exmask1162.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 4
  br i1 %exmask1162.i, label %preload2340.i, label %postload2341.i

preload2340.i:                                    ; preds = %postload2338.i
  %.sum2768.i = add i64 %extract407.i, 4
  %290 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2768.i
  %vload1163.i = load float addrspace(3)* %290, align 4
  br label %postload2341.i

postload2341.i:                                   ; preds = %preload2340.i, %postload2338.i
  %phi2342.i = phi float [ undef, %postload2338.i ], [ %vload1163.i, %preload2340.i ]
  %vpack1164.i = insertelement <16 x float> %vpack1160.i, float %phi2342.i, i32 4
  %exmask1166.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 5
  br i1 %exmask1166.i, label %preload2343.i, label %postload2344.i

preload2343.i:                                    ; preds = %postload2341.i
  %.sum2767.i = add i64 %extract407.i, 5
  %291 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2767.i
  %vload1167.i = load float addrspace(3)* %291, align 4
  br label %postload2344.i

postload2344.i:                                   ; preds = %preload2343.i, %postload2341.i
  %phi2345.i = phi float [ undef, %postload2341.i ], [ %vload1167.i, %preload2343.i ]
  %vpack1168.i = insertelement <16 x float> %vpack1164.i, float %phi2345.i, i32 5
  %exmask1170.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 6
  br i1 %exmask1170.i, label %preload2346.i, label %postload2347.i

preload2346.i:                                    ; preds = %postload2344.i
  %.sum2766.i = add i64 %extract407.i, 6
  %292 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2766.i
  %vload1171.i = load float addrspace(3)* %292, align 4
  br label %postload2347.i

postload2347.i:                                   ; preds = %preload2346.i, %postload2344.i
  %phi2348.i = phi float [ undef, %postload2344.i ], [ %vload1171.i, %preload2346.i ]
  %vpack1172.i = insertelement <16 x float> %vpack1168.i, float %phi2348.i, i32 6
  %exmask1174.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 7
  br i1 %exmask1174.i, label %preload2349.i, label %postload2350.i

preload2349.i:                                    ; preds = %postload2347.i
  %.sum2765.i = add i64 %extract407.i, 7
  %293 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2765.i
  %vload1175.i = load float addrspace(3)* %293, align 4
  br label %postload2350.i

postload2350.i:                                   ; preds = %preload2349.i, %postload2347.i
  %phi2351.i = phi float [ undef, %postload2347.i ], [ %vload1175.i, %preload2349.i ]
  %vpack1176.i = insertelement <16 x float> %vpack1172.i, float %phi2351.i, i32 7
  %exmask1178.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 8
  br i1 %exmask1178.i, label %preload2352.i, label %postload2353.i

preload2352.i:                                    ; preds = %postload2350.i
  %.sum2764.i = add i64 %extract407.i, 8
  %294 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2764.i
  %vload1179.i = load float addrspace(3)* %294, align 4
  br label %postload2353.i

postload2353.i:                                   ; preds = %preload2352.i, %postload2350.i
  %phi2354.i = phi float [ undef, %postload2350.i ], [ %vload1179.i, %preload2352.i ]
  %vpack1180.i = insertelement <16 x float> %vpack1176.i, float %phi2354.i, i32 8
  %exmask1182.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 9
  br i1 %exmask1182.i, label %preload2355.i, label %postload2356.i

preload2355.i:                                    ; preds = %postload2353.i
  %.sum2763.i = add i64 %extract407.i, 9
  %295 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2763.i
  %vload1183.i = load float addrspace(3)* %295, align 4
  br label %postload2356.i

postload2356.i:                                   ; preds = %preload2355.i, %postload2353.i
  %phi2357.i = phi float [ undef, %postload2353.i ], [ %vload1183.i, %preload2355.i ]
  %vpack1184.i = insertelement <16 x float> %vpack1180.i, float %phi2357.i, i32 9
  %exmask1186.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 10
  br i1 %exmask1186.i, label %preload2358.i, label %postload2359.i

preload2358.i:                                    ; preds = %postload2356.i
  %.sum2762.i = add i64 %extract407.i, 10
  %296 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2762.i
  %vload1187.i = load float addrspace(3)* %296, align 4
  br label %postload2359.i

postload2359.i:                                   ; preds = %preload2358.i, %postload2356.i
  %phi2360.i = phi float [ undef, %postload2356.i ], [ %vload1187.i, %preload2358.i ]
  %vpack1188.i = insertelement <16 x float> %vpack1184.i, float %phi2360.i, i32 10
  %exmask1190.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 11
  br i1 %exmask1190.i, label %preload2361.i, label %postload2362.i

preload2361.i:                                    ; preds = %postload2359.i
  %.sum2761.i = add i64 %extract407.i, 11
  %297 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2761.i
  %vload1191.i = load float addrspace(3)* %297, align 4
  br label %postload2362.i

postload2362.i:                                   ; preds = %preload2361.i, %postload2359.i
  %phi2363.i = phi float [ undef, %postload2359.i ], [ %vload1191.i, %preload2361.i ]
  %vpack1192.i = insertelement <16 x float> %vpack1188.i, float %phi2363.i, i32 11
  %exmask1194.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 12
  br i1 %exmask1194.i, label %preload2364.i, label %postload2365.i

preload2364.i:                                    ; preds = %postload2362.i
  %.sum2760.i = add i64 %extract407.i, 12
  %298 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2760.i
  %vload1195.i = load float addrspace(3)* %298, align 4
  br label %postload2365.i

postload2365.i:                                   ; preds = %preload2364.i, %postload2362.i
  %phi2366.i = phi float [ undef, %postload2362.i ], [ %vload1195.i, %preload2364.i ]
  %vpack1196.i = insertelement <16 x float> %vpack1192.i, float %phi2366.i, i32 12
  %exmask1198.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 13
  br i1 %exmask1198.i, label %preload2367.i, label %postload2368.i

preload2367.i:                                    ; preds = %postload2365.i
  %.sum2759.i = add i64 %extract407.i, 13
  %299 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2759.i
  %vload1199.i = load float addrspace(3)* %299, align 4
  br label %postload2368.i

postload2368.i:                                   ; preds = %preload2367.i, %postload2365.i
  %phi2369.i = phi float [ undef, %postload2365.i ], [ %vload1199.i, %preload2367.i ]
  %vpack1200.i = insertelement <16 x float> %vpack1196.i, float %phi2369.i, i32 13
  %exmask1202.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 14
  br i1 %exmask1202.i, label %preload2370.i, label %postload2371.i

preload2370.i:                                    ; preds = %postload2368.i
  %.sum2758.i = add i64 %extract407.i, 14
  %300 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2758.i
  %vload1203.i = load float addrspace(3)* %300, align 4
  br label %postload2371.i

postload2371.i:                                   ; preds = %preload2370.i, %postload2368.i
  %phi2372.i = phi float [ undef, %postload2368.i ], [ %vload1203.i, %preload2370.i ]
  %vpack1204.i = insertelement <16 x float> %vpack1200.i, float %phi2372.i, i32 14
  %exmask1206.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 15
  br i1 %exmask1206.i, label %preload2373.i, label %postload2374.i

preload2373.i:                                    ; preds = %postload2371.i
  %.sum2757.i = add i64 %extract407.i, 15
  %301 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2757.i
  %vload1207.i = load float addrspace(3)* %301, align 4
  br label %postload2374.i

postload2374.i:                                   ; preds = %preload2373.i, %postload2371.i
  %phi2375.i = phi float [ undef, %postload2371.i ], [ %vload1207.i, %preload2373.i ]
  %vpack1208.i = insertelement <16 x float> %vpack1204.i, float %phi2375.i, i32 15
  br i1 %exmask1146.i, label %preload2376.i, label %postload2377.i

preload2376.i:                                    ; preds = %postload2374.i
  %"&(pSB[currWI].offset)3924.i" = add nuw i64 %CurrSBIndex..8.i, 80
  %"&pSB[currWI].offset3925.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3924.i"
  %CastToValueType3926.i = bitcast i8* %"&pSB[currWI].offset3925.i" to float addrspace(3)**
  %loadedValue3927.i = load float addrspace(3)** %CastToValueType3926.i, align 8
  %vload1212.i = load float addrspace(3)* %loadedValue3927.i, align 4
  br label %postload2377.i

postload2377.i:                                   ; preds = %preload2376.i, %postload2374.i
  %phi2378.i = phi float [ undef, %postload2374.i ], [ %vload1212.i, %preload2376.i ]
  %vpack1213.i = insertelement <16 x float> undef, float %phi2378.i, i32 0
  br i1 %exmask1150.i, label %preload2379.i, label %postload2380.i

preload2379.i:                                    ; preds = %postload2377.i
  %"&(pSB[currWI].offset)3510.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3511.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3510.i"
  %CastToValueType3512.i = bitcast i8* %"&pSB[currWI].offset3511.i" to i64*
  %loadedValue3513.i = load i64* %CastToValueType3512.i, align 8
  %.sum2756.i = add i64 %loadedValue3513.i, 1
  %302 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2756.i
  %vload1216.i = load float addrspace(3)* %302, align 4
  br label %postload2380.i

postload2380.i:                                   ; preds = %preload2379.i, %postload2377.i
  %phi2381.i = phi float [ undef, %postload2377.i ], [ %vload1216.i, %preload2379.i ]
  %vpack1217.i = insertelement <16 x float> %vpack1213.i, float %phi2381.i, i32 1
  br i1 %exmask1154.i, label %preload2382.i, label %postload2383.i

preload2382.i:                                    ; preds = %postload2380.i
  %"&(pSB[currWI].offset)3515.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3516.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3515.i"
  %CastToValueType3517.i = bitcast i8* %"&pSB[currWI].offset3516.i" to i64*
  %loadedValue3518.i = load i64* %CastToValueType3517.i, align 8
  %.sum2755.i = add i64 %loadedValue3518.i, 2
  %303 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2755.i
  %vload1220.i = load float addrspace(3)* %303, align 4
  br label %postload2383.i

postload2383.i:                                   ; preds = %preload2382.i, %postload2380.i
  %phi2384.i = phi float [ undef, %postload2380.i ], [ %vload1220.i, %preload2382.i ]
  %vpack1221.i = insertelement <16 x float> %vpack1217.i, float %phi2384.i, i32 2
  br i1 %exmask1158.i, label %preload2385.i, label %postload2386.i

preload2385.i:                                    ; preds = %postload2383.i
  %"&(pSB[currWI].offset)3520.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3521.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3520.i"
  %CastToValueType3522.i = bitcast i8* %"&pSB[currWI].offset3521.i" to i64*
  %loadedValue3523.i = load i64* %CastToValueType3522.i, align 8
  %.sum2754.i = add i64 %loadedValue3523.i, 3
  %304 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2754.i
  %vload1224.i = load float addrspace(3)* %304, align 4
  br label %postload2386.i

postload2386.i:                                   ; preds = %preload2385.i, %postload2383.i
  %phi2387.i = phi float [ undef, %postload2383.i ], [ %vload1224.i, %preload2385.i ]
  %vpack1225.i = insertelement <16 x float> %vpack1221.i, float %phi2387.i, i32 3
  br i1 %exmask1162.i, label %preload2388.i, label %postload2389.i

preload2388.i:                                    ; preds = %postload2386.i
  %"&(pSB[currWI].offset)3525.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3526.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3525.i"
  %CastToValueType3527.i = bitcast i8* %"&pSB[currWI].offset3526.i" to i64*
  %loadedValue3528.i = load i64* %CastToValueType3527.i, align 8
  %.sum2753.i = add i64 %loadedValue3528.i, 4
  %305 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2753.i
  %vload1228.i = load float addrspace(3)* %305, align 4
  br label %postload2389.i

postload2389.i:                                   ; preds = %preload2388.i, %postload2386.i
  %phi2390.i = phi float [ undef, %postload2386.i ], [ %vload1228.i, %preload2388.i ]
  %vpack1229.i = insertelement <16 x float> %vpack1225.i, float %phi2390.i, i32 4
  br i1 %exmask1166.i, label %preload2391.i, label %postload2392.i

preload2391.i:                                    ; preds = %postload2389.i
  %"&(pSB[currWI].offset)3530.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3531.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3530.i"
  %CastToValueType3532.i = bitcast i8* %"&pSB[currWI].offset3531.i" to i64*
  %loadedValue3533.i = load i64* %CastToValueType3532.i, align 8
  %.sum2752.i = add i64 %loadedValue3533.i, 5
  %306 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2752.i
  %vload1232.i = load float addrspace(3)* %306, align 4
  br label %postload2392.i

postload2392.i:                                   ; preds = %preload2391.i, %postload2389.i
  %phi2393.i = phi float [ undef, %postload2389.i ], [ %vload1232.i, %preload2391.i ]
  %vpack1233.i = insertelement <16 x float> %vpack1229.i, float %phi2393.i, i32 5
  br i1 %exmask1170.i, label %preload2394.i, label %postload2395.i

preload2394.i:                                    ; preds = %postload2392.i
  %"&(pSB[currWI].offset)3535.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3536.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3535.i"
  %CastToValueType3537.i = bitcast i8* %"&pSB[currWI].offset3536.i" to i64*
  %loadedValue3538.i = load i64* %CastToValueType3537.i, align 8
  %.sum2751.i = add i64 %loadedValue3538.i, 6
  %307 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2751.i
  %vload1236.i = load float addrspace(3)* %307, align 4
  br label %postload2395.i

postload2395.i:                                   ; preds = %preload2394.i, %postload2392.i
  %phi2396.i = phi float [ undef, %postload2392.i ], [ %vload1236.i, %preload2394.i ]
  %vpack1237.i = insertelement <16 x float> %vpack1233.i, float %phi2396.i, i32 6
  br i1 %exmask1174.i, label %preload2397.i, label %postload2398.i

preload2397.i:                                    ; preds = %postload2395.i
  %"&(pSB[currWI].offset)3540.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3541.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3540.i"
  %CastToValueType3542.i = bitcast i8* %"&pSB[currWI].offset3541.i" to i64*
  %loadedValue3543.i = load i64* %CastToValueType3542.i, align 8
  %.sum2750.i = add i64 %loadedValue3543.i, 7
  %308 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2750.i
  %vload1240.i = load float addrspace(3)* %308, align 4
  br label %postload2398.i

postload2398.i:                                   ; preds = %preload2397.i, %postload2395.i
  %phi2399.i = phi float [ undef, %postload2395.i ], [ %vload1240.i, %preload2397.i ]
  %vpack1241.i = insertelement <16 x float> %vpack1237.i, float %phi2399.i, i32 7
  br i1 %exmask1178.i, label %preload2400.i, label %postload2401.i

preload2400.i:                                    ; preds = %postload2398.i
  %"&(pSB[currWI].offset)3545.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3546.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3545.i"
  %CastToValueType3547.i = bitcast i8* %"&pSB[currWI].offset3546.i" to i64*
  %loadedValue3548.i = load i64* %CastToValueType3547.i, align 8
  %.sum2749.i = add i64 %loadedValue3548.i, 8
  %309 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2749.i
  %vload1244.i = load float addrspace(3)* %309, align 4
  br label %postload2401.i

postload2401.i:                                   ; preds = %preload2400.i, %postload2398.i
  %phi2402.i = phi float [ undef, %postload2398.i ], [ %vload1244.i, %preload2400.i ]
  %vpack1245.i = insertelement <16 x float> %vpack1241.i, float %phi2402.i, i32 8
  br i1 %exmask1182.i, label %preload2403.i, label %postload2404.i

preload2403.i:                                    ; preds = %postload2401.i
  %"&(pSB[currWI].offset)3550.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3551.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3550.i"
  %CastToValueType3552.i = bitcast i8* %"&pSB[currWI].offset3551.i" to i64*
  %loadedValue3553.i = load i64* %CastToValueType3552.i, align 8
  %.sum2748.i = add i64 %loadedValue3553.i, 9
  %310 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2748.i
  %vload1248.i = load float addrspace(3)* %310, align 4
  br label %postload2404.i

postload2404.i:                                   ; preds = %preload2403.i, %postload2401.i
  %phi2405.i = phi float [ undef, %postload2401.i ], [ %vload1248.i, %preload2403.i ]
  %vpack1249.i = insertelement <16 x float> %vpack1245.i, float %phi2405.i, i32 9
  br i1 %exmask1186.i, label %preload2406.i, label %postload2407.i

preload2406.i:                                    ; preds = %postload2404.i
  %"&(pSB[currWI].offset)3555.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3556.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3555.i"
  %CastToValueType3557.i = bitcast i8* %"&pSB[currWI].offset3556.i" to i64*
  %loadedValue3558.i = load i64* %CastToValueType3557.i, align 8
  %.sum2747.i = add i64 %loadedValue3558.i, 10
  %311 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2747.i
  %vload1252.i = load float addrspace(3)* %311, align 4
  br label %postload2407.i

postload2407.i:                                   ; preds = %preload2406.i, %postload2404.i
  %phi2408.i = phi float [ undef, %postload2404.i ], [ %vload1252.i, %preload2406.i ]
  %vpack1253.i = insertelement <16 x float> %vpack1249.i, float %phi2408.i, i32 10
  br i1 %exmask1190.i, label %preload2409.i, label %postload2410.i

preload2409.i:                                    ; preds = %postload2407.i
  %"&(pSB[currWI].offset)3560.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3561.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3560.i"
  %CastToValueType3562.i = bitcast i8* %"&pSB[currWI].offset3561.i" to i64*
  %loadedValue3563.i = load i64* %CastToValueType3562.i, align 8
  %.sum2746.i = add i64 %loadedValue3563.i, 11
  %312 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2746.i
  %vload1256.i = load float addrspace(3)* %312, align 4
  br label %postload2410.i

postload2410.i:                                   ; preds = %preload2409.i, %postload2407.i
  %phi2411.i = phi float [ undef, %postload2407.i ], [ %vload1256.i, %preload2409.i ]
  %vpack1257.i = insertelement <16 x float> %vpack1253.i, float %phi2411.i, i32 11
  br i1 %exmask1194.i, label %preload2412.i, label %postload2413.i

preload2412.i:                                    ; preds = %postload2410.i
  %"&(pSB[currWI].offset)3565.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3566.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3565.i"
  %CastToValueType3567.i = bitcast i8* %"&pSB[currWI].offset3566.i" to i64*
  %loadedValue3568.i = load i64* %CastToValueType3567.i, align 8
  %.sum2745.i = add i64 %loadedValue3568.i, 12
  %313 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2745.i
  %vload1260.i = load float addrspace(3)* %313, align 4
  br label %postload2413.i

postload2413.i:                                   ; preds = %preload2412.i, %postload2410.i
  %phi2414.i = phi float [ undef, %postload2410.i ], [ %vload1260.i, %preload2412.i ]
  %vpack1261.i = insertelement <16 x float> %vpack1257.i, float %phi2414.i, i32 12
  br i1 %exmask1198.i, label %preload2415.i, label %postload2416.i

preload2415.i:                                    ; preds = %postload2413.i
  %"&(pSB[currWI].offset)3570.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3571.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3570.i"
  %CastToValueType3572.i = bitcast i8* %"&pSB[currWI].offset3571.i" to i64*
  %loadedValue3573.i = load i64* %CastToValueType3572.i, align 8
  %.sum2744.i = add i64 %loadedValue3573.i, 13
  %314 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2744.i
  %vload1264.i = load float addrspace(3)* %314, align 4
  br label %postload2416.i

postload2416.i:                                   ; preds = %preload2415.i, %postload2413.i
  %phi2417.i = phi float [ undef, %postload2413.i ], [ %vload1264.i, %preload2415.i ]
  %vpack1265.i = insertelement <16 x float> %vpack1261.i, float %phi2417.i, i32 13
  br i1 %exmask1202.i, label %preload2418.i, label %postload2419.i

preload2418.i:                                    ; preds = %postload2416.i
  %"&(pSB[currWI].offset)3575.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3576.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3575.i"
  %CastToValueType3577.i = bitcast i8* %"&pSB[currWI].offset3576.i" to i64*
  %loadedValue3578.i = load i64* %CastToValueType3577.i, align 8
  %.sum2743.i = add i64 %loadedValue3578.i, 14
  %315 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2743.i
  %vload1268.i = load float addrspace(3)* %315, align 4
  br label %postload2419.i

postload2419.i:                                   ; preds = %preload2418.i, %postload2416.i
  %phi2420.i = phi float [ undef, %postload2416.i ], [ %vload1268.i, %preload2418.i ]
  %vpack1269.i = insertelement <16 x float> %vpack1265.i, float %phi2420.i, i32 14
  br i1 %exmask1206.i, label %preload2421.i, label %postload2422.i

preload2421.i:                                    ; preds = %postload2419.i
  %"&(pSB[currWI].offset)3580.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3581.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3580.i"
  %CastToValueType3582.i = bitcast i8* %"&pSB[currWI].offset3581.i" to i64*
  %loadedValue3583.i = load i64* %CastToValueType3582.i, align 8
  %.sum2742.i = add i64 %loadedValue3583.i, 15
  %316 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2742.i
  %vload1272.i = load float addrspace(3)* %316, align 4
  br label %postload2422.i

postload2422.i:                                   ; preds = %preload2421.i, %postload2419.i
  %phi2423.i = phi float [ undef, %postload2419.i ], [ %vload1272.i, %preload2421.i ]
  %vpack1273.i = insertelement <16 x float> %vpack1269.i, float %phi2423.i, i32 15
  %add65425.i = fadd <16 x float> %vpack1273.i, %vpack1208.i
  br i1 %exmask1146.i, label %preload2424.i, label %postload2425.i

preload2424.i:                                    ; preds = %postload2422.i
  %exData1277.i = extractelement <16 x float> %add65425.i, i32 0
  %"&(pSB[currWI].offset)3929.i" = add nuw i64 %CurrSBIndex..8.i, 80
  %"&pSB[currWI].offset3930.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3929.i"
  %CastToValueType3931.i = bitcast i8* %"&pSB[currWI].offset3930.i" to float addrspace(3)**
  %loadedValue3932.i = load float addrspace(3)** %CastToValueType3931.i, align 8
  store float %exData1277.i, float addrspace(3)* %loadedValue3932.i, align 4
  br label %postload2425.i

postload2425.i:                                   ; preds = %preload2424.i, %postload2422.i
  br i1 %exmask1150.i, label %preload2427.i, label %postload2428.i

preload2427.i:                                    ; preds = %postload2425.i
  %"&(pSB[currWI].offset)3585.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3586.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3585.i"
  %CastToValueType3587.i = bitcast i8* %"&pSB[currWI].offset3586.i" to i64*
  %loadedValue3588.i = load i64* %CastToValueType3587.i, align 8
  %.sum2741.i = add i64 %loadedValue3588.i, 1
  %317 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2741.i
  %exData1280.i = extractelement <16 x float> %add65425.i, i32 1
  store float %exData1280.i, float addrspace(3)* %317, align 4
  br label %postload2428.i

postload2428.i:                                   ; preds = %preload2427.i, %postload2425.i
  br i1 %exmask1154.i, label %preload2430.i, label %postload2431.i

preload2430.i:                                    ; preds = %postload2428.i
  %"&(pSB[currWI].offset)3590.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3591.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3590.i"
  %CastToValueType3592.i = bitcast i8* %"&pSB[currWI].offset3591.i" to i64*
  %loadedValue3593.i = load i64* %CastToValueType3592.i, align 8
  %.sum2740.i = add i64 %loadedValue3593.i, 2
  %318 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2740.i
  %exData1283.i = extractelement <16 x float> %add65425.i, i32 2
  store float %exData1283.i, float addrspace(3)* %318, align 4
  br label %postload2431.i

postload2431.i:                                   ; preds = %preload2430.i, %postload2428.i
  br i1 %exmask1158.i, label %preload2433.i, label %postload2434.i

preload2433.i:                                    ; preds = %postload2431.i
  %"&(pSB[currWI].offset)3595.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3596.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3595.i"
  %CastToValueType3597.i = bitcast i8* %"&pSB[currWI].offset3596.i" to i64*
  %loadedValue3598.i = load i64* %CastToValueType3597.i, align 8
  %.sum2739.i = add i64 %loadedValue3598.i, 3
  %319 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2739.i
  %exData1286.i = extractelement <16 x float> %add65425.i, i32 3
  store float %exData1286.i, float addrspace(3)* %319, align 4
  br label %postload2434.i

postload2434.i:                                   ; preds = %preload2433.i, %postload2431.i
  br i1 %exmask1162.i, label %preload2436.i, label %postload2437.i

preload2436.i:                                    ; preds = %postload2434.i
  %"&(pSB[currWI].offset)3600.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3601.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3600.i"
  %CastToValueType3602.i = bitcast i8* %"&pSB[currWI].offset3601.i" to i64*
  %loadedValue3603.i = load i64* %CastToValueType3602.i, align 8
  %.sum2738.i = add i64 %loadedValue3603.i, 4
  %320 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2738.i
  %exData1289.i = extractelement <16 x float> %add65425.i, i32 4
  store float %exData1289.i, float addrspace(3)* %320, align 4
  br label %postload2437.i

postload2437.i:                                   ; preds = %preload2436.i, %postload2434.i
  br i1 %exmask1166.i, label %preload2439.i, label %postload2440.i

preload2439.i:                                    ; preds = %postload2437.i
  %"&(pSB[currWI].offset)3605.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3606.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3605.i"
  %CastToValueType3607.i = bitcast i8* %"&pSB[currWI].offset3606.i" to i64*
  %loadedValue3608.i = load i64* %CastToValueType3607.i, align 8
  %.sum2737.i = add i64 %loadedValue3608.i, 5
  %321 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2737.i
  %exData1292.i = extractelement <16 x float> %add65425.i, i32 5
  store float %exData1292.i, float addrspace(3)* %321, align 4
  br label %postload2440.i

postload2440.i:                                   ; preds = %preload2439.i, %postload2437.i
  br i1 %exmask1170.i, label %preload2442.i, label %postload2443.i

preload2442.i:                                    ; preds = %postload2440.i
  %"&(pSB[currWI].offset)3610.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3611.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3610.i"
  %CastToValueType3612.i = bitcast i8* %"&pSB[currWI].offset3611.i" to i64*
  %loadedValue3613.i = load i64* %CastToValueType3612.i, align 8
  %.sum2736.i = add i64 %loadedValue3613.i, 6
  %322 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2736.i
  %exData1295.i = extractelement <16 x float> %add65425.i, i32 6
  store float %exData1295.i, float addrspace(3)* %322, align 4
  br label %postload2443.i

postload2443.i:                                   ; preds = %preload2442.i, %postload2440.i
  br i1 %exmask1174.i, label %preload2445.i, label %postload2446.i

preload2445.i:                                    ; preds = %postload2443.i
  %"&(pSB[currWI].offset)3615.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3616.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3615.i"
  %CastToValueType3617.i = bitcast i8* %"&pSB[currWI].offset3616.i" to i64*
  %loadedValue3618.i = load i64* %CastToValueType3617.i, align 8
  %.sum2735.i = add i64 %loadedValue3618.i, 7
  %323 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2735.i
  %exData1298.i = extractelement <16 x float> %add65425.i, i32 7
  store float %exData1298.i, float addrspace(3)* %323, align 4
  br label %postload2446.i

postload2446.i:                                   ; preds = %preload2445.i, %postload2443.i
  br i1 %exmask1178.i, label %preload2448.i, label %postload2449.i

preload2448.i:                                    ; preds = %postload2446.i
  %"&(pSB[currWI].offset)3620.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3621.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3620.i"
  %CastToValueType3622.i = bitcast i8* %"&pSB[currWI].offset3621.i" to i64*
  %loadedValue3623.i = load i64* %CastToValueType3622.i, align 8
  %.sum2734.i = add i64 %loadedValue3623.i, 8
  %324 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2734.i
  %exData1301.i = extractelement <16 x float> %add65425.i, i32 8
  store float %exData1301.i, float addrspace(3)* %324, align 4
  br label %postload2449.i

postload2449.i:                                   ; preds = %preload2448.i, %postload2446.i
  br i1 %exmask1182.i, label %preload2451.i, label %postload2452.i

preload2451.i:                                    ; preds = %postload2449.i
  %"&(pSB[currWI].offset)3625.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3626.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3625.i"
  %CastToValueType3627.i = bitcast i8* %"&pSB[currWI].offset3626.i" to i64*
  %loadedValue3628.i = load i64* %CastToValueType3627.i, align 8
  %.sum2733.i = add i64 %loadedValue3628.i, 9
  %325 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2733.i
  %exData1304.i = extractelement <16 x float> %add65425.i, i32 9
  store float %exData1304.i, float addrspace(3)* %325, align 4
  br label %postload2452.i

postload2452.i:                                   ; preds = %preload2451.i, %postload2449.i
  br i1 %exmask1186.i, label %preload2454.i, label %postload2455.i

preload2454.i:                                    ; preds = %postload2452.i
  %"&(pSB[currWI].offset)3630.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3631.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3630.i"
  %CastToValueType3632.i = bitcast i8* %"&pSB[currWI].offset3631.i" to i64*
  %loadedValue3633.i = load i64* %CastToValueType3632.i, align 8
  %.sum2732.i = add i64 %loadedValue3633.i, 10
  %326 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2732.i
  %exData1307.i = extractelement <16 x float> %add65425.i, i32 10
  store float %exData1307.i, float addrspace(3)* %326, align 4
  br label %postload2455.i

postload2455.i:                                   ; preds = %preload2454.i, %postload2452.i
  br i1 %exmask1190.i, label %preload2457.i, label %postload2458.i

preload2457.i:                                    ; preds = %postload2455.i
  %"&(pSB[currWI].offset)3635.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3636.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3635.i"
  %CastToValueType3637.i = bitcast i8* %"&pSB[currWI].offset3636.i" to i64*
  %loadedValue3638.i = load i64* %CastToValueType3637.i, align 8
  %.sum2731.i = add i64 %loadedValue3638.i, 11
  %327 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2731.i
  %exData1310.i = extractelement <16 x float> %add65425.i, i32 11
  store float %exData1310.i, float addrspace(3)* %327, align 4
  br label %postload2458.i

postload2458.i:                                   ; preds = %preload2457.i, %postload2455.i
  br i1 %exmask1194.i, label %preload2460.i, label %postload2461.i

preload2460.i:                                    ; preds = %postload2458.i
  %"&(pSB[currWI].offset)3640.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3641.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3640.i"
  %CastToValueType3642.i = bitcast i8* %"&pSB[currWI].offset3641.i" to i64*
  %loadedValue3643.i = load i64* %CastToValueType3642.i, align 8
  %.sum2730.i = add i64 %loadedValue3643.i, 12
  %328 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2730.i
  %exData1313.i = extractelement <16 x float> %add65425.i, i32 12
  store float %exData1313.i, float addrspace(3)* %328, align 4
  br label %postload2461.i

postload2461.i:                                   ; preds = %preload2460.i, %postload2458.i
  br i1 %exmask1198.i, label %preload2463.i, label %postload2464.i

preload2463.i:                                    ; preds = %postload2461.i
  %"&(pSB[currWI].offset)3645.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3646.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3645.i"
  %CastToValueType3647.i = bitcast i8* %"&pSB[currWI].offset3646.i" to i64*
  %loadedValue3648.i = load i64* %CastToValueType3647.i, align 8
  %.sum2729.i = add i64 %loadedValue3648.i, 13
  %329 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2729.i
  %exData1316.i = extractelement <16 x float> %add65425.i, i32 13
  store float %exData1316.i, float addrspace(3)* %329, align 4
  br label %postload2464.i

postload2464.i:                                   ; preds = %preload2463.i, %postload2461.i
  br i1 %exmask1202.i, label %preload2466.i, label %postload2467.i

preload2466.i:                                    ; preds = %postload2464.i
  %"&(pSB[currWI].offset)3650.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3651.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3650.i"
  %CastToValueType3652.i = bitcast i8* %"&pSB[currWI].offset3651.i" to i64*
  %loadedValue3653.i = load i64* %CastToValueType3652.i, align 8
  %.sum2728.i = add i64 %loadedValue3653.i, 14
  %330 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2728.i
  %exData1319.i = extractelement <16 x float> %add65425.i, i32 14
  store float %exData1319.i, float addrspace(3)* %330, align 4
  br label %postload2467.i

postload2467.i:                                   ; preds = %preload2466.i, %postload2464.i
  br i1 %exmask1206.i, label %preload2469.i, label %postload2467.i.if.end66.i_crit_edge

postload2467.i.if.end66.i_crit_edge:              ; preds = %postload2467.i
  br label %if.end66.i

preload2469.i:                                    ; preds = %postload2467.i
  %"&(pSB[currWI].offset)3655.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3656.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3655.i"
  %CastToValueType3657.i = bitcast i8* %"&pSB[currWI].offset3656.i" to i64*
  %loadedValue3658.i = load i64* %CastToValueType3657.i, align 8
  %.sum2727.i = add i64 %loadedValue3658.i, 15
  %331 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2727.i
  %exData1322.i = extractelement <16 x float> %add65425.i, i32 15
  store float %exData1322.i, float addrspace(3)* %331, align 4
  br label %if.end66.i

if.end66.i:                                       ; preds = %postload2467.i.if.end66.i_crit_edge, %preload2469.i
  %"&(pSB[currWI].offset)4002.i" = add nuw i64 %CurrSBIndex..8.i, 98
  %"&pSB[currWI].offset4003.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4002.i"
  %CastToValueType4004.i = bitcast i8* %"&pSB[currWI].offset4003.i" to i1*
  %loadedValue4005.i = load i1* %CastToValueType4004.i, align 1
  br i1 %loadedValue4005.i, label %preload1683.i, label %if.end66.i.postload1684.i_crit_edge

if.end66.i.postload1684.i_crit_edge:              ; preds = %if.end66.i
  br label %postload1684.i

preload1683.i:                                    ; preds = %if.end66.i
  %check.WI.iter4598.i = icmp ult i64 %CurrWI..8.i, %31
  br i1 %check.WI.iter4598.i, label %thenBB4595.i, label %preload1683.i.postload1684.i_crit_edge

preload1683.i.postload1684.i_crit_edge:           ; preds = %preload1683.i
  br label %postload1684.i

thenBB4595.i:                                     ; preds = %preload1683.i
  %"CurrWI++4599.i" = add nuw i64 %CurrWI..8.i, 1
  %"loadedCurrSB+Stride4601.i" = add nuw i64 %CurrSBIndex..8.i, 256
  switch i32 %currBarrier.8.i, label %thenBB4595.i.postload1682.i_crit_edge [
    i32 8, label %thenBB4595.i.postload1680.i_crit_edge
    i32 7, label %thenBB4595.i.postload1678.i_crit_edge
    i32 6, label %thenBB4595.i.postload1676.i_crit_edge
    i32 14, label %SyncBB4569.outer.i
  ]

thenBB4595.i.postload1676.i_crit_edge:            ; preds = %thenBB4595.i
  br label %postload1676.i

thenBB4595.i.postload1678.i_crit_edge:            ; preds = %thenBB4595.i
  br label %postload1678.i

thenBB4595.i.postload1680.i_crit_edge:            ; preds = %thenBB4595.i
  br label %postload1680.i

thenBB4595.i.postload1682.i_crit_edge:            ; preds = %thenBB4595.i
  br label %postload1682.i

postload1684.i:                                   ; preds = %thenBB4611.i.postload1684.i_crit_edge, %thenBB4603.i.postload1684.i_crit_edge, %preload1683.i.postload1684.i_crit_edge, %if.end66.i.postload1684.i_crit_edge
  %CurrWI..10.i = phi i64 [ %CurrWI..8.i, %if.end66.i.postload1684.i_crit_edge ], [ 0, %preload1683.i.postload1684.i_crit_edge ], [ %"CurrWI++4607.i", %thenBB4603.i.postload1684.i_crit_edge ], [ %"CurrWI++4615.i", %thenBB4611.i.postload1684.i_crit_edge ]
  %CurrSBIndex..10.i = phi i64 [ %CurrSBIndex..8.i, %if.end66.i.postload1684.i_crit_edge ], [ 0, %preload1683.i.postload1684.i_crit_edge ], [ %"loadedCurrSB+Stride4609.i", %thenBB4603.i.postload1684.i_crit_edge ], [ %"loadedCurrSB+Stride4617.i", %thenBB4611.i.postload1684.i_crit_edge ]
  %currBarrier.10.i = phi i32 [ %currBarrier.8.i, %if.end66.i.postload1684.i_crit_edge ], [ 10, %preload1683.i.postload1684.i_crit_edge ], [ %currBarrier.10.i, %thenBB4603.i.postload1684.i_crit_edge ], [ %currBarrier.12.i, %thenBB4611.i.postload1684.i_crit_edge ]
  %"&pSB[currWI].offset2924.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..10.i
  %CastToValueType2925.i = bitcast i8* %"&pSB[currWI].offset2924.i" to <16 x i32>*
  %loadedValue.i = load <16 x i32>* %CastToValueType2925.i, align 64
  %cmp67.i = icmp eq <16 x i32> %loadedValue.i, zeroinitializer
  %"&(pSB[currWI].offset)3983.i" = add nuw i64 %CurrSBIndex..10.i, 96
  %"&pSB[currWI].offset3984.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3983.i"
  %CastToValueType3985.i = bitcast i8* %"&pSB[currWI].offset3984.i" to <16 x i1>*
  %loadedValue3986.i = load <16 x i1>* %CastToValueType3985.i, align 16
  %if.end66_to_if.then69427.i = and <16 x i1> %loadedValue3986.i, %cmp67.i
  %"&(pSB[currWI].offset)2956.i" = add nuw i64 %CurrSBIndex..10.i, 64
  %"&pSB[currWI].offset2957.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)2956.i"
  %CastToValueType2958.i = bitcast i8* %"&pSB[currWI].offset2957.i" to i32*
  %loadedValue2959.i = load i32* %CastToValueType2958.i, align 4
  %332 = add i32 %loadedValue2959.i, 1
  %extract430.i = sext i32 %332 to i64
  %exmask1325.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 0
  %"&(pSB[currWI].offset)4181.i" = add nuw i64 %CurrSBIndex..10.i, 232
  %"&pSB[currWI].offset4182.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4181.i"
  %CastToValueType4183.i = bitcast i8* %"&pSB[currWI].offset4182.i" to i1*
  store i1 %exmask1325.i, i1* %CastToValueType4183.i, align 1
  br i1 %exmask1325.i, label %preload2472.i, label %postload2473.i

preload2472.i:                                    ; preds = %postload1684.i
  %333 = getelementptr inbounds [128 x float] addrspace(3)* %35, i64 0, i64 %extract430.i
  %vload1326.i = load float addrspace(3)* %333, align 4
  br label %postload2473.i

postload2473.i:                                   ; preds = %preload2472.i, %postload1684.i
  %phi2474.i = phi float [ undef, %postload1684.i ], [ %vload1326.i, %preload2472.i ]
  %vpack1327.i = insertelement <16 x float> undef, float %phi2474.i, i32 0
  %exmask1329.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 1
  %"&(pSB[currWI].offset)4205.i" = add nuw i64 %CurrSBIndex..10.i, 233
  %"&pSB[currWI].offset4206.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4205.i"
  %CastToValueType4207.i = bitcast i8* %"&pSB[currWI].offset4206.i" to i1*
  store i1 %exmask1329.i, i1* %CastToValueType4207.i, align 1
  br i1 %exmask1329.i, label %preload2475.i, label %postload2476.i

preload2475.i:                                    ; preds = %postload2473.i
  %.sum2726.i = add i64 %extract430.i, 1
  %334 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2726.i
  %vload1330.i = load float addrspace(3)* %334, align 4
  br label %postload2476.i

postload2476.i:                                   ; preds = %preload2475.i, %postload2473.i
  %phi2477.i = phi float [ undef, %postload2473.i ], [ %vload1330.i, %preload2475.i ]
  %vpack1331.i = insertelement <16 x float> %vpack1327.i, float %phi2477.i, i32 1
  %exmask1333.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 2
  %"&(pSB[currWI].offset)4229.i" = add nuw i64 %CurrSBIndex..10.i, 234
  %"&pSB[currWI].offset4230.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4229.i"
  %CastToValueType4231.i = bitcast i8* %"&pSB[currWI].offset4230.i" to i1*
  store i1 %exmask1333.i, i1* %CastToValueType4231.i, align 1
  br i1 %exmask1333.i, label %preload2478.i, label %postload2479.i

preload2478.i:                                    ; preds = %postload2476.i
  %.sum2725.i = add i64 %extract430.i, 2
  %335 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2725.i
  %vload1334.i = load float addrspace(3)* %335, align 4
  br label %postload2479.i

postload2479.i:                                   ; preds = %preload2478.i, %postload2476.i
  %phi2480.i = phi float [ undef, %postload2476.i ], [ %vload1334.i, %preload2478.i ]
  %vpack1335.i = insertelement <16 x float> %vpack1331.i, float %phi2480.i, i32 2
  %exmask1337.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 3
  %"&(pSB[currWI].offset)4253.i" = add nuw i64 %CurrSBIndex..10.i, 235
  %"&pSB[currWI].offset4254.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4253.i"
  %CastToValueType4255.i = bitcast i8* %"&pSB[currWI].offset4254.i" to i1*
  store i1 %exmask1337.i, i1* %CastToValueType4255.i, align 1
  br i1 %exmask1337.i, label %preload2481.i, label %postload2482.i

preload2481.i:                                    ; preds = %postload2479.i
  %.sum2724.i = add i64 %extract430.i, 3
  %336 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2724.i
  %vload1338.i = load float addrspace(3)* %336, align 4
  br label %postload2482.i

postload2482.i:                                   ; preds = %preload2481.i, %postload2479.i
  %phi2483.i = phi float [ undef, %postload2479.i ], [ %vload1338.i, %preload2481.i ]
  %vpack1339.i = insertelement <16 x float> %vpack1335.i, float %phi2483.i, i32 3
  %exmask1341.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 4
  %"&(pSB[currWI].offset)4277.i" = add nuw i64 %CurrSBIndex..10.i, 236
  %"&pSB[currWI].offset4278.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4277.i"
  %CastToValueType4279.i = bitcast i8* %"&pSB[currWI].offset4278.i" to i1*
  store i1 %exmask1341.i, i1* %CastToValueType4279.i, align 1
  br i1 %exmask1341.i, label %preload2484.i, label %postload2485.i

preload2484.i:                                    ; preds = %postload2482.i
  %.sum2723.i = add i64 %extract430.i, 4
  %337 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2723.i
  %vload1342.i = load float addrspace(3)* %337, align 4
  br label %postload2485.i

postload2485.i:                                   ; preds = %preload2484.i, %postload2482.i
  %phi2486.i = phi float [ undef, %postload2482.i ], [ %vload1342.i, %preload2484.i ]
  %vpack1343.i = insertelement <16 x float> %vpack1339.i, float %phi2486.i, i32 4
  %exmask1345.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 5
  %"&(pSB[currWI].offset)4301.i" = add nuw i64 %CurrSBIndex..10.i, 237
  %"&pSB[currWI].offset4302.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4301.i"
  %CastToValueType4303.i = bitcast i8* %"&pSB[currWI].offset4302.i" to i1*
  store i1 %exmask1345.i, i1* %CastToValueType4303.i, align 1
  br i1 %exmask1345.i, label %preload2487.i, label %postload2488.i

preload2487.i:                                    ; preds = %postload2485.i
  %.sum2722.i = add i64 %extract430.i, 5
  %338 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2722.i
  %vload1346.i = load float addrspace(3)* %338, align 4
  br label %postload2488.i

postload2488.i:                                   ; preds = %preload2487.i, %postload2485.i
  %phi2489.i = phi float [ undef, %postload2485.i ], [ %vload1346.i, %preload2487.i ]
  %vpack1347.i = insertelement <16 x float> %vpack1343.i, float %phi2489.i, i32 5
  %exmask1349.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 6
  %"&(pSB[currWI].offset)4325.i" = add nuw i64 %CurrSBIndex..10.i, 238
  %"&pSB[currWI].offset4326.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4325.i"
  %CastToValueType4327.i = bitcast i8* %"&pSB[currWI].offset4326.i" to i1*
  store i1 %exmask1349.i, i1* %CastToValueType4327.i, align 1
  br i1 %exmask1349.i, label %preload2490.i, label %postload2491.i

preload2490.i:                                    ; preds = %postload2488.i
  %.sum2721.i = add i64 %extract430.i, 6
  %339 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2721.i
  %vload1350.i = load float addrspace(3)* %339, align 4
  br label %postload2491.i

postload2491.i:                                   ; preds = %preload2490.i, %postload2488.i
  %phi2492.i = phi float [ undef, %postload2488.i ], [ %vload1350.i, %preload2490.i ]
  %vpack1351.i = insertelement <16 x float> %vpack1347.i, float %phi2492.i, i32 6
  %exmask1353.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 7
  %"&(pSB[currWI].offset)4349.i" = add nuw i64 %CurrSBIndex..10.i, 239
  %"&pSB[currWI].offset4350.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4349.i"
  %CastToValueType4351.i = bitcast i8* %"&pSB[currWI].offset4350.i" to i1*
  store i1 %exmask1353.i, i1* %CastToValueType4351.i, align 1
  br i1 %exmask1353.i, label %preload2493.i, label %postload2494.i

preload2493.i:                                    ; preds = %postload2491.i
  %.sum2720.i = add i64 %extract430.i, 7
  %340 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2720.i
  %vload1354.i = load float addrspace(3)* %340, align 4
  br label %postload2494.i

postload2494.i:                                   ; preds = %preload2493.i, %postload2491.i
  %phi2495.i = phi float [ undef, %postload2491.i ], [ %vload1354.i, %preload2493.i ]
  %vpack1355.i = insertelement <16 x float> %vpack1351.i, float %phi2495.i, i32 7
  %exmask1357.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 8
  %"&(pSB[currWI].offset)4373.i" = add nuw i64 %CurrSBIndex..10.i, 240
  %"&pSB[currWI].offset4374.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4373.i"
  %CastToValueType4375.i = bitcast i8* %"&pSB[currWI].offset4374.i" to i1*
  store i1 %exmask1357.i, i1* %CastToValueType4375.i, align 1
  br i1 %exmask1357.i, label %preload2496.i, label %postload2497.i

preload2496.i:                                    ; preds = %postload2494.i
  %.sum2719.i = add i64 %extract430.i, 8
  %341 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2719.i
  %vload1358.i = load float addrspace(3)* %341, align 4
  br label %postload2497.i

postload2497.i:                                   ; preds = %preload2496.i, %postload2494.i
  %phi2498.i = phi float [ undef, %postload2494.i ], [ %vload1358.i, %preload2496.i ]
  %vpack1359.i = insertelement <16 x float> %vpack1355.i, float %phi2498.i, i32 8
  %exmask1361.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 9
  %"&(pSB[currWI].offset)4397.i" = add nuw i64 %CurrSBIndex..10.i, 241
  %"&pSB[currWI].offset4398.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4397.i"
  %CastToValueType4399.i = bitcast i8* %"&pSB[currWI].offset4398.i" to i1*
  store i1 %exmask1361.i, i1* %CastToValueType4399.i, align 1
  br i1 %exmask1361.i, label %preload2499.i, label %postload2500.i

preload2499.i:                                    ; preds = %postload2497.i
  %.sum2718.i = add i64 %extract430.i, 9
  %342 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2718.i
  %vload1362.i = load float addrspace(3)* %342, align 4
  br label %postload2500.i

postload2500.i:                                   ; preds = %preload2499.i, %postload2497.i
  %phi2501.i = phi float [ undef, %postload2497.i ], [ %vload1362.i, %preload2499.i ]
  %vpack1363.i = insertelement <16 x float> %vpack1359.i, float %phi2501.i, i32 9
  %exmask1365.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 10
  %"&(pSB[currWI].offset)4421.i" = add nuw i64 %CurrSBIndex..10.i, 242
  %"&pSB[currWI].offset4422.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4421.i"
  %CastToValueType4423.i = bitcast i8* %"&pSB[currWI].offset4422.i" to i1*
  store i1 %exmask1365.i, i1* %CastToValueType4423.i, align 1
  br i1 %exmask1365.i, label %preload2502.i, label %postload2503.i

preload2502.i:                                    ; preds = %postload2500.i
  %.sum2717.i = add i64 %extract430.i, 10
  %343 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2717.i
  %vload1366.i = load float addrspace(3)* %343, align 4
  br label %postload2503.i

postload2503.i:                                   ; preds = %preload2502.i, %postload2500.i
  %phi2504.i = phi float [ undef, %postload2500.i ], [ %vload1366.i, %preload2502.i ]
  %vpack1367.i = insertelement <16 x float> %vpack1363.i, float %phi2504.i, i32 10
  %exmask1369.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 11
  %"&(pSB[currWI].offset)4445.i" = add nuw i64 %CurrSBIndex..10.i, 243
  %"&pSB[currWI].offset4446.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4445.i"
  %CastToValueType4447.i = bitcast i8* %"&pSB[currWI].offset4446.i" to i1*
  store i1 %exmask1369.i, i1* %CastToValueType4447.i, align 1
  br i1 %exmask1369.i, label %preload2505.i, label %postload2506.i

preload2505.i:                                    ; preds = %postload2503.i
  %.sum2716.i = add i64 %extract430.i, 11
  %344 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2716.i
  %vload1370.i = load float addrspace(3)* %344, align 4
  br label %postload2506.i

postload2506.i:                                   ; preds = %preload2505.i, %postload2503.i
  %phi2507.i = phi float [ undef, %postload2503.i ], [ %vload1370.i, %preload2505.i ]
  %vpack1371.i = insertelement <16 x float> %vpack1367.i, float %phi2507.i, i32 11
  %exmask1373.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 12
  %"&(pSB[currWI].offset)4469.i" = add nuw i64 %CurrSBIndex..10.i, 244
  %"&pSB[currWI].offset4470.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4469.i"
  %CastToValueType4471.i = bitcast i8* %"&pSB[currWI].offset4470.i" to i1*
  store i1 %exmask1373.i, i1* %CastToValueType4471.i, align 1
  br i1 %exmask1373.i, label %preload2508.i, label %postload2509.i

preload2508.i:                                    ; preds = %postload2506.i
  %.sum2715.i = add i64 %extract430.i, 12
  %345 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2715.i
  %vload1374.i = load float addrspace(3)* %345, align 4
  br label %postload2509.i

postload2509.i:                                   ; preds = %preload2508.i, %postload2506.i
  %phi2510.i = phi float [ undef, %postload2506.i ], [ %vload1374.i, %preload2508.i ]
  %vpack1375.i = insertelement <16 x float> %vpack1371.i, float %phi2510.i, i32 12
  %exmask1377.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 13
  %"&(pSB[currWI].offset)4493.i" = add nuw i64 %CurrSBIndex..10.i, 245
  %"&pSB[currWI].offset4494.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4493.i"
  %CastToValueType4495.i = bitcast i8* %"&pSB[currWI].offset4494.i" to i1*
  store i1 %exmask1377.i, i1* %CastToValueType4495.i, align 1
  br i1 %exmask1377.i, label %preload2511.i, label %postload2512.i

preload2511.i:                                    ; preds = %postload2509.i
  %.sum2714.i = add i64 %extract430.i, 13
  %346 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2714.i
  %vload1378.i = load float addrspace(3)* %346, align 4
  br label %postload2512.i

postload2512.i:                                   ; preds = %preload2511.i, %postload2509.i
  %phi2513.i = phi float [ undef, %postload2509.i ], [ %vload1378.i, %preload2511.i ]
  %vpack1379.i = insertelement <16 x float> %vpack1375.i, float %phi2513.i, i32 13
  %exmask1381.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 14
  %"&(pSB[currWI].offset)4517.i" = add nuw i64 %CurrSBIndex..10.i, 246
  %"&pSB[currWI].offset4518.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4517.i"
  %CastToValueType4519.i = bitcast i8* %"&pSB[currWI].offset4518.i" to i1*
  store i1 %exmask1381.i, i1* %CastToValueType4519.i, align 1
  br i1 %exmask1381.i, label %preload2514.i, label %postload2515.i

preload2514.i:                                    ; preds = %postload2512.i
  %.sum2713.i = add i64 %extract430.i, 14
  %347 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2713.i
  %vload1382.i = load float addrspace(3)* %347, align 4
  br label %postload2515.i

postload2515.i:                                   ; preds = %preload2514.i, %postload2512.i
  %phi2516.i = phi float [ undef, %postload2512.i ], [ %vload1382.i, %preload2514.i ]
  %vpack1383.i = insertelement <16 x float> %vpack1379.i, float %phi2516.i, i32 14
  %exmask1385.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 15
  %"&(pSB[currWI].offset)4541.i" = add nuw i64 %CurrSBIndex..10.i, 247
  %"&pSB[currWI].offset4542.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4541.i"
  %CastToValueType4543.i = bitcast i8* %"&pSB[currWI].offset4542.i" to i1*
  store i1 %exmask1385.i, i1* %CastToValueType4543.i, align 1
  br i1 %exmask1385.i, label %preload2517.i, label %postload2518.i

preload2517.i:                                    ; preds = %postload2515.i
  %.sum2712.i = add i64 %extract430.i, 15
  %348 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2712.i
  %vload1386.i = load float addrspace(3)* %348, align 4
  br label %postload2518.i

postload2518.i:                                   ; preds = %preload2517.i, %postload2515.i
  %phi2519.i = phi float [ undef, %postload2515.i ], [ %vload1386.i, %preload2517.i ]
  %vpack1387.i = insertelement <16 x float> %vpack1383.i, float %phi2519.i, i32 15
  br i1 %exmask1325.i, label %preload2520.i, label %postload2521.i

preload2520.i:                                    ; preds = %postload2518.i
  %"&(pSB[currWI].offset)3934.i" = add nuw i64 %CurrSBIndex..10.i, 80
  %"&pSB[currWI].offset3935.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3934.i"
  %CastToValueType3936.i = bitcast i8* %"&pSB[currWI].offset3935.i" to float addrspace(3)**
  %loadedValue3937.i = load float addrspace(3)** %CastToValueType3936.i, align 8
  %vload1391.i = load float addrspace(3)* %loadedValue3937.i, align 4
  br label %postload2521.i

postload2521.i:                                   ; preds = %preload2520.i, %postload2518.i
  %phi2522.i = phi float [ undef, %postload2518.i ], [ %vload1391.i, %preload2520.i ]
  %vpack1392.i = insertelement <16 x float> undef, float %phi2522.i, i32 0
  br i1 %exmask1329.i, label %preload2523.i, label %postload2524.i

preload2523.i:                                    ; preds = %postload2521.i
  %"&(pSB[currWI].offset)3660.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3661.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3660.i"
  %CastToValueType3662.i = bitcast i8* %"&pSB[currWI].offset3661.i" to i64*
  %loadedValue3663.i = load i64* %CastToValueType3662.i, align 8
  %.sum2711.i = add i64 %loadedValue3663.i, 1
  %349 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2711.i
  %vload1395.i = load float addrspace(3)* %349, align 4
  br label %postload2524.i

postload2524.i:                                   ; preds = %preload2523.i, %postload2521.i
  %phi2525.i = phi float [ undef, %postload2521.i ], [ %vload1395.i, %preload2523.i ]
  %vpack1396.i = insertelement <16 x float> %vpack1392.i, float %phi2525.i, i32 1
  br i1 %exmask1333.i, label %preload2526.i, label %postload2527.i

preload2526.i:                                    ; preds = %postload2524.i
  %"&(pSB[currWI].offset)3665.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3666.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3665.i"
  %CastToValueType3667.i = bitcast i8* %"&pSB[currWI].offset3666.i" to i64*
  %loadedValue3668.i = load i64* %CastToValueType3667.i, align 8
  %.sum2710.i = add i64 %loadedValue3668.i, 2
  %350 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2710.i
  %vload1399.i = load float addrspace(3)* %350, align 4
  br label %postload2527.i

postload2527.i:                                   ; preds = %preload2526.i, %postload2524.i
  %phi2528.i = phi float [ undef, %postload2524.i ], [ %vload1399.i, %preload2526.i ]
  %vpack1400.i = insertelement <16 x float> %vpack1396.i, float %phi2528.i, i32 2
  br i1 %exmask1337.i, label %preload2529.i, label %postload2530.i

preload2529.i:                                    ; preds = %postload2527.i
  %"&(pSB[currWI].offset)3670.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3671.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3670.i"
  %CastToValueType3672.i = bitcast i8* %"&pSB[currWI].offset3671.i" to i64*
  %loadedValue3673.i = load i64* %CastToValueType3672.i, align 8
  %.sum2709.i = add i64 %loadedValue3673.i, 3
  %351 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2709.i
  %vload1403.i = load float addrspace(3)* %351, align 4
  br label %postload2530.i

postload2530.i:                                   ; preds = %preload2529.i, %postload2527.i
  %phi2531.i = phi float [ undef, %postload2527.i ], [ %vload1403.i, %preload2529.i ]
  %vpack1404.i = insertelement <16 x float> %vpack1400.i, float %phi2531.i, i32 3
  br i1 %exmask1341.i, label %preload2532.i, label %postload2533.i

preload2532.i:                                    ; preds = %postload2530.i
  %"&(pSB[currWI].offset)3675.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3676.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3675.i"
  %CastToValueType3677.i = bitcast i8* %"&pSB[currWI].offset3676.i" to i64*
  %loadedValue3678.i = load i64* %CastToValueType3677.i, align 8
  %.sum2708.i = add i64 %loadedValue3678.i, 4
  %352 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2708.i
  %vload1407.i = load float addrspace(3)* %352, align 4
  br label %postload2533.i

postload2533.i:                                   ; preds = %preload2532.i, %postload2530.i
  %phi2534.i = phi float [ undef, %postload2530.i ], [ %vload1407.i, %preload2532.i ]
  %vpack1408.i = insertelement <16 x float> %vpack1404.i, float %phi2534.i, i32 4
  br i1 %exmask1345.i, label %preload2535.i, label %postload2536.i

preload2535.i:                                    ; preds = %postload2533.i
  %"&(pSB[currWI].offset)3680.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3681.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3680.i"
  %CastToValueType3682.i = bitcast i8* %"&pSB[currWI].offset3681.i" to i64*
  %loadedValue3683.i = load i64* %CastToValueType3682.i, align 8
  %.sum2707.i = add i64 %loadedValue3683.i, 5
  %353 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2707.i
  %vload1411.i = load float addrspace(3)* %353, align 4
  br label %postload2536.i

postload2536.i:                                   ; preds = %preload2535.i, %postload2533.i
  %phi2537.i = phi float [ undef, %postload2533.i ], [ %vload1411.i, %preload2535.i ]
  %vpack1412.i = insertelement <16 x float> %vpack1408.i, float %phi2537.i, i32 5
  br i1 %exmask1349.i, label %preload2538.i, label %postload2539.i

preload2538.i:                                    ; preds = %postload2536.i
  %"&(pSB[currWI].offset)3685.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3686.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3685.i"
  %CastToValueType3687.i = bitcast i8* %"&pSB[currWI].offset3686.i" to i64*
  %loadedValue3688.i = load i64* %CastToValueType3687.i, align 8
  %.sum2706.i = add i64 %loadedValue3688.i, 6
  %354 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2706.i
  %vload1415.i = load float addrspace(3)* %354, align 4
  br label %postload2539.i

postload2539.i:                                   ; preds = %preload2538.i, %postload2536.i
  %phi2540.i = phi float [ undef, %postload2536.i ], [ %vload1415.i, %preload2538.i ]
  %vpack1416.i = insertelement <16 x float> %vpack1412.i, float %phi2540.i, i32 6
  br i1 %exmask1353.i, label %preload2541.i, label %postload2542.i

preload2541.i:                                    ; preds = %postload2539.i
  %"&(pSB[currWI].offset)3690.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3691.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3690.i"
  %CastToValueType3692.i = bitcast i8* %"&pSB[currWI].offset3691.i" to i64*
  %loadedValue3693.i = load i64* %CastToValueType3692.i, align 8
  %.sum2705.i = add i64 %loadedValue3693.i, 7
  %355 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2705.i
  %vload1419.i = load float addrspace(3)* %355, align 4
  br label %postload2542.i

postload2542.i:                                   ; preds = %preload2541.i, %postload2539.i
  %phi2543.i = phi float [ undef, %postload2539.i ], [ %vload1419.i, %preload2541.i ]
  %vpack1420.i = insertelement <16 x float> %vpack1416.i, float %phi2543.i, i32 7
  br i1 %exmask1357.i, label %preload2544.i, label %postload2545.i

preload2544.i:                                    ; preds = %postload2542.i
  %"&(pSB[currWI].offset)3695.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3696.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3695.i"
  %CastToValueType3697.i = bitcast i8* %"&pSB[currWI].offset3696.i" to i64*
  %loadedValue3698.i = load i64* %CastToValueType3697.i, align 8
  %.sum2704.i = add i64 %loadedValue3698.i, 8
  %356 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2704.i
  %vload1423.i = load float addrspace(3)* %356, align 4
  br label %postload2545.i

postload2545.i:                                   ; preds = %preload2544.i, %postload2542.i
  %phi2546.i = phi float [ undef, %postload2542.i ], [ %vload1423.i, %preload2544.i ]
  %vpack1424.i = insertelement <16 x float> %vpack1420.i, float %phi2546.i, i32 8
  br i1 %exmask1361.i, label %preload2547.i, label %postload2548.i

preload2547.i:                                    ; preds = %postload2545.i
  %"&(pSB[currWI].offset)3700.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3701.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3700.i"
  %CastToValueType3702.i = bitcast i8* %"&pSB[currWI].offset3701.i" to i64*
  %loadedValue3703.i = load i64* %CastToValueType3702.i, align 8
  %.sum2703.i = add i64 %loadedValue3703.i, 9
  %357 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2703.i
  %vload1427.i = load float addrspace(3)* %357, align 4
  br label %postload2548.i

postload2548.i:                                   ; preds = %preload2547.i, %postload2545.i
  %phi2549.i = phi float [ undef, %postload2545.i ], [ %vload1427.i, %preload2547.i ]
  %vpack1428.i = insertelement <16 x float> %vpack1424.i, float %phi2549.i, i32 9
  br i1 %exmask1365.i, label %preload2550.i, label %postload2551.i

preload2550.i:                                    ; preds = %postload2548.i
  %"&(pSB[currWI].offset)3705.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3706.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3705.i"
  %CastToValueType3707.i = bitcast i8* %"&pSB[currWI].offset3706.i" to i64*
  %loadedValue3708.i = load i64* %CastToValueType3707.i, align 8
  %.sum2702.i = add i64 %loadedValue3708.i, 10
  %358 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2702.i
  %vload1431.i = load float addrspace(3)* %358, align 4
  br label %postload2551.i

postload2551.i:                                   ; preds = %preload2550.i, %postload2548.i
  %phi2552.i = phi float [ undef, %postload2548.i ], [ %vload1431.i, %preload2550.i ]
  %vpack1432.i = insertelement <16 x float> %vpack1428.i, float %phi2552.i, i32 10
  br i1 %exmask1369.i, label %preload2553.i, label %postload2554.i

preload2553.i:                                    ; preds = %postload2551.i
  %"&(pSB[currWI].offset)3710.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3711.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3710.i"
  %CastToValueType3712.i = bitcast i8* %"&pSB[currWI].offset3711.i" to i64*
  %loadedValue3713.i = load i64* %CastToValueType3712.i, align 8
  %.sum2701.i = add i64 %loadedValue3713.i, 11
  %359 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2701.i
  %vload1435.i = load float addrspace(3)* %359, align 4
  br label %postload2554.i

postload2554.i:                                   ; preds = %preload2553.i, %postload2551.i
  %phi2555.i = phi float [ undef, %postload2551.i ], [ %vload1435.i, %preload2553.i ]
  %vpack1436.i = insertelement <16 x float> %vpack1432.i, float %phi2555.i, i32 11
  br i1 %exmask1373.i, label %preload2556.i, label %postload2557.i

preload2556.i:                                    ; preds = %postload2554.i
  %"&(pSB[currWI].offset)3715.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3716.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3715.i"
  %CastToValueType3717.i = bitcast i8* %"&pSB[currWI].offset3716.i" to i64*
  %loadedValue3718.i = load i64* %CastToValueType3717.i, align 8
  %.sum2700.i = add i64 %loadedValue3718.i, 12
  %360 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2700.i
  %vload1439.i = load float addrspace(3)* %360, align 4
  br label %postload2557.i

postload2557.i:                                   ; preds = %preload2556.i, %postload2554.i
  %phi2558.i = phi float [ undef, %postload2554.i ], [ %vload1439.i, %preload2556.i ]
  %vpack1440.i = insertelement <16 x float> %vpack1436.i, float %phi2558.i, i32 12
  br i1 %exmask1377.i, label %preload2559.i, label %postload2560.i

preload2559.i:                                    ; preds = %postload2557.i
  %"&(pSB[currWI].offset)3720.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3721.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3720.i"
  %CastToValueType3722.i = bitcast i8* %"&pSB[currWI].offset3721.i" to i64*
  %loadedValue3723.i = load i64* %CastToValueType3722.i, align 8
  %.sum2699.i = add i64 %loadedValue3723.i, 13
  %361 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2699.i
  %vload1443.i = load float addrspace(3)* %361, align 4
  br label %postload2560.i

postload2560.i:                                   ; preds = %preload2559.i, %postload2557.i
  %phi2561.i = phi float [ undef, %postload2557.i ], [ %vload1443.i, %preload2559.i ]
  %vpack1444.i = insertelement <16 x float> %vpack1440.i, float %phi2561.i, i32 13
  br i1 %exmask1381.i, label %preload2562.i, label %postload2563.i

preload2562.i:                                    ; preds = %postload2560.i
  %"&(pSB[currWI].offset)3725.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3726.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3725.i"
  %CastToValueType3727.i = bitcast i8* %"&pSB[currWI].offset3726.i" to i64*
  %loadedValue3728.i = load i64* %CastToValueType3727.i, align 8
  %.sum2698.i = add i64 %loadedValue3728.i, 14
  %362 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2698.i
  %vload1447.i = load float addrspace(3)* %362, align 4
  br label %postload2563.i

postload2563.i:                                   ; preds = %preload2562.i, %postload2560.i
  %phi2564.i = phi float [ undef, %postload2560.i ], [ %vload1447.i, %preload2562.i ]
  %vpack1448.i = insertelement <16 x float> %vpack1444.i, float %phi2564.i, i32 14
  br i1 %exmask1385.i, label %preload2565.i, label %postload2566.i

preload2565.i:                                    ; preds = %postload2563.i
  %"&(pSB[currWI].offset)3730.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3731.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3730.i"
  %CastToValueType3732.i = bitcast i8* %"&pSB[currWI].offset3731.i" to i64*
  %loadedValue3733.i = load i64* %CastToValueType3732.i, align 8
  %.sum2697.i = add i64 %loadedValue3733.i, 15
  %363 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2697.i
  %vload1451.i = load float addrspace(3)* %363, align 4
  br label %postload2566.i

postload2566.i:                                   ; preds = %preload2565.i, %postload2563.i
  %phi2567.i = phi float [ undef, %postload2563.i ], [ %vload1451.i, %preload2565.i ]
  %vpack1452.i = insertelement <16 x float> %vpack1448.i, float %phi2567.i, i32 15
  %add75448.i = fadd <16 x float> %vpack1452.i, %vpack1387.i
  br i1 %exmask1325.i, label %preload2568.i, label %postload2569.i

preload2568.i:                                    ; preds = %postload2566.i
  %exData1456.i = extractelement <16 x float> %add75448.i, i32 0
  %"&(pSB[currWI].offset)3939.i" = add nuw i64 %CurrSBIndex..10.i, 80
  %"&pSB[currWI].offset3940.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3939.i"
  %CastToValueType3941.i = bitcast i8* %"&pSB[currWI].offset3940.i" to float addrspace(3)**
  %loadedValue3942.i = load float addrspace(3)** %CastToValueType3941.i, align 8
  store float %exData1456.i, float addrspace(3)* %loadedValue3942.i, align 4
  %loadedValue4222.pre.i = load i1* %CastToValueType4207.i, align 1
  br label %postload2569.i

postload2569.i:                                   ; preds = %preload2568.i, %postload2566.i
  %loadedValue4222.i = phi i1 [ %loadedValue4222.pre.i, %preload2568.i ], [ %exmask1329.i, %postload2566.i ]
  br i1 %loadedValue4222.i, label %preload2571.i, label %postload2569.i.postload2572.i_crit_edge

postload2569.i.postload2572.i_crit_edge:          ; preds = %postload2569.i
  br label %postload2572.i

preload2571.i:                                    ; preds = %postload2569.i
  %"&(pSB[currWI].offset)3735.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3736.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3735.i"
  %CastToValueType3737.i = bitcast i8* %"&pSB[currWI].offset3736.i" to i64*
  %loadedValue3738.i = load i64* %CastToValueType3737.i, align 8
  %.sum2696.i = add i64 %loadedValue3738.i, 1
  %364 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2696.i
  %exData1459.i = extractelement <16 x float> %add75448.i, i32 1
  store float %exData1459.i, float addrspace(3)* %364, align 4
  br label %postload2572.i

postload2572.i:                                   ; preds = %postload2569.i.postload2572.i_crit_edge, %preload2571.i
  %loadedValue4246.i = load i1* %CastToValueType4231.i, align 1
  br i1 %loadedValue4246.i, label %preload2574.i, label %postload2572.i.postload2575.i_crit_edge

postload2572.i.postload2575.i_crit_edge:          ; preds = %postload2572.i
  br label %postload2575.i

preload2574.i:                                    ; preds = %postload2572.i
  %"&(pSB[currWI].offset)3740.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3741.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3740.i"
  %CastToValueType3742.i = bitcast i8* %"&pSB[currWI].offset3741.i" to i64*
  %loadedValue3743.i = load i64* %CastToValueType3742.i, align 8
  %.sum2695.i = add i64 %loadedValue3743.i, 2
  %365 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2695.i
  %exData1462.i = extractelement <16 x float> %add75448.i, i32 2
  store float %exData1462.i, float addrspace(3)* %365, align 4
  br label %postload2575.i

postload2575.i:                                   ; preds = %postload2572.i.postload2575.i_crit_edge, %preload2574.i
  %loadedValue4270.i = load i1* %CastToValueType4255.i, align 1
  br i1 %loadedValue4270.i, label %preload2577.i, label %postload2575.i.postload2578.i_crit_edge

postload2575.i.postload2578.i_crit_edge:          ; preds = %postload2575.i
  br label %postload2578.i

preload2577.i:                                    ; preds = %postload2575.i
  %"&(pSB[currWI].offset)3745.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3746.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3745.i"
  %CastToValueType3747.i = bitcast i8* %"&pSB[currWI].offset3746.i" to i64*
  %loadedValue3748.i = load i64* %CastToValueType3747.i, align 8
  %.sum2694.i = add i64 %loadedValue3748.i, 3
  %366 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2694.i
  %exData1465.i = extractelement <16 x float> %add75448.i, i32 3
  store float %exData1465.i, float addrspace(3)* %366, align 4
  br label %postload2578.i

postload2578.i:                                   ; preds = %postload2575.i.postload2578.i_crit_edge, %preload2577.i
  %loadedValue4294.i = load i1* %CastToValueType4279.i, align 1
  br i1 %loadedValue4294.i, label %preload2580.i, label %postload2578.i.postload2581.i_crit_edge

postload2578.i.postload2581.i_crit_edge:          ; preds = %postload2578.i
  br label %postload2581.i

preload2580.i:                                    ; preds = %postload2578.i
  %"&(pSB[currWI].offset)3750.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3751.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3750.i"
  %CastToValueType3752.i = bitcast i8* %"&pSB[currWI].offset3751.i" to i64*
  %loadedValue3753.i = load i64* %CastToValueType3752.i, align 8
  %.sum2693.i = add i64 %loadedValue3753.i, 4
  %367 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2693.i
  %exData1468.i = extractelement <16 x float> %add75448.i, i32 4
  store float %exData1468.i, float addrspace(3)* %367, align 4
  br label %postload2581.i

postload2581.i:                                   ; preds = %postload2578.i.postload2581.i_crit_edge, %preload2580.i
  %loadedValue4318.i = load i1* %CastToValueType4303.i, align 1
  br i1 %loadedValue4318.i, label %preload2583.i, label %postload2581.i.postload2584.i_crit_edge

postload2581.i.postload2584.i_crit_edge:          ; preds = %postload2581.i
  br label %postload2584.i

preload2583.i:                                    ; preds = %postload2581.i
  %"&(pSB[currWI].offset)3755.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3756.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3755.i"
  %CastToValueType3757.i = bitcast i8* %"&pSB[currWI].offset3756.i" to i64*
  %loadedValue3758.i = load i64* %CastToValueType3757.i, align 8
  %.sum2692.i = add i64 %loadedValue3758.i, 5
  %368 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2692.i
  %exData1471.i = extractelement <16 x float> %add75448.i, i32 5
  store float %exData1471.i, float addrspace(3)* %368, align 4
  br label %postload2584.i

postload2584.i:                                   ; preds = %postload2581.i.postload2584.i_crit_edge, %preload2583.i
  %loadedValue4342.i = load i1* %CastToValueType4327.i, align 1
  br i1 %loadedValue4342.i, label %preload2586.i, label %postload2584.i.postload2587.i_crit_edge

postload2584.i.postload2587.i_crit_edge:          ; preds = %postload2584.i
  br label %postload2587.i

preload2586.i:                                    ; preds = %postload2584.i
  %"&(pSB[currWI].offset)3760.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3761.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3760.i"
  %CastToValueType3762.i = bitcast i8* %"&pSB[currWI].offset3761.i" to i64*
  %loadedValue3763.i = load i64* %CastToValueType3762.i, align 8
  %.sum2691.i = add i64 %loadedValue3763.i, 6
  %369 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2691.i
  %exData1474.i = extractelement <16 x float> %add75448.i, i32 6
  store float %exData1474.i, float addrspace(3)* %369, align 4
  br label %postload2587.i

postload2587.i:                                   ; preds = %postload2584.i.postload2587.i_crit_edge, %preload2586.i
  %loadedValue4366.i = load i1* %CastToValueType4351.i, align 1
  br i1 %loadedValue4366.i, label %preload2589.i, label %postload2587.i.postload2590.i_crit_edge

postload2587.i.postload2590.i_crit_edge:          ; preds = %postload2587.i
  br label %postload2590.i

preload2589.i:                                    ; preds = %postload2587.i
  %"&(pSB[currWI].offset)3765.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3766.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3765.i"
  %CastToValueType3767.i = bitcast i8* %"&pSB[currWI].offset3766.i" to i64*
  %loadedValue3768.i = load i64* %CastToValueType3767.i, align 8
  %.sum2690.i = add i64 %loadedValue3768.i, 7
  %370 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2690.i
  %exData1477.i = extractelement <16 x float> %add75448.i, i32 7
  store float %exData1477.i, float addrspace(3)* %370, align 4
  br label %postload2590.i

postload2590.i:                                   ; preds = %postload2587.i.postload2590.i_crit_edge, %preload2589.i
  %loadedValue4390.i = load i1* %CastToValueType4375.i, align 1
  br i1 %loadedValue4390.i, label %preload2592.i, label %postload2590.i.postload2593.i_crit_edge

postload2590.i.postload2593.i_crit_edge:          ; preds = %postload2590.i
  br label %postload2593.i

preload2592.i:                                    ; preds = %postload2590.i
  %"&(pSB[currWI].offset)3770.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3771.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3770.i"
  %CastToValueType3772.i = bitcast i8* %"&pSB[currWI].offset3771.i" to i64*
  %loadedValue3773.i = load i64* %CastToValueType3772.i, align 8
  %.sum2689.i = add i64 %loadedValue3773.i, 8
  %371 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2689.i
  %exData1480.i = extractelement <16 x float> %add75448.i, i32 8
  store float %exData1480.i, float addrspace(3)* %371, align 4
  br label %postload2593.i

postload2593.i:                                   ; preds = %postload2590.i.postload2593.i_crit_edge, %preload2592.i
  %loadedValue4414.i = load i1* %CastToValueType4399.i, align 1
  br i1 %loadedValue4414.i, label %preload2595.i, label %postload2593.i.postload2596.i_crit_edge

postload2593.i.postload2596.i_crit_edge:          ; preds = %postload2593.i
  br label %postload2596.i

preload2595.i:                                    ; preds = %postload2593.i
  %"&(pSB[currWI].offset)3775.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3776.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3775.i"
  %CastToValueType3777.i = bitcast i8* %"&pSB[currWI].offset3776.i" to i64*
  %loadedValue3778.i = load i64* %CastToValueType3777.i, align 8
  %.sum2688.i = add i64 %loadedValue3778.i, 9
  %372 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2688.i
  %exData1483.i = extractelement <16 x float> %add75448.i, i32 9
  store float %exData1483.i, float addrspace(3)* %372, align 4
  br label %postload2596.i

postload2596.i:                                   ; preds = %postload2593.i.postload2596.i_crit_edge, %preload2595.i
  %loadedValue4438.i = load i1* %CastToValueType4423.i, align 1
  br i1 %loadedValue4438.i, label %preload2598.i, label %postload2596.i.postload2599.i_crit_edge

postload2596.i.postload2599.i_crit_edge:          ; preds = %postload2596.i
  br label %postload2599.i

preload2598.i:                                    ; preds = %postload2596.i
  %"&(pSB[currWI].offset)3780.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3781.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3780.i"
  %CastToValueType3782.i = bitcast i8* %"&pSB[currWI].offset3781.i" to i64*
  %loadedValue3783.i = load i64* %CastToValueType3782.i, align 8
  %.sum2687.i = add i64 %loadedValue3783.i, 10
  %373 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2687.i
  %exData1486.i = extractelement <16 x float> %add75448.i, i32 10
  store float %exData1486.i, float addrspace(3)* %373, align 4
  br label %postload2599.i

postload2599.i:                                   ; preds = %postload2596.i.postload2599.i_crit_edge, %preload2598.i
  %loadedValue4462.i = load i1* %CastToValueType4447.i, align 1
  br i1 %loadedValue4462.i, label %preload2601.i, label %postload2599.i.postload2602.i_crit_edge

postload2599.i.postload2602.i_crit_edge:          ; preds = %postload2599.i
  br label %postload2602.i

preload2601.i:                                    ; preds = %postload2599.i
  %"&(pSB[currWI].offset)3785.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3786.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3785.i"
  %CastToValueType3787.i = bitcast i8* %"&pSB[currWI].offset3786.i" to i64*
  %loadedValue3788.i = load i64* %CastToValueType3787.i, align 8
  %.sum2686.i = add i64 %loadedValue3788.i, 11
  %374 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2686.i
  %exData1489.i = extractelement <16 x float> %add75448.i, i32 11
  store float %exData1489.i, float addrspace(3)* %374, align 4
  br label %postload2602.i

postload2602.i:                                   ; preds = %postload2599.i.postload2602.i_crit_edge, %preload2601.i
  %loadedValue4486.i = load i1* %CastToValueType4471.i, align 1
  br i1 %loadedValue4486.i, label %preload2604.i, label %postload2602.i.postload2605.i_crit_edge

postload2602.i.postload2605.i_crit_edge:          ; preds = %postload2602.i
  br label %postload2605.i

preload2604.i:                                    ; preds = %postload2602.i
  %"&(pSB[currWI].offset)3790.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3791.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3790.i"
  %CastToValueType3792.i = bitcast i8* %"&pSB[currWI].offset3791.i" to i64*
  %loadedValue3793.i = load i64* %CastToValueType3792.i, align 8
  %.sum2685.i = add i64 %loadedValue3793.i, 12
  %375 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2685.i
  %exData1492.i = extractelement <16 x float> %add75448.i, i32 12
  store float %exData1492.i, float addrspace(3)* %375, align 4
  br label %postload2605.i

postload2605.i:                                   ; preds = %postload2602.i.postload2605.i_crit_edge, %preload2604.i
  %loadedValue4510.i = load i1* %CastToValueType4495.i, align 1
  br i1 %loadedValue4510.i, label %preload2607.i, label %postload2605.i.postload2608.i_crit_edge

postload2605.i.postload2608.i_crit_edge:          ; preds = %postload2605.i
  br label %postload2608.i

preload2607.i:                                    ; preds = %postload2605.i
  %"&(pSB[currWI].offset)3795.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3796.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3795.i"
  %CastToValueType3797.i = bitcast i8* %"&pSB[currWI].offset3796.i" to i64*
  %loadedValue3798.i = load i64* %CastToValueType3797.i, align 8
  %.sum2684.i = add i64 %loadedValue3798.i, 13
  %376 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2684.i
  %exData1495.i = extractelement <16 x float> %add75448.i, i32 13
  store float %exData1495.i, float addrspace(3)* %376, align 4
  br label %postload2608.i

postload2608.i:                                   ; preds = %postload2605.i.postload2608.i_crit_edge, %preload2607.i
  %loadedValue4534.i = load i1* %CastToValueType4519.i, align 1
  br i1 %loadedValue4534.i, label %preload2610.i, label %postload2608.i.postload2611.i_crit_edge

postload2608.i.postload2611.i_crit_edge:          ; preds = %postload2608.i
  br label %postload2611.i

preload2610.i:                                    ; preds = %postload2608.i
  %"&(pSB[currWI].offset)3800.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3801.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3800.i"
  %CastToValueType3802.i = bitcast i8* %"&pSB[currWI].offset3801.i" to i64*
  %loadedValue3803.i = load i64* %CastToValueType3802.i, align 8
  %.sum2683.i = add i64 %loadedValue3803.i, 14
  %377 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2683.i
  %exData1498.i = extractelement <16 x float> %add75448.i, i32 14
  store float %exData1498.i, float addrspace(3)* %377, align 4
  br label %postload2611.i

postload2611.i:                                   ; preds = %postload2608.i.postload2611.i_crit_edge, %preload2610.i
  %loadedValue4558.i = load i1* %CastToValueType4543.i, align 1
  br i1 %loadedValue4558.i, label %preload2613.i, label %postload2611.i.if.end76.i_crit_edge

postload2611.i.if.end76.i_crit_edge:              ; preds = %postload2611.i
  br label %if.end76.i

preload2613.i:                                    ; preds = %postload2611.i
  %"&(pSB[currWI].offset)3805.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3806.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3805.i"
  %CastToValueType3807.i = bitcast i8* %"&pSB[currWI].offset3806.i" to i64*
  %loadedValue3808.i = load i64* %CastToValueType3807.i, align 8
  %.sum2682.i = add i64 %loadedValue3808.i, 15
  %378 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2682.i
  %exData1501.i = extractelement <16 x float> %add75448.i, i32 15
  store float %exData1501.i, float addrspace(3)* %378, align 4
  br label %if.end76.i

if.end76.i:                                       ; preds = %postload2611.i.if.end76.i_crit_edge, %preload2613.i
  %"&(pSB[currWI].offset)3997.i" = add nuw i64 %CurrSBIndex..10.i, 98
  %"&pSB[currWI].offset3998.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3997.i"
  %CastToValueType3999.i = bitcast i8* %"&pSB[currWI].offset3998.i" to i1*
  %loadedValue4000.i = load i1* %CastToValueType3999.i, align 1
  br i1 %loadedValue4000.i, label %preload1685.i, label %if.end76.i.postload1686.i_crit_edge

if.end76.i.postload1686.i_crit_edge:              ; preds = %if.end76.i
  br label %postload1686.i

preload1685.i:                                    ; preds = %if.end76.i
  %check.WI.iter4606.i = icmp ult i64 %CurrWI..10.i, %31
  br i1 %check.WI.iter4606.i, label %thenBB4603.i, label %preload1685.i.postload1686.i_crit_edge

preload1685.i.postload1686.i_crit_edge:           ; preds = %preload1685.i
  br label %postload1686.i

thenBB4603.i:                                     ; preds = %preload1685.i
  %"CurrWI++4607.i" = add nuw i64 %CurrWI..10.i, 1
  %"loadedCurrSB+Stride4609.i" = add nuw i64 %CurrSBIndex..10.i, 256
  switch i32 %currBarrier.10.i, label %thenBB4603.i.postload1684.i_crit_edge [
    i32 9, label %thenBB4603.i.postload1682.i_crit_edge
    i32 8, label %thenBB4603.i.postload1680.i_crit_edge
    i32 7, label %thenBB4603.i.postload1678.i_crit_edge
    i32 6, label %thenBB4603.i.postload1676.i_crit_edge
    i32 14, label %SyncBB4569.outer.i
  ]

thenBB4603.i.postload1676.i_crit_edge:            ; preds = %thenBB4603.i
  br label %postload1676.i

thenBB4603.i.postload1678.i_crit_edge:            ; preds = %thenBB4603.i
  br label %postload1678.i

thenBB4603.i.postload1680.i_crit_edge:            ; preds = %thenBB4603.i
  br label %postload1680.i

thenBB4603.i.postload1682.i_crit_edge:            ; preds = %thenBB4603.i
  br label %postload1682.i

thenBB4603.i.postload1684.i_crit_edge:            ; preds = %thenBB4603.i
  br label %postload1684.i

postload1686.i:                                   ; preds = %thenBB4611.i.postload1686.i_crit_edge, %preload1685.i.postload1686.i_crit_edge, %if.end76.i.postload1686.i_crit_edge
  %CurrWI..12.i = phi i64 [ %CurrWI..10.i, %if.end76.i.postload1686.i_crit_edge ], [ 0, %preload1685.i.postload1686.i_crit_edge ], [ %"CurrWI++4615.i", %thenBB4611.i.postload1686.i_crit_edge ]
  %CurrSBIndex..12.i = phi i64 [ %CurrSBIndex..10.i, %if.end76.i.postload1686.i_crit_edge ], [ 0, %preload1685.i.postload1686.i_crit_edge ], [ %"loadedCurrSB+Stride4617.i", %thenBB4611.i.postload1686.i_crit_edge ]
  %currBarrier.12.i = phi i32 [ %currBarrier.10.i, %if.end76.i.postload1686.i_crit_edge ], [ 11, %preload1685.i.postload1686.i_crit_edge ], [ %currBarrier.12.i, %thenBB4611.i.postload1686.i_crit_edge ]
  %"&(pSB[currWI].offset)4185.i" = add nuw i64 %CurrSBIndex..12.i, 232
  %"&pSB[currWI].offset4186.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4185.i"
  %CastToValueType4187.i = bitcast i8* %"&pSB[currWI].offset4186.i" to i1*
  %loadedValue4188.i = load i1* %CastToValueType4187.i, align 1
  br i1 %loadedValue4188.i, label %preload2616.i, label %postload1686.i.postload2617.i_crit_edge

postload1686.i.postload2617.i_crit_edge:          ; preds = %postload1686.i
  br label %postload2617.i

preload2616.i:                                    ; preds = %postload1686.i
  %"&(pSB[currWI].offset)3944.i" = add nuw i64 %CurrSBIndex..12.i, 80
  %"&pSB[currWI].offset3945.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3944.i"
  %CastToValueType3946.i = bitcast i8* %"&pSB[currWI].offset3945.i" to float addrspace(3)**
  %loadedValue3947.i = load float addrspace(3)** %CastToValueType3946.i, align 8
  %vload1505.i = load float addrspace(3)* %loadedValue3947.i, align 4
  br label %postload2617.i

postload2617.i:                                   ; preds = %postload1686.i.postload2617.i_crit_edge, %preload2616.i
  %phi2618.i = phi float [ %vload1505.i, %preload2616.i ], [ undef, %postload1686.i.postload2617.i_crit_edge ]
  %"&(pSB[currWI].offset)4209.i" = add nuw i64 %CurrSBIndex..12.i, 233
  %"&pSB[currWI].offset4210.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4209.i"
  %CastToValueType4211.i = bitcast i8* %"&pSB[currWI].offset4210.i" to i1*
  %loadedValue4212.i = load i1* %CastToValueType4211.i, align 1
  br i1 %loadedValue4212.i, label %preload2619.i, label %postload2617.i.postload2620.i_crit_edge

postload2617.i.postload2620.i_crit_edge:          ; preds = %postload2617.i
  br label %postload2620.i

preload2619.i:                                    ; preds = %postload2617.i
  %"&(pSB[currWI].offset)3810.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3811.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3810.i"
  %CastToValueType3812.i = bitcast i8* %"&pSB[currWI].offset3811.i" to i64*
  %loadedValue3813.i = load i64* %CastToValueType3812.i, align 8
  %.sum2681.i = add i64 %loadedValue3813.i, 1
  %379 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2681.i
  %vload1509.i = load float addrspace(3)* %379, align 4
  br label %postload2620.i

postload2620.i:                                   ; preds = %postload2617.i.postload2620.i_crit_edge, %preload2619.i
  %phi2621.i = phi float [ %vload1509.i, %preload2619.i ], [ undef, %postload2617.i.postload2620.i_crit_edge ]
  %"&(pSB[currWI].offset)4233.i" = add nuw i64 %CurrSBIndex..12.i, 234
  %"&pSB[currWI].offset4234.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4233.i"
  %CastToValueType4235.i = bitcast i8* %"&pSB[currWI].offset4234.i" to i1*
  %loadedValue4236.i = load i1* %CastToValueType4235.i, align 1
  br i1 %loadedValue4236.i, label %preload2622.i, label %postload2620.i.postload2623.i_crit_edge

postload2620.i.postload2623.i_crit_edge:          ; preds = %postload2620.i
  br label %postload2623.i

preload2622.i:                                    ; preds = %postload2620.i
  %"&(pSB[currWI].offset)3815.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3816.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3815.i"
  %CastToValueType3817.i = bitcast i8* %"&pSB[currWI].offset3816.i" to i64*
  %loadedValue3818.i = load i64* %CastToValueType3817.i, align 8
  %.sum2680.i = add i64 %loadedValue3818.i, 2
  %380 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2680.i
  %vload1513.i = load float addrspace(3)* %380, align 4
  br label %postload2623.i

postload2623.i:                                   ; preds = %postload2620.i.postload2623.i_crit_edge, %preload2622.i
  %phi2624.i = phi float [ %vload1513.i, %preload2622.i ], [ undef, %postload2620.i.postload2623.i_crit_edge ]
  %"&(pSB[currWI].offset)4257.i" = add nuw i64 %CurrSBIndex..12.i, 235
  %"&pSB[currWI].offset4258.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4257.i"
  %CastToValueType4259.i = bitcast i8* %"&pSB[currWI].offset4258.i" to i1*
  %loadedValue4260.i = load i1* %CastToValueType4259.i, align 1
  br i1 %loadedValue4260.i, label %preload2625.i, label %postload2623.i.postload2626.i_crit_edge

postload2623.i.postload2626.i_crit_edge:          ; preds = %postload2623.i
  br label %postload2626.i

preload2625.i:                                    ; preds = %postload2623.i
  %"&(pSB[currWI].offset)3820.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3821.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3820.i"
  %CastToValueType3822.i = bitcast i8* %"&pSB[currWI].offset3821.i" to i64*
  %loadedValue3823.i = load i64* %CastToValueType3822.i, align 8
  %.sum2679.i = add i64 %loadedValue3823.i, 3
  %381 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2679.i
  %vload1517.i = load float addrspace(3)* %381, align 4
  br label %postload2626.i

postload2626.i:                                   ; preds = %postload2623.i.postload2626.i_crit_edge, %preload2625.i
  %phi2627.i = phi float [ %vload1517.i, %preload2625.i ], [ undef, %postload2623.i.postload2626.i_crit_edge ]
  %"&(pSB[currWI].offset)4281.i" = add nuw i64 %CurrSBIndex..12.i, 236
  %"&pSB[currWI].offset4282.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4281.i"
  %CastToValueType4283.i = bitcast i8* %"&pSB[currWI].offset4282.i" to i1*
  %loadedValue4284.i = load i1* %CastToValueType4283.i, align 1
  br i1 %loadedValue4284.i, label %preload2628.i, label %postload2626.i.postload2629.i_crit_edge

postload2626.i.postload2629.i_crit_edge:          ; preds = %postload2626.i
  br label %postload2629.i

preload2628.i:                                    ; preds = %postload2626.i
  %"&(pSB[currWI].offset)3825.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3826.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3825.i"
  %CastToValueType3827.i = bitcast i8* %"&pSB[currWI].offset3826.i" to i64*
  %loadedValue3828.i = load i64* %CastToValueType3827.i, align 8
  %.sum2678.i = add i64 %loadedValue3828.i, 4
  %382 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2678.i
  %vload1521.i = load float addrspace(3)* %382, align 4
  br label %postload2629.i

postload2629.i:                                   ; preds = %postload2626.i.postload2629.i_crit_edge, %preload2628.i
  %phi2630.i = phi float [ %vload1521.i, %preload2628.i ], [ undef, %postload2626.i.postload2629.i_crit_edge ]
  %"&(pSB[currWI].offset)4305.i" = add nuw i64 %CurrSBIndex..12.i, 237
  %"&pSB[currWI].offset4306.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4305.i"
  %CastToValueType4307.i = bitcast i8* %"&pSB[currWI].offset4306.i" to i1*
  %loadedValue4308.i = load i1* %CastToValueType4307.i, align 1
  br i1 %loadedValue4308.i, label %preload2631.i, label %postload2629.i.postload2632.i_crit_edge

postload2629.i.postload2632.i_crit_edge:          ; preds = %postload2629.i
  br label %postload2632.i

preload2631.i:                                    ; preds = %postload2629.i
  %"&(pSB[currWI].offset)3830.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3831.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3830.i"
  %CastToValueType3832.i = bitcast i8* %"&pSB[currWI].offset3831.i" to i64*
  %loadedValue3833.i = load i64* %CastToValueType3832.i, align 8
  %.sum2677.i = add i64 %loadedValue3833.i, 5
  %383 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2677.i
  %vload1525.i = load float addrspace(3)* %383, align 4
  br label %postload2632.i

postload2632.i:                                   ; preds = %postload2629.i.postload2632.i_crit_edge, %preload2631.i
  %phi2633.i = phi float [ %vload1525.i, %preload2631.i ], [ undef, %postload2629.i.postload2632.i_crit_edge ]
  %"&(pSB[currWI].offset)4329.i" = add nuw i64 %CurrSBIndex..12.i, 238
  %"&pSB[currWI].offset4330.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4329.i"
  %CastToValueType4331.i = bitcast i8* %"&pSB[currWI].offset4330.i" to i1*
  %loadedValue4332.i = load i1* %CastToValueType4331.i, align 1
  br i1 %loadedValue4332.i, label %preload2634.i, label %postload2632.i.postload2635.i_crit_edge

postload2632.i.postload2635.i_crit_edge:          ; preds = %postload2632.i
  br label %postload2635.i

preload2634.i:                                    ; preds = %postload2632.i
  %"&(pSB[currWI].offset)3835.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3836.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3835.i"
  %CastToValueType3837.i = bitcast i8* %"&pSB[currWI].offset3836.i" to i64*
  %loadedValue3838.i = load i64* %CastToValueType3837.i, align 8
  %.sum2676.i = add i64 %loadedValue3838.i, 6
  %384 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2676.i
  %vload1529.i = load float addrspace(3)* %384, align 4
  br label %postload2635.i

postload2635.i:                                   ; preds = %postload2632.i.postload2635.i_crit_edge, %preload2634.i
  %phi2636.i = phi float [ %vload1529.i, %preload2634.i ], [ undef, %postload2632.i.postload2635.i_crit_edge ]
  %"&(pSB[currWI].offset)4353.i" = add nuw i64 %CurrSBIndex..12.i, 239
  %"&pSB[currWI].offset4354.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4353.i"
  %CastToValueType4355.i = bitcast i8* %"&pSB[currWI].offset4354.i" to i1*
  %loadedValue4356.i = load i1* %CastToValueType4355.i, align 1
  br i1 %loadedValue4356.i, label %preload2637.i, label %postload2635.i.postload2638.i_crit_edge

postload2635.i.postload2638.i_crit_edge:          ; preds = %postload2635.i
  br label %postload2638.i

preload2637.i:                                    ; preds = %postload2635.i
  %"&(pSB[currWI].offset)3840.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3841.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3840.i"
  %CastToValueType3842.i = bitcast i8* %"&pSB[currWI].offset3841.i" to i64*
  %loadedValue3843.i = load i64* %CastToValueType3842.i, align 8
  %.sum2675.i = add i64 %loadedValue3843.i, 7
  %385 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2675.i
  %vload1533.i = load float addrspace(3)* %385, align 4
  br label %postload2638.i

postload2638.i:                                   ; preds = %postload2635.i.postload2638.i_crit_edge, %preload2637.i
  %phi2639.i = phi float [ %vload1533.i, %preload2637.i ], [ undef, %postload2635.i.postload2638.i_crit_edge ]
  %"&(pSB[currWI].offset)4377.i" = add nuw i64 %CurrSBIndex..12.i, 240
  %"&pSB[currWI].offset4378.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4377.i"
  %CastToValueType4379.i = bitcast i8* %"&pSB[currWI].offset4378.i" to i1*
  %loadedValue4380.i = load i1* %CastToValueType4379.i, align 1
  br i1 %loadedValue4380.i, label %preload2640.i, label %postload2638.i.postload2641.i_crit_edge

postload2638.i.postload2641.i_crit_edge:          ; preds = %postload2638.i
  br label %postload2641.i

preload2640.i:                                    ; preds = %postload2638.i
  %"&(pSB[currWI].offset)3845.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3846.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3845.i"
  %CastToValueType3847.i = bitcast i8* %"&pSB[currWI].offset3846.i" to i64*
  %loadedValue3848.i = load i64* %CastToValueType3847.i, align 8
  %.sum2674.i = add i64 %loadedValue3848.i, 8
  %386 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2674.i
  %vload1537.i = load float addrspace(3)* %386, align 4
  br label %postload2641.i

postload2641.i:                                   ; preds = %postload2638.i.postload2641.i_crit_edge, %preload2640.i
  %phi2642.i = phi float [ %vload1537.i, %preload2640.i ], [ undef, %postload2638.i.postload2641.i_crit_edge ]
  %"&(pSB[currWI].offset)4401.i" = add nuw i64 %CurrSBIndex..12.i, 241
  %"&pSB[currWI].offset4402.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4401.i"
  %CastToValueType4403.i = bitcast i8* %"&pSB[currWI].offset4402.i" to i1*
  %loadedValue4404.i = load i1* %CastToValueType4403.i, align 1
  br i1 %loadedValue4404.i, label %preload2643.i, label %postload2641.i.postload2644.i_crit_edge

postload2641.i.postload2644.i_crit_edge:          ; preds = %postload2641.i
  br label %postload2644.i

preload2643.i:                                    ; preds = %postload2641.i
  %"&(pSB[currWI].offset)3850.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3851.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3850.i"
  %CastToValueType3852.i = bitcast i8* %"&pSB[currWI].offset3851.i" to i64*
  %loadedValue3853.i = load i64* %CastToValueType3852.i, align 8
  %.sum2673.i = add i64 %loadedValue3853.i, 9
  %387 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2673.i
  %vload1541.i = load float addrspace(3)* %387, align 4
  br label %postload2644.i

postload2644.i:                                   ; preds = %postload2641.i.postload2644.i_crit_edge, %preload2643.i
  %phi2645.i = phi float [ %vload1541.i, %preload2643.i ], [ undef, %postload2641.i.postload2644.i_crit_edge ]
  %"&(pSB[currWI].offset)4425.i" = add nuw i64 %CurrSBIndex..12.i, 242
  %"&pSB[currWI].offset4426.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4425.i"
  %CastToValueType4427.i = bitcast i8* %"&pSB[currWI].offset4426.i" to i1*
  %loadedValue4428.i = load i1* %CastToValueType4427.i, align 1
  br i1 %loadedValue4428.i, label %preload2646.i, label %postload2644.i.postload2647.i_crit_edge

postload2644.i.postload2647.i_crit_edge:          ; preds = %postload2644.i
  br label %postload2647.i

preload2646.i:                                    ; preds = %postload2644.i
  %"&(pSB[currWI].offset)3855.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3856.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3855.i"
  %CastToValueType3857.i = bitcast i8* %"&pSB[currWI].offset3856.i" to i64*
  %loadedValue3858.i = load i64* %CastToValueType3857.i, align 8
  %.sum2672.i = add i64 %loadedValue3858.i, 10
  %388 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2672.i
  %vload1545.i = load float addrspace(3)* %388, align 4
  br label %postload2647.i

postload2647.i:                                   ; preds = %postload2644.i.postload2647.i_crit_edge, %preload2646.i
  %phi2648.i = phi float [ %vload1545.i, %preload2646.i ], [ undef, %postload2644.i.postload2647.i_crit_edge ]
  %"&(pSB[currWI].offset)4449.i" = add nuw i64 %CurrSBIndex..12.i, 243
  %"&pSB[currWI].offset4450.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4449.i"
  %CastToValueType4451.i = bitcast i8* %"&pSB[currWI].offset4450.i" to i1*
  %loadedValue4452.i = load i1* %CastToValueType4451.i, align 1
  br i1 %loadedValue4452.i, label %preload2649.i, label %postload2647.i.postload2650.i_crit_edge

postload2647.i.postload2650.i_crit_edge:          ; preds = %postload2647.i
  br label %postload2650.i

preload2649.i:                                    ; preds = %postload2647.i
  %"&(pSB[currWI].offset)3860.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3861.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3860.i"
  %CastToValueType3862.i = bitcast i8* %"&pSB[currWI].offset3861.i" to i64*
  %loadedValue3863.i = load i64* %CastToValueType3862.i, align 8
  %.sum2671.i = add i64 %loadedValue3863.i, 11
  %389 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2671.i
  %vload1549.i = load float addrspace(3)* %389, align 4
  br label %postload2650.i

postload2650.i:                                   ; preds = %postload2647.i.postload2650.i_crit_edge, %preload2649.i
  %phi2651.i = phi float [ %vload1549.i, %preload2649.i ], [ undef, %postload2647.i.postload2650.i_crit_edge ]
  %"&(pSB[currWI].offset)4473.i" = add nuw i64 %CurrSBIndex..12.i, 244
  %"&pSB[currWI].offset4474.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4473.i"
  %CastToValueType4475.i = bitcast i8* %"&pSB[currWI].offset4474.i" to i1*
  %loadedValue4476.i = load i1* %CastToValueType4475.i, align 1
  br i1 %loadedValue4476.i, label %preload2652.i, label %postload2650.i.postload2653.i_crit_edge

postload2650.i.postload2653.i_crit_edge:          ; preds = %postload2650.i
  br label %postload2653.i

preload2652.i:                                    ; preds = %postload2650.i
  %"&(pSB[currWI].offset)3865.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3866.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3865.i"
  %CastToValueType3867.i = bitcast i8* %"&pSB[currWI].offset3866.i" to i64*
  %loadedValue3868.i = load i64* %CastToValueType3867.i, align 8
  %.sum2670.i = add i64 %loadedValue3868.i, 12
  %390 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2670.i
  %vload1553.i = load float addrspace(3)* %390, align 4
  br label %postload2653.i

postload2653.i:                                   ; preds = %postload2650.i.postload2653.i_crit_edge, %preload2652.i
  %phi2654.i = phi float [ %vload1553.i, %preload2652.i ], [ undef, %postload2650.i.postload2653.i_crit_edge ]
  %"&(pSB[currWI].offset)4497.i" = add nuw i64 %CurrSBIndex..12.i, 245
  %"&pSB[currWI].offset4498.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4497.i"
  %CastToValueType4499.i = bitcast i8* %"&pSB[currWI].offset4498.i" to i1*
  %loadedValue4500.i = load i1* %CastToValueType4499.i, align 1
  br i1 %loadedValue4500.i, label %preload2655.i, label %postload2653.i.postload2656.i_crit_edge

postload2653.i.postload2656.i_crit_edge:          ; preds = %postload2653.i
  br label %postload2656.i

preload2655.i:                                    ; preds = %postload2653.i
  %"&(pSB[currWI].offset)3870.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3871.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3870.i"
  %CastToValueType3872.i = bitcast i8* %"&pSB[currWI].offset3871.i" to i64*
  %loadedValue3873.i = load i64* %CastToValueType3872.i, align 8
  %.sum2669.i = add i64 %loadedValue3873.i, 13
  %391 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2669.i
  %vload1557.i = load float addrspace(3)* %391, align 4
  br label %postload2656.i

postload2656.i:                                   ; preds = %postload2653.i.postload2656.i_crit_edge, %preload2655.i
  %phi2657.i = phi float [ %vload1557.i, %preload2655.i ], [ undef, %postload2653.i.postload2656.i_crit_edge ]
  %"&(pSB[currWI].offset)4521.i" = add nuw i64 %CurrSBIndex..12.i, 246
  %"&pSB[currWI].offset4522.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4521.i"
  %CastToValueType4523.i = bitcast i8* %"&pSB[currWI].offset4522.i" to i1*
  %loadedValue4524.i = load i1* %CastToValueType4523.i, align 1
  br i1 %loadedValue4524.i, label %preload2658.i, label %postload2656.i.postload2659.i_crit_edge

postload2656.i.postload2659.i_crit_edge:          ; preds = %postload2656.i
  br label %postload2659.i

preload2658.i:                                    ; preds = %postload2656.i
  %"&(pSB[currWI].offset)3875.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3876.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3875.i"
  %CastToValueType3877.i = bitcast i8* %"&pSB[currWI].offset3876.i" to i64*
  %loadedValue3878.i = load i64* %CastToValueType3877.i, align 8
  %.sum2668.i = add i64 %loadedValue3878.i, 14
  %392 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum2668.i
  %vload1561.i = load float addrspace(3)* %392, align 4
  br label %postload2659.i

postload2659.i:                                   ; preds = %postload2656.i.postload2659.i_crit_edge, %preload2658.i
  %phi2660.i = phi float [ %vload1561.i, %preload2658.i ], [ undef, %postload2656.i.postload2659.i_crit_edge ]
  %"&(pSB[currWI].offset)4545.i" = add nuw i64 %CurrSBIndex..12.i, 247
  %"&pSB[currWI].offset4546.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4545.i"
  %CastToValueType4547.i = bitcast i8* %"&pSB[currWI].offset4546.i" to i1*
  %loadedValue4548.i = load i1* %CastToValueType4547.i, align 1
  br i1 %loadedValue4548.i, label %preload2661.i, label %postload2662.i

preload2661.i:                                    ; preds = %postload2659.i
  %"&(pSB[currWI].offset)3880.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3881.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3880.i"
  %CastToValueType3882.i = bitcast i8* %"&pSB[currWI].offset3881.i" to i64*
  %loadedValue3883.i = load i64* %CastToValueType3882.i, align 8
  %.sum.i = add i64 %loadedValue3883.i, 15
  %393 = getelementptr [128 x float] addrspace(3)* %35, i64 0, i64 %.sum.i
  %vload1565.i = load float addrspace(3)* %393, align 4
  br label %postload2662.i

postload2662.i:                                   ; preds = %preload2661.i, %postload2659.i
  %phi2663.i = phi float [ undef, %postload2659.i ], [ %vload1565.i, %preload2661.i ]
  %"&(pSB[currWI].offset)4050.i" = add nuw i64 %CurrSBIndex..12.i, 112
  %"&pSB[currWI].offset4051.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4050.i"
  %CastToValueType4052.i = bitcast i8* %"&pSB[currWI].offset4051.i" to i64*
  %loadedValue4053.i = load i64* %CastToValueType4052.i, align 8
  %394 = getelementptr inbounds float addrspace(1)* %16, i64 %loadedValue4053.i
  %"&(pSB[currWI].offset)4059.i" = add nuw i64 %CurrSBIndex..12.i, 120
  %"&pSB[currWI].offset4060.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4059.i"
  %CastToValueType4061.i = bitcast i8* %"&pSB[currWI].offset4060.i" to i64*
  %loadedValue4062.i = load i64* %CastToValueType4061.i, align 8
  %395 = getelementptr inbounds float addrspace(1)* %16, i64 %loadedValue4062.i
  %"&(pSB[currWI].offset)4068.i" = add nuw i64 %CurrSBIndex..12.i, 128
  %"&pSB[currWI].offset4069.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4068.i"
  %CastToValueType4070.i = bitcast i8* %"&pSB[currWI].offset4069.i" to i64*
  %loadedValue4071.i = load i64* %CastToValueType4070.i, align 8
  %396 = getelementptr inbounds float addrspace(1)* %16, i64 %loadedValue4071.i
  %"&(pSB[currWI].offset)4077.i" = add nuw i64 %CurrSBIndex..12.i, 136
  %"&pSB[currWI].offset4078.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4077.i"
  %CastToValueType4079.i = bitcast i8* %"&pSB[currWI].offset4078.i" to i64*
  %loadedValue4080.i = load i64* %CastToValueType4079.i, align 8
  %397 = getelementptr inbounds float addrspace(1)* %16, i64 %loadedValue4080.i
  %"&(pSB[currWI].offset)4086.i" = add nuw i64 %CurrSBIndex..12.i, 144
  %"&pSB[currWI].offset4087.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4086.i"
  %CastToValueType4088.i = bitcast i8* %"&pSB[currWI].offset4087.i" to i64*
  %loadedValue4089.i = load i64* %CastToValueType4088.i, align 8
  %398 = getelementptr inbounds float addrspace(1)* %16, i64 %loadedValue4089.i
  %"&(pSB[currWI].offset)4095.i" = add nuw i64 %CurrSBIndex..12.i, 152
  %"&pSB[currWI].offset4096.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4095.i"
  %CastToValueType4097.i = bitcast i8* %"&pSB[currWI].offset4096.i" to i64*
  %loadedValue4098.i = load i64* %CastToValueType4097.i, align 8
  %399 = getelementptr inbounds float addrspace(1)* %16, i64 %loadedValue4098.i
  %"&(pSB[currWI].offset)4104.i" = add nuw i64 %CurrSBIndex..12.i, 160
  %"&pSB[currWI].offset4105.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4104.i"
  %CastToValueType4106.i = bitcast i8* %"&pSB[currWI].offset4105.i" to i64*
  %loadedValue4107.i = load i64* %CastToValueType4106.i, align 8
  %400 = getelementptr inbounds float addrspace(1)* %16, i64 %loadedValue4107.i
  %"&(pSB[currWI].offset)4113.i" = add nuw i64 %CurrSBIndex..12.i, 168
  %"&pSB[currWI].offset4114.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4113.i"
  %CastToValueType4115.i = bitcast i8* %"&pSB[currWI].offset4114.i" to i64*
  %loadedValue4116.i = load i64* %CastToValueType4115.i, align 8
  %401 = getelementptr inbounds float addrspace(1)* %16, i64 %loadedValue4116.i
  %"&(pSB[currWI].offset)4122.i" = add nuw i64 %CurrSBIndex..12.i, 176
  %"&pSB[currWI].offset4123.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4122.i"
  %CastToValueType4124.i = bitcast i8* %"&pSB[currWI].offset4123.i" to i64*
  %loadedValue4125.i = load i64* %CastToValueType4124.i, align 8
  %402 = getelementptr inbounds float addrspace(1)* %16, i64 %loadedValue4125.i
  %"&(pSB[currWI].offset)4131.i" = add nuw i64 %CurrSBIndex..12.i, 184
  %"&pSB[currWI].offset4132.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4131.i"
  %CastToValueType4133.i = bitcast i8* %"&pSB[currWI].offset4132.i" to i64*
  %loadedValue4134.i = load i64* %CastToValueType4133.i, align 8
  %403 = getelementptr inbounds float addrspace(1)* %16, i64 %loadedValue4134.i
  %"&(pSB[currWI].offset)4140.i" = add nuw i64 %CurrSBIndex..12.i, 192
  %"&pSB[currWI].offset4141.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4140.i"
  %CastToValueType4142.i = bitcast i8* %"&pSB[currWI].offset4141.i" to i64*
  %loadedValue4143.i = load i64* %CastToValueType4142.i, align 8
  %404 = getelementptr inbounds float addrspace(1)* %16, i64 %loadedValue4143.i
  %"&(pSB[currWI].offset)4149.i" = add nuw i64 %CurrSBIndex..12.i, 200
  %"&pSB[currWI].offset4150.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4149.i"
  %CastToValueType4151.i = bitcast i8* %"&pSB[currWI].offset4150.i" to i64*
  %loadedValue4152.i = load i64* %CastToValueType4151.i, align 8
  %405 = getelementptr inbounds float addrspace(1)* %16, i64 %loadedValue4152.i
  %"&(pSB[currWI].offset)4158.i" = add nuw i64 %CurrSBIndex..12.i, 208
  %"&pSB[currWI].offset4159.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4158.i"
  %CastToValueType4160.i = bitcast i8* %"&pSB[currWI].offset4159.i" to i64*
  %loadedValue4161.i = load i64* %CastToValueType4160.i, align 8
  %406 = getelementptr inbounds float addrspace(1)* %16, i64 %loadedValue4161.i
  %"&(pSB[currWI].offset)4167.i" = add nuw i64 %CurrSBIndex..12.i, 216
  %"&pSB[currWI].offset4168.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4167.i"
  %CastToValueType4169.i = bitcast i8* %"&pSB[currWI].offset4168.i" to i64*
  %loadedValue4170.i = load i64* %CastToValueType4169.i, align 8
  %407 = getelementptr inbounds float addrspace(1)* %16, i64 %loadedValue4170.i
  %"&(pSB[currWI].offset)4176.i" = add nuw i64 %CurrSBIndex..12.i, 224
  %"&pSB[currWI].offset4177.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4176.i"
  %CastToValueType4178.i = bitcast i8* %"&pSB[currWI].offset4177.i" to i64*
  %loadedValue4179.i = load i64* %CastToValueType4178.i, align 8
  %408 = getelementptr inbounds float addrspace(1)* %16, i64 %loadedValue4179.i
  br i1 %loadedValue4188.i, label %preload2059.i, label %postload2060.i

preload2059.i:                                    ; preds = %postload2662.i
  %"&(pSB[currWI].offset)4036.i" = add nuw i64 %CurrSBIndex..12.i, 104
  %"&pSB[currWI].offset4037.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4036.i"
  %CastToValueType4038.i = bitcast i8* %"&pSB[currWI].offset4037.i" to i64*
  %loadedValue4039.i = load i64* %CastToValueType4038.i, align 8
  %409 = getelementptr inbounds float addrspace(1)* %16, i64 %loadedValue4039.i
  store float %phi2618.i, float addrspace(1)* %409, align 4
  br label %postload2060.i

postload2060.i:                                   ; preds = %preload2059.i, %postload2662.i
  br i1 %loadedValue4212.i, label %preload2061.i, label %postload2062.i

preload2061.i:                                    ; preds = %postload2060.i
  store float %phi2621.i, float addrspace(1)* %394, align 4
  br label %postload2062.i

postload2062.i:                                   ; preds = %preload2061.i, %postload2060.i
  br i1 %loadedValue4236.i, label %preload2063.i, label %postload2064.i

preload2063.i:                                    ; preds = %postload2062.i
  store float %phi2624.i, float addrspace(1)* %395, align 4
  br label %postload2064.i

postload2064.i:                                   ; preds = %preload2063.i, %postload2062.i
  br i1 %loadedValue4260.i, label %preload2065.i, label %postload2066.i

preload2065.i:                                    ; preds = %postload2064.i
  store float %phi2627.i, float addrspace(1)* %396, align 4
  br label %postload2066.i

postload2066.i:                                   ; preds = %preload2065.i, %postload2064.i
  br i1 %loadedValue4284.i, label %preload2067.i, label %postload2068.i

preload2067.i:                                    ; preds = %postload2066.i
  store float %phi2630.i, float addrspace(1)* %397, align 4
  br label %postload2068.i

postload2068.i:                                   ; preds = %preload2067.i, %postload2066.i
  br i1 %loadedValue4308.i, label %preload2069.i, label %postload2070.i

preload2069.i:                                    ; preds = %postload2068.i
  store float %phi2633.i, float addrspace(1)* %398, align 4
  br label %postload2070.i

postload2070.i:                                   ; preds = %preload2069.i, %postload2068.i
  br i1 %loadedValue4332.i, label %preload2071.i, label %postload2072.i

preload2071.i:                                    ; preds = %postload2070.i
  store float %phi2636.i, float addrspace(1)* %399, align 4
  br label %postload2072.i

postload2072.i:                                   ; preds = %preload2071.i, %postload2070.i
  br i1 %loadedValue4356.i, label %preload2073.i, label %postload2074.i

preload2073.i:                                    ; preds = %postload2072.i
  store float %phi2639.i, float addrspace(1)* %400, align 4
  br label %postload2074.i

postload2074.i:                                   ; preds = %preload2073.i, %postload2072.i
  br i1 %loadedValue4380.i, label %preload2075.i, label %postload2076.i

preload2075.i:                                    ; preds = %postload2074.i
  store float %phi2642.i, float addrspace(1)* %401, align 4
  br label %postload2076.i

postload2076.i:                                   ; preds = %preload2075.i, %postload2074.i
  br i1 %loadedValue4404.i, label %preload2077.i, label %postload2078.i

preload2077.i:                                    ; preds = %postload2076.i
  store float %phi2645.i, float addrspace(1)* %402, align 4
  br label %postload2078.i

postload2078.i:                                   ; preds = %preload2077.i, %postload2076.i
  br i1 %loadedValue4428.i, label %preload2079.i, label %postload2080.i

preload2079.i:                                    ; preds = %postload2078.i
  store float %phi2648.i, float addrspace(1)* %403, align 4
  br label %postload2080.i

postload2080.i:                                   ; preds = %preload2079.i, %postload2078.i
  br i1 %loadedValue4452.i, label %preload2081.i, label %postload2082.i

preload2081.i:                                    ; preds = %postload2080.i
  store float %phi2651.i, float addrspace(1)* %404, align 4
  br label %postload2082.i

postload2082.i:                                   ; preds = %preload2081.i, %postload2080.i
  br i1 %loadedValue4476.i, label %preload2083.i, label %postload2084.i

preload2083.i:                                    ; preds = %postload2082.i
  store float %phi2654.i, float addrspace(1)* %405, align 4
  br label %postload2084.i

postload2084.i:                                   ; preds = %preload2083.i, %postload2082.i
  br i1 %loadedValue4500.i, label %preload2085.i, label %postload2086.i

preload2085.i:                                    ; preds = %postload2084.i
  store float %phi2657.i, float addrspace(1)* %406, align 4
  br label %postload2086.i

postload2086.i:                                   ; preds = %preload2085.i, %postload2084.i
  br i1 %loadedValue4524.i, label %preload2087.i, label %postload2088.i

preload2087.i:                                    ; preds = %postload2086.i
  store float %phi2660.i, float addrspace(1)* %407, align 4
  br label %postload2088.i

postload2088.i:                                   ; preds = %preload2087.i, %postload2086.i
  br i1 %loadedValue4548.i, label %preload2089.i, label %if.end85.i

preload2089.i:                                    ; preds = %postload2088.i
  store float %phi2663.i, float addrspace(1)* %408, align 4
  br label %if.end85.i

if.end85.i:                                       ; preds = %preload2089.i, %postload2088.i
  %check.WI.iter4614.i = icmp ult i64 %CurrWI..12.i, %31
  br i1 %check.WI.iter4614.i, label %thenBB4611.i, label %____Vectorized_.spmv_csr_vector_kernel_separated_args.exit

thenBB4611.i:                                     ; preds = %if.end85.i
  %"CurrWI++4615.i" = add nuw i64 %CurrWI..12.i, 1
  %"loadedCurrSB+Stride4617.i" = add nuw i64 %CurrSBIndex..12.i, 256
  switch i32 %currBarrier.12.i, label %thenBB4611.i.postload1686.i_crit_edge [
    i32 10, label %thenBB4611.i.postload1684.i_crit_edge
    i32 9, label %thenBB4611.i.postload1682.i_crit_edge
    i32 8, label %thenBB4611.i.postload1680.i_crit_edge
    i32 7, label %thenBB4611.i.postload1678.i_crit_edge
    i32 6, label %thenBB4611.i.postload1676.i_crit_edge
    i32 14, label %SyncBB4569.outer.i
  ]

thenBB4611.i.postload1676.i_crit_edge:            ; preds = %thenBB4611.i
  br label %postload1676.i

thenBB4611.i.postload1678.i_crit_edge:            ; preds = %thenBB4611.i
  br label %postload1678.i

thenBB4611.i.postload1680.i_crit_edge:            ; preds = %thenBB4611.i
  br label %postload1680.i

thenBB4611.i.postload1682.i_crit_edge:            ; preds = %thenBB4611.i
  br label %postload1682.i

thenBB4611.i.postload1684.i_crit_edge:            ; preds = %thenBB4611.i
  br label %postload1684.i

thenBB4611.i.postload1686.i_crit_edge:            ; preds = %thenBB4611.i
  br label %postload1686.i

____Vectorized_.spmv_csr_vector_kernel_separated_args.exit: ; preds = %if.end85.i
  ret void
}

!opencl.kernels = !{!0, !2, !3}
!opencl.build.options = !{!4}
!cl.noBarrierPath.kernels = !{!5}
!opencl.wrappers = !{!6, !7, !8}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, float addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__spmv_csr_scalar_kernel_separated_args, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3, i32 3, i32 3}
!2 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, float addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__spmv_csr_vector_kernel_separated_args, metadata !1}
!3 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, float addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__spmv_ellpackr_kernel_separated_args, metadata !1}
!4 = metadata !{}
!5 = metadata !{metadata !"spmv_csr_scalar_kernel", metadata !"spmv_ellpackr_kernel"}
!6 = metadata !{void (i8*)* @spmv_csr_scalar_kernel}
!7 = metadata !{void (i8*)* @spmv_csr_vector_kernel}
!8 = metadata !{void (i8*)* @spmv_ellpackr_kernel}
