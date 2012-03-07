; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @__reduce_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(3)* nocapture, i32) nounwind

declare i64 @get_local_id(i32) nounwind readnone

declare i64 @get_group_id(i32) nounwind readnone

declare i64 @get_local_size(i32) nounwind readnone

declare i64 @get_num_groups(i32) nounwind readnone

declare void @barrier(i64)

declare void @__reduceNoLocal_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32) nounwind

declare [7 x i64] @__WG.boundaries.reduceNoLocal_original(float addrspace(1)*, float addrspace(1)*, i32)

declare i64 @get_base_global_id.(i32)

declare void @____Vectorized_.reduce_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(3)* nocapture, i32) nounwind

declare i1 @allOne(i1)

declare i1 @allZero(i1)

declare float @masked_load_align4_0(i1, float addrspace(1)*)

declare float @masked_load_align4_1(i1, float addrspace(1)*)

declare void @masked_store_align4_0(i1, float, float addrspace(3)*)

declare float @masked_load_align4_2(i1, float addrspace(3)*)

declare float @masked_load_align4_3(i1, float addrspace(3)*)

declare void @masked_store_align4_1(i1, float, float addrspace(3)*)

declare void @maskedf_0_barrier(i1, i64)

declare float @masked_load_align4_4(i1, float addrspace(3)*)

declare void @masked_store_align4_2(i1, float, float addrspace(1)*)

declare i1 @allZero_v16(<16 x i1>)

declare void @masked_store_align4_3(<16 x i1>, <16 x float>, <16 x float> addrspace(3)*)

declare i1 @allOne_v16(<16 x i1>)

declare <16 x float> @masked_load_align4_8(<16 x i1>, <16 x float> addrspace(3)*)

declare void @masked_store_align4_4(<16 x i1>, <16 x float>, <16 x float> addrspace(3)*)

declare void @dummybarrier.()

declare i8* @get_special_buffer.() nounwind readnone

declare i64 @get_iter_count.() nounwind readnone

declare i64 @get_new_local_id.(i32, i64) nounwind readnone

