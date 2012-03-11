; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@spmv_csr_vector_kernel.partialSums = internal addrspace(3) global [128 x double] zeroinitializer, align 16

declare void @__spmv_csr_scalar_kernel_original(double addrspace(1)* noalias nocapture, double addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32, double addrspace(1)* noalias nocapture) nounwind

declare i64 @get_global_id(i32) nounwind readnone

declare void @__spmv_csr_vector_kernel_original(double addrspace(1)* noalias nocapture, double addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32, double addrspace(1)* noalias nocapture) nounwind

declare i64 @get_local_id(i32) nounwind readnone

declare i64 @get_local_size(i32) nounwind readnone

declare i64 @get_group_id(i32) nounwind readnone

declare void @barrier(i64)

declare void @__spmv_ellpackr_kernel_original(double addrspace(1)* noalias nocapture, double addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32, double addrspace(1)* noalias nocapture) nounwind

declare [7 x i64] @__WG.boundaries.spmv_csr_scalar_kernel_original(double addrspace(1)*, double addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, double addrspace(1)*)

declare i64 @get_base_global_id.(i32)

declare [7 x i64] @__WG.boundaries.spmv_ellpackr_kernel_original(double addrspace(1)*, double addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, double addrspace(1)*)

declare void @____Vectorized_.spmv_csr_vector_kernel_original(double addrspace(1)* noalias nocapture, double addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32, double addrspace(1)* noalias nocapture) nounwind

declare i1 @allOne(i1)

declare i1 @allZero(i1)

declare i32 @masked_load_align4_176(i1, i32 addrspace(1)*)

declare double @masked_load_align8_177(i1, double addrspace(1)*)

declare double @masked_load_align8_178(i1, double addrspace(1)*)

declare i1 @allZero_v16(<16 x i1>)

declare i1 @allOne_v16(<16 x i1>)

declare i32 @masked_load_align4_182(i1, i32 addrspace(1)*)

declare i32 @masked_load_align4_183(i1, i32 addrspace(1)*)

declare i32 @masked_load_align4_184(i1, i32 addrspace(1)*)

declare double @masked_load_align8_185(i1, double addrspace(1)*)

declare double @masked_load_align8_186(i1, double addrspace(1)*)

declare void @masked_store_align8_56(i1, double, double addrspace(3)*)

declare void @maskedf_24_barrier(i1, i64)

declare double @masked_load_align8_187(i1, double addrspace(3)*)

declare double @masked_load_align8_188(i1, double addrspace(3)*)

declare void @masked_store_align8_57(i1, double, double addrspace(3)*)

declare void @maskedf_25_barrier(i1, i64)

declare double @masked_load_align8_189(i1, double addrspace(3)*)

declare double @masked_load_align8_190(i1, double addrspace(3)*)

declare void @masked_store_align8_58(i1, double, double addrspace(3)*)

declare void @maskedf_26_barrier(i1, i64)

declare double @masked_load_align8_191(i1, double addrspace(3)*)

declare double @masked_load_align8_192(i1, double addrspace(3)*)

declare void @masked_store_align8_59(i1, double, double addrspace(3)*)

declare void @maskedf_27_barrier(i1, i64)

declare double @masked_load_align8_193(i1, double addrspace(3)*)

declare double @masked_load_align8_194(i1, double addrspace(3)*)

declare void @masked_store_align8_60(i1, double, double addrspace(3)*)

declare void @maskedf_28_barrier(i1, i64)

declare double @masked_load_align8_195(i1, double addrspace(3)*)

declare double @masked_load_align8_196(i1, double addrspace(3)*)

declare void @masked_store_align8_61(i1, double, double addrspace(3)*)

declare void @maskedf_29_barrier(i1, i64)

declare double @masked_load_align8_197(i1, double addrspace(3)*)

declare void @masked_store_align8_62(i1, double, double addrspace(1)*)

declare void @masked_store_align8_63(<16 x i1>, <16 x double>, <16 x double> addrspace(3)*)

declare <16 x double> @masked_load_align8_203(<16 x i1>, <16 x double> addrspace(3)*)

declare <16 x double> @masked_load_align8_204(<16 x i1>, <16 x double> addrspace(3)*)

declare void @masked_store_align8_64(<16 x i1>, <16 x double>, <16 x double> addrspace(3)*)

declare <16 x double> @masked_load_align8_205(<16 x i1>, <16 x double> addrspace(3)*)

declare <16 x double> @masked_load_align8_206(<16 x i1>, <16 x double> addrspace(3)*)

declare void @masked_store_align8_65(<16 x i1>, <16 x double>, <16 x double> addrspace(3)*)

declare <16 x double> @masked_load_align8_207(<16 x i1>, <16 x double> addrspace(3)*)

declare <16 x double> @masked_load_align8_208(<16 x i1>, <16 x double> addrspace(3)*)

declare void @masked_store_align8_66(<16 x i1>, <16 x double>, <16 x double> addrspace(3)*)

declare <16 x double> @masked_load_align8_209(<16 x i1>, <16 x double> addrspace(3)*)

declare <16 x double> @masked_load_align8_210(<16 x i1>, <16 x double> addrspace(3)*)

declare void @masked_store_align8_67(<16 x i1>, <16 x double>, <16 x double> addrspace(3)*)

declare <16 x double> @masked_load_align8_211(<16 x i1>, <16 x double> addrspace(3)*)

declare <16 x double> @masked_load_align8_212(<16 x i1>, <16 x double> addrspace(3)*)

declare void @masked_store_align8_68(<16 x i1>, <16 x double>, <16 x double> addrspace(3)*)

declare <16 x double> @masked_load_align8_213(<16 x i1>, <16 x double> addrspace(3)*)

declare double @masked_load_align8_214(i1, double addrspace(1)*)

declare i32 @masked_load_align4_215(i1, i32 addrspace(1)*)

declare double @masked_load_align8_216(i1, double addrspace(1)*)

declare <16 x double> @masked_load_align8_217(<16 x i1>, <16 x double> addrspace(1)*)

declare <16 x i32> @masked_load_align4_218(<16 x i1>, <16 x i32> addrspace(1)*)

declare void @dummybarrier.()

declare i8* @get_special_buffer.() nounwind readnone

declare i64 @get_iter_count.() nounwind readnone

declare i64 @get_new_local_id.(i32, i64) nounwind readnone

declare void @__spmv_csr_scalar_kernel_separated_args(double addrspace(1)* noalias nocapture, double addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32, double addrspace(1)* noalias nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare void @__spmv_csr_vector_kernel_separated_args(double addrspace(1)* noalias nocapture, double addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32, double addrspace(1)* noalias nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare void @__spmv_ellpackr_kernel_separated_args(double addrspace(1)* noalias nocapture, double addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32, double addrspace(1)* noalias nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare [7 x i64] @WG.boundaries.spmv_csr_scalar_kernel(double addrspace(1)*, double addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, double addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)

declare [7 x i64] @WG.boundaries.spmv_ellpackr_kernel(double addrspace(1)*, double addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, double addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)

declare void @____Vectorized_.spmv_csr_vector_kernel_separated_args(double addrspace(1)* noalias nocapture, double addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32, double addrspace(1)* noalias nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare i32 @llvm.x86.mic.kortestc(i16, i16) nounwind readnone

declare i32 @llvm.x86.mic.kortestz(i16, i16) nounwind readnone

define void @spmv_ellpackr_kernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
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
  %15 = bitcast i8* %14 to double addrspace(1)**
  %16 = load double addrspace(1)** %15, align 8
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

for.bodyvector_func.i:                            ; preds = %postload354vector_func.i, %for.bodyvector_func.preheader.i
  %vectorPHIvector_func.i = phi <16 x i1> [ %loop_mask1109vector_func.i, %postload354vector_func.i ], [ %negIncomingLoopMask28vector_func.i, %for.bodyvector_func.preheader.i ]
  %vectorPHI30vector_func.i = phi <16 x double> [ %out_sel103vector_func.i, %postload354vector_func.i ], [ undef, %for.bodyvector_func.preheader.i ]
  %vectorPHI31vector_func.i = phi <16 x i1> [ %local_edge111vector_func.i, %postload354vector_func.i ], [ %cmp21vector_func.i, %for.bodyvector_func.preheader.i ]
  %indvars.ivvector_func.i = phi i64 [ %indvars.iv.nextvector_func.i, %postload354vector_func.i ], [ 0, %for.bodyvector_func.preheader.i ]
  %vectorPHI32vector_func.i = phi <16 x double> [ %add11102vector_func.i, %postload354vector_func.i ], [ zeroinitializer, %for.bodyvector_func.preheader.i ]
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
  br i1 %extract70vector_func.i, label %preload290vector_func.i, label %postload291vector_func.i

preload290vector_func.i:                          ; preds = %for.bodyvector_func.i
  %43 = getelementptr inbounds double addrspace(1)* %1, i64 %extract35vector_func.i
  %vload116vector_func.i = load double addrspace(1)* %43, align 8
  br label %postload291vector_func.i

postload291vector_func.i:                         ; preds = %preload290vector_func.i, %for.bodyvector_func.i
  %phi292vector_func.i = phi double [ undef, %for.bodyvector_func.i ], [ %vload116vector_func.i, %preload290vector_func.i ]
  %vpackvector_func.i = insertelement <16 x double> undef, double %phi292vector_func.i, i32 0
  br i1 %extract71vector_func.i, label %preload389vector_func.i, label %postload390vector_func.i

preload389vector_func.i:                          ; preds = %postload291vector_func.i
  %.sum426vector_func.i = add i64 %extract35vector_func.i, 1
  %44 = getelementptr double addrspace(1)* %1, i64 %.sum426vector_func.i
  %vload119vector_func.i = load double addrspace(1)* %44, align 8
  br label %postload390vector_func.i

postload390vector_func.i:                         ; preds = %preload389vector_func.i, %postload291vector_func.i
  %phi391vector_func.i = phi double [ undef, %postload291vector_func.i ], [ %vload119vector_func.i, %preload389vector_func.i ]
  %vpack120vector_func.i = insertelement <16 x double> %vpackvector_func.i, double %phi391vector_func.i, i32 1
  br i1 %extract72vector_func.i, label %preload326vector_func.i, label %postload327vector_func.i

preload326vector_func.i:                          ; preds = %postload390vector_func.i
  %.sum425vector_func.i = add i64 %extract35vector_func.i, 2
  %45 = getelementptr double addrspace(1)* %1, i64 %.sum425vector_func.i
  %vload123vector_func.i = load double addrspace(1)* %45, align 8
  br label %postload327vector_func.i

postload327vector_func.i:                         ; preds = %preload326vector_func.i, %postload390vector_func.i
  %phi328vector_func.i = phi double [ undef, %postload390vector_func.i ], [ %vload123vector_func.i, %preload326vector_func.i ]
  %vpack124vector_func.i = insertelement <16 x double> %vpack120vector_func.i, double %phi328vector_func.i, i32 2
  br i1 %extract73vector_func.i, label %preload269vector_func.i, label %postload270vector_func.i

preload269vector_func.i:                          ; preds = %postload327vector_func.i
  %.sum424vector_func.i = add i64 %extract35vector_func.i, 3
  %46 = getelementptr double addrspace(1)* %1, i64 %.sum424vector_func.i
  %vload127vector_func.i = load double addrspace(1)* %46, align 8
  br label %postload270vector_func.i

postload270vector_func.i:                         ; preds = %preload269vector_func.i, %postload327vector_func.i
  %phi271vector_func.i = phi double [ undef, %postload327vector_func.i ], [ %vload127vector_func.i, %preload269vector_func.i ]
  %vpack128vector_func.i = insertelement <16 x double> %vpack124vector_func.i, double %phi271vector_func.i, i32 3
  br i1 %extract74vector_func.i, label %preload377vector_func.i, label %postload378vector_func.i

preload377vector_func.i:                          ; preds = %postload270vector_func.i
  %.sum423vector_func.i = add i64 %extract35vector_func.i, 4
  %47 = getelementptr double addrspace(1)* %1, i64 %.sum423vector_func.i
  %vload131vector_func.i = load double addrspace(1)* %47, align 8
  br label %postload378vector_func.i

postload378vector_func.i:                         ; preds = %preload377vector_func.i, %postload270vector_func.i
  %phi379vector_func.i = phi double [ undef, %postload270vector_func.i ], [ %vload131vector_func.i, %preload377vector_func.i ]
  %vpack132vector_func.i = insertelement <16 x double> %vpack128vector_func.i, double %phi379vector_func.i, i32 4
  br i1 %extract75vector_func.i, label %preload299vector_func.i, label %postload300vector_func.i

preload299vector_func.i:                          ; preds = %postload378vector_func.i
  %.sum422vector_func.i = add i64 %extract35vector_func.i, 5
  %48 = getelementptr double addrspace(1)* %1, i64 %.sum422vector_func.i
  %vload135vector_func.i = load double addrspace(1)* %48, align 8
  br label %postload300vector_func.i

postload300vector_func.i:                         ; preds = %preload299vector_func.i, %postload378vector_func.i
  %phi301vector_func.i = phi double [ undef, %postload378vector_func.i ], [ %vload135vector_func.i, %preload299vector_func.i ]
  %vpack136vector_func.i = insertelement <16 x double> %vpack132vector_func.i, double %phi301vector_func.i, i32 5
  br i1 %extract76vector_func.i, label %preload272vector_func.i, label %postload273vector_func.i

preload272vector_func.i:                          ; preds = %postload300vector_func.i
  %.sum421vector_func.i = add i64 %extract35vector_func.i, 6
  %49 = getelementptr double addrspace(1)* %1, i64 %.sum421vector_func.i
  %vload139vector_func.i = load double addrspace(1)* %49, align 8
  br label %postload273vector_func.i

postload273vector_func.i:                         ; preds = %preload272vector_func.i, %postload300vector_func.i
  %phi274vector_func.i = phi double [ undef, %postload300vector_func.i ], [ %vload139vector_func.i, %preload272vector_func.i ]
  %vpack140vector_func.i = insertelement <16 x double> %vpack136vector_func.i, double %phi274vector_func.i, i32 6
  br i1 %extract77vector_func.i, label %preload308vector_func.i, label %postload309vector_func.i

preload308vector_func.i:                          ; preds = %postload273vector_func.i
  %.sum420vector_func.i = add i64 %extract35vector_func.i, 7
  %50 = getelementptr double addrspace(1)* %1, i64 %.sum420vector_func.i
  %vload143vector_func.i = load double addrspace(1)* %50, align 8
  br label %postload309vector_func.i

postload309vector_func.i:                         ; preds = %preload308vector_func.i, %postload273vector_func.i
  %phi310vector_func.i = phi double [ undef, %postload273vector_func.i ], [ %vload143vector_func.i, %preload308vector_func.i ]
  %vpack144vector_func.i = insertelement <16 x double> %vpack140vector_func.i, double %phi310vector_func.i, i32 7
  br i1 %extract78vector_func.i, label %preloadvector_func.i, label %postloadvector_func.i

preloadvector_func.i:                             ; preds = %postload309vector_func.i
  %.sum419vector_func.i = add i64 %extract35vector_func.i, 8
  %51 = getelementptr double addrspace(1)* %1, i64 %.sum419vector_func.i
  %vload147vector_func.i = load double addrspace(1)* %51, align 8
  br label %postloadvector_func.i

postloadvector_func.i:                            ; preds = %preloadvector_func.i, %postload309vector_func.i
  %phivector_func.i = phi double [ undef, %postload309vector_func.i ], [ %vload147vector_func.i, %preloadvector_func.i ]
  %vpack148vector_func.i = insertelement <16 x double> %vpack144vector_func.i, double %phivector_func.i, i32 8
  br i1 %extract79vector_func.i, label %preload260vector_func.i, label %postload261vector_func.i

preload260vector_func.i:                          ; preds = %postloadvector_func.i
  %.sum418vector_func.i = add i64 %extract35vector_func.i, 9
  %52 = getelementptr double addrspace(1)* %1, i64 %.sum418vector_func.i
  %vload151vector_func.i = load double addrspace(1)* %52, align 8
  br label %postload261vector_func.i

postload261vector_func.i:                         ; preds = %preload260vector_func.i, %postloadvector_func.i
  %phi262vector_func.i = phi double [ undef, %postloadvector_func.i ], [ %vload151vector_func.i, %preload260vector_func.i ]
  %vpack152vector_func.i = insertelement <16 x double> %vpack148vector_func.i, double %phi262vector_func.i, i32 9
  br i1 %extract80vector_func.i, label %preload266vector_func.i, label %postload267vector_func.i

preload266vector_func.i:                          ; preds = %postload261vector_func.i
  %.sum417vector_func.i = add i64 %extract35vector_func.i, 10
  %53 = getelementptr double addrspace(1)* %1, i64 %.sum417vector_func.i
  %vload155vector_func.i = load double addrspace(1)* %53, align 8
  br label %postload267vector_func.i

postload267vector_func.i:                         ; preds = %preload266vector_func.i, %postload261vector_func.i
  %phi268vector_func.i = phi double [ undef, %postload261vector_func.i ], [ %vload155vector_func.i, %preload266vector_func.i ]
  %vpack156vector_func.i = insertelement <16 x double> %vpack152vector_func.i, double %phi268vector_func.i, i32 10
  br i1 %extract81vector_func.i, label %preload371vector_func.i, label %postload372vector_func.i

preload371vector_func.i:                          ; preds = %postload267vector_func.i
  %.sum416vector_func.i = add i64 %extract35vector_func.i, 11
  %54 = getelementptr double addrspace(1)* %1, i64 %.sum416vector_func.i
  %vload159vector_func.i = load double addrspace(1)* %54, align 8
  br label %postload372vector_func.i

postload372vector_func.i:                         ; preds = %preload371vector_func.i, %postload267vector_func.i
  %phi373vector_func.i = phi double [ undef, %postload267vector_func.i ], [ %vload159vector_func.i, %preload371vector_func.i ]
  %vpack160vector_func.i = insertelement <16 x double> %vpack156vector_func.i, double %phi373vector_func.i, i32 11
  br i1 %extract82vector_func.i, label %preload332vector_func.i, label %postload333vector_func.i

preload332vector_func.i:                          ; preds = %postload372vector_func.i
  %.sum415vector_func.i = add i64 %extract35vector_func.i, 12
  %55 = getelementptr double addrspace(1)* %1, i64 %.sum415vector_func.i
  %vload163vector_func.i = load double addrspace(1)* %55, align 8
  br label %postload333vector_func.i

postload333vector_func.i:                         ; preds = %preload332vector_func.i, %postload372vector_func.i
  %phi334vector_func.i = phi double [ undef, %postload372vector_func.i ], [ %vload163vector_func.i, %preload332vector_func.i ]
  %vpack164vector_func.i = insertelement <16 x double> %vpack160vector_func.i, double %phi334vector_func.i, i32 12
  br i1 %extract83vector_func.i, label %preload302vector_func.i, label %postload303vector_func.i

preload302vector_func.i:                          ; preds = %postload333vector_func.i
  %.sum414vector_func.i = add i64 %extract35vector_func.i, 13
  %56 = getelementptr double addrspace(1)* %1, i64 %.sum414vector_func.i
  %vload167vector_func.i = load double addrspace(1)* %56, align 8
  br label %postload303vector_func.i

postload303vector_func.i:                         ; preds = %preload302vector_func.i, %postload333vector_func.i
  %phi304vector_func.i = phi double [ undef, %postload333vector_func.i ], [ %vload167vector_func.i, %preload302vector_func.i ]
  %vpack168vector_func.i = insertelement <16 x double> %vpack164vector_func.i, double %phi304vector_func.i, i32 13
  br i1 %extract84vector_func.i, label %preload329vector_func.i, label %postload330vector_func.i

preload329vector_func.i:                          ; preds = %postload303vector_func.i
  %.sum413vector_func.i = add i64 %extract35vector_func.i, 14
  %57 = getelementptr double addrspace(1)* %1, i64 %.sum413vector_func.i
  %vload171vector_func.i = load double addrspace(1)* %57, align 8
  br label %postload330vector_func.i

postload330vector_func.i:                         ; preds = %preload329vector_func.i, %postload303vector_func.i
  %phi331vector_func.i = phi double [ undef, %postload303vector_func.i ], [ %vload171vector_func.i, %preload329vector_func.i ]
  %vpack172vector_func.i = insertelement <16 x double> %vpack168vector_func.i, double %phi331vector_func.i, i32 14
  br i1 %extract85vector_func.i, label %preload386vector_func.i, label %postload387vector_func.i

preload386vector_func.i:                          ; preds = %postload330vector_func.i
  %.sum412vector_func.i = add i64 %extract35vector_func.i, 15
  %58 = getelementptr double addrspace(1)* %1, i64 %.sum412vector_func.i
  %vload175vector_func.i = load double addrspace(1)* %58, align 8
  br label %postload387vector_func.i

postload387vector_func.i:                         ; preds = %preload386vector_func.i, %postload330vector_func.i
  %phi388vector_func.i = phi double [ undef, %postload330vector_func.i ], [ %vload175vector_func.i, %preload386vector_func.i ]
  %vpack176vector_func.i = insertelement <16 x double> %vpack172vector_func.i, double %phi388vector_func.i, i32 15
  br i1 %extract70vector_func.i, label %preload392vector_func.i, label %postload393vector_func.i

preload392vector_func.i:                          ; preds = %postload387vector_func.i
  %59 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract35vector_func.i
  %vload180vector_func.i = load i32 addrspace(1)* %59, align 4
  br label %postload393vector_func.i

postload393vector_func.i:                         ; preds = %preload392vector_func.i, %postload387vector_func.i
  %phi394vector_func.i = phi i32 [ undef, %postload387vector_func.i ], [ %vload180vector_func.i, %preload392vector_func.i ]
  %vpack181vector_func.i = insertelement <16 x i32> undef, i32 %phi394vector_func.i, i32 0
  br i1 %extract71vector_func.i, label %preload263vector_func.i, label %postload264vector_func.i

preload263vector_func.i:                          ; preds = %postload393vector_func.i
  %.sum411vector_func.i = add i64 %extract35vector_func.i, 1
  %60 = getelementptr i32 addrspace(1)* %7, i64 %.sum411vector_func.i
  %vload184vector_func.i = load i32 addrspace(1)* %60, align 4
  br label %postload264vector_func.i

postload264vector_func.i:                         ; preds = %preload263vector_func.i, %postload393vector_func.i
  %phi265vector_func.i = phi i32 [ undef, %postload393vector_func.i ], [ %vload184vector_func.i, %preload263vector_func.i ]
  %vpack185vector_func.i = insertelement <16 x i32> %vpack181vector_func.i, i32 %phi265vector_func.i, i32 1
  br i1 %extract72vector_func.i, label %preload335vector_func.i, label %postload336vector_func.i

preload335vector_func.i:                          ; preds = %postload264vector_func.i
  %.sum410vector_func.i = add i64 %extract35vector_func.i, 2
  %61 = getelementptr i32 addrspace(1)* %7, i64 %.sum410vector_func.i
  %vload188vector_func.i = load i32 addrspace(1)* %61, align 4
  br label %postload336vector_func.i

postload336vector_func.i:                         ; preds = %preload335vector_func.i, %postload264vector_func.i
  %phi337vector_func.i = phi i32 [ undef, %postload264vector_func.i ], [ %vload188vector_func.i, %preload335vector_func.i ]
  %vpack189vector_func.i = insertelement <16 x i32> %vpack185vector_func.i, i32 %phi337vector_func.i, i32 2
  br i1 %extract73vector_func.i, label %preload374vector_func.i, label %postload375vector_func.i

preload374vector_func.i:                          ; preds = %postload336vector_func.i
  %.sum409vector_func.i = add i64 %extract35vector_func.i, 3
  %62 = getelementptr i32 addrspace(1)* %7, i64 %.sum409vector_func.i
  %vload192vector_func.i = load i32 addrspace(1)* %62, align 4
  br label %postload375vector_func.i

postload375vector_func.i:                         ; preds = %preload374vector_func.i, %postload336vector_func.i
  %phi376vector_func.i = phi i32 [ undef, %postload336vector_func.i ], [ %vload192vector_func.i, %preload374vector_func.i ]
  %vpack193vector_func.i = insertelement <16 x i32> %vpack189vector_func.i, i32 %phi376vector_func.i, i32 3
  br i1 %extract74vector_func.i, label %preload383vector_func.i, label %postload384vector_func.i

preload383vector_func.i:                          ; preds = %postload375vector_func.i
  %.sum408vector_func.i = add i64 %extract35vector_func.i, 4
  %63 = getelementptr i32 addrspace(1)* %7, i64 %.sum408vector_func.i
  %vload196vector_func.i = load i32 addrspace(1)* %63, align 4
  br label %postload384vector_func.i

postload384vector_func.i:                         ; preds = %preload383vector_func.i, %postload375vector_func.i
  %phi385vector_func.i = phi i32 [ undef, %postload375vector_func.i ], [ %vload196vector_func.i, %preload383vector_func.i ]
  %vpack197vector_func.i = insertelement <16 x i32> %vpack193vector_func.i, i32 %phi385vector_func.i, i32 4
  br i1 %extract75vector_func.i, label %preload380vector_func.i, label %postload381vector_func.i

preload380vector_func.i:                          ; preds = %postload384vector_func.i
  %.sum407vector_func.i = add i64 %extract35vector_func.i, 5
  %64 = getelementptr i32 addrspace(1)* %7, i64 %.sum407vector_func.i
  %vload200vector_func.i = load i32 addrspace(1)* %64, align 4
  br label %postload381vector_func.i

postload381vector_func.i:                         ; preds = %preload380vector_func.i, %postload384vector_func.i
  %phi382vector_func.i = phi i32 [ undef, %postload384vector_func.i ], [ %vload200vector_func.i, %preload380vector_func.i ]
  %vpack201vector_func.i = insertelement <16 x i32> %vpack197vector_func.i, i32 %phi382vector_func.i, i32 5
  br i1 %extract76vector_func.i, label %preload395vector_func.i, label %postload396vector_func.i

preload395vector_func.i:                          ; preds = %postload381vector_func.i
  %.sum406vector_func.i = add i64 %extract35vector_func.i, 6
  %65 = getelementptr i32 addrspace(1)* %7, i64 %.sum406vector_func.i
  %vload204vector_func.i = load i32 addrspace(1)* %65, align 4
  br label %postload396vector_func.i

postload396vector_func.i:                         ; preds = %preload395vector_func.i, %postload381vector_func.i
  %phi397vector_func.i = phi i32 [ undef, %postload381vector_func.i ], [ %vload204vector_func.i, %preload395vector_func.i ]
  %vpack205vector_func.i = insertelement <16 x i32> %vpack201vector_func.i, i32 %phi397vector_func.i, i32 6
  br i1 %extract77vector_func.i, label %preload347vector_func.i, label %postload348vector_func.i

preload347vector_func.i:                          ; preds = %postload396vector_func.i
  %.sum405vector_func.i = add i64 %extract35vector_func.i, 7
  %66 = getelementptr i32 addrspace(1)* %7, i64 %.sum405vector_func.i
  %vload208vector_func.i = load i32 addrspace(1)* %66, align 4
  br label %postload348vector_func.i

postload348vector_func.i:                         ; preds = %preload347vector_func.i, %postload396vector_func.i
  %phi349vector_func.i = phi i32 [ undef, %postload396vector_func.i ], [ %vload208vector_func.i, %preload347vector_func.i ]
  %vpack209vector_func.i = insertelement <16 x i32> %vpack205vector_func.i, i32 %phi349vector_func.i, i32 7
  br i1 %extract78vector_func.i, label %preload323vector_func.i, label %postload324vector_func.i

preload323vector_func.i:                          ; preds = %postload348vector_func.i
  %.sum404vector_func.i = add i64 %extract35vector_func.i, 8
  %67 = getelementptr i32 addrspace(1)* %7, i64 %.sum404vector_func.i
  %vload212vector_func.i = load i32 addrspace(1)* %67, align 4
  br label %postload324vector_func.i

postload324vector_func.i:                         ; preds = %preload323vector_func.i, %postload348vector_func.i
  %phi325vector_func.i = phi i32 [ undef, %postload348vector_func.i ], [ %vload212vector_func.i, %preload323vector_func.i ]
  %vpack213vector_func.i = insertelement <16 x i32> %vpack209vector_func.i, i32 %phi325vector_func.i, i32 8
  br i1 %extract79vector_func.i, label %preload305vector_func.i, label %postload306vector_func.i

preload305vector_func.i:                          ; preds = %postload324vector_func.i
  %.sum403vector_func.i = add i64 %extract35vector_func.i, 9
  %68 = getelementptr i32 addrspace(1)* %7, i64 %.sum403vector_func.i
  %vload216vector_func.i = load i32 addrspace(1)* %68, align 4
  br label %postload306vector_func.i

postload306vector_func.i:                         ; preds = %preload305vector_func.i, %postload324vector_func.i
  %phi307vector_func.i = phi i32 [ undef, %postload324vector_func.i ], [ %vload216vector_func.i, %preload305vector_func.i ]
  %vpack217vector_func.i = insertelement <16 x i32> %vpack213vector_func.i, i32 %phi307vector_func.i, i32 9
  br i1 %extract80vector_func.i, label %preload257vector_func.i, label %postload258vector_func.i

preload257vector_func.i:                          ; preds = %postload306vector_func.i
  %.sum402vector_func.i = add i64 %extract35vector_func.i, 10
  %69 = getelementptr i32 addrspace(1)* %7, i64 %.sum402vector_func.i
  %vload220vector_func.i = load i32 addrspace(1)* %69, align 4
  br label %postload258vector_func.i

postload258vector_func.i:                         ; preds = %preload257vector_func.i, %postload306vector_func.i
  %phi259vector_func.i = phi i32 [ undef, %postload306vector_func.i ], [ %vload220vector_func.i, %preload257vector_func.i ]
  %vpack221vector_func.i = insertelement <16 x i32> %vpack217vector_func.i, i32 %phi259vector_func.i, i32 10
  br i1 %extract81vector_func.i, label %preload344vector_func.i, label %postload345vector_func.i

preload344vector_func.i:                          ; preds = %postload258vector_func.i
  %.sum401vector_func.i = add i64 %extract35vector_func.i, 11
  %70 = getelementptr i32 addrspace(1)* %7, i64 %.sum401vector_func.i
  %vload224vector_func.i = load i32 addrspace(1)* %70, align 4
  br label %postload345vector_func.i

postload345vector_func.i:                         ; preds = %preload344vector_func.i, %postload258vector_func.i
  %phi346vector_func.i = phi i32 [ undef, %postload258vector_func.i ], [ %vload224vector_func.i, %preload344vector_func.i ]
  %vpack225vector_func.i = insertelement <16 x i32> %vpack221vector_func.i, i32 %phi346vector_func.i, i32 11
  br i1 %extract82vector_func.i, label %preload341vector_func.i, label %postload342vector_func.i

preload341vector_func.i:                          ; preds = %postload345vector_func.i
  %.sum400vector_func.i = add i64 %extract35vector_func.i, 12
  %71 = getelementptr i32 addrspace(1)* %7, i64 %.sum400vector_func.i
  %vload228vector_func.i = load i32 addrspace(1)* %71, align 4
  br label %postload342vector_func.i

postload342vector_func.i:                         ; preds = %preload341vector_func.i, %postload345vector_func.i
  %phi343vector_func.i = phi i32 [ undef, %postload345vector_func.i ], [ %vload228vector_func.i, %preload341vector_func.i ]
  %vpack229vector_func.i = insertelement <16 x i32> %vpack225vector_func.i, i32 %phi343vector_func.i, i32 12
  br i1 %extract83vector_func.i, label %preload338vector_func.i, label %postload339vector_func.i

preload338vector_func.i:                          ; preds = %postload342vector_func.i
  %.sum399vector_func.i = add i64 %extract35vector_func.i, 13
  %72 = getelementptr i32 addrspace(1)* %7, i64 %.sum399vector_func.i
  %vload232vector_func.i = load i32 addrspace(1)* %72, align 4
  br label %postload339vector_func.i

postload339vector_func.i:                         ; preds = %preload338vector_func.i, %postload342vector_func.i
  %phi340vector_func.i = phi i32 [ undef, %postload342vector_func.i ], [ %vload232vector_func.i, %preload338vector_func.i ]
  %vpack233vector_func.i = insertelement <16 x i32> %vpack229vector_func.i, i32 %phi340vector_func.i, i32 13
  br i1 %extract84vector_func.i, label %preload320vector_func.i, label %postload321vector_func.i

preload320vector_func.i:                          ; preds = %postload339vector_func.i
  %.sum398vector_func.i = add i64 %extract35vector_func.i, 14
  %73 = getelementptr i32 addrspace(1)* %7, i64 %.sum398vector_func.i
  %vload236vector_func.i = load i32 addrspace(1)* %73, align 4
  br label %postload321vector_func.i

postload321vector_func.i:                         ; preds = %preload320vector_func.i, %postload339vector_func.i
  %phi322vector_func.i = phi i32 [ undef, %postload339vector_func.i ], [ %vload236vector_func.i, %preload320vector_func.i ]
  %vpack237vector_func.i = insertelement <16 x i32> %vpack233vector_func.i, i32 %phi322vector_func.i, i32 14
  br i1 %extract85vector_func.i, label %preload287vector_func.i, label %postload288vector_func.i

preload287vector_func.i:                          ; preds = %postload321vector_func.i
  %.sumvector_func.i = add i64 %extract35vector_func.i, 15
  %74 = getelementptr i32 addrspace(1)* %7, i64 %.sumvector_func.i
  %vload240vector_func.i = load i32 addrspace(1)* %74, align 4
  br label %postload288vector_func.i

postload288vector_func.i:                         ; preds = %preload287vector_func.i, %postload321vector_func.i
  %phi289vector_func.i = phi i32 [ undef, %postload321vector_func.i ], [ %vload240vector_func.i, %preload287vector_func.i ]
  %vpack241vector_func.i = insertelement <16 x i32> %vpack237vector_func.i, i32 %phi289vector_func.i, i32 15
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
  %75 = getelementptr inbounds double addrspace(1)* %4, i64 %extract55vector_func.i
  %76 = getelementptr inbounds double addrspace(1)* %4, i64 %extract56vector_func.i
  %77 = getelementptr inbounds double addrspace(1)* %4, i64 %extract57vector_func.i
  %78 = getelementptr inbounds double addrspace(1)* %4, i64 %extract58vector_func.i
  %79 = getelementptr inbounds double addrspace(1)* %4, i64 %extract59vector_func.i
  %80 = getelementptr inbounds double addrspace(1)* %4, i64 %extract60vector_func.i
  %81 = getelementptr inbounds double addrspace(1)* %4, i64 %extract61vector_func.i
  %82 = getelementptr inbounds double addrspace(1)* %4, i64 %extract62vector_func.i
  %83 = getelementptr inbounds double addrspace(1)* %4, i64 %extract63vector_func.i
  %84 = getelementptr inbounds double addrspace(1)* %4, i64 %extract64vector_func.i
  %85 = getelementptr inbounds double addrspace(1)* %4, i64 %extract65vector_func.i
  %86 = getelementptr inbounds double addrspace(1)* %4, i64 %extract66vector_func.i
  %87 = getelementptr inbounds double addrspace(1)* %4, i64 %extract67vector_func.i
  %88 = getelementptr inbounds double addrspace(1)* %4, i64 %extract68vector_func.i
  %89 = getelementptr inbounds double addrspace(1)* %4, i64 %extract69vector_func.i
  br i1 %extract70vector_func.i, label %preload293vector_func.i, label %postload294vector_func.i

preload293vector_func.i:                          ; preds = %postload288vector_func.i
  %extract54vector_func.i = extractelement <16 x i64> %idxprom853vector_func.i, i32 0
  %90 = getelementptr inbounds double addrspace(1)* %4, i64 %extract54vector_func.i
  %masked_loadvector_func.i = load double addrspace(1)* %90, align 8
  br label %postload294vector_func.i

postload294vector_func.i:                         ; preds = %preload293vector_func.i, %postload288vector_func.i
  %phi295vector_func.i = phi double [ undef, %postload288vector_func.i ], [ %masked_loadvector_func.i, %preload293vector_func.i ]
  br i1 %extract71vector_func.i, label %preload296vector_func.i, label %postload297vector_func.i

preload296vector_func.i:                          ; preds = %postload294vector_func.i
  %masked_load242vector_func.i = load double addrspace(1)* %75, align 8
  br label %postload297vector_func.i

postload297vector_func.i:                         ; preds = %preload296vector_func.i, %postload294vector_func.i
  %phi298vector_func.i = phi double [ undef, %postload294vector_func.i ], [ %masked_load242vector_func.i, %preload296vector_func.i ]
  br i1 %extract72vector_func.i, label %preload311vector_func.i, label %postload312vector_func.i

preload311vector_func.i:                          ; preds = %postload297vector_func.i
  %masked_load243vector_func.i = load double addrspace(1)* %76, align 8
  br label %postload312vector_func.i

postload312vector_func.i:                         ; preds = %preload311vector_func.i, %postload297vector_func.i
  %phi313vector_func.i = phi double [ undef, %postload297vector_func.i ], [ %masked_load243vector_func.i, %preload311vector_func.i ]
  br i1 %extract73vector_func.i, label %preload314vector_func.i, label %postload315vector_func.i

preload314vector_func.i:                          ; preds = %postload312vector_func.i
  %masked_load244vector_func.i = load double addrspace(1)* %77, align 8
  br label %postload315vector_func.i

postload315vector_func.i:                         ; preds = %preload314vector_func.i, %postload312vector_func.i
  %phi316vector_func.i = phi double [ undef, %postload312vector_func.i ], [ %masked_load244vector_func.i, %preload314vector_func.i ]
  br i1 %extract74vector_func.i, label %preload317vector_func.i, label %postload318vector_func.i

preload317vector_func.i:                          ; preds = %postload315vector_func.i
  %masked_load245vector_func.i = load double addrspace(1)* %78, align 8
  br label %postload318vector_func.i

postload318vector_func.i:                         ; preds = %preload317vector_func.i, %postload315vector_func.i
  %phi319vector_func.i = phi double [ undef, %postload315vector_func.i ], [ %masked_load245vector_func.i, %preload317vector_func.i ]
  br i1 %extract75vector_func.i, label %preload275vector_func.i, label %postload276vector_func.i

preload275vector_func.i:                          ; preds = %postload318vector_func.i
  %masked_load246vector_func.i = load double addrspace(1)* %79, align 8
  br label %postload276vector_func.i

postload276vector_func.i:                         ; preds = %preload275vector_func.i, %postload318vector_func.i
  %phi277vector_func.i = phi double [ undef, %postload318vector_func.i ], [ %masked_load246vector_func.i, %preload275vector_func.i ]
  br i1 %extract76vector_func.i, label %preload278vector_func.i, label %postload279vector_func.i

preload278vector_func.i:                          ; preds = %postload276vector_func.i
  %masked_load247vector_func.i = load double addrspace(1)* %80, align 8
  br label %postload279vector_func.i

postload279vector_func.i:                         ; preds = %preload278vector_func.i, %postload276vector_func.i
  %phi280vector_func.i = phi double [ undef, %postload276vector_func.i ], [ %masked_load247vector_func.i, %preload278vector_func.i ]
  br i1 %extract77vector_func.i, label %preload281vector_func.i, label %postload282vector_func.i

preload281vector_func.i:                          ; preds = %postload279vector_func.i
  %masked_load248vector_func.i = load double addrspace(1)* %81, align 8
  br label %postload282vector_func.i

postload282vector_func.i:                         ; preds = %preload281vector_func.i, %postload279vector_func.i
  %phi283vector_func.i = phi double [ undef, %postload279vector_func.i ], [ %masked_load248vector_func.i, %preload281vector_func.i ]
  br i1 %extract78vector_func.i, label %preload284vector_func.i, label %postload285vector_func.i

preload284vector_func.i:                          ; preds = %postload282vector_func.i
  %masked_load249vector_func.i = load double addrspace(1)* %82, align 8
  br label %postload285vector_func.i

postload285vector_func.i:                         ; preds = %preload284vector_func.i, %postload282vector_func.i
  %phi286vector_func.i = phi double [ undef, %postload282vector_func.i ], [ %masked_load249vector_func.i, %preload284vector_func.i ]
  br i1 %extract79vector_func.i, label %preload356vector_func.i, label %postload357vector_func.i

preload356vector_func.i:                          ; preds = %postload285vector_func.i
  %masked_load250vector_func.i = load double addrspace(1)* %83, align 8
  br label %postload357vector_func.i

postload357vector_func.i:                         ; preds = %preload356vector_func.i, %postload285vector_func.i
  %phi358vector_func.i = phi double [ undef, %postload285vector_func.i ], [ %masked_load250vector_func.i, %preload356vector_func.i ]
  br i1 %extract80vector_func.i, label %preload359vector_func.i, label %postload360vector_func.i

preload359vector_func.i:                          ; preds = %postload357vector_func.i
  %masked_load251vector_func.i = load double addrspace(1)* %84, align 8
  br label %postload360vector_func.i

postload360vector_func.i:                         ; preds = %preload359vector_func.i, %postload357vector_func.i
  %phi361vector_func.i = phi double [ undef, %postload357vector_func.i ], [ %masked_load251vector_func.i, %preload359vector_func.i ]
  br i1 %extract81vector_func.i, label %preload362vector_func.i, label %postload363vector_func.i

preload362vector_func.i:                          ; preds = %postload360vector_func.i
  %masked_load252vector_func.i = load double addrspace(1)* %85, align 8
  br label %postload363vector_func.i

postload363vector_func.i:                         ; preds = %preload362vector_func.i, %postload360vector_func.i
  %phi364vector_func.i = phi double [ undef, %postload360vector_func.i ], [ %masked_load252vector_func.i, %preload362vector_func.i ]
  br i1 %extract82vector_func.i, label %preload365vector_func.i, label %postload366vector_func.i

preload365vector_func.i:                          ; preds = %postload363vector_func.i
  %masked_load253vector_func.i = load double addrspace(1)* %86, align 8
  br label %postload366vector_func.i

postload366vector_func.i:                         ; preds = %preload365vector_func.i, %postload363vector_func.i
  %phi367vector_func.i = phi double [ undef, %postload363vector_func.i ], [ %masked_load253vector_func.i, %preload365vector_func.i ]
  br i1 %extract83vector_func.i, label %preload368vector_func.i, label %postload369vector_func.i

preload368vector_func.i:                          ; preds = %postload366vector_func.i
  %masked_load254vector_func.i = load double addrspace(1)* %87, align 8
  br label %postload369vector_func.i

postload369vector_func.i:                         ; preds = %preload368vector_func.i, %postload366vector_func.i
  %phi370vector_func.i = phi double [ undef, %postload366vector_func.i ], [ %masked_load254vector_func.i, %preload368vector_func.i ]
  br i1 %extract84vector_func.i, label %preload350vector_func.i, label %postload351vector_func.i

preload350vector_func.i:                          ; preds = %postload369vector_func.i
  %masked_load255vector_func.i = load double addrspace(1)* %88, align 8
  br label %postload351vector_func.i

postload351vector_func.i:                         ; preds = %preload350vector_func.i, %postload369vector_func.i
  %phi352vector_func.i = phi double [ undef, %postload369vector_func.i ], [ %masked_load255vector_func.i, %preload350vector_func.i ]
  br i1 %extract85vector_func.i, label %preload353vector_func.i, label %postload354vector_func.i

preload353vector_func.i:                          ; preds = %postload351vector_func.i
  %masked_load256vector_func.i = load double addrspace(1)* %89, align 8
  br label %postload354vector_func.i

postload354vector_func.i:                         ; preds = %preload353vector_func.i, %postload351vector_func.i
  %phi355vector_func.i = phi double [ undef, %postload351vector_func.i ], [ %masked_load256vector_func.i, %preload353vector_func.i ]
  %temp.vectvector_func.i = insertelement <16 x double> undef, double %phi295vector_func.i, i32 0
  %temp.vect86vector_func.i = insertelement <16 x double> %temp.vectvector_func.i, double %phi298vector_func.i, i32 1
  %temp.vect87vector_func.i = insertelement <16 x double> %temp.vect86vector_func.i, double %phi313vector_func.i, i32 2
  %temp.vect88vector_func.i = insertelement <16 x double> %temp.vect87vector_func.i, double %phi316vector_func.i, i32 3
  %temp.vect89vector_func.i = insertelement <16 x double> %temp.vect88vector_func.i, double %phi319vector_func.i, i32 4
  %temp.vect90vector_func.i = insertelement <16 x double> %temp.vect89vector_func.i, double %phi277vector_func.i, i32 5
  %temp.vect91vector_func.i = insertelement <16 x double> %temp.vect90vector_func.i, double %phi280vector_func.i, i32 6
  %temp.vect92vector_func.i = insertelement <16 x double> %temp.vect91vector_func.i, double %phi283vector_func.i, i32 7
  %temp.vect93vector_func.i = insertelement <16 x double> %temp.vect92vector_func.i, double %phi286vector_func.i, i32 8
  %temp.vect94vector_func.i = insertelement <16 x double> %temp.vect93vector_func.i, double %phi358vector_func.i, i32 9
  %temp.vect95vector_func.i = insertelement <16 x double> %temp.vect94vector_func.i, double %phi361vector_func.i, i32 10
  %temp.vect96vector_func.i = insertelement <16 x double> %temp.vect95vector_func.i, double %phi364vector_func.i, i32 11
  %temp.vect97vector_func.i = insertelement <16 x double> %temp.vect96vector_func.i, double %phi367vector_func.i, i32 12
  %temp.vect98vector_func.i = insertelement <16 x double> %temp.vect97vector_func.i, double %phi370vector_func.i, i32 13
  %temp.vect99vector_func.i = insertelement <16 x double> %temp.vect98vector_func.i, double %phi352vector_func.i, i32 14
  %temp.vect100vector_func.i = insertelement <16 x double> %temp.vect99vector_func.i, double %phi355vector_func.i, i32 15
  %mul10101vector_func.i = fmul <16 x double> %vpack176vector_func.i, %temp.vect100vector_func.i
  %add11102vector_func.i = fadd <16 x double> %vectorPHI32vector_func.i, %mul10101vector_func.i
  %out_sel103vector_func.i = select <16 x i1> %vectorPHI31vector_func.i, <16 x double> %add11102vector_func.i, <16 x double> %vectorPHI30vector_func.i
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

for.endvector_func.i:                             ; preds = %postload354vector_func.i, %entryvector_func.i
  %vectorPHI112vector_func.i = phi <16 x double> [ undef, %entryvector_func.i ], [ %out_sel103vector_func.i, %postload354vector_func.i ]
  %merge113vector_func.i = select <16 x i1> %cmp21vector_func.i, <16 x double> %vectorPHI112vector_func.i, <16 x double> zeroinitializer
  %92 = getelementptr inbounds double addrspace(1)* %16, i64 %extractvector_func.i
  %ptrTypeCast114vector_func.i = bitcast double addrspace(1)* %92 to <16 x double> addrspace(1)*
  store <16 x double> %merge113vector_func.i, <16 x double> addrspace(1)* %ptrTypeCast114vector_func.i, align 8
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
  %result.02.i = phi double [ %add11.i, %for.body.i ], [ 0.000000e+00, %scalar_kernel_entry.i ]
  %95 = trunc i64 %indvars.iv.i to i32
  %mul.i = mul nsw i32 %95, %13
  %add.i = add nsw i32 %mul.i, %conv.i
  %idxprom4.i = sext i32 %add.i to i64
  %arrayidx5.i = getelementptr inbounds double addrspace(1)* %1, i64 %idxprom4.i
  %96 = load double addrspace(1)* %arrayidx5.i, align 8
  %arrayidx7.i = getelementptr inbounds i32 addrspace(1)* %7, i64 %idxprom4.i
  %97 = load i32 addrspace(1)* %arrayidx7.i, align 4
  %idxprom8.i = sext i32 %97 to i64
  %arrayidx9.i = getelementptr inbounds double addrspace(1)* %4, i64 %idxprom8.i
  %98 = load double addrspace(1)* %arrayidx9.i, align 8
  %mul10.i = fmul double %96, %98
  %add11.i = fadd double %result.02.i, %mul10.i
  %indvars.iv.next.i = add i64 %indvars.iv.i, 1
  %lftr.wideiv.i = trunc i64 %indvars.iv.next.i to i32
  %exitcond.i = icmp eq i32 %lftr.wideiv.i, %94
  br i1 %exitcond.i, label %for.end.i, label %for.body.i

for.end.i:                                        ; preds = %for.body.i, %scalar_kernel_entry.i
  %result.0.lcssa.i = phi double [ 0.000000e+00, %scalar_kernel_entry.i ], [ %add11.i, %for.body.i ]
  %arrayidx13.i = getelementptr inbounds double addrspace(1)* %16, i64 %idxprom.i
  store double %result.0.lcssa.i, double addrspace(1)* %arrayidx13.i, align 8
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
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
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
  %15 = bitcast i8* %14 to double addrspace(1)**
  %16 = load double addrspace(1)** %15, align 8
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

for.bodyvector_func.i:                            ; preds = %postload286vector_func.i, %for.bodyvector_func.preheader.i
  %vectorPHIvector_func.i = phi <16 x i1> [ %loop_mask1157vector_func.i, %postload286vector_func.i ], [ %negIncomingLoopMask47vector_func.i, %for.bodyvector_func.preheader.i ]
  %vectorPHI49vector_func.i = phi <16 x double> [ %out_sel151vector_func.i, %postload286vector_func.i ], [ undef, %for.bodyvector_func.preheader.i ]
  %vectorPHI50vector_func.i = phi <16 x i1> [ %local_edge159vector_func.i, %postload286vector_func.i ], [ %cmp41vector_func.i, %for.bodyvector_func.preheader.i ]
  %vectorPHI51vector_func.i = phi <16 x i64> [ %indvars.iv.next152vector_func.i, %postload286vector_func.i ], [ %45, %for.bodyvector_func.preheader.i ]
  %vectorPHI52vector_func.i = phi <16 x double> [ %add12150vector_func.i, %postload286vector_func.i ], [ zeroinitializer, %for.bodyvector_func.preheader.i ]
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
  br i1 %extract69vector_func.i, label %preload297vector_func.i, label %postload298vector_func.i

preload297vector_func.i:                          ; preds = %for.bodyvector_func.i
  %61 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract53vector_func.i
  %masked_loadvector_func.i = load i32 addrspace(1)* %61, align 4
  br label %postload298vector_func.i

postload298vector_func.i:                         ; preds = %preload297vector_func.i, %for.bodyvector_func.i
  %phi299vector_func.i = phi i32 [ undef, %for.bodyvector_func.i ], [ %masked_loadvector_func.i, %preload297vector_func.i ]
  br i1 %extract70vector_func.i, label %preload252vector_func.i, label %postload253vector_func.i

preload252vector_func.i:                          ; preds = %postload298vector_func.i
  %masked_load163vector_func.i = load i32 addrspace(1)* %46, align 4
  br label %postload253vector_func.i

postload253vector_func.i:                         ; preds = %preload252vector_func.i, %postload298vector_func.i
  %phi254vector_func.i = phi i32 [ undef, %postload298vector_func.i ], [ %masked_load163vector_func.i, %preload252vector_func.i ]
  br i1 %extract71vector_func.i, label %preload288vector_func.i, label %postload289vector_func.i

preload288vector_func.i:                          ; preds = %postload253vector_func.i
  %masked_load164vector_func.i = load i32 addrspace(1)* %47, align 4
  br label %postload289vector_func.i

postload289vector_func.i:                         ; preds = %preload288vector_func.i, %postload253vector_func.i
  %phi290vector_func.i = phi i32 [ undef, %postload253vector_func.i ], [ %masked_load164vector_func.i, %preload288vector_func.i ]
  br i1 %extract72vector_func.i, label %preload306vector_func.i, label %postload307vector_func.i

preload306vector_func.i:                          ; preds = %postload289vector_func.i
  %masked_load165vector_func.i = load i32 addrspace(1)* %48, align 4
  br label %postload307vector_func.i

postload307vector_func.i:                         ; preds = %preload306vector_func.i, %postload289vector_func.i
  %phi308vector_func.i = phi i32 [ undef, %postload289vector_func.i ], [ %masked_load165vector_func.i, %preload306vector_func.i ]
  br i1 %extract73vector_func.i, label %preload315vector_func.i, label %postload316vector_func.i

preload315vector_func.i:                          ; preds = %postload307vector_func.i
  %masked_load166vector_func.i = load i32 addrspace(1)* %49, align 4
  br label %postload316vector_func.i

postload316vector_func.i:                         ; preds = %preload315vector_func.i, %postload307vector_func.i
  %phi317vector_func.i = phi i32 [ undef, %postload307vector_func.i ], [ %masked_load166vector_func.i, %preload315vector_func.i ]
  br i1 %extract74vector_func.i, label %preload324vector_func.i, label %postload325vector_func.i

preload324vector_func.i:                          ; preds = %postload316vector_func.i
  %masked_load167vector_func.i = load i32 addrspace(1)* %50, align 4
  br label %postload325vector_func.i

postload325vector_func.i:                         ; preds = %preload324vector_func.i, %postload316vector_func.i
  %phi326vector_func.i = phi i32 [ undef, %postload316vector_func.i ], [ %masked_load167vector_func.i, %preload324vector_func.i ]
  br i1 %extract75vector_func.i, label %preload333vector_func.i, label %postload334vector_func.i

preload333vector_func.i:                          ; preds = %postload325vector_func.i
  %masked_load168vector_func.i = load i32 addrspace(1)* %51, align 4
  br label %postload334vector_func.i

postload334vector_func.i:                         ; preds = %preload333vector_func.i, %postload325vector_func.i
  %phi335vector_func.i = phi i32 [ undef, %postload325vector_func.i ], [ %masked_load168vector_func.i, %preload333vector_func.i ]
  br i1 %extract76vector_func.i, label %preload342vector_func.i, label %postload343vector_func.i

preload342vector_func.i:                          ; preds = %postload334vector_func.i
  %masked_load169vector_func.i = load i32 addrspace(1)* %52, align 4
  br label %postload343vector_func.i

postload343vector_func.i:                         ; preds = %preload342vector_func.i, %postload334vector_func.i
  %phi344vector_func.i = phi i32 [ undef, %postload334vector_func.i ], [ %masked_load169vector_func.i, %preload342vector_func.i ]
  br i1 %extract77vector_func.i, label %preloadvector_func.i, label %postloadvector_func.i

preloadvector_func.i:                             ; preds = %postload343vector_func.i
  %masked_load170vector_func.i = load i32 addrspace(1)* %53, align 4
  br label %postloadvector_func.i

postloadvector_func.i:                            ; preds = %preloadvector_func.i, %postload343vector_func.i
  %phivector_func.i = phi i32 [ undef, %postload343vector_func.i ], [ %masked_load170vector_func.i, %preloadvector_func.i ]
  br i1 %extract78vector_func.i, label %preload216vector_func.i, label %postload217vector_func.i

preload216vector_func.i:                          ; preds = %postloadvector_func.i
  %masked_load171vector_func.i = load i32 addrspace(1)* %54, align 4
  br label %postload217vector_func.i

postload217vector_func.i:                         ; preds = %preload216vector_func.i, %postloadvector_func.i
  %phi218vector_func.i = phi i32 [ undef, %postloadvector_func.i ], [ %masked_load171vector_func.i, %preload216vector_func.i ]
  br i1 %extract79vector_func.i, label %preload225vector_func.i, label %postload226vector_func.i

preload225vector_func.i:                          ; preds = %postload217vector_func.i
  %masked_load172vector_func.i = load i32 addrspace(1)* %55, align 4
  br label %postload226vector_func.i

postload226vector_func.i:                         ; preds = %preload225vector_func.i, %postload217vector_func.i
  %phi227vector_func.i = phi i32 [ undef, %postload217vector_func.i ], [ %masked_load172vector_func.i, %preload225vector_func.i ]
  br i1 %extract80vector_func.i, label %preload234vector_func.i, label %postload235vector_func.i

preload234vector_func.i:                          ; preds = %postload226vector_func.i
  %masked_load173vector_func.i = load i32 addrspace(1)* %56, align 4
  br label %postload235vector_func.i

postload235vector_func.i:                         ; preds = %preload234vector_func.i, %postload226vector_func.i
  %phi236vector_func.i = phi i32 [ undef, %postload226vector_func.i ], [ %masked_load173vector_func.i, %preload234vector_func.i ]
  br i1 %extract81vector_func.i, label %preload243vector_func.i, label %postload244vector_func.i

preload243vector_func.i:                          ; preds = %postload235vector_func.i
  %masked_load174vector_func.i = load i32 addrspace(1)* %57, align 4
  br label %postload244vector_func.i

postload244vector_func.i:                         ; preds = %preload243vector_func.i, %postload235vector_func.i
  %phi245vector_func.i = phi i32 [ undef, %postload235vector_func.i ], [ %masked_load174vector_func.i, %preload243vector_func.i ]
  br i1 %extract82vector_func.i, label %preload261vector_func.i, label %postload262vector_func.i

preload261vector_func.i:                          ; preds = %postload244vector_func.i
  %masked_load175vector_func.i = load i32 addrspace(1)* %58, align 4
  br label %postload262vector_func.i

postload262vector_func.i:                         ; preds = %preload261vector_func.i, %postload244vector_func.i
  %phi263vector_func.i = phi i32 [ undef, %postload244vector_func.i ], [ %masked_load175vector_func.i, %preload261vector_func.i ]
  br i1 %extract83vector_func.i, label %preload270vector_func.i, label %postload271vector_func.i

preload270vector_func.i:                          ; preds = %postload262vector_func.i
  %masked_load176vector_func.i = load i32 addrspace(1)* %59, align 4
  br label %postload271vector_func.i

postload271vector_func.i:                         ; preds = %preload270vector_func.i, %postload262vector_func.i
  %phi272vector_func.i = phi i32 [ undef, %postload262vector_func.i ], [ %masked_load176vector_func.i, %preload270vector_func.i ]
  br i1 %extract84vector_func.i, label %preload279vector_func.i, label %postload280vector_func.i

preload279vector_func.i:                          ; preds = %postload271vector_func.i
  %masked_load177vector_func.i = load i32 addrspace(1)* %60, align 4
  br label %postload280vector_func.i

postload280vector_func.i:                         ; preds = %preload279vector_func.i, %postload271vector_func.i
  %phi281vector_func.i = phi i32 [ undef, %postload271vector_func.i ], [ %masked_load177vector_func.i, %preload279vector_func.i ]
  %temp.vectvector_func.i = insertelement <16 x i32> undef, i32 %phi299vector_func.i, i32 0
  %temp.vect85vector_func.i = insertelement <16 x i32> %temp.vectvector_func.i, i32 %phi254vector_func.i, i32 1
  %temp.vect86vector_func.i = insertelement <16 x i32> %temp.vect85vector_func.i, i32 %phi290vector_func.i, i32 2
  %temp.vect87vector_func.i = insertelement <16 x i32> %temp.vect86vector_func.i, i32 %phi308vector_func.i, i32 3
  %temp.vect88vector_func.i = insertelement <16 x i32> %temp.vect87vector_func.i, i32 %phi317vector_func.i, i32 4
  %temp.vect89vector_func.i = insertelement <16 x i32> %temp.vect88vector_func.i, i32 %phi326vector_func.i, i32 5
  %temp.vect90vector_func.i = insertelement <16 x i32> %temp.vect89vector_func.i, i32 %phi335vector_func.i, i32 6
  %temp.vect91vector_func.i = insertelement <16 x i32> %temp.vect90vector_func.i, i32 %phi344vector_func.i, i32 7
  %temp.vect92vector_func.i = insertelement <16 x i32> %temp.vect91vector_func.i, i32 %phivector_func.i, i32 8
  %temp.vect93vector_func.i = insertelement <16 x i32> %temp.vect92vector_func.i, i32 %phi218vector_func.i, i32 9
  %temp.vect94vector_func.i = insertelement <16 x i32> %temp.vect93vector_func.i, i32 %phi227vector_func.i, i32 10
  %temp.vect95vector_func.i = insertelement <16 x i32> %temp.vect94vector_func.i, i32 %phi236vector_func.i, i32 11
  %temp.vect96vector_func.i = insertelement <16 x i32> %temp.vect95vector_func.i, i32 %phi245vector_func.i, i32 12
  %temp.vect97vector_func.i = insertelement <16 x i32> %temp.vect96vector_func.i, i32 %phi263vector_func.i, i32 13
  %temp.vect98vector_func.i = insertelement <16 x i32> %temp.vect97vector_func.i, i32 %phi272vector_func.i, i32 14
  %temp.vect99vector_func.i = insertelement <16 x i32> %temp.vect98vector_func.i, i32 %phi281vector_func.i, i32 15
  %62 = getelementptr inbounds double addrspace(1)* %1, i64 %extract54vector_func.i
  %63 = getelementptr inbounds double addrspace(1)* %1, i64 %extract55vector_func.i
  %64 = getelementptr inbounds double addrspace(1)* %1, i64 %extract56vector_func.i
  %65 = getelementptr inbounds double addrspace(1)* %1, i64 %extract57vector_func.i
  %66 = getelementptr inbounds double addrspace(1)* %1, i64 %extract58vector_func.i
  %67 = getelementptr inbounds double addrspace(1)* %1, i64 %extract59vector_func.i
  %68 = getelementptr inbounds double addrspace(1)* %1, i64 %extract60vector_func.i
  %69 = getelementptr inbounds double addrspace(1)* %1, i64 %extract61vector_func.i
  %70 = getelementptr inbounds double addrspace(1)* %1, i64 %extract62vector_func.i
  %71 = getelementptr inbounds double addrspace(1)* %1, i64 %extract63vector_func.i
  %72 = getelementptr inbounds double addrspace(1)* %1, i64 %extract64vector_func.i
  %73 = getelementptr inbounds double addrspace(1)* %1, i64 %extract65vector_func.i
  %74 = getelementptr inbounds double addrspace(1)* %1, i64 %extract66vector_func.i
  %75 = getelementptr inbounds double addrspace(1)* %1, i64 %extract67vector_func.i
  %76 = getelementptr inbounds double addrspace(1)* %1, i64 %extract68vector_func.i
  br i1 %extract69vector_func.i, label %preload300vector_func.i, label %postload301vector_func.i

preload300vector_func.i:                          ; preds = %postload280vector_func.i
  %77 = getelementptr inbounds double addrspace(1)* %1, i64 %extract53vector_func.i
  %masked_load178vector_func.i = load double addrspace(1)* %77, align 8
  br label %postload301vector_func.i

postload301vector_func.i:                         ; preds = %preload300vector_func.i, %postload280vector_func.i
  %phi302vector_func.i = phi double [ undef, %postload280vector_func.i ], [ %masked_load178vector_func.i, %preload300vector_func.i ]
  br i1 %extract70vector_func.i, label %preload255vector_func.i, label %postload256vector_func.i

preload255vector_func.i:                          ; preds = %postload301vector_func.i
  %masked_load179vector_func.i = load double addrspace(1)* %62, align 8
  br label %postload256vector_func.i

postload256vector_func.i:                         ; preds = %preload255vector_func.i, %postload301vector_func.i
  %phi257vector_func.i = phi double [ undef, %postload301vector_func.i ], [ %masked_load179vector_func.i, %preload255vector_func.i ]
  br i1 %extract71vector_func.i, label %preload291vector_func.i, label %postload292vector_func.i

preload291vector_func.i:                          ; preds = %postload256vector_func.i
  %masked_load180vector_func.i = load double addrspace(1)* %63, align 8
  br label %postload292vector_func.i

postload292vector_func.i:                         ; preds = %preload291vector_func.i, %postload256vector_func.i
  %phi293vector_func.i = phi double [ undef, %postload256vector_func.i ], [ %masked_load180vector_func.i, %preload291vector_func.i ]
  br i1 %extract72vector_func.i, label %preload309vector_func.i, label %postload310vector_func.i

preload309vector_func.i:                          ; preds = %postload292vector_func.i
  %masked_load181vector_func.i = load double addrspace(1)* %64, align 8
  br label %postload310vector_func.i

postload310vector_func.i:                         ; preds = %preload309vector_func.i, %postload292vector_func.i
  %phi311vector_func.i = phi double [ undef, %postload292vector_func.i ], [ %masked_load181vector_func.i, %preload309vector_func.i ]
  br i1 %extract73vector_func.i, label %preload318vector_func.i, label %postload319vector_func.i

preload318vector_func.i:                          ; preds = %postload310vector_func.i
  %masked_load182vector_func.i = load double addrspace(1)* %65, align 8
  br label %postload319vector_func.i

postload319vector_func.i:                         ; preds = %preload318vector_func.i, %postload310vector_func.i
  %phi320vector_func.i = phi double [ undef, %postload310vector_func.i ], [ %masked_load182vector_func.i, %preload318vector_func.i ]
  br i1 %extract74vector_func.i, label %preload327vector_func.i, label %postload328vector_func.i

preload327vector_func.i:                          ; preds = %postload319vector_func.i
  %masked_load183vector_func.i = load double addrspace(1)* %66, align 8
  br label %postload328vector_func.i

postload328vector_func.i:                         ; preds = %preload327vector_func.i, %postload319vector_func.i
  %phi329vector_func.i = phi double [ undef, %postload319vector_func.i ], [ %masked_load183vector_func.i, %preload327vector_func.i ]
  br i1 %extract75vector_func.i, label %preload336vector_func.i, label %postload337vector_func.i

preload336vector_func.i:                          ; preds = %postload328vector_func.i
  %masked_load184vector_func.i = load double addrspace(1)* %67, align 8
  br label %postload337vector_func.i

postload337vector_func.i:                         ; preds = %preload336vector_func.i, %postload328vector_func.i
  %phi338vector_func.i = phi double [ undef, %postload328vector_func.i ], [ %masked_load184vector_func.i, %preload336vector_func.i ]
  br i1 %extract76vector_func.i, label %preload345vector_func.i, label %postload346vector_func.i

preload345vector_func.i:                          ; preds = %postload337vector_func.i
  %masked_load185vector_func.i = load double addrspace(1)* %68, align 8
  br label %postload346vector_func.i

postload346vector_func.i:                         ; preds = %preload345vector_func.i, %postload337vector_func.i
  %phi347vector_func.i = phi double [ undef, %postload337vector_func.i ], [ %masked_load185vector_func.i, %preload345vector_func.i ]
  br i1 %extract77vector_func.i, label %preload210vector_func.i, label %postload211vector_func.i

preload210vector_func.i:                          ; preds = %postload346vector_func.i
  %masked_load186vector_func.i = load double addrspace(1)* %69, align 8
  br label %postload211vector_func.i

postload211vector_func.i:                         ; preds = %preload210vector_func.i, %postload346vector_func.i
  %phi212vector_func.i = phi double [ undef, %postload346vector_func.i ], [ %masked_load186vector_func.i, %preload210vector_func.i ]
  br i1 %extract78vector_func.i, label %preload219vector_func.i, label %postload220vector_func.i

preload219vector_func.i:                          ; preds = %postload211vector_func.i
  %masked_load187vector_func.i = load double addrspace(1)* %70, align 8
  br label %postload220vector_func.i

postload220vector_func.i:                         ; preds = %preload219vector_func.i, %postload211vector_func.i
  %phi221vector_func.i = phi double [ undef, %postload211vector_func.i ], [ %masked_load187vector_func.i, %preload219vector_func.i ]
  br i1 %extract79vector_func.i, label %preload228vector_func.i, label %postload229vector_func.i

preload228vector_func.i:                          ; preds = %postload220vector_func.i
  %masked_load188vector_func.i = load double addrspace(1)* %71, align 8
  br label %postload229vector_func.i

postload229vector_func.i:                         ; preds = %preload228vector_func.i, %postload220vector_func.i
  %phi230vector_func.i = phi double [ undef, %postload220vector_func.i ], [ %masked_load188vector_func.i, %preload228vector_func.i ]
  br i1 %extract80vector_func.i, label %preload237vector_func.i, label %postload238vector_func.i

preload237vector_func.i:                          ; preds = %postload229vector_func.i
  %masked_load189vector_func.i = load double addrspace(1)* %72, align 8
  br label %postload238vector_func.i

postload238vector_func.i:                         ; preds = %preload237vector_func.i, %postload229vector_func.i
  %phi239vector_func.i = phi double [ undef, %postload229vector_func.i ], [ %masked_load189vector_func.i, %preload237vector_func.i ]
  br i1 %extract81vector_func.i, label %preload246vector_func.i, label %postload247vector_func.i

preload246vector_func.i:                          ; preds = %postload238vector_func.i
  %masked_load190vector_func.i = load double addrspace(1)* %73, align 8
  br label %postload247vector_func.i

postload247vector_func.i:                         ; preds = %preload246vector_func.i, %postload238vector_func.i
  %phi248vector_func.i = phi double [ undef, %postload238vector_func.i ], [ %masked_load190vector_func.i, %preload246vector_func.i ]
  br i1 %extract82vector_func.i, label %preload264vector_func.i, label %postload265vector_func.i

preload264vector_func.i:                          ; preds = %postload247vector_func.i
  %masked_load191vector_func.i = load double addrspace(1)* %74, align 8
  br label %postload265vector_func.i

postload265vector_func.i:                         ; preds = %preload264vector_func.i, %postload247vector_func.i
  %phi266vector_func.i = phi double [ undef, %postload247vector_func.i ], [ %masked_load191vector_func.i, %preload264vector_func.i ]
  br i1 %extract83vector_func.i, label %preload273vector_func.i, label %postload274vector_func.i

preload273vector_func.i:                          ; preds = %postload265vector_func.i
  %masked_load192vector_func.i = load double addrspace(1)* %75, align 8
  br label %postload274vector_func.i

postload274vector_func.i:                         ; preds = %preload273vector_func.i, %postload265vector_func.i
  %phi275vector_func.i = phi double [ undef, %postload265vector_func.i ], [ %masked_load192vector_func.i, %preload273vector_func.i ]
  br i1 %extract84vector_func.i, label %preload282vector_func.i, label %postload283vector_func.i

preload282vector_func.i:                          ; preds = %postload274vector_func.i
  %masked_load193vector_func.i = load double addrspace(1)* %76, align 8
  br label %postload283vector_func.i

postload283vector_func.i:                         ; preds = %preload282vector_func.i, %postload274vector_func.i
  %phi284vector_func.i = phi double [ undef, %postload274vector_func.i ], [ %masked_load193vector_func.i, %preload282vector_func.i ]
  %temp.vect117vector_func.i = insertelement <16 x double> undef, double %phi302vector_func.i, i32 0
  %temp.vect118vector_func.i = insertelement <16 x double> %temp.vect117vector_func.i, double %phi257vector_func.i, i32 1
  %temp.vect119vector_func.i = insertelement <16 x double> %temp.vect118vector_func.i, double %phi293vector_func.i, i32 2
  %temp.vect120vector_func.i = insertelement <16 x double> %temp.vect119vector_func.i, double %phi311vector_func.i, i32 3
  %temp.vect121vector_func.i = insertelement <16 x double> %temp.vect120vector_func.i, double %phi320vector_func.i, i32 4
  %temp.vect122vector_func.i = insertelement <16 x double> %temp.vect121vector_func.i, double %phi329vector_func.i, i32 5
  %temp.vect123vector_func.i = insertelement <16 x double> %temp.vect122vector_func.i, double %phi338vector_func.i, i32 6
  %temp.vect124vector_func.i = insertelement <16 x double> %temp.vect123vector_func.i, double %phi347vector_func.i, i32 7
  %temp.vect125vector_func.i = insertelement <16 x double> %temp.vect124vector_func.i, double %phi212vector_func.i, i32 8
  %temp.vect126vector_func.i = insertelement <16 x double> %temp.vect125vector_func.i, double %phi221vector_func.i, i32 9
  %temp.vect127vector_func.i = insertelement <16 x double> %temp.vect126vector_func.i, double %phi230vector_func.i, i32 10
  %temp.vect128vector_func.i = insertelement <16 x double> %temp.vect127vector_func.i, double %phi239vector_func.i, i32 11
  %temp.vect129vector_func.i = insertelement <16 x double> %temp.vect128vector_func.i, double %phi248vector_func.i, i32 12
  %temp.vect130vector_func.i = insertelement <16 x double> %temp.vect129vector_func.i, double %phi266vector_func.i, i32 13
  %temp.vect131vector_func.i = insertelement <16 x double> %temp.vect130vector_func.i, double %phi275vector_func.i, i32 14
  %temp.vect132vector_func.i = insertelement <16 x double> %temp.vect131vector_func.i, double %phi284vector_func.i, i32 15
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
  %78 = getelementptr inbounds double addrspace(1)* %4, i64 %extract102vector_func.i
  %79 = getelementptr inbounds double addrspace(1)* %4, i64 %extract103vector_func.i
  %80 = getelementptr inbounds double addrspace(1)* %4, i64 %extract104vector_func.i
  %81 = getelementptr inbounds double addrspace(1)* %4, i64 %extract105vector_func.i
  %82 = getelementptr inbounds double addrspace(1)* %4, i64 %extract106vector_func.i
  %83 = getelementptr inbounds double addrspace(1)* %4, i64 %extract107vector_func.i
  %84 = getelementptr inbounds double addrspace(1)* %4, i64 %extract108vector_func.i
  %85 = getelementptr inbounds double addrspace(1)* %4, i64 %extract109vector_func.i
  %86 = getelementptr inbounds double addrspace(1)* %4, i64 %extract110vector_func.i
  %87 = getelementptr inbounds double addrspace(1)* %4, i64 %extract111vector_func.i
  %88 = getelementptr inbounds double addrspace(1)* %4, i64 %extract112vector_func.i
  %89 = getelementptr inbounds double addrspace(1)* %4, i64 %extract113vector_func.i
  %90 = getelementptr inbounds double addrspace(1)* %4, i64 %extract114vector_func.i
  %91 = getelementptr inbounds double addrspace(1)* %4, i64 %extract115vector_func.i
  %92 = getelementptr inbounds double addrspace(1)* %4, i64 %extract116vector_func.i
  br i1 %extract69vector_func.i, label %preload303vector_func.i, label %postload304vector_func.i

preload303vector_func.i:                          ; preds = %postload283vector_func.i
  %extract101vector_func.i = extractelement <16 x i64> %idxprom10100vector_func.i, i32 0
  %93 = getelementptr inbounds double addrspace(1)* %4, i64 %extract101vector_func.i
  %masked_load194vector_func.i = load double addrspace(1)* %93, align 8
  br label %postload304vector_func.i

postload304vector_func.i:                         ; preds = %preload303vector_func.i, %postload283vector_func.i
  %phi305vector_func.i = phi double [ undef, %postload283vector_func.i ], [ %masked_load194vector_func.i, %preload303vector_func.i ]
  br i1 %extract70vector_func.i, label %preload258vector_func.i, label %postload259vector_func.i

preload258vector_func.i:                          ; preds = %postload304vector_func.i
  %masked_load195vector_func.i = load double addrspace(1)* %78, align 8
  br label %postload259vector_func.i

postload259vector_func.i:                         ; preds = %preload258vector_func.i, %postload304vector_func.i
  %phi260vector_func.i = phi double [ undef, %postload304vector_func.i ], [ %masked_load195vector_func.i, %preload258vector_func.i ]
  br i1 %extract71vector_func.i, label %preload294vector_func.i, label %postload295vector_func.i

preload294vector_func.i:                          ; preds = %postload259vector_func.i
  %masked_load196vector_func.i = load double addrspace(1)* %79, align 8
  br label %postload295vector_func.i

postload295vector_func.i:                         ; preds = %preload294vector_func.i, %postload259vector_func.i
  %phi296vector_func.i = phi double [ undef, %postload259vector_func.i ], [ %masked_load196vector_func.i, %preload294vector_func.i ]
  br i1 %extract72vector_func.i, label %preload312vector_func.i, label %postload313vector_func.i

preload312vector_func.i:                          ; preds = %postload295vector_func.i
  %masked_load197vector_func.i = load double addrspace(1)* %80, align 8
  br label %postload313vector_func.i

postload313vector_func.i:                         ; preds = %preload312vector_func.i, %postload295vector_func.i
  %phi314vector_func.i = phi double [ undef, %postload295vector_func.i ], [ %masked_load197vector_func.i, %preload312vector_func.i ]
  br i1 %extract73vector_func.i, label %preload321vector_func.i, label %postload322vector_func.i

preload321vector_func.i:                          ; preds = %postload313vector_func.i
  %masked_load198vector_func.i = load double addrspace(1)* %81, align 8
  br label %postload322vector_func.i

postload322vector_func.i:                         ; preds = %preload321vector_func.i, %postload313vector_func.i
  %phi323vector_func.i = phi double [ undef, %postload313vector_func.i ], [ %masked_load198vector_func.i, %preload321vector_func.i ]
  br i1 %extract74vector_func.i, label %preload330vector_func.i, label %postload331vector_func.i

preload330vector_func.i:                          ; preds = %postload322vector_func.i
  %masked_load199vector_func.i = load double addrspace(1)* %82, align 8
  br label %postload331vector_func.i

postload331vector_func.i:                         ; preds = %preload330vector_func.i, %postload322vector_func.i
  %phi332vector_func.i = phi double [ undef, %postload322vector_func.i ], [ %masked_load199vector_func.i, %preload330vector_func.i ]
  br i1 %extract75vector_func.i, label %preload339vector_func.i, label %postload340vector_func.i

preload339vector_func.i:                          ; preds = %postload331vector_func.i
  %masked_load200vector_func.i = load double addrspace(1)* %83, align 8
  br label %postload340vector_func.i

postload340vector_func.i:                         ; preds = %preload339vector_func.i, %postload331vector_func.i
  %phi341vector_func.i = phi double [ undef, %postload331vector_func.i ], [ %masked_load200vector_func.i, %preload339vector_func.i ]
  br i1 %extract76vector_func.i, label %preload348vector_func.i, label %postload349vector_func.i

preload348vector_func.i:                          ; preds = %postload340vector_func.i
  %masked_load201vector_func.i = load double addrspace(1)* %84, align 8
  br label %postload349vector_func.i

postload349vector_func.i:                         ; preds = %preload348vector_func.i, %postload340vector_func.i
  %phi350vector_func.i = phi double [ undef, %postload340vector_func.i ], [ %masked_load201vector_func.i, %preload348vector_func.i ]
  br i1 %extract77vector_func.i, label %preload213vector_func.i, label %postload214vector_func.i

preload213vector_func.i:                          ; preds = %postload349vector_func.i
  %masked_load202vector_func.i = load double addrspace(1)* %85, align 8
  br label %postload214vector_func.i

postload214vector_func.i:                         ; preds = %preload213vector_func.i, %postload349vector_func.i
  %phi215vector_func.i = phi double [ undef, %postload349vector_func.i ], [ %masked_load202vector_func.i, %preload213vector_func.i ]
  br i1 %extract78vector_func.i, label %preload222vector_func.i, label %postload223vector_func.i

preload222vector_func.i:                          ; preds = %postload214vector_func.i
  %masked_load203vector_func.i = load double addrspace(1)* %86, align 8
  br label %postload223vector_func.i

postload223vector_func.i:                         ; preds = %preload222vector_func.i, %postload214vector_func.i
  %phi224vector_func.i = phi double [ undef, %postload214vector_func.i ], [ %masked_load203vector_func.i, %preload222vector_func.i ]
  br i1 %extract79vector_func.i, label %preload231vector_func.i, label %postload232vector_func.i

preload231vector_func.i:                          ; preds = %postload223vector_func.i
  %masked_load204vector_func.i = load double addrspace(1)* %87, align 8
  br label %postload232vector_func.i

postload232vector_func.i:                         ; preds = %preload231vector_func.i, %postload223vector_func.i
  %phi233vector_func.i = phi double [ undef, %postload223vector_func.i ], [ %masked_load204vector_func.i, %preload231vector_func.i ]
  br i1 %extract80vector_func.i, label %preload240vector_func.i, label %postload241vector_func.i

preload240vector_func.i:                          ; preds = %postload232vector_func.i
  %masked_load205vector_func.i = load double addrspace(1)* %88, align 8
  br label %postload241vector_func.i

postload241vector_func.i:                         ; preds = %preload240vector_func.i, %postload232vector_func.i
  %phi242vector_func.i = phi double [ undef, %postload232vector_func.i ], [ %masked_load205vector_func.i, %preload240vector_func.i ]
  br i1 %extract81vector_func.i, label %preload249vector_func.i, label %postload250vector_func.i

preload249vector_func.i:                          ; preds = %postload241vector_func.i
  %masked_load206vector_func.i = load double addrspace(1)* %89, align 8
  br label %postload250vector_func.i

postload250vector_func.i:                         ; preds = %preload249vector_func.i, %postload241vector_func.i
  %phi251vector_func.i = phi double [ undef, %postload241vector_func.i ], [ %masked_load206vector_func.i, %preload249vector_func.i ]
  br i1 %extract82vector_func.i, label %preload267vector_func.i, label %postload268vector_func.i

preload267vector_func.i:                          ; preds = %postload250vector_func.i
  %masked_load207vector_func.i = load double addrspace(1)* %90, align 8
  br label %postload268vector_func.i

postload268vector_func.i:                         ; preds = %preload267vector_func.i, %postload250vector_func.i
  %phi269vector_func.i = phi double [ undef, %postload250vector_func.i ], [ %masked_load207vector_func.i, %preload267vector_func.i ]
  br i1 %extract83vector_func.i, label %preload276vector_func.i, label %postload277vector_func.i

preload276vector_func.i:                          ; preds = %postload268vector_func.i
  %masked_load208vector_func.i = load double addrspace(1)* %91, align 8
  br label %postload277vector_func.i

postload277vector_func.i:                         ; preds = %preload276vector_func.i, %postload268vector_func.i
  %phi278vector_func.i = phi double [ undef, %postload268vector_func.i ], [ %masked_load208vector_func.i, %preload276vector_func.i ]
  br i1 %extract84vector_func.i, label %preload285vector_func.i, label %postload286vector_func.i

preload285vector_func.i:                          ; preds = %postload277vector_func.i
  %masked_load209vector_func.i = load double addrspace(1)* %92, align 8
  br label %postload286vector_func.i

postload286vector_func.i:                         ; preds = %preload285vector_func.i, %postload277vector_func.i
  %phi287vector_func.i = phi double [ undef, %postload277vector_func.i ], [ %masked_load209vector_func.i, %preload285vector_func.i ]
  %temp.vect133vector_func.i = insertelement <16 x double> undef, double %phi305vector_func.i, i32 0
  %temp.vect134vector_func.i = insertelement <16 x double> %temp.vect133vector_func.i, double %phi260vector_func.i, i32 1
  %temp.vect135vector_func.i = insertelement <16 x double> %temp.vect134vector_func.i, double %phi296vector_func.i, i32 2
  %temp.vect136vector_func.i = insertelement <16 x double> %temp.vect135vector_func.i, double %phi314vector_func.i, i32 3
  %temp.vect137vector_func.i = insertelement <16 x double> %temp.vect136vector_func.i, double %phi323vector_func.i, i32 4
  %temp.vect138vector_func.i = insertelement <16 x double> %temp.vect137vector_func.i, double %phi332vector_func.i, i32 5
  %temp.vect139vector_func.i = insertelement <16 x double> %temp.vect138vector_func.i, double %phi341vector_func.i, i32 6
  %temp.vect140vector_func.i = insertelement <16 x double> %temp.vect139vector_func.i, double %phi350vector_func.i, i32 7
  %temp.vect141vector_func.i = insertelement <16 x double> %temp.vect140vector_func.i, double %phi215vector_func.i, i32 8
  %temp.vect142vector_func.i = insertelement <16 x double> %temp.vect141vector_func.i, double %phi224vector_func.i, i32 9
  %temp.vect143vector_func.i = insertelement <16 x double> %temp.vect142vector_func.i, double %phi233vector_func.i, i32 10
  %temp.vect144vector_func.i = insertelement <16 x double> %temp.vect143vector_func.i, double %phi242vector_func.i, i32 11
  %temp.vect145vector_func.i = insertelement <16 x double> %temp.vect144vector_func.i, double %phi251vector_func.i, i32 12
  %temp.vect146vector_func.i = insertelement <16 x double> %temp.vect145vector_func.i, double %phi269vector_func.i, i32 13
  %temp.vect147vector_func.i = insertelement <16 x double> %temp.vect146vector_func.i, double %phi278vector_func.i, i32 14
  %temp.vect148vector_func.i = insertelement <16 x double> %temp.vect147vector_func.i, double %phi287vector_func.i, i32 15
  %mul149vector_func.i = fmul <16 x double> %temp.vect132vector_func.i, %temp.vect148vector_func.i
  %add12150vector_func.i = fadd <16 x double> %vectorPHI52vector_func.i, %mul149vector_func.i
  %out_sel151vector_func.i = select <16 x i1> %vectorPHI50vector_func.i, <16 x double> %add12150vector_func.i, <16 x double> %vectorPHI49vector_func.i
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

for.endvector_func.i:                             ; preds = %postload286vector_func.i, %entryvector_func.i
  %vectorPHI160vector_func.i = phi <16 x double> [ undef, %entryvector_func.i ], [ %out_sel151vector_func.i, %postload286vector_func.i ]
  %merge161vector_func.i = select <16 x i1> %cmp41vector_func.i, <16 x double> %vectorPHI160vector_func.i, <16 x double> zeroinitializer
  %95 = getelementptr inbounds double addrspace(1)* %16, i64 %extractvector_func.i
  %ptrTypeCast162vector_func.i = bitcast double addrspace(1)* %95 to <16 x double> addrspace(1)*
  store <16 x double> %merge161vector_func.i, <16 x double> addrspace(1)* %ptrTypeCast162vector_func.i, align 8
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
  %t.02.i = phi double [ 0.000000e+00, %for.body.lr.ph.i ], [ %add12.i, %for.body.i ]
  %arrayidx7.i = getelementptr inbounds i32 addrspace(1)* %7, i64 %indvars.iv.i
  %100 = load i32 addrspace(1)* %arrayidx7.i, align 4
  %arrayidx9.i = getelementptr inbounds double addrspace(1)* %1, i64 %indvars.iv.i
  %101 = load double addrspace(1)* %arrayidx9.i, align 8
  %idxprom10.i = sext i32 %100 to i64
  %arrayidx11.i = getelementptr inbounds double addrspace(1)* %4, i64 %idxprom10.i
  %102 = load double addrspace(1)* %arrayidx11.i, align 8
  %mul.i = fmul double %101, %102
  %add12.i = fadd double %t.02.i, %mul.i
  %indvars.iv.next.i = add i64 %indvars.iv.i, 1
  %lftr.wideiv.i = trunc i64 %indvars.iv.next.i to i32
  %exitcond.i = icmp eq i32 %lftr.wideiv.i, %98
  br i1 %exitcond.i, label %for.end.i, label %for.body.i

for.end.i:                                        ; preds = %for.body.i, %scalar_kernel_entry.i
  %t.0.lcssa.i = phi double [ 0.000000e+00, %scalar_kernel_entry.i ], [ %add12.i, %for.body.i ]
  %arrayidx14.i = getelementptr inbounds double addrspace(1)* %16, i64 %idxprom.i
  store double %t.0.lcssa.i, double addrspace(1)* %arrayidx14.i, align 8
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
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
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
  %15 = bitcast i8* %14 to double addrspace(1)**
  %16 = load double addrspace(1)** %15, align 8
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
  %35 = bitcast i8 addrspace(3)* %19 to [128 x double] addrspace(3)*
  br label %SyncBB150.outer.i

SyncBB150.outer.i:                                ; preds = %thenBB188.i, %entry
  %CurrWI..0.ph.i = phi i64 [ 0, %entry ], [ %"CurrWI++192.i", %thenBB188.i ]
  %CurrSBIndex..0.ph.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride194.i", %thenBB188.i ]
  %currBarrier.0.ph.i = phi i32 [ 15, %entry ], [ %currBarrier.2.i, %thenBB188.i ]
  br label %SyncBB150.i

SyncBB150.i:                                      ; preds = %thenBB167.i, %SyncBB150.outer.i
  %CurrWI..0.i = phi i64 [ %"CurrWI++171.i", %thenBB167.i ], [ %CurrWI..0.ph.i, %SyncBB150.outer.i ]
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride173.i", %thenBB167.i ], [ %CurrSBIndex..0.ph.i, %SyncBB150.outer.i ]
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
  %arrayidx.i = getelementptr inbounds [128 x double] addrspace(3)* %35, i64 0, i64 %idxprom.i
  %"&(pSB[currWI].offset)64.i" = add nuw i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset65.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)64.i"
  %CastToValueType66.i = bitcast i8* %"&pSB[currWI].offset65.i" to double addrspace(3)**
  store double addrspace(3)* %arrayidx.i, double addrspace(3)** %CastToValueType66.i, align 8
  store volatile double 0.000000e+00, double addrspace(3)* %arrayidx.i, align 8
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
  %mySum.03.i = phi double [ 0.000000e+00, %for.body.lr.ph.i ], [ %add24.i, %for.body.i ]
  %arrayidx18.i = getelementptr inbounds i32 addrspace(1)* %7, i64 %indvars.iv.i
  %45 = load i32 addrspace(1)* %arrayidx18.i, align 4
  %arrayidx20.i = getelementptr inbounds double addrspace(1)* %1, i64 %indvars.iv.i
  %46 = load double addrspace(1)* %arrayidx20.i, align 8
  %idxprom21.i = sext i32 %45 to i64
  %arrayidx22.i = getelementptr inbounds double addrspace(1)* %4, i64 %idxprom21.i
  %47 = load double addrspace(1)* %arrayidx22.i, align 8
  %mul23.i = fmul double %46, %47
  %add24.i = fadd double %mySum.03.i, %mul23.i
  %indvars.iv.next.i = add i64 %indvars.iv.i, 32
  %48 = trunc i64 %indvars.iv.next.i to i32
  %cmp15.i = icmp slt i32 %48, %43
  br i1 %cmp15.i, label %for.body.i, label %for.end.i

for.end.i:                                        ; preds = %for.body.i, %if.then.i
  %mySum.0.lcssa.i = phi double [ 0.000000e+00, %if.then.i ], [ %add24.i, %for.body.i ]
  %loadedValue126.i = load double addrspace(3)** %CastToValueType66.i, align 8
  store volatile double %mySum.0.lcssa.i, double addrspace(3)* %loadedValue126.i, align 8
  %check.WI.iter170.i = icmp ult i64 %CurrWI..0.i, %31
  br i1 %check.WI.iter170.i, label %thenBB167.i, label %SyncBB147.i

thenBB167.i:                                      ; preds = %for.end.i
  %"CurrWI++171.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride173.i" = add nuw i64 %CurrSBIndex..0.i, 32
  br label %SyncBB150.i

SyncBB147.i:                                      ; preds = %thenBB181.i, %for.end.i
  %CurrWI..1.i = phi i64 [ %"CurrWI++185.i", %thenBB181.i ], [ 0, %for.end.i ]
  %CurrSBIndex..1.i = phi i64 [ %"loadedCurrSB+Stride187.i", %thenBB181.i ], [ 0, %for.end.i ]
  %"&(pSB[currWI].offset)591.i" = or i64 %CurrSBIndex..1.i, 4
  %"&pSB[currWI].offset60.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)591.i"
  %CastToValueType61.i = bitcast i8* %"&pSB[currWI].offset60.i" to i32*
  %loadedValue62.i = load i32* %CastToValueType61.i, align 4
  %cmp28.i = icmp ult i32 %loadedValue62.i, 16
  br i1 %cmp28.i, label %if.then30.i, label %if.end.i

if.then30.i:                                      ; preds = %SyncBB147.i
  %"&pSB[currWI].offset26.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..1.i
  %CastToValueType27.i = bitcast i8* %"&pSB[currWI].offset26.i" to i32*
  %loadedValue28.i = load i32* %CastToValueType27.i, align 4
  %add31.i = add nsw i32 %loadedValue28.i, 16
  %idxprom32.i = sext i32 %add31.i to i64
  %arrayidx33.i = getelementptr inbounds [128 x double] addrspace(3)* %35, i64 0, i64 %idxprom32.i
  %49 = load volatile double addrspace(3)* %arrayidx33.i, align 8
  %"&(pSB[currWI].offset)11315.i" = or i64 %CurrSBIndex..1.i, 8
  %"&pSB[currWI].offset114.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)11315.i"
  %CastToValueType115.i = bitcast i8* %"&pSB[currWI].offset114.i" to double addrspace(3)**
  %loadedValue116.i = load double addrspace(3)** %CastToValueType115.i, align 8
  %50 = load volatile double addrspace(3)* %loadedValue116.i, align 8
  %add36.i = fadd double %50, %49
  store volatile double %add36.i, double addrspace(3)* %loadedValue116.i, align 8
  br label %if.end.i

if.end.i:                                         ; preds = %if.then30.i, %SyncBB147.i
  %check.WI.iter184.i = icmp ult i64 %CurrWI..1.i, %31
  br i1 %check.WI.iter184.i, label %thenBB181.i, label %SyncBB149.i

thenBB181.i:                                      ; preds = %if.end.i
  %"CurrWI++185.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride187.i" = add nuw i64 %CurrSBIndex..1.i, 32
  br label %SyncBB147.i

SyncBB149.i:                                      ; preds = %thenBB160.i, %if.end.i
  %CurrWI..2.i = phi i64 [ %"CurrWI++164.i", %thenBB160.i ], [ 0, %if.end.i ]
  %CurrSBIndex..2.i = phi i64 [ %"loadedCurrSB+Stride166.i", %thenBB160.i ], [ 0, %if.end.i ]
  %"&(pSB[currWI].offset)542.i" = or i64 %CurrSBIndex..2.i, 4
  %"&pSB[currWI].offset55.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)542.i"
  %CastToValueType56.i = bitcast i8* %"&pSB[currWI].offset55.i" to i32*
  %loadedValue57.i = load i32* %CastToValueType56.i, align 4
  %cmp37.i = icmp ult i32 %loadedValue57.i, 8
  br i1 %cmp37.i, label %if.then39.i, label %if.end46.i

if.then39.i:                                      ; preds = %SyncBB149.i
  %"&pSB[currWI].offset21.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..2.i
  %CastToValueType22.i = bitcast i8* %"&pSB[currWI].offset21.i" to i32*
  %loadedValue23.i = load i32* %CastToValueType22.i, align 4
  %add40.i = add nsw i32 %loadedValue23.i, 8
  %idxprom41.i = sext i32 %add40.i to i64
  %arrayidx42.i = getelementptr inbounds [128 x double] addrspace(3)* %35, i64 0, i64 %idxprom41.i
  %51 = load volatile double addrspace(3)* %arrayidx42.i, align 8
  %"&(pSB[currWI].offset)10313.i" = or i64 %CurrSBIndex..2.i, 8
  %"&pSB[currWI].offset104.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)10313.i"
  %CastToValueType105.i = bitcast i8* %"&pSB[currWI].offset104.i" to double addrspace(3)**
  %loadedValue106.i = load double addrspace(3)** %CastToValueType105.i, align 8
  %52 = load volatile double addrspace(3)* %loadedValue106.i, align 8
  %add45.i = fadd double %52, %51
  store volatile double %add45.i, double addrspace(3)* %loadedValue106.i, align 8
  br label %if.end46.i

if.end46.i:                                       ; preds = %if.then39.i, %SyncBB149.i
  %check.WI.iter163.i = icmp ult i64 %CurrWI..2.i, %31
  br i1 %check.WI.iter163.i, label %thenBB160.i, label %SyncBB146.i

thenBB160.i:                                      ; preds = %if.end46.i
  %"CurrWI++164.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride166.i" = add nuw i64 %CurrSBIndex..2.i, 32
  br label %SyncBB149.i

SyncBB146.i:                                      ; preds = %thenBB174.i, %if.end46.i
  %CurrWI..3.i = phi i64 [ %"CurrWI++178.i", %thenBB174.i ], [ 0, %if.end46.i ]
  %CurrSBIndex..3.i = phi i64 [ %"loadedCurrSB+Stride180.i", %thenBB174.i ], [ 0, %if.end46.i ]
  %"&(pSB[currWI].offset)493.i" = or i64 %CurrSBIndex..3.i, 4
  %"&pSB[currWI].offset50.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)493.i"
  %CastToValueType51.i = bitcast i8* %"&pSB[currWI].offset50.i" to i32*
  %loadedValue52.i = load i32* %CastToValueType51.i, align 4
  %cmp47.i = icmp ult i32 %loadedValue52.i, 4
  br i1 %cmp47.i, label %if.then49.i, label %if.end56.i

if.then49.i:                                      ; preds = %SyncBB146.i
  %"&pSB[currWI].offset16.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..3.i
  %CastToValueType17.i = bitcast i8* %"&pSB[currWI].offset16.i" to i32*
  %loadedValue18.i = load i32* %CastToValueType17.i, align 4
  %add50.i = add nsw i32 %loadedValue18.i, 4
  %idxprom51.i = sext i32 %add50.i to i64
  %arrayidx52.i = getelementptr inbounds [128 x double] addrspace(3)* %35, i64 0, i64 %idxprom51.i
  %53 = load volatile double addrspace(3)* %arrayidx52.i, align 8
  %"&(pSB[currWI].offset)9311.i" = or i64 %CurrSBIndex..3.i, 8
  %"&pSB[currWI].offset94.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)9311.i"
  %CastToValueType95.i = bitcast i8* %"&pSB[currWI].offset94.i" to double addrspace(3)**
  %loadedValue96.i = load double addrspace(3)** %CastToValueType95.i, align 8
  %54 = load volatile double addrspace(3)* %loadedValue96.i, align 8
  %add55.i = fadd double %54, %53
  store volatile double %add55.i, double addrspace(3)* %loadedValue96.i, align 8
  br label %if.end56.i

if.end56.i:                                       ; preds = %if.then49.i, %SyncBB146.i
  %check.WI.iter177.i = icmp ult i64 %CurrWI..3.i, %31
  br i1 %check.WI.iter177.i, label %thenBB174.i, label %SyncBB148.i

thenBB174.i:                                      ; preds = %if.end56.i
  %"CurrWI++178.i" = add nuw i64 %CurrWI..3.i, 1
  %"loadedCurrSB+Stride180.i" = add nuw i64 %CurrSBIndex..3.i, 32
  br label %SyncBB146.i

SyncBB148.i:                                      ; preds = %thenBB.i, %if.end56.i
  %CurrWI..4.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %if.end56.i ]
  %CurrSBIndex..4.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %if.end56.i ]
  %"&(pSB[currWI].offset)444.i" = or i64 %CurrSBIndex..4.i, 4
  %"&pSB[currWI].offset45.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)444.i"
  %CastToValueType46.i = bitcast i8* %"&pSB[currWI].offset45.i" to i32*
  %loadedValue47.i = load i32* %CastToValueType46.i, align 4
  %cmp57.i = icmp ult i32 %loadedValue47.i, 2
  br i1 %cmp57.i, label %if.then59.i, label %if.end66.i

if.then59.i:                                      ; preds = %SyncBB148.i
  %"&pSB[currWI].offset11.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..4.i
  %CastToValueType12.i = bitcast i8* %"&pSB[currWI].offset11.i" to i32*
  %loadedValue13.i = load i32* %CastToValueType12.i, align 4
  %add60.i = add nsw i32 %loadedValue13.i, 2
  %idxprom61.i = sext i32 %add60.i to i64
  %arrayidx62.i = getelementptr inbounds [128 x double] addrspace(3)* %35, i64 0, i64 %idxprom61.i
  %55 = load volatile double addrspace(3)* %arrayidx62.i, align 8
  %"&(pSB[currWI].offset)839.i" = or i64 %CurrSBIndex..4.i, 8
  %"&pSB[currWI].offset84.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)839.i"
  %CastToValueType85.i = bitcast i8* %"&pSB[currWI].offset84.i" to double addrspace(3)**
  %loadedValue86.i = load double addrspace(3)** %CastToValueType85.i, align 8
  %56 = load volatile double addrspace(3)* %loadedValue86.i, align 8
  %add65.i = fadd double %56, %55
  store volatile double %add65.i, double addrspace(3)* %loadedValue86.i, align 8
  br label %if.end66.i

if.end66.i:                                       ; preds = %if.then59.i, %SyncBB148.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..4.i, %31
  br i1 %check.WI.iter.i, label %thenBB.i, label %SyncBB.i

thenBB.i:                                         ; preds = %if.end66.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..4.i, 32
  br label %SyncBB148.i

SyncBB.i:                                         ; preds = %thenBB153.i, %if.end66.i
  %CurrWI..5.i = phi i64 [ %"CurrWI++157.i", %thenBB153.i ], [ 0, %if.end66.i ]
  %CurrSBIndex..5.i = phi i64 [ %"loadedCurrSB+Stride159.i", %thenBB153.i ], [ 0, %if.end66.i ]
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

if.then69.i:                                      ; preds = %SyncBB.i
  %"&pSB[currWI].offset7.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..5.i
  %CastToValueType8.i = bitcast i8* %"&pSB[currWI].offset7.i" to i32*
  %loadedValue.i = load i32* %CastToValueType8.i, align 4
  %add70.i = add nsw i32 %loadedValue.i, 1
  %idxprom71.i = sext i32 %add70.i to i64
  %arrayidx72.i = getelementptr inbounds [128 x double] addrspace(3)* %35, i64 0, i64 %idxprom71.i
  %57 = load volatile double addrspace(3)* %arrayidx72.i, align 8
  %"&(pSB[currWI].offset)737.i" = or i64 %CurrSBIndex..5.i, 8
  %"&pSB[currWI].offset74.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)737.i"
  %CastToValueType75.i = bitcast i8* %"&pSB[currWI].offset74.i" to double addrspace(3)**
  %loadedValue76.i = load double addrspace(3)** %CastToValueType75.i, align 8
  %58 = load volatile double addrspace(3)* %loadedValue76.i, align 8
  %add75.i = fadd double %58, %57
  store volatile double %add75.i, double addrspace(3)* %loadedValue76.i, align 8
  br label %if.end76.i

if.end76.i:                                       ; preds = %if.then69.i, %SyncBB.i
  %check.WI.iter156.i = icmp ult i64 %CurrWI..5.i, %31
  br i1 %check.WI.iter156.i, label %thenBB153.i, label %SyncBB145.i

thenBB153.i:                                      ; preds = %if.end76.i
  %"CurrWI++157.i" = add nuw i64 %CurrWI..5.i, 1
  %"loadedCurrSB+Stride159.i" = add nuw i64 %CurrSBIndex..5.i, 32
  br label %SyncBB.i

SyncBB145.i:                                      ; preds = %thenBB188.i, %if.end76.i
  %CurrWI..6.i = phi i64 [ %"CurrWI++192.i", %thenBB188.i ], [ 0, %if.end76.i ]
  %CurrSBIndex..6.i = phi i64 [ %"loadedCurrSB+Stride194.i", %thenBB188.i ], [ 0, %if.end76.i ]
  %currBarrier.1.i = phi i32 [ %currBarrier.2.i, %thenBB188.i ], [ 1, %if.end76.i ]
  %"&(pSB[currWI].offset)141.i" = add nuw i64 %CurrSBIndex..6.i, 24
  %"&pSB[currWI].offset142.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)141.i"
  %CastToValueType143.i = bitcast i8* %"&pSB[currWI].offset142.i" to i1*
  %loadedValue144.i = load i1* %CastToValueType143.i, align 1
  br i1 %loadedValue144.i, label %if.then79.i, label %if.end85.i

if.then79.i:                                      ; preds = %SyncBB145.i
  %"&(pSB[currWI].offset)68.i" = add nuw i64 %CurrSBIndex..6.i, 8
  %"&pSB[currWI].offset69.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)68.i"
  %CastToValueType70.i = bitcast i8* %"&pSB[currWI].offset69.i" to double addrspace(3)**
  %loadedValue71.i = load double addrspace(3)** %CastToValueType70.i, align 8
  %59 = load volatile double addrspace(3)* %loadedValue71.i, align 8
  %"&(pSB[currWI].offset)132.i" = add nuw i64 %CurrSBIndex..6.i, 16
  %"&pSB[currWI].offset133.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)132.i"
  %CastToValueType134.i = bitcast i8* %"&pSB[currWI].offset133.i" to i64*
  %loadedValue135.i = load i64* %CastToValueType134.i, align 8
  %arrayidx83.i = getelementptr inbounds double addrspace(1)* %16, i64 %loadedValue135.i
  store double %59, double addrspace(1)* %arrayidx83.i, align 8
  br label %if.end85.i

if.end85.i:                                       ; preds = %if.then79.i, %SyncBB145.i, %SyncBB150.i
  %CurrWI..7.i = phi i64 [ %CurrWI..6.i, %if.then79.i ], [ %CurrWI..6.i, %SyncBB145.i ], [ %CurrWI..0.i, %SyncBB150.i ]
  %CurrSBIndex..7.i = phi i64 [ %CurrSBIndex..6.i, %if.then79.i ], [ %CurrSBIndex..6.i, %SyncBB145.i ], [ %CurrSBIndex..0.i, %SyncBB150.i ]
  %currBarrier.2.i = phi i32 [ %currBarrier.1.i, %if.then79.i ], [ %currBarrier.1.i, %SyncBB145.i ], [ %currBarrier.0.ph.i, %SyncBB150.i ]
  %check.WI.iter191.i = icmp ult i64 %CurrWI..7.i, %31
  br i1 %check.WI.iter191.i, label %thenBB188.i, label %__spmv_csr_vector_kernel_separated_args.exit

thenBB188.i:                                      ; preds = %if.end85.i
  %"CurrWI++192.i" = add nuw i64 %CurrWI..7.i, 1
  %"loadedCurrSB+Stride194.i" = add nuw i64 %CurrSBIndex..7.i, 32
  %cond.i = icmp eq i32 %currBarrier.2.i, 1
  br i1 %cond.i, label %SyncBB145.i, label %SyncBB150.outer.i

__spmv_csr_vector_kernel_separated_args.exit:     ; preds = %if.end85.i
  ret void
}

define void @__Vectorized_.spmv_csr_vector_kernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
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
  %15 = bitcast i8* %14 to double addrspace(1)**
  %16 = load double addrspace(1)** %15, align 8
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
  %35 = bitcast i8 addrspace(3)* %19 to [128 x double] addrspace(3)*
  %temp97.i = insertelement <16 x i32> undef, i32 %13, i32 0
  %vector98.i = shufflevector <16 x i32> %temp97.i, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB.outer.i

SyncBB.outer.i:                                   ; preds = %thenBB4603.i, %thenBB4595.i, %thenBB4572.i, %thenBB.i, %thenBB4580.i, %thenBB4611.i, %entry
  %CurrWI..0.ph.i = phi i64 [ 0, %entry ], [ %"CurrWI++4615.i", %thenBB4611.i ], [ %"CurrWI++4584.i", %thenBB4580.i ], [ %"CurrWI++.i", %thenBB.i ], [ %"CurrWI++4576.i", %thenBB4572.i ], [ %"CurrWI++4599.i", %thenBB4595.i ], [ %"CurrWI++4607.i", %thenBB4603.i ]
  %CurrSBIndex..0.ph.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride4617.i", %thenBB4611.i ], [ %"loadedCurrSB+Stride4586.i", %thenBB4580.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ %"loadedCurrSB+Stride4578.i", %thenBB4572.i ], [ %"loadedCurrSB+Stride4601.i", %thenBB4595.i ], [ %"loadedCurrSB+Stride4609.i", %thenBB4603.i ]
  %currBarrier.0.ph.i = phi i32 [ 14, %entry ], [ %currBarrier.2.i, %thenBB4611.i ], [ %currBarrier.4.i, %thenBB4580.i ], [ %currBarrier.6.i, %thenBB.i ], [ %currBarrier.8.i, %thenBB4572.i ], [ %currBarrier.10.i, %thenBB4595.i ], [ %currBarrier.12.i, %thenBB4603.i ]
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB4588.i, %SyncBB.outer.i
  %CurrWI..0.i = phi i64 [ %"CurrWI++4592.i", %thenBB4588.i ], [ %CurrWI..0.ph.i, %SyncBB.outer.i ]
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride4594.i", %thenBB4588.i ], [ %CurrSBIndex..0.ph.i, %SyncBB.outer.i ]
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
  %44 = getelementptr inbounds [128 x double] addrspace(3)* %35, i64 0, i64 %extract.i
  %"&(pSB[currWI].offset)3885.i" = add nuw i64 %CurrSBIndex..0.i, 80
  %"&pSB[currWI].offset3886.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3885.i"
  %CastToValueType3887.i = bitcast i8* %"&pSB[currWI].offset3886.i" to double addrspace(3)**
  store double addrspace(3)* %44, double addrspace(3)** %CastToValueType3887.i, align 8
  %ptrTypeCast.i = bitcast double addrspace(3)* %44 to <16 x double> addrspace(3)*
  store <16 x double> zeroinitializer, <16 x double> addrspace(3)* %ptrTypeCast.i, align 8
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
  br i1 %extract117.i, label %preload2505.i, label %postload2506.i

preload2505.i:                                    ; preds = %SyncBB.i
  %60 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract101.i
  %masked_load.i = load i32 addrspace(1)* %60, align 4
  br label %postload2506.i

postload2506.i:                                   ; preds = %preload2505.i, %SyncBB.i
  %phi2507.i = phi i32 [ undef, %SyncBB.i ], [ %masked_load.i, %preload2505.i ]
  br i1 %extract118.i, label %preload2541.i, label %postload2542.i

preload2541.i:                                    ; preds = %postload2506.i
  %masked_load484.i = load i32 addrspace(1)* %45, align 4
  br label %postload2542.i

postload2542.i:                                   ; preds = %preload2541.i, %postload2506.i
  %phi2543.i = phi i32 [ undef, %postload2506.i ], [ %masked_load484.i, %preload2541.i ]
  br i1 %extract119.i, label %preload2547.i, label %postload2548.i

preload2547.i:                                    ; preds = %postload2542.i
  %masked_load485.i = load i32 addrspace(1)* %46, align 4
  br label %postload2548.i

postload2548.i:                                   ; preds = %preload2547.i, %postload2542.i
  %phi2549.i = phi i32 [ undef, %postload2542.i ], [ %masked_load485.i, %preload2547.i ]
  br i1 %extract120.i, label %preload2553.i, label %postload2554.i

preload2553.i:                                    ; preds = %postload2548.i
  %masked_load486.i = load i32 addrspace(1)* %47, align 4
  br label %postload2554.i

postload2554.i:                                   ; preds = %preload2553.i, %postload2548.i
  %phi2555.i = phi i32 [ undef, %postload2548.i ], [ %masked_load486.i, %preload2553.i ]
  br i1 %extract121.i, label %preload2559.i, label %postload2560.i

preload2559.i:                                    ; preds = %postload2554.i
  %masked_load487.i = load i32 addrspace(1)* %48, align 4
  br label %postload2560.i

postload2560.i:                                   ; preds = %preload2559.i, %postload2554.i
  %phi2561.i = phi i32 [ undef, %postload2554.i ], [ %masked_load487.i, %preload2559.i ]
  br i1 %extract122.i, label %preload2565.i, label %postload2566.i

preload2565.i:                                    ; preds = %postload2560.i
  %masked_load488.i = load i32 addrspace(1)* %49, align 4
  br label %postload2566.i

postload2566.i:                                   ; preds = %preload2565.i, %postload2560.i
  %phi2567.i = phi i32 [ undef, %postload2560.i ], [ %masked_load488.i, %preload2565.i ]
  br i1 %extract123.i, label %preload2571.i, label %postload2572.i

preload2571.i:                                    ; preds = %postload2566.i
  %masked_load489.i = load i32 addrspace(1)* %50, align 4
  br label %postload2572.i

postload2572.i:                                   ; preds = %preload2571.i, %postload2566.i
  %phi2573.i = phi i32 [ undef, %postload2566.i ], [ %masked_load489.i, %preload2571.i ]
  br i1 %extract124.i, label %preload1977.i, label %postload1978.i

preload1977.i:                                    ; preds = %postload2572.i
  %masked_load490.i = load i32 addrspace(1)* %51, align 4
  br label %postload1978.i

postload1978.i:                                   ; preds = %preload1977.i, %postload2572.i
  %phi1979.i = phi i32 [ undef, %postload2572.i ], [ %masked_load490.i, %preload1977.i ]
  br i1 %extract125.i, label %preload1983.i, label %postload1984.i

preload1983.i:                                    ; preds = %postload1978.i
  %masked_load491.i = load i32 addrspace(1)* %52, align 4
  br label %postload1984.i

postload1984.i:                                   ; preds = %preload1983.i, %postload1978.i
  %phi1985.i = phi i32 [ undef, %postload1978.i ], [ %masked_load491.i, %preload1983.i ]
  br i1 %extract126.i, label %preload1989.i, label %postload1990.i

preload1989.i:                                    ; preds = %postload1984.i
  %masked_load492.i = load i32 addrspace(1)* %53, align 4
  br label %postload1990.i

postload1990.i:                                   ; preds = %preload1989.i, %postload1984.i
  %phi1991.i = phi i32 [ undef, %postload1984.i ], [ %masked_load492.i, %preload1989.i ]
  br i1 %extract127.i, label %preload1995.i, label %postload1996.i

preload1995.i:                                    ; preds = %postload1990.i
  %masked_load493.i = load i32 addrspace(1)* %54, align 4
  br label %postload1996.i

postload1996.i:                                   ; preds = %preload1995.i, %postload1990.i
  %phi1997.i = phi i32 [ undef, %postload1990.i ], [ %masked_load493.i, %preload1995.i ]
  br i1 %extract128.i, label %preload2001.i, label %postload2002.i

preload2001.i:                                    ; preds = %postload1996.i
  %masked_load494.i = load i32 addrspace(1)* %55, align 4
  br label %postload2002.i

postload2002.i:                                   ; preds = %preload2001.i, %postload1996.i
  %phi2003.i = phi i32 [ undef, %postload1996.i ], [ %masked_load494.i, %preload2001.i ]
  br i1 %extract129.i, label %preload2007.i, label %postload2008.i

preload2007.i:                                    ; preds = %postload2002.i
  %masked_load495.i = load i32 addrspace(1)* %56, align 4
  br label %postload2008.i

postload2008.i:                                   ; preds = %preload2007.i, %postload2002.i
  %phi2009.i = phi i32 [ undef, %postload2002.i ], [ %masked_load495.i, %preload2007.i ]
  br i1 %extract130.i, label %preload2346.i, label %postload2347.i

preload2346.i:                                    ; preds = %postload2008.i
  %masked_load496.i = load i32 addrspace(1)* %57, align 4
  br label %postload2347.i

postload2347.i:                                   ; preds = %preload2346.i, %postload2008.i
  %phi2348.i = phi i32 [ undef, %postload2008.i ], [ %masked_load496.i, %preload2346.i ]
  br i1 %extract131.i, label %preload2352.i, label %postload2353.i

preload2352.i:                                    ; preds = %postload2347.i
  %masked_load497.i = load i32 addrspace(1)* %58, align 4
  br label %postload2353.i

postload2353.i:                                   ; preds = %preload2352.i, %postload2347.i
  %phi2354.i = phi i32 [ undef, %postload2347.i ], [ %masked_load497.i, %preload2352.i ]
  br i1 %extract132.i, label %preload2358.i, label %postload2359.i

preload2358.i:                                    ; preds = %postload2353.i
  %masked_load498.i = load i32 addrspace(1)* %59, align 4
  br label %postload2359.i

postload2359.i:                                   ; preds = %preload2358.i, %postload2353.i
  %phi2360.i = phi i32 [ undef, %postload2353.i ], [ %masked_load498.i, %preload2358.i ]
  %temp.vect.i = insertelement <16 x i32> undef, i32 %phi2507.i, i32 0
  %temp.vect151.i = insertelement <16 x i32> %temp.vect.i, i32 %phi2543.i, i32 1
  %temp.vect152.i = insertelement <16 x i32> %temp.vect151.i, i32 %phi2549.i, i32 2
  %temp.vect153.i = insertelement <16 x i32> %temp.vect152.i, i32 %phi2555.i, i32 3
  %temp.vect154.i = insertelement <16 x i32> %temp.vect153.i, i32 %phi2561.i, i32 4
  %temp.vect155.i = insertelement <16 x i32> %temp.vect154.i, i32 %phi2567.i, i32 5
  %temp.vect156.i = insertelement <16 x i32> %temp.vect155.i, i32 %phi2573.i, i32 6
  %temp.vect157.i = insertelement <16 x i32> %temp.vect156.i, i32 %phi1979.i, i32 7
  %temp.vect158.i = insertelement <16 x i32> %temp.vect157.i, i32 %phi1985.i, i32 8
  %temp.vect159.i = insertelement <16 x i32> %temp.vect158.i, i32 %phi1991.i, i32 9
  %temp.vect160.i = insertelement <16 x i32> %temp.vect159.i, i32 %phi1997.i, i32 10
  %temp.vect161.i = insertelement <16 x i32> %temp.vect160.i, i32 %phi2003.i, i32 11
  %temp.vect162.i = insertelement <16 x i32> %temp.vect161.i, i32 %phi2009.i, i32 12
  %temp.vect163.i = insertelement <16 x i32> %temp.vect162.i, i32 %phi2348.i, i32 13
  %temp.vect164.i = insertelement <16 x i32> %temp.vect163.i, i32 %phi2354.i, i32 14
  %temp.vect165.i = insertelement <16 x i32> %temp.vect164.i, i32 %phi2360.i, i32 15
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
  br i1 %extract117.i, label %preload2508.i, label %postload2509.i

preload2508.i:                                    ; preds = %postload2359.i
  %extract135.i = extractelement <16 x i64> %idxprom12134.i, i32 0
  %76 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract135.i
  %masked_load499.i = load i32 addrspace(1)* %76, align 4
  br label %postload2509.i

postload2509.i:                                   ; preds = %preload2508.i, %postload2359.i
  %phi2510.i = phi i32 [ undef, %postload2359.i ], [ %masked_load499.i, %preload2508.i ]
  br i1 %extract118.i, label %preload2544.i, label %postload2545.i

preload2544.i:                                    ; preds = %postload2509.i
  %masked_load500.i = load i32 addrspace(1)* %61, align 4
  br label %postload2545.i

postload2545.i:                                   ; preds = %preload2544.i, %postload2509.i
  %phi2546.i = phi i32 [ undef, %postload2509.i ], [ %masked_load500.i, %preload2544.i ]
  br i1 %extract119.i, label %preload2550.i, label %postload2551.i

preload2550.i:                                    ; preds = %postload2545.i
  %masked_load501.i = load i32 addrspace(1)* %62, align 4
  br label %postload2551.i

postload2551.i:                                   ; preds = %preload2550.i, %postload2545.i
  %phi2552.i = phi i32 [ undef, %postload2545.i ], [ %masked_load501.i, %preload2550.i ]
  br i1 %extract120.i, label %preload2556.i, label %postload2557.i

preload2556.i:                                    ; preds = %postload2551.i
  %masked_load502.i = load i32 addrspace(1)* %63, align 4
  br label %postload2557.i

postload2557.i:                                   ; preds = %preload2556.i, %postload2551.i
  %phi2558.i = phi i32 [ undef, %postload2551.i ], [ %masked_load502.i, %preload2556.i ]
  br i1 %extract121.i, label %preload2562.i, label %postload2563.i

preload2562.i:                                    ; preds = %postload2557.i
  %masked_load503.i = load i32 addrspace(1)* %64, align 4
  br label %postload2563.i

postload2563.i:                                   ; preds = %preload2562.i, %postload2557.i
  %phi2564.i = phi i32 [ undef, %postload2557.i ], [ %masked_load503.i, %preload2562.i ]
  br i1 %extract122.i, label %preload2568.i, label %postload2569.i

preload2568.i:                                    ; preds = %postload2563.i
  %masked_load504.i = load i32 addrspace(1)* %65, align 4
  br label %postload2569.i

postload2569.i:                                   ; preds = %preload2568.i, %postload2563.i
  %phi2570.i = phi i32 [ undef, %postload2563.i ], [ %masked_load504.i, %preload2568.i ]
  br i1 %extract123.i, label %preload2574.i, label %postload2575.i

preload2574.i:                                    ; preds = %postload2569.i
  %masked_load505.i = load i32 addrspace(1)* %66, align 4
  br label %postload2575.i

postload2575.i:                                   ; preds = %preload2574.i, %postload2569.i
  %phi2576.i = phi i32 [ undef, %postload2569.i ], [ %masked_load505.i, %preload2574.i ]
  br i1 %extract124.i, label %preload1980.i, label %postload1981.i

preload1980.i:                                    ; preds = %postload2575.i
  %masked_load506.i = load i32 addrspace(1)* %67, align 4
  br label %postload1981.i

postload1981.i:                                   ; preds = %preload1980.i, %postload2575.i
  %phi1982.i = phi i32 [ undef, %postload2575.i ], [ %masked_load506.i, %preload1980.i ]
  br i1 %extract125.i, label %preload1986.i, label %postload1987.i

preload1986.i:                                    ; preds = %postload1981.i
  %masked_load507.i = load i32 addrspace(1)* %68, align 4
  br label %postload1987.i

postload1987.i:                                   ; preds = %preload1986.i, %postload1981.i
  %phi1988.i = phi i32 [ undef, %postload1981.i ], [ %masked_load507.i, %preload1986.i ]
  br i1 %extract126.i, label %preload1992.i, label %postload1993.i

preload1992.i:                                    ; preds = %postload1987.i
  %masked_load508.i = load i32 addrspace(1)* %69, align 4
  br label %postload1993.i

postload1993.i:                                   ; preds = %preload1992.i, %postload1987.i
  %phi1994.i = phi i32 [ undef, %postload1987.i ], [ %masked_load508.i, %preload1992.i ]
  br i1 %extract127.i, label %preload1998.i, label %postload1999.i

preload1998.i:                                    ; preds = %postload1993.i
  %masked_load509.i = load i32 addrspace(1)* %70, align 4
  br label %postload1999.i

postload1999.i:                                   ; preds = %preload1998.i, %postload1993.i
  %phi2000.i = phi i32 [ undef, %postload1993.i ], [ %masked_load509.i, %preload1998.i ]
  br i1 %extract128.i, label %preload2004.i, label %postload2005.i

preload2004.i:                                    ; preds = %postload1999.i
  %masked_load510.i = load i32 addrspace(1)* %71, align 4
  br label %postload2005.i

postload2005.i:                                   ; preds = %preload2004.i, %postload1999.i
  %phi2006.i = phi i32 [ undef, %postload1999.i ], [ %masked_load510.i, %preload2004.i ]
  br i1 %extract129.i, label %preload2010.i, label %postload2011.i

preload2010.i:                                    ; preds = %postload2005.i
  %masked_load511.i = load i32 addrspace(1)* %72, align 4
  br label %postload2011.i

postload2011.i:                                   ; preds = %preload2010.i, %postload2005.i
  %phi2012.i = phi i32 [ undef, %postload2005.i ], [ %masked_load511.i, %preload2010.i ]
  br i1 %extract130.i, label %preload2349.i, label %postload2350.i

preload2349.i:                                    ; preds = %postload2011.i
  %masked_load512.i = load i32 addrspace(1)* %73, align 4
  br label %postload2350.i

postload2350.i:                                   ; preds = %preload2349.i, %postload2011.i
  %phi2351.i = phi i32 [ undef, %postload2011.i ], [ %masked_load512.i, %preload2349.i ]
  br i1 %extract131.i, label %preload2355.i, label %postload2356.i

preload2355.i:                                    ; preds = %postload2350.i
  %masked_load513.i = load i32 addrspace(1)* %74, align 4
  br label %postload2356.i

postload2356.i:                                   ; preds = %preload2355.i, %postload2350.i
  %phi2357.i = phi i32 [ undef, %postload2350.i ], [ %masked_load513.i, %preload2355.i ]
  br i1 %extract132.i, label %preload2361.i, label %postload2362.i

preload2361.i:                                    ; preds = %postload2356.i
  %masked_load514.i = load i32 addrspace(1)* %75, align 4
  br label %postload2362.i

postload2362.i:                                   ; preds = %preload2361.i, %postload2356.i
  %phi2363.i = phi i32 [ undef, %postload2356.i ], [ %masked_load514.i, %preload2361.i ]
  %temp.vect167.i = insertelement <16 x i32> undef, i32 %phi2510.i, i32 0
  %temp.vect168.i = insertelement <16 x i32> %temp.vect167.i, i32 %phi2546.i, i32 1
  %temp.vect169.i = insertelement <16 x i32> %temp.vect168.i, i32 %phi2552.i, i32 2
  %temp.vect170.i = insertelement <16 x i32> %temp.vect169.i, i32 %phi2558.i, i32 3
  %temp.vect171.i = insertelement <16 x i32> %temp.vect170.i, i32 %phi2564.i, i32 4
  %temp.vect172.i = insertelement <16 x i32> %temp.vect171.i, i32 %phi2570.i, i32 5
  %temp.vect173.i = insertelement <16 x i32> %temp.vect172.i, i32 %phi2576.i, i32 6
  %temp.vect174.i = insertelement <16 x i32> %temp.vect173.i, i32 %phi1982.i, i32 7
  %temp.vect175.i = insertelement <16 x i32> %temp.vect174.i, i32 %phi1988.i, i32 8
  %temp.vect176.i = insertelement <16 x i32> %temp.vect175.i, i32 %phi1994.i, i32 9
  %temp.vect177.i = insertelement <16 x i32> %temp.vect176.i, i32 %phi2000.i, i32 10
  %temp.vect178.i = insertelement <16 x i32> %temp.vect177.i, i32 %phi2006.i, i32 11
  %temp.vect179.i = insertelement <16 x i32> %temp.vect178.i, i32 %phi2012.i, i32 12
  %temp.vect180.i = insertelement <16 x i32> %temp.vect179.i, i32 %phi2351.i, i32 13
  %temp.vect181.i = insertelement <16 x i32> %temp.vect180.i, i32 %phi2357.i, i32 14
  %temp.vect182.i = insertelement <16 x i32> %temp.vect181.i, i32 %phi2363.i, i32 15
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

for.body.preheader.i:                             ; preds = %postload2362.i
  %78 = sext <16 x i32> %add14166.i to <16 x i64>
  %negIncomingLoopMask186.i = xor <16 x i1> %if.then_to_for.body.lr.ph185.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  br label %for.body.i

for.body.i:                                       ; preds = %postload2245.i, %for.body.preheader.i
  %vectorPHI.i = phi <16 x i1> [ %loop_mask3296.i, %postload2245.i ], [ %negIncomingLoopMask186.i, %for.body.preheader.i ]
  %vectorPHI188.i = phi <16 x double> [ %out_sel291.i, %postload2245.i ], [ undef, %for.body.preheader.i ]
  %vectorPHI189.i = phi <16 x i1> [ %local_edge315.i, %postload2245.i ], [ %if.then_to_for.body.lr.ph185.i, %for.body.preheader.i ]
  %vectorPHI190.i = phi <16 x i64> [ %indvars.iv.next292.i, %postload2245.i ], [ %78, %for.body.preheader.i ]
  %vectorPHI191.i = phi <16 x double> [ %add24290.i, %postload2245.i ], [ zeroinitializer, %for.body.preheader.i ]
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
  br i1 %extract208.i, label %preload2103.i, label %postload2104.i

preload2103.i:                                    ; preds = %for.body.i
  %94 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract192.i
  %masked_load515.i = load i32 addrspace(1)* %94, align 4
  br label %postload2104.i

postload2104.i:                                   ; preds = %preload2103.i, %for.body.i
  %phi2105.i = phi i32 [ undef, %for.body.i ], [ %masked_load515.i, %preload2103.i ]
  br i1 %extract209.i, label %preload2112.i, label %postload2113.i

preload2112.i:                                    ; preds = %postload2104.i
  %masked_load516.i = load i32 addrspace(1)* %79, align 4
  br label %postload2113.i

postload2113.i:                                   ; preds = %preload2112.i, %postload2104.i
  %phi2114.i = phi i32 [ undef, %postload2104.i ], [ %masked_load516.i, %preload2112.i ]
  br i1 %extract210.i, label %preload2121.i, label %postload2122.i

preload2121.i:                                    ; preds = %postload2113.i
  %masked_load517.i = load i32 addrspace(1)* %80, align 4
  br label %postload2122.i

postload2122.i:                                   ; preds = %preload2121.i, %postload2113.i
  %phi2123.i = phi i32 [ undef, %postload2113.i ], [ %masked_load517.i, %preload2121.i ]
  br i1 %extract211.i, label %preload2130.i, label %postload2131.i

preload2130.i:                                    ; preds = %postload2122.i
  %masked_load518.i = load i32 addrspace(1)* %81, align 4
  br label %postload2131.i

postload2131.i:                                   ; preds = %preload2130.i, %postload2122.i
  %phi2132.i = phi i32 [ undef, %postload2122.i ], [ %masked_load518.i, %preload2130.i ]
  br i1 %extract212.i, label %preload2139.i, label %postload2140.i

preload2139.i:                                    ; preds = %postload2131.i
  %masked_load519.i = load i32 addrspace(1)* %82, align 4
  br label %postload2140.i

postload2140.i:                                   ; preds = %preload2139.i, %postload2131.i
  %phi2141.i = phi i32 [ undef, %postload2131.i ], [ %masked_load519.i, %preload2139.i ]
  br i1 %extract213.i, label %preload2148.i, label %postload2149.i

preload2148.i:                                    ; preds = %postload2140.i
  %masked_load520.i = load i32 addrspace(1)* %83, align 4
  br label %postload2149.i

postload2149.i:                                   ; preds = %preload2148.i, %postload2140.i
  %phi2150.i = phi i32 [ undef, %postload2140.i ], [ %masked_load520.i, %preload2148.i ]
  br i1 %extract214.i, label %preload2157.i, label %postload2158.i

preload2157.i:                                    ; preds = %postload2149.i
  %masked_load521.i = load i32 addrspace(1)* %84, align 4
  br label %postload2158.i

postload2158.i:                                   ; preds = %preload2157.i, %postload2149.i
  %phi2159.i = phi i32 [ undef, %postload2149.i ], [ %masked_load521.i, %preload2157.i ]
  br i1 %extract215.i, label %preload2166.i, label %postload2167.i

preload2166.i:                                    ; preds = %postload2158.i
  %masked_load522.i = load i32 addrspace(1)* %85, align 4
  br label %postload2167.i

postload2167.i:                                   ; preds = %preload2166.i, %postload2158.i
  %phi2168.i = phi i32 [ undef, %postload2158.i ], [ %masked_load522.i, %preload2166.i ]
  br i1 %extract216.i, label %preload2175.i, label %postload2176.i

preload2175.i:                                    ; preds = %postload2167.i
  %masked_load523.i = load i32 addrspace(1)* %86, align 4
  br label %postload2176.i

postload2176.i:                                   ; preds = %preload2175.i, %postload2167.i
  %phi2177.i = phi i32 [ undef, %postload2167.i ], [ %masked_load523.i, %preload2175.i ]
  br i1 %extract217.i, label %preload2184.i, label %postload2185.i

preload2184.i:                                    ; preds = %postload2176.i
  %masked_load524.i = load i32 addrspace(1)* %87, align 4
  br label %postload2185.i

postload2185.i:                                   ; preds = %preload2184.i, %postload2176.i
  %phi2186.i = phi i32 [ undef, %postload2176.i ], [ %masked_load524.i, %preload2184.i ]
  br i1 %extract218.i, label %preload2193.i, label %postload2194.i

preload2193.i:                                    ; preds = %postload2185.i
  %masked_load525.i = load i32 addrspace(1)* %88, align 4
  br label %postload2194.i

postload2194.i:                                   ; preds = %preload2193.i, %postload2185.i
  %phi2195.i = phi i32 [ undef, %postload2185.i ], [ %masked_load525.i, %preload2193.i ]
  br i1 %extract219.i, label %preload2202.i, label %postload2203.i

preload2202.i:                                    ; preds = %postload2194.i
  %masked_load526.i = load i32 addrspace(1)* %89, align 4
  br label %postload2203.i

postload2203.i:                                   ; preds = %preload2202.i, %postload2194.i
  %phi2204.i = phi i32 [ undef, %postload2194.i ], [ %masked_load526.i, %preload2202.i ]
  br i1 %extract220.i, label %preload2211.i, label %postload2212.i

preload2211.i:                                    ; preds = %postload2203.i
  %masked_load527.i = load i32 addrspace(1)* %90, align 4
  br label %postload2212.i

postload2212.i:                                   ; preds = %preload2211.i, %postload2203.i
  %phi2213.i = phi i32 [ undef, %postload2203.i ], [ %masked_load527.i, %preload2211.i ]
  br i1 %extract221.i, label %preload2220.i, label %postload2221.i

preload2220.i:                                    ; preds = %postload2212.i
  %masked_load528.i = load i32 addrspace(1)* %91, align 4
  br label %postload2221.i

postload2221.i:                                   ; preds = %preload2220.i, %postload2212.i
  %phi2222.i = phi i32 [ undef, %postload2212.i ], [ %masked_load528.i, %preload2220.i ]
  br i1 %extract222.i, label %preload2229.i, label %postload2230.i

preload2229.i:                                    ; preds = %postload2221.i
  %masked_load529.i = load i32 addrspace(1)* %92, align 4
  br label %postload2230.i

postload2230.i:                                   ; preds = %preload2229.i, %postload2221.i
  %phi2231.i = phi i32 [ undef, %postload2221.i ], [ %masked_load529.i, %preload2229.i ]
  br i1 %extract223.i, label %preload2238.i, label %postload2239.i

preload2238.i:                                    ; preds = %postload2230.i
  %masked_load530.i = load i32 addrspace(1)* %93, align 4
  br label %postload2239.i

postload2239.i:                                   ; preds = %preload2238.i, %postload2230.i
  %phi2240.i = phi i32 [ undef, %postload2230.i ], [ %masked_load530.i, %preload2238.i ]
  %temp.vect224.i = insertelement <16 x i32> undef, i32 %phi2105.i, i32 0
  %temp.vect225.i = insertelement <16 x i32> %temp.vect224.i, i32 %phi2114.i, i32 1
  %temp.vect226.i = insertelement <16 x i32> %temp.vect225.i, i32 %phi2123.i, i32 2
  %temp.vect227.i = insertelement <16 x i32> %temp.vect226.i, i32 %phi2132.i, i32 3
  %temp.vect228.i = insertelement <16 x i32> %temp.vect227.i, i32 %phi2141.i, i32 4
  %temp.vect229.i = insertelement <16 x i32> %temp.vect228.i, i32 %phi2150.i, i32 5
  %temp.vect230.i = insertelement <16 x i32> %temp.vect229.i, i32 %phi2159.i, i32 6
  %temp.vect231.i = insertelement <16 x i32> %temp.vect230.i, i32 %phi2168.i, i32 7
  %temp.vect232.i = insertelement <16 x i32> %temp.vect231.i, i32 %phi2177.i, i32 8
  %temp.vect233.i = insertelement <16 x i32> %temp.vect232.i, i32 %phi2186.i, i32 9
  %temp.vect234.i = insertelement <16 x i32> %temp.vect233.i, i32 %phi2195.i, i32 10
  %temp.vect235.i = insertelement <16 x i32> %temp.vect234.i, i32 %phi2204.i, i32 11
  %temp.vect236.i = insertelement <16 x i32> %temp.vect235.i, i32 %phi2213.i, i32 12
  %temp.vect237.i = insertelement <16 x i32> %temp.vect236.i, i32 %phi2222.i, i32 13
  %temp.vect238.i = insertelement <16 x i32> %temp.vect237.i, i32 %phi2231.i, i32 14
  %temp.vect239.i = insertelement <16 x i32> %temp.vect238.i, i32 %phi2240.i, i32 15
  %95 = getelementptr inbounds double addrspace(1)* %1, i64 %extract193.i
  %96 = getelementptr inbounds double addrspace(1)* %1, i64 %extract194.i
  %97 = getelementptr inbounds double addrspace(1)* %1, i64 %extract195.i
  %98 = getelementptr inbounds double addrspace(1)* %1, i64 %extract196.i
  %99 = getelementptr inbounds double addrspace(1)* %1, i64 %extract197.i
  %100 = getelementptr inbounds double addrspace(1)* %1, i64 %extract198.i
  %101 = getelementptr inbounds double addrspace(1)* %1, i64 %extract199.i
  %102 = getelementptr inbounds double addrspace(1)* %1, i64 %extract200.i
  %103 = getelementptr inbounds double addrspace(1)* %1, i64 %extract201.i
  %104 = getelementptr inbounds double addrspace(1)* %1, i64 %extract202.i
  %105 = getelementptr inbounds double addrspace(1)* %1, i64 %extract203.i
  %106 = getelementptr inbounds double addrspace(1)* %1, i64 %extract204.i
  %107 = getelementptr inbounds double addrspace(1)* %1, i64 %extract205.i
  %108 = getelementptr inbounds double addrspace(1)* %1, i64 %extract206.i
  %109 = getelementptr inbounds double addrspace(1)* %1, i64 %extract207.i
  br i1 %extract208.i, label %preload2106.i, label %postload2107.i

preload2106.i:                                    ; preds = %postload2239.i
  %110 = getelementptr inbounds double addrspace(1)* %1, i64 %extract192.i
  %masked_load531.i = load double addrspace(1)* %110, align 8
  br label %postload2107.i

postload2107.i:                                   ; preds = %preload2106.i, %postload2239.i
  %phi2108.i = phi double [ undef, %postload2239.i ], [ %masked_load531.i, %preload2106.i ]
  br i1 %extract209.i, label %preload2115.i, label %postload2116.i

preload2115.i:                                    ; preds = %postload2107.i
  %masked_load532.i = load double addrspace(1)* %95, align 8
  br label %postload2116.i

postload2116.i:                                   ; preds = %preload2115.i, %postload2107.i
  %phi2117.i = phi double [ undef, %postload2107.i ], [ %masked_load532.i, %preload2115.i ]
  br i1 %extract210.i, label %preload2124.i, label %postload2125.i

preload2124.i:                                    ; preds = %postload2116.i
  %masked_load533.i = load double addrspace(1)* %96, align 8
  br label %postload2125.i

postload2125.i:                                   ; preds = %preload2124.i, %postload2116.i
  %phi2126.i = phi double [ undef, %postload2116.i ], [ %masked_load533.i, %preload2124.i ]
  br i1 %extract211.i, label %preload2133.i, label %postload2134.i

preload2133.i:                                    ; preds = %postload2125.i
  %masked_load534.i = load double addrspace(1)* %97, align 8
  br label %postload2134.i

postload2134.i:                                   ; preds = %preload2133.i, %postload2125.i
  %phi2135.i = phi double [ undef, %postload2125.i ], [ %masked_load534.i, %preload2133.i ]
  br i1 %extract212.i, label %preload2142.i, label %postload2143.i

preload2142.i:                                    ; preds = %postload2134.i
  %masked_load535.i = load double addrspace(1)* %98, align 8
  br label %postload2143.i

postload2143.i:                                   ; preds = %preload2142.i, %postload2134.i
  %phi2144.i = phi double [ undef, %postload2134.i ], [ %masked_load535.i, %preload2142.i ]
  br i1 %extract213.i, label %preload2151.i, label %postload2152.i

preload2151.i:                                    ; preds = %postload2143.i
  %masked_load536.i = load double addrspace(1)* %99, align 8
  br label %postload2152.i

postload2152.i:                                   ; preds = %preload2151.i, %postload2143.i
  %phi2153.i = phi double [ undef, %postload2143.i ], [ %masked_load536.i, %preload2151.i ]
  br i1 %extract214.i, label %preload2160.i, label %postload2161.i

preload2160.i:                                    ; preds = %postload2152.i
  %masked_load537.i = load double addrspace(1)* %100, align 8
  br label %postload2161.i

postload2161.i:                                   ; preds = %preload2160.i, %postload2152.i
  %phi2162.i = phi double [ undef, %postload2152.i ], [ %masked_load537.i, %preload2160.i ]
  br i1 %extract215.i, label %preload2169.i, label %postload2170.i

preload2169.i:                                    ; preds = %postload2161.i
  %masked_load538.i = load double addrspace(1)* %101, align 8
  br label %postload2170.i

postload2170.i:                                   ; preds = %preload2169.i, %postload2161.i
  %phi2171.i = phi double [ undef, %postload2161.i ], [ %masked_load538.i, %preload2169.i ]
  br i1 %extract216.i, label %preload2178.i, label %postload2179.i

preload2178.i:                                    ; preds = %postload2170.i
  %masked_load539.i = load double addrspace(1)* %102, align 8
  br label %postload2179.i

postload2179.i:                                   ; preds = %preload2178.i, %postload2170.i
  %phi2180.i = phi double [ undef, %postload2170.i ], [ %masked_load539.i, %preload2178.i ]
  br i1 %extract217.i, label %preload2187.i, label %postload2188.i

preload2187.i:                                    ; preds = %postload2179.i
  %masked_load540.i = load double addrspace(1)* %103, align 8
  br label %postload2188.i

postload2188.i:                                   ; preds = %preload2187.i, %postload2179.i
  %phi2189.i = phi double [ undef, %postload2179.i ], [ %masked_load540.i, %preload2187.i ]
  br i1 %extract218.i, label %preload2196.i, label %postload2197.i

preload2196.i:                                    ; preds = %postload2188.i
  %masked_load541.i = load double addrspace(1)* %104, align 8
  br label %postload2197.i

postload2197.i:                                   ; preds = %preload2196.i, %postload2188.i
  %phi2198.i = phi double [ undef, %postload2188.i ], [ %masked_load541.i, %preload2196.i ]
  br i1 %extract219.i, label %preload2205.i, label %postload2206.i

preload2205.i:                                    ; preds = %postload2197.i
  %masked_load542.i = load double addrspace(1)* %105, align 8
  br label %postload2206.i

postload2206.i:                                   ; preds = %preload2205.i, %postload2197.i
  %phi2207.i = phi double [ undef, %postload2197.i ], [ %masked_load542.i, %preload2205.i ]
  br i1 %extract220.i, label %preload2214.i, label %postload2215.i

preload2214.i:                                    ; preds = %postload2206.i
  %masked_load543.i = load double addrspace(1)* %106, align 8
  br label %postload2215.i

postload2215.i:                                   ; preds = %preload2214.i, %postload2206.i
  %phi2216.i = phi double [ undef, %postload2206.i ], [ %masked_load543.i, %preload2214.i ]
  br i1 %extract221.i, label %preload2223.i, label %postload2224.i

preload2223.i:                                    ; preds = %postload2215.i
  %masked_load544.i = load double addrspace(1)* %107, align 8
  br label %postload2224.i

postload2224.i:                                   ; preds = %preload2223.i, %postload2215.i
  %phi2225.i = phi double [ undef, %postload2215.i ], [ %masked_load544.i, %preload2223.i ]
  br i1 %extract222.i, label %preload2232.i, label %postload2233.i

preload2232.i:                                    ; preds = %postload2224.i
  %masked_load545.i = load double addrspace(1)* %108, align 8
  br label %postload2233.i

postload2233.i:                                   ; preds = %preload2232.i, %postload2224.i
  %phi2234.i = phi double [ undef, %postload2224.i ], [ %masked_load545.i, %preload2232.i ]
  br i1 %extract223.i, label %preload2241.i, label %postload2242.i

preload2241.i:                                    ; preds = %postload2233.i
  %masked_load546.i = load double addrspace(1)* %109, align 8
  br label %postload2242.i

postload2242.i:                                   ; preds = %preload2241.i, %postload2233.i
  %phi2243.i = phi double [ undef, %postload2233.i ], [ %masked_load546.i, %preload2241.i ]
  %temp.vect257.i = insertelement <16 x double> undef, double %phi2108.i, i32 0
  %temp.vect258.i = insertelement <16 x double> %temp.vect257.i, double %phi2117.i, i32 1
  %temp.vect259.i = insertelement <16 x double> %temp.vect258.i, double %phi2126.i, i32 2
  %temp.vect260.i = insertelement <16 x double> %temp.vect259.i, double %phi2135.i, i32 3
  %temp.vect261.i = insertelement <16 x double> %temp.vect260.i, double %phi2144.i, i32 4
  %temp.vect262.i = insertelement <16 x double> %temp.vect261.i, double %phi2153.i, i32 5
  %temp.vect263.i = insertelement <16 x double> %temp.vect262.i, double %phi2162.i, i32 6
  %temp.vect264.i = insertelement <16 x double> %temp.vect263.i, double %phi2171.i, i32 7
  %temp.vect265.i = insertelement <16 x double> %temp.vect264.i, double %phi2180.i, i32 8
  %temp.vect266.i = insertelement <16 x double> %temp.vect265.i, double %phi2189.i, i32 9
  %temp.vect267.i = insertelement <16 x double> %temp.vect266.i, double %phi2198.i, i32 10
  %temp.vect268.i = insertelement <16 x double> %temp.vect267.i, double %phi2207.i, i32 11
  %temp.vect269.i = insertelement <16 x double> %temp.vect268.i, double %phi2216.i, i32 12
  %temp.vect270.i = insertelement <16 x double> %temp.vect269.i, double %phi2225.i, i32 13
  %temp.vect271.i = insertelement <16 x double> %temp.vect270.i, double %phi2234.i, i32 14
  %temp.vect272.i = insertelement <16 x double> %temp.vect271.i, double %phi2243.i, i32 15
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
  %111 = getelementptr inbounds double addrspace(1)* %4, i64 %extract242.i
  %112 = getelementptr inbounds double addrspace(1)* %4, i64 %extract243.i
  %113 = getelementptr inbounds double addrspace(1)* %4, i64 %extract244.i
  %114 = getelementptr inbounds double addrspace(1)* %4, i64 %extract245.i
  %115 = getelementptr inbounds double addrspace(1)* %4, i64 %extract246.i
  %116 = getelementptr inbounds double addrspace(1)* %4, i64 %extract247.i
  %117 = getelementptr inbounds double addrspace(1)* %4, i64 %extract248.i
  %118 = getelementptr inbounds double addrspace(1)* %4, i64 %extract249.i
  %119 = getelementptr inbounds double addrspace(1)* %4, i64 %extract250.i
  %120 = getelementptr inbounds double addrspace(1)* %4, i64 %extract251.i
  %121 = getelementptr inbounds double addrspace(1)* %4, i64 %extract252.i
  %122 = getelementptr inbounds double addrspace(1)* %4, i64 %extract253.i
  %123 = getelementptr inbounds double addrspace(1)* %4, i64 %extract254.i
  %124 = getelementptr inbounds double addrspace(1)* %4, i64 %extract255.i
  %125 = getelementptr inbounds double addrspace(1)* %4, i64 %extract256.i
  br i1 %extract208.i, label %preload2109.i, label %postload2110.i

preload2109.i:                                    ; preds = %postload2242.i
  %extract241.i = extractelement <16 x i64> %idxprom21240.i, i32 0
  %126 = getelementptr inbounds double addrspace(1)* %4, i64 %extract241.i
  %masked_load547.i = load double addrspace(1)* %126, align 8
  br label %postload2110.i

postload2110.i:                                   ; preds = %preload2109.i, %postload2242.i
  %phi2111.i = phi double [ undef, %postload2242.i ], [ %masked_load547.i, %preload2109.i ]
  br i1 %extract209.i, label %preload2118.i, label %postload2119.i

preload2118.i:                                    ; preds = %postload2110.i
  %masked_load548.i = load double addrspace(1)* %111, align 8
  br label %postload2119.i

postload2119.i:                                   ; preds = %preload2118.i, %postload2110.i
  %phi2120.i = phi double [ undef, %postload2110.i ], [ %masked_load548.i, %preload2118.i ]
  br i1 %extract210.i, label %preload2127.i, label %postload2128.i

preload2127.i:                                    ; preds = %postload2119.i
  %masked_load549.i = load double addrspace(1)* %112, align 8
  br label %postload2128.i

postload2128.i:                                   ; preds = %preload2127.i, %postload2119.i
  %phi2129.i = phi double [ undef, %postload2119.i ], [ %masked_load549.i, %preload2127.i ]
  br i1 %extract211.i, label %preload2136.i, label %postload2137.i

preload2136.i:                                    ; preds = %postload2128.i
  %masked_load550.i = load double addrspace(1)* %113, align 8
  br label %postload2137.i

postload2137.i:                                   ; preds = %preload2136.i, %postload2128.i
  %phi2138.i = phi double [ undef, %postload2128.i ], [ %masked_load550.i, %preload2136.i ]
  br i1 %extract212.i, label %preload2145.i, label %postload2146.i

preload2145.i:                                    ; preds = %postload2137.i
  %masked_load551.i = load double addrspace(1)* %114, align 8
  br label %postload2146.i

postload2146.i:                                   ; preds = %preload2145.i, %postload2137.i
  %phi2147.i = phi double [ undef, %postload2137.i ], [ %masked_load551.i, %preload2145.i ]
  br i1 %extract213.i, label %preload2154.i, label %postload2155.i

preload2154.i:                                    ; preds = %postload2146.i
  %masked_load552.i = load double addrspace(1)* %115, align 8
  br label %postload2155.i

postload2155.i:                                   ; preds = %preload2154.i, %postload2146.i
  %phi2156.i = phi double [ undef, %postload2146.i ], [ %masked_load552.i, %preload2154.i ]
  br i1 %extract214.i, label %preload2163.i, label %postload2164.i

preload2163.i:                                    ; preds = %postload2155.i
  %masked_load553.i = load double addrspace(1)* %116, align 8
  br label %postload2164.i

postload2164.i:                                   ; preds = %preload2163.i, %postload2155.i
  %phi2165.i = phi double [ undef, %postload2155.i ], [ %masked_load553.i, %preload2163.i ]
  br i1 %extract215.i, label %preload2172.i, label %postload2173.i

preload2172.i:                                    ; preds = %postload2164.i
  %masked_load554.i = load double addrspace(1)* %117, align 8
  br label %postload2173.i

postload2173.i:                                   ; preds = %preload2172.i, %postload2164.i
  %phi2174.i = phi double [ undef, %postload2164.i ], [ %masked_load554.i, %preload2172.i ]
  br i1 %extract216.i, label %preload2181.i, label %postload2182.i

preload2181.i:                                    ; preds = %postload2173.i
  %masked_load555.i = load double addrspace(1)* %118, align 8
  br label %postload2182.i

postload2182.i:                                   ; preds = %preload2181.i, %postload2173.i
  %phi2183.i = phi double [ undef, %postload2173.i ], [ %masked_load555.i, %preload2181.i ]
  br i1 %extract217.i, label %preload2190.i, label %postload2191.i

preload2190.i:                                    ; preds = %postload2182.i
  %masked_load556.i = load double addrspace(1)* %119, align 8
  br label %postload2191.i

postload2191.i:                                   ; preds = %preload2190.i, %postload2182.i
  %phi2192.i = phi double [ undef, %postload2182.i ], [ %masked_load556.i, %preload2190.i ]
  br i1 %extract218.i, label %preload2199.i, label %postload2200.i

preload2199.i:                                    ; preds = %postload2191.i
  %masked_load557.i = load double addrspace(1)* %120, align 8
  br label %postload2200.i

postload2200.i:                                   ; preds = %preload2199.i, %postload2191.i
  %phi2201.i = phi double [ undef, %postload2191.i ], [ %masked_load557.i, %preload2199.i ]
  br i1 %extract219.i, label %preload2208.i, label %postload2209.i

preload2208.i:                                    ; preds = %postload2200.i
  %masked_load558.i = load double addrspace(1)* %121, align 8
  br label %postload2209.i

postload2209.i:                                   ; preds = %preload2208.i, %postload2200.i
  %phi2210.i = phi double [ undef, %postload2200.i ], [ %masked_load558.i, %preload2208.i ]
  br i1 %extract220.i, label %preload2217.i, label %postload2218.i

preload2217.i:                                    ; preds = %postload2209.i
  %masked_load559.i = load double addrspace(1)* %122, align 8
  br label %postload2218.i

postload2218.i:                                   ; preds = %preload2217.i, %postload2209.i
  %phi2219.i = phi double [ undef, %postload2209.i ], [ %masked_load559.i, %preload2217.i ]
  br i1 %extract221.i, label %preload2226.i, label %postload2227.i

preload2226.i:                                    ; preds = %postload2218.i
  %masked_load560.i = load double addrspace(1)* %123, align 8
  br label %postload2227.i

postload2227.i:                                   ; preds = %preload2226.i, %postload2218.i
  %phi2228.i = phi double [ undef, %postload2218.i ], [ %masked_load560.i, %preload2226.i ]
  br i1 %extract222.i, label %preload2235.i, label %postload2236.i

preload2235.i:                                    ; preds = %postload2227.i
  %masked_load561.i = load double addrspace(1)* %124, align 8
  br label %postload2236.i

postload2236.i:                                   ; preds = %preload2235.i, %postload2227.i
  %phi2237.i = phi double [ undef, %postload2227.i ], [ %masked_load561.i, %preload2235.i ]
  br i1 %extract223.i, label %preload2244.i, label %postload2245.i

preload2244.i:                                    ; preds = %postload2236.i
  %masked_load562.i = load double addrspace(1)* %125, align 8
  br label %postload2245.i

postload2245.i:                                   ; preds = %preload2244.i, %postload2236.i
  %phi2246.i = phi double [ undef, %postload2236.i ], [ %masked_load562.i, %preload2244.i ]
  %temp.vect273.i = insertelement <16 x double> undef, double %phi2111.i, i32 0
  %temp.vect274.i = insertelement <16 x double> %temp.vect273.i, double %phi2120.i, i32 1
  %temp.vect275.i = insertelement <16 x double> %temp.vect274.i, double %phi2129.i, i32 2
  %temp.vect276.i = insertelement <16 x double> %temp.vect275.i, double %phi2138.i, i32 3
  %temp.vect277.i = insertelement <16 x double> %temp.vect276.i, double %phi2147.i, i32 4
  %temp.vect278.i = insertelement <16 x double> %temp.vect277.i, double %phi2156.i, i32 5
  %temp.vect279.i = insertelement <16 x double> %temp.vect278.i, double %phi2165.i, i32 6
  %temp.vect280.i = insertelement <16 x double> %temp.vect279.i, double %phi2174.i, i32 7
  %temp.vect281.i = insertelement <16 x double> %temp.vect280.i, double %phi2183.i, i32 8
  %temp.vect282.i = insertelement <16 x double> %temp.vect281.i, double %phi2192.i, i32 9
  %temp.vect283.i = insertelement <16 x double> %temp.vect282.i, double %phi2201.i, i32 10
  %temp.vect284.i = insertelement <16 x double> %temp.vect283.i, double %phi2210.i, i32 11
  %temp.vect285.i = insertelement <16 x double> %temp.vect284.i, double %phi2219.i, i32 12
  %temp.vect286.i = insertelement <16 x double> %temp.vect285.i, double %phi2228.i, i32 13
  %temp.vect287.i = insertelement <16 x double> %temp.vect286.i, double %phi2237.i, i32 14
  %temp.vect288.i = insertelement <16 x double> %temp.vect287.i, double %phi2246.i, i32 15
  %mul23289.i = fmul <16 x double> %temp.vect272.i, %temp.vect288.i
  %add24290.i = fadd <16 x double> %vectorPHI191.i, %mul23289.i
  %out_sel291.i = select <16 x i1> %vectorPHI189.i, <16 x double> %add24290.i, <16 x double> %vectorPHI188.i
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

for.end.i:                                        ; preds = %postload2245.i, %postload2362.i
  %vectorPHI332.i = phi <16 x double> [ undef, %postload2362.i ], [ %out_sel291.i, %postload2245.i ]
  %merge333.i = select <16 x i1> %if.then_to_for.end184.i, <16 x double> zeroinitializer, <16 x double> %vectorPHI332.i
  br i1 %extract117.i, label %preload1650.i, label %postload1651.i

preload1650.i:                                    ; preds = %for.end.i
  %exData.i = extractelement <16 x double> %merge333.i, i32 0
  %loadedValue3892.i = load double addrspace(3)** %CastToValueType3887.i, align 8
  store double %exData.i, double addrspace(3)* %loadedValue3892.i, align 8
  br label %postload1651.i

postload1651.i:                                   ; preds = %preload1650.i, %for.end.i
  br i1 %extract118.i, label %preload2343.i, label %postload2344.i

preload2343.i:                                    ; preds = %postload1651.i
  %loadedValue2988.i = load i64* %CastToValueType2983.i, align 8
  %.sum2921.i = add i64 %loadedValue2988.i, 1
  %129 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2921.i
  %exData566.i = extractelement <16 x double> %merge333.i, i32 1
  store double %exData566.i, double addrspace(3)* %129, align 8
  br label %postload2344.i

postload2344.i:                                   ; preds = %preload2343.i, %postload1651.i
  br i1 %extract119.i, label %preload1621.i, label %postload1622.i

preload1621.i:                                    ; preds = %postload2344.i
  %loadedValue2993.i = load i64* %CastToValueType2983.i, align 8
  %.sum2920.i = add i64 %loadedValue2993.i, 2
  %130 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2920.i
  %exData569.i = extractelement <16 x double> %merge333.i, i32 2
  store double %exData569.i, double addrspace(3)* %130, align 8
  br label %postload1622.i

postload1622.i:                                   ; preds = %preload1621.i, %postload2344.i
  br i1 %extract120.i, label %preload2451.i, label %postload2452.i

preload2451.i:                                    ; preds = %postload1622.i
  %loadedValue2998.i = load i64* %CastToValueType2983.i, align 8
  %.sum2919.i = add i64 %loadedValue2998.i, 3
  %131 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2919.i
  %exData572.i = extractelement <16 x double> %merge333.i, i32 3
  store double %exData572.i, double addrspace(3)* %131, align 8
  br label %postload2452.i

postload2452.i:                                   ; preds = %preload2451.i, %postload1622.i
  br i1 %extract121.i, label %preload1971.i, label %postload1972.i

preload1971.i:                                    ; preds = %postload2452.i
  %loadedValue3003.i = load i64* %CastToValueType2983.i, align 8
  %.sum2918.i = add i64 %loadedValue3003.i, 4
  %132 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2918.i
  %exData575.i = extractelement <16 x double> %merge333.i, i32 4
  store double %exData575.i, double addrspace(3)* %132, align 8
  br label %postload1972.i

postload1972.i:                                   ; preds = %preload1971.i, %postload2452.i
  br i1 %extract122.i, label %preload1618.i, label %postload1619.i

preload1618.i:                                    ; preds = %postload1972.i
  %loadedValue3008.i = load i64* %CastToValueType2983.i, align 8
  %.sum2917.i = add i64 %loadedValue3008.i, 5
  %133 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2917.i
  %exData578.i = extractelement <16 x double> %merge333.i, i32 5
  store double %exData578.i, double addrspace(3)* %133, align 8
  br label %postload1619.i

postload1619.i:                                   ; preds = %preload1618.i, %postload1972.i
  br i1 %extract123.i, label %preload2538.i, label %postload2539.i

preload2538.i:                                    ; preds = %postload1619.i
  %loadedValue3013.i = load i64* %CastToValueType2983.i, align 8
  %.sum2916.i = add i64 %loadedValue3013.i, 6
  %134 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2916.i
  %exData581.i = extractelement <16 x double> %merge333.i, i32 6
  store double %exData581.i, double addrspace(3)* %134, align 8
  br label %postload2539.i

postload2539.i:                                   ; preds = %preload2538.i, %postload1619.i
  br i1 %extract124.i, label %preload2454.i, label %postload2455.i

preload2454.i:                                    ; preds = %postload2539.i
  %loadedValue3018.i = load i64* %CastToValueType2983.i, align 8
  %.sum2915.i = add i64 %loadedValue3018.i, 7
  %135 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2915.i
  %exData584.i = extractelement <16 x double> %merge333.i, i32 7
  store double %exData584.i, double addrspace(3)* %135, align 8
  br label %postload2455.i

postload2455.i:                                   ; preds = %preload2454.i, %postload2539.i
  br i1 %extract125.i, label %preload2436.i, label %postload2437.i

preload2436.i:                                    ; preds = %postload2455.i
  %loadedValue3023.i = load i64* %CastToValueType2983.i, align 8
  %.sum2914.i = add i64 %loadedValue3023.i, 8
  %136 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2914.i
  %exData587.i = extractelement <16 x double> %merge333.i, i32 8
  store double %exData587.i, double addrspace(3)* %136, align 8
  br label %postload2437.i

postload2437.i:                                   ; preds = %preload2436.i, %postload2455.i
  br i1 %extract126.i, label %preload1612.i, label %postload1613.i

preload1612.i:                                    ; preds = %postload2437.i
  %loadedValue3028.i = load i64* %CastToValueType2983.i, align 8
  %.sum2913.i = add i64 %loadedValue3028.i, 9
  %137 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2913.i
  %exData590.i = extractelement <16 x double> %merge333.i, i32 9
  store double %exData590.i, double addrspace(3)* %137, align 8
  br label %postload1613.i

postload1613.i:                                   ; preds = %preload1612.i, %postload2437.i
  br i1 %extract127.i, label %preload2022.i, label %postload2023.i

preload2022.i:                                    ; preds = %postload1613.i
  %loadedValue3033.i = load i64* %CastToValueType2983.i, align 8
  %.sum2912.i = add i64 %loadedValue3033.i, 10
  %138 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2912.i
  %exData593.i = extractelement <16 x double> %merge333.i, i32 10
  store double %exData593.i, double addrspace(3)* %138, align 8
  br label %postload2023.i

postload2023.i:                                   ; preds = %preload2022.i, %postload1613.i
  br i1 %extract128.i, label %preload1854.i, label %postload1855.i

preload1854.i:                                    ; preds = %postload2023.i
  %loadedValue3038.i = load i64* %CastToValueType2983.i, align 8
  %.sum2911.i = add i64 %loadedValue3038.i, 11
  %139 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2911.i
  %exData596.i = extractelement <16 x double> %merge333.i, i32 11
  store double %exData596.i, double addrspace(3)* %139, align 8
  br label %postload1855.i

postload1855.i:                                   ; preds = %preload1854.i, %postload2023.i
  br i1 %extract129.i, label %preload1923.i, label %postload1924.i

preload1923.i:                                    ; preds = %postload1855.i
  %loadedValue3043.i = load i64* %CastToValueType2983.i, align 8
  %.sum2910.i = add i64 %loadedValue3043.i, 12
  %140 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2910.i
  %exData599.i = extractelement <16 x double> %merge333.i, i32 12
  store double %exData599.i, double addrspace(3)* %140, align 8
  br label %postload1924.i

postload1924.i:                                   ; preds = %preload1923.i, %postload1855.i
  br i1 %extract130.i, label %preload1920.i, label %postload1921.i

preload1920.i:                                    ; preds = %postload1924.i
  %loadedValue3048.i = load i64* %CastToValueType2983.i, align 8
  %.sum2909.i = add i64 %loadedValue3048.i, 13
  %141 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2909.i
  %exData602.i = extractelement <16 x double> %merge333.i, i32 13
  store double %exData602.i, double addrspace(3)* %141, align 8
  br label %postload1921.i

postload1921.i:                                   ; preds = %preload1920.i, %postload1924.i
  br i1 %extract131.i, label %preload1833.i, label %postload1834.i

preload1833.i:                                    ; preds = %postload1921.i
  %loadedValue3053.i = load i64* %CastToValueType2983.i, align 8
  %.sum2908.i = add i64 %loadedValue3053.i, 14
  %142 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2908.i
  %exData605.i = extractelement <16 x double> %merge333.i, i32 14
  store double %exData605.i, double addrspace(3)* %142, align 8
  br label %postload1834.i

postload1834.i:                                   ; preds = %preload1833.i, %postload1921.i
  br i1 %extract132.i, label %preload1615.i, label %postload1834.i.postload1616.i_crit_edge

postload1834.i.postload1616.i_crit_edge:          ; preds = %postload1834.i
  br label %postload1616.i

preload1615.i:                                    ; preds = %postload1834.i
  %loadedValue3058.i = load i64* %CastToValueType2983.i, align 8
  %.sum2907.i = add i64 %loadedValue3058.i, 15
  %143 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2907.i
  %exData608.i = extractelement <16 x double> %merge333.i, i32 15
  store double %exData608.i, double addrspace(3)* %143, align 8
  br label %postload1616.i

postload1616.i:                                   ; preds = %postload1834.i.postload1616.i_crit_edge, %preload1615.i
  %loadedValue4025.i = load i1* %CastToValueType3990.i, align 1
  br i1 %loadedValue4025.i, label %preload2511.i, label %postload1616.i.postload2512.i_crit_edge

postload1616.i.postload2512.i_crit_edge:          ; preds = %postload1616.i
  br label %postload2512.i

preload2511.i:                                    ; preds = %postload1616.i
  %check.WI.iter4591.i = icmp ult i64 %CurrWI..0.i, %31
  br i1 %check.WI.iter4591.i, label %thenBB4588.i, label %preload2511.i.postload2512.i_crit_edge

preload2511.i.postload2512.i_crit_edge:           ; preds = %preload2511.i
  br label %postload2512.i

thenBB4588.i:                                     ; preds = %preload2511.i
  %"CurrWI++4592.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride4594.i" = add nuw i64 %CurrSBIndex..0.i, 256
  br label %SyncBB.i

postload2512.i:                                   ; preds = %thenBB4603.i.postload2512.i_crit_edge, %thenBB4595.i.postload2512.i_crit_edge, %thenBB4572.i.postload2512.i_crit_edge, %thenBB.i.postload2512.i_crit_edge, %thenBB4580.i.postload2512.i_crit_edge, %thenBB4611.i.postload2512.i_crit_edge, %preload2511.i.postload2512.i_crit_edge, %postload1616.i.postload2512.i_crit_edge
  %CurrWI..2.i = phi i64 [ %CurrWI..0.i, %postload1616.i.postload2512.i_crit_edge ], [ 0, %preload2511.i.postload2512.i_crit_edge ], [ %"CurrWI++4615.i", %thenBB4611.i.postload2512.i_crit_edge ], [ %"CurrWI++4584.i", %thenBB4580.i.postload2512.i_crit_edge ], [ %"CurrWI++.i", %thenBB.i.postload2512.i_crit_edge ], [ %"CurrWI++4576.i", %thenBB4572.i.postload2512.i_crit_edge ], [ %"CurrWI++4599.i", %thenBB4595.i.postload2512.i_crit_edge ], [ %"CurrWI++4607.i", %thenBB4603.i.postload2512.i_crit_edge ]
  %CurrSBIndex..2.i = phi i64 [ %CurrSBIndex..0.i, %postload1616.i.postload2512.i_crit_edge ], [ 0, %preload2511.i.postload2512.i_crit_edge ], [ %"loadedCurrSB+Stride4617.i", %thenBB4611.i.postload2512.i_crit_edge ], [ %"loadedCurrSB+Stride4586.i", %thenBB4580.i.postload2512.i_crit_edge ], [ %"loadedCurrSB+Stride.i", %thenBB.i.postload2512.i_crit_edge ], [ %"loadedCurrSB+Stride4578.i", %thenBB4572.i.postload2512.i_crit_edge ], [ %"loadedCurrSB+Stride4601.i", %thenBB4595.i.postload2512.i_crit_edge ], [ %"loadedCurrSB+Stride4609.i", %thenBB4603.i.postload2512.i_crit_edge ]
  %currBarrier.2.i = phi i32 [ %currBarrier.0.ph.i, %postload1616.i.postload2512.i_crit_edge ], [ 8, %preload2511.i.postload2512.i_crit_edge ], [ %currBarrier.2.i, %thenBB4611.i.postload2512.i_crit_edge ], [ %currBarrier.4.i, %thenBB4580.i.postload2512.i_crit_edge ], [ %currBarrier.6.i, %thenBB.i.postload2512.i_crit_edge ], [ %currBarrier.8.i, %thenBB4572.i.postload2512.i_crit_edge ], [ %currBarrier.10.i, %thenBB4595.i.postload2512.i_crit_edge ], [ %currBarrier.12.i, %thenBB4603.i.postload2512.i_crit_edge ]
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
  br i1 %exmask610.i, label %preload2049.i, label %postload2050.i

preload2049.i:                                    ; preds = %postload2512.i
  %145 = getelementptr inbounds [128 x double] addrspace(3)* %35, i64 0, i64 %extract338.i
  %vload611.i = load double addrspace(3)* %145, align 8
  br label %postload2050.i

postload2050.i:                                   ; preds = %preload2049.i, %postload2512.i
  %phi2051.i = phi double [ undef, %postload2512.i ], [ %vload611.i, %preload2049.i ]
  %vpack.i = insertelement <16 x double> undef, double %phi2051.i, i32 0
  %exmask613.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 1
  br i1 %exmask613.i, label %preload1677.i, label %postload1678.i

preload1677.i:                                    ; preds = %postload2050.i
  %.sum2906.i = add i64 %extract338.i, 1
  %146 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2906.i
  %vload614.i = load double addrspace(3)* %146, align 8
  br label %postload1678.i

postload1678.i:                                   ; preds = %preload1677.i, %postload2050.i
  %phi1679.i = phi double [ undef, %postload2050.i ], [ %vload614.i, %preload1677.i ]
  %vpack615.i = insertelement <16 x double> %vpack.i, double %phi1679.i, i32 1
  %exmask617.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 2
  br i1 %exmask617.i, label %preload2385.i, label %postload2386.i

preload2385.i:                                    ; preds = %postload1678.i
  %.sum2905.i = add i64 %extract338.i, 2
  %147 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2905.i
  %vload618.i = load double addrspace(3)* %147, align 8
  br label %postload2386.i

postload2386.i:                                   ; preds = %preload2385.i, %postload1678.i
  %phi2387.i = phi double [ undef, %postload1678.i ], [ %vload618.i, %preload2385.i ]
  %vpack619.i = insertelement <16 x double> %vpack615.i, double %phi2387.i, i32 2
  %exmask621.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 3
  br i1 %exmask621.i, label %preload2619.i, label %postload2620.i

preload2619.i:                                    ; preds = %postload2386.i
  %.sum2904.i = add i64 %extract338.i, 3
  %148 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2904.i
  %vload622.i = load double addrspace(3)* %148, align 8
  br label %postload2620.i

postload2620.i:                                   ; preds = %preload2619.i, %postload2386.i
  %phi2621.i = phi double [ undef, %postload2386.i ], [ %vload622.i, %preload2619.i ]
  %vpack623.i = insertelement <16 x double> %vpack619.i, double %phi2621.i, i32 3
  %exmask625.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 4
  br i1 %exmask625.i, label %preload1851.i, label %postload1852.i

preload1851.i:                                    ; preds = %postload2620.i
  %.sum2903.i = add i64 %extract338.i, 4
  %149 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2903.i
  %vload626.i = load double addrspace(3)* %149, align 8
  br label %postload1852.i

postload1852.i:                                   ; preds = %preload1851.i, %postload2620.i
  %phi1853.i = phi double [ undef, %postload2620.i ], [ %vload626.i, %preload1851.i ]
  %vpack627.i = insertelement <16 x double> %vpack623.i, double %phi1853.i, i32 4
  %exmask629.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 5
  br i1 %exmask629.i, label %preload2502.i, label %postload2503.i

preload2502.i:                                    ; preds = %postload1852.i
  %.sum2902.i = add i64 %extract338.i, 5
  %150 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2902.i
  %vload630.i = load double addrspace(3)* %150, align 8
  br label %postload2503.i

postload2503.i:                                   ; preds = %preload2502.i, %postload1852.i
  %phi2504.i = phi double [ undef, %postload1852.i ], [ %vload630.i, %preload2502.i ]
  %vpack631.i = insertelement <16 x double> %vpack627.i, double %phi2504.i, i32 5
  %exmask633.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 6
  br i1 %exmask633.i, label %preload2448.i, label %postload2449.i

preload2448.i:                                    ; preds = %postload2503.i
  %.sum2901.i = add i64 %extract338.i, 6
  %151 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2901.i
  %vload634.i = load double addrspace(3)* %151, align 8
  br label %postload2449.i

postload2449.i:                                   ; preds = %preload2448.i, %postload2503.i
  %phi2450.i = phi double [ undef, %postload2503.i ], [ %vload634.i, %preload2448.i ]
  %vpack635.i = insertelement <16 x double> %vpack631.i, double %phi2450.i, i32 6
  %exmask637.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 7
  br i1 %exmask637.i, label %preload2013.i, label %postload2014.i

preload2013.i:                                    ; preds = %postload2449.i
  %.sum2900.i = add i64 %extract338.i, 7
  %152 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2900.i
  %vload638.i = load double addrspace(3)* %152, align 8
  br label %postload2014.i

postload2014.i:                                   ; preds = %preload2013.i, %postload2449.i
  %phi2015.i = phi double [ undef, %postload2449.i ], [ %vload638.i, %preload2013.i ]
  %vpack639.i = insertelement <16 x double> %vpack635.i, double %phi2015.i, i32 7
  %exmask641.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 8
  br i1 %exmask641.i, label %preload2523.i, label %postload2524.i

preload2523.i:                                    ; preds = %postload2014.i
  %.sum2899.i = add i64 %extract338.i, 8
  %153 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2899.i
  %vload642.i = load double addrspace(3)* %153, align 8
  br label %postload2524.i

postload2524.i:                                   ; preds = %preload2523.i, %postload2014.i
  %phi2525.i = phi double [ undef, %postload2014.i ], [ %vload642.i, %preload2523.i ]
  %vpack643.i = insertelement <16 x double> %vpack639.i, double %phi2525.i, i32 8
  %exmask645.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 9
  br i1 %exmask645.i, label %preload1848.i, label %postload1849.i

preload1848.i:                                    ; preds = %postload2524.i
  %.sum2898.i = add i64 %extract338.i, 9
  %154 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2898.i
  %vload646.i = load double addrspace(3)* %154, align 8
  br label %postload1849.i

postload1849.i:                                   ; preds = %preload1848.i, %postload2524.i
  %phi1850.i = phi double [ undef, %postload2524.i ], [ %vload646.i, %preload1848.i ]
  %vpack647.i = insertelement <16 x double> %vpack643.i, double %phi1850.i, i32 9
  %exmask649.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 10
  br i1 %exmask649.i, label %preload2295.i, label %postload2296.i

preload2295.i:                                    ; preds = %postload1849.i
  %.sum2897.i = add i64 %extract338.i, 10
  %155 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2897.i
  %vload650.i = load double addrspace(3)* %155, align 8
  br label %postload2296.i

postload2296.i:                                   ; preds = %preload2295.i, %postload1849.i
  %phi2297.i = phi double [ undef, %postload1849.i ], [ %vload650.i, %preload2295.i ]
  %vpack651.i = insertelement <16 x double> %vpack647.i, double %phi2297.i, i32 10
  %exmask653.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 11
  br i1 %exmask653.i, label %preload1974.i, label %postload1975.i

preload1974.i:                                    ; preds = %postload2296.i
  %.sum2896.i = add i64 %extract338.i, 11
  %156 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2896.i
  %vload654.i = load double addrspace(3)* %156, align 8
  br label %postload1975.i

postload1975.i:                                   ; preds = %preload1974.i, %postload2296.i
  %phi1976.i = phi double [ undef, %postload2296.i ], [ %vload654.i, %preload1974.i ]
  %vpack655.i = insertelement <16 x double> %vpack651.i, double %phi1976.i, i32 11
  %exmask657.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 12
  br i1 %exmask657.i, label %preload2052.i, label %postload2053.i

preload2052.i:                                    ; preds = %postload1975.i
  %.sum2895.i = add i64 %extract338.i, 12
  %157 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2895.i
  %vload658.i = load double addrspace(3)* %157, align 8
  br label %postload2053.i

postload2053.i:                                   ; preds = %preload2052.i, %postload1975.i
  %phi2054.i = phi double [ undef, %postload1975.i ], [ %vload658.i, %preload2052.i ]
  %vpack659.i = insertelement <16 x double> %vpack655.i, double %phi2054.i, i32 12
  %exmask661.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 13
  br i1 %exmask661.i, label %preload2055.i, label %postload2056.i

preload2055.i:                                    ; preds = %postload2053.i
  %.sum2894.i = add i64 %extract338.i, 13
  %158 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2894.i
  %vload662.i = load double addrspace(3)* %158, align 8
  br label %postload2056.i

postload2056.i:                                   ; preds = %preload2055.i, %postload2053.i
  %phi2057.i = phi double [ undef, %postload2053.i ], [ %vload662.i, %preload2055.i ]
  %vpack663.i = insertelement <16 x double> %vpack659.i, double %phi2057.i, i32 13
  %exmask665.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 14
  br i1 %exmask665.i, label %preload2079.i, label %postload2080.i

preload2079.i:                                    ; preds = %postload2056.i
  %.sum2893.i = add i64 %extract338.i, 14
  %159 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2893.i
  %vload666.i = load double addrspace(3)* %159, align 8
  br label %postload2080.i

postload2080.i:                                   ; preds = %preload2079.i, %postload2056.i
  %phi2081.i = phi double [ undef, %postload2056.i ], [ %vload666.i, %preload2079.i ]
  %vpack667.i = insertelement <16 x double> %vpack663.i, double %phi2081.i, i32 14
  %exmask669.i = extractelement <16 x i1> %for.end_to_if.then30335.i, i32 15
  br i1 %exmask669.i, label %preload2082.i, label %postload2083.i

preload2082.i:                                    ; preds = %postload2080.i
  %.sum2892.i = add i64 %extract338.i, 15
  %160 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2892.i
  %vload670.i = load double addrspace(3)* %160, align 8
  br label %postload2083.i

postload2083.i:                                   ; preds = %preload2082.i, %postload2080.i
  %phi2084.i = phi double [ undef, %postload2080.i ], [ %vload670.i, %preload2082.i ]
  %vpack671.i = insertelement <16 x double> %vpack667.i, double %phi2084.i, i32 15
  br i1 %exmask610.i, label %preload2016.i, label %postload2017.i

preload2016.i:                                    ; preds = %postload2083.i
  %"&(pSB[currWI].offset)3894.i" = add nuw i64 %CurrSBIndex..2.i, 80
  %"&pSB[currWI].offset3895.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3894.i"
  %CastToValueType3896.i = bitcast i8* %"&pSB[currWI].offset3895.i" to double addrspace(3)**
  %loadedValue3897.i = load double addrspace(3)** %CastToValueType3896.i, align 8
  %vload675.i = load double addrspace(3)* %loadedValue3897.i, align 8
  br label %postload2017.i

postload2017.i:                                   ; preds = %preload2016.i, %postload2083.i
  %phi2018.i = phi double [ undef, %postload2083.i ], [ %vload675.i, %preload2016.i ]
  %vpack676.i = insertelement <16 x double> undef, double %phi2018.i, i32 0
  br i1 %exmask613.i, label %preload2019.i, label %postload2020.i

preload2019.i:                                    ; preds = %postload2017.i
  %"&(pSB[currWI].offset)3060.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3061.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3060.i"
  %CastToValueType3062.i = bitcast i8* %"&pSB[currWI].offset3061.i" to i64*
  %loadedValue3063.i = load i64* %CastToValueType3062.i, align 8
  %.sum2891.i = add i64 %loadedValue3063.i, 1
  %161 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2891.i
  %vload679.i = load double addrspace(3)* %161, align 8
  br label %postload2020.i

postload2020.i:                                   ; preds = %preload2019.i, %postload2017.i
  %phi2021.i = phi double [ undef, %postload2017.i ], [ %vload679.i, %preload2019.i ]
  %vpack680.i = insertelement <16 x double> %vpack676.i, double %phi2021.i, i32 1
  br i1 %exmask617.i, label %preload2058.i, label %postload2059.i

preload2058.i:                                    ; preds = %postload2020.i
  %"&(pSB[currWI].offset)3065.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3066.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3065.i"
  %CastToValueType3067.i = bitcast i8* %"&pSB[currWI].offset3066.i" to i64*
  %loadedValue3068.i = load i64* %CastToValueType3067.i, align 8
  %.sum2890.i = add i64 %loadedValue3068.i, 2
  %162 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2890.i
  %vload683.i = load double addrspace(3)* %162, align 8
  br label %postload2059.i

postload2059.i:                                   ; preds = %preload2058.i, %postload2020.i
  %phi2060.i = phi double [ undef, %postload2020.i ], [ %vload683.i, %preload2058.i ]
  %vpack684.i = insertelement <16 x double> %vpack680.i, double %phi2060.i, i32 2
  br i1 %exmask621.i, label %preload2061.i, label %postload2062.i

preload2061.i:                                    ; preds = %postload2059.i
  %"&(pSB[currWI].offset)3070.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3071.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3070.i"
  %CastToValueType3072.i = bitcast i8* %"&pSB[currWI].offset3071.i" to i64*
  %loadedValue3073.i = load i64* %CastToValueType3072.i, align 8
  %.sum2889.i = add i64 %loadedValue3073.i, 3
  %163 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2889.i
  %vload687.i = load double addrspace(3)* %163, align 8
  br label %postload2062.i

postload2062.i:                                   ; preds = %preload2061.i, %postload2059.i
  %phi2063.i = phi double [ undef, %postload2059.i ], [ %vload687.i, %preload2061.i ]
  %vpack688.i = insertelement <16 x double> %vpack684.i, double %phi2063.i, i32 3
  br i1 %exmask625.i, label %preload2064.i, label %postload2065.i

preload2064.i:                                    ; preds = %postload2062.i
  %"&(pSB[currWI].offset)3075.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3076.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3075.i"
  %CastToValueType3077.i = bitcast i8* %"&pSB[currWI].offset3076.i" to i64*
  %loadedValue3078.i = load i64* %CastToValueType3077.i, align 8
  %.sum2888.i = add i64 %loadedValue3078.i, 4
  %164 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2888.i
  %vload691.i = load double addrspace(3)* %164, align 8
  br label %postload2065.i

postload2065.i:                                   ; preds = %preload2064.i, %postload2062.i
  %phi2066.i = phi double [ undef, %postload2062.i ], [ %vload691.i, %preload2064.i ]
  %vpack692.i = insertelement <16 x double> %vpack688.i, double %phi2066.i, i32 4
  br i1 %exmask629.i, label %preload2097.i, label %postload2098.i

preload2097.i:                                    ; preds = %postload2065.i
  %"&(pSB[currWI].offset)3080.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3081.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3080.i"
  %CastToValueType3082.i = bitcast i8* %"&pSB[currWI].offset3081.i" to i64*
  %loadedValue3083.i = load i64* %CastToValueType3082.i, align 8
  %.sum2887.i = add i64 %loadedValue3083.i, 5
  %165 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2887.i
  %vload695.i = load double addrspace(3)* %165, align 8
  br label %postload2098.i

postload2098.i:                                   ; preds = %preload2097.i, %postload2065.i
  %phi2099.i = phi double [ undef, %postload2065.i ], [ %vload695.i, %preload2097.i ]
  %vpack696.i = insertelement <16 x double> %vpack692.i, double %phi2099.i, i32 5
  br i1 %exmask633.i, label %preload2100.i, label %postload2101.i

preload2100.i:                                    ; preds = %postload2098.i
  %"&(pSB[currWI].offset)3085.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3086.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3085.i"
  %CastToValueType3087.i = bitcast i8* %"&pSB[currWI].offset3086.i" to i64*
  %loadedValue3088.i = load i64* %CastToValueType3087.i, align 8
  %.sum2886.i = add i64 %loadedValue3088.i, 6
  %166 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2886.i
  %vload699.i = load double addrspace(3)* %166, align 8
  br label %postload2101.i

postload2101.i:                                   ; preds = %preload2100.i, %postload2098.i
  %phi2102.i = phi double [ undef, %postload2098.i ], [ %vload699.i, %preload2100.i ]
  %vpack700.i = insertelement <16 x double> %vpack696.i, double %phi2102.i, i32 6
  br i1 %exmask637.i, label %preload2259.i, label %postload2260.i

preload2259.i:                                    ; preds = %postload2101.i
  %"&(pSB[currWI].offset)3090.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3091.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3090.i"
  %CastToValueType3092.i = bitcast i8* %"&pSB[currWI].offset3091.i" to i64*
  %loadedValue3093.i = load i64* %CastToValueType3092.i, align 8
  %.sum2885.i = add i64 %loadedValue3093.i, 7
  %167 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2885.i
  %vload703.i = load double addrspace(3)* %167, align 8
  br label %postload2260.i

postload2260.i:                                   ; preds = %preload2259.i, %postload2101.i
  %phi2261.i = phi double [ undef, %postload2101.i ], [ %vload703.i, %preload2259.i ]
  %vpack704.i = insertelement <16 x double> %vpack700.i, double %phi2261.i, i32 7
  br i1 %exmask641.i, label %preload2262.i, label %postload2263.i

preload2262.i:                                    ; preds = %postload2260.i
  %"&(pSB[currWI].offset)3095.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3096.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3095.i"
  %CastToValueType3097.i = bitcast i8* %"&pSB[currWI].offset3096.i" to i64*
  %loadedValue3098.i = load i64* %CastToValueType3097.i, align 8
  %.sum2884.i = add i64 %loadedValue3098.i, 8
  %168 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2884.i
  %vload707.i = load double addrspace(3)* %168, align 8
  br label %postload2263.i

postload2263.i:                                   ; preds = %preload2262.i, %postload2260.i
  %phi2264.i = phi double [ undef, %postload2260.i ], [ %vload707.i, %preload2262.i ]
  %vpack708.i = insertelement <16 x double> %vpack704.i, double %phi2264.i, i32 8
  br i1 %exmask645.i, label %preload2265.i, label %postload2266.i

preload2265.i:                                    ; preds = %postload2263.i
  %"&(pSB[currWI].offset)3100.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3101.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3100.i"
  %CastToValueType3102.i = bitcast i8* %"&pSB[currWI].offset3101.i" to i64*
  %loadedValue3103.i = load i64* %CastToValueType3102.i, align 8
  %.sum2883.i = add i64 %loadedValue3103.i, 9
  %169 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2883.i
  %vload711.i = load double addrspace(3)* %169, align 8
  br label %postload2266.i

postload2266.i:                                   ; preds = %preload2265.i, %postload2263.i
  %phi2267.i = phi double [ undef, %postload2263.i ], [ %vload711.i, %preload2265.i ]
  %vpack712.i = insertelement <16 x double> %vpack708.i, double %phi2267.i, i32 9
  br i1 %exmask649.i, label %preload1662.i, label %postload1663.i

preload1662.i:                                    ; preds = %postload2266.i
  %"&(pSB[currWI].offset)3105.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3106.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3105.i"
  %CastToValueType3107.i = bitcast i8* %"&pSB[currWI].offset3106.i" to i64*
  %loadedValue3108.i = load i64* %CastToValueType3107.i, align 8
  %.sum2882.i = add i64 %loadedValue3108.i, 10
  %170 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2882.i
  %vload715.i = load double addrspace(3)* %170, align 8
  br label %postload1663.i

postload1663.i:                                   ; preds = %preload1662.i, %postload2266.i
  %phi1664.i = phi double [ undef, %postload2266.i ], [ %vload715.i, %preload1662.i ]
  %vpack716.i = insertelement <16 x double> %vpack712.i, double %phi1664.i, i32 10
  br i1 %exmask653.i, label %preload1665.i, label %postload1666.i

preload1665.i:                                    ; preds = %postload1663.i
  %"&(pSB[currWI].offset)3110.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3111.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3110.i"
  %CastToValueType3112.i = bitcast i8* %"&pSB[currWI].offset3111.i" to i64*
  %loadedValue3113.i = load i64* %CastToValueType3112.i, align 8
  %.sum2881.i = add i64 %loadedValue3113.i, 11
  %171 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2881.i
  %vload719.i = load double addrspace(3)* %171, align 8
  br label %postload1666.i

postload1666.i:                                   ; preds = %preload1665.i, %postload1663.i
  %phi1667.i = phi double [ undef, %postload1663.i ], [ %vload719.i, %preload1665.i ]
  %vpack720.i = insertelement <16 x double> %vpack716.i, double %phi1667.i, i32 11
  br i1 %exmask657.i, label %preload2364.i, label %postload2365.i

preload2364.i:                                    ; preds = %postload1666.i
  %"&(pSB[currWI].offset)3115.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3116.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3115.i"
  %CastToValueType3117.i = bitcast i8* %"&pSB[currWI].offset3116.i" to i64*
  %loadedValue3118.i = load i64* %CastToValueType3117.i, align 8
  %.sum2880.i = add i64 %loadedValue3118.i, 12
  %172 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2880.i
  %vload723.i = load double addrspace(3)* %172, align 8
  br label %postload2365.i

postload2365.i:                                   ; preds = %preload2364.i, %postload1666.i
  %phi2366.i = phi double [ undef, %postload1666.i ], [ %vload723.i, %preload2364.i ]
  %vpack724.i = insertelement <16 x double> %vpack720.i, double %phi2366.i, i32 12
  br i1 %exmask661.i, label %preload2367.i, label %postload2368.i

preload2367.i:                                    ; preds = %postload2365.i
  %"&(pSB[currWI].offset)3120.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3121.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3120.i"
  %CastToValueType3122.i = bitcast i8* %"&pSB[currWI].offset3121.i" to i64*
  %loadedValue3123.i = load i64* %CastToValueType3122.i, align 8
  %.sum2879.i = add i64 %loadedValue3123.i, 13
  %173 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2879.i
  %vload727.i = load double addrspace(3)* %173, align 8
  br label %postload2368.i

postload2368.i:                                   ; preds = %preload2367.i, %postload2365.i
  %phi2369.i = phi double [ undef, %postload2365.i ], [ %vload727.i, %preload2367.i ]
  %vpack728.i = insertelement <16 x double> %vpack724.i, double %phi2369.i, i32 13
  br i1 %exmask665.i, label %preload2370.i, label %postload2371.i

preload2370.i:                                    ; preds = %postload2368.i
  %"&(pSB[currWI].offset)3125.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3126.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3125.i"
  %CastToValueType3127.i = bitcast i8* %"&pSB[currWI].offset3126.i" to i64*
  %loadedValue3128.i = load i64* %CastToValueType3127.i, align 8
  %.sum2878.i = add i64 %loadedValue3128.i, 14
  %174 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2878.i
  %vload731.i = load double addrspace(3)* %174, align 8
  br label %postload2371.i

postload2371.i:                                   ; preds = %preload2370.i, %postload2368.i
  %phi2372.i = phi double [ undef, %postload2368.i ], [ %vload731.i, %preload2370.i ]
  %vpack732.i = insertelement <16 x double> %vpack728.i, double %phi2372.i, i32 14
  br i1 %exmask669.i, label %preload2469.i, label %postload2470.i

preload2469.i:                                    ; preds = %postload2371.i
  %"&(pSB[currWI].offset)3130.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3131.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3130.i"
  %CastToValueType3132.i = bitcast i8* %"&pSB[currWI].offset3131.i" to i64*
  %loadedValue3133.i = load i64* %CastToValueType3132.i, align 8
  %.sum2877.i = add i64 %loadedValue3133.i, 15
  %175 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2877.i
  %vload735.i = load double addrspace(3)* %175, align 8
  br label %postload2470.i

postload2470.i:                                   ; preds = %preload2469.i, %postload2371.i
  %phi2471.i = phi double [ undef, %postload2371.i ], [ %vload735.i, %preload2469.i ]
  %vpack736.i = insertelement <16 x double> %vpack732.i, double %phi2471.i, i32 15
  %add36356.i = fadd <16 x double> %vpack736.i, %vpack671.i
  br i1 %exmask610.i, label %preload2472.i, label %postload2473.i

preload2472.i:                                    ; preds = %postload2470.i
  %exData740.i = extractelement <16 x double> %add36356.i, i32 0
  %"&(pSB[currWI].offset)3899.i" = add nuw i64 %CurrSBIndex..2.i, 80
  %"&pSB[currWI].offset3900.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3899.i"
  %CastToValueType3901.i = bitcast i8* %"&pSB[currWI].offset3900.i" to double addrspace(3)**
  %loadedValue3902.i = load double addrspace(3)** %CastToValueType3901.i, align 8
  store double %exData740.i, double addrspace(3)* %loadedValue3902.i, align 8
  br label %postload2473.i

postload2473.i:                                   ; preds = %preload2472.i, %postload2470.i
  br i1 %exmask613.i, label %preload2475.i, label %postload2476.i

preload2475.i:                                    ; preds = %postload2473.i
  %"&(pSB[currWI].offset)3135.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3136.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3135.i"
  %CastToValueType3137.i = bitcast i8* %"&pSB[currWI].offset3136.i" to i64*
  %loadedValue3138.i = load i64* %CastToValueType3137.i, align 8
  %.sum2876.i = add i64 %loadedValue3138.i, 1
  %176 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2876.i
  %exData743.i = extractelement <16 x double> %add36356.i, i32 1
  store double %exData743.i, double addrspace(3)* %176, align 8
  br label %postload2476.i

postload2476.i:                                   ; preds = %preload2475.i, %postload2473.i
  br i1 %exmask617.i, label %preload2439.i, label %postload2440.i

preload2439.i:                                    ; preds = %postload2476.i
  %"&(pSB[currWI].offset)3140.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3141.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3140.i"
  %CastToValueType3142.i = bitcast i8* %"&pSB[currWI].offset3141.i" to i64*
  %loadedValue3143.i = load i64* %CastToValueType3142.i, align 8
  %.sum2875.i = add i64 %loadedValue3143.i, 2
  %177 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2875.i
  %exData746.i = extractelement <16 x double> %add36356.i, i32 2
  store double %exData746.i, double addrspace(3)* %177, align 8
  br label %postload2440.i

postload2440.i:                                   ; preds = %preload2439.i, %postload2476.i
  br i1 %exmask621.i, label %preload2442.i, label %postload2443.i

preload2442.i:                                    ; preds = %postload2440.i
  %"&(pSB[currWI].offset)3145.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3146.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3145.i"
  %CastToValueType3147.i = bitcast i8* %"&pSB[currWI].offset3146.i" to i64*
  %loadedValue3148.i = load i64* %CastToValueType3147.i, align 8
  %.sum2874.i = add i64 %loadedValue3148.i, 3
  %178 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2874.i
  %exData749.i = extractelement <16 x double> %add36356.i, i32 3
  store double %exData749.i, double addrspace(3)* %178, align 8
  br label %postload2443.i

postload2443.i:                                   ; preds = %preload2442.i, %postload2440.i
  br i1 %exmask625.i, label %preload2445.i, label %postload2446.i

preload2445.i:                                    ; preds = %postload2443.i
  %"&(pSB[currWI].offset)3150.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3151.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3150.i"
  %CastToValueType3152.i = bitcast i8* %"&pSB[currWI].offset3151.i" to i64*
  %loadedValue3153.i = load i64* %CastToValueType3152.i, align 8
  %.sum2873.i = add i64 %loadedValue3153.i, 4
  %179 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2873.i
  %exData752.i = extractelement <16 x double> %add36356.i, i32 4
  store double %exData752.i, double addrspace(3)* %179, align 8
  br label %postload2446.i

postload2446.i:                                   ; preds = %preload2445.i, %postload2443.i
  br i1 %exmask629.i, label %preload1800.i, label %postload1801.i

preload1800.i:                                    ; preds = %postload2446.i
  %"&(pSB[currWI].offset)3155.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3156.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3155.i"
  %CastToValueType3157.i = bitcast i8* %"&pSB[currWI].offset3156.i" to i64*
  %loadedValue3158.i = load i64* %CastToValueType3157.i, align 8
  %.sum2872.i = add i64 %loadedValue3158.i, 5
  %180 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2872.i
  %exData755.i = extractelement <16 x double> %add36356.i, i32 5
  store double %exData755.i, double addrspace(3)* %180, align 8
  br label %postload1801.i

postload1801.i:                                   ; preds = %preload1800.i, %postload2446.i
  br i1 %exmask633.i, label %preload1803.i, label %postload1804.i

preload1803.i:                                    ; preds = %postload1801.i
  %"&(pSB[currWI].offset)3160.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3161.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3160.i"
  %CastToValueType3162.i = bitcast i8* %"&pSB[currWI].offset3161.i" to i64*
  %loadedValue3163.i = load i64* %CastToValueType3162.i, align 8
  %.sum2871.i = add i64 %loadedValue3163.i, 6
  %181 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2871.i
  %exData758.i = extractelement <16 x double> %add36356.i, i32 6
  store double %exData758.i, double addrspace(3)* %181, align 8
  br label %postload1804.i

postload1804.i:                                   ; preds = %preload1803.i, %postload1801.i
  br i1 %exmask637.i, label %preload1806.i, label %postload1807.i

preload1806.i:                                    ; preds = %postload1804.i
  %"&(pSB[currWI].offset)3165.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3166.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3165.i"
  %CastToValueType3167.i = bitcast i8* %"&pSB[currWI].offset3166.i" to i64*
  %loadedValue3168.i = load i64* %CastToValueType3167.i, align 8
  %.sum2870.i = add i64 %loadedValue3168.i, 7
  %182 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2870.i
  %exData761.i = extractelement <16 x double> %add36356.i, i32 7
  store double %exData761.i, double addrspace(3)* %182, align 8
  br label %postload1807.i

postload1807.i:                                   ; preds = %preload1806.i, %postload1804.i
  br i1 %exmask641.i, label %preload1959.i, label %postload1960.i

preload1959.i:                                    ; preds = %postload1807.i
  %"&(pSB[currWI].offset)3170.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3171.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3170.i"
  %CastToValueType3172.i = bitcast i8* %"&pSB[currWI].offset3171.i" to i64*
  %loadedValue3173.i = load i64* %CastToValueType3172.i, align 8
  %.sum2869.i = add i64 %loadedValue3173.i, 8
  %183 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2869.i
  %exData764.i = extractelement <16 x double> %add36356.i, i32 8
  store double %exData764.i, double addrspace(3)* %183, align 8
  br label %postload1960.i

postload1960.i:                                   ; preds = %preload1959.i, %postload1807.i
  br i1 %exmask645.i, label %preload1962.i, label %postload1963.i

preload1962.i:                                    ; preds = %postload1960.i
  %"&(pSB[currWI].offset)3175.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3176.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3175.i"
  %CastToValueType3177.i = bitcast i8* %"&pSB[currWI].offset3176.i" to i64*
  %loadedValue3178.i = load i64* %CastToValueType3177.i, align 8
  %.sum2868.i = add i64 %loadedValue3178.i, 9
  %184 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2868.i
  %exData767.i = extractelement <16 x double> %add36356.i, i32 9
  store double %exData767.i, double addrspace(3)* %184, align 8
  br label %postload1963.i

postload1963.i:                                   ; preds = %preload1962.i, %postload1960.i
  br i1 %exmask649.i, label %preload1965.i, label %postload1966.i

preload1965.i:                                    ; preds = %postload1963.i
  %"&(pSB[currWI].offset)3180.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3181.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3180.i"
  %CastToValueType3182.i = bitcast i8* %"&pSB[currWI].offset3181.i" to i64*
  %loadedValue3183.i = load i64* %CastToValueType3182.i, align 8
  %.sum2867.i = add i64 %loadedValue3183.i, 10
  %185 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2867.i
  %exData770.i = extractelement <16 x double> %add36356.i, i32 10
  store double %exData770.i, double addrspace(3)* %185, align 8
  br label %postload1966.i

postload1966.i:                                   ; preds = %preload1965.i, %postload1963.i
  br i1 %exmask653.i, label %preload1968.i, label %postload1969.i

preload1968.i:                                    ; preds = %postload1966.i
  %"&(pSB[currWI].offset)3185.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3186.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3185.i"
  %CastToValueType3187.i = bitcast i8* %"&pSB[currWI].offset3186.i" to i64*
  %loadedValue3188.i = load i64* %CastToValueType3187.i, align 8
  %.sum2866.i = add i64 %loadedValue3188.i, 11
  %186 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2866.i
  %exData773.i = extractelement <16 x double> %add36356.i, i32 11
  store double %exData773.i, double addrspace(3)* %186, align 8
  br label %postload1969.i

postload1969.i:                                   ; preds = %preload1968.i, %postload1966.i
  br i1 %exmask657.i, label %preload1902.i, label %postload1903.i

preload1902.i:                                    ; preds = %postload1969.i
  %"&(pSB[currWI].offset)3190.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3191.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3190.i"
  %CastToValueType3192.i = bitcast i8* %"&pSB[currWI].offset3191.i" to i64*
  %loadedValue3193.i = load i64* %CastToValueType3192.i, align 8
  %.sum2865.i = add i64 %loadedValue3193.i, 12
  %187 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2865.i
  %exData776.i = extractelement <16 x double> %add36356.i, i32 12
  store double %exData776.i, double addrspace(3)* %187, align 8
  br label %postload1903.i

postload1903.i:                                   ; preds = %preload1902.i, %postload1969.i
  br i1 %exmask661.i, label %preload1905.i, label %postload1906.i

preload1905.i:                                    ; preds = %postload1903.i
  %"&(pSB[currWI].offset)3195.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3196.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3195.i"
  %CastToValueType3197.i = bitcast i8* %"&pSB[currWI].offset3196.i" to i64*
  %loadedValue3198.i = load i64* %CastToValueType3197.i, align 8
  %.sum2864.i = add i64 %loadedValue3198.i, 13
  %188 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2864.i
  %exData779.i = extractelement <16 x double> %add36356.i, i32 13
  store double %exData779.i, double addrspace(3)* %188, align 8
  br label %postload1906.i

postload1906.i:                                   ; preds = %preload1905.i, %postload1903.i
  br i1 %exmask665.i, label %preload1908.i, label %postload1909.i

preload1908.i:                                    ; preds = %postload1906.i
  %"&(pSB[currWI].offset)3200.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3201.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3200.i"
  %CastToValueType3202.i = bitcast i8* %"&pSB[currWI].offset3201.i" to i64*
  %loadedValue3203.i = load i64* %CastToValueType3202.i, align 8
  %.sum2863.i = add i64 %loadedValue3203.i, 14
  %189 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2863.i
  %exData782.i = extractelement <16 x double> %add36356.i, i32 14
  store double %exData782.i, double addrspace(3)* %189, align 8
  br label %postload1909.i

postload1909.i:                                   ; preds = %preload1908.i, %postload1906.i
  br i1 %exmask669.i, label %preload2622.i, label %postload1909.i.if.end.i_crit_edge

postload1909.i.if.end.i_crit_edge:                ; preds = %postload1909.i
  br label %if.end.i

preload2622.i:                                    ; preds = %postload1909.i
  %"&(pSB[currWI].offset)3205.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset3206.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3205.i"
  %CastToValueType3207.i = bitcast i8* %"&pSB[currWI].offset3206.i" to i64*
  %loadedValue3208.i = load i64* %CastToValueType3207.i, align 8
  %.sum2862.i = add i64 %loadedValue3208.i, 15
  %190 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2862.i
  %exData785.i = extractelement <16 x double> %add36356.i, i32 15
  store double %exData785.i, double addrspace(3)* %190, align 8
  br label %if.end.i

if.end.i:                                         ; preds = %postload1909.i.if.end.i_crit_edge, %preload2622.i
  %"&(pSB[currWI].offset)4017.i" = add nuw i64 %CurrSBIndex..2.i, 98
  %"&pSB[currWI].offset4018.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4017.i"
  %CastToValueType4019.i = bitcast i8* %"&pSB[currWI].offset4018.i" to i1*
  %loadedValue4020.i = load i1* %CastToValueType4019.i, align 1
  br i1 %loadedValue4020.i, label %preload2513.i, label %if.end.i.postload2514.i_crit_edge

if.end.i.postload2514.i_crit_edge:                ; preds = %if.end.i
  br label %postload2514.i

preload2513.i:                                    ; preds = %if.end.i
  %check.WI.iter4614.i = icmp ult i64 %CurrWI..2.i, %31
  br i1 %check.WI.iter4614.i, label %thenBB4611.i, label %preload2513.i.postload2514.i_crit_edge

preload2513.i.postload2514.i_crit_edge:           ; preds = %preload2513.i
  br label %postload2514.i

thenBB4611.i:                                     ; preds = %preload2513.i
  %"CurrWI++4615.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride4617.i" = add nuw i64 %CurrSBIndex..2.i, 256
  %cond.i = icmp eq i32 %currBarrier.2.i, 14
  br i1 %cond.i, label %SyncBB.outer.i, label %thenBB4611.i.postload2512.i_crit_edge

thenBB4611.i.postload2512.i_crit_edge:            ; preds = %thenBB4611.i
  br label %postload2512.i

postload2514.i:                                   ; preds = %thenBB4603.i.postload2514.i_crit_edge, %thenBB4595.i.postload2514.i_crit_edge, %thenBB4572.i.postload2514.i_crit_edge, %thenBB.i.postload2514.i_crit_edge, %thenBB4580.i.postload2514.i_crit_edge, %preload2513.i.postload2514.i_crit_edge, %if.end.i.postload2514.i_crit_edge
  %CurrWI..4.i = phi i64 [ %CurrWI..2.i, %if.end.i.postload2514.i_crit_edge ], [ 0, %preload2513.i.postload2514.i_crit_edge ], [ %"CurrWI++4584.i", %thenBB4580.i.postload2514.i_crit_edge ], [ %"CurrWI++.i", %thenBB.i.postload2514.i_crit_edge ], [ %"CurrWI++4576.i", %thenBB4572.i.postload2514.i_crit_edge ], [ %"CurrWI++4599.i", %thenBB4595.i.postload2514.i_crit_edge ], [ %"CurrWI++4607.i", %thenBB4603.i.postload2514.i_crit_edge ]
  %CurrSBIndex..4.i = phi i64 [ %CurrSBIndex..2.i, %if.end.i.postload2514.i_crit_edge ], [ 0, %preload2513.i.postload2514.i_crit_edge ], [ %"loadedCurrSB+Stride4586.i", %thenBB4580.i.postload2514.i_crit_edge ], [ %"loadedCurrSB+Stride.i", %thenBB.i.postload2514.i_crit_edge ], [ %"loadedCurrSB+Stride4578.i", %thenBB4572.i.postload2514.i_crit_edge ], [ %"loadedCurrSB+Stride4601.i", %thenBB4595.i.postload2514.i_crit_edge ], [ %"loadedCurrSB+Stride4609.i", %thenBB4603.i.postload2514.i_crit_edge ]
  %currBarrier.4.i = phi i32 [ %currBarrier.2.i, %if.end.i.postload2514.i_crit_edge ], [ 13, %preload2513.i.postload2514.i_crit_edge ], [ %currBarrier.4.i, %thenBB4580.i.postload2514.i_crit_edge ], [ %currBarrier.6.i, %thenBB.i.postload2514.i_crit_edge ], [ %currBarrier.8.i, %thenBB4572.i.postload2514.i_crit_edge ], [ %currBarrier.10.i, %thenBB4595.i.postload2514.i_crit_edge ], [ %currBarrier.12.i, %thenBB4603.i.postload2514.i_crit_edge ]
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
  br i1 %exmask788.i, label %preload2625.i, label %postload2626.i

preload2625.i:                                    ; preds = %postload2514.i
  %192 = getelementptr inbounds [128 x double] addrspace(3)* %35, i64 0, i64 %extract361.i
  %vload789.i = load double addrspace(3)* %192, align 8
  br label %postload2626.i

postload2626.i:                                   ; preds = %preload2625.i, %postload2514.i
  %phi2627.i = phi double [ undef, %postload2514.i ], [ %vload789.i, %preload2625.i ]
  %vpack790.i = insertelement <16 x double> undef, double %phi2627.i, i32 0
  %exmask792.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 1
  br i1 %exmask792.i, label %preload2628.i, label %postload2629.i

preload2628.i:                                    ; preds = %postload2626.i
  %.sum2861.i = add i64 %extract361.i, 1
  %193 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2861.i
  %vload793.i = load double addrspace(3)* %193, align 8
  br label %postload2629.i

postload2629.i:                                   ; preds = %preload2628.i, %postload2626.i
  %phi2630.i = phi double [ undef, %postload2626.i ], [ %vload793.i, %preload2628.i ]
  %vpack794.i = insertelement <16 x double> %vpack790.i, double %phi2630.i, i32 1
  %exmask796.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 2
  br i1 %exmask796.i, label %preload1809.i, label %postload1810.i

preload1809.i:                                    ; preds = %postload2629.i
  %.sum2860.i = add i64 %extract361.i, 2
  %194 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2860.i
  %vload797.i = load double addrspace(3)* %194, align 8
  br label %postload1810.i

postload1810.i:                                   ; preds = %preload1809.i, %postload2629.i
  %phi1811.i = phi double [ undef, %postload2629.i ], [ %vload797.i, %preload1809.i ]
  %vpack798.i = insertelement <16 x double> %vpack794.i, double %phi1811.i, i32 2
  %exmask800.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 3
  br i1 %exmask800.i, label %preload1812.i, label %postload1813.i

preload1812.i:                                    ; preds = %postload1810.i
  %.sum2859.i = add i64 %extract361.i, 3
  %195 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2859.i
  %vload801.i = load double addrspace(3)* %195, align 8
  br label %postload1813.i

postload1813.i:                                   ; preds = %preload1812.i, %postload1810.i
  %phi1814.i = phi double [ undef, %postload1810.i ], [ %vload801.i, %preload1812.i ]
  %vpack802.i = insertelement <16 x double> %vpack798.i, double %phi1814.i, i32 3
  %exmask804.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 4
  br i1 %exmask804.i, label %preload1815.i, label %postload1816.i

preload1815.i:                                    ; preds = %postload1813.i
  %.sum2858.i = add i64 %extract361.i, 4
  %196 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2858.i
  %vload805.i = load double addrspace(3)* %196, align 8
  br label %postload1816.i

postload1816.i:                                   ; preds = %preload1815.i, %postload1813.i
  %phi1817.i = phi double [ undef, %postload1813.i ], [ %vload805.i, %preload1815.i ]
  %vpack806.i = insertelement <16 x double> %vpack802.i, double %phi1817.i, i32 4
  %exmask808.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 5
  br i1 %exmask808.i, label %preload1818.i, label %postload1819.i

preload1818.i:                                    ; preds = %postload1816.i
  %.sum2857.i = add i64 %extract361.i, 5
  %197 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2857.i
  %vload809.i = load double addrspace(3)* %197, align 8
  br label %postload1819.i

postload1819.i:                                   ; preds = %preload1818.i, %postload1816.i
  %phi1820.i = phi double [ undef, %postload1816.i ], [ %vload809.i, %preload1818.i ]
  %vpack810.i = insertelement <16 x double> %vpack806.i, double %phi1820.i, i32 5
  %exmask812.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 6
  br i1 %exmask812.i, label %preload1653.i, label %postload1654.i

preload1653.i:                                    ; preds = %postload1819.i
  %.sum2856.i = add i64 %extract361.i, 6
  %198 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2856.i
  %vload813.i = load double addrspace(3)* %198, align 8
  br label %postload1654.i

postload1654.i:                                   ; preds = %preload1653.i, %postload1819.i
  %phi1655.i = phi double [ undef, %postload1819.i ], [ %vload813.i, %preload1653.i ]
  %vpack814.i = insertelement <16 x double> %vpack810.i, double %phi1655.i, i32 6
  %exmask816.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 7
  br i1 %exmask816.i, label %preload1656.i, label %postload1657.i

preload1656.i:                                    ; preds = %postload1654.i
  %.sum2855.i = add i64 %extract361.i, 7
  %199 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2855.i
  %vload817.i = load double addrspace(3)* %199, align 8
  br label %postload1657.i

postload1657.i:                                   ; preds = %preload1656.i, %postload1654.i
  %phi1658.i = phi double [ undef, %postload1654.i ], [ %vload817.i, %preload1656.i ]
  %vpack818.i = insertelement <16 x double> %vpack814.i, double %phi1658.i, i32 7
  %exmask820.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 8
  br i1 %exmask820.i, label %preload1659.i, label %postload1660.i

preload1659.i:                                    ; preds = %postload1657.i
  %.sum2854.i = add i64 %extract361.i, 8
  %200 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2854.i
  %vload821.i = load double addrspace(3)* %200, align 8
  br label %postload1660.i

postload1660.i:                                   ; preds = %preload1659.i, %postload1657.i
  %phi1661.i = phi double [ undef, %postload1657.i ], [ %vload821.i, %preload1659.i ]
  %vpack822.i = insertelement <16 x double> %vpack818.i, double %phi1661.i, i32 8
  %exmask824.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 9
  br i1 %exmask824.i, label %preload2085.i, label %postload2086.i

preload2085.i:                                    ; preds = %postload1660.i
  %.sum2853.i = add i64 %extract361.i, 9
  %201 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2853.i
  %vload825.i = load double addrspace(3)* %201, align 8
  br label %postload2086.i

postload2086.i:                                   ; preds = %preload2085.i, %postload1660.i
  %phi2087.i = phi double [ undef, %postload1660.i ], [ %vload825.i, %preload2085.i ]
  %vpack826.i = insertelement <16 x double> %vpack822.i, double %phi2087.i, i32 9
  %exmask828.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 10
  br i1 %exmask828.i, label %preload2088.i, label %postload2089.i

preload2088.i:                                    ; preds = %postload2086.i
  %.sum2852.i = add i64 %extract361.i, 10
  %202 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2852.i
  %vload829.i = load double addrspace(3)* %202, align 8
  br label %postload2089.i

postload2089.i:                                   ; preds = %preload2088.i, %postload2086.i
  %phi2090.i = phi double [ undef, %postload2086.i ], [ %vload829.i, %preload2088.i ]
  %vpack830.i = insertelement <16 x double> %vpack826.i, double %phi2090.i, i32 10
  %exmask832.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 11
  br i1 %exmask832.i, label %preload2091.i, label %postload2092.i

preload2091.i:                                    ; preds = %postload2089.i
  %.sum2851.i = add i64 %extract361.i, 11
  %203 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2851.i
  %vload833.i = load double addrspace(3)* %203, align 8
  br label %postload2092.i

postload2092.i:                                   ; preds = %preload2091.i, %postload2089.i
  %phi2093.i = phi double [ undef, %postload2089.i ], [ %vload833.i, %preload2091.i ]
  %vpack834.i = insertelement <16 x double> %vpack830.i, double %phi2093.i, i32 11
  %exmask836.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 12
  br i1 %exmask836.i, label %preload2094.i, label %postload2095.i

preload2094.i:                                    ; preds = %postload2092.i
  %.sum2850.i = add i64 %extract361.i, 12
  %204 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2850.i
  %vload837.i = load double addrspace(3)* %204, align 8
  br label %postload2095.i

postload2095.i:                                   ; preds = %preload2094.i, %postload2092.i
  %phi2096.i = phi double [ undef, %postload2092.i ], [ %vload837.i, %preload2094.i ]
  %vpack838.i = insertelement <16 x double> %vpack834.i, double %phi2096.i, i32 12
  %exmask840.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 13
  br i1 %exmask840.i, label %preload2388.i, label %postload2389.i

preload2388.i:                                    ; preds = %postload2095.i
  %.sum2849.i = add i64 %extract361.i, 13
  %205 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2849.i
  %vload841.i = load double addrspace(3)* %205, align 8
  br label %postload2389.i

postload2389.i:                                   ; preds = %preload2388.i, %postload2095.i
  %phi2390.i = phi double [ undef, %postload2095.i ], [ %vload841.i, %preload2388.i ]
  %vpack842.i = insertelement <16 x double> %vpack838.i, double %phi2390.i, i32 13
  %exmask844.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 14
  br i1 %exmask844.i, label %preload2391.i, label %postload2392.i

preload2391.i:                                    ; preds = %postload2389.i
  %.sum2848.i = add i64 %extract361.i, 14
  %206 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2848.i
  %vload845.i = load double addrspace(3)* %206, align 8
  br label %postload2392.i

postload2392.i:                                   ; preds = %preload2391.i, %postload2389.i
  %phi2393.i = phi double [ undef, %postload2389.i ], [ %vload845.i, %preload2391.i ]
  %vpack846.i = insertelement <16 x double> %vpack842.i, double %phi2393.i, i32 14
  %exmask848.i = extractelement <16 x i1> %if.end_to_if.then39358.i, i32 15
  br i1 %exmask848.i, label %preload2394.i, label %postload2395.i

preload2394.i:                                    ; preds = %postload2392.i
  %.sum2847.i = add i64 %extract361.i, 15
  %207 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2847.i
  %vload849.i = load double addrspace(3)* %207, align 8
  br label %postload2395.i

postload2395.i:                                   ; preds = %preload2394.i, %postload2392.i
  %phi2396.i = phi double [ undef, %postload2392.i ], [ %vload849.i, %preload2394.i ]
  %vpack850.i = insertelement <16 x double> %vpack846.i, double %phi2396.i, i32 15
  br i1 %exmask788.i, label %preload2247.i, label %postload2248.i

preload2247.i:                                    ; preds = %postload2395.i
  %"&(pSB[currWI].offset)3904.i" = add nuw i64 %CurrSBIndex..4.i, 80
  %"&pSB[currWI].offset3905.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3904.i"
  %CastToValueType3906.i = bitcast i8* %"&pSB[currWI].offset3905.i" to double addrspace(3)**
  %loadedValue3907.i = load double addrspace(3)** %CastToValueType3906.i, align 8
  %vload854.i = load double addrspace(3)* %loadedValue3907.i, align 8
  br label %postload2248.i

postload2248.i:                                   ; preds = %preload2247.i, %postload2395.i
  %phi2249.i = phi double [ undef, %postload2395.i ], [ %vload854.i, %preload2247.i ]
  %vpack855.i = insertelement <16 x double> undef, double %phi2249.i, i32 0
  br i1 %exmask792.i, label %preload2250.i, label %postload2251.i

preload2250.i:                                    ; preds = %postload2248.i
  %"&(pSB[currWI].offset)3210.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3211.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3210.i"
  %CastToValueType3212.i = bitcast i8* %"&pSB[currWI].offset3211.i" to i64*
  %loadedValue3213.i = load i64* %CastToValueType3212.i, align 8
  %.sum2846.i = add i64 %loadedValue3213.i, 1
  %208 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2846.i
  %vload858.i = load double addrspace(3)* %208, align 8
  br label %postload2251.i

postload2251.i:                                   ; preds = %preload2250.i, %postload2248.i
  %phi2252.i = phi double [ undef, %postload2248.i ], [ %vload858.i, %preload2250.i ]
  %vpack859.i = insertelement <16 x double> %vpack855.i, double %phi2252.i, i32 1
  br i1 %exmask796.i, label %preload2253.i, label %postload2254.i

preload2253.i:                                    ; preds = %postload2251.i
  %"&(pSB[currWI].offset)3215.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3216.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3215.i"
  %CastToValueType3217.i = bitcast i8* %"&pSB[currWI].offset3216.i" to i64*
  %loadedValue3218.i = load i64* %CastToValueType3217.i, align 8
  %.sum2845.i = add i64 %loadedValue3218.i, 2
  %209 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2845.i
  %vload862.i = load double addrspace(3)* %209, align 8
  br label %postload2254.i

postload2254.i:                                   ; preds = %preload2253.i, %postload2251.i
  %phi2255.i = phi double [ undef, %postload2251.i ], [ %vload862.i, %preload2253.i ]
  %vpack863.i = insertelement <16 x double> %vpack859.i, double %phi2255.i, i32 2
  br i1 %exmask800.i, label %preload2256.i, label %postload2257.i

preload2256.i:                                    ; preds = %postload2254.i
  %"&(pSB[currWI].offset)3220.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3221.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3220.i"
  %CastToValueType3222.i = bitcast i8* %"&pSB[currWI].offset3221.i" to i64*
  %loadedValue3223.i = load i64* %CastToValueType3222.i, align 8
  %.sum2844.i = add i64 %loadedValue3223.i, 3
  %210 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2844.i
  %vload866.i = load double addrspace(3)* %210, align 8
  br label %postload2257.i

postload2257.i:                                   ; preds = %preload2256.i, %postload2254.i
  %phi2258.i = phi double [ undef, %postload2254.i ], [ %vload866.i, %preload2256.i ]
  %vpack867.i = insertelement <16 x double> %vpack863.i, double %phi2258.i, i32 3
  br i1 %exmask804.i, label %preload2610.i, label %postload2611.i

preload2610.i:                                    ; preds = %postload2257.i
  %"&(pSB[currWI].offset)3225.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3226.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3225.i"
  %CastToValueType3227.i = bitcast i8* %"&pSB[currWI].offset3226.i" to i64*
  %loadedValue3228.i = load i64* %CastToValueType3227.i, align 8
  %.sum2843.i = add i64 %loadedValue3228.i, 4
  %211 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2843.i
  %vload870.i = load double addrspace(3)* %211, align 8
  br label %postload2611.i

postload2611.i:                                   ; preds = %preload2610.i, %postload2257.i
  %phi2612.i = phi double [ undef, %postload2257.i ], [ %vload870.i, %preload2610.i ]
  %vpack871.i = insertelement <16 x double> %vpack867.i, double %phi2612.i, i32 4
  br i1 %exmask808.i, label %preload2613.i, label %postload2614.i

preload2613.i:                                    ; preds = %postload2611.i
  %"&(pSB[currWI].offset)3230.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3231.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3230.i"
  %CastToValueType3232.i = bitcast i8* %"&pSB[currWI].offset3231.i" to i64*
  %loadedValue3233.i = load i64* %CastToValueType3232.i, align 8
  %.sum2842.i = add i64 %loadedValue3233.i, 5
  %212 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2842.i
  %vload874.i = load double addrspace(3)* %212, align 8
  br label %postload2614.i

postload2614.i:                                   ; preds = %preload2613.i, %postload2611.i
  %phi2615.i = phi double [ undef, %postload2611.i ], [ %vload874.i, %preload2613.i ]
  %vpack875.i = insertelement <16 x double> %vpack871.i, double %phi2615.i, i32 5
  br i1 %exmask812.i, label %preload2616.i, label %postload2617.i

preload2616.i:                                    ; preds = %postload2614.i
  %"&(pSB[currWI].offset)3235.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3236.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3235.i"
  %CastToValueType3237.i = bitcast i8* %"&pSB[currWI].offset3236.i" to i64*
  %loadedValue3238.i = load i64* %CastToValueType3237.i, align 8
  %.sum2841.i = add i64 %loadedValue3238.i, 6
  %213 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2841.i
  %vload878.i = load double addrspace(3)* %213, align 8
  br label %postload2617.i

postload2617.i:                                   ; preds = %preload2616.i, %postload2614.i
  %phi2618.i = phi double [ undef, %postload2614.i ], [ %vload878.i, %preload2616.i ]
  %vpack879.i = insertelement <16 x double> %vpack875.i, double %phi2618.i, i32 6
  br i1 %exmask816.i, label %preload2478.i, label %postload2479.i

preload2478.i:                                    ; preds = %postload2617.i
  %"&(pSB[currWI].offset)3240.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3241.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3240.i"
  %CastToValueType3242.i = bitcast i8* %"&pSB[currWI].offset3241.i" to i64*
  %loadedValue3243.i = load i64* %CastToValueType3242.i, align 8
  %.sum2840.i = add i64 %loadedValue3243.i, 7
  %214 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2840.i
  %vload882.i = load double addrspace(3)* %214, align 8
  br label %postload2479.i

postload2479.i:                                   ; preds = %preload2478.i, %postload2617.i
  %phi2480.i = phi double [ undef, %postload2617.i ], [ %vload882.i, %preload2478.i ]
  %vpack883.i = insertelement <16 x double> %vpack879.i, double %phi2480.i, i32 7
  br i1 %exmask820.i, label %preload2481.i, label %postload2482.i

preload2481.i:                                    ; preds = %postload2479.i
  %"&(pSB[currWI].offset)3245.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3246.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3245.i"
  %CastToValueType3247.i = bitcast i8* %"&pSB[currWI].offset3246.i" to i64*
  %loadedValue3248.i = load i64* %CastToValueType3247.i, align 8
  %.sum2839.i = add i64 %loadedValue3248.i, 8
  %215 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2839.i
  %vload886.i = load double addrspace(3)* %215, align 8
  br label %postload2482.i

postload2482.i:                                   ; preds = %preload2481.i, %postload2479.i
  %phi2483.i = phi double [ undef, %postload2479.i ], [ %vload886.i, %preload2481.i ]
  %vpack887.i = insertelement <16 x double> %vpack883.i, double %phi2483.i, i32 8
  br i1 %exmask824.i, label %preload2484.i, label %postload2485.i

preload2484.i:                                    ; preds = %postload2482.i
  %"&(pSB[currWI].offset)3250.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3251.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3250.i"
  %CastToValueType3252.i = bitcast i8* %"&pSB[currWI].offset3251.i" to i64*
  %loadedValue3253.i = load i64* %CastToValueType3252.i, align 8
  %.sum2838.i = add i64 %loadedValue3253.i, 9
  %216 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2838.i
  %vload890.i = load double addrspace(3)* %216, align 8
  br label %postload2485.i

postload2485.i:                                   ; preds = %preload2484.i, %postload2482.i
  %phi2486.i = phi double [ undef, %postload2482.i ], [ %vload890.i, %preload2484.i ]
  %vpack891.i = insertelement <16 x double> %vpack887.i, double %phi2486.i, i32 9
  br i1 %exmask828.i, label %preload2487.i, label %postload2488.i

preload2487.i:                                    ; preds = %postload2485.i
  %"&(pSB[currWI].offset)3255.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3256.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3255.i"
  %CastToValueType3257.i = bitcast i8* %"&pSB[currWI].offset3256.i" to i64*
  %loadedValue3258.i = load i64* %CastToValueType3257.i, align 8
  %.sum2837.i = add i64 %loadedValue3258.i, 10
  %217 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2837.i
  %vload894.i = load double addrspace(3)* %217, align 8
  br label %postload2488.i

postload2488.i:                                   ; preds = %preload2487.i, %postload2485.i
  %phi2489.i = phi double [ undef, %postload2485.i ], [ %vload894.i, %preload2487.i ]
  %vpack895.i = insertelement <16 x double> %vpack891.i, double %phi2489.i, i32 10
  br i1 %exmask832.i, label %preload2277.i, label %postload2278.i

preload2277.i:                                    ; preds = %postload2488.i
  %"&(pSB[currWI].offset)3260.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3261.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3260.i"
  %CastToValueType3262.i = bitcast i8* %"&pSB[currWI].offset3261.i" to i64*
  %loadedValue3263.i = load i64* %CastToValueType3262.i, align 8
  %.sum2836.i = add i64 %loadedValue3263.i, 11
  %218 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2836.i
  %vload898.i = load double addrspace(3)* %218, align 8
  br label %postload2278.i

postload2278.i:                                   ; preds = %preload2277.i, %postload2488.i
  %phi2279.i = phi double [ undef, %postload2488.i ], [ %vload898.i, %preload2277.i ]
  %vpack899.i = insertelement <16 x double> %vpack895.i, double %phi2279.i, i32 11
  br i1 %exmask836.i, label %preload2280.i, label %postload2281.i

preload2280.i:                                    ; preds = %postload2278.i
  %"&(pSB[currWI].offset)3265.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3266.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3265.i"
  %CastToValueType3267.i = bitcast i8* %"&pSB[currWI].offset3266.i" to i64*
  %loadedValue3268.i = load i64* %CastToValueType3267.i, align 8
  %.sum2835.i = add i64 %loadedValue3268.i, 12
  %219 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2835.i
  %vload902.i = load double addrspace(3)* %219, align 8
  br label %postload2281.i

postload2281.i:                                   ; preds = %preload2280.i, %postload2278.i
  %phi2282.i = phi double [ undef, %postload2278.i ], [ %vload902.i, %preload2280.i ]
  %vpack903.i = insertelement <16 x double> %vpack899.i, double %phi2282.i, i32 12
  br i1 %exmask840.i, label %preload1938.i, label %postload1939.i

preload1938.i:                                    ; preds = %postload2281.i
  %"&(pSB[currWI].offset)3270.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3271.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3270.i"
  %CastToValueType3272.i = bitcast i8* %"&pSB[currWI].offset3271.i" to i64*
  %loadedValue3273.i = load i64* %CastToValueType3272.i, align 8
  %.sum2834.i = add i64 %loadedValue3273.i, 13
  %220 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2834.i
  %vload906.i = load double addrspace(3)* %220, align 8
  br label %postload1939.i

postload1939.i:                                   ; preds = %preload1938.i, %postload2281.i
  %phi1940.i = phi double [ undef, %postload2281.i ], [ %vload906.i, %preload1938.i ]
  %vpack907.i = insertelement <16 x double> %vpack903.i, double %phi1940.i, i32 13
  br i1 %exmask844.i, label %preload1941.i, label %postload1942.i

preload1941.i:                                    ; preds = %postload1939.i
  %"&(pSB[currWI].offset)3275.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3276.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3275.i"
  %CastToValueType3277.i = bitcast i8* %"&pSB[currWI].offset3276.i" to i64*
  %loadedValue3278.i = load i64* %CastToValueType3277.i, align 8
  %.sum2833.i = add i64 %loadedValue3278.i, 14
  %221 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2833.i
  %vload910.i = load double addrspace(3)* %221, align 8
  br label %postload1942.i

postload1942.i:                                   ; preds = %preload1941.i, %postload1939.i
  %phi1943.i = phi double [ undef, %postload1939.i ], [ %vload910.i, %preload1941.i ]
  %vpack911.i = insertelement <16 x double> %vpack907.i, double %phi1943.i, i32 14
  br i1 %exmask848.i, label %preload1944.i, label %postload1945.i

preload1944.i:                                    ; preds = %postload1942.i
  %"&(pSB[currWI].offset)3280.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3281.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3280.i"
  %CastToValueType3282.i = bitcast i8* %"&pSB[currWI].offset3281.i" to i64*
  %loadedValue3283.i = load i64* %CastToValueType3282.i, align 8
  %.sum2832.i = add i64 %loadedValue3283.i, 15
  %222 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2832.i
  %vload914.i = load double addrspace(3)* %222, align 8
  br label %postload1945.i

postload1945.i:                                   ; preds = %preload1944.i, %postload1942.i
  %phi1946.i = phi double [ undef, %postload1942.i ], [ %vload914.i, %preload1944.i ]
  %vpack915.i = insertelement <16 x double> %vpack911.i, double %phi1946.i, i32 15
  %add45379.i = fadd <16 x double> %vpack915.i, %vpack850.i
  br i1 %exmask788.i, label %preload2331.i, label %postload2332.i

preload2331.i:                                    ; preds = %postload1945.i
  %exData919.i = extractelement <16 x double> %add45379.i, i32 0
  %"&(pSB[currWI].offset)3909.i" = add nuw i64 %CurrSBIndex..4.i, 80
  %"&pSB[currWI].offset3910.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3909.i"
  %CastToValueType3911.i = bitcast i8* %"&pSB[currWI].offset3910.i" to double addrspace(3)**
  %loadedValue3912.i = load double addrspace(3)** %CastToValueType3911.i, align 8
  store double %exData919.i, double addrspace(3)* %loadedValue3912.i, align 8
  br label %postload2332.i

postload2332.i:                                   ; preds = %preload2331.i, %postload1945.i
  br i1 %exmask792.i, label %preload2334.i, label %postload2335.i

preload2334.i:                                    ; preds = %postload2332.i
  %"&(pSB[currWI].offset)3285.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3286.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3285.i"
  %CastToValueType3287.i = bitcast i8* %"&pSB[currWI].offset3286.i" to i64*
  %loadedValue3288.i = load i64* %CastToValueType3287.i, align 8
  %.sum2831.i = add i64 %loadedValue3288.i, 1
  %223 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2831.i
  %exData922.i = extractelement <16 x double> %add45379.i, i32 1
  store double %exData922.i, double addrspace(3)* %223, align 8
  br label %postload2335.i

postload2335.i:                                   ; preds = %preload2334.i, %postload2332.i
  br i1 %exmask796.i, label %preload2337.i, label %postload2338.i

preload2337.i:                                    ; preds = %postload2335.i
  %"&(pSB[currWI].offset)3290.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3291.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3290.i"
  %CastToValueType3292.i = bitcast i8* %"&pSB[currWI].offset3291.i" to i64*
  %loadedValue3293.i = load i64* %CastToValueType3292.i, align 8
  %.sum2830.i = add i64 %loadedValue3293.i, 2
  %224 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2830.i
  %exData925.i = extractelement <16 x double> %add45379.i, i32 2
  store double %exData925.i, double addrspace(3)* %224, align 8
  br label %postload2338.i

postload2338.i:                                   ; preds = %preload2337.i, %postload2335.i
  br i1 %exmask800.i, label %preload2340.i, label %postload2341.i

preload2340.i:                                    ; preds = %postload2338.i
  %"&(pSB[currWI].offset)3295.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3296.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3295.i"
  %CastToValueType3297.i = bitcast i8* %"&pSB[currWI].offset3296.i" to i64*
  %loadedValue3298.i = load i64* %CastToValueType3297.i, align 8
  %.sum2829.i = add i64 %loadedValue3298.i, 3
  %225 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2829.i
  %exData928.i = extractelement <16 x double> %add45379.i, i32 3
  store double %exData928.i, double addrspace(3)* %225, align 8
  br label %postload2341.i

postload2341.i:                                   ; preds = %preload2340.i, %postload2338.i
  br i1 %exmask804.i, label %preload1911.i, label %postload1912.i

preload1911.i:                                    ; preds = %postload2341.i
  %"&(pSB[currWI].offset)3300.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3301.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3300.i"
  %CastToValueType3302.i = bitcast i8* %"&pSB[currWI].offset3301.i" to i64*
  %loadedValue3303.i = load i64* %CastToValueType3302.i, align 8
  %.sum2828.i = add i64 %loadedValue3303.i, 4
  %226 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2828.i
  %exData931.i = extractelement <16 x double> %add45379.i, i32 4
  store double %exData931.i, double addrspace(3)* %226, align 8
  br label %postload1912.i

postload1912.i:                                   ; preds = %preload1911.i, %postload2341.i
  br i1 %exmask808.i, label %preload1914.i, label %postload1915.i

preload1914.i:                                    ; preds = %postload1912.i
  %"&(pSB[currWI].offset)3305.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3306.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3305.i"
  %CastToValueType3307.i = bitcast i8* %"&pSB[currWI].offset3306.i" to i64*
  %loadedValue3308.i = load i64* %CastToValueType3307.i, align 8
  %.sum2827.i = add i64 %loadedValue3308.i, 5
  %227 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2827.i
  %exData934.i = extractelement <16 x double> %add45379.i, i32 5
  store double %exData934.i, double addrspace(3)* %227, align 8
  br label %postload1915.i

postload1915.i:                                   ; preds = %preload1914.i, %postload1912.i
  br i1 %exmask812.i, label %preload1917.i, label %postload1918.i

preload1917.i:                                    ; preds = %postload1915.i
  %"&(pSB[currWI].offset)3310.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3311.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3310.i"
  %CastToValueType3312.i = bitcast i8* %"&pSB[currWI].offset3311.i" to i64*
  %loadedValue3313.i = load i64* %CastToValueType3312.i, align 8
  %.sum2826.i = add i64 %loadedValue3313.i, 6
  %228 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2826.i
  %exData937.i = extractelement <16 x double> %add45379.i, i32 6
  store double %exData937.i, double addrspace(3)* %228, align 8
  br label %postload1918.i

postload1918.i:                                   ; preds = %preload1917.i, %postload1915.i
  br i1 %exmask816.i, label %preload2319.i, label %postload2320.i

preload2319.i:                                    ; preds = %postload1918.i
  %"&(pSB[currWI].offset)3315.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3316.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3315.i"
  %CastToValueType3317.i = bitcast i8* %"&pSB[currWI].offset3316.i" to i64*
  %loadedValue3318.i = load i64* %CastToValueType3317.i, align 8
  %.sum2825.i = add i64 %loadedValue3318.i, 7
  %229 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2825.i
  %exData940.i = extractelement <16 x double> %add45379.i, i32 7
  store double %exData940.i, double addrspace(3)* %229, align 8
  br label %postload2320.i

postload2320.i:                                   ; preds = %preload2319.i, %postload1918.i
  br i1 %exmask820.i, label %preload2322.i, label %postload2323.i

preload2322.i:                                    ; preds = %postload2320.i
  %"&(pSB[currWI].offset)3320.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3321.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3320.i"
  %CastToValueType3322.i = bitcast i8* %"&pSB[currWI].offset3321.i" to i64*
  %loadedValue3323.i = load i64* %CastToValueType3322.i, align 8
  %.sum2824.i = add i64 %loadedValue3323.i, 8
  %230 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2824.i
  %exData943.i = extractelement <16 x double> %add45379.i, i32 8
  store double %exData943.i, double addrspace(3)* %230, align 8
  br label %postload2323.i

postload2323.i:                                   ; preds = %preload2322.i, %postload2320.i
  br i1 %exmask824.i, label %preload2325.i, label %postload2326.i

preload2325.i:                                    ; preds = %postload2323.i
  %"&(pSB[currWI].offset)3325.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3326.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3325.i"
  %CastToValueType3327.i = bitcast i8* %"&pSB[currWI].offset3326.i" to i64*
  %loadedValue3328.i = load i64* %CastToValueType3327.i, align 8
  %.sum2823.i = add i64 %loadedValue3328.i, 9
  %231 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2823.i
  %exData946.i = extractelement <16 x double> %add45379.i, i32 9
  store double %exData946.i, double addrspace(3)* %231, align 8
  br label %postload2326.i

postload2326.i:                                   ; preds = %preload2325.i, %postload2323.i
  br i1 %exmask828.i, label %preload2328.i, label %postload2329.i

preload2328.i:                                    ; preds = %postload2326.i
  %"&(pSB[currWI].offset)3330.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3331.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3330.i"
  %CastToValueType3332.i = bitcast i8* %"&pSB[currWI].offset3331.i" to i64*
  %loadedValue3333.i = load i64* %CastToValueType3332.i, align 8
  %.sum2822.i = add i64 %loadedValue3333.i, 10
  %232 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2822.i
  %exData949.i = extractelement <16 x double> %add45379.i, i32 10
  store double %exData949.i, double addrspace(3)* %232, align 8
  br label %postload2329.i

postload2329.i:                                   ; preds = %preload2328.i, %postload2326.i
  br i1 %exmask832.i, label %preload1776.i, label %postload1777.i

preload1776.i:                                    ; preds = %postload2329.i
  %"&(pSB[currWI].offset)3335.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3336.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3335.i"
  %CastToValueType3337.i = bitcast i8* %"&pSB[currWI].offset3336.i" to i64*
  %loadedValue3338.i = load i64* %CastToValueType3337.i, align 8
  %.sum2821.i = add i64 %loadedValue3338.i, 11
  %233 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2821.i
  %exData952.i = extractelement <16 x double> %add45379.i, i32 11
  store double %exData952.i, double addrspace(3)* %233, align 8
  br label %postload1777.i

postload1777.i:                                   ; preds = %preload1776.i, %postload2329.i
  br i1 %exmask836.i, label %preload1779.i, label %postload1780.i

preload1779.i:                                    ; preds = %postload1777.i
  %"&(pSB[currWI].offset)3340.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3341.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3340.i"
  %CastToValueType3342.i = bitcast i8* %"&pSB[currWI].offset3341.i" to i64*
  %loadedValue3343.i = load i64* %CastToValueType3342.i, align 8
  %.sum2820.i = add i64 %loadedValue3343.i, 12
  %234 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2820.i
  %exData955.i = extractelement <16 x double> %add45379.i, i32 12
  store double %exData955.i, double addrspace(3)* %234, align 8
  br label %postload1780.i

postload1780.i:                                   ; preds = %preload1779.i, %postload1777.i
  br i1 %exmask840.i, label %preload1782.i, label %postload1783.i

preload1782.i:                                    ; preds = %postload1780.i
  %"&(pSB[currWI].offset)3345.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3346.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3345.i"
  %CastToValueType3347.i = bitcast i8* %"&pSB[currWI].offset3346.i" to i64*
  %loadedValue3348.i = load i64* %CastToValueType3347.i, align 8
  %.sum2819.i = add i64 %loadedValue3348.i, 13
  %235 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2819.i
  %exData958.i = extractelement <16 x double> %add45379.i, i32 13
  store double %exData958.i, double addrspace(3)* %235, align 8
  br label %postload1783.i

postload1783.i:                                   ; preds = %preload1782.i, %postload1780.i
  br i1 %exmask844.i, label %preload1785.i, label %postload1786.i

preload1785.i:                                    ; preds = %postload1783.i
  %"&(pSB[currWI].offset)3350.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3351.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3350.i"
  %CastToValueType3352.i = bitcast i8* %"&pSB[currWI].offset3351.i" to i64*
  %loadedValue3353.i = load i64* %CastToValueType3352.i, align 8
  %.sum2818.i = add i64 %loadedValue3353.i, 14
  %236 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2818.i
  %exData961.i = extractelement <16 x double> %add45379.i, i32 14
  store double %exData961.i, double addrspace(3)* %236, align 8
  br label %postload1786.i

postload1786.i:                                   ; preds = %preload1785.i, %postload1783.i
  br i1 %exmask848.i, label %preload1634.i, label %postload1786.i.if.end46.i_crit_edge

postload1786.i.if.end46.i_crit_edge:              ; preds = %postload1786.i
  br label %if.end46.i

preload1634.i:                                    ; preds = %postload1786.i
  %"&(pSB[currWI].offset)3355.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset3356.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3355.i"
  %CastToValueType3357.i = bitcast i8* %"&pSB[currWI].offset3356.i" to i64*
  %loadedValue3358.i = load i64* %CastToValueType3357.i, align 8
  %.sum2817.i = add i64 %loadedValue3358.i, 15
  %237 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2817.i
  %exData964.i = extractelement <16 x double> %add45379.i, i32 15
  store double %exData964.i, double addrspace(3)* %237, align 8
  br label %if.end46.i

if.end46.i:                                       ; preds = %postload1786.i.if.end46.i_crit_edge, %preload1634.i
  %"&(pSB[currWI].offset)4012.i" = add nuw i64 %CurrSBIndex..4.i, 98
  %"&pSB[currWI].offset4013.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4012.i"
  %CastToValueType4014.i = bitcast i8* %"&pSB[currWI].offset4013.i" to i1*
  %loadedValue4015.i = load i1* %CastToValueType4014.i, align 1
  br i1 %loadedValue4015.i, label %preload2515.i, label %if.end46.i.postload2516.i_crit_edge

if.end46.i.postload2516.i_crit_edge:              ; preds = %if.end46.i
  br label %postload2516.i

preload2515.i:                                    ; preds = %if.end46.i
  %check.WI.iter4583.i = icmp ult i64 %CurrWI..4.i, %31
  br i1 %check.WI.iter4583.i, label %thenBB4580.i, label %preload2515.i.postload2516.i_crit_edge

preload2515.i.postload2516.i_crit_edge:           ; preds = %preload2515.i
  br label %postload2516.i

thenBB4580.i:                                     ; preds = %preload2515.i
  %"CurrWI++4584.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride4586.i" = add nuw i64 %CurrSBIndex..4.i, 256
  switch i32 %currBarrier.4.i, label %thenBB4580.i.postload2514.i_crit_edge [
    i32 8, label %thenBB4580.i.postload2512.i_crit_edge
    i32 14, label %SyncBB.outer.i
  ]

thenBB4580.i.postload2512.i_crit_edge:            ; preds = %thenBB4580.i
  br label %postload2512.i

thenBB4580.i.postload2514.i_crit_edge:            ; preds = %thenBB4580.i
  br label %postload2514.i

postload2516.i:                                   ; preds = %thenBB4603.i.postload2516.i_crit_edge, %thenBB4595.i.postload2516.i_crit_edge, %thenBB4572.i.postload2516.i_crit_edge, %thenBB.i.postload2516.i_crit_edge, %preload2515.i.postload2516.i_crit_edge, %if.end46.i.postload2516.i_crit_edge
  %CurrWI..6.i = phi i64 [ %CurrWI..4.i, %if.end46.i.postload2516.i_crit_edge ], [ 0, %preload2515.i.postload2516.i_crit_edge ], [ %"CurrWI++.i", %thenBB.i.postload2516.i_crit_edge ], [ %"CurrWI++4576.i", %thenBB4572.i.postload2516.i_crit_edge ], [ %"CurrWI++4599.i", %thenBB4595.i.postload2516.i_crit_edge ], [ %"CurrWI++4607.i", %thenBB4603.i.postload2516.i_crit_edge ]
  %CurrSBIndex..6.i = phi i64 [ %CurrSBIndex..4.i, %if.end46.i.postload2516.i_crit_edge ], [ 0, %preload2515.i.postload2516.i_crit_edge ], [ %"loadedCurrSB+Stride.i", %thenBB.i.postload2516.i_crit_edge ], [ %"loadedCurrSB+Stride4578.i", %thenBB4572.i.postload2516.i_crit_edge ], [ %"loadedCurrSB+Stride4601.i", %thenBB4595.i.postload2516.i_crit_edge ], [ %"loadedCurrSB+Stride4609.i", %thenBB4603.i.postload2516.i_crit_edge ]
  %currBarrier.6.i = phi i32 [ %currBarrier.4.i, %if.end46.i.postload2516.i_crit_edge ], [ 5, %preload2515.i.postload2516.i_crit_edge ], [ %currBarrier.6.i, %thenBB.i.postload2516.i_crit_edge ], [ %currBarrier.8.i, %thenBB4572.i.postload2516.i_crit_edge ], [ %currBarrier.10.i, %thenBB4595.i.postload2516.i_crit_edge ], [ %currBarrier.12.i, %thenBB4603.i.postload2516.i_crit_edge ]
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
  br i1 %exmask967.i, label %preload1637.i, label %postload1638.i

preload1637.i:                                    ; preds = %postload2516.i
  %239 = getelementptr inbounds [128 x double] addrspace(3)* %35, i64 0, i64 %extract384.i
  %vload968.i = load double addrspace(3)* %239, align 8
  br label %postload1638.i

postload1638.i:                                   ; preds = %preload1637.i, %postload2516.i
  %phi1639.i = phi double [ undef, %postload2516.i ], [ %vload968.i, %preload1637.i ]
  %vpack969.i = insertelement <16 x double> undef, double %phi1639.i, i32 0
  %exmask971.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 1
  br i1 %exmask971.i, label %preload1640.i, label %postload1641.i

preload1640.i:                                    ; preds = %postload1638.i
  %.sum2816.i = add i64 %extract384.i, 1
  %240 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2816.i
  %vload972.i = load double addrspace(3)* %240, align 8
  br label %postload1641.i

postload1641.i:                                   ; preds = %preload1640.i, %postload1638.i
  %phi1642.i = phi double [ undef, %postload1638.i ], [ %vload972.i, %preload1640.i ]
  %vpack973.i = insertelement <16 x double> %vpack969.i, double %phi1642.i, i32 1
  %exmask975.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 2
  br i1 %exmask975.i, label %preload1643.i, label %postload1644.i

preload1643.i:                                    ; preds = %postload1641.i
  %.sum2815.i = add i64 %extract384.i, 2
  %241 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2815.i
  %vload976.i = load double addrspace(3)* %241, align 8
  br label %postload1644.i

postload1644.i:                                   ; preds = %preload1643.i, %postload1641.i
  %phi1645.i = phi double [ undef, %postload1641.i ], [ %vload976.i, %preload1643.i ]
  %vpack977.i = insertelement <16 x double> %vpack973.i, double %phi1645.i, i32 2
  %exmask979.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 3
  br i1 %exmask979.i, label %preload2631.i, label %postload2632.i

preload2631.i:                                    ; preds = %postload1644.i
  %.sum2814.i = add i64 %extract384.i, 3
  %242 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2814.i
  %vload980.i = load double addrspace(3)* %242, align 8
  br label %postload2632.i

postload2632.i:                                   ; preds = %preload2631.i, %postload1644.i
  %phi2633.i = phi double [ undef, %postload1644.i ], [ %vload980.i, %preload2631.i ]
  %vpack981.i = insertelement <16 x double> %vpack977.i, double %phi2633.i, i32 3
  %exmask983.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 4
  br i1 %exmask983.i, label %preload2634.i, label %postload2635.i

preload2634.i:                                    ; preds = %postload2632.i
  %.sum2813.i = add i64 %extract384.i, 4
  %243 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2813.i
  %vload984.i = load double addrspace(3)* %243, align 8
  br label %postload2635.i

postload2635.i:                                   ; preds = %preload2634.i, %postload2632.i
  %phi2636.i = phi double [ undef, %postload2632.i ], [ %vload984.i, %preload2634.i ]
  %vpack985.i = insertelement <16 x double> %vpack981.i, double %phi2636.i, i32 4
  %exmask987.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 5
  br i1 %exmask987.i, label %preload2637.i, label %postload2638.i

preload2637.i:                                    ; preds = %postload2635.i
  %.sum2812.i = add i64 %extract384.i, 5
  %244 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2812.i
  %vload988.i = load double addrspace(3)* %244, align 8
  br label %postload2638.i

postload2638.i:                                   ; preds = %preload2637.i, %postload2635.i
  %phi2639.i = phi double [ undef, %postload2635.i ], [ %vload988.i, %preload2637.i ]
  %vpack989.i = insertelement <16 x double> %vpack985.i, double %phi2639.i, i32 5
  %exmask991.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 6
  br i1 %exmask991.i, label %preload1764.i, label %postload1765.i

preload1764.i:                                    ; preds = %postload2638.i
  %.sum2811.i = add i64 %extract384.i, 6
  %245 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2811.i
  %vload992.i = load double addrspace(3)* %245, align 8
  br label %postload1765.i

postload1765.i:                                   ; preds = %preload1764.i, %postload2638.i
  %phi1766.i = phi double [ undef, %postload2638.i ], [ %vload992.i, %preload1764.i ]
  %vpack993.i = insertelement <16 x double> %vpack989.i, double %phi1766.i, i32 6
  %exmask995.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 7
  br i1 %exmask995.i, label %preload1767.i, label %postload1768.i

preload1767.i:                                    ; preds = %postload1765.i
  %.sum2810.i = add i64 %extract384.i, 7
  %246 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2810.i
  %vload996.i = load double addrspace(3)* %246, align 8
  br label %postload1768.i

postload1768.i:                                   ; preds = %preload1767.i, %postload1765.i
  %phi1769.i = phi double [ undef, %postload1765.i ], [ %vload996.i, %preload1767.i ]
  %vpack997.i = insertelement <16 x double> %vpack993.i, double %phi1769.i, i32 7
  %exmask999.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 8
  br i1 %exmask999.i, label %preload1770.i, label %postload1771.i

preload1770.i:                                    ; preds = %postload1768.i
  %.sum2809.i = add i64 %extract384.i, 8
  %247 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2809.i
  %vload1000.i = load double addrspace(3)* %247, align 8
  br label %postload1771.i

postload1771.i:                                   ; preds = %preload1770.i, %postload1768.i
  %phi1772.i = phi double [ undef, %postload1768.i ], [ %vload1000.i, %preload1770.i ]
  %vpack1001.i = insertelement <16 x double> %vpack997.i, double %phi1772.i, i32 8
  %exmask1003.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 9
  br i1 %exmask1003.i, label %preload1773.i, label %postload1774.i

preload1773.i:                                    ; preds = %postload1771.i
  %.sum2808.i = add i64 %extract384.i, 9
  %248 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2808.i
  %vload1004.i = load double addrspace(3)* %248, align 8
  br label %postload1774.i

postload1774.i:                                   ; preds = %preload1773.i, %postload1771.i
  %phi1775.i = phi double [ undef, %postload1771.i ], [ %vload1004.i, %preload1773.i ]
  %vpack1005.i = insertelement <16 x double> %vpack1001.i, double %phi1775.i, i32 9
  %exmask1007.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 10
  br i1 %exmask1007.i, label %preload1668.i, label %postload1669.i

preload1668.i:                                    ; preds = %postload1774.i
  %.sum2807.i = add i64 %extract384.i, 10
  %249 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2807.i
  %vload1008.i = load double addrspace(3)* %249, align 8
  br label %postload1669.i

postload1669.i:                                   ; preds = %preload1668.i, %postload1774.i
  %phi1670.i = phi double [ undef, %postload1774.i ], [ %vload1008.i, %preload1668.i ]
  %vpack1009.i = insertelement <16 x double> %vpack1005.i, double %phi1670.i, i32 10
  %exmask1011.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 11
  br i1 %exmask1011.i, label %preload1671.i, label %postload1672.i

preload1671.i:                                    ; preds = %postload1669.i
  %.sum2806.i = add i64 %extract384.i, 11
  %250 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2806.i
  %vload1012.i = load double addrspace(3)* %250, align 8
  br label %postload1672.i

postload1672.i:                                   ; preds = %preload1671.i, %postload1669.i
  %phi1673.i = phi double [ undef, %postload1669.i ], [ %vload1012.i, %preload1671.i ]
  %vpack1013.i = insertelement <16 x double> %vpack1009.i, double %phi1673.i, i32 11
  %exmask1015.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 12
  br i1 %exmask1015.i, label %preload1674.i, label %postload1675.i

preload1674.i:                                    ; preds = %postload1672.i
  %.sum2805.i = add i64 %extract384.i, 12
  %251 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2805.i
  %vload1016.i = load double addrspace(3)* %251, align 8
  br label %postload1675.i

postload1675.i:                                   ; preds = %preload1674.i, %postload1672.i
  %phi1676.i = phi double [ undef, %postload1672.i ], [ %vload1016.i, %preload1674.i ]
  %vpack1017.i = insertelement <16 x double> %vpack1013.i, double %phi1676.i, i32 12
  %exmask1019.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 13
  br i1 %exmask1019.i, label %preload2373.i, label %postload2374.i

preload2373.i:                                    ; preds = %postload1675.i
  %.sum2804.i = add i64 %extract384.i, 13
  %252 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2804.i
  %vload1020.i = load double addrspace(3)* %252, align 8
  br label %postload2374.i

postload2374.i:                                   ; preds = %preload2373.i, %postload1675.i
  %phi2375.i = phi double [ undef, %postload1675.i ], [ %vload1020.i, %preload2373.i ]
  %vpack1021.i = insertelement <16 x double> %vpack1017.i, double %phi2375.i, i32 13
  %exmask1023.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 14
  br i1 %exmask1023.i, label %preload2376.i, label %postload2377.i

preload2376.i:                                    ; preds = %postload2374.i
  %.sum2803.i = add i64 %extract384.i, 14
  %253 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2803.i
  %vload1024.i = load double addrspace(3)* %253, align 8
  br label %postload2377.i

postload2377.i:                                   ; preds = %preload2376.i, %postload2374.i
  %phi2378.i = phi double [ undef, %postload2374.i ], [ %vload1024.i, %preload2376.i ]
  %vpack1025.i = insertelement <16 x double> %vpack1021.i, double %phi2378.i, i32 14
  %exmask1027.i = extractelement <16 x i1> %if.end46_to_if.then49381.i, i32 15
  br i1 %exmask1027.i, label %preload2379.i, label %postload2380.i

preload2379.i:                                    ; preds = %postload2377.i
  %.sum2802.i = add i64 %extract384.i, 15
  %254 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2802.i
  %vload1028.i = load double addrspace(3)* %254, align 8
  br label %postload2380.i

postload2380.i:                                   ; preds = %preload2379.i, %postload2377.i
  %phi2381.i = phi double [ undef, %postload2377.i ], [ %vload1028.i, %preload2379.i ]
  %vpack1029.i = insertelement <16 x double> %vpack1025.i, double %phi2381.i, i32 15
  br i1 %exmask967.i, label %preload2382.i, label %postload2383.i

preload2382.i:                                    ; preds = %postload2380.i
  %"&(pSB[currWI].offset)3914.i" = add nuw i64 %CurrSBIndex..6.i, 80
  %"&pSB[currWI].offset3915.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3914.i"
  %CastToValueType3916.i = bitcast i8* %"&pSB[currWI].offset3915.i" to double addrspace(3)**
  %loadedValue3917.i = load double addrspace(3)** %CastToValueType3916.i, align 8
  %vload1033.i = load double addrspace(3)* %loadedValue3917.i, align 8
  br label %postload2383.i

postload2383.i:                                   ; preds = %preload2382.i, %postload2380.i
  %phi2384.i = phi double [ undef, %postload2380.i ], [ %vload1033.i, %preload2382.i ]
  %vpack1034.i = insertelement <16 x double> undef, double %phi2384.i, i32 0
  br i1 %exmask971.i, label %preload1893.i, label %postload1894.i

preload1893.i:                                    ; preds = %postload2383.i
  %"&(pSB[currWI].offset)3360.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3361.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3360.i"
  %CastToValueType3362.i = bitcast i8* %"&pSB[currWI].offset3361.i" to i64*
  %loadedValue3363.i = load i64* %CastToValueType3362.i, align 8
  %.sum2801.i = add i64 %loadedValue3363.i, 1
  %255 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2801.i
  %vload1037.i = load double addrspace(3)* %255, align 8
  br label %postload1894.i

postload1894.i:                                   ; preds = %preload1893.i, %postload2383.i
  %phi1895.i = phi double [ undef, %postload2383.i ], [ %vload1037.i, %preload1893.i ]
  %vpack1038.i = insertelement <16 x double> %vpack1034.i, double %phi1895.i, i32 1
  br i1 %exmask975.i, label %preload1896.i, label %postload1897.i

preload1896.i:                                    ; preds = %postload1894.i
  %"&(pSB[currWI].offset)3365.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3366.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3365.i"
  %CastToValueType3367.i = bitcast i8* %"&pSB[currWI].offset3366.i" to i64*
  %loadedValue3368.i = load i64* %CastToValueType3367.i, align 8
  %.sum2800.i = add i64 %loadedValue3368.i, 2
  %256 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2800.i
  %vload1041.i = load double addrspace(3)* %256, align 8
  br label %postload1897.i

postload1897.i:                                   ; preds = %preload1896.i, %postload1894.i
  %phi1898.i = phi double [ undef, %postload1894.i ], [ %vload1041.i, %preload1896.i ]
  %vpack1042.i = insertelement <16 x double> %vpack1038.i, double %phi1898.i, i32 2
  br i1 %exmask979.i, label %preload1899.i, label %postload1900.i

preload1899.i:                                    ; preds = %postload1897.i
  %"&(pSB[currWI].offset)3370.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3371.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3370.i"
  %CastToValueType3372.i = bitcast i8* %"&pSB[currWI].offset3371.i" to i64*
  %loadedValue3373.i = load i64* %CastToValueType3372.i, align 8
  %.sum2799.i = add i64 %loadedValue3373.i, 3
  %257 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2799.i
  %vload1045.i = load double addrspace(3)* %257, align 8
  br label %postload1900.i

postload1900.i:                                   ; preds = %preload1899.i, %postload1897.i
  %phi1901.i = phi double [ undef, %postload1897.i ], [ %vload1045.i, %preload1899.i ]
  %vpack1046.i = insertelement <16 x double> %vpack1042.i, double %phi1901.i, i32 3
  br i1 %exmask983.i, label %preload1719.i, label %postload1720.i

preload1719.i:                                    ; preds = %postload1900.i
  %"&(pSB[currWI].offset)3375.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3376.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3375.i"
  %CastToValueType3377.i = bitcast i8* %"&pSB[currWI].offset3376.i" to i64*
  %loadedValue3378.i = load i64* %CastToValueType3377.i, align 8
  %.sum2798.i = add i64 %loadedValue3378.i, 4
  %258 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2798.i
  %vload1049.i = load double addrspace(3)* %258, align 8
  br label %postload1720.i

postload1720.i:                                   ; preds = %preload1719.i, %postload1900.i
  %phi1721.i = phi double [ undef, %postload1900.i ], [ %vload1049.i, %preload1719.i ]
  %vpack1050.i = insertelement <16 x double> %vpack1046.i, double %phi1721.i, i32 4
  br i1 %exmask987.i, label %preload1722.i, label %postload1723.i

preload1722.i:                                    ; preds = %postload1720.i
  %"&(pSB[currWI].offset)3380.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3381.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3380.i"
  %CastToValueType3382.i = bitcast i8* %"&pSB[currWI].offset3381.i" to i64*
  %loadedValue3383.i = load i64* %CastToValueType3382.i, align 8
  %.sum2797.i = add i64 %loadedValue3383.i, 5
  %259 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2797.i
  %vload1053.i = load double addrspace(3)* %259, align 8
  br label %postload1723.i

postload1723.i:                                   ; preds = %preload1722.i, %postload1720.i
  %phi1724.i = phi double [ undef, %postload1720.i ], [ %vload1053.i, %preload1722.i ]
  %vpack1054.i = insertelement <16 x double> %vpack1050.i, double %phi1724.i, i32 5
  br i1 %exmask991.i, label %preload1725.i, label %postload1726.i

preload1725.i:                                    ; preds = %postload1723.i
  %"&(pSB[currWI].offset)3385.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3386.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3385.i"
  %CastToValueType3387.i = bitcast i8* %"&pSB[currWI].offset3386.i" to i64*
  %loadedValue3388.i = load i64* %CastToValueType3387.i, align 8
  %.sum2796.i = add i64 %loadedValue3388.i, 6
  %260 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2796.i
  %vload1057.i = load double addrspace(3)* %260, align 8
  br label %postload1726.i

postload1726.i:                                   ; preds = %preload1725.i, %postload1723.i
  %phi1727.i = phi double [ undef, %postload1723.i ], [ %vload1057.i, %preload1725.i ]
  %vpack1058.i = insertelement <16 x double> %vpack1054.i, double %phi1727.i, i32 6
  br i1 %exmask995.i, label %preload1728.i, label %postload1729.i

preload1728.i:                                    ; preds = %postload1726.i
  %"&(pSB[currWI].offset)3390.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3391.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3390.i"
  %CastToValueType3392.i = bitcast i8* %"&pSB[currWI].offset3391.i" to i64*
  %loadedValue3393.i = load i64* %CastToValueType3392.i, align 8
  %.sum2795.i = add i64 %loadedValue3393.i, 7
  %261 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2795.i
  %vload1061.i = load double addrspace(3)* %261, align 8
  br label %postload1729.i

postload1729.i:                                   ; preds = %preload1728.i, %postload1726.i
  %phi1730.i = phi double [ undef, %postload1726.i ], [ %vload1061.i, %preload1728.i ]
  %vpack1062.i = insertelement <16 x double> %vpack1058.i, double %phi1730.i, i32 7
  br i1 %exmask999.i, label %preload1821.i, label %postload1822.i

preload1821.i:                                    ; preds = %postload1729.i
  %"&(pSB[currWI].offset)3395.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3396.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3395.i"
  %CastToValueType3397.i = bitcast i8* %"&pSB[currWI].offset3396.i" to i64*
  %loadedValue3398.i = load i64* %CastToValueType3397.i, align 8
  %.sum2794.i = add i64 %loadedValue3398.i, 8
  %262 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2794.i
  %vload1065.i = load double addrspace(3)* %262, align 8
  br label %postload1822.i

postload1822.i:                                   ; preds = %preload1821.i, %postload1729.i
  %phi1823.i = phi double [ undef, %postload1729.i ], [ %vload1065.i, %preload1821.i ]
  %vpack1066.i = insertelement <16 x double> %vpack1062.i, double %phi1823.i, i32 8
  br i1 %exmask1003.i, label %preload1824.i, label %postload1825.i

preload1824.i:                                    ; preds = %postload1822.i
  %"&(pSB[currWI].offset)3400.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3401.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3400.i"
  %CastToValueType3402.i = bitcast i8* %"&pSB[currWI].offset3401.i" to i64*
  %loadedValue3403.i = load i64* %CastToValueType3402.i, align 8
  %.sum2793.i = add i64 %loadedValue3403.i, 9
  %263 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2793.i
  %vload1069.i = load double addrspace(3)* %263, align 8
  br label %postload1825.i

postload1825.i:                                   ; preds = %preload1824.i, %postload1822.i
  %phi1826.i = phi double [ undef, %postload1822.i ], [ %vload1069.i, %preload1824.i ]
  %vpack1070.i = insertelement <16 x double> %vpack1066.i, double %phi1826.i, i32 9
  br i1 %exmask1007.i, label %preload1827.i, label %postload1828.i

preload1827.i:                                    ; preds = %postload1825.i
  %"&(pSB[currWI].offset)3405.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3406.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3405.i"
  %CastToValueType3407.i = bitcast i8* %"&pSB[currWI].offset3406.i" to i64*
  %loadedValue3408.i = load i64* %CastToValueType3407.i, align 8
  %.sum2792.i = add i64 %loadedValue3408.i, 10
  %264 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2792.i
  %vload1073.i = load double addrspace(3)* %264, align 8
  br label %postload1828.i

postload1828.i:                                   ; preds = %preload1827.i, %postload1825.i
  %phi1829.i = phi double [ undef, %postload1825.i ], [ %vload1073.i, %preload1827.i ]
  %vpack1074.i = insertelement <16 x double> %vpack1070.i, double %phi1829.i, i32 10
  br i1 %exmask1011.i, label %preload1830.i, label %postload1831.i

preload1830.i:                                    ; preds = %postload1828.i
  %"&(pSB[currWI].offset)3410.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3411.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3410.i"
  %CastToValueType3412.i = bitcast i8* %"&pSB[currWI].offset3411.i" to i64*
  %loadedValue3413.i = load i64* %CastToValueType3412.i, align 8
  %.sum2791.i = add i64 %loadedValue3413.i, 11
  %265 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2791.i
  %vload1077.i = load double addrspace(3)* %265, align 8
  br label %postload1831.i

postload1831.i:                                   ; preds = %preload1830.i, %postload1828.i
  %phi1832.i = phi double [ undef, %postload1828.i ], [ %vload1077.i, %preload1830.i ]
  %vpack1078.i = insertelement <16 x double> %vpack1074.i, double %phi1832.i, i32 11
  br i1 %exmask1015.i, label %preload1743.i, label %postload1744.i

preload1743.i:                                    ; preds = %postload1831.i
  %"&(pSB[currWI].offset)3415.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3416.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3415.i"
  %CastToValueType3417.i = bitcast i8* %"&pSB[currWI].offset3416.i" to i64*
  %loadedValue3418.i = load i64* %CastToValueType3417.i, align 8
  %.sum2790.i = add i64 %loadedValue3418.i, 12
  %266 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2790.i
  %vload1081.i = load double addrspace(3)* %266, align 8
  br label %postload1744.i

postload1744.i:                                   ; preds = %preload1743.i, %postload1831.i
  %phi1745.i = phi double [ undef, %postload1831.i ], [ %vload1081.i, %preload1743.i ]
  %vpack1082.i = insertelement <16 x double> %vpack1078.i, double %phi1745.i, i32 12
  br i1 %exmask1019.i, label %preload1746.i, label %postload1747.i

preload1746.i:                                    ; preds = %postload1744.i
  %"&(pSB[currWI].offset)3420.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3421.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3420.i"
  %CastToValueType3422.i = bitcast i8* %"&pSB[currWI].offset3421.i" to i64*
  %loadedValue3423.i = load i64* %CastToValueType3422.i, align 8
  %.sum2789.i = add i64 %loadedValue3423.i, 13
  %267 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2789.i
  %vload1085.i = load double addrspace(3)* %267, align 8
  br label %postload1747.i

postload1747.i:                                   ; preds = %preload1746.i, %postload1744.i
  %phi1748.i = phi double [ undef, %postload1744.i ], [ %vload1085.i, %preload1746.i ]
  %vpack1086.i = insertelement <16 x double> %vpack1082.i, double %phi1748.i, i32 13
  br i1 %exmask1023.i, label %preload1749.i, label %postload1750.i

preload1749.i:                                    ; preds = %postload1747.i
  %"&(pSB[currWI].offset)3425.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3426.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3425.i"
  %CastToValueType3427.i = bitcast i8* %"&pSB[currWI].offset3426.i" to i64*
  %loadedValue3428.i = load i64* %CastToValueType3427.i, align 8
  %.sum2788.i = add i64 %loadedValue3428.i, 14
  %268 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2788.i
  %vload1089.i = load double addrspace(3)* %268, align 8
  br label %postload1750.i

postload1750.i:                                   ; preds = %preload1749.i, %postload1747.i
  %phi1751.i = phi double [ undef, %postload1747.i ], [ %vload1089.i, %preload1749.i ]
  %vpack1090.i = insertelement <16 x double> %vpack1086.i, double %phi1751.i, i32 14
  br i1 %exmask1027.i, label %preload1926.i, label %postload1927.i

preload1926.i:                                    ; preds = %postload1750.i
  %"&(pSB[currWI].offset)3430.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3431.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3430.i"
  %CastToValueType3432.i = bitcast i8* %"&pSB[currWI].offset3431.i" to i64*
  %loadedValue3433.i = load i64* %CastToValueType3432.i, align 8
  %.sum2787.i = add i64 %loadedValue3433.i, 15
  %269 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2787.i
  %vload1093.i = load double addrspace(3)* %269, align 8
  br label %postload1927.i

postload1927.i:                                   ; preds = %preload1926.i, %postload1750.i
  %phi1928.i = phi double [ undef, %postload1750.i ], [ %vload1093.i, %preload1926.i ]
  %vpack1094.i = insertelement <16 x double> %vpack1090.i, double %phi1928.i, i32 15
  %add55402.i = fadd <16 x double> %vpack1094.i, %vpack1029.i
  br i1 %exmask967.i, label %preload1929.i, label %postload1930.i

preload1929.i:                                    ; preds = %postload1927.i
  %exData1098.i = extractelement <16 x double> %add55402.i, i32 0
  %"&(pSB[currWI].offset)3919.i" = add nuw i64 %CurrSBIndex..6.i, 80
  %"&pSB[currWI].offset3920.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3919.i"
  %CastToValueType3921.i = bitcast i8* %"&pSB[currWI].offset3920.i" to double addrspace(3)**
  %loadedValue3922.i = load double addrspace(3)** %CastToValueType3921.i, align 8
  store double %exData1098.i, double addrspace(3)* %loadedValue3922.i, align 8
  br label %postload1930.i

postload1930.i:                                   ; preds = %preload1929.i, %postload1927.i
  br i1 %exmask971.i, label %preload1932.i, label %postload1933.i

preload1932.i:                                    ; preds = %postload1930.i
  %"&(pSB[currWI].offset)3435.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3436.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3435.i"
  %CastToValueType3437.i = bitcast i8* %"&pSB[currWI].offset3436.i" to i64*
  %loadedValue3438.i = load i64* %CastToValueType3437.i, align 8
  %.sum2786.i = add i64 %loadedValue3438.i, 1
  %270 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2786.i
  %exData1101.i = extractelement <16 x double> %add55402.i, i32 1
  store double %exData1101.i, double addrspace(3)* %270, align 8
  br label %postload1933.i

postload1933.i:                                   ; preds = %preload1932.i, %postload1930.i
  br i1 %exmask975.i, label %preload1935.i, label %postload1936.i

preload1935.i:                                    ; preds = %postload1933.i
  %"&(pSB[currWI].offset)3440.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3441.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3440.i"
  %CastToValueType3442.i = bitcast i8* %"&pSB[currWI].offset3441.i" to i64*
  %loadedValue3443.i = load i64* %CastToValueType3442.i, align 8
  %.sum2785.i = add i64 %loadedValue3443.i, 2
  %271 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2785.i
  %exData1104.i = extractelement <16 x double> %add55402.i, i32 2
  store double %exData1104.i, double addrspace(3)* %271, align 8
  br label %postload1936.i

postload1936.i:                                   ; preds = %preload1935.i, %postload1933.i
  br i1 %exmask979.i, label %preload1600.i, label %postload1601.i

preload1600.i:                                    ; preds = %postload1936.i
  %"&(pSB[currWI].offset)3445.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3446.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3445.i"
  %CastToValueType3447.i = bitcast i8* %"&pSB[currWI].offset3446.i" to i64*
  %loadedValue3448.i = load i64* %CastToValueType3447.i, align 8
  %.sum2784.i = add i64 %loadedValue3448.i, 3
  %272 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2784.i
  %exData1107.i = extractelement <16 x double> %add55402.i, i32 3
  store double %exData1107.i, double addrspace(3)* %272, align 8
  br label %postload1601.i

postload1601.i:                                   ; preds = %preload1600.i, %postload1936.i
  br i1 %exmask983.i, label %preload1603.i, label %postload1604.i

preload1603.i:                                    ; preds = %postload1601.i
  %"&(pSB[currWI].offset)3450.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3451.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3450.i"
  %CastToValueType3452.i = bitcast i8* %"&pSB[currWI].offset3451.i" to i64*
  %loadedValue3453.i = load i64* %CastToValueType3452.i, align 8
  %.sum2783.i = add i64 %loadedValue3453.i, 4
  %273 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2783.i
  %exData1110.i = extractelement <16 x double> %add55402.i, i32 4
  store double %exData1110.i, double addrspace(3)* %273, align 8
  br label %postload1604.i

postload1604.i:                                   ; preds = %preload1603.i, %postload1601.i
  br i1 %exmask987.i, label %preload1606.i, label %postload1607.i

preload1606.i:                                    ; preds = %postload1604.i
  %"&(pSB[currWI].offset)3455.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3456.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3455.i"
  %CastToValueType3457.i = bitcast i8* %"&pSB[currWI].offset3456.i" to i64*
  %loadedValue3458.i = load i64* %CastToValueType3457.i, align 8
  %.sum2782.i = add i64 %loadedValue3458.i, 5
  %274 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2782.i
  %exData1113.i = extractelement <16 x double> %add55402.i, i32 5
  store double %exData1113.i, double addrspace(3)* %274, align 8
  br label %postload1607.i

postload1607.i:                                   ; preds = %preload1606.i, %postload1604.i
  br i1 %exmask991.i, label %preload1609.i, label %postload1610.i

preload1609.i:                                    ; preds = %postload1607.i
  %"&(pSB[currWI].offset)3460.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3461.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3460.i"
  %CastToValueType3462.i = bitcast i8* %"&pSB[currWI].offset3461.i" to i64*
  %loadedValue3463.i = load i64* %CastToValueType3462.i, align 8
  %.sum2781.i = add i64 %loadedValue3463.i, 6
  %275 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2781.i
  %exData1116.i = extractelement <16 x double> %add55402.i, i32 6
  store double %exData1116.i, double addrspace(3)* %275, align 8
  br label %postload1610.i

postload1610.i:                                   ; preds = %preload1609.i, %postload1607.i
  br i1 %exmask995.i, label %preload1576.i, label %postload1577.i

preload1576.i:                                    ; preds = %postload1610.i
  %"&(pSB[currWI].offset)3465.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3466.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3465.i"
  %CastToValueType3467.i = bitcast i8* %"&pSB[currWI].offset3466.i" to i64*
  %loadedValue3468.i = load i64* %CastToValueType3467.i, align 8
  %.sum2780.i = add i64 %loadedValue3468.i, 7
  %276 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2780.i
  %exData1119.i = extractelement <16 x double> %add55402.i, i32 7
  store double %exData1119.i, double addrspace(3)* %276, align 8
  br label %postload1577.i

postload1577.i:                                   ; preds = %preload1576.i, %postload1610.i
  br i1 %exmask999.i, label %preload1579.i, label %postload1580.i

preload1579.i:                                    ; preds = %postload1577.i
  %"&(pSB[currWI].offset)3470.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3471.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3470.i"
  %CastToValueType3472.i = bitcast i8* %"&pSB[currWI].offset3471.i" to i64*
  %loadedValue3473.i = load i64* %CastToValueType3472.i, align 8
  %.sum2779.i = add i64 %loadedValue3473.i, 8
  %277 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2779.i
  %exData1122.i = extractelement <16 x double> %add55402.i, i32 8
  store double %exData1122.i, double addrspace(3)* %277, align 8
  br label %postload1580.i

postload1580.i:                                   ; preds = %preload1579.i, %postload1577.i
  br i1 %exmask1003.i, label %preload1582.i, label %postload1583.i

preload1582.i:                                    ; preds = %postload1580.i
  %"&(pSB[currWI].offset)3475.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3476.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3475.i"
  %CastToValueType3477.i = bitcast i8* %"&pSB[currWI].offset3476.i" to i64*
  %loadedValue3478.i = load i64* %CastToValueType3477.i, align 8
  %.sum2778.i = add i64 %loadedValue3478.i, 9
  %278 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2778.i
  %exData1125.i = extractelement <16 x double> %add55402.i, i32 9
  store double %exData1125.i, double addrspace(3)* %278, align 8
  br label %postload1583.i

postload1583.i:                                   ; preds = %preload1582.i, %postload1580.i
  br i1 %exmask1007.i, label %preload1585.i, label %postload1586.i

preload1585.i:                                    ; preds = %postload1583.i
  %"&(pSB[currWI].offset)3480.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3481.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3480.i"
  %CastToValueType3482.i = bitcast i8* %"&pSB[currWI].offset3481.i" to i64*
  %loadedValue3483.i = load i64* %CastToValueType3482.i, align 8
  %.sum2777.i = add i64 %loadedValue3483.i, 10
  %279 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2777.i
  %exData1128.i = extractelement <16 x double> %add55402.i, i32 10
  store double %exData1128.i, double addrspace(3)* %279, align 8
  br label %postload1586.i

postload1586.i:                                   ; preds = %preload1585.i, %postload1583.i
  br i1 %exmask1011.i, label %preload1707.i, label %postload1708.i

preload1707.i:                                    ; preds = %postload1586.i
  %"&(pSB[currWI].offset)3485.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3486.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3485.i"
  %CastToValueType3487.i = bitcast i8* %"&pSB[currWI].offset3486.i" to i64*
  %loadedValue3488.i = load i64* %CastToValueType3487.i, align 8
  %.sum2776.i = add i64 %loadedValue3488.i, 11
  %280 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2776.i
  %exData1131.i = extractelement <16 x double> %add55402.i, i32 11
  store double %exData1131.i, double addrspace(3)* %280, align 8
  br label %postload1708.i

postload1708.i:                                   ; preds = %preload1707.i, %postload1586.i
  br i1 %exmask1015.i, label %preload1710.i, label %postload1711.i

preload1710.i:                                    ; preds = %postload1708.i
  %"&(pSB[currWI].offset)3490.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3491.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3490.i"
  %CastToValueType3492.i = bitcast i8* %"&pSB[currWI].offset3491.i" to i64*
  %loadedValue3493.i = load i64* %CastToValueType3492.i, align 8
  %.sum2775.i = add i64 %loadedValue3493.i, 12
  %281 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2775.i
  %exData1134.i = extractelement <16 x double> %add55402.i, i32 12
  store double %exData1134.i, double addrspace(3)* %281, align 8
  br label %postload1711.i

postload1711.i:                                   ; preds = %preload1710.i, %postload1708.i
  br i1 %exmask1019.i, label %preload1713.i, label %postload1714.i

preload1713.i:                                    ; preds = %postload1711.i
  %"&(pSB[currWI].offset)3495.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3496.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3495.i"
  %CastToValueType3497.i = bitcast i8* %"&pSB[currWI].offset3496.i" to i64*
  %loadedValue3498.i = load i64* %CastToValueType3497.i, align 8
  %.sum2774.i = add i64 %loadedValue3498.i, 13
  %282 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2774.i
  %exData1137.i = extractelement <16 x double> %add55402.i, i32 13
  store double %exData1137.i, double addrspace(3)* %282, align 8
  br label %postload1714.i

postload1714.i:                                   ; preds = %preload1713.i, %postload1711.i
  br i1 %exmask1023.i, label %preload1716.i, label %postload1717.i

preload1716.i:                                    ; preds = %postload1714.i
  %"&(pSB[currWI].offset)3500.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3501.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3500.i"
  %CastToValueType3502.i = bitcast i8* %"&pSB[currWI].offset3501.i" to i64*
  %loadedValue3503.i = load i64* %CastToValueType3502.i, align 8
  %.sum2773.i = add i64 %loadedValue3503.i, 14
  %283 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2773.i
  %exData1140.i = extractelement <16 x double> %add55402.i, i32 14
  store double %exData1140.i, double addrspace(3)* %283, align 8
  br label %postload1717.i

postload1717.i:                                   ; preds = %preload1716.i, %postload1714.i
  br i1 %exmask1027.i, label %preload1947.i, label %postload1717.i.if.end56.i_crit_edge

postload1717.i.if.end56.i_crit_edge:              ; preds = %postload1717.i
  br label %if.end56.i

preload1947.i:                                    ; preds = %postload1717.i
  %"&(pSB[currWI].offset)3505.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset3506.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3505.i"
  %CastToValueType3507.i = bitcast i8* %"&pSB[currWI].offset3506.i" to i64*
  %loadedValue3508.i = load i64* %CastToValueType3507.i, align 8
  %.sum2772.i = add i64 %loadedValue3508.i, 15
  %284 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2772.i
  %exData1143.i = extractelement <16 x double> %add55402.i, i32 15
  store double %exData1143.i, double addrspace(3)* %284, align 8
  br label %if.end56.i

if.end56.i:                                       ; preds = %postload1717.i.if.end56.i_crit_edge, %preload1947.i
  %"&(pSB[currWI].offset)4007.i" = add nuw i64 %CurrSBIndex..6.i, 98
  %"&pSB[currWI].offset4008.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4007.i"
  %CastToValueType4009.i = bitcast i8* %"&pSB[currWI].offset4008.i" to i1*
  %loadedValue4010.i = load i1* %CastToValueType4009.i, align 1
  br i1 %loadedValue4010.i, label %preload2517.i, label %if.end56.i.postload2518.i_crit_edge

if.end56.i.postload2518.i_crit_edge:              ; preds = %if.end56.i
  br label %postload2518.i

preload2517.i:                                    ; preds = %if.end56.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..6.i, %31
  br i1 %check.WI.iter.i, label %thenBB.i, label %preload2517.i.postload2518.i_crit_edge

preload2517.i.postload2518.i_crit_edge:           ; preds = %preload2517.i
  br label %postload2518.i

thenBB.i:                                         ; preds = %preload2517.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..6.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..6.i, 256
  switch i32 %currBarrier.6.i, label %thenBB.i.postload2516.i_crit_edge [
    i32 13, label %thenBB.i.postload2514.i_crit_edge
    i32 8, label %thenBB.i.postload2512.i_crit_edge
    i32 14, label %SyncBB.outer.i
  ]

thenBB.i.postload2512.i_crit_edge:                ; preds = %thenBB.i
  br label %postload2512.i

thenBB.i.postload2514.i_crit_edge:                ; preds = %thenBB.i
  br label %postload2514.i

thenBB.i.postload2516.i_crit_edge:                ; preds = %thenBB.i
  br label %postload2516.i

postload2518.i:                                   ; preds = %thenBB4603.i.postload2518.i_crit_edge, %thenBB4595.i.postload2518.i_crit_edge, %thenBB4572.i.postload2518.i_crit_edge, %preload2517.i.postload2518.i_crit_edge, %if.end56.i.postload2518.i_crit_edge
  %CurrWI..8.i = phi i64 [ %CurrWI..6.i, %if.end56.i.postload2518.i_crit_edge ], [ 0, %preload2517.i.postload2518.i_crit_edge ], [ %"CurrWI++4576.i", %thenBB4572.i.postload2518.i_crit_edge ], [ %"CurrWI++4599.i", %thenBB4595.i.postload2518.i_crit_edge ], [ %"CurrWI++4607.i", %thenBB4603.i.postload2518.i_crit_edge ]
  %CurrSBIndex..8.i = phi i64 [ %CurrSBIndex..6.i, %if.end56.i.postload2518.i_crit_edge ], [ 0, %preload2517.i.postload2518.i_crit_edge ], [ %"loadedCurrSB+Stride4578.i", %thenBB4572.i.postload2518.i_crit_edge ], [ %"loadedCurrSB+Stride4601.i", %thenBB4595.i.postload2518.i_crit_edge ], [ %"loadedCurrSB+Stride4609.i", %thenBB4603.i.postload2518.i_crit_edge ]
  %currBarrier.8.i = phi i32 [ %currBarrier.6.i, %if.end56.i.postload2518.i_crit_edge ], [ 2, %preload2517.i.postload2518.i_crit_edge ], [ %currBarrier.8.i, %thenBB4572.i.postload2518.i_crit_edge ], [ %currBarrier.10.i, %thenBB4595.i.postload2518.i_crit_edge ], [ %currBarrier.12.i, %thenBB4603.i.postload2518.i_crit_edge ]
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
  br i1 %exmask1146.i, label %preload1950.i, label %postload1951.i

preload1950.i:                                    ; preds = %postload2518.i
  %286 = getelementptr inbounds [128 x double] addrspace(3)* %35, i64 0, i64 %extract407.i
  %vload1147.i = load double addrspace(3)* %286, align 8
  br label %postload1951.i

postload1951.i:                                   ; preds = %preload1950.i, %postload2518.i
  %phi1952.i = phi double [ undef, %postload2518.i ], [ %vload1147.i, %preload1950.i ]
  %vpack1148.i = insertelement <16 x double> undef, double %phi1952.i, i32 0
  %exmask1150.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 1
  br i1 %exmask1150.i, label %preload1953.i, label %postload1954.i

preload1953.i:                                    ; preds = %postload1951.i
  %.sum2771.i = add i64 %extract407.i, 1
  %287 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2771.i
  %vload1151.i = load double addrspace(3)* %287, align 8
  br label %postload1954.i

postload1954.i:                                   ; preds = %preload1953.i, %postload1951.i
  %phi1955.i = phi double [ undef, %postload1951.i ], [ %vload1151.i, %preload1953.i ]
  %vpack1152.i = insertelement <16 x double> %vpack1148.i, double %phi1955.i, i32 1
  %exmask1154.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 2
  br i1 %exmask1154.i, label %preload1956.i, label %postload1957.i

preload1956.i:                                    ; preds = %postload1954.i
  %.sum2770.i = add i64 %extract407.i, 2
  %288 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2770.i
  %vload1155.i = load double addrspace(3)* %288, align 8
  br label %postload1957.i

postload1957.i:                                   ; preds = %preload1956.i, %postload1954.i
  %phi1958.i = phi double [ undef, %postload1954.i ], [ %vload1155.i, %preload1956.i ]
  %vpack1156.i = insertelement <16 x double> %vpack1152.i, double %phi1958.i, i32 2
  %exmask1158.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 3
  br i1 %exmask1158.i, label %preload2601.i, label %postload2602.i

preload2601.i:                                    ; preds = %postload1957.i
  %.sum2769.i = add i64 %extract407.i, 3
  %289 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2769.i
  %vload1159.i = load double addrspace(3)* %289, align 8
  br label %postload2602.i

postload2602.i:                                   ; preds = %preload2601.i, %postload1957.i
  %phi2603.i = phi double [ undef, %postload1957.i ], [ %vload1159.i, %preload2601.i ]
  %vpack1160.i = insertelement <16 x double> %vpack1156.i, double %phi2603.i, i32 3
  %exmask1162.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 4
  br i1 %exmask1162.i, label %preload2604.i, label %postload2605.i

preload2604.i:                                    ; preds = %postload2602.i
  %.sum2768.i = add i64 %extract407.i, 4
  %290 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2768.i
  %vload1163.i = load double addrspace(3)* %290, align 8
  br label %postload2605.i

postload2605.i:                                   ; preds = %preload2604.i, %postload2602.i
  %phi2606.i = phi double [ undef, %postload2602.i ], [ %vload1163.i, %preload2604.i ]
  %vpack1164.i = insertelement <16 x double> %vpack1160.i, double %phi2606.i, i32 4
  %exmask1166.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 5
  br i1 %exmask1166.i, label %preload2607.i, label %postload2608.i

preload2607.i:                                    ; preds = %postload2605.i
  %.sum2767.i = add i64 %extract407.i, 5
  %291 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2767.i
  %vload1167.i = load double addrspace(3)* %291, align 8
  br label %postload2608.i

postload2608.i:                                   ; preds = %preload2607.i, %postload2605.i
  %phi2609.i = phi double [ undef, %postload2605.i ], [ %vload1167.i, %preload2607.i ]
  %vpack1168.i = insertelement <16 x double> %vpack1164.i, double %phi2609.i, i32 5
  %exmask1170.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 6
  br i1 %exmask1170.i, label %preload2067.i, label %postload2068.i

preload2067.i:                                    ; preds = %postload2608.i
  %.sum2766.i = add i64 %extract407.i, 6
  %292 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2766.i
  %vload1171.i = load double addrspace(3)* %292, align 8
  br label %postload2068.i

postload2068.i:                                   ; preds = %preload2067.i, %postload2608.i
  %phi2069.i = phi double [ undef, %postload2608.i ], [ %vload1171.i, %preload2067.i ]
  %vpack1172.i = insertelement <16 x double> %vpack1168.i, double %phi2069.i, i32 6
  %exmask1174.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 7
  br i1 %exmask1174.i, label %preload2070.i, label %postload2071.i

preload2070.i:                                    ; preds = %postload2068.i
  %.sum2765.i = add i64 %extract407.i, 7
  %293 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2765.i
  %vload1175.i = load double addrspace(3)* %293, align 8
  br label %postload2071.i

postload2071.i:                                   ; preds = %preload2070.i, %postload2068.i
  %phi2072.i = phi double [ undef, %postload2068.i ], [ %vload1175.i, %preload2070.i ]
  %vpack1176.i = insertelement <16 x double> %vpack1172.i, double %phi2072.i, i32 7
  %exmask1178.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 8
  br i1 %exmask1178.i, label %preload2073.i, label %postload2074.i

preload2073.i:                                    ; preds = %postload2071.i
  %.sum2764.i = add i64 %extract407.i, 8
  %294 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2764.i
  %vload1179.i = load double addrspace(3)* %294, align 8
  br label %postload2074.i

postload2074.i:                                   ; preds = %preload2073.i, %postload2071.i
  %phi2075.i = phi double [ undef, %postload2071.i ], [ %vload1179.i, %preload2073.i ]
  %vpack1180.i = insertelement <16 x double> %vpack1176.i, double %phi2075.i, i32 8
  %exmask1182.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 9
  br i1 %exmask1182.i, label %preload2076.i, label %postload2077.i

preload2076.i:                                    ; preds = %postload2074.i
  %.sum2763.i = add i64 %extract407.i, 9
  %295 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2763.i
  %vload1183.i = load double addrspace(3)* %295, align 8
  br label %postload2077.i

postload2077.i:                                   ; preds = %preload2076.i, %postload2074.i
  %phi2078.i = phi double [ undef, %postload2074.i ], [ %vload1183.i, %preload2076.i ]
  %vpack1184.i = insertelement <16 x double> %vpack1180.i, double %phi2078.i, i32 9
  %exmask1186.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 10
  br i1 %exmask1186.i, label %preload2397.i, label %postload2398.i

preload2397.i:                                    ; preds = %postload2077.i
  %.sum2762.i = add i64 %extract407.i, 10
  %296 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2762.i
  %vload1187.i = load double addrspace(3)* %296, align 8
  br label %postload2398.i

postload2398.i:                                   ; preds = %preload2397.i, %postload2077.i
  %phi2399.i = phi double [ undef, %postload2077.i ], [ %vload1187.i, %preload2397.i ]
  %vpack1188.i = insertelement <16 x double> %vpack1184.i, double %phi2399.i, i32 10
  %exmask1190.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 11
  br i1 %exmask1190.i, label %preload2400.i, label %postload2401.i

preload2400.i:                                    ; preds = %postload2398.i
  %.sum2761.i = add i64 %extract407.i, 11
  %297 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2761.i
  %vload1191.i = load double addrspace(3)* %297, align 8
  br label %postload2401.i

postload2401.i:                                   ; preds = %preload2400.i, %postload2398.i
  %phi2402.i = phi double [ undef, %postload2398.i ], [ %vload1191.i, %preload2400.i ]
  %vpack1192.i = insertelement <16 x double> %vpack1188.i, double %phi2402.i, i32 11
  %exmask1194.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 12
  br i1 %exmask1194.i, label %preload2403.i, label %postload2404.i

preload2403.i:                                    ; preds = %postload2401.i
  %.sum2760.i = add i64 %extract407.i, 12
  %298 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2760.i
  %vload1195.i = load double addrspace(3)* %298, align 8
  br label %postload2404.i

postload2404.i:                                   ; preds = %preload2403.i, %postload2401.i
  %phi2405.i = phi double [ undef, %postload2401.i ], [ %vload1195.i, %preload2403.i ]
  %vpack1196.i = insertelement <16 x double> %vpack1192.i, double %phi2405.i, i32 12
  %exmask1198.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 13
  br i1 %exmask1198.i, label %preload2406.i, label %postload2407.i

preload2406.i:                                    ; preds = %postload2404.i
  %.sum2759.i = add i64 %extract407.i, 13
  %299 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2759.i
  %vload1199.i = load double addrspace(3)* %299, align 8
  br label %postload2407.i

postload2407.i:                                   ; preds = %preload2406.i, %postload2404.i
  %phi2408.i = phi double [ undef, %postload2404.i ], [ %vload1199.i, %preload2406.i ]
  %vpack1200.i = insertelement <16 x double> %vpack1196.i, double %phi2408.i, i32 13
  %exmask1202.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 14
  br i1 %exmask1202.i, label %preload1857.i, label %postload1858.i

preload1857.i:                                    ; preds = %postload2407.i
  %.sum2758.i = add i64 %extract407.i, 14
  %300 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2758.i
  %vload1203.i = load double addrspace(3)* %300, align 8
  br label %postload1858.i

postload1858.i:                                   ; preds = %preload1857.i, %postload2407.i
  %phi1859.i = phi double [ undef, %postload2407.i ], [ %vload1203.i, %preload1857.i ]
  %vpack1204.i = insertelement <16 x double> %vpack1200.i, double %phi1859.i, i32 14
  %exmask1206.i = extractelement <16 x i1> %if.end56_to_if.then59404.i, i32 15
  br i1 %exmask1206.i, label %preload1860.i, label %postload1861.i

preload1860.i:                                    ; preds = %postload1858.i
  %.sum2757.i = add i64 %extract407.i, 15
  %301 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2757.i
  %vload1207.i = load double addrspace(3)* %301, align 8
  br label %postload1861.i

postload1861.i:                                   ; preds = %preload1860.i, %postload1858.i
  %phi1862.i = phi double [ undef, %postload1858.i ], [ %vload1207.i, %preload1860.i ]
  %vpack1208.i = insertelement <16 x double> %vpack1204.i, double %phi1862.i, i32 15
  br i1 %exmask1146.i, label %preload1863.i, label %postload1864.i

preload1863.i:                                    ; preds = %postload1861.i
  %"&(pSB[currWI].offset)3924.i" = add nuw i64 %CurrSBIndex..8.i, 80
  %"&pSB[currWI].offset3925.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3924.i"
  %CastToValueType3926.i = bitcast i8* %"&pSB[currWI].offset3925.i" to double addrspace(3)**
  %loadedValue3927.i = load double addrspace(3)** %CastToValueType3926.i, align 8
  %vload1212.i = load double addrspace(3)* %loadedValue3927.i, align 8
  br label %postload1864.i

postload1864.i:                                   ; preds = %preload1863.i, %postload1861.i
  %phi1865.i = phi double [ undef, %postload1861.i ], [ %vload1212.i, %preload1863.i ]
  %vpack1213.i = insertelement <16 x double> undef, double %phi1865.i, i32 0
  br i1 %exmask1150.i, label %preload2457.i, label %postload2458.i

preload2457.i:                                    ; preds = %postload1864.i
  %"&(pSB[currWI].offset)3510.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3511.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3510.i"
  %CastToValueType3512.i = bitcast i8* %"&pSB[currWI].offset3511.i" to i64*
  %loadedValue3513.i = load i64* %CastToValueType3512.i, align 8
  %.sum2756.i = add i64 %loadedValue3513.i, 1
  %302 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2756.i
  %vload1216.i = load double addrspace(3)* %302, align 8
  br label %postload2458.i

postload2458.i:                                   ; preds = %preload2457.i, %postload1864.i
  %phi2459.i = phi double [ undef, %postload1864.i ], [ %vload1216.i, %preload2457.i ]
  %vpack1217.i = insertelement <16 x double> %vpack1213.i, double %phi2459.i, i32 1
  br i1 %exmask1154.i, label %preload2460.i, label %postload2461.i

preload2460.i:                                    ; preds = %postload2458.i
  %"&(pSB[currWI].offset)3515.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3516.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3515.i"
  %CastToValueType3517.i = bitcast i8* %"&pSB[currWI].offset3516.i" to i64*
  %loadedValue3518.i = load i64* %CastToValueType3517.i, align 8
  %.sum2755.i = add i64 %loadedValue3518.i, 2
  %303 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2755.i
  %vload1220.i = load double addrspace(3)* %303, align 8
  br label %postload2461.i

postload2461.i:                                   ; preds = %preload2460.i, %postload2458.i
  %phi2462.i = phi double [ undef, %postload2458.i ], [ %vload1220.i, %preload2460.i ]
  %vpack1221.i = insertelement <16 x double> %vpack1217.i, double %phi2462.i, i32 2
  br i1 %exmask1158.i, label %preload2463.i, label %postload2464.i

preload2463.i:                                    ; preds = %postload2461.i
  %"&(pSB[currWI].offset)3520.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3521.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3520.i"
  %CastToValueType3522.i = bitcast i8* %"&pSB[currWI].offset3521.i" to i64*
  %loadedValue3523.i = load i64* %CastToValueType3522.i, align 8
  %.sum2754.i = add i64 %loadedValue3523.i, 3
  %304 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2754.i
  %vload1224.i = load double addrspace(3)* %304, align 8
  br label %postload2464.i

postload2464.i:                                   ; preds = %preload2463.i, %postload2461.i
  %phi2465.i = phi double [ undef, %postload2461.i ], [ %vload1224.i, %preload2463.i ]
  %vpack1225.i = insertelement <16 x double> %vpack1221.i, double %phi2465.i, i32 3
  br i1 %exmask1162.i, label %preload2466.i, label %postload2467.i

preload2466.i:                                    ; preds = %postload2464.i
  %"&(pSB[currWI].offset)3525.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3526.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3525.i"
  %CastToValueType3527.i = bitcast i8* %"&pSB[currWI].offset3526.i" to i64*
  %loadedValue3528.i = load i64* %CastToValueType3527.i, align 8
  %.sum2753.i = add i64 %loadedValue3528.i, 4
  %305 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2753.i
  %vload1228.i = load double addrspace(3)* %305, align 8
  br label %postload2467.i

postload2467.i:                                   ; preds = %preload2466.i, %postload2464.i
  %phi2468.i = phi double [ undef, %postload2464.i ], [ %vload1228.i, %preload2466.i ]
  %vpack1229.i = insertelement <16 x double> %vpack1225.i, double %phi2468.i, i32 4
  br i1 %exmask1166.i, label %preload2268.i, label %postload2269.i

preload2268.i:                                    ; preds = %postload2467.i
  %"&(pSB[currWI].offset)3530.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3531.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3530.i"
  %CastToValueType3532.i = bitcast i8* %"&pSB[currWI].offset3531.i" to i64*
  %loadedValue3533.i = load i64* %CastToValueType3532.i, align 8
  %.sum2752.i = add i64 %loadedValue3533.i, 5
  %306 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2752.i
  %vload1232.i = load double addrspace(3)* %306, align 8
  br label %postload2269.i

postload2269.i:                                   ; preds = %preload2268.i, %postload2467.i
  %phi2270.i = phi double [ undef, %postload2467.i ], [ %vload1232.i, %preload2268.i ]
  %vpack1233.i = insertelement <16 x double> %vpack1229.i, double %phi2270.i, i32 5
  br i1 %exmask1170.i, label %preload2271.i, label %postload2272.i

preload2271.i:                                    ; preds = %postload2269.i
  %"&(pSB[currWI].offset)3535.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3536.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3535.i"
  %CastToValueType3537.i = bitcast i8* %"&pSB[currWI].offset3536.i" to i64*
  %loadedValue3538.i = load i64* %CastToValueType3537.i, align 8
  %.sum2751.i = add i64 %loadedValue3538.i, 6
  %307 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2751.i
  %vload1236.i = load double addrspace(3)* %307, align 8
  br label %postload2272.i

postload2272.i:                                   ; preds = %preload2271.i, %postload2269.i
  %phi2273.i = phi double [ undef, %postload2269.i ], [ %vload1236.i, %preload2271.i ]
  %vpack1237.i = insertelement <16 x double> %vpack1233.i, double %phi2273.i, i32 6
  br i1 %exmask1174.i, label %preload2274.i, label %postload2275.i

preload2274.i:                                    ; preds = %postload2272.i
  %"&(pSB[currWI].offset)3540.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3541.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3540.i"
  %CastToValueType3542.i = bitcast i8* %"&pSB[currWI].offset3541.i" to i64*
  %loadedValue3543.i = load i64* %CastToValueType3542.i, align 8
  %.sum2750.i = add i64 %loadedValue3543.i, 7
  %308 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2750.i
  %vload1240.i = load double addrspace(3)* %308, align 8
  br label %postload2275.i

postload2275.i:                                   ; preds = %preload2274.i, %postload2272.i
  %phi2276.i = phi double [ undef, %postload2272.i ], [ %vload1240.i, %preload2274.i ]
  %vpack1241.i = insertelement <16 x double> %vpack1237.i, double %phi2276.i, i32 7
  br i1 %exmask1178.i, label %preload2589.i, label %postload2590.i

preload2589.i:                                    ; preds = %postload2275.i
  %"&(pSB[currWI].offset)3545.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3546.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3545.i"
  %CastToValueType3547.i = bitcast i8* %"&pSB[currWI].offset3546.i" to i64*
  %loadedValue3548.i = load i64* %CastToValueType3547.i, align 8
  %.sum2749.i = add i64 %loadedValue3548.i, 8
  %309 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2749.i
  %vload1244.i = load double addrspace(3)* %309, align 8
  br label %postload2590.i

postload2590.i:                                   ; preds = %preload2589.i, %postload2275.i
  %phi2591.i = phi double [ undef, %postload2275.i ], [ %vload1244.i, %preload2589.i ]
  %vpack1245.i = insertelement <16 x double> %vpack1241.i, double %phi2591.i, i32 8
  br i1 %exmask1182.i, label %preload2592.i, label %postload2593.i

preload2592.i:                                    ; preds = %postload2590.i
  %"&(pSB[currWI].offset)3550.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3551.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3550.i"
  %CastToValueType3552.i = bitcast i8* %"&pSB[currWI].offset3551.i" to i64*
  %loadedValue3553.i = load i64* %CastToValueType3552.i, align 8
  %.sum2748.i = add i64 %loadedValue3553.i, 9
  %310 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2748.i
  %vload1248.i = load double addrspace(3)* %310, align 8
  br label %postload2593.i

postload2593.i:                                   ; preds = %preload2592.i, %postload2590.i
  %phi2594.i = phi double [ undef, %postload2590.i ], [ %vload1248.i, %preload2592.i ]
  %vpack1249.i = insertelement <16 x double> %vpack1245.i, double %phi2594.i, i32 9
  br i1 %exmask1186.i, label %preload2595.i, label %postload2596.i

preload2595.i:                                    ; preds = %postload2593.i
  %"&(pSB[currWI].offset)3555.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3556.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3555.i"
  %CastToValueType3557.i = bitcast i8* %"&pSB[currWI].offset3556.i" to i64*
  %loadedValue3558.i = load i64* %CastToValueType3557.i, align 8
  %.sum2747.i = add i64 %loadedValue3558.i, 10
  %311 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2747.i
  %vload1252.i = load double addrspace(3)* %311, align 8
  br label %postload2596.i

postload2596.i:                                   ; preds = %preload2595.i, %postload2593.i
  %phi2597.i = phi double [ undef, %postload2593.i ], [ %vload1252.i, %preload2595.i ]
  %vpack1253.i = insertelement <16 x double> %vpack1249.i, double %phi2597.i, i32 10
  br i1 %exmask1190.i, label %preload2598.i, label %postload2599.i

preload2598.i:                                    ; preds = %postload2596.i
  %"&(pSB[currWI].offset)3560.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3561.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3560.i"
  %CastToValueType3562.i = bitcast i8* %"&pSB[currWI].offset3561.i" to i64*
  %loadedValue3563.i = load i64* %CastToValueType3562.i, align 8
  %.sum2746.i = add i64 %loadedValue3563.i, 11
  %312 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2746.i
  %vload1256.i = load double addrspace(3)* %312, align 8
  br label %postload2599.i

postload2599.i:                                   ; preds = %preload2598.i, %postload2596.i
  %phi2600.i = phi double [ undef, %postload2596.i ], [ %vload1256.i, %preload2598.i ]
  %vpack1257.i = insertelement <16 x double> %vpack1253.i, double %phi2600.i, i32 11
  br i1 %exmask1194.i, label %preload2283.i, label %postload2284.i

preload2283.i:                                    ; preds = %postload2599.i
  %"&(pSB[currWI].offset)3565.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3566.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3565.i"
  %CastToValueType3567.i = bitcast i8* %"&pSB[currWI].offset3566.i" to i64*
  %loadedValue3568.i = load i64* %CastToValueType3567.i, align 8
  %.sum2745.i = add i64 %loadedValue3568.i, 12
  %313 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2745.i
  %vload1260.i = load double addrspace(3)* %313, align 8
  br label %postload2284.i

postload2284.i:                                   ; preds = %preload2283.i, %postload2599.i
  %phi2285.i = phi double [ undef, %postload2599.i ], [ %vload1260.i, %preload2283.i ]
  %vpack1261.i = insertelement <16 x double> %vpack1257.i, double %phi2285.i, i32 12
  br i1 %exmask1198.i, label %preload2286.i, label %postload2287.i

preload2286.i:                                    ; preds = %postload2284.i
  %"&(pSB[currWI].offset)3570.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3571.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3570.i"
  %CastToValueType3572.i = bitcast i8* %"&pSB[currWI].offset3571.i" to i64*
  %loadedValue3573.i = load i64* %CastToValueType3572.i, align 8
  %.sum2744.i = add i64 %loadedValue3573.i, 13
  %314 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2744.i
  %vload1264.i = load double addrspace(3)* %314, align 8
  br label %postload2287.i

postload2287.i:                                   ; preds = %preload2286.i, %postload2284.i
  %phi2288.i = phi double [ undef, %postload2284.i ], [ %vload1264.i, %preload2286.i ]
  %vpack1265.i = insertelement <16 x double> %vpack1261.i, double %phi2288.i, i32 13
  br i1 %exmask1202.i, label %preload2289.i, label %postload2290.i

preload2289.i:                                    ; preds = %postload2287.i
  %"&(pSB[currWI].offset)3575.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3576.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3575.i"
  %CastToValueType3577.i = bitcast i8* %"&pSB[currWI].offset3576.i" to i64*
  %loadedValue3578.i = load i64* %CastToValueType3577.i, align 8
  %.sum2743.i = add i64 %loadedValue3578.i, 14
  %315 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2743.i
  %vload1268.i = load double addrspace(3)* %315, align 8
  br label %postload2290.i

postload2290.i:                                   ; preds = %preload2289.i, %postload2287.i
  %phi2291.i = phi double [ undef, %postload2287.i ], [ %vload1268.i, %preload2289.i ]
  %vpack1269.i = insertelement <16 x double> %vpack1265.i, double %phi2291.i, i32 14
  br i1 %exmask1206.i, label %preload2292.i, label %postload2293.i

preload2292.i:                                    ; preds = %postload2290.i
  %"&(pSB[currWI].offset)3580.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3581.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3580.i"
  %CastToValueType3582.i = bitcast i8* %"&pSB[currWI].offset3581.i" to i64*
  %loadedValue3583.i = load i64* %CastToValueType3582.i, align 8
  %.sum2742.i = add i64 %loadedValue3583.i, 15
  %316 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2742.i
  %vload1272.i = load double addrspace(3)* %316, align 8
  br label %postload2293.i

postload2293.i:                                   ; preds = %preload2292.i, %postload2290.i
  %phi2294.i = phi double [ undef, %postload2290.i ], [ %vload1272.i, %preload2292.i ]
  %vpack1273.i = insertelement <16 x double> %vpack1269.i, double %phi2294.i, i32 15
  %add65425.i = fadd <16 x double> %vpack1273.i, %vpack1208.i
  br i1 %exmask1146.i, label %preload1680.i, label %postload1681.i

preload1680.i:                                    ; preds = %postload2293.i
  %exData1277.i = extractelement <16 x double> %add65425.i, i32 0
  %"&(pSB[currWI].offset)3929.i" = add nuw i64 %CurrSBIndex..8.i, 80
  %"&pSB[currWI].offset3930.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3929.i"
  %CastToValueType3931.i = bitcast i8* %"&pSB[currWI].offset3930.i" to double addrspace(3)**
  %loadedValue3932.i = load double addrspace(3)** %CastToValueType3931.i, align 8
  store double %exData1277.i, double addrspace(3)* %loadedValue3932.i, align 8
  br label %postload1681.i

postload1681.i:                                   ; preds = %preload1680.i, %postload2293.i
  br i1 %exmask1150.i, label %preload1683.i, label %postload1684.i

preload1683.i:                                    ; preds = %postload1681.i
  %"&(pSB[currWI].offset)3585.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3586.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3585.i"
  %CastToValueType3587.i = bitcast i8* %"&pSB[currWI].offset3586.i" to i64*
  %loadedValue3588.i = load i64* %CastToValueType3587.i, align 8
  %.sum2741.i = add i64 %loadedValue3588.i, 1
  %317 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2741.i
  %exData1280.i = extractelement <16 x double> %add65425.i, i32 1
  store double %exData1280.i, double addrspace(3)* %317, align 8
  br label %postload1684.i

postload1684.i:                                   ; preds = %preload1683.i, %postload1681.i
  br i1 %exmask1154.i, label %preload1686.i, label %postload1687.i

preload1686.i:                                    ; preds = %postload1684.i
  %"&(pSB[currWI].offset)3590.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3591.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3590.i"
  %CastToValueType3592.i = bitcast i8* %"&pSB[currWI].offset3591.i" to i64*
  %loadedValue3593.i = load i64* %CastToValueType3592.i, align 8
  %.sum2740.i = add i64 %loadedValue3593.i, 2
  %318 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2740.i
  %exData1283.i = extractelement <16 x double> %add65425.i, i32 2
  store double %exData1283.i, double addrspace(3)* %318, align 8
  br label %postload1687.i

postload1687.i:                                   ; preds = %preload1686.i, %postload1684.i
  br i1 %exmask1158.i, label %preload1689.i, label %postload1690.i

preload1689.i:                                    ; preds = %postload1687.i
  %"&(pSB[currWI].offset)3595.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3596.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3595.i"
  %CastToValueType3597.i = bitcast i8* %"&pSB[currWI].offset3596.i" to i64*
  %loadedValue3598.i = load i64* %CastToValueType3597.i, align 8
  %.sum2739.i = add i64 %loadedValue3598.i, 3
  %319 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2739.i
  %exData1286.i = extractelement <16 x double> %add65425.i, i32 3
  store double %exData1286.i, double addrspace(3)* %319, align 8
  br label %postload1690.i

postload1690.i:                                   ; preds = %preload1689.i, %postload1687.i
  br i1 %exmask1162.i, label %preload2490.i, label %postload2491.i

preload2490.i:                                    ; preds = %postload1690.i
  %"&(pSB[currWI].offset)3600.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3601.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3600.i"
  %CastToValueType3602.i = bitcast i8* %"&pSB[currWI].offset3601.i" to i64*
  %loadedValue3603.i = load i64* %CastToValueType3602.i, align 8
  %.sum2738.i = add i64 %loadedValue3603.i, 4
  %320 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2738.i
  %exData1289.i = extractelement <16 x double> %add65425.i, i32 4
  store double %exData1289.i, double addrspace(3)* %320, align 8
  br label %postload2491.i

postload2491.i:                                   ; preds = %preload2490.i, %postload1690.i
  br i1 %exmask1166.i, label %preload2493.i, label %postload2494.i

preload2493.i:                                    ; preds = %postload2491.i
  %"&(pSB[currWI].offset)3605.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3606.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3605.i"
  %CastToValueType3607.i = bitcast i8* %"&pSB[currWI].offset3606.i" to i64*
  %loadedValue3608.i = load i64* %CastToValueType3607.i, align 8
  %.sum2737.i = add i64 %loadedValue3608.i, 5
  %321 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2737.i
  %exData1292.i = extractelement <16 x double> %add65425.i, i32 5
  store double %exData1292.i, double addrspace(3)* %321, align 8
  br label %postload2494.i

postload2494.i:                                   ; preds = %preload2493.i, %postload2491.i
  br i1 %exmask1170.i, label %preload2496.i, label %postload2497.i

preload2496.i:                                    ; preds = %postload2494.i
  %"&(pSB[currWI].offset)3610.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3611.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3610.i"
  %CastToValueType3612.i = bitcast i8* %"&pSB[currWI].offset3611.i" to i64*
  %loadedValue3613.i = load i64* %CastToValueType3612.i, align 8
  %.sum2736.i = add i64 %loadedValue3613.i, 6
  %322 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2736.i
  %exData1295.i = extractelement <16 x double> %add65425.i, i32 6
  store double %exData1295.i, double addrspace(3)* %322, align 8
  br label %postload2497.i

postload2497.i:                                   ; preds = %preload2496.i, %postload2494.i
  br i1 %exmask1174.i, label %preload2499.i, label %postload2500.i

preload2499.i:                                    ; preds = %postload2497.i
  %"&(pSB[currWI].offset)3615.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3616.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3615.i"
  %CastToValueType3617.i = bitcast i8* %"&pSB[currWI].offset3616.i" to i64*
  %loadedValue3618.i = load i64* %CastToValueType3617.i, align 8
  %.sum2735.i = add i64 %loadedValue3618.i, 7
  %323 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2735.i
  %exData1298.i = extractelement <16 x double> %add65425.i, i32 7
  store double %exData1298.i, double addrspace(3)* %323, align 8
  br label %postload2500.i

postload2500.i:                                   ; preds = %preload2499.i, %postload2497.i
  br i1 %exmask1178.i, label %preload2652.i, label %postload2653.i

preload2652.i:                                    ; preds = %postload2500.i
  %"&(pSB[currWI].offset)3620.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3621.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3620.i"
  %CastToValueType3622.i = bitcast i8* %"&pSB[currWI].offset3621.i" to i64*
  %loadedValue3623.i = load i64* %CastToValueType3622.i, align 8
  %.sum2734.i = add i64 %loadedValue3623.i, 8
  %324 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2734.i
  %exData1301.i = extractelement <16 x double> %add65425.i, i32 8
  store double %exData1301.i, double addrspace(3)* %324, align 8
  br label %postload2653.i

postload2653.i:                                   ; preds = %preload2652.i, %postload2500.i
  br i1 %exmask1182.i, label %preload2655.i, label %postload2656.i

preload2655.i:                                    ; preds = %postload2653.i
  %"&(pSB[currWI].offset)3625.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3626.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3625.i"
  %CastToValueType3627.i = bitcast i8* %"&pSB[currWI].offset3626.i" to i64*
  %loadedValue3628.i = load i64* %CastToValueType3627.i, align 8
  %.sum2733.i = add i64 %loadedValue3628.i, 9
  %325 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2733.i
  %exData1304.i = extractelement <16 x double> %add65425.i, i32 9
  store double %exData1304.i, double addrspace(3)* %325, align 8
  br label %postload2656.i

postload2656.i:                                   ; preds = %preload2655.i, %postload2653.i
  br i1 %exmask1186.i, label %preload2658.i, label %postload2659.i

preload2658.i:                                    ; preds = %postload2656.i
  %"&(pSB[currWI].offset)3630.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3631.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3630.i"
  %CastToValueType3632.i = bitcast i8* %"&pSB[currWI].offset3631.i" to i64*
  %loadedValue3633.i = load i64* %CastToValueType3632.i, align 8
  %.sum2732.i = add i64 %loadedValue3633.i, 10
  %326 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2732.i
  %exData1307.i = extractelement <16 x double> %add65425.i, i32 10
  store double %exData1307.i, double addrspace(3)* %326, align 8
  br label %postload2659.i

postload2659.i:                                   ; preds = %preload2658.i, %postload2656.i
  br i1 %exmask1190.i, label %preload2661.i, label %postload2662.i

preload2661.i:                                    ; preds = %postload2659.i
  %"&(pSB[currWI].offset)3635.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3636.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3635.i"
  %CastToValueType3637.i = bitcast i8* %"&pSB[currWI].offset3636.i" to i64*
  %loadedValue3638.i = load i64* %CastToValueType3637.i, align 8
  %.sum2731.i = add i64 %loadedValue3638.i, 11
  %327 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2731.i
  %exData1310.i = extractelement <16 x double> %add65425.i, i32 11
  store double %exData1310.i, double addrspace(3)* %327, align 8
  br label %postload2662.i

postload2662.i:                                   ; preds = %preload2661.i, %postload2659.i
  br i1 %exmask1194.i, label %preload2526.i, label %postload2527.i

preload2526.i:                                    ; preds = %postload2662.i
  %"&(pSB[currWI].offset)3640.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3641.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3640.i"
  %CastToValueType3642.i = bitcast i8* %"&pSB[currWI].offset3641.i" to i64*
  %loadedValue3643.i = load i64* %CastToValueType3642.i, align 8
  %.sum2730.i = add i64 %loadedValue3643.i, 12
  %328 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2730.i
  %exData1313.i = extractelement <16 x double> %add65425.i, i32 12
  store double %exData1313.i, double addrspace(3)* %328, align 8
  br label %postload2527.i

postload2527.i:                                   ; preds = %preload2526.i, %postload2662.i
  br i1 %exmask1198.i, label %preload2529.i, label %postload2530.i

preload2529.i:                                    ; preds = %postload2527.i
  %"&(pSB[currWI].offset)3645.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3646.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3645.i"
  %CastToValueType3647.i = bitcast i8* %"&pSB[currWI].offset3646.i" to i64*
  %loadedValue3648.i = load i64* %CastToValueType3647.i, align 8
  %.sum2729.i = add i64 %loadedValue3648.i, 13
  %329 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2729.i
  %exData1316.i = extractelement <16 x double> %add65425.i, i32 13
  store double %exData1316.i, double addrspace(3)* %329, align 8
  br label %postload2530.i

postload2530.i:                                   ; preds = %preload2529.i, %postload2527.i
  br i1 %exmask1202.i, label %preload2532.i, label %postload2533.i

preload2532.i:                                    ; preds = %postload2530.i
  %"&(pSB[currWI].offset)3650.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3651.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3650.i"
  %CastToValueType3652.i = bitcast i8* %"&pSB[currWI].offset3651.i" to i64*
  %loadedValue3653.i = load i64* %CastToValueType3652.i, align 8
  %.sum2728.i = add i64 %loadedValue3653.i, 14
  %330 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2728.i
  %exData1319.i = extractelement <16 x double> %add65425.i, i32 14
  store double %exData1319.i, double addrspace(3)* %330, align 8
  br label %postload2533.i

postload2533.i:                                   ; preds = %preload2532.i, %postload2530.i
  br i1 %exmask1206.i, label %preload2535.i, label %postload2533.i.if.end66.i_crit_edge

postload2533.i.if.end66.i_crit_edge:              ; preds = %postload2533.i
  br label %if.end66.i

preload2535.i:                                    ; preds = %postload2533.i
  %"&(pSB[currWI].offset)3655.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset3656.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3655.i"
  %CastToValueType3657.i = bitcast i8* %"&pSB[currWI].offset3656.i" to i64*
  %loadedValue3658.i = load i64* %CastToValueType3657.i, align 8
  %.sum2727.i = add i64 %loadedValue3658.i, 15
  %331 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2727.i
  %exData1322.i = extractelement <16 x double> %add65425.i, i32 15
  store double %exData1322.i, double addrspace(3)* %331, align 8
  br label %if.end66.i

if.end66.i:                                       ; preds = %postload2533.i.if.end66.i_crit_edge, %preload2535.i
  %"&(pSB[currWI].offset)4002.i" = add nuw i64 %CurrSBIndex..8.i, 98
  %"&pSB[currWI].offset4003.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4002.i"
  %CastToValueType4004.i = bitcast i8* %"&pSB[currWI].offset4003.i" to i1*
  %loadedValue4005.i = load i1* %CastToValueType4004.i, align 1
  br i1 %loadedValue4005.i, label %preload2519.i, label %if.end66.i.postload2520.i_crit_edge

if.end66.i.postload2520.i_crit_edge:              ; preds = %if.end66.i
  br label %postload2520.i

preload2519.i:                                    ; preds = %if.end66.i
  %check.WI.iter4575.i = icmp ult i64 %CurrWI..8.i, %31
  br i1 %check.WI.iter4575.i, label %thenBB4572.i, label %preload2519.i.postload2520.i_crit_edge

preload2519.i.postload2520.i_crit_edge:           ; preds = %preload2519.i
  br label %postload2520.i

thenBB4572.i:                                     ; preds = %preload2519.i
  %"CurrWI++4576.i" = add nuw i64 %CurrWI..8.i, 1
  %"loadedCurrSB+Stride4578.i" = add nuw i64 %CurrSBIndex..8.i, 256
  switch i32 %currBarrier.8.i, label %thenBB4572.i.postload2518.i_crit_edge [
    i32 5, label %thenBB4572.i.postload2516.i_crit_edge
    i32 13, label %thenBB4572.i.postload2514.i_crit_edge
    i32 8, label %thenBB4572.i.postload2512.i_crit_edge
    i32 14, label %SyncBB.outer.i
  ]

thenBB4572.i.postload2512.i_crit_edge:            ; preds = %thenBB4572.i
  br label %postload2512.i

thenBB4572.i.postload2514.i_crit_edge:            ; preds = %thenBB4572.i
  br label %postload2514.i

thenBB4572.i.postload2516.i_crit_edge:            ; preds = %thenBB4572.i
  br label %postload2516.i

thenBB4572.i.postload2518.i_crit_edge:            ; preds = %thenBB4572.i
  br label %postload2518.i

postload2520.i:                                   ; preds = %thenBB4603.i.postload2520.i_crit_edge, %thenBB4595.i.postload2520.i_crit_edge, %preload2519.i.postload2520.i_crit_edge, %if.end66.i.postload2520.i_crit_edge
  %CurrWI..10.i = phi i64 [ %CurrWI..8.i, %if.end66.i.postload2520.i_crit_edge ], [ 0, %preload2519.i.postload2520.i_crit_edge ], [ %"CurrWI++4599.i", %thenBB4595.i.postload2520.i_crit_edge ], [ %"CurrWI++4607.i", %thenBB4603.i.postload2520.i_crit_edge ]
  %CurrSBIndex..10.i = phi i64 [ %CurrSBIndex..8.i, %if.end66.i.postload2520.i_crit_edge ], [ 0, %preload2519.i.postload2520.i_crit_edge ], [ %"loadedCurrSB+Stride4601.i", %thenBB4595.i.postload2520.i_crit_edge ], [ %"loadedCurrSB+Stride4609.i", %thenBB4603.i.postload2520.i_crit_edge ]
  %currBarrier.10.i = phi i32 [ %currBarrier.8.i, %if.end66.i.postload2520.i_crit_edge ], [ 3, %preload2519.i.postload2520.i_crit_edge ], [ %currBarrier.10.i, %thenBB4595.i.postload2520.i_crit_edge ], [ %currBarrier.12.i, %thenBB4603.i.postload2520.i_crit_edge ]
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
  br i1 %exmask1325.i, label %preload1588.i, label %postload1589.i

preload1588.i:                                    ; preds = %postload2520.i
  %333 = getelementptr inbounds [128 x double] addrspace(3)* %35, i64 0, i64 %extract430.i
  %vload1326.i = load double addrspace(3)* %333, align 8
  br label %postload1589.i

postload1589.i:                                   ; preds = %preload1588.i, %postload2520.i
  %phi1590.i = phi double [ undef, %postload2520.i ], [ %vload1326.i, %preload1588.i ]
  %vpack1327.i = insertelement <16 x double> undef, double %phi1590.i, i32 0
  %exmask1329.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 1
  %"&(pSB[currWI].offset)4205.i" = add nuw i64 %CurrSBIndex..10.i, 233
  %"&pSB[currWI].offset4206.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4205.i"
  %CastToValueType4207.i = bitcast i8* %"&pSB[currWI].offset4206.i" to i1*
  store i1 %exmask1329.i, i1* %CastToValueType4207.i, align 1
  br i1 %exmask1329.i, label %preload1591.i, label %postload1592.i

preload1591.i:                                    ; preds = %postload1589.i
  %.sum2726.i = add i64 %extract430.i, 1
  %334 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2726.i
  %vload1330.i = load double addrspace(3)* %334, align 8
  br label %postload1592.i

postload1592.i:                                   ; preds = %preload1591.i, %postload1589.i
  %phi1593.i = phi double [ undef, %postload1589.i ], [ %vload1330.i, %preload1591.i ]
  %vpack1331.i = insertelement <16 x double> %vpack1327.i, double %phi1593.i, i32 1
  %exmask1333.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 2
  %"&(pSB[currWI].offset)4229.i" = add nuw i64 %CurrSBIndex..10.i, 234
  %"&pSB[currWI].offset4230.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4229.i"
  %CastToValueType4231.i = bitcast i8* %"&pSB[currWI].offset4230.i" to i1*
  store i1 %exmask1333.i, i1* %CastToValueType4231.i, align 1
  br i1 %exmask1333.i, label %preload1594.i, label %postload1595.i

preload1594.i:                                    ; preds = %postload1592.i
  %.sum2725.i = add i64 %extract430.i, 2
  %335 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2725.i
  %vload1334.i = load double addrspace(3)* %335, align 8
  br label %postload1595.i

postload1595.i:                                   ; preds = %preload1594.i, %postload1592.i
  %phi1596.i = phi double [ undef, %postload1592.i ], [ %vload1334.i, %preload1594.i ]
  %vpack1335.i = insertelement <16 x double> %vpack1331.i, double %phi1596.i, i32 2
  %exmask1337.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 3
  %"&(pSB[currWI].offset)4253.i" = add nuw i64 %CurrSBIndex..10.i, 235
  %"&pSB[currWI].offset4254.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4253.i"
  %CastToValueType4255.i = bitcast i8* %"&pSB[currWI].offset4254.i" to i1*
  store i1 %exmask1337.i, i1* %CastToValueType4255.i, align 1
  br i1 %exmask1337.i, label %preload1597.i, label %postload1598.i

preload1597.i:                                    ; preds = %postload1595.i
  %.sum2724.i = add i64 %extract430.i, 3
  %336 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2724.i
  %vload1338.i = load double addrspace(3)* %336, align 8
  br label %postload1598.i

postload1598.i:                                   ; preds = %preload1597.i, %postload1595.i
  %phi1599.i = phi double [ undef, %postload1595.i ], [ %vload1338.i, %preload1597.i ]
  %vpack1339.i = insertelement <16 x double> %vpack1335.i, double %phi1599.i, i32 3
  %exmask1341.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 4
  %"&(pSB[currWI].offset)4277.i" = add nuw i64 %CurrSBIndex..10.i, 236
  %"&pSB[currWI].offset4278.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4277.i"
  %CastToValueType4279.i = bitcast i8* %"&pSB[currWI].offset4278.i" to i1*
  store i1 %exmask1341.i, i1* %CastToValueType4279.i, align 1
  br i1 %exmask1341.i, label %preload1884.i, label %postload1885.i

preload1884.i:                                    ; preds = %postload1598.i
  %.sum2723.i = add i64 %extract430.i, 4
  %337 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2723.i
  %vload1342.i = load double addrspace(3)* %337, align 8
  br label %postload1885.i

postload1885.i:                                   ; preds = %preload1884.i, %postload1598.i
  %phi1886.i = phi double [ undef, %postload1598.i ], [ %vload1342.i, %preload1884.i ]
  %vpack1343.i = insertelement <16 x double> %vpack1339.i, double %phi1886.i, i32 4
  %exmask1345.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 5
  %"&(pSB[currWI].offset)4301.i" = add nuw i64 %CurrSBIndex..10.i, 237
  %"&pSB[currWI].offset4302.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4301.i"
  %CastToValueType4303.i = bitcast i8* %"&pSB[currWI].offset4302.i" to i1*
  store i1 %exmask1345.i, i1* %CastToValueType4303.i, align 1
  br i1 %exmask1345.i, label %preload1887.i, label %postload1888.i

preload1887.i:                                    ; preds = %postload1885.i
  %.sum2722.i = add i64 %extract430.i, 5
  %338 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2722.i
  %vload1346.i = load double addrspace(3)* %338, align 8
  br label %postload1888.i

postload1888.i:                                   ; preds = %preload1887.i, %postload1885.i
  %phi1889.i = phi double [ undef, %postload1885.i ], [ %vload1346.i, %preload1887.i ]
  %vpack1347.i = insertelement <16 x double> %vpack1343.i, double %phi1889.i, i32 5
  %exmask1349.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 6
  %"&(pSB[currWI].offset)4325.i" = add nuw i64 %CurrSBIndex..10.i, 238
  %"&pSB[currWI].offset4326.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4325.i"
  %CastToValueType4327.i = bitcast i8* %"&pSB[currWI].offset4326.i" to i1*
  store i1 %exmask1349.i, i1* %CastToValueType4327.i, align 1
  br i1 %exmask1349.i, label %preload1890.i, label %postload1891.i

preload1890.i:                                    ; preds = %postload1888.i
  %.sum2721.i = add i64 %extract430.i, 6
  %339 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2721.i
  %vload1350.i = load double addrspace(3)* %339, align 8
  br label %postload1891.i

postload1891.i:                                   ; preds = %preload1890.i, %postload1888.i
  %phi1892.i = phi double [ undef, %postload1888.i ], [ %vload1350.i, %preload1890.i ]
  %vpack1351.i = insertelement <16 x double> %vpack1347.i, double %phi1892.i, i32 6
  %exmask1353.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 7
  %"&(pSB[currWI].offset)4349.i" = add nuw i64 %CurrSBIndex..10.i, 239
  %"&pSB[currWI].offset4350.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4349.i"
  %CastToValueType4351.i = bitcast i8* %"&pSB[currWI].offset4350.i" to i1*
  store i1 %exmask1353.i, i1* %CastToValueType4351.i, align 1
  br i1 %exmask1353.i, label %preload1788.i, label %postload1789.i

preload1788.i:                                    ; preds = %postload1891.i
  %.sum2720.i = add i64 %extract430.i, 7
  %340 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2720.i
  %vload1354.i = load double addrspace(3)* %340, align 8
  br label %postload1789.i

postload1789.i:                                   ; preds = %preload1788.i, %postload1891.i
  %phi1790.i = phi double [ undef, %postload1891.i ], [ %vload1354.i, %preload1788.i ]
  %vpack1355.i = insertelement <16 x double> %vpack1351.i, double %phi1790.i, i32 7
  %exmask1357.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 8
  %"&(pSB[currWI].offset)4373.i" = add nuw i64 %CurrSBIndex..10.i, 240
  %"&pSB[currWI].offset4374.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4373.i"
  %CastToValueType4375.i = bitcast i8* %"&pSB[currWI].offset4374.i" to i1*
  store i1 %exmask1357.i, i1* %CastToValueType4375.i, align 1
  br i1 %exmask1357.i, label %preload1791.i, label %postload1792.i

preload1791.i:                                    ; preds = %postload1789.i
  %.sum2719.i = add i64 %extract430.i, 8
  %341 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2719.i
  %vload1358.i = load double addrspace(3)* %341, align 8
  br label %postload1792.i

postload1792.i:                                   ; preds = %preload1791.i, %postload1789.i
  %phi1793.i = phi double [ undef, %postload1789.i ], [ %vload1358.i, %preload1791.i ]
  %vpack1359.i = insertelement <16 x double> %vpack1355.i, double %phi1793.i, i32 8
  %exmask1361.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 9
  %"&(pSB[currWI].offset)4397.i" = add nuw i64 %CurrSBIndex..10.i, 241
  %"&pSB[currWI].offset4398.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4397.i"
  %CastToValueType4399.i = bitcast i8* %"&pSB[currWI].offset4398.i" to i1*
  store i1 %exmask1361.i, i1* %CastToValueType4399.i, align 1
  br i1 %exmask1361.i, label %preload1794.i, label %postload1795.i

preload1794.i:                                    ; preds = %postload1792.i
  %.sum2718.i = add i64 %extract430.i, 9
  %342 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2718.i
  %vload1362.i = load double addrspace(3)* %342, align 8
  br label %postload1795.i

postload1795.i:                                   ; preds = %preload1794.i, %postload1792.i
  %phi1796.i = phi double [ undef, %postload1792.i ], [ %vload1362.i, %preload1794.i ]
  %vpack1363.i = insertelement <16 x double> %vpack1359.i, double %phi1796.i, i32 9
  %exmask1365.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 10
  %"&(pSB[currWI].offset)4421.i" = add nuw i64 %CurrSBIndex..10.i, 242
  %"&pSB[currWI].offset4422.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4421.i"
  %CastToValueType4423.i = bitcast i8* %"&pSB[currWI].offset4422.i" to i1*
  store i1 %exmask1365.i, i1* %CastToValueType4423.i, align 1
  br i1 %exmask1365.i, label %preload1797.i, label %postload1798.i

preload1797.i:                                    ; preds = %postload1795.i
  %.sum2717.i = add i64 %extract430.i, 10
  %343 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2717.i
  %vload1366.i = load double addrspace(3)* %343, align 8
  br label %postload1798.i

postload1798.i:                                   ; preds = %preload1797.i, %postload1795.i
  %phi1799.i = phi double [ undef, %postload1795.i ], [ %vload1366.i, %preload1797.i ]
  %vpack1367.i = insertelement <16 x double> %vpack1363.i, double %phi1799.i, i32 10
  %exmask1369.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 11
  %"&(pSB[currWI].offset)4445.i" = add nuw i64 %CurrSBIndex..10.i, 243
  %"&pSB[currWI].offset4446.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4445.i"
  %CastToValueType4447.i = bitcast i8* %"&pSB[currWI].offset4446.i" to i1*
  store i1 %exmask1369.i, i1* %CastToValueType4447.i, align 1
  br i1 %exmask1369.i, label %preload2424.i, label %postload2425.i

preload2424.i:                                    ; preds = %postload1798.i
  %.sum2716.i = add i64 %extract430.i, 11
  %344 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2716.i
  %vload1370.i = load double addrspace(3)* %344, align 8
  br label %postload2425.i

postload2425.i:                                   ; preds = %preload2424.i, %postload1798.i
  %phi2426.i = phi double [ undef, %postload1798.i ], [ %vload1370.i, %preload2424.i ]
  %vpack1371.i = insertelement <16 x double> %vpack1367.i, double %phi2426.i, i32 11
  %exmask1373.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 12
  %"&(pSB[currWI].offset)4469.i" = add nuw i64 %CurrSBIndex..10.i, 244
  %"&pSB[currWI].offset4470.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4469.i"
  %CastToValueType4471.i = bitcast i8* %"&pSB[currWI].offset4470.i" to i1*
  store i1 %exmask1373.i, i1* %CastToValueType4471.i, align 1
  br i1 %exmask1373.i, label %preload2427.i, label %postload2428.i

preload2427.i:                                    ; preds = %postload2425.i
  %.sum2715.i = add i64 %extract430.i, 12
  %345 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2715.i
  %vload1374.i = load double addrspace(3)* %345, align 8
  br label %postload2428.i

postload2428.i:                                   ; preds = %preload2427.i, %postload2425.i
  %phi2429.i = phi double [ undef, %postload2425.i ], [ %vload1374.i, %preload2427.i ]
  %vpack1375.i = insertelement <16 x double> %vpack1371.i, double %phi2429.i, i32 12
  %exmask1377.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 13
  %"&(pSB[currWI].offset)4493.i" = add nuw i64 %CurrSBIndex..10.i, 245
  %"&pSB[currWI].offset4494.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4493.i"
  %CastToValueType4495.i = bitcast i8* %"&pSB[currWI].offset4494.i" to i1*
  store i1 %exmask1377.i, i1* %CastToValueType4495.i, align 1
  br i1 %exmask1377.i, label %preload2430.i, label %postload2431.i

preload2430.i:                                    ; preds = %postload2428.i
  %.sum2714.i = add i64 %extract430.i, 13
  %346 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2714.i
  %vload1378.i = load double addrspace(3)* %346, align 8
  br label %postload2431.i

postload2431.i:                                   ; preds = %preload2430.i, %postload2428.i
  %phi2432.i = phi double [ undef, %postload2428.i ], [ %vload1378.i, %preload2430.i ]
  %vpack1379.i = insertelement <16 x double> %vpack1375.i, double %phi2432.i, i32 13
  %exmask1381.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 14
  %"&(pSB[currWI].offset)4517.i" = add nuw i64 %CurrSBIndex..10.i, 246
  %"&pSB[currWI].offset4518.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4517.i"
  %CastToValueType4519.i = bitcast i8* %"&pSB[currWI].offset4518.i" to i1*
  store i1 %exmask1381.i, i1* %CastToValueType4519.i, align 1
  br i1 %exmask1381.i, label %preload2433.i, label %postload2434.i

preload2433.i:                                    ; preds = %postload2431.i
  %.sum2713.i = add i64 %extract430.i, 14
  %347 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2713.i
  %vload1382.i = load double addrspace(3)* %347, align 8
  br label %postload2434.i

postload2434.i:                                   ; preds = %preload2433.i, %postload2431.i
  %phi2435.i = phi double [ undef, %postload2431.i ], [ %vload1382.i, %preload2433.i ]
  %vpack1383.i = insertelement <16 x double> %vpack1379.i, double %phi2435.i, i32 14
  %exmask1385.i = extractelement <16 x i1> %if.end66_to_if.then69427.i, i32 15
  %"&(pSB[currWI].offset)4541.i" = add nuw i64 %CurrSBIndex..10.i, 247
  %"&pSB[currWI].offset4542.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4541.i"
  %CastToValueType4543.i = bitcast i8* %"&pSB[currWI].offset4542.i" to i1*
  store i1 %exmask1385.i, i1* %CastToValueType4543.i, align 1
  br i1 %exmask1385.i, label %preload2640.i, label %postload2641.i

preload2640.i:                                    ; preds = %postload2434.i
  %.sum2712.i = add i64 %extract430.i, 15
  %348 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2712.i
  %vload1386.i = load double addrspace(3)* %348, align 8
  br label %postload2641.i

postload2641.i:                                   ; preds = %preload2640.i, %postload2434.i
  %phi2642.i = phi double [ undef, %postload2434.i ], [ %vload1386.i, %preload2640.i ]
  %vpack1387.i = insertelement <16 x double> %vpack1383.i, double %phi2642.i, i32 15
  br i1 %exmask1325.i, label %preload2643.i, label %postload2644.i

preload2643.i:                                    ; preds = %postload2641.i
  %"&(pSB[currWI].offset)3934.i" = add nuw i64 %CurrSBIndex..10.i, 80
  %"&pSB[currWI].offset3935.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3934.i"
  %CastToValueType3936.i = bitcast i8* %"&pSB[currWI].offset3935.i" to double addrspace(3)**
  %loadedValue3937.i = load double addrspace(3)** %CastToValueType3936.i, align 8
  %vload1391.i = load double addrspace(3)* %loadedValue3937.i, align 8
  br label %postload2644.i

postload2644.i:                                   ; preds = %preload2643.i, %postload2641.i
  %phi2645.i = phi double [ undef, %postload2641.i ], [ %vload1391.i, %preload2643.i ]
  %vpack1392.i = insertelement <16 x double> undef, double %phi2645.i, i32 0
  br i1 %exmask1329.i, label %preload2646.i, label %postload2647.i

preload2646.i:                                    ; preds = %postload2644.i
  %"&(pSB[currWI].offset)3660.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3661.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3660.i"
  %CastToValueType3662.i = bitcast i8* %"&pSB[currWI].offset3661.i" to i64*
  %loadedValue3663.i = load i64* %CastToValueType3662.i, align 8
  %.sum2711.i = add i64 %loadedValue3663.i, 1
  %349 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2711.i
  %vload1395.i = load double addrspace(3)* %349, align 8
  br label %postload2647.i

postload2647.i:                                   ; preds = %preload2646.i, %postload2644.i
  %phi2648.i = phi double [ undef, %postload2644.i ], [ %vload1395.i, %preload2646.i ]
  %vpack1396.i = insertelement <16 x double> %vpack1392.i, double %phi2648.i, i32 1
  br i1 %exmask1333.i, label %preload2649.i, label %postload2650.i

preload2649.i:                                    ; preds = %postload2647.i
  %"&(pSB[currWI].offset)3665.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3666.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3665.i"
  %CastToValueType3667.i = bitcast i8* %"&pSB[currWI].offset3666.i" to i64*
  %loadedValue3668.i = load i64* %CastToValueType3667.i, align 8
  %.sum2710.i = add i64 %loadedValue3668.i, 2
  %350 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2710.i
  %vload1399.i = load double addrspace(3)* %350, align 8
  br label %postload2650.i

postload2650.i:                                   ; preds = %preload2649.i, %postload2647.i
  %phi2651.i = phi double [ undef, %postload2647.i ], [ %vload1399.i, %preload2649.i ]
  %vpack1400.i = insertelement <16 x double> %vpack1396.i, double %phi2651.i, i32 2
  br i1 %exmask1337.i, label %preload2307.i, label %postload2308.i

preload2307.i:                                    ; preds = %postload2650.i
  %"&(pSB[currWI].offset)3670.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3671.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3670.i"
  %CastToValueType3672.i = bitcast i8* %"&pSB[currWI].offset3671.i" to i64*
  %loadedValue3673.i = load i64* %CastToValueType3672.i, align 8
  %.sum2709.i = add i64 %loadedValue3673.i, 3
  %351 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2709.i
  %vload1403.i = load double addrspace(3)* %351, align 8
  br label %postload2308.i

postload2308.i:                                   ; preds = %preload2307.i, %postload2650.i
  %phi2309.i = phi double [ undef, %postload2650.i ], [ %vload1403.i, %preload2307.i ]
  %vpack1404.i = insertelement <16 x double> %vpack1400.i, double %phi2309.i, i32 3
  br i1 %exmask1341.i, label %preload2310.i, label %postload2311.i

preload2310.i:                                    ; preds = %postload2308.i
  %"&(pSB[currWI].offset)3675.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3676.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3675.i"
  %CastToValueType3677.i = bitcast i8* %"&pSB[currWI].offset3676.i" to i64*
  %loadedValue3678.i = load i64* %CastToValueType3677.i, align 8
  %.sum2708.i = add i64 %loadedValue3678.i, 4
  %352 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2708.i
  %vload1407.i = load double addrspace(3)* %352, align 8
  br label %postload2311.i

postload2311.i:                                   ; preds = %preload2310.i, %postload2308.i
  %phi2312.i = phi double [ undef, %postload2308.i ], [ %vload1407.i, %preload2310.i ]
  %vpack1408.i = insertelement <16 x double> %vpack1404.i, double %phi2312.i, i32 4
  br i1 %exmask1345.i, label %preload2313.i, label %postload2314.i

preload2313.i:                                    ; preds = %postload2311.i
  %"&(pSB[currWI].offset)3680.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3681.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3680.i"
  %CastToValueType3682.i = bitcast i8* %"&pSB[currWI].offset3681.i" to i64*
  %loadedValue3683.i = load i64* %CastToValueType3682.i, align 8
  %.sum2707.i = add i64 %loadedValue3683.i, 5
  %353 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2707.i
  %vload1411.i = load double addrspace(3)* %353, align 8
  br label %postload2314.i

postload2314.i:                                   ; preds = %preload2313.i, %postload2311.i
  %phi2315.i = phi double [ undef, %postload2311.i ], [ %vload1411.i, %preload2313.i ]
  %vpack1412.i = insertelement <16 x double> %vpack1408.i, double %phi2315.i, i32 5
  br i1 %exmask1349.i, label %preload2316.i, label %postload2317.i

preload2316.i:                                    ; preds = %postload2314.i
  %"&(pSB[currWI].offset)3685.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3686.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3685.i"
  %CastToValueType3687.i = bitcast i8* %"&pSB[currWI].offset3686.i" to i64*
  %loadedValue3688.i = load i64* %CastToValueType3687.i, align 8
  %.sum2706.i = add i64 %loadedValue3688.i, 6
  %354 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2706.i
  %vload1415.i = load double addrspace(3)* %354, align 8
  br label %postload2317.i

postload2317.i:                                   ; preds = %preload2316.i, %postload2314.i
  %phi2318.i = phi double [ undef, %postload2314.i ], [ %vload1415.i, %preload2316.i ]
  %vpack1416.i = insertelement <16 x double> %vpack1412.i, double %phi2318.i, i32 6
  br i1 %exmask1353.i, label %preload2577.i, label %postload2578.i

preload2577.i:                                    ; preds = %postload2317.i
  %"&(pSB[currWI].offset)3690.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3691.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3690.i"
  %CastToValueType3692.i = bitcast i8* %"&pSB[currWI].offset3691.i" to i64*
  %loadedValue3693.i = load i64* %CastToValueType3692.i, align 8
  %.sum2705.i = add i64 %loadedValue3693.i, 7
  %355 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2705.i
  %vload1419.i = load double addrspace(3)* %355, align 8
  br label %postload2578.i

postload2578.i:                                   ; preds = %preload2577.i, %postload2317.i
  %phi2579.i = phi double [ undef, %postload2317.i ], [ %vload1419.i, %preload2577.i ]
  %vpack1420.i = insertelement <16 x double> %vpack1416.i, double %phi2579.i, i32 7
  br i1 %exmask1357.i, label %preload2580.i, label %postload2581.i

preload2580.i:                                    ; preds = %postload2578.i
  %"&(pSB[currWI].offset)3695.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3696.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3695.i"
  %CastToValueType3697.i = bitcast i8* %"&pSB[currWI].offset3696.i" to i64*
  %loadedValue3698.i = load i64* %CastToValueType3697.i, align 8
  %.sum2704.i = add i64 %loadedValue3698.i, 8
  %356 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2704.i
  %vload1423.i = load double addrspace(3)* %356, align 8
  br label %postload2581.i

postload2581.i:                                   ; preds = %preload2580.i, %postload2578.i
  %phi2582.i = phi double [ undef, %postload2578.i ], [ %vload1423.i, %preload2580.i ]
  %vpack1424.i = insertelement <16 x double> %vpack1420.i, double %phi2582.i, i32 8
  br i1 %exmask1361.i, label %preload2583.i, label %postload2584.i

preload2583.i:                                    ; preds = %postload2581.i
  %"&(pSB[currWI].offset)3700.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3701.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3700.i"
  %CastToValueType3702.i = bitcast i8* %"&pSB[currWI].offset3701.i" to i64*
  %loadedValue3703.i = load i64* %CastToValueType3702.i, align 8
  %.sum2703.i = add i64 %loadedValue3703.i, 9
  %357 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2703.i
  %vload1427.i = load double addrspace(3)* %357, align 8
  br label %postload2584.i

postload2584.i:                                   ; preds = %preload2583.i, %postload2581.i
  %phi2585.i = phi double [ undef, %postload2581.i ], [ %vload1427.i, %preload2583.i ]
  %vpack1428.i = insertelement <16 x double> %vpack1424.i, double %phi2585.i, i32 9
  br i1 %exmask1365.i, label %preload2586.i, label %postload2587.i

preload2586.i:                                    ; preds = %postload2584.i
  %"&(pSB[currWI].offset)3705.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3706.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3705.i"
  %CastToValueType3707.i = bitcast i8* %"&pSB[currWI].offset3706.i" to i64*
  %loadedValue3708.i = load i64* %CastToValueType3707.i, align 8
  %.sum2702.i = add i64 %loadedValue3708.i, 10
  %358 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2702.i
  %vload1431.i = load double addrspace(3)* %358, align 8
  br label %postload2587.i

postload2587.i:                                   ; preds = %preload2586.i, %postload2584.i
  %phi2588.i = phi double [ undef, %postload2584.i ], [ %vload1431.i, %preload2586.i ]
  %vpack1432.i = insertelement <16 x double> %vpack1428.i, double %phi2588.i, i32 10
  br i1 %exmask1369.i, label %preload2037.i, label %postload2038.i

preload2037.i:                                    ; preds = %postload2587.i
  %"&(pSB[currWI].offset)3710.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3711.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3710.i"
  %CastToValueType3712.i = bitcast i8* %"&pSB[currWI].offset3711.i" to i64*
  %loadedValue3713.i = load i64* %CastToValueType3712.i, align 8
  %.sum2701.i = add i64 %loadedValue3713.i, 11
  %359 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2701.i
  %vload1435.i = load double addrspace(3)* %359, align 8
  br label %postload2038.i

postload2038.i:                                   ; preds = %preload2037.i, %postload2587.i
  %phi2039.i = phi double [ undef, %postload2587.i ], [ %vload1435.i, %preload2037.i ]
  %vpack1436.i = insertelement <16 x double> %vpack1432.i, double %phi2039.i, i32 11
  br i1 %exmask1373.i, label %preload2040.i, label %postload2041.i

preload2040.i:                                    ; preds = %postload2038.i
  %"&(pSB[currWI].offset)3715.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3716.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3715.i"
  %CastToValueType3717.i = bitcast i8* %"&pSB[currWI].offset3716.i" to i64*
  %loadedValue3718.i = load i64* %CastToValueType3717.i, align 8
  %.sum2700.i = add i64 %loadedValue3718.i, 12
  %360 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2700.i
  %vload1439.i = load double addrspace(3)* %360, align 8
  br label %postload2041.i

postload2041.i:                                   ; preds = %preload2040.i, %postload2038.i
  %phi2042.i = phi double [ undef, %postload2038.i ], [ %vload1439.i, %preload2040.i ]
  %vpack1440.i = insertelement <16 x double> %vpack1436.i, double %phi2042.i, i32 12
  br i1 %exmask1377.i, label %preload2043.i, label %postload2044.i

preload2043.i:                                    ; preds = %postload2041.i
  %"&(pSB[currWI].offset)3720.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3721.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3720.i"
  %CastToValueType3722.i = bitcast i8* %"&pSB[currWI].offset3721.i" to i64*
  %loadedValue3723.i = load i64* %CastToValueType3722.i, align 8
  %.sum2699.i = add i64 %loadedValue3723.i, 13
  %361 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2699.i
  %vload1443.i = load double addrspace(3)* %361, align 8
  br label %postload2044.i

postload2044.i:                                   ; preds = %preload2043.i, %postload2041.i
  %phi2045.i = phi double [ undef, %postload2041.i ], [ %vload1443.i, %preload2043.i ]
  %vpack1444.i = insertelement <16 x double> %vpack1440.i, double %phi2045.i, i32 13
  br i1 %exmask1381.i, label %preload2046.i, label %postload2047.i

preload2046.i:                                    ; preds = %postload2044.i
  %"&(pSB[currWI].offset)3725.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3726.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3725.i"
  %CastToValueType3727.i = bitcast i8* %"&pSB[currWI].offset3726.i" to i64*
  %loadedValue3728.i = load i64* %CastToValueType3727.i, align 8
  %.sum2698.i = add i64 %loadedValue3728.i, 14
  %362 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2698.i
  %vload1447.i = load double addrspace(3)* %362, align 8
  br label %postload2047.i

postload2047.i:                                   ; preds = %preload2046.i, %postload2044.i
  %phi2048.i = phi double [ undef, %postload2044.i ], [ %vload1447.i, %preload2046.i ]
  %vpack1448.i = insertelement <16 x double> %vpack1444.i, double %phi2048.i, i32 14
  br i1 %exmask1385.i, label %preload.i, label %postload.i

preload.i:                                        ; preds = %postload2047.i
  %"&(pSB[currWI].offset)3730.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3731.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3730.i"
  %CastToValueType3732.i = bitcast i8* %"&pSB[currWI].offset3731.i" to i64*
  %loadedValue3733.i = load i64* %CastToValueType3732.i, align 8
  %.sum2697.i = add i64 %loadedValue3733.i, 15
  %363 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2697.i
  %vload1451.i = load double addrspace(3)* %363, align 8
  br label %postload.i

postload.i:                                       ; preds = %preload.i, %postload2047.i
  %phi.i = phi double [ undef, %postload2047.i ], [ %vload1451.i, %preload.i ]
  %vpack1452.i = insertelement <16 x double> %vpack1448.i, double %phi.i, i32 15
  %add75448.i = fadd <16 x double> %vpack1452.i, %vpack1387.i
  br i1 %exmask1325.i, label %preload1567.i, label %postload1568.i

preload1567.i:                                    ; preds = %postload.i
  %exData1456.i = extractelement <16 x double> %add75448.i, i32 0
  %"&(pSB[currWI].offset)3939.i" = add nuw i64 %CurrSBIndex..10.i, 80
  %"&pSB[currWI].offset3940.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3939.i"
  %CastToValueType3941.i = bitcast i8* %"&pSB[currWI].offset3940.i" to double addrspace(3)**
  %loadedValue3942.i = load double addrspace(3)** %CastToValueType3941.i, align 8
  store double %exData1456.i, double addrspace(3)* %loadedValue3942.i, align 8
  %loadedValue4222.pre.i = load i1* %CastToValueType4207.i, align 1
  br label %postload1568.i

postload1568.i:                                   ; preds = %preload1567.i, %postload.i
  %loadedValue4222.i = phi i1 [ %loadedValue4222.pre.i, %preload1567.i ], [ %exmask1329.i, %postload.i ]
  br i1 %loadedValue4222.i, label %preload1570.i, label %postload1568.i.postload1571.i_crit_edge

postload1568.i.postload1571.i_crit_edge:          ; preds = %postload1568.i
  br label %postload1571.i

preload1570.i:                                    ; preds = %postload1568.i
  %"&(pSB[currWI].offset)3735.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3736.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3735.i"
  %CastToValueType3737.i = bitcast i8* %"&pSB[currWI].offset3736.i" to i64*
  %loadedValue3738.i = load i64* %CastToValueType3737.i, align 8
  %.sum2696.i = add i64 %loadedValue3738.i, 1
  %364 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2696.i
  %exData1459.i = extractelement <16 x double> %add75448.i, i32 1
  store double %exData1459.i, double addrspace(3)* %364, align 8
  br label %postload1571.i

postload1571.i:                                   ; preds = %postload1568.i.postload1571.i_crit_edge, %preload1570.i
  %loadedValue4246.i = load i1* %CastToValueType4231.i, align 1
  br i1 %loadedValue4246.i, label %preload1573.i, label %postload1571.i.postload1574.i_crit_edge

postload1571.i.postload1574.i_crit_edge:          ; preds = %postload1571.i
  br label %postload1574.i

preload1573.i:                                    ; preds = %postload1571.i
  %"&(pSB[currWI].offset)3740.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3741.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3740.i"
  %CastToValueType3742.i = bitcast i8* %"&pSB[currWI].offset3741.i" to i64*
  %loadedValue3743.i = load i64* %CastToValueType3742.i, align 8
  %.sum2695.i = add i64 %loadedValue3743.i, 2
  %365 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2695.i
  %exData1462.i = extractelement <16 x double> %add75448.i, i32 2
  store double %exData1462.i, double addrspace(3)* %365, align 8
  br label %postload1574.i

postload1574.i:                                   ; preds = %postload1571.i.postload1574.i_crit_edge, %preload1573.i
  %loadedValue4270.i = load i1* %CastToValueType4255.i, align 1
  br i1 %loadedValue4270.i, label %preload2025.i, label %postload1574.i.postload2026.i_crit_edge

postload1574.i.postload2026.i_crit_edge:          ; preds = %postload1574.i
  br label %postload2026.i

preload2025.i:                                    ; preds = %postload1574.i
  %"&(pSB[currWI].offset)3745.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3746.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3745.i"
  %CastToValueType3747.i = bitcast i8* %"&pSB[currWI].offset3746.i" to i64*
  %loadedValue3748.i = load i64* %CastToValueType3747.i, align 8
  %.sum2694.i = add i64 %loadedValue3748.i, 3
  %366 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2694.i
  %exData1465.i = extractelement <16 x double> %add75448.i, i32 3
  store double %exData1465.i, double addrspace(3)* %366, align 8
  br label %postload2026.i

postload2026.i:                                   ; preds = %postload1574.i.postload2026.i_crit_edge, %preload2025.i
  %loadedValue4294.i = load i1* %CastToValueType4279.i, align 1
  br i1 %loadedValue4294.i, label %preload2028.i, label %postload2026.i.postload2029.i_crit_edge

postload2026.i.postload2029.i_crit_edge:          ; preds = %postload2026.i
  br label %postload2029.i

preload2028.i:                                    ; preds = %postload2026.i
  %"&(pSB[currWI].offset)3750.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3751.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3750.i"
  %CastToValueType3752.i = bitcast i8* %"&pSB[currWI].offset3751.i" to i64*
  %loadedValue3753.i = load i64* %CastToValueType3752.i, align 8
  %.sum2693.i = add i64 %loadedValue3753.i, 4
  %367 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2693.i
  %exData1468.i = extractelement <16 x double> %add75448.i, i32 4
  store double %exData1468.i, double addrspace(3)* %367, align 8
  br label %postload2029.i

postload2029.i:                                   ; preds = %postload2026.i.postload2029.i_crit_edge, %preload2028.i
  %loadedValue4318.i = load i1* %CastToValueType4303.i, align 1
  br i1 %loadedValue4318.i, label %preload2031.i, label %postload2029.i.postload2032.i_crit_edge

postload2029.i.postload2032.i_crit_edge:          ; preds = %postload2029.i
  br label %postload2032.i

preload2031.i:                                    ; preds = %postload2029.i
  %"&(pSB[currWI].offset)3755.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3756.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3755.i"
  %CastToValueType3757.i = bitcast i8* %"&pSB[currWI].offset3756.i" to i64*
  %loadedValue3758.i = load i64* %CastToValueType3757.i, align 8
  %.sum2692.i = add i64 %loadedValue3758.i, 5
  %368 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2692.i
  %exData1471.i = extractelement <16 x double> %add75448.i, i32 5
  store double %exData1471.i, double addrspace(3)* %368, align 8
  br label %postload2032.i

postload2032.i:                                   ; preds = %postload2029.i.postload2032.i_crit_edge, %preload2031.i
  %loadedValue4342.i = load i1* %CastToValueType4327.i, align 1
  br i1 %loadedValue4342.i, label %preload2034.i, label %postload2032.i.postload2035.i_crit_edge

postload2032.i.postload2035.i_crit_edge:          ; preds = %postload2032.i
  br label %postload2035.i

preload2034.i:                                    ; preds = %postload2032.i
  %"&(pSB[currWI].offset)3760.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3761.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3760.i"
  %CastToValueType3762.i = bitcast i8* %"&pSB[currWI].offset3761.i" to i64*
  %loadedValue3763.i = load i64* %CastToValueType3762.i, align 8
  %.sum2691.i = add i64 %loadedValue3763.i, 6
  %369 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2691.i
  %exData1474.i = extractelement <16 x double> %add75448.i, i32 6
  store double %exData1474.i, double addrspace(3)* %369, align 8
  br label %postload2035.i

postload2035.i:                                   ; preds = %postload2032.i.postload2035.i_crit_edge, %preload2034.i
  %loadedValue4366.i = load i1* %CastToValueType4351.i, align 1
  br i1 %loadedValue4366.i, label %preload1692.i, label %postload2035.i.postload1693.i_crit_edge

postload2035.i.postload1693.i_crit_edge:          ; preds = %postload2035.i
  br label %postload1693.i

preload1692.i:                                    ; preds = %postload2035.i
  %"&(pSB[currWI].offset)3765.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3766.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3765.i"
  %CastToValueType3767.i = bitcast i8* %"&pSB[currWI].offset3766.i" to i64*
  %loadedValue3768.i = load i64* %CastToValueType3767.i, align 8
  %.sum2690.i = add i64 %loadedValue3768.i, 7
  %370 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2690.i
  %exData1477.i = extractelement <16 x double> %add75448.i, i32 7
  store double %exData1477.i, double addrspace(3)* %370, align 8
  br label %postload1693.i

postload1693.i:                                   ; preds = %postload2035.i.postload1693.i_crit_edge, %preload1692.i
  %loadedValue4390.i = load i1* %CastToValueType4375.i, align 1
  br i1 %loadedValue4390.i, label %preload1695.i, label %postload1693.i.postload1696.i_crit_edge

postload1693.i.postload1696.i_crit_edge:          ; preds = %postload1693.i
  br label %postload1696.i

preload1695.i:                                    ; preds = %postload1693.i
  %"&(pSB[currWI].offset)3770.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3771.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3770.i"
  %CastToValueType3772.i = bitcast i8* %"&pSB[currWI].offset3771.i" to i64*
  %loadedValue3773.i = load i64* %CastToValueType3772.i, align 8
  %.sum2689.i = add i64 %loadedValue3773.i, 8
  %371 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2689.i
  %exData1480.i = extractelement <16 x double> %add75448.i, i32 8
  store double %exData1480.i, double addrspace(3)* %371, align 8
  br label %postload1696.i

postload1696.i:                                   ; preds = %postload1693.i.postload1696.i_crit_edge, %preload1695.i
  %loadedValue4414.i = load i1* %CastToValueType4399.i, align 1
  br i1 %loadedValue4414.i, label %preload1698.i, label %postload1696.i.postload1699.i_crit_edge

postload1696.i.postload1699.i_crit_edge:          ; preds = %postload1696.i
  br label %postload1699.i

preload1698.i:                                    ; preds = %postload1696.i
  %"&(pSB[currWI].offset)3775.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3776.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3775.i"
  %CastToValueType3777.i = bitcast i8* %"&pSB[currWI].offset3776.i" to i64*
  %loadedValue3778.i = load i64* %CastToValueType3777.i, align 8
  %.sum2688.i = add i64 %loadedValue3778.i, 9
  %372 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2688.i
  %exData1483.i = extractelement <16 x double> %add75448.i, i32 9
  store double %exData1483.i, double addrspace(3)* %372, align 8
  br label %postload1699.i

postload1699.i:                                   ; preds = %postload1696.i.postload1699.i_crit_edge, %preload1698.i
  %loadedValue4438.i = load i1* %CastToValueType4423.i, align 1
  br i1 %loadedValue4438.i, label %preload1701.i, label %postload1699.i.postload1702.i_crit_edge

postload1699.i.postload1702.i_crit_edge:          ; preds = %postload1699.i
  br label %postload1702.i

preload1701.i:                                    ; preds = %postload1699.i
  %"&(pSB[currWI].offset)3780.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3781.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3780.i"
  %CastToValueType3782.i = bitcast i8* %"&pSB[currWI].offset3781.i" to i64*
  %loadedValue3783.i = load i64* %CastToValueType3782.i, align 8
  %.sum2687.i = add i64 %loadedValue3783.i, 10
  %373 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2687.i
  %exData1486.i = extractelement <16 x double> %add75448.i, i32 10
  store double %exData1486.i, double addrspace(3)* %373, align 8
  br label %postload1702.i

postload1702.i:                                   ; preds = %postload1699.i.postload1702.i_crit_edge, %preload1701.i
  %loadedValue4462.i = load i1* %CastToValueType4447.i, align 1
  br i1 %loadedValue4462.i, label %preload1704.i, label %postload1702.i.postload1705.i_crit_edge

postload1702.i.postload1705.i_crit_edge:          ; preds = %postload1702.i
  br label %postload1705.i

preload1704.i:                                    ; preds = %postload1702.i
  %"&(pSB[currWI].offset)3785.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3786.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3785.i"
  %CastToValueType3787.i = bitcast i8* %"&pSB[currWI].offset3786.i" to i64*
  %loadedValue3788.i = load i64* %CastToValueType3787.i, align 8
  %.sum2686.i = add i64 %loadedValue3788.i, 11
  %374 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2686.i
  %exData1489.i = extractelement <16 x double> %add75448.i, i32 11
  store double %exData1489.i, double addrspace(3)* %374, align 8
  br label %postload1705.i

postload1705.i:                                   ; preds = %postload1702.i.postload1705.i_crit_edge, %preload1704.i
  %loadedValue4486.i = load i1* %CastToValueType4471.i, align 1
  br i1 %loadedValue4486.i, label %preload1731.i, label %postload1705.i.postload1732.i_crit_edge

postload1705.i.postload1732.i_crit_edge:          ; preds = %postload1705.i
  br label %postload1732.i

preload1731.i:                                    ; preds = %postload1705.i
  %"&(pSB[currWI].offset)3790.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3791.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3790.i"
  %CastToValueType3792.i = bitcast i8* %"&pSB[currWI].offset3791.i" to i64*
  %loadedValue3793.i = load i64* %CastToValueType3792.i, align 8
  %.sum2685.i = add i64 %loadedValue3793.i, 12
  %375 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2685.i
  %exData1492.i = extractelement <16 x double> %add75448.i, i32 12
  store double %exData1492.i, double addrspace(3)* %375, align 8
  br label %postload1732.i

postload1732.i:                                   ; preds = %postload1705.i.postload1732.i_crit_edge, %preload1731.i
  %loadedValue4510.i = load i1* %CastToValueType4495.i, align 1
  br i1 %loadedValue4510.i, label %preload1734.i, label %postload1732.i.postload1735.i_crit_edge

postload1732.i.postload1735.i_crit_edge:          ; preds = %postload1732.i
  br label %postload1735.i

preload1734.i:                                    ; preds = %postload1732.i
  %"&(pSB[currWI].offset)3795.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3796.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3795.i"
  %CastToValueType3797.i = bitcast i8* %"&pSB[currWI].offset3796.i" to i64*
  %loadedValue3798.i = load i64* %CastToValueType3797.i, align 8
  %.sum2684.i = add i64 %loadedValue3798.i, 13
  %376 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2684.i
  %exData1495.i = extractelement <16 x double> %add75448.i, i32 13
  store double %exData1495.i, double addrspace(3)* %376, align 8
  br label %postload1735.i

postload1735.i:                                   ; preds = %postload1732.i.postload1735.i_crit_edge, %preload1734.i
  %loadedValue4534.i = load i1* %CastToValueType4519.i, align 1
  br i1 %loadedValue4534.i, label %preload1737.i, label %postload1735.i.postload1738.i_crit_edge

postload1735.i.postload1738.i_crit_edge:          ; preds = %postload1735.i
  br label %postload1738.i

preload1737.i:                                    ; preds = %postload1735.i
  %"&(pSB[currWI].offset)3800.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3801.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3800.i"
  %CastToValueType3802.i = bitcast i8* %"&pSB[currWI].offset3801.i" to i64*
  %loadedValue3803.i = load i64* %CastToValueType3802.i, align 8
  %.sum2683.i = add i64 %loadedValue3803.i, 14
  %377 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2683.i
  %exData1498.i = extractelement <16 x double> %add75448.i, i32 14
  store double %exData1498.i, double addrspace(3)* %377, align 8
  br label %postload1738.i

postload1738.i:                                   ; preds = %postload1735.i.postload1738.i_crit_edge, %preload1737.i
  %loadedValue4558.i = load i1* %CastToValueType4543.i, align 1
  br i1 %loadedValue4558.i, label %preload1740.i, label %postload1738.i.if.end76.i_crit_edge

postload1738.i.if.end76.i_crit_edge:              ; preds = %postload1738.i
  br label %if.end76.i

preload1740.i:                                    ; preds = %postload1738.i
  %"&(pSB[currWI].offset)3805.i" = add nuw i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset3806.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3805.i"
  %CastToValueType3807.i = bitcast i8* %"&pSB[currWI].offset3806.i" to i64*
  %loadedValue3808.i = load i64* %CastToValueType3807.i, align 8
  %.sum2682.i = add i64 %loadedValue3808.i, 15
  %378 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2682.i
  %exData1501.i = extractelement <16 x double> %add75448.i, i32 15
  store double %exData1501.i, double addrspace(3)* %378, align 8
  br label %if.end76.i

if.end76.i:                                       ; preds = %postload1738.i.if.end76.i_crit_edge, %preload1740.i
  %"&(pSB[currWI].offset)3997.i" = add nuw i64 %CurrSBIndex..10.i, 98
  %"&pSB[currWI].offset3998.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3997.i"
  %CastToValueType3999.i = bitcast i8* %"&pSB[currWI].offset3998.i" to i1*
  %loadedValue4000.i = load i1* %CastToValueType3999.i, align 1
  br i1 %loadedValue4000.i, label %preload2521.i, label %if.end76.i.postload2522.i_crit_edge

if.end76.i.postload2522.i_crit_edge:              ; preds = %if.end76.i
  br label %postload2522.i

preload2521.i:                                    ; preds = %if.end76.i
  %check.WI.iter4598.i = icmp ult i64 %CurrWI..10.i, %31
  br i1 %check.WI.iter4598.i, label %thenBB4595.i, label %preload2521.i.postload2522.i_crit_edge

preload2521.i.postload2522.i_crit_edge:           ; preds = %preload2521.i
  br label %postload2522.i

thenBB4595.i:                                     ; preds = %preload2521.i
  %"CurrWI++4599.i" = add nuw i64 %CurrWI..10.i, 1
  %"loadedCurrSB+Stride4601.i" = add nuw i64 %CurrSBIndex..10.i, 256
  switch i32 %currBarrier.10.i, label %thenBB4595.i.postload2520.i_crit_edge [
    i32 2, label %thenBB4595.i.postload2518.i_crit_edge
    i32 5, label %thenBB4595.i.postload2516.i_crit_edge
    i32 13, label %thenBB4595.i.postload2514.i_crit_edge
    i32 8, label %thenBB4595.i.postload2512.i_crit_edge
    i32 14, label %SyncBB.outer.i
  ]

thenBB4595.i.postload2512.i_crit_edge:            ; preds = %thenBB4595.i
  br label %postload2512.i

thenBB4595.i.postload2514.i_crit_edge:            ; preds = %thenBB4595.i
  br label %postload2514.i

thenBB4595.i.postload2516.i_crit_edge:            ; preds = %thenBB4595.i
  br label %postload2516.i

thenBB4595.i.postload2518.i_crit_edge:            ; preds = %thenBB4595.i
  br label %postload2518.i

thenBB4595.i.postload2520.i_crit_edge:            ; preds = %thenBB4595.i
  br label %postload2520.i

postload2522.i:                                   ; preds = %thenBB4603.i.postload2522.i_crit_edge, %preload2521.i.postload2522.i_crit_edge, %if.end76.i.postload2522.i_crit_edge
  %CurrWI..12.i = phi i64 [ %CurrWI..10.i, %if.end76.i.postload2522.i_crit_edge ], [ 0, %preload2521.i.postload2522.i_crit_edge ], [ %"CurrWI++4607.i", %thenBB4603.i.postload2522.i_crit_edge ]
  %CurrSBIndex..12.i = phi i64 [ %CurrSBIndex..10.i, %if.end76.i.postload2522.i_crit_edge ], [ 0, %preload2521.i.postload2522.i_crit_edge ], [ %"loadedCurrSB+Stride4609.i", %thenBB4603.i.postload2522.i_crit_edge ]
  %currBarrier.12.i = phi i32 [ %currBarrier.10.i, %if.end76.i.postload2522.i_crit_edge ], [ 10, %preload2521.i.postload2522.i_crit_edge ], [ %currBarrier.12.i, %thenBB4603.i.postload2522.i_crit_edge ]
  %"&(pSB[currWI].offset)4185.i" = add nuw i64 %CurrSBIndex..12.i, 232
  %"&pSB[currWI].offset4186.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4185.i"
  %CastToValueType4187.i = bitcast i8* %"&pSB[currWI].offset4186.i" to i1*
  %loadedValue4188.i = load i1* %CastToValueType4187.i, align 1
  br i1 %loadedValue4188.i, label %preload1836.i, label %postload2522.i.postload1837.i_crit_edge

postload2522.i.postload1837.i_crit_edge:          ; preds = %postload2522.i
  br label %postload1837.i

preload1836.i:                                    ; preds = %postload2522.i
  %"&(pSB[currWI].offset)3944.i" = add nuw i64 %CurrSBIndex..12.i, 80
  %"&pSB[currWI].offset3945.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3944.i"
  %CastToValueType3946.i = bitcast i8* %"&pSB[currWI].offset3945.i" to double addrspace(3)**
  %loadedValue3947.i = load double addrspace(3)** %CastToValueType3946.i, align 8
  %vload1505.i = load double addrspace(3)* %loadedValue3947.i, align 8
  br label %postload1837.i

postload1837.i:                                   ; preds = %postload2522.i.postload1837.i_crit_edge, %preload1836.i
  %phi1838.i = phi double [ %vload1505.i, %preload1836.i ], [ undef, %postload2522.i.postload1837.i_crit_edge ]
  %"&(pSB[currWI].offset)4209.i" = add nuw i64 %CurrSBIndex..12.i, 233
  %"&pSB[currWI].offset4210.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4209.i"
  %CastToValueType4211.i = bitcast i8* %"&pSB[currWI].offset4210.i" to i1*
  %loadedValue4212.i = load i1* %CastToValueType4211.i, align 1
  br i1 %loadedValue4212.i, label %preload1839.i, label %postload1837.i.postload1840.i_crit_edge

postload1837.i.postload1840.i_crit_edge:          ; preds = %postload1837.i
  br label %postload1840.i

preload1839.i:                                    ; preds = %postload1837.i
  %"&(pSB[currWI].offset)3810.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3811.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3810.i"
  %CastToValueType3812.i = bitcast i8* %"&pSB[currWI].offset3811.i" to i64*
  %loadedValue3813.i = load i64* %CastToValueType3812.i, align 8
  %.sum2681.i = add i64 %loadedValue3813.i, 1
  %379 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2681.i
  %vload1509.i = load double addrspace(3)* %379, align 8
  br label %postload1840.i

postload1840.i:                                   ; preds = %postload1837.i.postload1840.i_crit_edge, %preload1839.i
  %phi1841.i = phi double [ %vload1509.i, %preload1839.i ], [ undef, %postload1837.i.postload1840.i_crit_edge ]
  %"&(pSB[currWI].offset)4233.i" = add nuw i64 %CurrSBIndex..12.i, 234
  %"&pSB[currWI].offset4234.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4233.i"
  %CastToValueType4235.i = bitcast i8* %"&pSB[currWI].offset4234.i" to i1*
  %loadedValue4236.i = load i1* %CastToValueType4235.i, align 1
  br i1 %loadedValue4236.i, label %preload1842.i, label %postload1840.i.postload1843.i_crit_edge

postload1840.i.postload1843.i_crit_edge:          ; preds = %postload1840.i
  br label %postload1843.i

preload1842.i:                                    ; preds = %postload1840.i
  %"&(pSB[currWI].offset)3815.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3816.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3815.i"
  %CastToValueType3817.i = bitcast i8* %"&pSB[currWI].offset3816.i" to i64*
  %loadedValue3818.i = load i64* %CastToValueType3817.i, align 8
  %.sum2680.i = add i64 %loadedValue3818.i, 2
  %380 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2680.i
  %vload1513.i = load double addrspace(3)* %380, align 8
  br label %postload1843.i

postload1843.i:                                   ; preds = %postload1840.i.postload1843.i_crit_edge, %preload1842.i
  %phi1844.i = phi double [ %vload1513.i, %preload1842.i ], [ undef, %postload1840.i.postload1843.i_crit_edge ]
  %"&(pSB[currWI].offset)4257.i" = add nuw i64 %CurrSBIndex..12.i, 235
  %"&pSB[currWI].offset4258.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4257.i"
  %CastToValueType4259.i = bitcast i8* %"&pSB[currWI].offset4258.i" to i1*
  %loadedValue4260.i = load i1* %CastToValueType4259.i, align 1
  br i1 %loadedValue4260.i, label %preload1845.i, label %postload1843.i.postload1846.i_crit_edge

postload1843.i.postload1846.i_crit_edge:          ; preds = %postload1843.i
  br label %postload1846.i

preload1845.i:                                    ; preds = %postload1843.i
  %"&(pSB[currWI].offset)3820.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3821.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3820.i"
  %CastToValueType3822.i = bitcast i8* %"&pSB[currWI].offset3821.i" to i64*
  %loadedValue3823.i = load i64* %CastToValueType3822.i, align 8
  %.sum2679.i = add i64 %loadedValue3823.i, 3
  %381 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2679.i
  %vload1517.i = load double addrspace(3)* %381, align 8
  br label %postload1846.i

postload1846.i:                                   ; preds = %postload1843.i.postload1846.i_crit_edge, %preload1845.i
  %phi1847.i = phi double [ %vload1517.i, %preload1845.i ], [ undef, %postload1843.i.postload1846.i_crit_edge ]
  %"&(pSB[currWI].offset)4281.i" = add nuw i64 %CurrSBIndex..12.i, 236
  %"&pSB[currWI].offset4282.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4281.i"
  %CastToValueType4283.i = bitcast i8* %"&pSB[currWI].offset4282.i" to i1*
  %loadedValue4284.i = load i1* %CastToValueType4283.i, align 1
  br i1 %loadedValue4284.i, label %preload2409.i, label %postload1846.i.postload2410.i_crit_edge

postload1846.i.postload2410.i_crit_edge:          ; preds = %postload1846.i
  br label %postload2410.i

preload2409.i:                                    ; preds = %postload1846.i
  %"&(pSB[currWI].offset)3825.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3826.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3825.i"
  %CastToValueType3827.i = bitcast i8* %"&pSB[currWI].offset3826.i" to i64*
  %loadedValue3828.i = load i64* %CastToValueType3827.i, align 8
  %.sum2678.i = add i64 %loadedValue3828.i, 4
  %382 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2678.i
  %vload1521.i = load double addrspace(3)* %382, align 8
  br label %postload2410.i

postload2410.i:                                   ; preds = %postload1846.i.postload2410.i_crit_edge, %preload2409.i
  %phi2411.i = phi double [ %vload1521.i, %preload2409.i ], [ undef, %postload1846.i.postload2410.i_crit_edge ]
  %"&(pSB[currWI].offset)4305.i" = add nuw i64 %CurrSBIndex..12.i, 237
  %"&pSB[currWI].offset4306.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4305.i"
  %CastToValueType4307.i = bitcast i8* %"&pSB[currWI].offset4306.i" to i1*
  %loadedValue4308.i = load i1* %CastToValueType4307.i, align 1
  br i1 %loadedValue4308.i, label %preload2412.i, label %postload2410.i.postload2413.i_crit_edge

postload2410.i.postload2413.i_crit_edge:          ; preds = %postload2410.i
  br label %postload2413.i

preload2412.i:                                    ; preds = %postload2410.i
  %"&(pSB[currWI].offset)3830.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3831.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3830.i"
  %CastToValueType3832.i = bitcast i8* %"&pSB[currWI].offset3831.i" to i64*
  %loadedValue3833.i = load i64* %CastToValueType3832.i, align 8
  %.sum2677.i = add i64 %loadedValue3833.i, 5
  %383 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2677.i
  %vload1525.i = load double addrspace(3)* %383, align 8
  br label %postload2413.i

postload2413.i:                                   ; preds = %postload2410.i.postload2413.i_crit_edge, %preload2412.i
  %phi2414.i = phi double [ %vload1525.i, %preload2412.i ], [ undef, %postload2410.i.postload2413.i_crit_edge ]
  %"&(pSB[currWI].offset)4329.i" = add nuw i64 %CurrSBIndex..12.i, 238
  %"&pSB[currWI].offset4330.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4329.i"
  %CastToValueType4331.i = bitcast i8* %"&pSB[currWI].offset4330.i" to i1*
  %loadedValue4332.i = load i1* %CastToValueType4331.i, align 1
  br i1 %loadedValue4332.i, label %preload2415.i, label %postload2413.i.postload2416.i_crit_edge

postload2413.i.postload2416.i_crit_edge:          ; preds = %postload2413.i
  br label %postload2416.i

preload2415.i:                                    ; preds = %postload2413.i
  %"&(pSB[currWI].offset)3835.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3836.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3835.i"
  %CastToValueType3837.i = bitcast i8* %"&pSB[currWI].offset3836.i" to i64*
  %loadedValue3838.i = load i64* %CastToValueType3837.i, align 8
  %.sum2676.i = add i64 %loadedValue3838.i, 6
  %384 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2676.i
  %vload1529.i = load double addrspace(3)* %384, align 8
  br label %postload2416.i

postload2416.i:                                   ; preds = %postload2413.i.postload2416.i_crit_edge, %preload2415.i
  %phi2417.i = phi double [ %vload1529.i, %preload2415.i ], [ undef, %postload2413.i.postload2416.i_crit_edge ]
  %"&(pSB[currWI].offset)4353.i" = add nuw i64 %CurrSBIndex..12.i, 239
  %"&pSB[currWI].offset4354.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4353.i"
  %CastToValueType4355.i = bitcast i8* %"&pSB[currWI].offset4354.i" to i1*
  %loadedValue4356.i = load i1* %CastToValueType4355.i, align 1
  br i1 %loadedValue4356.i, label %preload2418.i, label %postload2416.i.postload2419.i_crit_edge

postload2416.i.postload2419.i_crit_edge:          ; preds = %postload2416.i
  br label %postload2419.i

preload2418.i:                                    ; preds = %postload2416.i
  %"&(pSB[currWI].offset)3840.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3841.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3840.i"
  %CastToValueType3842.i = bitcast i8* %"&pSB[currWI].offset3841.i" to i64*
  %loadedValue3843.i = load i64* %CastToValueType3842.i, align 8
  %.sum2675.i = add i64 %loadedValue3843.i, 7
  %385 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2675.i
  %vload1533.i = load double addrspace(3)* %385, align 8
  br label %postload2419.i

postload2419.i:                                   ; preds = %postload2416.i.postload2419.i_crit_edge, %preload2418.i
  %phi2420.i = phi double [ %vload1533.i, %preload2418.i ], [ undef, %postload2416.i.postload2419.i_crit_edge ]
  %"&(pSB[currWI].offset)4377.i" = add nuw i64 %CurrSBIndex..12.i, 240
  %"&pSB[currWI].offset4378.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4377.i"
  %CastToValueType4379.i = bitcast i8* %"&pSB[currWI].offset4378.i" to i1*
  %loadedValue4380.i = load i1* %CastToValueType4379.i, align 1
  br i1 %loadedValue4380.i, label %preload2421.i, label %postload2419.i.postload2422.i_crit_edge

postload2419.i.postload2422.i_crit_edge:          ; preds = %postload2419.i
  br label %postload2422.i

preload2421.i:                                    ; preds = %postload2419.i
  %"&(pSB[currWI].offset)3845.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3846.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3845.i"
  %CastToValueType3847.i = bitcast i8* %"&pSB[currWI].offset3846.i" to i64*
  %loadedValue3848.i = load i64* %CastToValueType3847.i, align 8
  %.sum2674.i = add i64 %loadedValue3848.i, 8
  %386 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2674.i
  %vload1537.i = load double addrspace(3)* %386, align 8
  br label %postload2422.i

postload2422.i:                                   ; preds = %postload2419.i.postload2422.i_crit_edge, %preload2421.i
  %phi2423.i = phi double [ %vload1537.i, %preload2421.i ], [ undef, %postload2419.i.postload2422.i_crit_edge ]
  %"&(pSB[currWI].offset)4401.i" = add nuw i64 %CurrSBIndex..12.i, 241
  %"&pSB[currWI].offset4402.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4401.i"
  %CastToValueType4403.i = bitcast i8* %"&pSB[currWI].offset4402.i" to i1*
  %loadedValue4404.i = load i1* %CastToValueType4403.i, align 1
  br i1 %loadedValue4404.i, label %preload1752.i, label %postload2422.i.postload1753.i_crit_edge

postload2422.i.postload1753.i_crit_edge:          ; preds = %postload2422.i
  br label %postload1753.i

preload1752.i:                                    ; preds = %postload2422.i
  %"&(pSB[currWI].offset)3850.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3851.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3850.i"
  %CastToValueType3852.i = bitcast i8* %"&pSB[currWI].offset3851.i" to i64*
  %loadedValue3853.i = load i64* %CastToValueType3852.i, align 8
  %.sum2673.i = add i64 %loadedValue3853.i, 9
  %387 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2673.i
  %vload1541.i = load double addrspace(3)* %387, align 8
  br label %postload1753.i

postload1753.i:                                   ; preds = %postload2422.i.postload1753.i_crit_edge, %preload1752.i
  %phi1754.i = phi double [ %vload1541.i, %preload1752.i ], [ undef, %postload2422.i.postload1753.i_crit_edge ]
  %"&(pSB[currWI].offset)4425.i" = add nuw i64 %CurrSBIndex..12.i, 242
  %"&pSB[currWI].offset4426.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4425.i"
  %CastToValueType4427.i = bitcast i8* %"&pSB[currWI].offset4426.i" to i1*
  %loadedValue4428.i = load i1* %CastToValueType4427.i, align 1
  br i1 %loadedValue4428.i, label %preload1755.i, label %postload1753.i.postload1756.i_crit_edge

postload1753.i.postload1756.i_crit_edge:          ; preds = %postload1753.i
  br label %postload1756.i

preload1755.i:                                    ; preds = %postload1753.i
  %"&(pSB[currWI].offset)3855.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3856.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3855.i"
  %CastToValueType3857.i = bitcast i8* %"&pSB[currWI].offset3856.i" to i64*
  %loadedValue3858.i = load i64* %CastToValueType3857.i, align 8
  %.sum2672.i = add i64 %loadedValue3858.i, 10
  %388 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2672.i
  %vload1545.i = load double addrspace(3)* %388, align 8
  br label %postload1756.i

postload1756.i:                                   ; preds = %postload1753.i.postload1756.i_crit_edge, %preload1755.i
  %phi1757.i = phi double [ %vload1545.i, %preload1755.i ], [ undef, %postload1753.i.postload1756.i_crit_edge ]
  %"&(pSB[currWI].offset)4449.i" = add nuw i64 %CurrSBIndex..12.i, 243
  %"&pSB[currWI].offset4450.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4449.i"
  %CastToValueType4451.i = bitcast i8* %"&pSB[currWI].offset4450.i" to i1*
  %loadedValue4452.i = load i1* %CastToValueType4451.i, align 1
  br i1 %loadedValue4452.i, label %preload1758.i, label %postload1756.i.postload1759.i_crit_edge

postload1756.i.postload1759.i_crit_edge:          ; preds = %postload1756.i
  br label %postload1759.i

preload1758.i:                                    ; preds = %postload1756.i
  %"&(pSB[currWI].offset)3860.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3861.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3860.i"
  %CastToValueType3862.i = bitcast i8* %"&pSB[currWI].offset3861.i" to i64*
  %loadedValue3863.i = load i64* %CastToValueType3862.i, align 8
  %.sum2671.i = add i64 %loadedValue3863.i, 11
  %389 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2671.i
  %vload1549.i = load double addrspace(3)* %389, align 8
  br label %postload1759.i

postload1759.i:                                   ; preds = %postload1756.i.postload1759.i_crit_edge, %preload1758.i
  %phi1760.i = phi double [ %vload1549.i, %preload1758.i ], [ undef, %postload1756.i.postload1759.i_crit_edge ]
  %"&(pSB[currWI].offset)4473.i" = add nuw i64 %CurrSBIndex..12.i, 244
  %"&pSB[currWI].offset4474.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4473.i"
  %CastToValueType4475.i = bitcast i8* %"&pSB[currWI].offset4474.i" to i1*
  %loadedValue4476.i = load i1* %CastToValueType4475.i, align 1
  br i1 %loadedValue4476.i, label %preload1761.i, label %postload1759.i.postload1762.i_crit_edge

postload1759.i.postload1762.i_crit_edge:          ; preds = %postload1759.i
  br label %postload1762.i

preload1761.i:                                    ; preds = %postload1759.i
  %"&(pSB[currWI].offset)3865.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3866.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3865.i"
  %CastToValueType3867.i = bitcast i8* %"&pSB[currWI].offset3866.i" to i64*
  %loadedValue3868.i = load i64* %CastToValueType3867.i, align 8
  %.sum2670.i = add i64 %loadedValue3868.i, 12
  %390 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2670.i
  %vload1553.i = load double addrspace(3)* %390, align 8
  br label %postload1762.i

postload1762.i:                                   ; preds = %postload1759.i.postload1762.i_crit_edge, %preload1761.i
  %phi1763.i = phi double [ %vload1553.i, %preload1761.i ], [ undef, %postload1759.i.postload1762.i_crit_edge ]
  %"&(pSB[currWI].offset)4497.i" = add nuw i64 %CurrSBIndex..12.i, 245
  %"&pSB[currWI].offset4498.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4497.i"
  %CastToValueType4499.i = bitcast i8* %"&pSB[currWI].offset4498.i" to i1*
  %loadedValue4500.i = load i1* %CastToValueType4499.i, align 1
  br i1 %loadedValue4500.i, label %preload2298.i, label %postload1762.i.postload2299.i_crit_edge

postload1762.i.postload2299.i_crit_edge:          ; preds = %postload1762.i
  br label %postload2299.i

preload2298.i:                                    ; preds = %postload1762.i
  %"&(pSB[currWI].offset)3870.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3871.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3870.i"
  %CastToValueType3872.i = bitcast i8* %"&pSB[currWI].offset3871.i" to i64*
  %loadedValue3873.i = load i64* %CastToValueType3872.i, align 8
  %.sum2669.i = add i64 %loadedValue3873.i, 13
  %391 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2669.i
  %vload1557.i = load double addrspace(3)* %391, align 8
  br label %postload2299.i

postload2299.i:                                   ; preds = %postload1762.i.postload2299.i_crit_edge, %preload2298.i
  %phi2300.i = phi double [ %vload1557.i, %preload2298.i ], [ undef, %postload1762.i.postload2299.i_crit_edge ]
  %"&(pSB[currWI].offset)4521.i" = add nuw i64 %CurrSBIndex..12.i, 246
  %"&pSB[currWI].offset4522.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4521.i"
  %CastToValueType4523.i = bitcast i8* %"&pSB[currWI].offset4522.i" to i1*
  %loadedValue4524.i = load i1* %CastToValueType4523.i, align 1
  br i1 %loadedValue4524.i, label %preload2301.i, label %postload2299.i.postload2302.i_crit_edge

postload2299.i.postload2302.i_crit_edge:          ; preds = %postload2299.i
  br label %postload2302.i

preload2301.i:                                    ; preds = %postload2299.i
  %"&(pSB[currWI].offset)3875.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3876.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3875.i"
  %CastToValueType3877.i = bitcast i8* %"&pSB[currWI].offset3876.i" to i64*
  %loadedValue3878.i = load i64* %CastToValueType3877.i, align 8
  %.sum2668.i = add i64 %loadedValue3878.i, 14
  %392 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum2668.i
  %vload1561.i = load double addrspace(3)* %392, align 8
  br label %postload2302.i

postload2302.i:                                   ; preds = %postload2299.i.postload2302.i_crit_edge, %preload2301.i
  %phi2303.i = phi double [ %vload1561.i, %preload2301.i ], [ undef, %postload2299.i.postload2302.i_crit_edge ]
  %"&(pSB[currWI].offset)4545.i" = add nuw i64 %CurrSBIndex..12.i, 247
  %"&pSB[currWI].offset4546.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4545.i"
  %CastToValueType4547.i = bitcast i8* %"&pSB[currWI].offset4546.i" to i1*
  %loadedValue4548.i = load i1* %CastToValueType4547.i, align 1
  br i1 %loadedValue4548.i, label %preload2304.i, label %postload2305.i

preload2304.i:                                    ; preds = %postload2302.i
  %"&(pSB[currWI].offset)3880.i" = add nuw i64 %CurrSBIndex..12.i, 72
  %"&pSB[currWI].offset3881.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)3880.i"
  %CastToValueType3882.i = bitcast i8* %"&pSB[currWI].offset3881.i" to i64*
  %loadedValue3883.i = load i64* %CastToValueType3882.i, align 8
  %.sum.i = add i64 %loadedValue3883.i, 15
  %393 = getelementptr [128 x double] addrspace(3)* %35, i64 0, i64 %.sum.i
  %vload1565.i = load double addrspace(3)* %393, align 8
  br label %postload2305.i

postload2305.i:                                   ; preds = %preload2304.i, %postload2302.i
  %phi2306.i = phi double [ undef, %postload2302.i ], [ %vload1565.i, %preload2304.i ]
  %"&(pSB[currWI].offset)4050.i" = add nuw i64 %CurrSBIndex..12.i, 112
  %"&pSB[currWI].offset4051.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4050.i"
  %CastToValueType4052.i = bitcast i8* %"&pSB[currWI].offset4051.i" to i64*
  %loadedValue4053.i = load i64* %CastToValueType4052.i, align 8
  %394 = getelementptr inbounds double addrspace(1)* %16, i64 %loadedValue4053.i
  %"&(pSB[currWI].offset)4059.i" = add nuw i64 %CurrSBIndex..12.i, 120
  %"&pSB[currWI].offset4060.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4059.i"
  %CastToValueType4061.i = bitcast i8* %"&pSB[currWI].offset4060.i" to i64*
  %loadedValue4062.i = load i64* %CastToValueType4061.i, align 8
  %395 = getelementptr inbounds double addrspace(1)* %16, i64 %loadedValue4062.i
  %"&(pSB[currWI].offset)4068.i" = add nuw i64 %CurrSBIndex..12.i, 128
  %"&pSB[currWI].offset4069.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4068.i"
  %CastToValueType4070.i = bitcast i8* %"&pSB[currWI].offset4069.i" to i64*
  %loadedValue4071.i = load i64* %CastToValueType4070.i, align 8
  %396 = getelementptr inbounds double addrspace(1)* %16, i64 %loadedValue4071.i
  %"&(pSB[currWI].offset)4077.i" = add nuw i64 %CurrSBIndex..12.i, 136
  %"&pSB[currWI].offset4078.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4077.i"
  %CastToValueType4079.i = bitcast i8* %"&pSB[currWI].offset4078.i" to i64*
  %loadedValue4080.i = load i64* %CastToValueType4079.i, align 8
  %397 = getelementptr inbounds double addrspace(1)* %16, i64 %loadedValue4080.i
  %"&(pSB[currWI].offset)4086.i" = add nuw i64 %CurrSBIndex..12.i, 144
  %"&pSB[currWI].offset4087.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4086.i"
  %CastToValueType4088.i = bitcast i8* %"&pSB[currWI].offset4087.i" to i64*
  %loadedValue4089.i = load i64* %CastToValueType4088.i, align 8
  %398 = getelementptr inbounds double addrspace(1)* %16, i64 %loadedValue4089.i
  %"&(pSB[currWI].offset)4095.i" = add nuw i64 %CurrSBIndex..12.i, 152
  %"&pSB[currWI].offset4096.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4095.i"
  %CastToValueType4097.i = bitcast i8* %"&pSB[currWI].offset4096.i" to i64*
  %loadedValue4098.i = load i64* %CastToValueType4097.i, align 8
  %399 = getelementptr inbounds double addrspace(1)* %16, i64 %loadedValue4098.i
  %"&(pSB[currWI].offset)4104.i" = add nuw i64 %CurrSBIndex..12.i, 160
  %"&pSB[currWI].offset4105.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4104.i"
  %CastToValueType4106.i = bitcast i8* %"&pSB[currWI].offset4105.i" to i64*
  %loadedValue4107.i = load i64* %CastToValueType4106.i, align 8
  %400 = getelementptr inbounds double addrspace(1)* %16, i64 %loadedValue4107.i
  %"&(pSB[currWI].offset)4113.i" = add nuw i64 %CurrSBIndex..12.i, 168
  %"&pSB[currWI].offset4114.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4113.i"
  %CastToValueType4115.i = bitcast i8* %"&pSB[currWI].offset4114.i" to i64*
  %loadedValue4116.i = load i64* %CastToValueType4115.i, align 8
  %401 = getelementptr inbounds double addrspace(1)* %16, i64 %loadedValue4116.i
  %"&(pSB[currWI].offset)4122.i" = add nuw i64 %CurrSBIndex..12.i, 176
  %"&pSB[currWI].offset4123.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4122.i"
  %CastToValueType4124.i = bitcast i8* %"&pSB[currWI].offset4123.i" to i64*
  %loadedValue4125.i = load i64* %CastToValueType4124.i, align 8
  %402 = getelementptr inbounds double addrspace(1)* %16, i64 %loadedValue4125.i
  %"&(pSB[currWI].offset)4131.i" = add nuw i64 %CurrSBIndex..12.i, 184
  %"&pSB[currWI].offset4132.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4131.i"
  %CastToValueType4133.i = bitcast i8* %"&pSB[currWI].offset4132.i" to i64*
  %loadedValue4134.i = load i64* %CastToValueType4133.i, align 8
  %403 = getelementptr inbounds double addrspace(1)* %16, i64 %loadedValue4134.i
  %"&(pSB[currWI].offset)4140.i" = add nuw i64 %CurrSBIndex..12.i, 192
  %"&pSB[currWI].offset4141.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4140.i"
  %CastToValueType4142.i = bitcast i8* %"&pSB[currWI].offset4141.i" to i64*
  %loadedValue4143.i = load i64* %CastToValueType4142.i, align 8
  %404 = getelementptr inbounds double addrspace(1)* %16, i64 %loadedValue4143.i
  %"&(pSB[currWI].offset)4149.i" = add nuw i64 %CurrSBIndex..12.i, 200
  %"&pSB[currWI].offset4150.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4149.i"
  %CastToValueType4151.i = bitcast i8* %"&pSB[currWI].offset4150.i" to i64*
  %loadedValue4152.i = load i64* %CastToValueType4151.i, align 8
  %405 = getelementptr inbounds double addrspace(1)* %16, i64 %loadedValue4152.i
  %"&(pSB[currWI].offset)4158.i" = add nuw i64 %CurrSBIndex..12.i, 208
  %"&pSB[currWI].offset4159.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4158.i"
  %CastToValueType4160.i = bitcast i8* %"&pSB[currWI].offset4159.i" to i64*
  %loadedValue4161.i = load i64* %CastToValueType4160.i, align 8
  %406 = getelementptr inbounds double addrspace(1)* %16, i64 %loadedValue4161.i
  %"&(pSB[currWI].offset)4167.i" = add nuw i64 %CurrSBIndex..12.i, 216
  %"&pSB[currWI].offset4168.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4167.i"
  %CastToValueType4169.i = bitcast i8* %"&pSB[currWI].offset4168.i" to i64*
  %loadedValue4170.i = load i64* %CastToValueType4169.i, align 8
  %407 = getelementptr inbounds double addrspace(1)* %16, i64 %loadedValue4170.i
  %"&(pSB[currWI].offset)4176.i" = add nuw i64 %CurrSBIndex..12.i, 224
  %"&pSB[currWI].offset4177.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4176.i"
  %CastToValueType4178.i = bitcast i8* %"&pSB[currWI].offset4177.i" to i64*
  %loadedValue4179.i = load i64* %CastToValueType4178.i, align 8
  %408 = getelementptr inbounds double addrspace(1)* %16, i64 %loadedValue4179.i
  br i1 %loadedValue4188.i, label %preload1646.i, label %postload1647.i

preload1646.i:                                    ; preds = %postload2305.i
  %"&(pSB[currWI].offset)4036.i" = add nuw i64 %CurrSBIndex..12.i, 104
  %"&pSB[currWI].offset4037.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)4036.i"
  %CastToValueType4038.i = bitcast i8* %"&pSB[currWI].offset4037.i" to i64*
  %loadedValue4039.i = load i64* %CastToValueType4038.i, align 8
  %409 = getelementptr inbounds double addrspace(1)* %16, i64 %loadedValue4039.i
  store double %phi1838.i, double addrspace(1)* %409, align 8
  br label %postload1647.i

postload1647.i:                                   ; preds = %preload1646.i, %postload2305.i
  br i1 %loadedValue4212.i, label %preload1624.i, label %postload1625.i

preload1624.i:                                    ; preds = %postload1647.i
  store double %phi1841.i, double addrspace(1)* %394, align 8
  br label %postload1625.i

postload1625.i:                                   ; preds = %preload1624.i, %postload1647.i
  br i1 %loadedValue4236.i, label %preload1626.i, label %postload1627.i

preload1626.i:                                    ; preds = %postload1625.i
  store double %phi1844.i, double addrspace(1)* %395, align 8
  br label %postload1627.i

postload1627.i:                                   ; preds = %preload1626.i, %postload1625.i
  br i1 %loadedValue4260.i, label %preload1628.i, label %postload1629.i

preload1628.i:                                    ; preds = %postload1627.i
  store double %phi1847.i, double addrspace(1)* %396, align 8
  br label %postload1629.i

postload1629.i:                                   ; preds = %preload1628.i, %postload1627.i
  br i1 %loadedValue4284.i, label %preload1630.i, label %postload1631.i

preload1630.i:                                    ; preds = %postload1629.i
  store double %phi2411.i, double addrspace(1)* %397, align 8
  br label %postload1631.i

postload1631.i:                                   ; preds = %preload1630.i, %postload1629.i
  br i1 %loadedValue4308.i, label %preload1632.i, label %postload1633.i

preload1632.i:                                    ; preds = %postload1631.i
  store double %phi2414.i, double addrspace(1)* %398, align 8
  br label %postload1633.i

postload1633.i:                                   ; preds = %preload1632.i, %postload1631.i
  br i1 %loadedValue4332.i, label %preload1648.i, label %postload1649.i

preload1648.i:                                    ; preds = %postload1633.i
  store double %phi2417.i, double addrspace(1)* %399, align 8
  br label %postload1649.i

postload1649.i:                                   ; preds = %preload1648.i, %postload1633.i
  br i1 %loadedValue4356.i, label %preload1866.i, label %postload1867.i

preload1866.i:                                    ; preds = %postload1649.i
  store double %phi2420.i, double addrspace(1)* %400, align 8
  br label %postload1867.i

postload1867.i:                                   ; preds = %preload1866.i, %postload1649.i
  br i1 %loadedValue4380.i, label %preload1868.i, label %postload1869.i

preload1868.i:                                    ; preds = %postload1867.i
  store double %phi2423.i, double addrspace(1)* %401, align 8
  br label %postload1869.i

postload1869.i:                                   ; preds = %preload1868.i, %postload1867.i
  br i1 %loadedValue4404.i, label %preload1870.i, label %postload1871.i

preload1870.i:                                    ; preds = %postload1869.i
  store double %phi1754.i, double addrspace(1)* %402, align 8
  br label %postload1871.i

postload1871.i:                                   ; preds = %preload1870.i, %postload1869.i
  br i1 %loadedValue4428.i, label %preload1872.i, label %postload1873.i

preload1872.i:                                    ; preds = %postload1871.i
  store double %phi1757.i, double addrspace(1)* %403, align 8
  br label %postload1873.i

postload1873.i:                                   ; preds = %preload1872.i, %postload1871.i
  br i1 %loadedValue4452.i, label %preload1874.i, label %postload1875.i

preload1874.i:                                    ; preds = %postload1873.i
  store double %phi1760.i, double addrspace(1)* %404, align 8
  br label %postload1875.i

postload1875.i:                                   ; preds = %preload1874.i, %postload1873.i
  br i1 %loadedValue4476.i, label %preload1876.i, label %postload1877.i

preload1876.i:                                    ; preds = %postload1875.i
  store double %phi1763.i, double addrspace(1)* %405, align 8
  br label %postload1877.i

postload1877.i:                                   ; preds = %preload1876.i, %postload1875.i
  br i1 %loadedValue4500.i, label %preload1878.i, label %postload1879.i

preload1878.i:                                    ; preds = %postload1877.i
  store double %phi2300.i, double addrspace(1)* %406, align 8
  br label %postload1879.i

postload1879.i:                                   ; preds = %preload1878.i, %postload1877.i
  br i1 %loadedValue4524.i, label %preload1880.i, label %postload1881.i

preload1880.i:                                    ; preds = %postload1879.i
  store double %phi2303.i, double addrspace(1)* %407, align 8
  br label %postload1881.i

postload1881.i:                                   ; preds = %preload1880.i, %postload1879.i
  br i1 %loadedValue4548.i, label %preload1882.i, label %if.end85.i

preload1882.i:                                    ; preds = %postload1881.i
  store double %phi2306.i, double addrspace(1)* %408, align 8
  br label %if.end85.i

if.end85.i:                                       ; preds = %preload1882.i, %postload1881.i
  %check.WI.iter4606.i = icmp ult i64 %CurrWI..12.i, %31
  br i1 %check.WI.iter4606.i, label %thenBB4603.i, label %____Vectorized_.spmv_csr_vector_kernel_separated_args.exit

thenBB4603.i:                                     ; preds = %if.end85.i
  %"CurrWI++4607.i" = add nuw i64 %CurrWI..12.i, 1
  %"loadedCurrSB+Stride4609.i" = add nuw i64 %CurrSBIndex..12.i, 256
  switch i32 %currBarrier.12.i, label %thenBB4603.i.postload2522.i_crit_edge [
    i32 3, label %thenBB4603.i.postload2520.i_crit_edge
    i32 2, label %thenBB4603.i.postload2518.i_crit_edge
    i32 5, label %thenBB4603.i.postload2516.i_crit_edge
    i32 13, label %thenBB4603.i.postload2514.i_crit_edge
    i32 8, label %thenBB4603.i.postload2512.i_crit_edge
    i32 14, label %SyncBB.outer.i
  ]

thenBB4603.i.postload2512.i_crit_edge:            ; preds = %thenBB4603.i
  br label %postload2512.i

thenBB4603.i.postload2514.i_crit_edge:            ; preds = %thenBB4603.i
  br label %postload2514.i

thenBB4603.i.postload2516.i_crit_edge:            ; preds = %thenBB4603.i
  br label %postload2516.i

thenBB4603.i.postload2518.i_crit_edge:            ; preds = %thenBB4603.i
  br label %postload2518.i

thenBB4603.i.postload2520.i_crit_edge:            ; preds = %thenBB4603.i
  br label %postload2520.i

thenBB4603.i.postload2522.i_crit_edge:            ; preds = %thenBB4603.i
  br label %postload2522.i

____Vectorized_.spmv_csr_vector_kernel_separated_args.exit: ; preds = %if.end85.i
  ret void
}

!opencl.kernels = !{!0, !2, !3}
!opencl.build.options = !{!4}
!cl.noBarrierPath.kernels = !{!5}
!opencl.wrappers = !{!6, !7, !8}

!0 = metadata !{void (double addrspace(1)*, double addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, double addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__spmv_csr_scalar_kernel_separated_args, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3, i32 3, i32 3}
!2 = metadata !{void (double addrspace(1)*, double addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, double addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__spmv_csr_vector_kernel_separated_args, metadata !1}
!3 = metadata !{void (double addrspace(1)*, double addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, double addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__spmv_ellpackr_kernel_separated_args, metadata !1}
!4 = metadata !{}
!5 = metadata !{metadata !"spmv_csr_scalar_kernel", metadata !"spmv_ellpackr_kernel"}
!6 = metadata !{void (i8*)* @spmv_csr_scalar_kernel}
!7 = metadata !{void (i8*)* @spmv_csr_vector_kernel}
!8 = metadata !{void (i8*)* @spmv_ellpackr_kernel}
