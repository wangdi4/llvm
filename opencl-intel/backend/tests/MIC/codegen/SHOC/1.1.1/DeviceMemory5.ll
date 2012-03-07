; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @__writeGlobalMemoryUnit_original(float addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32) nounwind readnone

declare [7 x i64] @__WG.boundaries.writeGlobalMemoryUnit_original(float addrspace(1)*, i32)

declare i64 @get_local_size(i32)

declare i64 @get_base_global_id.(i32)

declare i1 @allOne(i1)

declare i1 @allZero(i1)

declare void @__writeGlobalMemoryUnit_separated_args(float addrspace(1)* nocapture, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare [7 x i64] @WG.boundaries.writeGlobalMemoryUnit(float addrspace(1)*, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)

define void @writeGlobalMemoryUnit(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32*
  %4 = load i32* %3, align 4
  %5 = getelementptr i8* %pBuffer, i64 24
  %6 = bitcast i8* %5 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %7 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 40
  %9 = bitcast i8* %8 to <{ [4 x i64] }>**
  %10 = load <{ [4 x i64] }>** %9, align 8
  %11 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %7, i64 0, i32 3, i64 0
  %12 = load i64* %11, align 8
  %13 = getelementptr <{ [4 x i64] }>* %10, i64 0, i32 0, i64 0
  %14 = load i64* %13, align 8
  %15 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %7, i64 0, i32 3, i64 1
  %16 = load i64* %15, align 8
  %17 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %7, i64 0, i32 3, i64 2
  %18 = load i64* %17, align 8
  %vector.size.i = ashr i64 %12, 4
  %num.vector.wi.i = and i64 %12, -16
  %max.vector.gid.i = add i64 %num.vector.wi.i, %14
  %scalar.size.i = sub i64 %12, %num.vector.wi.i
  %19 = icmp eq i64 %vector.size.i, 0
  br i1 %19, label %scalarIf.i, label %dim_2_vector_pre_head.i

dim_2_vector_pre_head.i:                          ; preds = %entry
  %subvector_func.i = add nsw i32 %4, -1
  %tempvector_func.i = insertelement <16 x i32> undef, i32 %subvector_func.i, i32 0
  %vectorvector_func.i = shufflevector <16 x i32> %tempvector_func.i, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %dim_1_vector_pre_head.i

dim_1_vector_pre_head.i:                          ; preds = %dim_1_vector_exit.i, %dim_2_vector_pre_head.i
  %dim_2_vector_ind_var.i = phi i64 [ 0, %dim_2_vector_pre_head.i ], [ %dim_2_vector_inc_ind_var.i, %dim_1_vector_exit.i ]
  br label %dim_0_vector_pre_head.i

dim_0_vector_pre_head.i:                          ; preds = %dim_0_vector_exit.i, %dim_1_vector_pre_head.i
  %dim_1_vector_ind_var.i = phi i64 [ 0, %dim_1_vector_pre_head.i ], [ %dim_1_vector_inc_ind_var.i, %dim_0_vector_exit.i ]
  br label %entryvector_func.i

entryvector_func.i:                               ; preds = %for.endvector_func.i, %dim_0_vector_pre_head.i
  %dim_0_vector_ind_var.i = phi i64 [ 0, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_ind_var.i, %for.endvector_func.i ]
  %dim_0_vector_tid.i = phi i64 [ %14, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_tid.i, %for.endvector_func.i ]
  %broadcast1vector_func.i = insertelement <16 x i64> undef, i64 %dim_0_vector_tid.i, i32 0
  %broadcast2vector_func.i = shufflevector <16 x i64> %broadcast1vector_func.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %20 = add <16 x i64> %broadcast2vector_func.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %conv1vector_func.i = trunc <16 x i64> %20 to <16 x i32>
  %mul2vector_func.i = shl <16 x i32> %conv1vector_func.i, <i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10>
  %conv63vector_func.i = sitofp <16 x i32> %conv1vector_func.i to <16 x float>
  %extract21vector_func.i = extractelement <16 x float> %conv63vector_func.i, i32 0
  %extract22vector_func.i = extractelement <16 x float> %conv63vector_func.i, i32 1
  %extract23vector_func.i = extractelement <16 x float> %conv63vector_func.i, i32 2
  %extract24vector_func.i = extractelement <16 x float> %conv63vector_func.i, i32 3
  %extract25vector_func.i = extractelement <16 x float> %conv63vector_func.i, i32 4
  %extract26vector_func.i = extractelement <16 x float> %conv63vector_func.i, i32 5
  %extract27vector_func.i = extractelement <16 x float> %conv63vector_func.i, i32 6
  %extract28vector_func.i = extractelement <16 x float> %conv63vector_func.i, i32 7
  %extract29vector_func.i = extractelement <16 x float> %conv63vector_func.i, i32 8
  %extract30vector_func.i = extractelement <16 x float> %conv63vector_func.i, i32 9
  %extract31vector_func.i = extractelement <16 x float> %conv63vector_func.i, i32 10
  %extract32vector_func.i = extractelement <16 x float> %conv63vector_func.i, i32 11
  %extract33vector_func.i = extractelement <16 x float> %conv63vector_func.i, i32 12
  %extract34vector_func.i = extractelement <16 x float> %conv63vector_func.i, i32 13
  %extract35vector_func.i = extractelement <16 x float> %conv63vector_func.i, i32 14
  %extract36vector_func.i = extractelement <16 x float> %conv63vector_func.i, i32 15
  br label %for.bodyvector_func.i

for.bodyvector_func.i:                            ; preds = %for.bodyvector_func.i, %entryvector_func.i
  %vectorPHIvector_func.i = phi <16 x i32> [ %mul2vector_func.i, %entryvector_func.i ], [ %and99323vector_func.i, %for.bodyvector_func.i ]
  %j.01vector_func.i = phi i32 [ 0, %entryvector_func.i ], [ %incvector_func.i, %for.bodyvector_func.i ]
  %and4vector_func.i = and <16 x i32> %vectorPHIvector_func.i, %vectorvector_func.i
  %idxprom5vector_func.i = sext <16 x i32> %and4vector_func.i to <16 x i64>
  %extractvector_func.i = extractelement <16 x i64> %idxprom5vector_func.i, i32 0
  %extract6vector_func.i = extractelement <16 x i64> %idxprom5vector_func.i, i32 1
  %extract7vector_func.i = extractelement <16 x i64> %idxprom5vector_func.i, i32 2
  %extract8vector_func.i = extractelement <16 x i64> %idxprom5vector_func.i, i32 3
  %extract9vector_func.i = extractelement <16 x i64> %idxprom5vector_func.i, i32 4
  %extract10vector_func.i = extractelement <16 x i64> %idxprom5vector_func.i, i32 5
  %extract11vector_func.i = extractelement <16 x i64> %idxprom5vector_func.i, i32 6
  %extract12vector_func.i = extractelement <16 x i64> %idxprom5vector_func.i, i32 7
  %extract13vector_func.i = extractelement <16 x i64> %idxprom5vector_func.i, i32 8
  %extract14vector_func.i = extractelement <16 x i64> %idxprom5vector_func.i, i32 9
  %extract15vector_func.i = extractelement <16 x i64> %idxprom5vector_func.i, i32 10
  %extract16vector_func.i = extractelement <16 x i64> %idxprom5vector_func.i, i32 11
  %extract17vector_func.i = extractelement <16 x i64> %idxprom5vector_func.i, i32 12
  %extract18vector_func.i = extractelement <16 x i64> %idxprom5vector_func.i, i32 13
  %extract19vector_func.i = extractelement <16 x i64> %idxprom5vector_func.i, i32 14
  %extract20vector_func.i = extractelement <16 x i64> %idxprom5vector_func.i, i32 15
  %21 = getelementptr inbounds float addrspace(1)* %1, i64 %extractvector_func.i
  %22 = getelementptr inbounds float addrspace(1)* %1, i64 %extract6vector_func.i
  %23 = getelementptr inbounds float addrspace(1)* %1, i64 %extract7vector_func.i
  %24 = getelementptr inbounds float addrspace(1)* %1, i64 %extract8vector_func.i
  %25 = getelementptr inbounds float addrspace(1)* %1, i64 %extract9vector_func.i
  %26 = getelementptr inbounds float addrspace(1)* %1, i64 %extract10vector_func.i
  %27 = getelementptr inbounds float addrspace(1)* %1, i64 %extract11vector_func.i
  %28 = getelementptr inbounds float addrspace(1)* %1, i64 %extract12vector_func.i
  %29 = getelementptr inbounds float addrspace(1)* %1, i64 %extract13vector_func.i
  %30 = getelementptr inbounds float addrspace(1)* %1, i64 %extract14vector_func.i
  %31 = getelementptr inbounds float addrspace(1)* %1, i64 %extract15vector_func.i
  %32 = getelementptr inbounds float addrspace(1)* %1, i64 %extract16vector_func.i
  %33 = getelementptr inbounds float addrspace(1)* %1, i64 %extract17vector_func.i
  %34 = getelementptr inbounds float addrspace(1)* %1, i64 %extract18vector_func.i
  %35 = getelementptr inbounds float addrspace(1)* %1, i64 %extract19vector_func.i
  %36 = getelementptr inbounds float addrspace(1)* %1, i64 %extract20vector_func.i
  store float %extract21vector_func.i, float addrspace(1)* %21, align 4
  store float %extract22vector_func.i, float addrspace(1)* %22, align 4
  store float %extract23vector_func.i, float addrspace(1)* %23, align 4
  store float %extract24vector_func.i, float addrspace(1)* %24, align 4
  store float %extract25vector_func.i, float addrspace(1)* %25, align 4
  store float %extract26vector_func.i, float addrspace(1)* %26, align 4
  store float %extract27vector_func.i, float addrspace(1)* %27, align 4
  store float %extract28vector_func.i, float addrspace(1)* %28, align 4
  store float %extract29vector_func.i, float addrspace(1)* %29, align 4
  store float %extract30vector_func.i, float addrspace(1)* %30, align 4
  store float %extract31vector_func.i, float addrspace(1)* %31, align 4
  store float %extract32vector_func.i, float addrspace(1)* %32, align 4
  store float %extract33vector_func.i, float addrspace(1)* %33, align 4
  store float %extract34vector_func.i, float addrspace(1)* %34, align 4
  store float %extract35vector_func.i, float addrspace(1)* %35, align 4
  store float %extract36vector_func.i, float addrspace(1)* %36, align 4
  %add837vector_func.i = add nsw <16 x i32> %vectorPHIvector_func.i, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %and1038vector_func.i = and <16 x i32> %add837vector_func.i, %vectorvector_func.i
  %idxprom1139vector_func.i = sext <16 x i32> %and1038vector_func.i to <16 x i64>
  %extract40vector_func.i = extractelement <16 x i64> %idxprom1139vector_func.i, i32 0
  %extract41vector_func.i = extractelement <16 x i64> %idxprom1139vector_func.i, i32 1
  %extract42vector_func.i = extractelement <16 x i64> %idxprom1139vector_func.i, i32 2
  %extract43vector_func.i = extractelement <16 x i64> %idxprom1139vector_func.i, i32 3
  %extract44vector_func.i = extractelement <16 x i64> %idxprom1139vector_func.i, i32 4
  %extract45vector_func.i = extractelement <16 x i64> %idxprom1139vector_func.i, i32 5
  %extract46vector_func.i = extractelement <16 x i64> %idxprom1139vector_func.i, i32 6
  %extract47vector_func.i = extractelement <16 x i64> %idxprom1139vector_func.i, i32 7
  %extract48vector_func.i = extractelement <16 x i64> %idxprom1139vector_func.i, i32 8
  %extract49vector_func.i = extractelement <16 x i64> %idxprom1139vector_func.i, i32 9
  %extract50vector_func.i = extractelement <16 x i64> %idxprom1139vector_func.i, i32 10
  %extract51vector_func.i = extractelement <16 x i64> %idxprom1139vector_func.i, i32 11
  %extract52vector_func.i = extractelement <16 x i64> %idxprom1139vector_func.i, i32 12
  %extract53vector_func.i = extractelement <16 x i64> %idxprom1139vector_func.i, i32 13
  %extract54vector_func.i = extractelement <16 x i64> %idxprom1139vector_func.i, i32 14
  %extract55vector_func.i = extractelement <16 x i64> %idxprom1139vector_func.i, i32 15
  %37 = getelementptr inbounds float addrspace(1)* %1, i64 %extract40vector_func.i
  %38 = getelementptr inbounds float addrspace(1)* %1, i64 %extract41vector_func.i
  %39 = getelementptr inbounds float addrspace(1)* %1, i64 %extract42vector_func.i
  %40 = getelementptr inbounds float addrspace(1)* %1, i64 %extract43vector_func.i
  %41 = getelementptr inbounds float addrspace(1)* %1, i64 %extract44vector_func.i
  %42 = getelementptr inbounds float addrspace(1)* %1, i64 %extract45vector_func.i
  %43 = getelementptr inbounds float addrspace(1)* %1, i64 %extract46vector_func.i
  %44 = getelementptr inbounds float addrspace(1)* %1, i64 %extract47vector_func.i
  %45 = getelementptr inbounds float addrspace(1)* %1, i64 %extract48vector_func.i
  %46 = getelementptr inbounds float addrspace(1)* %1, i64 %extract49vector_func.i
  %47 = getelementptr inbounds float addrspace(1)* %1, i64 %extract50vector_func.i
  %48 = getelementptr inbounds float addrspace(1)* %1, i64 %extract51vector_func.i
  %49 = getelementptr inbounds float addrspace(1)* %1, i64 %extract52vector_func.i
  %50 = getelementptr inbounds float addrspace(1)* %1, i64 %extract53vector_func.i
  %51 = getelementptr inbounds float addrspace(1)* %1, i64 %extract54vector_func.i
  %52 = getelementptr inbounds float addrspace(1)* %1, i64 %extract55vector_func.i
  store float %extract21vector_func.i, float addrspace(1)* %37, align 4
  store float %extract22vector_func.i, float addrspace(1)* %38, align 4
  store float %extract23vector_func.i, float addrspace(1)* %39, align 4
  store float %extract24vector_func.i, float addrspace(1)* %40, align 4
  store float %extract25vector_func.i, float addrspace(1)* %41, align 4
  store float %extract26vector_func.i, float addrspace(1)* %42, align 4
  store float %extract27vector_func.i, float addrspace(1)* %43, align 4
  store float %extract28vector_func.i, float addrspace(1)* %44, align 4
  store float %extract29vector_func.i, float addrspace(1)* %45, align 4
  store float %extract30vector_func.i, float addrspace(1)* %46, align 4
  store float %extract31vector_func.i, float addrspace(1)* %47, align 4
  store float %extract32vector_func.i, float addrspace(1)* %48, align 4
  store float %extract33vector_func.i, float addrspace(1)* %49, align 4
  store float %extract34vector_func.i, float addrspace(1)* %50, align 4
  store float %extract35vector_func.i, float addrspace(1)* %51, align 4
  store float %extract36vector_func.i, float addrspace(1)* %52, align 4
  %add1456vector_func.i = add nsw <16 x i32> %vectorPHIvector_func.i, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %and1657vector_func.i = and <16 x i32> %add1456vector_func.i, %vectorvector_func.i
  %idxprom1758vector_func.i = sext <16 x i32> %and1657vector_func.i to <16 x i64>
  %extract59vector_func.i = extractelement <16 x i64> %idxprom1758vector_func.i, i32 0
  %extract60vector_func.i = extractelement <16 x i64> %idxprom1758vector_func.i, i32 1
  %extract61vector_func.i = extractelement <16 x i64> %idxprom1758vector_func.i, i32 2
  %extract62vector_func.i = extractelement <16 x i64> %idxprom1758vector_func.i, i32 3
  %extract63vector_func.i = extractelement <16 x i64> %idxprom1758vector_func.i, i32 4
  %extract64vector_func.i = extractelement <16 x i64> %idxprom1758vector_func.i, i32 5
  %extract65vector_func.i = extractelement <16 x i64> %idxprom1758vector_func.i, i32 6
  %extract66vector_func.i = extractelement <16 x i64> %idxprom1758vector_func.i, i32 7
  %extract67vector_func.i = extractelement <16 x i64> %idxprom1758vector_func.i, i32 8
  %extract68vector_func.i = extractelement <16 x i64> %idxprom1758vector_func.i, i32 9
  %extract69vector_func.i = extractelement <16 x i64> %idxprom1758vector_func.i, i32 10
  %extract70vector_func.i = extractelement <16 x i64> %idxprom1758vector_func.i, i32 11
  %extract71vector_func.i = extractelement <16 x i64> %idxprom1758vector_func.i, i32 12
  %extract72vector_func.i = extractelement <16 x i64> %idxprom1758vector_func.i, i32 13
  %extract73vector_func.i = extractelement <16 x i64> %idxprom1758vector_func.i, i32 14
  %extract74vector_func.i = extractelement <16 x i64> %idxprom1758vector_func.i, i32 15
  %53 = getelementptr inbounds float addrspace(1)* %1, i64 %extract59vector_func.i
  %54 = getelementptr inbounds float addrspace(1)* %1, i64 %extract60vector_func.i
  %55 = getelementptr inbounds float addrspace(1)* %1, i64 %extract61vector_func.i
  %56 = getelementptr inbounds float addrspace(1)* %1, i64 %extract62vector_func.i
  %57 = getelementptr inbounds float addrspace(1)* %1, i64 %extract63vector_func.i
  %58 = getelementptr inbounds float addrspace(1)* %1, i64 %extract64vector_func.i
  %59 = getelementptr inbounds float addrspace(1)* %1, i64 %extract65vector_func.i
  %60 = getelementptr inbounds float addrspace(1)* %1, i64 %extract66vector_func.i
  %61 = getelementptr inbounds float addrspace(1)* %1, i64 %extract67vector_func.i
  %62 = getelementptr inbounds float addrspace(1)* %1, i64 %extract68vector_func.i
  %63 = getelementptr inbounds float addrspace(1)* %1, i64 %extract69vector_func.i
  %64 = getelementptr inbounds float addrspace(1)* %1, i64 %extract70vector_func.i
  %65 = getelementptr inbounds float addrspace(1)* %1, i64 %extract71vector_func.i
  %66 = getelementptr inbounds float addrspace(1)* %1, i64 %extract72vector_func.i
  %67 = getelementptr inbounds float addrspace(1)* %1, i64 %extract73vector_func.i
  %68 = getelementptr inbounds float addrspace(1)* %1, i64 %extract74vector_func.i
  store float %extract21vector_func.i, float addrspace(1)* %53, align 4
  store float %extract22vector_func.i, float addrspace(1)* %54, align 4
  store float %extract23vector_func.i, float addrspace(1)* %55, align 4
  store float %extract24vector_func.i, float addrspace(1)* %56, align 4
  store float %extract25vector_func.i, float addrspace(1)* %57, align 4
  store float %extract26vector_func.i, float addrspace(1)* %58, align 4
  store float %extract27vector_func.i, float addrspace(1)* %59, align 4
  store float %extract28vector_func.i, float addrspace(1)* %60, align 4
  store float %extract29vector_func.i, float addrspace(1)* %61, align 4
  store float %extract30vector_func.i, float addrspace(1)* %62, align 4
  store float %extract31vector_func.i, float addrspace(1)* %63, align 4
  store float %extract32vector_func.i, float addrspace(1)* %64, align 4
  store float %extract33vector_func.i, float addrspace(1)* %65, align 4
  store float %extract34vector_func.i, float addrspace(1)* %66, align 4
  store float %extract35vector_func.i, float addrspace(1)* %67, align 4
  store float %extract36vector_func.i, float addrspace(1)* %68, align 4
  %add2075vector_func.i = add nsw <16 x i32> %vectorPHIvector_func.i, <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
  %and2276vector_func.i = and <16 x i32> %add2075vector_func.i, %vectorvector_func.i
  %idxprom2377vector_func.i = sext <16 x i32> %and2276vector_func.i to <16 x i64>
  %extract78vector_func.i = extractelement <16 x i64> %idxprom2377vector_func.i, i32 0
  %extract79vector_func.i = extractelement <16 x i64> %idxprom2377vector_func.i, i32 1
  %extract80vector_func.i = extractelement <16 x i64> %idxprom2377vector_func.i, i32 2
  %extract81vector_func.i = extractelement <16 x i64> %idxprom2377vector_func.i, i32 3
  %extract82vector_func.i = extractelement <16 x i64> %idxprom2377vector_func.i, i32 4
  %extract83vector_func.i = extractelement <16 x i64> %idxprom2377vector_func.i, i32 5
  %extract84vector_func.i = extractelement <16 x i64> %idxprom2377vector_func.i, i32 6
  %extract85vector_func.i = extractelement <16 x i64> %idxprom2377vector_func.i, i32 7
  %extract86vector_func.i = extractelement <16 x i64> %idxprom2377vector_func.i, i32 8
  %extract87vector_func.i = extractelement <16 x i64> %idxprom2377vector_func.i, i32 9
  %extract88vector_func.i = extractelement <16 x i64> %idxprom2377vector_func.i, i32 10
  %extract89vector_func.i = extractelement <16 x i64> %idxprom2377vector_func.i, i32 11
  %extract90vector_func.i = extractelement <16 x i64> %idxprom2377vector_func.i, i32 12
  %extract91vector_func.i = extractelement <16 x i64> %idxprom2377vector_func.i, i32 13
  %extract92vector_func.i = extractelement <16 x i64> %idxprom2377vector_func.i, i32 14
  %extract93vector_func.i = extractelement <16 x i64> %idxprom2377vector_func.i, i32 15
  %69 = getelementptr inbounds float addrspace(1)* %1, i64 %extract78vector_func.i
  %70 = getelementptr inbounds float addrspace(1)* %1, i64 %extract79vector_func.i
  %71 = getelementptr inbounds float addrspace(1)* %1, i64 %extract80vector_func.i
  %72 = getelementptr inbounds float addrspace(1)* %1, i64 %extract81vector_func.i
  %73 = getelementptr inbounds float addrspace(1)* %1, i64 %extract82vector_func.i
  %74 = getelementptr inbounds float addrspace(1)* %1, i64 %extract83vector_func.i
  %75 = getelementptr inbounds float addrspace(1)* %1, i64 %extract84vector_func.i
  %76 = getelementptr inbounds float addrspace(1)* %1, i64 %extract85vector_func.i
  %77 = getelementptr inbounds float addrspace(1)* %1, i64 %extract86vector_func.i
  %78 = getelementptr inbounds float addrspace(1)* %1, i64 %extract87vector_func.i
  %79 = getelementptr inbounds float addrspace(1)* %1, i64 %extract88vector_func.i
  %80 = getelementptr inbounds float addrspace(1)* %1, i64 %extract89vector_func.i
  %81 = getelementptr inbounds float addrspace(1)* %1, i64 %extract90vector_func.i
  %82 = getelementptr inbounds float addrspace(1)* %1, i64 %extract91vector_func.i
  %83 = getelementptr inbounds float addrspace(1)* %1, i64 %extract92vector_func.i
  %84 = getelementptr inbounds float addrspace(1)* %1, i64 %extract93vector_func.i
  store float %extract21vector_func.i, float addrspace(1)* %69, align 4
  store float %extract22vector_func.i, float addrspace(1)* %70, align 4
  store float %extract23vector_func.i, float addrspace(1)* %71, align 4
  store float %extract24vector_func.i, float addrspace(1)* %72, align 4
  store float %extract25vector_func.i, float addrspace(1)* %73, align 4
  store float %extract26vector_func.i, float addrspace(1)* %74, align 4
  store float %extract27vector_func.i, float addrspace(1)* %75, align 4
  store float %extract28vector_func.i, float addrspace(1)* %76, align 4
  store float %extract29vector_func.i, float addrspace(1)* %77, align 4
  store float %extract30vector_func.i, float addrspace(1)* %78, align 4
  store float %extract31vector_func.i, float addrspace(1)* %79, align 4
  store float %extract32vector_func.i, float addrspace(1)* %80, align 4
  store float %extract33vector_func.i, float addrspace(1)* %81, align 4
  store float %extract34vector_func.i, float addrspace(1)* %82, align 4
  store float %extract35vector_func.i, float addrspace(1)* %83, align 4
  store float %extract36vector_func.i, float addrspace(1)* %84, align 4
  %add2694vector_func.i = add nsw <16 x i32> %vectorPHIvector_func.i, <i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4>
  %and2895vector_func.i = and <16 x i32> %add2694vector_func.i, %vectorvector_func.i
  %idxprom2996vector_func.i = sext <16 x i32> %and2895vector_func.i to <16 x i64>
  %extract97vector_func.i = extractelement <16 x i64> %idxprom2996vector_func.i, i32 0
  %extract98vector_func.i = extractelement <16 x i64> %idxprom2996vector_func.i, i32 1
  %extract99vector_func.i = extractelement <16 x i64> %idxprom2996vector_func.i, i32 2
  %extract100vector_func.i = extractelement <16 x i64> %idxprom2996vector_func.i, i32 3
  %extract101vector_func.i = extractelement <16 x i64> %idxprom2996vector_func.i, i32 4
  %extract102vector_func.i = extractelement <16 x i64> %idxprom2996vector_func.i, i32 5
  %extract103vector_func.i = extractelement <16 x i64> %idxprom2996vector_func.i, i32 6
  %extract104vector_func.i = extractelement <16 x i64> %idxprom2996vector_func.i, i32 7
  %extract105vector_func.i = extractelement <16 x i64> %idxprom2996vector_func.i, i32 8
  %extract106vector_func.i = extractelement <16 x i64> %idxprom2996vector_func.i, i32 9
  %extract107vector_func.i = extractelement <16 x i64> %idxprom2996vector_func.i, i32 10
  %extract108vector_func.i = extractelement <16 x i64> %idxprom2996vector_func.i, i32 11
  %extract109vector_func.i = extractelement <16 x i64> %idxprom2996vector_func.i, i32 12
  %extract110vector_func.i = extractelement <16 x i64> %idxprom2996vector_func.i, i32 13
  %extract111vector_func.i = extractelement <16 x i64> %idxprom2996vector_func.i, i32 14
  %extract112vector_func.i = extractelement <16 x i64> %idxprom2996vector_func.i, i32 15
  %85 = getelementptr inbounds float addrspace(1)* %1, i64 %extract97vector_func.i
  %86 = getelementptr inbounds float addrspace(1)* %1, i64 %extract98vector_func.i
  %87 = getelementptr inbounds float addrspace(1)* %1, i64 %extract99vector_func.i
  %88 = getelementptr inbounds float addrspace(1)* %1, i64 %extract100vector_func.i
  %89 = getelementptr inbounds float addrspace(1)* %1, i64 %extract101vector_func.i
  %90 = getelementptr inbounds float addrspace(1)* %1, i64 %extract102vector_func.i
  %91 = getelementptr inbounds float addrspace(1)* %1, i64 %extract103vector_func.i
  %92 = getelementptr inbounds float addrspace(1)* %1, i64 %extract104vector_func.i
  %93 = getelementptr inbounds float addrspace(1)* %1, i64 %extract105vector_func.i
  %94 = getelementptr inbounds float addrspace(1)* %1, i64 %extract106vector_func.i
  %95 = getelementptr inbounds float addrspace(1)* %1, i64 %extract107vector_func.i
  %96 = getelementptr inbounds float addrspace(1)* %1, i64 %extract108vector_func.i
  %97 = getelementptr inbounds float addrspace(1)* %1, i64 %extract109vector_func.i
  %98 = getelementptr inbounds float addrspace(1)* %1, i64 %extract110vector_func.i
  %99 = getelementptr inbounds float addrspace(1)* %1, i64 %extract111vector_func.i
  %100 = getelementptr inbounds float addrspace(1)* %1, i64 %extract112vector_func.i
  store float %extract21vector_func.i, float addrspace(1)* %85, align 4
  store float %extract22vector_func.i, float addrspace(1)* %86, align 4
  store float %extract23vector_func.i, float addrspace(1)* %87, align 4
  store float %extract24vector_func.i, float addrspace(1)* %88, align 4
  store float %extract25vector_func.i, float addrspace(1)* %89, align 4
  store float %extract26vector_func.i, float addrspace(1)* %90, align 4
  store float %extract27vector_func.i, float addrspace(1)* %91, align 4
  store float %extract28vector_func.i, float addrspace(1)* %92, align 4
  store float %extract29vector_func.i, float addrspace(1)* %93, align 4
  store float %extract30vector_func.i, float addrspace(1)* %94, align 4
  store float %extract31vector_func.i, float addrspace(1)* %95, align 4
  store float %extract32vector_func.i, float addrspace(1)* %96, align 4
  store float %extract33vector_func.i, float addrspace(1)* %97, align 4
  store float %extract34vector_func.i, float addrspace(1)* %98, align 4
  store float %extract35vector_func.i, float addrspace(1)* %99, align 4
  store float %extract36vector_func.i, float addrspace(1)* %100, align 4
  %add32113vector_func.i = add nsw <16 x i32> %vectorPHIvector_func.i, <i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5>
  %and34114vector_func.i = and <16 x i32> %add32113vector_func.i, %vectorvector_func.i
  %idxprom35115vector_func.i = sext <16 x i32> %and34114vector_func.i to <16 x i64>
  %extract116vector_func.i = extractelement <16 x i64> %idxprom35115vector_func.i, i32 0
  %extract117vector_func.i = extractelement <16 x i64> %idxprom35115vector_func.i, i32 1
  %extract118vector_func.i = extractelement <16 x i64> %idxprom35115vector_func.i, i32 2
  %extract119vector_func.i = extractelement <16 x i64> %idxprom35115vector_func.i, i32 3
  %extract120vector_func.i = extractelement <16 x i64> %idxprom35115vector_func.i, i32 4
  %extract121vector_func.i = extractelement <16 x i64> %idxprom35115vector_func.i, i32 5
  %extract122vector_func.i = extractelement <16 x i64> %idxprom35115vector_func.i, i32 6
  %extract123vector_func.i = extractelement <16 x i64> %idxprom35115vector_func.i, i32 7
  %extract124vector_func.i = extractelement <16 x i64> %idxprom35115vector_func.i, i32 8
  %extract125vector_func.i = extractelement <16 x i64> %idxprom35115vector_func.i, i32 9
  %extract126vector_func.i = extractelement <16 x i64> %idxprom35115vector_func.i, i32 10
  %extract127vector_func.i = extractelement <16 x i64> %idxprom35115vector_func.i, i32 11
  %extract128vector_func.i = extractelement <16 x i64> %idxprom35115vector_func.i, i32 12
  %extract129vector_func.i = extractelement <16 x i64> %idxprom35115vector_func.i, i32 13
  %extract130vector_func.i = extractelement <16 x i64> %idxprom35115vector_func.i, i32 14
  %extract131vector_func.i = extractelement <16 x i64> %idxprom35115vector_func.i, i32 15
  %101 = getelementptr inbounds float addrspace(1)* %1, i64 %extract116vector_func.i
  %102 = getelementptr inbounds float addrspace(1)* %1, i64 %extract117vector_func.i
  %103 = getelementptr inbounds float addrspace(1)* %1, i64 %extract118vector_func.i
  %104 = getelementptr inbounds float addrspace(1)* %1, i64 %extract119vector_func.i
  %105 = getelementptr inbounds float addrspace(1)* %1, i64 %extract120vector_func.i
  %106 = getelementptr inbounds float addrspace(1)* %1, i64 %extract121vector_func.i
  %107 = getelementptr inbounds float addrspace(1)* %1, i64 %extract122vector_func.i
  %108 = getelementptr inbounds float addrspace(1)* %1, i64 %extract123vector_func.i
  %109 = getelementptr inbounds float addrspace(1)* %1, i64 %extract124vector_func.i
  %110 = getelementptr inbounds float addrspace(1)* %1, i64 %extract125vector_func.i
  %111 = getelementptr inbounds float addrspace(1)* %1, i64 %extract126vector_func.i
  %112 = getelementptr inbounds float addrspace(1)* %1, i64 %extract127vector_func.i
  %113 = getelementptr inbounds float addrspace(1)* %1, i64 %extract128vector_func.i
  %114 = getelementptr inbounds float addrspace(1)* %1, i64 %extract129vector_func.i
  %115 = getelementptr inbounds float addrspace(1)* %1, i64 %extract130vector_func.i
  %116 = getelementptr inbounds float addrspace(1)* %1, i64 %extract131vector_func.i
  store float %extract21vector_func.i, float addrspace(1)* %101, align 4
  store float %extract22vector_func.i, float addrspace(1)* %102, align 4
  store float %extract23vector_func.i, float addrspace(1)* %103, align 4
  store float %extract24vector_func.i, float addrspace(1)* %104, align 4
  store float %extract25vector_func.i, float addrspace(1)* %105, align 4
  store float %extract26vector_func.i, float addrspace(1)* %106, align 4
  store float %extract27vector_func.i, float addrspace(1)* %107, align 4
  store float %extract28vector_func.i, float addrspace(1)* %108, align 4
  store float %extract29vector_func.i, float addrspace(1)* %109, align 4
  store float %extract30vector_func.i, float addrspace(1)* %110, align 4
  store float %extract31vector_func.i, float addrspace(1)* %111, align 4
  store float %extract32vector_func.i, float addrspace(1)* %112, align 4
  store float %extract33vector_func.i, float addrspace(1)* %113, align 4
  store float %extract34vector_func.i, float addrspace(1)* %114, align 4
  store float %extract35vector_func.i, float addrspace(1)* %115, align 4
  store float %extract36vector_func.i, float addrspace(1)* %116, align 4
  %add38132vector_func.i = add nsw <16 x i32> %vectorPHIvector_func.i, <i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6>
  %and40133vector_func.i = and <16 x i32> %add38132vector_func.i, %vectorvector_func.i
  %idxprom41134vector_func.i = sext <16 x i32> %and40133vector_func.i to <16 x i64>
  %extract135vector_func.i = extractelement <16 x i64> %idxprom41134vector_func.i, i32 0
  %extract136vector_func.i = extractelement <16 x i64> %idxprom41134vector_func.i, i32 1
  %extract137vector_func.i = extractelement <16 x i64> %idxprom41134vector_func.i, i32 2
  %extract138vector_func.i = extractelement <16 x i64> %idxprom41134vector_func.i, i32 3
  %extract139vector_func.i = extractelement <16 x i64> %idxprom41134vector_func.i, i32 4
  %extract140vector_func.i = extractelement <16 x i64> %idxprom41134vector_func.i, i32 5
  %extract141vector_func.i = extractelement <16 x i64> %idxprom41134vector_func.i, i32 6
  %extract142vector_func.i = extractelement <16 x i64> %idxprom41134vector_func.i, i32 7
  %extract143vector_func.i = extractelement <16 x i64> %idxprom41134vector_func.i, i32 8
  %extract144vector_func.i = extractelement <16 x i64> %idxprom41134vector_func.i, i32 9
  %extract145vector_func.i = extractelement <16 x i64> %idxprom41134vector_func.i, i32 10
  %extract146vector_func.i = extractelement <16 x i64> %idxprom41134vector_func.i, i32 11
  %extract147vector_func.i = extractelement <16 x i64> %idxprom41134vector_func.i, i32 12
  %extract148vector_func.i = extractelement <16 x i64> %idxprom41134vector_func.i, i32 13
  %extract149vector_func.i = extractelement <16 x i64> %idxprom41134vector_func.i, i32 14
  %extract150vector_func.i = extractelement <16 x i64> %idxprom41134vector_func.i, i32 15
  %117 = getelementptr inbounds float addrspace(1)* %1, i64 %extract135vector_func.i
  %118 = getelementptr inbounds float addrspace(1)* %1, i64 %extract136vector_func.i
  %119 = getelementptr inbounds float addrspace(1)* %1, i64 %extract137vector_func.i
  %120 = getelementptr inbounds float addrspace(1)* %1, i64 %extract138vector_func.i
  %121 = getelementptr inbounds float addrspace(1)* %1, i64 %extract139vector_func.i
  %122 = getelementptr inbounds float addrspace(1)* %1, i64 %extract140vector_func.i
  %123 = getelementptr inbounds float addrspace(1)* %1, i64 %extract141vector_func.i
  %124 = getelementptr inbounds float addrspace(1)* %1, i64 %extract142vector_func.i
  %125 = getelementptr inbounds float addrspace(1)* %1, i64 %extract143vector_func.i
  %126 = getelementptr inbounds float addrspace(1)* %1, i64 %extract144vector_func.i
  %127 = getelementptr inbounds float addrspace(1)* %1, i64 %extract145vector_func.i
  %128 = getelementptr inbounds float addrspace(1)* %1, i64 %extract146vector_func.i
  %129 = getelementptr inbounds float addrspace(1)* %1, i64 %extract147vector_func.i
  %130 = getelementptr inbounds float addrspace(1)* %1, i64 %extract148vector_func.i
  %131 = getelementptr inbounds float addrspace(1)* %1, i64 %extract149vector_func.i
  %132 = getelementptr inbounds float addrspace(1)* %1, i64 %extract150vector_func.i
  store float %extract21vector_func.i, float addrspace(1)* %117, align 4
  store float %extract22vector_func.i, float addrspace(1)* %118, align 4
  store float %extract23vector_func.i, float addrspace(1)* %119, align 4
  store float %extract24vector_func.i, float addrspace(1)* %120, align 4
  store float %extract25vector_func.i, float addrspace(1)* %121, align 4
  store float %extract26vector_func.i, float addrspace(1)* %122, align 4
  store float %extract27vector_func.i, float addrspace(1)* %123, align 4
  store float %extract28vector_func.i, float addrspace(1)* %124, align 4
  store float %extract29vector_func.i, float addrspace(1)* %125, align 4
  store float %extract30vector_func.i, float addrspace(1)* %126, align 4
  store float %extract31vector_func.i, float addrspace(1)* %127, align 4
  store float %extract32vector_func.i, float addrspace(1)* %128, align 4
  store float %extract33vector_func.i, float addrspace(1)* %129, align 4
  store float %extract34vector_func.i, float addrspace(1)* %130, align 4
  store float %extract35vector_func.i, float addrspace(1)* %131, align 4
  store float %extract36vector_func.i, float addrspace(1)* %132, align 4
  %add44151vector_func.i = add nsw <16 x i32> %vectorPHIvector_func.i, <i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7>
  %and46152vector_func.i = and <16 x i32> %add44151vector_func.i, %vectorvector_func.i
  %idxprom47153vector_func.i = sext <16 x i32> %and46152vector_func.i to <16 x i64>
  %extract154vector_func.i = extractelement <16 x i64> %idxprom47153vector_func.i, i32 0
  %extract155vector_func.i = extractelement <16 x i64> %idxprom47153vector_func.i, i32 1
  %extract156vector_func.i = extractelement <16 x i64> %idxprom47153vector_func.i, i32 2
  %extract157vector_func.i = extractelement <16 x i64> %idxprom47153vector_func.i, i32 3
  %extract158vector_func.i = extractelement <16 x i64> %idxprom47153vector_func.i, i32 4
  %extract159vector_func.i = extractelement <16 x i64> %idxprom47153vector_func.i, i32 5
  %extract160vector_func.i = extractelement <16 x i64> %idxprom47153vector_func.i, i32 6
  %extract161vector_func.i = extractelement <16 x i64> %idxprom47153vector_func.i, i32 7
  %extract162vector_func.i = extractelement <16 x i64> %idxprom47153vector_func.i, i32 8
  %extract163vector_func.i = extractelement <16 x i64> %idxprom47153vector_func.i, i32 9
  %extract164vector_func.i = extractelement <16 x i64> %idxprom47153vector_func.i, i32 10
  %extract165vector_func.i = extractelement <16 x i64> %idxprom47153vector_func.i, i32 11
  %extract166vector_func.i = extractelement <16 x i64> %idxprom47153vector_func.i, i32 12
  %extract167vector_func.i = extractelement <16 x i64> %idxprom47153vector_func.i, i32 13
  %extract168vector_func.i = extractelement <16 x i64> %idxprom47153vector_func.i, i32 14
  %extract169vector_func.i = extractelement <16 x i64> %idxprom47153vector_func.i, i32 15
  %133 = getelementptr inbounds float addrspace(1)* %1, i64 %extract154vector_func.i
  %134 = getelementptr inbounds float addrspace(1)* %1, i64 %extract155vector_func.i
  %135 = getelementptr inbounds float addrspace(1)* %1, i64 %extract156vector_func.i
  %136 = getelementptr inbounds float addrspace(1)* %1, i64 %extract157vector_func.i
  %137 = getelementptr inbounds float addrspace(1)* %1, i64 %extract158vector_func.i
  %138 = getelementptr inbounds float addrspace(1)* %1, i64 %extract159vector_func.i
  %139 = getelementptr inbounds float addrspace(1)* %1, i64 %extract160vector_func.i
  %140 = getelementptr inbounds float addrspace(1)* %1, i64 %extract161vector_func.i
  %141 = getelementptr inbounds float addrspace(1)* %1, i64 %extract162vector_func.i
  %142 = getelementptr inbounds float addrspace(1)* %1, i64 %extract163vector_func.i
  %143 = getelementptr inbounds float addrspace(1)* %1, i64 %extract164vector_func.i
  %144 = getelementptr inbounds float addrspace(1)* %1, i64 %extract165vector_func.i
  %145 = getelementptr inbounds float addrspace(1)* %1, i64 %extract166vector_func.i
  %146 = getelementptr inbounds float addrspace(1)* %1, i64 %extract167vector_func.i
  %147 = getelementptr inbounds float addrspace(1)* %1, i64 %extract168vector_func.i
  %148 = getelementptr inbounds float addrspace(1)* %1, i64 %extract169vector_func.i
  store float %extract21vector_func.i, float addrspace(1)* %133, align 4
  store float %extract22vector_func.i, float addrspace(1)* %134, align 4
  store float %extract23vector_func.i, float addrspace(1)* %135, align 4
  store float %extract24vector_func.i, float addrspace(1)* %136, align 4
  store float %extract25vector_func.i, float addrspace(1)* %137, align 4
  store float %extract26vector_func.i, float addrspace(1)* %138, align 4
  store float %extract27vector_func.i, float addrspace(1)* %139, align 4
  store float %extract28vector_func.i, float addrspace(1)* %140, align 4
  store float %extract29vector_func.i, float addrspace(1)* %141, align 4
  store float %extract30vector_func.i, float addrspace(1)* %142, align 4
  store float %extract31vector_func.i, float addrspace(1)* %143, align 4
  store float %extract32vector_func.i, float addrspace(1)* %144, align 4
  store float %extract33vector_func.i, float addrspace(1)* %145, align 4
  store float %extract34vector_func.i, float addrspace(1)* %146, align 4
  store float %extract35vector_func.i, float addrspace(1)* %147, align 4
  store float %extract36vector_func.i, float addrspace(1)* %148, align 4
  %add50170vector_func.i = add nsw <16 x i32> %vectorPHIvector_func.i, <i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8>
  %and52171vector_func.i = and <16 x i32> %add50170vector_func.i, %vectorvector_func.i
  %idxprom53172vector_func.i = sext <16 x i32> %and52171vector_func.i to <16 x i64>
  %extract173vector_func.i = extractelement <16 x i64> %idxprom53172vector_func.i, i32 0
  %extract174vector_func.i = extractelement <16 x i64> %idxprom53172vector_func.i, i32 1
  %extract175vector_func.i = extractelement <16 x i64> %idxprom53172vector_func.i, i32 2
  %extract176vector_func.i = extractelement <16 x i64> %idxprom53172vector_func.i, i32 3
  %extract177vector_func.i = extractelement <16 x i64> %idxprom53172vector_func.i, i32 4
  %extract178vector_func.i = extractelement <16 x i64> %idxprom53172vector_func.i, i32 5
  %extract179vector_func.i = extractelement <16 x i64> %idxprom53172vector_func.i, i32 6
  %extract180vector_func.i = extractelement <16 x i64> %idxprom53172vector_func.i, i32 7
  %extract181vector_func.i = extractelement <16 x i64> %idxprom53172vector_func.i, i32 8
  %extract182vector_func.i = extractelement <16 x i64> %idxprom53172vector_func.i, i32 9
  %extract183vector_func.i = extractelement <16 x i64> %idxprom53172vector_func.i, i32 10
  %extract184vector_func.i = extractelement <16 x i64> %idxprom53172vector_func.i, i32 11
  %extract185vector_func.i = extractelement <16 x i64> %idxprom53172vector_func.i, i32 12
  %extract186vector_func.i = extractelement <16 x i64> %idxprom53172vector_func.i, i32 13
  %extract187vector_func.i = extractelement <16 x i64> %idxprom53172vector_func.i, i32 14
  %extract188vector_func.i = extractelement <16 x i64> %idxprom53172vector_func.i, i32 15
  %149 = getelementptr inbounds float addrspace(1)* %1, i64 %extract173vector_func.i
  %150 = getelementptr inbounds float addrspace(1)* %1, i64 %extract174vector_func.i
  %151 = getelementptr inbounds float addrspace(1)* %1, i64 %extract175vector_func.i
  %152 = getelementptr inbounds float addrspace(1)* %1, i64 %extract176vector_func.i
  %153 = getelementptr inbounds float addrspace(1)* %1, i64 %extract177vector_func.i
  %154 = getelementptr inbounds float addrspace(1)* %1, i64 %extract178vector_func.i
  %155 = getelementptr inbounds float addrspace(1)* %1, i64 %extract179vector_func.i
  %156 = getelementptr inbounds float addrspace(1)* %1, i64 %extract180vector_func.i
  %157 = getelementptr inbounds float addrspace(1)* %1, i64 %extract181vector_func.i
  %158 = getelementptr inbounds float addrspace(1)* %1, i64 %extract182vector_func.i
  %159 = getelementptr inbounds float addrspace(1)* %1, i64 %extract183vector_func.i
  %160 = getelementptr inbounds float addrspace(1)* %1, i64 %extract184vector_func.i
  %161 = getelementptr inbounds float addrspace(1)* %1, i64 %extract185vector_func.i
  %162 = getelementptr inbounds float addrspace(1)* %1, i64 %extract186vector_func.i
  %163 = getelementptr inbounds float addrspace(1)* %1, i64 %extract187vector_func.i
  %164 = getelementptr inbounds float addrspace(1)* %1, i64 %extract188vector_func.i
  store float %extract21vector_func.i, float addrspace(1)* %149, align 4
  store float %extract22vector_func.i, float addrspace(1)* %150, align 4
  store float %extract23vector_func.i, float addrspace(1)* %151, align 4
  store float %extract24vector_func.i, float addrspace(1)* %152, align 4
  store float %extract25vector_func.i, float addrspace(1)* %153, align 4
  store float %extract26vector_func.i, float addrspace(1)* %154, align 4
  store float %extract27vector_func.i, float addrspace(1)* %155, align 4
  store float %extract28vector_func.i, float addrspace(1)* %156, align 4
  store float %extract29vector_func.i, float addrspace(1)* %157, align 4
  store float %extract30vector_func.i, float addrspace(1)* %158, align 4
  store float %extract31vector_func.i, float addrspace(1)* %159, align 4
  store float %extract32vector_func.i, float addrspace(1)* %160, align 4
  store float %extract33vector_func.i, float addrspace(1)* %161, align 4
  store float %extract34vector_func.i, float addrspace(1)* %162, align 4
  store float %extract35vector_func.i, float addrspace(1)* %163, align 4
  store float %extract36vector_func.i, float addrspace(1)* %164, align 4
  %add56189vector_func.i = add nsw <16 x i32> %vectorPHIvector_func.i, <i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9>
  %and58190vector_func.i = and <16 x i32> %add56189vector_func.i, %vectorvector_func.i
  %idxprom59191vector_func.i = sext <16 x i32> %and58190vector_func.i to <16 x i64>
  %extract192vector_func.i = extractelement <16 x i64> %idxprom59191vector_func.i, i32 0
  %extract193vector_func.i = extractelement <16 x i64> %idxprom59191vector_func.i, i32 1
  %extract194vector_func.i = extractelement <16 x i64> %idxprom59191vector_func.i, i32 2
  %extract195vector_func.i = extractelement <16 x i64> %idxprom59191vector_func.i, i32 3
  %extract196vector_func.i = extractelement <16 x i64> %idxprom59191vector_func.i, i32 4
  %extract197vector_func.i = extractelement <16 x i64> %idxprom59191vector_func.i, i32 5
  %extract198vector_func.i = extractelement <16 x i64> %idxprom59191vector_func.i, i32 6
  %extract199vector_func.i = extractelement <16 x i64> %idxprom59191vector_func.i, i32 7
  %extract200vector_func.i = extractelement <16 x i64> %idxprom59191vector_func.i, i32 8
  %extract201vector_func.i = extractelement <16 x i64> %idxprom59191vector_func.i, i32 9
  %extract202vector_func.i = extractelement <16 x i64> %idxprom59191vector_func.i, i32 10
  %extract203vector_func.i = extractelement <16 x i64> %idxprom59191vector_func.i, i32 11
  %extract204vector_func.i = extractelement <16 x i64> %idxprom59191vector_func.i, i32 12
  %extract205vector_func.i = extractelement <16 x i64> %idxprom59191vector_func.i, i32 13
  %extract206vector_func.i = extractelement <16 x i64> %idxprom59191vector_func.i, i32 14
  %extract207vector_func.i = extractelement <16 x i64> %idxprom59191vector_func.i, i32 15
  %165 = getelementptr inbounds float addrspace(1)* %1, i64 %extract192vector_func.i
  %166 = getelementptr inbounds float addrspace(1)* %1, i64 %extract193vector_func.i
  %167 = getelementptr inbounds float addrspace(1)* %1, i64 %extract194vector_func.i
  %168 = getelementptr inbounds float addrspace(1)* %1, i64 %extract195vector_func.i
  %169 = getelementptr inbounds float addrspace(1)* %1, i64 %extract196vector_func.i
  %170 = getelementptr inbounds float addrspace(1)* %1, i64 %extract197vector_func.i
  %171 = getelementptr inbounds float addrspace(1)* %1, i64 %extract198vector_func.i
  %172 = getelementptr inbounds float addrspace(1)* %1, i64 %extract199vector_func.i
  %173 = getelementptr inbounds float addrspace(1)* %1, i64 %extract200vector_func.i
  %174 = getelementptr inbounds float addrspace(1)* %1, i64 %extract201vector_func.i
  %175 = getelementptr inbounds float addrspace(1)* %1, i64 %extract202vector_func.i
  %176 = getelementptr inbounds float addrspace(1)* %1, i64 %extract203vector_func.i
  %177 = getelementptr inbounds float addrspace(1)* %1, i64 %extract204vector_func.i
  %178 = getelementptr inbounds float addrspace(1)* %1, i64 %extract205vector_func.i
  %179 = getelementptr inbounds float addrspace(1)* %1, i64 %extract206vector_func.i
  %180 = getelementptr inbounds float addrspace(1)* %1, i64 %extract207vector_func.i
  store float %extract21vector_func.i, float addrspace(1)* %165, align 4
  store float %extract22vector_func.i, float addrspace(1)* %166, align 4
  store float %extract23vector_func.i, float addrspace(1)* %167, align 4
  store float %extract24vector_func.i, float addrspace(1)* %168, align 4
  store float %extract25vector_func.i, float addrspace(1)* %169, align 4
  store float %extract26vector_func.i, float addrspace(1)* %170, align 4
  store float %extract27vector_func.i, float addrspace(1)* %171, align 4
  store float %extract28vector_func.i, float addrspace(1)* %172, align 4
  store float %extract29vector_func.i, float addrspace(1)* %173, align 4
  store float %extract30vector_func.i, float addrspace(1)* %174, align 4
  store float %extract31vector_func.i, float addrspace(1)* %175, align 4
  store float %extract32vector_func.i, float addrspace(1)* %176, align 4
  store float %extract33vector_func.i, float addrspace(1)* %177, align 4
  store float %extract34vector_func.i, float addrspace(1)* %178, align 4
  store float %extract35vector_func.i, float addrspace(1)* %179, align 4
  store float %extract36vector_func.i, float addrspace(1)* %180, align 4
  %add62208vector_func.i = add nsw <16 x i32> %vectorPHIvector_func.i, <i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10>
  %and64209vector_func.i = and <16 x i32> %add62208vector_func.i, %vectorvector_func.i
  %idxprom65210vector_func.i = sext <16 x i32> %and64209vector_func.i to <16 x i64>
  %extract211vector_func.i = extractelement <16 x i64> %idxprom65210vector_func.i, i32 0
  %extract212vector_func.i = extractelement <16 x i64> %idxprom65210vector_func.i, i32 1
  %extract213vector_func.i = extractelement <16 x i64> %idxprom65210vector_func.i, i32 2
  %extract214vector_func.i = extractelement <16 x i64> %idxprom65210vector_func.i, i32 3
  %extract215vector_func.i = extractelement <16 x i64> %idxprom65210vector_func.i, i32 4
  %extract216vector_func.i = extractelement <16 x i64> %idxprom65210vector_func.i, i32 5
  %extract217vector_func.i = extractelement <16 x i64> %idxprom65210vector_func.i, i32 6
  %extract218vector_func.i = extractelement <16 x i64> %idxprom65210vector_func.i, i32 7
  %extract219vector_func.i = extractelement <16 x i64> %idxprom65210vector_func.i, i32 8
  %extract220vector_func.i = extractelement <16 x i64> %idxprom65210vector_func.i, i32 9
  %extract221vector_func.i = extractelement <16 x i64> %idxprom65210vector_func.i, i32 10
  %extract222vector_func.i = extractelement <16 x i64> %idxprom65210vector_func.i, i32 11
  %extract223vector_func.i = extractelement <16 x i64> %idxprom65210vector_func.i, i32 12
  %extract224vector_func.i = extractelement <16 x i64> %idxprom65210vector_func.i, i32 13
  %extract225vector_func.i = extractelement <16 x i64> %idxprom65210vector_func.i, i32 14
  %extract226vector_func.i = extractelement <16 x i64> %idxprom65210vector_func.i, i32 15
  %181 = getelementptr inbounds float addrspace(1)* %1, i64 %extract211vector_func.i
  %182 = getelementptr inbounds float addrspace(1)* %1, i64 %extract212vector_func.i
  %183 = getelementptr inbounds float addrspace(1)* %1, i64 %extract213vector_func.i
  %184 = getelementptr inbounds float addrspace(1)* %1, i64 %extract214vector_func.i
  %185 = getelementptr inbounds float addrspace(1)* %1, i64 %extract215vector_func.i
  %186 = getelementptr inbounds float addrspace(1)* %1, i64 %extract216vector_func.i
  %187 = getelementptr inbounds float addrspace(1)* %1, i64 %extract217vector_func.i
  %188 = getelementptr inbounds float addrspace(1)* %1, i64 %extract218vector_func.i
  %189 = getelementptr inbounds float addrspace(1)* %1, i64 %extract219vector_func.i
  %190 = getelementptr inbounds float addrspace(1)* %1, i64 %extract220vector_func.i
  %191 = getelementptr inbounds float addrspace(1)* %1, i64 %extract221vector_func.i
  %192 = getelementptr inbounds float addrspace(1)* %1, i64 %extract222vector_func.i
  %193 = getelementptr inbounds float addrspace(1)* %1, i64 %extract223vector_func.i
  %194 = getelementptr inbounds float addrspace(1)* %1, i64 %extract224vector_func.i
  %195 = getelementptr inbounds float addrspace(1)* %1, i64 %extract225vector_func.i
  %196 = getelementptr inbounds float addrspace(1)* %1, i64 %extract226vector_func.i
  store float %extract21vector_func.i, float addrspace(1)* %181, align 4
  store float %extract22vector_func.i, float addrspace(1)* %182, align 4
  store float %extract23vector_func.i, float addrspace(1)* %183, align 4
  store float %extract24vector_func.i, float addrspace(1)* %184, align 4
  store float %extract25vector_func.i, float addrspace(1)* %185, align 4
  store float %extract26vector_func.i, float addrspace(1)* %186, align 4
  store float %extract27vector_func.i, float addrspace(1)* %187, align 4
  store float %extract28vector_func.i, float addrspace(1)* %188, align 4
  store float %extract29vector_func.i, float addrspace(1)* %189, align 4
  store float %extract30vector_func.i, float addrspace(1)* %190, align 4
  store float %extract31vector_func.i, float addrspace(1)* %191, align 4
  store float %extract32vector_func.i, float addrspace(1)* %192, align 4
  store float %extract33vector_func.i, float addrspace(1)* %193, align 4
  store float %extract34vector_func.i, float addrspace(1)* %194, align 4
  store float %extract35vector_func.i, float addrspace(1)* %195, align 4
  store float %extract36vector_func.i, float addrspace(1)* %196, align 4
  %add68227vector_func.i = add nsw <16 x i32> %vectorPHIvector_func.i, <i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11>
  %and70228vector_func.i = and <16 x i32> %add68227vector_func.i, %vectorvector_func.i
  %idxprom71229vector_func.i = sext <16 x i32> %and70228vector_func.i to <16 x i64>
  %extract230vector_func.i = extractelement <16 x i64> %idxprom71229vector_func.i, i32 0
  %extract231vector_func.i = extractelement <16 x i64> %idxprom71229vector_func.i, i32 1
  %extract232vector_func.i = extractelement <16 x i64> %idxprom71229vector_func.i, i32 2
  %extract233vector_func.i = extractelement <16 x i64> %idxprom71229vector_func.i, i32 3
  %extract234vector_func.i = extractelement <16 x i64> %idxprom71229vector_func.i, i32 4
  %extract235vector_func.i = extractelement <16 x i64> %idxprom71229vector_func.i, i32 5
  %extract236vector_func.i = extractelement <16 x i64> %idxprom71229vector_func.i, i32 6
  %extract237vector_func.i = extractelement <16 x i64> %idxprom71229vector_func.i, i32 7
  %extract238vector_func.i = extractelement <16 x i64> %idxprom71229vector_func.i, i32 8
  %extract239vector_func.i = extractelement <16 x i64> %idxprom71229vector_func.i, i32 9
  %extract240vector_func.i = extractelement <16 x i64> %idxprom71229vector_func.i, i32 10
  %extract241vector_func.i = extractelement <16 x i64> %idxprom71229vector_func.i, i32 11
  %extract242vector_func.i = extractelement <16 x i64> %idxprom71229vector_func.i, i32 12
  %extract243vector_func.i = extractelement <16 x i64> %idxprom71229vector_func.i, i32 13
  %extract244vector_func.i = extractelement <16 x i64> %idxprom71229vector_func.i, i32 14
  %extract245vector_func.i = extractelement <16 x i64> %idxprom71229vector_func.i, i32 15
  %197 = getelementptr inbounds float addrspace(1)* %1, i64 %extract230vector_func.i
  %198 = getelementptr inbounds float addrspace(1)* %1, i64 %extract231vector_func.i
  %199 = getelementptr inbounds float addrspace(1)* %1, i64 %extract232vector_func.i
  %200 = getelementptr inbounds float addrspace(1)* %1, i64 %extract233vector_func.i
  %201 = getelementptr inbounds float addrspace(1)* %1, i64 %extract234vector_func.i
  %202 = getelementptr inbounds float addrspace(1)* %1, i64 %extract235vector_func.i
  %203 = getelementptr inbounds float addrspace(1)* %1, i64 %extract236vector_func.i
  %204 = getelementptr inbounds float addrspace(1)* %1, i64 %extract237vector_func.i
  %205 = getelementptr inbounds float addrspace(1)* %1, i64 %extract238vector_func.i
  %206 = getelementptr inbounds float addrspace(1)* %1, i64 %extract239vector_func.i
  %207 = getelementptr inbounds float addrspace(1)* %1, i64 %extract240vector_func.i
  %208 = getelementptr inbounds float addrspace(1)* %1, i64 %extract241vector_func.i
  %209 = getelementptr inbounds float addrspace(1)* %1, i64 %extract242vector_func.i
  %210 = getelementptr inbounds float addrspace(1)* %1, i64 %extract243vector_func.i
  %211 = getelementptr inbounds float addrspace(1)* %1, i64 %extract244vector_func.i
  %212 = getelementptr inbounds float addrspace(1)* %1, i64 %extract245vector_func.i
  store float %extract21vector_func.i, float addrspace(1)* %197, align 4
  store float %extract22vector_func.i, float addrspace(1)* %198, align 4
  store float %extract23vector_func.i, float addrspace(1)* %199, align 4
  store float %extract24vector_func.i, float addrspace(1)* %200, align 4
  store float %extract25vector_func.i, float addrspace(1)* %201, align 4
  store float %extract26vector_func.i, float addrspace(1)* %202, align 4
  store float %extract27vector_func.i, float addrspace(1)* %203, align 4
  store float %extract28vector_func.i, float addrspace(1)* %204, align 4
  store float %extract29vector_func.i, float addrspace(1)* %205, align 4
  store float %extract30vector_func.i, float addrspace(1)* %206, align 4
  store float %extract31vector_func.i, float addrspace(1)* %207, align 4
  store float %extract32vector_func.i, float addrspace(1)* %208, align 4
  store float %extract33vector_func.i, float addrspace(1)* %209, align 4
  store float %extract34vector_func.i, float addrspace(1)* %210, align 4
  store float %extract35vector_func.i, float addrspace(1)* %211, align 4
  store float %extract36vector_func.i, float addrspace(1)* %212, align 4
  %add74246vector_func.i = add nsw <16 x i32> %vectorPHIvector_func.i, <i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12>
  %and76247vector_func.i = and <16 x i32> %add74246vector_func.i, %vectorvector_func.i
  %idxprom77248vector_func.i = sext <16 x i32> %and76247vector_func.i to <16 x i64>
  %extract249vector_func.i = extractelement <16 x i64> %idxprom77248vector_func.i, i32 0
  %extract250vector_func.i = extractelement <16 x i64> %idxprom77248vector_func.i, i32 1
  %extract251vector_func.i = extractelement <16 x i64> %idxprom77248vector_func.i, i32 2
  %extract252vector_func.i = extractelement <16 x i64> %idxprom77248vector_func.i, i32 3
  %extract253vector_func.i = extractelement <16 x i64> %idxprom77248vector_func.i, i32 4
  %extract254vector_func.i = extractelement <16 x i64> %idxprom77248vector_func.i, i32 5
  %extract255vector_func.i = extractelement <16 x i64> %idxprom77248vector_func.i, i32 6
  %extract256vector_func.i = extractelement <16 x i64> %idxprom77248vector_func.i, i32 7
  %extract257vector_func.i = extractelement <16 x i64> %idxprom77248vector_func.i, i32 8
  %extract258vector_func.i = extractelement <16 x i64> %idxprom77248vector_func.i, i32 9
  %extract259vector_func.i = extractelement <16 x i64> %idxprom77248vector_func.i, i32 10
  %extract260vector_func.i = extractelement <16 x i64> %idxprom77248vector_func.i, i32 11
  %extract261vector_func.i = extractelement <16 x i64> %idxprom77248vector_func.i, i32 12
  %extract262vector_func.i = extractelement <16 x i64> %idxprom77248vector_func.i, i32 13
  %extract263vector_func.i = extractelement <16 x i64> %idxprom77248vector_func.i, i32 14
  %extract264vector_func.i = extractelement <16 x i64> %idxprom77248vector_func.i, i32 15
  %213 = getelementptr inbounds float addrspace(1)* %1, i64 %extract249vector_func.i
  %214 = getelementptr inbounds float addrspace(1)* %1, i64 %extract250vector_func.i
  %215 = getelementptr inbounds float addrspace(1)* %1, i64 %extract251vector_func.i
  %216 = getelementptr inbounds float addrspace(1)* %1, i64 %extract252vector_func.i
  %217 = getelementptr inbounds float addrspace(1)* %1, i64 %extract253vector_func.i
  %218 = getelementptr inbounds float addrspace(1)* %1, i64 %extract254vector_func.i
  %219 = getelementptr inbounds float addrspace(1)* %1, i64 %extract255vector_func.i
  %220 = getelementptr inbounds float addrspace(1)* %1, i64 %extract256vector_func.i
  %221 = getelementptr inbounds float addrspace(1)* %1, i64 %extract257vector_func.i
  %222 = getelementptr inbounds float addrspace(1)* %1, i64 %extract258vector_func.i
  %223 = getelementptr inbounds float addrspace(1)* %1, i64 %extract259vector_func.i
  %224 = getelementptr inbounds float addrspace(1)* %1, i64 %extract260vector_func.i
  %225 = getelementptr inbounds float addrspace(1)* %1, i64 %extract261vector_func.i
  %226 = getelementptr inbounds float addrspace(1)* %1, i64 %extract262vector_func.i
  %227 = getelementptr inbounds float addrspace(1)* %1, i64 %extract263vector_func.i
  %228 = getelementptr inbounds float addrspace(1)* %1, i64 %extract264vector_func.i
  store float %extract21vector_func.i, float addrspace(1)* %213, align 4
  store float %extract22vector_func.i, float addrspace(1)* %214, align 4
  store float %extract23vector_func.i, float addrspace(1)* %215, align 4
  store float %extract24vector_func.i, float addrspace(1)* %216, align 4
  store float %extract25vector_func.i, float addrspace(1)* %217, align 4
  store float %extract26vector_func.i, float addrspace(1)* %218, align 4
  store float %extract27vector_func.i, float addrspace(1)* %219, align 4
  store float %extract28vector_func.i, float addrspace(1)* %220, align 4
  store float %extract29vector_func.i, float addrspace(1)* %221, align 4
  store float %extract30vector_func.i, float addrspace(1)* %222, align 4
  store float %extract31vector_func.i, float addrspace(1)* %223, align 4
  store float %extract32vector_func.i, float addrspace(1)* %224, align 4
  store float %extract33vector_func.i, float addrspace(1)* %225, align 4
  store float %extract34vector_func.i, float addrspace(1)* %226, align 4
  store float %extract35vector_func.i, float addrspace(1)* %227, align 4
  store float %extract36vector_func.i, float addrspace(1)* %228, align 4
  %add80265vector_func.i = add nsw <16 x i32> %vectorPHIvector_func.i, <i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13>
  %and82266vector_func.i = and <16 x i32> %add80265vector_func.i, %vectorvector_func.i
  %idxprom83267vector_func.i = sext <16 x i32> %and82266vector_func.i to <16 x i64>
  %extract268vector_func.i = extractelement <16 x i64> %idxprom83267vector_func.i, i32 0
  %extract269vector_func.i = extractelement <16 x i64> %idxprom83267vector_func.i, i32 1
  %extract270vector_func.i = extractelement <16 x i64> %idxprom83267vector_func.i, i32 2
  %extract271vector_func.i = extractelement <16 x i64> %idxprom83267vector_func.i, i32 3
  %extract272vector_func.i = extractelement <16 x i64> %idxprom83267vector_func.i, i32 4
  %extract273vector_func.i = extractelement <16 x i64> %idxprom83267vector_func.i, i32 5
  %extract274vector_func.i = extractelement <16 x i64> %idxprom83267vector_func.i, i32 6
  %extract275vector_func.i = extractelement <16 x i64> %idxprom83267vector_func.i, i32 7
  %extract276vector_func.i = extractelement <16 x i64> %idxprom83267vector_func.i, i32 8
  %extract277vector_func.i = extractelement <16 x i64> %idxprom83267vector_func.i, i32 9
  %extract278vector_func.i = extractelement <16 x i64> %idxprom83267vector_func.i, i32 10
  %extract279vector_func.i = extractelement <16 x i64> %idxprom83267vector_func.i, i32 11
  %extract280vector_func.i = extractelement <16 x i64> %idxprom83267vector_func.i, i32 12
  %extract281vector_func.i = extractelement <16 x i64> %idxprom83267vector_func.i, i32 13
  %extract282vector_func.i = extractelement <16 x i64> %idxprom83267vector_func.i, i32 14
  %extract283vector_func.i = extractelement <16 x i64> %idxprom83267vector_func.i, i32 15
  %229 = getelementptr inbounds float addrspace(1)* %1, i64 %extract268vector_func.i
  %230 = getelementptr inbounds float addrspace(1)* %1, i64 %extract269vector_func.i
  %231 = getelementptr inbounds float addrspace(1)* %1, i64 %extract270vector_func.i
  %232 = getelementptr inbounds float addrspace(1)* %1, i64 %extract271vector_func.i
  %233 = getelementptr inbounds float addrspace(1)* %1, i64 %extract272vector_func.i
  %234 = getelementptr inbounds float addrspace(1)* %1, i64 %extract273vector_func.i
  %235 = getelementptr inbounds float addrspace(1)* %1, i64 %extract274vector_func.i
  %236 = getelementptr inbounds float addrspace(1)* %1, i64 %extract275vector_func.i
  %237 = getelementptr inbounds float addrspace(1)* %1, i64 %extract276vector_func.i
  %238 = getelementptr inbounds float addrspace(1)* %1, i64 %extract277vector_func.i
  %239 = getelementptr inbounds float addrspace(1)* %1, i64 %extract278vector_func.i
  %240 = getelementptr inbounds float addrspace(1)* %1, i64 %extract279vector_func.i
  %241 = getelementptr inbounds float addrspace(1)* %1, i64 %extract280vector_func.i
  %242 = getelementptr inbounds float addrspace(1)* %1, i64 %extract281vector_func.i
  %243 = getelementptr inbounds float addrspace(1)* %1, i64 %extract282vector_func.i
  %244 = getelementptr inbounds float addrspace(1)* %1, i64 %extract283vector_func.i
  store float %extract21vector_func.i, float addrspace(1)* %229, align 4
  store float %extract22vector_func.i, float addrspace(1)* %230, align 4
  store float %extract23vector_func.i, float addrspace(1)* %231, align 4
  store float %extract24vector_func.i, float addrspace(1)* %232, align 4
  store float %extract25vector_func.i, float addrspace(1)* %233, align 4
  store float %extract26vector_func.i, float addrspace(1)* %234, align 4
  store float %extract27vector_func.i, float addrspace(1)* %235, align 4
  store float %extract28vector_func.i, float addrspace(1)* %236, align 4
  store float %extract29vector_func.i, float addrspace(1)* %237, align 4
  store float %extract30vector_func.i, float addrspace(1)* %238, align 4
  store float %extract31vector_func.i, float addrspace(1)* %239, align 4
  store float %extract32vector_func.i, float addrspace(1)* %240, align 4
  store float %extract33vector_func.i, float addrspace(1)* %241, align 4
  store float %extract34vector_func.i, float addrspace(1)* %242, align 4
  store float %extract35vector_func.i, float addrspace(1)* %243, align 4
  store float %extract36vector_func.i, float addrspace(1)* %244, align 4
  %add86284vector_func.i = add nsw <16 x i32> %vectorPHIvector_func.i, <i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14>
  %and88285vector_func.i = and <16 x i32> %add86284vector_func.i, %vectorvector_func.i
  %idxprom89286vector_func.i = sext <16 x i32> %and88285vector_func.i to <16 x i64>
  %extract287vector_func.i = extractelement <16 x i64> %idxprom89286vector_func.i, i32 0
  %extract288vector_func.i = extractelement <16 x i64> %idxprom89286vector_func.i, i32 1
  %extract289vector_func.i = extractelement <16 x i64> %idxprom89286vector_func.i, i32 2
  %extract290vector_func.i = extractelement <16 x i64> %idxprom89286vector_func.i, i32 3
  %extract291vector_func.i = extractelement <16 x i64> %idxprom89286vector_func.i, i32 4
  %extract292vector_func.i = extractelement <16 x i64> %idxprom89286vector_func.i, i32 5
  %extract293vector_func.i = extractelement <16 x i64> %idxprom89286vector_func.i, i32 6
  %extract294vector_func.i = extractelement <16 x i64> %idxprom89286vector_func.i, i32 7
  %extract295vector_func.i = extractelement <16 x i64> %idxprom89286vector_func.i, i32 8
  %extract296vector_func.i = extractelement <16 x i64> %idxprom89286vector_func.i, i32 9
  %extract297vector_func.i = extractelement <16 x i64> %idxprom89286vector_func.i, i32 10
  %extract298vector_func.i = extractelement <16 x i64> %idxprom89286vector_func.i, i32 11
  %extract299vector_func.i = extractelement <16 x i64> %idxprom89286vector_func.i, i32 12
  %extract300vector_func.i = extractelement <16 x i64> %idxprom89286vector_func.i, i32 13
  %extract301vector_func.i = extractelement <16 x i64> %idxprom89286vector_func.i, i32 14
  %extract302vector_func.i = extractelement <16 x i64> %idxprom89286vector_func.i, i32 15
  %245 = getelementptr inbounds float addrspace(1)* %1, i64 %extract287vector_func.i
  %246 = getelementptr inbounds float addrspace(1)* %1, i64 %extract288vector_func.i
  %247 = getelementptr inbounds float addrspace(1)* %1, i64 %extract289vector_func.i
  %248 = getelementptr inbounds float addrspace(1)* %1, i64 %extract290vector_func.i
  %249 = getelementptr inbounds float addrspace(1)* %1, i64 %extract291vector_func.i
  %250 = getelementptr inbounds float addrspace(1)* %1, i64 %extract292vector_func.i
  %251 = getelementptr inbounds float addrspace(1)* %1, i64 %extract293vector_func.i
  %252 = getelementptr inbounds float addrspace(1)* %1, i64 %extract294vector_func.i
  %253 = getelementptr inbounds float addrspace(1)* %1, i64 %extract295vector_func.i
  %254 = getelementptr inbounds float addrspace(1)* %1, i64 %extract296vector_func.i
  %255 = getelementptr inbounds float addrspace(1)* %1, i64 %extract297vector_func.i
  %256 = getelementptr inbounds float addrspace(1)* %1, i64 %extract298vector_func.i
  %257 = getelementptr inbounds float addrspace(1)* %1, i64 %extract299vector_func.i
  %258 = getelementptr inbounds float addrspace(1)* %1, i64 %extract300vector_func.i
  %259 = getelementptr inbounds float addrspace(1)* %1, i64 %extract301vector_func.i
  %260 = getelementptr inbounds float addrspace(1)* %1, i64 %extract302vector_func.i
  store float %extract21vector_func.i, float addrspace(1)* %245, align 4
  store float %extract22vector_func.i, float addrspace(1)* %246, align 4
  store float %extract23vector_func.i, float addrspace(1)* %247, align 4
  store float %extract24vector_func.i, float addrspace(1)* %248, align 4
  store float %extract25vector_func.i, float addrspace(1)* %249, align 4
  store float %extract26vector_func.i, float addrspace(1)* %250, align 4
  store float %extract27vector_func.i, float addrspace(1)* %251, align 4
  store float %extract28vector_func.i, float addrspace(1)* %252, align 4
  store float %extract29vector_func.i, float addrspace(1)* %253, align 4
  store float %extract30vector_func.i, float addrspace(1)* %254, align 4
  store float %extract31vector_func.i, float addrspace(1)* %255, align 4
  store float %extract32vector_func.i, float addrspace(1)* %256, align 4
  store float %extract33vector_func.i, float addrspace(1)* %257, align 4
  store float %extract34vector_func.i, float addrspace(1)* %258, align 4
  store float %extract35vector_func.i, float addrspace(1)* %259, align 4
  store float %extract36vector_func.i, float addrspace(1)* %260, align 4
  %add92303vector_func.i = add nsw <16 x i32> %vectorPHIvector_func.i, <i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15>
  %and94304vector_func.i = and <16 x i32> %add92303vector_func.i, %vectorvector_func.i
  %idxprom95305vector_func.i = sext <16 x i32> %and94304vector_func.i to <16 x i64>
  %extract306vector_func.i = extractelement <16 x i64> %idxprom95305vector_func.i, i32 0
  %extract307vector_func.i = extractelement <16 x i64> %idxprom95305vector_func.i, i32 1
  %extract308vector_func.i = extractelement <16 x i64> %idxprom95305vector_func.i, i32 2
  %extract309vector_func.i = extractelement <16 x i64> %idxprom95305vector_func.i, i32 3
  %extract310vector_func.i = extractelement <16 x i64> %idxprom95305vector_func.i, i32 4
  %extract311vector_func.i = extractelement <16 x i64> %idxprom95305vector_func.i, i32 5
  %extract312vector_func.i = extractelement <16 x i64> %idxprom95305vector_func.i, i32 6
  %extract313vector_func.i = extractelement <16 x i64> %idxprom95305vector_func.i, i32 7
  %extract314vector_func.i = extractelement <16 x i64> %idxprom95305vector_func.i, i32 8
  %extract315vector_func.i = extractelement <16 x i64> %idxprom95305vector_func.i, i32 9
  %extract316vector_func.i = extractelement <16 x i64> %idxprom95305vector_func.i, i32 10
  %extract317vector_func.i = extractelement <16 x i64> %idxprom95305vector_func.i, i32 11
  %extract318vector_func.i = extractelement <16 x i64> %idxprom95305vector_func.i, i32 12
  %extract319vector_func.i = extractelement <16 x i64> %idxprom95305vector_func.i, i32 13
  %extract320vector_func.i = extractelement <16 x i64> %idxprom95305vector_func.i, i32 14
  %extract321vector_func.i = extractelement <16 x i64> %idxprom95305vector_func.i, i32 15
  %261 = getelementptr inbounds float addrspace(1)* %1, i64 %extract306vector_func.i
  %262 = getelementptr inbounds float addrspace(1)* %1, i64 %extract307vector_func.i
  %263 = getelementptr inbounds float addrspace(1)* %1, i64 %extract308vector_func.i
  %264 = getelementptr inbounds float addrspace(1)* %1, i64 %extract309vector_func.i
  %265 = getelementptr inbounds float addrspace(1)* %1, i64 %extract310vector_func.i
  %266 = getelementptr inbounds float addrspace(1)* %1, i64 %extract311vector_func.i
  %267 = getelementptr inbounds float addrspace(1)* %1, i64 %extract312vector_func.i
  %268 = getelementptr inbounds float addrspace(1)* %1, i64 %extract313vector_func.i
  %269 = getelementptr inbounds float addrspace(1)* %1, i64 %extract314vector_func.i
  %270 = getelementptr inbounds float addrspace(1)* %1, i64 %extract315vector_func.i
  %271 = getelementptr inbounds float addrspace(1)* %1, i64 %extract316vector_func.i
  %272 = getelementptr inbounds float addrspace(1)* %1, i64 %extract317vector_func.i
  %273 = getelementptr inbounds float addrspace(1)* %1, i64 %extract318vector_func.i
  %274 = getelementptr inbounds float addrspace(1)* %1, i64 %extract319vector_func.i
  %275 = getelementptr inbounds float addrspace(1)* %1, i64 %extract320vector_func.i
  %276 = getelementptr inbounds float addrspace(1)* %1, i64 %extract321vector_func.i
  store float %extract21vector_func.i, float addrspace(1)* %261, align 4
  store float %extract22vector_func.i, float addrspace(1)* %262, align 4
  store float %extract23vector_func.i, float addrspace(1)* %263, align 4
  store float %extract24vector_func.i, float addrspace(1)* %264, align 4
  store float %extract25vector_func.i, float addrspace(1)* %265, align 4
  store float %extract26vector_func.i, float addrspace(1)* %266, align 4
  store float %extract27vector_func.i, float addrspace(1)* %267, align 4
  store float %extract28vector_func.i, float addrspace(1)* %268, align 4
  store float %extract29vector_func.i, float addrspace(1)* %269, align 4
  store float %extract30vector_func.i, float addrspace(1)* %270, align 4
  store float %extract31vector_func.i, float addrspace(1)* %271, align 4
  store float %extract32vector_func.i, float addrspace(1)* %272, align 4
  store float %extract33vector_func.i, float addrspace(1)* %273, align 4
  store float %extract34vector_func.i, float addrspace(1)* %274, align 4
  store float %extract35vector_func.i, float addrspace(1)* %275, align 4
  store float %extract36vector_func.i, float addrspace(1)* %276, align 4
  %add97322vector_func.i = add nsw <16 x i32> %vectorPHIvector_func.i, <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>
  %and99323vector_func.i = and <16 x i32> %add97322vector_func.i, %vectorvector_func.i
  %incvector_func.i = add nsw i32 %j.01vector_func.i, 1
  %exitcondvector_func.i = icmp eq i32 %incvector_func.i, 512
  br i1 %exitcondvector_func.i, label %for.endvector_func.i, label %for.bodyvector_func.i

for.endvector_func.i:                             ; preds = %for.bodyvector_func.i
  %dim_0_vector_inc_ind_var.i = add i64 %dim_0_vector_ind_var.i, 1
  %dim_0_vector_cmp.to.max.i = icmp eq i64 %dim_0_vector_inc_ind_var.i, %vector.size.i
  %dim_0_vector_inc_tid.i = add i64 %dim_0_vector_tid.i, 16
  br i1 %dim_0_vector_cmp.to.max.i, label %dim_0_vector_exit.i, label %entryvector_func.i

dim_0_vector_exit.i:                              ; preds = %for.endvector_func.i
  %dim_1_vector_inc_ind_var.i = add i64 %dim_1_vector_ind_var.i, 1
  %dim_1_vector_cmp.to.max.i = icmp eq i64 %dim_1_vector_inc_ind_var.i, %16
  br i1 %dim_1_vector_cmp.to.max.i, label %dim_1_vector_exit.i, label %dim_0_vector_pre_head.i

dim_1_vector_exit.i:                              ; preds = %dim_0_vector_exit.i
  %dim_2_vector_inc_ind_var.i = add i64 %dim_2_vector_ind_var.i, 1
  %dim_2_vector_cmp.to.max.i = icmp eq i64 %dim_2_vector_inc_ind_var.i, %18
  br i1 %dim_2_vector_cmp.to.max.i, label %scalarIf.i, label %dim_1_vector_pre_head.i

scalarIf.i:                                       ; preds = %dim_1_vector_exit.i, %entry
  %277 = icmp eq i64 %12, %num.vector.wi.i
  br i1 %277, label %__writeGlobalMemoryUnit_separated_args.exit, label %dim_2_pre_head.i

dim_2_pre_head.i:                                 ; preds = %scalarIf.i
  %sub.i = add nsw i32 %4, -1
  br label %dim_1_pre_head.i

dim_1_pre_head.i:                                 ; preds = %dim_1_exit.i, %dim_2_pre_head.i
  %dim_2_ind_var.i = phi i64 [ 0, %dim_2_pre_head.i ], [ %dim_2_inc_ind_var.i, %dim_1_exit.i ]
  br label %dim_0_pre_head.i

dim_0_pre_head.i:                                 ; preds = %dim_0_exit.i, %dim_1_pre_head.i
  %dim_1_ind_var.i = phi i64 [ 0, %dim_1_pre_head.i ], [ %dim_1_inc_ind_var.i, %dim_0_exit.i ]
  br label %scalar_kernel_entry.i

scalar_kernel_entry.i:                            ; preds = %for.end.i, %dim_0_pre_head.i
  %dim_0_ind_var.i = phi i64 [ 0, %dim_0_pre_head.i ], [ %dim_0_inc_ind_var.i, %for.end.i ]
  %dim_0_tid.i = phi i64 [ %max.vector.gid.i, %dim_0_pre_head.i ], [ %dim_0_inc_tid.i, %for.end.i ]
  %conv.i = trunc i64 %dim_0_tid.i to i32
  %mul.i = shl nsw i32 %conv.i, 10
  %conv6.i = sitofp i32 %conv.i to float
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %scalar_kernel_entry.i
  %s.02.i = phi i32 [ %mul.i, %scalar_kernel_entry.i ], [ %and99.i, %for.body.i ]
  %j.01.i = phi i32 [ 0, %scalar_kernel_entry.i ], [ %inc.i, %for.body.i ]
  %and.i = and i32 %s.02.i, %sub.i
  %idxprom.i = sext i32 %and.i to i64
  %arrayidx.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom.i
  store float %conv6.i, float addrspace(1)* %arrayidx.i, align 4
  %add8.i = add nsw i32 %s.02.i, 1
  %and10.i = and i32 %add8.i, %sub.i
  %idxprom11.i = sext i32 %and10.i to i64
  %arrayidx12.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom11.i
  store float %conv6.i, float addrspace(1)* %arrayidx12.i, align 4
  %add14.i = add nsw i32 %s.02.i, 2
  %and16.i = and i32 %add14.i, %sub.i
  %idxprom17.i = sext i32 %and16.i to i64
  %arrayidx18.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom17.i
  store float %conv6.i, float addrspace(1)* %arrayidx18.i, align 4
  %add20.i = add nsw i32 %s.02.i, 3
  %and22.i = and i32 %add20.i, %sub.i
  %idxprom23.i = sext i32 %and22.i to i64
  %arrayidx24.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom23.i
  store float %conv6.i, float addrspace(1)* %arrayidx24.i, align 4
  %add26.i = add nsw i32 %s.02.i, 4
  %and28.i = and i32 %add26.i, %sub.i
  %idxprom29.i = sext i32 %and28.i to i64
  %arrayidx30.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom29.i
  store float %conv6.i, float addrspace(1)* %arrayidx30.i, align 4
  %add32.i = add nsw i32 %s.02.i, 5
  %and34.i = and i32 %add32.i, %sub.i
  %idxprom35.i = sext i32 %and34.i to i64
  %arrayidx36.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom35.i
  store float %conv6.i, float addrspace(1)* %arrayidx36.i, align 4
  %add38.i = add nsw i32 %s.02.i, 6
  %and40.i = and i32 %add38.i, %sub.i
  %idxprom41.i = sext i32 %and40.i to i64
  %arrayidx42.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom41.i
  store float %conv6.i, float addrspace(1)* %arrayidx42.i, align 4
  %add44.i = add nsw i32 %s.02.i, 7
  %and46.i = and i32 %add44.i, %sub.i
  %idxprom47.i = sext i32 %and46.i to i64
  %arrayidx48.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom47.i
  store float %conv6.i, float addrspace(1)* %arrayidx48.i, align 4
  %add50.i = add nsw i32 %s.02.i, 8
  %and52.i = and i32 %add50.i, %sub.i
  %idxprom53.i = sext i32 %and52.i to i64
  %arrayidx54.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom53.i
  store float %conv6.i, float addrspace(1)* %arrayidx54.i, align 4
  %add56.i = add nsw i32 %s.02.i, 9
  %and58.i = and i32 %add56.i, %sub.i
  %idxprom59.i = sext i32 %and58.i to i64
  %arrayidx60.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom59.i
  store float %conv6.i, float addrspace(1)* %arrayidx60.i, align 4
  %add62.i = add nsw i32 %s.02.i, 10
  %and64.i = and i32 %add62.i, %sub.i
  %idxprom65.i = sext i32 %and64.i to i64
  %arrayidx66.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom65.i
  store float %conv6.i, float addrspace(1)* %arrayidx66.i, align 4
  %add68.i = add nsw i32 %s.02.i, 11
  %and70.i = and i32 %add68.i, %sub.i
  %idxprom71.i = sext i32 %and70.i to i64
  %arrayidx72.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom71.i
  store float %conv6.i, float addrspace(1)* %arrayidx72.i, align 4
  %add74.i = add nsw i32 %s.02.i, 12
  %and76.i = and i32 %add74.i, %sub.i
  %idxprom77.i = sext i32 %and76.i to i64
  %arrayidx78.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom77.i
  store float %conv6.i, float addrspace(1)* %arrayidx78.i, align 4
  %add80.i = add nsw i32 %s.02.i, 13
  %and82.i = and i32 %add80.i, %sub.i
  %idxprom83.i = sext i32 %and82.i to i64
  %arrayidx84.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom83.i
  store float %conv6.i, float addrspace(1)* %arrayidx84.i, align 4
  %add86.i = add nsw i32 %s.02.i, 14
  %and88.i = and i32 %add86.i, %sub.i
  %idxprom89.i = sext i32 %and88.i to i64
  %arrayidx90.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom89.i
  store float %conv6.i, float addrspace(1)* %arrayidx90.i, align 4
  %add92.i = add nsw i32 %s.02.i, 15
  %and94.i = and i32 %add92.i, %sub.i
  %idxprom95.i = sext i32 %and94.i to i64
  %arrayidx96.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom95.i
  store float %conv6.i, float addrspace(1)* %arrayidx96.i, align 4
  %add97.i = add nsw i32 %s.02.i, 16
  %and99.i = and i32 %add97.i, %sub.i
  %inc.i = add nsw i32 %j.01.i, 1
  %exitcond.i = icmp eq i32 %inc.i, 512
  br i1 %exitcond.i, label %for.end.i, label %for.body.i

for.end.i:                                        ; preds = %for.body.i
  %dim_0_inc_ind_var.i = add i64 %dim_0_ind_var.i, 1
  %dim_0_cmp.to.max.i = icmp eq i64 %dim_0_inc_ind_var.i, %scalar.size.i
  %dim_0_inc_tid.i = add i64 %dim_0_tid.i, 1
  br i1 %dim_0_cmp.to.max.i, label %dim_0_exit.i, label %scalar_kernel_entry.i

dim_0_exit.i:                                     ; preds = %for.end.i
  %dim_1_inc_ind_var.i = add i64 %dim_1_ind_var.i, 1
  %dim_1_cmp.to.max.i = icmp eq i64 %dim_1_inc_ind_var.i, %16
  br i1 %dim_1_cmp.to.max.i, label %dim_1_exit.i, label %dim_0_pre_head.i

dim_1_exit.i:                                     ; preds = %dim_0_exit.i
  %dim_2_inc_ind_var.i = add i64 %dim_2_ind_var.i, 1
  %dim_2_cmp.to.max.i = icmp eq i64 %dim_2_inc_ind_var.i, %18
  br i1 %dim_2_cmp.to.max.i, label %__writeGlobalMemoryUnit_separated_args.exit, label %dim_1_pre_head.i

__writeGlobalMemoryUnit_separated_args.exit:      ; preds = %scalarIf.i, %dim_1_exit.i
  ret void
}

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}
!cl.noBarrierPath.kernels = !{!3}
!opencl.wrappers = !{!4}

!0 = metadata !{void (float addrspace(1)*, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__writeGlobalMemoryUnit_separated_args, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3}
!2 = metadata !{}
!3 = metadata !{metadata !"writeGlobalMemoryUnit"}
!4 = metadata !{void (i8*)* @writeGlobalMemoryUnit}
