; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@bottom_scan.s_seed = internal addrspace(3) unnamed_addr global float 0.000000e+00, align 4

declare void @__reduce_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, float addrspace(3)* nocapture) nounwind

declare i64 @get_num_groups(i32) nounwind readnone

declare i64 @get_group_id(i32) nounwind readnone

declare i64 @get_local_id(i32) nounwind readnone

declare i64 @get_local_size(i32) nounwind readnone

declare void @barrier(i64)

declare void @__top_scan_original(float addrspace(1)* nocapture, i32, float addrspace(3)* nocapture) nounwind

declare void @__bottom_scan_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, float addrspace(3)* nocapture) nounwind

declare void @____Vectorized_.reduce_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, float addrspace(3)* nocapture) nounwind

declare void @____Vectorized_.top_scan_original(float addrspace(1)* nocapture, i32, float addrspace(3)* nocapture) nounwind

declare void @____Vectorized_.bottom_scan_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, float addrspace(3)* nocapture) nounwind

declare i1 @allOne(i1)

declare i1 @allZero(i1)

declare float @masked_load_align4_0(i1, float addrspace(1)*)

declare float @masked_load_align4_1(i1, float addrspace(3)*)

declare float @masked_load_align4_2(i1, float addrspace(3)*)

declare void @masked_store_align4_0(i1, float, float addrspace(3)*)

declare void @maskedf_0_barrier(i1, i64)

declare float @masked_load_align4_3(i1, float addrspace(3)*)

declare void @masked_store_align4_1(i1, float, float addrspace(1)*)

declare i1 @allZero_v16(<16 x i1>)

declare i1 @allOne_v16(<16 x i1>)

declare <16 x float> @masked_load_align4_6(<16 x i1>, <16 x float> addrspace(3)*)

declare void @masked_store_align4_2(<16 x i1>, <16 x float>, <16 x float> addrspace(3)*)

declare float @masked_load_align4_8(i1, float addrspace(1)*)

declare float @masked_load_align4_9(i1, float addrspace(3)*)

declare void @maskedf_1_barrier(i1, i64)

declare float @masked_load_align4_10(i1, float addrspace(3)*)

declare void @masked_store_align4_4(i1, float, float addrspace(3)*)

declare void @maskedf_2_barrier(i1, i64)

declare float @masked_load_align4_11(i1, float addrspace(3)*)

declare void @masked_store_align4_5(i1, float, float addrspace(1)*)

declare <16 x float> @masked_load_align4_12(<16 x i1>, <16 x float> addrspace(1)*)

declare <16 x float> @masked_load_align4_13(i1, <16 x float> addrspace(3)*)

declare <16 x float> @masked_load_align4_14(i1, <16 x float> addrspace(3)*)

declare void @masked_store_align4_6(i1, <16 x float>, <16 x float> addrspace(3)*)

declare <16 x float> @masked_load_align4_15(<16 x i1>, <16 x float> addrspace(3)*)

declare void @masked_store_align4_7(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare float @masked_load_align4_16(i1, float addrspace(1)*)

declare i64 @maskedf_3_get_local_size(i1, i32)

declare <4 x float> @masked_load_align16_17(i1, <4 x float> addrspace(1)*)

declare i64 @maskedf_4_get_local_id(i1, i32)

declare void @masked_store_align4_8(i1, float, float addrspace(3)*)

declare i64 @maskedf_5_get_local_size(i1, i32)

declare void @masked_store_align4_9(i1, float, float addrspace(3)*)

declare void @maskedf_6_barrier(i1, i64)

declare float @masked_load_align4_18(i1, float addrspace(3)*)

declare void @maskedf_7_barrier(i1, i64)

declare float @masked_load_align4_19(i1, float addrspace(3)*)

declare void @masked_store_align4_10(i1, float, float addrspace(3)*)

declare void @maskedf_8_barrier(i1, i64)

declare float @masked_load_align4_20(i1, float addrspace(3)*)

declare void @masked_store_align16_11(i1, <4 x float>, <4 x float> addrspace(1)*)

declare void @masked_store_align4_12(i1, float, float addrspace(3)*)

declare void @maskedf_9_barrier(i1, i64)

declare float @masked_load_align4_21(i1, float addrspace(3)*)

declare void @masked_store_align4_13(<16 x i1>, <16 x float>, <16 x float> addrspace(3)*)

declare void @dummybarrier.()

declare i8* @get_special_buffer.() nounwind readnone

declare i64 @get_iter_count.() nounwind readnone

declare i64 @get_new_local_id.(i32, i64) nounwind readnone

declare void @__reduce_separated_args(float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, float addrspace(3)* nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare void @__top_scan_separated_args(float addrspace(1)* nocapture, i32, float addrspace(3)* nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare void @__bottom_scan_separated_args(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, float addrspace(3)* nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare void @____Vectorized_.reduce_separated_args(float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, float addrspace(3)* nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare void @____Vectorized_.top_scan_separated_args(float addrspace(1)* nocapture, i32, float addrspace(3)* nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare void @____Vectorized_.bottom_scan_separated_args(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, float addrspace(3)* nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare i32 @llvm.x86.mic.kortestc(i16, i16) nounwind readnone

declare i32 @llvm.x86.mic.kortestz(i16, i16) nounwind readnone

define void @reduce(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to float addrspace(3)**
  %10 = load float addrspace(3)** %9, align 8
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
  %div.i = sdiv i32 %7, 4
  %conv.i = sext i32 %div.i to i64
  br label %SyncBB64.i

SyncBB64.i:                                       ; preds = %thenBB.i, %entry
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %26 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %13, i64 0, i32 4, i64 0
  %27 = load i64* %26, align 8
  %isDivisorZero7.i = icmp eq i64 %27, 0
  %newiDvisor9.i = select i1 %isDivisorZero7.i, i64 1, i64 %27
  %div1.i = udiv i64 %conv.i, %newiDvisor9.i
  %mul.i = shl i64 %div1.i, 2
  %conv2.i = trunc i64 %mul.i to i32
  %28 = load i64* %16, align 8
  %conv4.i = sext i32 %conv2.i to i64
  %mul5.i = mul i64 %conv4.i, %28
  %conv6.i = trunc i64 %mul5.i to i32
  %sub.i = add i64 %27, -1
  %cmp.i = icmp eq i64 %28, %sub.i
  %add.i = add nsw i32 %conv6.i, %conv2.i
  %cond.i = select i1 %cmp.i, i32 %7, i32 %add.i
  %29 = getelementptr <{ [4 x i64] }>* %19, i64 %CurrWI..0.i, i32 0, i64 0
  %30 = load i64* %29, align 8
  %conv11.i = trunc i64 %30 to i32
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %conv11.i, i32* %CastToValueType.i, align 4
  %add12.i = add nsw i32 %conv6.i, %conv11.i
  %cmp134.i = icmp slt i32 %add12.i, %cond.i
  %31 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %13, i64 0, i32 3, i64 0
  %32 = load i64* %31, align 8
  br i1 %cmp134.i, label %while.body.i, label %while.end.i

while.body.i:                                     ; preds = %while.body.i, %SyncBB64.i
  %sum.06.i = phi float [ %add15.i, %while.body.i ], [ 0.000000e+00, %SyncBB64.i ]
  %i.05.i = phi i32 [ %conv19.i, %while.body.i ], [ %add12.i, %SyncBB64.i ]
  %idxprom.i = sext i32 %i.05.i to i64
  %arrayidx.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom.i
  %33 = load float addrspace(1)* %arrayidx.i, align 4
  %add15.i = fadd float %sum.06.i, %33
  %conv171.i = zext i32 %i.05.i to i64
  %add18.i = add i64 %32, %conv171.i
  %conv19.i = trunc i64 %add18.i to i32
  %cmp13.i = icmp slt i32 %conv19.i, %cond.i
  br i1 %cmp13.i, label %while.body.i, label %while.end.i

while.end.i:                                      ; preds = %while.body.i, %SyncBB64.i
  %sum.0.lcssa.i = phi float [ 0.000000e+00, %SyncBB64.i ], [ %add15.i, %while.body.i ]
  %idxprom20.i = sext i32 %conv11.i to i64
  %arrayidx21.i = getelementptr inbounds float addrspace(3)* %10, i64 %idxprom20.i
  %"&(pSB[currWI].offset)30.i" = add nuw i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset31.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)30.i"
  %CastToValueType32.i = bitcast i8* %"&pSB[currWI].offset31.i" to float addrspace(3)**
  store float addrspace(3)* %arrayidx21.i, float addrspace(3)** %CastToValueType32.i, align 8
  store float %sum.0.lcssa.i, float addrspace(3)* %arrayidx21.i, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %22
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %while.end.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 24
  br label %SyncBB64.i

elseBB.i:                                         ; preds = %while.end.i
  %div23.i = lshr i64 %32, 1
  %conv24.i = trunc i64 %div23.i to i32
  %cmp252.i = icmp eq i32 %conv24.i, 0
  %arrayidx40.i = getelementptr inbounds float addrspace(1)* %4, i64 %28
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB73.i, %thenBB66.i, %elseBB.i
  %currBarrier.0.i = phi i32 [ 0, %elseBB.i ], [ %currBarrier.3.i, %thenBB73.i ], [ %currBarrier.1.i, %thenBB66.i ]
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride79.i", %thenBB73.i ], [ %"loadedCurrSB+Stride72.i", %thenBB66.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++77.i", %thenBB73.i ], [ %"CurrWI++70.i", %thenBB66.i ]
  br i1 %cmp252.i, label %for.end.i, label %for.body.i

for.body.i:                                       ; preds = %SyncBB62.i, %SyncBB.i
  %currBarrier.1.i = phi i32 [ %currBarrier.2.i, %SyncBB62.i ], [ %currBarrier.0.i, %SyncBB.i ]
  %CurrSBIndex..2.i = phi i64 [ %CurrSBIndex..3.i, %SyncBB62.i ], [ %CurrSBIndex..1.i, %SyncBB.i ]
  %CurrWI..2.i = phi i64 [ %CurrWI..3.i, %SyncBB62.i ], [ %CurrWI..1.i, %SyncBB.i ]
  %s.03.i = phi i32 [ %shr.i, %SyncBB62.i ], [ %conv24.i, %SyncBB.i ]
  %"&(pSB[currWI].offset)44.i" = add nuw i64 %CurrSBIndex..2.i, 16
  %"&pSB[currWI].offset45.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)44.i"
  %CastToValueType46.i = bitcast i8* %"&pSB[currWI].offset45.i" to i32*
  store i32 %s.03.i, i32* %CastToValueType46.i, align 4
  %"&pSB[currWI].offset21.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..2.i
  %CastToValueType22.i = bitcast i8* %"&pSB[currWI].offset21.i" to i32*
  %loadedValue23.i = load i32* %CastToValueType22.i, align 4
  %cmp27.i = icmp ult i32 %loadedValue23.i, %s.03.i
  br i1 %cmp27.i, label %if.then.i, label %if.end.i

if.then.i:                                        ; preds = %for.body.i
  %add29.i = add i32 %s.03.i, %loadedValue23.i
  %idxprom30.i = zext i32 %add29.i to i64
  %arrayidx31.i = getelementptr inbounds float addrspace(3)* %10, i64 %idxprom30.i
  %34 = load float addrspace(3)* %arrayidx31.i, align 4
  %"&(pSB[currWI].offset)34.i" = add nuw i64 %CurrSBIndex..2.i, 8
  %"&pSB[currWI].offset35.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)34.i"
  %CastToValueType36.i = bitcast i8* %"&pSB[currWI].offset35.i" to float addrspace(3)**
  %loadedValue37.i = load float addrspace(3)** %CastToValueType36.i, align 8
  %35 = load float addrspace(3)* %loadedValue37.i, align 4
  %add34.i = fadd float %35, %34
  store float %add34.i, float addrspace(3)* %loadedValue37.i, align 4
  br label %if.end.i

if.end.i:                                         ; preds = %if.then.i, %for.body.i
  %check.WI.iter69.i = icmp ult i64 %CurrWI..2.i, %22
  br i1 %check.WI.iter69.i, label %thenBB66.i, label %SyncBB62.i

thenBB66.i:                                       ; preds = %if.end.i
  %"CurrWI++70.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride72.i" = add nuw i64 %CurrSBIndex..2.i, 24
  %cond2.i = icmp eq i32 %currBarrier.1.i, 0
  br i1 %cond2.i, label %SyncBB.i, label %SyncBB62.i

SyncBB62.i:                                       ; preds = %thenBB73.i, %thenBB66.i, %if.end.i
  %currBarrier.2.i = phi i32 [ %currBarrier.3.i, %thenBB73.i ], [ %currBarrier.1.i, %thenBB66.i ], [ 1, %if.end.i ]
  %CurrSBIndex..3.i = phi i64 [ %"loadedCurrSB+Stride79.i", %thenBB73.i ], [ %"loadedCurrSB+Stride72.i", %thenBB66.i ], [ 0, %if.end.i ]
  %CurrWI..3.i = phi i64 [ %"CurrWI++77.i", %thenBB73.i ], [ %"CurrWI++70.i", %thenBB66.i ], [ 0, %if.end.i ]
  %"&(pSB[currWI].offset)48.i" = add nuw i64 %CurrSBIndex..3.i, 16
  %"&pSB[currWI].offset49.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)48.i"
  %CastToValueType50.i = bitcast i8* %"&pSB[currWI].offset49.i" to i32*
  %loadedValue51.i = load i32* %CastToValueType50.i, align 4
  %shr.i = lshr i32 %loadedValue51.i, 1
  %cmp25.i = icmp eq i32 %shr.i, 0
  br i1 %cmp25.i, label %for.end.i, label %for.body.i

for.end.i:                                        ; preds = %SyncBB62.i, %SyncBB.i
  %currBarrier.3.i = phi i32 [ %currBarrier.0.i, %SyncBB.i ], [ %currBarrier.2.i, %SyncBB62.i ]
  %CurrSBIndex..4.i = phi i64 [ %CurrSBIndex..1.i, %SyncBB.i ], [ %CurrSBIndex..3.i, %SyncBB62.i ]
  %CurrWI..4.i = phi i64 [ %CurrWI..1.i, %SyncBB.i ], [ %CurrWI..3.i, %SyncBB62.i ]
  %"&pSB[currWI].offset26.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..4.i
  %CastToValueType27.i = bitcast i8* %"&pSB[currWI].offset26.i" to i32*
  %loadedValue28.i = load i32* %CastToValueType27.i, align 4
  %cmp35.i = icmp eq i32 %loadedValue28.i, 0
  br i1 %cmp35.i, label %if.then37.i, label %if.end41.i

if.then37.i:                                      ; preds = %for.end.i
  %36 = load float addrspace(3)* %10, align 4
  store float %36, float addrspace(1)* %arrayidx40.i, align 4
  br label %if.end41.i

if.end41.i:                                       ; preds = %if.then37.i, %for.end.i
  %check.WI.iter76.i = icmp ult i64 %CurrWI..4.i, %22
  br i1 %check.WI.iter76.i, label %thenBB73.i, label %__reduce_separated_args.exit

thenBB73.i:                                       ; preds = %if.end41.i
  %"CurrWI++77.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride79.i" = add nuw i64 %CurrSBIndex..4.i, 24
  %cond1.i = icmp eq i32 %currBarrier.3.i, 1
  br i1 %cond1.i, label %SyncBB62.i, label %SyncBB.i

__reduce_separated_args.exit:                     ; preds = %if.end41.i
  ret void
}

define void @bottom_scan(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float addrspace(1)**
  %7 = load float addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to float addrspace(3)**
  %13 = load float addrspace(3)** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to i8 addrspace(3)**
  %16 = load i8 addrspace(3)** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 48
  %18 = bitcast i8* %17 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %19 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 56
  %21 = bitcast i8* %20 to i64**
  %22 = load i64** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 72
  %24 = bitcast i8* %23 to <{ [4 x i64] }>**
  %25 = load <{ [4 x i64] }>** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 88
  %27 = bitcast i8* %26 to i64*
  %28 = load i64* %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 96
  %30 = bitcast i8* %29 to i8**
  %31 = load i8** %30, align 8
  %32 = bitcast i8 addrspace(3)* %16 to float addrspace(3)*
  %33 = bitcast float addrspace(1)* %1 to <4 x float> addrspace(1)*
  %34 = bitcast float addrspace(1)* %7 to <4 x float> addrspace(1)*
  %div.i = sdiv i32 %10, 4
  %conv.i = sext i32 %div.i to i64
  br label %SyncBB183.i

SyncBB183.i:                                      ; preds = %thenBB209.i, %thenBB186.i, %entry
  %call38174.0.i = phi i64 [ undef, %entry ], [ %call38174.3.i, %thenBB209.i ], [ %call38174.1.i, %thenBB186.i ]
  %currBarrier.0.i = phi i32 [ 28, %entry ], [ %currBarrier.7.i, %thenBB209.i ], [ %currBarrier.1.i, %thenBB186.i ]
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride215.i", %thenBB209.i ], [ %"loadedCurrSB+Stride192.i", %thenBB186.i ]
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++213.i", %thenBB209.i ], [ %"CurrWI++190.i", %thenBB186.i ]
  %35 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 4, i64 0
  %36 = load i64* %35, align 8
  %isDivisorZero8.i = icmp eq i64 %36, 0
  %newiDvisor10.i = select i1 %isDivisorZero8.i, i64 1, i64 %36
  %div1.i = udiv i64 %conv.i, %newiDvisor10.i
  %conv2.i = trunc i64 %div1.i to i32
  %37 = load i64* %22, align 8
  %conv4.i = sext i32 %conv2.i to i64
  %mul.i = mul i64 %conv4.i, %37
  %conv5.i = trunc i64 %mul.i to i32
  %sub.i = add i64 %36, -1
  %cmp.i = icmp eq i64 %37, %sub.i
  %add.i = add nsw i32 %conv5.i, %conv2.i
  %cond.i = select i1 %cmp.i, i32 %div.i, i32 %add.i
  %38 = getelementptr <{ [4 x i64] }>* %25, i64 %CurrWI..0.i, i32 0, i64 0
  %39 = load i64* %38, align 8
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %31, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i64*
  store i64 %39, i64* %CastToValueType.i, align 8
  %cmp144.i = icmp slt i32 %conv5.i, %cond.i
  br i1 %cmp144.i, label %while.body.lr.ph.i, label %while.end.i

while.body.lr.ph.i:                               ; preds = %SyncBB183.i
  %arrayidx.i = getelementptr inbounds float addrspace(1)* %4, i64 %37
  %add11.i = add i64 %mul.i, %39
  %40 = load float addrspace(1)* %arrayidx.i, align 4
  %41 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %42 = load i64* %41, align 8
  %sub39.i = add i64 %42, -1
  %cmp40.i = icmp eq i64 %39, %sub39.i
  %"&(pSB[currWI].offset)33.i" = add nuw i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset34.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)33.i"
  %CastToValueType35.i = bitcast i8* %"&pSB[currWI].offset34.i" to i1*
  store i1 %cmp40.i, i1* %CastToValueType35.i, align 1
  br label %while.body.i

while.body.i:                                     ; preds = %SyncBB.i, %while.body.lr.ph.i
  %call38174.1.i = phi i64 [ %42, %while.body.lr.ph.i ], [ %call38174.2.i, %SyncBB.i ]
  %currBarrier.1.i = phi i32 [ %currBarrier.0.i, %while.body.lr.ph.i ], [ %currBarrier.6.i, %SyncBB.i ]
  %CurrSBIndex..1.i = phi i64 [ %CurrSBIndex..0.i, %while.body.lr.ph.i ], [ %CurrSBIndex..7.i, %SyncBB.i ]
  %CurrWI..1.i = phi i64 [ %CurrWI..0.i, %while.body.lr.ph.i ], [ %CurrWI..7.i, %SyncBB.i ]
  %i.07.in.i = phi i64 [ %add11.i, %while.body.lr.ph.i ], [ %add50.i, %SyncBB.i ]
  %seed.06.i = phi float [ %40, %while.body.lr.ph.i ], [ %55, %SyncBB.i ]
  %window.05.i = phi i32 [ %conv5.i, %while.body.lr.ph.i ], [ %conv47.i, %SyncBB.i ]
  %"&(pSB[currWI].offset)51.i" = add nuw i64 %CurrSBIndex..1.i, 16
  %"&pSB[currWI].offset52.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)51.i"
  %CastToValueType53.i = bitcast i8* %"&pSB[currWI].offset52.i" to i32*
  store i32 %window.05.i, i32* %CastToValueType53.i, align 4
  %"&(pSB[currWI].offset)42.i" = add nuw i64 %CurrSBIndex..1.i, 12
  %"&pSB[currWI].offset43.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)42.i"
  %CastToValueType44.i = bitcast i8* %"&pSB[currWI].offset43.i" to float*
  store float %seed.06.i, float* %CastToValueType44.i, align 4
  %i.07.i = trunc i64 %i.07.in.i to i32
  %"&(pSB[currWI].offset)60.i" = add nuw i64 %CurrSBIndex..1.i, 20
  %"&pSB[currWI].offset61.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)60.i"
  %CastToValueType62.i = bitcast i8* %"&pSB[currWI].offset61.i" to i32*
  store i32 %i.07.i, i32* %CastToValueType62.i, align 4
  %cmp16.i = icmp slt i32 %i.07.i, %cond.i
  %"&(pSB[currWI].offset)79.i" = add nuw i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset80.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)79.i"
  %CastToValueType81.i = bitcast i8* %"&pSB[currWI].offset80.i" to i1*
  store i1 %cmp16.i, i1* %CastToValueType81.i, align 1
  br i1 %cmp16.i, label %if.then.i, label %while.body.i.if.end.i_crit_edge

while.body.i.if.end.i_crit_edge:                  ; preds = %while.body.i
  br label %if.end.i

if.then.i:                                        ; preds = %while.body.i
  %idxprom.i = sext i32 %i.07.i to i64
  %arrayidx18.i = getelementptr inbounds <4 x float> addrspace(1)* %33, i64 %idxprom.i
  %43 = load <4 x float> addrspace(1)* %arrayidx18.i, align 16
  br label %if.end.i

if.end.i:                                         ; preds = %while.body.i.if.end.i_crit_edge, %if.then.i
  %val_4.1.i = phi <4 x float> [ %43, %if.then.i ], [ zeroinitializer, %while.body.i.if.end.i_crit_edge ]
  %44 = extractelement <4 x float> %val_4.1.i, i32 0
  %"&(pSB[currWI].offset)88.i" = add nuw i64 %CurrSBIndex..1.i, 28
  %"&pSB[currWI].offset89.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)88.i"
  %CastToValueType90.i = bitcast i8* %"&pSB[currWI].offset89.i" to float*
  store float %44, float* %CastToValueType90.i, align 4
  %45 = extractelement <4 x float> %val_4.1.i, i32 1
  %add19.i = fadd float %45, %44
  %"&(pSB[currWI].offset)97.i" = add nuw i64 %CurrSBIndex..1.i, 32
  %"&pSB[currWI].offset98.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)97.i"
  %CastToValueType99.i = bitcast i8* %"&pSB[currWI].offset98.i" to float*
  store float %add19.i, float* %CastToValueType99.i, align 4
  %46 = extractelement <4 x float> %val_4.1.i, i32 2
  %add20.i = fadd float %46, %add19.i
  %"&(pSB[currWI].offset)106.i" = add nuw i64 %CurrSBIndex..1.i, 36
  %"&pSB[currWI].offset107.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)106.i"
  %CastToValueType108.i = bitcast i8* %"&pSB[currWI].offset107.i" to float*
  store float %add20.i, float* %CastToValueType108.i, align 4
  %47 = extractelement <4 x float> %val_4.1.i, i32 3
  %add21.i = fadd float %47, %add20.i
  %"&(pSB[currWI].offset)115.i" = add nuw i64 %CurrSBIndex..1.i, 40
  %"&pSB[currWI].offset116.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)115.i"
  %CastToValueType117.i = bitcast i8* %"&pSB[currWI].offset116.i" to float*
  store float %add21.i, float* %CastToValueType117.i, align 4
  %"&pSB[currWI].offset15.i" = getelementptr inbounds i8* %31, i64 %CurrSBIndex..1.i
  %CastToValueType16.i = bitcast i8* %"&pSB[currWI].offset15.i" to i64*
  %loadedValue.i = load i64* %CastToValueType16.i, align 8
  %sext.i.i = shl i64 %loadedValue.i, 32
  %idxprom.i.i = ashr exact i64 %sext.i.i, 32
  %arrayidx.i.i = getelementptr inbounds float addrspace(3)* %13, i64 %idxprom.i.i
  store float 0.000000e+00, float addrspace(3)* %arrayidx.i.i, align 4
  %loadedValue21.i = load i64* %CastToValueType16.i, align 8
  %add.i.i = add i64 %call38174.1.i, %loadedValue21.i
  %conv3.i.i = trunc i64 %add.i.i to i32
  %"&(pSB[currWI].offset)124.i" = add nuw i64 %CurrSBIndex..1.i, 44
  %"&pSB[currWI].offset125.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)124.i"
  %CastToValueType126.i = bitcast i8* %"&pSB[currWI].offset125.i" to i32*
  store i32 %conv3.i.i, i32* %CastToValueType126.i, align 4
  %idxprom4.i.i = sext i32 %conv3.i.i to i64
  %arrayidx5.i.i = getelementptr inbounds float addrspace(3)* %13, i64 %idxprom4.i.i
  %"&(pSB[currWI].offset)138.i" = add nuw i64 %CurrSBIndex..1.i, 48
  %"&pSB[currWI].offset139.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)138.i"
  %CastToValueType140.i = bitcast i8* %"&pSB[currWI].offset139.i" to float addrspace(3)**
  store float addrspace(3)* %arrayidx5.i.i, float addrspace(3)** %CastToValueType140.i, align 8
  store float %add21.i, float addrspace(3)* %arrayidx5.i.i, align 4
  %check.WI.iter189.i = icmp ult i64 %CurrWI..1.i, %28
  br i1 %check.WI.iter189.i, label %thenBB186.i, label %elseBB187.i

thenBB186.i:                                      ; preds = %if.end.i
  %"CurrWI++190.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride192.i" = add nuw i64 %CurrSBIndex..1.i, 64
  %cond7.i = icmp eq i32 %currBarrier.1.i, 28
  br i1 %cond7.i, label %SyncBB183.i, label %thenBB186.i.SyncBB.i_crit_edge

thenBB186.i.SyncBB.i_crit_edge:                   ; preds = %thenBB186.i
  br label %SyncBB.i

elseBB187.i:                                      ; preds = %if.end.i
  %cmp1.i.i = icmp ugt i64 %call38174.1.i, 1
  br label %SyncBB180.i

SyncBB180.i:                                      ; preds = %thenBB.i, %thenBB194.i, %elseBB187.i
  %currBarrier.2.i = phi i32 [ 8, %elseBB187.i ], [ %currBarrier.5.i, %thenBB.i ], [ %currBarrier.3.i, %thenBB194.i ]
  %CurrSBIndex..2.i = phi i64 [ 0, %elseBB187.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ %"loadedCurrSB+Stride200.i", %thenBB194.i ]
  %CurrWI..2.i = phi i64 [ 0, %elseBB187.i ], [ %"CurrWI++.i", %thenBB.i ], [ %"CurrWI++198.i", %thenBB194.i ]
  br i1 %cmp1.i.i, label %for.body.i.i, label %scanLocalMem.exit.i

for.body.i.i:                                     ; preds = %SyncBB182.i, %SyncBB180.i
  %currBarrier.3.i = phi i32 [ %currBarrier.4.i, %SyncBB182.i ], [ %currBarrier.2.i, %SyncBB180.i ]
  %CurrSBIndex..3.i = phi i64 [ %CurrSBIndex..5.i, %SyncBB182.i ], [ %CurrSBIndex..2.i, %SyncBB180.i ]
  %CurrWI..3.i = phi i64 [ %CurrWI..5.i, %SyncBB182.i ], [ %CurrWI..2.i, %SyncBB180.i ]
  %i.02.i.i = phi i32 [ %mul.i.i, %SyncBB182.i ], [ 1, %SyncBB180.i ]
  %"&(pSB[currWI].offset)152.i" = add nuw i64 %CurrSBIndex..3.i, 56
  %"&pSB[currWI].offset153.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)152.i"
  %CastToValueType154.i = bitcast i8* %"&pSB[currWI].offset153.i" to i32*
  store i32 %i.02.i.i, i32* %CastToValueType154.i, align 4
  %"&(pSB[currWI].offset)133.i" = add nuw i64 %CurrSBIndex..3.i, 44
  %"&pSB[currWI].offset134.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)133.i"
  %CastToValueType135.i = bitcast i8* %"&pSB[currWI].offset134.i" to i32*
  %loadedValue136.i = load i32* %CastToValueType135.i, align 4
  %sub.i.i = sub nsw i32 %loadedValue136.i, %i.02.i.i
  %idxprom9.i.i = sext i32 %sub.i.i to i64
  %arrayidx10.i.i = getelementptr inbounds float addrspace(3)* %13, i64 %idxprom9.i.i
  %48 = load float addrspace(3)* %arrayidx10.i.i, align 4
  %"&(pSB[currWI].offset)161.i" = add nuw i64 %CurrSBIndex..3.i, 60
  %"&pSB[currWI].offset162.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)161.i"
  %CastToValueType163.i = bitcast i8* %"&pSB[currWI].offset162.i" to float*
  store float %48, float* %CastToValueType163.i, align 4
  %check.WI.iter197.i = icmp ult i64 %CurrWI..3.i, %28
  br i1 %check.WI.iter197.i, label %thenBB194.i, label %for.body.i.i.SyncBB181.i_crit_edge

for.body.i.i.SyncBB181.i_crit_edge:               ; preds = %for.body.i.i
  br label %SyncBB181.i

thenBB194.i:                                      ; preds = %for.body.i.i
  %"CurrWI++198.i" = add nuw i64 %CurrWI..3.i, 1
  %"loadedCurrSB+Stride200.i" = add nuw i64 %CurrSBIndex..3.i, 64
  %cond6.i = icmp eq i32 %currBarrier.3.i, 8
  br i1 %cond6.i, label %SyncBB180.i, label %SyncBB182.i

SyncBB181.i:                                      ; preds = %for.body.i.i.SyncBB181.i_crit_edge, %thenBB202.i
  %CurrSBIndex..4.i = phi i64 [ %"loadedCurrSB+Stride208.i", %thenBB202.i ], [ 0, %for.body.i.i.SyncBB181.i_crit_edge ]
  %CurrWI..4.i = phi i64 [ %"CurrWI++206.i", %thenBB202.i ], [ 0, %for.body.i.i.SyncBB181.i_crit_edge ]
  %"&(pSB[currWI].offset)1471.i" = or i64 %CurrSBIndex..4.i, 48
  %"&pSB[currWI].offset148.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1471.i"
  %CastToValueType149.i = bitcast i8* %"&pSB[currWI].offset148.i" to float addrspace(3)**
  %loadedValue150.i = load float addrspace(3)** %CastToValueType149.i, align 8
  %49 = load float addrspace(3)* %loadedValue150.i, align 4
  %"&(pSB[currWI].offset)1652.i" = or i64 %CurrSBIndex..4.i, 60
  %"&pSB[currWI].offset166.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1652.i"
  %CastToValueType167.i = bitcast i8* %"&pSB[currWI].offset166.i" to float*
  %loadedValue168.i = load float* %CastToValueType167.i, align 4
  %add13.i.i = fadd float %49, %loadedValue168.i
  store float %add13.i.i, float addrspace(3)* %loadedValue150.i, align 4
  %check.WI.iter205.i = icmp ult i64 %CurrWI..4.i, %28
  br i1 %check.WI.iter205.i, label %thenBB202.i, label %SyncBB182.i

thenBB202.i:                                      ; preds = %SyncBB181.i
  %"CurrWI++206.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride208.i" = add nuw i64 %CurrSBIndex..4.i, 64
  br label %SyncBB181.i

SyncBB182.i:                                      ; preds = %thenBB.i, %SyncBB181.i, %thenBB194.i
  %currBarrier.4.i = phi i32 [ %currBarrier.3.i, %thenBB194.i ], [ %currBarrier.5.i, %thenBB.i ], [ 10, %SyncBB181.i ]
  %CurrSBIndex..5.i = phi i64 [ %"loadedCurrSB+Stride200.i", %thenBB194.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %SyncBB181.i ]
  %CurrWI..5.i = phi i64 [ %"CurrWI++198.i", %thenBB194.i ], [ %"CurrWI++.i", %thenBB.i ], [ 0, %SyncBB181.i ]
  %"&(pSB[currWI].offset)156.i" = add nuw i64 %CurrSBIndex..5.i, 56
  %"&pSB[currWI].offset157.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)156.i"
  %CastToValueType158.i = bitcast i8* %"&pSB[currWI].offset157.i" to i32*
  %loadedValue159.i = load i32* %CastToValueType158.i, align 4
  %mul.i.i = shl nsw i32 %loadedValue159.i, 1
  %conv6.i.i = sext i32 %mul.i.i to i64
  %cmp.i.i = icmp ult i64 %conv6.i.i, %call38174.1.i
  br i1 %cmp.i.i, label %for.body.i.i, label %scanLocalMem.exit.i

scanLocalMem.exit.i:                              ; preds = %SyncBB182.i, %SyncBB180.i
  %currBarrier.5.i = phi i32 [ %currBarrier.2.i, %SyncBB180.i ], [ %currBarrier.4.i, %SyncBB182.i ]
  %CurrSBIndex..6.i = phi i64 [ %CurrSBIndex..2.i, %SyncBB180.i ], [ %CurrSBIndex..5.i, %SyncBB182.i ]
  %CurrWI..6.i = phi i64 [ %CurrWI..2.i, %SyncBB180.i ], [ %CurrWI..5.i, %SyncBB182.i ]
  %"&(pSB[currWI].offset)128.i" = add nuw i64 %CurrSBIndex..6.i, 44
  %"&pSB[currWI].offset129.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)128.i"
  %CastToValueType130.i = bitcast i8* %"&pSB[currWI].offset129.i" to i32*
  %loadedValue131.i = load i32* %CastToValueType130.i, align 4
  %sub14.i.i = add nsw i32 %loadedValue131.i, -1
  %idxprom15.i.i = sext i32 %sub14.i.i to i64
  %arrayidx16.i.i = getelementptr inbounds float addrspace(3)* %13, i64 %idxprom15.i.i
  %50 = load float addrspace(3)* %arrayidx16.i.i, align 4
  %"&(pSB[currWI].offset)46.i" = add nuw i64 %CurrSBIndex..6.i, 12
  %"&pSB[currWI].offset47.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)46.i"
  %CastToValueType48.i = bitcast i8* %"&pSB[currWI].offset47.i" to float*
  %loadedValue49.i = load float* %CastToValueType48.i, align 4
  %add29.i = fadd float %50, %loadedValue49.i
  %"&(pSB[currWI].offset)119.i" = add nuw i64 %CurrSBIndex..6.i, 40
  %"&pSB[currWI].offset120.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)119.i"
  %CastToValueType121.i = bitcast i8* %"&pSB[currWI].offset120.i" to float*
  %loadedValue122.i = load float* %CastToValueType121.i, align 4
  %add30.i = fadd float %loadedValue122.i, %add29.i
  %"&(pSB[currWI].offset)83.i" = add nuw i64 %CurrSBIndex..6.i, 24
  %"&pSB[currWI].offset84.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)83.i"
  %CastToValueType85.i = bitcast i8* %"&pSB[currWI].offset84.i" to i1*
  %loadedValue86.i = load i1* %CastToValueType85.i, align 1
  br i1 %loadedValue86.i, label %if.then33.i, label %if.end36.i

if.then33.i:                                      ; preds = %scanLocalMem.exit.i
  %"&(pSB[currWI].offset)92.i" = add nuw i64 %CurrSBIndex..6.i, 28
  %"&pSB[currWI].offset93.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)92.i"
  %CastToValueType94.i = bitcast i8* %"&pSB[currWI].offset93.i" to float*
  %loadedValue95.i = load float* %CastToValueType94.i, align 4
  %add24.i = fadd float %loadedValue95.i, %add29.i
  %"&(pSB[currWI].offset)101.i" = add nuw i64 %CurrSBIndex..6.i, 32
  %"&pSB[currWI].offset102.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)101.i"
  %CastToValueType103.i = bitcast i8* %"&pSB[currWI].offset102.i" to float*
  %loadedValue104.i = load float* %CastToValueType103.i, align 4
  %add26.i = fadd float %loadedValue104.i, %add29.i
  %51 = insertelement <4 x float> undef, float %add24.i, i32 0
  %"&(pSB[currWI].offset)110.i" = add nuw i64 %CurrSBIndex..6.i, 36
  %"&pSB[currWI].offset111.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)110.i"
  %CastToValueType112.i = bitcast i8* %"&pSB[currWI].offset111.i" to float*
  %loadedValue113.i = load float* %CastToValueType112.i, align 4
  %add28.i = fadd float %loadedValue113.i, %add29.i
  %52 = insertelement <4 x float> %51, float %add26.i, i32 1
  %53 = insertelement <4 x float> %52, float %add28.i, i32 2
  %54 = insertelement <4 x float> %53, float %add30.i, i32 3
  %"&(pSB[currWI].offset)69.i" = add nuw i64 %CurrSBIndex..6.i, 20
  %"&pSB[currWI].offset70.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)69.i"
  %CastToValueType71.i = bitcast i8* %"&pSB[currWI].offset70.i" to i32*
  %loadedValue72.i = load i32* %CastToValueType71.i, align 4
  %idxprom34.i = sext i32 %loadedValue72.i to i64
  %arrayidx35.i = getelementptr inbounds <4 x float> addrspace(1)* %34, i64 %idxprom34.i
  store <4 x float> %54, <4 x float> addrspace(1)* %arrayidx35.i, align 16
  br label %if.end36.i

if.end36.i:                                       ; preds = %if.then33.i, %scanLocalMem.exit.i
  %"&(pSB[currWI].offset)37.i" = add nuw i64 %CurrSBIndex..6.i, 8
  %"&pSB[currWI].offset38.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)37.i"
  %CastToValueType39.i = bitcast i8* %"&pSB[currWI].offset38.i" to i1*
  %loadedValue40.i = load i1* %CastToValueType39.i, align 1
  br i1 %loadedValue40.i, label %if.then42.i, label %if.end43.i

if.then42.i:                                      ; preds = %if.end36.i
  store float %add30.i, float addrspace(3)* %32, align 4
  br label %if.end43.i

if.end43.i:                                       ; preds = %if.then42.i, %if.end36.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..6.i, %28
  br i1 %check.WI.iter.i, label %thenBB.i, label %if.end43.i.SyncBB.i_crit_edge

if.end43.i.SyncBB.i_crit_edge:                    ; preds = %if.end43.i
  br label %SyncBB.i

thenBB.i:                                         ; preds = %if.end43.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..6.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..6.i, 64
  %cond5.i = icmp eq i32 %currBarrier.5.i, 10
  br i1 %cond5.i, label %SyncBB182.i, label %SyncBB180.i

SyncBB.i:                                         ; preds = %thenBB209.i.SyncBB.i_crit_edge, %if.end43.i.SyncBB.i_crit_edge, %thenBB186.i.SyncBB.i_crit_edge
  %call38174.2.i = phi i64 [ %call38174.1.i, %thenBB186.i.SyncBB.i_crit_edge ], [ %call38174.1.i, %if.end43.i.SyncBB.i_crit_edge ], [ %call38174.3.i, %thenBB209.i.SyncBB.i_crit_edge ]
  %currBarrier.6.i = phi i32 [ %currBarrier.1.i, %thenBB186.i.SyncBB.i_crit_edge ], [ 2, %if.end43.i.SyncBB.i_crit_edge ], [ %currBarrier.7.i, %thenBB209.i.SyncBB.i_crit_edge ]
  %CurrSBIndex..7.i = phi i64 [ %"loadedCurrSB+Stride192.i", %thenBB186.i.SyncBB.i_crit_edge ], [ 0, %if.end43.i.SyncBB.i_crit_edge ], [ %"loadedCurrSB+Stride215.i", %thenBB209.i.SyncBB.i_crit_edge ]
  %CurrWI..7.i = phi i64 [ %"CurrWI++190.i", %thenBB186.i.SyncBB.i_crit_edge ], [ 0, %if.end43.i.SyncBB.i_crit_edge ], [ %"CurrWI++213.i", %thenBB209.i.SyncBB.i_crit_edge ]
  %55 = load float addrspace(3)* %32, align 4
  %"&(pSB[currWI].offset)55.i" = add nuw i64 %CurrSBIndex..7.i, 16
  %"&pSB[currWI].offset56.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)55.i"
  %CastToValueType57.i = bitcast i8* %"&pSB[currWI].offset56.i" to i32*
  %loadedValue58.i = load i32* %CastToValueType57.i, align 4
  %conv452.i = zext i32 %loadedValue58.i to i64
  %add46.i = add i64 %call38174.2.i, %conv452.i
  %conv47.i = trunc i64 %add46.i to i32
  %"&(pSB[currWI].offset)74.i" = add nuw i64 %CurrSBIndex..7.i, 20
  %"&pSB[currWI].offset75.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)74.i"
  %CastToValueType76.i = bitcast i8* %"&pSB[currWI].offset75.i" to i32*
  %loadedValue77.i = load i32* %CastToValueType76.i, align 4
  %conv49.i = sext i32 %loadedValue77.i to i64
  %add50.i = add i64 %call38174.2.i, %conv49.i
  %cmp14.i = icmp slt i32 %conv47.i, %cond.i
  br i1 %cmp14.i, label %while.body.i, label %while.end.i

while.end.i:                                      ; preds = %SyncBB.i, %SyncBB183.i
  %call38174.3.i = phi i64 [ %call38174.0.i, %SyncBB183.i ], [ %call38174.2.i, %SyncBB.i ]
  %currBarrier.7.i = phi i32 [ %currBarrier.0.i, %SyncBB183.i ], [ %currBarrier.6.i, %SyncBB.i ]
  %CurrSBIndex..8.i = phi i64 [ %CurrSBIndex..0.i, %SyncBB183.i ], [ %CurrSBIndex..7.i, %SyncBB.i ]
  %CurrWI..8.i = phi i64 [ %CurrWI..0.i, %SyncBB183.i ], [ %CurrWI..7.i, %SyncBB.i ]
  %check.WI.iter212.i = icmp ult i64 %CurrWI..8.i, %28
  br i1 %check.WI.iter212.i, label %thenBB209.i, label %__bottom_scan_separated_args.exit

thenBB209.i:                                      ; preds = %while.end.i
  %"CurrWI++213.i" = add nuw i64 %CurrWI..8.i, 1
  %"loadedCurrSB+Stride215.i" = add nuw i64 %CurrSBIndex..8.i, 64
  %cond4.i = icmp eq i32 %currBarrier.7.i, 2
  br i1 %cond4.i, label %thenBB209.i.SyncBB.i_crit_edge, label %SyncBB183.i

thenBB209.i.SyncBB.i_crit_edge:                   ; preds = %thenBB209.i
  br label %SyncBB.i

__bottom_scan_separated_args.exit:                ; preds = %while.end.i
  ret void
}

define void @top_scan(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32*
  %4 = load i32* %3, align 4
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float addrspace(3)**
  %7 = load float addrspace(3)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 32
  %9 = bitcast i8* %8 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %10 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to <{ [4 x i64] }>**
  %13 = load <{ [4 x i64] }>** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 72
  %15 = bitcast i8* %14 to i64*
  %16 = load i64* %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 80
  %18 = bitcast i8* %17 to i8**
  %19 = load i8** %18, align 8
  %conv.i = sext i32 %4 to i64
  br label %SyncBB82.i

SyncBB82.i:                                       ; preds = %thenBB.i, %entry
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %20 = getelementptr <{ [4 x i64] }>* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %21 = load i64* %20, align 8
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i64*
  store i64 %21, i64* %CastToValueType.i, align 8
  %cmp.i = icmp ult i64 %21, %conv.i
  %"&(pSB[currWI].offset)231.i" = or i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset24.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)231.i"
  %CastToValueType25.i = bitcast i8* %"&pSB[currWI].offset24.i" to i1*
  store i1 %cmp.i, i1* %CastToValueType25.i, align 1
  br i1 %cmp.i, label %if.then.i, label %if.end.i

if.then.i:                                        ; preds = %SyncBB82.i
  %arrayidx.i = getelementptr inbounds float addrspace(1)* %1, i64 %21
  %22 = load float addrspace(1)* %arrayidx.i, align 4
  br label %if.end.i

if.end.i:                                         ; preds = %if.then.i, %SyncBB82.i
  %val.0.i = phi float [ %22, %if.then.i ], [ 0.000000e+00, %SyncBB82.i ]
  %sext.i.i = shl i64 %21, 32
  %idxprom.i.i = ashr exact i64 %sext.i.i, 32
  %arrayidx.i.i = getelementptr inbounds float addrspace(3)* %7, i64 %idxprom.i.i
  store float 0.000000e+00, float addrspace(3)* %arrayidx.i.i, align 4
  %23 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %10, i64 0, i32 3, i64 0
  %24 = load i64* %23, align 8
  %loadedValue11.i = load i64* %CastToValueType.i, align 8
  %add.i.i = add i64 %24, %loadedValue11.i
  %conv3.i.i = trunc i64 %add.i.i to i32
  %"&(pSB[currWI].offset)322.i" = or i64 %CurrSBIndex..0.i, 12
  %"&pSB[currWI].offset33.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)322.i"
  %CastToValueType34.i = bitcast i8* %"&pSB[currWI].offset33.i" to i32*
  store i32 %conv3.i.i, i32* %CastToValueType34.i, align 4
  %idxprom4.i.i = sext i32 %conv3.i.i to i64
  %arrayidx5.i.i = getelementptr inbounds float addrspace(3)* %7, i64 %idxprom4.i.i
  %"&(pSB[currWI].offset)463.i" = or i64 %CurrSBIndex..0.i, 16
  %"&pSB[currWI].offset47.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)463.i"
  %CastToValueType48.i = bitcast i8* %"&pSB[currWI].offset47.i" to float addrspace(3)**
  store float addrspace(3)* %arrayidx5.i.i, float addrspace(3)** %CastToValueType48.i, align 8
  store float %val.0.i, float addrspace(3)* %arrayidx5.i.i, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %if.end.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 32
  br label %SyncBB82.i

elseBB.i:                                         ; preds = %if.end.i
  %cmp1.i.i = icmp ugt i64 %24, 1
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB99.i, %thenBB85.i, %elseBB.i
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride105.i", %thenBB99.i ], [ %"loadedCurrSB+Stride91.i", %thenBB85.i ]
  %currBarrier.0.i = phi i32 [ 5, %elseBB.i ], [ %currBarrier.3.i, %thenBB99.i ], [ %currBarrier.1.i, %thenBB85.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++103.i", %thenBB99.i ], [ %"CurrWI++89.i", %thenBB85.i ]
  br i1 %cmp1.i.i, label %for.body.i.i, label %scanLocalMem.exit.i

for.body.i.i:                                     ; preds = %SyncBB81.i, %SyncBB.i
  %CurrSBIndex..2.i = phi i64 [ %CurrSBIndex..4.i, %SyncBB81.i ], [ %CurrSBIndex..1.i, %SyncBB.i ]
  %currBarrier.1.i = phi i32 [ %currBarrier.2.i, %SyncBB81.i ], [ %currBarrier.0.i, %SyncBB.i ]
  %CurrWI..2.i = phi i64 [ %CurrWI..4.i, %SyncBB81.i ], [ %CurrWI..1.i, %SyncBB.i ]
  %i.02.i.i = phi i32 [ %mul.i.i, %SyncBB81.i ], [ 1, %SyncBB.i ]
  %"&(pSB[currWI].offset)60.i" = add nuw i64 %CurrSBIndex..2.i, 24
  %"&pSB[currWI].offset61.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)60.i"
  %CastToValueType62.i = bitcast i8* %"&pSB[currWI].offset61.i" to i32*
  store i32 %i.02.i.i, i32* %CastToValueType62.i, align 4
  %"&(pSB[currWI].offset)41.i" = add nuw i64 %CurrSBIndex..2.i, 12
  %"&pSB[currWI].offset42.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)41.i"
  %CastToValueType43.i = bitcast i8* %"&pSB[currWI].offset42.i" to i32*
  %loadedValue44.i = load i32* %CastToValueType43.i, align 4
  %sub.i.i = sub nsw i32 %loadedValue44.i, %i.02.i.i
  %idxprom9.i.i = sext i32 %sub.i.i to i64
  %arrayidx10.i.i = getelementptr inbounds float addrspace(3)* %7, i64 %idxprom9.i.i
  %25 = load float addrspace(3)* %arrayidx10.i.i, align 4
  %"&(pSB[currWI].offset)69.i" = add nuw i64 %CurrSBIndex..2.i, 28
  %"&pSB[currWI].offset70.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)69.i"
  %CastToValueType71.i = bitcast i8* %"&pSB[currWI].offset70.i" to float*
  store float %25, float* %CastToValueType71.i, align 4
  %check.WI.iter88.i = icmp ult i64 %CurrWI..2.i, %16
  br i1 %check.WI.iter88.i, label %thenBB85.i, label %for.body.i.i.SyncBB80.i_crit_edge

for.body.i.i.SyncBB80.i_crit_edge:                ; preds = %for.body.i.i
  br label %SyncBB80.i

thenBB85.i:                                       ; preds = %for.body.i.i
  %"CurrWI++89.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride91.i" = add nuw i64 %CurrSBIndex..2.i, 32
  %cond7.i = icmp eq i32 %currBarrier.1.i, 5
  br i1 %cond7.i, label %SyncBB.i, label %SyncBB81.i

SyncBB80.i:                                       ; preds = %for.body.i.i.SyncBB80.i_crit_edge, %thenBB92.i
  %CurrSBIndex..3.i = phi i64 [ %"loadedCurrSB+Stride98.i", %thenBB92.i ], [ 0, %for.body.i.i.SyncBB80.i_crit_edge ]
  %CurrWI..3.i = phi i64 [ %"CurrWI++96.i", %thenBB92.i ], [ 0, %for.body.i.i.SyncBB80.i_crit_edge ]
  %"&(pSB[currWI].offset)554.i" = or i64 %CurrSBIndex..3.i, 16
  %"&pSB[currWI].offset56.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)554.i"
  %CastToValueType57.i = bitcast i8* %"&pSB[currWI].offset56.i" to float addrspace(3)**
  %loadedValue58.i = load float addrspace(3)** %CastToValueType57.i, align 8
  %26 = load float addrspace(3)* %loadedValue58.i, align 4
  %"&(pSB[currWI].offset)735.i" = or i64 %CurrSBIndex..3.i, 28
  %"&pSB[currWI].offset74.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)735.i"
  %CastToValueType75.i = bitcast i8* %"&pSB[currWI].offset74.i" to float*
  %loadedValue76.i = load float* %CastToValueType75.i, align 4
  %add13.i.i = fadd float %26, %loadedValue76.i
  store float %add13.i.i, float addrspace(3)* %loadedValue58.i, align 4
  %check.WI.iter95.i = icmp ult i64 %CurrWI..3.i, %16
  br i1 %check.WI.iter95.i, label %thenBB92.i, label %SyncBB81.i

thenBB92.i:                                       ; preds = %SyncBB80.i
  %"CurrWI++96.i" = add nuw i64 %CurrWI..3.i, 1
  %"loadedCurrSB+Stride98.i" = add nuw i64 %CurrSBIndex..3.i, 32
  br label %SyncBB80.i

SyncBB81.i:                                       ; preds = %thenBB99.i, %SyncBB80.i, %thenBB85.i
  %CurrSBIndex..4.i = phi i64 [ %"loadedCurrSB+Stride91.i", %thenBB85.i ], [ %"loadedCurrSB+Stride105.i", %thenBB99.i ], [ 0, %SyncBB80.i ]
  %currBarrier.2.i = phi i32 [ %currBarrier.1.i, %thenBB85.i ], [ %currBarrier.3.i, %thenBB99.i ], [ 7, %SyncBB80.i ]
  %CurrWI..4.i = phi i64 [ %"CurrWI++89.i", %thenBB85.i ], [ %"CurrWI++103.i", %thenBB99.i ], [ 0, %SyncBB80.i ]
  %"&(pSB[currWI].offset)64.i" = add nuw i64 %CurrSBIndex..4.i, 24
  %"&pSB[currWI].offset65.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)64.i"
  %CastToValueType66.i = bitcast i8* %"&pSB[currWI].offset65.i" to i32*
  %loadedValue67.i = load i32* %CastToValueType66.i, align 4
  %mul.i.i = shl nsw i32 %loadedValue67.i, 1
  %conv6.i.i = sext i32 %mul.i.i to i64
  %cmp.i.i = icmp ult i64 %conv6.i.i, %24
  br i1 %cmp.i.i, label %for.body.i.i, label %scanLocalMem.exit.i

scanLocalMem.exit.i:                              ; preds = %SyncBB81.i, %SyncBB.i
  %CurrSBIndex..5.i = phi i64 [ %CurrSBIndex..1.i, %SyncBB.i ], [ %CurrSBIndex..4.i, %SyncBB81.i ]
  %currBarrier.3.i = phi i32 [ %currBarrier.0.i, %SyncBB.i ], [ %currBarrier.2.i, %SyncBB81.i ]
  %CurrWI..5.i = phi i64 [ %CurrWI..1.i, %SyncBB.i ], [ %CurrWI..4.i, %SyncBB81.i ]
  %"&(pSB[currWI].offset)27.i" = add nuw i64 %CurrSBIndex..5.i, 8
  %"&pSB[currWI].offset28.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)27.i"
  %CastToValueType29.i = bitcast i8* %"&pSB[currWI].offset28.i" to i1*
  %loadedValue30.i = load i1* %CastToValueType29.i, align 1
  br i1 %loadedValue30.i, label %if.then8.i, label %if.end11.i

if.then8.i:                                       ; preds = %scanLocalMem.exit.i
  %"&(pSB[currWI].offset)36.i" = add nuw i64 %CurrSBIndex..5.i, 12
  %"&pSB[currWI].offset37.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)36.i"
  %CastToValueType38.i = bitcast i8* %"&pSB[currWI].offset37.i" to i32*
  %loadedValue39.i = load i32* %CastToValueType38.i, align 4
  %sub14.i.i = add nsw i32 %loadedValue39.i, -1
  %idxprom15.i.i = sext i32 %sub14.i.i to i64
  %arrayidx16.i.i = getelementptr inbounds float addrspace(3)* %7, i64 %idxprom15.i.i
  %27 = load float addrspace(3)* %arrayidx16.i.i, align 4
  %"&pSB[currWI].offset14.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..5.i
  %CastToValueType15.i = bitcast i8* %"&pSB[currWI].offset14.i" to i64*
  %loadedValue16.i = load i64* %CastToValueType15.i, align 8
  %arrayidx10.i = getelementptr inbounds float addrspace(1)* %1, i64 %loadedValue16.i
  store float %27, float addrspace(1)* %arrayidx10.i, align 4
  br label %if.end11.i

if.end11.i:                                       ; preds = %if.then8.i, %scanLocalMem.exit.i
  %check.WI.iter102.i = icmp ult i64 %CurrWI..5.i, %16
  br i1 %check.WI.iter102.i, label %thenBB99.i, label %__top_scan_separated_args.exit

thenBB99.i:                                       ; preds = %if.end11.i
  %"CurrWI++103.i" = add nuw i64 %CurrWI..5.i, 1
  %"loadedCurrSB+Stride105.i" = add nuw i64 %CurrSBIndex..5.i, 32
  %cond.i = icmp eq i32 %currBarrier.3.i, 7
  br i1 %cond.i, label %SyncBB81.i, label %SyncBB.i

__top_scan_separated_args.exit:                   ; preds = %if.end11.i
  ret void
}

define void @__Vectorized_.top_scan(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32*
  %4 = load i32* %3, align 4
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float addrspace(3)**
  %7 = load float addrspace(3)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 32
  %9 = bitcast i8* %8 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %10 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to <{ [4 x i64] }>**
  %13 = load <{ [4 x i64] }>** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 72
  %15 = bitcast i8* %14 to i64*
  %16 = load i64* %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 80
  %18 = bitcast i8* %17 to i8**
  %19 = load i8** %18, align 8
  %conv.i = sext i32 %4 to i64
  %temp.i = insertelement <16 x i64> undef, i64 %conv.i, i32 0
  %vector.i = shufflevector <16 x i64> %temp.i, <16 x i64> undef, <16 x i32> zeroinitializer
  br label %SyncBB973.i

SyncBB973.i:                                      ; preds = %thenBB.i, %entry
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %20 = getelementptr <{ [4 x i64] }>* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %21 = load i64* %20, align 8
  %broadcast1.i = insertelement <16 x i64> undef, i64 %21, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %22 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %extract.i = extractelement <16 x i64> %22, i32 0
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i64*
  store i64 %extract.i, i64* %CastToValueType.i, align 8
  %cmp.i = icmp ult <16 x i64> %22, %vector.i
  %exmask.i = extractelement <16 x i1> %cmp.i, i32 0
  %"&(pSB[currWI].offset)6631.i" = or i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset664.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)6631.i"
  %CastToValueType665.i = bitcast i8* %"&pSB[currWI].offset664.i" to i1*
  store i1 %exmask.i, i1* %CastToValueType665.i, align 1
  br i1 %exmask.i, label %preload418.i, label %postload419.i

preload418.i:                                     ; preds = %SyncBB973.i
  %23 = getelementptr inbounds float addrspace(1)* %1, i64 %extract.i
  %vload128.i = load float addrspace(1)* %23, align 4
  br label %postload419.i

postload419.i:                                    ; preds = %preload418.i, %SyncBB973.i
  %phi420.i = phi float [ undef, %SyncBB973.i ], [ %vload128.i, %preload418.i ]
  %vpack.i = insertelement <16 x float> undef, float %phi420.i, i32 0
  %exmask130.i = extractelement <16 x i1> %cmp.i, i32 1
  %"&(pSB[currWI].offset)6772.i" = or i64 %CurrSBIndex..0.i, 9
  %"&pSB[currWI].offset678.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)6772.i"
  %CastToValueType679.i = bitcast i8* %"&pSB[currWI].offset678.i" to i1*
  store i1 %exmask130.i, i1* %CastToValueType679.i, align 1
  br i1 %exmask130.i, label %preload329.i, label %postload330.i

preload329.i:                                     ; preds = %postload419.i
  %.sum495.i = add i64 %extract.i, 1
  %24 = getelementptr float addrspace(1)* %1, i64 %.sum495.i
  %vload131.i = load float addrspace(1)* %24, align 4
  br label %postload330.i

postload330.i:                                    ; preds = %preload329.i, %postload419.i
  %phi331.i = phi float [ undef, %postload419.i ], [ %vload131.i, %preload329.i ]
  %vpack132.i = insertelement <16 x float> %vpack.i, float %phi331.i, i32 1
  %exmask134.i = extractelement <16 x i1> %cmp.i, i32 2
  %"&(pSB[currWI].offset)6913.i" = or i64 %CurrSBIndex..0.i, 10
  %"&pSB[currWI].offset692.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)6913.i"
  %CastToValueType693.i = bitcast i8* %"&pSB[currWI].offset692.i" to i1*
  store i1 %exmask134.i, i1* %CastToValueType693.i, align 1
  br i1 %exmask134.i, label %preload326.i, label %postload327.i

preload326.i:                                     ; preds = %postload330.i
  %.sum494.i = add i64 %extract.i, 2
  %25 = getelementptr float addrspace(1)* %1, i64 %.sum494.i
  %vload135.i = load float addrspace(1)* %25, align 4
  br label %postload327.i

postload327.i:                                    ; preds = %preload326.i, %postload330.i
  %phi328.i = phi float [ undef, %postload330.i ], [ %vload135.i, %preload326.i ]
  %vpack136.i = insertelement <16 x float> %vpack132.i, float %phi328.i, i32 2
  %exmask138.i = extractelement <16 x i1> %cmp.i, i32 3
  %"&(pSB[currWI].offset)7054.i" = or i64 %CurrSBIndex..0.i, 11
  %"&pSB[currWI].offset706.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)7054.i"
  %CastToValueType707.i = bitcast i8* %"&pSB[currWI].offset706.i" to i1*
  store i1 %exmask138.i, i1* %CastToValueType707.i, align 1
  br i1 %exmask138.i, label %preload323.i, label %postload324.i

preload323.i:                                     ; preds = %postload327.i
  %.sum493.i = add i64 %extract.i, 3
  %26 = getelementptr float addrspace(1)* %1, i64 %.sum493.i
  %vload139.i = load float addrspace(1)* %26, align 4
  br label %postload324.i

postload324.i:                                    ; preds = %preload323.i, %postload327.i
  %phi325.i = phi float [ undef, %postload327.i ], [ %vload139.i, %preload323.i ]
  %vpack140.i = insertelement <16 x float> %vpack136.i, float %phi325.i, i32 3
  %exmask142.i = extractelement <16 x i1> %cmp.i, i32 4
  %"&(pSB[currWI].offset)7195.i" = or i64 %CurrSBIndex..0.i, 12
  %"&pSB[currWI].offset720.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)7195.i"
  %CastToValueType721.i = bitcast i8* %"&pSB[currWI].offset720.i" to i1*
  store i1 %exmask142.i, i1* %CastToValueType721.i, align 1
  br i1 %exmask142.i, label %preload320.i, label %postload321.i

preload320.i:                                     ; preds = %postload324.i
  %.sum492.i = add i64 %extract.i, 4
  %27 = getelementptr float addrspace(1)* %1, i64 %.sum492.i
  %vload143.i = load float addrspace(1)* %27, align 4
  br label %postload321.i

postload321.i:                                    ; preds = %preload320.i, %postload324.i
  %phi322.i = phi float [ undef, %postload324.i ], [ %vload143.i, %preload320.i ]
  %vpack144.i = insertelement <16 x float> %vpack140.i, float %phi322.i, i32 4
  %exmask146.i = extractelement <16 x i1> %cmp.i, i32 5
  %"&(pSB[currWI].offset)7336.i" = or i64 %CurrSBIndex..0.i, 13
  %"&pSB[currWI].offset734.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)7336.i"
  %CastToValueType735.i = bitcast i8* %"&pSB[currWI].offset734.i" to i1*
  store i1 %exmask146.i, i1* %CastToValueType735.i, align 1
  br i1 %exmask146.i, label %preload338.i, label %postload339.i

preload338.i:                                     ; preds = %postload321.i
  %.sum491.i = add i64 %extract.i, 5
  %28 = getelementptr float addrspace(1)* %1, i64 %.sum491.i
  %vload147.i = load float addrspace(1)* %28, align 4
  br label %postload339.i

postload339.i:                                    ; preds = %preload338.i, %postload321.i
  %phi340.i = phi float [ undef, %postload321.i ], [ %vload147.i, %preload338.i ]
  %vpack148.i = insertelement <16 x float> %vpack144.i, float %phi340.i, i32 5
  %exmask150.i = extractelement <16 x i1> %cmp.i, i32 6
  %"&(pSB[currWI].offset)7477.i" = or i64 %CurrSBIndex..0.i, 14
  %"&pSB[currWI].offset748.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)7477.i"
  %CastToValueType749.i = bitcast i8* %"&pSB[currWI].offset748.i" to i1*
  store i1 %exmask150.i, i1* %CastToValueType749.i, align 1
  br i1 %exmask150.i, label %preload.i, label %postload.i

preload.i:                                        ; preds = %postload339.i
  %.sum490.i = add i64 %extract.i, 6
  %29 = getelementptr float addrspace(1)* %1, i64 %.sum490.i
  %vload151.i = load float addrspace(1)* %29, align 4
  br label %postload.i

postload.i:                                       ; preds = %preload.i, %postload339.i
  %phi.i = phi float [ undef, %postload339.i ], [ %vload151.i, %preload.i ]
  %vpack152.i = insertelement <16 x float> %vpack148.i, float %phi.i, i32 6
  %exmask154.i = extractelement <16 x i1> %cmp.i, i32 7
  %"&(pSB[currWI].offset)7618.i" = or i64 %CurrSBIndex..0.i, 15
  %"&pSB[currWI].offset762.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)7618.i"
  %CastToValueType763.i = bitcast i8* %"&pSB[currWI].offset762.i" to i1*
  store i1 %exmask154.i, i1* %CastToValueType763.i, align 1
  br i1 %exmask154.i, label %preload314.i, label %postload315.i

preload314.i:                                     ; preds = %postload.i
  %.sum489.i = add i64 %extract.i, 7
  %30 = getelementptr float addrspace(1)* %1, i64 %.sum489.i
  %vload155.i = load float addrspace(1)* %30, align 4
  br label %postload315.i

postload315.i:                                    ; preds = %preload314.i, %postload.i
  %phi316.i = phi float [ undef, %postload.i ], [ %vload155.i, %preload314.i ]
  %vpack156.i = insertelement <16 x float> %vpack152.i, float %phi316.i, i32 7
  %exmask158.i = extractelement <16 x i1> %cmp.i, i32 8
  %"&(pSB[currWI].offset)7759.i" = or i64 %CurrSBIndex..0.i, 16
  %"&pSB[currWI].offset776.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)7759.i"
  %CastToValueType777.i = bitcast i8* %"&pSB[currWI].offset776.i" to i1*
  store i1 %exmask158.i, i1* %CastToValueType777.i, align 1
  br i1 %exmask158.i, label %preload335.i, label %postload336.i

preload335.i:                                     ; preds = %postload315.i
  %.sum488.i = add i64 %extract.i, 8
  %31 = getelementptr float addrspace(1)* %1, i64 %.sum488.i
  %vload159.i = load float addrspace(1)* %31, align 4
  br label %postload336.i

postload336.i:                                    ; preds = %preload335.i, %postload315.i
  %phi337.i = phi float [ undef, %postload315.i ], [ %vload159.i, %preload335.i ]
  %vpack160.i = insertelement <16 x float> %vpack156.i, float %phi337.i, i32 8
  %exmask162.i = extractelement <16 x i1> %cmp.i, i32 9
  %"&(pSB[currWI].offset)78910.i" = or i64 %CurrSBIndex..0.i, 17
  %"&pSB[currWI].offset790.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)78910.i"
  %CastToValueType791.i = bitcast i8* %"&pSB[currWI].offset790.i" to i1*
  store i1 %exmask162.i, i1* %CastToValueType791.i, align 1
  br i1 %exmask162.i, label %preload332.i, label %postload333.i

preload332.i:                                     ; preds = %postload336.i
  %.sum487.i = add i64 %extract.i, 9
  %32 = getelementptr float addrspace(1)* %1, i64 %.sum487.i
  %vload163.i = load float addrspace(1)* %32, align 4
  br label %postload333.i

postload333.i:                                    ; preds = %preload332.i, %postload336.i
  %phi334.i = phi float [ undef, %postload336.i ], [ %vload163.i, %preload332.i ]
  %vpack164.i = insertelement <16 x float> %vpack160.i, float %phi334.i, i32 9
  %exmask166.i = extractelement <16 x i1> %cmp.i, i32 10
  %"&(pSB[currWI].offset)80311.i" = or i64 %CurrSBIndex..0.i, 18
  %"&pSB[currWI].offset804.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)80311.i"
  %CastToValueType805.i = bitcast i8* %"&pSB[currWI].offset804.i" to i1*
  store i1 %exmask166.i, i1* %CastToValueType805.i, align 1
  br i1 %exmask166.i, label %preload355.i, label %postload356.i

preload355.i:                                     ; preds = %postload333.i
  %.sum486.i = add i64 %extract.i, 10
  %33 = getelementptr float addrspace(1)* %1, i64 %.sum486.i
  %vload167.i = load float addrspace(1)* %33, align 4
  br label %postload356.i

postload356.i:                                    ; preds = %preload355.i, %postload333.i
  %phi357.i = phi float [ undef, %postload333.i ], [ %vload167.i, %preload355.i ]
  %vpack168.i = insertelement <16 x float> %vpack164.i, float %phi357.i, i32 10
  %exmask170.i = extractelement <16 x i1> %cmp.i, i32 11
  %"&(pSB[currWI].offset)81712.i" = or i64 %CurrSBIndex..0.i, 19
  %"&pSB[currWI].offset818.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)81712.i"
  %CastToValueType819.i = bitcast i8* %"&pSB[currWI].offset818.i" to i1*
  store i1 %exmask170.i, i1* %CastToValueType819.i, align 1
  br i1 %exmask170.i, label %preload349.i, label %postload350.i

preload349.i:                                     ; preds = %postload356.i
  %.sum485.i = add i64 %extract.i, 11
  %34 = getelementptr float addrspace(1)* %1, i64 %.sum485.i
  %vload171.i = load float addrspace(1)* %34, align 4
  br label %postload350.i

postload350.i:                                    ; preds = %preload349.i, %postload356.i
  %phi351.i = phi float [ undef, %postload356.i ], [ %vload171.i, %preload349.i ]
  %vpack172.i = insertelement <16 x float> %vpack168.i, float %phi351.i, i32 11
  %exmask174.i = extractelement <16 x i1> %cmp.i, i32 12
  %"&(pSB[currWI].offset)83113.i" = or i64 %CurrSBIndex..0.i, 20
  %"&pSB[currWI].offset832.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)83113.i"
  %CastToValueType833.i = bitcast i8* %"&pSB[currWI].offset832.i" to i1*
  store i1 %exmask174.i, i1* %CastToValueType833.i, align 1
  br i1 %exmask174.i, label %preload311.i, label %postload312.i

preload311.i:                                     ; preds = %postload350.i
  %.sum484.i = add i64 %extract.i, 12
  %35 = getelementptr float addrspace(1)* %1, i64 %.sum484.i
  %vload175.i = load float addrspace(1)* %35, align 4
  br label %postload312.i

postload312.i:                                    ; preds = %preload311.i, %postload350.i
  %phi313.i = phi float [ undef, %postload350.i ], [ %vload175.i, %preload311.i ]
  %vpack176.i = insertelement <16 x float> %vpack172.i, float %phi313.i, i32 12
  %exmask178.i = extractelement <16 x i1> %cmp.i, i32 13
  %"&(pSB[currWI].offset)84514.i" = or i64 %CurrSBIndex..0.i, 21
  %"&pSB[currWI].offset846.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)84514.i"
  %CastToValueType847.i = bitcast i8* %"&pSB[currWI].offset846.i" to i1*
  store i1 %exmask178.i, i1* %CastToValueType847.i, align 1
  br i1 %exmask178.i, label %preload302.i, label %postload303.i

preload302.i:                                     ; preds = %postload312.i
  %.sum483.i = add i64 %extract.i, 13
  %36 = getelementptr float addrspace(1)* %1, i64 %.sum483.i
  %vload179.i = load float addrspace(1)* %36, align 4
  br label %postload303.i

postload303.i:                                    ; preds = %preload302.i, %postload312.i
  %phi304.i = phi float [ undef, %postload312.i ], [ %vload179.i, %preload302.i ]
  %vpack180.i = insertelement <16 x float> %vpack176.i, float %phi304.i, i32 13
  %exmask182.i = extractelement <16 x i1> %cmp.i, i32 14
  %"&(pSB[currWI].offset)85915.i" = or i64 %CurrSBIndex..0.i, 22
  %"&pSB[currWI].offset860.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)85915.i"
  %CastToValueType861.i = bitcast i8* %"&pSB[currWI].offset860.i" to i1*
  store i1 %exmask182.i, i1* %CastToValueType861.i, align 1
  br i1 %exmask182.i, label %preload352.i, label %postload353.i

preload352.i:                                     ; preds = %postload303.i
  %.sum482.i = add i64 %extract.i, 14
  %37 = getelementptr float addrspace(1)* %1, i64 %.sum482.i
  %vload183.i = load float addrspace(1)* %37, align 4
  br label %postload353.i

postload353.i:                                    ; preds = %preload352.i, %postload303.i
  %phi354.i = phi float [ undef, %postload303.i ], [ %vload183.i, %preload352.i ]
  %vpack184.i = insertelement <16 x float> %vpack180.i, float %phi354.i, i32 14
  %exmask186.i = extractelement <16 x i1> %cmp.i, i32 15
  %"&(pSB[currWI].offset)87316.i" = or i64 %CurrSBIndex..0.i, 23
  %"&pSB[currWI].offset874.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)87316.i"
  %CastToValueType875.i = bitcast i8* %"&pSB[currWI].offset874.i" to i1*
  store i1 %exmask186.i, i1* %CastToValueType875.i, align 1
  br i1 %exmask186.i, label %preload308.i, label %if.end.i

preload308.i:                                     ; preds = %postload353.i
  %.sum481.i = add i64 %extract.i, 15
  %38 = getelementptr float addrspace(1)* %1, i64 %.sum481.i
  %vload187.i = load float addrspace(1)* %38, align 4
  br label %if.end.i

if.end.i:                                         ; preds = %preload308.i, %postload353.i
  %phi310.i = phi float [ undef, %postload353.i ], [ %vload187.i, %preload308.i ]
  %vpack188.i = insertelement <16 x float> %vpack184.i, float %phi310.i, i32 15
  %merge38.i = select <16 x i1> %cmp.i, <16 x float> %vpack188.i, <16 x float> zeroinitializer
  %extract43.lhs.i = shl i64 %extract.i, 32
  %extract43.i = ashr exact i64 %extract43.lhs.i, 32
  %39 = getelementptr inbounds float addrspace(3)* %7, i64 %extract43.i
  %ptrTypeCast59.i = bitcast float addrspace(3)* %39 to <16 x float> addrspace(3)*
  store <16 x float> zeroinitializer, <16 x float> addrspace(3)* %ptrTypeCast59.i, align 4
  %40 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %10, i64 0, i32 3, i64 0
  %41 = load i64* %40, align 8
  %temp60.i = insertelement <16 x i64> undef, i64 %41, i32 0
  %vector61.i = shufflevector <16 x i64> %temp60.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %add.i62.i = add <16 x i64> %vector61.i, %22
  %conv3.i63.i = trunc <16 x i64> %add.i62.i to <16 x i32>
  %42 = extractelement <16 x i32> %conv3.i63.i, i32 0
  %"&(pSB[currWI].offset)88717.i" = or i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset888.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)88717.i"
  %CastToValueType889.i = bitcast i8* %"&pSB[currWI].offset888.i" to i32*
  store i32 %42, i32* %CastToValueType889.i, align 4
  %extract65.i = sext i32 %42 to i64
  %43 = getelementptr inbounds float addrspace(3)* %7, i64 %extract65.i
  %ptrTypeCast81.i = bitcast float addrspace(3)* %43 to <16 x float> addrspace(3)*
  %"&(pSB[currWI].offset)90118.i" = or i64 %CurrSBIndex..0.i, 32
  %"&pSB[currWI].offset902.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)90118.i"
  %CastToValueType903.i = bitcast i8* %"&pSB[currWI].offset902.i" to <16 x float> addrspace(3)**
  store <16 x float> addrspace(3)* %ptrTypeCast81.i, <16 x float> addrspace(3)** %CastToValueType903.i, align 8
  store <16 x float> %merge38.i, <16 x float> addrspace(3)* %ptrTypeCast81.i, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %if.end.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 128
  br label %SyncBB973.i

elseBB.i:                                         ; preds = %if.end.i
  %cmp1.i.i = icmp ugt i64 %41, 1
  %negIncomingLoopMask.i = xor i1 %cmp1.i.i, true
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB978.i, %thenBB993.i, %thenBB985.i, %elseBB.i
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride984.i", %thenBB978.i ], [ %"loadedCurrSB+Stride991.i", %thenBB985.i ], [ %"loadedCurrSB+Stride999.i", %thenBB993.i ]
  %currBarrier.0.i = phi i32 [ 11, %elseBB.i ], [ %currBarrier.6.i, %thenBB978.i ], [ %currBarrier.1.i, %thenBB985.i ], [ %currBarrier.3.i, %thenBB993.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++982.i", %thenBB978.i ], [ %"CurrWI++989.i", %thenBB985.i ], [ %"CurrWI++997.i", %thenBB993.i ]
  br i1 %cmp1.i.i, label %for.body.i.i, label %if.then8.i

for.body.i.i:                                     ; preds = %postload348.i, %SyncBB.i
  %CurrSBIndex..2.i = phi i64 [ %CurrSBIndex..6.i, %postload348.i ], [ %CurrSBIndex..1.i, %SyncBB.i ]
  %currBarrier.1.i = phi i32 [ %currBarrier.5.i, %postload348.i ], [ %currBarrier.0.i, %SyncBB.i ]
  %CurrWI..2.i = phi i64 [ %CurrWI..6.i, %postload348.i ], [ %CurrWI..1.i, %SyncBB.i ]
  %for.body.i_loop_mask.0.i = phi i1 [ %loop_mask3.i, %postload348.i ], [ %negIncomingLoopMask.i, %SyncBB.i ]
  %for.body.i_Min.i = phi i1 [ %local_edge.i, %postload348.i ], [ %cmp1.i.i, %SyncBB.i ]
  %loadedValue955.i = phi i32 [ %mul.i.i, %postload348.i ], [ 1, %SyncBB.i ]
  %"&(pSB[currWI].offset)948.i" = add nuw i64 %CurrSBIndex..2.i, 44
  %"&pSB[currWI].offset949.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)948.i"
  %CastToValueType950.i = bitcast i8* %"&pSB[currWI].offset949.i" to i32*
  store i32 %loadedValue955.i, i32* %CastToValueType950.i, align 4
  %"&(pSB[currWI].offset)924.i" = add nuw i64 %CurrSBIndex..2.i, 41
  %"&pSB[currWI].offset925.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)924.i"
  %CastToValueType926.i = bitcast i8* %"&pSB[currWI].offset925.i" to i1*
  store i1 %for.body.i_Min.i, i1* %CastToValueType926.i, align 1
  %"&(pSB[currWI].offset)915.i" = add nuw i64 %CurrSBIndex..2.i, 40
  %"&pSB[currWI].offset916.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)915.i"
  %CastToValueType917.i = bitcast i8* %"&pSB[currWI].offset916.i" to i1*
  store i1 %for.body.i_loop_mask.0.i, i1* %CastToValueType917.i, align 1
  br i1 %for.body.i_Min.i, label %preload341.i, label %postload342.i

preload341.i:                                     ; preds = %for.body.i.i
  %"&(pSB[currWI].offset)891.i" = add nuw i64 %CurrSBIndex..2.i, 24
  %"&pSB[currWI].offset892.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)891.i"
  %CastToValueType893.i = bitcast i8* %"&pSB[currWI].offset892.i" to i32*
  %loadedValue894.i = load i32* %CastToValueType893.i, align 4
  %44 = sub i32 %loadedValue894.i, %loadedValue955.i
  %extract86.i = sext i32 %44 to i64
  %45 = getelementptr inbounds float addrspace(3)* %7, i64 %extract86.i
  %ptrTypeCast102.i = bitcast float addrspace(3)* %45 to <16 x float> addrspace(3)*
  %masked_load.i = load <16 x float> addrspace(3)* %ptrTypeCast102.i, align 4
  %"&(pSB[currWI].offset)962.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset963.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)962.i"
  %CastToValueType964.i = bitcast i8* %"&pSB[currWI].offset963.i" to <16 x float>*
  store <16 x float> %masked_load.i, <16 x float>* %CastToValueType964.i, align 64
  %check.WI.iter988.i = icmp ult i64 %CurrWI..2.i, %16
  br i1 %check.WI.iter988.i, label %thenBB985.i, label %SyncBB975.i

thenBB985.i:                                      ; preds = %preload341.i
  %"CurrWI++989.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride991.i" = add nuw i64 %CurrSBIndex..2.i, 128
  switch i32 %currBarrier.1.i, label %SyncBB.i [
    i32 19, label %thenBB985.i.postload348.i_crit_edge
    i32 18, label %SyncBB975.i
  ]

thenBB985.i.postload348.i_crit_edge:              ; preds = %thenBB985.i
  br label %postload348.i

SyncBB975.i:                                      ; preds = %thenBB978.i, %thenBB993.i, %thenBB985.i, %preload341.i
  %CurrSBIndex..3.i = phi i64 [ %"loadedCurrSB+Stride984.i", %thenBB978.i ], [ %"loadedCurrSB+Stride999.i", %thenBB993.i ], [ %"loadedCurrSB+Stride991.i", %thenBB985.i ], [ 0, %preload341.i ]
  %currBarrier.2.i = phi i32 [ %currBarrier.6.i, %thenBB978.i ], [ %currBarrier.3.i, %thenBB993.i ], [ %currBarrier.1.i, %thenBB985.i ], [ 18, %preload341.i ]
  %CurrWI..3.i = phi i64 [ %"CurrWI++982.i", %thenBB978.i ], [ %"CurrWI++997.i", %thenBB993.i ], [ %"CurrWI++989.i", %thenBB985.i ], [ 0, %preload341.i ]
  %"&(pSB[currWI].offset)966.i" = add nuw i64 %CurrSBIndex..3.i, 64
  %"&pSB[currWI].offset967.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)966.i"
  %CastToValueType968.i = bitcast i8* %"&pSB[currWI].offset967.i" to <16 x float>*
  %loadedValue969.i = load <16 x float>* %CastToValueType968.i, align 64
  br label %postload342.i

postload342.i:                                    ; preds = %SyncBB975.i, %for.body.i.i
  %CurrSBIndex..4.i = phi i64 [ %CurrSBIndex..3.i, %SyncBB975.i ], [ %CurrSBIndex..2.i, %for.body.i.i ]
  %currBarrier.3.i = phi i32 [ %currBarrier.2.i, %SyncBB975.i ], [ %currBarrier.1.i, %for.body.i.i ]
  %CurrWI..4.i = phi i64 [ %CurrWI..3.i, %SyncBB975.i ], [ %CurrWI..2.i, %for.body.i.i ]
  %phi343.i = phi <16 x float> [ %loadedValue969.i, %SyncBB975.i ], [ undef, %for.body.i.i ]
  %"&(pSB[currWI].offset)933.i" = add nuw i64 %CurrSBIndex..4.i, 41
  %"&pSB[currWI].offset934.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)933.i"
  %CastToValueType935.i = bitcast i8* %"&pSB[currWI].offset934.i" to i1*
  %loadedValue936.i = load i1* %CastToValueType935.i, align 1
  br i1 %loadedValue936.i, label %preload344.i, label %postload345.i

preload344.i:                                     ; preds = %postload342.i
  %"&(pSB[currWI].offset)910.i" = add nuw i64 %CurrSBIndex..4.i, 32
  %"&pSB[currWI].offset911.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)910.i"
  %CastToValueType912.i = bitcast i8* %"&pSB[currWI].offset911.i" to <16 x float> addrspace(3)**
  %loadedValue913.i = load <16 x float> addrspace(3)** %CastToValueType912.i, align 8
  %masked_load189.i = load <16 x float> addrspace(3)* %loadedValue913.i, align 4
  br label %postload345.i

postload345.i:                                    ; preds = %preload344.i, %postload342.i
  %phi346.i = phi <16 x float> [ undef, %postload342.i ], [ %masked_load189.i, %preload344.i ]
  br i1 %loadedValue936.i, label %preload347.i, label %postload348.i

preload347.i:                                     ; preds = %postload345.i
  %add13.i104.i = fadd <16 x float> %phi346.i, %phi343.i
  %"&(pSB[currWI].offset)905.i" = add nuw i64 %CurrSBIndex..4.i, 32
  %"&pSB[currWI].offset906.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)905.i"
  %CastToValueType907.i = bitcast i8* %"&pSB[currWI].offset906.i" to <16 x float> addrspace(3)**
  %loadedValue908.i = load <16 x float> addrspace(3)** %CastToValueType907.i, align 8
  store <16 x float> %add13.i104.i, <16 x float> addrspace(3)* %loadedValue908.i, align 4
  %check.WI.iter996.i = icmp ult i64 %CurrWI..4.i, %16
  br i1 %check.WI.iter996.i, label %thenBB993.i, label %preload347.i.postload348.i_crit_edge

preload347.i.postload348.i_crit_edge:             ; preds = %preload347.i
  br label %postload348.i

thenBB993.i:                                      ; preds = %preload347.i
  %"CurrWI++997.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride999.i" = add nuw i64 %CurrSBIndex..4.i, 128
  switch i32 %currBarrier.3.i, label %SyncBB975.i [
    i32 11, label %SyncBB.i
    i32 19, label %thenBB993.i.postload348.i_crit_edge
  ]

thenBB993.i.postload348.i_crit_edge:              ; preds = %thenBB993.i
  br label %postload348.i

postload348.i:                                    ; preds = %thenBB978.i.postload348.i_crit_edge, %thenBB993.i.postload348.i_crit_edge, %preload347.i.postload348.i_crit_edge, %thenBB985.i.postload348.i_crit_edge, %postload345.i
  %CurrSBIndex..6.i = phi i64 [ %CurrSBIndex..4.i, %postload345.i ], [ %"loadedCurrSB+Stride991.i", %thenBB985.i.postload348.i_crit_edge ], [ 0, %preload347.i.postload348.i_crit_edge ], [ %"loadedCurrSB+Stride999.i", %thenBB993.i.postload348.i_crit_edge ], [ %"loadedCurrSB+Stride984.i", %thenBB978.i.postload348.i_crit_edge ]
  %currBarrier.5.i = phi i32 [ %currBarrier.3.i, %postload345.i ], [ %currBarrier.1.i, %thenBB985.i.postload348.i_crit_edge ], [ 19, %preload347.i.postload348.i_crit_edge ], [ %currBarrier.3.i, %thenBB993.i.postload348.i_crit_edge ], [ %currBarrier.6.i, %thenBB978.i.postload348.i_crit_edge ]
  %CurrWI..6.i = phi i64 [ %CurrWI..4.i, %postload345.i ], [ %"CurrWI++989.i", %thenBB985.i.postload348.i_crit_edge ], [ 0, %preload347.i.postload348.i_crit_edge ], [ %"CurrWI++997.i", %thenBB993.i.postload348.i_crit_edge ], [ %"CurrWI++982.i", %thenBB978.i.postload348.i_crit_edge ]
  %"&(pSB[currWI].offset)957.i" = add nuw i64 %CurrSBIndex..6.i, 44
  %"&pSB[currWI].offset958.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)957.i"
  %CastToValueType959.i = bitcast i8* %"&pSB[currWI].offset958.i" to i32*
  %loadedValue960.i = load i32* %CastToValueType959.i, align 4
  %mul.i.i = shl nsw i32 %loadedValue960.i, 1
  %conv6.i.i = sext i32 %mul.i.i to i64
  %cmp.i.i = icmp ult i64 %conv6.i.i, %41
  %notCond.i = xor i1 %cmp.i.i, true
  %"&(pSB[currWI].offset)938.i" = add nuw i64 %CurrSBIndex..6.i, 41
  %"&pSB[currWI].offset939.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)938.i"
  %CastToValueType940.i = bitcast i8* %"&pSB[currWI].offset939.i" to i1*
  %loadedValue941.i = load i1* %CastToValueType940.i, align 1
  %who_left_tr.i = and i1 %loadedValue941.i, %notCond.i
  %"&(pSB[currWI].offset)919.i" = add nuw i64 %CurrSBIndex..6.i, 40
  %"&pSB[currWI].offset920.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)919.i"
  %CastToValueType921.i = bitcast i8* %"&pSB[currWI].offset920.i" to i1*
  %loadedValue922.i = load i1* %CastToValueType921.i, align 1
  %loop_mask3.i = or i1 %loadedValue922.i, %who_left_tr.i
  %local_edge.i = and i1 %loadedValue941.i, %cmp.i.i
  br i1 %loop_mask3.i, label %if.then8.i, label %for.body.i.i

if.then8.i:                                       ; preds = %postload348.i, %SyncBB.i
  %CurrSBIndex..7.i = phi i64 [ %CurrSBIndex..1.i, %SyncBB.i ], [ %CurrSBIndex..6.i, %postload348.i ]
  %currBarrier.6.i = phi i32 [ %currBarrier.0.i, %SyncBB.i ], [ %currBarrier.5.i, %postload348.i ]
  %CurrWI..7.i = phi i64 [ %CurrWI..1.i, %SyncBB.i ], [ %CurrWI..6.i, %postload348.i ]
  %"&(pSB[currWI].offset)896.i" = add nuw i64 %CurrSBIndex..7.i, 24
  %"&pSB[currWI].offset897.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)896.i"
  %CastToValueType898.i = bitcast i8* %"&pSB[currWI].offset897.i" to i32*
  %loadedValue899.i = load i32* %CastToValueType898.i, align 4
  %46 = add i32 %loadedValue899.i, -1
  %extract109.i = sext i32 %46 to i64
  %"&(pSB[currWI].offset)672.i" = add nuw i64 %CurrSBIndex..7.i, 8
  %"&pSB[currWI].offset673.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)672.i"
  %CastToValueType674.i = bitcast i8* %"&pSB[currWI].offset673.i" to i1*
  %loadedValue675.i = load i1* %CastToValueType674.i, align 1
  br i1 %loadedValue675.i, label %preload317.i, label %postload318.i

preload317.i:                                     ; preds = %if.then8.i
  %47 = getelementptr inbounds float addrspace(3)* %7, i64 %extract109.i
  %vload193.i = load float addrspace(3)* %47, align 4
  br label %postload318.i

postload318.i:                                    ; preds = %preload317.i, %if.then8.i
  %phi319.i = phi float [ undef, %if.then8.i ], [ %vload193.i, %preload317.i ]
  %"&(pSB[currWI].offset)686.i" = add nuw i64 %CurrSBIndex..7.i, 9
  %"&pSB[currWI].offset687.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)686.i"
  %CastToValueType688.i = bitcast i8* %"&pSB[currWI].offset687.i" to i1*
  %loadedValue689.i = load i1* %CastToValueType688.i, align 1
  br i1 %loadedValue689.i, label %preload373.i, label %postload374.i

preload373.i:                                     ; preds = %postload318.i
  %.sum480.i = add i64 %extract109.i, 1
  %48 = getelementptr float addrspace(3)* %7, i64 %.sum480.i
  %vload197.i = load float addrspace(3)* %48, align 4
  br label %postload374.i

postload374.i:                                    ; preds = %preload373.i, %postload318.i
  %phi375.i = phi float [ undef, %postload318.i ], [ %vload197.i, %preload373.i ]
  %"&(pSB[currWI].offset)700.i" = add nuw i64 %CurrSBIndex..7.i, 10
  %"&pSB[currWI].offset701.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)700.i"
  %CastToValueType702.i = bitcast i8* %"&pSB[currWI].offset701.i" to i1*
  %loadedValue703.i = load i1* %CastToValueType702.i, align 1
  br i1 %loadedValue703.i, label %preload305.i, label %postload306.i

preload305.i:                                     ; preds = %postload374.i
  %.sum479.i = add i64 %extract109.i, 2
  %49 = getelementptr float addrspace(3)* %7, i64 %.sum479.i
  %vload201.i = load float addrspace(3)* %49, align 4
  br label %postload306.i

postload306.i:                                    ; preds = %preload305.i, %postload374.i
  %phi307.i = phi float [ undef, %postload374.i ], [ %vload201.i, %preload305.i ]
  %"&(pSB[currWI].offset)714.i" = add nuw i64 %CurrSBIndex..7.i, 11
  %"&pSB[currWI].offset715.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)714.i"
  %CastToValueType716.i = bitcast i8* %"&pSB[currWI].offset715.i" to i1*
  %loadedValue717.i = load i1* %CastToValueType716.i, align 1
  br i1 %loadedValue717.i, label %preload412.i, label %postload413.i

preload412.i:                                     ; preds = %postload306.i
  %.sum478.i = add i64 %extract109.i, 3
  %50 = getelementptr float addrspace(3)* %7, i64 %.sum478.i
  %vload205.i = load float addrspace(3)* %50, align 4
  br label %postload413.i

postload413.i:                                    ; preds = %preload412.i, %postload306.i
  %phi414.i = phi float [ undef, %postload306.i ], [ %vload205.i, %preload412.i ]
  %"&(pSB[currWI].offset)728.i" = add nuw i64 %CurrSBIndex..7.i, 12
  %"&pSB[currWI].offset729.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)728.i"
  %CastToValueType730.i = bitcast i8* %"&pSB[currWI].offset729.i" to i1*
  %loadedValue731.i = load i1* %CastToValueType730.i, align 1
  br i1 %loadedValue731.i, label %preload415.i, label %postload416.i

preload415.i:                                     ; preds = %postload413.i
  %.sum477.i = add i64 %extract109.i, 4
  %51 = getelementptr float addrspace(3)* %7, i64 %.sum477.i
  %vload209.i = load float addrspace(3)* %51, align 4
  br label %postload416.i

postload416.i:                                    ; preds = %preload415.i, %postload413.i
  %phi417.i = phi float [ undef, %postload413.i ], [ %vload209.i, %preload415.i ]
  %"&(pSB[currWI].offset)742.i" = add nuw i64 %CurrSBIndex..7.i, 13
  %"&pSB[currWI].offset743.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)742.i"
  %CastToValueType744.i = bitcast i8* %"&pSB[currWI].offset743.i" to i1*
  %loadedValue745.i = load i1* %CastToValueType744.i, align 1
  br i1 %loadedValue745.i, label %preload406.i, label %postload407.i

preload406.i:                                     ; preds = %postload416.i
  %.sum476.i = add i64 %extract109.i, 5
  %52 = getelementptr float addrspace(3)* %7, i64 %.sum476.i
  %vload213.i = load float addrspace(3)* %52, align 4
  br label %postload407.i

postload407.i:                                    ; preds = %preload406.i, %postload416.i
  %phi408.i = phi float [ undef, %postload416.i ], [ %vload213.i, %preload406.i ]
  %"&(pSB[currWI].offset)756.i" = add nuw i64 %CurrSBIndex..7.i, 14
  %"&pSB[currWI].offset757.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)756.i"
  %CastToValueType758.i = bitcast i8* %"&pSB[currWI].offset757.i" to i1*
  %loadedValue759.i = load i1* %CastToValueType758.i, align 1
  br i1 %loadedValue759.i, label %preload409.i, label %postload410.i

preload409.i:                                     ; preds = %postload407.i
  %.sum475.i = add i64 %extract109.i, 6
  %53 = getelementptr float addrspace(3)* %7, i64 %.sum475.i
  %vload217.i = load float addrspace(3)* %53, align 4
  br label %postload410.i

postload410.i:                                    ; preds = %preload409.i, %postload407.i
  %phi411.i = phi float [ undef, %postload407.i ], [ %vload217.i, %preload409.i ]
  %"&(pSB[currWI].offset)770.i" = add nuw i64 %CurrSBIndex..7.i, 15
  %"&pSB[currWI].offset771.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)770.i"
  %CastToValueType772.i = bitcast i8* %"&pSB[currWI].offset771.i" to i1*
  %loadedValue773.i = load i1* %CastToValueType772.i, align 1
  br i1 %loadedValue773.i, label %preload391.i, label %postload392.i

preload391.i:                                     ; preds = %postload410.i
  %.sum474.i = add i64 %extract109.i, 7
  %54 = getelementptr float addrspace(3)* %7, i64 %.sum474.i
  %vload221.i = load float addrspace(3)* %54, align 4
  br label %postload392.i

postload392.i:                                    ; preds = %preload391.i, %postload410.i
  %phi393.i = phi float [ undef, %postload410.i ], [ %vload221.i, %preload391.i ]
  %"&(pSB[currWI].offset)784.i" = add nuw i64 %CurrSBIndex..7.i, 16
  %"&pSB[currWI].offset785.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)784.i"
  %CastToValueType786.i = bitcast i8* %"&pSB[currWI].offset785.i" to i1*
  %loadedValue787.i = load i1* %CastToValueType786.i, align 1
  br i1 %loadedValue787.i, label %preload394.i, label %postload395.i

preload394.i:                                     ; preds = %postload392.i
  %.sum473.i = add i64 %extract109.i, 8
  %55 = getelementptr float addrspace(3)* %7, i64 %.sum473.i
  %vload225.i = load float addrspace(3)* %55, align 4
  br label %postload395.i

postload395.i:                                    ; preds = %preload394.i, %postload392.i
  %phi396.i = phi float [ undef, %postload392.i ], [ %vload225.i, %preload394.i ]
  %"&(pSB[currWI].offset)798.i" = add nuw i64 %CurrSBIndex..7.i, 17
  %"&pSB[currWI].offset799.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)798.i"
  %CastToValueType800.i = bitcast i8* %"&pSB[currWI].offset799.i" to i1*
  %loadedValue801.i = load i1* %CastToValueType800.i, align 1
  br i1 %loadedValue801.i, label %preload397.i, label %postload398.i

preload397.i:                                     ; preds = %postload395.i
  %.sum472.i = add i64 %extract109.i, 9
  %56 = getelementptr float addrspace(3)* %7, i64 %.sum472.i
  %vload229.i = load float addrspace(3)* %56, align 4
  br label %postload398.i

postload398.i:                                    ; preds = %preload397.i, %postload395.i
  %phi399.i = phi float [ undef, %postload395.i ], [ %vload229.i, %preload397.i ]
  %"&(pSB[currWI].offset)812.i" = add nuw i64 %CurrSBIndex..7.i, 18
  %"&pSB[currWI].offset813.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)812.i"
  %CastToValueType814.i = bitcast i8* %"&pSB[currWI].offset813.i" to i1*
  %loadedValue815.i = load i1* %CastToValueType814.i, align 1
  br i1 %loadedValue815.i, label %preload400.i, label %postload401.i

preload400.i:                                     ; preds = %postload398.i
  %.sum471.i = add i64 %extract109.i, 10
  %57 = getelementptr float addrspace(3)* %7, i64 %.sum471.i
  %vload233.i = load float addrspace(3)* %57, align 4
  br label %postload401.i

postload401.i:                                    ; preds = %preload400.i, %postload398.i
  %phi402.i = phi float [ undef, %postload398.i ], [ %vload233.i, %preload400.i ]
  %"&(pSB[currWI].offset)826.i" = add nuw i64 %CurrSBIndex..7.i, 19
  %"&pSB[currWI].offset827.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)826.i"
  %CastToValueType828.i = bitcast i8* %"&pSB[currWI].offset827.i" to i1*
  %loadedValue829.i = load i1* %CastToValueType828.i, align 1
  br i1 %loadedValue829.i, label %preload403.i, label %postload404.i

preload403.i:                                     ; preds = %postload401.i
  %.sum470.i = add i64 %extract109.i, 11
  %58 = getelementptr float addrspace(3)* %7, i64 %.sum470.i
  %vload237.i = load float addrspace(3)* %58, align 4
  br label %postload404.i

postload404.i:                                    ; preds = %preload403.i, %postload401.i
  %phi405.i = phi float [ undef, %postload401.i ], [ %vload237.i, %preload403.i ]
  %"&(pSB[currWI].offset)840.i" = add nuw i64 %CurrSBIndex..7.i, 20
  %"&pSB[currWI].offset841.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)840.i"
  %CastToValueType842.i = bitcast i8* %"&pSB[currWI].offset841.i" to i1*
  %loadedValue843.i = load i1* %CastToValueType842.i, align 1
  br i1 %loadedValue843.i, label %preload376.i, label %postload377.i

preload376.i:                                     ; preds = %postload404.i
  %.sum469.i = add i64 %extract109.i, 12
  %59 = getelementptr float addrspace(3)* %7, i64 %.sum469.i
  %vload241.i = load float addrspace(3)* %59, align 4
  br label %postload377.i

postload377.i:                                    ; preds = %preload376.i, %postload404.i
  %phi378.i = phi float [ undef, %postload404.i ], [ %vload241.i, %preload376.i ]
  %"&(pSB[currWI].offset)854.i" = add nuw i64 %CurrSBIndex..7.i, 21
  %"&pSB[currWI].offset855.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)854.i"
  %CastToValueType856.i = bitcast i8* %"&pSB[currWI].offset855.i" to i1*
  %loadedValue857.i = load i1* %CastToValueType856.i, align 1
  br i1 %loadedValue857.i, label %preload379.i, label %postload380.i

preload379.i:                                     ; preds = %postload377.i
  %.sum468.i = add i64 %extract109.i, 13
  %60 = getelementptr float addrspace(3)* %7, i64 %.sum468.i
  %vload245.i = load float addrspace(3)* %60, align 4
  br label %postload380.i

postload380.i:                                    ; preds = %preload379.i, %postload377.i
  %phi381.i = phi float [ undef, %postload377.i ], [ %vload245.i, %preload379.i ]
  %"&(pSB[currWI].offset)868.i" = add nuw i64 %CurrSBIndex..7.i, 22
  %"&pSB[currWI].offset869.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)868.i"
  %CastToValueType870.i = bitcast i8* %"&pSB[currWI].offset869.i" to i1*
  %loadedValue871.i = load i1* %CastToValueType870.i, align 1
  br i1 %loadedValue871.i, label %preload382.i, label %postload383.i

preload382.i:                                     ; preds = %postload380.i
  %.sum467.i = add i64 %extract109.i, 14
  %61 = getelementptr float addrspace(3)* %7, i64 %.sum467.i
  %vload249.i = load float addrspace(3)* %61, align 4
  br label %postload383.i

postload383.i:                                    ; preds = %preload382.i, %postload380.i
  %phi384.i = phi float [ undef, %postload380.i ], [ %vload249.i, %preload382.i ]
  %"&(pSB[currWI].offset)882.i" = add nuw i64 %CurrSBIndex..7.i, 23
  %"&pSB[currWI].offset883.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)882.i"
  %CastToValueType884.i = bitcast i8* %"&pSB[currWI].offset883.i" to i1*
  %loadedValue885.i = load i1* %CastToValueType884.i, align 1
  br i1 %loadedValue885.i, label %preload385.i, label %postload386.i

preload385.i:                                     ; preds = %postload383.i
  %.sum466.i = add i64 %extract109.i, 15
  %62 = getelementptr float addrspace(3)* %7, i64 %.sum466.i
  %vload253.i = load float addrspace(3)* %62, align 4
  br label %postload386.i

postload386.i:                                    ; preds = %preload385.i, %postload383.i
  %phi387.i = phi float [ undef, %postload383.i ], [ %vload253.i, %preload385.i ]
  br i1 %loadedValue675.i, label %preload388.i, label %postload389.i

preload388.i:                                     ; preds = %postload386.i
  %"&pSB[currWI].offset654.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType655.i = bitcast i8* %"&pSB[currWI].offset654.i" to i64*
  %loadedValue656.i = load i64* %CastToValueType655.i, align 8
  %63 = getelementptr inbounds float addrspace(1)* %1, i64 %loadedValue656.i
  store float %phi319.i, float addrspace(1)* %63, align 4
  %loadedValue684.pre.i = load i1* %CastToValueType688.i, align 1
  br label %postload389.i

postload389.i:                                    ; preds = %preload388.i, %postload386.i
  %loadedValue684.i = phi i1 [ %loadedValue684.pre.i, %preload388.i ], [ %loadedValue689.i, %postload386.i ]
  br i1 %loadedValue684.i, label %preload421.i, label %postload389.i.postload422.i_crit_edge

postload389.i.postload422.i_crit_edge:            ; preds = %postload389.i
  br label %postload422.i

preload421.i:                                     ; preds = %postload389.i
  %"&pSB[currWI].offset579.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType580.i = bitcast i8* %"&pSB[currWI].offset579.i" to i64*
  %loadedValue581.i = load i64* %CastToValueType580.i, align 8
  %.sum465.i = add i64 %loadedValue581.i, 1
  %64 = getelementptr float addrspace(1)* %1, i64 %.sum465.i
  store float %phi375.i, float addrspace(1)* %64, align 4
  br label %postload422.i

postload422.i:                                    ; preds = %postload389.i.postload422.i_crit_edge, %preload421.i
  %loadedValue698.i = load i1* %CastToValueType702.i, align 1
  br i1 %loadedValue698.i, label %preload424.i, label %postload422.i.postload425.i_crit_edge

postload422.i.postload425.i_crit_edge:            ; preds = %postload422.i
  br label %postload425.i

preload424.i:                                     ; preds = %postload422.i
  %"&pSB[currWI].offset584.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType585.i = bitcast i8* %"&pSB[currWI].offset584.i" to i64*
  %loadedValue586.i = load i64* %CastToValueType585.i, align 8
  %.sum464.i = add i64 %loadedValue586.i, 2
  %65 = getelementptr float addrspace(1)* %1, i64 %.sum464.i
  store float %phi307.i, float addrspace(1)* %65, align 4
  br label %postload425.i

postload425.i:                                    ; preds = %postload422.i.postload425.i_crit_edge, %preload424.i
  %loadedValue712.i = load i1* %CastToValueType716.i, align 1
  br i1 %loadedValue712.i, label %preload427.i, label %postload425.i.postload428.i_crit_edge

postload425.i.postload428.i_crit_edge:            ; preds = %postload425.i
  br label %postload428.i

preload427.i:                                     ; preds = %postload425.i
  %"&pSB[currWI].offset589.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType590.i = bitcast i8* %"&pSB[currWI].offset589.i" to i64*
  %loadedValue591.i = load i64* %CastToValueType590.i, align 8
  %.sum463.i = add i64 %loadedValue591.i, 3
  %66 = getelementptr float addrspace(1)* %1, i64 %.sum463.i
  store float %phi414.i, float addrspace(1)* %66, align 4
  br label %postload428.i

postload428.i:                                    ; preds = %postload425.i.postload428.i_crit_edge, %preload427.i
  %loadedValue726.i = load i1* %CastToValueType730.i, align 1
  br i1 %loadedValue726.i, label %preload430.i, label %postload428.i.postload431.i_crit_edge

postload428.i.postload431.i_crit_edge:            ; preds = %postload428.i
  br label %postload431.i

preload430.i:                                     ; preds = %postload428.i
  %"&pSB[currWI].offset594.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType595.i = bitcast i8* %"&pSB[currWI].offset594.i" to i64*
  %loadedValue596.i = load i64* %CastToValueType595.i, align 8
  %.sum462.i = add i64 %loadedValue596.i, 4
  %67 = getelementptr float addrspace(1)* %1, i64 %.sum462.i
  store float %phi417.i, float addrspace(1)* %67, align 4
  br label %postload431.i

postload431.i:                                    ; preds = %postload428.i.postload431.i_crit_edge, %preload430.i
  %loadedValue740.i = load i1* %CastToValueType744.i, align 1
  br i1 %loadedValue740.i, label %preload433.i, label %postload431.i.postload434.i_crit_edge

postload431.i.postload434.i_crit_edge:            ; preds = %postload431.i
  br label %postload434.i

preload433.i:                                     ; preds = %postload431.i
  %"&pSB[currWI].offset599.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType600.i = bitcast i8* %"&pSB[currWI].offset599.i" to i64*
  %loadedValue601.i = load i64* %CastToValueType600.i, align 8
  %.sum461.i = add i64 %loadedValue601.i, 5
  %68 = getelementptr float addrspace(1)* %1, i64 %.sum461.i
  store float %phi408.i, float addrspace(1)* %68, align 4
  br label %postload434.i

postload434.i:                                    ; preds = %postload431.i.postload434.i_crit_edge, %preload433.i
  %loadedValue754.i = load i1* %CastToValueType758.i, align 1
  br i1 %loadedValue754.i, label %preload436.i, label %postload434.i.postload437.i_crit_edge

postload434.i.postload437.i_crit_edge:            ; preds = %postload434.i
  br label %postload437.i

preload436.i:                                     ; preds = %postload434.i
  %"&pSB[currWI].offset604.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType605.i = bitcast i8* %"&pSB[currWI].offset604.i" to i64*
  %loadedValue606.i = load i64* %CastToValueType605.i, align 8
  %.sum460.i = add i64 %loadedValue606.i, 6
  %69 = getelementptr float addrspace(1)* %1, i64 %.sum460.i
  store float %phi411.i, float addrspace(1)* %69, align 4
  br label %postload437.i

postload437.i:                                    ; preds = %postload434.i.postload437.i_crit_edge, %preload436.i
  %loadedValue768.i = load i1* %CastToValueType772.i, align 1
  br i1 %loadedValue768.i, label %preload439.i, label %postload437.i.postload440.i_crit_edge

postload437.i.postload440.i_crit_edge:            ; preds = %postload437.i
  br label %postload440.i

preload439.i:                                     ; preds = %postload437.i
  %"&pSB[currWI].offset609.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType610.i = bitcast i8* %"&pSB[currWI].offset609.i" to i64*
  %loadedValue611.i = load i64* %CastToValueType610.i, align 8
  %.sum459.i = add i64 %loadedValue611.i, 7
  %70 = getelementptr float addrspace(1)* %1, i64 %.sum459.i
  store float %phi393.i, float addrspace(1)* %70, align 4
  br label %postload440.i

postload440.i:                                    ; preds = %postload437.i.postload440.i_crit_edge, %preload439.i
  %loadedValue782.i = load i1* %CastToValueType786.i, align 1
  br i1 %loadedValue782.i, label %preload442.i, label %postload440.i.postload443.i_crit_edge

postload440.i.postload443.i_crit_edge:            ; preds = %postload440.i
  br label %postload443.i

preload442.i:                                     ; preds = %postload440.i
  %"&pSB[currWI].offset614.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType615.i = bitcast i8* %"&pSB[currWI].offset614.i" to i64*
  %loadedValue616.i = load i64* %CastToValueType615.i, align 8
  %.sum458.i = add i64 %loadedValue616.i, 8
  %71 = getelementptr float addrspace(1)* %1, i64 %.sum458.i
  store float %phi396.i, float addrspace(1)* %71, align 4
  br label %postload443.i

postload443.i:                                    ; preds = %postload440.i.postload443.i_crit_edge, %preload442.i
  %loadedValue796.i = load i1* %CastToValueType800.i, align 1
  br i1 %loadedValue796.i, label %preload358.i, label %postload443.i.postload359.i_crit_edge

postload443.i.postload359.i_crit_edge:            ; preds = %postload443.i
  br label %postload359.i

preload358.i:                                     ; preds = %postload443.i
  %"&pSB[currWI].offset619.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType620.i = bitcast i8* %"&pSB[currWI].offset619.i" to i64*
  %loadedValue621.i = load i64* %CastToValueType620.i, align 8
  %.sum457.i = add i64 %loadedValue621.i, 9
  %72 = getelementptr float addrspace(1)* %1, i64 %.sum457.i
  store float %phi399.i, float addrspace(1)* %72, align 4
  br label %postload359.i

postload359.i:                                    ; preds = %postload443.i.postload359.i_crit_edge, %preload358.i
  %loadedValue810.i = load i1* %CastToValueType814.i, align 1
  br i1 %loadedValue810.i, label %preload361.i, label %postload359.i.postload362.i_crit_edge

postload359.i.postload362.i_crit_edge:            ; preds = %postload359.i
  br label %postload362.i

preload361.i:                                     ; preds = %postload359.i
  %"&pSB[currWI].offset624.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType625.i = bitcast i8* %"&pSB[currWI].offset624.i" to i64*
  %loadedValue626.i = load i64* %CastToValueType625.i, align 8
  %.sum456.i = add i64 %loadedValue626.i, 10
  %73 = getelementptr float addrspace(1)* %1, i64 %.sum456.i
  store float %phi402.i, float addrspace(1)* %73, align 4
  br label %postload362.i

postload362.i:                                    ; preds = %postload359.i.postload362.i_crit_edge, %preload361.i
  %loadedValue824.i = load i1* %CastToValueType828.i, align 1
  br i1 %loadedValue824.i, label %preload364.i, label %postload362.i.postload365.i_crit_edge

postload362.i.postload365.i_crit_edge:            ; preds = %postload362.i
  br label %postload365.i

preload364.i:                                     ; preds = %postload362.i
  %"&pSB[currWI].offset629.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType630.i = bitcast i8* %"&pSB[currWI].offset629.i" to i64*
  %loadedValue631.i = load i64* %CastToValueType630.i, align 8
  %.sum455.i = add i64 %loadedValue631.i, 11
  %74 = getelementptr float addrspace(1)* %1, i64 %.sum455.i
  store float %phi405.i, float addrspace(1)* %74, align 4
  br label %postload365.i

postload365.i:                                    ; preds = %postload362.i.postload365.i_crit_edge, %preload364.i
  %loadedValue838.i = load i1* %CastToValueType842.i, align 1
  br i1 %loadedValue838.i, label %preload367.i, label %postload365.i.postload368.i_crit_edge

postload365.i.postload368.i_crit_edge:            ; preds = %postload365.i
  br label %postload368.i

preload367.i:                                     ; preds = %postload365.i
  %"&pSB[currWI].offset634.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType635.i = bitcast i8* %"&pSB[currWI].offset634.i" to i64*
  %loadedValue636.i = load i64* %CastToValueType635.i, align 8
  %.sum454.i = add i64 %loadedValue636.i, 12
  %75 = getelementptr float addrspace(1)* %1, i64 %.sum454.i
  store float %phi378.i, float addrspace(1)* %75, align 4
  br label %postload368.i

postload368.i:                                    ; preds = %postload365.i.postload368.i_crit_edge, %preload367.i
  %loadedValue852.i = load i1* %CastToValueType856.i, align 1
  br i1 %loadedValue852.i, label %preload370.i, label %postload368.i.postload371.i_crit_edge

postload368.i.postload371.i_crit_edge:            ; preds = %postload368.i
  br label %postload371.i

preload370.i:                                     ; preds = %postload368.i
  %"&pSB[currWI].offset639.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType640.i = bitcast i8* %"&pSB[currWI].offset639.i" to i64*
  %loadedValue641.i = load i64* %CastToValueType640.i, align 8
  %.sum453.i = add i64 %loadedValue641.i, 13
  %76 = getelementptr float addrspace(1)* %1, i64 %.sum453.i
  store float %phi381.i, float addrspace(1)* %76, align 4
  br label %postload371.i

postload371.i:                                    ; preds = %postload368.i.postload371.i_crit_edge, %preload370.i
  %loadedValue866.i = load i1* %CastToValueType870.i, align 1
  br i1 %loadedValue866.i, label %preload445.i, label %postload371.i.postload446.i_crit_edge

postload371.i.postload446.i_crit_edge:            ; preds = %postload371.i
  br label %postload446.i

preload445.i:                                     ; preds = %postload371.i
  %"&pSB[currWI].offset644.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType645.i = bitcast i8* %"&pSB[currWI].offset644.i" to i64*
  %loadedValue646.i = load i64* %CastToValueType645.i, align 8
  %.sum452.i = add i64 %loadedValue646.i, 14
  %77 = getelementptr float addrspace(1)* %1, i64 %.sum452.i
  store float %phi384.i, float addrspace(1)* %77, align 4
  br label %postload446.i

postload446.i:                                    ; preds = %postload371.i.postload446.i_crit_edge, %preload445.i
  %loadedValue880.i = load i1* %CastToValueType884.i, align 1
  br i1 %loadedValue880.i, label %preload448.i, label %if.end11.i

preload448.i:                                     ; preds = %postload446.i
  %"&pSB[currWI].offset649.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType650.i = bitcast i8* %"&pSB[currWI].offset649.i" to i64*
  %loadedValue651.i = load i64* %CastToValueType650.i, align 8
  %.sum.i = add i64 %loadedValue651.i, 15
  %78 = getelementptr float addrspace(1)* %1, i64 %.sum.i
  store float %phi387.i, float addrspace(1)* %78, align 4
  br label %if.end11.i

if.end11.i:                                       ; preds = %preload448.i, %postload446.i
  %check.WI.iter981.i = icmp ult i64 %CurrWI..7.i, %16
  br i1 %check.WI.iter981.i, label %thenBB978.i, label %____Vectorized_.top_scan_separated_args.exit

thenBB978.i:                                      ; preds = %if.end11.i
  %"CurrWI++982.i" = add nuw i64 %CurrWI..7.i, 1
  %"loadedCurrSB+Stride984.i" = add nuw i64 %CurrSBIndex..7.i, 128
  switch i32 %currBarrier.6.i, label %SyncBB.i [
    i32 19, label %thenBB978.i.postload348.i_crit_edge
    i32 18, label %SyncBB975.i
  ]

thenBB978.i.postload348.i_crit_edge:              ; preds = %thenBB978.i
  br label %postload348.i

____Vectorized_.top_scan_separated_args.exit:     ; preds = %if.end11.i
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
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to float addrspace(3)**
  %10 = load float addrspace(3)** %9, align 8
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
  %div.i = sdiv i32 %7, 4
  %conv.i = sext i32 %div.i to i64
  br label %SyncBB968.i

SyncBB968.i:                                      ; preds = %thenBB.i, %entry
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %26 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %13, i64 0, i32 4, i64 0
  %27 = load i64* %26, align 8
  %isDivisorZero707.i = icmp eq i64 %27, 0
  %newiDvisor709.i = select i1 %isDivisorZero707.i, i64 1, i64 %27
  %div1.i = udiv i64 %conv.i, %newiDvisor709.i
  %mul.i = shl i64 %div1.i, 2
  %conv2.i = trunc i64 %mul.i to i32
  %28 = load i64* %16, align 8
  %conv4.i = sext i32 %conv2.i to i64
  %mul5.i = mul i64 %conv4.i, %28
  %conv6.i = trunc i64 %mul5.i to i32
  %temp.i = insertelement <16 x i32> undef, i32 %conv6.i, i32 0
  %vector.i = shufflevector <16 x i32> %temp.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %sub.i = add i64 %27, -1
  %cmp.i = icmp eq i64 %28, %sub.i
  %add.i = add nsw i32 %conv6.i, %conv2.i
  %cond.i = select i1 %cmp.i, i32 %7, i32 %add.i
  %temp48.i = insertelement <16 x i32> undef, i32 %cond.i, i32 0
  %vector49.i = shufflevector <16 x i32> %temp48.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %29 = getelementptr <{ [4 x i64] }>* %19, i64 %CurrWI..0.i, i32 0, i64 0
  %30 = load i64* %29, align 8
  %broadcast1.i = insertelement <16 x i64> undef, i64 %30, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %31 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %conv1146.i = trunc <16 x i64> %31 to <16 x i32>
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to <16 x i32>*
  store <16 x i32> %conv1146.i, <16 x i32>* %CastToValueType.i, align 64
  %add1247.i = add nsw <16 x i32> %vector.i, %conv1146.i
  %cmp134.i = icmp slt <16 x i32> %add1247.i, %vector49.i
  %32 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %13, i64 0, i32 3, i64 0
  %33 = load i64* %32, align 8
  %temp109.i = insertelement <16 x i64> undef, i64 %33, i32 0
  %vector110.i = shufflevector <16 x i64> %temp109.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %ipred.i.i = bitcast <16 x i1> %cmp134.i to i16
  %val.i.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i.i, i16 %ipred.i.i) nounwind
  %34 = and i32 %val.i.i, 1
  %res.i.i = icmp eq i32 %34, 0
  br i1 %res.i.i, label %while.body.preheader.i, label %while.end.i

while.body.preheader.i:                           ; preds = %SyncBB968.i
  %negIncomingLoopMask53.i = xor <16 x i1> %cmp134.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  br label %while.body.i

while.body.i:                                     ; preds = %postload461.i, %while.body.preheader.i
  %vectorPHI.i = phi <16 x i1> [ %loop_mask1116.i, %postload461.i ], [ %negIncomingLoopMask53.i, %while.body.preheader.i ]
  %vectorPHI55.i = phi <16 x float> [ %out_sel107.i, %postload461.i ], [ undef, %while.body.preheader.i ]
  %vectorPHI56.i = phi <16 x i1> [ %local_edge135.i, %postload461.i ], [ %cmp134.i, %while.body.preheader.i ]
  %vectorPHI57.i = phi <16 x float> [ %add15106.i, %postload461.i ], [ zeroinitializer, %while.body.preheader.i ]
  %vectorPHI58.i = phi <16 x i32> [ %conv19112.i, %postload461.i ], [ %add1247.i, %while.body.preheader.i ]
  %extract75.i = extractelement <16 x i1> %vectorPHI56.i, i32 0
  %extract76.i = extractelement <16 x i1> %vectorPHI56.i, i32 1
  %extract77.i = extractelement <16 x i1> %vectorPHI56.i, i32 2
  %extract78.i = extractelement <16 x i1> %vectorPHI56.i, i32 3
  %extract79.i = extractelement <16 x i1> %vectorPHI56.i, i32 4
  %extract80.i = extractelement <16 x i1> %vectorPHI56.i, i32 5
  %extract81.i = extractelement <16 x i1> %vectorPHI56.i, i32 6
  %extract82.i = extractelement <16 x i1> %vectorPHI56.i, i32 7
  %extract83.i = extractelement <16 x i1> %vectorPHI56.i, i32 8
  %extract84.i = extractelement <16 x i1> %vectorPHI56.i, i32 9
  %extract85.i = extractelement <16 x i1> %vectorPHI56.i, i32 10
  %extract86.i = extractelement <16 x i1> %vectorPHI56.i, i32 11
  %extract87.i = extractelement <16 x i1> %vectorPHI56.i, i32 12
  %extract88.i = extractelement <16 x i1> %vectorPHI56.i, i32 13
  %extract89.i = extractelement <16 x i1> %vectorPHI56.i, i32 14
  %extract90.i = extractelement <16 x i1> %vectorPHI56.i, i32 15
  %idxprom59.i = sext <16 x i32> %vectorPHI58.i to <16 x i64>
  %extract60.i = extractelement <16 x i64> %idxprom59.i, i32 1
  %extract61.i = extractelement <16 x i64> %idxprom59.i, i32 2
  %extract62.i = extractelement <16 x i64> %idxprom59.i, i32 3
  %extract63.i = extractelement <16 x i64> %idxprom59.i, i32 4
  %extract64.i = extractelement <16 x i64> %idxprom59.i, i32 5
  %extract65.i = extractelement <16 x i64> %idxprom59.i, i32 6
  %extract66.i = extractelement <16 x i64> %idxprom59.i, i32 7
  %extract67.i = extractelement <16 x i64> %idxprom59.i, i32 8
  %extract68.i = extractelement <16 x i64> %idxprom59.i, i32 9
  %extract69.i = extractelement <16 x i64> %idxprom59.i, i32 10
  %extract70.i = extractelement <16 x i64> %idxprom59.i, i32 11
  %extract71.i = extractelement <16 x i64> %idxprom59.i, i32 12
  %extract72.i = extractelement <16 x i64> %idxprom59.i, i32 13
  %extract73.i = extractelement <16 x i64> %idxprom59.i, i32 14
  %extract74.i = extractelement <16 x i64> %idxprom59.i, i32 15
  %35 = getelementptr inbounds float addrspace(1)* %1, i64 %extract60.i
  %36 = getelementptr inbounds float addrspace(1)* %1, i64 %extract61.i
  %37 = getelementptr inbounds float addrspace(1)* %1, i64 %extract62.i
  %38 = getelementptr inbounds float addrspace(1)* %1, i64 %extract63.i
  %39 = getelementptr inbounds float addrspace(1)* %1, i64 %extract64.i
  %40 = getelementptr inbounds float addrspace(1)* %1, i64 %extract65.i
  %41 = getelementptr inbounds float addrspace(1)* %1, i64 %extract66.i
  %42 = getelementptr inbounds float addrspace(1)* %1, i64 %extract67.i
  %43 = getelementptr inbounds float addrspace(1)* %1, i64 %extract68.i
  %44 = getelementptr inbounds float addrspace(1)* %1, i64 %extract69.i
  %45 = getelementptr inbounds float addrspace(1)* %1, i64 %extract70.i
  %46 = getelementptr inbounds float addrspace(1)* %1, i64 %extract71.i
  %47 = getelementptr inbounds float addrspace(1)* %1, i64 %extract72.i
  %48 = getelementptr inbounds float addrspace(1)* %1, i64 %extract73.i
  %49 = getelementptr inbounds float addrspace(1)* %1, i64 %extract74.i
  br i1 %extract75.i, label %preload466.i, label %postload467.i

preload466.i:                                     ; preds = %while.body.i
  %extract.i = extractelement <16 x i64> %idxprom59.i, i32 0
  %50 = getelementptr inbounds float addrspace(1)* %1, i64 %extract.i
  %masked_load.i = load float addrspace(1)* %50, align 4
  br label %postload467.i

postload467.i:                                    ; preds = %preload466.i, %while.body.i
  %phi468.i = phi float [ undef, %while.body.i ], [ %masked_load.i, %preload466.i ]
  br i1 %extract76.i, label %preload475.i, label %postload476.i

preload475.i:                                     ; preds = %postload467.i
  %masked_load280.i = load float addrspace(1)* %35, align 4
  br label %postload476.i

postload476.i:                                    ; preds = %preload475.i, %postload467.i
  %phi477.i = phi float [ undef, %postload467.i ], [ %masked_load280.i, %preload475.i ]
  br i1 %extract77.i, label %preload478.i, label %postload479.i

preload478.i:                                     ; preds = %postload476.i
  %masked_load281.i = load float addrspace(1)* %36, align 4
  br label %postload479.i

postload479.i:                                    ; preds = %preload478.i, %postload476.i
  %phi480.i = phi float [ undef, %postload476.i ], [ %masked_load281.i, %preload478.i ]
  br i1 %extract78.i, label %preload484.i, label %postload485.i

preload484.i:                                     ; preds = %postload479.i
  %masked_load282.i = load float addrspace(1)* %37, align 4
  br label %postload485.i

postload485.i:                                    ; preds = %preload484.i, %postload479.i
  %phi486.i = phi float [ undef, %postload479.i ], [ %masked_load282.i, %preload484.i ]
  br i1 %extract79.i, label %preload487.i, label %postload488.i

preload487.i:                                     ; preds = %postload485.i
  %masked_load283.i = load float addrspace(1)* %38, align 4
  br label %postload488.i

postload488.i:                                    ; preds = %preload487.i, %postload485.i
  %phi489.i = phi float [ undef, %postload485.i ], [ %masked_load283.i, %preload487.i ]
  br i1 %extract80.i, label %preload490.i, label %postload491.i

preload490.i:                                     ; preds = %postload488.i
  %masked_load284.i = load float addrspace(1)* %39, align 4
  br label %postload491.i

postload491.i:                                    ; preds = %preload490.i, %postload488.i
  %phi492.i = phi float [ undef, %postload488.i ], [ %masked_load284.i, %preload490.i ]
  br i1 %extract81.i, label %preload493.i, label %postload494.i

preload493.i:                                     ; preds = %postload491.i
  %masked_load285.i = load float addrspace(1)* %40, align 4
  br label %postload494.i

postload494.i:                                    ; preds = %preload493.i, %postload491.i
  %phi495.i = phi float [ undef, %postload491.i ], [ %masked_load285.i, %preload493.i ]
  br i1 %extract82.i, label %preload496.i, label %postload497.i

preload496.i:                                     ; preds = %postload494.i
  %masked_load286.i = load float addrspace(1)* %41, align 4
  br label %postload497.i

postload497.i:                                    ; preds = %preload496.i, %postload494.i
  %phi498.i = phi float [ undef, %postload494.i ], [ %masked_load286.i, %preload496.i ]
  br i1 %extract83.i, label %preload439.i, label %postload440.i

preload439.i:                                     ; preds = %postload497.i
  %masked_load287.i = load float addrspace(1)* %42, align 4
  br label %postload440.i

postload440.i:                                    ; preds = %preload439.i, %postload497.i
  %phi441.i = phi float [ undef, %postload497.i ], [ %masked_load287.i, %preload439.i ]
  br i1 %extract84.i, label %preload442.i, label %postload443.i

preload442.i:                                     ; preds = %postload440.i
  %masked_load288.i = load float addrspace(1)* %43, align 4
  br label %postload443.i

postload443.i:                                    ; preds = %preload442.i, %postload440.i
  %phi444.i = phi float [ undef, %postload440.i ], [ %masked_load288.i, %preload442.i ]
  br i1 %extract85.i, label %preload445.i, label %postload446.i

preload445.i:                                     ; preds = %postload443.i
  %masked_load289.i = load float addrspace(1)* %44, align 4
  br label %postload446.i

postload446.i:                                    ; preds = %preload445.i, %postload443.i
  %phi447.i = phi float [ undef, %postload443.i ], [ %masked_load289.i, %preload445.i ]
  br i1 %extract86.i, label %preload448.i, label %postload449.i

preload448.i:                                     ; preds = %postload446.i
  %masked_load290.i = load float addrspace(1)* %45, align 4
  br label %postload449.i

postload449.i:                                    ; preds = %preload448.i, %postload446.i
  %phi450.i = phi float [ undef, %postload446.i ], [ %masked_load290.i, %preload448.i ]
  br i1 %extract87.i, label %preload451.i, label %postload452.i

preload451.i:                                     ; preds = %postload449.i
  %masked_load291.i = load float addrspace(1)* %46, align 4
  br label %postload452.i

postload452.i:                                    ; preds = %preload451.i, %postload449.i
  %phi453.i = phi float [ undef, %postload449.i ], [ %masked_load291.i, %preload451.i ]
  br i1 %extract88.i, label %preload454.i, label %postload455.i

preload454.i:                                     ; preds = %postload452.i
  %masked_load292.i = load float addrspace(1)* %47, align 4
  br label %postload455.i

postload455.i:                                    ; preds = %preload454.i, %postload452.i
  %phi456.i = phi float [ undef, %postload452.i ], [ %masked_load292.i, %preload454.i ]
  br i1 %extract89.i, label %preload457.i, label %postload458.i

preload457.i:                                     ; preds = %postload455.i
  %masked_load293.i = load float addrspace(1)* %48, align 4
  br label %postload458.i

postload458.i:                                    ; preds = %preload457.i, %postload455.i
  %phi459.i = phi float [ undef, %postload455.i ], [ %masked_load293.i, %preload457.i ]
  br i1 %extract90.i, label %preload460.i, label %postload461.i

preload460.i:                                     ; preds = %postload458.i
  %masked_load294.i = load float addrspace(1)* %49, align 4
  br label %postload461.i

postload461.i:                                    ; preds = %preload460.i, %postload458.i
  %phi462.i = phi float [ undef, %postload458.i ], [ %masked_load294.i, %preload460.i ]
  %temp.vect.i = insertelement <16 x float> undef, float %phi468.i, i32 0
  %temp.vect91.i = insertelement <16 x float> %temp.vect.i, float %phi477.i, i32 1
  %temp.vect92.i = insertelement <16 x float> %temp.vect91.i, float %phi480.i, i32 2
  %temp.vect93.i = insertelement <16 x float> %temp.vect92.i, float %phi486.i, i32 3
  %temp.vect94.i = insertelement <16 x float> %temp.vect93.i, float %phi489.i, i32 4
  %temp.vect95.i = insertelement <16 x float> %temp.vect94.i, float %phi492.i, i32 5
  %temp.vect96.i = insertelement <16 x float> %temp.vect95.i, float %phi495.i, i32 6
  %temp.vect97.i = insertelement <16 x float> %temp.vect96.i, float %phi498.i, i32 7
  %temp.vect98.i = insertelement <16 x float> %temp.vect97.i, float %phi441.i, i32 8
  %temp.vect99.i = insertelement <16 x float> %temp.vect98.i, float %phi444.i, i32 9
  %temp.vect100.i = insertelement <16 x float> %temp.vect99.i, float %phi447.i, i32 10
  %temp.vect101.i = insertelement <16 x float> %temp.vect100.i, float %phi450.i, i32 11
  %temp.vect102.i = insertelement <16 x float> %temp.vect101.i, float %phi453.i, i32 12
  %temp.vect103.i = insertelement <16 x float> %temp.vect102.i, float %phi456.i, i32 13
  %temp.vect104.i = insertelement <16 x float> %temp.vect103.i, float %phi459.i, i32 14
  %temp.vect105.i = insertelement <16 x float> %temp.vect104.i, float %phi462.i, i32 15
  %add15106.i = fadd <16 x float> %vectorPHI57.i, %temp.vect105.i
  %out_sel107.i = select <16 x i1> %vectorPHI56.i, <16 x float> %add15106.i, <16 x float> %vectorPHI55.i
  %conv171108.i = zext <16 x i32> %vectorPHI58.i to <16 x i64>
  %add18111.i = add <16 x i64> %vector110.i, %conv171108.i
  %conv19112.i = trunc <16 x i64> %add18111.i to <16 x i32>
  %cmp13.i = icmp slt <16 x i32> %conv19112.i, %vector49.i
  %notCond113.i = xor <16 x i1> %cmp13.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %who_left_tr114.i = and <16 x i1> %vectorPHI56.i, %notCond113.i
  %loop_mask1116.i = or <16 x i1> %vectorPHI.i, %who_left_tr114.i
  %ipred.i4.i = bitcast <16 x i1> %loop_mask1116.i to i16
  %val.i5.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i4.i, i16 %ipred.i4.i) nounwind
  %51 = and i32 %val.i5.i, 1
  %res.i6.i = icmp eq i32 %51, 0
  %local_edge135.i = and <16 x i1> %vectorPHI56.i, %cmp13.i
  br i1 %res.i6.i, label %while.body.i, label %while.end.i

while.end.i:                                      ; preds = %postload461.i, %SyncBB968.i
  %vectorPHI152.i = phi <16 x float> [ undef, %SyncBB968.i ], [ %out_sel107.i, %postload461.i ]
  %merge153.i = select <16 x i1> %cmp134.i, <16 x float> %vectorPHI152.i, <16 x float> zeroinitializer
  %52 = extractelement <16 x i32> %conv1146.i, i32 0
  %extract155.i = sext i32 %52 to i64
  %"&(pSB[currWI].offset)7597.i" = or i64 %CurrSBIndex..0.i, 64
  %"&pSB[currWI].offset760.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)7597.i"
  %CastToValueType761.i = bitcast i8* %"&pSB[currWI].offset760.i" to i64*
  store i64 %extract155.i, i64* %CastToValueType761.i, align 8
  %53 = getelementptr inbounds float addrspace(3)* %10, i64 %extract155.i
  %"&(pSB[currWI].offset)9138.i" = or i64 %CurrSBIndex..0.i, 72
  %"&pSB[currWI].offset914.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)9138.i"
  %CastToValueType915.i = bitcast i8* %"&pSB[currWI].offset914.i" to float addrspace(3)**
  store float addrspace(3)* %53, float addrspace(3)** %CastToValueType915.i, align 8
  %ptrTypeCast.i = bitcast float addrspace(3)* %53 to <16 x float> addrspace(3)*
  store <16 x float> %merge153.i, <16 x float> addrspace(3)* %ptrTypeCast.i, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %22
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %while.end.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 128
  br label %SyncBB968.i

elseBB.i:                                         ; preds = %while.end.i
  %div23.i = lshr i64 %33, 1
  %conv24.i = trunc i64 %div23.i to i32
  %Mneg3.i = icmp ne i32 %conv24.i, 0
  %temp176.i = insertelement <16 x i1> undef, i1 %Mneg3.i, i32 0
  %vector177.i = shufflevector <16 x i1> %temp176.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %negIncomingLoopMask26.i = xor i1 %Mneg3.i, true
  %temp173.i = insertelement <16 x i1> undef, i1 %negIncomingLoopMask26.i, i32 0
  %vector174.i = shufflevector <16 x i1> %temp173.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %arrayidx40.i = getelementptr inbounds float addrspace(1)* %4, i64 %28
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB978.i, %thenBB971.i, %elseBB.i
  %currBarrier.0.i = phi i32 [ 3, %elseBB.i ], [ %currBarrier.4.i, %thenBB978.i ], [ %currBarrier.1.i, %thenBB971.i ]
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride984.i", %thenBB978.i ], [ %"loadedCurrSB+Stride977.i", %thenBB971.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++982.i", %thenBB978.i ], [ %"CurrWI++975.i", %thenBB971.i ]
  br i1 %Mneg3.i, label %for.body.i, label %for.end.i

for.body.i:                                       ; preds = %postload605.i, %SyncBB.i
  %currBarrier.1.i = phi i32 [ %currBarrier.3.i, %postload605.i ], [ %currBarrier.0.i, %SyncBB.i ]
  %CurrSBIndex..2.i = phi i64 [ %CurrSBIndex..4.i, %postload605.i ], [ %CurrSBIndex..1.i, %SyncBB.i ]
  %CurrWI..2.i = phi i64 [ %CurrWI..4.i, %postload605.i ], [ %CurrWI..1.i, %SyncBB.i ]
  %vectorPHI172.i = phi <16 x i1> [ %loop_mask12258.i, %postload605.i ], [ %vector174.i, %SyncBB.i ]
  %vectorPHI175.i = phi <16 x i1> [ %local_edge17262.i, %postload605.i ], [ %vector177.i, %SyncBB.i ]
  %s.03.i = phi i32 [ %shr.i, %postload605.i ], [ %conv24.i, %SyncBB.i ]
  %"&(pSB[currWI].offset)955.i" = add nuw i64 %CurrSBIndex..2.i, 100
  %"&pSB[currWI].offset956.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)955.i"
  %CastToValueType957.i = bitcast i8* %"&pSB[currWI].offset956.i" to i32*
  store i32 %s.03.i, i32* %CastToValueType957.i, align 4
  %"&(pSB[currWI].offset)936.i" = add nuw i64 %CurrSBIndex..2.i, 96
  %"&pSB[currWI].offset937.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)936.i"
  %CastToValueType938.i = bitcast i8* %"&pSB[currWI].offset937.i" to <16 x i1>*
  store <16 x i1> %vectorPHI175.i, <16 x i1>* %CastToValueType938.i, align 16
  %"&(pSB[currWI].offset)927.i" = add nuw i64 %CurrSBIndex..2.i, 80
  %"&pSB[currWI].offset928.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)927.i"
  %CastToValueType929.i = bitcast i8* %"&pSB[currWI].offset928.i" to <16 x i1>*
  store <16 x i1> %vectorPHI172.i, <16 x i1>* %CastToValueType929.i, align 16
  %temp178.i = insertelement <16 x i32> undef, i32 %s.03.i, i32 0
  %vector179.i = shufflevector <16 x i32> %temp178.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %"&pSB[currWI].offset755.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..2.i
  %CastToValueType756.i = bitcast i8* %"&pSB[currWI].offset755.i" to <16 x i32>*
  %loadedValue757.i = load <16 x i32>* %CastToValueType756.i, align 64
  %cmp27.i = icmp ult <16 x i32> %loadedValue757.i, %vector179.i
  %for.body_to_if.then182.i = and <16 x i1> %vectorPHI175.i, %cmp27.i
  %extract202.i = extractelement <16 x i1> %for.body_to_if.then182.i, i32 1
  %extract203.i = extractelement <16 x i1> %for.body_to_if.then182.i, i32 2
  %extract204.i = extractelement <16 x i1> %for.body_to_if.then182.i, i32 3
  %extract205.i = extractelement <16 x i1> %for.body_to_if.then182.i, i32 4
  %extract206.i = extractelement <16 x i1> %for.body_to_if.then182.i, i32 5
  %extract207.i = extractelement <16 x i1> %for.body_to_if.then182.i, i32 6
  %extract208.i = extractelement <16 x i1> %for.body_to_if.then182.i, i32 7
  %extract209.i = extractelement <16 x i1> %for.body_to_if.then182.i, i32 8
  %extract210.i = extractelement <16 x i1> %for.body_to_if.then182.i, i32 9
  %extract211.i = extractelement <16 x i1> %for.body_to_if.then182.i, i32 10
  %extract212.i = extractelement <16 x i1> %for.body_to_if.then182.i, i32 11
  %extract213.i = extractelement <16 x i1> %for.body_to_if.then182.i, i32 12
  %extract214.i = extractelement <16 x i1> %for.body_to_if.then182.i, i32 13
  %extract215.i = extractelement <16 x i1> %for.body_to_if.then182.i, i32 14
  %extract216.i = extractelement <16 x i1> %for.body_to_if.then182.i, i32 15
  %extract201.i = extractelement <16 x i1> %for.body_to_if.then182.i, i32 0
  %add29183.i = add <16 x i32> %vector179.i, %loadedValue757.i
  %idxprom30184.i = zext <16 x i32> %add29183.i to <16 x i64>
  %extract186.i = extractelement <16 x i64> %idxprom30184.i, i32 1
  %extract187.i = extractelement <16 x i64> %idxprom30184.i, i32 2
  %extract188.i = extractelement <16 x i64> %idxprom30184.i, i32 3
  %extract189.i = extractelement <16 x i64> %idxprom30184.i, i32 4
  %extract190.i = extractelement <16 x i64> %idxprom30184.i, i32 5
  %extract191.i = extractelement <16 x i64> %idxprom30184.i, i32 6
  %extract192.i = extractelement <16 x i64> %idxprom30184.i, i32 7
  %extract193.i = extractelement <16 x i64> %idxprom30184.i, i32 8
  %extract194.i = extractelement <16 x i64> %idxprom30184.i, i32 9
  %extract195.i = extractelement <16 x i64> %idxprom30184.i, i32 10
  %extract196.i = extractelement <16 x i64> %idxprom30184.i, i32 11
  %extract197.i = extractelement <16 x i64> %idxprom30184.i, i32 12
  %extract198.i = extractelement <16 x i64> %idxprom30184.i, i32 13
  %extract199.i = extractelement <16 x i64> %idxprom30184.i, i32 14
  %extract200.i = extractelement <16 x i64> %idxprom30184.i, i32 15
  %54 = getelementptr inbounds float addrspace(3)* %10, i64 %extract186.i
  %55 = getelementptr inbounds float addrspace(3)* %10, i64 %extract187.i
  %56 = getelementptr inbounds float addrspace(3)* %10, i64 %extract188.i
  %57 = getelementptr inbounds float addrspace(3)* %10, i64 %extract189.i
  %58 = getelementptr inbounds float addrspace(3)* %10, i64 %extract190.i
  %59 = getelementptr inbounds float addrspace(3)* %10, i64 %extract191.i
  %60 = getelementptr inbounds float addrspace(3)* %10, i64 %extract192.i
  %61 = getelementptr inbounds float addrspace(3)* %10, i64 %extract193.i
  %62 = getelementptr inbounds float addrspace(3)* %10, i64 %extract194.i
  %63 = getelementptr inbounds float addrspace(3)* %10, i64 %extract195.i
  %64 = getelementptr inbounds float addrspace(3)* %10, i64 %extract196.i
  %65 = getelementptr inbounds float addrspace(3)* %10, i64 %extract197.i
  %66 = getelementptr inbounds float addrspace(3)* %10, i64 %extract198.i
  %67 = getelementptr inbounds float addrspace(3)* %10, i64 %extract199.i
  %68 = getelementptr inbounds float addrspace(3)* %10, i64 %extract200.i
  br i1 %extract201.i, label %preload547.i, label %postload548.i

preload547.i:                                     ; preds = %for.body.i
  %extract185.i = extractelement <16 x i64> %idxprom30184.i, i32 0
  %69 = getelementptr inbounds float addrspace(3)* %10, i64 %extract185.i
  %masked_load295.i = load float addrspace(3)* %69, align 4
  br label %postload548.i

postload548.i:                                    ; preds = %preload547.i, %for.body.i
  %phi549.i = phi float [ undef, %for.body.i ], [ %masked_load295.i, %preload547.i ]
  br i1 %extract202.i, label %preload559.i, label %postload560.i

preload559.i:                                     ; preds = %postload548.i
  %masked_load296.i = load float addrspace(3)* %54, align 4
  br label %postload560.i

postload560.i:                                    ; preds = %preload559.i, %postload548.i
  %phi561.i = phi float [ undef, %postload548.i ], [ %masked_load296.i, %preload559.i ]
  br i1 %extract203.i, label %preload562.i, label %postload563.i

preload562.i:                                     ; preds = %postload560.i
  %masked_load297.i = load float addrspace(3)* %55, align 4
  br label %postload563.i

postload563.i:                                    ; preds = %preload562.i, %postload560.i
  %phi564.i = phi float [ undef, %postload560.i ], [ %masked_load297.i, %preload562.i ]
  br i1 %extract204.i, label %preload565.i, label %postload566.i

preload565.i:                                     ; preds = %postload563.i
  %masked_load298.i = load float addrspace(3)* %56, align 4
  br label %postload566.i

postload566.i:                                    ; preds = %preload565.i, %postload563.i
  %phi567.i = phi float [ undef, %postload563.i ], [ %masked_load298.i, %preload565.i ]
  br i1 %extract205.i, label %preload568.i, label %postload569.i

preload568.i:                                     ; preds = %postload566.i
  %masked_load299.i = load float addrspace(3)* %57, align 4
  br label %postload569.i

postload569.i:                                    ; preds = %preload568.i, %postload566.i
  %phi570.i = phi float [ undef, %postload566.i ], [ %masked_load299.i, %preload568.i ]
  br i1 %extract206.i, label %preload571.i, label %postload572.i

preload571.i:                                     ; preds = %postload569.i
  %masked_load300.i = load float addrspace(3)* %58, align 4
  br label %postload572.i

postload572.i:                                    ; preds = %preload571.i, %postload569.i
  %phi573.i = phi float [ undef, %postload569.i ], [ %masked_load300.i, %preload571.i ]
  br i1 %extract207.i, label %preload574.i, label %postload575.i

preload574.i:                                     ; preds = %postload572.i
  %masked_load301.i = load float addrspace(3)* %59, align 4
  br label %postload575.i

postload575.i:                                    ; preds = %preload574.i, %postload572.i
  %phi576.i = phi float [ undef, %postload572.i ], [ %masked_load301.i, %preload574.i ]
  br i1 %extract208.i, label %preload577.i, label %postload578.i

preload577.i:                                     ; preds = %postload575.i
  %masked_load302.i = load float addrspace(3)* %60, align 4
  br label %postload578.i

postload578.i:                                    ; preds = %preload577.i, %postload575.i
  %phi579.i = phi float [ undef, %postload575.i ], [ %masked_load302.i, %preload577.i ]
  br i1 %extract209.i, label %preload580.i, label %postload581.i

preload580.i:                                     ; preds = %postload578.i
  %masked_load303.i = load float addrspace(3)* %61, align 4
  br label %postload581.i

postload581.i:                                    ; preds = %preload580.i, %postload578.i
  %phi582.i = phi float [ undef, %postload578.i ], [ %masked_load303.i, %preload580.i ]
  br i1 %extract210.i, label %preload583.i, label %postload584.i

preload583.i:                                     ; preds = %postload581.i
  %masked_load304.i = load float addrspace(3)* %62, align 4
  br label %postload584.i

postload584.i:                                    ; preds = %preload583.i, %postload581.i
  %phi585.i = phi float [ undef, %postload581.i ], [ %masked_load304.i, %preload583.i ]
  br i1 %extract211.i, label %preload586.i, label %postload587.i

preload586.i:                                     ; preds = %postload584.i
  %masked_load305.i = load float addrspace(3)* %63, align 4
  br label %postload587.i

postload587.i:                                    ; preds = %preload586.i, %postload584.i
  %phi588.i = phi float [ undef, %postload584.i ], [ %masked_load305.i, %preload586.i ]
  br i1 %extract212.i, label %preload589.i, label %postload590.i

preload589.i:                                     ; preds = %postload587.i
  %masked_load306.i = load float addrspace(3)* %64, align 4
  br label %postload590.i

postload590.i:                                    ; preds = %preload589.i, %postload587.i
  %phi591.i = phi float [ undef, %postload587.i ], [ %masked_load306.i, %preload589.i ]
  br i1 %extract213.i, label %preload592.i, label %postload593.i

preload592.i:                                     ; preds = %postload590.i
  %masked_load307.i = load float addrspace(3)* %65, align 4
  br label %postload593.i

postload593.i:                                    ; preds = %preload592.i, %postload590.i
  %phi594.i = phi float [ undef, %postload590.i ], [ %masked_load307.i, %preload592.i ]
  br i1 %extract214.i, label %preload595.i, label %postload596.i

preload595.i:                                     ; preds = %postload593.i
  %masked_load308.i = load float addrspace(3)* %66, align 4
  br label %postload596.i

postload596.i:                                    ; preds = %preload595.i, %postload593.i
  %phi597.i = phi float [ undef, %postload593.i ], [ %masked_load308.i, %preload595.i ]
  br i1 %extract215.i, label %preload598.i, label %postload599.i

preload598.i:                                     ; preds = %postload596.i
  %masked_load309.i = load float addrspace(3)* %67, align 4
  br label %postload599.i

postload599.i:                                    ; preds = %preload598.i, %postload596.i
  %phi600.i = phi float [ undef, %postload596.i ], [ %masked_load309.i, %preload598.i ]
  br i1 %extract216.i, label %preload601.i, label %postload602.i

preload601.i:                                     ; preds = %postload599.i
  %masked_load310.i = load float addrspace(3)* %68, align 4
  br label %postload602.i

postload602.i:                                    ; preds = %preload601.i, %postload599.i
  %phi603.i = phi float [ undef, %postload599.i ], [ %masked_load310.i, %preload601.i ]
  %temp.vect218.i = insertelement <16 x float> undef, float %phi549.i, i32 0
  %temp.vect219.i = insertelement <16 x float> %temp.vect218.i, float %phi561.i, i32 1
  %temp.vect220.i = insertelement <16 x float> %temp.vect219.i, float %phi564.i, i32 2
  %temp.vect221.i = insertelement <16 x float> %temp.vect220.i, float %phi567.i, i32 3
  %temp.vect222.i = insertelement <16 x float> %temp.vect221.i, float %phi570.i, i32 4
  %temp.vect223.i = insertelement <16 x float> %temp.vect222.i, float %phi573.i, i32 5
  %temp.vect224.i = insertelement <16 x float> %temp.vect223.i, float %phi576.i, i32 6
  %temp.vect225.i = insertelement <16 x float> %temp.vect224.i, float %phi579.i, i32 7
  %temp.vect226.i = insertelement <16 x float> %temp.vect225.i, float %phi582.i, i32 8
  %temp.vect227.i = insertelement <16 x float> %temp.vect226.i, float %phi585.i, i32 9
  %temp.vect228.i = insertelement <16 x float> %temp.vect227.i, float %phi588.i, i32 10
  %temp.vect229.i = insertelement <16 x float> %temp.vect228.i, float %phi591.i, i32 11
  %temp.vect230.i = insertelement <16 x float> %temp.vect229.i, float %phi594.i, i32 12
  %temp.vect231.i = insertelement <16 x float> %temp.vect230.i, float %phi597.i, i32 13
  %temp.vect232.i = insertelement <16 x float> %temp.vect231.i, float %phi600.i, i32 14
  %temp.vect233.i = insertelement <16 x float> %temp.vect232.i, float %phi603.i, i32 15
  br i1 %extract201.i, label %preload.i, label %postload.i

preload.i:                                        ; preds = %postload602.i
  %"&(pSB[currWI].offset)917.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset918.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)917.i"
  %CastToValueType919.i = bitcast i8* %"&pSB[currWI].offset918.i" to float addrspace(3)**
  %loadedValue920.i = load float addrspace(3)** %CastToValueType919.i, align 8
  %vload312.i = load float addrspace(3)* %loadedValue920.i, align 4
  br label %postload.i

postload.i:                                       ; preds = %preload.i, %postload602.i
  %phi.i = phi float [ undef, %postload602.i ], [ %vload312.i, %preload.i ]
  %vpack.i = insertelement <16 x float> undef, float %phi.i, i32 0
  br i1 %extract202.i, label %preload436.i, label %postload437.i

preload436.i:                                     ; preds = %postload.i
  %"&(pSB[currWI].offset)763.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset764.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)763.i"
  %CastToValueType765.i = bitcast i8* %"&pSB[currWI].offset764.i" to i64*
  %loadedValue766.i = load i64* %CastToValueType765.i, align 8
  %.sum738.i = add i64 %loadedValue766.i, 1
  %70 = getelementptr float addrspace(3)* %10, i64 %.sum738.i
  %vload315.i = load float addrspace(3)* %70, align 4
  br label %postload437.i

postload437.i:                                    ; preds = %preload436.i, %postload.i
  %phi438.i = phi float [ undef, %postload.i ], [ %vload315.i, %preload436.i ]
  %vpack316.i = insertelement <16 x float> %vpack.i, float %phi438.i, i32 1
  br i1 %extract203.i, label %preload481.i, label %postload482.i

preload481.i:                                     ; preds = %postload437.i
  %"&(pSB[currWI].offset)768.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset769.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)768.i"
  %CastToValueType770.i = bitcast i8* %"&pSB[currWI].offset769.i" to i64*
  %loadedValue771.i = load i64* %CastToValueType770.i, align 8
  %.sum737.i = add i64 %loadedValue771.i, 2
  %71 = getelementptr float addrspace(3)* %10, i64 %.sum737.i
  %vload319.i = load float addrspace(3)* %71, align 4
  br label %postload482.i

postload482.i:                                    ; preds = %preload481.i, %postload437.i
  %phi483.i = phi float [ undef, %postload437.i ], [ %vload319.i, %preload481.i ]
  %vpack320.i = insertelement <16 x float> %vpack316.i, float %phi483.i, i32 2
  br i1 %extract204.i, label %preload469.i, label %postload470.i

preload469.i:                                     ; preds = %postload482.i
  %"&(pSB[currWI].offset)773.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset774.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)773.i"
  %CastToValueType775.i = bitcast i8* %"&pSB[currWI].offset774.i" to i64*
  %loadedValue776.i = load i64* %CastToValueType775.i, align 8
  %.sum736.i = add i64 %loadedValue776.i, 3
  %72 = getelementptr float addrspace(3)* %10, i64 %.sum736.i
  %vload323.i = load float addrspace(3)* %72, align 4
  br label %postload470.i

postload470.i:                                    ; preds = %preload469.i, %postload482.i
  %phi471.i = phi float [ undef, %postload482.i ], [ %vload323.i, %preload469.i ]
  %vpack324.i = insertelement <16 x float> %vpack320.i, float %phi471.i, i32 3
  br i1 %extract205.i, label %preload472.i, label %postload473.i

preload472.i:                                     ; preds = %postload470.i
  %"&(pSB[currWI].offset)778.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset779.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)778.i"
  %CastToValueType780.i = bitcast i8* %"&pSB[currWI].offset779.i" to i64*
  %loadedValue781.i = load i64* %CastToValueType780.i, align 8
  %.sum735.i = add i64 %loadedValue781.i, 4
  %73 = getelementptr float addrspace(3)* %10, i64 %.sum735.i
  %vload327.i = load float addrspace(3)* %73, align 4
  br label %postload473.i

postload473.i:                                    ; preds = %preload472.i, %postload470.i
  %phi474.i = phi float [ undef, %postload470.i ], [ %vload327.i, %preload472.i ]
  %vpack328.i = insertelement <16 x float> %vpack324.i, float %phi474.i, i32 4
  br i1 %extract206.i, label %preload502.i, label %postload503.i

preload502.i:                                     ; preds = %postload473.i
  %"&(pSB[currWI].offset)783.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset784.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)783.i"
  %CastToValueType785.i = bitcast i8* %"&pSB[currWI].offset784.i" to i64*
  %loadedValue786.i = load i64* %CastToValueType785.i, align 8
  %.sum734.i = add i64 %loadedValue786.i, 5
  %74 = getelementptr float addrspace(3)* %10, i64 %.sum734.i
  %vload331.i = load float addrspace(3)* %74, align 4
  br label %postload503.i

postload503.i:                                    ; preds = %preload502.i, %postload473.i
  %phi504.i = phi float [ undef, %postload473.i ], [ %vload331.i, %preload502.i ]
  %vpack332.i = insertelement <16 x float> %vpack328.i, float %phi504.i, i32 5
  br i1 %extract207.i, label %preload463.i, label %postload464.i

preload463.i:                                     ; preds = %postload503.i
  %"&(pSB[currWI].offset)788.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset789.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)788.i"
  %CastToValueType790.i = bitcast i8* %"&pSB[currWI].offset789.i" to i64*
  %loadedValue791.i = load i64* %CastToValueType790.i, align 8
  %.sum733.i = add i64 %loadedValue791.i, 6
  %75 = getelementptr float addrspace(3)* %10, i64 %.sum733.i
  %vload335.i = load float addrspace(3)* %75, align 4
  br label %postload464.i

postload464.i:                                    ; preds = %preload463.i, %postload503.i
  %phi465.i = phi float [ undef, %postload503.i ], [ %vload335.i, %preload463.i ]
  %vpack336.i = insertelement <16 x float> %vpack332.i, float %phi465.i, i32 6
  br i1 %extract208.i, label %preload499.i, label %postload500.i

preload499.i:                                     ; preds = %postload464.i
  %"&(pSB[currWI].offset)793.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset794.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)793.i"
  %CastToValueType795.i = bitcast i8* %"&pSB[currWI].offset794.i" to i64*
  %loadedValue796.i = load i64* %CastToValueType795.i, align 8
  %.sum732.i = add i64 %loadedValue796.i, 7
  %76 = getelementptr float addrspace(3)* %10, i64 %.sum732.i
  %vload339.i = load float addrspace(3)* %76, align 4
  br label %postload500.i

postload500.i:                                    ; preds = %preload499.i, %postload464.i
  %phi501.i = phi float [ undef, %postload464.i ], [ %vload339.i, %preload499.i ]
  %vpack340.i = insertelement <16 x float> %vpack336.i, float %phi501.i, i32 7
  br i1 %extract209.i, label %preload535.i, label %postload536.i

preload535.i:                                     ; preds = %postload500.i
  %"&(pSB[currWI].offset)798.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset799.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)798.i"
  %CastToValueType800.i = bitcast i8* %"&pSB[currWI].offset799.i" to i64*
  %loadedValue801.i = load i64* %CastToValueType800.i, align 8
  %.sum731.i = add i64 %loadedValue801.i, 8
  %77 = getelementptr float addrspace(3)* %10, i64 %.sum731.i
  %vload343.i = load float addrspace(3)* %77, align 4
  br label %postload536.i

postload536.i:                                    ; preds = %preload535.i, %postload500.i
  %phi537.i = phi float [ undef, %postload500.i ], [ %vload343.i, %preload535.i ]
  %vpack344.i = insertelement <16 x float> %vpack340.i, float %phi537.i, i32 8
  br i1 %extract210.i, label %preload550.i, label %postload551.i

preload550.i:                                     ; preds = %postload536.i
  %"&(pSB[currWI].offset)803.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset804.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)803.i"
  %CastToValueType805.i = bitcast i8* %"&pSB[currWI].offset804.i" to i64*
  %loadedValue806.i = load i64* %CastToValueType805.i, align 8
  %.sum730.i = add i64 %loadedValue806.i, 9
  %78 = getelementptr float addrspace(3)* %10, i64 %.sum730.i
  %vload347.i = load float addrspace(3)* %78, align 4
  br label %postload551.i

postload551.i:                                    ; preds = %preload550.i, %postload536.i
  %phi552.i = phi float [ undef, %postload536.i ], [ %vload347.i, %preload550.i ]
  %vpack348.i = insertelement <16 x float> %vpack344.i, float %phi552.i, i32 9
  br i1 %extract211.i, label %preload553.i, label %postload554.i

preload553.i:                                     ; preds = %postload551.i
  %"&(pSB[currWI].offset)808.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset809.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)808.i"
  %CastToValueType810.i = bitcast i8* %"&pSB[currWI].offset809.i" to i64*
  %loadedValue811.i = load i64* %CastToValueType810.i, align 8
  %.sum729.i = add i64 %loadedValue811.i, 10
  %79 = getelementptr float addrspace(3)* %10, i64 %.sum729.i
  %vload351.i = load float addrspace(3)* %79, align 4
  br label %postload554.i

postload554.i:                                    ; preds = %preload553.i, %postload551.i
  %phi555.i = phi float [ undef, %postload551.i ], [ %vload351.i, %preload553.i ]
  %vpack352.i = insertelement <16 x float> %vpack348.i, float %phi555.i, i32 10
  br i1 %extract212.i, label %preload556.i, label %postload557.i

preload556.i:                                     ; preds = %postload554.i
  %"&(pSB[currWI].offset)813.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset814.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)813.i"
  %CastToValueType815.i = bitcast i8* %"&pSB[currWI].offset814.i" to i64*
  %loadedValue816.i = load i64* %CastToValueType815.i, align 8
  %.sum728.i = add i64 %loadedValue816.i, 11
  %80 = getelementptr float addrspace(3)* %10, i64 %.sum728.i
  %vload355.i = load float addrspace(3)* %80, align 4
  br label %postload557.i

postload557.i:                                    ; preds = %preload556.i, %postload554.i
  %phi558.i = phi float [ undef, %postload554.i ], [ %vload355.i, %preload556.i ]
  %vpack356.i = insertelement <16 x float> %vpack352.i, float %phi558.i, i32 11
  br i1 %extract213.i, label %preload538.i, label %postload539.i

preload538.i:                                     ; preds = %postload557.i
  %"&(pSB[currWI].offset)818.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset819.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)818.i"
  %CastToValueType820.i = bitcast i8* %"&pSB[currWI].offset819.i" to i64*
  %loadedValue821.i = load i64* %CastToValueType820.i, align 8
  %.sum727.i = add i64 %loadedValue821.i, 12
  %81 = getelementptr float addrspace(3)* %10, i64 %.sum727.i
  %vload359.i = load float addrspace(3)* %81, align 4
  br label %postload539.i

postload539.i:                                    ; preds = %preload538.i, %postload557.i
  %phi540.i = phi float [ undef, %postload557.i ], [ %vload359.i, %preload538.i ]
  %vpack360.i = insertelement <16 x float> %vpack356.i, float %phi540.i, i32 12
  br i1 %extract214.i, label %preload541.i, label %postload542.i

preload541.i:                                     ; preds = %postload539.i
  %"&(pSB[currWI].offset)823.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset824.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)823.i"
  %CastToValueType825.i = bitcast i8* %"&pSB[currWI].offset824.i" to i64*
  %loadedValue826.i = load i64* %CastToValueType825.i, align 8
  %.sum726.i = add i64 %loadedValue826.i, 13
  %82 = getelementptr float addrspace(3)* %10, i64 %.sum726.i
  %vload363.i = load float addrspace(3)* %82, align 4
  br label %postload542.i

postload542.i:                                    ; preds = %preload541.i, %postload539.i
  %phi543.i = phi float [ undef, %postload539.i ], [ %vload363.i, %preload541.i ]
  %vpack364.i = insertelement <16 x float> %vpack360.i, float %phi543.i, i32 13
  br i1 %extract215.i, label %preload544.i, label %postload545.i

preload544.i:                                     ; preds = %postload542.i
  %"&(pSB[currWI].offset)828.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset829.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)828.i"
  %CastToValueType830.i = bitcast i8* %"&pSB[currWI].offset829.i" to i64*
  %loadedValue831.i = load i64* %CastToValueType830.i, align 8
  %.sum725.i = add i64 %loadedValue831.i, 14
  %83 = getelementptr float addrspace(3)* %10, i64 %.sum725.i
  %vload367.i = load float addrspace(3)* %83, align 4
  br label %postload545.i

postload545.i:                                    ; preds = %preload544.i, %postload542.i
  %phi546.i = phi float [ undef, %postload542.i ], [ %vload367.i, %preload544.i ]
  %vpack368.i = insertelement <16 x float> %vpack364.i, float %phi546.i, i32 14
  br i1 %extract216.i, label %preload606.i, label %postload607.i

preload606.i:                                     ; preds = %postload545.i
  %"&(pSB[currWI].offset)833.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset834.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)833.i"
  %CastToValueType835.i = bitcast i8* %"&pSB[currWI].offset834.i" to i64*
  %loadedValue836.i = load i64* %CastToValueType835.i, align 8
  %.sum724.i = add i64 %loadedValue836.i, 15
  %84 = getelementptr float addrspace(3)* %10, i64 %.sum724.i
  %vload371.i = load float addrspace(3)* %84, align 4
  br label %postload607.i

postload607.i:                                    ; preds = %preload606.i, %postload545.i
  %phi608.i = phi float [ undef, %postload545.i ], [ %vload371.i, %preload606.i ]
  %vpack372.i = insertelement <16 x float> %vpack368.i, float %phi608.i, i32 15
  %add34234.i = fadd <16 x float> %vpack372.i, %temp.vect233.i
  br i1 %extract201.i, label %preload609.i, label %postload610.i

preload609.i:                                     ; preds = %postload607.i
  %exData.i = extractelement <16 x float> %add34234.i, i32 0
  %"&(pSB[currWI].offset)922.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset923.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)922.i"
  %CastToValueType924.i = bitcast i8* %"&pSB[currWI].offset923.i" to float addrspace(3)**
  %loadedValue925.i = load float addrspace(3)** %CastToValueType924.i, align 8
  store float %exData.i, float addrspace(3)* %loadedValue925.i, align 4
  br label %postload610.i

postload610.i:                                    ; preds = %preload609.i, %postload607.i
  br i1 %extract202.i, label %preload612.i, label %postload613.i

preload612.i:                                     ; preds = %postload610.i
  %"&(pSB[currWI].offset)838.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset839.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)838.i"
  %CastToValueType840.i = bitcast i8* %"&pSB[currWI].offset839.i" to i64*
  %loadedValue841.i = load i64* %CastToValueType840.i, align 8
  %.sum723.i = add i64 %loadedValue841.i, 1
  %85 = getelementptr float addrspace(3)* %10, i64 %.sum723.i
  %exData377.i = extractelement <16 x float> %add34234.i, i32 1
  store float %exData377.i, float addrspace(3)* %85, align 4
  br label %postload613.i

postload613.i:                                    ; preds = %preload612.i, %postload610.i
  br i1 %extract203.i, label %preload615.i, label %postload616.i

preload615.i:                                     ; preds = %postload613.i
  %"&(pSB[currWI].offset)843.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset844.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)843.i"
  %CastToValueType845.i = bitcast i8* %"&pSB[currWI].offset844.i" to i64*
  %loadedValue846.i = load i64* %CastToValueType845.i, align 8
  %.sum722.i = add i64 %loadedValue846.i, 2
  %86 = getelementptr float addrspace(3)* %10, i64 %.sum722.i
  %exData380.i = extractelement <16 x float> %add34234.i, i32 2
  store float %exData380.i, float addrspace(3)* %86, align 4
  br label %postload616.i

postload616.i:                                    ; preds = %preload615.i, %postload613.i
  br i1 %extract204.i, label %preload526.i, label %postload527.i

preload526.i:                                     ; preds = %postload616.i
  %"&(pSB[currWI].offset)848.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset849.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)848.i"
  %CastToValueType850.i = bitcast i8* %"&pSB[currWI].offset849.i" to i64*
  %loadedValue851.i = load i64* %CastToValueType850.i, align 8
  %.sum721.i = add i64 %loadedValue851.i, 3
  %87 = getelementptr float addrspace(3)* %10, i64 %.sum721.i
  %exData383.i = extractelement <16 x float> %add34234.i, i32 3
  store float %exData383.i, float addrspace(3)* %87, align 4
  br label %postload527.i

postload527.i:                                    ; preds = %preload526.i, %postload616.i
  br i1 %extract205.i, label %preload529.i, label %postload530.i

preload529.i:                                     ; preds = %postload527.i
  %"&(pSB[currWI].offset)853.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset854.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)853.i"
  %CastToValueType855.i = bitcast i8* %"&pSB[currWI].offset854.i" to i64*
  %loadedValue856.i = load i64* %CastToValueType855.i, align 8
  %.sum720.i = add i64 %loadedValue856.i, 4
  %88 = getelementptr float addrspace(3)* %10, i64 %.sum720.i
  %exData386.i = extractelement <16 x float> %add34234.i, i32 4
  store float %exData386.i, float addrspace(3)* %88, align 4
  br label %postload530.i

postload530.i:                                    ; preds = %preload529.i, %postload527.i
  br i1 %extract206.i, label %preload532.i, label %postload533.i

preload532.i:                                     ; preds = %postload530.i
  %"&(pSB[currWI].offset)858.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset859.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)858.i"
  %CastToValueType860.i = bitcast i8* %"&pSB[currWI].offset859.i" to i64*
  %loadedValue861.i = load i64* %CastToValueType860.i, align 8
  %.sum719.i = add i64 %loadedValue861.i, 5
  %89 = getelementptr float addrspace(3)* %10, i64 %.sum719.i
  %exData389.i = extractelement <16 x float> %add34234.i, i32 5
  store float %exData389.i, float addrspace(3)* %89, align 4
  br label %postload533.i

postload533.i:                                    ; preds = %preload532.i, %postload530.i
  br i1 %extract207.i, label %preload698.i, label %postload699.i

preload698.i:                                     ; preds = %postload533.i
  %"&(pSB[currWI].offset)863.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset864.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)863.i"
  %CastToValueType865.i = bitcast i8* %"&pSB[currWI].offset864.i" to i64*
  %loadedValue866.i = load i64* %CastToValueType865.i, align 8
  %.sum718.i = add i64 %loadedValue866.i, 6
  %90 = getelementptr float addrspace(3)* %10, i64 %.sum718.i
  %exData392.i = extractelement <16 x float> %add34234.i, i32 6
  store float %exData392.i, float addrspace(3)* %90, align 4
  br label %postload699.i

postload699.i:                                    ; preds = %preload698.i, %postload533.i
  br i1 %extract208.i, label %preload701.i, label %postload702.i

preload701.i:                                     ; preds = %postload699.i
  %"&(pSB[currWI].offset)868.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset869.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)868.i"
  %CastToValueType870.i = bitcast i8* %"&pSB[currWI].offset869.i" to i64*
  %loadedValue871.i = load i64* %CastToValueType870.i, align 8
  %.sum717.i = add i64 %loadedValue871.i, 7
  %91 = getelementptr float addrspace(3)* %10, i64 %.sum717.i
  %exData395.i = extractelement <16 x float> %add34234.i, i32 7
  store float %exData395.i, float addrspace(3)* %91, align 4
  br label %postload702.i

postload702.i:                                    ; preds = %preload701.i, %postload699.i
  br i1 %extract209.i, label %preload704.i, label %postload705.i

preload704.i:                                     ; preds = %postload702.i
  %"&(pSB[currWI].offset)873.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset874.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)873.i"
  %CastToValueType875.i = bitcast i8* %"&pSB[currWI].offset874.i" to i64*
  %loadedValue876.i = load i64* %CastToValueType875.i, align 8
  %.sum716.i = add i64 %loadedValue876.i, 8
  %92 = getelementptr float addrspace(3)* %10, i64 %.sum716.i
  %exData398.i = extractelement <16 x float> %add34234.i, i32 8
  store float %exData398.i, float addrspace(3)* %92, align 4
  br label %postload705.i

postload705.i:                                    ; preds = %preload704.i, %postload702.i
  br i1 %extract210.i, label %preload505.i, label %postload506.i

preload505.i:                                     ; preds = %postload705.i
  %"&(pSB[currWI].offset)878.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset879.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)878.i"
  %CastToValueType880.i = bitcast i8* %"&pSB[currWI].offset879.i" to i64*
  %loadedValue881.i = load i64* %CastToValueType880.i, align 8
  %.sum715.i = add i64 %loadedValue881.i, 9
  %93 = getelementptr float addrspace(3)* %10, i64 %.sum715.i
  %exData401.i = extractelement <16 x float> %add34234.i, i32 9
  store float %exData401.i, float addrspace(3)* %93, align 4
  br label %postload506.i

postload506.i:                                    ; preds = %preload505.i, %postload705.i
  br i1 %extract211.i, label %preload508.i, label %postload509.i

preload508.i:                                     ; preds = %postload506.i
  %"&(pSB[currWI].offset)883.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset884.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)883.i"
  %CastToValueType885.i = bitcast i8* %"&pSB[currWI].offset884.i" to i64*
  %loadedValue886.i = load i64* %CastToValueType885.i, align 8
  %.sum714.i = add i64 %loadedValue886.i, 10
  %94 = getelementptr float addrspace(3)* %10, i64 %.sum714.i
  %exData404.i = extractelement <16 x float> %add34234.i, i32 10
  store float %exData404.i, float addrspace(3)* %94, align 4
  br label %postload509.i

postload509.i:                                    ; preds = %preload508.i, %postload506.i
  br i1 %extract212.i, label %preload511.i, label %postload512.i

preload511.i:                                     ; preds = %postload509.i
  %"&(pSB[currWI].offset)888.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset889.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)888.i"
  %CastToValueType890.i = bitcast i8* %"&pSB[currWI].offset889.i" to i64*
  %loadedValue891.i = load i64* %CastToValueType890.i, align 8
  %.sum713.i = add i64 %loadedValue891.i, 11
  %95 = getelementptr float addrspace(3)* %10, i64 %.sum713.i
  %exData407.i = extractelement <16 x float> %add34234.i, i32 11
  store float %exData407.i, float addrspace(3)* %95, align 4
  br label %postload512.i

postload512.i:                                    ; preds = %preload511.i, %postload509.i
  br i1 %extract213.i, label %preload514.i, label %postload515.i

preload514.i:                                     ; preds = %postload512.i
  %"&(pSB[currWI].offset)893.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset894.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)893.i"
  %CastToValueType895.i = bitcast i8* %"&pSB[currWI].offset894.i" to i64*
  %loadedValue896.i = load i64* %CastToValueType895.i, align 8
  %.sum712.i = add i64 %loadedValue896.i, 12
  %96 = getelementptr float addrspace(3)* %10, i64 %.sum712.i
  %exData410.i = extractelement <16 x float> %add34234.i, i32 12
  store float %exData410.i, float addrspace(3)* %96, align 4
  br label %postload515.i

postload515.i:                                    ; preds = %preload514.i, %postload512.i
  br i1 %extract214.i, label %preload517.i, label %postload518.i

preload517.i:                                     ; preds = %postload515.i
  %"&(pSB[currWI].offset)898.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset899.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)898.i"
  %CastToValueType900.i = bitcast i8* %"&pSB[currWI].offset899.i" to i64*
  %loadedValue901.i = load i64* %CastToValueType900.i, align 8
  %.sum711.i = add i64 %loadedValue901.i, 13
  %97 = getelementptr float addrspace(3)* %10, i64 %.sum711.i
  %exData413.i = extractelement <16 x float> %add34234.i, i32 13
  store float %exData413.i, float addrspace(3)* %97, align 4
  br label %postload518.i

postload518.i:                                    ; preds = %preload517.i, %postload515.i
  br i1 %extract215.i, label %preload520.i, label %postload521.i

preload520.i:                                     ; preds = %postload518.i
  %"&(pSB[currWI].offset)903.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset904.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)903.i"
  %CastToValueType905.i = bitcast i8* %"&pSB[currWI].offset904.i" to i64*
  %loadedValue906.i = load i64* %CastToValueType905.i, align 8
  %.sum710.i = add i64 %loadedValue906.i, 14
  %98 = getelementptr float addrspace(3)* %10, i64 %.sum710.i
  %exData416.i = extractelement <16 x float> %add34234.i, i32 14
  store float %exData416.i, float addrspace(3)* %98, align 4
  br label %postload521.i

postload521.i:                                    ; preds = %preload520.i, %postload518.i
  br i1 %extract216.i, label %preload523.i, label %postload521.i.if.end.i_crit_edge

postload521.i.if.end.i_crit_edge:                 ; preds = %postload521.i
  br label %if.end.i

preload523.i:                                     ; preds = %postload521.i
  %"&(pSB[currWI].offset)908.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset909.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)908.i"
  %CastToValueType910.i = bitcast i8* %"&pSB[currWI].offset909.i" to i64*
  %loadedValue911.i = load i64* %CastToValueType910.i, align 8
  %.sum.i = add i64 %loadedValue911.i, 15
  %99 = getelementptr float addrspace(3)* %10, i64 %.sum.i
  %exData419.i = extractelement <16 x float> %add34234.i, i32 15
  store float %exData419.i, float addrspace(3)* %99, align 4
  br label %if.end.i

if.end.i:                                         ; preds = %postload521.i.if.end.i_crit_edge, %preload523.i
  %loadedValue943.i = load <16 x i1>* %CastToValueType938.i, align 16
  %extract238.i = extractelement <16 x i1> %loadedValue943.i, i32 0
  br i1 %extract238.i, label %preload604.i, label %if.end.i.postload605.i_crit_edge

if.end.i.postload605.i_crit_edge:                 ; preds = %if.end.i
  br label %postload605.i

preload604.i:                                     ; preds = %if.end.i
  %check.WI.iter974.i = icmp ult i64 %CurrWI..2.i, %22
  br i1 %check.WI.iter974.i, label %thenBB971.i, label %preload604.i.postload605.i_crit_edge

preload604.i.postload605.i_crit_edge:             ; preds = %preload604.i
  br label %postload605.i

thenBB971.i:                                      ; preds = %preload604.i
  %"CurrWI++975.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride977.i" = add nuw i64 %CurrSBIndex..2.i, 128
  %cond10.i = icmp eq i32 %currBarrier.1.i, 4
  br i1 %cond10.i, label %thenBB971.i.postload605.i_crit_edge, label %SyncBB.i

thenBB971.i.postload605.i_crit_edge:              ; preds = %thenBB971.i
  br label %postload605.i

postload605.i:                                    ; preds = %thenBB978.i.postload605.i_crit_edge, %thenBB971.i.postload605.i_crit_edge, %preload604.i.postload605.i_crit_edge, %if.end.i.postload605.i_crit_edge
  %currBarrier.3.i = phi i32 [ %currBarrier.1.i, %if.end.i.postload605.i_crit_edge ], [ 4, %preload604.i.postload605.i_crit_edge ], [ %currBarrier.1.i, %thenBB971.i.postload605.i_crit_edge ], [ %currBarrier.4.i, %thenBB978.i.postload605.i_crit_edge ]
  %CurrSBIndex..4.i = phi i64 [ %CurrSBIndex..2.i, %if.end.i.postload605.i_crit_edge ], [ 0, %preload604.i.postload605.i_crit_edge ], [ %"loadedCurrSB+Stride977.i", %thenBB971.i.postload605.i_crit_edge ], [ %"loadedCurrSB+Stride984.i", %thenBB978.i.postload605.i_crit_edge ]
  %CurrWI..4.i = phi i64 [ %CurrWI..2.i, %if.end.i.postload605.i_crit_edge ], [ 0, %preload604.i.postload605.i_crit_edge ], [ %"CurrWI++975.i", %thenBB971.i.postload605.i_crit_edge ], [ %"CurrWI++982.i", %thenBB978.i.postload605.i_crit_edge ]
  %"&(pSB[currWI].offset)959.i" = add nuw i64 %CurrSBIndex..4.i, 100
  %"&pSB[currWI].offset960.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)959.i"
  %CastToValueType961.i = bitcast i8* %"&pSB[currWI].offset960.i" to i32*
  %loadedValue962.i = load i32* %CastToValueType961.i, align 4
  %shr.i = lshr i32 %loadedValue962.i, 1
  %cmp25.i = icmp eq i32 %shr.i, 0
  %temp254.i = insertelement <16 x i1> undef, i1 %cmp25.i, i32 0
  %vector255.i = shufflevector <16 x i1> %temp254.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %notCond8.i = xor i1 %cmp25.i, true
  %temp260.i = insertelement <16 x i1> undef, i1 %notCond8.i, i32 0
  %vector261.i = shufflevector <16 x i1> %temp260.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)945.i" = add nuw i64 %CurrSBIndex..4.i, 96
  %"&pSB[currWI].offset946.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)945.i"
  %CastToValueType947.i = bitcast i8* %"&pSB[currWI].offset946.i" to <16 x i1>*
  %loadedValue948.i = load <16 x i1>* %CastToValueType947.i, align 16
  %who_left_tr9256.i = and <16 x i1> %loadedValue948.i, %vector255.i
  %"&(pSB[currWI].offset)931.i" = add nuw i64 %CurrSBIndex..4.i, 80
  %"&pSB[currWI].offset932.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)931.i"
  %CastToValueType933.i = bitcast i8* %"&pSB[currWI].offset932.i" to <16 x i1>*
  %loadedValue934.i = load <16 x i1>* %CastToValueType933.i, align 16
  %loop_mask12258.i = or <16 x i1> %loadedValue934.i, %who_left_tr9256.i
  %ipred.i1.i = bitcast <16 x i1> %loop_mask12258.i to i16
  %val.i2.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i1.i, i16 %ipred.i1.i) nounwind
  %100 = and i32 %val.i2.i, 1
  %res.i3.i = icmp eq i32 %100, 0
  %local_edge17262.i = and <16 x i1> %loadedValue948.i, %vector261.i
  br i1 %res.i3.i, label %for.body.i, label %for.end.i

for.end.i:                                        ; preds = %postload605.i, %SyncBB.i
  %currBarrier.4.i = phi i32 [ %currBarrier.0.i, %SyncBB.i ], [ %currBarrier.3.i, %postload605.i ]
  %CurrSBIndex..5.i = phi i64 [ %CurrSBIndex..1.i, %SyncBB.i ], [ %CurrSBIndex..4.i, %postload605.i ]
  %CurrWI..5.i = phi i64 [ %CurrWI..1.i, %SyncBB.i ], [ %CurrWI..4.i, %postload605.i ]
  %"&pSB[currWI].offset745.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..5.i
  %CastToValueType746.i = bitcast i8* %"&pSB[currWI].offset745.i" to <16 x i32>*
  %loadedValue747.i = load <16 x i32>* %CastToValueType746.i, align 64
  %cmp35.i = icmp eq <16 x i32> %loadedValue747.i, zeroinitializer
  %extract264.i = extractelement <16 x i1> %cmp35.i, i32 0
  %extract265.i = extractelement <16 x i1> %cmp35.i, i32 1
  %extract266.i = extractelement <16 x i1> %cmp35.i, i32 2
  %extract267.i = extractelement <16 x i1> %cmp35.i, i32 3
  %extract268.i = extractelement <16 x i1> %cmp35.i, i32 4
  %extract269.i = extractelement <16 x i1> %cmp35.i, i32 5
  %extract270.i = extractelement <16 x i1> %cmp35.i, i32 6
  %extract271.i = extractelement <16 x i1> %cmp35.i, i32 7
  %extract272.i = extractelement <16 x i1> %cmp35.i, i32 8
  %extract273.i = extractelement <16 x i1> %cmp35.i, i32 9
  %extract274.i = extractelement <16 x i1> %cmp35.i, i32 10
  %extract275.i = extractelement <16 x i1> %cmp35.i, i32 11
  %extract276.i = extractelement <16 x i1> %cmp35.i, i32 12
  %extract277.i = extractelement <16 x i1> %cmp35.i, i32 13
  %extract278.i = extractelement <16 x i1> %cmp35.i, i32 14
  %extract279.i = extractelement <16 x i1> %cmp35.i, i32 15
  br i1 %extract264.i, label %preload618.i, label %postload619.i

preload618.i:                                     ; preds = %for.end.i
  %masked_load420.i = load float addrspace(3)* %10, align 4
  br label %postload619.i

postload619.i:                                    ; preds = %preload618.i, %for.end.i
  %phi620.i = phi float [ undef, %for.end.i ], [ %masked_load420.i, %preload618.i ]
  br i1 %extract265.i, label %preload623.i, label %postload624.i

preload623.i:                                     ; preds = %postload619.i
  %masked_load421.i = load float addrspace(3)* %10, align 4
  br label %postload624.i

postload624.i:                                    ; preds = %preload623.i, %postload619.i
  %phi625.i = phi float [ undef, %postload619.i ], [ %masked_load421.i, %preload623.i ]
  br i1 %extract266.i, label %preload628.i, label %postload629.i

preload628.i:                                     ; preds = %postload624.i
  %masked_load422.i = load float addrspace(3)* %10, align 4
  br label %postload629.i

postload629.i:                                    ; preds = %preload628.i, %postload624.i
  %phi630.i = phi float [ undef, %postload624.i ], [ %masked_load422.i, %preload628.i ]
  br i1 %extract267.i, label %preload633.i, label %postload634.i

preload633.i:                                     ; preds = %postload629.i
  %masked_load423.i = load float addrspace(3)* %10, align 4
  br label %postload634.i

postload634.i:                                    ; preds = %preload633.i, %postload629.i
  %phi635.i = phi float [ undef, %postload629.i ], [ %masked_load423.i, %preload633.i ]
  br i1 %extract268.i, label %preload638.i, label %postload639.i

preload638.i:                                     ; preds = %postload634.i
  %masked_load424.i = load float addrspace(3)* %10, align 4
  br label %postload639.i

postload639.i:                                    ; preds = %preload638.i, %postload634.i
  %phi640.i = phi float [ undef, %postload634.i ], [ %masked_load424.i, %preload638.i ]
  br i1 %extract269.i, label %preload643.i, label %postload644.i

preload643.i:                                     ; preds = %postload639.i
  %masked_load425.i = load float addrspace(3)* %10, align 4
  br label %postload644.i

postload644.i:                                    ; preds = %preload643.i, %postload639.i
  %phi645.i = phi float [ undef, %postload639.i ], [ %masked_load425.i, %preload643.i ]
  br i1 %extract270.i, label %preload648.i, label %postload649.i

preload648.i:                                     ; preds = %postload644.i
  %masked_load426.i = load float addrspace(3)* %10, align 4
  br label %postload649.i

postload649.i:                                    ; preds = %preload648.i, %postload644.i
  %phi650.i = phi float [ undef, %postload644.i ], [ %masked_load426.i, %preload648.i ]
  br i1 %extract271.i, label %preload653.i, label %postload654.i

preload653.i:                                     ; preds = %postload649.i
  %masked_load427.i = load float addrspace(3)* %10, align 4
  br label %postload654.i

postload654.i:                                    ; preds = %preload653.i, %postload649.i
  %phi655.i = phi float [ undef, %postload649.i ], [ %masked_load427.i, %preload653.i ]
  br i1 %extract272.i, label %preload658.i, label %postload659.i

preload658.i:                                     ; preds = %postload654.i
  %masked_load428.i = load float addrspace(3)* %10, align 4
  br label %postload659.i

postload659.i:                                    ; preds = %preload658.i, %postload654.i
  %phi660.i = phi float [ undef, %postload654.i ], [ %masked_load428.i, %preload658.i ]
  br i1 %extract273.i, label %preload663.i, label %postload664.i

preload663.i:                                     ; preds = %postload659.i
  %masked_load429.i = load float addrspace(3)* %10, align 4
  br label %postload664.i

postload664.i:                                    ; preds = %preload663.i, %postload659.i
  %phi665.i = phi float [ undef, %postload659.i ], [ %masked_load429.i, %preload663.i ]
  br i1 %extract274.i, label %preload668.i, label %postload669.i

preload668.i:                                     ; preds = %postload664.i
  %masked_load430.i = load float addrspace(3)* %10, align 4
  br label %postload669.i

postload669.i:                                    ; preds = %preload668.i, %postload664.i
  %phi670.i = phi float [ undef, %postload664.i ], [ %masked_load430.i, %preload668.i ]
  br i1 %extract275.i, label %preload673.i, label %postload674.i

preload673.i:                                     ; preds = %postload669.i
  %masked_load431.i = load float addrspace(3)* %10, align 4
  br label %postload674.i

postload674.i:                                    ; preds = %preload673.i, %postload669.i
  %phi675.i = phi float [ undef, %postload669.i ], [ %masked_load431.i, %preload673.i ]
  br i1 %extract276.i, label %preload678.i, label %postload679.i

preload678.i:                                     ; preds = %postload674.i
  %masked_load432.i = load float addrspace(3)* %10, align 4
  br label %postload679.i

postload679.i:                                    ; preds = %preload678.i, %postload674.i
  %phi680.i = phi float [ undef, %postload674.i ], [ %masked_load432.i, %preload678.i ]
  br i1 %extract277.i, label %preload683.i, label %postload684.i

preload683.i:                                     ; preds = %postload679.i
  %masked_load433.i = load float addrspace(3)* %10, align 4
  br label %postload684.i

postload684.i:                                    ; preds = %preload683.i, %postload679.i
  %phi685.i = phi float [ undef, %postload679.i ], [ %masked_load433.i, %preload683.i ]
  br i1 %extract278.i, label %preload688.i, label %postload689.i

preload688.i:                                     ; preds = %postload684.i
  %masked_load434.i = load float addrspace(3)* %10, align 4
  br label %postload689.i

postload689.i:                                    ; preds = %preload688.i, %postload684.i
  %phi690.i = phi float [ undef, %postload684.i ], [ %masked_load434.i, %preload688.i ]
  br i1 %extract279.i, label %preload693.i, label %postload694.i

preload693.i:                                     ; preds = %postload689.i
  %masked_load435.i = load float addrspace(3)* %10, align 4
  br label %postload694.i

postload694.i:                                    ; preds = %preload693.i, %postload689.i
  %phi695.i = phi float [ undef, %postload689.i ], [ %masked_load435.i, %preload693.i ]
  br i1 %extract264.i, label %preload621.i, label %postload622.i

preload621.i:                                     ; preds = %postload694.i
  store float %phi620.i, float addrspace(1)* %arrayidx40.i, align 4
  br label %postload622.i

postload622.i:                                    ; preds = %preload621.i, %postload694.i
  br i1 %extract265.i, label %preload626.i, label %postload627.i

preload626.i:                                     ; preds = %postload622.i
  store float %phi625.i, float addrspace(1)* %arrayidx40.i, align 4
  br label %postload627.i

postload627.i:                                    ; preds = %preload626.i, %postload622.i
  br i1 %extract266.i, label %preload631.i, label %postload632.i

preload631.i:                                     ; preds = %postload627.i
  store float %phi630.i, float addrspace(1)* %arrayidx40.i, align 4
  br label %postload632.i

postload632.i:                                    ; preds = %preload631.i, %postload627.i
  br i1 %extract267.i, label %preload636.i, label %postload637.i

preload636.i:                                     ; preds = %postload632.i
  store float %phi635.i, float addrspace(1)* %arrayidx40.i, align 4
  br label %postload637.i

postload637.i:                                    ; preds = %preload636.i, %postload632.i
  br i1 %extract268.i, label %preload641.i, label %postload642.i

preload641.i:                                     ; preds = %postload637.i
  store float %phi640.i, float addrspace(1)* %arrayidx40.i, align 4
  br label %postload642.i

postload642.i:                                    ; preds = %preload641.i, %postload637.i
  br i1 %extract269.i, label %preload646.i, label %postload647.i

preload646.i:                                     ; preds = %postload642.i
  store float %phi645.i, float addrspace(1)* %arrayidx40.i, align 4
  br label %postload647.i

postload647.i:                                    ; preds = %preload646.i, %postload642.i
  br i1 %extract270.i, label %preload651.i, label %postload652.i

preload651.i:                                     ; preds = %postload647.i
  store float %phi650.i, float addrspace(1)* %arrayidx40.i, align 4
  br label %postload652.i

postload652.i:                                    ; preds = %preload651.i, %postload647.i
  br i1 %extract271.i, label %preload656.i, label %postload657.i

preload656.i:                                     ; preds = %postload652.i
  store float %phi655.i, float addrspace(1)* %arrayidx40.i, align 4
  br label %postload657.i

postload657.i:                                    ; preds = %preload656.i, %postload652.i
  br i1 %extract272.i, label %preload661.i, label %postload662.i

preload661.i:                                     ; preds = %postload657.i
  store float %phi660.i, float addrspace(1)* %arrayidx40.i, align 4
  br label %postload662.i

postload662.i:                                    ; preds = %preload661.i, %postload657.i
  br i1 %extract273.i, label %preload666.i, label %postload667.i

preload666.i:                                     ; preds = %postload662.i
  store float %phi665.i, float addrspace(1)* %arrayidx40.i, align 4
  br label %postload667.i

postload667.i:                                    ; preds = %preload666.i, %postload662.i
  br i1 %extract274.i, label %preload671.i, label %postload672.i

preload671.i:                                     ; preds = %postload667.i
  store float %phi670.i, float addrspace(1)* %arrayidx40.i, align 4
  br label %postload672.i

postload672.i:                                    ; preds = %preload671.i, %postload667.i
  br i1 %extract275.i, label %preload676.i, label %postload677.i

preload676.i:                                     ; preds = %postload672.i
  store float %phi675.i, float addrspace(1)* %arrayidx40.i, align 4
  br label %postload677.i

postload677.i:                                    ; preds = %preload676.i, %postload672.i
  br i1 %extract276.i, label %preload681.i, label %postload682.i

preload681.i:                                     ; preds = %postload677.i
  store float %phi680.i, float addrspace(1)* %arrayidx40.i, align 4
  br label %postload682.i

postload682.i:                                    ; preds = %preload681.i, %postload677.i
  br i1 %extract277.i, label %preload686.i, label %postload687.i

preload686.i:                                     ; preds = %postload682.i
  store float %phi685.i, float addrspace(1)* %arrayidx40.i, align 4
  br label %postload687.i

postload687.i:                                    ; preds = %preload686.i, %postload682.i
  br i1 %extract278.i, label %preload691.i, label %postload692.i

preload691.i:                                     ; preds = %postload687.i
  store float %phi690.i, float addrspace(1)* %arrayidx40.i, align 4
  br label %postload692.i

postload692.i:                                    ; preds = %preload691.i, %postload687.i
  br i1 %extract279.i, label %preload696.i, label %if.end41.i

preload696.i:                                     ; preds = %postload692.i
  store float %phi695.i, float addrspace(1)* %arrayidx40.i, align 4
  br label %if.end41.i

if.end41.i:                                       ; preds = %preload696.i, %postload692.i
  %check.WI.iter981.i = icmp ult i64 %CurrWI..5.i, %22
  br i1 %check.WI.iter981.i, label %thenBB978.i, label %____Vectorized_.reduce_separated_args.exit

thenBB978.i:                                      ; preds = %if.end41.i
  %"CurrWI++982.i" = add nuw i64 %CurrWI..5.i, 1
  %"loadedCurrSB+Stride984.i" = add nuw i64 %CurrSBIndex..5.i, 128
  %cond9.i = icmp eq i32 %currBarrier.4.i, 4
  br i1 %cond9.i, label %thenBB978.i.postload605.i_crit_edge, label %SyncBB.i

thenBB978.i.postload605.i_crit_edge:              ; preds = %thenBB978.i
  br label %postload605.i

____Vectorized_.reduce_separated_args.exit:       ; preds = %if.end41.i
  ret void
}

define void @__Vectorized_.bottom_scan(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float addrspace(1)**
  %7 = load float addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to float addrspace(3)**
  %13 = load float addrspace(3)** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to i8 addrspace(3)**
  %16 = load i8 addrspace(3)** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 48
  %18 = bitcast i8* %17 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %19 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 56
  %21 = bitcast i8* %20 to i64**
  %22 = load i64** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 72
  %24 = bitcast i8* %23 to <{ [4 x i64] }>**
  %25 = load <{ [4 x i64] }>** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 88
  %27 = bitcast i8* %26 to i64*
  %28 = load i64* %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 96
  %30 = bitcast i8* %29 to i8**
  %31 = load i8** %30, align 8
  %32 = bitcast i8 addrspace(3)* %16 to float addrspace(3)*
  %33 = bitcast float addrspace(1)* %1 to <4 x float> addrspace(1)*
  %34 = bitcast float addrspace(1)* %7 to <4 x float> addrspace(1)*
  %div.i = sdiv i32 %10, 4
  %conv.i = sext i32 %div.i to i64
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %thenBB2969.i, %thenBB2961.i, %thenBB2953.i, %thenBB2945.i, %entry
  %currBarrier.0.i = phi i32 [ 24, %entry ], [ %currBarrier.1.i, %thenBB2945.i ], [ %currBarrier.6.i, %thenBB2961.i ], [ %currBarrier.4.i, %thenBB2953.i ], [ %currBarrier.9.i, %thenBB2969.i ], [ %currBarrier.12.i, %thenBB.i ]
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride2951.i", %thenBB2945.i ], [ %"loadedCurrSB+Stride2967.i", %thenBB2961.i ], [ %"loadedCurrSB+Stride2959.i", %thenBB2953.i ], [ %"loadedCurrSB+Stride2975.i", %thenBB2969.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++2949.i", %thenBB2945.i ], [ %"CurrWI++2965.i", %thenBB2961.i ], [ %"CurrWI++2957.i", %thenBB2953.i ], [ %"CurrWI++2973.i", %thenBB2969.i ], [ %"CurrWI++.i", %thenBB.i ]
  %35 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 4, i64 0
  %36 = load i64* %35, align 8
  %isDivisorZero1238.i = icmp eq i64 %36, 0
  %newiDvisor1240.i = select i1 %isDivisorZero1238.i, i64 1, i64 %36
  %div1.i = udiv i64 %conv.i, %newiDvisor1240.i
  %conv2.i = trunc i64 %div1.i to i32
  %37 = load i64* %22, align 8
  %conv4.i = sext i32 %conv2.i to i64
  %mul.i = mul i64 %conv4.i, %37
  %conv5.i = trunc i64 %mul.i to i32
  %sub.i = add i64 %36, -1
  %cmp.i = icmp eq i64 %37, %sub.i
  %add.i = add nsw i32 %conv5.i, %conv2.i
  %cond.i = select i1 %cmp.i, i32 %div.i, i32 %add.i
  %temp97.i = insertelement <16 x i32> undef, i32 %cond.i, i32 0
  %vector98.i = shufflevector <16 x i32> %temp97.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %38 = getelementptr <{ [4 x i64] }>* %25, i64 %CurrWI..0.i, i32 0, i64 0
  %39 = load i64* %38, align 8
  %broadcast1.i = insertelement <16 x i64> undef, i64 %39, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %40 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %31, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to <16 x i64>*
  store <16 x i64> %40, <16 x i64>* %CastToValueType.i, align 128
  %cmp144.i = icmp slt i32 %conv5.i, %cond.i
  %temp90.i = insertelement <16 x i1> undef, i1 %cmp144.i, i32 0
  %vector91.i = shufflevector <16 x i1> %temp90.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %temp.i = insertelement <16 x i64> undef, i64 %mul.i, i32 0
  %vector.i = shufflevector <16 x i64> %temp.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %add1183.i = add <16 x i64> %vector.i, %40
  br i1 %cmp144.i, label %preload824.i, label %postload825.i

preload824.i:                                     ; preds = %SyncBB.i
  %arrayidx.i = getelementptr inbounds float addrspace(1)* %4, i64 %37
  %masked_load.i = load float addrspace(1)* %arrayidx.i, align 4
  br label %postload825.i

postload825.i:                                    ; preds = %preload824.i, %SyncBB.i
  %phi826.i = phi float [ undef, %SyncBB.i ], [ %masked_load.i, %preload824.i ]
  %temp94.i = insertelement <16 x float> undef, float %phi826.i, i32 0
  %vector95.i = shufflevector <16 x float> %temp94.i, <16 x float> undef, <16 x i32> zeroinitializer
  br i1 %cmp144.i, label %preload827.i, label %postload828.i

preload827.i:                                     ; preds = %postload825.i
  %41 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %42 = load i64* %41, align 8
  br label %postload828.i

postload828.i:                                    ; preds = %preload827.i, %postload825.i
  %phi829.i = phi i64 [ undef, %postload825.i ], [ %42, %preload827.i ]
  %"&(pSB[currWI].offset)1271.i" = add nuw i64 %CurrSBIndex..0.i, 128
  %"&pSB[currWI].offset1272.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1271.i"
  %CastToValueType1273.i = bitcast i8* %"&pSB[currWI].offset1272.i" to i64*
  store i64 %phi829.i, i64* %CastToValueType1273.i, align 8
  %temp580.i = insertelement <16 x i64> undef, i64 %phi829.i, i32 0
  %vector581.i = shufflevector <16 x i64> %temp580.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)1280.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1281.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1280.i"
  %CastToValueType1282.i = bitcast i8* %"&pSB[currWI].offset1281.i" to <16 x i64>*
  store <16 x i64> %vector581.i, <16 x i64>* %CastToValueType1282.i, align 128
  %sub39.i = add i64 %phi829.i, -1
  %temp84.i = insertelement <16 x i64> undef, i64 %sub39.i, i32 0
  %vector85.i = shufflevector <16 x i64> %temp84.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %cmp40.i = icmp eq <16 x i64> %40, %vector85.i
  %"&(pSB[currWI].offset)1289.i" = add nuw i64 %CurrSBIndex..0.i, 384
  %"&pSB[currWI].offset1290.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1289.i"
  %CastToValueType1291.i = bitcast i8* %"&pSB[currWI].offset1290.i" to <16 x i1>*
  store <16 x i1> %cmp40.i, <16 x i1>* %CastToValueType1291.i, align 16
  %negIncomingLoopMask.i = xor i1 %cmp144.i, true
  %temp87.i = insertelement <16 x i1> undef, i1 %negIncomingLoopMask.i, i32 0
  %vector88.i = shufflevector <16 x i1> %temp87.i, <16 x i1> undef, <16 x i32> zeroinitializer
  br i1 %cmp144.i, label %while.body.i, label %while.end.i

while.body.i:                                     ; preds = %postload973.i, %postload828.i
  %currBarrier.1.i = phi i32 [ %currBarrier.11.i, %postload973.i ], [ %currBarrier.0.i, %postload828.i ]
  %CurrSBIndex..1.i = phi i64 [ %CurrSBIndex..11.i, %postload973.i ], [ %CurrSBIndex..0.i, %postload828.i ]
  %CurrWI..1.i = phi i64 [ %CurrWI..11.i, %postload973.i ], [ %CurrWI..0.i, %postload828.i ]
  %vectorPHI86.i = phi <16 x i1> [ %loop_mask26587.i, %postload973.i ], [ %vector88.i, %postload828.i ]
  %loadedValue1389.i = phi <16 x i1> [ %local_edge31608.i, %postload973.i ], [ %vector91.i, %postload828.i ]
  %vectorPHI92.i = phi <16 x i64> [ %add50582.i, %postload973.i ], [ %add1183.i, %postload828.i ]
  %vectorPHI93.i = phi <16 x float> [ %temp.vect640.i, %postload973.i ], [ %vector95.i, %postload828.i ]
  %window.05.i = phi i32 [ %conv47.i, %postload973.i ], [ %conv5.i, %postload828.i ]
  %"&(pSB[currWI].offset)1420.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1421.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1420.i"
  %CastToValueType1422.i = bitcast i8* %"&pSB[currWI].offset1421.i" to i32*
  store i32 %window.05.i, i32* %CastToValueType1422.i, align 4
  %"&(pSB[currWI].offset)1411.i" = add nuw i64 %CurrSBIndex..1.i, 448
  %"&pSB[currWI].offset1412.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1411.i"
  %CastToValueType1413.i = bitcast i8* %"&pSB[currWI].offset1412.i" to <16 x float>*
  store <16 x float> %vectorPHI93.i, <16 x float>* %CastToValueType1413.i, align 64
  %"&(pSB[currWI].offset)1307.i" = add nuw i64 %CurrSBIndex..1.i, 416
  %"&pSB[currWI].offset1308.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1307.i"
  %CastToValueType1309.i = bitcast i8* %"&pSB[currWI].offset1308.i" to <16 x i1>*
  store <16 x i1> %loadedValue1389.i, <16 x i1>* %CastToValueType1309.i, align 16
  %"&(pSB[currWI].offset)1298.i" = add nuw i64 %CurrSBIndex..1.i, 400
  %"&pSB[currWI].offset1299.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1298.i"
  %CastToValueType1300.i = bitcast i8* %"&pSB[currWI].offset1299.i" to <16 x i1>*
  store <16 x i1> %vectorPHI86.i, <16 x i1>* %CastToValueType1300.i, align 16
  %i.0796.i = trunc <16 x i64> %vectorPHI92.i to <16 x i32>
  %cmp16.i = icmp slt <16 x i32> %i.0796.i, %vector98.i
  %while.body_to_if.then101.i = and <16 x i1> %loadedValue1389.i, %cmp16.i
  %extract119.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 1
  %"&(pSB[currWI].offset)1429.i" = add nuw i64 %CurrSBIndex..1.i, 516
  %"&pSB[currWI].offset1430.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1429.i"
  %CastToValueType1431.i = bitcast i8* %"&pSB[currWI].offset1430.i" to i1*
  store i1 %extract119.i, i1* %CastToValueType1431.i, align 1
  %extract120.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 2
  %"&(pSB[currWI].offset)1443.i" = add nuw i64 %CurrSBIndex..1.i, 517
  %"&pSB[currWI].offset1444.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1443.i"
  %CastToValueType1445.i = bitcast i8* %"&pSB[currWI].offset1444.i" to i1*
  store i1 %extract120.i, i1* %CastToValueType1445.i, align 1
  %extract121.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 3
  %"&(pSB[currWI].offset)1457.i" = add nuw i64 %CurrSBIndex..1.i, 518
  %"&pSB[currWI].offset1458.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1457.i"
  %CastToValueType1459.i = bitcast i8* %"&pSB[currWI].offset1458.i" to i1*
  store i1 %extract121.i, i1* %CastToValueType1459.i, align 1
  %extract122.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 4
  %"&(pSB[currWI].offset)1471.i" = add nuw i64 %CurrSBIndex..1.i, 519
  %"&pSB[currWI].offset1472.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1471.i"
  %CastToValueType1473.i = bitcast i8* %"&pSB[currWI].offset1472.i" to i1*
  store i1 %extract122.i, i1* %CastToValueType1473.i, align 1
  %extract123.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 5
  %"&(pSB[currWI].offset)1485.i" = add nuw i64 %CurrSBIndex..1.i, 520
  %"&pSB[currWI].offset1486.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1485.i"
  %CastToValueType1487.i = bitcast i8* %"&pSB[currWI].offset1486.i" to i1*
  store i1 %extract123.i, i1* %CastToValueType1487.i, align 1
  %extract124.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 6
  %"&(pSB[currWI].offset)1499.i" = add nuw i64 %CurrSBIndex..1.i, 521
  %"&pSB[currWI].offset1500.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1499.i"
  %CastToValueType1501.i = bitcast i8* %"&pSB[currWI].offset1500.i" to i1*
  store i1 %extract124.i, i1* %CastToValueType1501.i, align 1
  %extract125.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 7
  %"&(pSB[currWI].offset)1513.i" = add nuw i64 %CurrSBIndex..1.i, 522
  %"&pSB[currWI].offset1514.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1513.i"
  %CastToValueType1515.i = bitcast i8* %"&pSB[currWI].offset1514.i" to i1*
  store i1 %extract125.i, i1* %CastToValueType1515.i, align 1
  %extract126.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 8
  %"&(pSB[currWI].offset)1527.i" = add nuw i64 %CurrSBIndex..1.i, 523
  %"&pSB[currWI].offset1528.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1527.i"
  %CastToValueType1529.i = bitcast i8* %"&pSB[currWI].offset1528.i" to i1*
  store i1 %extract126.i, i1* %CastToValueType1529.i, align 1
  %extract127.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 9
  %"&(pSB[currWI].offset)1541.i" = add nuw i64 %CurrSBIndex..1.i, 524
  %"&pSB[currWI].offset1542.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1541.i"
  %CastToValueType1543.i = bitcast i8* %"&pSB[currWI].offset1542.i" to i1*
  store i1 %extract127.i, i1* %CastToValueType1543.i, align 1
  %extract128.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 10
  %"&(pSB[currWI].offset)1555.i" = add nuw i64 %CurrSBIndex..1.i, 525
  %"&pSB[currWI].offset1556.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1555.i"
  %CastToValueType1557.i = bitcast i8* %"&pSB[currWI].offset1556.i" to i1*
  store i1 %extract128.i, i1* %CastToValueType1557.i, align 1
  %extract129.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 11
  %"&(pSB[currWI].offset)1569.i" = add nuw i64 %CurrSBIndex..1.i, 526
  %"&pSB[currWI].offset1570.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1569.i"
  %CastToValueType1571.i = bitcast i8* %"&pSB[currWI].offset1570.i" to i1*
  store i1 %extract129.i, i1* %CastToValueType1571.i, align 1
  %extract130.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 12
  %"&(pSB[currWI].offset)1583.i" = add nuw i64 %CurrSBIndex..1.i, 527
  %"&pSB[currWI].offset1584.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1583.i"
  %CastToValueType1585.i = bitcast i8* %"&pSB[currWI].offset1584.i" to i1*
  store i1 %extract130.i, i1* %CastToValueType1585.i, align 1
  %extract131.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 13
  %"&(pSB[currWI].offset)1597.i" = add nuw i64 %CurrSBIndex..1.i, 528
  %"&pSB[currWI].offset1598.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1597.i"
  %CastToValueType1599.i = bitcast i8* %"&pSB[currWI].offset1598.i" to i1*
  store i1 %extract131.i, i1* %CastToValueType1599.i, align 1
  %extract132.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 14
  %"&(pSB[currWI].offset)1611.i" = add nuw i64 %CurrSBIndex..1.i, 529
  %"&pSB[currWI].offset1612.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1611.i"
  %CastToValueType1613.i = bitcast i8* %"&pSB[currWI].offset1612.i" to i1*
  store i1 %extract132.i, i1* %CastToValueType1613.i, align 1
  %extract133.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 15
  %"&(pSB[currWI].offset)1625.i" = add nuw i64 %CurrSBIndex..1.i, 530
  %"&pSB[currWI].offset1626.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1625.i"
  %CastToValueType1627.i = bitcast i8* %"&pSB[currWI].offset1626.i" to i1*
  store i1 %extract133.i, i1* %CastToValueType1627.i, align 1
  %extract118.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 0
  %"&(pSB[currWI].offset)1639.i" = add nuw i64 %CurrSBIndex..1.i, 531
  %"&pSB[currWI].offset1640.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1639.i"
  %CastToValueType1641.i = bitcast i8* %"&pSB[currWI].offset1640.i" to i1*
  store i1 %extract118.i, i1* %CastToValueType1641.i, align 1
  %idxprom102.i = sext <16 x i32> %i.0796.i to <16 x i64>
  %"&(pSB[currWI].offset)1648.i" = add nuw i64 %CurrSBIndex..1.i, 640
  %"&pSB[currWI].offset1649.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1648.i"
  %CastToValueType1650.i = bitcast i8* %"&pSB[currWI].offset1649.i" to <16 x i64>*
  store <16 x i64> %idxprom102.i, <16 x i64>* %CastToValueType1650.i, align 128
  %extract103.i = extractelement <16 x i64> %idxprom102.i, i32 1
  %"&(pSB[currWI].offset)1667.i" = add nuw i64 %CurrSBIndex..1.i, 768
  %"&pSB[currWI].offset1668.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1667.i"
  %CastToValueType1669.i = bitcast i8* %"&pSB[currWI].offset1668.i" to i64*
  store i64 %extract103.i, i64* %CastToValueType1669.i, align 8
  %extract104.i = extractelement <16 x i64> %idxprom102.i, i32 2
  %"&(pSB[currWI].offset)1676.i" = add nuw i64 %CurrSBIndex..1.i, 776
  %"&pSB[currWI].offset1677.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1676.i"
  %CastToValueType1678.i = bitcast i8* %"&pSB[currWI].offset1677.i" to i64*
  store i64 %extract104.i, i64* %CastToValueType1678.i, align 8
  %extract105.i = extractelement <16 x i64> %idxprom102.i, i32 3
  %"&(pSB[currWI].offset)1685.i" = add nuw i64 %CurrSBIndex..1.i, 784
  %"&pSB[currWI].offset1686.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1685.i"
  %CastToValueType1687.i = bitcast i8* %"&pSB[currWI].offset1686.i" to i64*
  store i64 %extract105.i, i64* %CastToValueType1687.i, align 8
  %extract106.i = extractelement <16 x i64> %idxprom102.i, i32 4
  %"&(pSB[currWI].offset)1694.i" = add nuw i64 %CurrSBIndex..1.i, 792
  %"&pSB[currWI].offset1695.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1694.i"
  %CastToValueType1696.i = bitcast i8* %"&pSB[currWI].offset1695.i" to i64*
  store i64 %extract106.i, i64* %CastToValueType1696.i, align 8
  %extract107.i = extractelement <16 x i64> %idxprom102.i, i32 5
  %"&(pSB[currWI].offset)1703.i" = add nuw i64 %CurrSBIndex..1.i, 800
  %"&pSB[currWI].offset1704.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1703.i"
  %CastToValueType1705.i = bitcast i8* %"&pSB[currWI].offset1704.i" to i64*
  store i64 %extract107.i, i64* %CastToValueType1705.i, align 8
  %extract108.i = extractelement <16 x i64> %idxprom102.i, i32 6
  %"&(pSB[currWI].offset)1712.i" = add nuw i64 %CurrSBIndex..1.i, 808
  %"&pSB[currWI].offset1713.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1712.i"
  %CastToValueType1714.i = bitcast i8* %"&pSB[currWI].offset1713.i" to i64*
  store i64 %extract108.i, i64* %CastToValueType1714.i, align 8
  %extract109.i = extractelement <16 x i64> %idxprom102.i, i32 7
  %"&(pSB[currWI].offset)1721.i" = add nuw i64 %CurrSBIndex..1.i, 816
  %"&pSB[currWI].offset1722.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1721.i"
  %CastToValueType1723.i = bitcast i8* %"&pSB[currWI].offset1722.i" to i64*
  store i64 %extract109.i, i64* %CastToValueType1723.i, align 8
  %extract110.i = extractelement <16 x i64> %idxprom102.i, i32 8
  %"&(pSB[currWI].offset)1730.i" = add nuw i64 %CurrSBIndex..1.i, 824
  %"&pSB[currWI].offset1731.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1730.i"
  %CastToValueType1732.i = bitcast i8* %"&pSB[currWI].offset1731.i" to i64*
  store i64 %extract110.i, i64* %CastToValueType1732.i, align 8
  %extract111.i = extractelement <16 x i64> %idxprom102.i, i32 9
  %"&(pSB[currWI].offset)1739.i" = add nuw i64 %CurrSBIndex..1.i, 832
  %"&pSB[currWI].offset1740.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1739.i"
  %CastToValueType1741.i = bitcast i8* %"&pSB[currWI].offset1740.i" to i64*
  store i64 %extract111.i, i64* %CastToValueType1741.i, align 8
  %extract112.i = extractelement <16 x i64> %idxprom102.i, i32 10
  %"&(pSB[currWI].offset)1748.i" = add nuw i64 %CurrSBIndex..1.i, 840
  %"&pSB[currWI].offset1749.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1748.i"
  %CastToValueType1750.i = bitcast i8* %"&pSB[currWI].offset1749.i" to i64*
  store i64 %extract112.i, i64* %CastToValueType1750.i, align 8
  %extract113.i = extractelement <16 x i64> %idxprom102.i, i32 11
  %"&(pSB[currWI].offset)1757.i" = add nuw i64 %CurrSBIndex..1.i, 848
  %"&pSB[currWI].offset1758.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1757.i"
  %CastToValueType1759.i = bitcast i8* %"&pSB[currWI].offset1758.i" to i64*
  store i64 %extract113.i, i64* %CastToValueType1759.i, align 8
  %extract114.i = extractelement <16 x i64> %idxprom102.i, i32 12
  %"&(pSB[currWI].offset)1766.i" = add nuw i64 %CurrSBIndex..1.i, 856
  %"&pSB[currWI].offset1767.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1766.i"
  %CastToValueType1768.i = bitcast i8* %"&pSB[currWI].offset1767.i" to i64*
  store i64 %extract114.i, i64* %CastToValueType1768.i, align 8
  %extract115.i = extractelement <16 x i64> %idxprom102.i, i32 13
  %"&(pSB[currWI].offset)1775.i" = add nuw i64 %CurrSBIndex..1.i, 864
  %"&pSB[currWI].offset1776.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1775.i"
  %CastToValueType1777.i = bitcast i8* %"&pSB[currWI].offset1776.i" to i64*
  store i64 %extract115.i, i64* %CastToValueType1777.i, align 8
  %extract116.i = extractelement <16 x i64> %idxprom102.i, i32 14
  %"&(pSB[currWI].offset)1784.i" = add nuw i64 %CurrSBIndex..1.i, 872
  %"&pSB[currWI].offset1785.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1784.i"
  %CastToValueType1786.i = bitcast i8* %"&pSB[currWI].offset1785.i" to i64*
  store i64 %extract116.i, i64* %CastToValueType1786.i, align 8
  %extract117.i = extractelement <16 x i64> %idxprom102.i, i32 15
  %"&(pSB[currWI].offset)1793.i" = add nuw i64 %CurrSBIndex..1.i, 880
  %"&pSB[currWI].offset1794.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1793.i"
  %CastToValueType1795.i = bitcast i8* %"&pSB[currWI].offset1794.i" to i64*
  store i64 %extract117.i, i64* %CastToValueType1795.i, align 8
  %43 = getelementptr inbounds <4 x float> addrspace(1)* %33, i64 %extract103.i
  %44 = getelementptr inbounds <4 x float> addrspace(1)* %33, i64 %extract104.i
  %45 = getelementptr inbounds <4 x float> addrspace(1)* %33, i64 %extract105.i
  %46 = getelementptr inbounds <4 x float> addrspace(1)* %33, i64 %extract106.i
  %47 = getelementptr inbounds <4 x float> addrspace(1)* %33, i64 %extract107.i
  %48 = getelementptr inbounds <4 x float> addrspace(1)* %33, i64 %extract108.i
  %49 = getelementptr inbounds <4 x float> addrspace(1)* %33, i64 %extract109.i
  %50 = getelementptr inbounds <4 x float> addrspace(1)* %33, i64 %extract110.i
  %51 = getelementptr inbounds <4 x float> addrspace(1)* %33, i64 %extract111.i
  %52 = getelementptr inbounds <4 x float> addrspace(1)* %33, i64 %extract112.i
  %53 = getelementptr inbounds <4 x float> addrspace(1)* %33, i64 %extract113.i
  %54 = getelementptr inbounds <4 x float> addrspace(1)* %33, i64 %extract114.i
  %55 = getelementptr inbounds <4 x float> addrspace(1)* %33, i64 %extract115.i
  %56 = getelementptr inbounds <4 x float> addrspace(1)* %33, i64 %extract116.i
  %57 = getelementptr inbounds <4 x float> addrspace(1)* %33, i64 %extract117.i
  br i1 %extract118.i, label %preload791.i, label %postload792.i

preload791.i:                                     ; preds = %while.body.i
  %extract.i = extractelement <16 x i64> %idxprom102.i, i32 0
  %58 = getelementptr inbounds <4 x float> addrspace(1)* %33, i64 %extract.i
  %masked_load641.i = load <4 x float> addrspace(1)* %58, align 16
  br label %postload792.i

postload792.i:                                    ; preds = %preload791.i, %while.body.i
  %phi793.i = phi <4 x float> [ undef, %while.body.i ], [ %masked_load641.i, %preload791.i ]
  br i1 %extract119.i, label %preload794.i, label %postload795.i

preload794.i:                                     ; preds = %postload792.i
  %masked_load642.i = load <4 x float> addrspace(1)* %43, align 16
  br label %postload795.i

postload795.i:                                    ; preds = %preload794.i, %postload792.i
  %phi796.i = phi <4 x float> [ undef, %postload792.i ], [ %masked_load642.i, %preload794.i ]
  br i1 %extract120.i, label %preload797.i, label %postload798.i

preload797.i:                                     ; preds = %postload795.i
  %masked_load643.i = load <4 x float> addrspace(1)* %44, align 16
  br label %postload798.i

postload798.i:                                    ; preds = %preload797.i, %postload795.i
  %phi799.i = phi <4 x float> [ undef, %postload795.i ], [ %masked_load643.i, %preload797.i ]
  br i1 %extract121.i, label %preload800.i, label %postload801.i

preload800.i:                                     ; preds = %postload798.i
  %masked_load644.i = load <4 x float> addrspace(1)* %45, align 16
  br label %postload801.i

postload801.i:                                    ; preds = %preload800.i, %postload798.i
  %phi802.i = phi <4 x float> [ undef, %postload798.i ], [ %masked_load644.i, %preload800.i ]
  br i1 %extract122.i, label %preload803.i, label %postload804.i

preload803.i:                                     ; preds = %postload801.i
  %masked_load645.i = load <4 x float> addrspace(1)* %46, align 16
  br label %postload804.i

postload804.i:                                    ; preds = %preload803.i, %postload801.i
  %phi805.i = phi <4 x float> [ undef, %postload801.i ], [ %masked_load645.i, %preload803.i ]
  br i1 %extract123.i, label %preload947.i, label %postload948.i

preload947.i:                                     ; preds = %postload804.i
  %masked_load646.i = load <4 x float> addrspace(1)* %47, align 16
  br label %postload948.i

postload948.i:                                    ; preds = %preload947.i, %postload804.i
  %phi949.i = phi <4 x float> [ undef, %postload804.i ], [ %masked_load646.i, %preload947.i ]
  br i1 %extract124.i, label %preload950.i, label %postload951.i

preload950.i:                                     ; preds = %postload948.i
  %masked_load647.i = load <4 x float> addrspace(1)* %48, align 16
  br label %postload951.i

postload951.i:                                    ; preds = %preload950.i, %postload948.i
  %phi952.i = phi <4 x float> [ undef, %postload948.i ], [ %masked_load647.i, %preload950.i ]
  br i1 %extract125.i, label %preload953.i, label %postload954.i

preload953.i:                                     ; preds = %postload951.i
  %masked_load648.i = load <4 x float> addrspace(1)* %49, align 16
  br label %postload954.i

postload954.i:                                    ; preds = %preload953.i, %postload951.i
  %phi955.i = phi <4 x float> [ undef, %postload951.i ], [ %masked_load648.i, %preload953.i ]
  br i1 %extract126.i, label %preload956.i, label %postload957.i

preload956.i:                                     ; preds = %postload954.i
  %masked_load649.i = load <4 x float> addrspace(1)* %50, align 16
  br label %postload957.i

postload957.i:                                    ; preds = %preload956.i, %postload954.i
  %phi958.i = phi <4 x float> [ undef, %postload954.i ], [ %masked_load649.i, %preload956.i ]
  br i1 %extract127.i, label %preload959.i, label %postload960.i

preload959.i:                                     ; preds = %postload957.i
  %masked_load650.i = load <4 x float> addrspace(1)* %51, align 16
  br label %postload960.i

postload960.i:                                    ; preds = %preload959.i, %postload957.i
  %phi961.i = phi <4 x float> [ undef, %postload957.i ], [ %masked_load650.i, %preload959.i ]
  br i1 %extract128.i, label %preload830.i, label %postload831.i

preload830.i:                                     ; preds = %postload960.i
  %masked_load651.i = load <4 x float> addrspace(1)* %52, align 16
  br label %postload831.i

postload831.i:                                    ; preds = %preload830.i, %postload960.i
  %phi832.i = phi <4 x float> [ undef, %postload960.i ], [ %masked_load651.i, %preload830.i ]
  br i1 %extract129.i, label %preload833.i, label %postload834.i

preload833.i:                                     ; preds = %postload831.i
  %masked_load652.i = load <4 x float> addrspace(1)* %53, align 16
  br label %postload834.i

postload834.i:                                    ; preds = %preload833.i, %postload831.i
  %phi835.i = phi <4 x float> [ undef, %postload831.i ], [ %masked_load652.i, %preload833.i ]
  br i1 %extract130.i, label %preload836.i, label %postload837.i

preload836.i:                                     ; preds = %postload834.i
  %masked_load653.i = load <4 x float> addrspace(1)* %54, align 16
  br label %postload837.i

postload837.i:                                    ; preds = %preload836.i, %postload834.i
  %phi838.i = phi <4 x float> [ undef, %postload834.i ], [ %masked_load653.i, %preload836.i ]
  br i1 %extract131.i, label %preload839.i, label %postload840.i

preload839.i:                                     ; preds = %postload837.i
  %masked_load654.i = load <4 x float> addrspace(1)* %55, align 16
  br label %postload840.i

postload840.i:                                    ; preds = %preload839.i, %postload837.i
  %phi841.i = phi <4 x float> [ undef, %postload837.i ], [ %masked_load654.i, %preload839.i ]
  br i1 %extract132.i, label %preload842.i, label %postload843.i

preload842.i:                                     ; preds = %postload840.i
  %masked_load655.i = load <4 x float> addrspace(1)* %56, align 16
  br label %postload843.i

postload843.i:                                    ; preds = %preload842.i, %postload840.i
  %phi844.i = phi <4 x float> [ undef, %postload840.i ], [ %masked_load655.i, %preload842.i ]
  br i1 %extract133.i, label %preload845.i, label %postload843.i.if.end.i_crit_edge

postload843.i.if.end.i_crit_edge:                 ; preds = %postload843.i
  br label %if.end.i

preload845.i:                                     ; preds = %postload843.i
  %masked_load656.i = load <4 x float> addrspace(1)* %57, align 16
  br label %if.end.i

if.end.i:                                         ; preds = %postload843.i.if.end.i_crit_edge, %preload845.i
  %phi847.i = phi <4 x float> [ %masked_load656.i, %preload845.i ], [ undef, %postload843.i.if.end.i_crit_edge ]
  %59 = extractelement <4 x float> %phi793.i, i32 3
  %60 = extractelement <4 x float> %phi793.i, i32 2
  %61 = extractelement <4 x float> %phi793.i, i32 1
  %62 = extractelement <4 x float> %phi793.i, i32 0
  %temp.vect136.i = insertelement <16 x float> undef, float %59, i32 0
  %63 = extractelement <4 x float> %phi796.i, i32 3
  %temp.vect153.i = insertelement <16 x float> undef, float %60, i32 0
  %64 = extractelement <4 x float> %phi796.i, i32 2
  %temp.vect170.i = insertelement <16 x float> undef, float %61, i32 0
  %65 = extractelement <4 x float> %phi796.i, i32 1
  %temp.vect187.i = insertelement <16 x float> undef, float %62, i32 0
  %66 = extractelement <4 x float> %phi796.i, i32 0
  %temp.vect137.i = insertelement <16 x float> %temp.vect136.i, float %63, i32 1
  %67 = extractelement <4 x float> %phi799.i, i32 3
  %temp.vect154.i = insertelement <16 x float> %temp.vect153.i, float %64, i32 1
  %68 = extractelement <4 x float> %phi799.i, i32 2
  %temp.vect171.i = insertelement <16 x float> %temp.vect170.i, float %65, i32 1
  %69 = extractelement <4 x float> %phi799.i, i32 1
  %temp.vect188.i = insertelement <16 x float> %temp.vect187.i, float %66, i32 1
  %70 = extractelement <4 x float> %phi799.i, i32 0
  %temp.vect138.i = insertelement <16 x float> %temp.vect137.i, float %67, i32 2
  %71 = extractelement <4 x float> %phi802.i, i32 3
  %temp.vect155.i = insertelement <16 x float> %temp.vect154.i, float %68, i32 2
  %72 = extractelement <4 x float> %phi802.i, i32 2
  %temp.vect172.i = insertelement <16 x float> %temp.vect171.i, float %69, i32 2
  %73 = extractelement <4 x float> %phi802.i, i32 1
  %temp.vect189.i = insertelement <16 x float> %temp.vect188.i, float %70, i32 2
  %74 = extractelement <4 x float> %phi802.i, i32 0
  %temp.vect139.i = insertelement <16 x float> %temp.vect138.i, float %71, i32 3
  %75 = extractelement <4 x float> %phi805.i, i32 3
  %temp.vect156.i = insertelement <16 x float> %temp.vect155.i, float %72, i32 3
  %76 = extractelement <4 x float> %phi805.i, i32 2
  %temp.vect173.i = insertelement <16 x float> %temp.vect172.i, float %73, i32 3
  %77 = extractelement <4 x float> %phi805.i, i32 1
  %temp.vect190.i = insertelement <16 x float> %temp.vect189.i, float %74, i32 3
  %78 = extractelement <4 x float> %phi805.i, i32 0
  %temp.vect140.i = insertelement <16 x float> %temp.vect139.i, float %75, i32 4
  %79 = extractelement <4 x float> %phi949.i, i32 3
  %temp.vect157.i = insertelement <16 x float> %temp.vect156.i, float %76, i32 4
  %80 = extractelement <4 x float> %phi949.i, i32 2
  %temp.vect174.i = insertelement <16 x float> %temp.vect173.i, float %77, i32 4
  %81 = extractelement <4 x float> %phi949.i, i32 1
  %temp.vect191.i = insertelement <16 x float> %temp.vect190.i, float %78, i32 4
  %82 = extractelement <4 x float> %phi949.i, i32 0
  %temp.vect141.i = insertelement <16 x float> %temp.vect140.i, float %79, i32 5
  %83 = extractelement <4 x float> %phi952.i, i32 3
  %temp.vect158.i = insertelement <16 x float> %temp.vect157.i, float %80, i32 5
  %84 = extractelement <4 x float> %phi952.i, i32 2
  %temp.vect175.i = insertelement <16 x float> %temp.vect174.i, float %81, i32 5
  %85 = extractelement <4 x float> %phi952.i, i32 1
  %temp.vect192.i = insertelement <16 x float> %temp.vect191.i, float %82, i32 5
  %86 = extractelement <4 x float> %phi952.i, i32 0
  %temp.vect142.i = insertelement <16 x float> %temp.vect141.i, float %83, i32 6
  %87 = extractelement <4 x float> %phi955.i, i32 3
  %temp.vect159.i = insertelement <16 x float> %temp.vect158.i, float %84, i32 6
  %88 = extractelement <4 x float> %phi955.i, i32 2
  %temp.vect176.i = insertelement <16 x float> %temp.vect175.i, float %85, i32 6
  %89 = extractelement <4 x float> %phi955.i, i32 1
  %temp.vect193.i = insertelement <16 x float> %temp.vect192.i, float %86, i32 6
  %90 = extractelement <4 x float> %phi955.i, i32 0
  %temp.vect143.i = insertelement <16 x float> %temp.vect142.i, float %87, i32 7
  %91 = extractelement <4 x float> %phi958.i, i32 3
  %temp.vect160.i = insertelement <16 x float> %temp.vect159.i, float %88, i32 7
  %92 = extractelement <4 x float> %phi958.i, i32 2
  %temp.vect177.i = insertelement <16 x float> %temp.vect176.i, float %89, i32 7
  %93 = extractelement <4 x float> %phi958.i, i32 1
  %temp.vect194.i = insertelement <16 x float> %temp.vect193.i, float %90, i32 7
  %94 = extractelement <4 x float> %phi958.i, i32 0
  %temp.vect144.i = insertelement <16 x float> %temp.vect143.i, float %91, i32 8
  %95 = extractelement <4 x float> %phi961.i, i32 3
  %temp.vect161.i = insertelement <16 x float> %temp.vect160.i, float %92, i32 8
  %96 = extractelement <4 x float> %phi961.i, i32 2
  %temp.vect178.i = insertelement <16 x float> %temp.vect177.i, float %93, i32 8
  %97 = extractelement <4 x float> %phi961.i, i32 1
  %temp.vect195.i = insertelement <16 x float> %temp.vect194.i, float %94, i32 8
  %98 = extractelement <4 x float> %phi961.i, i32 0
  %temp.vect145.i = insertelement <16 x float> %temp.vect144.i, float %95, i32 9
  %99 = extractelement <4 x float> %phi832.i, i32 3
  %temp.vect162.i = insertelement <16 x float> %temp.vect161.i, float %96, i32 9
  %100 = extractelement <4 x float> %phi832.i, i32 2
  %temp.vect179.i = insertelement <16 x float> %temp.vect178.i, float %97, i32 9
  %101 = extractelement <4 x float> %phi832.i, i32 1
  %temp.vect196.i = insertelement <16 x float> %temp.vect195.i, float %98, i32 9
  %102 = extractelement <4 x float> %phi832.i, i32 0
  %temp.vect146.i = insertelement <16 x float> %temp.vect145.i, float %99, i32 10
  %103 = extractelement <4 x float> %phi835.i, i32 3
  %temp.vect163.i = insertelement <16 x float> %temp.vect162.i, float %100, i32 10
  %104 = extractelement <4 x float> %phi835.i, i32 2
  %temp.vect180.i = insertelement <16 x float> %temp.vect179.i, float %101, i32 10
  %105 = extractelement <4 x float> %phi835.i, i32 1
  %temp.vect197.i = insertelement <16 x float> %temp.vect196.i, float %102, i32 10
  %106 = extractelement <4 x float> %phi835.i, i32 0
  %temp.vect147.i = insertelement <16 x float> %temp.vect146.i, float %103, i32 11
  %107 = extractelement <4 x float> %phi838.i, i32 3
  %temp.vect164.i = insertelement <16 x float> %temp.vect163.i, float %104, i32 11
  %108 = extractelement <4 x float> %phi838.i, i32 2
  %temp.vect181.i = insertelement <16 x float> %temp.vect180.i, float %105, i32 11
  %109 = extractelement <4 x float> %phi838.i, i32 1
  %temp.vect198.i = insertelement <16 x float> %temp.vect197.i, float %106, i32 11
  %110 = extractelement <4 x float> %phi838.i, i32 0
  %temp.vect148.i = insertelement <16 x float> %temp.vect147.i, float %107, i32 12
  %111 = extractelement <4 x float> %phi841.i, i32 3
  %temp.vect165.i = insertelement <16 x float> %temp.vect164.i, float %108, i32 12
  %112 = extractelement <4 x float> %phi841.i, i32 2
  %temp.vect182.i = insertelement <16 x float> %temp.vect181.i, float %109, i32 12
  %113 = extractelement <4 x float> %phi841.i, i32 1
  %temp.vect199.i = insertelement <16 x float> %temp.vect198.i, float %110, i32 12
  %114 = extractelement <4 x float> %phi841.i, i32 0
  %temp.vect149.i = insertelement <16 x float> %temp.vect148.i, float %111, i32 13
  %115 = extractelement <4 x float> %phi844.i, i32 3
  %temp.vect166.i = insertelement <16 x float> %temp.vect165.i, float %112, i32 13
  %116 = extractelement <4 x float> %phi844.i, i32 2
  %temp.vect183.i = insertelement <16 x float> %temp.vect182.i, float %113, i32 13
  %117 = extractelement <4 x float> %phi844.i, i32 1
  %temp.vect200.i = insertelement <16 x float> %temp.vect199.i, float %114, i32 13
  %118 = extractelement <4 x float> %phi844.i, i32 0
  %temp.vect150.i = insertelement <16 x float> %temp.vect149.i, float %115, i32 14
  %119 = extractelement <4 x float> %phi847.i, i32 3
  %temp.vect167.i = insertelement <16 x float> %temp.vect166.i, float %116, i32 14
  %120 = extractelement <4 x float> %phi847.i, i32 2
  %temp.vect184.i = insertelement <16 x float> %temp.vect183.i, float %117, i32 14
  %121 = extractelement <4 x float> %phi847.i, i32 1
  %temp.vect201.i = insertelement <16 x float> %temp.vect200.i, float %118, i32 14
  %122 = extractelement <4 x float> %phi847.i, i32 0
  %temp.vect151.i = insertelement <16 x float> %temp.vect150.i, float %119, i32 15
  %temp.vect168.i = insertelement <16 x float> %temp.vect167.i, float %120, i32 15
  %temp.vect185.i = insertelement <16 x float> %temp.vect184.i, float %121, i32 15
  %temp.vect202.i = insertelement <16 x float> %temp.vect201.i, float %122, i32 15
  %extract227.i = extractelement <16 x i1> %loadedValue1389.i, i32 0
  %"&(pSB[currWI].offset)1802.i" = add nuw i64 %CurrSBIndex..1.i, 888
  %"&pSB[currWI].offset1803.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1802.i"
  %CastToValueType1804.i = bitcast i8* %"&pSB[currWI].offset1803.i" to i1*
  store i1 %extract227.i, i1* %CastToValueType1804.i, align 1
  %extract228.i = extractelement <16 x i1> %loadedValue1389.i, i32 1
  %"&(pSB[currWI].offset)1831.i" = add nuw i64 %CurrSBIndex..1.i, 889
  %"&pSB[currWI].offset1832.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1831.i"
  %CastToValueType1833.i = bitcast i8* %"&pSB[currWI].offset1832.i" to i1*
  store i1 %extract228.i, i1* %CastToValueType1833.i, align 1
  %extract229.i = extractelement <16 x i1> %loadedValue1389.i, i32 2
  %"&(pSB[currWI].offset)1855.i" = add nuw i64 %CurrSBIndex..1.i, 890
  %"&pSB[currWI].offset1856.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1855.i"
  %CastToValueType1857.i = bitcast i8* %"&pSB[currWI].offset1856.i" to i1*
  store i1 %extract229.i, i1* %CastToValueType1857.i, align 1
  %extract230.i = extractelement <16 x i1> %loadedValue1389.i, i32 3
  %"&(pSB[currWI].offset)1879.i" = add nuw i64 %CurrSBIndex..1.i, 891
  %"&pSB[currWI].offset1880.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1879.i"
  %CastToValueType1881.i = bitcast i8* %"&pSB[currWI].offset1880.i" to i1*
  store i1 %extract230.i, i1* %CastToValueType1881.i, align 1
  %extract231.i = extractelement <16 x i1> %loadedValue1389.i, i32 4
  %"&(pSB[currWI].offset)1903.i" = add nuw i64 %CurrSBIndex..1.i, 892
  %"&pSB[currWI].offset1904.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1903.i"
  %CastToValueType1905.i = bitcast i8* %"&pSB[currWI].offset1904.i" to i1*
  store i1 %extract231.i, i1* %CastToValueType1905.i, align 1
  %extract232.i = extractelement <16 x i1> %loadedValue1389.i, i32 5
  %"&(pSB[currWI].offset)1927.i" = add nuw i64 %CurrSBIndex..1.i, 893
  %"&pSB[currWI].offset1928.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1927.i"
  %CastToValueType1929.i = bitcast i8* %"&pSB[currWI].offset1928.i" to i1*
  store i1 %extract232.i, i1* %CastToValueType1929.i, align 1
  %extract233.i = extractelement <16 x i1> %loadedValue1389.i, i32 6
  %"&(pSB[currWI].offset)1951.i" = add nuw i64 %CurrSBIndex..1.i, 894
  %"&pSB[currWI].offset1952.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1951.i"
  %CastToValueType1953.i = bitcast i8* %"&pSB[currWI].offset1952.i" to i1*
  store i1 %extract233.i, i1* %CastToValueType1953.i, align 1
  %extract234.i = extractelement <16 x i1> %loadedValue1389.i, i32 7
  %"&(pSB[currWI].offset)1975.i" = add nuw i64 %CurrSBIndex..1.i, 895
  %"&pSB[currWI].offset1976.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1975.i"
  %CastToValueType1977.i = bitcast i8* %"&pSB[currWI].offset1976.i" to i1*
  store i1 %extract234.i, i1* %CastToValueType1977.i, align 1
  %extract235.i = extractelement <16 x i1> %loadedValue1389.i, i32 8
  %"&(pSB[currWI].offset)1999.i" = add nuw i64 %CurrSBIndex..1.i, 896
  %"&pSB[currWI].offset2000.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1999.i"
  %CastToValueType2001.i = bitcast i8* %"&pSB[currWI].offset2000.i" to i1*
  store i1 %extract235.i, i1* %CastToValueType2001.i, align 1
  %extract236.i = extractelement <16 x i1> %loadedValue1389.i, i32 9
  %"&(pSB[currWI].offset)2023.i" = add nuw i64 %CurrSBIndex..1.i, 897
  %"&pSB[currWI].offset2024.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2023.i"
  %CastToValueType2025.i = bitcast i8* %"&pSB[currWI].offset2024.i" to i1*
  store i1 %extract236.i, i1* %CastToValueType2025.i, align 1
  %extract237.i = extractelement <16 x i1> %loadedValue1389.i, i32 10
  %"&(pSB[currWI].offset)2047.i" = add nuw i64 %CurrSBIndex..1.i, 898
  %"&pSB[currWI].offset2048.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2047.i"
  %CastToValueType2049.i = bitcast i8* %"&pSB[currWI].offset2048.i" to i1*
  store i1 %extract237.i, i1* %CastToValueType2049.i, align 1
  %extract238.i = extractelement <16 x i1> %loadedValue1389.i, i32 11
  %"&(pSB[currWI].offset)2071.i" = add nuw i64 %CurrSBIndex..1.i, 899
  %"&pSB[currWI].offset2072.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2071.i"
  %CastToValueType2073.i = bitcast i8* %"&pSB[currWI].offset2072.i" to i1*
  store i1 %extract238.i, i1* %CastToValueType2073.i, align 1
  %extract239.i = extractelement <16 x i1> %loadedValue1389.i, i32 12
  %"&(pSB[currWI].offset)2095.i" = add nuw i64 %CurrSBIndex..1.i, 900
  %"&pSB[currWI].offset2096.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2095.i"
  %CastToValueType2097.i = bitcast i8* %"&pSB[currWI].offset2096.i" to i1*
  store i1 %extract239.i, i1* %CastToValueType2097.i, align 1
  %extract240.i = extractelement <16 x i1> %loadedValue1389.i, i32 13
  %"&(pSB[currWI].offset)2119.i" = add nuw i64 %CurrSBIndex..1.i, 901
  %"&pSB[currWI].offset2120.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2119.i"
  %CastToValueType2121.i = bitcast i8* %"&pSB[currWI].offset2120.i" to i1*
  store i1 %extract240.i, i1* %CastToValueType2121.i, align 1
  %extract241.i = extractelement <16 x i1> %loadedValue1389.i, i32 14
  %"&(pSB[currWI].offset)2143.i" = add nuw i64 %CurrSBIndex..1.i, 902
  %"&pSB[currWI].offset2144.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2143.i"
  %CastToValueType2145.i = bitcast i8* %"&pSB[currWI].offset2144.i" to i1*
  store i1 %extract241.i, i1* %CastToValueType2145.i, align 1
  %extract242.i = extractelement <16 x i1> %loadedValue1389.i, i32 15
  %"&(pSB[currWI].offset)2167.i" = add nuw i64 %CurrSBIndex..1.i, 903
  %"&pSB[currWI].offset2168.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2167.i"
  %CastToValueType2169.i = bitcast i8* %"&pSB[currWI].offset2168.i" to i1*
  store i1 %extract242.i, i1* %CastToValueType2169.i, align 1
  %merge53152.i = select <16 x i1> %while.body_to_if.then101.i, <16 x float> %temp.vect151.i, <16 x float> zeroinitializer
  %merge51169.i = select <16 x i1> %while.body_to_if.then101.i, <16 x float> %temp.vect168.i, <16 x float> zeroinitializer
  %merge49186.i = select <16 x i1> %while.body_to_if.then101.i, <16 x float> %temp.vect185.i, <16 x float> zeroinitializer
  %merge203.i = select <16 x i1> %while.body_to_if.then101.i, <16 x float> %temp.vect202.i, <16 x float> zeroinitializer
  %"&(pSB[currWI].offset)2191.i" = add nuw i64 %CurrSBIndex..1.i, 960
  %"&pSB[currWI].offset2192.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2191.i"
  %CastToValueType2193.i = bitcast i8* %"&pSB[currWI].offset2192.i" to <16 x float>*
  store <16 x float> %merge203.i, <16 x float>* %CastToValueType2193.i, align 64
  %add19204.i = fadd <16 x float> %merge49186.i, %merge203.i
  %"&(pSB[currWI].offset)2200.i" = add nuw i64 %CurrSBIndex..1.i, 1024
  %"&pSB[currWI].offset2201.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2200.i"
  %CastToValueType2202.i = bitcast i8* %"&pSB[currWI].offset2201.i" to <16 x float>*
  store <16 x float> %add19204.i, <16 x float>* %CastToValueType2202.i, align 64
  %add20205.i = fadd <16 x float> %merge51169.i, %add19204.i
  %"&(pSB[currWI].offset)2209.i" = add nuw i64 %CurrSBIndex..1.i, 1088
  %"&pSB[currWI].offset2210.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2209.i"
  %CastToValueType2211.i = bitcast i8* %"&pSB[currWI].offset2210.i" to <16 x float>*
  store <16 x float> %add20205.i, <16 x float>* %CastToValueType2211.i, align 64
  %add21206.i = fadd <16 x float> %merge53152.i, %add20205.i
  %"&(pSB[currWI].offset)2218.i" = add nuw i64 %CurrSBIndex..1.i, 1152
  %"&pSB[currWI].offset2219.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2218.i"
  %CastToValueType2220.i = bitcast i8* %"&pSB[currWI].offset2219.i" to <16 x float>*
  store <16 x float> %add21206.i, <16 x float>* %CastToValueType2220.i, align 64
  %extract278.i = extractelement <16 x float> %add21206.i, i32 0
  %extract279.i = extractelement <16 x float> %add21206.i, i32 1
  %extract280.i = extractelement <16 x float> %add21206.i, i32 2
  %extract281.i = extractelement <16 x float> %add21206.i, i32 3
  %extract282.i = extractelement <16 x float> %add21206.i, i32 4
  %extract283.i = extractelement <16 x float> %add21206.i, i32 5
  %extract284.i = extractelement <16 x float> %add21206.i, i32 6
  %extract285.i = extractelement <16 x float> %add21206.i, i32 7
  %extract286.i = extractelement <16 x float> %add21206.i, i32 8
  %extract287.i = extractelement <16 x float> %add21206.i, i32 9
  %extract288.i = extractelement <16 x float> %add21206.i, i32 10
  %extract289.i = extractelement <16 x float> %add21206.i, i32 11
  %extract290.i = extractelement <16 x float> %add21206.i, i32 12
  %extract291.i = extractelement <16 x float> %add21206.i, i32 13
  %extract292.i = extractelement <16 x float> %add21206.i, i32 14
  %extract293.i = extractelement <16 x float> %add21206.i, i32 15
  %"&pSB[currWI].offset1262.i" = getelementptr inbounds i8* %31, i64 %CurrSBIndex..1.i
  %CastToValueType1263.i = bitcast i8* %"&pSB[currWI].offset1262.i" to <16 x i64>*
  %loadedValue1264.i = load <16 x i64>* %CastToValueType1263.i, align 128
  %extract211.lhs.lhs.i = extractelement <16 x i64> %loadedValue1264.i, i32 0
  %extract211.lhs.i = shl i64 %extract211.lhs.lhs.i, 32
  %extract211.i = ashr exact i64 %extract211.lhs.i, 32
  br i1 %extract227.i, label %preload815.i, label %postload816.i

preload815.i:                                     ; preds = %if.end.i
  %123 = getelementptr inbounds float addrspace(3)* %13, i64 %extract211.i
  store float 0.000000e+00, float addrspace(3)* %123, align 4
  %loadedValue1838.pre.i = load i1* %CastToValueType1833.i, align 1
  br label %postload816.i

postload816.i:                                    ; preds = %preload815.i, %if.end.i
  %loadedValue1838.i = phi i1 [ %loadedValue1838.pre.i, %preload815.i ], [ %extract228.i, %if.end.i ]
  br i1 %loadedValue1838.i, label %preload785.i, label %postload816.i.postload786.i_crit_edge

postload816.i.postload786.i_crit_edge:            ; preds = %postload816.i
  br label %postload786.i

preload785.i:                                     ; preds = %postload816.i
  %.sum1254.i = add i64 %extract211.i, 1
  %124 = getelementptr float addrspace(3)* %13, i64 %.sum1254.i
  store float 0.000000e+00, float addrspace(3)* %124, align 4
  br label %postload786.i

postload786.i:                                    ; preds = %postload816.i.postload786.i_crit_edge, %preload785.i
  %loadedValue1862.i = load i1* %CastToValueType1857.i, align 1
  br i1 %loadedValue1862.i, label %preload767.i, label %postload786.i.postload768.i_crit_edge

postload786.i.postload768.i_crit_edge:            ; preds = %postload786.i
  br label %postload768.i

preload767.i:                                     ; preds = %postload786.i
  %.sum1253.i = add i64 %extract211.i, 2
  %125 = getelementptr float addrspace(3)* %13, i64 %.sum1253.i
  store float 0.000000e+00, float addrspace(3)* %125, align 4
  br label %postload768.i

postload768.i:                                    ; preds = %postload786.i.postload768.i_crit_edge, %preload767.i
  %loadedValue1886.i = load i1* %CastToValueType1881.i, align 1
  br i1 %loadedValue1886.i, label %preload776.i, label %postload768.i.postload777.i_crit_edge

postload768.i.postload777.i_crit_edge:            ; preds = %postload768.i
  br label %postload777.i

preload776.i:                                     ; preds = %postload768.i
  %.sum1252.i = add i64 %extract211.i, 3
  %126 = getelementptr float addrspace(3)* %13, i64 %.sum1252.i
  store float 0.000000e+00, float addrspace(3)* %126, align 4
  br label %postload777.i

postload777.i:                                    ; preds = %postload768.i.postload777.i_crit_edge, %preload776.i
  %loadedValue1910.i = load i1* %CastToValueType1905.i, align 1
  br i1 %loadedValue1910.i, label %preload812.i, label %postload777.i.postload813.i_crit_edge

postload777.i.postload813.i_crit_edge:            ; preds = %postload777.i
  br label %postload813.i

preload812.i:                                     ; preds = %postload777.i
  %.sum1251.i = add i64 %extract211.i, 4
  %127 = getelementptr float addrspace(3)* %13, i64 %.sum1251.i
  store float 0.000000e+00, float addrspace(3)* %127, align 4
  br label %postload813.i

postload813.i:                                    ; preds = %postload777.i.postload813.i_crit_edge, %preload812.i
  %loadedValue1934.i = load i1* %CastToValueType1929.i, align 1
  br i1 %loadedValue1934.i, label %preload788.i, label %postload813.i.postload789.i_crit_edge

postload813.i.postload789.i_crit_edge:            ; preds = %postload813.i
  br label %postload789.i

preload788.i:                                     ; preds = %postload813.i
  %.sum1250.i = add i64 %extract211.i, 5
  %128 = getelementptr float addrspace(3)* %13, i64 %.sum1250.i
  store float 0.000000e+00, float addrspace(3)* %128, align 4
  br label %postload789.i

postload789.i:                                    ; preds = %postload813.i.postload789.i_crit_edge, %preload788.i
  %loadedValue1958.i = load i1* %CastToValueType1953.i, align 1
  br i1 %loadedValue1958.i, label %preload944.i, label %postload789.i.postload945.i_crit_edge

postload789.i.postload945.i_crit_edge:            ; preds = %postload789.i
  br label %postload945.i

preload944.i:                                     ; preds = %postload789.i
  %.sum1249.i = add i64 %extract211.i, 6
  %129 = getelementptr float addrspace(3)* %13, i64 %.sum1249.i
  store float 0.000000e+00, float addrspace(3)* %129, align 4
  br label %postload945.i

postload945.i:                                    ; preds = %postload789.i.postload945.i_crit_edge, %preload944.i
  %loadedValue1982.i = load i1* %CastToValueType1977.i, align 1
  br i1 %loadedValue1982.i, label %preload821.i, label %postload945.i.postload822.i_crit_edge

postload945.i.postload822.i_crit_edge:            ; preds = %postload945.i
  br label %postload822.i

preload821.i:                                     ; preds = %postload945.i
  %.sum1248.i = add i64 %extract211.i, 7
  %130 = getelementptr float addrspace(3)* %13, i64 %.sum1248.i
  store float 0.000000e+00, float addrspace(3)* %130, align 4
  br label %postload822.i

postload822.i:                                    ; preds = %postload945.i.postload822.i_crit_edge, %preload821.i
  %loadedValue2006.i = load i1* %CastToValueType2001.i, align 1
  br i1 %loadedValue2006.i, label %preload770.i, label %postload822.i.postload771.i_crit_edge

postload822.i.postload771.i_crit_edge:            ; preds = %postload822.i
  br label %postload771.i

preload770.i:                                     ; preds = %postload822.i
  %.sum1247.i = add i64 %extract211.i, 8
  %131 = getelementptr float addrspace(3)* %13, i64 %.sum1247.i
  store float 0.000000e+00, float addrspace(3)* %131, align 4
  br label %postload771.i

postload771.i:                                    ; preds = %postload822.i.postload771.i_crit_edge, %preload770.i
  %loadedValue2030.i = load i1* %CastToValueType2025.i, align 1
  br i1 %loadedValue2030.i, label %preload773.i, label %postload771.i.postload774.i_crit_edge

postload771.i.postload774.i_crit_edge:            ; preds = %postload771.i
  br label %postload774.i

preload773.i:                                     ; preds = %postload771.i
  %.sum1246.i = add i64 %extract211.i, 9
  %132 = getelementptr float addrspace(3)* %13, i64 %.sum1246.i
  store float 0.000000e+00, float addrspace(3)* %132, align 4
  br label %postload774.i

postload774.i:                                    ; preds = %postload771.i.postload774.i_crit_edge, %preload773.i
  %loadedValue2054.i = load i1* %CastToValueType2049.i, align 1
  br i1 %loadedValue2054.i, label %preload818.i, label %postload774.i.postload819.i_crit_edge

postload774.i.postload819.i_crit_edge:            ; preds = %postload774.i
  br label %postload819.i

preload818.i:                                     ; preds = %postload774.i
  %.sum1245.i = add i64 %extract211.i, 10
  %133 = getelementptr float addrspace(3)* %13, i64 %.sum1245.i
  store float 0.000000e+00, float addrspace(3)* %133, align 4
  br label %postload819.i

postload819.i:                                    ; preds = %postload774.i.postload819.i_crit_edge, %preload818.i
  %loadedValue2078.i = load i1* %CastToValueType2073.i, align 1
  br i1 %loadedValue2078.i, label %preload.i, label %postload819.i.postload.i_crit_edge

postload819.i.postload.i_crit_edge:               ; preds = %postload819.i
  br label %postload.i

preload.i:                                        ; preds = %postload819.i
  %.sum1244.i = add i64 %extract211.i, 11
  %134 = getelementptr float addrspace(3)* %13, i64 %.sum1244.i
  store float 0.000000e+00, float addrspace(3)* %134, align 4
  br label %postload.i

postload.i:                                       ; preds = %postload819.i.postload.i_crit_edge, %preload.i
  %loadedValue2102.i = load i1* %CastToValueType2097.i, align 1
  br i1 %loadedValue2102.i, label %preload782.i, label %postload.i.postload783.i_crit_edge

postload.i.postload783.i_crit_edge:               ; preds = %postload.i
  br label %postload783.i

preload782.i:                                     ; preds = %postload.i
  %.sum1243.i = add i64 %extract211.i, 12
  %135 = getelementptr float addrspace(3)* %13, i64 %.sum1243.i
  store float 0.000000e+00, float addrspace(3)* %135, align 4
  br label %postload783.i

postload783.i:                                    ; preds = %postload.i.postload783.i_crit_edge, %preload782.i
  %loadedValue2126.i = load i1* %CastToValueType2121.i, align 1
  br i1 %loadedValue2126.i, label %preload779.i, label %postload783.i.postload780.i_crit_edge

postload783.i.postload780.i_crit_edge:            ; preds = %postload783.i
  br label %postload780.i

preload779.i:                                     ; preds = %postload783.i
  %.sum1242.i = add i64 %extract211.i, 13
  %136 = getelementptr float addrspace(3)* %13, i64 %.sum1242.i
  store float 0.000000e+00, float addrspace(3)* %136, align 4
  br label %postload780.i

postload780.i:                                    ; preds = %postload783.i.postload780.i_crit_edge, %preload779.i
  %loadedValue2150.i = load i1* %CastToValueType2145.i, align 1
  br i1 %loadedValue2150.i, label %preload809.i, label %postload780.i.postload810.i_crit_edge

postload780.i.postload810.i_crit_edge:            ; preds = %postload780.i
  br label %postload810.i

preload809.i:                                     ; preds = %postload780.i
  %.sum1241.i = add i64 %extract211.i, 14
  %137 = getelementptr float addrspace(3)* %13, i64 %.sum1241.i
  store float 0.000000e+00, float addrspace(3)* %137, align 4
  br label %postload810.i

postload810.i:                                    ; preds = %postload780.i.postload810.i_crit_edge, %preload809.i
  %loadedValue2174.i = load i1* %CastToValueType2169.i, align 1
  br i1 %loadedValue2174.i, label %preload806.i, label %postload810.i.postload807.i_crit_edge

postload810.i.postload807.i_crit_edge:            ; preds = %postload810.i
  br label %postload807.i

preload806.i:                                     ; preds = %postload810.i
  %.sum.i = add i64 %extract211.i, 15
  %138 = getelementptr float addrspace(3)* %13, i64 %.sum.i
  store float 0.000000e+00, float addrspace(3)* %138, align 4
  br label %postload807.i

postload807.i:                                    ; preds = %postload810.i.postload807.i_crit_edge, %preload806.i
  %loadedValue1829.i = load i1* %CastToValueType1804.i, align 1
  br i1 %loadedValue1829.i, label %preload962.i, label %postload807.i.postload963.i_crit_edge

postload807.i.postload963.i_crit_edge:            ; preds = %postload807.i
  br label %postload963.i

preload962.i:                                     ; preds = %postload807.i
  %139 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %140 = load i64* %139, align 8
  br label %postload963.i

postload963.i:                                    ; preds = %postload807.i.postload963.i_crit_edge, %preload962.i
  %phi964.i = phi i64 [ %140, %preload962.i ], [ undef, %postload807.i.postload963.i_crit_edge ]
  %loadedValue1853.i = load i1* %CastToValueType1833.i, align 1
  br i1 %loadedValue1853.i, label %preload975.i, label %postload963.i.postload976.i_crit_edge

postload963.i.postload976.i_crit_edge:            ; preds = %postload963.i
  br label %postload976.i

preload975.i:                                     ; preds = %postload963.i
  %141 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %142 = load i64* %141, align 8
  br label %postload976.i

postload976.i:                                    ; preds = %postload963.i.postload976.i_crit_edge, %preload975.i
  %phi977.i = phi i64 [ %142, %preload975.i ], [ undef, %postload963.i.postload976.i_crit_edge ]
  %loadedValue1877.i = load i1* %CastToValueType1857.i, align 1
  br i1 %loadedValue1877.i, label %preload986.i, label %postload976.i.postload987.i_crit_edge

postload976.i.postload987.i_crit_edge:            ; preds = %postload976.i
  br label %postload987.i

preload986.i:                                     ; preds = %postload976.i
  %143 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %144 = load i64* %143, align 8
  br label %postload987.i

postload987.i:                                    ; preds = %postload976.i.postload987.i_crit_edge, %preload986.i
  %phi988.i = phi i64 [ %144, %preload986.i ], [ undef, %postload976.i.postload987.i_crit_edge ]
  %loadedValue1901.i = load i1* %CastToValueType1881.i, align 1
  br i1 %loadedValue1901.i, label %preload997.i, label %postload987.i.postload998.i_crit_edge

postload987.i.postload998.i_crit_edge:            ; preds = %postload987.i
  br label %postload998.i

preload997.i:                                     ; preds = %postload987.i
  %145 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %146 = load i64* %145, align 8
  br label %postload998.i

postload998.i:                                    ; preds = %postload987.i.postload998.i_crit_edge, %preload997.i
  %phi999.i = phi i64 [ %146, %preload997.i ], [ undef, %postload987.i.postload998.i_crit_edge ]
  %loadedValue1925.i = load i1* %CastToValueType1905.i, align 1
  br i1 %loadedValue1925.i, label %preload1008.i, label %postload998.i.postload1009.i_crit_edge

postload998.i.postload1009.i_crit_edge:           ; preds = %postload998.i
  br label %postload1009.i

preload1008.i:                                    ; preds = %postload998.i
  %147 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %148 = load i64* %147, align 8
  br label %postload1009.i

postload1009.i:                                   ; preds = %postload998.i.postload1009.i_crit_edge, %preload1008.i
  %phi1010.i = phi i64 [ %148, %preload1008.i ], [ undef, %postload998.i.postload1009.i_crit_edge ]
  %loadedValue1949.i = load i1* %CastToValueType1929.i, align 1
  br i1 %loadedValue1949.i, label %preload1019.i, label %postload1009.i.postload1020.i_crit_edge

postload1009.i.postload1020.i_crit_edge:          ; preds = %postload1009.i
  br label %postload1020.i

preload1019.i:                                    ; preds = %postload1009.i
  %149 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %150 = load i64* %149, align 8
  br label %postload1020.i

postload1020.i:                                   ; preds = %postload1009.i.postload1020.i_crit_edge, %preload1019.i
  %phi1021.i = phi i64 [ %150, %preload1019.i ], [ undef, %postload1009.i.postload1020.i_crit_edge ]
  %loadedValue1973.i = load i1* %CastToValueType1953.i, align 1
  br i1 %loadedValue1973.i, label %preload1030.i, label %postload1020.i.postload1031.i_crit_edge

postload1020.i.postload1031.i_crit_edge:          ; preds = %postload1020.i
  br label %postload1031.i

preload1030.i:                                    ; preds = %postload1020.i
  %151 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %152 = load i64* %151, align 8
  br label %postload1031.i

postload1031.i:                                   ; preds = %postload1020.i.postload1031.i_crit_edge, %preload1030.i
  %phi1032.i = phi i64 [ %152, %preload1030.i ], [ undef, %postload1020.i.postload1031.i_crit_edge ]
  %loadedValue1997.i = load i1* %CastToValueType1977.i, align 1
  br i1 %loadedValue1997.i, label %preload1041.i, label %postload1031.i.postload1042.i_crit_edge

postload1031.i.postload1042.i_crit_edge:          ; preds = %postload1031.i
  br label %postload1042.i

preload1041.i:                                    ; preds = %postload1031.i
  %153 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %154 = load i64* %153, align 8
  br label %postload1042.i

postload1042.i:                                   ; preds = %postload1031.i.postload1042.i_crit_edge, %preload1041.i
  %phi1043.i = phi i64 [ %154, %preload1041.i ], [ undef, %postload1031.i.postload1042.i_crit_edge ]
  %loadedValue2021.i = load i1* %CastToValueType2001.i, align 1
  br i1 %loadedValue2021.i, label %preload1052.i, label %postload1042.i.postload1053.i_crit_edge

postload1042.i.postload1053.i_crit_edge:          ; preds = %postload1042.i
  br label %postload1053.i

preload1052.i:                                    ; preds = %postload1042.i
  %155 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %156 = load i64* %155, align 8
  br label %postload1053.i

postload1053.i:                                   ; preds = %postload1042.i.postload1053.i_crit_edge, %preload1052.i
  %phi1054.i = phi i64 [ %156, %preload1052.i ], [ undef, %postload1042.i.postload1053.i_crit_edge ]
  %loadedValue2045.i = load i1* %CastToValueType2025.i, align 1
  br i1 %loadedValue2045.i, label %preload1063.i, label %postload1053.i.postload1064.i_crit_edge

postload1053.i.postload1064.i_crit_edge:          ; preds = %postload1053.i
  br label %postload1064.i

preload1063.i:                                    ; preds = %postload1053.i
  %157 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %158 = load i64* %157, align 8
  br label %postload1064.i

postload1064.i:                                   ; preds = %postload1053.i.postload1064.i_crit_edge, %preload1063.i
  %phi1065.i = phi i64 [ %158, %preload1063.i ], [ undef, %postload1053.i.postload1064.i_crit_edge ]
  %loadedValue2069.i = load i1* %CastToValueType2049.i, align 1
  br i1 %loadedValue2069.i, label %preload1074.i, label %postload1064.i.postload1075.i_crit_edge

postload1064.i.postload1075.i_crit_edge:          ; preds = %postload1064.i
  br label %postload1075.i

preload1074.i:                                    ; preds = %postload1064.i
  %159 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %160 = load i64* %159, align 8
  br label %postload1075.i

postload1075.i:                                   ; preds = %postload1064.i.postload1075.i_crit_edge, %preload1074.i
  %phi1076.i = phi i64 [ %160, %preload1074.i ], [ undef, %postload1064.i.postload1075.i_crit_edge ]
  %loadedValue2093.i = load i1* %CastToValueType2073.i, align 1
  br i1 %loadedValue2093.i, label %preload1085.i, label %postload1075.i.postload1086.i_crit_edge

postload1075.i.postload1086.i_crit_edge:          ; preds = %postload1075.i
  br label %postload1086.i

preload1085.i:                                    ; preds = %postload1075.i
  %161 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %162 = load i64* %161, align 8
  br label %postload1086.i

postload1086.i:                                   ; preds = %postload1075.i.postload1086.i_crit_edge, %preload1085.i
  %phi1087.i = phi i64 [ %162, %preload1085.i ], [ undef, %postload1075.i.postload1086.i_crit_edge ]
  %loadedValue2117.i = load i1* %CastToValueType2097.i, align 1
  br i1 %loadedValue2117.i, label %preload1096.i, label %postload1086.i.postload1097.i_crit_edge

postload1086.i.postload1097.i_crit_edge:          ; preds = %postload1086.i
  br label %postload1097.i

preload1096.i:                                    ; preds = %postload1086.i
  %163 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %164 = load i64* %163, align 8
  br label %postload1097.i

postload1097.i:                                   ; preds = %postload1086.i.postload1097.i_crit_edge, %preload1096.i
  %phi1098.i = phi i64 [ %164, %preload1096.i ], [ undef, %postload1086.i.postload1097.i_crit_edge ]
  %loadedValue2141.i = load i1* %CastToValueType2121.i, align 1
  br i1 %loadedValue2141.i, label %preload1107.i, label %postload1097.i.postload1108.i_crit_edge

postload1097.i.postload1108.i_crit_edge:          ; preds = %postload1097.i
  br label %postload1108.i

preload1107.i:                                    ; preds = %postload1097.i
  %165 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %166 = load i64* %165, align 8
  br label %postload1108.i

postload1108.i:                                   ; preds = %postload1097.i.postload1108.i_crit_edge, %preload1107.i
  %phi1109.i = phi i64 [ %166, %preload1107.i ], [ undef, %postload1097.i.postload1108.i_crit_edge ]
  %loadedValue2165.i = load i1* %CastToValueType2145.i, align 1
  br i1 %loadedValue2165.i, label %preload1118.i, label %postload1108.i.postload1119.i_crit_edge

postload1108.i.postload1119.i_crit_edge:          ; preds = %postload1108.i
  br label %postload1119.i

preload1118.i:                                    ; preds = %postload1108.i
  %167 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %168 = load i64* %167, align 8
  br label %postload1119.i

postload1119.i:                                   ; preds = %postload1108.i.postload1119.i_crit_edge, %preload1118.i
  %phi1120.i = phi i64 [ %168, %preload1118.i ], [ undef, %postload1108.i.postload1119.i_crit_edge ]
  %loadedValue2189.i = load i1* %CastToValueType2169.i, align 1
  br i1 %loadedValue2189.i, label %preload1129.i, label %postload1119.i.postload1130.i_crit_edge

postload1119.i.postload1130.i_crit_edge:          ; preds = %postload1119.i
  br label %postload1130.i

preload1129.i:                                    ; preds = %postload1119.i
  %169 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %170 = load i64* %169, align 8
  br label %postload1130.i

postload1130.i:                                   ; preds = %postload1119.i.postload1130.i_crit_edge, %preload1129.i
  %phi1131.i = phi i64 [ %170, %preload1129.i ], [ undef, %postload1119.i.postload1130.i_crit_edge ]
  %temp.vect243.i = insertelement <16 x i64> undef, i64 %phi964.i, i32 0
  %temp.vect244.i = insertelement <16 x i64> %temp.vect243.i, i64 %phi977.i, i32 1
  %temp.vect245.i = insertelement <16 x i64> %temp.vect244.i, i64 %phi988.i, i32 2
  %temp.vect246.i = insertelement <16 x i64> %temp.vect245.i, i64 %phi999.i, i32 3
  %temp.vect247.i = insertelement <16 x i64> %temp.vect246.i, i64 %phi1010.i, i32 4
  %temp.vect248.i = insertelement <16 x i64> %temp.vect247.i, i64 %phi1021.i, i32 5
  %temp.vect249.i = insertelement <16 x i64> %temp.vect248.i, i64 %phi1032.i, i32 6
  %temp.vect250.i = insertelement <16 x i64> %temp.vect249.i, i64 %phi1043.i, i32 7
  %temp.vect251.i = insertelement <16 x i64> %temp.vect250.i, i64 %phi1054.i, i32 8
  %temp.vect252.i = insertelement <16 x i64> %temp.vect251.i, i64 %phi1065.i, i32 9
  %temp.vect253.i = insertelement <16 x i64> %temp.vect252.i, i64 %phi1076.i, i32 10
  %temp.vect254.i = insertelement <16 x i64> %temp.vect253.i, i64 %phi1087.i, i32 11
  %temp.vect255.i = insertelement <16 x i64> %temp.vect254.i, i64 %phi1098.i, i32 12
  %temp.vect256.i = insertelement <16 x i64> %temp.vect255.i, i64 %phi1109.i, i32 13
  %temp.vect257.i = insertelement <16 x i64> %temp.vect256.i, i64 %phi1120.i, i32 14
  %temp.vect258.i = insertelement <16 x i64> %temp.vect257.i, i64 %phi1131.i, i32 15
  %"&(pSB[currWI].offset)2227.i" = add nuw i64 %CurrSBIndex..1.i, 1280
  %"&pSB[currWI].offset2228.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2227.i"
  %CastToValueType2229.i = bitcast i8* %"&pSB[currWI].offset2228.i" to <16 x i64>*
  store <16 x i64> %temp.vect258.i, <16 x i64>* %CastToValueType2229.i, align 128
  %loadedValue.i = load <16 x i64>* %CastToValueType1263.i, align 128
  %add.i259.i = add <16 x i64> %temp.vect258.i, %loadedValue.i
  %conv3.i260.i = trunc <16 x i64> %add.i259.i to <16 x i32>
  %"&(pSB[currWI].offset)2241.i" = add nuw i64 %CurrSBIndex..1.i, 1408
  %"&pSB[currWI].offset2242.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2241.i"
  %CastToValueType2243.i = bitcast i8* %"&pSB[currWI].offset2242.i" to <16 x i32>*
  store <16 x i32> %conv3.i260.i, <16 x i32>* %CastToValueType2243.i, align 64
  %idxprom4.i261.i = sext <16 x i32> %conv3.i260.i to <16 x i64>
  %extract262.i = extractelement <16 x i64> %idxprom4.i261.i, i32 0
  %extract263.i = extractelement <16 x i64> %idxprom4.i261.i, i32 1
  %extract264.i = extractelement <16 x i64> %idxprom4.i261.i, i32 2
  %extract265.i = extractelement <16 x i64> %idxprom4.i261.i, i32 3
  %extract266.i = extractelement <16 x i64> %idxprom4.i261.i, i32 4
  %extract267.i = extractelement <16 x i64> %idxprom4.i261.i, i32 5
  %extract268.i = extractelement <16 x i64> %idxprom4.i261.i, i32 6
  %extract269.i = extractelement <16 x i64> %idxprom4.i261.i, i32 7
  %extract270.i = extractelement <16 x i64> %idxprom4.i261.i, i32 8
  %extract271.i = extractelement <16 x i64> %idxprom4.i261.i, i32 9
  %extract272.i = extractelement <16 x i64> %idxprom4.i261.i, i32 10
  %extract273.i = extractelement <16 x i64> %idxprom4.i261.i, i32 11
  %extract274.i = extractelement <16 x i64> %idxprom4.i261.i, i32 12
  %extract275.i = extractelement <16 x i64> %idxprom4.i261.i, i32 13
  %extract276.i = extractelement <16 x i64> %idxprom4.i261.i, i32 14
  %extract277.i = extractelement <16 x i64> %idxprom4.i261.i, i32 15
  %171 = getelementptr inbounds float addrspace(3)* %13, i64 %extract262.i
  %"&(pSB[currWI].offset)2255.i" = add nuw i64 %CurrSBIndex..1.i, 1472
  %"&pSB[currWI].offset2256.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2255.i"
  %CastToValueType2257.i = bitcast i8* %"&pSB[currWI].offset2256.i" to float addrspace(3)**
  store float addrspace(3)* %171, float addrspace(3)** %CastToValueType2257.i, align 8
  %172 = getelementptr inbounds float addrspace(3)* %13, i64 %extract263.i
  %"&(pSB[currWI].offset)2274.i" = add nuw i64 %CurrSBIndex..1.i, 1480
  %"&pSB[currWI].offset2275.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2274.i"
  %CastToValueType2276.i = bitcast i8* %"&pSB[currWI].offset2275.i" to float addrspace(3)**
  store float addrspace(3)* %172, float addrspace(3)** %CastToValueType2276.i, align 8
  %173 = getelementptr inbounds float addrspace(3)* %13, i64 %extract264.i
  %"&(pSB[currWI].offset)2293.i" = add nuw i64 %CurrSBIndex..1.i, 1488
  %"&pSB[currWI].offset2294.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2293.i"
  %CastToValueType2295.i = bitcast i8* %"&pSB[currWI].offset2294.i" to float addrspace(3)**
  store float addrspace(3)* %173, float addrspace(3)** %CastToValueType2295.i, align 8
  %174 = getelementptr inbounds float addrspace(3)* %13, i64 %extract265.i
  %"&(pSB[currWI].offset)2312.i" = add nuw i64 %CurrSBIndex..1.i, 1496
  %"&pSB[currWI].offset2313.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2312.i"
  %CastToValueType2314.i = bitcast i8* %"&pSB[currWI].offset2313.i" to float addrspace(3)**
  store float addrspace(3)* %174, float addrspace(3)** %CastToValueType2314.i, align 8
  %175 = getelementptr inbounds float addrspace(3)* %13, i64 %extract266.i
  %"&(pSB[currWI].offset)2331.i" = add nuw i64 %CurrSBIndex..1.i, 1504
  %"&pSB[currWI].offset2332.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2331.i"
  %CastToValueType2333.i = bitcast i8* %"&pSB[currWI].offset2332.i" to float addrspace(3)**
  store float addrspace(3)* %175, float addrspace(3)** %CastToValueType2333.i, align 8
  %176 = getelementptr inbounds float addrspace(3)* %13, i64 %extract267.i
  %"&(pSB[currWI].offset)2350.i" = add nuw i64 %CurrSBIndex..1.i, 1512
  %"&pSB[currWI].offset2351.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2350.i"
  %CastToValueType2352.i = bitcast i8* %"&pSB[currWI].offset2351.i" to float addrspace(3)**
  store float addrspace(3)* %176, float addrspace(3)** %CastToValueType2352.i, align 8
  %177 = getelementptr inbounds float addrspace(3)* %13, i64 %extract268.i
  %"&(pSB[currWI].offset)2369.i" = add nuw i64 %CurrSBIndex..1.i, 1520
  %"&pSB[currWI].offset2370.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2369.i"
  %CastToValueType2371.i = bitcast i8* %"&pSB[currWI].offset2370.i" to float addrspace(3)**
  store float addrspace(3)* %177, float addrspace(3)** %CastToValueType2371.i, align 8
  %178 = getelementptr inbounds float addrspace(3)* %13, i64 %extract269.i
  %"&(pSB[currWI].offset)2388.i" = add nuw i64 %CurrSBIndex..1.i, 1528
  %"&pSB[currWI].offset2389.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2388.i"
  %CastToValueType2390.i = bitcast i8* %"&pSB[currWI].offset2389.i" to float addrspace(3)**
  store float addrspace(3)* %178, float addrspace(3)** %CastToValueType2390.i, align 8
  %179 = getelementptr inbounds float addrspace(3)* %13, i64 %extract270.i
  %"&(pSB[currWI].offset)2407.i" = add nuw i64 %CurrSBIndex..1.i, 1536
  %"&pSB[currWI].offset2408.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2407.i"
  %CastToValueType2409.i = bitcast i8* %"&pSB[currWI].offset2408.i" to float addrspace(3)**
  store float addrspace(3)* %179, float addrspace(3)** %CastToValueType2409.i, align 8
  %180 = getelementptr inbounds float addrspace(3)* %13, i64 %extract271.i
  %"&(pSB[currWI].offset)2426.i" = add nuw i64 %CurrSBIndex..1.i, 1544
  %"&pSB[currWI].offset2427.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2426.i"
  %CastToValueType2428.i = bitcast i8* %"&pSB[currWI].offset2427.i" to float addrspace(3)**
  store float addrspace(3)* %180, float addrspace(3)** %CastToValueType2428.i, align 8
  %181 = getelementptr inbounds float addrspace(3)* %13, i64 %extract272.i
  %"&(pSB[currWI].offset)2445.i" = add nuw i64 %CurrSBIndex..1.i, 1552
  %"&pSB[currWI].offset2446.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2445.i"
  %CastToValueType2447.i = bitcast i8* %"&pSB[currWI].offset2446.i" to float addrspace(3)**
  store float addrspace(3)* %181, float addrspace(3)** %CastToValueType2447.i, align 8
  %182 = getelementptr inbounds float addrspace(3)* %13, i64 %extract273.i
  %"&(pSB[currWI].offset)2464.i" = add nuw i64 %CurrSBIndex..1.i, 1560
  %"&pSB[currWI].offset2465.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2464.i"
  %CastToValueType2466.i = bitcast i8* %"&pSB[currWI].offset2465.i" to float addrspace(3)**
  store float addrspace(3)* %182, float addrspace(3)** %CastToValueType2466.i, align 8
  %183 = getelementptr inbounds float addrspace(3)* %13, i64 %extract274.i
  %"&(pSB[currWI].offset)2483.i" = add nuw i64 %CurrSBIndex..1.i, 1568
  %"&pSB[currWI].offset2484.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2483.i"
  %CastToValueType2485.i = bitcast i8* %"&pSB[currWI].offset2484.i" to float addrspace(3)**
  store float addrspace(3)* %183, float addrspace(3)** %CastToValueType2485.i, align 8
  %184 = getelementptr inbounds float addrspace(3)* %13, i64 %extract275.i
  %"&(pSB[currWI].offset)2502.i" = add nuw i64 %CurrSBIndex..1.i, 1576
  %"&pSB[currWI].offset2503.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2502.i"
  %CastToValueType2504.i = bitcast i8* %"&pSB[currWI].offset2503.i" to float addrspace(3)**
  store float addrspace(3)* %184, float addrspace(3)** %CastToValueType2504.i, align 8
  %185 = getelementptr inbounds float addrspace(3)* %13, i64 %extract276.i
  %"&(pSB[currWI].offset)2521.i" = add nuw i64 %CurrSBIndex..1.i, 1584
  %"&pSB[currWI].offset2522.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2521.i"
  %CastToValueType2523.i = bitcast i8* %"&pSB[currWI].offset2522.i" to float addrspace(3)**
  store float addrspace(3)* %185, float addrspace(3)** %CastToValueType2523.i, align 8
  %186 = getelementptr inbounds float addrspace(3)* %13, i64 %extract277.i
  %"&(pSB[currWI].offset)2540.i" = add nuw i64 %CurrSBIndex..1.i, 1592
  %"&pSB[currWI].offset2541.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2540.i"
  %CastToValueType2542.i = bitcast i8* %"&pSB[currWI].offset2541.i" to float addrspace(3)**
  store float addrspace(3)* %186, float addrspace(3)** %CastToValueType2542.i, align 8
  br i1 %loadedValue1829.i, label %preload965.i, label %postload966.i

preload965.i:                                     ; preds = %postload1130.i
  store float %extract278.i, float addrspace(3)* %171, align 4
  %loadedValue1848.pre.i = load i1* %CastToValueType1833.i, align 1
  br label %postload966.i

postload966.i:                                    ; preds = %preload965.i, %postload1130.i
  %loadedValue1848.i = phi i1 [ %loadedValue1848.pre.i, %preload965.i ], [ %loadedValue1853.i, %postload1130.i ]
  br i1 %loadedValue1848.i, label %preload978.i, label %postload966.i.postload979.i_crit_edge

postload966.i.postload979.i_crit_edge:            ; preds = %postload966.i
  br label %postload979.i

preload978.i:                                     ; preds = %postload966.i
  %loadedValue2291.i = load float addrspace(3)** %CastToValueType2276.i, align 8
  store float %extract279.i, float addrspace(3)* %loadedValue2291.i, align 4
  br label %postload979.i

postload979.i:                                    ; preds = %postload966.i.postload979.i_crit_edge, %preload978.i
  %loadedValue1872.i = load i1* %CastToValueType1857.i, align 1
  br i1 %loadedValue1872.i, label %preload989.i, label %postload979.i.postload990.i_crit_edge

postload979.i.postload990.i_crit_edge:            ; preds = %postload979.i
  br label %postload990.i

preload989.i:                                     ; preds = %postload979.i
  %loadedValue2310.i = load float addrspace(3)** %CastToValueType2295.i, align 8
  store float %extract280.i, float addrspace(3)* %loadedValue2310.i, align 4
  br label %postload990.i

postload990.i:                                    ; preds = %postload979.i.postload990.i_crit_edge, %preload989.i
  %loadedValue1896.i = load i1* %CastToValueType1881.i, align 1
  br i1 %loadedValue1896.i, label %preload1000.i, label %postload990.i.postload1001.i_crit_edge

postload990.i.postload1001.i_crit_edge:           ; preds = %postload990.i
  br label %postload1001.i

preload1000.i:                                    ; preds = %postload990.i
  %loadedValue2329.i = load float addrspace(3)** %CastToValueType2314.i, align 8
  store float %extract281.i, float addrspace(3)* %loadedValue2329.i, align 4
  br label %postload1001.i

postload1001.i:                                   ; preds = %postload990.i.postload1001.i_crit_edge, %preload1000.i
  %loadedValue1920.i = load i1* %CastToValueType1905.i, align 1
  br i1 %loadedValue1920.i, label %preload1011.i, label %postload1001.i.postload1012.i_crit_edge

postload1001.i.postload1012.i_crit_edge:          ; preds = %postload1001.i
  br label %postload1012.i

preload1011.i:                                    ; preds = %postload1001.i
  %loadedValue2348.i = load float addrspace(3)** %CastToValueType2333.i, align 8
  store float %extract282.i, float addrspace(3)* %loadedValue2348.i, align 4
  br label %postload1012.i

postload1012.i:                                   ; preds = %postload1001.i.postload1012.i_crit_edge, %preload1011.i
  %loadedValue1944.i = load i1* %CastToValueType1929.i, align 1
  br i1 %loadedValue1944.i, label %preload1022.i, label %postload1012.i.postload1023.i_crit_edge

postload1012.i.postload1023.i_crit_edge:          ; preds = %postload1012.i
  br label %postload1023.i

preload1022.i:                                    ; preds = %postload1012.i
  %loadedValue2367.i = load float addrspace(3)** %CastToValueType2352.i, align 8
  store float %extract283.i, float addrspace(3)* %loadedValue2367.i, align 4
  br label %postload1023.i

postload1023.i:                                   ; preds = %postload1012.i.postload1023.i_crit_edge, %preload1022.i
  %loadedValue1968.i = load i1* %CastToValueType1953.i, align 1
  br i1 %loadedValue1968.i, label %preload1033.i, label %postload1023.i.postload1034.i_crit_edge

postload1023.i.postload1034.i_crit_edge:          ; preds = %postload1023.i
  br label %postload1034.i

preload1033.i:                                    ; preds = %postload1023.i
  %loadedValue2386.i = load float addrspace(3)** %CastToValueType2371.i, align 8
  store float %extract284.i, float addrspace(3)* %loadedValue2386.i, align 4
  br label %postload1034.i

postload1034.i:                                   ; preds = %postload1023.i.postload1034.i_crit_edge, %preload1033.i
  %loadedValue1992.i = load i1* %CastToValueType1977.i, align 1
  br i1 %loadedValue1992.i, label %preload1044.i, label %postload1034.i.postload1045.i_crit_edge

postload1034.i.postload1045.i_crit_edge:          ; preds = %postload1034.i
  br label %postload1045.i

preload1044.i:                                    ; preds = %postload1034.i
  %loadedValue2405.i = load float addrspace(3)** %CastToValueType2390.i, align 8
  store float %extract285.i, float addrspace(3)* %loadedValue2405.i, align 4
  br label %postload1045.i

postload1045.i:                                   ; preds = %postload1034.i.postload1045.i_crit_edge, %preload1044.i
  %loadedValue2016.i = load i1* %CastToValueType2001.i, align 1
  br i1 %loadedValue2016.i, label %preload1055.i, label %postload1045.i.postload1056.i_crit_edge

postload1045.i.postload1056.i_crit_edge:          ; preds = %postload1045.i
  br label %postload1056.i

preload1055.i:                                    ; preds = %postload1045.i
  %loadedValue2424.i = load float addrspace(3)** %CastToValueType2409.i, align 8
  store float %extract286.i, float addrspace(3)* %loadedValue2424.i, align 4
  br label %postload1056.i

postload1056.i:                                   ; preds = %postload1045.i.postload1056.i_crit_edge, %preload1055.i
  %loadedValue2040.i = load i1* %CastToValueType2025.i, align 1
  br i1 %loadedValue2040.i, label %preload1066.i, label %postload1056.i.postload1067.i_crit_edge

postload1056.i.postload1067.i_crit_edge:          ; preds = %postload1056.i
  br label %postload1067.i

preload1066.i:                                    ; preds = %postload1056.i
  %loadedValue2443.i = load float addrspace(3)** %CastToValueType2428.i, align 8
  store float %extract287.i, float addrspace(3)* %loadedValue2443.i, align 4
  br label %postload1067.i

postload1067.i:                                   ; preds = %postload1056.i.postload1067.i_crit_edge, %preload1066.i
  %loadedValue2064.i = load i1* %CastToValueType2049.i, align 1
  br i1 %loadedValue2064.i, label %preload1077.i, label %postload1067.i.postload1078.i_crit_edge

postload1067.i.postload1078.i_crit_edge:          ; preds = %postload1067.i
  br label %postload1078.i

preload1077.i:                                    ; preds = %postload1067.i
  %loadedValue2462.i = load float addrspace(3)** %CastToValueType2447.i, align 8
  store float %extract288.i, float addrspace(3)* %loadedValue2462.i, align 4
  br label %postload1078.i

postload1078.i:                                   ; preds = %postload1067.i.postload1078.i_crit_edge, %preload1077.i
  %loadedValue2088.i = load i1* %CastToValueType2073.i, align 1
  br i1 %loadedValue2088.i, label %preload1088.i, label %postload1078.i.postload1089.i_crit_edge

postload1078.i.postload1089.i_crit_edge:          ; preds = %postload1078.i
  br label %postload1089.i

preload1088.i:                                    ; preds = %postload1078.i
  %loadedValue2481.i = load float addrspace(3)** %CastToValueType2466.i, align 8
  store float %extract289.i, float addrspace(3)* %loadedValue2481.i, align 4
  br label %postload1089.i

postload1089.i:                                   ; preds = %postload1078.i.postload1089.i_crit_edge, %preload1088.i
  %loadedValue2112.i = load i1* %CastToValueType2097.i, align 1
  br i1 %loadedValue2112.i, label %preload1099.i, label %postload1089.i.postload1100.i_crit_edge

postload1089.i.postload1100.i_crit_edge:          ; preds = %postload1089.i
  br label %postload1100.i

preload1099.i:                                    ; preds = %postload1089.i
  %loadedValue2500.i = load float addrspace(3)** %CastToValueType2485.i, align 8
  store float %extract290.i, float addrspace(3)* %loadedValue2500.i, align 4
  br label %postload1100.i

postload1100.i:                                   ; preds = %postload1089.i.postload1100.i_crit_edge, %preload1099.i
  %loadedValue2136.i = load i1* %CastToValueType2121.i, align 1
  br i1 %loadedValue2136.i, label %preload1110.i, label %postload1100.i.postload1111.i_crit_edge

postload1100.i.postload1111.i_crit_edge:          ; preds = %postload1100.i
  br label %postload1111.i

preload1110.i:                                    ; preds = %postload1100.i
  %loadedValue2519.i = load float addrspace(3)** %CastToValueType2504.i, align 8
  store float %extract291.i, float addrspace(3)* %loadedValue2519.i, align 4
  br label %postload1111.i

postload1111.i:                                   ; preds = %postload1100.i.postload1111.i_crit_edge, %preload1110.i
  %loadedValue2160.i = load i1* %CastToValueType2145.i, align 1
  br i1 %loadedValue2160.i, label %preload1121.i, label %postload1111.i.postload1122.i_crit_edge

postload1111.i.postload1122.i_crit_edge:          ; preds = %postload1111.i
  br label %postload1122.i

preload1121.i:                                    ; preds = %postload1111.i
  %loadedValue2538.i = load float addrspace(3)** %CastToValueType2523.i, align 8
  store float %extract292.i, float addrspace(3)* %loadedValue2538.i, align 4
  br label %postload1122.i

postload1122.i:                                   ; preds = %postload1111.i.postload1122.i_crit_edge, %preload1121.i
  %loadedValue2184.i = load i1* %CastToValueType2169.i, align 1
  br i1 %loadedValue2184.i, label %preload1132.i, label %postload1122.i.postload1133.i_crit_edge

postload1122.i.postload1133.i_crit_edge:          ; preds = %postload1122.i
  br label %postload1133.i

preload1132.i:                                    ; preds = %postload1122.i
  %loadedValue2557.i = load float addrspace(3)** %CastToValueType2542.i, align 8
  store float %extract293.i, float addrspace(3)* %loadedValue2557.i, align 4
  br label %postload1133.i

postload1133.i:                                   ; preds = %postload1122.i.postload1133.i_crit_edge, %preload1132.i
  %loadedValue1819.i = load i1* %CastToValueType1804.i, align 1
  br i1 %loadedValue1819.i, label %preload967.i, label %postload1133.i.postload968.i_crit_edge

postload1133.i.postload968.i_crit_edge:           ; preds = %postload1133.i
  br label %postload968.i

preload967.i:                                     ; preds = %postload1133.i
  %check.WI.iter2948.i = icmp ult i64 %CurrWI..1.i, %28
  br i1 %check.WI.iter2948.i, label %thenBB2945.i, label %preload967.i.postload968.i_crit_edge

preload967.i.postload968.i_crit_edge:             ; preds = %preload967.i
  br label %postload968.i

thenBB2945.i:                                     ; preds = %preload967.i
  %"CurrWI++2949.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride2951.i" = add nuw i64 %CurrSBIndex..1.i, 1792
  switch i32 %currBarrier.1.i, label %SyncBB.i [
    i32 23, label %thenBB2945.i.SyncBB2943.i_crit_edge
    i32 22, label %postload1149.i
    i32 21, label %SyncBB2941.i
    i32 20, label %thenBB2945.i.postload968.i_crit_edge
  ]

thenBB2945.i.SyncBB2943.i_crit_edge:              ; preds = %thenBB2945.i
  br label %SyncBB2943.i

thenBB2945.i.postload968.i_crit_edge:             ; preds = %thenBB2945.i
  br label %postload968.i

postload968.i:                                    ; preds = %thenBB.i.postload968.i_crit_edge, %thenBB2969.i.postload968.i_crit_edge, %thenBB2961.i.postload968.i_crit_edge, %thenBB2953.i.postload968.i_crit_edge, %thenBB2945.i.postload968.i_crit_edge, %preload967.i.postload968.i_crit_edge, %postload1133.i.postload968.i_crit_edge
  %currBarrier.3.i = phi i32 [ %currBarrier.1.i, %postload1133.i.postload968.i_crit_edge ], [ 20, %preload967.i.postload968.i_crit_edge ], [ %currBarrier.1.i, %thenBB2945.i.postload968.i_crit_edge ], [ %currBarrier.4.i, %thenBB2953.i.postload968.i_crit_edge ], [ %currBarrier.6.i, %thenBB2961.i.postload968.i_crit_edge ], [ %currBarrier.9.i, %thenBB2969.i.postload968.i_crit_edge ], [ %currBarrier.12.i, %thenBB.i.postload968.i_crit_edge ]
  %CurrSBIndex..3.i = phi i64 [ %CurrSBIndex..1.i, %postload1133.i.postload968.i_crit_edge ], [ 0, %preload967.i.postload968.i_crit_edge ], [ %"loadedCurrSB+Stride2951.i", %thenBB2945.i.postload968.i_crit_edge ], [ %"loadedCurrSB+Stride2959.i", %thenBB2953.i.postload968.i_crit_edge ], [ %"loadedCurrSB+Stride2967.i", %thenBB2961.i.postload968.i_crit_edge ], [ %"loadedCurrSB+Stride2975.i", %thenBB2969.i.postload968.i_crit_edge ], [ %"loadedCurrSB+Stride.i", %thenBB.i.postload968.i_crit_edge ]
  %CurrWI..3.i = phi i64 [ %CurrWI..1.i, %postload1133.i.postload968.i_crit_edge ], [ 0, %preload967.i.postload968.i_crit_edge ], [ %"CurrWI++2949.i", %thenBB2945.i.postload968.i_crit_edge ], [ %"CurrWI++2957.i", %thenBB2953.i.postload968.i_crit_edge ], [ %"CurrWI++2965.i", %thenBB2961.i.postload968.i_crit_edge ], [ %"CurrWI++2973.i", %thenBB2969.i.postload968.i_crit_edge ], [ %"CurrWI++.i", %thenBB.i.postload968.i_crit_edge ]
  %"&(pSB[currWI].offset)2236.i" = add nuw i64 %CurrSBIndex..3.i, 1280
  %"&pSB[currWI].offset2237.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2236.i"
  %CastToValueType2238.i = bitcast i8* %"&pSB[currWI].offset2237.i" to <16 x i64>*
  %loadedValue2239.i = load <16 x i64>* %CastToValueType2238.i, align 128
  %cmp1.i.i = icmp ugt <16 x i64> %loadedValue2239.i, <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %"&(pSB[currWI].offset)1391.i" = add nuw i64 %CurrSBIndex..3.i, 416
  %"&pSB[currWI].offset1392.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1391.i"
  %CastToValueType1393.i = bitcast i8* %"&pSB[currWI].offset1392.i" to <16 x i1>*
  %loadedValue1394.i = load <16 x i1>* %CastToValueType1393.i, align 16
  %if.end_to_for.body.i.preheader294.i = and <16 x i1> %loadedValue1394.i, %cmp1.i.i
  %negIncomingLoopMask40295.i = xor <16 x i1> %if.end_to_for.body.i.preheader294.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %ipred.i4.i = bitcast <16 x i1> %if.end_to_for.body.i.preheader294.i to i16
  %val.i5.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i4.i, i16 %ipred.i4.i) nounwind
  %187 = and i32 %val.i5.i, 1
  %res.i6.i = icmp eq i32 %187, 0
  br i1 %res.i6.i, label %for.body.i.i, label %scanLocalMem.exit.i

for.body.i.i:                                     ; preds = %postload1149.i, %postload968.i
  %currBarrier.4.i = phi i32 [ %currBarrier.8.i, %postload1149.i ], [ %currBarrier.3.i, %postload968.i ]
  %CurrSBIndex..4.i = phi i64 [ %CurrSBIndex..8.i, %postload1149.i ], [ %CurrSBIndex..3.i, %postload968.i ]
  %CurrWI..4.i = phi i64 [ %CurrWI..8.i, %postload1149.i ], [ %CurrWI..3.i, %postload968.i ]
  %vectorPHI296.i = phi <16 x i1> [ %loop_mask15389.i, %postload1149.i ], [ %negIncomingLoopMask40295.i, %postload968.i ]
  %vectorPHI298.i = phi <16 x i1> [ %local_edge408.i, %postload1149.i ], [ %if.end_to_for.body.i.preheader294.i, %postload968.i ]
  %i.02.i.i = phi i32 [ %mul.i.i, %postload1149.i ], [ 1, %postload968.i ]
  %"&(pSB[currWI].offset)2582.i" = add nuw i64 %CurrSBIndex..4.i, 1620
  %"&pSB[currWI].offset2583.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2582.i"
  %CastToValueType2584.i = bitcast i8* %"&pSB[currWI].offset2583.i" to i32*
  store i32 %i.02.i.i, i32* %CastToValueType2584.i, align 4
  %"&(pSB[currWI].offset)2568.i" = add nuw i64 %CurrSBIndex..4.i, 1616
  %"&pSB[currWI].offset2569.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2568.i"
  %CastToValueType2570.i = bitcast i8* %"&pSB[currWI].offset2569.i" to <16 x i1>*
  store <16 x i1> %vectorPHI298.i, <16 x i1>* %CastToValueType2570.i, align 16
  %"&(pSB[currWI].offset)2559.i" = add nuw i64 %CurrSBIndex..4.i, 1600
  %"&pSB[currWI].offset2560.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2559.i"
  %CastToValueType2561.i = bitcast i8* %"&pSB[currWI].offset2560.i" to <16 x i1>*
  store <16 x i1> %vectorPHI296.i, <16 x i1>* %CastToValueType2561.i, align 16
  %extract319.i = extractelement <16 x i1> %vectorPHI298.i, i32 0
  %"&(pSB[currWI].offset)2591.i" = add nuw i64 %CurrSBIndex..4.i, 1624
  %"&pSB[currWI].offset2592.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2591.i"
  %CastToValueType2593.i = bitcast i8* %"&pSB[currWI].offset2592.i" to i1*
  store i1 %extract319.i, i1* %CastToValueType2593.i, align 1
  %extract320.i = extractelement <16 x i1> %vectorPHI298.i, i32 1
  %"&(pSB[currWI].offset)2610.i" = add nuw i64 %CurrSBIndex..4.i, 1625
  %"&pSB[currWI].offset2611.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2610.i"
  %CastToValueType2612.i = bitcast i8* %"&pSB[currWI].offset2611.i" to i1*
  store i1 %extract320.i, i1* %CastToValueType2612.i, align 1
  %extract321.i = extractelement <16 x i1> %vectorPHI298.i, i32 2
  %"&(pSB[currWI].offset)2629.i" = add nuw i64 %CurrSBIndex..4.i, 1626
  %"&pSB[currWI].offset2630.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2629.i"
  %CastToValueType2631.i = bitcast i8* %"&pSB[currWI].offset2630.i" to i1*
  store i1 %extract321.i, i1* %CastToValueType2631.i, align 1
  %extract322.i = extractelement <16 x i1> %vectorPHI298.i, i32 3
  %"&(pSB[currWI].offset)2648.i" = add nuw i64 %CurrSBIndex..4.i, 1627
  %"&pSB[currWI].offset2649.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2648.i"
  %CastToValueType2650.i = bitcast i8* %"&pSB[currWI].offset2649.i" to i1*
  store i1 %extract322.i, i1* %CastToValueType2650.i, align 1
  %extract323.i = extractelement <16 x i1> %vectorPHI298.i, i32 4
  %"&(pSB[currWI].offset)2667.i" = add nuw i64 %CurrSBIndex..4.i, 1628
  %"&pSB[currWI].offset2668.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2667.i"
  %CastToValueType2669.i = bitcast i8* %"&pSB[currWI].offset2668.i" to i1*
  store i1 %extract323.i, i1* %CastToValueType2669.i, align 1
  %extract324.i = extractelement <16 x i1> %vectorPHI298.i, i32 5
  %"&(pSB[currWI].offset)2686.i" = add nuw i64 %CurrSBIndex..4.i, 1629
  %"&pSB[currWI].offset2687.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2686.i"
  %CastToValueType2688.i = bitcast i8* %"&pSB[currWI].offset2687.i" to i1*
  store i1 %extract324.i, i1* %CastToValueType2688.i, align 1
  %extract325.i = extractelement <16 x i1> %vectorPHI298.i, i32 6
  %"&(pSB[currWI].offset)2705.i" = add nuw i64 %CurrSBIndex..4.i, 1630
  %"&pSB[currWI].offset2706.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2705.i"
  %CastToValueType2707.i = bitcast i8* %"&pSB[currWI].offset2706.i" to i1*
  store i1 %extract325.i, i1* %CastToValueType2707.i, align 1
  %extract326.i = extractelement <16 x i1> %vectorPHI298.i, i32 7
  %"&(pSB[currWI].offset)2724.i" = add nuw i64 %CurrSBIndex..4.i, 1631
  %"&pSB[currWI].offset2725.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2724.i"
  %CastToValueType2726.i = bitcast i8* %"&pSB[currWI].offset2725.i" to i1*
  store i1 %extract326.i, i1* %CastToValueType2726.i, align 1
  %extract327.i = extractelement <16 x i1> %vectorPHI298.i, i32 8
  %"&(pSB[currWI].offset)2743.i" = add nuw i64 %CurrSBIndex..4.i, 1632
  %"&pSB[currWI].offset2744.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2743.i"
  %CastToValueType2745.i = bitcast i8* %"&pSB[currWI].offset2744.i" to i1*
  store i1 %extract327.i, i1* %CastToValueType2745.i, align 1
  %extract328.i = extractelement <16 x i1> %vectorPHI298.i, i32 9
  %"&(pSB[currWI].offset)2762.i" = add nuw i64 %CurrSBIndex..4.i, 1633
  %"&pSB[currWI].offset2763.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2762.i"
  %CastToValueType2764.i = bitcast i8* %"&pSB[currWI].offset2763.i" to i1*
  store i1 %extract328.i, i1* %CastToValueType2764.i, align 1
  %extract329.i = extractelement <16 x i1> %vectorPHI298.i, i32 10
  %"&(pSB[currWI].offset)2781.i" = add nuw i64 %CurrSBIndex..4.i, 1634
  %"&pSB[currWI].offset2782.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2781.i"
  %CastToValueType2783.i = bitcast i8* %"&pSB[currWI].offset2782.i" to i1*
  store i1 %extract329.i, i1* %CastToValueType2783.i, align 1
  %extract330.i = extractelement <16 x i1> %vectorPHI298.i, i32 11
  %"&(pSB[currWI].offset)2800.i" = add nuw i64 %CurrSBIndex..4.i, 1635
  %"&pSB[currWI].offset2801.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2800.i"
  %CastToValueType2802.i = bitcast i8* %"&pSB[currWI].offset2801.i" to i1*
  store i1 %extract330.i, i1* %CastToValueType2802.i, align 1
  %extract331.i = extractelement <16 x i1> %vectorPHI298.i, i32 12
  %"&(pSB[currWI].offset)2819.i" = add nuw i64 %CurrSBIndex..4.i, 1636
  %"&pSB[currWI].offset2820.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2819.i"
  %CastToValueType2821.i = bitcast i8* %"&pSB[currWI].offset2820.i" to i1*
  store i1 %extract331.i, i1* %CastToValueType2821.i, align 1
  %extract332.i = extractelement <16 x i1> %vectorPHI298.i, i32 13
  %"&(pSB[currWI].offset)2838.i" = add nuw i64 %CurrSBIndex..4.i, 1637
  %"&pSB[currWI].offset2839.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2838.i"
  %CastToValueType2840.i = bitcast i8* %"&pSB[currWI].offset2839.i" to i1*
  store i1 %extract332.i, i1* %CastToValueType2840.i, align 1
  %extract333.i = extractelement <16 x i1> %vectorPHI298.i, i32 14
  %"&(pSB[currWI].offset)2857.i" = add nuw i64 %CurrSBIndex..4.i, 1638
  %"&pSB[currWI].offset2858.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2857.i"
  %CastToValueType2859.i = bitcast i8* %"&pSB[currWI].offset2858.i" to i1*
  store i1 %extract333.i, i1* %CastToValueType2859.i, align 1
  %extract334.i = extractelement <16 x i1> %vectorPHI298.i, i32 15
  %"&(pSB[currWI].offset)2876.i" = add nuw i64 %CurrSBIndex..4.i, 1639
  %"&pSB[currWI].offset2877.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2876.i"
  %CastToValueType2878.i = bitcast i8* %"&pSB[currWI].offset2877.i" to i1*
  store i1 %extract334.i, i1* %CastToValueType2878.i, align 1
  %temp299.i = insertelement <16 x i32> undef, i32 %i.02.i.i, i32 0
  %vector300.i = shufflevector <16 x i32> %temp299.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)2250.i" = add nuw i64 %CurrSBIndex..4.i, 1408
  %"&pSB[currWI].offset2251.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2250.i"
  %CastToValueType2252.i = bitcast i8* %"&pSB[currWI].offset2251.i" to <16 x i32>*
  %loadedValue2253.i = load <16 x i32>* %CastToValueType2252.i, align 64
  %sub.i301.i = sub nsw <16 x i32> %loadedValue2253.i, %vector300.i
  %idxprom9.i302.i = sext <16 x i32> %sub.i301.i to <16 x i64>
  %extract304.i = extractelement <16 x i64> %idxprom9.i302.i, i32 1
  %extract305.i = extractelement <16 x i64> %idxprom9.i302.i, i32 2
  %extract306.i = extractelement <16 x i64> %idxprom9.i302.i, i32 3
  %extract307.i = extractelement <16 x i64> %idxprom9.i302.i, i32 4
  %extract308.i = extractelement <16 x i64> %idxprom9.i302.i, i32 5
  %extract309.i = extractelement <16 x i64> %idxprom9.i302.i, i32 6
  %extract310.i = extractelement <16 x i64> %idxprom9.i302.i, i32 7
  %extract311.i = extractelement <16 x i64> %idxprom9.i302.i, i32 8
  %extract312.i = extractelement <16 x i64> %idxprom9.i302.i, i32 9
  %extract313.i = extractelement <16 x i64> %idxprom9.i302.i, i32 10
  %extract314.i = extractelement <16 x i64> %idxprom9.i302.i, i32 11
  %extract315.i = extractelement <16 x i64> %idxprom9.i302.i, i32 12
  %extract316.i = extractelement <16 x i64> %idxprom9.i302.i, i32 13
  %extract317.i = extractelement <16 x i64> %idxprom9.i302.i, i32 14
  %extract318.i = extractelement <16 x i64> %idxprom9.i302.i, i32 15
  %188 = getelementptr inbounds float addrspace(3)* %13, i64 %extract304.i
  %189 = getelementptr inbounds float addrspace(3)* %13, i64 %extract305.i
  %190 = getelementptr inbounds float addrspace(3)* %13, i64 %extract306.i
  %191 = getelementptr inbounds float addrspace(3)* %13, i64 %extract307.i
  %192 = getelementptr inbounds float addrspace(3)* %13, i64 %extract308.i
  %193 = getelementptr inbounds float addrspace(3)* %13, i64 %extract309.i
  %194 = getelementptr inbounds float addrspace(3)* %13, i64 %extract310.i
  %195 = getelementptr inbounds float addrspace(3)* %13, i64 %extract311.i
  %196 = getelementptr inbounds float addrspace(3)* %13, i64 %extract312.i
  %197 = getelementptr inbounds float addrspace(3)* %13, i64 %extract313.i
  %198 = getelementptr inbounds float addrspace(3)* %13, i64 %extract314.i
  %199 = getelementptr inbounds float addrspace(3)* %13, i64 %extract315.i
  %200 = getelementptr inbounds float addrspace(3)* %13, i64 %extract316.i
  %201 = getelementptr inbounds float addrspace(3)* %13, i64 %extract317.i
  %202 = getelementptr inbounds float addrspace(3)* %13, i64 %extract318.i
  br i1 %extract319.i, label %preload1140.i, label %postload1141.i

preload1140.i:                                    ; preds = %for.body.i.i
  %extract303.i = extractelement <16 x i64> %idxprom9.i302.i, i32 0
  %203 = getelementptr inbounds float addrspace(3)* %13, i64 %extract303.i
  %masked_load703.i = load float addrspace(3)* %203, align 4
  br label %postload1141.i

postload1141.i:                                   ; preds = %preload1140.i, %for.body.i.i
  %phi1142.i = phi float [ undef, %for.body.i.i ], [ %masked_load703.i, %preload1140.i ]
  br i1 %extract320.i, label %preload1150.i, label %postload1151.i

preload1150.i:                                    ; preds = %postload1141.i
  %masked_load704.i = load float addrspace(3)* %188, align 4
  br label %postload1151.i

postload1151.i:                                   ; preds = %preload1150.i, %postload1141.i
  %phi1152.i = phi float [ undef, %postload1141.i ], [ %masked_load704.i, %preload1150.i ]
  br i1 %extract321.i, label %preload1158.i, label %postload1159.i

preload1158.i:                                    ; preds = %postload1151.i
  %masked_load705.i = load float addrspace(3)* %189, align 4
  br label %postload1159.i

postload1159.i:                                   ; preds = %preload1158.i, %postload1151.i
  %phi1160.i = phi float [ undef, %postload1151.i ], [ %masked_load705.i, %preload1158.i ]
  br i1 %extract322.i, label %preload1166.i, label %postload1167.i

preload1166.i:                                    ; preds = %postload1159.i
  %masked_load706.i = load float addrspace(3)* %190, align 4
  br label %postload1167.i

postload1167.i:                                   ; preds = %preload1166.i, %postload1159.i
  %phi1168.i = phi float [ undef, %postload1159.i ], [ %masked_load706.i, %preload1166.i ]
  br i1 %extract323.i, label %preload848.i, label %postload849.i

preload848.i:                                     ; preds = %postload1167.i
  %masked_load707.i = load float addrspace(3)* %191, align 4
  br label %postload849.i

postload849.i:                                    ; preds = %preload848.i, %postload1167.i
  %phi850.i = phi float [ undef, %postload1167.i ], [ %masked_load707.i, %preload848.i ]
  br i1 %extract324.i, label %preload856.i, label %postload857.i

preload856.i:                                     ; preds = %postload849.i
  %masked_load708.i = load float addrspace(3)* %192, align 4
  br label %postload857.i

postload857.i:                                    ; preds = %preload856.i, %postload849.i
  %phi858.i = phi float [ undef, %postload849.i ], [ %masked_load708.i, %preload856.i ]
  br i1 %extract325.i, label %preload864.i, label %postload865.i

preload864.i:                                     ; preds = %postload857.i
  %masked_load709.i = load float addrspace(3)* %193, align 4
  br label %postload865.i

postload865.i:                                    ; preds = %preload864.i, %postload857.i
  %phi866.i = phi float [ undef, %postload857.i ], [ %masked_load709.i, %preload864.i ]
  br i1 %extract326.i, label %preload872.i, label %postload873.i

preload872.i:                                     ; preds = %postload865.i
  %masked_load710.i = load float addrspace(3)* %194, align 4
  br label %postload873.i

postload873.i:                                    ; preds = %preload872.i, %postload865.i
  %phi874.i = phi float [ undef, %postload865.i ], [ %masked_load710.i, %preload872.i ]
  br i1 %extract327.i, label %preload880.i, label %postload881.i

preload880.i:                                     ; preds = %postload873.i
  %masked_load711.i = load float addrspace(3)* %195, align 4
  br label %postload881.i

postload881.i:                                    ; preds = %preload880.i, %postload873.i
  %phi882.i = phi float [ undef, %postload873.i ], [ %masked_load711.i, %preload880.i ]
  br i1 %extract328.i, label %preload888.i, label %postload889.i

preload888.i:                                     ; preds = %postload881.i
  %masked_load712.i = load float addrspace(3)* %196, align 4
  br label %postload889.i

postload889.i:                                    ; preds = %preload888.i, %postload881.i
  %phi890.i = phi float [ undef, %postload881.i ], [ %masked_load712.i, %preload888.i ]
  br i1 %extract329.i, label %preload896.i, label %postload897.i

preload896.i:                                     ; preds = %postload889.i
  %masked_load713.i = load float addrspace(3)* %197, align 4
  br label %postload897.i

postload897.i:                                    ; preds = %preload896.i, %postload889.i
  %phi898.i = phi float [ undef, %postload889.i ], [ %masked_load713.i, %preload896.i ]
  br i1 %extract330.i, label %preload904.i, label %postload905.i

preload904.i:                                     ; preds = %postload897.i
  %masked_load714.i = load float addrspace(3)* %198, align 4
  br label %postload905.i

postload905.i:                                    ; preds = %preload904.i, %postload897.i
  %phi906.i = phi float [ undef, %postload897.i ], [ %masked_load714.i, %preload904.i ]
  br i1 %extract331.i, label %preload912.i, label %postload913.i

preload912.i:                                     ; preds = %postload905.i
  %masked_load715.i = load float addrspace(3)* %199, align 4
  br label %postload913.i

postload913.i:                                    ; preds = %preload912.i, %postload905.i
  %phi914.i = phi float [ undef, %postload905.i ], [ %masked_load715.i, %preload912.i ]
  br i1 %extract332.i, label %preload920.i, label %postload921.i

preload920.i:                                     ; preds = %postload913.i
  %masked_load716.i = load float addrspace(3)* %200, align 4
  br label %postload921.i

postload921.i:                                    ; preds = %preload920.i, %postload913.i
  %phi922.i = phi float [ undef, %postload913.i ], [ %masked_load716.i, %preload920.i ]
  br i1 %extract333.i, label %preload928.i, label %postload929.i

preload928.i:                                     ; preds = %postload921.i
  %masked_load717.i = load float addrspace(3)* %201, align 4
  br label %postload929.i

postload929.i:                                    ; preds = %preload928.i, %postload921.i
  %phi930.i = phi float [ undef, %postload921.i ], [ %masked_load717.i, %preload928.i ]
  br i1 %extract334.i, label %preload936.i, label %postload937.i

preload936.i:                                     ; preds = %postload929.i
  %masked_load718.i = load float addrspace(3)* %202, align 4
  br label %postload937.i

postload937.i:                                    ; preds = %preload936.i, %postload929.i
  %phi938.i = phi float [ undef, %postload929.i ], [ %masked_load718.i, %preload936.i ]
  %temp.vect351.i = insertelement <16 x float> undef, float %phi1142.i, i32 0
  %temp.vect352.i = insertelement <16 x float> %temp.vect351.i, float %phi1152.i, i32 1
  %temp.vect353.i = insertelement <16 x float> %temp.vect352.i, float %phi1160.i, i32 2
  %temp.vect354.i = insertelement <16 x float> %temp.vect353.i, float %phi1168.i, i32 3
  %temp.vect355.i = insertelement <16 x float> %temp.vect354.i, float %phi850.i, i32 4
  %temp.vect356.i = insertelement <16 x float> %temp.vect355.i, float %phi858.i, i32 5
  %temp.vect357.i = insertelement <16 x float> %temp.vect356.i, float %phi866.i, i32 6
  %temp.vect358.i = insertelement <16 x float> %temp.vect357.i, float %phi874.i, i32 7
  %temp.vect359.i = insertelement <16 x float> %temp.vect358.i, float %phi882.i, i32 8
  %temp.vect360.i = insertelement <16 x float> %temp.vect359.i, float %phi890.i, i32 9
  %temp.vect361.i = insertelement <16 x float> %temp.vect360.i, float %phi898.i, i32 10
  %temp.vect362.i = insertelement <16 x float> %temp.vect361.i, float %phi906.i, i32 11
  %temp.vect363.i = insertelement <16 x float> %temp.vect362.i, float %phi914.i, i32 12
  %temp.vect364.i = insertelement <16 x float> %temp.vect363.i, float %phi922.i, i32 13
  %temp.vect365.i = insertelement <16 x float> %temp.vect364.i, float %phi930.i, i32 14
  %temp.vect366.i = insertelement <16 x float> %temp.vect365.i, float %phi938.i, i32 15
  %"&(pSB[currWI].offset)2895.i" = add nuw i64 %CurrSBIndex..4.i, 1664
  %"&pSB[currWI].offset2896.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2895.i"
  %CastToValueType2897.i = bitcast i8* %"&pSB[currWI].offset2896.i" to <16 x float>*
  store <16 x float> %temp.vect366.i, <16 x float>* %CastToValueType2897.i, align 64
  br i1 %extract319.i, label %preload1143.i, label %postload1144.i

preload1143.i:                                    ; preds = %postload937.i
  %check.WI.iter2956.i = icmp ult i64 %CurrWI..4.i, %28
  br i1 %check.WI.iter2956.i, label %thenBB2953.i, label %SyncBB2941.i

thenBB2953.i:                                     ; preds = %preload1143.i
  %"CurrWI++2957.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride2959.i" = add nuw i64 %CurrSBIndex..4.i, 1792
  switch i32 %currBarrier.4.i, label %thenBB2953.i.postload968.i_crit_edge [
    i32 24, label %SyncBB.i
    i32 23, label %thenBB2953.i.SyncBB2943.i_crit_edge
    i32 22, label %postload1149.i
    i32 21, label %SyncBB2941.i
  ]

thenBB2953.i.SyncBB2943.i_crit_edge:              ; preds = %thenBB2953.i
  br label %SyncBB2943.i

thenBB2953.i.postload968.i_crit_edge:             ; preds = %thenBB2953.i
  br label %postload968.i

SyncBB2941.i:                                     ; preds = %thenBB.i, %thenBB2969.i, %thenBB2961.i, %thenBB2953.i, %preload1143.i, %thenBB2945.i
  %currBarrier.5.i = phi i32 [ %currBarrier.4.i, %thenBB2953.i ], [ %currBarrier.6.i, %thenBB2961.i ], [ %currBarrier.9.i, %thenBB2969.i ], [ %currBarrier.1.i, %thenBB2945.i ], [ %currBarrier.12.i, %thenBB.i ], [ 21, %preload1143.i ]
  %CurrSBIndex..5.i = phi i64 [ %"loadedCurrSB+Stride2959.i", %thenBB2953.i ], [ %"loadedCurrSB+Stride2967.i", %thenBB2961.i ], [ %"loadedCurrSB+Stride2975.i", %thenBB2969.i ], [ %"loadedCurrSB+Stride2951.i", %thenBB2945.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %preload1143.i ]
  %CurrWI..5.i = phi i64 [ %"CurrWI++2957.i", %thenBB2953.i ], [ %"CurrWI++2965.i", %thenBB2961.i ], [ %"CurrWI++2973.i", %thenBB2969.i ], [ %"CurrWI++2949.i", %thenBB2945.i ], [ %"CurrWI++.i", %thenBB.i ], [ 0, %preload1143.i ]
  %"&(pSB[currWI].offset)2264.i" = add nuw i64 %CurrSBIndex..5.i, 1472
  %"&pSB[currWI].offset2265.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2264.i"
  %CastToValueType2266.i = bitcast i8* %"&pSB[currWI].offset2265.i" to float addrspace(3)**
  %loadedValue2267.i = load float addrspace(3)** %CastToValueType2266.i, align 8
  %masked_load719.i = load float addrspace(3)* %loadedValue2267.i, align 4
  br label %postload1144.i

postload1144.i:                                   ; preds = %SyncBB2941.i, %postload937.i
  %currBarrier.6.i = phi i32 [ %currBarrier.5.i, %SyncBB2941.i ], [ %currBarrier.4.i, %postload937.i ]
  %CurrSBIndex..6.i = phi i64 [ %CurrSBIndex..5.i, %SyncBB2941.i ], [ %CurrSBIndex..4.i, %postload937.i ]
  %CurrWI..6.i = phi i64 [ %CurrWI..5.i, %SyncBB2941.i ], [ %CurrWI..4.i, %postload937.i ]
  %phi1145.i = phi float [ %masked_load719.i, %SyncBB2941.i ], [ undef, %postload937.i ]
  %"&(pSB[currWI].offset)2619.i" = add nuw i64 %CurrSBIndex..6.i, 1625
  %"&pSB[currWI].offset2620.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2619.i"
  %CastToValueType2621.i = bitcast i8* %"&pSB[currWI].offset2620.i" to i1*
  %loadedValue2622.i = load i1* %CastToValueType2621.i, align 1
  br i1 %loadedValue2622.i, label %preload1153.i, label %postload1144.i.postload1154.i_crit_edge

postload1144.i.postload1154.i_crit_edge:          ; preds = %postload1144.i
  br label %postload1154.i

preload1153.i:                                    ; preds = %postload1144.i
  %"&(pSB[currWI].offset)2283.i" = add nuw i64 %CurrSBIndex..6.i, 1480
  %"&pSB[currWI].offset2284.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2283.i"
  %CastToValueType2285.i = bitcast i8* %"&pSB[currWI].offset2284.i" to float addrspace(3)**
  %loadedValue2286.i = load float addrspace(3)** %CastToValueType2285.i, align 8
  %masked_load720.i = load float addrspace(3)* %loadedValue2286.i, align 4
  br label %postload1154.i

postload1154.i:                                   ; preds = %postload1144.i.postload1154.i_crit_edge, %preload1153.i
  %phi1155.i = phi float [ %masked_load720.i, %preload1153.i ], [ undef, %postload1144.i.postload1154.i_crit_edge ]
  %"&(pSB[currWI].offset)2638.i" = add nuw i64 %CurrSBIndex..6.i, 1626
  %"&pSB[currWI].offset2639.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2638.i"
  %CastToValueType2640.i = bitcast i8* %"&pSB[currWI].offset2639.i" to i1*
  %loadedValue2641.i = load i1* %CastToValueType2640.i, align 1
  br i1 %loadedValue2641.i, label %preload1161.i, label %postload1154.i.postload1162.i_crit_edge

postload1154.i.postload1162.i_crit_edge:          ; preds = %postload1154.i
  br label %postload1162.i

preload1161.i:                                    ; preds = %postload1154.i
  %"&(pSB[currWI].offset)2302.i" = add nuw i64 %CurrSBIndex..6.i, 1488
  %"&pSB[currWI].offset2303.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2302.i"
  %CastToValueType2304.i = bitcast i8* %"&pSB[currWI].offset2303.i" to float addrspace(3)**
  %loadedValue2305.i = load float addrspace(3)** %CastToValueType2304.i, align 8
  %masked_load721.i = load float addrspace(3)* %loadedValue2305.i, align 4
  br label %postload1162.i

postload1162.i:                                   ; preds = %postload1154.i.postload1162.i_crit_edge, %preload1161.i
  %phi1163.i = phi float [ %masked_load721.i, %preload1161.i ], [ undef, %postload1154.i.postload1162.i_crit_edge ]
  %"&(pSB[currWI].offset)2657.i" = add nuw i64 %CurrSBIndex..6.i, 1627
  %"&pSB[currWI].offset2658.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2657.i"
  %CastToValueType2659.i = bitcast i8* %"&pSB[currWI].offset2658.i" to i1*
  %loadedValue2660.i = load i1* %CastToValueType2659.i, align 1
  br i1 %loadedValue2660.i, label %preload1169.i, label %postload1162.i.postload1170.i_crit_edge

postload1162.i.postload1170.i_crit_edge:          ; preds = %postload1162.i
  br label %postload1170.i

preload1169.i:                                    ; preds = %postload1162.i
  %"&(pSB[currWI].offset)2321.i" = add nuw i64 %CurrSBIndex..6.i, 1496
  %"&pSB[currWI].offset2322.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2321.i"
  %CastToValueType2323.i = bitcast i8* %"&pSB[currWI].offset2322.i" to float addrspace(3)**
  %loadedValue2324.i = load float addrspace(3)** %CastToValueType2323.i, align 8
  %masked_load722.i = load float addrspace(3)* %loadedValue2324.i, align 4
  br label %postload1170.i

postload1170.i:                                   ; preds = %postload1162.i.postload1170.i_crit_edge, %preload1169.i
  %phi1171.i = phi float [ %masked_load722.i, %preload1169.i ], [ undef, %postload1162.i.postload1170.i_crit_edge ]
  %"&(pSB[currWI].offset)2676.i" = add nuw i64 %CurrSBIndex..6.i, 1628
  %"&pSB[currWI].offset2677.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2676.i"
  %CastToValueType2678.i = bitcast i8* %"&pSB[currWI].offset2677.i" to i1*
  %loadedValue2679.i = load i1* %CastToValueType2678.i, align 1
  br i1 %loadedValue2679.i, label %preload851.i, label %postload1170.i.postload852.i_crit_edge

postload1170.i.postload852.i_crit_edge:           ; preds = %postload1170.i
  br label %postload852.i

preload851.i:                                     ; preds = %postload1170.i
  %"&(pSB[currWI].offset)2340.i" = add nuw i64 %CurrSBIndex..6.i, 1504
  %"&pSB[currWI].offset2341.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2340.i"
  %CastToValueType2342.i = bitcast i8* %"&pSB[currWI].offset2341.i" to float addrspace(3)**
  %loadedValue2343.i = load float addrspace(3)** %CastToValueType2342.i, align 8
  %masked_load723.i = load float addrspace(3)* %loadedValue2343.i, align 4
  br label %postload852.i

postload852.i:                                    ; preds = %postload1170.i.postload852.i_crit_edge, %preload851.i
  %phi853.i = phi float [ %masked_load723.i, %preload851.i ], [ undef, %postload1170.i.postload852.i_crit_edge ]
  %"&(pSB[currWI].offset)2695.i" = add nuw i64 %CurrSBIndex..6.i, 1629
  %"&pSB[currWI].offset2696.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2695.i"
  %CastToValueType2697.i = bitcast i8* %"&pSB[currWI].offset2696.i" to i1*
  %loadedValue2698.i = load i1* %CastToValueType2697.i, align 1
  br i1 %loadedValue2698.i, label %preload859.i, label %postload852.i.postload860.i_crit_edge

postload852.i.postload860.i_crit_edge:            ; preds = %postload852.i
  br label %postload860.i

preload859.i:                                     ; preds = %postload852.i
  %"&(pSB[currWI].offset)2359.i" = add nuw i64 %CurrSBIndex..6.i, 1512
  %"&pSB[currWI].offset2360.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2359.i"
  %CastToValueType2361.i = bitcast i8* %"&pSB[currWI].offset2360.i" to float addrspace(3)**
  %loadedValue2362.i = load float addrspace(3)** %CastToValueType2361.i, align 8
  %masked_load724.i = load float addrspace(3)* %loadedValue2362.i, align 4
  br label %postload860.i

postload860.i:                                    ; preds = %postload852.i.postload860.i_crit_edge, %preload859.i
  %phi861.i = phi float [ %masked_load724.i, %preload859.i ], [ undef, %postload852.i.postload860.i_crit_edge ]
  %"&(pSB[currWI].offset)2714.i" = add nuw i64 %CurrSBIndex..6.i, 1630
  %"&pSB[currWI].offset2715.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2714.i"
  %CastToValueType2716.i = bitcast i8* %"&pSB[currWI].offset2715.i" to i1*
  %loadedValue2717.i = load i1* %CastToValueType2716.i, align 1
  br i1 %loadedValue2717.i, label %preload867.i, label %postload860.i.postload868.i_crit_edge

postload860.i.postload868.i_crit_edge:            ; preds = %postload860.i
  br label %postload868.i

preload867.i:                                     ; preds = %postload860.i
  %"&(pSB[currWI].offset)2378.i" = add nuw i64 %CurrSBIndex..6.i, 1520
  %"&pSB[currWI].offset2379.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2378.i"
  %CastToValueType2380.i = bitcast i8* %"&pSB[currWI].offset2379.i" to float addrspace(3)**
  %loadedValue2381.i = load float addrspace(3)** %CastToValueType2380.i, align 8
  %masked_load725.i = load float addrspace(3)* %loadedValue2381.i, align 4
  br label %postload868.i

postload868.i:                                    ; preds = %postload860.i.postload868.i_crit_edge, %preload867.i
  %phi869.i = phi float [ %masked_load725.i, %preload867.i ], [ undef, %postload860.i.postload868.i_crit_edge ]
  %"&(pSB[currWI].offset)2733.i" = add nuw i64 %CurrSBIndex..6.i, 1631
  %"&pSB[currWI].offset2734.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2733.i"
  %CastToValueType2735.i = bitcast i8* %"&pSB[currWI].offset2734.i" to i1*
  %loadedValue2736.i = load i1* %CastToValueType2735.i, align 1
  br i1 %loadedValue2736.i, label %preload875.i, label %postload868.i.postload876.i_crit_edge

postload868.i.postload876.i_crit_edge:            ; preds = %postload868.i
  br label %postload876.i

preload875.i:                                     ; preds = %postload868.i
  %"&(pSB[currWI].offset)2397.i" = add nuw i64 %CurrSBIndex..6.i, 1528
  %"&pSB[currWI].offset2398.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2397.i"
  %CastToValueType2399.i = bitcast i8* %"&pSB[currWI].offset2398.i" to float addrspace(3)**
  %loadedValue2400.i = load float addrspace(3)** %CastToValueType2399.i, align 8
  %masked_load726.i = load float addrspace(3)* %loadedValue2400.i, align 4
  br label %postload876.i

postload876.i:                                    ; preds = %postload868.i.postload876.i_crit_edge, %preload875.i
  %phi877.i = phi float [ %masked_load726.i, %preload875.i ], [ undef, %postload868.i.postload876.i_crit_edge ]
  %"&(pSB[currWI].offset)2752.i" = add nuw i64 %CurrSBIndex..6.i, 1632
  %"&pSB[currWI].offset2753.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2752.i"
  %CastToValueType2754.i = bitcast i8* %"&pSB[currWI].offset2753.i" to i1*
  %loadedValue2755.i = load i1* %CastToValueType2754.i, align 1
  br i1 %loadedValue2755.i, label %preload883.i, label %postload876.i.postload884.i_crit_edge

postload876.i.postload884.i_crit_edge:            ; preds = %postload876.i
  br label %postload884.i

preload883.i:                                     ; preds = %postload876.i
  %"&(pSB[currWI].offset)2416.i" = add nuw i64 %CurrSBIndex..6.i, 1536
  %"&pSB[currWI].offset2417.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2416.i"
  %CastToValueType2418.i = bitcast i8* %"&pSB[currWI].offset2417.i" to float addrspace(3)**
  %loadedValue2419.i = load float addrspace(3)** %CastToValueType2418.i, align 8
  %masked_load727.i = load float addrspace(3)* %loadedValue2419.i, align 4
  br label %postload884.i

postload884.i:                                    ; preds = %postload876.i.postload884.i_crit_edge, %preload883.i
  %phi885.i = phi float [ %masked_load727.i, %preload883.i ], [ undef, %postload876.i.postload884.i_crit_edge ]
  %"&(pSB[currWI].offset)2771.i" = add nuw i64 %CurrSBIndex..6.i, 1633
  %"&pSB[currWI].offset2772.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2771.i"
  %CastToValueType2773.i = bitcast i8* %"&pSB[currWI].offset2772.i" to i1*
  %loadedValue2774.i = load i1* %CastToValueType2773.i, align 1
  br i1 %loadedValue2774.i, label %preload891.i, label %postload884.i.postload892.i_crit_edge

postload884.i.postload892.i_crit_edge:            ; preds = %postload884.i
  br label %postload892.i

preload891.i:                                     ; preds = %postload884.i
  %"&(pSB[currWI].offset)2435.i" = add nuw i64 %CurrSBIndex..6.i, 1544
  %"&pSB[currWI].offset2436.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2435.i"
  %CastToValueType2437.i = bitcast i8* %"&pSB[currWI].offset2436.i" to float addrspace(3)**
  %loadedValue2438.i = load float addrspace(3)** %CastToValueType2437.i, align 8
  %masked_load728.i = load float addrspace(3)* %loadedValue2438.i, align 4
  br label %postload892.i

postload892.i:                                    ; preds = %postload884.i.postload892.i_crit_edge, %preload891.i
  %phi893.i = phi float [ %masked_load728.i, %preload891.i ], [ undef, %postload884.i.postload892.i_crit_edge ]
  %"&(pSB[currWI].offset)2790.i" = add nuw i64 %CurrSBIndex..6.i, 1634
  %"&pSB[currWI].offset2791.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2790.i"
  %CastToValueType2792.i = bitcast i8* %"&pSB[currWI].offset2791.i" to i1*
  %loadedValue2793.i = load i1* %CastToValueType2792.i, align 1
  br i1 %loadedValue2793.i, label %preload899.i, label %postload892.i.postload900.i_crit_edge

postload892.i.postload900.i_crit_edge:            ; preds = %postload892.i
  br label %postload900.i

preload899.i:                                     ; preds = %postload892.i
  %"&(pSB[currWI].offset)2454.i" = add nuw i64 %CurrSBIndex..6.i, 1552
  %"&pSB[currWI].offset2455.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2454.i"
  %CastToValueType2456.i = bitcast i8* %"&pSB[currWI].offset2455.i" to float addrspace(3)**
  %loadedValue2457.i = load float addrspace(3)** %CastToValueType2456.i, align 8
  %masked_load729.i = load float addrspace(3)* %loadedValue2457.i, align 4
  br label %postload900.i

postload900.i:                                    ; preds = %postload892.i.postload900.i_crit_edge, %preload899.i
  %phi901.i = phi float [ %masked_load729.i, %preload899.i ], [ undef, %postload892.i.postload900.i_crit_edge ]
  %"&(pSB[currWI].offset)2809.i" = add nuw i64 %CurrSBIndex..6.i, 1635
  %"&pSB[currWI].offset2810.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2809.i"
  %CastToValueType2811.i = bitcast i8* %"&pSB[currWI].offset2810.i" to i1*
  %loadedValue2812.i = load i1* %CastToValueType2811.i, align 1
  br i1 %loadedValue2812.i, label %preload907.i, label %postload900.i.postload908.i_crit_edge

postload900.i.postload908.i_crit_edge:            ; preds = %postload900.i
  br label %postload908.i

preload907.i:                                     ; preds = %postload900.i
  %"&(pSB[currWI].offset)2473.i" = add nuw i64 %CurrSBIndex..6.i, 1560
  %"&pSB[currWI].offset2474.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2473.i"
  %CastToValueType2475.i = bitcast i8* %"&pSB[currWI].offset2474.i" to float addrspace(3)**
  %loadedValue2476.i = load float addrspace(3)** %CastToValueType2475.i, align 8
  %masked_load730.i = load float addrspace(3)* %loadedValue2476.i, align 4
  br label %postload908.i

postload908.i:                                    ; preds = %postload900.i.postload908.i_crit_edge, %preload907.i
  %phi909.i = phi float [ %masked_load730.i, %preload907.i ], [ undef, %postload900.i.postload908.i_crit_edge ]
  %"&(pSB[currWI].offset)2828.i" = add nuw i64 %CurrSBIndex..6.i, 1636
  %"&pSB[currWI].offset2829.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2828.i"
  %CastToValueType2830.i = bitcast i8* %"&pSB[currWI].offset2829.i" to i1*
  %loadedValue2831.i = load i1* %CastToValueType2830.i, align 1
  br i1 %loadedValue2831.i, label %preload915.i, label %postload908.i.postload916.i_crit_edge

postload908.i.postload916.i_crit_edge:            ; preds = %postload908.i
  br label %postload916.i

preload915.i:                                     ; preds = %postload908.i
  %"&(pSB[currWI].offset)2492.i" = add nuw i64 %CurrSBIndex..6.i, 1568
  %"&pSB[currWI].offset2493.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2492.i"
  %CastToValueType2494.i = bitcast i8* %"&pSB[currWI].offset2493.i" to float addrspace(3)**
  %loadedValue2495.i = load float addrspace(3)** %CastToValueType2494.i, align 8
  %masked_load731.i = load float addrspace(3)* %loadedValue2495.i, align 4
  br label %postload916.i

postload916.i:                                    ; preds = %postload908.i.postload916.i_crit_edge, %preload915.i
  %phi917.i = phi float [ %masked_load731.i, %preload915.i ], [ undef, %postload908.i.postload916.i_crit_edge ]
  %"&(pSB[currWI].offset)2847.i" = add nuw i64 %CurrSBIndex..6.i, 1637
  %"&pSB[currWI].offset2848.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2847.i"
  %CastToValueType2849.i = bitcast i8* %"&pSB[currWI].offset2848.i" to i1*
  %loadedValue2850.i = load i1* %CastToValueType2849.i, align 1
  br i1 %loadedValue2850.i, label %preload923.i, label %postload916.i.postload924.i_crit_edge

postload916.i.postload924.i_crit_edge:            ; preds = %postload916.i
  br label %postload924.i

preload923.i:                                     ; preds = %postload916.i
  %"&(pSB[currWI].offset)2511.i" = add nuw i64 %CurrSBIndex..6.i, 1576
  %"&pSB[currWI].offset2512.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2511.i"
  %CastToValueType2513.i = bitcast i8* %"&pSB[currWI].offset2512.i" to float addrspace(3)**
  %loadedValue2514.i = load float addrspace(3)** %CastToValueType2513.i, align 8
  %masked_load732.i = load float addrspace(3)* %loadedValue2514.i, align 4
  br label %postload924.i

postload924.i:                                    ; preds = %postload916.i.postload924.i_crit_edge, %preload923.i
  %phi925.i = phi float [ %masked_load732.i, %preload923.i ], [ undef, %postload916.i.postload924.i_crit_edge ]
  %"&(pSB[currWI].offset)2866.i" = add nuw i64 %CurrSBIndex..6.i, 1638
  %"&pSB[currWI].offset2867.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2866.i"
  %CastToValueType2868.i = bitcast i8* %"&pSB[currWI].offset2867.i" to i1*
  %loadedValue2869.i = load i1* %CastToValueType2868.i, align 1
  br i1 %loadedValue2869.i, label %preload931.i, label %postload924.i.postload932.i_crit_edge

postload924.i.postload932.i_crit_edge:            ; preds = %postload924.i
  br label %postload932.i

preload931.i:                                     ; preds = %postload924.i
  %"&(pSB[currWI].offset)2530.i" = add nuw i64 %CurrSBIndex..6.i, 1584
  %"&pSB[currWI].offset2531.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2530.i"
  %CastToValueType2532.i = bitcast i8* %"&pSB[currWI].offset2531.i" to float addrspace(3)**
  %loadedValue2533.i = load float addrspace(3)** %CastToValueType2532.i, align 8
  %masked_load733.i = load float addrspace(3)* %loadedValue2533.i, align 4
  br label %postload932.i

postload932.i:                                    ; preds = %postload924.i.postload932.i_crit_edge, %preload931.i
  %phi933.i = phi float [ %masked_load733.i, %preload931.i ], [ undef, %postload924.i.postload932.i_crit_edge ]
  %"&(pSB[currWI].offset)2885.i" = add nuw i64 %CurrSBIndex..6.i, 1639
  %"&pSB[currWI].offset2886.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2885.i"
  %CastToValueType2887.i = bitcast i8* %"&pSB[currWI].offset2886.i" to i1*
  %loadedValue2888.i = load i1* %CastToValueType2887.i, align 1
  br i1 %loadedValue2888.i, label %preload939.i, label %postload932.i.postload940.i_crit_edge

postload932.i.postload940.i_crit_edge:            ; preds = %postload932.i
  br label %postload940.i

preload939.i:                                     ; preds = %postload932.i
  %"&(pSB[currWI].offset)2549.i" = add nuw i64 %CurrSBIndex..6.i, 1592
  %"&pSB[currWI].offset2550.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2549.i"
  %CastToValueType2551.i = bitcast i8* %"&pSB[currWI].offset2550.i" to float addrspace(3)**
  %loadedValue2552.i = load float addrspace(3)** %CastToValueType2551.i, align 8
  %masked_load734.i = load float addrspace(3)* %loadedValue2552.i, align 4
  br label %postload940.i

postload940.i:                                    ; preds = %postload932.i.postload940.i_crit_edge, %preload939.i
  %phi941.i = phi float [ %masked_load734.i, %preload939.i ], [ undef, %postload932.i.postload940.i_crit_edge ]
  %temp.vect335.i = insertelement <16 x float> undef, float %phi1145.i, i32 0
  %temp.vect336.i = insertelement <16 x float> %temp.vect335.i, float %phi1155.i, i32 1
  %temp.vect337.i = insertelement <16 x float> %temp.vect336.i, float %phi1163.i, i32 2
  %temp.vect338.i = insertelement <16 x float> %temp.vect337.i, float %phi1171.i, i32 3
  %temp.vect339.i = insertelement <16 x float> %temp.vect338.i, float %phi853.i, i32 4
  %temp.vect340.i = insertelement <16 x float> %temp.vect339.i, float %phi861.i, i32 5
  %temp.vect341.i = insertelement <16 x float> %temp.vect340.i, float %phi869.i, i32 6
  %temp.vect342.i = insertelement <16 x float> %temp.vect341.i, float %phi877.i, i32 7
  %temp.vect343.i = insertelement <16 x float> %temp.vect342.i, float %phi885.i, i32 8
  %temp.vect344.i = insertelement <16 x float> %temp.vect343.i, float %phi893.i, i32 9
  %temp.vect345.i = insertelement <16 x float> %temp.vect344.i, float %phi901.i, i32 10
  %temp.vect346.i = insertelement <16 x float> %temp.vect345.i, float %phi909.i, i32 11
  %temp.vect347.i = insertelement <16 x float> %temp.vect346.i, float %phi917.i, i32 12
  %temp.vect348.i = insertelement <16 x float> %temp.vect347.i, float %phi925.i, i32 13
  %temp.vect349.i = insertelement <16 x float> %temp.vect348.i, float %phi933.i, i32 14
  %temp.vect350.i = insertelement <16 x float> %temp.vect349.i, float %phi941.i, i32 15
  %"&(pSB[currWI].offset)2899.i" = add nuw i64 %CurrSBIndex..6.i, 1664
  %"&pSB[currWI].offset2900.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2899.i"
  %CastToValueType2901.i = bitcast i8* %"&pSB[currWI].offset2900.i" to <16 x float>*
  %loadedValue2902.i = load <16 x float>* %CastToValueType2901.i, align 64
  %add13.i367.i = fadd <16 x float> %temp.vect350.i, %loadedValue2902.i
  %extract369.i = extractelement <16 x float> %add13.i367.i, i32 1
  %extract370.i = extractelement <16 x float> %add13.i367.i, i32 2
  %extract371.i = extractelement <16 x float> %add13.i367.i, i32 3
  %extract372.i = extractelement <16 x float> %add13.i367.i, i32 4
  %extract373.i = extractelement <16 x float> %add13.i367.i, i32 5
  %extract374.i = extractelement <16 x float> %add13.i367.i, i32 6
  %extract375.i = extractelement <16 x float> %add13.i367.i, i32 7
  %extract376.i = extractelement <16 x float> %add13.i367.i, i32 8
  %extract377.i = extractelement <16 x float> %add13.i367.i, i32 9
  %extract378.i = extractelement <16 x float> %add13.i367.i, i32 10
  %extract379.i = extractelement <16 x float> %add13.i367.i, i32 11
  %extract380.i = extractelement <16 x float> %add13.i367.i, i32 12
  %extract381.i = extractelement <16 x float> %add13.i367.i, i32 13
  %extract382.i = extractelement <16 x float> %add13.i367.i, i32 14
  %extract383.i = extractelement <16 x float> %add13.i367.i, i32 15
  %"&(pSB[currWI].offset)2600.i" = add nuw i64 %CurrSBIndex..6.i, 1624
  %"&pSB[currWI].offset2601.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2600.i"
  %CastToValueType2602.i = bitcast i8* %"&pSB[currWI].offset2601.i" to i1*
  %loadedValue2603.i = load i1* %CastToValueType2602.i, align 1
  br i1 %loadedValue2603.i, label %preload1146.i, label %postload1147.i

preload1146.i:                                    ; preds = %postload940.i
  %extract368.i = extractelement <16 x float> %add13.i367.i, i32 0
  %"&(pSB[currWI].offset)2259.i" = add nuw i64 %CurrSBIndex..6.i, 1472
  %"&pSB[currWI].offset2260.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2259.i"
  %CastToValueType2261.i = bitcast i8* %"&pSB[currWI].offset2260.i" to float addrspace(3)**
  %loadedValue2262.i = load float addrspace(3)** %CastToValueType2261.i, align 8
  store float %extract368.i, float addrspace(3)* %loadedValue2262.i, align 4
  %loadedValue2617.pre.i = load i1* %CastToValueType2621.i, align 1
  br label %postload1147.i

postload1147.i:                                   ; preds = %preload1146.i, %postload940.i
  %loadedValue2617.i = phi i1 [ %loadedValue2617.pre.i, %preload1146.i ], [ %loadedValue2622.i, %postload940.i ]
  br i1 %loadedValue2617.i, label %preload1156.i, label %postload1147.i.postload1157.i_crit_edge

postload1147.i.postload1157.i_crit_edge:          ; preds = %postload1147.i
  br label %postload1157.i

preload1156.i:                                    ; preds = %postload1147.i
  %"&(pSB[currWI].offset)2278.i" = add nuw i64 %CurrSBIndex..6.i, 1480
  %"&pSB[currWI].offset2279.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2278.i"
  %CastToValueType2280.i = bitcast i8* %"&pSB[currWI].offset2279.i" to float addrspace(3)**
  %loadedValue2281.i = load float addrspace(3)** %CastToValueType2280.i, align 8
  store float %extract369.i, float addrspace(3)* %loadedValue2281.i, align 4
  br label %postload1157.i

postload1157.i:                                   ; preds = %postload1147.i.postload1157.i_crit_edge, %preload1156.i
  %loadedValue2636.i = load i1* %CastToValueType2640.i, align 1
  br i1 %loadedValue2636.i, label %preload1164.i, label %postload1157.i.postload1165.i_crit_edge

postload1157.i.postload1165.i_crit_edge:          ; preds = %postload1157.i
  br label %postload1165.i

preload1164.i:                                    ; preds = %postload1157.i
  %"&(pSB[currWI].offset)2297.i" = add nuw i64 %CurrSBIndex..6.i, 1488
  %"&pSB[currWI].offset2298.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2297.i"
  %CastToValueType2299.i = bitcast i8* %"&pSB[currWI].offset2298.i" to float addrspace(3)**
  %loadedValue2300.i = load float addrspace(3)** %CastToValueType2299.i, align 8
  store float %extract370.i, float addrspace(3)* %loadedValue2300.i, align 4
  br label %postload1165.i

postload1165.i:                                   ; preds = %postload1157.i.postload1165.i_crit_edge, %preload1164.i
  %loadedValue2655.i = load i1* %CastToValueType2659.i, align 1
  br i1 %loadedValue2655.i, label %preload1172.i, label %postload1165.i.postload1173.i_crit_edge

postload1165.i.postload1173.i_crit_edge:          ; preds = %postload1165.i
  br label %postload1173.i

preload1172.i:                                    ; preds = %postload1165.i
  %"&(pSB[currWI].offset)2316.i" = add nuw i64 %CurrSBIndex..6.i, 1496
  %"&pSB[currWI].offset2317.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2316.i"
  %CastToValueType2318.i = bitcast i8* %"&pSB[currWI].offset2317.i" to float addrspace(3)**
  %loadedValue2319.i = load float addrspace(3)** %CastToValueType2318.i, align 8
  store float %extract371.i, float addrspace(3)* %loadedValue2319.i, align 4
  br label %postload1173.i

postload1173.i:                                   ; preds = %postload1165.i.postload1173.i_crit_edge, %preload1172.i
  %loadedValue2674.i = load i1* %CastToValueType2678.i, align 1
  br i1 %loadedValue2674.i, label %preload854.i, label %postload1173.i.postload855.i_crit_edge

postload1173.i.postload855.i_crit_edge:           ; preds = %postload1173.i
  br label %postload855.i

preload854.i:                                     ; preds = %postload1173.i
  %"&(pSB[currWI].offset)2335.i" = add nuw i64 %CurrSBIndex..6.i, 1504
  %"&pSB[currWI].offset2336.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2335.i"
  %CastToValueType2337.i = bitcast i8* %"&pSB[currWI].offset2336.i" to float addrspace(3)**
  %loadedValue2338.i = load float addrspace(3)** %CastToValueType2337.i, align 8
  store float %extract372.i, float addrspace(3)* %loadedValue2338.i, align 4
  br label %postload855.i

postload855.i:                                    ; preds = %postload1173.i.postload855.i_crit_edge, %preload854.i
  %loadedValue2693.i = load i1* %CastToValueType2697.i, align 1
  br i1 %loadedValue2693.i, label %preload862.i, label %postload855.i.postload863.i_crit_edge

postload855.i.postload863.i_crit_edge:            ; preds = %postload855.i
  br label %postload863.i

preload862.i:                                     ; preds = %postload855.i
  %"&(pSB[currWI].offset)2354.i" = add nuw i64 %CurrSBIndex..6.i, 1512
  %"&pSB[currWI].offset2355.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2354.i"
  %CastToValueType2356.i = bitcast i8* %"&pSB[currWI].offset2355.i" to float addrspace(3)**
  %loadedValue2357.i = load float addrspace(3)** %CastToValueType2356.i, align 8
  store float %extract373.i, float addrspace(3)* %loadedValue2357.i, align 4
  br label %postload863.i

postload863.i:                                    ; preds = %postload855.i.postload863.i_crit_edge, %preload862.i
  %loadedValue2712.i = load i1* %CastToValueType2716.i, align 1
  br i1 %loadedValue2712.i, label %preload870.i, label %postload863.i.postload871.i_crit_edge

postload863.i.postload871.i_crit_edge:            ; preds = %postload863.i
  br label %postload871.i

preload870.i:                                     ; preds = %postload863.i
  %"&(pSB[currWI].offset)2373.i" = add nuw i64 %CurrSBIndex..6.i, 1520
  %"&pSB[currWI].offset2374.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2373.i"
  %CastToValueType2375.i = bitcast i8* %"&pSB[currWI].offset2374.i" to float addrspace(3)**
  %loadedValue2376.i = load float addrspace(3)** %CastToValueType2375.i, align 8
  store float %extract374.i, float addrspace(3)* %loadedValue2376.i, align 4
  br label %postload871.i

postload871.i:                                    ; preds = %postload863.i.postload871.i_crit_edge, %preload870.i
  %loadedValue2731.i = load i1* %CastToValueType2735.i, align 1
  br i1 %loadedValue2731.i, label %preload878.i, label %postload871.i.postload879.i_crit_edge

postload871.i.postload879.i_crit_edge:            ; preds = %postload871.i
  br label %postload879.i

preload878.i:                                     ; preds = %postload871.i
  %"&(pSB[currWI].offset)2392.i" = add nuw i64 %CurrSBIndex..6.i, 1528
  %"&pSB[currWI].offset2393.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2392.i"
  %CastToValueType2394.i = bitcast i8* %"&pSB[currWI].offset2393.i" to float addrspace(3)**
  %loadedValue2395.i = load float addrspace(3)** %CastToValueType2394.i, align 8
  store float %extract375.i, float addrspace(3)* %loadedValue2395.i, align 4
  br label %postload879.i

postload879.i:                                    ; preds = %postload871.i.postload879.i_crit_edge, %preload878.i
  %loadedValue2750.i = load i1* %CastToValueType2754.i, align 1
  br i1 %loadedValue2750.i, label %preload886.i, label %postload879.i.postload887.i_crit_edge

postload879.i.postload887.i_crit_edge:            ; preds = %postload879.i
  br label %postload887.i

preload886.i:                                     ; preds = %postload879.i
  %"&(pSB[currWI].offset)2411.i" = add nuw i64 %CurrSBIndex..6.i, 1536
  %"&pSB[currWI].offset2412.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2411.i"
  %CastToValueType2413.i = bitcast i8* %"&pSB[currWI].offset2412.i" to float addrspace(3)**
  %loadedValue2414.i = load float addrspace(3)** %CastToValueType2413.i, align 8
  store float %extract376.i, float addrspace(3)* %loadedValue2414.i, align 4
  br label %postload887.i

postload887.i:                                    ; preds = %postload879.i.postload887.i_crit_edge, %preload886.i
  %loadedValue2769.i = load i1* %CastToValueType2773.i, align 1
  br i1 %loadedValue2769.i, label %preload894.i, label %postload887.i.postload895.i_crit_edge

postload887.i.postload895.i_crit_edge:            ; preds = %postload887.i
  br label %postload895.i

preload894.i:                                     ; preds = %postload887.i
  %"&(pSB[currWI].offset)2430.i" = add nuw i64 %CurrSBIndex..6.i, 1544
  %"&pSB[currWI].offset2431.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2430.i"
  %CastToValueType2432.i = bitcast i8* %"&pSB[currWI].offset2431.i" to float addrspace(3)**
  %loadedValue2433.i = load float addrspace(3)** %CastToValueType2432.i, align 8
  store float %extract377.i, float addrspace(3)* %loadedValue2433.i, align 4
  br label %postload895.i

postload895.i:                                    ; preds = %postload887.i.postload895.i_crit_edge, %preload894.i
  %loadedValue2788.i = load i1* %CastToValueType2792.i, align 1
  br i1 %loadedValue2788.i, label %preload902.i, label %postload895.i.postload903.i_crit_edge

postload895.i.postload903.i_crit_edge:            ; preds = %postload895.i
  br label %postload903.i

preload902.i:                                     ; preds = %postload895.i
  %"&(pSB[currWI].offset)2449.i" = add nuw i64 %CurrSBIndex..6.i, 1552
  %"&pSB[currWI].offset2450.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2449.i"
  %CastToValueType2451.i = bitcast i8* %"&pSB[currWI].offset2450.i" to float addrspace(3)**
  %loadedValue2452.i = load float addrspace(3)** %CastToValueType2451.i, align 8
  store float %extract378.i, float addrspace(3)* %loadedValue2452.i, align 4
  br label %postload903.i

postload903.i:                                    ; preds = %postload895.i.postload903.i_crit_edge, %preload902.i
  %loadedValue2807.i = load i1* %CastToValueType2811.i, align 1
  br i1 %loadedValue2807.i, label %preload910.i, label %postload903.i.postload911.i_crit_edge

postload903.i.postload911.i_crit_edge:            ; preds = %postload903.i
  br label %postload911.i

preload910.i:                                     ; preds = %postload903.i
  %"&(pSB[currWI].offset)2468.i" = add nuw i64 %CurrSBIndex..6.i, 1560
  %"&pSB[currWI].offset2469.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2468.i"
  %CastToValueType2470.i = bitcast i8* %"&pSB[currWI].offset2469.i" to float addrspace(3)**
  %loadedValue2471.i = load float addrspace(3)** %CastToValueType2470.i, align 8
  store float %extract379.i, float addrspace(3)* %loadedValue2471.i, align 4
  br label %postload911.i

postload911.i:                                    ; preds = %postload903.i.postload911.i_crit_edge, %preload910.i
  %loadedValue2826.i = load i1* %CastToValueType2830.i, align 1
  br i1 %loadedValue2826.i, label %preload918.i, label %postload911.i.postload919.i_crit_edge

postload911.i.postload919.i_crit_edge:            ; preds = %postload911.i
  br label %postload919.i

preload918.i:                                     ; preds = %postload911.i
  %"&(pSB[currWI].offset)2487.i" = add nuw i64 %CurrSBIndex..6.i, 1568
  %"&pSB[currWI].offset2488.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2487.i"
  %CastToValueType2489.i = bitcast i8* %"&pSB[currWI].offset2488.i" to float addrspace(3)**
  %loadedValue2490.i = load float addrspace(3)** %CastToValueType2489.i, align 8
  store float %extract380.i, float addrspace(3)* %loadedValue2490.i, align 4
  br label %postload919.i

postload919.i:                                    ; preds = %postload911.i.postload919.i_crit_edge, %preload918.i
  %loadedValue2845.i = load i1* %CastToValueType2849.i, align 1
  br i1 %loadedValue2845.i, label %preload926.i, label %postload919.i.postload927.i_crit_edge

postload919.i.postload927.i_crit_edge:            ; preds = %postload919.i
  br label %postload927.i

preload926.i:                                     ; preds = %postload919.i
  %"&(pSB[currWI].offset)2506.i" = add nuw i64 %CurrSBIndex..6.i, 1576
  %"&pSB[currWI].offset2507.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2506.i"
  %CastToValueType2508.i = bitcast i8* %"&pSB[currWI].offset2507.i" to float addrspace(3)**
  %loadedValue2509.i = load float addrspace(3)** %CastToValueType2508.i, align 8
  store float %extract381.i, float addrspace(3)* %loadedValue2509.i, align 4
  br label %postload927.i

postload927.i:                                    ; preds = %postload919.i.postload927.i_crit_edge, %preload926.i
  %loadedValue2864.i = load i1* %CastToValueType2868.i, align 1
  br i1 %loadedValue2864.i, label %preload934.i, label %postload927.i.postload935.i_crit_edge

postload927.i.postload935.i_crit_edge:            ; preds = %postload927.i
  br label %postload935.i

preload934.i:                                     ; preds = %postload927.i
  %"&(pSB[currWI].offset)2525.i" = add nuw i64 %CurrSBIndex..6.i, 1584
  %"&pSB[currWI].offset2526.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2525.i"
  %CastToValueType2527.i = bitcast i8* %"&pSB[currWI].offset2526.i" to float addrspace(3)**
  %loadedValue2528.i = load float addrspace(3)** %CastToValueType2527.i, align 8
  store float %extract382.i, float addrspace(3)* %loadedValue2528.i, align 4
  br label %postload935.i

postload935.i:                                    ; preds = %postload927.i.postload935.i_crit_edge, %preload934.i
  %loadedValue2883.i = load i1* %CastToValueType2887.i, align 1
  br i1 %loadedValue2883.i, label %preload942.i, label %postload935.i.postload943.i_crit_edge

postload935.i.postload943.i_crit_edge:            ; preds = %postload935.i
  br label %postload943.i

preload942.i:                                     ; preds = %postload935.i
  %"&(pSB[currWI].offset)2544.i" = add nuw i64 %CurrSBIndex..6.i, 1592
  %"&pSB[currWI].offset2545.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2544.i"
  %CastToValueType2546.i = bitcast i8* %"&pSB[currWI].offset2545.i" to float addrspace(3)**
  %loadedValue2547.i = load float addrspace(3)** %CastToValueType2546.i, align 8
  store float %extract383.i, float addrspace(3)* %loadedValue2547.i, align 4
  br label %postload943.i

postload943.i:                                    ; preds = %postload935.i.postload943.i_crit_edge, %preload942.i
  %loadedValue2598.i = load i1* %CastToValueType2602.i, align 1
  br i1 %loadedValue2598.i, label %preload1148.i, label %postload1149.i

preload1148.i:                                    ; preds = %postload943.i
  %check.WI.iter2964.i = icmp ult i64 %CurrWI..6.i, %28
  br i1 %check.WI.iter2964.i, label %thenBB2961.i, label %postload1149.i

thenBB2961.i:                                     ; preds = %preload1148.i
  %"CurrWI++2965.i" = add nuw i64 %CurrWI..6.i, 1
  %"loadedCurrSB+Stride2967.i" = add nuw i64 %CurrSBIndex..6.i, 1792
  switch i32 %currBarrier.6.i, label %SyncBB2941.i [
    i32 20, label %thenBB2961.i.postload968.i_crit_edge
    i32 24, label %SyncBB.i
    i32 23, label %thenBB2961.i.SyncBB2943.i_crit_edge
    i32 22, label %postload1149.i
  ]

thenBB2961.i.SyncBB2943.i_crit_edge:              ; preds = %thenBB2961.i
  br label %SyncBB2943.i

thenBB2961.i.postload968.i_crit_edge:             ; preds = %thenBB2961.i
  br label %postload968.i

postload1149.i:                                   ; preds = %thenBB.i, %thenBB2969.i, %thenBB2961.i, %preload1148.i, %postload943.i, %thenBB2953.i, %thenBB2945.i
  %currBarrier.8.i = phi i32 [ %currBarrier.6.i, %postload943.i ], [ %currBarrier.9.i, %thenBB2969.i ], [ %currBarrier.6.i, %thenBB2961.i ], [ %currBarrier.4.i, %thenBB2953.i ], [ %currBarrier.1.i, %thenBB2945.i ], [ %currBarrier.12.i, %thenBB.i ], [ 22, %preload1148.i ]
  %CurrSBIndex..8.i = phi i64 [ %CurrSBIndex..6.i, %postload943.i ], [ %"loadedCurrSB+Stride2975.i", %thenBB2969.i ], [ %"loadedCurrSB+Stride2967.i", %thenBB2961.i ], [ %"loadedCurrSB+Stride2959.i", %thenBB2953.i ], [ %"loadedCurrSB+Stride2951.i", %thenBB2945.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %preload1148.i ]
  %CurrWI..8.i = phi i64 [ %CurrWI..6.i, %postload943.i ], [ %"CurrWI++2973.i", %thenBB2969.i ], [ %"CurrWI++2965.i", %thenBB2961.i ], [ %"CurrWI++2957.i", %thenBB2953.i ], [ %"CurrWI++2949.i", %thenBB2945.i ], [ %"CurrWI++.i", %thenBB.i ], [ 0, %preload1148.i ]
  %"&(pSB[currWI].offset)2586.i" = add nuw i64 %CurrSBIndex..8.i, 1620
  %"&pSB[currWI].offset2587.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2586.i"
  %CastToValueType2588.i = bitcast i8* %"&pSB[currWI].offset2587.i" to i32*
  %loadedValue2589.i = load i32* %CastToValueType2588.i, align 4
  %mul.i.i = shl nsw i32 %loadedValue2589.i, 1
  %conv6.i.i = sext i32 %mul.i.i to i64
  %temp384.i = insertelement <16 x i64> undef, i64 %conv6.i.i, i32 0
  %vector385.i = shufflevector <16 x i64> %temp384.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)2231.i" = add nuw i64 %CurrSBIndex..8.i, 1280
  %"&pSB[currWI].offset2232.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2231.i"
  %CastToValueType2233.i = bitcast i8* %"&pSB[currWI].offset2232.i" to <16 x i64>*
  %loadedValue2234.i = load <16 x i64>* %CastToValueType2233.i, align 128
  %cmp.i.i = icmp ult <16 x i64> %vector385.i, %loadedValue2234.i
  %notCond386.i = xor <16 x i1> %cmp.i.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %"&(pSB[currWI].offset)2577.i" = add nuw i64 %CurrSBIndex..8.i, 1616
  %"&pSB[currWI].offset2578.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2577.i"
  %CastToValueType2579.i = bitcast i8* %"&pSB[currWI].offset2578.i" to <16 x i1>*
  %loadedValue2580.i = load <16 x i1>* %CastToValueType2579.i, align 16
  %who_left_tr387.i = and <16 x i1> %loadedValue2580.i, %notCond386.i
  %"&(pSB[currWI].offset)2563.i" = add nuw i64 %CurrSBIndex..8.i, 1600
  %"&pSB[currWI].offset2564.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2563.i"
  %CastToValueType2565.i = bitcast i8* %"&pSB[currWI].offset2564.i" to <16 x i1>*
  %loadedValue2566.i = load <16 x i1>* %CastToValueType2565.i, align 16
  %loop_mask15389.i = or <16 x i1> %loadedValue2566.i, %who_left_tr387.i
  %ipred.i1.i = bitcast <16 x i1> %loop_mask15389.i to i16
  %val.i2.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i1.i, i16 %ipred.i1.i) nounwind
  %204 = and i32 %val.i2.i, 1
  %res.i3.i = icmp eq i32 %204, 0
  %local_edge408.i = and <16 x i1> %loadedValue2580.i, %cmp.i.i
  br i1 %res.i3.i, label %for.body.i.i, label %scanLocalMem.exit.i

scanLocalMem.exit.i:                              ; preds = %postload1149.i, %postload968.i
  %currBarrier.9.i = phi i32 [ %currBarrier.3.i, %postload968.i ], [ %currBarrier.8.i, %postload1149.i ]
  %CurrSBIndex..9.i = phi i64 [ %CurrSBIndex..3.i, %postload968.i ], [ %CurrSBIndex..8.i, %postload1149.i ]
  %CurrWI..9.i = phi i64 [ %CurrWI..3.i, %postload968.i ], [ %CurrWI..8.i, %postload1149.i ]
  %"&(pSB[currWI].offset)2245.i" = add nuw i64 %CurrSBIndex..9.i, 1408
  %"&pSB[currWI].offset2246.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2245.i"
  %CastToValueType2247.i = bitcast i8* %"&pSB[currWI].offset2246.i" to <16 x i32>*
  %loadedValue2248.i = load <16 x i32>* %CastToValueType2247.i, align 64
  %sub14.i425.i = add nsw <16 x i32> %loadedValue2248.i, <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>
  %idxprom15.i426.i = sext <16 x i32> %sub14.i425.i to <16 x i64>
  %extract428.i = extractelement <16 x i64> %idxprom15.i426.i, i32 1
  %extract429.i = extractelement <16 x i64> %idxprom15.i426.i, i32 2
  %extract430.i = extractelement <16 x i64> %idxprom15.i426.i, i32 3
  %extract431.i = extractelement <16 x i64> %idxprom15.i426.i, i32 4
  %extract432.i = extractelement <16 x i64> %idxprom15.i426.i, i32 5
  %extract433.i = extractelement <16 x i64> %idxprom15.i426.i, i32 6
  %extract434.i = extractelement <16 x i64> %idxprom15.i426.i, i32 7
  %extract435.i = extractelement <16 x i64> %idxprom15.i426.i, i32 8
  %extract436.i = extractelement <16 x i64> %idxprom15.i426.i, i32 9
  %extract437.i = extractelement <16 x i64> %idxprom15.i426.i, i32 10
  %extract438.i = extractelement <16 x i64> %idxprom15.i426.i, i32 11
  %extract439.i = extractelement <16 x i64> %idxprom15.i426.i, i32 12
  %extract440.i = extractelement <16 x i64> %idxprom15.i426.i, i32 13
  %extract441.i = extractelement <16 x i64> %idxprom15.i426.i, i32 14
  %extract442.i = extractelement <16 x i64> %idxprom15.i426.i, i32 15
  %205 = getelementptr inbounds float addrspace(3)* %13, i64 %extract428.i
  %206 = getelementptr inbounds float addrspace(3)* %13, i64 %extract429.i
  %207 = getelementptr inbounds float addrspace(3)* %13, i64 %extract430.i
  %208 = getelementptr inbounds float addrspace(3)* %13, i64 %extract431.i
  %209 = getelementptr inbounds float addrspace(3)* %13, i64 %extract432.i
  %210 = getelementptr inbounds float addrspace(3)* %13, i64 %extract433.i
  %211 = getelementptr inbounds float addrspace(3)* %13, i64 %extract434.i
  %212 = getelementptr inbounds float addrspace(3)* %13, i64 %extract435.i
  %213 = getelementptr inbounds float addrspace(3)* %13, i64 %extract436.i
  %214 = getelementptr inbounds float addrspace(3)* %13, i64 %extract437.i
  %215 = getelementptr inbounds float addrspace(3)* %13, i64 %extract438.i
  %216 = getelementptr inbounds float addrspace(3)* %13, i64 %extract439.i
  %217 = getelementptr inbounds float addrspace(3)* %13, i64 %extract440.i
  %218 = getelementptr inbounds float addrspace(3)* %13, i64 %extract441.i
  %219 = getelementptr inbounds float addrspace(3)* %13, i64 %extract442.i
  %"&(pSB[currWI].offset)1811.i" = add nuw i64 %CurrSBIndex..9.i, 888
  %"&pSB[currWI].offset1812.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1811.i"
  %CastToValueType1813.i = bitcast i8* %"&pSB[currWI].offset1812.i" to i1*
  %loadedValue1814.i = load i1* %CastToValueType1813.i, align 1
  br i1 %loadedValue1814.i, label %preload969.i, label %postload970.i

preload969.i:                                     ; preds = %scanLocalMem.exit.i
  %extract427.i = extractelement <16 x i64> %idxprom15.i426.i, i32 0
  %220 = getelementptr inbounds float addrspace(3)* %13, i64 %extract427.i
  %masked_load735.i = load float addrspace(3)* %220, align 4
  br label %postload970.i

postload970.i:                                    ; preds = %preload969.i, %scanLocalMem.exit.i
  %phi971.i = phi float [ undef, %scanLocalMem.exit.i ], [ %masked_load735.i, %preload969.i ]
  %"&(pSB[currWI].offset)1840.i" = add nuw i64 %CurrSBIndex..9.i, 889
  %"&pSB[currWI].offset1841.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1840.i"
  %CastToValueType1842.i = bitcast i8* %"&pSB[currWI].offset1841.i" to i1*
  %loadedValue1843.i = load i1* %CastToValueType1842.i, align 1
  br i1 %loadedValue1843.i, label %preload980.i, label %postload981.i

preload980.i:                                     ; preds = %postload970.i
  %masked_load736.i = load float addrspace(3)* %205, align 4
  br label %postload981.i

postload981.i:                                    ; preds = %preload980.i, %postload970.i
  %phi982.i = phi float [ undef, %postload970.i ], [ %masked_load736.i, %preload980.i ]
  %"&(pSB[currWI].offset)1864.i" = add nuw i64 %CurrSBIndex..9.i, 890
  %"&pSB[currWI].offset1865.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1864.i"
  %CastToValueType1866.i = bitcast i8* %"&pSB[currWI].offset1865.i" to i1*
  %loadedValue1867.i = load i1* %CastToValueType1866.i, align 1
  br i1 %loadedValue1867.i, label %preload991.i, label %postload992.i

preload991.i:                                     ; preds = %postload981.i
  %masked_load737.i = load float addrspace(3)* %206, align 4
  br label %postload992.i

postload992.i:                                    ; preds = %preload991.i, %postload981.i
  %phi993.i = phi float [ undef, %postload981.i ], [ %masked_load737.i, %preload991.i ]
  %"&(pSB[currWI].offset)1888.i" = add nuw i64 %CurrSBIndex..9.i, 891
  %"&pSB[currWI].offset1889.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1888.i"
  %CastToValueType1890.i = bitcast i8* %"&pSB[currWI].offset1889.i" to i1*
  %loadedValue1891.i = load i1* %CastToValueType1890.i, align 1
  br i1 %loadedValue1891.i, label %preload1002.i, label %postload1003.i

preload1002.i:                                    ; preds = %postload992.i
  %masked_load738.i = load float addrspace(3)* %207, align 4
  br label %postload1003.i

postload1003.i:                                   ; preds = %preload1002.i, %postload992.i
  %phi1004.i = phi float [ undef, %postload992.i ], [ %masked_load738.i, %preload1002.i ]
  %"&(pSB[currWI].offset)1912.i" = add nuw i64 %CurrSBIndex..9.i, 892
  %"&pSB[currWI].offset1913.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1912.i"
  %CastToValueType1914.i = bitcast i8* %"&pSB[currWI].offset1913.i" to i1*
  %loadedValue1915.i = load i1* %CastToValueType1914.i, align 1
  br i1 %loadedValue1915.i, label %preload1013.i, label %postload1014.i

preload1013.i:                                    ; preds = %postload1003.i
  %masked_load739.i = load float addrspace(3)* %208, align 4
  br label %postload1014.i

postload1014.i:                                   ; preds = %preload1013.i, %postload1003.i
  %phi1015.i = phi float [ undef, %postload1003.i ], [ %masked_load739.i, %preload1013.i ]
  %"&(pSB[currWI].offset)1936.i" = add nuw i64 %CurrSBIndex..9.i, 893
  %"&pSB[currWI].offset1937.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1936.i"
  %CastToValueType1938.i = bitcast i8* %"&pSB[currWI].offset1937.i" to i1*
  %loadedValue1939.i = load i1* %CastToValueType1938.i, align 1
  br i1 %loadedValue1939.i, label %preload1024.i, label %postload1025.i

preload1024.i:                                    ; preds = %postload1014.i
  %masked_load740.i = load float addrspace(3)* %209, align 4
  br label %postload1025.i

postload1025.i:                                   ; preds = %preload1024.i, %postload1014.i
  %phi1026.i = phi float [ undef, %postload1014.i ], [ %masked_load740.i, %preload1024.i ]
  %"&(pSB[currWI].offset)1960.i" = add nuw i64 %CurrSBIndex..9.i, 894
  %"&pSB[currWI].offset1961.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1960.i"
  %CastToValueType1962.i = bitcast i8* %"&pSB[currWI].offset1961.i" to i1*
  %loadedValue1963.i = load i1* %CastToValueType1962.i, align 1
  br i1 %loadedValue1963.i, label %preload1035.i, label %postload1036.i

preload1035.i:                                    ; preds = %postload1025.i
  %masked_load741.i = load float addrspace(3)* %210, align 4
  br label %postload1036.i

postload1036.i:                                   ; preds = %preload1035.i, %postload1025.i
  %phi1037.i = phi float [ undef, %postload1025.i ], [ %masked_load741.i, %preload1035.i ]
  %"&(pSB[currWI].offset)1984.i" = add nuw i64 %CurrSBIndex..9.i, 895
  %"&pSB[currWI].offset1985.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1984.i"
  %CastToValueType1986.i = bitcast i8* %"&pSB[currWI].offset1985.i" to i1*
  %loadedValue1987.i = load i1* %CastToValueType1986.i, align 1
  br i1 %loadedValue1987.i, label %preload1046.i, label %postload1047.i

preload1046.i:                                    ; preds = %postload1036.i
  %masked_load742.i = load float addrspace(3)* %211, align 4
  br label %postload1047.i

postload1047.i:                                   ; preds = %preload1046.i, %postload1036.i
  %phi1048.i = phi float [ undef, %postload1036.i ], [ %masked_load742.i, %preload1046.i ]
  %"&(pSB[currWI].offset)2008.i" = add nuw i64 %CurrSBIndex..9.i, 896
  %"&pSB[currWI].offset2009.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2008.i"
  %CastToValueType2010.i = bitcast i8* %"&pSB[currWI].offset2009.i" to i1*
  %loadedValue2011.i = load i1* %CastToValueType2010.i, align 1
  br i1 %loadedValue2011.i, label %preload1057.i, label %postload1058.i

preload1057.i:                                    ; preds = %postload1047.i
  %masked_load743.i = load float addrspace(3)* %212, align 4
  br label %postload1058.i

postload1058.i:                                   ; preds = %preload1057.i, %postload1047.i
  %phi1059.i = phi float [ undef, %postload1047.i ], [ %masked_load743.i, %preload1057.i ]
  %"&(pSB[currWI].offset)2032.i" = add nuw i64 %CurrSBIndex..9.i, 897
  %"&pSB[currWI].offset2033.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2032.i"
  %CastToValueType2034.i = bitcast i8* %"&pSB[currWI].offset2033.i" to i1*
  %loadedValue2035.i = load i1* %CastToValueType2034.i, align 1
  br i1 %loadedValue2035.i, label %preload1068.i, label %postload1069.i

preload1068.i:                                    ; preds = %postload1058.i
  %masked_load744.i = load float addrspace(3)* %213, align 4
  br label %postload1069.i

postload1069.i:                                   ; preds = %preload1068.i, %postload1058.i
  %phi1070.i = phi float [ undef, %postload1058.i ], [ %masked_load744.i, %preload1068.i ]
  %"&(pSB[currWI].offset)2056.i" = add nuw i64 %CurrSBIndex..9.i, 898
  %"&pSB[currWI].offset2057.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2056.i"
  %CastToValueType2058.i = bitcast i8* %"&pSB[currWI].offset2057.i" to i1*
  %loadedValue2059.i = load i1* %CastToValueType2058.i, align 1
  br i1 %loadedValue2059.i, label %preload1079.i, label %postload1080.i

preload1079.i:                                    ; preds = %postload1069.i
  %masked_load745.i = load float addrspace(3)* %214, align 4
  br label %postload1080.i

postload1080.i:                                   ; preds = %preload1079.i, %postload1069.i
  %phi1081.i = phi float [ undef, %postload1069.i ], [ %masked_load745.i, %preload1079.i ]
  %"&(pSB[currWI].offset)2080.i" = add nuw i64 %CurrSBIndex..9.i, 899
  %"&pSB[currWI].offset2081.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2080.i"
  %CastToValueType2082.i = bitcast i8* %"&pSB[currWI].offset2081.i" to i1*
  %loadedValue2083.i = load i1* %CastToValueType2082.i, align 1
  br i1 %loadedValue2083.i, label %preload1090.i, label %postload1091.i

preload1090.i:                                    ; preds = %postload1080.i
  %masked_load746.i = load float addrspace(3)* %215, align 4
  br label %postload1091.i

postload1091.i:                                   ; preds = %preload1090.i, %postload1080.i
  %phi1092.i = phi float [ undef, %postload1080.i ], [ %masked_load746.i, %preload1090.i ]
  %"&(pSB[currWI].offset)2104.i" = add nuw i64 %CurrSBIndex..9.i, 900
  %"&pSB[currWI].offset2105.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2104.i"
  %CastToValueType2106.i = bitcast i8* %"&pSB[currWI].offset2105.i" to i1*
  %loadedValue2107.i = load i1* %CastToValueType2106.i, align 1
  br i1 %loadedValue2107.i, label %preload1101.i, label %postload1102.i

preload1101.i:                                    ; preds = %postload1091.i
  %masked_load747.i = load float addrspace(3)* %216, align 4
  br label %postload1102.i

postload1102.i:                                   ; preds = %preload1101.i, %postload1091.i
  %phi1103.i = phi float [ undef, %postload1091.i ], [ %masked_load747.i, %preload1101.i ]
  %"&(pSB[currWI].offset)2128.i" = add nuw i64 %CurrSBIndex..9.i, 901
  %"&pSB[currWI].offset2129.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2128.i"
  %CastToValueType2130.i = bitcast i8* %"&pSB[currWI].offset2129.i" to i1*
  %loadedValue2131.i = load i1* %CastToValueType2130.i, align 1
  br i1 %loadedValue2131.i, label %preload1112.i, label %postload1113.i

preload1112.i:                                    ; preds = %postload1102.i
  %masked_load748.i = load float addrspace(3)* %217, align 4
  br label %postload1113.i

postload1113.i:                                   ; preds = %preload1112.i, %postload1102.i
  %phi1114.i = phi float [ undef, %postload1102.i ], [ %masked_load748.i, %preload1112.i ]
  %"&(pSB[currWI].offset)2152.i" = add nuw i64 %CurrSBIndex..9.i, 902
  %"&pSB[currWI].offset2153.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2152.i"
  %CastToValueType2154.i = bitcast i8* %"&pSB[currWI].offset2153.i" to i1*
  %loadedValue2155.i = load i1* %CastToValueType2154.i, align 1
  br i1 %loadedValue2155.i, label %preload1123.i, label %postload1124.i

preload1123.i:                                    ; preds = %postload1113.i
  %masked_load749.i = load float addrspace(3)* %218, align 4
  br label %postload1124.i

postload1124.i:                                   ; preds = %preload1123.i, %postload1113.i
  %phi1125.i = phi float [ undef, %postload1113.i ], [ %masked_load749.i, %preload1123.i ]
  %"&(pSB[currWI].offset)2176.i" = add nuw i64 %CurrSBIndex..9.i, 903
  %"&pSB[currWI].offset2177.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2176.i"
  %CastToValueType2178.i = bitcast i8* %"&pSB[currWI].offset2177.i" to i1*
  %loadedValue2179.i = load i1* %CastToValueType2178.i, align 1
  br i1 %loadedValue2179.i, label %preload1134.i, label %postload1135.i

preload1134.i:                                    ; preds = %postload1124.i
  %masked_load750.i = load float addrspace(3)* %219, align 4
  br label %postload1135.i

postload1135.i:                                   ; preds = %preload1134.i, %postload1124.i
  %phi1136.i = phi float [ undef, %postload1124.i ], [ %masked_load750.i, %preload1134.i ]
  %temp.vect443.i = insertelement <16 x float> undef, float %phi971.i, i32 0
  %temp.vect444.i = insertelement <16 x float> %temp.vect443.i, float %phi982.i, i32 1
  %temp.vect445.i = insertelement <16 x float> %temp.vect444.i, float %phi993.i, i32 2
  %temp.vect446.i = insertelement <16 x float> %temp.vect445.i, float %phi1004.i, i32 3
  %temp.vect447.i = insertelement <16 x float> %temp.vect446.i, float %phi1015.i, i32 4
  %temp.vect448.i = insertelement <16 x float> %temp.vect447.i, float %phi1026.i, i32 5
  %temp.vect449.i = insertelement <16 x float> %temp.vect448.i, float %phi1037.i, i32 6
  %temp.vect450.i = insertelement <16 x float> %temp.vect449.i, float %phi1048.i, i32 7
  %temp.vect451.i = insertelement <16 x float> %temp.vect450.i, float %phi1059.i, i32 8
  %temp.vect452.i = insertelement <16 x float> %temp.vect451.i, float %phi1070.i, i32 9
  %temp.vect453.i = insertelement <16 x float> %temp.vect452.i, float %phi1081.i, i32 10
  %temp.vect454.i = insertelement <16 x float> %temp.vect453.i, float %phi1092.i, i32 11
  %temp.vect455.i = insertelement <16 x float> %temp.vect454.i, float %phi1103.i, i32 12
  %temp.vect456.i = insertelement <16 x float> %temp.vect455.i, float %phi1114.i, i32 13
  %temp.vect457.i = insertelement <16 x float> %temp.vect456.i, float %phi1125.i, i32 14
  %temp.vect458.i = insertelement <16 x float> %temp.vect457.i, float %phi1136.i, i32 15
  %"&(pSB[currWI].offset)1415.i" = add nuw i64 %CurrSBIndex..9.i, 448
  %"&pSB[currWI].offset1416.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1415.i"
  %CastToValueType1417.i = bitcast i8* %"&pSB[currWI].offset1416.i" to <16 x float>*
  %loadedValue1418.i = load <16 x float>* %CastToValueType1417.i, align 64
  %add29459.i = fadd <16 x float> %temp.vect458.i, %loadedValue1418.i
  %"&(pSB[currWI].offset)2222.i" = add nuw i64 %CurrSBIndex..9.i, 1152
  %"&pSB[currWI].offset2223.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2222.i"
  %CastToValueType2224.i = bitcast i8* %"&pSB[currWI].offset2223.i" to <16 x float>*
  %loadedValue2225.i = load <16 x float>* %CastToValueType2224.i, align 64
  %add30460.i = fadd <16 x float> %loadedValue2225.i, %add29459.i
  %extract513.i = extractelement <16 x float> %add30460.i, i32 0
  %extract514.i = extractelement <16 x float> %add30460.i, i32 1
  %extract515.i = extractelement <16 x float> %add30460.i, i32 2
  %extract516.i = extractelement <16 x float> %add30460.i, i32 3
  %extract517.i = extractelement <16 x float> %add30460.i, i32 4
  %extract518.i = extractelement <16 x float> %add30460.i, i32 5
  %extract519.i = extractelement <16 x float> %add30460.i, i32 6
  %extract520.i = extractelement <16 x float> %add30460.i, i32 7
  %extract521.i = extractelement <16 x float> %add30460.i, i32 8
  %extract522.i = extractelement <16 x float> %add30460.i, i32 9
  %extract523.i = extractelement <16 x float> %add30460.i, i32 10
  %extract524.i = extractelement <16 x float> %add30460.i, i32 11
  %extract525.i = extractelement <16 x float> %add30460.i, i32 12
  %extract526.i = extractelement <16 x float> %add30460.i, i32 13
  %extract527.i = extractelement <16 x float> %add30460.i, i32 14
  %extract528.i = extractelement <16 x float> %add30460.i, i32 15
  %"&(pSB[currWI].offset)2195.i" = add nuw i64 %CurrSBIndex..9.i, 960
  %"&pSB[currWI].offset2196.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2195.i"
  %CastToValueType2197.i = bitcast i8* %"&pSB[currWI].offset2196.i" to <16 x float>*
  %loadedValue2198.i = load <16 x float>* %CastToValueType2197.i, align 64
  %add24462.i = fadd <16 x float> %loadedValue2198.i, %add29459.i
  %extract466.i = extractelement <16 x float> %add24462.i, i32 1
  %extract467.i = extractelement <16 x float> %add24462.i, i32 2
  %extract468.i = extractelement <16 x float> %add24462.i, i32 3
  %extract469.i = extractelement <16 x float> %add24462.i, i32 4
  %extract470.i = extractelement <16 x float> %add24462.i, i32 5
  %extract471.i = extractelement <16 x float> %add24462.i, i32 6
  %extract472.i = extractelement <16 x float> %add24462.i, i32 7
  %extract473.i = extractelement <16 x float> %add24462.i, i32 8
  %extract474.i = extractelement <16 x float> %add24462.i, i32 9
  %extract475.i = extractelement <16 x float> %add24462.i, i32 10
  %extract476.i = extractelement <16 x float> %add24462.i, i32 11
  %extract477.i = extractelement <16 x float> %add24462.i, i32 12
  %extract478.i = extractelement <16 x float> %add24462.i, i32 13
  %extract479.i = extractelement <16 x float> %add24462.i, i32 14
  %extract480.i = extractelement <16 x float> %add24462.i, i32 15
  %"&(pSB[currWI].offset)2204.i" = add nuw i64 %CurrSBIndex..9.i, 1024
  %"&pSB[currWI].offset2205.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2204.i"
  %CastToValueType2206.i = bitcast i8* %"&pSB[currWI].offset2205.i" to <16 x float>*
  %loadedValue2207.i = load <16 x float>* %CastToValueType2206.i, align 64
  %add26463.i = fadd <16 x float> %loadedValue2207.i, %add29459.i
  %extract482.i = extractelement <16 x float> %add26463.i, i32 1
  %extract483.i = extractelement <16 x float> %add26463.i, i32 2
  %extract484.i = extractelement <16 x float> %add26463.i, i32 3
  %extract485.i = extractelement <16 x float> %add26463.i, i32 4
  %extract486.i = extractelement <16 x float> %add26463.i, i32 5
  %extract487.i = extractelement <16 x float> %add26463.i, i32 6
  %extract488.i = extractelement <16 x float> %add26463.i, i32 7
  %extract489.i = extractelement <16 x float> %add26463.i, i32 8
  %extract490.i = extractelement <16 x float> %add26463.i, i32 9
  %extract491.i = extractelement <16 x float> %add26463.i, i32 10
  %extract492.i = extractelement <16 x float> %add26463.i, i32 11
  %extract493.i = extractelement <16 x float> %add26463.i, i32 12
  %extract494.i = extractelement <16 x float> %add26463.i, i32 13
  %extract495.i = extractelement <16 x float> %add26463.i, i32 14
  %extract496.i = extractelement <16 x float> %add26463.i, i32 15
  %"&(pSB[currWI].offset)2213.i" = add nuw i64 %CurrSBIndex..9.i, 1088
  %"&pSB[currWI].offset2214.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2213.i"
  %CastToValueType2215.i = bitcast i8* %"&pSB[currWI].offset2214.i" to <16 x float>*
  %loadedValue2216.i = load <16 x float>* %CastToValueType2215.i, align 64
  %add28464.i = fadd <16 x float> %loadedValue2216.i, %add29459.i
  %extract498.i = extractelement <16 x float> %add28464.i, i32 1
  %extract499.i = extractelement <16 x float> %add28464.i, i32 2
  %extract500.i = extractelement <16 x float> %add28464.i, i32 3
  %extract501.i = extractelement <16 x float> %add28464.i, i32 4
  %extract502.i = extractelement <16 x float> %add28464.i, i32 5
  %extract503.i = extractelement <16 x float> %add28464.i, i32 6
  %extract504.i = extractelement <16 x float> %add28464.i, i32 7
  %extract505.i = extractelement <16 x float> %add28464.i, i32 8
  %extract506.i = extractelement <16 x float> %add28464.i, i32 9
  %extract507.i = extractelement <16 x float> %add28464.i, i32 10
  %extract508.i = extractelement <16 x float> %add28464.i, i32 11
  %extract509.i = extractelement <16 x float> %add28464.i, i32 12
  %extract510.i = extractelement <16 x float> %add28464.i, i32 13
  %extract511.i = extractelement <16 x float> %add28464.i, i32 14
  %extract512.i = extractelement <16 x float> %add28464.i, i32 15
  %221 = insertelement <4 x float> undef, float %extract466.i, i32 0
  %222 = insertelement <4 x float> undef, float %extract467.i, i32 0
  %223 = insertelement <4 x float> undef, float %extract468.i, i32 0
  %224 = insertelement <4 x float> undef, float %extract469.i, i32 0
  %225 = insertelement <4 x float> undef, float %extract470.i, i32 0
  %226 = insertelement <4 x float> undef, float %extract471.i, i32 0
  %227 = insertelement <4 x float> undef, float %extract472.i, i32 0
  %228 = insertelement <4 x float> undef, float %extract473.i, i32 0
  %229 = insertelement <4 x float> undef, float %extract474.i, i32 0
  %230 = insertelement <4 x float> undef, float %extract475.i, i32 0
  %231 = insertelement <4 x float> undef, float %extract476.i, i32 0
  %232 = insertelement <4 x float> undef, float %extract477.i, i32 0
  %233 = insertelement <4 x float> undef, float %extract478.i, i32 0
  %234 = insertelement <4 x float> undef, float %extract479.i, i32 0
  %235 = insertelement <4 x float> undef, float %extract480.i, i32 0
  %236 = insertelement <4 x float> %221, float %extract482.i, i32 1
  %237 = insertelement <4 x float> %222, float %extract483.i, i32 1
  %238 = insertelement <4 x float> %223, float %extract484.i, i32 1
  %239 = insertelement <4 x float> %224, float %extract485.i, i32 1
  %240 = insertelement <4 x float> %225, float %extract486.i, i32 1
  %241 = insertelement <4 x float> %226, float %extract487.i, i32 1
  %242 = insertelement <4 x float> %227, float %extract488.i, i32 1
  %243 = insertelement <4 x float> %228, float %extract489.i, i32 1
  %244 = insertelement <4 x float> %229, float %extract490.i, i32 1
  %245 = insertelement <4 x float> %230, float %extract491.i, i32 1
  %246 = insertelement <4 x float> %231, float %extract492.i, i32 1
  %247 = insertelement <4 x float> %232, float %extract493.i, i32 1
  %248 = insertelement <4 x float> %233, float %extract494.i, i32 1
  %249 = insertelement <4 x float> %234, float %extract495.i, i32 1
  %250 = insertelement <4 x float> %235, float %extract496.i, i32 1
  %251 = insertelement <4 x float> %236, float %extract498.i, i32 2
  %252 = insertelement <4 x float> %237, float %extract499.i, i32 2
  %253 = insertelement <4 x float> %238, float %extract500.i, i32 2
  %254 = insertelement <4 x float> %239, float %extract501.i, i32 2
  %255 = insertelement <4 x float> %240, float %extract502.i, i32 2
  %256 = insertelement <4 x float> %241, float %extract503.i, i32 2
  %257 = insertelement <4 x float> %242, float %extract504.i, i32 2
  %258 = insertelement <4 x float> %243, float %extract505.i, i32 2
  %259 = insertelement <4 x float> %244, float %extract506.i, i32 2
  %260 = insertelement <4 x float> %245, float %extract507.i, i32 2
  %261 = insertelement <4 x float> %246, float %extract508.i, i32 2
  %262 = insertelement <4 x float> %247, float %extract509.i, i32 2
  %263 = insertelement <4 x float> %248, float %extract510.i, i32 2
  %264 = insertelement <4 x float> %249, float %extract511.i, i32 2
  %265 = insertelement <4 x float> %250, float %extract512.i, i32 2
  %266 = insertelement <4 x float> %251, float %extract514.i, i32 3
  %267 = insertelement <4 x float> %252, float %extract515.i, i32 3
  %268 = insertelement <4 x float> %253, float %extract516.i, i32 3
  %269 = insertelement <4 x float> %254, float %extract517.i, i32 3
  %270 = insertelement <4 x float> %255, float %extract518.i, i32 3
  %271 = insertelement <4 x float> %256, float %extract519.i, i32 3
  %272 = insertelement <4 x float> %257, float %extract520.i, i32 3
  %273 = insertelement <4 x float> %258, float %extract521.i, i32 3
  %274 = insertelement <4 x float> %259, float %extract522.i, i32 3
  %275 = insertelement <4 x float> %260, float %extract523.i, i32 3
  %276 = insertelement <4 x float> %261, float %extract524.i, i32 3
  %277 = insertelement <4 x float> %262, float %extract525.i, i32 3
  %278 = insertelement <4 x float> %263, float %extract526.i, i32 3
  %279 = insertelement <4 x float> %264, float %extract527.i, i32 3
  %280 = insertelement <4 x float> %265, float %extract528.i, i32 3
  %"&(pSB[currWI].offset)1671.i" = add nuw i64 %CurrSBIndex..9.i, 768
  %"&pSB[currWI].offset1672.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1671.i"
  %CastToValueType1673.i = bitcast i8* %"&pSB[currWI].offset1672.i" to i64*
  %loadedValue1674.i = load i64* %CastToValueType1673.i, align 8
  %281 = getelementptr inbounds <4 x float> addrspace(1)* %34, i64 %loadedValue1674.i
  %"&(pSB[currWI].offset)1680.i" = add nuw i64 %CurrSBIndex..9.i, 776
  %"&pSB[currWI].offset1681.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1680.i"
  %CastToValueType1682.i = bitcast i8* %"&pSB[currWI].offset1681.i" to i64*
  %loadedValue1683.i = load i64* %CastToValueType1682.i, align 8
  %282 = getelementptr inbounds <4 x float> addrspace(1)* %34, i64 %loadedValue1683.i
  %"&(pSB[currWI].offset)1689.i" = add nuw i64 %CurrSBIndex..9.i, 784
  %"&pSB[currWI].offset1690.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1689.i"
  %CastToValueType1691.i = bitcast i8* %"&pSB[currWI].offset1690.i" to i64*
  %loadedValue1692.i = load i64* %CastToValueType1691.i, align 8
  %283 = getelementptr inbounds <4 x float> addrspace(1)* %34, i64 %loadedValue1692.i
  %"&(pSB[currWI].offset)1698.i" = add nuw i64 %CurrSBIndex..9.i, 792
  %"&pSB[currWI].offset1699.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1698.i"
  %CastToValueType1700.i = bitcast i8* %"&pSB[currWI].offset1699.i" to i64*
  %loadedValue1701.i = load i64* %CastToValueType1700.i, align 8
  %284 = getelementptr inbounds <4 x float> addrspace(1)* %34, i64 %loadedValue1701.i
  %"&(pSB[currWI].offset)1707.i" = add nuw i64 %CurrSBIndex..9.i, 800
  %"&pSB[currWI].offset1708.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1707.i"
  %CastToValueType1709.i = bitcast i8* %"&pSB[currWI].offset1708.i" to i64*
  %loadedValue1710.i = load i64* %CastToValueType1709.i, align 8
  %285 = getelementptr inbounds <4 x float> addrspace(1)* %34, i64 %loadedValue1710.i
  %"&(pSB[currWI].offset)1716.i" = add nuw i64 %CurrSBIndex..9.i, 808
  %"&pSB[currWI].offset1717.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1716.i"
  %CastToValueType1718.i = bitcast i8* %"&pSB[currWI].offset1717.i" to i64*
  %loadedValue1719.i = load i64* %CastToValueType1718.i, align 8
  %286 = getelementptr inbounds <4 x float> addrspace(1)* %34, i64 %loadedValue1719.i
  %"&(pSB[currWI].offset)1725.i" = add nuw i64 %CurrSBIndex..9.i, 816
  %"&pSB[currWI].offset1726.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1725.i"
  %CastToValueType1727.i = bitcast i8* %"&pSB[currWI].offset1726.i" to i64*
  %loadedValue1728.i = load i64* %CastToValueType1727.i, align 8
  %287 = getelementptr inbounds <4 x float> addrspace(1)* %34, i64 %loadedValue1728.i
  %"&(pSB[currWI].offset)1734.i" = add nuw i64 %CurrSBIndex..9.i, 824
  %"&pSB[currWI].offset1735.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1734.i"
  %CastToValueType1736.i = bitcast i8* %"&pSB[currWI].offset1735.i" to i64*
  %loadedValue1737.i = load i64* %CastToValueType1736.i, align 8
  %288 = getelementptr inbounds <4 x float> addrspace(1)* %34, i64 %loadedValue1737.i
  %"&(pSB[currWI].offset)1743.i" = add nuw i64 %CurrSBIndex..9.i, 832
  %"&pSB[currWI].offset1744.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1743.i"
  %CastToValueType1745.i = bitcast i8* %"&pSB[currWI].offset1744.i" to i64*
  %loadedValue1746.i = load i64* %CastToValueType1745.i, align 8
  %289 = getelementptr inbounds <4 x float> addrspace(1)* %34, i64 %loadedValue1746.i
  %"&(pSB[currWI].offset)1752.i" = add nuw i64 %CurrSBIndex..9.i, 840
  %"&pSB[currWI].offset1753.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1752.i"
  %CastToValueType1754.i = bitcast i8* %"&pSB[currWI].offset1753.i" to i64*
  %loadedValue1755.i = load i64* %CastToValueType1754.i, align 8
  %290 = getelementptr inbounds <4 x float> addrspace(1)* %34, i64 %loadedValue1755.i
  %"&(pSB[currWI].offset)1761.i" = add nuw i64 %CurrSBIndex..9.i, 848
  %"&pSB[currWI].offset1762.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1761.i"
  %CastToValueType1763.i = bitcast i8* %"&pSB[currWI].offset1762.i" to i64*
  %loadedValue1764.i = load i64* %CastToValueType1763.i, align 8
  %291 = getelementptr inbounds <4 x float> addrspace(1)* %34, i64 %loadedValue1764.i
  %"&(pSB[currWI].offset)1770.i" = add nuw i64 %CurrSBIndex..9.i, 856
  %"&pSB[currWI].offset1771.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1770.i"
  %CastToValueType1772.i = bitcast i8* %"&pSB[currWI].offset1771.i" to i64*
  %loadedValue1773.i = load i64* %CastToValueType1772.i, align 8
  %292 = getelementptr inbounds <4 x float> addrspace(1)* %34, i64 %loadedValue1773.i
  %"&(pSB[currWI].offset)1779.i" = add nuw i64 %CurrSBIndex..9.i, 864
  %"&pSB[currWI].offset1780.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1779.i"
  %CastToValueType1781.i = bitcast i8* %"&pSB[currWI].offset1780.i" to i64*
  %loadedValue1782.i = load i64* %CastToValueType1781.i, align 8
  %293 = getelementptr inbounds <4 x float> addrspace(1)* %34, i64 %loadedValue1782.i
  %"&(pSB[currWI].offset)1788.i" = add nuw i64 %CurrSBIndex..9.i, 872
  %"&pSB[currWI].offset1789.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1788.i"
  %CastToValueType1790.i = bitcast i8* %"&pSB[currWI].offset1789.i" to i64*
  %loadedValue1791.i = load i64* %CastToValueType1790.i, align 8
  %294 = getelementptr inbounds <4 x float> addrspace(1)* %34, i64 %loadedValue1791.i
  %"&(pSB[currWI].offset)1797.i" = add nuw i64 %CurrSBIndex..9.i, 880
  %"&pSB[currWI].offset1798.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1797.i"
  %CastToValueType1799.i = bitcast i8* %"&pSB[currWI].offset1798.i" to i64*
  %loadedValue1800.i = load i64* %CastToValueType1799.i, align 8
  %295 = getelementptr inbounds <4 x float> addrspace(1)* %34, i64 %loadedValue1800.i
  %"&(pSB[currWI].offset)1643.i" = add nuw i64 %CurrSBIndex..9.i, 531
  %"&pSB[currWI].offset1644.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1643.i"
  %CastToValueType1645.i = bitcast i8* %"&pSB[currWI].offset1644.i" to i1*
  %loadedValue1646.i = load i1* %CastToValueType1645.i, align 1
  br i1 %loadedValue1646.i, label %preload1174.i, label %postload1175.i

preload1174.i:                                    ; preds = %postload1135.i
  %extract465.i = extractelement <16 x float> %add24462.i, i32 0
  %296 = insertelement <4 x float> undef, float %extract465.i, i32 0
  %extract481.i = extractelement <16 x float> %add26463.i, i32 0
  %297 = insertelement <4 x float> %296, float %extract481.i, i32 1
  %extract497.i = extractelement <16 x float> %add28464.i, i32 0
  %"&(pSB[currWI].offset)1657.i" = add nuw i64 %CurrSBIndex..9.i, 640
  %"&pSB[currWI].offset1658.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1657.i"
  %CastToValueType1659.i = bitcast i8* %"&pSB[currWI].offset1658.i" to <16 x i64>*
  %loadedValue1660.i = load <16 x i64>* %CastToValueType1659.i, align 128
  %extract530.i = extractelement <16 x i64> %loadedValue1660.i, i32 0
  %298 = insertelement <4 x float> %297, float %extract497.i, i32 2
  %299 = getelementptr inbounds <4 x float> addrspace(1)* %34, i64 %extract530.i
  %300 = insertelement <4 x float> %298, float %extract513.i, i32 3
  store <4 x float> %300, <4 x float> addrspace(1)* %299, align 16
  br label %postload1175.i

postload1175.i:                                   ; preds = %preload1174.i, %postload1135.i
  %"&(pSB[currWI].offset)1433.i" = add nuw i64 %CurrSBIndex..9.i, 516
  %"&pSB[currWI].offset1434.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1433.i"
  %CastToValueType1435.i = bitcast i8* %"&pSB[currWI].offset1434.i" to i1*
  %loadedValue1436.i = load i1* %CastToValueType1435.i, align 1
  br i1 %loadedValue1436.i, label %preload1176.i, label %postload1177.i

preload1176.i:                                    ; preds = %postload1175.i
  store <4 x float> %266, <4 x float> addrspace(1)* %281, align 16
  br label %postload1177.i

postload1177.i:                                   ; preds = %preload1176.i, %postload1175.i
  %"&(pSB[currWI].offset)1447.i" = add nuw i64 %CurrSBIndex..9.i, 517
  %"&pSB[currWI].offset1448.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1447.i"
  %CastToValueType1449.i = bitcast i8* %"&pSB[currWI].offset1448.i" to i1*
  %loadedValue1450.i = load i1* %CastToValueType1449.i, align 1
  br i1 %loadedValue1450.i, label %preload1178.i, label %postload1179.i

preload1178.i:                                    ; preds = %postload1177.i
  store <4 x float> %267, <4 x float> addrspace(1)* %282, align 16
  br label %postload1179.i

postload1179.i:                                   ; preds = %preload1178.i, %postload1177.i
  %"&(pSB[currWI].offset)1461.i" = add nuw i64 %CurrSBIndex..9.i, 518
  %"&pSB[currWI].offset1462.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1461.i"
  %CastToValueType1463.i = bitcast i8* %"&pSB[currWI].offset1462.i" to i1*
  %loadedValue1464.i = load i1* %CastToValueType1463.i, align 1
  br i1 %loadedValue1464.i, label %preload1180.i, label %postload1181.i

preload1180.i:                                    ; preds = %postload1179.i
  store <4 x float> %268, <4 x float> addrspace(1)* %283, align 16
  br label %postload1181.i

postload1181.i:                                   ; preds = %preload1180.i, %postload1179.i
  %"&(pSB[currWI].offset)1475.i" = add nuw i64 %CurrSBIndex..9.i, 519
  %"&pSB[currWI].offset1476.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1475.i"
  %CastToValueType1477.i = bitcast i8* %"&pSB[currWI].offset1476.i" to i1*
  %loadedValue1478.i = load i1* %CastToValueType1477.i, align 1
  br i1 %loadedValue1478.i, label %preload1182.i, label %postload1183.i

preload1182.i:                                    ; preds = %postload1181.i
  store <4 x float> %269, <4 x float> addrspace(1)* %284, align 16
  br label %postload1183.i

postload1183.i:                                   ; preds = %preload1182.i, %postload1181.i
  %"&(pSB[currWI].offset)1489.i" = add nuw i64 %CurrSBIndex..9.i, 520
  %"&pSB[currWI].offset1490.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1489.i"
  %CastToValueType1491.i = bitcast i8* %"&pSB[currWI].offset1490.i" to i1*
  %loadedValue1492.i = load i1* %CastToValueType1491.i, align 1
  br i1 %loadedValue1492.i, label %preload1184.i, label %postload1185.i

preload1184.i:                                    ; preds = %postload1183.i
  store <4 x float> %270, <4 x float> addrspace(1)* %285, align 16
  br label %postload1185.i

postload1185.i:                                   ; preds = %preload1184.i, %postload1183.i
  %"&(pSB[currWI].offset)1503.i" = add nuw i64 %CurrSBIndex..9.i, 521
  %"&pSB[currWI].offset1504.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1503.i"
  %CastToValueType1505.i = bitcast i8* %"&pSB[currWI].offset1504.i" to i1*
  %loadedValue1506.i = load i1* %CastToValueType1505.i, align 1
  br i1 %loadedValue1506.i, label %preload1186.i, label %postload1187.i

preload1186.i:                                    ; preds = %postload1185.i
  store <4 x float> %271, <4 x float> addrspace(1)* %286, align 16
  br label %postload1187.i

postload1187.i:                                   ; preds = %preload1186.i, %postload1185.i
  %"&(pSB[currWI].offset)1517.i" = add nuw i64 %CurrSBIndex..9.i, 522
  %"&pSB[currWI].offset1518.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1517.i"
  %CastToValueType1519.i = bitcast i8* %"&pSB[currWI].offset1518.i" to i1*
  %loadedValue1520.i = load i1* %CastToValueType1519.i, align 1
  br i1 %loadedValue1520.i, label %preload1188.i, label %postload1189.i

preload1188.i:                                    ; preds = %postload1187.i
  store <4 x float> %272, <4 x float> addrspace(1)* %287, align 16
  br label %postload1189.i

postload1189.i:                                   ; preds = %preload1188.i, %postload1187.i
  %"&(pSB[currWI].offset)1531.i" = add nuw i64 %CurrSBIndex..9.i, 523
  %"&pSB[currWI].offset1532.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1531.i"
  %CastToValueType1533.i = bitcast i8* %"&pSB[currWI].offset1532.i" to i1*
  %loadedValue1534.i = load i1* %CastToValueType1533.i, align 1
  br i1 %loadedValue1534.i, label %preload1190.i, label %postload1191.i

preload1190.i:                                    ; preds = %postload1189.i
  store <4 x float> %273, <4 x float> addrspace(1)* %288, align 16
  br label %postload1191.i

postload1191.i:                                   ; preds = %preload1190.i, %postload1189.i
  %"&(pSB[currWI].offset)1545.i" = add nuw i64 %CurrSBIndex..9.i, 524
  %"&pSB[currWI].offset1546.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1545.i"
  %CastToValueType1547.i = bitcast i8* %"&pSB[currWI].offset1546.i" to i1*
  %loadedValue1548.i = load i1* %CastToValueType1547.i, align 1
  br i1 %loadedValue1548.i, label %preload1192.i, label %postload1193.i

preload1192.i:                                    ; preds = %postload1191.i
  store <4 x float> %274, <4 x float> addrspace(1)* %289, align 16
  br label %postload1193.i

postload1193.i:                                   ; preds = %preload1192.i, %postload1191.i
  %"&(pSB[currWI].offset)1559.i" = add nuw i64 %CurrSBIndex..9.i, 525
  %"&pSB[currWI].offset1560.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1559.i"
  %CastToValueType1561.i = bitcast i8* %"&pSB[currWI].offset1560.i" to i1*
  %loadedValue1562.i = load i1* %CastToValueType1561.i, align 1
  br i1 %loadedValue1562.i, label %preload1194.i, label %postload1195.i

preload1194.i:                                    ; preds = %postload1193.i
  store <4 x float> %275, <4 x float> addrspace(1)* %290, align 16
  br label %postload1195.i

postload1195.i:                                   ; preds = %preload1194.i, %postload1193.i
  %"&(pSB[currWI].offset)1573.i" = add nuw i64 %CurrSBIndex..9.i, 526
  %"&pSB[currWI].offset1574.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1573.i"
  %CastToValueType1575.i = bitcast i8* %"&pSB[currWI].offset1574.i" to i1*
  %loadedValue1576.i = load i1* %CastToValueType1575.i, align 1
  br i1 %loadedValue1576.i, label %preload1196.i, label %postload1197.i

preload1196.i:                                    ; preds = %postload1195.i
  store <4 x float> %276, <4 x float> addrspace(1)* %291, align 16
  br label %postload1197.i

postload1197.i:                                   ; preds = %preload1196.i, %postload1195.i
  %"&(pSB[currWI].offset)1587.i" = add nuw i64 %CurrSBIndex..9.i, 527
  %"&pSB[currWI].offset1588.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1587.i"
  %CastToValueType1589.i = bitcast i8* %"&pSB[currWI].offset1588.i" to i1*
  %loadedValue1590.i = load i1* %CastToValueType1589.i, align 1
  br i1 %loadedValue1590.i, label %preload1198.i, label %postload1199.i

preload1198.i:                                    ; preds = %postload1197.i
  store <4 x float> %277, <4 x float> addrspace(1)* %292, align 16
  br label %postload1199.i

postload1199.i:                                   ; preds = %preload1198.i, %postload1197.i
  %"&(pSB[currWI].offset)1601.i" = add nuw i64 %CurrSBIndex..9.i, 528
  %"&pSB[currWI].offset1602.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1601.i"
  %CastToValueType1603.i = bitcast i8* %"&pSB[currWI].offset1602.i" to i1*
  %loadedValue1604.i = load i1* %CastToValueType1603.i, align 1
  br i1 %loadedValue1604.i, label %preload1200.i, label %postload1201.i

preload1200.i:                                    ; preds = %postload1199.i
  store <4 x float> %278, <4 x float> addrspace(1)* %293, align 16
  br label %postload1201.i

postload1201.i:                                   ; preds = %preload1200.i, %postload1199.i
  %"&(pSB[currWI].offset)1615.i" = add nuw i64 %CurrSBIndex..9.i, 529
  %"&pSB[currWI].offset1616.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1615.i"
  %CastToValueType1617.i = bitcast i8* %"&pSB[currWI].offset1616.i" to i1*
  %loadedValue1618.i = load i1* %CastToValueType1617.i, align 1
  br i1 %loadedValue1618.i, label %preload1202.i, label %postload1203.i

preload1202.i:                                    ; preds = %postload1201.i
  store <4 x float> %279, <4 x float> addrspace(1)* %294, align 16
  br label %postload1203.i

postload1203.i:                                   ; preds = %preload1202.i, %postload1201.i
  %"&(pSB[currWI].offset)1629.i" = add nuw i64 %CurrSBIndex..9.i, 530
  %"&pSB[currWI].offset1630.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1629.i"
  %CastToValueType1631.i = bitcast i8* %"&pSB[currWI].offset1630.i" to i1*
  %loadedValue1632.i = load i1* %CastToValueType1631.i, align 1
  br i1 %loadedValue1632.i, label %preload1204.i, label %postload1203.i.if.end36.i_crit_edge

postload1203.i.if.end36.i_crit_edge:              ; preds = %postload1203.i
  br label %if.end36.i

preload1204.i:                                    ; preds = %postload1203.i
  store <4 x float> %280, <4 x float> addrspace(1)* %295, align 16
  br label %if.end36.i

if.end36.i:                                       ; preds = %postload1203.i.if.end36.i_crit_edge, %preload1204.i
  %"&(pSB[currWI].offset)1293.i" = add nuw i64 %CurrSBIndex..9.i, 384
  %"&pSB[currWI].offset1294.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1293.i"
  %CastToValueType1295.i = bitcast i8* %"&pSB[currWI].offset1294.i" to <16 x i1>*
  %loadedValue1296.i = load <16 x i1>* %CastToValueType1295.i, align 16
  %"&(pSB[currWI].offset)1396.i" = add nuw i64 %CurrSBIndex..9.i, 416
  %"&pSB[currWI].offset1397.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1396.i"
  %CastToValueType1398.i = bitcast i8* %"&pSB[currWI].offset1397.i" to <16 x i1>*
  %loadedValue1399.i = load <16 x i1>* %CastToValueType1398.i, align 16
  %if.end36_to_if.then42562.i = and <16 x i1> %loadedValue1399.i, %loadedValue1296.i
  %extract564.i = extractelement <16 x i1> %if.end36_to_if.then42562.i, i32 1
  %extract565.i = extractelement <16 x i1> %if.end36_to_if.then42562.i, i32 2
  %extract566.i = extractelement <16 x i1> %if.end36_to_if.then42562.i, i32 3
  %extract567.i = extractelement <16 x i1> %if.end36_to_if.then42562.i, i32 4
  %extract568.i = extractelement <16 x i1> %if.end36_to_if.then42562.i, i32 5
  %extract569.i = extractelement <16 x i1> %if.end36_to_if.then42562.i, i32 6
  %extract570.i = extractelement <16 x i1> %if.end36_to_if.then42562.i, i32 7
  %extract571.i = extractelement <16 x i1> %if.end36_to_if.then42562.i, i32 8
  %extract572.i = extractelement <16 x i1> %if.end36_to_if.then42562.i, i32 9
  %extract573.i = extractelement <16 x i1> %if.end36_to_if.then42562.i, i32 10
  %extract574.i = extractelement <16 x i1> %if.end36_to_if.then42562.i, i32 11
  %extract575.i = extractelement <16 x i1> %if.end36_to_if.then42562.i, i32 12
  %extract576.i = extractelement <16 x i1> %if.end36_to_if.then42562.i, i32 13
  %extract577.i = extractelement <16 x i1> %if.end36_to_if.then42562.i, i32 14
  %extract578.i = extractelement <16 x i1> %if.end36_to_if.then42562.i, i32 15
  %extract563.i = extractelement <16 x i1> %if.end36_to_if.then42562.i, i32 0
  br i1 %extract563.i, label %preload1208.i, label %postload1209.i

preload1208.i:                                    ; preds = %if.end36.i
  store float %extract513.i, float addrspace(3)* %32, align 4
  br label %postload1209.i

postload1209.i:                                   ; preds = %preload1208.i, %if.end36.i
  br i1 %extract564.i, label %preload1206.i, label %postload1207.i

preload1206.i:                                    ; preds = %postload1209.i
  store float %extract514.i, float addrspace(3)* %32, align 4
  br label %postload1207.i

postload1207.i:                                   ; preds = %preload1206.i, %postload1209.i
  br i1 %extract565.i, label %preload1210.i, label %postload1211.i

preload1210.i:                                    ; preds = %postload1207.i
  store float %extract515.i, float addrspace(3)* %32, align 4
  br label %postload1211.i

postload1211.i:                                   ; preds = %preload1210.i, %postload1207.i
  br i1 %extract566.i, label %preload1212.i, label %postload1213.i

preload1212.i:                                    ; preds = %postload1211.i
  store float %extract516.i, float addrspace(3)* %32, align 4
  br label %postload1213.i

postload1213.i:                                   ; preds = %preload1212.i, %postload1211.i
  br i1 %extract567.i, label %preload1214.i, label %postload1215.i

preload1214.i:                                    ; preds = %postload1213.i
  store float %extract517.i, float addrspace(3)* %32, align 4
  br label %postload1215.i

postload1215.i:                                   ; preds = %preload1214.i, %postload1213.i
  br i1 %extract568.i, label %preload1216.i, label %postload1217.i

preload1216.i:                                    ; preds = %postload1215.i
  store float %extract518.i, float addrspace(3)* %32, align 4
  br label %postload1217.i

postload1217.i:                                   ; preds = %preload1216.i, %postload1215.i
  br i1 %extract569.i, label %preload1218.i, label %postload1219.i

preload1218.i:                                    ; preds = %postload1217.i
  store float %extract519.i, float addrspace(3)* %32, align 4
  br label %postload1219.i

postload1219.i:                                   ; preds = %preload1218.i, %postload1217.i
  br i1 %extract570.i, label %preload1220.i, label %postload1221.i

preload1220.i:                                    ; preds = %postload1219.i
  store float %extract520.i, float addrspace(3)* %32, align 4
  br label %postload1221.i

postload1221.i:                                   ; preds = %preload1220.i, %postload1219.i
  br i1 %extract571.i, label %preload1222.i, label %postload1223.i

preload1222.i:                                    ; preds = %postload1221.i
  store float %extract521.i, float addrspace(3)* %32, align 4
  br label %postload1223.i

postload1223.i:                                   ; preds = %preload1222.i, %postload1221.i
  br i1 %extract572.i, label %preload1224.i, label %postload1225.i

preload1224.i:                                    ; preds = %postload1223.i
  store float %extract522.i, float addrspace(3)* %32, align 4
  br label %postload1225.i

postload1225.i:                                   ; preds = %preload1224.i, %postload1223.i
  br i1 %extract573.i, label %preload1226.i, label %postload1227.i

preload1226.i:                                    ; preds = %postload1225.i
  store float %extract523.i, float addrspace(3)* %32, align 4
  br label %postload1227.i

postload1227.i:                                   ; preds = %preload1226.i, %postload1225.i
  br i1 %extract574.i, label %preload1228.i, label %postload1229.i

preload1228.i:                                    ; preds = %postload1227.i
  store float %extract524.i, float addrspace(3)* %32, align 4
  br label %postload1229.i

postload1229.i:                                   ; preds = %preload1228.i, %postload1227.i
  br i1 %extract575.i, label %preload1230.i, label %postload1231.i

preload1230.i:                                    ; preds = %postload1229.i
  store float %extract525.i, float addrspace(3)* %32, align 4
  br label %postload1231.i

postload1231.i:                                   ; preds = %preload1230.i, %postload1229.i
  br i1 %extract576.i, label %preload1232.i, label %postload1233.i

preload1232.i:                                    ; preds = %postload1231.i
  store float %extract526.i, float addrspace(3)* %32, align 4
  br label %postload1233.i

postload1233.i:                                   ; preds = %preload1232.i, %postload1231.i
  br i1 %extract577.i, label %preload1234.i, label %postload1235.i

preload1234.i:                                    ; preds = %postload1233.i
  store float %extract527.i, float addrspace(3)* %32, align 4
  br label %postload1235.i

postload1235.i:                                   ; preds = %preload1234.i, %postload1233.i
  br i1 %extract578.i, label %preload1236.i, label %postload1235.i.if.end43.i_crit_edge

postload1235.i.if.end43.i_crit_edge:              ; preds = %postload1235.i
  br label %if.end43.i

preload1236.i:                                    ; preds = %postload1235.i
  store float %extract528.i, float addrspace(3)* %32, align 4
  br label %if.end43.i

if.end43.i:                                       ; preds = %postload1235.i.if.end43.i_crit_edge, %preload1236.i
  %loadedValue1809.i = load i1* %CastToValueType1813.i, align 1
  br i1 %loadedValue1809.i, label %preload972.i, label %if.end43.postload973_crit_edge.i

if.end43.postload973_crit_edge.i:                 ; preds = %if.end43.i
  %masked_load752.pre.i = load float addrspace(3)* %32, align 4
  br label %postload973.i

preload972.i:                                     ; preds = %if.end43.i
  %check.WI.iter2972.i = icmp ult i64 %CurrWI..9.i, %28
  br i1 %check.WI.iter2972.i, label %thenBB2969.i, label %preload972.i.SyncBB2943.i_crit_edge

preload972.i.SyncBB2943.i_crit_edge:              ; preds = %preload972.i
  br label %SyncBB2943.i

thenBB2969.i:                                     ; preds = %preload972.i
  %"CurrWI++2973.i" = add nuw i64 %CurrWI..9.i, 1
  %"loadedCurrSB+Stride2975.i" = add nuw i64 %CurrSBIndex..9.i, 1792
  switch i32 %currBarrier.9.i, label %postload1149.i [
    i32 21, label %SyncBB2941.i
    i32 20, label %thenBB2969.i.postload968.i_crit_edge
    i32 24, label %SyncBB.i
    i32 23, label %thenBB2969.i.SyncBB2943.i_crit_edge
  ]

thenBB2969.i.SyncBB2943.i_crit_edge:              ; preds = %thenBB2969.i
  br label %SyncBB2943.i

thenBB2969.i.postload968.i_crit_edge:             ; preds = %thenBB2969.i
  br label %postload968.i

SyncBB2943.i:                                     ; preds = %thenBB.i.SyncBB2943.i_crit_edge, %thenBB2969.i.SyncBB2943.i_crit_edge, %preload972.i.SyncBB2943.i_crit_edge, %thenBB2961.i.SyncBB2943.i_crit_edge, %thenBB2953.i.SyncBB2943.i_crit_edge, %thenBB2945.i.SyncBB2943.i_crit_edge
  %currBarrier.10.i = phi i32 [ %currBarrier.1.i, %thenBB2945.i.SyncBB2943.i_crit_edge ], [ %currBarrier.4.i, %thenBB2953.i.SyncBB2943.i_crit_edge ], [ %currBarrier.6.i, %thenBB2961.i.SyncBB2943.i_crit_edge ], [ 23, %preload972.i.SyncBB2943.i_crit_edge ], [ %currBarrier.9.i, %thenBB2969.i.SyncBB2943.i_crit_edge ], [ %currBarrier.12.i, %thenBB.i.SyncBB2943.i_crit_edge ]
  %CurrSBIndex..10.i = phi i64 [ %"loadedCurrSB+Stride2951.i", %thenBB2945.i.SyncBB2943.i_crit_edge ], [ %"loadedCurrSB+Stride2959.i", %thenBB2953.i.SyncBB2943.i_crit_edge ], [ %"loadedCurrSB+Stride2967.i", %thenBB2961.i.SyncBB2943.i_crit_edge ], [ 0, %preload972.i.SyncBB2943.i_crit_edge ], [ %"loadedCurrSB+Stride2975.i", %thenBB2969.i.SyncBB2943.i_crit_edge ], [ %"loadedCurrSB+Stride.i", %thenBB.i.SyncBB2943.i_crit_edge ]
  %CurrWI..10.i = phi i64 [ %"CurrWI++2949.i", %thenBB2945.i.SyncBB2943.i_crit_edge ], [ %"CurrWI++2957.i", %thenBB2953.i.SyncBB2943.i_crit_edge ], [ %"CurrWI++2965.i", %thenBB2961.i.SyncBB2943.i_crit_edge ], [ 0, %preload972.i.SyncBB2943.i_crit_edge ], [ %"CurrWI++2973.i", %thenBB2969.i.SyncBB2943.i_crit_edge ], [ %"CurrWI++.i", %thenBB.i.SyncBB2943.i_crit_edge ]
  %masked_load751.i = load float addrspace(3)* %32, align 4
  br label %postload973.i

postload973.i:                                    ; preds = %SyncBB2943.i, %if.end43.postload973_crit_edge.i
  %currBarrier.11.i = phi i32 [ %currBarrier.10.i, %SyncBB2943.i ], [ %currBarrier.9.i, %if.end43.postload973_crit_edge.i ]
  %CurrSBIndex..11.i = phi i64 [ %CurrSBIndex..10.i, %SyncBB2943.i ], [ %CurrSBIndex..9.i, %if.end43.postload973_crit_edge.i ]
  %CurrWI..11.i = phi i64 [ %CurrWI..10.i, %SyncBB2943.i ], [ %CurrWI..9.i, %if.end43.postload973_crit_edge.i ]
  %masked_load752.i = phi float [ %masked_load751.i, %SyncBB2943.i ], [ %masked_load752.pre.i, %if.end43.postload973_crit_edge.i ]
  %phi974.i = phi float [ %masked_load751.i, %SyncBB2943.i ], [ undef, %if.end43.postload973_crit_edge.i ]
  %temp.vect625.i = insertelement <16 x float> undef, float %phi974.i, i32 0
  %temp.vect626.i = insertelement <16 x float> %temp.vect625.i, float %masked_load752.i, i32 1
  %temp.vect627.i = insertelement <16 x float> %temp.vect626.i, float %masked_load752.i, i32 2
  %temp.vect628.i = insertelement <16 x float> %temp.vect627.i, float %masked_load752.i, i32 3
  %temp.vect629.i = insertelement <16 x float> %temp.vect628.i, float %masked_load752.i, i32 4
  %temp.vect630.i = insertelement <16 x float> %temp.vect629.i, float %masked_load752.i, i32 5
  %temp.vect631.i = insertelement <16 x float> %temp.vect630.i, float %masked_load752.i, i32 6
  %temp.vect632.i = insertelement <16 x float> %temp.vect631.i, float %masked_load752.i, i32 7
  %temp.vect633.i = insertelement <16 x float> %temp.vect632.i, float %masked_load752.i, i32 8
  %temp.vect634.i = insertelement <16 x float> %temp.vect633.i, float %masked_load752.i, i32 9
  %temp.vect635.i = insertelement <16 x float> %temp.vect634.i, float %masked_load752.i, i32 10
  %temp.vect636.i = insertelement <16 x float> %temp.vect635.i, float %masked_load752.i, i32 11
  %temp.vect637.i = insertelement <16 x float> %temp.vect636.i, float %masked_load752.i, i32 12
  %temp.vect638.i = insertelement <16 x float> %temp.vect637.i, float %masked_load752.i, i32 13
  %temp.vect639.i = insertelement <16 x float> %temp.vect638.i, float %masked_load752.i, i32 14
  %temp.vect640.i = insertelement <16 x float> %temp.vect639.i, float %masked_load752.i, i32 15
  %"&(pSB[currWI].offset)1424.i" = add nuw i64 %CurrSBIndex..11.i, 512
  %"&pSB[currWI].offset1425.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1424.i"
  %CastToValueType1426.i = bitcast i8* %"&pSB[currWI].offset1425.i" to i32*
  %loadedValue1427.i = load i32* %CastToValueType1426.i, align 4
  %conv452.i = zext i32 %loadedValue1427.i to i64
  %"&(pSB[currWI].offset)1275.i" = add nuw i64 %CurrSBIndex..11.i, 128
  %"&pSB[currWI].offset1276.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1275.i"
  %CastToValueType1277.i = bitcast i8* %"&pSB[currWI].offset1276.i" to i64*
  %loadedValue1278.i = load i64* %CastToValueType1277.i, align 8
  %add46.i = add i64 %loadedValue1278.i, %conv452.i
  %conv47.i = trunc i64 %add46.i to i32
  %"&(pSB[currWI].offset)1284.i" = add nuw i64 %CurrSBIndex..11.i, 256
  %"&pSB[currWI].offset1285.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1284.i"
  %CastToValueType1286.i = bitcast i8* %"&pSB[currWI].offset1285.i" to <16 x i64>*
  %loadedValue1287.i = load <16 x i64>* %CastToValueType1286.i, align 128
  %"&(pSB[currWI].offset)1652.i" = add nuw i64 %CurrSBIndex..11.i, 640
  %"&pSB[currWI].offset1653.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1652.i"
  %CastToValueType1654.i = bitcast i8* %"&pSB[currWI].offset1653.i" to <16 x i64>*
  %loadedValue1655.i = load <16 x i64>* %CastToValueType1654.i, align 128
  %add50582.i = add <16 x i64> %loadedValue1287.i, %loadedValue1655.i
  %cmp14.i = icmp slt i32 %conv47.i, %cond.i
  %temp606.i = insertelement <16 x i1> undef, i1 %cmp14.i, i32 0
  %vector607.i = shufflevector <16 x i1> %temp606.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %notCond22.i = xor i1 %cmp14.i, true
  %temp583.i = insertelement <16 x i1> undef, i1 %notCond22.i, i32 0
  %vector584.i = shufflevector <16 x i1> %temp583.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)1401.i" = add nuw i64 %CurrSBIndex..11.i, 416
  %"&pSB[currWI].offset1402.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1401.i"
  %CastToValueType1403.i = bitcast i8* %"&pSB[currWI].offset1402.i" to <16 x i1>*
  %loadedValue1404.i = load <16 x i1>* %CastToValueType1403.i, align 16
  %who_left_tr23585.i = and <16 x i1> %loadedValue1404.i, %vector584.i
  %"&(pSB[currWI].offset)1302.i" = add nuw i64 %CurrSBIndex..11.i, 400
  %"&pSB[currWI].offset1303.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1302.i"
  %CastToValueType1304.i = bitcast i8* %"&pSB[currWI].offset1303.i" to <16 x i1>*
  %loadedValue1305.i = load <16 x i1>* %CastToValueType1304.i, align 16
  %loop_mask26587.i = or <16 x i1> %loadedValue1305.i, %who_left_tr23585.i
  %ipred.i.i = bitcast <16 x i1> %loop_mask26587.i to i16
  %val.i.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i.i, i16 %ipred.i.i) nounwind
  %301 = and i32 %val.i.i, 1
  %res.i.i = icmp eq i32 %301, 0
  %local_edge31608.i = and <16 x i1> %loadedValue1404.i, %vector607.i
  br i1 %res.i.i, label %while.body.i, label %while.end.i

while.end.i:                                      ; preds = %postload973.i, %postload828.i
  %currBarrier.12.i = phi i32 [ %currBarrier.0.i, %postload828.i ], [ %currBarrier.11.i, %postload973.i ]
  %CurrSBIndex..12.i = phi i64 [ %CurrSBIndex..0.i, %postload828.i ], [ %CurrSBIndex..11.i, %postload973.i ]
  %CurrWI..12.i = phi i64 [ %CurrWI..0.i, %postload828.i ], [ %CurrWI..11.i, %postload973.i ]
  %check.WI.iter.i = icmp ult i64 %CurrWI..12.i, %28
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.bottom_scan_separated_args.exit

thenBB.i:                                         ; preds = %while.end.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..12.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..12.i, 1792
  switch i32 %currBarrier.12.i, label %thenBB.i.SyncBB2943.i_crit_edge [
    i32 22, label %postload1149.i
    i32 21, label %SyncBB2941.i
    i32 20, label %thenBB.i.postload968.i_crit_edge
    i32 24, label %SyncBB.i
  ]

thenBB.i.SyncBB2943.i_crit_edge:                  ; preds = %thenBB.i
  br label %SyncBB2943.i

thenBB.i.postload968.i_crit_edge:                 ; preds = %thenBB.i
  br label %postload968.i

____Vectorized_.bottom_scan_separated_args.exit:  ; preds = %while.end.i
  ret void
}

!opencl.kernels = !{!0, !2, !4}
!opencl.build.options = !{!6}
!cl.noBarrierPath.kernels = !{!6}
!opencl.wrappers = !{!7, !8, !9}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32, float addrspace(3)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__reduce_separated_args, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3}
!2 = metadata !{void (float addrspace(1)*, i32, float addrspace(3)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__top_scan_separated_args, metadata !3}
!3 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3}
!4 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32, float addrspace(3)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__bottom_scan_separated_args, metadata !5}
!5 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3, i32 3}
!6 = metadata !{}
!7 = metadata !{void (i8*)* @reduce}
!8 = metadata !{void (i8*)* @top_scan}
!9 = metadata !{void (i8*)* @bottom_scan}