declare void @__reduce_separated_args(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(3)* nocapture, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare void @__reduceNoLocal_separated_args(float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare [7 x i64] @WG.boundaries.reduceNoLocal(float addrspace(1)*, float addrspace(1)*, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)

declare void @____Vectorized_.reduce_separated_args(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(3)* nocapture, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare i32 @llvm.x86.mic.kortestc(i16, i16) nounwind readnone

declare i32 @llvm.x86.mic.kortestz(i16, i16) nounwind readnone

define void @reduceNoLocal(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 32
  %9 = bitcast i8* %8 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %10 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %9, align 8
  %11 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %10, i64 0, i32 3, i64 0
  %12 = load i64* %11, align 8
  %13 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %10, i64 0, i32 3, i64 1
  %14 = load i64* %13, align 8
  %15 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %10, i64 0, i32 3, i64 2
  %16 = load i64* %15, align 8
  %vector.size.i = ashr i64 %12, 4
  %scalar.size.i = and i64 %12, 15
  %17 = icmp eq i64 %vector.size.i, 0
  br i1 %17, label %scalarIf.i, label %dim_2_vector_pre_head.i

dim_2_vector_pre_head.i:                          ; preds = %entry
  %cmp1vector_func.i = icmp eq i32 %7, 0
  br label %dim_1_vector_pre_head.i

dim_1_vector_pre_head.i:                          ; preds = %dim_1_vector_exit.i, %dim_2_vector_pre_head.i
  %dim_2_vector_ind_var.i = phi i64 [ 0, %dim_2_vector_pre_head.i ], [ %dim_2_vector_inc_ind_var.i, %dim_1_vector_exit.i ]
  br label %dim_0_vector_pre_head.i

dim_0_vector_pre_head.i:                          ; preds = %dim_0_vector_exit.i, %dim_1_vector_pre_head.i
  %dim_1_vector_ind_var.i = phi i64 [ 0, %dim_1_vector_pre_head.i ], [ %dim_1_vector_inc_ind_var.i, %dim_0_vector_exit.i ]
  br label %entryvector_func.i

entryvector_func.i:                               ; preds = %for.endvector_func.i, %dim_0_vector_pre_head.i
  %dim_0_vector_ind_var.i = phi i64 [ 0, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_ind_var.i, %for.endvector_func.i ]
  br i1 %cmp1vector_func.i, label %for.endvector_func.i, label %for.bodyvector_func.i

for.bodyvector_func.i:                            ; preds = %for.bodyvector_func.i, %entryvector_func.i
  %indvars.ivvector_func.i = phi i64 [ %indvars.iv.nextvector_func.i, %for.bodyvector_func.i ], [ 0, %entryvector_func.i ]
  %sum.02vector_func.i = phi float [ %addvector_func.i, %for.bodyvector_func.i ], [ 0.000000e+00, %entryvector_func.i ]
  %arrayidxvector_func.i = getelementptr inbounds float addrspace(1)* %1, i64 %indvars.ivvector_func.i
  %18 = load float addrspace(1)* %arrayidxvector_func.i, align 4
  %addvector_func.i = fadd float %sum.02vector_func.i, %18
  %indvars.iv.nextvector_func.i = add i64 %indvars.ivvector_func.i, 1
  %lftr.wideivvector_func.i = trunc i64 %indvars.iv.nextvector_func.i to i32
  %exitcondvector_func.i = icmp eq i32 %lftr.wideivvector_func.i, %7
  br i1 %exitcondvector_func.i, label %for.endvector_func.i, label %for.bodyvector_func.i

for.endvector_func.i:                             ; preds = %for.bodyvector_func.i, %entryvector_func.i
  %sum.0.lcssavector_func.i = phi float [ 0.000000e+00, %entryvector_func.i ], [ %addvector_func.i, %for.bodyvector_func.i ]
  store float %sum.0.lcssavector_func.i, float addrspace(1)* %4, align 4
  %dim_0_vector_inc_ind_var.i = add i64 %dim_0_vector_ind_var.i, 1
  %dim_0_vector_cmp.to.max.i = icmp eq i64 %dim_0_vector_inc_ind_var.i, %vector.size.i
  br i1 %dim_0_vector_cmp.to.max.i, label %dim_0_vector_exit.i, label %entryvector_func.i

dim_0_vector_exit.i:                              ; preds = %for.endvector_func.i
  %dim_1_vector_inc_ind_var.i = add i64 %dim_1_vector_ind_var.i, 1
  %dim_1_vector_cmp.to.max.i = icmp eq i64 %dim_1_vector_inc_ind_var.i, %14
  br i1 %dim_1_vector_cmp.to.max.i, label %dim_1_vector_exit.i, label %dim_0_vector_pre_head.i

dim_1_vector_exit.i:                              ; preds = %dim_0_vector_exit.i
  %dim_2_vector_inc_ind_var.i = add i64 %dim_2_vector_ind_var.i, 1
  %dim_2_vector_cmp.to.max.i = icmp eq i64 %dim_2_vector_inc_ind_var.i, %16
  br i1 %dim_2_vector_cmp.to.max.i, label %scalarIf.i, label %dim_1_vector_pre_head.i

scalarIf.i:                                       ; preds = %dim_1_vector_exit.i, %entry
  %19 = icmp eq i64 %scalar.size.i, 0
  br i1 %19, label %__reduceNoLocal_separated_args.exit, label %dim_2_pre_head.i

dim_2_pre_head.i:                                 ; preds = %scalarIf.i
  %cmp1.i = icmp eq i32 %7, 0
  br label %dim_1_pre_head.i

dim_1_pre_head.i:                                 ; preds = %dim_1_exit.i, %dim_2_pre_head.i
  %dim_2_ind_var.i = phi i64 [ 0, %dim_2_pre_head.i ], [ %dim_2_inc_ind_var.i, %dim_1_exit.i ]
  br label %dim_0_pre_head.i

dim_0_pre_head.i:                                 ; preds = %dim_0_exit.i, %dim_1_pre_head.i
  %dim_1_ind_var.i = phi i64 [ 0, %dim_1_pre_head.i ], [ %dim_1_inc_ind_var.i, %dim_0_exit.i ]
  br label %scalar_kernel_entry.i

scalar_kernel_entry.i:                            ; preds = %for.end.i, %dim_0_pre_head.i
  %dim_0_ind_var.i = phi i64 [ 0, %dim_0_pre_head.i ], [ %dim_0_inc_ind_var.i, %for.end.i ]
  br i1 %cmp1.i, label %for.end.i, label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %scalar_kernel_entry.i
  %indvars.iv.i = phi i64 [ %indvars.iv.next.i, %for.body.i ], [ 0, %scalar_kernel_entry.i ]
  %sum.02.i = phi float [ %add.i, %for.body.i ], [ 0.000000e+00, %scalar_kernel_entry.i ]
  %arrayidx.i = getelementptr inbounds float addrspace(1)* %1, i64 %indvars.iv.i
  %20 = load float addrspace(1)* %arrayidx.i, align 4
  %add.i = fadd float %sum.02.i, %20
  %indvars.iv.next.i = add i64 %indvars.iv.i, 1
  %lftr.wideiv.i = trunc i64 %indvars.iv.next.i to i32
  %exitcond.i = icmp eq i32 %lftr.wideiv.i, %7
  br i1 %exitcond.i, label %for.end.i, label %for.body.i

for.end.i:                                        ; preds = %for.body.i, %scalar_kernel_entry.i
  %sum.0.lcssa.i = phi float [ 0.000000e+00, %scalar_kernel_entry.i ], [ %add.i, %for.body.i ]
  store float %sum.0.lcssa.i, float addrspace(1)* %4, align 4
  %dim_0_inc_ind_var.i = add i64 %dim_0_ind_var.i, 1
  %dim_0_cmp.to.max.i = icmp eq i64 %dim_0_inc_ind_var.i, %scalar.size.i
  br i1 %dim_0_cmp.to.max.i, label %dim_0_exit.i, label %scalar_kernel_entry.i

dim_0_exit.i:                                     ; preds = %for.end.i
  %dim_1_inc_ind_var.i = add i64 %dim_1_ind_var.i, 1
  %dim_1_cmp.to.max.i = icmp eq i64 %dim_1_inc_ind_var.i, %14
  br i1 %dim_1_cmp.to.max.i, label %dim_1_exit.i, label %dim_0_pre_head.i

dim_1_exit.i:                                     ; preds = %dim_0_exit.i
  %dim_2_inc_ind_var.i = add i64 %dim_2_ind_var.i, 1
  %dim_2_cmp.to.max.i = icmp eq i64 %dim_2_inc_ind_var.i, %16
  br i1 %dim_2_cmp.to.max.i, label %__reduceNoLocal_separated_args.exit, label %dim_1_pre_head.i

__reduceNoLocal_separated_args.exit:              ; preds = %scalarIf.i, %dim_1_exit.i
  ret void
}

define void @reduce(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float addrspace(3)**
  %7 = load float addrspace(3)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 40
  %12 = bitcast i8* %11 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %13 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 48
  %15 = bitcast i8* %14 to i64**
  %16 = load i64** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 64
  %18 = bitcast i8* %17 to <{ [4 x i64] }>**
  %19 = load <{ [4 x i64] }>** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 80
  %21 = bitcast i8* %20 to i64*
  %22 = load i64* %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 88
  %24 = bitcast i8* %23 to i8**
  %25 = load i8** %24, align 8
  br label %SyncBB59.i

SyncBB59.i:                                       ; preds = %thenBB.i, %entry
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %26 = getelementptr <{ [4 x i64] }>* %19, i64 %CurrWI..0.i, i32 0, i64 0
  %27 = load i64* %26, align 8
  %conv.i = trunc i64 %27 to i32
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %conv.i, i32* %CastToValueType.i, align 4
  %28 = load i64* %16, align 8
  %29 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %13, i64 0, i32 3, i64 0
  %30 = load i64* %29, align 8
  %mul.i = shl i64 %30, 1
  %mul3.i = mul i64 %mul.i, %28
  %add.i = add i64 %mul3.i, %27
  %conv5.i = trunc i64 %add.i to i32
  %31 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %13, i64 0, i32 4, i64 0
  %32 = load i64* %31, align 8
  %mul9.i = mul i64 %mul.i, %32
  %conv10.i = trunc i64 %mul9.i to i32
  %conv12.i = trunc i64 %30 to i32
  %idxprom.i = and i64 %27, 4294967295
  %arrayidx.i = getelementptr inbounds float addrspace(3)* %7, i64 %idxprom.i
  %"&(pSB[currWI].offset)21.i" = add nuw i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset22.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)21.i"
  %CastToValueType23.i = bitcast i8* %"&pSB[currWI].offset22.i" to float addrspace(3)**
  store float addrspace(3)* %arrayidx.i, float addrspace(3)** %CastToValueType23.i, align 8
  store float 0.000000e+00, float addrspace(3)* %arrayidx.i, align 4
  %cmp4.i = icmp ult i32 %conv5.i, %10
  br i1 %cmp4.i, label %while.body.i, label %while.end.i

while.body.i:                                     ; preds = %SyncBB59.i, %while.body.i
  %33 = phi float [ %add22.i, %while.body.i ], [ 0.000000e+00, %SyncBB59.i ]
  %i.05.i = phi i32 [ %add23.i, %while.body.i ], [ %conv5.i, %SyncBB59.i ]
  %idxprom14.i = zext i32 %i.05.i to i64
  %arrayidx15.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom14.i
  %34 = load float addrspace(1)* %arrayidx15.i, align 4
  %add16.i = add i32 %i.05.i, %conv12.i
  %idxprom17.i = zext i32 %add16.i to i64
  %arrayidx18.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom17.i
  %35 = load float addrspace(1)* %arrayidx18.i, align 4
  %add19.i = fadd float %34, %35
  %add22.i = fadd float %33, %add19.i
  %loadedValue38.i = load float addrspace(3)** %CastToValueType23.i, align 8
  store float %add22.i, float addrspace(3)* %loadedValue38.i, align 4
  %add23.i = add i32 %i.05.i, %conv10.i
  %cmp.i = icmp ult i32 %add23.i, %10
  br i1 %cmp.i, label %while.body.i, label %while.end.i

while.end.i:                                      ; preds = %while.body.i, %SyncBB59.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %22
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %while.end.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 24
  br label %SyncBB59.i

elseBB.i:                                         ; preds = %while.end.i
  %s.01.i = lshr i32 %conv12.i, 1
  %cmp242.i = icmp eq i32 %s.01.i, 0
  %arrayidx39.i = getelementptr inbounds float addrspace(1)* %4, i64 %28
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB69.i, %thenBB62.i, %elseBB.i
  %currBarrier.0.i = phi i32 [ 0, %elseBB.i ], [ %currBarrier.3.i, %thenBB69.i ], [ %currBarrier.1.i, %thenBB62.i ]
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride75.i", %thenBB69.i ], [ %"loadedCurrSB+Stride68.i", %thenBB62.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++73.i", %thenBB69.i ], [ %"CurrWI++66.i", %thenBB62.i ]
  br i1 %cmp242.i, label %for.end.i, label %for.body.i

for.body.i:                                       ; preds = %SyncBB58.i, %SyncBB.i
  %currBarrier.1.i = phi i32 [ %currBarrier.2.i, %SyncBB58.i ], [ %currBarrier.0.i, %SyncBB.i ]
  %CurrSBIndex..2.i = phi i64 [ %CurrSBIndex..3.i, %SyncBB58.i ], [ %CurrSBIndex..1.i, %SyncBB.i ]
  %CurrWI..2.i = phi i64 [ %CurrWI..3.i, %SyncBB58.i ], [ %CurrWI..1.i, %SyncBB.i ]
  %s.03.i = phi i32 [ %s.0.i, %SyncBB58.i ], [ %s.01.i, %SyncBB.i ]
  %"&(pSB[currWI].offset)40.i" = add nuw i64 %CurrSBIndex..2.i, 16
  %"&pSB[currWI].offset41.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)40.i"
  %CastToValueType42.i = bitcast i8* %"&pSB[currWI].offset41.i" to i32*
  store i32 %s.03.i, i32* %CastToValueType42.i, align 4
  %"&pSB[currWI].offset12.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..2.i
  %CastToValueType13.i = bitcast i8* %"&pSB[currWI].offset12.i" to i32*
  %loadedValue14.i = load i32* %CastToValueType13.i, align 4
  %cmp26.i = icmp ult i32 %loadedValue14.i, %s.03.i
  br i1 %cmp26.i, label %if.then.i, label %if.end.i

if.then.i:                                        ; preds = %for.body.i
  %add28.i = add i32 %s.03.i, %loadedValue14.i
  %idxprom29.i = zext i32 %add28.i to i64
  %arrayidx30.i = getelementptr inbounds float addrspace(3)* %7, i64 %idxprom29.i
  %36 = load float addrspace(3)* %arrayidx30.i, align 4
  %"&(pSB[currWI].offset)25.i" = add nuw i64 %CurrSBIndex..2.i, 8
  %"&pSB[currWI].offset26.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)25.i"
  %CastToValueType27.i = bitcast i8* %"&pSB[currWI].offset26.i" to float addrspace(3)**
  %loadedValue28.i = load float addrspace(3)** %CastToValueType27.i, align 8
  %37 = load float addrspace(3)* %loadedValue28.i, align 4
  %add33.i = fadd float %37, %36
  store float %add33.i, float addrspace(3)* %loadedValue28.i, align 4
  br label %if.end.i

if.end.i:                                         ; preds = %if.then.i, %for.body.i
  %check.WI.iter65.i = icmp ult i64 %CurrWI..2.i, %22
  br i1 %check.WI.iter65.i, label %thenBB62.i, label %SyncBB58.i

thenBB62.i:                                       ; preds = %if.end.i
  %"CurrWI++66.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride68.i" = add nuw i64 %CurrSBIndex..2.i, 24
  %cond1.i = icmp eq i32 %currBarrier.1.i, 0
  br i1 %cond1.i, label %SyncBB.i, label %SyncBB58.i

SyncBB58.i:                                       ; preds = %thenBB69.i, %thenBB62.i, %if.end.i
  %currBarrier.2.i = phi i32 [ %currBarrier.3.i, %thenBB69.i ], [ %currBarrier.1.i, %thenBB62.i ], [ 1, %if.end.i ]
  %CurrSBIndex..3.i = phi i64 [ %"loadedCurrSB+Stride75.i", %thenBB69.i ], [ %"loadedCurrSB+Stride68.i", %thenBB62.i ], [ 0, %if.end.i ]
  %CurrWI..3.i = phi i64 [ %"CurrWI++73.i", %thenBB69.i ], [ %"CurrWI++66.i", %thenBB62.i ], [ 0, %if.end.i ]
  %"&(pSB[currWI].offset)44.i" = add nuw i64 %CurrSBIndex..3.i, 16
  %"&pSB[currWI].offset45.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)44.i"
  %CastToValueType46.i = bitcast i8* %"&pSB[currWI].offset45.i" to i32*
  %loadedValue47.i = load i32* %CastToValueType46.i, align 4
  %s.0.i = lshr i32 %loadedValue47.i, 1
  %cmp24.i = icmp eq i32 %s.0.i, 0
  br i1 %cmp24.i, label %for.end.i, label %for.body.i

for.end.i:                                        ; preds = %SyncBB58.i, %SyncBB.i
  %currBarrier.3.i = phi i32 [ %currBarrier.0.i, %SyncBB.i ], [ %currBarrier.2.i, %SyncBB58.i ]
  %CurrSBIndex..4.i = phi i64 [ %CurrSBIndex..1.i, %SyncBB.i ], [ %CurrSBIndex..3.i, %SyncBB58.i ]
  %CurrWI..4.i = phi i64 [ %CurrWI..1.i, %SyncBB.i ], [ %CurrWI..3.i, %SyncBB58.i ]
  %"&pSB[currWI].offset17.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..4.i
  %CastToValueType18.i = bitcast i8* %"&pSB[currWI].offset17.i" to i32*
  %loadedValue19.i = load i32* %CastToValueType18.i, align 4
  %cmp34.i = icmp eq i32 %loadedValue19.i, 0
  br i1 %cmp34.i, label %if.then36.i, label %if.end40.i

if.then36.i:                                      ; preds = %for.end.i
  %38 = load float addrspace(3)* %7, align 4
  store float %38, float addrspace(1)* %arrayidx39.i, align 4
  br label %if.end40.i

if.end40.i:                                       ; preds = %if.then36.i, %for.end.i
  %check.WI.iter72.i = icmp ult i64 %CurrWI..4.i, %22
  br i1 %check.WI.iter72.i, label %thenBB69.i, label %__reduce_separated_args.exit

thenBB69.i:                                       ; preds = %if.end40.i
  %"CurrWI++73.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride75.i" = add nuw i64 %CurrSBIndex..4.i, 24
  %cond.i = icmp eq i32 %currBarrier.3.i, 1
  br i1 %cond.i, label %SyncBB58.i, label %SyncBB.i

__reduce_separated_args.exit:                     ; preds = %if.end40.i
  ret void
}

define void @__Vectorized_.reduce(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float addrspace(3)**
  %7 = load float addrspace(3)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 40
  %12 = bitcast i8* %11 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %13 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 48
  %15 = bitcast i8* %14 to i64**
  %16 = load i64** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 64
  %18 = bitcast i8* %17 to <{ [4 x i64] }>**
  %19 = load <{ [4 x i64] }>** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 80
  %21 = bitcast i8* %20 to i64*
  %22 = load i64* %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 88
  %24 = bitcast i8* %23 to i8**
  %25 = load i8** %24, align 8
  %temp68.i = insertelement <16 x i32> undef, i32 %10, i32 0
  %vector69.i = shufflevector <16 x i32> %temp68.i, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB1250.i

SyncBB1250.i:                                     ; preds = %thenBB.i, %entry
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %26 = getelementptr <{ [4 x i64] }>* %19, i64 %CurrWI..0.i, i32 0, i64 0
  %27 = load i64* %26, align 8
  %broadcast1.i = insertelement <16 x i64> undef, i64 %27, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %28 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %conv49.i = trunc <16 x i64> %28 to <16 x i32>
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to <16 x i32>*
  store <16 x i32> %conv49.i, <16 x i32>* %CastToValueType.i, align 64
  %29 = load i64* %16, align 8
  %30 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %13, i64 0, i32 3, i64 0
  %31 = load i64* %30, align 8
  %mul.i = shl i64 %31, 1
  %mul3.i = mul i64 %mul.i, %29
  %temp.i = insertelement <16 x i64> undef, i64 %mul3.i, i32 0
  %vector.i = shufflevector <16 x i64> %temp.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %add50.i = add <16 x i64> %vector.i, %28
  %conv551.i = trunc <16 x i64> %add50.i to <16 x i32>
  %32 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %13, i64 0, i32 4, i64 0
  %33 = load i64* %32, align 8
  %mul9.i = mul i64 %mul.i, %33
  %conv10.i = trunc i64 %mul9.i to i32
  %temp163.i = insertelement <16 x i32> undef, i32 %conv10.i, i32 0
  %vector164.i = shufflevector <16 x i32> %temp163.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %conv12.i = trunc i64 %31 to i32
  %temp109.i = insertelement <16 x i32> undef, i32 %conv12.i, i32 0
  %vector110.i = shufflevector <16 x i32> %temp109.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %extract.lhs.i = extractelement <16 x i64> %28, i32 0
  %extract.i = and i64 %extract.lhs.i, 4294967295
  %"&(pSB[currWI].offset)9617.i" = or i64 %CurrSBIndex..0.i, 64
  %"&pSB[currWI].offset962.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)9617.i"
  %CastToValueType963.i = bitcast i8* %"&pSB[currWI].offset962.i" to i64*
  store i64 %extract.i, i64* %CastToValueType963.i, align 8
  %34 = getelementptr inbounds float addrspace(3)* %7, i64 %extract.i
  %"&(pSB[currWI].offset)11908.i" = or i64 %CurrSBIndex..0.i, 72
  %"&pSB[currWI].offset1191.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)11908.i"
  %CastToValueType1192.i = bitcast i8* %"&pSB[currWI].offset1191.i" to float addrspace(3)**
  store float addrspace(3)* %34, float addrspace(3)** %CastToValueType1192.i, align 8
  %ptrTypeCast.i = bitcast float addrspace(3)* %34 to <16 x float> addrspace(3)*
  store <16 x float> zeroinitializer, <16 x float> addrspace(3)* %ptrTypeCast.i, align 4
  %cmp4.i = icmp ult <16 x i32> %conv551.i, %vector69.i
  %ipred.i.i = bitcast <16 x i1> %cmp4.i to i16
  %val.i.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i.i, i16 %ipred.i.i) nounwind
  %35 = and i32 %val.i.i, 1
  %res.i.i = icmp eq i32 %35, 0
  br i1 %res.i.i, label %while.body.preheader.i, label %while.end.i

while.body.preheader.i:                           ; preds = %SyncBB1250.i
  %negIncomingLoopMask71.i = xor <16 x i1> %cmp4.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  br label %while.body.i

while.body.i:                                     ; preds = %postload674.i, %while.body.preheader.i
  %vectorPHI.i = phi <16 x i1> [ %loop_mask1169.i, %postload674.i ], [ %negIncomingLoopMask71.i, %while.body.preheader.i ]
  %vectorPHI73.i = phi <16 x i1> [ %local_edge188.i, %postload674.i ], [ %cmp4.i, %while.body.preheader.i ]
  %vectorPHI74.i = phi <16 x float> [ %add22161.i, %postload674.i ], [ zeroinitializer, %while.body.preheader.i ]
  %vectorPHI75.i = phi <16 x i32> [ %add23165.i, %postload674.i ], [ %conv551.i, %while.body.preheader.i ]
  %extract93.i = extractelement <16 x i1> %vectorPHI73.i, i32 0
  %extract94.i = extractelement <16 x i1> %vectorPHI73.i, i32 1
  %extract95.i = extractelement <16 x i1> %vectorPHI73.i, i32 2
  %extract96.i = extractelement <16 x i1> %vectorPHI73.i, i32 3
  %extract97.i = extractelement <16 x i1> %vectorPHI73.i, i32 4
  %extract98.i = extractelement <16 x i1> %vectorPHI73.i, i32 5
  %extract99.i = extractelement <16 x i1> %vectorPHI73.i, i32 6
  %extract100.i = extractelement <16 x i1> %vectorPHI73.i, i32 7
  %extract101.i = extractelement <16 x i1> %vectorPHI73.i, i32 8
  %extract102.i = extractelement <16 x i1> %vectorPHI73.i, i32 9
  %extract103.i = extractelement <16 x i1> %vectorPHI73.i, i32 10
  %extract104.i = extractelement <16 x i1> %vectorPHI73.i, i32 11
  %extract105.i = extractelement <16 x i1> %vectorPHI73.i, i32 12
  %extract106.i = extractelement <16 x i1> %vectorPHI73.i, i32 13
  %extract107.i = extractelement <16 x i1> %vectorPHI73.i, i32 14
  %extract108.i = extractelement <16 x i1> %vectorPHI73.i, i32 15
  %idxprom1476.i = zext <16 x i32> %vectorPHI75.i to <16 x i64>
  %extract78.i = extractelement <16 x i64> %idxprom1476.i, i32 1
  %extract79.i = extractelement <16 x i64> %idxprom1476.i, i32 2
  %extract80.i = extractelement <16 x i64> %idxprom1476.i, i32 3
  %extract81.i = extractelement <16 x i64> %idxprom1476.i, i32 4
  %extract82.i = extractelement <16 x i64> %idxprom1476.i, i32 5
  %extract83.i = extractelement <16 x i64> %idxprom1476.i, i32 6
  %extract84.i = extractelement <16 x i64> %idxprom1476.i, i32 7
  %extract85.i = extractelement <16 x i64> %idxprom1476.i, i32 8
  %extract86.i = extractelement <16 x i64> %idxprom1476.i, i32 9
  %extract87.i = extractelement <16 x i64> %idxprom1476.i, i32 10
  %extract88.i = extractelement <16 x i64> %idxprom1476.i, i32 11
  %extract89.i = extractelement <16 x i64> %idxprom1476.i, i32 12
  %extract90.i = extractelement <16 x i64> %idxprom1476.i, i32 13
  %extract91.i = extractelement <16 x i64> %idxprom1476.i, i32 14
  %extract92.i = extractelement <16 x i64> %idxprom1476.i, i32 15
  %36 = getelementptr inbounds float addrspace(1)* %1, i64 %extract78.i
  %37 = getelementptr inbounds float addrspace(1)* %1, i64 %extract79.i
  %38 = getelementptr inbounds float addrspace(1)* %1, i64 %extract80.i
  %39 = getelementptr inbounds float addrspace(1)* %1, i64 %extract81.i
  %40 = getelementptr inbounds float addrspace(1)* %1, i64 %extract82.i
  %41 = getelementptr inbounds float addrspace(1)* %1, i64 %extract83.i
  %42 = getelementptr inbounds float addrspace(1)* %1, i64 %extract84.i
  %43 = getelementptr inbounds float addrspace(1)* %1, i64 %extract85.i
  %44 = getelementptr inbounds float addrspace(1)* %1, i64 %extract86.i
  %45 = getelementptr inbounds float addrspace(1)* %1, i64 %extract87.i
  %46 = getelementptr inbounds float addrspace(1)* %1, i64 %extract88.i
  %47 = getelementptr inbounds float addrspace(1)* %1, i64 %extract89.i
  %48 = getelementptr inbounds float addrspace(1)* %1, i64 %extract90.i
  %49 = getelementptr inbounds float addrspace(1)* %1, i64 %extract91.i
  %50 = getelementptr inbounds float addrspace(1)* %1, i64 %extract92.i
  br i1 %extract93.i, label %preload574.i, label %postload575.i

preload574.i:                                     ; preds = %while.body.i
  %extract77.i = extractelement <16 x i64> %idxprom1476.i, i32 0
  %51 = getelementptr inbounds float addrspace(1)* %1, i64 %extract77.i
  %masked_load.i = load float addrspace(1)* %51, align 4
  br label %postload575.i

postload575.i:                                    ; preds = %preload574.i, %while.body.i
  %phi576.i = phi float [ undef, %while.body.i ], [ %masked_load.i, %preload574.i ]
  br i1 %extract94.i, label %preload580.i, label %postload581.i

preload580.i:                                     ; preds = %postload575.i
  %masked_load314.i = load float addrspace(1)* %36, align 4
  br label %postload581.i

postload581.i:                                    ; preds = %preload580.i, %postload575.i
  %phi582.i = phi float [ undef, %postload575.i ], [ %masked_load314.i, %preload580.i ]
  br i1 %extract95.i, label %preload586.i, label %postload587.i

preload586.i:                                     ; preds = %postload581.i
  %masked_load315.i = load float addrspace(1)* %37, align 4
  br label %postload587.i

postload587.i:                                    ; preds = %preload586.i, %postload581.i
  %phi588.i = phi float [ undef, %postload581.i ], [ %masked_load315.i, %preload586.i ]
  br i1 %extract96.i, label %preload592.i, label %postload593.i

preload592.i:                                     ; preds = %postload587.i
  %masked_load316.i = load float addrspace(1)* %38, align 4
  br label %postload593.i

postload593.i:                                    ; preds = %preload592.i, %postload587.i
  %phi594.i = phi float [ undef, %postload587.i ], [ %masked_load316.i, %preload592.i ]
  br i1 %extract97.i, label %preload598.i, label %postload599.i

preload598.i:                                     ; preds = %postload593.i
  %masked_load317.i = load float addrspace(1)* %39, align 4
  br label %postload599.i

postload599.i:                                    ; preds = %preload598.i, %postload593.i
  %phi600.i = phi float [ undef, %postload593.i ], [ %masked_load317.i, %preload598.i ]
  br i1 %extract98.i, label %preload604.i, label %postload605.i

preload604.i:                                     ; preds = %postload599.i
  %masked_load318.i = load float addrspace(1)* %40, align 4
  br label %postload605.i

postload605.i:                                    ; preds = %preload604.i, %postload599.i
  %phi606.i = phi float [ undef, %postload599.i ], [ %masked_load318.i, %preload604.i ]
  br i1 %extract99.i, label %preload610.i, label %postload611.i

preload610.i:                                     ; preds = %postload605.i
  %masked_load319.i = load float addrspace(1)* %41, align 4
  br label %postload611.i

postload611.i:                                    ; preds = %preload610.i, %postload605.i
  %phi612.i = phi float [ undef, %postload605.i ], [ %masked_load319.i, %preload610.i ]
  br i1 %extract100.i, label %preload616.i, label %postload617.i

preload616.i:                                     ; preds = %postload611.i
  %masked_load320.i = load float addrspace(1)* %42, align 4
  br label %postload617.i

postload617.i:                                    ; preds = %preload616.i, %postload611.i
  %phi618.i = phi float [ undef, %postload611.i ], [ %masked_load320.i, %preload616.i ]
  br i1 %extract101.i, label %preload622.i, label %postload623.i

preload622.i:                                     ; preds = %postload617.i
  %masked_load321.i = load float addrspace(1)* %43, align 4
  br label %postload623.i

postload623.i:                                    ; preds = %preload622.i, %postload617.i
  %phi624.i = phi float [ undef, %postload617.i ], [ %masked_load321.i, %preload622.i ]
  br i1 %extract102.i, label %preload628.i, label %postload629.i

preload628.i:                                     ; preds = %postload623.i
  %masked_load322.i = load float addrspace(1)* %44, align 4
  br label %postload629.i

postload629.i:                                    ; preds = %preload628.i, %postload623.i
  %phi630.i = phi float [ undef, %postload623.i ], [ %masked_load322.i, %preload628.i ]
  br i1 %extract103.i, label %preload634.i, label %postload635.i

preload634.i:                                     ; preds = %postload629.i
  %masked_load323.i = load float addrspace(1)* %45, align 4
  br label %postload635.i

postload635.i:                                    ; preds = %preload634.i, %postload629.i
  %phi636.i = phi float [ undef, %postload629.i ], [ %masked_load323.i, %preload634.i ]
  br i1 %extract104.i, label %preload640.i, label %postload641.i

preload640.i:                                     ; preds = %postload635.i
  %masked_load324.i = load float addrspace(1)* %46, align 4
  br label %postload641.i

postload641.i:                                    ; preds = %preload640.i, %postload635.i
  %phi642.i = phi float [ undef, %postload635.i ], [ %masked_load324.i, %preload640.i ]
  br i1 %extract105.i, label %preload646.i, label %postload647.i

preload646.i:                                     ; preds = %postload641.i
  %masked_load325.i = load float addrspace(1)* %47, align 4
  br label %postload647.i

postload647.i:                                    ; preds = %preload646.i, %postload641.i
  %phi648.i = phi float [ undef, %postload641.i ], [ %masked_load325.i, %preload646.i ]
  br i1 %extract106.i, label %preload652.i, label %postload653.i

preload652.i:                                     ; preds = %postload647.i
  %masked_load326.i = load float addrspace(1)* %48, align 4
  br label %postload653.i

postload653.i:                                    ; preds = %preload652.i, %postload647.i
  %phi654.i = phi float [ undef, %postload647.i ], [ %masked_load326.i, %preload652.i ]
  br i1 %extract107.i, label %preload658.i, label %postload659.i

preload658.i:                                     ; preds = %postload653.i
  %masked_load327.i = load float addrspace(1)* %49, align 4
  br label %postload659.i

postload659.i:                                    ; preds = %preload658.i, %postload653.i
  %phi660.i = phi float [ undef, %postload653.i ], [ %masked_load327.i, %preload658.i ]
  br i1 %extract108.i, label %preload664.i, label %postload665.i

preload664.i:                                     ; preds = %postload659.i
  %masked_load328.i = load float addrspace(1)* %50, align 4
  br label %postload665.i

postload665.i:                                    ; preds = %preload664.i, %postload659.i
  %phi666.i = phi float [ undef, %postload659.i ], [ %masked_load328.i, %preload664.i ]
  %temp.vect.i = insertelement <16 x float> undef, float %phi576.i, i32 0
  %temp.vect129.i = insertelement <16 x float> %temp.vect.i, float %phi582.i, i32 1
  %temp.vect130.i = insertelement <16 x float> %temp.vect129.i, float %phi588.i, i32 2
  %temp.vect131.i = insertelement <16 x float> %temp.vect130.i, float %phi594.i, i32 3
  %temp.vect132.i = insertelement <16 x float> %temp.vect131.i, float %phi600.i, i32 4
  %temp.vect133.i = insertelement <16 x float> %temp.vect132.i, float %phi606.i, i32 5
  %temp.vect134.i = insertelement <16 x float> %temp.vect133.i, float %phi612.i, i32 6
  %temp.vect135.i = insertelement <16 x float> %temp.vect134.i, float %phi618.i, i32 7
  %temp.vect136.i = insertelement <16 x float> %temp.vect135.i, float %phi624.i, i32 8
  %temp.vect137.i = insertelement <16 x float> %temp.vect136.i, float %phi630.i, i32 9
  %temp.vect138.i = insertelement <16 x float> %temp.vect137.i, float %phi636.i, i32 10
  %temp.vect139.i = insertelement <16 x float> %temp.vect138.i, float %phi642.i, i32 11
  %temp.vect140.i = insertelement <16 x float> %temp.vect139.i, float %phi648.i, i32 12
  %temp.vect141.i = insertelement <16 x float> %temp.vect140.i, float %phi654.i, i32 13
  %temp.vect142.i = insertelement <16 x float> %temp.vect141.i, float %phi660.i, i32 14
  %temp.vect143.i = insertelement <16 x float> %temp.vect142.i, float %phi666.i, i32 15
  %add16111.i = add <16 x i32> %vectorPHI75.i, %vector110.i
  %idxprom17112.i = zext <16 x i32> %add16111.i to <16 x i64>
  %extract114.i = extractelement <16 x i64> %idxprom17112.i, i32 1
  %extract115.i = extractelement <16 x i64> %idxprom17112.i, i32 2
  %extract116.i = extractelement <16 x i64> %idxprom17112.i, i32 3
  %extract117.i = extractelement <16 x i64> %idxprom17112.i, i32 4
  %extract118.i = extractelement <16 x i64> %idxprom17112.i, i32 5
  %extract119.i = extractelement <16 x i64> %idxprom17112.i, i32 6
  %extract120.i = extractelement <16 x i64> %idxprom17112.i, i32 7
  %extract121.i = extractelement <16 x i64> %idxprom17112.i, i32 8
  %extract122.i = extractelement <16 x i64> %idxprom17112.i, i32 9
  %extract123.i = extractelement <16 x i64> %idxprom17112.i, i32 10
  %extract124.i = extractelement <16 x i64> %idxprom17112.i, i32 11
  %extract125.i = extractelement <16 x i64> %idxprom17112.i, i32 12
  %extract126.i = extractelement <16 x i64> %idxprom17112.i, i32 13
  %extract127.i = extractelement <16 x i64> %idxprom17112.i, i32 14
  %extract128.i = extractelement <16 x i64> %idxprom17112.i, i32 15
  %52 = getelementptr inbounds float addrspace(1)* %1, i64 %extract114.i
  %53 = getelementptr inbounds float addrspace(1)* %1, i64 %extract115.i
  %54 = getelementptr inbounds float addrspace(1)* %1, i64 %extract116.i
  %55 = getelementptr inbounds float addrspace(1)* %1, i64 %extract117.i
  %56 = getelementptr inbounds float addrspace(1)* %1, i64 %extract118.i
  %57 = getelementptr inbounds float addrspace(1)* %1, i64 %extract119.i
  %58 = getelementptr inbounds float addrspace(1)* %1, i64 %extract120.i
  %59 = getelementptr inbounds float addrspace(1)* %1, i64 %extract121.i
  %60 = getelementptr inbounds float addrspace(1)* %1, i64 %extract122.i
  %61 = getelementptr inbounds float addrspace(1)* %1, i64 %extract123.i
  %62 = getelementptr inbounds float addrspace(1)* %1, i64 %extract124.i
  %63 = getelementptr inbounds float addrspace(1)* %1, i64 %extract125.i
  %64 = getelementptr inbounds float addrspace(1)* %1, i64 %extract126.i
  %65 = getelementptr inbounds float addrspace(1)* %1, i64 %extract127.i
  %66 = getelementptr inbounds float addrspace(1)* %1, i64 %extract128.i
  br i1 %extract93.i, label %preload577.i, label %postload578.i

preload577.i:                                     ; preds = %postload665.i
  %extract113.i = extractelement <16 x i64> %idxprom17112.i, i32 0
  %67 = getelementptr inbounds float addrspace(1)* %1, i64 %extract113.i
  %masked_load329.i = load float addrspace(1)* %67, align 4
  br label %postload578.i

postload578.i:                                    ; preds = %preload577.i, %postload665.i
  %phi579.i = phi float [ undef, %postload665.i ], [ %masked_load329.i, %preload577.i ]
  br i1 %extract94.i, label %preload583.i, label %postload584.i

preload583.i:                                     ; preds = %postload578.i
  %masked_load330.i = load float addrspace(1)* %52, align 4
  br label %postload584.i

postload584.i:                                    ; preds = %preload583.i, %postload578.i
  %phi585.i = phi float [ undef, %postload578.i ], [ %masked_load330.i, %preload583.i ]
  br i1 %extract95.i, label %preload589.i, label %postload590.i

preload589.i:                                     ; preds = %postload584.i
  %masked_load331.i = load float addrspace(1)* %53, align 4
  br label %postload590.i

postload590.i:                                    ; preds = %preload589.i, %postload584.i
  %phi591.i = phi float [ undef, %postload584.i ], [ %masked_load331.i, %preload589.i ]
  br i1 %extract96.i, label %preload595.i, label %postload596.i

preload595.i:                                     ; preds = %postload590.i
  %masked_load332.i = load float addrspace(1)* %54, align 4
  br label %postload596.i

postload596.i:                                    ; preds = %preload595.i, %postload590.i
  %phi597.i = phi float [ undef, %postload590.i ], [ %masked_load332.i, %preload595.i ]
  br i1 %extract97.i, label %preload601.i, label %postload602.i

preload601.i:                                     ; preds = %postload596.i
  %masked_load333.i = load float addrspace(1)* %55, align 4
  br label %postload602.i

postload602.i:                                    ; preds = %preload601.i, %postload596.i
  %phi603.i = phi float [ undef, %postload596.i ], [ %masked_load333.i, %preload601.i ]
  br i1 %extract98.i, label %preload607.i, label %postload608.i

preload607.i:                                     ; preds = %postload602.i
  %masked_load334.i = load float addrspace(1)* %56, align 4
  br label %postload608.i

postload608.i:                                    ; preds = %preload607.i, %postload602.i
  %phi609.i = phi float [ undef, %postload602.i ], [ %masked_load334.i, %preload607.i ]
  br i1 %extract99.i, label %preload613.i, label %postload614.i

preload613.i:                                     ; preds = %postload608.i
  %masked_load335.i = load float addrspace(1)* %57, align 4
  br label %postload614.i

postload614.i:                                    ; preds = %preload613.i, %postload608.i
  %phi615.i = phi float [ undef, %postload608.i ], [ %masked_load335.i, %preload613.i ]
  br i1 %extract100.i, label %preload619.i, label %postload620.i

preload619.i:                                     ; preds = %postload614.i
  %masked_load336.i = load float addrspace(1)* %58, align 4
  br label %postload620.i

postload620.i:                                    ; preds = %preload619.i, %postload614.i
  %phi621.i = phi float [ undef, %postload614.i ], [ %masked_load336.i, %preload619.i ]
  br i1 %extract101.i, label %preload625.i, label %postload626.i

preload625.i:                                     ; preds = %postload620.i
  %masked_load337.i = load float addrspace(1)* %59, align 4
  br label %postload626.i

postload626.i:                                    ; preds = %preload625.i, %postload620.i
  %phi627.i = phi float [ undef, %postload620.i ], [ %masked_load337.i, %preload625.i ]
  br i1 %extract102.i, label %preload631.i, label %postload632.i

preload631.i:                                     ; preds = %postload626.i
  %masked_load338.i = load float addrspace(1)* %60, align 4
  br label %postload632.i

postload632.i:                                    ; preds = %preload631.i, %postload626.i
  %phi633.i = phi float [ undef, %postload626.i ], [ %masked_load338.i, %preload631.i ]
  br i1 %extract103.i, label %preload637.i, label %postload638.i

preload637.i:                                     ; preds = %postload632.i
  %masked_load339.i = load float addrspace(1)* %61, align 4
  br label %postload638.i

postload638.i:                                    ; preds = %preload637.i, %postload632.i
  %phi639.i = phi float [ undef, %postload632.i ], [ %masked_load339.i, %preload637.i ]
  br i1 %extract104.i, label %preload643.i, label %postload644.i

preload643.i:                                     ; preds = %postload638.i
  %masked_load340.i = load float addrspace(1)* %62, align 4
  br label %postload644.i

postload644.i:                                    ; preds = %preload643.i, %postload638.i
  %phi645.i = phi float [ undef, %postload638.i ], [ %masked_load340.i, %preload643.i ]
  br i1 %extract105.i, label %preload649.i, label %postload650.i

preload649.i:                                     ; preds = %postload644.i
  %masked_load341.i = load float addrspace(1)* %63, align 4
  br label %postload650.i

postload650.i:                                    ; preds = %preload649.i, %postload644.i
  %phi651.i = phi float [ undef, %postload644.i ], [ %masked_load341.i, %preload649.i ]
  br i1 %extract106.i, label %preload655.i, label %postload656.i

preload655.i:                                     ; preds = %postload650.i
  %masked_load342.i = load float addrspace(1)* %64, align 4
  br label %postload656.i

postload656.i:                                    ; preds = %preload655.i, %postload650.i
  %phi657.i = phi float [ undef, %postload650.i ], [ %masked_load342.i, %preload655.i ]
  br i1 %extract107.i, label %preload661.i, label %postload662.i

preload661.i:                                     ; preds = %postload656.i
  %masked_load343.i = load float addrspace(1)* %65, align 4
  br label %postload662.i

postload662.i:                                    ; preds = %preload661.i, %postload656.i
  %phi663.i = phi float [ undef, %postload656.i ], [ %masked_load343.i, %preload661.i ]
  br i1 %extract108.i, label %preload667.i, label %postload668.i

preload667.i:                                     ; preds = %postload662.i
  %masked_load344.i = load float addrspace(1)* %66, align 4
  br label %postload668.i

postload668.i:                                    ; preds = %preload667.i, %postload662.i
  %phi669.i = phi float [ undef, %postload662.i ], [ %masked_load344.i, %preload667.i ]
  %temp.vect144.i = insertelement <16 x float> undef, float %phi579.i, i32 0
  %temp.vect145.i = insertelement <16 x float> %temp.vect144.i, float %phi585.i, i32 1
  %temp.vect146.i = insertelement <16 x float> %temp.vect145.i, float %phi591.i, i32 2
  %temp.vect147.i = insertelement <16 x float> %temp.vect146.i, float %phi597.i, i32 3
  %temp.vect148.i = insertelement <16 x float> %temp.vect147.i, float %phi603.i, i32 4
  %temp.vect149.i = insertelement <16 x float> %temp.vect148.i, float %phi609.i, i32 5
  %temp.vect150.i = insertelement <16 x float> %temp.vect149.i, float %phi615.i, i32 6
  %temp.vect151.i = insertelement <16 x float> %temp.vect150.i, float %phi621.i, i32 7
  %temp.vect152.i = insertelement <16 x float> %temp.vect151.i, float %phi627.i, i32 8
  %temp.vect153.i = insertelement <16 x float> %temp.vect152.i, float %phi633.i, i32 9
  %temp.vect154.i = insertelement <16 x float> %temp.vect153.i, float %phi639.i, i32 10
  %temp.vect155.i = insertelement <16 x float> %temp.vect154.i, float %phi645.i, i32 11
  %temp.vect156.i = insertelement <16 x float> %temp.vect155.i, float %phi651.i, i32 12
  %temp.vect157.i = insertelement <16 x float> %temp.vect156.i, float %phi657.i, i32 13
  %temp.vect158.i = insertelement <16 x float> %temp.vect157.i, float %phi663.i, i32 14
  %temp.vect159.i = insertelement <16 x float> %temp.vect158.i, float %phi669.i, i32 15
  %add19160.i = fadd <16 x float> %temp.vect143.i, %temp.vect159.i
  %add22161.i = fadd <16 x float> %vectorPHI74.i, %add19160.i
  br i1 %extract93.i, label %preload556.i, label %postload557.i

preload556.i:                                     ; preds = %postload668.i
  %exData.i = extractelement <16 x float> %add22161.i, i32 0
  %loadedValue1197.i = load float addrspace(3)** %CastToValueType1192.i, align 8
  store float %exData.i, float addrspace(3)* %loadedValue1197.i, align 4
  br label %postload557.i

postload557.i:                                    ; preds = %preload556.i, %postload668.i
  br i1 %extract94.i, label %preload553.i, label %postload554.i

preload553.i:                                     ; preds = %postload557.i
  %loadedValue968.i = load i64* %CastToValueType963.i, align 8
  %.sum945.i = add i64 %loadedValue968.i, 1
  %68 = getelementptr float addrspace(3)* %7, i64 %.sum945.i
  %exData348.i = extractelement <16 x float> %add22161.i, i32 1
  store float %exData348.i, float addrspace(3)* %68, align 4
  br label %postload554.i

postload554.i:                                    ; preds = %preload553.i, %postload557.i
  br i1 %extract95.i, label %preload.i, label %postload.i

preload.i:                                        ; preds = %postload554.i
  %loadedValue973.i = load i64* %CastToValueType963.i, align 8
  %.sum944.i = add i64 %loadedValue973.i, 2
  %69 = getelementptr float addrspace(3)* %7, i64 %.sum944.i
  %exData351.i = extractelement <16 x float> %add22161.i, i32 2
  store float %exData351.i, float addrspace(3)* %69, align 4
  br label %postload.i

postload.i:                                       ; preds = %preload.i, %postload554.i
  br i1 %extract96.i, label %preload559.i, label %postload560.i

preload559.i:                                     ; preds = %postload.i
  %loadedValue978.i = load i64* %CastToValueType963.i, align 8
  %.sum943.i = add i64 %loadedValue978.i, 3
  %70 = getelementptr float addrspace(3)* %7, i64 %.sum943.i
  %exData354.i = extractelement <16 x float> %add22161.i, i32 3
  store float %exData354.i, float addrspace(3)* %70, align 4
  br label %postload560.i

postload560.i:                                    ; preds = %preload559.i, %postload.i
  br i1 %extract97.i, label %preload541.i, label %postload542.i

preload541.i:                                     ; preds = %postload560.i
  %loadedValue983.i = load i64* %CastToValueType963.i, align 8
  %.sum942.i = add i64 %loadedValue983.i, 4
  %71 = getelementptr float addrspace(3)* %7, i64 %.sum942.i
  %exData357.i = extractelement <16 x float> %add22161.i, i32 4
  store float %exData357.i, float addrspace(3)* %71, align 4
  br label %postload542.i

postload542.i:                                    ; preds = %preload541.i, %postload560.i
  br i1 %extract98.i, label %preload562.i, label %postload563.i

preload562.i:                                     ; preds = %postload542.i
  %loadedValue988.i = load i64* %CastToValueType963.i, align 8
  %.sum941.i = add i64 %loadedValue988.i, 5
  %72 = getelementptr float addrspace(3)* %7, i64 %.sum941.i
  %exData360.i = extractelement <16 x float> %add22161.i, i32 5
  store float %exData360.i, float addrspace(3)* %72, align 4
  br label %postload563.i

postload563.i:                                    ; preds = %preload562.i, %postload542.i
  br i1 %extract99.i, label %preload670.i, label %postload671.i

preload670.i:                                     ; preds = %postload563.i
  %loadedValue993.i = load i64* %CastToValueType963.i, align 8
  %.sum940.i = add i64 %loadedValue993.i, 6
  %73 = getelementptr float addrspace(3)* %7, i64 %.sum940.i
  %exData363.i = extractelement <16 x float> %add22161.i, i32 6
  store float %exData363.i, float addrspace(3)* %73, align 4
  br label %postload671.i

postload671.i:                                    ; preds = %preload670.i, %postload563.i
  br i1 %extract100.i, label %preload535.i, label %postload536.i

preload535.i:                                     ; preds = %postload671.i
  %loadedValue998.i = load i64* %CastToValueType963.i, align 8
  %.sum939.i = add i64 %loadedValue998.i, 7
  %74 = getelementptr float addrspace(3)* %7, i64 %.sum939.i
  %exData366.i = extractelement <16 x float> %add22161.i, i32 7
  store float %exData366.i, float addrspace(3)* %74, align 4
  br label %postload536.i

postload536.i:                                    ; preds = %preload535.i, %postload671.i
  br i1 %extract101.i, label %preload544.i, label %postload545.i

preload544.i:                                     ; preds = %postload536.i
  %loadedValue1003.i = load i64* %CastToValueType963.i, align 8
  %.sum938.i = add i64 %loadedValue1003.i, 8
  %75 = getelementptr float addrspace(3)* %7, i64 %.sum938.i
  %exData369.i = extractelement <16 x float> %add22161.i, i32 8
  store float %exData369.i, float addrspace(3)* %75, align 4
  br label %postload545.i

postload545.i:                                    ; preds = %preload544.i, %postload536.i
  br i1 %extract102.i, label %preload565.i, label %postload566.i

preload565.i:                                     ; preds = %postload545.i
  %loadedValue1008.i = load i64* %CastToValueType963.i, align 8
  %.sum937.i = add i64 %loadedValue1008.i, 9
  %76 = getelementptr float addrspace(3)* %7, i64 %.sum937.i
  %exData372.i = extractelement <16 x float> %add22161.i, i32 9
  store float %exData372.i, float addrspace(3)* %76, align 4
  br label %postload566.i

postload566.i:                                    ; preds = %preload565.i, %postload545.i
  br i1 %extract103.i, label %preload538.i, label %postload539.i

preload538.i:                                     ; preds = %postload566.i
  %loadedValue1013.i = load i64* %CastToValueType963.i, align 8
  %.sum936.i = add i64 %loadedValue1013.i, 10
  %77 = getelementptr float addrspace(3)* %7, i64 %.sum936.i
  %exData375.i = extractelement <16 x float> %add22161.i, i32 10
  store float %exData375.i, float addrspace(3)* %77, align 4
  br label %postload539.i

postload539.i:                                    ; preds = %preload538.i, %postload566.i
  br i1 %extract104.i, label %preload547.i, label %postload548.i

preload547.i:                                     ; preds = %postload539.i
  %loadedValue1018.i = load i64* %CastToValueType963.i, align 8
  %.sum935.i = add i64 %loadedValue1018.i, 11
  %78 = getelementptr float addrspace(3)* %7, i64 %.sum935.i
  %exData378.i = extractelement <16 x float> %add22161.i, i32 11
  store float %exData378.i, float addrspace(3)* %78, align 4
  br label %postload548.i

postload548.i:                                    ; preds = %preload547.i, %postload539.i
  br i1 %extract105.i, label %preload550.i, label %postload551.i

preload550.i:                                     ; preds = %postload548.i
  %loadedValue1023.i = load i64* %CastToValueType963.i, align 8
  %.sum934.i = add i64 %loadedValue1023.i, 12
  %79 = getelementptr float addrspace(3)* %7, i64 %.sum934.i
  %exData381.i = extractelement <16 x float> %add22161.i, i32 12
  store float %exData381.i, float addrspace(3)* %79, align 4
  br label %postload551.i

postload551.i:                                    ; preds = %preload550.i, %postload548.i
  br i1 %extract106.i, label %preload568.i, label %postload569.i

preload568.i:                                     ; preds = %postload551.i
  %loadedValue1028.i = load i64* %CastToValueType963.i, align 8
  %.sum933.i = add i64 %loadedValue1028.i, 13
  %80 = getelementptr float addrspace(3)* %7, i64 %.sum933.i
  %exData384.i = extractelement <16 x float> %add22161.i, i32 13
  store float %exData384.i, float addrspace(3)* %80, align 4
  br label %postload569.i

postload569.i:                                    ; preds = %preload568.i, %postload551.i
  br i1 %extract107.i, label %preload571.i, label %postload572.i

preload571.i:                                     ; preds = %postload569.i
  %loadedValue1033.i = load i64* %CastToValueType963.i, align 8
  %.sum932.i = add i64 %loadedValue1033.i, 14
  %81 = getelementptr float addrspace(3)* %7, i64 %.sum932.i
  %exData387.i = extractelement <16 x float> %add22161.i, i32 14
  store float %exData387.i, float addrspace(3)* %81, align 4
  br label %postload572.i

postload572.i:                                    ; preds = %preload571.i, %postload569.i
  br i1 %extract108.i, label %preload673.i, label %postload674.i

preload673.i:                                     ; preds = %postload572.i
  %loadedValue1038.i = load i64* %CastToValueType963.i, align 8
  %.sum931.i = add i64 %loadedValue1038.i, 15
  %82 = getelementptr float addrspace(3)* %7, i64 %.sum931.i
  %exData390.i = extractelement <16 x float> %add22161.i, i32 15
  store float %exData390.i, float addrspace(3)* %82, align 4
  br label %postload674.i

postload674.i:                                    ; preds = %preload673.i, %postload572.i
  %add23165.i = add <16 x i32> %vectorPHI75.i, %vector164.i
  %cmp.i = icmp ult <16 x i32> %add23165.i, %vector69.i
  %notCond166.i = xor <16 x i1> %cmp.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %who_left_tr167.i = and <16 x i1> %vectorPHI73.i, %notCond166.i
  %loop_mask1169.i = or <16 x i1> %vectorPHI.i, %who_left_tr167.i
  %ipred.i4.i = bitcast <16 x i1> %loop_mask1169.i to i16
  %val.i5.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i4.i, i16 %ipred.i4.i) nounwind
  %83 = and i32 %val.i5.i, 1
  %res.i6.i = icmp eq i32 %83, 0
  %local_edge188.i = and <16 x i1> %vectorPHI73.i, %cmp.i
  br i1 %res.i6.i, label %while.body.i, label %while.end.i

