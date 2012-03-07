; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @__compute_lj_force_original(<4 x float> addrspace(1)* nocapture, <4 x float> addrspace(1)* nocapture, i32, i32 addrspace(1)* nocapture, float, float, float, i32) nounwind

declare i64 @get_global_id(i32) nounwind readnone

declare [7 x i64] @__WG.boundaries.compute_lj_force_original(<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32 addrspace(1)*, float, float, float, i32)

declare i64 @get_local_size(i32)

declare i64 @get_base_global_id.(i32)

declare i1 @allOne(i1)

declare i1 @allZero(i1)

declare i32 @masked_load_align4_0(i1, i32 addrspace(1)*)

declare <4 x float> @masked_load_align16_1(i1, <4 x float> addrspace(1)*)

declare i1 @allOne_v16(<16 x i1>)

declare void @__compute_lj_force_separated_args(<4 x float> addrspace(1)* nocapture, <4 x float> addrspace(1)* nocapture, i32, i32 addrspace(1)* nocapture, float, float, float, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare [7 x i64] @WG.boundaries.compute_lj_force(<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32 addrspace(1)*, float, float, float, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)

declare i32 @llvm.x86.mic.kortestc(i16, i16) nounwind readnone

define void @compute_lj_force(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <4 x float> addrspace(1)**
  %1 = load <4 x float> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to <4 x float> addrspace(1)**
  %4 = load <4 x float> addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32 addrspace(1)**
  %10 = load i32 addrspace(1)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to float*
  %13 = load float* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 36
  %15 = bitcast i8* %14 to float*
  %16 = load float* %15, align 4
  %17 = getelementptr i8* %pBuffer, i64 40
  %18 = bitcast i8* %17 to float*
  %19 = load float* %18, align 4
  %20 = getelementptr i8* %pBuffer, i64 44
  %21 = bitcast i8* %20 to i32*
  %22 = load i32* %21, align 4
  %23 = getelementptr i8* %pBuffer, i64 56
  %24 = bitcast i8* %23 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %25 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 72
  %27 = bitcast i8* %26 to <{ [4 x i64] }>**
  %28 = load <{ [4 x i64] }>** %27, align 8
  %29 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %25, i64 0, i32 3, i64 0
  %30 = load i64* %29, align 8
  %31 = getelementptr <{ [4 x i64] }>* %28, i64 0, i32 0, i64 0
  %32 = load i64* %31, align 8
  %33 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %25, i64 0, i32 3, i64 1
  %34 = load i64* %33, align 8
  %35 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %25, i64 0, i32 3, i64 2
  %36 = load i64* %35, align 8
  %vector.size.i = ashr i64 %30, 4
  %num.vector.wi.i = and i64 %30, -16
  %max.vector.gid.i = add i64 %num.vector.wi.i, %32
  %scalar.size.i = sub i64 %30, %num.vector.wi.i
  %37 = icmp eq i64 %vector.size.i, 0
  br i1 %37, label %scalarIf.i, label %dim_2_vector_pre_head.i

dim_2_vector_pre_head.i:                          ; preds = %entry
  %temp270vector_func.i = insertelement <16 x float> undef, float %19, i32 0
  %vector271vector_func.i = shufflevector <16 x float> %temp270vector_func.i, <16 x float> undef, <16 x i32> zeroinitializer
  %temp267vector_func.i = insertelement <16 x float> undef, float %16, i32 0
  %vector268vector_func.i = shufflevector <16 x float> %temp267vector_func.i, <16 x float> undef, <16 x i32> zeroinitializer
  %temp258vector_func.i = insertelement <16 x float> undef, float %13, i32 0
  %vector259vector_func.i = shufflevector <16 x float> %temp258vector_func.i, <16 x float> undef, <16 x i32> zeroinitializer
  %cmp1vector_func.i = icmp sgt i32 %7, 0
  %temp79vector_func.i = insertelement <16 x i1> undef, i1 %cmp1vector_func.i, i32 0
  %vector80vector_func.i = shufflevector <16 x i1> %temp79vector_func.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %negIncomingLoopMaskvector_func.i = xor i1 %cmp1vector_func.i, true
  %tempvector_func.i = insertelement <16 x i1> undef, i1 %negIncomingLoopMaskvector_func.i, i32 0
  %vectorvector_func.i = shufflevector <16 x i1> %tempvector_func.i, <16 x i1> undef, <16 x i32> zeroinitializer
  br label %dim_1_vector_pre_head.i

dim_1_vector_pre_head.i:                          ; preds = %dim_1_vector_exit.i, %dim_2_vector_pre_head.i
  %dim_2_vector_ind_var.i = phi i64 [ 0, %dim_2_vector_pre_head.i ], [ %dim_2_vector_inc_ind_var.i, %dim_1_vector_exit.i ]
  br label %dim_0_vector_pre_head.i

dim_0_vector_pre_head.i:                          ; preds = %dim_0_vector_exit.i, %dim_1_vector_pre_head.i
  %dim_1_vector_ind_var.i = phi i64 [ 0, %dim_1_vector_pre_head.i ], [ %dim_1_vector_inc_ind_var.i, %dim_0_vector_exit.i ]
  br label %entryvector_func.i

entryvector_func.i:                               ; preds = %while.endvector_func.i, %dim_0_vector_pre_head.i
  %dim_0_vector_ind_var.i = phi i64 [ 0, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_ind_var.i, %while.endvector_func.i ]
  %dim_0_vector_tid.i = phi i64 [ %32, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_tid.i, %while.endvector_func.i ]
  %broadcast1vector_func.i = insertelement <16 x i64> undef, i64 %dim_0_vector_tid.i, i32 0
  %broadcast2vector_func.i = shufflevector <16 x i64> %broadcast1vector_func.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %38 = add <16 x i64> %broadcast2vector_func.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %conv56vector_func.i = trunc <16 x i64> %38 to <16 x i32>
  %idxprom57vector_func.i = and <16 x i64> %38, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extractvector_func.i = extractelement <16 x i64> %idxprom57vector_func.i, i32 0
  %extract58vector_func.i = extractelement <16 x i64> %idxprom57vector_func.i, i32 1
  %extract59vector_func.i = extractelement <16 x i64> %idxprom57vector_func.i, i32 2
  %extract60vector_func.i = extractelement <16 x i64> %idxprom57vector_func.i, i32 3
  %extract61vector_func.i = extractelement <16 x i64> %idxprom57vector_func.i, i32 4
  %extract62vector_func.i = extractelement <16 x i64> %idxprom57vector_func.i, i32 5
  %extract63vector_func.i = extractelement <16 x i64> %idxprom57vector_func.i, i32 6
  %extract64vector_func.i = extractelement <16 x i64> %idxprom57vector_func.i, i32 7
  %extract65vector_func.i = extractelement <16 x i64> %idxprom57vector_func.i, i32 8
  %extract66vector_func.i = extractelement <16 x i64> %idxprom57vector_func.i, i32 9
  %extract67vector_func.i = extractelement <16 x i64> %idxprom57vector_func.i, i32 10
  %extract68vector_func.i = extractelement <16 x i64> %idxprom57vector_func.i, i32 11
  %extract69vector_func.i = extractelement <16 x i64> %idxprom57vector_func.i, i32 12
  %extract70vector_func.i = extractelement <16 x i64> %idxprom57vector_func.i, i32 13
  %extract71vector_func.i = extractelement <16 x i64> %idxprom57vector_func.i, i32 14
  %extract72vector_func.i = extractelement <16 x i64> %idxprom57vector_func.i, i32 15
  %39 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extractvector_func.i
  %40 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract58vector_func.i
  %41 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract59vector_func.i
  %42 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract60vector_func.i
  %43 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract61vector_func.i
  %44 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract62vector_func.i
  %45 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract63vector_func.i
  %46 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract64vector_func.i
  %47 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract65vector_func.i
  %48 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract66vector_func.i
  %49 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract67vector_func.i
  %50 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract68vector_func.i
  %51 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract69vector_func.i
  %52 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract70vector_func.i
  %53 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract71vector_func.i
  %54 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract72vector_func.i
  %55 = load <4 x float> addrspace(1)* %39, align 16
  %56 = load <4 x float> addrspace(1)* %40, align 16
  %57 = load <4 x float> addrspace(1)* %41, align 16
  %58 = load <4 x float> addrspace(1)* %42, align 16
  %59 = load <4 x float> addrspace(1)* %43, align 16
  %60 = load <4 x float> addrspace(1)* %44, align 16
  %61 = load <4 x float> addrspace(1)* %45, align 16
  %62 = load <4 x float> addrspace(1)* %46, align 16
  %63 = load <4 x float> addrspace(1)* %47, align 16
  %64 = load <4 x float> addrspace(1)* %48, align 16
  %65 = load <4 x float> addrspace(1)* %49, align 16
  %66 = load <4 x float> addrspace(1)* %50, align 16
  %67 = load <4 x float> addrspace(1)* %51, align 16
  %68 = load <4 x float> addrspace(1)* %52, align 16
  %69 = load <4 x float> addrspace(1)* %53, align 16
  %70 = load <4 x float> addrspace(1)* %54, align 16
  %71 = extractelement <4 x float> %55, i32 0
  %72 = extractelement <4 x float> %56, i32 0
  %73 = extractelement <4 x float> %57, i32 0
  %74 = extractelement <4 x float> %58, i32 0
  %75 = extractelement <4 x float> %59, i32 0
  %76 = extractelement <4 x float> %60, i32 0
  %77 = extractelement <4 x float> %61, i32 0
  %78 = extractelement <4 x float> %62, i32 0
  %79 = extractelement <4 x float> %63, i32 0
  %80 = extractelement <4 x float> %64, i32 0
  %81 = extractelement <4 x float> %65, i32 0
  %82 = extractelement <4 x float> %66, i32 0
  %83 = extractelement <4 x float> %67, i32 0
  %84 = extractelement <4 x float> %68, i32 0
  %85 = extractelement <4 x float> %69, i32 0
  %86 = extractelement <4 x float> %70, i32 0
  %temp.vect154vector_func.i = insertelement <16 x float> undef, float %71, i32 0
  %temp.vect155vector_func.i = insertelement <16 x float> %temp.vect154vector_func.i, float %72, i32 1
  %temp.vect156vector_func.i = insertelement <16 x float> %temp.vect155vector_func.i, float %73, i32 2
  %temp.vect157vector_func.i = insertelement <16 x float> %temp.vect156vector_func.i, float %74, i32 3
  %temp.vect158vector_func.i = insertelement <16 x float> %temp.vect157vector_func.i, float %75, i32 4
  %temp.vect159vector_func.i = insertelement <16 x float> %temp.vect158vector_func.i, float %76, i32 5
  %temp.vect160vector_func.i = insertelement <16 x float> %temp.vect159vector_func.i, float %77, i32 6
  %temp.vect161vector_func.i = insertelement <16 x float> %temp.vect160vector_func.i, float %78, i32 7
  %temp.vect162vector_func.i = insertelement <16 x float> %temp.vect161vector_func.i, float %79, i32 8
  %temp.vect163vector_func.i = insertelement <16 x float> %temp.vect162vector_func.i, float %80, i32 9
  %temp.vect164vector_func.i = insertelement <16 x float> %temp.vect163vector_func.i, float %81, i32 10
  %temp.vect165vector_func.i = insertelement <16 x float> %temp.vect164vector_func.i, float %82, i32 11
  %temp.vect166vector_func.i = insertelement <16 x float> %temp.vect165vector_func.i, float %83, i32 12
  %temp.vect167vector_func.i = insertelement <16 x float> %temp.vect166vector_func.i, float %84, i32 13
  %temp.vect168vector_func.i = insertelement <16 x float> %temp.vect167vector_func.i, float %85, i32 14
  %temp.vect169vector_func.i = insertelement <16 x float> %temp.vect168vector_func.i, float %86, i32 15
  %87 = extractelement <4 x float> %55, i32 1
  %88 = extractelement <4 x float> %56, i32 1
  %89 = extractelement <4 x float> %57, i32 1
  %90 = extractelement <4 x float> %58, i32 1
  %91 = extractelement <4 x float> %59, i32 1
  %92 = extractelement <4 x float> %60, i32 1
  %93 = extractelement <4 x float> %61, i32 1
  %94 = extractelement <4 x float> %62, i32 1
  %95 = extractelement <4 x float> %63, i32 1
  %96 = extractelement <4 x float> %64, i32 1
  %97 = extractelement <4 x float> %65, i32 1
  %98 = extractelement <4 x float> %66, i32 1
  %99 = extractelement <4 x float> %67, i32 1
  %100 = extractelement <4 x float> %68, i32 1
  %101 = extractelement <4 x float> %69, i32 1
  %102 = extractelement <4 x float> %70, i32 1
  %temp.vect187vector_func.i = insertelement <16 x float> undef, float %87, i32 0
  %temp.vect188vector_func.i = insertelement <16 x float> %temp.vect187vector_func.i, float %88, i32 1
  %temp.vect189vector_func.i = insertelement <16 x float> %temp.vect188vector_func.i, float %89, i32 2
  %temp.vect190vector_func.i = insertelement <16 x float> %temp.vect189vector_func.i, float %90, i32 3
  %temp.vect191vector_func.i = insertelement <16 x float> %temp.vect190vector_func.i, float %91, i32 4
  %temp.vect192vector_func.i = insertelement <16 x float> %temp.vect191vector_func.i, float %92, i32 5
  %temp.vect193vector_func.i = insertelement <16 x float> %temp.vect192vector_func.i, float %93, i32 6
  %temp.vect194vector_func.i = insertelement <16 x float> %temp.vect193vector_func.i, float %94, i32 7
  %temp.vect195vector_func.i = insertelement <16 x float> %temp.vect194vector_func.i, float %95, i32 8
  %temp.vect196vector_func.i = insertelement <16 x float> %temp.vect195vector_func.i, float %96, i32 9
  %temp.vect197vector_func.i = insertelement <16 x float> %temp.vect196vector_func.i, float %97, i32 10
  %temp.vect198vector_func.i = insertelement <16 x float> %temp.vect197vector_func.i, float %98, i32 11
  %temp.vect199vector_func.i = insertelement <16 x float> %temp.vect198vector_func.i, float %99, i32 12
  %temp.vect200vector_func.i = insertelement <16 x float> %temp.vect199vector_func.i, float %100, i32 13
  %temp.vect201vector_func.i = insertelement <16 x float> %temp.vect200vector_func.i, float %101, i32 14
  %temp.vect202vector_func.i = insertelement <16 x float> %temp.vect201vector_func.i, float %102, i32 15
  %103 = extractelement <4 x float> %55, i32 2
  %104 = extractelement <4 x float> %56, i32 2
  %105 = extractelement <4 x float> %57, i32 2
  %106 = extractelement <4 x float> %58, i32 2
  %107 = extractelement <4 x float> %59, i32 2
  %108 = extractelement <4 x float> %60, i32 2
  %109 = extractelement <4 x float> %61, i32 2
  %110 = extractelement <4 x float> %62, i32 2
  %111 = extractelement <4 x float> %63, i32 2
  %112 = extractelement <4 x float> %64, i32 2
  %113 = extractelement <4 x float> %65, i32 2
  %114 = extractelement <4 x float> %66, i32 2
  %115 = extractelement <4 x float> %67, i32 2
  %116 = extractelement <4 x float> %68, i32 2
  %117 = extractelement <4 x float> %69, i32 2
  %118 = extractelement <4 x float> %70, i32 2
  %temp.vect220vector_func.i = insertelement <16 x float> undef, float %103, i32 0
  %temp.vect221vector_func.i = insertelement <16 x float> %temp.vect220vector_func.i, float %104, i32 1
  %temp.vect222vector_func.i = insertelement <16 x float> %temp.vect221vector_func.i, float %105, i32 2
  %temp.vect223vector_func.i = insertelement <16 x float> %temp.vect222vector_func.i, float %106, i32 3
  %temp.vect224vector_func.i = insertelement <16 x float> %temp.vect223vector_func.i, float %107, i32 4
  %temp.vect225vector_func.i = insertelement <16 x float> %temp.vect224vector_func.i, float %108, i32 5
  %temp.vect226vector_func.i = insertelement <16 x float> %temp.vect225vector_func.i, float %109, i32 6
  %temp.vect227vector_func.i = insertelement <16 x float> %temp.vect226vector_func.i, float %110, i32 7
  %temp.vect228vector_func.i = insertelement <16 x float> %temp.vect227vector_func.i, float %111, i32 8
  %temp.vect229vector_func.i = insertelement <16 x float> %temp.vect228vector_func.i, float %112, i32 9
  %temp.vect230vector_func.i = insertelement <16 x float> %temp.vect229vector_func.i, float %113, i32 10
  %temp.vect231vector_func.i = insertelement <16 x float> %temp.vect230vector_func.i, float %114, i32 11
  %temp.vect232vector_func.i = insertelement <16 x float> %temp.vect231vector_func.i, float %115, i32 12
  %temp.vect233vector_func.i = insertelement <16 x float> %temp.vect232vector_func.i, float %116, i32 13
  %temp.vect234vector_func.i = insertelement <16 x float> %temp.vect233vector_func.i, float %117, i32 14
  %temp.vect235vector_func.i = insertelement <16 x float> %temp.vect234vector_func.i, float %118, i32 15
  br i1 %cmp1vector_func.i, label %while.bodyvector_func.i, label %while.endvector_func.i

while.bodyvector_func.i:                          ; preds = %postload493vector_func.i, %entryvector_func.i
  %vectorPHI73vector_func.i = phi <16 x i1> [ %loop_mask25294vector_func.i, %postload493vector_func.i ], [ %vectorvector_func.i, %entryvector_func.i ]
  %vectorPHI74vector_func.i = phi <16 x float> [ %out_sel289vector_func.i, %postload493vector_func.i ], [ undef, %entryvector_func.i ]
  %vectorPHI75vector_func.i = phi <16 x float> [ %out_sel45287vector_func.i, %postload493vector_func.i ], [ undef, %entryvector_func.i ]
  %vectorPHI76vector_func.i = phi <16 x float> [ %out_sel48285vector_func.i, %postload493vector_func.i ], [ undef, %entryvector_func.i ]
  %vectorPHI77vector_func.i = phi <16 x float> [ %out_sel51283vector_func.i, %postload493vector_func.i ], [ undef, %entryvector_func.i ]
  %vectorPHI78vector_func.i = phi <16 x i1> [ %local_edge298vector_func.i, %postload493vector_func.i ], [ %vector80vector_func.i, %entryvector_func.i ]
  %indvars.ivvector_func.i = phi i64 [ %indvars.iv.nextvector_func.i, %postload493vector_func.i ], [ 0, %entryvector_func.i ]
  %vectorPHI81vector_func.i = phi <16 x float> [ %merge288vector_func.i, %postload493vector_func.i ], [ zeroinitializer, %entryvector_func.i ]
  %vectorPHI82vector_func.i = phi <16 x float> [ %merge32286vector_func.i, %postload493vector_func.i ], [ zeroinitializer, %entryvector_func.i ]
  %vectorPHI83vector_func.i = phi <16 x float> [ %merge34284vector_func.i, %postload493vector_func.i ], [ zeroinitializer, %entryvector_func.i ]
  %extract105vector_func.i = extractelement <16 x i1> %vectorPHI78vector_func.i, i32 0
  %extract106vector_func.i = extractelement <16 x i1> %vectorPHI78vector_func.i, i32 1
  %extract107vector_func.i = extractelement <16 x i1> %vectorPHI78vector_func.i, i32 2
  %extract108vector_func.i = extractelement <16 x i1> %vectorPHI78vector_func.i, i32 3
  %extract109vector_func.i = extractelement <16 x i1> %vectorPHI78vector_func.i, i32 4
  %extract110vector_func.i = extractelement <16 x i1> %vectorPHI78vector_func.i, i32 5
  %extract111vector_func.i = extractelement <16 x i1> %vectorPHI78vector_func.i, i32 6
  %extract112vector_func.i = extractelement <16 x i1> %vectorPHI78vector_func.i, i32 7
  %extract113vector_func.i = extractelement <16 x i1> %vectorPHI78vector_func.i, i32 8
  %extract114vector_func.i = extractelement <16 x i1> %vectorPHI78vector_func.i, i32 9
  %extract115vector_func.i = extractelement <16 x i1> %vectorPHI78vector_func.i, i32 10
  %extract116vector_func.i = extractelement <16 x i1> %vectorPHI78vector_func.i, i32 11
  %extract117vector_func.i = extractelement <16 x i1> %vectorPHI78vector_func.i, i32 12
  %extract118vector_func.i = extractelement <16 x i1> %vectorPHI78vector_func.i, i32 13
  %extract119vector_func.i = extractelement <16 x i1> %vectorPHI78vector_func.i, i32 14
  %extract120vector_func.i = extractelement <16 x i1> %vectorPHI78vector_func.i, i32 15
  %119 = trunc i64 %indvars.ivvector_func.i to i32
  %mulvector_func.i = mul nsw i32 %119, %22
  %temp85vector_func.i = insertelement <16 x i32> undef, i32 %mulvector_func.i, i32 0
  %vector86vector_func.i = shufflevector <16 x i32> %temp85vector_func.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %add87vector_func.i = add <16 x i32> %vector86vector_func.i, %conv56vector_func.i
  %idxprom288vector_func.i = zext <16 x i32> %add87vector_func.i to <16 x i64>
  %extract90vector_func.i = extractelement <16 x i64> %idxprom288vector_func.i, i32 1
  %extract91vector_func.i = extractelement <16 x i64> %idxprom288vector_func.i, i32 2
  %extract92vector_func.i = extractelement <16 x i64> %idxprom288vector_func.i, i32 3
  %extract93vector_func.i = extractelement <16 x i64> %idxprom288vector_func.i, i32 4
  %extract94vector_func.i = extractelement <16 x i64> %idxprom288vector_func.i, i32 5
  %extract95vector_func.i = extractelement <16 x i64> %idxprom288vector_func.i, i32 6
  %extract96vector_func.i = extractelement <16 x i64> %idxprom288vector_func.i, i32 7
  %extract97vector_func.i = extractelement <16 x i64> %idxprom288vector_func.i, i32 8
  %extract98vector_func.i = extractelement <16 x i64> %idxprom288vector_func.i, i32 9
  %extract99vector_func.i = extractelement <16 x i64> %idxprom288vector_func.i, i32 10
  %extract100vector_func.i = extractelement <16 x i64> %idxprom288vector_func.i, i32 11
  %extract101vector_func.i = extractelement <16 x i64> %idxprom288vector_func.i, i32 12
  %extract102vector_func.i = extractelement <16 x i64> %idxprom288vector_func.i, i32 13
  %extract103vector_func.i = extractelement <16 x i64> %idxprom288vector_func.i, i32 14
  %extract104vector_func.i = extractelement <16 x i64> %idxprom288vector_func.i, i32 15
  %120 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract90vector_func.i
  %121 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract91vector_func.i
  %122 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract92vector_func.i
  %123 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract93vector_func.i
  %124 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract94vector_func.i
  %125 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract95vector_func.i
  %126 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract96vector_func.i
  %127 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract97vector_func.i
  %128 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract98vector_func.i
  %129 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract99vector_func.i
  %130 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract100vector_func.i
  %131 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract101vector_func.i
  %132 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract102vector_func.i
  %133 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract103vector_func.i
  %134 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract104vector_func.i
  br i1 %extract105vector_func.i, label %preload405vector_func.i, label %postload406vector_func.i

preload405vector_func.i:                          ; preds = %while.bodyvector_func.i
  %extract89vector_func.i = extractelement <16 x i64> %idxprom288vector_func.i, i32 0
  %135 = getelementptr inbounds i32 addrspace(1)* %10, i64 %extract89vector_func.i
  %masked_loadvector_func.i = load i32 addrspace(1)* %135, align 4
  br label %postload406vector_func.i

postload406vector_func.i:                         ; preds = %preload405vector_func.i, %while.bodyvector_func.i
  %phi407vector_func.i = phi i32 [ undef, %while.bodyvector_func.i ], [ %masked_loadvector_func.i, %preload405vector_func.i ]
  br i1 %extract106vector_func.i, label %preload411vector_func.i, label %postload412vector_func.i

preload411vector_func.i:                          ; preds = %postload406vector_func.i
  %masked_load371vector_func.i = load i32 addrspace(1)* %120, align 4
  br label %postload412vector_func.i

postload412vector_func.i:                         ; preds = %preload411vector_func.i, %postload406vector_func.i
  %phi413vector_func.i = phi i32 [ undef, %postload406vector_func.i ], [ %masked_load371vector_func.i, %preload411vector_func.i ]
  br i1 %extract107vector_func.i, label %preload417vector_func.i, label %postload418vector_func.i

preload417vector_func.i:                          ; preds = %postload412vector_func.i
  %masked_load372vector_func.i = load i32 addrspace(1)* %121, align 4
  br label %postload418vector_func.i

postload418vector_func.i:                         ; preds = %preload417vector_func.i, %postload412vector_func.i
  %phi419vector_func.i = phi i32 [ undef, %postload412vector_func.i ], [ %masked_load372vector_func.i, %preload417vector_func.i ]
  br i1 %extract108vector_func.i, label %preload423vector_func.i, label %postload424vector_func.i

preload423vector_func.i:                          ; preds = %postload418vector_func.i
  %masked_load373vector_func.i = load i32 addrspace(1)* %122, align 4
  br label %postload424vector_func.i

postload424vector_func.i:                         ; preds = %preload423vector_func.i, %postload418vector_func.i
  %phi425vector_func.i = phi i32 [ undef, %postload418vector_func.i ], [ %masked_load373vector_func.i, %preload423vector_func.i ]
  br i1 %extract109vector_func.i, label %preloadvector_func.i, label %postloadvector_func.i

preloadvector_func.i:                             ; preds = %postload424vector_func.i
  %masked_load374vector_func.i = load i32 addrspace(1)* %123, align 4
  br label %postloadvector_func.i

postloadvector_func.i:                            ; preds = %preloadvector_func.i, %postload424vector_func.i
  %phivector_func.i = phi i32 [ undef, %postload424vector_func.i ], [ %masked_load374vector_func.i, %preloadvector_func.i ]
  br i1 %extract110vector_func.i, label %preload429vector_func.i, label %postload430vector_func.i

preload429vector_func.i:                          ; preds = %postloadvector_func.i
  %masked_load375vector_func.i = load i32 addrspace(1)* %124, align 4
  br label %postload430vector_func.i

postload430vector_func.i:                         ; preds = %preload429vector_func.i, %postloadvector_func.i
  %phi431vector_func.i = phi i32 [ undef, %postloadvector_func.i ], [ %masked_load375vector_func.i, %preload429vector_func.i ]
  br i1 %extract111vector_func.i, label %preload435vector_func.i, label %postload436vector_func.i

preload435vector_func.i:                          ; preds = %postload430vector_func.i
  %masked_load376vector_func.i = load i32 addrspace(1)* %125, align 4
  br label %postload436vector_func.i

postload436vector_func.i:                         ; preds = %preload435vector_func.i, %postload430vector_func.i
  %phi437vector_func.i = phi i32 [ undef, %postload430vector_func.i ], [ %masked_load376vector_func.i, %preload435vector_func.i ]
  br i1 %extract112vector_func.i, label %preload441vector_func.i, label %postload442vector_func.i

preload441vector_func.i:                          ; preds = %postload436vector_func.i
  %masked_load377vector_func.i = load i32 addrspace(1)* %126, align 4
  br label %postload442vector_func.i

postload442vector_func.i:                         ; preds = %preload441vector_func.i, %postload436vector_func.i
  %phi443vector_func.i = phi i32 [ undef, %postload436vector_func.i ], [ %masked_load377vector_func.i, %preload441vector_func.i ]
  br i1 %extract113vector_func.i, label %preload447vector_func.i, label %postload448vector_func.i

preload447vector_func.i:                          ; preds = %postload442vector_func.i
  %masked_load378vector_func.i = load i32 addrspace(1)* %127, align 4
  br label %postload448vector_func.i

postload448vector_func.i:                         ; preds = %preload447vector_func.i, %postload442vector_func.i
  %phi449vector_func.i = phi i32 [ undef, %postload442vector_func.i ], [ %masked_load378vector_func.i, %preload447vector_func.i ]
  br i1 %extract114vector_func.i, label %preload453vector_func.i, label %postload454vector_func.i

preload453vector_func.i:                          ; preds = %postload448vector_func.i
  %masked_load379vector_func.i = load i32 addrspace(1)* %128, align 4
  br label %postload454vector_func.i

postload454vector_func.i:                         ; preds = %preload453vector_func.i, %postload448vector_func.i
  %phi455vector_func.i = phi i32 [ undef, %postload448vector_func.i ], [ %masked_load379vector_func.i, %preload453vector_func.i ]
  br i1 %extract115vector_func.i, label %preload459vector_func.i, label %postload460vector_func.i

preload459vector_func.i:                          ; preds = %postload454vector_func.i
  %masked_load380vector_func.i = load i32 addrspace(1)* %129, align 4
  br label %postload460vector_func.i

postload460vector_func.i:                         ; preds = %preload459vector_func.i, %postload454vector_func.i
  %phi461vector_func.i = phi i32 [ undef, %postload454vector_func.i ], [ %masked_load380vector_func.i, %preload459vector_func.i ]
  br i1 %extract116vector_func.i, label %preload465vector_func.i, label %postload466vector_func.i

preload465vector_func.i:                          ; preds = %postload460vector_func.i
  %masked_load381vector_func.i = load i32 addrspace(1)* %130, align 4
  br label %postload466vector_func.i

postload466vector_func.i:                         ; preds = %preload465vector_func.i, %postload460vector_func.i
  %phi467vector_func.i = phi i32 [ undef, %postload460vector_func.i ], [ %masked_load381vector_func.i, %preload465vector_func.i ]
  br i1 %extract117vector_func.i, label %preload471vector_func.i, label %postload472vector_func.i

preload471vector_func.i:                          ; preds = %postload466vector_func.i
  %masked_load382vector_func.i = load i32 addrspace(1)* %131, align 4
  br label %postload472vector_func.i

postload472vector_func.i:                         ; preds = %preload471vector_func.i, %postload466vector_func.i
  %phi473vector_func.i = phi i32 [ undef, %postload466vector_func.i ], [ %masked_load382vector_func.i, %preload471vector_func.i ]
  br i1 %extract118vector_func.i, label %preload477vector_func.i, label %postload478vector_func.i

preload477vector_func.i:                          ; preds = %postload472vector_func.i
  %masked_load383vector_func.i = load i32 addrspace(1)* %132, align 4
  br label %postload478vector_func.i

postload478vector_func.i:                         ; preds = %preload477vector_func.i, %postload472vector_func.i
  %phi479vector_func.i = phi i32 [ undef, %postload472vector_func.i ], [ %masked_load383vector_func.i, %preload477vector_func.i ]
  br i1 %extract119vector_func.i, label %preload483vector_func.i, label %postload484vector_func.i

preload483vector_func.i:                          ; preds = %postload478vector_func.i
  %masked_load384vector_func.i = load i32 addrspace(1)* %133, align 4
  br label %postload484vector_func.i

postload484vector_func.i:                         ; preds = %preload483vector_func.i, %postload478vector_func.i
  %phi485vector_func.i = phi i32 [ undef, %postload478vector_func.i ], [ %masked_load384vector_func.i, %preload483vector_func.i ]
  br i1 %extract120vector_func.i, label %preload489vector_func.i, label %postload490vector_func.i

preload489vector_func.i:                          ; preds = %postload484vector_func.i
  %masked_load385vector_func.i = load i32 addrspace(1)* %134, align 4
  br label %postload490vector_func.i

postload490vector_func.i:                         ; preds = %preload489vector_func.i, %postload484vector_func.i
  %phi491vector_func.i = phi i32 [ undef, %postload484vector_func.i ], [ %masked_load385vector_func.i, %preload489vector_func.i ]
  %temp.vect121vector_func.i = insertelement <16 x i32> undef, i32 %phi407vector_func.i, i32 0
  %temp.vect122vector_func.i = insertelement <16 x i32> %temp.vect121vector_func.i, i32 %phi413vector_func.i, i32 1
  %temp.vect123vector_func.i = insertelement <16 x i32> %temp.vect122vector_func.i, i32 %phi419vector_func.i, i32 2
  %temp.vect124vector_func.i = insertelement <16 x i32> %temp.vect123vector_func.i, i32 %phi425vector_func.i, i32 3
  %temp.vect125vector_func.i = insertelement <16 x i32> %temp.vect124vector_func.i, i32 %phivector_func.i, i32 4
  %temp.vect126vector_func.i = insertelement <16 x i32> %temp.vect125vector_func.i, i32 %phi431vector_func.i, i32 5
  %temp.vect127vector_func.i = insertelement <16 x i32> %temp.vect126vector_func.i, i32 %phi437vector_func.i, i32 6
  %temp.vect128vector_func.i = insertelement <16 x i32> %temp.vect127vector_func.i, i32 %phi443vector_func.i, i32 7
  %temp.vect129vector_func.i = insertelement <16 x i32> %temp.vect128vector_func.i, i32 %phi449vector_func.i, i32 8
  %temp.vect130vector_func.i = insertelement <16 x i32> %temp.vect129vector_func.i, i32 %phi455vector_func.i, i32 9
  %temp.vect131vector_func.i = insertelement <16 x i32> %temp.vect130vector_func.i, i32 %phi461vector_func.i, i32 10
  %temp.vect132vector_func.i = insertelement <16 x i32> %temp.vect131vector_func.i, i32 %phi467vector_func.i, i32 11
  %temp.vect133vector_func.i = insertelement <16 x i32> %temp.vect132vector_func.i, i32 %phi473vector_func.i, i32 12
  %temp.vect134vector_func.i = insertelement <16 x i32> %temp.vect133vector_func.i, i32 %phi479vector_func.i, i32 13
  %temp.vect135vector_func.i = insertelement <16 x i32> %temp.vect134vector_func.i, i32 %phi485vector_func.i, i32 14
  %temp.vect136vector_func.i = insertelement <16 x i32> %temp.vect135vector_func.i, i32 %phi491vector_func.i, i32 15
  %idxprom4137vector_func.i = sext <16 x i32> %temp.vect136vector_func.i to <16 x i64>
  %extract139vector_func.i = extractelement <16 x i64> %idxprom4137vector_func.i, i32 1
  %extract140vector_func.i = extractelement <16 x i64> %idxprom4137vector_func.i, i32 2
  %extract141vector_func.i = extractelement <16 x i64> %idxprom4137vector_func.i, i32 3
  %extract142vector_func.i = extractelement <16 x i64> %idxprom4137vector_func.i, i32 4
  %extract143vector_func.i = extractelement <16 x i64> %idxprom4137vector_func.i, i32 5
  %extract144vector_func.i = extractelement <16 x i64> %idxprom4137vector_func.i, i32 6
  %extract145vector_func.i = extractelement <16 x i64> %idxprom4137vector_func.i, i32 7
  %extract146vector_func.i = extractelement <16 x i64> %idxprom4137vector_func.i, i32 8
  %extract147vector_func.i = extractelement <16 x i64> %idxprom4137vector_func.i, i32 9
  %extract148vector_func.i = extractelement <16 x i64> %idxprom4137vector_func.i, i32 10
  %extract149vector_func.i = extractelement <16 x i64> %idxprom4137vector_func.i, i32 11
  %extract150vector_func.i = extractelement <16 x i64> %idxprom4137vector_func.i, i32 12
  %extract151vector_func.i = extractelement <16 x i64> %idxprom4137vector_func.i, i32 13
  %extract152vector_func.i = extractelement <16 x i64> %idxprom4137vector_func.i, i32 14
  %extract153vector_func.i = extractelement <16 x i64> %idxprom4137vector_func.i, i32 15
  %136 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract139vector_func.i
  %137 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract140vector_func.i
  %138 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract141vector_func.i
  %139 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract142vector_func.i
  %140 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract143vector_func.i
  %141 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract144vector_func.i
  %142 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract145vector_func.i
  %143 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract146vector_func.i
  %144 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract147vector_func.i
  %145 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract148vector_func.i
  %146 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract149vector_func.i
  %147 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract150vector_func.i
  %148 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract151vector_func.i
  %149 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract152vector_func.i
  %150 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract153vector_func.i
  br i1 %extract105vector_func.i, label %preload408vector_func.i, label %postload409vector_func.i

preload408vector_func.i:                          ; preds = %postload490vector_func.i
  %extract138vector_func.i = extractelement <16 x i64> %idxprom4137vector_func.i, i32 0
  %151 = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %extract138vector_func.i
  %masked_load386vector_func.i = load <4 x float> addrspace(1)* %151, align 16
  br label %postload409vector_func.i

postload409vector_func.i:                         ; preds = %preload408vector_func.i, %postload490vector_func.i
  %phi410vector_func.i = phi <4 x float> [ undef, %postload490vector_func.i ], [ %masked_load386vector_func.i, %preload408vector_func.i ]
  br i1 %extract106vector_func.i, label %preload414vector_func.i, label %postload415vector_func.i

preload414vector_func.i:                          ; preds = %postload409vector_func.i
  %masked_load387vector_func.i = load <4 x float> addrspace(1)* %136, align 16
  br label %postload415vector_func.i

postload415vector_func.i:                         ; preds = %preload414vector_func.i, %postload409vector_func.i
  %phi416vector_func.i = phi <4 x float> [ undef, %postload409vector_func.i ], [ %masked_load387vector_func.i, %preload414vector_func.i ]
  br i1 %extract107vector_func.i, label %preload420vector_func.i, label %postload421vector_func.i

preload420vector_func.i:                          ; preds = %postload415vector_func.i
  %masked_load388vector_func.i = load <4 x float> addrspace(1)* %137, align 16
  br label %postload421vector_func.i

postload421vector_func.i:                         ; preds = %preload420vector_func.i, %postload415vector_func.i
  %phi422vector_func.i = phi <4 x float> [ undef, %postload415vector_func.i ], [ %masked_load388vector_func.i, %preload420vector_func.i ]
  br i1 %extract108vector_func.i, label %preload426vector_func.i, label %postload427vector_func.i

preload426vector_func.i:                          ; preds = %postload421vector_func.i
  %masked_load389vector_func.i = load <4 x float> addrspace(1)* %138, align 16
  br label %postload427vector_func.i

postload427vector_func.i:                         ; preds = %preload426vector_func.i, %postload421vector_func.i
  %phi428vector_func.i = phi <4 x float> [ undef, %postload421vector_func.i ], [ %masked_load389vector_func.i, %preload426vector_func.i ]
  br i1 %extract109vector_func.i, label %preload402vector_func.i, label %postload403vector_func.i

preload402vector_func.i:                          ; preds = %postload427vector_func.i
  %masked_load390vector_func.i = load <4 x float> addrspace(1)* %139, align 16
  br label %postload403vector_func.i

postload403vector_func.i:                         ; preds = %preload402vector_func.i, %postload427vector_func.i
  %phi404vector_func.i = phi <4 x float> [ undef, %postload427vector_func.i ], [ %masked_load390vector_func.i, %preload402vector_func.i ]
  br i1 %extract110vector_func.i, label %preload432vector_func.i, label %postload433vector_func.i

preload432vector_func.i:                          ; preds = %postload403vector_func.i
  %masked_load391vector_func.i = load <4 x float> addrspace(1)* %140, align 16
  br label %postload433vector_func.i

postload433vector_func.i:                         ; preds = %preload432vector_func.i, %postload403vector_func.i
  %phi434vector_func.i = phi <4 x float> [ undef, %postload403vector_func.i ], [ %masked_load391vector_func.i, %preload432vector_func.i ]
  br i1 %extract111vector_func.i, label %preload438vector_func.i, label %postload439vector_func.i

preload438vector_func.i:                          ; preds = %postload433vector_func.i
  %masked_load392vector_func.i = load <4 x float> addrspace(1)* %141, align 16
  br label %postload439vector_func.i

postload439vector_func.i:                         ; preds = %preload438vector_func.i, %postload433vector_func.i
  %phi440vector_func.i = phi <4 x float> [ undef, %postload433vector_func.i ], [ %masked_load392vector_func.i, %preload438vector_func.i ]
  br i1 %extract112vector_func.i, label %preload444vector_func.i, label %postload445vector_func.i

preload444vector_func.i:                          ; preds = %postload439vector_func.i
  %masked_load393vector_func.i = load <4 x float> addrspace(1)* %142, align 16
  br label %postload445vector_func.i

postload445vector_func.i:                         ; preds = %preload444vector_func.i, %postload439vector_func.i
  %phi446vector_func.i = phi <4 x float> [ undef, %postload439vector_func.i ], [ %masked_load393vector_func.i, %preload444vector_func.i ]
  br i1 %extract113vector_func.i, label %preload450vector_func.i, label %postload451vector_func.i

preload450vector_func.i:                          ; preds = %postload445vector_func.i
  %masked_load394vector_func.i = load <4 x float> addrspace(1)* %143, align 16
  br label %postload451vector_func.i

postload451vector_func.i:                         ; preds = %preload450vector_func.i, %postload445vector_func.i
  %phi452vector_func.i = phi <4 x float> [ undef, %postload445vector_func.i ], [ %masked_load394vector_func.i, %preload450vector_func.i ]
  br i1 %extract114vector_func.i, label %preload456vector_func.i, label %postload457vector_func.i

preload456vector_func.i:                          ; preds = %postload451vector_func.i
  %masked_load395vector_func.i = load <4 x float> addrspace(1)* %144, align 16
  br label %postload457vector_func.i

postload457vector_func.i:                         ; preds = %preload456vector_func.i, %postload451vector_func.i
  %phi458vector_func.i = phi <4 x float> [ undef, %postload451vector_func.i ], [ %masked_load395vector_func.i, %preload456vector_func.i ]
  br i1 %extract115vector_func.i, label %preload462vector_func.i, label %postload463vector_func.i

preload462vector_func.i:                          ; preds = %postload457vector_func.i
  %masked_load396vector_func.i = load <4 x float> addrspace(1)* %145, align 16
  br label %postload463vector_func.i

postload463vector_func.i:                         ; preds = %preload462vector_func.i, %postload457vector_func.i
  %phi464vector_func.i = phi <4 x float> [ undef, %postload457vector_func.i ], [ %masked_load396vector_func.i, %preload462vector_func.i ]
  br i1 %extract116vector_func.i, label %preload468vector_func.i, label %postload469vector_func.i

preload468vector_func.i:                          ; preds = %postload463vector_func.i
  %masked_load397vector_func.i = load <4 x float> addrspace(1)* %146, align 16
  br label %postload469vector_func.i

postload469vector_func.i:                         ; preds = %preload468vector_func.i, %postload463vector_func.i
  %phi470vector_func.i = phi <4 x float> [ undef, %postload463vector_func.i ], [ %masked_load397vector_func.i, %preload468vector_func.i ]
  br i1 %extract117vector_func.i, label %preload474vector_func.i, label %postload475vector_func.i

preload474vector_func.i:                          ; preds = %postload469vector_func.i
  %masked_load398vector_func.i = load <4 x float> addrspace(1)* %147, align 16
  br label %postload475vector_func.i

postload475vector_func.i:                         ; preds = %preload474vector_func.i, %postload469vector_func.i
  %phi476vector_func.i = phi <4 x float> [ undef, %postload469vector_func.i ], [ %masked_load398vector_func.i, %preload474vector_func.i ]
  br i1 %extract118vector_func.i, label %preload480vector_func.i, label %postload481vector_func.i

preload480vector_func.i:                          ; preds = %postload475vector_func.i
  %masked_load399vector_func.i = load <4 x float> addrspace(1)* %148, align 16
  br label %postload481vector_func.i

postload481vector_func.i:                         ; preds = %preload480vector_func.i, %postload475vector_func.i
  %phi482vector_func.i = phi <4 x float> [ undef, %postload475vector_func.i ], [ %masked_load399vector_func.i, %preload480vector_func.i ]
  br i1 %extract119vector_func.i, label %preload486vector_func.i, label %postload487vector_func.i

preload486vector_func.i:                          ; preds = %postload481vector_func.i
  %masked_load400vector_func.i = load <4 x float> addrspace(1)* %149, align 16
  br label %postload487vector_func.i

postload487vector_func.i:                         ; preds = %preload486vector_func.i, %postload481vector_func.i
  %phi488vector_func.i = phi <4 x float> [ undef, %postload481vector_func.i ], [ %masked_load400vector_func.i, %preload486vector_func.i ]
  br i1 %extract120vector_func.i, label %preload492vector_func.i, label %postload493vector_func.i

preload492vector_func.i:                          ; preds = %postload487vector_func.i
  %masked_load401vector_func.i = load <4 x float> addrspace(1)* %150, align 16
  br label %postload493vector_func.i

postload493vector_func.i:                         ; preds = %preload492vector_func.i, %postload487vector_func.i
  %phi494vector_func.i = phi <4 x float> [ undef, %postload487vector_func.i ], [ %masked_load401vector_func.i, %preload492vector_func.i ]
  %152 = extractelement <4 x float> %phi410vector_func.i, i32 0
  %153 = extractelement <4 x float> %phi416vector_func.i, i32 0
  %154 = extractelement <4 x float> %phi422vector_func.i, i32 0
  %155 = extractelement <4 x float> %phi428vector_func.i, i32 0
  %156 = extractelement <4 x float> %phi404vector_func.i, i32 0
  %157 = extractelement <4 x float> %phi434vector_func.i, i32 0
  %158 = extractelement <4 x float> %phi440vector_func.i, i32 0
  %159 = extractelement <4 x float> %phi446vector_func.i, i32 0
  %160 = extractelement <4 x float> %phi452vector_func.i, i32 0
  %161 = extractelement <4 x float> %phi458vector_func.i, i32 0
  %162 = extractelement <4 x float> %phi464vector_func.i, i32 0
  %163 = extractelement <4 x float> %phi470vector_func.i, i32 0
  %164 = extractelement <4 x float> %phi476vector_func.i, i32 0
  %165 = extractelement <4 x float> %phi482vector_func.i, i32 0
  %166 = extractelement <4 x float> %phi488vector_func.i, i32 0
  %167 = extractelement <4 x float> %phi494vector_func.i, i32 0
  %temp.vect170vector_func.i = insertelement <16 x float> undef, float %152, i32 0
  %temp.vect171vector_func.i = insertelement <16 x float> %temp.vect170vector_func.i, float %153, i32 1
  %temp.vect172vector_func.i = insertelement <16 x float> %temp.vect171vector_func.i, float %154, i32 2
  %temp.vect173vector_func.i = insertelement <16 x float> %temp.vect172vector_func.i, float %155, i32 3
  %temp.vect174vector_func.i = insertelement <16 x float> %temp.vect173vector_func.i, float %156, i32 4
  %temp.vect175vector_func.i = insertelement <16 x float> %temp.vect174vector_func.i, float %157, i32 5
  %temp.vect176vector_func.i = insertelement <16 x float> %temp.vect175vector_func.i, float %158, i32 6
  %temp.vect177vector_func.i = insertelement <16 x float> %temp.vect176vector_func.i, float %159, i32 7
  %temp.vect178vector_func.i = insertelement <16 x float> %temp.vect177vector_func.i, float %160, i32 8
  %temp.vect179vector_func.i = insertelement <16 x float> %temp.vect178vector_func.i, float %161, i32 9
  %temp.vect180vector_func.i = insertelement <16 x float> %temp.vect179vector_func.i, float %162, i32 10
  %temp.vect181vector_func.i = insertelement <16 x float> %temp.vect180vector_func.i, float %163, i32 11
  %temp.vect182vector_func.i = insertelement <16 x float> %temp.vect181vector_func.i, float %164, i32 12
  %temp.vect183vector_func.i = insertelement <16 x float> %temp.vect182vector_func.i, float %165, i32 13
  %temp.vect184vector_func.i = insertelement <16 x float> %temp.vect183vector_func.i, float %166, i32 14
  %temp.vect185vector_func.i = insertelement <16 x float> %temp.vect184vector_func.i, float %167, i32 15
  %168 = extractelement <4 x float> %phi410vector_func.i, i32 1
  %169 = extractelement <4 x float> %phi416vector_func.i, i32 1
  %170 = extractelement <4 x float> %phi422vector_func.i, i32 1
  %171 = extractelement <4 x float> %phi428vector_func.i, i32 1
  %172 = extractelement <4 x float> %phi404vector_func.i, i32 1
  %173 = extractelement <4 x float> %phi434vector_func.i, i32 1
  %174 = extractelement <4 x float> %phi440vector_func.i, i32 1
  %175 = extractelement <4 x float> %phi446vector_func.i, i32 1
  %176 = extractelement <4 x float> %phi452vector_func.i, i32 1
  %177 = extractelement <4 x float> %phi458vector_func.i, i32 1
  %178 = extractelement <4 x float> %phi464vector_func.i, i32 1
  %179 = extractelement <4 x float> %phi470vector_func.i, i32 1
  %180 = extractelement <4 x float> %phi476vector_func.i, i32 1
  %181 = extractelement <4 x float> %phi482vector_func.i, i32 1
  %182 = extractelement <4 x float> %phi488vector_func.i, i32 1
  %183 = extractelement <4 x float> %phi494vector_func.i, i32 1
  %temp.vect203vector_func.i = insertelement <16 x float> undef, float %168, i32 0
  %temp.vect204vector_func.i = insertelement <16 x float> %temp.vect203vector_func.i, float %169, i32 1
  %temp.vect205vector_func.i = insertelement <16 x float> %temp.vect204vector_func.i, float %170, i32 2
  %temp.vect206vector_func.i = insertelement <16 x float> %temp.vect205vector_func.i, float %171, i32 3
  %temp.vect207vector_func.i = insertelement <16 x float> %temp.vect206vector_func.i, float %172, i32 4
  %temp.vect208vector_func.i = insertelement <16 x float> %temp.vect207vector_func.i, float %173, i32 5
  %temp.vect209vector_func.i = insertelement <16 x float> %temp.vect208vector_func.i, float %174, i32 6
  %temp.vect210vector_func.i = insertelement <16 x float> %temp.vect209vector_func.i, float %175, i32 7
  %temp.vect211vector_func.i = insertelement <16 x float> %temp.vect210vector_func.i, float %176, i32 8
  %temp.vect212vector_func.i = insertelement <16 x float> %temp.vect211vector_func.i, float %177, i32 9
  %temp.vect213vector_func.i = insertelement <16 x float> %temp.vect212vector_func.i, float %178, i32 10
  %temp.vect214vector_func.i = insertelement <16 x float> %temp.vect213vector_func.i, float %179, i32 11
  %temp.vect215vector_func.i = insertelement <16 x float> %temp.vect214vector_func.i, float %180, i32 12
  %temp.vect216vector_func.i = insertelement <16 x float> %temp.vect215vector_func.i, float %181, i32 13
  %temp.vect217vector_func.i = insertelement <16 x float> %temp.vect216vector_func.i, float %182, i32 14
  %temp.vect218vector_func.i = insertelement <16 x float> %temp.vect217vector_func.i, float %183, i32 15
  %184 = extractelement <4 x float> %phi410vector_func.i, i32 2
  %185 = extractelement <4 x float> %phi416vector_func.i, i32 2
  %186 = extractelement <4 x float> %phi422vector_func.i, i32 2
  %187 = extractelement <4 x float> %phi428vector_func.i, i32 2
  %188 = extractelement <4 x float> %phi404vector_func.i, i32 2
  %189 = extractelement <4 x float> %phi434vector_func.i, i32 2
  %190 = extractelement <4 x float> %phi440vector_func.i, i32 2
  %191 = extractelement <4 x float> %phi446vector_func.i, i32 2
  %192 = extractelement <4 x float> %phi452vector_func.i, i32 2
  %193 = extractelement <4 x float> %phi458vector_func.i, i32 2
  %194 = extractelement <4 x float> %phi464vector_func.i, i32 2
  %195 = extractelement <4 x float> %phi470vector_func.i, i32 2
  %196 = extractelement <4 x float> %phi476vector_func.i, i32 2
  %197 = extractelement <4 x float> %phi482vector_func.i, i32 2
  %198 = extractelement <4 x float> %phi488vector_func.i, i32 2
  %199 = extractelement <4 x float> %phi494vector_func.i, i32 2
  %temp.vect236vector_func.i = insertelement <16 x float> undef, float %184, i32 0
  %temp.vect237vector_func.i = insertelement <16 x float> %temp.vect236vector_func.i, float %185, i32 1
  %temp.vect238vector_func.i = insertelement <16 x float> %temp.vect237vector_func.i, float %186, i32 2
  %temp.vect239vector_func.i = insertelement <16 x float> %temp.vect238vector_func.i, float %187, i32 3
  %temp.vect240vector_func.i = insertelement <16 x float> %temp.vect239vector_func.i, float %188, i32 4
  %temp.vect241vector_func.i = insertelement <16 x float> %temp.vect240vector_func.i, float %189, i32 5
  %temp.vect242vector_func.i = insertelement <16 x float> %temp.vect241vector_func.i, float %190, i32 6
  %temp.vect243vector_func.i = insertelement <16 x float> %temp.vect242vector_func.i, float %191, i32 7
  %temp.vect244vector_func.i = insertelement <16 x float> %temp.vect243vector_func.i, float %192, i32 8
  %temp.vect245vector_func.i = insertelement <16 x float> %temp.vect244vector_func.i, float %193, i32 9
  %temp.vect246vector_func.i = insertelement <16 x float> %temp.vect245vector_func.i, float %194, i32 10
  %temp.vect247vector_func.i = insertelement <16 x float> %temp.vect246vector_func.i, float %195, i32 11
  %temp.vect248vector_func.i = insertelement <16 x float> %temp.vect247vector_func.i, float %196, i32 12
  %temp.vect249vector_func.i = insertelement <16 x float> %temp.vect248vector_func.i, float %197, i32 13
  %temp.vect250vector_func.i = insertelement <16 x float> %temp.vect249vector_func.i, float %198, i32 14
  %temp.vect251vector_func.i = insertelement <16 x float> %temp.vect250vector_func.i, float %199, i32 15
  %sub186vector_func.i = fsub <16 x float> %temp.vect169vector_func.i, %temp.vect185vector_func.i
  %sub6219vector_func.i = fsub <16 x float> %temp.vect202vector_func.i, %temp.vect218vector_func.i
  %sub7252vector_func.i = fsub <16 x float> %temp.vect235vector_func.i, %temp.vect251vector_func.i
  %mul8253vector_func.i = fmul <16 x float> %sub186vector_func.i, %sub186vector_func.i
  %mul9254vector_func.i = fmul <16 x float> %sub6219vector_func.i, %sub6219vector_func.i
  %add10255vector_func.i = fadd <16 x float> %mul8253vector_func.i, %mul9254vector_func.i
  %mul11256vector_func.i = fmul <16 x float> %sub7252vector_func.i, %sub7252vector_func.i
  %add12257vector_func.i = fadd <16 x float> %add10255vector_func.i, %mul11256vector_func.i
  %cmp13vector_func.i = fcmp olt <16 x float> %add12257vector_func.i, %vector259vector_func.i
  %while.body_to_if.then262vector_func.i = and <16 x i1> %vectorPHI78vector_func.i, %cmp13vector_func.i
  %div263vector_func.i = fdiv <16 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %add12257vector_func.i
  %mul15264vector_func.i = fmul <16 x float> %div263vector_func.i, %div263vector_func.i
  %mul16265vector_func.i = fmul <16 x float> %mul15264vector_func.i, %div263vector_func.i
  %mul17266vector_func.i = fmul <16 x float> %div263vector_func.i, %mul16265vector_func.i
  %mul18269vector_func.i = fmul <16 x float> %mul16265vector_func.i, %vector268vector_func.i
  %sub19272vector_func.i = fsub <16 x float> %mul18269vector_func.i, %vector271vector_func.i
  %mul20273vector_func.i = fmul <16 x float> %mul17266vector_func.i, %sub19272vector_func.i
  %mul25278vector_func.i = fmul <16 x float> %sub7252vector_func.i, %mul20273vector_func.i
  %mul23276vector_func.i = fmul <16 x float> %sub6219vector_func.i, %mul20273vector_func.i
  %mul21274vector_func.i = fmul <16 x float> %sub186vector_func.i, %mul20273vector_func.i
  %add26279vector_func.i = fadd <16 x float> %vectorPHI83vector_func.i, %mul25278vector_func.i
  %add24277vector_func.i = fadd <16 x float> %vectorPHI82vector_func.i, %mul23276vector_func.i
  %add22275vector_func.i = fadd <16 x float> %vectorPHI81vector_func.i, %mul21274vector_func.i
  %out_sel51283vector_func.i = select <16 x i1> %vectorPHI78vector_func.i, <16 x float> zeroinitializer, <16 x float> %vectorPHI77vector_func.i
  %merge34284vector_func.i = select <16 x i1> %while.body_to_if.then262vector_func.i, <16 x float> %add26279vector_func.i, <16 x float> %vectorPHI83vector_func.i
  %out_sel48285vector_func.i = select <16 x i1> %vectorPHI78vector_func.i, <16 x float> %merge34284vector_func.i, <16 x float> %vectorPHI76vector_func.i
  %merge32286vector_func.i = select <16 x i1> %while.body_to_if.then262vector_func.i, <16 x float> %add24277vector_func.i, <16 x float> %vectorPHI82vector_func.i
  %out_sel45287vector_func.i = select <16 x i1> %vectorPHI78vector_func.i, <16 x float> %merge32286vector_func.i, <16 x float> %vectorPHI75vector_func.i
  %merge288vector_func.i = select <16 x i1> %while.body_to_if.then262vector_func.i, <16 x float> %add22275vector_func.i, <16 x float> %vectorPHI81vector_func.i
  %out_sel289vector_func.i = select <16 x i1> %vectorPHI78vector_func.i, <16 x float> %merge288vector_func.i, <16 x float> %vectorPHI74vector_func.i
  %indvars.iv.nextvector_func.i = add i64 %indvars.ivvector_func.i, 1
  %lftr.wideivvector_func.i = trunc i64 %indvars.iv.nextvector_func.i to i32
  %exitcondvector_func.i = icmp eq i32 %lftr.wideivvector_func.i, %7
  %temp290vector_func.i = insertelement <16 x i1> undef, i1 %exitcondvector_func.i, i32 0
  %vector291vector_func.i = shufflevector <16 x i1> %temp290vector_func.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %notCondvector_func.i = xor i1 %exitcondvector_func.i, true
  %temp296vector_func.i = insertelement <16 x i1> undef, i1 %notCondvector_func.i, i32 0
  %vector297vector_func.i = shufflevector <16 x i1> %temp296vector_func.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %who_left_tr292vector_func.i = and <16 x i1> %vectorPHI78vector_func.i, %vector291vector_func.i
  %loop_mask25294vector_func.i = or <16 x i1> %vectorPHI73vector_func.i, %who_left_tr292vector_func.i
  %ipred.i.i = bitcast <16 x i1> %loop_mask25294vector_func.i to i16
  %val.i.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i.i, i16 %ipred.i.i) nounwind
  %200 = and i32 %val.i.i, 1
  %res.i.i = icmp eq i32 %200, 0
  %local_edge298vector_func.i = and <16 x i1> %vectorPHI78vector_func.i, %vector297vector_func.i
  br i1 %res.i.i, label %while.bodyvector_func.i, label %while.endvector_func.i

while.endvector_func.i:                           ; preds = %postload493vector_func.i, %entryvector_func.i
  %vectorPHI299vector_func.i = phi <16 x float> [ undef, %entryvector_func.i ], [ %out_sel289vector_func.i, %postload493vector_func.i ]
  %vectorPHI300vector_func.i = phi <16 x float> [ undef, %entryvector_func.i ], [ %out_sel45287vector_func.i, %postload493vector_func.i ]
  %vectorPHI301vector_func.i = phi <16 x float> [ undef, %entryvector_func.i ], [ %out_sel48285vector_func.i, %postload493vector_func.i ]
  %vectorPHI302vector_func.i = phi <16 x float> [ undef, %entryvector_func.i ], [ %out_sel51283vector_func.i, %postload493vector_func.i ]
  %merge44303vector_func.i = select i1 %cmp1vector_func.i, <16 x float> %vectorPHI302vector_func.i, <16 x float> zeroinitializer
  %extract355vector_func.i = extractelement <16 x float> %merge44303vector_func.i, i32 0
  %extract356vector_func.i = extractelement <16 x float> %merge44303vector_func.i, i32 1
  %extract357vector_func.i = extractelement <16 x float> %merge44303vector_func.i, i32 2
  %extract358vector_func.i = extractelement <16 x float> %merge44303vector_func.i, i32 3
  %extract359vector_func.i = extractelement <16 x float> %merge44303vector_func.i, i32 4
  %extract360vector_func.i = extractelement <16 x float> %merge44303vector_func.i, i32 5
  %extract361vector_func.i = extractelement <16 x float> %merge44303vector_func.i, i32 6
  %extract362vector_func.i = extractelement <16 x float> %merge44303vector_func.i, i32 7
  %extract363vector_func.i = extractelement <16 x float> %merge44303vector_func.i, i32 8
  %extract364vector_func.i = extractelement <16 x float> %merge44303vector_func.i, i32 9
  %extract365vector_func.i = extractelement <16 x float> %merge44303vector_func.i, i32 10
  %extract366vector_func.i = extractelement <16 x float> %merge44303vector_func.i, i32 11
  %extract367vector_func.i = extractelement <16 x float> %merge44303vector_func.i, i32 12
  %extract368vector_func.i = extractelement <16 x float> %merge44303vector_func.i, i32 13
  %extract369vector_func.i = extractelement <16 x float> %merge44303vector_func.i, i32 14
  %extract370vector_func.i = extractelement <16 x float> %merge44303vector_func.i, i32 15
  %merge42304vector_func.i = select i1 %cmp1vector_func.i, <16 x float> %vectorPHI301vector_func.i, <16 x float> zeroinitializer
  %extract339vector_func.i = extractelement <16 x float> %merge42304vector_func.i, i32 0
  %extract340vector_func.i = extractelement <16 x float> %merge42304vector_func.i, i32 1
  %extract341vector_func.i = extractelement <16 x float> %merge42304vector_func.i, i32 2
  %extract342vector_func.i = extractelement <16 x float> %merge42304vector_func.i, i32 3
  %extract343vector_func.i = extractelement <16 x float> %merge42304vector_func.i, i32 4
  %extract344vector_func.i = extractelement <16 x float> %merge42304vector_func.i, i32 5
  %extract345vector_func.i = extractelement <16 x float> %merge42304vector_func.i, i32 6
  %extract346vector_func.i = extractelement <16 x float> %merge42304vector_func.i, i32 7
  %extract347vector_func.i = extractelement <16 x float> %merge42304vector_func.i, i32 8
  %extract348vector_func.i = extractelement <16 x float> %merge42304vector_func.i, i32 9
  %extract349vector_func.i = extractelement <16 x float> %merge42304vector_func.i, i32 10
  %extract350vector_func.i = extractelement <16 x float> %merge42304vector_func.i, i32 11
  %extract351vector_func.i = extractelement <16 x float> %merge42304vector_func.i, i32 12
  %extract352vector_func.i = extractelement <16 x float> %merge42304vector_func.i, i32 13
  %extract353vector_func.i = extractelement <16 x float> %merge42304vector_func.i, i32 14
  %extract354vector_func.i = extractelement <16 x float> %merge42304vector_func.i, i32 15
  %merge40305vector_func.i = select i1 %cmp1vector_func.i, <16 x float> %vectorPHI300vector_func.i, <16 x float> zeroinitializer
  %extract323vector_func.i = extractelement <16 x float> %merge40305vector_func.i, i32 0
  %extract324vector_func.i = extractelement <16 x float> %merge40305vector_func.i, i32 1
  %extract325vector_func.i = extractelement <16 x float> %merge40305vector_func.i, i32 2
  %extract326vector_func.i = extractelement <16 x float> %merge40305vector_func.i, i32 3
  %extract327vector_func.i = extractelement <16 x float> %merge40305vector_func.i, i32 4
  %extract328vector_func.i = extractelement <16 x float> %merge40305vector_func.i, i32 5
  %extract329vector_func.i = extractelement <16 x float> %merge40305vector_func.i, i32 6
  %extract330vector_func.i = extractelement <16 x float> %merge40305vector_func.i, i32 7
  %extract331vector_func.i = extractelement <16 x float> %merge40305vector_func.i, i32 8
  %extract332vector_func.i = extractelement <16 x float> %merge40305vector_func.i, i32 9
  %extract333vector_func.i = extractelement <16 x float> %merge40305vector_func.i, i32 10
  %extract334vector_func.i = extractelement <16 x float> %merge40305vector_func.i, i32 11
  %extract335vector_func.i = extractelement <16 x float> %merge40305vector_func.i, i32 12
  %extract336vector_func.i = extractelement <16 x float> %merge40305vector_func.i, i32 13
  %extract337vector_func.i = extractelement <16 x float> %merge40305vector_func.i, i32 14
  %extract338vector_func.i = extractelement <16 x float> %merge40305vector_func.i, i32 15
  %merge38306vector_func.i = select i1 %cmp1vector_func.i, <16 x float> %vectorPHI299vector_func.i, <16 x float> zeroinitializer
  %extract307vector_func.i = extractelement <16 x float> %merge38306vector_func.i, i32 0
  %extract308vector_func.i = extractelement <16 x float> %merge38306vector_func.i, i32 1
  %extract309vector_func.i = extractelement <16 x float> %merge38306vector_func.i, i32 2
  %extract310vector_func.i = extractelement <16 x float> %merge38306vector_func.i, i32 3
  %extract311vector_func.i = extractelement <16 x float> %merge38306vector_func.i, i32 4
  %extract312vector_func.i = extractelement <16 x float> %merge38306vector_func.i, i32 5
  %extract313vector_func.i = extractelement <16 x float> %merge38306vector_func.i, i32 6
  %extract314vector_func.i = extractelement <16 x float> %merge38306vector_func.i, i32 7
  %extract315vector_func.i = extractelement <16 x float> %merge38306vector_func.i, i32 8
  %extract316vector_func.i = extractelement <16 x float> %merge38306vector_func.i, i32 9
  %extract317vector_func.i = extractelement <16 x float> %merge38306vector_func.i, i32 10
  %extract318vector_func.i = extractelement <16 x float> %merge38306vector_func.i, i32 11
  %extract319vector_func.i = extractelement <16 x float> %merge38306vector_func.i, i32 12
  %extract320vector_func.i = extractelement <16 x float> %merge38306vector_func.i, i32 13
  %extract321vector_func.i = extractelement <16 x float> %merge38306vector_func.i, i32 14
  %extract322vector_func.i = extractelement <16 x float> %merge38306vector_func.i, i32 15
  %201 = insertelement <4 x float> undef, float %extract307vector_func.i, i32 0
  %202 = insertelement <4 x float> undef, float %extract308vector_func.i, i32 0
  %203 = insertelement <4 x float> undef, float %extract309vector_func.i, i32 0
  %204 = insertelement <4 x float> undef, float %extract310vector_func.i, i32 0
  %205 = insertelement <4 x float> undef, float %extract311vector_func.i, i32 0
  %206 = insertelement <4 x float> undef, float %extract312vector_func.i, i32 0
  %207 = insertelement <4 x float> undef, float %extract313vector_func.i, i32 0
  %208 = insertelement <4 x float> undef, float %extract314vector_func.i, i32 0
  %209 = insertelement <4 x float> undef, float %extract315vector_func.i, i32 0
  %210 = insertelement <4 x float> undef, float %extract316vector_func.i, i32 0
  %211 = insertelement <4 x float> undef, float %extract317vector_func.i, i32 0
  %212 = insertelement <4 x float> undef, float %extract318vector_func.i, i32 0
  %213 = insertelement <4 x float> undef, float %extract319vector_func.i, i32 0
  %214 = insertelement <4 x float> undef, float %extract320vector_func.i, i32 0
  %215 = insertelement <4 x float> undef, float %extract321vector_func.i, i32 0
  %216 = insertelement <4 x float> undef, float %extract322vector_func.i, i32 0
  %217 = insertelement <4 x float> %201, float %extract323vector_func.i, i32 1
  %218 = insertelement <4 x float> %202, float %extract324vector_func.i, i32 1
  %219 = insertelement <4 x float> %203, float %extract325vector_func.i, i32 1
  %220 = insertelement <4 x float> %204, float %extract326vector_func.i, i32 1
  %221 = insertelement <4 x float> %205, float %extract327vector_func.i, i32 1
  %222 = insertelement <4 x float> %206, float %extract328vector_func.i, i32 1
  %223 = insertelement <4 x float> %207, float %extract329vector_func.i, i32 1
  %224 = insertelement <4 x float> %208, float %extract330vector_func.i, i32 1
  %225 = insertelement <4 x float> %209, float %extract331vector_func.i, i32 1
  %226 = insertelement <4 x float> %210, float %extract332vector_func.i, i32 1
  %227 = insertelement <4 x float> %211, float %extract333vector_func.i, i32 1
  %228 = insertelement <4 x float> %212, float %extract334vector_func.i, i32 1
  %229 = insertelement <4 x float> %213, float %extract335vector_func.i, i32 1
  %230 = insertelement <4 x float> %214, float %extract336vector_func.i, i32 1
  %231 = insertelement <4 x float> %215, float %extract337vector_func.i, i32 1
  %232 = insertelement <4 x float> %216, float %extract338vector_func.i, i32 1
  %233 = insertelement <4 x float> %217, float %extract339vector_func.i, i32 2
  %234 = insertelement <4 x float> %218, float %extract340vector_func.i, i32 2
  %235 = insertelement <4 x float> %219, float %extract341vector_func.i, i32 2
  %236 = insertelement <4 x float> %220, float %extract342vector_func.i, i32 2
  %237 = insertelement <4 x float> %221, float %extract343vector_func.i, i32 2
  %238 = insertelement <4 x float> %222, float %extract344vector_func.i, i32 2
  %239 = insertelement <4 x float> %223, float %extract345vector_func.i, i32 2
  %240 = insertelement <4 x float> %224, float %extract346vector_func.i, i32 2
  %241 = insertelement <4 x float> %225, float %extract347vector_func.i, i32 2
  %242 = insertelement <4 x float> %226, float %extract348vector_func.i, i32 2
  %243 = insertelement <4 x float> %227, float %extract349vector_func.i, i32 2
  %244 = insertelement <4 x float> %228, float %extract350vector_func.i, i32 2
  %245 = insertelement <4 x float> %229, float %extract351vector_func.i, i32 2
  %246 = insertelement <4 x float> %230, float %extract352vector_func.i, i32 2
  %247 = insertelement <4 x float> %231, float %extract353vector_func.i, i32 2
  %248 = insertelement <4 x float> %232, float %extract354vector_func.i, i32 2
  %249 = insertelement <4 x float> %233, float %extract355vector_func.i, i32 3
  %250 = insertelement <4 x float> %234, float %extract356vector_func.i, i32 3
  %251 = insertelement <4 x float> %235, float %extract357vector_func.i, i32 3
  %252 = insertelement <4 x float> %236, float %extract358vector_func.i, i32 3
  %253 = insertelement <4 x float> %237, float %extract359vector_func.i, i32 3
  %254 = insertelement <4 x float> %238, float %extract360vector_func.i, i32 3
  %255 = insertelement <4 x float> %239, float %extract361vector_func.i, i32 3
  %256 = insertelement <4 x float> %240, float %extract362vector_func.i, i32 3
  %257 = insertelement <4 x float> %241, float %extract363vector_func.i, i32 3
  %258 = insertelement <4 x float> %242, float %extract364vector_func.i, i32 3
  %259 = insertelement <4 x float> %243, float %extract365vector_func.i, i32 3
  %260 = insertelement <4 x float> %244, float %extract366vector_func.i, i32 3
  %261 = insertelement <4 x float> %245, float %extract367vector_func.i, i32 3
  %262 = insertelement <4 x float> %246, float %extract368vector_func.i, i32 3
  %263 = insertelement <4 x float> %247, float %extract369vector_func.i, i32 3
  %264 = insertelement <4 x float> %248, float %extract370vector_func.i, i32 3
  %265 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %extractvector_func.i
  %266 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %extract58vector_func.i
  %267 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %extract59vector_func.i
  %268 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %extract60vector_func.i
  %269 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %extract61vector_func.i
  %270 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %extract62vector_func.i
  %271 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %extract63vector_func.i
  %272 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %extract64vector_func.i
  %273 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %extract65vector_func.i
  %274 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %extract66vector_func.i
  %275 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %extract67vector_func.i
  %276 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %extract68vector_func.i
  %277 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %extract69vector_func.i
  %278 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %extract70vector_func.i
  %279 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %extract71vector_func.i
  %280 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %extract72vector_func.i
  store <4 x float> %249, <4 x float> addrspace(1)* %265, align 16
  store <4 x float> %250, <4 x float> addrspace(1)* %266, align 16
  store <4 x float> %251, <4 x float> addrspace(1)* %267, align 16
  store <4 x float> %252, <4 x float> addrspace(1)* %268, align 16
  store <4 x float> %253, <4 x float> addrspace(1)* %269, align 16
  store <4 x float> %254, <4 x float> addrspace(1)* %270, align 16
  store <4 x float> %255, <4 x float> addrspace(1)* %271, align 16
  store <4 x float> %256, <4 x float> addrspace(1)* %272, align 16
  store <4 x float> %257, <4 x float> addrspace(1)* %273, align 16
  store <4 x float> %258, <4 x float> addrspace(1)* %274, align 16
  store <4 x float> %259, <4 x float> addrspace(1)* %275, align 16
  store <4 x float> %260, <4 x float> addrspace(1)* %276, align 16
  store <4 x float> %261, <4 x float> addrspace(1)* %277, align 16
  store <4 x float> %262, <4 x float> addrspace(1)* %278, align 16
  store <4 x float> %263, <4 x float> addrspace(1)* %279, align 16
  store <4 x float> %264, <4 x float> addrspace(1)* %280, align 16
  %dim_0_vector_inc_ind_var.i = add i64 %dim_0_vector_ind_var.i, 1
  %dim_0_vector_cmp.to.max.i = icmp eq i64 %dim_0_vector_inc_ind_var.i, %vector.size.i
  %dim_0_vector_inc_tid.i = add i64 %dim_0_vector_tid.i, 16
  br i1 %dim_0_vector_cmp.to.max.i, label %dim_0_vector_exit.i, label %entryvector_func.i

dim_0_vector_exit.i:                              ; preds = %while.endvector_func.i
  %dim_1_vector_inc_ind_var.i = add i64 %dim_1_vector_ind_var.i, 1
  %dim_1_vector_cmp.to.max.i = icmp eq i64 %dim_1_vector_inc_ind_var.i, %34
  br i1 %dim_1_vector_cmp.to.max.i, label %dim_1_vector_exit.i, label %dim_0_vector_pre_head.i

dim_1_vector_exit.i:                              ; preds = %dim_0_vector_exit.i
  %dim_2_vector_inc_ind_var.i = add i64 %dim_2_vector_ind_var.i, 1
  %dim_2_vector_cmp.to.max.i = icmp eq i64 %dim_2_vector_inc_ind_var.i, %36
  br i1 %dim_2_vector_cmp.to.max.i, label %scalarIf.i, label %dim_1_vector_pre_head.i

scalarIf.i:                                       ; preds = %dim_1_vector_exit.i, %entry
  %281 = icmp eq i64 %30, %num.vector.wi.i
  br i1 %281, label %__compute_lj_force_separated_args.exit, label %dim_2_pre_head.i

dim_2_pre_head.i:                                 ; preds = %scalarIf.i
  %cmp1.i = icmp sgt i32 %7, 0
  br label %dim_1_pre_head.i

dim_1_pre_head.i:                                 ; preds = %dim_1_exit.i, %dim_2_pre_head.i
  %dim_2_ind_var.i = phi i64 [ 0, %dim_2_pre_head.i ], [ %dim_2_inc_ind_var.i, %dim_1_exit.i ]
  br label %dim_0_pre_head.i

dim_0_pre_head.i:                                 ; preds = %dim_0_exit.i, %dim_1_pre_head.i
  %dim_1_ind_var.i = phi i64 [ 0, %dim_1_pre_head.i ], [ %dim_1_inc_ind_var.i, %dim_0_exit.i ]
  br label %scalar_kernel_entry.i

scalar_kernel_entry.i:                            ; preds = %while.end.i, %dim_0_pre_head.i
  %dim_0_ind_var.i = phi i64 [ 0, %dim_0_pre_head.i ], [ %dim_0_inc_ind_var.i, %while.end.i ]
  %dim_0_tid.i = phi i64 [ %max.vector.gid.i, %dim_0_pre_head.i ], [ %dim_0_inc_tid.i, %while.end.i ]
  %conv.i = trunc i64 %dim_0_tid.i to i32
  %idxprom.i = and i64 %dim_0_tid.i, 4294967295
  %arrayidx.i = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %idxprom.i
  %282 = load <4 x float> addrspace(1)* %arrayidx.i, align 16
  br i1 %cmp1.i, label %while.body.lr.ph.i, label %while.end.i

while.body.lr.ph.i:                               ; preds = %scalar_kernel_entry.i
  %283 = extractelement <4 x float> %282, i32 0
  %284 = extractelement <4 x float> %282, i32 1
  %285 = extractelement <4 x float> %282, i32 2
  br label %while.body.i

while.body.i:                                     ; preds = %if.end.i, %while.body.lr.ph.i
  %indvars.iv.i = phi i64 [ 0, %while.body.lr.ph.i ], [ %indvars.iv.next.i, %if.end.i ]
  %f.02.i = phi <4 x float> [ zeroinitializer, %while.body.lr.ph.i ], [ %f.1.i, %if.end.i ]
  %286 = trunc i64 %indvars.iv.i to i32
  %mul.i = mul nsw i32 %286, %22
  %add.i = add i32 %mul.i, %conv.i
  %idxprom2.i = zext i32 %add.i to i64
  %arrayidx3.i = getelementptr inbounds i32 addrspace(1)* %10, i64 %idxprom2.i
  %287 = load i32 addrspace(1)* %arrayidx3.i, align 4
  %idxprom4.i = sext i32 %287 to i64
  %arrayidx5.i = getelementptr inbounds <4 x float> addrspace(1)* %4, i64 %idxprom4.i
  %288 = load <4 x float> addrspace(1)* %arrayidx5.i, align 16
  %289 = extractelement <4 x float> %288, i32 0
  %sub.i = fsub float %283, %289
  %290 = extractelement <4 x float> %288, i32 1
  %sub6.i = fsub float %284, %290
  %291 = extractelement <4 x float> %288, i32 2
  %sub7.i = fsub float %285, %291
  %mul8.i = fmul float %sub.i, %sub.i
  %mul9.i = fmul float %sub6.i, %sub6.i
  %add10.i = fadd float %mul8.i, %mul9.i
  %mul11.i = fmul float %sub7.i, %sub7.i
  %add12.i = fadd float %add10.i, %mul11.i
  %cmp13.i = fcmp olt float %add12.i, %13
  br i1 %cmp13.i, label %if.then.i, label %if.end.i

if.then.i:                                        ; preds = %while.body.i
  %div.i = fdiv float 1.000000e+00, %add12.i
  %mul15.i = fmul float %div.i, %div.i
  %mul16.i = fmul float %mul15.i, %div.i
  %mul17.i = fmul float %div.i, %mul16.i
  %mul18.i = fmul float %mul16.i, %16
  %sub19.i = fsub float %mul18.i, %19
  %mul20.i = fmul float %mul17.i, %sub19.i
  %mul21.i = fmul float %sub.i, %mul20.i
  %292 = extractelement <4 x float> %f.02.i, i32 0
  %add22.i = fadd float %292, %mul21.i
  %293 = insertelement <4 x float> %f.02.i, float %add22.i, i32 0
  %mul23.i = fmul float %sub6.i, %mul20.i
  %294 = extractelement <4 x float> %f.02.i, i32 1
  %add24.i = fadd float %294, %mul23.i
  %295 = insertelement <4 x float> %293, float %add24.i, i32 1
  %mul25.i = fmul float %sub7.i, %mul20.i
  %296 = extractelement <4 x float> %f.02.i, i32 2
  %add26.i = fadd float %296, %mul25.i
  %297 = insertelement <4 x float> %295, float %add26.i, i32 2
  br label %if.end.i

if.end.i:                                         ; preds = %if.then.i, %while.body.i
  %f.1.i = phi <4 x float> [ %297, %if.then.i ], [ %f.02.i, %while.body.i ]
  %indvars.iv.next.i = add i64 %indvars.iv.i, 1
  %lftr.wideiv.i = trunc i64 %indvars.iv.next.i to i32
  %exitcond.i = icmp eq i32 %lftr.wideiv.i, %7
  br i1 %exitcond.i, label %while.end.i, label %while.body.i

while.end.i:                                      ; preds = %if.end.i, %scalar_kernel_entry.i
  %f.0.lcssa.i = phi <4 x float> [ zeroinitializer, %scalar_kernel_entry.i ], [ %f.1.i, %if.end.i ]
  %arrayidx28.i = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %idxprom.i
  store <4 x float> %f.0.lcssa.i, <4 x float> addrspace(1)* %arrayidx28.i, align 16
  %dim_0_inc_ind_var.i = add i64 %dim_0_ind_var.i, 1
  %dim_0_cmp.to.max.i = icmp eq i64 %dim_0_inc_ind_var.i, %scalar.size.i
  %dim_0_inc_tid.i = add i64 %dim_0_tid.i, 1
  br i1 %dim_0_cmp.to.max.i, label %dim_0_exit.i, label %scalar_kernel_entry.i

dim_0_exit.i:                                     ; preds = %while.end.i
  %dim_1_inc_ind_var.i = add i64 %dim_1_ind_var.i, 1
  %dim_1_cmp.to.max.i = icmp eq i64 %dim_1_inc_ind_var.i, %34
  br i1 %dim_1_cmp.to.max.i, label %dim_1_exit.i, label %dim_0_pre_head.i

dim_1_exit.i:                                     ; preds = %dim_0_exit.i
  %dim_2_inc_ind_var.i = add i64 %dim_2_ind_var.i, 1
  %dim_2_cmp.to.max.i = icmp eq i64 %dim_2_inc_ind_var.i, %36
  br i1 %dim_2_cmp.to.max.i, label %__compute_lj_force_separated_args.exit, label %dim_1_pre_head.i

__compute_lj_force_separated_args.exit:           ; preds = %scalarIf.i, %dim_1_exit.i
  ret void
}

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}
!cl.noBarrierPath.kernels = !{!3}
!opencl.wrappers = !{!4}

!0 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32 addrspace(1)*, float, float, float, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__compute_lj_force_separated_args, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3}
!2 = metadata !{}
!3 = metadata !{metadata !"compute_lj_force"}
!4 = metadata !{void (i8*)* @compute_lj_force}