while.end.i:                                      ; preds = %postload674.i, %SyncBB1250.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %22
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %while.end.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 128
  br label %SyncBB1250.i

elseBB.i:                                         ; preds = %while.end.i
  %s.01.i = lshr i32 %conv12.i, 1
  %Mneg3.i = icmp ne i32 %s.01.i, 0
  %temp210.i = insertelement <16 x i1> undef, i1 %Mneg3.i, i32 0
  %vector211.i = shufflevector <16 x i1> %temp210.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %negIncomingLoopMask26.i = xor i1 %Mneg3.i, true
  %temp207.i = insertelement <16 x i1> undef, i1 %negIncomingLoopMask26.i, i32 0
  %vector208.i = shufflevector <16 x i1> %temp207.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %arrayidx39.i = getelementptr inbounds float addrspace(1)* %4, i64 %29
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB1260.i, %thenBB1253.i, %elseBB.i
  %currBarrier.0.i = phi i32 [ 2, %elseBB.i ], [ %currBarrier.4.i, %thenBB1260.i ], [ %currBarrier.1.i, %thenBB1253.i ]
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride1266.i", %thenBB1260.i ], [ %"loadedCurrSB+Stride1259.i", %thenBB1253.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++1264.i", %thenBB1260.i ], [ %"CurrWI++1257.i", %thenBB1253.i ]
  br i1 %Mneg3.i, label %for.body.i, label %for.end.i

for.body.i:                                       ; preds = %postload761.i, %SyncBB.i
  %currBarrier.1.i = phi i32 [ %currBarrier.3.i, %postload761.i ], [ %currBarrier.0.i, %SyncBB.i ]
  %CurrSBIndex..2.i = phi i64 [ %CurrSBIndex..4.i, %postload761.i ], [ %CurrSBIndex..1.i, %SyncBB.i ]
  %CurrWI..2.i = phi i64 [ %CurrWI..4.i, %postload761.i ], [ %CurrWI..1.i, %SyncBB.i ]
  %vectorPHI206.i = phi <16 x i1> [ %loop_mask12292.i, %postload761.i ], [ %vector208.i, %SyncBB.i ]
  %vectorPHI209.i = phi <16 x i1> [ %local_edge17296.i, %postload761.i ], [ %vector211.i, %SyncBB.i ]
  %s.03.i = phi i32 [ %s.0.i, %postload761.i ], [ %s.01.i, %SyncBB.i ]
  %"&(pSB[currWI].offset)1237.i" = add nuw i64 %CurrSBIndex..2.i, 100
  %"&pSB[currWI].offset1238.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1237.i"
  %CastToValueType1239.i = bitcast i8* %"&pSB[currWI].offset1238.i" to i32*
  store i32 %s.03.i, i32* %CastToValueType1239.i, align 4
  %"&(pSB[currWI].offset)1218.i" = add nuw i64 %CurrSBIndex..2.i, 96
  %"&pSB[currWI].offset1219.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1218.i"
  %CastToValueType1220.i = bitcast i8* %"&pSB[currWI].offset1219.i" to <16 x i1>*
  store <16 x i1> %vectorPHI209.i, <16 x i1>* %CastToValueType1220.i, align 16
  %"&(pSB[currWI].offset)1209.i" = add nuw i64 %CurrSBIndex..2.i, 80
  %"&pSB[currWI].offset1210.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1209.i"
  %CastToValueType1211.i = bitcast i8* %"&pSB[currWI].offset1210.i" to <16 x i1>*
  store <16 x i1> %vectorPHI206.i, <16 x i1>* %CastToValueType1211.i, align 16
  %temp212.i = insertelement <16 x i32> undef, i32 %s.03.i, i32 0
  %vector213.i = shufflevector <16 x i32> %temp212.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %"&pSB[currWI].offset957.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..2.i
  %CastToValueType958.i = bitcast i8* %"&pSB[currWI].offset957.i" to <16 x i32>*
  %loadedValue959.i = load <16 x i32>* %CastToValueType958.i, align 64
  %cmp26.i = icmp ult <16 x i32> %loadedValue959.i, %vector213.i
  %for.body_to_if.then216.i = and <16 x i1> %vectorPHI209.i, %cmp26.i
  %extract236.i = extractelement <16 x i1> %for.body_to_if.then216.i, i32 1
  %extract237.i = extractelement <16 x i1> %for.body_to_if.then216.i, i32 2
  %extract238.i = extractelement <16 x i1> %for.body_to_if.then216.i, i32 3
  %extract239.i = extractelement <16 x i1> %for.body_to_if.then216.i, i32 4
  %extract240.i = extractelement <16 x i1> %for.body_to_if.then216.i, i32 5
  %extract241.i = extractelement <16 x i1> %for.body_to_if.then216.i, i32 6
  %extract242.i = extractelement <16 x i1> %for.body_to_if.then216.i, i32 7
  %extract243.i = extractelement <16 x i1> %for.body_to_if.then216.i, i32 8
  %extract244.i = extractelement <16 x i1> %for.body_to_if.then216.i, i32 9
  %extract245.i = extractelement <16 x i1> %for.body_to_if.then216.i, i32 10
  %extract246.i = extractelement <16 x i1> %for.body_to_if.then216.i, i32 11
  %extract247.i = extractelement <16 x i1> %for.body_to_if.then216.i, i32 12
  %extract248.i = extractelement <16 x i1> %for.body_to_if.then216.i, i32 13
  %extract249.i = extractelement <16 x i1> %for.body_to_if.then216.i, i32 14
  %extract250.i = extractelement <16 x i1> %for.body_to_if.then216.i, i32 15
  %extract235.i = extractelement <16 x i1> %for.body_to_if.then216.i, i32 0
  %add28217.i = add <16 x i32> %vector213.i, %loadedValue959.i
  %idxprom29218.i = zext <16 x i32> %add28217.i to <16 x i64>
  %extract220.i = extractelement <16 x i64> %idxprom29218.i, i32 1
  %extract221.i = extractelement <16 x i64> %idxprom29218.i, i32 2
  %extract222.i = extractelement <16 x i64> %idxprom29218.i, i32 3
  %extract223.i = extractelement <16 x i64> %idxprom29218.i, i32 4
  %extract224.i = extractelement <16 x i64> %idxprom29218.i, i32 5
  %extract225.i = extractelement <16 x i64> %idxprom29218.i, i32 6
  %extract226.i = extractelement <16 x i64> %idxprom29218.i, i32 7
  %extract227.i = extractelement <16 x i64> %idxprom29218.i, i32 8
  %extract228.i = extractelement <16 x i64> %idxprom29218.i, i32 9
  %extract229.i = extractelement <16 x i64> %idxprom29218.i, i32 10
  %extract230.i = extractelement <16 x i64> %idxprom29218.i, i32 11
  %extract231.i = extractelement <16 x i64> %idxprom29218.i, i32 12
  %extract232.i = extractelement <16 x i64> %idxprom29218.i, i32 13
  %extract233.i = extractelement <16 x i64> %idxprom29218.i, i32 14
  %extract234.i = extractelement <16 x i64> %idxprom29218.i, i32 15
  %84 = getelementptr inbounds float addrspace(3)* %7, i64 %extract220.i
  %85 = getelementptr inbounds float addrspace(3)* %7, i64 %extract221.i
  %86 = getelementptr inbounds float addrspace(3)* %7, i64 %extract222.i
  %87 = getelementptr inbounds float addrspace(3)* %7, i64 %extract223.i
  %88 = getelementptr inbounds float addrspace(3)* %7, i64 %extract224.i
  %89 = getelementptr inbounds float addrspace(3)* %7, i64 %extract225.i
  %90 = getelementptr inbounds float addrspace(3)* %7, i64 %extract226.i
  %91 = getelementptr inbounds float addrspace(3)* %7, i64 %extract227.i
  %92 = getelementptr inbounds float addrspace(3)* %7, i64 %extract228.i
  %93 = getelementptr inbounds float addrspace(3)* %7, i64 %extract229.i
  %94 = getelementptr inbounds float addrspace(3)* %7, i64 %extract230.i
  %95 = getelementptr inbounds float addrspace(3)* %7, i64 %extract231.i
  %96 = getelementptr inbounds float addrspace(3)* %7, i64 %extract232.i
  %97 = getelementptr inbounds float addrspace(3)* %7, i64 %extract233.i
  %98 = getelementptr inbounds float addrspace(3)* %7, i64 %extract234.i
  br i1 %extract235.i, label %preload715.i, label %postload716.i

preload715.i:                                     ; preds = %for.body.i
  %extract219.i = extractelement <16 x i64> %idxprom29218.i, i32 0
  %99 = getelementptr inbounds float addrspace(3)* %7, i64 %extract219.i
  %masked_load391.i = load float addrspace(3)* %99, align 4
  br label %postload716.i

postload716.i:                                    ; preds = %preload715.i, %for.body.i
  %phi717.i = phi float [ undef, %for.body.i ], [ %masked_load391.i, %preload715.i ]
  br i1 %extract236.i, label %preload718.i, label %postload719.i

preload718.i:                                     ; preds = %postload716.i
  %masked_load392.i = load float addrspace(3)* %84, align 4
  br label %postload719.i

postload719.i:                                    ; preds = %preload718.i, %postload716.i
  %phi720.i = phi float [ undef, %postload716.i ], [ %masked_load392.i, %preload718.i ]
  br i1 %extract237.i, label %preload712.i, label %postload713.i

preload712.i:                                     ; preds = %postload719.i
  %masked_load393.i = load float addrspace(3)* %85, align 4
  br label %postload713.i

postload713.i:                                    ; preds = %preload712.i, %postload719.i
  %phi714.i = phi float [ undef, %postload719.i ], [ %masked_load393.i, %preload712.i ]
  br i1 %extract238.i, label %preload721.i, label %postload722.i

preload721.i:                                     ; preds = %postload713.i
  %masked_load394.i = load float addrspace(3)* %86, align 4
  br label %postload722.i

postload722.i:                                    ; preds = %preload721.i, %postload713.i
  %phi723.i = phi float [ undef, %postload713.i ], [ %masked_load394.i, %preload721.i ]
  br i1 %extract239.i, label %preload724.i, label %postload725.i

preload724.i:                                     ; preds = %postload722.i
  %masked_load395.i = load float addrspace(3)* %87, align 4
  br label %postload725.i

postload725.i:                                    ; preds = %preload724.i, %postload722.i
  %phi726.i = phi float [ undef, %postload722.i ], [ %masked_load395.i, %preload724.i ]
  br i1 %extract240.i, label %preload727.i, label %postload728.i

preload727.i:                                     ; preds = %postload725.i
  %masked_load396.i = load float addrspace(3)* %88, align 4
  br label %postload728.i

postload728.i:                                    ; preds = %preload727.i, %postload725.i
  %phi729.i = phi float [ undef, %postload725.i ], [ %masked_load396.i, %preload727.i ]
  br i1 %extract241.i, label %preload730.i, label %postload731.i

preload730.i:                                     ; preds = %postload728.i
  %masked_load397.i = load float addrspace(3)* %89, align 4
  br label %postload731.i

postload731.i:                                    ; preds = %preload730.i, %postload728.i
  %phi732.i = phi float [ undef, %postload728.i ], [ %masked_load397.i, %preload730.i ]
  br i1 %extract242.i, label %preload733.i, label %postload734.i

preload733.i:                                     ; preds = %postload731.i
  %masked_load398.i = load float addrspace(3)* %90, align 4
  br label %postload734.i

postload734.i:                                    ; preds = %preload733.i, %postload731.i
  %phi735.i = phi float [ undef, %postload731.i ], [ %masked_load398.i, %preload733.i ]
  br i1 %extract243.i, label %preload736.i, label %postload737.i

preload736.i:                                     ; preds = %postload734.i
  %masked_load399.i = load float addrspace(3)* %91, align 4
  br label %postload737.i

postload737.i:                                    ; preds = %preload736.i, %postload734.i
  %phi738.i = phi float [ undef, %postload734.i ], [ %masked_load399.i, %preload736.i ]
  br i1 %extract244.i, label %preload739.i, label %postload740.i

preload739.i:                                     ; preds = %postload737.i
  %masked_load400.i = load float addrspace(3)* %92, align 4
  br label %postload740.i

postload740.i:                                    ; preds = %preload739.i, %postload737.i
  %phi741.i = phi float [ undef, %postload737.i ], [ %masked_load400.i, %preload739.i ]
  br i1 %extract245.i, label %preload742.i, label %postload743.i

preload742.i:                                     ; preds = %postload740.i
  %masked_load401.i = load float addrspace(3)* %93, align 4
  br label %postload743.i

postload743.i:                                    ; preds = %preload742.i, %postload740.i
  %phi744.i = phi float [ undef, %postload740.i ], [ %masked_load401.i, %preload742.i ]
  br i1 %extract246.i, label %preload745.i, label %postload746.i

preload745.i:                                     ; preds = %postload743.i
  %masked_load402.i = load float addrspace(3)* %94, align 4
  br label %postload746.i

postload746.i:                                    ; preds = %preload745.i, %postload743.i
  %phi747.i = phi float [ undef, %postload743.i ], [ %masked_load402.i, %preload745.i ]
  br i1 %extract247.i, label %preload748.i, label %postload749.i

preload748.i:                                     ; preds = %postload746.i
  %masked_load403.i = load float addrspace(3)* %95, align 4
  br label %postload749.i

postload749.i:                                    ; preds = %preload748.i, %postload746.i
  %phi750.i = phi float [ undef, %postload746.i ], [ %masked_load403.i, %preload748.i ]
  br i1 %extract248.i, label %preload751.i, label %postload752.i

preload751.i:                                     ; preds = %postload749.i
  %masked_load404.i = load float addrspace(3)* %96, align 4
  br label %postload752.i

postload752.i:                                    ; preds = %preload751.i, %postload749.i
  %phi753.i = phi float [ undef, %postload749.i ], [ %masked_load404.i, %preload751.i ]
  br i1 %extract249.i, label %preload754.i, label %postload755.i

preload754.i:                                     ; preds = %postload752.i
  %masked_load405.i = load float addrspace(3)* %97, align 4
  br label %postload755.i

postload755.i:                                    ; preds = %preload754.i, %postload752.i
  %phi756.i = phi float [ undef, %postload752.i ], [ %masked_load405.i, %preload754.i ]
  br i1 %extract250.i, label %preload757.i, label %postload758.i

preload757.i:                                     ; preds = %postload755.i
  %masked_load406.i = load float addrspace(3)* %98, align 4
  br label %postload758.i

postload758.i:                                    ; preds = %preload757.i, %postload755.i
  %phi759.i = phi float [ undef, %postload755.i ], [ %masked_load406.i, %preload757.i ]
  %temp.vect252.i = insertelement <16 x float> undef, float %phi717.i, i32 0
  %temp.vect253.i = insertelement <16 x float> %temp.vect252.i, float %phi720.i, i32 1
  %temp.vect254.i = insertelement <16 x float> %temp.vect253.i, float %phi714.i, i32 2
  %temp.vect255.i = insertelement <16 x float> %temp.vect254.i, float %phi723.i, i32 3
  %temp.vect256.i = insertelement <16 x float> %temp.vect255.i, float %phi726.i, i32 4
  %temp.vect257.i = insertelement <16 x float> %temp.vect256.i, float %phi729.i, i32 5
  %temp.vect258.i = insertelement <16 x float> %temp.vect257.i, float %phi732.i, i32 6
  %temp.vect259.i = insertelement <16 x float> %temp.vect258.i, float %phi735.i, i32 7
  %temp.vect260.i = insertelement <16 x float> %temp.vect259.i, float %phi738.i, i32 8
  %temp.vect261.i = insertelement <16 x float> %temp.vect260.i, float %phi741.i, i32 9
  %temp.vect262.i = insertelement <16 x float> %temp.vect261.i, float %phi744.i, i32 10
  %temp.vect263.i = insertelement <16 x float> %temp.vect262.i, float %phi747.i, i32 11
  %temp.vect264.i = insertelement <16 x float> %temp.vect263.i, float %phi750.i, i32 12
  %temp.vect265.i = insertelement <16 x float> %temp.vect264.i, float %phi753.i, i32 13
  %temp.vect266.i = insertelement <16 x float> %temp.vect265.i, float %phi756.i, i32 14
  %temp.vect267.i = insertelement <16 x float> %temp.vect266.i, float %phi759.i, i32 15
  br i1 %extract235.i, label %preload762.i, label %postload763.i

preload762.i:                                     ; preds = %postload758.i
  %"&(pSB[currWI].offset)1199.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset1200.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1199.i"
  %CastToValueType1201.i = bitcast i8* %"&pSB[currWI].offset1200.i" to float addrspace(3)**
  %loadedValue1202.i = load float addrspace(3)** %CastToValueType1201.i, align 8
  %vload409.i = load float addrspace(3)* %loadedValue1202.i, align 4
  br label %postload763.i

postload763.i:                                    ; preds = %preload762.i, %postload758.i
  %phi764.i = phi float [ undef, %postload758.i ], [ %vload409.i, %preload762.i ]
  %vpack.i = insertelement <16 x float> undef, float %phi764.i, i32 0
  br i1 %extract236.i, label %preload765.i, label %postload766.i

preload765.i:                                     ; preds = %postload763.i
  %"&(pSB[currWI].offset)1040.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1041.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1040.i"
  %CastToValueType1042.i = bitcast i8* %"&pSB[currWI].offset1041.i" to i64*
  %loadedValue1043.i = load i64* %CastToValueType1042.i, align 8
  %.sum930.i = add i64 %loadedValue1043.i, 1
  %100 = getelementptr float addrspace(3)* %7, i64 %.sum930.i
  %vload412.i = load float addrspace(3)* %100, align 4
  br label %postload766.i

postload766.i:                                    ; preds = %preload765.i, %postload763.i
  %phi767.i = phi float [ undef, %postload763.i ], [ %vload412.i, %preload765.i ]
  %vpack413.i = insertelement <16 x float> %vpack.i, float %phi767.i, i32 1
  br i1 %extract237.i, label %preload768.i, label %postload769.i

preload768.i:                                     ; preds = %postload766.i
  %"&(pSB[currWI].offset)1045.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1046.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1045.i"
  %CastToValueType1047.i = bitcast i8* %"&pSB[currWI].offset1046.i" to i64*
  %loadedValue1048.i = load i64* %CastToValueType1047.i, align 8
  %.sum929.i = add i64 %loadedValue1048.i, 2
  %101 = getelementptr float addrspace(3)* %7, i64 %.sum929.i
  %vload416.i = load float addrspace(3)* %101, align 4
  br label %postload769.i

postload769.i:                                    ; preds = %preload768.i, %postload766.i
  %phi770.i = phi float [ undef, %postload766.i ], [ %vload416.i, %preload768.i ]
  %vpack417.i = insertelement <16 x float> %vpack413.i, float %phi770.i, i32 2
  br i1 %extract238.i, label %preload703.i, label %postload704.i

preload703.i:                                     ; preds = %postload769.i
  %"&(pSB[currWI].offset)1050.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1051.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1050.i"
  %CastToValueType1052.i = bitcast i8* %"&pSB[currWI].offset1051.i" to i64*
  %loadedValue1053.i = load i64* %CastToValueType1052.i, align 8
  %.sum928.i = add i64 %loadedValue1053.i, 3
  %102 = getelementptr float addrspace(3)* %7, i64 %.sum928.i
  %vload420.i = load float addrspace(3)* %102, align 4
  br label %postload704.i

postload704.i:                                    ; preds = %preload703.i, %postload769.i
  %phi705.i = phi float [ undef, %postload769.i ], [ %vload420.i, %preload703.i ]
  %vpack421.i = insertelement <16 x float> %vpack417.i, float %phi705.i, i32 3
  br i1 %extract239.i, label %preload706.i, label %postload707.i

preload706.i:                                     ; preds = %postload704.i
  %"&(pSB[currWI].offset)1055.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1056.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1055.i"
  %CastToValueType1057.i = bitcast i8* %"&pSB[currWI].offset1056.i" to i64*
  %loadedValue1058.i = load i64* %CastToValueType1057.i, align 8
  %.sum927.i = add i64 %loadedValue1058.i, 4
  %103 = getelementptr float addrspace(3)* %7, i64 %.sum927.i
  %vload424.i = load float addrspace(3)* %103, align 4
  br label %postload707.i

postload707.i:                                    ; preds = %preload706.i, %postload704.i
  %phi708.i = phi float [ undef, %postload704.i ], [ %vload424.i, %preload706.i ]
  %vpack425.i = insertelement <16 x float> %vpack421.i, float %phi708.i, i32 4
  br i1 %extract240.i, label %preload709.i, label %postload710.i

preload709.i:                                     ; preds = %postload707.i
  %"&(pSB[currWI].offset)1060.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1061.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1060.i"
  %CastToValueType1062.i = bitcast i8* %"&pSB[currWI].offset1061.i" to i64*
  %loadedValue1063.i = load i64* %CastToValueType1062.i, align 8
  %.sum926.i = add i64 %loadedValue1063.i, 5
  %104 = getelementptr float addrspace(3)* %7, i64 %.sum926.i
  %vload428.i = load float addrspace(3)* %104, align 4
  br label %postload710.i

postload710.i:                                    ; preds = %preload709.i, %postload707.i
  %phi711.i = phi float [ undef, %postload707.i ], [ %vload428.i, %preload709.i ]
  %vpack429.i = insertelement <16 x float> %vpack425.i, float %phi711.i, i32 5
  br i1 %extract241.i, label %preload676.i, label %postload677.i

preload676.i:                                     ; preds = %postload710.i
  %"&(pSB[currWI].offset)1065.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1066.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1065.i"
  %CastToValueType1067.i = bitcast i8* %"&pSB[currWI].offset1066.i" to i64*
  %loadedValue1068.i = load i64* %CastToValueType1067.i, align 8
  %.sum925.i = add i64 %loadedValue1068.i, 6
  %105 = getelementptr float addrspace(3)* %7, i64 %.sum925.i
  %vload432.i = load float addrspace(3)* %105, align 4
  br label %postload677.i

postload677.i:                                    ; preds = %preload676.i, %postload710.i
  %phi678.i = phi float [ undef, %postload710.i ], [ %vload432.i, %preload676.i ]
  %vpack433.i = insertelement <16 x float> %vpack429.i, float %phi678.i, i32 6
  br i1 %extract242.i, label %preload679.i, label %postload680.i

preload679.i:                                     ; preds = %postload677.i
  %"&(pSB[currWI].offset)1070.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1071.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1070.i"
  %CastToValueType1072.i = bitcast i8* %"&pSB[currWI].offset1071.i" to i64*
  %loadedValue1073.i = load i64* %CastToValueType1072.i, align 8
  %.sum924.i = add i64 %loadedValue1073.i, 7
  %106 = getelementptr float addrspace(3)* %7, i64 %.sum924.i
  %vload436.i = load float addrspace(3)* %106, align 4
  br label %postload680.i

postload680.i:                                    ; preds = %preload679.i, %postload677.i
  %phi681.i = phi float [ undef, %postload677.i ], [ %vload436.i, %preload679.i ]
  %vpack437.i = insertelement <16 x float> %vpack433.i, float %phi681.i, i32 7
  br i1 %extract243.i, label %preload682.i, label %postload683.i

preload682.i:                                     ; preds = %postload680.i
  %"&(pSB[currWI].offset)1075.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1076.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1075.i"
  %CastToValueType1077.i = bitcast i8* %"&pSB[currWI].offset1076.i" to i64*
  %loadedValue1078.i = load i64* %CastToValueType1077.i, align 8
  %.sum923.i = add i64 %loadedValue1078.i, 8
  %107 = getelementptr float addrspace(3)* %7, i64 %.sum923.i
  %vload440.i = load float addrspace(3)* %107, align 4
  br label %postload683.i

postload683.i:                                    ; preds = %preload682.i, %postload680.i
  %phi684.i = phi float [ undef, %postload680.i ], [ %vload440.i, %preload682.i ]
  %vpack441.i = insertelement <16 x float> %vpack437.i, float %phi684.i, i32 8
  br i1 %extract244.i, label %preload685.i, label %postload686.i

preload685.i:                                     ; preds = %postload683.i
  %"&(pSB[currWI].offset)1080.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1081.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1080.i"
  %CastToValueType1082.i = bitcast i8* %"&pSB[currWI].offset1081.i" to i64*
  %loadedValue1083.i = load i64* %CastToValueType1082.i, align 8
  %.sum922.i = add i64 %loadedValue1083.i, 9
  %108 = getelementptr float addrspace(3)* %7, i64 %.sum922.i
  %vload444.i = load float addrspace(3)* %108, align 4
  br label %postload686.i

postload686.i:                                    ; preds = %preload685.i, %postload683.i
  %phi687.i = phi float [ undef, %postload683.i ], [ %vload444.i, %preload685.i ]
  %vpack445.i = insertelement <16 x float> %vpack441.i, float %phi687.i, i32 9
  br i1 %extract245.i, label %preload688.i, label %postload689.i

preload688.i:                                     ; preds = %postload686.i
  %"&(pSB[currWI].offset)1085.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1086.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1085.i"
  %CastToValueType1087.i = bitcast i8* %"&pSB[currWI].offset1086.i" to i64*
  %loadedValue1088.i = load i64* %CastToValueType1087.i, align 8
  %.sum921.i = add i64 %loadedValue1088.i, 10
  %109 = getelementptr float addrspace(3)* %7, i64 %.sum921.i
  %vload448.i = load float addrspace(3)* %109, align 4
  br label %postload689.i

postload689.i:                                    ; preds = %preload688.i, %postload686.i
  %phi690.i = phi float [ undef, %postload686.i ], [ %vload448.i, %preload688.i ]
  %vpack449.i = insertelement <16 x float> %vpack445.i, float %phi690.i, i32 10
  br i1 %extract246.i, label %preload691.i, label %postload692.i

preload691.i:                                     ; preds = %postload689.i
  %"&(pSB[currWI].offset)1090.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1091.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1090.i"
  %CastToValueType1092.i = bitcast i8* %"&pSB[currWI].offset1091.i" to i64*
  %loadedValue1093.i = load i64* %CastToValueType1092.i, align 8
  %.sum920.i = add i64 %loadedValue1093.i, 11
  %110 = getelementptr float addrspace(3)* %7, i64 %.sum920.i
  %vload452.i = load float addrspace(3)* %110, align 4
  br label %postload692.i

postload692.i:                                    ; preds = %preload691.i, %postload689.i
  %phi693.i = phi float [ undef, %postload689.i ], [ %vload452.i, %preload691.i ]
  %vpack453.i = insertelement <16 x float> %vpack449.i, float %phi693.i, i32 11
  br i1 %extract247.i, label %preload694.i, label %postload695.i

preload694.i:                                     ; preds = %postload692.i
  %"&(pSB[currWI].offset)1095.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1096.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1095.i"
  %CastToValueType1097.i = bitcast i8* %"&pSB[currWI].offset1096.i" to i64*
  %loadedValue1098.i = load i64* %CastToValueType1097.i, align 8
  %.sum919.i = add i64 %loadedValue1098.i, 12
  %111 = getelementptr float addrspace(3)* %7, i64 %.sum919.i
  %vload456.i = load float addrspace(3)* %111, align 4
  br label %postload695.i

postload695.i:                                    ; preds = %preload694.i, %postload692.i
  %phi696.i = phi float [ undef, %postload692.i ], [ %vload456.i, %preload694.i ]
  %vpack457.i = insertelement <16 x float> %vpack453.i, float %phi696.i, i32 12
  br i1 %extract248.i, label %preload697.i, label %postload698.i

preload697.i:                                     ; preds = %postload695.i
  %"&(pSB[currWI].offset)1100.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1101.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1100.i"
  %CastToValueType1102.i = bitcast i8* %"&pSB[currWI].offset1101.i" to i64*
  %loadedValue1103.i = load i64* %CastToValueType1102.i, align 8
  %.sum918.i = add i64 %loadedValue1103.i, 13
  %112 = getelementptr float addrspace(3)* %7, i64 %.sum918.i
  %vload460.i = load float addrspace(3)* %112, align 4
  br label %postload698.i

postload698.i:                                    ; preds = %preload697.i, %postload695.i
  %phi699.i = phi float [ undef, %postload695.i ], [ %vload460.i, %preload697.i ]
  %vpack461.i = insertelement <16 x float> %vpack457.i, float %phi699.i, i32 13
  br i1 %extract249.i, label %preload700.i, label %postload701.i

preload700.i:                                     ; preds = %postload698.i
  %"&(pSB[currWI].offset)1105.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1106.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1105.i"
  %CastToValueType1107.i = bitcast i8* %"&pSB[currWI].offset1106.i" to i64*
  %loadedValue1108.i = load i64* %CastToValueType1107.i, align 8
  %.sum917.i = add i64 %loadedValue1108.i, 14
  %113 = getelementptr float addrspace(3)* %7, i64 %.sum917.i
  %vload464.i = load float addrspace(3)* %113, align 4
  br label %postload701.i

postload701.i:                                    ; preds = %preload700.i, %postload698.i
  %phi702.i = phi float [ undef, %postload698.i ], [ %vload464.i, %preload700.i ]
  %vpack465.i = insertelement <16 x float> %vpack461.i, float %phi702.i, i32 14
  br i1 %extract250.i, label %preload851.i, label %postload852.i

preload851.i:                                     ; preds = %postload701.i
  %"&(pSB[currWI].offset)1110.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1111.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1110.i"
  %CastToValueType1112.i = bitcast i8* %"&pSB[currWI].offset1111.i" to i64*
  %loadedValue1113.i = load i64* %CastToValueType1112.i, align 8
  %.sum916.i = add i64 %loadedValue1113.i, 15
  %114 = getelementptr float addrspace(3)* %7, i64 %.sum916.i
  %vload468.i = load float addrspace(3)* %114, align 4
  br label %postload852.i

postload852.i:                                    ; preds = %preload851.i, %postload701.i
  %phi853.i = phi float [ undef, %postload701.i ], [ %vload468.i, %preload851.i ]
  %vpack469.i = insertelement <16 x float> %vpack465.i, float %phi853.i, i32 15
  %add33268.i = fadd <16 x float> %vpack469.i, %temp.vect267.i
  br i1 %extract235.i, label %preload854.i, label %postload855.i

preload854.i:                                     ; preds = %postload852.i
  %exData473.i = extractelement <16 x float> %add33268.i, i32 0
  %"&(pSB[currWI].offset)1204.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset1205.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1204.i"
  %CastToValueType1206.i = bitcast i8* %"&pSB[currWI].offset1205.i" to float addrspace(3)**
  %loadedValue1207.i = load float addrspace(3)** %CastToValueType1206.i, align 8
  store float %exData473.i, float addrspace(3)* %loadedValue1207.i, align 4
  br label %postload855.i

postload855.i:                                    ; preds = %preload854.i, %postload852.i
  br i1 %extract236.i, label %preload857.i, label %postload858.i

preload857.i:                                     ; preds = %postload855.i
  %"&(pSB[currWI].offset)1115.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1116.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1115.i"
  %CastToValueType1117.i = bitcast i8* %"&pSB[currWI].offset1116.i" to i64*
  %loadedValue1118.i = load i64* %CastToValueType1117.i, align 8
  %.sum915.i = add i64 %loadedValue1118.i, 1
  %115 = getelementptr float addrspace(3)* %7, i64 %.sum915.i
  %exData476.i = extractelement <16 x float> %add33268.i, i32 1
  store float %exData476.i, float addrspace(3)* %115, align 4
  br label %postload858.i

postload858.i:                                    ; preds = %preload857.i, %postload855.i
  br i1 %extract237.i, label %preload860.i, label %postload861.i

preload860.i:                                     ; preds = %postload858.i
  %"&(pSB[currWI].offset)1120.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1121.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1120.i"
  %CastToValueType1122.i = bitcast i8* %"&pSB[currWI].offset1121.i" to i64*
  %loadedValue1123.i = load i64* %CastToValueType1122.i, align 8
  %.sum914.i = add i64 %loadedValue1123.i, 2
  %116 = getelementptr float addrspace(3)* %7, i64 %.sum914.i
  %exData479.i = extractelement <16 x float> %add33268.i, i32 2
  store float %exData479.i, float addrspace(3)* %116, align 4
  br label %postload861.i

postload861.i:                                    ; preds = %preload860.i, %postload858.i
  br i1 %extract238.i, label %preload863.i, label %postload864.i

preload863.i:                                     ; preds = %postload861.i
  %"&(pSB[currWI].offset)1125.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1126.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1125.i"
  %CastToValueType1127.i = bitcast i8* %"&pSB[currWI].offset1126.i" to i64*
  %loadedValue1128.i = load i64* %CastToValueType1127.i, align 8
  %.sum913.i = add i64 %loadedValue1128.i, 3
  %117 = getelementptr float addrspace(3)* %7, i64 %.sum913.i
  %exData482.i = extractelement <16 x float> %add33268.i, i32 3
  store float %exData482.i, float addrspace(3)* %117, align 4
  br label %postload864.i

postload864.i:                                    ; preds = %preload863.i, %postload861.i
  br i1 %extract239.i, label %preload866.i, label %postload867.i

preload866.i:                                     ; preds = %postload864.i
  %"&(pSB[currWI].offset)1130.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1131.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1130.i"
  %CastToValueType1132.i = bitcast i8* %"&pSB[currWI].offset1131.i" to i64*
  %loadedValue1133.i = load i64* %CastToValueType1132.i, align 8
  %.sum912.i = add i64 %loadedValue1133.i, 4
  %118 = getelementptr float addrspace(3)* %7, i64 %.sum912.i
  %exData485.i = extractelement <16 x float> %add33268.i, i32 4
  store float %exData485.i, float addrspace(3)* %118, align 4
  br label %postload867.i

postload867.i:                                    ; preds = %preload866.i, %postload864.i
  br i1 %extract240.i, label %preload869.i, label %postload870.i

preload869.i:                                     ; preds = %postload867.i
  %"&(pSB[currWI].offset)1135.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1136.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1135.i"
  %CastToValueType1137.i = bitcast i8* %"&pSB[currWI].offset1136.i" to i64*
  %loadedValue1138.i = load i64* %CastToValueType1137.i, align 8
  %.sum911.i = add i64 %loadedValue1138.i, 5
  %119 = getelementptr float addrspace(3)* %7, i64 %.sum911.i
  %exData488.i = extractelement <16 x float> %add33268.i, i32 5
  store float %exData488.i, float addrspace(3)* %119, align 4
  br label %postload870.i

postload870.i:                                    ; preds = %preload869.i, %postload867.i
  br i1 %extract241.i, label %preload872.i, label %postload873.i

preload872.i:                                     ; preds = %postload870.i
  %"&(pSB[currWI].offset)1140.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1141.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1140.i"
  %CastToValueType1142.i = bitcast i8* %"&pSB[currWI].offset1141.i" to i64*
  %loadedValue1143.i = load i64* %CastToValueType1142.i, align 8
  %.sum910.i = add i64 %loadedValue1143.i, 6
  %120 = getelementptr float addrspace(3)* %7, i64 %.sum910.i
  %exData491.i = extractelement <16 x float> %add33268.i, i32 6
  store float %exData491.i, float addrspace(3)* %120, align 4
  br label %postload873.i

postload873.i:                                    ; preds = %preload872.i, %postload870.i
  br i1 %extract242.i, label %preload875.i, label %postload876.i

preload875.i:                                     ; preds = %postload873.i
  %"&(pSB[currWI].offset)1145.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1146.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1145.i"
  %CastToValueType1147.i = bitcast i8* %"&pSB[currWI].offset1146.i" to i64*
  %loadedValue1148.i = load i64* %CastToValueType1147.i, align 8
  %.sum909.i = add i64 %loadedValue1148.i, 7
  %121 = getelementptr float addrspace(3)* %7, i64 %.sum909.i
  %exData494.i = extractelement <16 x float> %add33268.i, i32 7
  store float %exData494.i, float addrspace(3)* %121, align 4
  br label %postload876.i

postload876.i:                                    ; preds = %preload875.i, %postload873.i
  br i1 %extract243.i, label %preload878.i, label %postload879.i

preload878.i:                                     ; preds = %postload876.i
  %"&(pSB[currWI].offset)1150.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1151.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1150.i"
  %CastToValueType1152.i = bitcast i8* %"&pSB[currWI].offset1151.i" to i64*
  %loadedValue1153.i = load i64* %CastToValueType1152.i, align 8
  %.sum908.i = add i64 %loadedValue1153.i, 8
  %122 = getelementptr float addrspace(3)* %7, i64 %.sum908.i
  %exData497.i = extractelement <16 x float> %add33268.i, i32 8
  store float %exData497.i, float addrspace(3)* %122, align 4
  br label %postload879.i

postload879.i:                                    ; preds = %preload878.i, %postload876.i
  br i1 %extract244.i, label %preload881.i, label %postload882.i

preload881.i:                                     ; preds = %postload879.i
  %"&(pSB[currWI].offset)1155.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1156.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1155.i"
  %CastToValueType1157.i = bitcast i8* %"&pSB[currWI].offset1156.i" to i64*
  %loadedValue1158.i = load i64* %CastToValueType1157.i, align 8
  %.sum907.i = add i64 %loadedValue1158.i, 9
  %123 = getelementptr float addrspace(3)* %7, i64 %.sum907.i
  %exData500.i = extractelement <16 x float> %add33268.i, i32 9
  store float %exData500.i, float addrspace(3)* %123, align 4
  br label %postload882.i

postload882.i:                                    ; preds = %preload881.i, %postload879.i
  br i1 %extract245.i, label %preload884.i, label %postload885.i

preload884.i:                                     ; preds = %postload882.i
  %"&(pSB[currWI].offset)1160.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1161.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1160.i"
  %CastToValueType1162.i = bitcast i8* %"&pSB[currWI].offset1161.i" to i64*
  %loadedValue1163.i = load i64* %CastToValueType1162.i, align 8
  %.sum906.i = add i64 %loadedValue1163.i, 10
  %124 = getelementptr float addrspace(3)* %7, i64 %.sum906.i
  %exData503.i = extractelement <16 x float> %add33268.i, i32 10
  store float %exData503.i, float addrspace(3)* %124, align 4
  br label %postload885.i

postload885.i:                                    ; preds = %preload884.i, %postload882.i
  br i1 %extract246.i, label %preload887.i, label %postload888.i

preload887.i:                                     ; preds = %postload885.i
  %"&(pSB[currWI].offset)1165.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1166.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1165.i"
  %CastToValueType1167.i = bitcast i8* %"&pSB[currWI].offset1166.i" to i64*
  %loadedValue1168.i = load i64* %CastToValueType1167.i, align 8
  %.sum905.i = add i64 %loadedValue1168.i, 11
  %125 = getelementptr float addrspace(3)* %7, i64 %.sum905.i
  %exData506.i = extractelement <16 x float> %add33268.i, i32 11
  store float %exData506.i, float addrspace(3)* %125, align 4
  br label %postload888.i

postload888.i:                                    ; preds = %preload887.i, %postload885.i
  br i1 %extract247.i, label %preload890.i, label %postload891.i

preload890.i:                                     ; preds = %postload888.i
  %"&(pSB[currWI].offset)1170.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1171.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1170.i"
  %CastToValueType1172.i = bitcast i8* %"&pSB[currWI].offset1171.i" to i64*
  %loadedValue1173.i = load i64* %CastToValueType1172.i, align 8
  %.sum904.i = add i64 %loadedValue1173.i, 12
  %126 = getelementptr float addrspace(3)* %7, i64 %.sum904.i
  %exData509.i = extractelement <16 x float> %add33268.i, i32 12
  store float %exData509.i, float addrspace(3)* %126, align 4
  br label %postload891.i

postload891.i:                                    ; preds = %preload890.i, %postload888.i
  br i1 %extract248.i, label %preload893.i, label %postload894.i

preload893.i:                                     ; preds = %postload891.i
  %"&(pSB[currWI].offset)1175.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1176.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1175.i"
  %CastToValueType1177.i = bitcast i8* %"&pSB[currWI].offset1176.i" to i64*
  %loadedValue1178.i = load i64* %CastToValueType1177.i, align 8
  %.sum903.i = add i64 %loadedValue1178.i, 13
  %127 = getelementptr float addrspace(3)* %7, i64 %.sum903.i
  %exData512.i = extractelement <16 x float> %add33268.i, i32 13
  store float %exData512.i, float addrspace(3)* %127, align 4
  br label %postload894.i

postload894.i:                                    ; preds = %preload893.i, %postload891.i
  br i1 %extract249.i, label %preload896.i, label %postload897.i

preload896.i:                                     ; preds = %postload894.i
  %"&(pSB[currWI].offset)1180.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1181.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1180.i"
  %CastToValueType1182.i = bitcast i8* %"&pSB[currWI].offset1181.i" to i64*
  %loadedValue1183.i = load i64* %CastToValueType1182.i, align 8
  %.sum902.i = add i64 %loadedValue1183.i, 14
  %128 = getelementptr float addrspace(3)* %7, i64 %.sum902.i
  %exData515.i = extractelement <16 x float> %add33268.i, i32 14
  store float %exData515.i, float addrspace(3)* %128, align 4
  br label %postload897.i

postload897.i:                                    ; preds = %preload896.i, %postload894.i
  br i1 %extract250.i, label %preload899.i, label %postload897.i.if.end.i_crit_edge

postload897.i.if.end.i_crit_edge:                 ; preds = %postload897.i
  br label %if.end.i

preload899.i:                                     ; preds = %postload897.i
  %"&(pSB[currWI].offset)1185.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset1186.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1185.i"
  %CastToValueType1187.i = bitcast i8* %"&pSB[currWI].offset1186.i" to i64*
  %loadedValue1188.i = load i64* %CastToValueType1187.i, align 8
  %.sum.i = add i64 %loadedValue1188.i, 15
  %129 = getelementptr float addrspace(3)* %7, i64 %.sum.i
  %exData518.i = extractelement <16 x float> %add33268.i, i32 15
  store float %exData518.i, float addrspace(3)* %129, align 4
  br label %if.end.i

if.end.i:                                         ; preds = %postload897.i.if.end.i_crit_edge, %preload899.i
  %loadedValue1225.i = load <16 x i1>* %CastToValueType1220.i, align 16
  %extract272.i = extractelement <16 x i1> %loadedValue1225.i, i32 0
  br i1 %extract272.i, label %preload760.i, label %if.end.i.postload761.i_crit_edge

if.end.i.postload761.i_crit_edge:                 ; preds = %if.end.i
  br label %postload761.i

preload760.i:                                     ; preds = %if.end.i
  %check.WI.iter1256.i = icmp ult i64 %CurrWI..2.i, %22
  br i1 %check.WI.iter1256.i, label %thenBB1253.i, label %preload760.i.postload761.i_crit_edge

preload760.i.postload761.i_crit_edge:             ; preds = %preload760.i
  br label %postload761.i

thenBB1253.i:                                     ; preds = %preload760.i
  %"CurrWI++1257.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride1259.i" = add nuw i64 %CurrSBIndex..2.i, 128
  %cond25.i = icmp eq i32 %currBarrier.1.i, 3
  br i1 %cond25.i, label %thenBB1253.i.postload761.i_crit_edge, label %SyncBB.i

thenBB1253.i.postload761.i_crit_edge:             ; preds = %thenBB1253.i
  br label %postload761.i

postload761.i:                                    ; preds = %thenBB1260.i.postload761.i_crit_edge, %thenBB1253.i.postload761.i_crit_edge, %preload760.i.postload761.i_crit_edge, %if.end.i.postload761.i_crit_edge
  %currBarrier.3.i = phi i32 [ %currBarrier.1.i, %if.end.i.postload761.i_crit_edge ], [ 3, %preload760.i.postload761.i_crit_edge ], [ %currBarrier.1.i, %thenBB1253.i.postload761.i_crit_edge ], [ %currBarrier.4.i, %thenBB1260.i.postload761.i_crit_edge ]
  %CurrSBIndex..4.i = phi i64 [ %CurrSBIndex..2.i, %if.end.i.postload761.i_crit_edge ], [ 0, %preload760.i.postload761.i_crit_edge ], [ %"loadedCurrSB+Stride1259.i", %thenBB1253.i.postload761.i_crit_edge ], [ %"loadedCurrSB+Stride1266.i", %thenBB1260.i.postload761.i_crit_edge ]
  %CurrWI..4.i = phi i64 [ %CurrWI..2.i, %if.end.i.postload761.i_crit_edge ], [ 0, %preload760.i.postload761.i_crit_edge ], [ %"CurrWI++1257.i", %thenBB1253.i.postload761.i_crit_edge ], [ %"CurrWI++1264.i", %thenBB1260.i.postload761.i_crit_edge ]
  %"&(pSB[currWI].offset)1241.i" = add nuw i64 %CurrSBIndex..4.i, 100
  %"&pSB[currWI].offset1242.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1241.i"
  %CastToValueType1243.i = bitcast i8* %"&pSB[currWI].offset1242.i" to i32*
  %loadedValue1244.i = load i32* %CastToValueType1243.i, align 4
  %s.0.i = lshr i32 %loadedValue1244.i, 1
  %cmp24.i = icmp eq i32 %s.0.i, 0
  %temp288.i = insertelement <16 x i1> undef, i1 %cmp24.i, i32 0
  %vector289.i = shufflevector <16 x i1> %temp288.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %notCond8.i = xor i1 %cmp24.i, true
  %temp294.i = insertelement <16 x i1> undef, i1 %notCond8.i, i32 0
  %vector295.i = shufflevector <16 x i1> %temp294.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)1227.i" = add nuw i64 %CurrSBIndex..4.i, 96
  %"&pSB[currWI].offset1228.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1227.i"
  %CastToValueType1229.i = bitcast i8* %"&pSB[currWI].offset1228.i" to <16 x i1>*
  %loadedValue1230.i = load <16 x i1>* %CastToValueType1229.i, align 16
  %who_left_tr9290.i = and <16 x i1> %loadedValue1230.i, %vector289.i
  %"&(pSB[currWI].offset)1213.i" = add nuw i64 %CurrSBIndex..4.i, 80
  %"&pSB[currWI].offset1214.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1213.i"
  %CastToValueType1215.i = bitcast i8* %"&pSB[currWI].offset1214.i" to <16 x i1>*
  %loadedValue1216.i = load <16 x i1>* %CastToValueType1215.i, align 16
  %loop_mask12292.i = or <16 x i1> %loadedValue1216.i, %who_left_tr9290.i
  %ipred.i1.i = bitcast <16 x i1> %loop_mask12292.i to i16
  %val.i2.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i1.i, i16 %ipred.i1.i) nounwind
  %130 = and i32 %val.i2.i, 1
  %res.i3.i = icmp eq i32 %130, 0
  %local_edge17296.i = and <16 x i1> %loadedValue1230.i, %vector295.i
  br i1 %res.i3.i, label %for.body.i, label %for.end.i

for.end.i:                                        ; preds = %postload761.i, %SyncBB.i
  %currBarrier.4.i = phi i32 [ %currBarrier.0.i, %SyncBB.i ], [ %currBarrier.3.i, %postload761.i ]
  %CurrSBIndex..5.i = phi i64 [ %CurrSBIndex..1.i, %SyncBB.i ], [ %CurrSBIndex..4.i, %postload761.i ]
  %CurrWI..5.i = phi i64 [ %CurrWI..1.i, %SyncBB.i ], [ %CurrWI..4.i, %postload761.i ]
  %"&pSB[currWI].offset948.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..5.i
  %CastToValueType949.i = bitcast i8* %"&pSB[currWI].offset948.i" to <16 x i32>*
  %loadedValue.i = load <16 x i32>* %CastToValueType949.i, align 64
  %cmp34.i = icmp eq <16 x i32> %loadedValue.i, zeroinitializer
  %extract298.i = extractelement <16 x i1> %cmp34.i, i32 0
  %extract299.i = extractelement <16 x i1> %cmp34.i, i32 1
  %extract300.i = extractelement <16 x i1> %cmp34.i, i32 2
  %extract301.i = extractelement <16 x i1> %cmp34.i, i32 3
  %extract302.i = extractelement <16 x i1> %cmp34.i, i32 4
  %extract303.i = extractelement <16 x i1> %cmp34.i, i32 5
  %extract304.i = extractelement <16 x i1> %cmp34.i, i32 6
  %extract305.i = extractelement <16 x i1> %cmp34.i, i32 7
  %extract306.i = extractelement <16 x i1> %cmp34.i, i32 8
  %extract307.i = extractelement <16 x i1> %cmp34.i, i32 9
  %extract308.i = extractelement <16 x i1> %cmp34.i, i32 10
  %extract309.i = extractelement <16 x i1> %cmp34.i, i32 11
  %extract310.i = extractelement <16 x i1> %cmp34.i, i32 12
  %extract311.i = extractelement <16 x i1> %cmp34.i, i32 13
  %extract312.i = extractelement <16 x i1> %cmp34.i, i32 14
  %extract313.i = extractelement <16 x i1> %cmp34.i, i32 15
  br i1 %extract298.i, label %preload771.i, label %postload772.i

preload771.i:                                     ; preds = %for.end.i
  %masked_load519.i = load float addrspace(3)* %7, align 4
  br label %postload772.i

postload772.i:                                    ; preds = %preload771.i, %for.end.i
  %phi773.i = phi float [ undef, %for.end.i ], [ %masked_load519.i, %preload771.i ]
  br i1 %extract299.i, label %preload776.i, label %postload777.i

preload776.i:                                     ; preds = %postload772.i
  %masked_load520.i = load float addrspace(3)* %7, align 4
  br label %postload777.i

postload777.i:                                    ; preds = %preload776.i, %postload772.i
  %phi778.i = phi float [ undef, %postload772.i ], [ %masked_load520.i, %preload776.i ]
  br i1 %extract300.i, label %preload781.i, label %postload782.i

preload781.i:                                     ; preds = %postload777.i
  %masked_load521.i = load float addrspace(3)* %7, align 4
  br label %postload782.i

postload782.i:                                    ; preds = %preload781.i, %postload777.i
  %phi783.i = phi float [ undef, %postload777.i ], [ %masked_load521.i, %preload781.i ]
  br i1 %extract301.i, label %preload786.i, label %postload787.i

preload786.i:                                     ; preds = %postload782.i
  %masked_load522.i = load float addrspace(3)* %7, align 4
  br label %postload787.i

postload787.i:                                    ; preds = %preload786.i, %postload782.i
  %phi788.i = phi float [ undef, %postload782.i ], [ %masked_load522.i, %preload786.i ]
  br i1 %extract302.i, label %preload791.i, label %postload792.i

preload791.i:                                     ; preds = %postload787.i
  %masked_load523.i = load float addrspace(3)* %7, align 4
  br label %postload792.i

postload792.i:                                    ; preds = %preload791.i, %postload787.i
  %phi793.i = phi float [ undef, %postload787.i ], [ %masked_load523.i, %preload791.i ]
  br i1 %extract303.i, label %preload796.i, label %postload797.i

preload796.i:                                     ; preds = %postload792.i
  %masked_load524.i = load float addrspace(3)* %7, align 4
  br label %postload797.i

postload797.i:                                    ; preds = %preload796.i, %postload792.i
  %phi798.i = phi float [ undef, %postload792.i ], [ %masked_load524.i, %preload796.i ]
  br i1 %extract304.i, label %preload801.i, label %postload802.i

preload801.i:                                     ; preds = %postload797.i
  %masked_load525.i = load float addrspace(3)* %7, align 4
  br label %postload802.i

postload802.i:                                    ; preds = %preload801.i, %postload797.i
  %phi803.i = phi float [ undef, %postload797.i ], [ %masked_load525.i, %preload801.i ]
  br i1 %extract305.i, label %preload806.i, label %postload807.i

preload806.i:                                     ; preds = %postload802.i
  %masked_load526.i = load float addrspace(3)* %7, align 4
  br label %postload807.i

postload807.i:                                    ; preds = %preload806.i, %postload802.i
  %phi808.i = phi float [ undef, %postload802.i ], [ %masked_load526.i, %preload806.i ]
  br i1 %extract306.i, label %preload811.i, label %postload812.i

preload811.i:                                     ; preds = %postload807.i
  %masked_load527.i = load float addrspace(3)* %7, align 4
  br label %postload812.i

postload812.i:                                    ; preds = %preload811.i, %postload807.i
  %phi813.i = phi float [ undef, %postload807.i ], [ %masked_load527.i, %preload811.i ]
  br i1 %extract307.i, label %preload816.i, label %postload817.i

preload816.i:                                     ; preds = %postload812.i
  %masked_load528.i = load float addrspace(3)* %7, align 4
  br label %postload817.i

postload817.i:                                    ; preds = %preload816.i, %postload812.i
  %phi818.i = phi float [ undef, %postload812.i ], [ %masked_load528.i, %preload816.i ]
  br i1 %extract308.i, label %preload821.i, label %postload822.i

preload821.i:                                     ; preds = %postload817.i
  %masked_load529.i = load float addrspace(3)* %7, align 4
  br label %postload822.i

postload822.i:                                    ; preds = %preload821.i, %postload817.i
  %phi823.i = phi float [ undef, %postload817.i ], [ %masked_load529.i, %preload821.i ]
  br i1 %extract309.i, label %preload826.i, label %postload827.i

preload826.i:                                     ; preds = %postload822.i
  %masked_load530.i = load float addrspace(3)* %7, align 4
  br label %postload827.i

postload827.i:                                    ; preds = %preload826.i, %postload822.i
  %phi828.i = phi float [ undef, %postload822.i ], [ %masked_load530.i, %preload826.i ]
  br i1 %extract310.i, label %preload831.i, label %postload832.i

preload831.i:                                     ; preds = %postload827.i
  %masked_load531.i = load float addrspace(3)* %7, align 4
  br label %postload832.i

postload832.i:                                    ; preds = %preload831.i, %postload827.i
  %phi833.i = phi float [ undef, %postload827.i ], [ %masked_load531.i, %preload831.i ]
  br i1 %extract311.i, label %preload836.i, label %postload837.i

preload836.i:                                     ; preds = %postload832.i
  %masked_load532.i = load float addrspace(3)* %7, align 4
  br label %postload837.i

postload837.i:                                    ; preds = %preload836.i, %postload832.i
  %phi838.i = phi float [ undef, %postload832.i ], [ %masked_load532.i, %preload836.i ]
  br i1 %extract312.i, label %preload841.i, label %postload842.i

preload841.i:                                     ; preds = %postload837.i
  %masked_load533.i = load float addrspace(3)* %7, align 4
  br label %postload842.i

postload842.i:                                    ; preds = %preload841.i, %postload837.i
  %phi843.i = phi float [ undef, %postload837.i ], [ %masked_load533.i, %preload841.i ]
  br i1 %extract313.i, label %preload846.i, label %postload847.i

preload846.i:                                     ; preds = %postload842.i
  %masked_load534.i = load float addrspace(3)* %7, align 4
  br label %postload847.i

postload847.i:                                    ; preds = %preload846.i, %postload842.i
  %phi848.i = phi float [ undef, %postload842.i ], [ %masked_load534.i, %preload846.i ]
  br i1 %extract298.i, label %preload774.i, label %postload775.i

preload774.i:                                     ; preds = %postload847.i
  store float %phi773.i, float addrspace(1)* %arrayidx39.i, align 4
  br label %postload775.i

postload775.i:                                    ; preds = %preload774.i, %postload847.i
  br i1 %extract299.i, label %preload779.i, label %postload780.i

preload779.i:                                     ; preds = %postload775.i
  store float %phi778.i, float addrspace(1)* %arrayidx39.i, align 4
  br label %postload780.i

postload780.i:                                    ; preds = %preload779.i, %postload775.i
  br i1 %extract300.i, label %preload784.i, label %postload785.i

preload784.i:                                     ; preds = %postload780.i
  store float %phi783.i, float addrspace(1)* %arrayidx39.i, align 4
  br label %postload785.i

postload785.i:                                    ; preds = %preload784.i, %postload780.i
  br i1 %extract301.i, label %preload789.i, label %postload790.i

preload789.i:                                     ; preds = %postload785.i
  store float %phi788.i, float addrspace(1)* %arrayidx39.i, align 4
  br label %postload790.i

postload790.i:                                    ; preds = %preload789.i, %postload785.i
  br i1 %extract302.i, label %preload794.i, label %postload795.i

preload794.i:                                     ; preds = %postload790.i
  store float %phi793.i, float addrspace(1)* %arrayidx39.i, align 4
  br label %postload795.i

postload795.i:                                    ; preds = %preload794.i, %postload790.i
  br i1 %extract303.i, label %preload799.i, label %postload800.i

preload799.i:                                     ; preds = %postload795.i
  store float %phi798.i, float addrspace(1)* %arrayidx39.i, align 4
  br label %postload800.i

postload800.i:                                    ; preds = %preload799.i, %postload795.i
  br i1 %extract304.i, label %preload804.i, label %postload805.i

preload804.i:                                     ; preds = %postload800.i
  store float %phi803.i, float addrspace(1)* %arrayidx39.i, align 4
  br label %postload805.i

postload805.i:                                    ; preds = %preload804.i, %postload800.i
  br i1 %extract305.i, label %preload809.i, label %postload810.i

preload809.i:                                     ; preds = %postload805.i
  store float %phi808.i, float addrspace(1)* %arrayidx39.i, align 4
  br label %postload810.i

postload810.i:                                    ; preds = %preload809.i, %postload805.i
  br i1 %extract306.i, label %preload814.i, label %postload815.i

preload814.i:                                     ; preds = %postload810.i
  store float %phi813.i, float addrspace(1)* %arrayidx39.i, align 4
  br label %postload815.i

postload815.i:                                    ; preds = %preload814.i, %postload810.i
  br i1 %extract307.i, label %preload819.i, label %postload820.i

preload819.i:                                     ; preds = %postload815.i
  store float %phi818.i, float addrspace(1)* %arrayidx39.i, align 4
  br label %postload820.i

postload820.i:                                    ; preds = %preload819.i, %postload815.i
  br i1 %extract308.i, label %preload824.i, label %postload825.i

preload824.i:                                     ; preds = %postload820.i
  store float %phi823.i, float addrspace(1)* %arrayidx39.i, align 4
  br label %postload825.i

postload825.i:                                    ; preds = %preload824.i, %postload820.i
  br i1 %extract309.i, label %preload829.i, label %postload830.i

preload829.i:                                     ; preds = %postload825.i
  store float %phi828.i, float addrspace(1)* %arrayidx39.i, align 4
  br label %postload830.i

postload830.i:                                    ; preds = %preload829.i, %postload825.i
  br i1 %extract310.i, label %preload834.i, label %postload835.i

preload834.i:                                     ; preds = %postload830.i
  store float %phi833.i, float addrspace(1)* %arrayidx39.i, align 4
  br label %postload835.i

postload835.i:                                    ; preds = %preload834.i, %postload830.i
  br i1 %extract311.i, label %preload839.i, label %postload840.i

preload839.i:                                     ; preds = %postload835.i
  store float %phi838.i, float addrspace(1)* %arrayidx39.i, align 4
  br label %postload840.i

postload840.i:                                    ; preds = %preload839.i, %postload835.i
  br i1 %extract312.i, label %preload844.i, label %postload845.i

preload844.i:                                     ; preds = %postload840.i
  store float %phi843.i, float addrspace(1)* %arrayidx39.i, align 4
  br label %postload845.i

postload845.i:                                    ; preds = %preload844.i, %postload840.i
  br i1 %extract313.i, label %preload849.i, label %if.end40.i

preload849.i:                                     ; preds = %postload845.i
  store float %phi848.i, float addrspace(1)* %arrayidx39.i, align 4
  br label %if.end40.i

if.end40.i:                                       ; preds = %preload849.i, %postload845.i
  %check.WI.iter1263.i = icmp ult i64 %CurrWI..5.i, %22
  br i1 %check.WI.iter1263.i, label %thenBB1260.i, label %____Vectorized_.reduce_separated_args.exit

thenBB1260.i:                                     ; preds = %if.end40.i
  %"CurrWI++1264.i" = add nuw i64 %CurrWI..5.i, 1
  %"loadedCurrSB+Stride1266.i" = add nuw i64 %CurrSBIndex..5.i, 128
  %cond.i = icmp eq i32 %currBarrier.4.i, 3
  br i1 %cond.i, label %thenBB1260.i.postload761.i_crit_edge, label %SyncBB.i

thenBB1260.i.postload761.i_crit_edge:             ; preds = %thenBB1260.i
  br label %postload761.i

____Vectorized_.reduce_separated_args.exit:       ; preds = %if.end40.i
  ret void
}

!opencl.kernels = !{!0, !2}
!opencl.build.options = !{!4}
!cl.noBarrierPath.kernels = !{!5}
!opencl.wrappers = !{!6, !7}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(3)*, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__reduce_separated_args, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3}
!2 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__reduceNoLocal_separated_args, metadata !3}
!3 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3}
!4 = metadata !{}
!5 = metadata !{metadata !"reduceNoLocal"}
!6 = metadata !{void (i8*)* @reduce}
!7 = metadata !{void (i8*)* @reduceNoLocal}
