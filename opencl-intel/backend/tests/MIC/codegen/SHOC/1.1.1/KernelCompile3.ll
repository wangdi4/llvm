; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @__uniformAdd_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, i32, i32) nounwind

declare i64 @get_group_id(i32) nounwind readnone

declare i64 @get_local_size(i32) nounwind readnone

declare i64 @get_local_id(i32) nounwind readnone

declare void @barrier(i64)

declare void @__scan_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, i32, i32, i32, float addrspace(3)* nocapture) nounwind

declare void @____Vectorized_.uniformAdd_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, i32, i32) nounwind

declare void @____Vectorized_.scan_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, i32, i32, i32, float addrspace(3)* nocapture) nounwind

declare i1 @allOne(i1)

declare i1 @allZero(i1)

declare float @masked_load_align4_18(i1, float addrspace(1)*)

declare void @masked_store_align4_18(i1, float, float addrspace(1)*)

declare <16 x float> @masked_load_align4_19(<16 x i1>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_19(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare i64 @maskedf_6_get_local_size(i1, i32)

declare i64 @maskedf_7_get_group_id(i1, i32)

declare i64 @maskedf_8_get_local_size(i1, i32)

declare float @masked_load_align4_20(i1, float addrspace(1)*)

declare void @masked_store_align4_20(i1, float, float addrspace(3)*)

declare void @masked_store_align4_21(i1, float, float addrspace(3)*)

declare void @maskedf_9_barrier(i1, i64)

declare float @masked_load_align4_21(i1, float addrspace(3)*)

declare float @masked_load_align4_22(i1, float addrspace(3)*)

declare void @masked_store_align4_22(i1, float, float addrspace(3)*)

declare i64 @maskedf_10_get_group_id(i1, i32)

declare float @masked_load_align4_23(i1, float addrspace(3)*)

declare void @masked_store_align4_23(i1, float, float addrspace(1)*)

declare void @masked_store_align4_24(i1, float, float addrspace(3)*)

declare void @maskedf_11_barrier(i1, i64)

declare float @masked_load_align4_24(i1, float addrspace(3)*)

declare float @masked_load_align4_25(i1, float addrspace(3)*)

declare void @masked_store_align4_25(i1, float, float addrspace(3)*)

declare float @masked_load_align4_26(i1, float addrspace(3)*)

declare void @masked_store_align4_26(i1, float, float addrspace(3)*)

declare float @masked_load_align4_27(i1, float addrspace(3)*)

declare void @masked_store_align4_27(i1, float, float addrspace(1)*)

declare void @masked_store_align4_28(<16 x i1>, <16 x float>, <16 x float> addrspace(3)*)

declare void @masked_store_align4_29(<16 x i1>, <16 x float>, <16 x float> addrspace(3)*)

declare i1 @allOne_v16(<16 x i1>)

declare i1 @allZero_v16(<16 x i1>)

declare <16 x float> @masked_load_align4_35(<16 x i1>, <16 x float> addrspace(3)*)

declare void @dummybarrier.()

declare i8* @get_special_buffer.() nounwind readnone

declare i64 @get_iter_count.() nounwind readnone

declare i64 @get_new_local_id.(i32, i64) nounwind readnone

declare void @__uniformAdd_separated_args(float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, i32, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare void @__scan_separated_args(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, i32, i32, i32, float addrspace(3)* nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare void @____Vectorized_.uniformAdd_separated_args(float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, i32, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare void @____Vectorized_.scan_separated_args(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, i32, i32, i32, float addrspace(3)* nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare i32 @llvm.x86.mic.kortestz(i16, i16) nounwind readnone

declare i32 @llvm.x86.mic.kortestc(i16, i16) nounwind readnone

define void @scan(i8* %pBuffer) {
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
  %11 = getelementptr i8* %pBuffer, i64 28
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 32
  %15 = bitcast i8* %14 to i32*
  %16 = load i32* %15, align 4
  %17 = getelementptr i8* %pBuffer, i64 36
  %18 = bitcast i8* %17 to i32*
  %19 = load i32* %18, align 4
  %20 = getelementptr i8* %pBuffer, i64 40
  %21 = bitcast i8* %20 to float addrspace(3)**
  %22 = load float addrspace(3)** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 56
  %24 = bitcast i8* %23 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %25 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 64
  %27 = bitcast i8* %26 to i64**
  %28 = load i64** %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 80
  %30 = bitcast i8* %29 to <{ [4 x i64] }>**
  %31 = load <{ [4 x i64] }>** %30, align 8
  %32 = getelementptr i8* %pBuffer, i64 96
  %33 = bitcast i8* %32 to i64*
  %34 = load i64* %33, align 8
  %35 = getelementptr i8* %pBuffer, i64 104
  %36 = bitcast i8* %35 to i8**
  %37 = load i8** %36, align 8
  %cmp.i = icmp eq i32 %16, 0
  %cmp43.i = icmp eq i32 %13, 0
  %cmp56.i = icmp eq i32 %19, 1
  %conv46.i = sext i32 %13 to i64
  br label %SyncBB224.i

SyncBB224.i:                                      ; preds = %thenBB.i, %thenBB227.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ], [ %"CurrWI++231.i", %thenBB227.i ]
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ %"loadedCurrSB+Stride233.i", %thenBB227.i ]
  %currBarrier.0.i = phi i32 [ 15, %entry ], [ %currBarrier.3.i, %thenBB.i ], [ %currBarrier.1.i, %thenBB227.i ]
  br i1 %cmp.i, label %if.then.i, label %entry.if.end_crit_edge.i

entry.if.end_crit_edge.i:                         ; preds = %SyncBB224.i
  %38 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %25, i64 0, i32 3, i64 0
  %39 = load i64* %38, align 8
  br label %if.end.i

if.then.i:                                        ; preds = %SyncBB224.i
  %40 = load i64* %28, align 8
  %41 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %25, i64 0, i32 3, i64 0
  %42 = load i64* %41, align 8
  %shl.i = shl i64 %40, 1
  %mul.i = mul i64 %shl.i, %42
  %conv.i = trunc i64 %mul.i to i32
  br label %if.end.i

if.end.i:                                         ; preds = %if.then.i, %entry.if.end_crit_edge.i
  %call5.pre-phi.i = phi i64 [ %39, %entry.if.end_crit_edge.i ], [ %42, %if.then.i ]
  %bIndex.0.i = phi i32 [ %16, %entry.if.end_crit_edge.i ], [ %conv.i, %if.then.i ]
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %37, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i64*
  store i64 %call5.pre-phi.i, i64* %CastToValueType.i, align 8
  %43 = getelementptr <{ [4 x i64] }>* %31, i64 %CurrWI..0.i, i32 0, i64 0
  %44 = load i64* %43, align 8
  %"&(pSB[currWI].offset)31.i" = add nuw i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset32.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)31.i"
  %CastToValueType33.i = bitcast i8* %"&pSB[currWI].offset32.i" to i64*
  store i64 %44, i64* %CastToValueType33.i, align 8
  %conv3.i = trunc i64 %44 to i32
  %"&(pSB[currWI].offset)40.i" = add nuw i64 %CurrSBIndex..0.i, 16
  %"&pSB[currWI].offset41.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)40.i"
  %CastToValueType42.i = bitcast i8* %"&pSB[currWI].offset41.i" to i32*
  store i32 %conv3.i, i32* %CastToValueType42.i, align 4
  %add.i = add nsw i32 %conv3.i, %bIndex.0.i
  %conv41.i = zext i32 %add.i to i64
  %add6.i = add i64 %conv41.i, %call5.pre-phi.i
  %conv7.i = trunc i64 %add6.i to i32
  %"&(pSB[currWI].offset)64.i" = add nuw i64 %CurrSBIndex..0.i, 20
  %"&pSB[currWI].offset65.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)64.i"
  %CastToValueType66.i = bitcast i8* %"&pSB[currWI].offset65.i" to i32*
  store i32 %conv7.i, i32* %CastToValueType66.i, align 4
  %add10.i = add i64 %call5.pre-phi.i, %44
  %conv11.i = trunc i64 %add10.i to i32
  %"&(pSB[currWI].offset)78.i" = add nuw i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset79.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)78.i"
  %CastToValueType80.i = bitcast i8* %"&pSB[currWI].offset79.i" to i32*
  store i32 %conv11.i, i32* %CastToValueType80.i, align 4
  %idxprom.i = sext i32 %add.i to i64
  %"&(pSB[currWI].offset)97.i" = add nuw i64 %CurrSBIndex..0.i, 32
  %"&pSB[currWI].offset98.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)97.i"
  %CastToValueType99.i = bitcast i8* %"&pSB[currWI].offset98.i" to i64*
  store i64 %idxprom.i, i64* %CastToValueType99.i, align 8
  %arrayidx.i = getelementptr inbounds float addrspace(1)* %4, i64 %idxprom.i
  %45 = load float addrspace(1)* %arrayidx.i, align 4
  %idxprom12.i = sext i32 %conv3.i to i64
  %arrayidx13.i = getelementptr inbounds float addrspace(3)* %22, i64 %idxprom12.i
  %"&(pSB[currWI].offset)106.i" = add nuw i64 %CurrSBIndex..0.i, 40
  %"&pSB[currWI].offset107.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)106.i"
  %CastToValueType108.i = bitcast i8* %"&pSB[currWI].offset107.i" to float addrspace(3)**
  store float addrspace(3)* %arrayidx13.i, float addrspace(3)** %CastToValueType108.i, align 8
  store float %45, float addrspace(3)* %arrayidx13.i, align 4
  %cmp14.i = icmp slt i32 %conv11.i, %10
  %"&(pSB[currWI].offset)115.i" = add nuw i64 %CurrSBIndex..0.i, 48
  %"&pSB[currWI].offset116.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)115.i"
  %CastToValueType117.i = bitcast i8* %"&pSB[currWI].offset116.i" to i1*
  store i1 %cmp14.i, i1* %CastToValueType117.i, align 1
  br i1 %cmp14.i, label %if.then16.i, label %if.else21.i

if.then16.i:                                      ; preds = %if.end.i
  %loadedValue71.i = load i32* %CastToValueType66.i, align 4
  %idxprom17.i = sext i32 %loadedValue71.i to i64
  %arrayidx18.i = getelementptr inbounds float addrspace(1)* %4, i64 %idxprom17.i
  %46 = load float addrspace(1)* %arrayidx18.i, align 4
  %loadedValue85.i = load i32* %CastToValueType80.i, align 4
  %idxprom19.i = sext i32 %loadedValue85.i to i64
  %arrayidx20.i = getelementptr inbounds float addrspace(3)* %22, i64 %idxprom19.i
  store float %46, float addrspace(3)* %arrayidx20.i, align 4
  br label %if.end24.i

if.else21.i:                                      ; preds = %if.end.i
  %loadedValue90.i = load i32* %CastToValueType80.i, align 4
  %idxprom22.i = sext i32 %loadedValue90.i to i64
  %arrayidx23.i = getelementptr inbounds float addrspace(3)* %22, i64 %idxprom22.i
  store float 0.000000e+00, float addrspace(3)* %arrayidx23.i, align 4
  br label %if.end24.i

if.end24.i:                                       ; preds = %if.else21.i, %if.then16.i
  %loadedValue.i = load i64* %CastToValueType.i, align 8
  %conv26.i = trunc i64 %loadedValue.i to i32
  %cmp276.i = icmp sgt i32 %conv26.i, 0
  br i1 %cmp276.i, label %for.body.lr.ph.i, label %for.end.i

for.body.lr.ph.i:                                 ; preds = %if.end24.i
  %loadedValue47.i = load i32* %CastToValueType42.i, align 4
  %mul32.i = shl i32 %loadedValue47.i, 1
  %"&(pSB[currWI].offset)124.i" = add nuw i64 %CurrSBIndex..0.i, 52
  %"&pSB[currWI].offset125.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)124.i"
  %CastToValueType126.i = bitcast i8* %"&pSB[currWI].offset125.i" to i32*
  store i32 %mul32.i, i32* %CastToValueType126.i, align 4
  br label %for.body.i

for.body.i:                                       ; preds = %if.end41.i, %for.body.lr.ph.i
  %CurrWI..1.i = phi i64 [ %CurrWI..0.i, %for.body.lr.ph.i ], [ %CurrWI..2.i, %if.end41.i ]
  %CurrSBIndex..1.i = phi i64 [ %CurrSBIndex..0.i, %for.body.lr.ph.i ], [ %CurrSBIndex..2.i, %if.end41.i ]
  %currBarrier.1.i = phi i32 [ %currBarrier.0.i, %for.body.lr.ph.i ], [ %currBarrier.2.i, %if.end41.i ]
  %d.08.i = phi i32 [ %conv26.i, %for.body.lr.ph.i ], [ %shr.i, %if.end41.i ]
  %stride.07.i = phi i32 [ 1, %for.body.lr.ph.i ], [ %mul42.i, %if.end41.i ]
  %"&(pSB[currWI].offset)147.i" = add nuw i64 %CurrSBIndex..1.i, 60
  %"&pSB[currWI].offset148.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)147.i"
  %CastToValueType149.i = bitcast i8* %"&pSB[currWI].offset148.i" to i32*
  store i32 %stride.07.i, i32* %CastToValueType149.i, align 4
  %"&(pSB[currWI].offset)133.i" = add nuw i64 %CurrSBIndex..1.i, 56
  %"&pSB[currWI].offset134.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)133.i"
  %CastToValueType135.i = bitcast i8* %"&pSB[currWI].offset134.i" to i32*
  store i32 %d.08.i, i32* %CastToValueType135.i, align 4
  %check.WI.iter230.i = icmp ult i64 %CurrWI..1.i, %34
  br i1 %check.WI.iter230.i, label %thenBB227.i, label %SyncBB221.i

thenBB227.i:                                      ; preds = %for.body.i
  %"CurrWI++231.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride233.i" = add nuw i64 %CurrSBIndex..1.i, 80
  %cond4.i = icmp eq i32 %currBarrier.1.i, 15
  br i1 %cond4.i, label %SyncBB224.i, label %SyncBB221.i

SyncBB221.i:                                      ; preds = %thenBB.i, %thenBB227.i, %for.body.i
  %CurrWI..2.i = phi i64 [ %"CurrWI++231.i", %thenBB227.i ], [ %"CurrWI++.i", %thenBB.i ], [ 0, %for.body.i ]
  %CurrSBIndex..2.i = phi i64 [ %"loadedCurrSB+Stride233.i", %thenBB227.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %for.body.i ]
  %currBarrier.2.i = phi i32 [ %currBarrier.1.i, %thenBB227.i ], [ %currBarrier.3.i, %thenBB.i ], [ 2, %for.body.i ]
  %"&(pSB[currWI].offset)54.i" = add nuw i64 %CurrSBIndex..2.i, 16
  %"&pSB[currWI].offset55.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)54.i"
  %CastToValueType56.i = bitcast i8* %"&pSB[currWI].offset55.i" to i32*
  %loadedValue57.i = load i32* %CastToValueType56.i, align 4
  %"&(pSB[currWI].offset)137.i" = add nuw i64 %CurrSBIndex..2.i, 56
  %"&pSB[currWI].offset138.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)137.i"
  %CastToValueType139.i = bitcast i8* %"&pSB[currWI].offset138.i" to i32*
  %loadedValue140.i = load i32* %CastToValueType139.i, align 4
  %cmp29.i = icmp slt i32 %loadedValue57.i, %loadedValue140.i
  br i1 %cmp29.i, label %if.then31.i, label %SyncBB221.if.end41_crit_edge.i

SyncBB221.if.end41_crit_edge.i:                   ; preds = %SyncBB221.i
  %"&(pSB[currWI].offset)151.pre.i" = add nuw i64 %CurrSBIndex..2.i, 60
  %"&pSB[currWI].offset152.pre.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)151.pre.i"
  %CastToValueType153.pre.i = bitcast i8* %"&pSB[currWI].offset152.pre.i" to i32*
  br label %if.end41.i

if.then31.i:                                      ; preds = %SyncBB221.i
  %"&(pSB[currWI].offset)128.i" = add nuw i64 %CurrSBIndex..2.i, 52
  %"&pSB[currWI].offset129.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)128.i"
  %CastToValueType130.i = bitcast i8* %"&pSB[currWI].offset129.i" to i32*
  %loadedValue131.i = load i32* %CastToValueType130.i, align 4
  %"&(pSB[currWI].offset)156.i" = add nuw i64 %CurrSBIndex..2.i, 60
  %"&pSB[currWI].offset157.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)156.i"
  %CastToValueType158.i = bitcast i8* %"&pSB[currWI].offset157.i" to i32*
  %loadedValue159.i = load i32* %CastToValueType158.i, align 4
  %mul33.i = mul i32 %loadedValue131.i, %loadedValue159.i
  %add34.i = add i32 %loadedValue159.i, -1
  %sub.i = add i32 %add34.i, %mul33.i
  %add35.i = add i32 %sub.i, %loadedValue159.i
  %idxprom36.i = sext i32 %sub.i to i64
  %arrayidx37.i = getelementptr inbounds float addrspace(3)* %22, i64 %idxprom36.i
  %47 = load float addrspace(3)* %arrayidx37.i, align 4
  %idxprom38.i = sext i32 %add35.i to i64
  %arrayidx39.i = getelementptr inbounds float addrspace(3)* %22, i64 %idxprom38.i
  %48 = load float addrspace(3)* %arrayidx39.i, align 4
  %add40.i = fadd float %48, %47
  store float %add40.i, float addrspace(3)* %arrayidx39.i, align 4
  %loadedValue145.pre.i = load i32* %CastToValueType139.i, align 4
  br label %if.end41.i

if.end41.i:                                       ; preds = %if.then31.i, %SyncBB221.if.end41_crit_edge.i
  %CastToValueType153.pre-phi.i = phi i32* [ %CastToValueType153.pre.i, %SyncBB221.if.end41_crit_edge.i ], [ %CastToValueType158.i, %if.then31.i ]
  %loadedValue145.i = phi i32 [ %loadedValue140.i, %SyncBB221.if.end41_crit_edge.i ], [ %loadedValue145.pre.i, %if.then31.i ]
  %loadedValue154.i = load i32* %CastToValueType153.pre-phi.i, align 4
  %mul42.i = shl i32 %loadedValue154.i, 1
  %shr.i = ashr i32 %loadedValue145.i, 1
  %cmp27.i = icmp sgt i32 %shr.i, 0
  br i1 %cmp27.i, label %for.body.i, label %for.end.i

for.end.i:                                        ; preds = %if.end41.i, %if.end24.i
  %CurrWI..3.i = phi i64 [ %CurrWI..0.i, %if.end24.i ], [ %CurrWI..2.i, %if.end41.i ]
  %CurrSBIndex..3.i = phi i64 [ %CurrSBIndex..0.i, %if.end24.i ], [ %CurrSBIndex..2.i, %if.end41.i ]
  %currBarrier.3.i = phi i32 [ %currBarrier.0.i, %if.end24.i ], [ %currBarrier.2.i, %if.end41.i ]
  %stride.0.lcssa.i = phi i32 [ 1, %if.end24.i ], [ %mul42.i, %if.end41.i ]
  %"&(pSB[currWI].offset)171.i" = add nuw i64 %CurrSBIndex..3.i, 64
  %"&pSB[currWI].offset172.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)171.i"
  %CastToValueType173.i = bitcast i8* %"&pSB[currWI].offset172.i" to i32*
  store i32 %stride.0.lcssa.i, i32* %CastToValueType173.i, align 4
  br i1 %cmp43.i, label %cond.true.i, label %cond.end.i

cond.true.i:                                      ; preds = %for.end.i
  %49 = load i64* %28, align 8
  br label %cond.end.i

cond.end.i:                                       ; preds = %cond.true.i, %for.end.i
  %cond.i = phi i64 [ %49, %cond.true.i ], [ %conv46.i, %for.end.i ]
  %"&(pSB[currWI].offset)35.i" = add nuw i64 %CurrSBIndex..3.i, 8
  %"&pSB[currWI].offset36.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)35.i"
  %CastToValueType37.i = bitcast i8* %"&pSB[currWI].offset36.i" to i64*
  %loadedValue38.i = load i64* %CastToValueType37.i, align 8
  %cmp49.i = icmp eq i64 %loadedValue38.i, 0
  br i1 %cmp49.i, label %if.then51.i, label %if.end66.i

if.then51.i:                                      ; preds = %cond.end.i
  %"&pSB[currWI].offset27.i" = getelementptr inbounds i8* %37, i64 %CurrSBIndex..3.i
  %CastToValueType28.i = bitcast i8* %"&pSB[currWI].offset27.i" to i64*
  %loadedValue29.i = load i64* %CastToValueType28.i, align 8
  %shl53.i = shl i64 %loadedValue29.i, 33
  %sext9.i = add i64 %shl53.i, -4294967296
  %idxprom59.i = ashr exact i64 %sext9.i, 32
  %arrayidx60.i = getelementptr inbounds float addrspace(3)* %22, i64 %idxprom59.i
  br i1 %cmp56.i, label %if.then58.i, label %if.end63.i

if.then58.i:                                      ; preds = %if.then51.i
  %50 = load float addrspace(3)* %arrayidx60.i, align 4
  %sext.i = shl i64 %cond.i, 32
  %idxprom61.i = ashr exact i64 %sext.i, 32
  %arrayidx62.i = getelementptr inbounds float addrspace(1)* %7, i64 %idxprom61.i
  store float %50, float addrspace(1)* %arrayidx62.i, align 4
  br label %if.end63.i

if.end63.i:                                       ; preds = %if.then58.i, %if.then51.i
  store float 0.000000e+00, float addrspace(3)* %arrayidx60.i, align 4
  br label %if.end66.i

if.end66.i:                                       ; preds = %if.end63.i, %cond.end.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..3.i, %34
  br i1 %check.WI.iter.i, label %thenBB.i, label %SyncBB.i

thenBB.i:                                         ; preds = %if.end66.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..3.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..3.i, 80
  %cond3.i = icmp eq i32 %currBarrier.3.i, 2
  br i1 %cond3.i, label %SyncBB221.i, label %SyncBB224.i

SyncBB.i:                                         ; preds = %thenBB243.i, %thenBB235.i, %if.end66.i
  %CurrWI..4.i = phi i64 [ 0, %if.end66.i ], [ %"CurrWI++247.i", %thenBB243.i ], [ %"CurrWI++239.i", %thenBB235.i ]
  %CurrSBIndex..4.i = phi i64 [ 0, %if.end66.i ], [ %"loadedCurrSB+Stride249.i", %thenBB243.i ], [ %"loadedCurrSB+Stride241.i", %thenBB235.i ]
  %currBarrier.4.i = phi i32 [ 1, %if.end66.i ], [ %currBarrier.7.i, %thenBB243.i ], [ %currBarrier.5.i, %thenBB235.i ]
  %"&pSB[currWI].offset22.i" = getelementptr inbounds i8* %37, i64 %CurrSBIndex..4.i
  %CastToValueType23.i = bitcast i8* %"&pSB[currWI].offset22.i" to i64*
  %loadedValue24.i = load i64* %CastToValueType23.i, align 8
  %cmp713.i = icmp eq i64 %loadedValue24.i, 0
  br i1 %cmp713.i, label %for.end98.i, label %for.body73.preheader.i

for.body73.preheader.i:                           ; preds = %SyncBB.i
  %"&(pSB[currWI].offset)175.i" = add nuw i64 %CurrSBIndex..4.i, 64
  br label %for.body73.i

for.body73.i:                                     ; preds = %for.inc96.i, %for.body73.preheader.i
  %CurrWI..5.i = phi i64 [ %CurrWI..6.i, %for.inc96.i ], [ %CurrWI..4.i, %for.body73.preheader.i ]
  %CurrSBIndex..5.i = phi i64 [ %CurrSBIndex..6.i, %for.inc96.i ], [ %CurrSBIndex..4.i, %for.body73.preheader.i ]
  %currBarrier.5.i = phi i32 [ %currBarrier.6.i, %for.inc96.i ], [ %currBarrier.4.i, %for.body73.preheader.i ]
  %d67.05.i = phi i32 [ %mul97.i, %for.inc96.i ], [ 1, %for.body73.preheader.i ]
  %"&(pSB[currWI].offset)207.pn.i" = phi i64 [ %"&(pSB[currWI].offset)207.pre-phi.i", %for.inc96.i ], [ %"&(pSB[currWI].offset)175.i", %for.body73.preheader.i ]
  %stride.14.in.in.i = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)207.pn.i"
  %stride.14.in.i = bitcast i8* %stride.14.in.in.i to i32*
  %stride.14.i = load i32* %stride.14.in.i, align 4
  %"&(pSB[currWI].offset)194.i" = add nuw i64 %CurrSBIndex..5.i, 72
  %"&pSB[currWI].offset195.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)194.i"
  %CastToValueType196.i = bitcast i8* %"&pSB[currWI].offset195.i" to i32*
  store i32 %stride.14.i, i32* %CastToValueType196.i, align 4
  %"&(pSB[currWI].offset)180.i" = add nuw i64 %CurrSBIndex..5.i, 68
  %"&pSB[currWI].offset181.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)180.i"
  %CastToValueType182.i = bitcast i8* %"&pSB[currWI].offset181.i" to i32*
  store i32 %d67.05.i, i32* %CastToValueType182.i, align 4
  %shr74.i = lshr i32 %stride.14.i, 1
  %"&(pSB[currWI].offset)203.i" = add nuw i64 %CurrSBIndex..5.i, 76
  %"&pSB[currWI].offset204.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)203.i"
  %CastToValueType205.i = bitcast i8* %"&pSB[currWI].offset204.i" to i32*
  store i32 %shr74.i, i32* %CastToValueType205.i, align 4
  %check.WI.iter238.i = icmp ult i64 %CurrWI..5.i, %34
  br i1 %check.WI.iter238.i, label %thenBB235.i, label %SyncBB222.i

thenBB235.i:                                      ; preds = %for.body73.i
  %"CurrWI++239.i" = add nuw i64 %CurrWI..5.i, 1
  %"loadedCurrSB+Stride241.i" = add nuw i64 %CurrSBIndex..5.i, 80
  %cond2.i = icmp eq i32 %currBarrier.5.i, 1
  br i1 %cond2.i, label %SyncBB.i, label %SyncBB222.i

SyncBB222.i:                                      ; preds = %thenBB243.i, %thenBB235.i, %for.body73.i
  %CurrWI..6.i = phi i64 [ %"CurrWI++247.i", %thenBB243.i ], [ %"CurrWI++239.i", %thenBB235.i ], [ 0, %for.body73.i ]
  %CurrSBIndex..6.i = phi i64 [ %"loadedCurrSB+Stride249.i", %thenBB243.i ], [ %"loadedCurrSB+Stride241.i", %thenBB235.i ], [ 0, %for.body73.i ]
  %currBarrier.6.i = phi i32 [ %currBarrier.7.i, %thenBB243.i ], [ %currBarrier.5.i, %thenBB235.i ], [ 4, %for.body73.i ]
  %"&(pSB[currWI].offset)59.i" = add nuw i64 %CurrSBIndex..6.i, 16
  %"&pSB[currWI].offset60.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)59.i"
  %CastToValueType61.i = bitcast i8* %"&pSB[currWI].offset60.i" to i32*
  %loadedValue62.i = load i32* %CastToValueType61.i, align 4
  %"&(pSB[currWI].offset)184.i" = add nuw i64 %CurrSBIndex..6.i, 68
  %"&pSB[currWI].offset185.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)184.i"
  %CastToValueType186.i = bitcast i8* %"&pSB[currWI].offset185.i" to i32*
  %loadedValue187.i = load i32* %CastToValueType186.i, align 4
  %cmp75.i = icmp slt i32 %loadedValue62.i, %loadedValue187.i
  br i1 %cmp75.i, label %if.then77.i, label %SyncBB222.for.inc96_crit_edge.i

SyncBB222.for.inc96_crit_edge.i:                  ; preds = %SyncBB222.i
  %"&(pSB[currWI].offset)207.pre.i" = add nuw i64 %CurrSBIndex..6.i, 76
  br label %for.inc96.i

if.then77.i:                                      ; preds = %SyncBB222.i
  %"&(pSB[currWI].offset)198.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset199.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)198.i"
  %CastToValueType200.i = bitcast i8* %"&pSB[currWI].offset199.i" to i32*
  %loadedValue201.i = load i32* %CastToValueType200.i, align 4
  %mul79.i = and i32 %loadedValue201.i, -2
  %mul80.i = mul i32 %mul79.i, %loadedValue62.i
  %"&(pSB[currWI].offset)217.i" = add nuw i64 %CurrSBIndex..6.i, 76
  %"&pSB[currWI].offset218.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)217.i"
  %CastToValueType219.i = bitcast i8* %"&pSB[currWI].offset218.i" to i32*
  %loadedValue220.i = load i32* %CastToValueType219.i, align 4
  %add82.i = add i32 %loadedValue220.i, -1
  %sub83.i = add i32 %add82.i, %mul80.i
  %add85.i = add i32 %sub83.i, %loadedValue220.i
  %idxprom86.i = sext i32 %sub83.i to i64
  %arrayidx87.i = getelementptr inbounds float addrspace(3)* %22, i64 %idxprom86.i
  %51 = load float addrspace(3)* %arrayidx87.i, align 4
  %idxprom88.i = sext i32 %add85.i to i64
  %arrayidx89.i = getelementptr inbounds float addrspace(3)* %22, i64 %idxprom88.i
  %52 = load float addrspace(3)* %arrayidx89.i, align 4
  store float %52, float addrspace(3)* %arrayidx87.i, align 4
  %53 = load float addrspace(3)* %arrayidx89.i, align 4
  %add94.i = fadd float %53, %51
  store float %add94.i, float addrspace(3)* %arrayidx89.i, align 4
  %loadedValue192.pre.i = load i32* %CastToValueType186.i, align 4
  br label %for.inc96.i

for.inc96.i:                                      ; preds = %if.then77.i, %SyncBB222.for.inc96_crit_edge.i
  %"&(pSB[currWI].offset)207.pre-phi.i" = phi i64 [ %"&(pSB[currWI].offset)207.pre.i", %SyncBB222.for.inc96_crit_edge.i ], [ %"&(pSB[currWI].offset)217.i", %if.then77.i ]
  %loadedValue192.i = phi i32 [ %loadedValue187.i, %SyncBB222.for.inc96_crit_edge.i ], [ %loadedValue192.pre.i, %if.then77.i ]
  %mul97.i = shl nsw i32 %loadedValue192.i, 1
  %conv69.i = sext i32 %mul97.i to i64
  %"&pSB[currWI].offset17.i" = getelementptr inbounds i8* %37, i64 %CurrSBIndex..6.i
  %CastToValueType18.i = bitcast i8* %"&pSB[currWI].offset17.i" to i64*
  %loadedValue19.i = load i64* %CastToValueType18.i, align 8
  %cmp71.i = icmp ugt i64 %conv69.i, %loadedValue19.i
  br i1 %cmp71.i, label %for.end98.i, label %for.body73.i

for.end98.i:                                      ; preds = %for.inc96.i, %SyncBB.i
  %CurrWI..7.i = phi i64 [ %CurrWI..4.i, %SyncBB.i ], [ %CurrWI..6.i, %for.inc96.i ]
  %CurrSBIndex..7.i = phi i64 [ %CurrSBIndex..4.i, %SyncBB.i ], [ %CurrSBIndex..6.i, %for.inc96.i ]
  %currBarrier.7.i = phi i32 [ %currBarrier.4.i, %SyncBB.i ], [ %currBarrier.6.i, %for.inc96.i ]
  %check.WI.iter246.i = icmp ult i64 %CurrWI..7.i, %34
  br i1 %check.WI.iter246.i, label %thenBB243.i, label %SyncBB223.i

thenBB243.i:                                      ; preds = %for.end98.i
  %"CurrWI++247.i" = add nuw i64 %CurrWI..7.i, 1
  %"loadedCurrSB+Stride249.i" = add nuw i64 %CurrSBIndex..7.i, 80
  %cond1.i = icmp eq i32 %currBarrier.7.i, 4
  br i1 %cond1.i, label %SyncBB222.i, label %SyncBB.i

SyncBB223.i:                                      ; preds = %thenBB251.i, %for.end98.i
  %CurrWI..8.i = phi i64 [ %"CurrWI++255.i", %thenBB251.i ], [ 0, %for.end98.i ]
  %CurrSBIndex..8.i = phi i64 [ %"loadedCurrSB+Stride257.i", %thenBB251.i ], [ 0, %for.end98.i ]
  %"&(pSB[currWI].offset)110.i" = add nuw i64 %CurrSBIndex..8.i, 40
  %"&pSB[currWI].offset111.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)110.i"
  %CastToValueType112.i = bitcast i8* %"&pSB[currWI].offset111.i" to float addrspace(3)**
  %loadedValue113.i = load float addrspace(3)** %CastToValueType112.i, align 8
  %54 = load float addrspace(3)* %loadedValue113.i, align 4
  %"&(pSB[currWI].offset)101.i" = add nuw i64 %CurrSBIndex..8.i, 32
  %"&pSB[currWI].offset102.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)101.i"
  %CastToValueType103.i = bitcast i8* %"&pSB[currWI].offset102.i" to i64*
  %loadedValue104.i = load i64* %CastToValueType103.i, align 8
  %arrayidx102.i = getelementptr inbounds float addrspace(1)* %1, i64 %loadedValue104.i
  store float %54, float addrspace(1)* %arrayidx102.i, align 4
  %"&(pSB[currWI].offset)119.i" = add nuw i64 %CurrSBIndex..8.i, 48
  %"&pSB[currWI].offset120.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)119.i"
  %CastToValueType121.i = bitcast i8* %"&pSB[currWI].offset120.i" to i1*
  %loadedValue122.i = load i1* %CastToValueType121.i, align 1
  br i1 %loadedValue122.i, label %if.then105.i, label %if.end110.i

if.then105.i:                                     ; preds = %SyncBB223.i
  %"&(pSB[currWI].offset)92.i" = add nuw i64 %CurrSBIndex..8.i, 24
  %"&pSB[currWI].offset93.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)92.i"
  %CastToValueType94.i = bitcast i8* %"&pSB[currWI].offset93.i" to i32*
  %loadedValue95.i = load i32* %CastToValueType94.i, align 4
  %idxprom106.i = sext i32 %loadedValue95.i to i64
  %arrayidx107.i = getelementptr inbounds float addrspace(3)* %22, i64 %idxprom106.i
  %55 = load float addrspace(3)* %arrayidx107.i, align 4
  %"&(pSB[currWI].offset)73.i" = add nuw i64 %CurrSBIndex..8.i, 20
  %"&pSB[currWI].offset74.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)73.i"
  %CastToValueType75.i = bitcast i8* %"&pSB[currWI].offset74.i" to i32*
  %loadedValue76.i = load i32* %CastToValueType75.i, align 4
  %idxprom108.i = sext i32 %loadedValue76.i to i64
  %arrayidx109.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom108.i
  store float %55, float addrspace(1)* %arrayidx109.i, align 4
  br label %if.end110.i

if.end110.i:                                      ; preds = %if.then105.i, %SyncBB223.i
  %check.WI.iter254.i = icmp ult i64 %CurrWI..8.i, %34
  br i1 %check.WI.iter254.i, label %thenBB251.i, label %__scan_separated_args.exit

thenBB251.i:                                      ; preds = %if.end110.i
  %"CurrWI++255.i" = add nuw i64 %CurrWI..8.i, 1
  %"loadedCurrSB+Stride257.i" = add nuw i64 %CurrSBIndex..8.i, 80
  br label %SyncBB223.i

__scan_separated_args.exit:                       ; preds = %if.end110.i
  ret void
}

define void @uniformAdd(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 20
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 24
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %16 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 48
  %18 = bitcast i8* %17 to i64**
  %19 = load i64** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 64
  %21 = bitcast i8* %20 to <{ [4 x i64] }>**
  %22 = load <{ [4 x i64] }>** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 80
  %24 = bitcast i8* %23 to i64*
  %25 = load i64* %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 88
  %27 = bitcast i8* %26 to i8**
  %28 = load i8** %27, align 8
  %conv.i = sext i32 %10 to i64
  %conv31.i = zext i32 %13 to i64
  br label %SyncBB20.i

SyncBB20.i:                                       ; preds = %thenBB.i, %entry
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %29 = load i64* %19, align 8
  %add.i = add i64 %29, %conv.i
  %arrayidx.i = getelementptr inbounds float addrspace(1)* %4, i64 %add.i
  %30 = load float addrspace(1)* %arrayidx.i, align 4
  %31 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %16, i64 0, i32 3, i64 0
  %32 = load i64* %31, align 8
  %shl.i = shl i64 %29, 1
  %mul.i = mul i64 %shl.i, %32
  %33 = getelementptr <{ [4 x i64] }>* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %34 = load i64* %33, align 8
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %28, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i64*
  store i64 %34, i64* %CastToValueType.i, align 8
  %add4.i = add i64 %34, %conv31.i
  %add6.i = add i64 %add4.i, %mul.i
  %"&(pSB[currWI].offset)71.i" = or i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset8.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)71.i"
  %CastToValueType9.i = bitcast i8* %"&pSB[currWI].offset8.i" to i64*
  store i64 %add6.i, i64* %CastToValueType9.i, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %25
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %SyncBB20.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 16
  br label %SyncBB20.i

elseBB.i:                                         ; preds = %SyncBB20.i
  %conv13.i = sext i32 %7 to i64
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB23.i, %elseBB.i
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride29.i", %thenBB23.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++27.i", %thenBB23.i ]
  %"&(pSB[currWI].offset)112.i" = or i64 %CurrSBIndex..1.i, 8
  %"&pSB[currWI].offset12.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)112.i"
  %CastToValueType13.i = bitcast i8* %"&pSB[currWI].offset12.i" to i64*
  %loadedValue14.i = load i64* %CastToValueType13.i, align 8
  %idxprom.i = and i64 %loadedValue14.i, 4294967295
  %arrayidx8.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom.i
  %35 = load float addrspace(1)* %arrayidx8.i, align 4
  %add9.i = fadd float %35, %30
  store float %add9.i, float addrspace(1)* %arrayidx8.i, align 4
  %"&pSB[currWI].offset4.i" = getelementptr inbounds i8* %28, i64 %CurrSBIndex..1.i
  %CastToValueType5.i = bitcast i8* %"&pSB[currWI].offset4.i" to i64*
  %loadedValue.i = load i64* %CastToValueType5.i, align 8
  %add12.i = add i64 %loadedValue.i, %32
  %cmp.i = icmp ult i64 %add12.i, %conv13.i
  br i1 %cmp.i, label %if.then.i, label %if.end.i

if.then.i:                                        ; preds = %SyncBB.i
  %add17.i = add i64 %idxprom.i, %32
  %arrayidx18.i = getelementptr inbounds float addrspace(1)* %1, i64 %add17.i
  %36 = load float addrspace(1)* %arrayidx18.i, align 4
  %add19.i = fadd float %36, %30
  store float %add19.i, float addrspace(1)* %arrayidx18.i, align 4
  br label %if.end.i

if.end.i:                                         ; preds = %if.then.i, %SyncBB.i
  %check.WI.iter26.i = icmp ult i64 %CurrWI..1.i, %25
  br i1 %check.WI.iter26.i, label %thenBB23.i, label %__uniformAdd_separated_args.exit

thenBB23.i:                                       ; preds = %if.end.i
  %"CurrWI++27.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride29.i" = add nuw i64 %CurrSBIndex..1.i, 16
  br label %SyncBB.i

__uniformAdd_separated_args.exit:                 ; preds = %if.end.i
  ret void
}

define void @__Vectorized_.uniformAdd(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 20
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 24
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %16 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 48
  %18 = bitcast i8* %17 to i64**
  %19 = load i64** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 64
  %21 = bitcast i8* %20 to <{ [4 x i64] }>**
  %22 = load <{ [4 x i64] }>** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 80
  %24 = bitcast i8* %23 to i64*
  %25 = load i64* %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 88
  %27 = bitcast i8* %26 to i8**
  %28 = load i8** %27, align 8
  %conv.i = sext i32 %10 to i64
  %conv31.i = zext i32 %13 to i64
  %temp.i = insertelement <16 x i64> undef, i64 %conv31.i, i32 0
  %vector.i = shufflevector <16 x i64> %temp.i, <16 x i64> undef, <16 x i32> zeroinitializer
  br label %SyncBB302.i

SyncBB302.i:                                      ; preds = %thenBB.i, %entry
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %29 = load i64* %19, align 8
  %add.i = add i64 %29, %conv.i
  %arrayidx.i = getelementptr inbounds float addrspace(1)* %4, i64 %add.i
  %30 = load float addrspace(1)* %arrayidx.i, align 4
  %31 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %16, i64 0, i32 3, i64 0
  %32 = load i64* %31, align 8
  %shl.i = shl i64 %29, 1
  %mul.i = mul i64 %shl.i, %32
  %temp3.i = insertelement <16 x i64> undef, i64 %mul.i, i32 0
  %vector4.i = shufflevector <16 x i64> %temp3.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %33 = getelementptr <{ [4 x i64] }>* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %34 = load i64* %33, align 8
  %broadcast1.i = insertelement <16 x i64> undef, i64 %34, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %35 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %28, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to <16 x i64>*
  store <16 x i64> %35, <16 x i64>* %CastToValueType.i, align 128
  %add42.i = add <16 x i64> %35, %vector.i
  %add65.i = add <16 x i64> %add42.i, %vector4.i
  %"&(pSB[currWI].offset)2881.i" = or i64 %CurrSBIndex..0.i, 128
  %"&pSB[currWI].offset289.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)2881.i"
  %CastToValueType290.i = bitcast i8* %"&pSB[currWI].offset289.i" to <16 x i64>*
  store <16 x i64> %add65.i, <16 x i64>* %CastToValueType290.i, align 128
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %25
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %SyncBB302.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 256
  br label %SyncBB302.i

elseBB.i:                                         ; preds = %SyncBB302.i
  %temp22.i = insertelement <16 x float> undef, float %30, i32 0
  %vector23.i = shufflevector <16 x float> %temp22.i, <16 x float> undef, <16 x i32> zeroinitializer
  %temp26.i = insertelement <16 x i64> undef, i64 %32, i32 0
  %vector27.i = shufflevector <16 x i64> %temp26.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %conv13.i = sext i32 %7 to i64
  %temp29.i = insertelement <16 x i64> undef, i64 %conv13.i, i32 0
  %vector30.i = shufflevector <16 x i64> %temp29.i, <16 x i64> undef, <16 x i32> zeroinitializer
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB305.i, %elseBB.i
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride311.i", %thenBB305.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++309.i", %thenBB305.i ]
  %"&(pSB[currWI].offset)2922.i" = or i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset293.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)2922.i"
  %CastToValueType294.i = bitcast i8* %"&pSB[currWI].offset293.i" to <16 x i64>*
  %loadedValue295.i = load <16 x i64>* %CastToValueType294.i, align 128
  %idxprom6.i = and <16 x i64> %loadedValue295.i, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract.i = extractelement <16 x i64> %idxprom6.i, i32 0
  %36 = getelementptr inbounds float addrspace(1)* %1, i64 %extract.i
  %ptrTypeCast.i = bitcast float addrspace(1)* %36 to <16 x float> addrspace(1)*
  %37 = load <16 x float> addrspace(1)* %ptrTypeCast.i, align 4
  %add924.i = fadd <16 x float> %37, %vector23.i
  store <16 x float> %add924.i, <16 x float> addrspace(1)* %ptrTypeCast.i, align 4
  %"&pSB[currWI].offset285.i" = getelementptr inbounds i8* %28, i64 %CurrSBIndex..1.i
  %CastToValueType286.i = bitcast i8* %"&pSB[currWI].offset285.i" to <16 x i64>*
  %loadedValue.i = load <16 x i64>* %CastToValueType286.i, align 128
  %add1228.i = add <16 x i64> %loadedValue.i, %vector27.i
  %cmp.i = icmp ult <16 x i64> %add1228.i, %vector30.i
  %add1732.i = add <16 x i64> %idxprom6.i, %vector27.i
  %extract33.i = extractelement <16 x i64> %add1732.i, i32 0
  %38 = getelementptr inbounds float addrspace(1)* %1, i64 %extract33.i
  %exmask.i = extractelement <16 x i1> %cmp.i, i32 0
  br i1 %exmask.i, label %preload206.i, label %postload207.i

preload206.i:                                     ; preds = %SyncBB.i
  %vload53.i = load float addrspace(1)* %38, align 4
  br label %postload207.i

postload207.i:                                    ; preds = %preload206.i, %SyncBB.i
  %phi208.i = phi float [ undef, %SyncBB.i ], [ %vload53.i, %preload206.i ]
  %vpack.i = insertelement <16 x float> undef, float %phi208.i, i32 0
  %exmask55.i = extractelement <16 x i1> %cmp.i, i32 1
  br i1 %exmask55.i, label %preload203.i, label %postload204.i

preload203.i:                                     ; preds = %postload207.i
  %.sum282.i = add i64 %extract33.i, 1
  %39 = getelementptr float addrspace(1)* %1, i64 %.sum282.i
  %vload56.i = load float addrspace(1)* %39, align 4
  br label %postload204.i

postload204.i:                                    ; preds = %preload203.i, %postload207.i
  %phi205.i = phi float [ undef, %postload207.i ], [ %vload56.i, %preload203.i ]
  %vpack57.i = insertelement <16 x float> %vpack.i, float %phi205.i, i32 1
  %exmask59.i = extractelement <16 x i1> %cmp.i, i32 2
  br i1 %exmask59.i, label %preload200.i, label %postload201.i

preload200.i:                                     ; preds = %postload204.i
  %.sum281.i = add i64 %extract33.i, 2
  %40 = getelementptr float addrspace(1)* %1, i64 %.sum281.i
  %vload60.i = load float addrspace(1)* %40, align 4
  br label %postload201.i

postload201.i:                                    ; preds = %preload200.i, %postload204.i
  %phi202.i = phi float [ undef, %postload204.i ], [ %vload60.i, %preload200.i ]
  %vpack61.i = insertelement <16 x float> %vpack57.i, float %phi202.i, i32 2
  %exmask63.i = extractelement <16 x i1> %cmp.i, i32 3
  br i1 %exmask63.i, label %preload197.i, label %postload198.i

preload197.i:                                     ; preds = %postload201.i
  %.sum280.i = add i64 %extract33.i, 3
  %41 = getelementptr float addrspace(1)* %1, i64 %.sum280.i
  %vload64.i = load float addrspace(1)* %41, align 4
  br label %postload198.i

postload198.i:                                    ; preds = %preload197.i, %postload201.i
  %phi199.i = phi float [ undef, %postload201.i ], [ %vload64.i, %preload197.i ]
  %vpack65.i = insertelement <16 x float> %vpack61.i, float %phi199.i, i32 3
  %exmask67.i = extractelement <16 x i1> %cmp.i, i32 4
  br i1 %exmask67.i, label %preload188.i, label %postload189.i

preload188.i:                                     ; preds = %postload198.i
  %.sum279.i = add i64 %extract33.i, 4
  %42 = getelementptr float addrspace(1)* %1, i64 %.sum279.i
  %vload68.i = load float addrspace(1)* %42, align 4
  br label %postload189.i

postload189.i:                                    ; preds = %preload188.i, %postload198.i
  %phi190.i = phi float [ undef, %postload198.i ], [ %vload68.i, %preload188.i ]
  %vpack69.i = insertelement <16 x float> %vpack65.i, float %phi190.i, i32 4
  %exmask71.i = extractelement <16 x i1> %cmp.i, i32 5
  br i1 %exmask71.i, label %preload.i, label %postload.i

preload.i:                                        ; preds = %postload189.i
  %.sum278.i = add i64 %extract33.i, 5
  %43 = getelementptr float addrspace(1)* %1, i64 %.sum278.i
  %vload72.i = load float addrspace(1)* %43, align 4
  br label %postload.i

postload.i:                                       ; preds = %preload.i, %postload189.i
  %phi.i = phi float [ undef, %postload189.i ], [ %vload72.i, %preload.i ]
  %vpack73.i = insertelement <16 x float> %vpack69.i, float %phi.i, i32 5
  %exmask75.i = extractelement <16 x i1> %cmp.i, i32 6
  br i1 %exmask75.i, label %preload176.i, label %postload177.i

preload176.i:                                     ; preds = %postload.i
  %.sum277.i = add i64 %extract33.i, 6
  %44 = getelementptr float addrspace(1)* %1, i64 %.sum277.i
  %vload76.i = load float addrspace(1)* %44, align 4
  br label %postload177.i

postload177.i:                                    ; preds = %preload176.i, %postload.i
  %phi178.i = phi float [ undef, %postload.i ], [ %vload76.i, %preload176.i ]
  %vpack77.i = insertelement <16 x float> %vpack73.i, float %phi178.i, i32 6
  %exmask79.i = extractelement <16 x i1> %cmp.i, i32 7
  br i1 %exmask79.i, label %preload173.i, label %postload174.i

preload173.i:                                     ; preds = %postload177.i
  %.sum276.i = add i64 %extract33.i, 7
  %45 = getelementptr float addrspace(1)* %1, i64 %.sum276.i
  %vload80.i = load float addrspace(1)* %45, align 4
  br label %postload174.i

postload174.i:                                    ; preds = %preload173.i, %postload177.i
  %phi175.i = phi float [ undef, %postload177.i ], [ %vload80.i, %preload173.i ]
  %vpack81.i = insertelement <16 x float> %vpack77.i, float %phi175.i, i32 7
  %exmask83.i = extractelement <16 x i1> %cmp.i, i32 8
  br i1 %exmask83.i, label %preload170.i, label %postload171.i

preload170.i:                                     ; preds = %postload174.i
  %.sum275.i = add i64 %extract33.i, 8
  %46 = getelementptr float addrspace(1)* %1, i64 %.sum275.i
  %vload84.i = load float addrspace(1)* %46, align 4
  br label %postload171.i

postload171.i:                                    ; preds = %preload170.i, %postload174.i
  %phi172.i = phi float [ undef, %postload174.i ], [ %vload84.i, %preload170.i ]
  %vpack85.i = insertelement <16 x float> %vpack81.i, float %phi172.i, i32 8
  %exmask87.i = extractelement <16 x i1> %cmp.i, i32 9
  br i1 %exmask87.i, label %preload167.i, label %postload168.i

preload167.i:                                     ; preds = %postload171.i
  %.sum274.i = add i64 %extract33.i, 9
  %47 = getelementptr float addrspace(1)* %1, i64 %.sum274.i
  %vload88.i = load float addrspace(1)* %47, align 4
  br label %postload168.i

postload168.i:                                    ; preds = %preload167.i, %postload171.i
  %phi169.i = phi float [ undef, %postload171.i ], [ %vload88.i, %preload167.i ]
  %vpack89.i = insertelement <16 x float> %vpack85.i, float %phi169.i, i32 9
  %exmask91.i = extractelement <16 x i1> %cmp.i, i32 10
  br i1 %exmask91.i, label %preload161.i, label %postload162.i

preload161.i:                                     ; preds = %postload168.i
  %.sum273.i = add i64 %extract33.i, 10
  %48 = getelementptr float addrspace(1)* %1, i64 %.sum273.i
  %vload92.i = load float addrspace(1)* %48, align 4
  br label %postload162.i

postload162.i:                                    ; preds = %preload161.i, %postload168.i
  %phi163.i = phi float [ undef, %postload168.i ], [ %vload92.i, %preload161.i ]
  %vpack93.i = insertelement <16 x float> %vpack89.i, float %phi163.i, i32 10
  %exmask95.i = extractelement <16 x i1> %cmp.i, i32 11
  br i1 %exmask95.i, label %preload164.i, label %postload165.i

preload164.i:                                     ; preds = %postload162.i
  %.sum272.i = add i64 %extract33.i, 11
  %49 = getelementptr float addrspace(1)* %1, i64 %.sum272.i
  %vload96.i = load float addrspace(1)* %49, align 4
  br label %postload165.i

postload165.i:                                    ; preds = %preload164.i, %postload162.i
  %phi166.i = phi float [ undef, %postload162.i ], [ %vload96.i, %preload164.i ]
  %vpack97.i = insertelement <16 x float> %vpack93.i, float %phi166.i, i32 11
  %exmask99.i = extractelement <16 x i1> %cmp.i, i32 12
  br i1 %exmask99.i, label %preload191.i, label %postload192.i

preload191.i:                                     ; preds = %postload165.i
  %.sum271.i = add i64 %extract33.i, 12
  %50 = getelementptr float addrspace(1)* %1, i64 %.sum271.i
  %vload100.i = load float addrspace(1)* %50, align 4
  br label %postload192.i

postload192.i:                                    ; preds = %preload191.i, %postload165.i
  %phi193.i = phi float [ undef, %postload165.i ], [ %vload100.i, %preload191.i ]
  %vpack101.i = insertelement <16 x float> %vpack97.i, float %phi193.i, i32 12
  %exmask103.i = extractelement <16 x i1> %cmp.i, i32 13
  br i1 %exmask103.i, label %preload194.i, label %postload195.i

preload194.i:                                     ; preds = %postload192.i
  %.sum270.i = add i64 %extract33.i, 13
  %51 = getelementptr float addrspace(1)* %1, i64 %.sum270.i
  %vload104.i = load float addrspace(1)* %51, align 4
  br label %postload195.i

postload195.i:                                    ; preds = %preload194.i, %postload192.i
  %phi196.i = phi float [ undef, %postload192.i ], [ %vload104.i, %preload194.i ]
  %vpack105.i = insertelement <16 x float> %vpack101.i, float %phi196.i, i32 13
  %exmask107.i = extractelement <16 x i1> %cmp.i, i32 14
  br i1 %exmask107.i, label %preload179.i, label %postload180.i

preload179.i:                                     ; preds = %postload195.i
  %.sum269.i = add i64 %extract33.i, 14
  %52 = getelementptr float addrspace(1)* %1, i64 %.sum269.i
  %vload108.i = load float addrspace(1)* %52, align 4
  br label %postload180.i

postload180.i:                                    ; preds = %preload179.i, %postload195.i
  %phi181.i = phi float [ undef, %postload195.i ], [ %vload108.i, %preload179.i ]
  %vpack109.i = insertelement <16 x float> %vpack105.i, float %phi181.i, i32 14
  %exmask111.i = extractelement <16 x i1> %cmp.i, i32 15
  br i1 %exmask111.i, label %preload182.i, label %postload183.i

preload182.i:                                     ; preds = %postload180.i
  %.sum268.i = add i64 %extract33.i, 15
  %53 = getelementptr float addrspace(1)* %1, i64 %.sum268.i
  %vload112.i = load float addrspace(1)* %53, align 4
  br label %postload183.i

postload183.i:                                    ; preds = %preload182.i, %postload180.i
  %phi184.i = phi float [ undef, %postload180.i ], [ %vload112.i, %preload182.i ]
  %vpack113.i = insertelement <16 x float> %vpack109.i, float %phi184.i, i32 15
  %add1950.i = fadd <16 x float> %vpack113.i, %vector23.i
  br i1 %exmask.i, label %preload185.i, label %postload186.i

preload185.i:                                     ; preds = %postload183.i
  %exData.i = extractelement <16 x float> %add1950.i, i32 0
  store float %exData.i, float addrspace(1)* %38, align 4
  br label %postload186.i

postload186.i:                                    ; preds = %preload185.i, %postload183.i
  br i1 %exmask55.i, label %preload209.i, label %postload210.i

preload209.i:                                     ; preds = %postload186.i
  %.sum267.i = add i64 %extract33.i, 1
  %54 = getelementptr float addrspace(1)* %1, i64 %.sum267.i
  %exData118.i = extractelement <16 x float> %add1950.i, i32 1
  store float %exData118.i, float addrspace(1)* %54, align 4
  br label %postload210.i

postload210.i:                                    ; preds = %preload209.i, %postload186.i
  br i1 %exmask59.i, label %preload212.i, label %postload213.i

preload212.i:                                     ; preds = %postload210.i
  %.sum266.i = add i64 %extract33.i, 2
  %55 = getelementptr float addrspace(1)* %1, i64 %.sum266.i
  %exData121.i = extractelement <16 x float> %add1950.i, i32 2
  store float %exData121.i, float addrspace(1)* %55, align 4
  br label %postload213.i

postload213.i:                                    ; preds = %preload212.i, %postload210.i
  br i1 %exmask63.i, label %preload215.i, label %postload216.i

preload215.i:                                     ; preds = %postload213.i
  %.sum265.i = add i64 %extract33.i, 3
  %56 = getelementptr float addrspace(1)* %1, i64 %.sum265.i
  %exData124.i = extractelement <16 x float> %add1950.i, i32 3
  store float %exData124.i, float addrspace(1)* %56, align 4
  br label %postload216.i

postload216.i:                                    ; preds = %preload215.i, %postload213.i
  br i1 %exmask67.i, label %preload218.i, label %postload219.i

preload218.i:                                     ; preds = %postload216.i
  %.sum264.i = add i64 %extract33.i, 4
  %57 = getelementptr float addrspace(1)* %1, i64 %.sum264.i
  %exData127.i = extractelement <16 x float> %add1950.i, i32 4
  store float %exData127.i, float addrspace(1)* %57, align 4
  br label %postload219.i

postload219.i:                                    ; preds = %preload218.i, %postload216.i
  br i1 %exmask71.i, label %preload221.i, label %postload222.i

preload221.i:                                     ; preds = %postload219.i
  %.sum263.i = add i64 %extract33.i, 5
  %58 = getelementptr float addrspace(1)* %1, i64 %.sum263.i
  %exData130.i = extractelement <16 x float> %add1950.i, i32 5
  store float %exData130.i, float addrspace(1)* %58, align 4
  br label %postload222.i

postload222.i:                                    ; preds = %preload221.i, %postload219.i
  br i1 %exmask75.i, label %preload224.i, label %postload225.i

preload224.i:                                     ; preds = %postload222.i
  %.sum262.i = add i64 %extract33.i, 6
  %59 = getelementptr float addrspace(1)* %1, i64 %.sum262.i
  %exData133.i = extractelement <16 x float> %add1950.i, i32 6
  store float %exData133.i, float addrspace(1)* %59, align 4
  br label %postload225.i

postload225.i:                                    ; preds = %preload224.i, %postload222.i
  br i1 %exmask79.i, label %preload227.i, label %postload228.i

preload227.i:                                     ; preds = %postload225.i
  %.sum261.i = add i64 %extract33.i, 7
  %60 = getelementptr float addrspace(1)* %1, i64 %.sum261.i
  %exData136.i = extractelement <16 x float> %add1950.i, i32 7
  store float %exData136.i, float addrspace(1)* %60, align 4
  br label %postload228.i

postload228.i:                                    ; preds = %preload227.i, %postload225.i
  br i1 %exmask83.i, label %preload230.i, label %postload231.i

preload230.i:                                     ; preds = %postload228.i
  %.sum260.i = add i64 %extract33.i, 8
  %61 = getelementptr float addrspace(1)* %1, i64 %.sum260.i
  %exData139.i = extractelement <16 x float> %add1950.i, i32 8
  store float %exData139.i, float addrspace(1)* %61, align 4
  br label %postload231.i

postload231.i:                                    ; preds = %preload230.i, %postload228.i
  br i1 %exmask87.i, label %preload233.i, label %postload234.i

preload233.i:                                     ; preds = %postload231.i
  %.sum259.i = add i64 %extract33.i, 9
  %62 = getelementptr float addrspace(1)* %1, i64 %.sum259.i
  %exData142.i = extractelement <16 x float> %add1950.i, i32 9
  store float %exData142.i, float addrspace(1)* %62, align 4
  br label %postload234.i

postload234.i:                                    ; preds = %preload233.i, %postload231.i
  br i1 %exmask91.i, label %preload236.i, label %postload237.i

preload236.i:                                     ; preds = %postload234.i
  %.sum258.i = add i64 %extract33.i, 10
  %63 = getelementptr float addrspace(1)* %1, i64 %.sum258.i
  %exData145.i = extractelement <16 x float> %add1950.i, i32 10
  store float %exData145.i, float addrspace(1)* %63, align 4
  br label %postload237.i

postload237.i:                                    ; preds = %preload236.i, %postload234.i
  br i1 %exmask95.i, label %preload239.i, label %postload240.i

preload239.i:                                     ; preds = %postload237.i
  %.sum257.i = add i64 %extract33.i, 11
  %64 = getelementptr float addrspace(1)* %1, i64 %.sum257.i
  %exData148.i = extractelement <16 x float> %add1950.i, i32 11
  store float %exData148.i, float addrspace(1)* %64, align 4
  br label %postload240.i

postload240.i:                                    ; preds = %preload239.i, %postload237.i
  br i1 %exmask99.i, label %preload242.i, label %postload243.i

preload242.i:                                     ; preds = %postload240.i
  %.sum256.i = add i64 %extract33.i, 12
  %65 = getelementptr float addrspace(1)* %1, i64 %.sum256.i
  %exData151.i = extractelement <16 x float> %add1950.i, i32 12
  store float %exData151.i, float addrspace(1)* %65, align 4
  br label %postload243.i

postload243.i:                                    ; preds = %preload242.i, %postload240.i
  br i1 %exmask103.i, label %preload245.i, label %postload246.i

preload245.i:                                     ; preds = %postload243.i
  %.sum255.i = add i64 %extract33.i, 13
  %66 = getelementptr float addrspace(1)* %1, i64 %.sum255.i
  %exData154.i = extractelement <16 x float> %add1950.i, i32 13
  store float %exData154.i, float addrspace(1)* %66, align 4
  br label %postload246.i

postload246.i:                                    ; preds = %preload245.i, %postload243.i
  br i1 %exmask107.i, label %preload248.i, label %postload249.i

preload248.i:                                     ; preds = %postload246.i
  %.sum254.i = add i64 %extract33.i, 14
  %67 = getelementptr float addrspace(1)* %1, i64 %.sum254.i
  %exData157.i = extractelement <16 x float> %add1950.i, i32 14
  store float %exData157.i, float addrspace(1)* %67, align 4
  br label %postload249.i

postload249.i:                                    ; preds = %preload248.i, %postload246.i
  br i1 %exmask111.i, label %preload251.i, label %if.end.i

preload251.i:                                     ; preds = %postload249.i
  %.sum.i = add i64 %extract33.i, 15
  %68 = getelementptr float addrspace(1)* %1, i64 %.sum.i
  %exData160.i = extractelement <16 x float> %add1950.i, i32 15
  store float %exData160.i, float addrspace(1)* %68, align 4
  br label %if.end.i

if.end.i:                                         ; preds = %preload251.i, %postload249.i
  %check.WI.iter308.i = icmp ult i64 %CurrWI..1.i, %25
  br i1 %check.WI.iter308.i, label %thenBB305.i, label %____Vectorized_.uniformAdd_separated_args.exit

thenBB305.i:                                      ; preds = %if.end.i
  %"CurrWI++309.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride311.i" = add nuw i64 %CurrSBIndex..1.i, 256
  br label %SyncBB.i

____Vectorized_.uniformAdd_separated_args.exit:   ; preds = %if.end.i
  ret void
}

define void @__Vectorized_.scan(i8* %pBuffer) {
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
  %11 = getelementptr i8* %pBuffer, i64 28
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 32
  %15 = bitcast i8* %14 to i32*
  %16 = load i32* %15, align 4
  %17 = getelementptr i8* %pBuffer, i64 36
  %18 = bitcast i8* %17 to i32*
  %19 = load i32* %18, align 4
  %20 = getelementptr i8* %pBuffer, i64 40
  %21 = bitcast i8* %20 to float addrspace(3)**
  %22 = load float addrspace(3)** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 56
  %24 = bitcast i8* %23 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %25 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 64
  %27 = bitcast i8* %26 to i64**
  %28 = load i64** %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 80
  %30 = bitcast i8* %29 to <{ [4 x i64] }>**
  %31 = load <{ [4 x i64] }>** %30, align 8
  %32 = getelementptr i8* %pBuffer, i64 96
  %33 = bitcast i8* %32 to i64*
  %34 = load i64* %33, align 8
  %35 = getelementptr i8* %pBuffer, i64 104
  %36 = bitcast i8* %35 to i8**
  %37 = load i8** %36, align 8
  %temp147.i = insertelement <16 x i32> undef, i32 %10, i32 0
  %vector148.i = shufflevector <16 x i32> %temp147.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %cmp.i = icmp eq i32 %16, 0
  %cmp43.i = icmp eq i32 %13, 0
  %conv461623.i = zext i32 %13 to i64
  %cmp56.i = icmp eq i32 %19, 1
  %temp421.i = insertelement <16 x i1> undef, i1 %cmp56.i, i32 0
  %vector422.i = shufflevector <16 x i1> %temp421.i, <16 x i1> undef, <16 x i32> zeroinitializer
  br label %SyncBB2701.i

SyncBB2701.i:                                     ; preds = %thenBB.i, %thenBB2713.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++2717.i", %thenBB2713.i ], [ %"CurrWI++.i", %thenBB.i ]
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride2719.i", %thenBB2713.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %currBarrier.0.i = phi i32 [ 17, %entry ], [ %currBarrier.1.i, %thenBB2713.i ], [ %currBarrier.4.i, %thenBB.i ]
  br i1 %cmp.i, label %if.then.i, label %preload1014.i

preload1014.i:                                    ; preds = %SyncBB2701.i
  %38 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %25, i64 0, i32 3, i64 0
  %39 = load i64* %38, align 8
  br label %if.then.i

if.then.i:                                        ; preds = %preload1014.i, %SyncBB2701.i
  %phi1016.i = phi i64 [ undef, %SyncBB2701.i ], [ %39, %preload1014.i ]
  br i1 %cmp.i, label %preload1017.i, label %if.end.i

preload1017.i:                                    ; preds = %if.then.i
  %40 = load i64* %28, align 8
  %41 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %25, i64 0, i32 3, i64 0
  %42 = load i64* %41, align 8
  %phitmp.i = shl i64 %40, 1
  br label %if.end.i

if.end.i:                                         ; preds = %preload1017.i, %if.then.i
  %phi1019.i = phi i64 [ 0, %if.then.i ], [ %phitmp.i, %preload1017.i ]
  %phi1020.i = phi i64 [ undef, %if.then.i ], [ %42, %preload1017.i ]
  %mul.i = mul i64 %phi1019.i, %phi1020.i
  %conv.i = trunc i64 %mul.i to i32
  %merge65.i = select i1 %cmp.i, i32 %conv.i, i32 %16
  %temp.i = insertelement <16 x i32> undef, i32 %merge65.i, i32 0
  %vector.i = shufflevector <16 x i32> %temp.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %merge.i = select i1 %cmp.i, i64 %phi1020.i, i64 %phi1016.i
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %37, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i64*
  store i64 %merge.i, i64* %CastToValueType.i, align 8
  %temp107.i = insertelement <16 x i64> undef, i64 %merge.i, i32 0
  %vector108.i = shufflevector <16 x i64> %temp107.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %43 = getelementptr <{ [4 x i64] }>* %31, i64 %CurrWI..0.i, i32 0, i64 0
  %44 = load i64* %43, align 8
  %broadcast1.i = insertelement <16 x i64> undef, i64 %44, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %45 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %"&(pSB[currWI].offset)1688.i" = add nuw i64 %CurrSBIndex..0.i, 128
  %"&pSB[currWI].offset1689.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1688.i"
  %CastToValueType1690.i = bitcast i8* %"&pSB[currWI].offset1689.i" to <16 x i64>*
  store <16 x i64> %45, <16 x i64>* %CastToValueType1690.i, align 128
  %conv3104.i = trunc <16 x i64> %45 to <16 x i32>
  %"&(pSB[currWI].offset)1697.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1698.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1697.i"
  %CastToValueType1699.i = bitcast i8* %"&pSB[currWI].offset1698.i" to <16 x i32>*
  store <16 x i32> %conv3104.i, <16 x i32>* %CastToValueType1699.i, align 64
  %add105.i = add nsw <16 x i32> %conv3104.i, %vector.i
  %conv41106.i = zext <16 x i32> %add105.i to <16 x i64>
  %add6109.i = add <16 x i64> %conv41106.i, %vector108.i
  %add10111.i = add <16 x i64> %vector108.i, %45
  %conv11112.i = trunc <16 x i64> %add10111.i to <16 x i32>
  %idxprom113.i = sext <16 x i32> %add105.i to <16 x i64>
  %extract.i = extractelement <16 x i64> %idxprom113.i, i32 0
  %"&(pSB[currWI].offset)1721.i" = add nuw i64 %CurrSBIndex..0.i, 320
  %"&pSB[currWI].offset1722.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1721.i"
  %CastToValueType1723.i = bitcast i8* %"&pSB[currWI].offset1722.i" to i64*
  store i64 %extract.i, i64* %CastToValueType1723.i, align 8
  %46 = getelementptr inbounds float addrspace(1)* %4, i64 %extract.i
  %ptrTypeCast.i = bitcast float addrspace(1)* %46 to <16 x float> addrspace(1)*
  %47 = load <16 x float> addrspace(1)* %ptrTypeCast.i, align 4
  %48 = extractelement <16 x i32> %conv3104.i, i32 0
  %extract130.i = sext i32 %48 to i64
  %49 = getelementptr inbounds float addrspace(3)* %22, i64 %extract130.i
  %ptrTypeCast146.i = bitcast float addrspace(3)* %49 to <16 x float> addrspace(3)*
  %"&(pSB[currWI].offset)1730.i" = add nuw i64 %CurrSBIndex..0.i, 328
  %"&pSB[currWI].offset1731.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1730.i"
  %CastToValueType1732.i = bitcast i8* %"&pSB[currWI].offset1731.i" to <16 x float> addrspace(3)**
  store <16 x float> addrspace(3)* %ptrTypeCast146.i, <16 x float> addrspace(3)** %CastToValueType1732.i, align 8
  store <16 x float> %47, <16 x float> addrspace(3)* %ptrTypeCast146.i, align 4
  %cmp14.i = icmp slt <16 x i32> %conv11112.i, %vector148.i
  %Mneg2149.i = xor <16 x i1> %cmp14.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %extract170.i = extractelement <16 x i1> %cmp14.i, i32 1
  %"&(pSB[currWI].offset)1739.i" = add nuw i64 %CurrSBIndex..0.i, 336
  %"&pSB[currWI].offset1740.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1739.i"
  %CastToValueType1741.i = bitcast i8* %"&pSB[currWI].offset1740.i" to i1*
  store i1 %extract170.i, i1* %CastToValueType1741.i, align 1
  %extract171.i = extractelement <16 x i1> %cmp14.i, i32 2
  %"&(pSB[currWI].offset)1763.i" = add nuw i64 %CurrSBIndex..0.i, 337
  %"&pSB[currWI].offset1764.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1763.i"
  %CastToValueType1765.i = bitcast i8* %"&pSB[currWI].offset1764.i" to i1*
  store i1 %extract171.i, i1* %CastToValueType1765.i, align 1
  %extract172.i = extractelement <16 x i1> %cmp14.i, i32 3
  %"&(pSB[currWI].offset)1787.i" = add nuw i64 %CurrSBIndex..0.i, 338
  %"&pSB[currWI].offset1788.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1787.i"
  %CastToValueType1789.i = bitcast i8* %"&pSB[currWI].offset1788.i" to i1*
  store i1 %extract172.i, i1* %CastToValueType1789.i, align 1
  %extract173.i = extractelement <16 x i1> %cmp14.i, i32 4
  %"&(pSB[currWI].offset)1811.i" = add nuw i64 %CurrSBIndex..0.i, 339
  %"&pSB[currWI].offset1812.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1811.i"
  %CastToValueType1813.i = bitcast i8* %"&pSB[currWI].offset1812.i" to i1*
  store i1 %extract173.i, i1* %CastToValueType1813.i, align 1
  %extract174.i = extractelement <16 x i1> %cmp14.i, i32 5
  %"&(pSB[currWI].offset)1835.i" = add nuw i64 %CurrSBIndex..0.i, 340
  %"&pSB[currWI].offset1836.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1835.i"
  %CastToValueType1837.i = bitcast i8* %"&pSB[currWI].offset1836.i" to i1*
  store i1 %extract174.i, i1* %CastToValueType1837.i, align 1
  %extract175.i = extractelement <16 x i1> %cmp14.i, i32 6
  %"&(pSB[currWI].offset)1859.i" = add nuw i64 %CurrSBIndex..0.i, 341
  %"&pSB[currWI].offset1860.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1859.i"
  %CastToValueType1861.i = bitcast i8* %"&pSB[currWI].offset1860.i" to i1*
  store i1 %extract175.i, i1* %CastToValueType1861.i, align 1
  %extract176.i = extractelement <16 x i1> %cmp14.i, i32 7
  %"&(pSB[currWI].offset)1883.i" = add nuw i64 %CurrSBIndex..0.i, 342
  %"&pSB[currWI].offset1884.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1883.i"
  %CastToValueType1885.i = bitcast i8* %"&pSB[currWI].offset1884.i" to i1*
  store i1 %extract176.i, i1* %CastToValueType1885.i, align 1
  %extract177.i = extractelement <16 x i1> %cmp14.i, i32 8
  %"&(pSB[currWI].offset)1907.i" = add nuw i64 %CurrSBIndex..0.i, 343
  %"&pSB[currWI].offset1908.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1907.i"
  %CastToValueType1909.i = bitcast i8* %"&pSB[currWI].offset1908.i" to i1*
  store i1 %extract177.i, i1* %CastToValueType1909.i, align 1
  %extract178.i = extractelement <16 x i1> %cmp14.i, i32 9
  %"&(pSB[currWI].offset)1931.i" = add nuw i64 %CurrSBIndex..0.i, 344
  %"&pSB[currWI].offset1932.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1931.i"
  %CastToValueType1933.i = bitcast i8* %"&pSB[currWI].offset1932.i" to i1*
  store i1 %extract178.i, i1* %CastToValueType1933.i, align 1
  %extract179.i = extractelement <16 x i1> %cmp14.i, i32 10
  %"&(pSB[currWI].offset)1955.i" = add nuw i64 %CurrSBIndex..0.i, 345
  %"&pSB[currWI].offset1956.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1955.i"
  %CastToValueType1957.i = bitcast i8* %"&pSB[currWI].offset1956.i" to i1*
  store i1 %extract179.i, i1* %CastToValueType1957.i, align 1
  %extract180.i = extractelement <16 x i1> %cmp14.i, i32 11
  %"&(pSB[currWI].offset)1979.i" = add nuw i64 %CurrSBIndex..0.i, 346
  %"&pSB[currWI].offset1980.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1979.i"
  %CastToValueType1981.i = bitcast i8* %"&pSB[currWI].offset1980.i" to i1*
  store i1 %extract180.i, i1* %CastToValueType1981.i, align 1
  %extract181.i = extractelement <16 x i1> %cmp14.i, i32 12
  %"&(pSB[currWI].offset)2003.i" = add nuw i64 %CurrSBIndex..0.i, 347
  %"&pSB[currWI].offset2004.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2003.i"
  %CastToValueType2005.i = bitcast i8* %"&pSB[currWI].offset2004.i" to i1*
  store i1 %extract181.i, i1* %CastToValueType2005.i, align 1
  %extract182.i = extractelement <16 x i1> %cmp14.i, i32 13
  %"&(pSB[currWI].offset)2027.i" = add nuw i64 %CurrSBIndex..0.i, 348
  %"&pSB[currWI].offset2028.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2027.i"
  %CastToValueType2029.i = bitcast i8* %"&pSB[currWI].offset2028.i" to i1*
  store i1 %extract182.i, i1* %CastToValueType2029.i, align 1
  %extract183.i = extractelement <16 x i1> %cmp14.i, i32 14
  %"&(pSB[currWI].offset)2051.i" = add nuw i64 %CurrSBIndex..0.i, 349
  %"&pSB[currWI].offset2052.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2051.i"
  %CastToValueType2053.i = bitcast i8* %"&pSB[currWI].offset2052.i" to i1*
  store i1 %extract183.i, i1* %CastToValueType2053.i, align 1
  %extract184.i = extractelement <16 x i1> %cmp14.i, i32 15
  %"&(pSB[currWI].offset)2075.i" = add nuw i64 %CurrSBIndex..0.i, 350
  %"&pSB[currWI].offset2076.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2075.i"
  %CastToValueType2077.i = bitcast i8* %"&pSB[currWI].offset2076.i" to i1*
  store i1 %extract184.i, i1* %CastToValueType2077.i, align 1
  %extract169.i = extractelement <16 x i1> %cmp14.i, i32 0
  %"&(pSB[currWI].offset)2099.i" = add nuw i64 %CurrSBIndex..0.i, 351
  %"&pSB[currWI].offset2100.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2099.i"
  %CastToValueType2101.i = bitcast i8* %"&pSB[currWI].offset2100.i" to i1*
  store i1 %extract169.i, i1* %CastToValueType2101.i, align 1
  %sext8.i = shl <16 x i64> %add6109.i, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %idxprom17152.i = ashr <16 x i64> %sext8.i, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %"&(pSB[currWI].offset)2118.i" = add nuw i64 %CurrSBIndex..0.i, 384
  %"&pSB[currWI].offset2119.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2118.i"
  %CastToValueType2120.i = bitcast i8* %"&pSB[currWI].offset2119.i" to <16 x i64>*
  store <16 x i64> %idxprom17152.i, <16 x i64>* %CastToValueType2120.i, align 128
  %extract154.i = extractelement <16 x i64> %idxprom17152.i, i32 1
  %"&(pSB[currWI].offset)2132.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset2133.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2132.i"
  %CastToValueType2134.i = bitcast i8* %"&pSB[currWI].offset2133.i" to i64*
  store i64 %extract154.i, i64* %CastToValueType2134.i, align 8
  %extract155.i = extractelement <16 x i64> %idxprom17152.i, i32 2
  %"&(pSB[currWI].offset)2141.i" = add nuw i64 %CurrSBIndex..0.i, 520
  %"&pSB[currWI].offset2142.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2141.i"
  %CastToValueType2143.i = bitcast i8* %"&pSB[currWI].offset2142.i" to i64*
  store i64 %extract155.i, i64* %CastToValueType2143.i, align 8
  %extract156.i = extractelement <16 x i64> %idxprom17152.i, i32 3
  %"&(pSB[currWI].offset)2150.i" = add nuw i64 %CurrSBIndex..0.i, 528
  %"&pSB[currWI].offset2151.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2150.i"
  %CastToValueType2152.i = bitcast i8* %"&pSB[currWI].offset2151.i" to i64*
  store i64 %extract156.i, i64* %CastToValueType2152.i, align 8
  %extract157.i = extractelement <16 x i64> %idxprom17152.i, i32 4
  %"&(pSB[currWI].offset)2159.i" = add nuw i64 %CurrSBIndex..0.i, 536
  %"&pSB[currWI].offset2160.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2159.i"
  %CastToValueType2161.i = bitcast i8* %"&pSB[currWI].offset2160.i" to i64*
  store i64 %extract157.i, i64* %CastToValueType2161.i, align 8
  %extract158.i = extractelement <16 x i64> %idxprom17152.i, i32 5
  %"&(pSB[currWI].offset)2168.i" = add nuw i64 %CurrSBIndex..0.i, 544
  %"&pSB[currWI].offset2169.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2168.i"
  %CastToValueType2170.i = bitcast i8* %"&pSB[currWI].offset2169.i" to i64*
  store i64 %extract158.i, i64* %CastToValueType2170.i, align 8
  %extract159.i = extractelement <16 x i64> %idxprom17152.i, i32 6
  %"&(pSB[currWI].offset)2177.i" = add nuw i64 %CurrSBIndex..0.i, 552
  %"&pSB[currWI].offset2178.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2177.i"
  %CastToValueType2179.i = bitcast i8* %"&pSB[currWI].offset2178.i" to i64*
  store i64 %extract159.i, i64* %CastToValueType2179.i, align 8
  %extract160.i = extractelement <16 x i64> %idxprom17152.i, i32 7
  %"&(pSB[currWI].offset)2186.i" = add nuw i64 %CurrSBIndex..0.i, 560
  %"&pSB[currWI].offset2187.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2186.i"
  %CastToValueType2188.i = bitcast i8* %"&pSB[currWI].offset2187.i" to i64*
  store i64 %extract160.i, i64* %CastToValueType2188.i, align 8
  %extract161.i = extractelement <16 x i64> %idxprom17152.i, i32 8
  %"&(pSB[currWI].offset)2195.i" = add nuw i64 %CurrSBIndex..0.i, 568
  %"&pSB[currWI].offset2196.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2195.i"
  %CastToValueType2197.i = bitcast i8* %"&pSB[currWI].offset2196.i" to i64*
  store i64 %extract161.i, i64* %CastToValueType2197.i, align 8
  %extract162.i = extractelement <16 x i64> %idxprom17152.i, i32 9
  %"&(pSB[currWI].offset)2204.i" = add nuw i64 %CurrSBIndex..0.i, 576
  %"&pSB[currWI].offset2205.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2204.i"
  %CastToValueType2206.i = bitcast i8* %"&pSB[currWI].offset2205.i" to i64*
  store i64 %extract162.i, i64* %CastToValueType2206.i, align 8
  %extract163.i = extractelement <16 x i64> %idxprom17152.i, i32 10
  %"&(pSB[currWI].offset)2213.i" = add nuw i64 %CurrSBIndex..0.i, 584
  %"&pSB[currWI].offset2214.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2213.i"
  %CastToValueType2215.i = bitcast i8* %"&pSB[currWI].offset2214.i" to i64*
  store i64 %extract163.i, i64* %CastToValueType2215.i, align 8
  %extract164.i = extractelement <16 x i64> %idxprom17152.i, i32 11
  %"&(pSB[currWI].offset)2222.i" = add nuw i64 %CurrSBIndex..0.i, 592
  %"&pSB[currWI].offset2223.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2222.i"
  %CastToValueType2224.i = bitcast i8* %"&pSB[currWI].offset2223.i" to i64*
  store i64 %extract164.i, i64* %CastToValueType2224.i, align 8
  %extract165.i = extractelement <16 x i64> %idxprom17152.i, i32 12
  %"&(pSB[currWI].offset)2231.i" = add nuw i64 %CurrSBIndex..0.i, 600
  %"&pSB[currWI].offset2232.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2231.i"
  %CastToValueType2233.i = bitcast i8* %"&pSB[currWI].offset2232.i" to i64*
  store i64 %extract165.i, i64* %CastToValueType2233.i, align 8
  %extract166.i = extractelement <16 x i64> %idxprom17152.i, i32 13
  %"&(pSB[currWI].offset)2240.i" = add nuw i64 %CurrSBIndex..0.i, 608
  %"&pSB[currWI].offset2241.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2240.i"
  %CastToValueType2242.i = bitcast i8* %"&pSB[currWI].offset2241.i" to i64*
  store i64 %extract166.i, i64* %CastToValueType2242.i, align 8
  %extract167.i = extractelement <16 x i64> %idxprom17152.i, i32 14
  %"&(pSB[currWI].offset)2249.i" = add nuw i64 %CurrSBIndex..0.i, 616
  %"&pSB[currWI].offset2250.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2249.i"
  %CastToValueType2251.i = bitcast i8* %"&pSB[currWI].offset2250.i" to i64*
  store i64 %extract167.i, i64* %CastToValueType2251.i, align 8
  %extract168.i = extractelement <16 x i64> %idxprom17152.i, i32 15
  %"&(pSB[currWI].offset)2258.i" = add nuw i64 %CurrSBIndex..0.i, 624
  %"&pSB[currWI].offset2259.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2258.i"
  %CastToValueType2260.i = bitcast i8* %"&pSB[currWI].offset2259.i" to i64*
  store i64 %extract168.i, i64* %CastToValueType2260.i, align 8
  %50 = getelementptr inbounds float addrspace(1)* %4, i64 %extract154.i
  %51 = getelementptr inbounds float addrspace(1)* %4, i64 %extract155.i
  %52 = getelementptr inbounds float addrspace(1)* %4, i64 %extract156.i
  %53 = getelementptr inbounds float addrspace(1)* %4, i64 %extract157.i
  %54 = getelementptr inbounds float addrspace(1)* %4, i64 %extract158.i
  %55 = getelementptr inbounds float addrspace(1)* %4, i64 %extract159.i
  %56 = getelementptr inbounds float addrspace(1)* %4, i64 %extract160.i
  %57 = getelementptr inbounds float addrspace(1)* %4, i64 %extract161.i
  %58 = getelementptr inbounds float addrspace(1)* %4, i64 %extract162.i
  %59 = getelementptr inbounds float addrspace(1)* %4, i64 %extract163.i
  %60 = getelementptr inbounds float addrspace(1)* %4, i64 %extract164.i
  %61 = getelementptr inbounds float addrspace(1)* %4, i64 %extract165.i
  %62 = getelementptr inbounds float addrspace(1)* %4, i64 %extract166.i
  %63 = getelementptr inbounds float addrspace(1)* %4, i64 %extract167.i
  %64 = getelementptr inbounds float addrspace(1)* %4, i64 %extract168.i
  br i1 %extract169.i, label %preload1131.i, label %postload1132.i

preload1131.i:                                    ; preds = %if.end.i
  %extract153.i = extractelement <16 x i64> %idxprom17152.i, i32 0
  %65 = getelementptr inbounds float addrspace(1)* %4, i64 %extract153.i
  %masked_load.i = load float addrspace(1)* %65, align 4
  br label %postload1132.i

postload1132.i:                                   ; preds = %preload1131.i, %if.end.i
  %phi1133.i = phi float [ undef, %if.end.i ], [ %masked_load.i, %preload1131.i ]
  br i1 %extract170.i, label %preload1134.i, label %postload1135.i

preload1134.i:                                    ; preds = %postload1132.i
  %masked_load671.i = load float addrspace(1)* %50, align 4
  br label %postload1135.i

postload1135.i:                                   ; preds = %preload1134.i, %postload1132.i
  %phi1136.i = phi float [ undef, %postload1132.i ], [ %masked_load671.i, %preload1134.i ]
  br i1 %extract171.i, label %preload1137.i, label %postload1138.i

preload1137.i:                                    ; preds = %postload1135.i
  %masked_load672.i = load float addrspace(1)* %51, align 4
  br label %postload1138.i

postload1138.i:                                   ; preds = %preload1137.i, %postload1135.i
  %phi1139.i = phi float [ undef, %postload1135.i ], [ %masked_load672.i, %preload1137.i ]
  br i1 %extract172.i, label %preload985.i, label %postload986.i

preload985.i:                                     ; preds = %postload1138.i
  %masked_load673.i = load float addrspace(1)* %52, align 4
  br label %postload986.i

postload986.i:                                    ; preds = %preload985.i, %postload1138.i
  %phi987.i = phi float [ undef, %postload1138.i ], [ %masked_load673.i, %preload985.i ]
  br i1 %extract173.i, label %preload988.i, label %postload989.i

preload988.i:                                     ; preds = %postload986.i
  %masked_load674.i = load float addrspace(1)* %53, align 4
  br label %postload989.i

postload989.i:                                    ; preds = %preload988.i, %postload986.i
  %phi990.i = phi float [ undef, %postload986.i ], [ %masked_load674.i, %preload988.i ]
  br i1 %extract174.i, label %preload991.i, label %postload992.i

preload991.i:                                     ; preds = %postload989.i
  %masked_load675.i = load float addrspace(1)* %54, align 4
  br label %postload992.i

postload992.i:                                    ; preds = %preload991.i, %postload989.i
  %phi993.i = phi float [ undef, %postload989.i ], [ %masked_load675.i, %preload991.i ]
  br i1 %extract175.i, label %preload994.i, label %postload995.i

preload994.i:                                     ; preds = %postload992.i
  %masked_load676.i = load float addrspace(1)* %55, align 4
  br label %postload995.i

postload995.i:                                    ; preds = %preload994.i, %postload992.i
  %phi996.i = phi float [ undef, %postload992.i ], [ %masked_load676.i, %preload994.i ]
  br i1 %extract176.i, label %preload997.i, label %postload998.i

preload997.i:                                     ; preds = %postload995.i
  %masked_load677.i = load float addrspace(1)* %56, align 4
  br label %postload998.i

postload998.i:                                    ; preds = %preload997.i, %postload995.i
  %phi999.i = phi float [ undef, %postload995.i ], [ %masked_load677.i, %preload997.i ]
  br i1 %extract177.i, label %preload1000.i, label %postload1001.i

preload1000.i:                                    ; preds = %postload998.i
  %masked_load678.i = load float addrspace(1)* %57, align 4
  br label %postload1001.i

postload1001.i:                                   ; preds = %preload1000.i, %postload998.i
  %phi1002.i = phi float [ undef, %postload998.i ], [ %masked_load678.i, %preload1000.i ]
  br i1 %extract178.i, label %preload1173.i, label %postload1174.i

preload1173.i:                                    ; preds = %postload1001.i
  %masked_load679.i = load float addrspace(1)* %58, align 4
  br label %postload1174.i

postload1174.i:                                   ; preds = %preload1173.i, %postload1001.i
  %phi1175.i = phi float [ undef, %postload1001.i ], [ %masked_load679.i, %preload1173.i ]
  br i1 %extract179.i, label %preload1176.i, label %postload1177.i

preload1176.i:                                    ; preds = %postload1174.i
  %masked_load680.i = load float addrspace(1)* %59, align 4
  br label %postload1177.i

postload1177.i:                                   ; preds = %preload1176.i, %postload1174.i
  %phi1178.i = phi float [ undef, %postload1174.i ], [ %masked_load680.i, %preload1176.i ]
  br i1 %extract180.i, label %preload1179.i, label %postload1180.i

preload1179.i:                                    ; preds = %postload1177.i
  %masked_load681.i = load float addrspace(1)* %60, align 4
  br label %postload1180.i

postload1180.i:                                   ; preds = %preload1179.i, %postload1177.i
  %phi1181.i = phi float [ undef, %postload1177.i ], [ %masked_load681.i, %preload1179.i ]
  br i1 %extract181.i, label %preload1182.i, label %postload1183.i

preload1182.i:                                    ; preds = %postload1180.i
  %masked_load682.i = load float addrspace(1)* %61, align 4
  br label %postload1183.i

postload1183.i:                                   ; preds = %preload1182.i, %postload1180.i
  %phi1184.i = phi float [ undef, %postload1180.i ], [ %masked_load682.i, %preload1182.i ]
  br i1 %extract182.i, label %preload1185.i, label %postload1186.i

preload1185.i:                                    ; preds = %postload1183.i
  %masked_load683.i = load float addrspace(1)* %62, align 4
  br label %postload1186.i

postload1186.i:                                   ; preds = %preload1185.i, %postload1183.i
  %phi1187.i = phi float [ undef, %postload1183.i ], [ %masked_load683.i, %preload1185.i ]
  br i1 %extract183.i, label %preload1188.i, label %postload1189.i

preload1188.i:                                    ; preds = %postload1186.i
  %masked_load684.i = load float addrspace(1)* %63, align 4
  br label %postload1189.i

postload1189.i:                                   ; preds = %preload1188.i, %postload1186.i
  %phi1190.i = phi float [ undef, %postload1186.i ], [ %masked_load684.i, %preload1188.i ]
  br i1 %extract184.i, label %preload1224.i, label %postload1225.i

preload1224.i:                                    ; preds = %postload1189.i
  %masked_load685.i = load float addrspace(1)* %64, align 4
  br label %postload1225.i

postload1225.i:                                   ; preds = %preload1224.i, %postload1189.i
  %phi1226.i = phi float [ undef, %postload1189.i ], [ %masked_load685.i, %preload1224.i ]
  %66 = extractelement <16 x i32> %conv11112.i, i32 0
  %extract186.i = sext i32 %66 to i64
  %"&(pSB[currWI].offset)2267.i" = add nuw i64 %CurrSBIndex..0.i, 632
  %"&pSB[currWI].offset2268.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2267.i"
  %CastToValueType2269.i = bitcast i8* %"&pSB[currWI].offset2268.i" to i64*
  store i64 %extract186.i, i64* %CastToValueType2269.i, align 8
  br i1 %extract169.i, label %preload967.i, label %postload968.i

preload967.i:                                     ; preds = %postload1225.i
  %67 = getelementptr inbounds float addrspace(3)* %22, i64 %extract186.i
  store float %phi1133.i, float addrspace(3)* %67, align 4
  %loadedValue1756.pre.i = load i1* %CastToValueType1741.i, align 1
  br label %postload968.i

postload968.i:                                    ; preds = %preload967.i, %postload1225.i
  %loadedValue1756.i = phi i1 [ %loadedValue1756.pre.i, %preload967.i ], [ %extract170.i, %postload1225.i ]
  br i1 %loadedValue1756.i, label %preload940.i, label %postload968.i.postload941.i_crit_edge

postload968.i.postload941.i_crit_edge:            ; preds = %postload968.i
  br label %postload941.i

preload940.i:                                     ; preds = %postload968.i
  %loadedValue2434.i = load i64* %CastToValueType2269.i, align 8
  %.sum1667.i = add i64 %loadedValue2434.i, 1
  %68 = getelementptr float addrspace(3)* %22, i64 %.sum1667.i
  store float %phi1136.i, float addrspace(3)* %68, align 4
  br label %postload941.i

postload941.i:                                    ; preds = %postload968.i.postload941.i_crit_edge, %preload940.i
  %loadedValue1780.i = load i1* %CastToValueType1765.i, align 1
  br i1 %loadedValue1780.i, label %preload1203.i, label %postload941.i.postload1204.i_crit_edge

postload941.i.postload1204.i_crit_edge:           ; preds = %postload941.i
  br label %postload1204.i

preload1203.i:                                    ; preds = %postload941.i
  %loadedValue2439.i = load i64* %CastToValueType2269.i, align 8
  %.sum1666.i = add i64 %loadedValue2439.i, 2
  %69 = getelementptr float addrspace(3)* %22, i64 %.sum1666.i
  store float %phi1139.i, float addrspace(3)* %69, align 4
  br label %postload1204.i

postload1204.i:                                   ; preds = %postload941.i.postload1204.i_crit_edge, %preload1203.i
  %loadedValue1804.i = load i1* %CastToValueType1789.i, align 1
  br i1 %loadedValue1804.i, label %preload1215.i, label %postload1204.i.postload1216.i_crit_edge

postload1204.i.postload1216.i_crit_edge:          ; preds = %postload1204.i
  br label %postload1216.i

preload1215.i:                                    ; preds = %postload1204.i
  %loadedValue2444.i = load i64* %CastToValueType2269.i, align 8
  %.sum1665.i = add i64 %loadedValue2444.i, 3
  %70 = getelementptr float addrspace(3)* %22, i64 %.sum1665.i
  store float %phi987.i, float addrspace(3)* %70, align 4
  br label %postload1216.i

postload1216.i:                                   ; preds = %postload1204.i.postload1216.i_crit_edge, %preload1215.i
  %loadedValue1828.i = load i1* %CastToValueType1813.i, align 1
  br i1 %loadedValue1828.i, label %preload1206.i, label %postload1216.i.postload1207.i_crit_edge

postload1216.i.postload1207.i_crit_edge:          ; preds = %postload1216.i
  br label %postload1207.i

preload1206.i:                                    ; preds = %postload1216.i
  %loadedValue2449.i = load i64* %CastToValueType2269.i, align 8
  %.sum1664.i = add i64 %loadedValue2449.i, 4
  %71 = getelementptr float addrspace(3)* %22, i64 %.sum1664.i
  store float %phi990.i, float addrspace(3)* %71, align 4
  br label %postload1207.i

postload1207.i:                                   ; preds = %postload1216.i.postload1207.i_crit_edge, %preload1206.i
  %loadedValue1852.i = load i1* %CastToValueType1837.i, align 1
  br i1 %loadedValue1852.i, label %preload1161.i, label %postload1207.i.postload1162.i_crit_edge

postload1207.i.postload1162.i_crit_edge:          ; preds = %postload1207.i
  br label %postload1162.i

preload1161.i:                                    ; preds = %postload1207.i
  %loadedValue2454.i = load i64* %CastToValueType2269.i, align 8
  %.sum1663.i = add i64 %loadedValue2454.i, 5
  %72 = getelementptr float addrspace(3)* %22, i64 %.sum1663.i
  store float %phi993.i, float addrspace(3)* %72, align 4
  br label %postload1162.i

postload1162.i:                                   ; preds = %postload1207.i.postload1162.i_crit_edge, %preload1161.i
  %loadedValue1876.i = load i1* %CastToValueType1861.i, align 1
  br i1 %loadedValue1876.i, label %preload1209.i, label %postload1162.i.postload1210.i_crit_edge

postload1162.i.postload1210.i_crit_edge:          ; preds = %postload1162.i
  br label %postload1210.i

preload1209.i:                                    ; preds = %postload1162.i
  %loadedValue2459.i = load i64* %CastToValueType2269.i, align 8
  %.sum1662.i = add i64 %loadedValue2459.i, 6
  %73 = getelementptr float addrspace(3)* %22, i64 %.sum1662.i
  store float %phi996.i, float addrspace(3)* %73, align 4
  br label %postload1210.i

postload1210.i:                                   ; preds = %postload1162.i.postload1210.i_crit_edge, %preload1209.i
  %loadedValue1900.i = load i1* %CastToValueType1885.i, align 1
  br i1 %loadedValue1900.i, label %preload1212.i, label %postload1210.i.postload1213.i_crit_edge

postload1210.i.postload1213.i_crit_edge:          ; preds = %postload1210.i
  br label %postload1213.i

preload1212.i:                                    ; preds = %postload1210.i
  %loadedValue2464.i = load i64* %CastToValueType2269.i, align 8
  %.sum1661.i = add i64 %loadedValue2464.i, 7
  %74 = getelementptr float addrspace(3)* %22, i64 %.sum1661.i
  store float %phi999.i, float addrspace(3)* %74, align 4
  br label %postload1213.i

postload1213.i:                                   ; preds = %postload1210.i.postload1213.i_crit_edge, %preload1212.i
  %loadedValue1924.i = load i1* %CastToValueType1909.i, align 1
  br i1 %loadedValue1924.i, label %preload1170.i, label %postload1213.i.postload1171.i_crit_edge

postload1213.i.postload1171.i_crit_edge:          ; preds = %postload1213.i
  br label %postload1171.i

preload1170.i:                                    ; preds = %postload1213.i
  %loadedValue2469.i = load i64* %CastToValueType2269.i, align 8
  %.sum1660.i = add i64 %loadedValue2469.i, 8
  %75 = getelementptr float addrspace(3)* %22, i64 %.sum1660.i
  store float %phi1002.i, float addrspace(3)* %75, align 4
  br label %postload1171.i

postload1171.i:                                   ; preds = %postload1213.i.postload1171.i_crit_edge, %preload1170.i
  %loadedValue1948.i = load i1* %CastToValueType1933.i, align 1
  br i1 %loadedValue1948.i, label %preload949.i, label %postload1171.i.postload950.i_crit_edge

postload1171.i.postload950.i_crit_edge:           ; preds = %postload1171.i
  br label %postload950.i

preload949.i:                                     ; preds = %postload1171.i
  %loadedValue2474.i = load i64* %CastToValueType2269.i, align 8
  %.sum1659.i = add i64 %loadedValue2474.i, 9
  %76 = getelementptr float addrspace(3)* %22, i64 %.sum1659.i
  store float %phi1175.i, float addrspace(3)* %76, align 4
  br label %postload950.i

postload950.i:                                    ; preds = %postload1171.i.postload950.i_crit_edge, %preload949.i
  %loadedValue1972.i = load i1* %CastToValueType1957.i, align 1
  br i1 %loadedValue1972.i, label %preload1191.i, label %postload950.i.postload1192.i_crit_edge

postload950.i.postload1192.i_crit_edge:           ; preds = %postload950.i
  br label %postload1192.i

preload1191.i:                                    ; preds = %postload950.i
  %loadedValue2479.i = load i64* %CastToValueType2269.i, align 8
  %.sum1658.i = add i64 %loadedValue2479.i, 10
  %77 = getelementptr float addrspace(3)* %22, i64 %.sum1658.i
  store float %phi1178.i, float addrspace(3)* %77, align 4
  br label %postload1192.i

postload1192.i:                                   ; preds = %postload950.i.postload1192.i_crit_edge, %preload1191.i
  %loadedValue1996.i = load i1* %CastToValueType1981.i, align 1
  br i1 %loadedValue1996.i, label %preload1155.i, label %postload1192.i.postload1156.i_crit_edge

postload1192.i.postload1156.i_crit_edge:          ; preds = %postload1192.i
  br label %postload1156.i

preload1155.i:                                    ; preds = %postload1192.i
  %loadedValue2484.i = load i64* %CastToValueType2269.i, align 8
  %.sum1657.i = add i64 %loadedValue2484.i, 11
  %78 = getelementptr float addrspace(3)* %22, i64 %.sum1657.i
  store float %phi1181.i, float addrspace(3)* %78, align 4
  br label %postload1156.i

postload1156.i:                                   ; preds = %postload1192.i.postload1156.i_crit_edge, %preload1155.i
  %loadedValue2020.i = load i1* %CastToValueType2005.i, align 1
  br i1 %loadedValue2020.i, label %preload.i, label %postload1156.i.postload.i_crit_edge

postload1156.i.postload.i_crit_edge:              ; preds = %postload1156.i
  br label %postload.i

preload.i:                                        ; preds = %postload1156.i
  %loadedValue2489.i = load i64* %CastToValueType2269.i, align 8
  %.sum1656.i = add i64 %loadedValue2489.i, 12
  %79 = getelementptr float addrspace(3)* %22, i64 %.sum1656.i
  store float %phi1184.i, float addrspace(3)* %79, align 4
  br label %postload.i

postload.i:                                       ; preds = %postload1156.i.postload.i_crit_edge, %preload.i
  %loadedValue2044.i = load i1* %CastToValueType2029.i, align 1
  br i1 %loadedValue2044.i, label %preload943.i, label %postload.i.postload944.i_crit_edge

postload.i.postload944.i_crit_edge:               ; preds = %postload.i
  br label %postload944.i

preload943.i:                                     ; preds = %postload.i
  %loadedValue2494.i = load i64* %CastToValueType2269.i, align 8
  %.sum1655.i = add i64 %loadedValue2494.i, 13
  %80 = getelementptr float addrspace(3)* %22, i64 %.sum1655.i
  store float %phi1187.i, float addrspace(3)* %80, align 4
  br label %postload944.i

postload944.i:                                    ; preds = %postload.i.postload944.i_crit_edge, %preload943.i
  %loadedValue2068.i = load i1* %CastToValueType2053.i, align 1
  br i1 %loadedValue2068.i, label %preload958.i, label %postload944.i.postload959.i_crit_edge

postload944.i.postload959.i_crit_edge:            ; preds = %postload944.i
  br label %postload959.i

preload958.i:                                     ; preds = %postload944.i
  %loadedValue2499.i = load i64* %CastToValueType2269.i, align 8
  %.sum1654.i = add i64 %loadedValue2499.i, 14
  %81 = getelementptr float addrspace(3)* %22, i64 %.sum1654.i
  store float %phi1190.i, float addrspace(3)* %81, align 4
  br label %postload959.i

postload959.i:                                    ; preds = %postload944.i.postload959.i_crit_edge, %preload958.i
  %loadedValue2092.i = load i1* %CastToValueType2077.i, align 1
  br i1 %loadedValue2092.i, label %preload973.i, label %if.else21.i

preload973.i:                                     ; preds = %postload959.i
  %loadedValue2504.i = load i64* %CastToValueType2269.i, align 8
  %.sum1653.i = add i64 %loadedValue2504.i, 15
  %82 = getelementptr float addrspace(3)* %22, i64 %.sum1653.i
  store float %phi1226.i, float addrspace(3)* %82, align 4
  br label %if.else21.i

if.else21.i:                                      ; preds = %preload973.i, %postload959.i
  %exmask734.i = extractelement <16 x i1> %Mneg2149.i, i32 0
  br i1 %exmask734.i, label %preload1200.i, label %postload1201.i

preload1200.i:                                    ; preds = %if.else21.i
  %loadedValue2354.i = load i64* %CastToValueType2269.i, align 8
  %83 = getelementptr inbounds float addrspace(3)* %22, i64 %loadedValue2354.i
  store float 0.000000e+00, float addrspace(3)* %83, align 4
  br label %postload1201.i

postload1201.i:                                   ; preds = %preload1200.i, %if.else21.i
  %exmask737.i = extractelement <16 x i1> %Mneg2149.i, i32 1
  br i1 %exmask737.i, label %preload1125.i, label %postload1126.i

preload1125.i:                                    ; preds = %postload1201.i
  %loadedValue2429.i = load i64* %CastToValueType2269.i, align 8
  %.sum1652.i = add i64 %loadedValue2429.i, 1
  %84 = getelementptr float addrspace(3)* %22, i64 %.sum1652.i
  store float 0.000000e+00, float addrspace(3)* %84, align 4
  br label %postload1126.i

postload1126.i:                                   ; preds = %preload1125.i, %postload1201.i
  %exmask740.i = extractelement <16 x i1> %Mneg2149.i, i32 2
  br i1 %exmask740.i, label %preload970.i, label %postload971.i

preload970.i:                                     ; preds = %postload1126.i
  %loadedValue2424.i = load i64* %CastToValueType2269.i, align 8
  %.sum1651.i = add i64 %loadedValue2424.i, 2
  %85 = getelementptr float addrspace(3)* %22, i64 %.sum1651.i
  store float 0.000000e+00, float addrspace(3)* %85, align 4
  br label %postload971.i

postload971.i:                                    ; preds = %preload970.i, %postload1126.i
  %exmask743.i = extractelement <16 x i1> %Mneg2149.i, i32 3
  br i1 %exmask743.i, label %preload946.i, label %postload947.i

preload946.i:                                     ; preds = %postload971.i
  %loadedValue2419.i = load i64* %CastToValueType2269.i, align 8
  %.sum1650.i = add i64 %loadedValue2419.i, 3
  %86 = getelementptr float addrspace(3)* %22, i64 %.sum1650.i
  store float 0.000000e+00, float addrspace(3)* %86, align 4
  br label %postload947.i

postload947.i:                                    ; preds = %preload946.i, %postload971.i
  %exmask746.i = extractelement <16 x i1> %Mneg2149.i, i32 4
  br i1 %exmask746.i, label %preload982.i, label %postload983.i

preload982.i:                                     ; preds = %postload947.i
  %loadedValue2414.i = load i64* %CastToValueType2269.i, align 8
  %.sum1649.i = add i64 %loadedValue2414.i, 4
  %87 = getelementptr float addrspace(3)* %22, i64 %.sum1649.i
  store float 0.000000e+00, float addrspace(3)* %87, align 4
  br label %postload983.i

postload983.i:                                    ; preds = %preload982.i, %postload947.i
  %exmask749.i = extractelement <16 x i1> %Mneg2149.i, i32 5
  br i1 %exmask749.i, label %preload964.i, label %postload965.i

preload964.i:                                     ; preds = %postload983.i
  %loadedValue2409.i = load i64* %CastToValueType2269.i, align 8
  %.sum1648.i = add i64 %loadedValue2409.i, 5
  %88 = getelementptr float addrspace(3)* %22, i64 %.sum1648.i
  store float 0.000000e+00, float addrspace(3)* %88, align 4
  br label %postload965.i

postload965.i:                                    ; preds = %preload964.i, %postload983.i
  %exmask752.i = extractelement <16 x i1> %Mneg2149.i, i32 6
  br i1 %exmask752.i, label %preload1011.i, label %postload1012.i

preload1011.i:                                    ; preds = %postload965.i
  %loadedValue2404.i = load i64* %CastToValueType2269.i, align 8
  %.sum1647.i = add i64 %loadedValue2404.i, 6
  %89 = getelementptr float addrspace(3)* %22, i64 %.sum1647.i
  store float 0.000000e+00, float addrspace(3)* %89, align 4
  br label %postload1012.i

postload1012.i:                                   ; preds = %preload1011.i, %postload965.i
  %exmask755.i = extractelement <16 x i1> %Mneg2149.i, i32 7
  br i1 %exmask755.i, label %preload1167.i, label %postload1168.i

preload1167.i:                                    ; preds = %postload1012.i
  %loadedValue2399.i = load i64* %CastToValueType2269.i, align 8
  %.sum1646.i = add i64 %loadedValue2399.i, 7
  %90 = getelementptr float addrspace(3)* %22, i64 %.sum1646.i
  store float 0.000000e+00, float addrspace(3)* %90, align 4
  br label %postload1168.i

postload1168.i:                                   ; preds = %preload1167.i, %postload1012.i
  %exmask758.i = extractelement <16 x i1> %Mneg2149.i, i32 8
  br i1 %exmask758.i, label %preload1152.i, label %postload1153.i

preload1152.i:                                    ; preds = %postload1168.i
  %loadedValue2394.i = load i64* %CastToValueType2269.i, align 8
  %.sum1645.i = add i64 %loadedValue2394.i, 8
  %91 = getelementptr float addrspace(3)* %22, i64 %.sum1645.i
  store float 0.000000e+00, float addrspace(3)* %91, align 4
  br label %postload1153.i

postload1153.i:                                   ; preds = %preload1152.i, %postload1168.i
  %exmask761.i = extractelement <16 x i1> %Mneg2149.i, i32 9
  br i1 %exmask761.i, label %preload955.i, label %postload956.i

preload955.i:                                     ; preds = %postload1153.i
  %loadedValue2389.i = load i64* %CastToValueType2269.i, align 8
  %.sum1644.i = add i64 %loadedValue2389.i, 9
  %92 = getelementptr float addrspace(3)* %22, i64 %.sum1644.i
  store float 0.000000e+00, float addrspace(3)* %92, align 4
  br label %postload956.i

postload956.i:                                    ; preds = %preload955.i, %postload1153.i
  %exmask764.i = extractelement <16 x i1> %Mneg2149.i, i32 10
  br i1 %exmask764.i, label %preload1158.i, label %postload1159.i

preload1158.i:                                    ; preds = %postload956.i
  %loadedValue2384.i = load i64* %CastToValueType2269.i, align 8
  %.sum1643.i = add i64 %loadedValue2384.i, 10
  %93 = getelementptr float addrspace(3)* %22, i64 %.sum1643.i
  store float 0.000000e+00, float addrspace(3)* %93, align 4
  br label %postload1159.i

postload1159.i:                                   ; preds = %preload1158.i, %postload956.i
  %exmask767.i = extractelement <16 x i1> %Mneg2149.i, i32 11
  br i1 %exmask767.i, label %preload952.i, label %postload953.i

preload952.i:                                     ; preds = %postload1159.i
  %loadedValue2379.i = load i64* %CastToValueType2269.i, align 8
  %.sum1642.i = add i64 %loadedValue2379.i, 11
  %94 = getelementptr float addrspace(3)* %22, i64 %.sum1642.i
  store float 0.000000e+00, float addrspace(3)* %94, align 4
  br label %postload953.i

postload953.i:                                    ; preds = %preload952.i, %postload1159.i
  %exmask770.i = extractelement <16 x i1> %Mneg2149.i, i32 12
  br i1 %exmask770.i, label %preload961.i, label %postload962.i

preload961.i:                                     ; preds = %postload953.i
  %loadedValue2374.i = load i64* %CastToValueType2269.i, align 8
  %.sum1641.i = add i64 %loadedValue2374.i, 12
  %95 = getelementptr float addrspace(3)* %22, i64 %.sum1641.i
  store float 0.000000e+00, float addrspace(3)* %95, align 4
  br label %postload962.i

postload962.i:                                    ; preds = %preload961.i, %postload953.i
  %exmask773.i = extractelement <16 x i1> %Mneg2149.i, i32 13
  br i1 %exmask773.i, label %preload1128.i, label %postload1129.i

preload1128.i:                                    ; preds = %postload962.i
  %loadedValue2369.i = load i64* %CastToValueType2269.i, align 8
  %.sum1640.i = add i64 %loadedValue2369.i, 13
  %96 = getelementptr float addrspace(3)* %22, i64 %.sum1640.i
  store float 0.000000e+00, float addrspace(3)* %96, align 4
  br label %postload1129.i

postload1129.i:                                   ; preds = %preload1128.i, %postload962.i
  %exmask776.i = extractelement <16 x i1> %Mneg2149.i, i32 14
  br i1 %exmask776.i, label %preload1143.i, label %postload1144.i

preload1143.i:                                    ; preds = %postload1129.i
  %loadedValue2364.i = load i64* %CastToValueType2269.i, align 8
  %.sum1639.i = add i64 %loadedValue2364.i, 14
  %97 = getelementptr float addrspace(3)* %22, i64 %.sum1639.i
  store float 0.000000e+00, float addrspace(3)* %97, align 4
  br label %postload1144.i

postload1144.i:                                   ; preds = %preload1143.i, %postload1129.i
  %exmask779.i = extractelement <16 x i1> %Mneg2149.i, i32 15
  br i1 %exmask779.i, label %preload1164.i, label %if.end24.i

preload1164.i:                                    ; preds = %postload1144.i
  %loadedValue2359.i = load i64* %CastToValueType2269.i, align 8
  %.sum1638.i = add i64 %loadedValue2359.i, 15
  %98 = getelementptr float addrspace(3)* %22, i64 %.sum1638.i
  store float 0.000000e+00, float addrspace(3)* %98, align 4
  br label %if.end24.i

if.end24.i:                                       ; preds = %preload1164.i, %postload1144.i
  %loadedValue1676.i = load i64* %CastToValueType.i, align 8
  %conv26.i = trunc i64 %loadedValue1676.i to i32
  %cmp276.i = icmp sgt i32 %conv26.i, 0
  %"&(pSB[currWI].offset)2511.i" = add nuw i64 %CurrSBIndex..0.i, 640
  %"&pSB[currWI].offset2512.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2511.i"
  %CastToValueType2513.i = bitcast i8* %"&pSB[currWI].offset2512.i" to i1*
  store i1 %cmp276.i, i1* %CastToValueType2513.i, align 1
  %temp242.i = insertelement <16 x i1> undef, i1 %cmp276.i, i32 0
  %vector243.i = shufflevector <16 x i1> %temp242.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %loadedValue1719.i = load <16 x i32>* %CastToValueType1699.i, align 64
  %mul32236.i = shl <16 x i32> %loadedValue1719.i, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %"&(pSB[currWI].offset)2520.i" = add nuw i64 %CurrSBIndex..0.i, 704
  %"&pSB[currWI].offset2521.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2520.i"
  %CastToValueType2522.i = bitcast i8* %"&pSB[currWI].offset2521.i" to <16 x i32>*
  store <16 x i32> %mul32236.i, <16 x i32>* %CastToValueType2522.i, align 64
  %negIncomingLoopMask.i = xor i1 %cmp276.i, true
  %temp238.i = insertelement <16 x i1> undef, i1 %negIncomingLoopMask.i, i32 0
  %vector239.i = shufflevector <16 x i1> %temp238.i, <16 x i1> undef, <16 x i32> zeroinitializer
  br i1 %cmp276.i, label %for.body.i, label %for.end.i

for.body.i:                                       ; preds = %if.end41.i, %if.end24.i
  %CurrWI..1.i = phi i64 [ %CurrWI..3.i, %if.end41.i ], [ %CurrWI..0.i, %if.end24.i ]
  %CurrSBIndex..1.i = phi i64 [ %CurrSBIndex..3.i, %if.end41.i ], [ %CurrSBIndex..0.i, %if.end24.i ]
  %currBarrier.1.i = phi i32 [ %currBarrier.3.i, %if.end41.i ], [ %currBarrier.0.i, %if.end24.i ]
  %vectorPHI237.i = phi <16 x i1> [ %loop_mask7380.i, %if.end41.i ], [ %vector239.i, %if.end24.i ]
  %vectorPHI240.i = phi <16 x i32> [ %out_sel375.i, %if.end41.i ], [ undef, %if.end24.i ]
  %vectorPHI241.i = phi <16 x i1> [ %local_edge401.i, %if.end41.i ], [ %vector243.i, %if.end24.i ]
  %d.08.i = phi i32 [ %shr.i, %if.end41.i ], [ %conv26.i, %if.end24.i ]
  %stride.07.i = phi i32 [ %mul42.i, %if.end41.i ], [ 1, %if.end24.i ]
  %"&(pSB[currWI].offset)2580.i" = add nuw i64 %CurrSBIndex..1.i, 904
  %"&pSB[currWI].offset2581.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2580.i"
  %CastToValueType2582.i = bitcast i8* %"&pSB[currWI].offset2581.i" to i32*
  store i32 %stride.07.i, i32* %CastToValueType2582.i, align 4
  %"&(pSB[currWI].offset)2571.i" = add nuw i64 %CurrSBIndex..1.i, 900
  %"&pSB[currWI].offset2572.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2571.i"
  %CastToValueType2573.i = bitcast i8* %"&pSB[currWI].offset2572.i" to i32*
  store i32 %d.08.i, i32* %CastToValueType2573.i, align 4
  %"&(pSB[currWI].offset)2547.i" = add nuw i64 %CurrSBIndex..1.i, 896
  %"&pSB[currWI].offset2548.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2547.i"
  %CastToValueType2549.i = bitcast i8* %"&pSB[currWI].offset2548.i" to <16 x i1>*
  store <16 x i1> %vectorPHI241.i, <16 x i1>* %CastToValueType2549.i, align 16
  %"&(pSB[currWI].offset)2538.i" = add nuw i64 %CurrSBIndex..1.i, 832
  %"&pSB[currWI].offset2539.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2538.i"
  %CastToValueType2540.i = bitcast i8* %"&pSB[currWI].offset2539.i" to <16 x i32>*
  store <16 x i32> %vectorPHI240.i, <16 x i32>* %CastToValueType2540.i, align 64
  %"&(pSB[currWI].offset)2529.i" = add nuw i64 %CurrSBIndex..1.i, 768
  %"&pSB[currWI].offset2530.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2529.i"
  %CastToValueType2531.i = bitcast i8* %"&pSB[currWI].offset2530.i" to <16 x i1>*
  store <16 x i1> %vectorPHI237.i, <16 x i1>* %CastToValueType2531.i, align 16
  %temp265.i = insertelement <16 x i32> undef, i32 %stride.07.i, i32 0
  %vector266.i = shufflevector <16 x i32> %temp265.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)2594.i" = add nuw i64 %CurrSBIndex..1.i, 960
  %"&pSB[currWI].offset2595.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2594.i"
  %CastToValueType2596.i = bitcast i8* %"&pSB[currWI].offset2595.i" to <16 x i32>*
  store <16 x i32> %vector266.i, <16 x i32>* %CastToValueType2596.i, align 64
  %temp260.i = insertelement <16 x i32> undef, i32 %d.08.i, i32 0
  %vector261.i = shufflevector <16 x i32> %temp260.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)2608.i" = add nuw i64 %CurrSBIndex..1.i, 1024
  %"&pSB[currWI].offset2609.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2608.i"
  %CastToValueType2610.i = bitcast i8* %"&pSB[currWI].offset2609.i" to <16 x i32>*
  store <16 x i32> %vector261.i, <16 x i32>* %CastToValueType2610.i, align 64
  %extract244.i = extractelement <16 x i1> %vectorPHI241.i, i32 0
  br i1 %extract244.i, label %preload1003.i, label %postload1004.i

preload1003.i:                                    ; preds = %for.body.i
  %check.WI.iter2716.i = icmp ult i64 %CurrWI..1.i, %34
  br i1 %check.WI.iter2716.i, label %thenBB2713.i, label %preload1003.i.postload1004.i_crit_edge

preload1003.i.postload1004.i_crit_edge:           ; preds = %preload1003.i
  br label %postload1004.i

thenBB2713.i:                                     ; preds = %preload1003.i
  %"CurrWI++2717.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride2719.i" = add nuw i64 %CurrSBIndex..1.i, 1408
  %cond11.i = icmp eq i32 %currBarrier.1.i, 8
  br i1 %cond11.i, label %thenBB2713.i.postload1004.i_crit_edge, label %SyncBB2701.i

thenBB2713.i.postload1004.i_crit_edge:            ; preds = %thenBB2713.i
  br label %postload1004.i

postload1004.i:                                   ; preds = %thenBB.i.postload1004.i_crit_edge, %thenBB2713.i.postload1004.i_crit_edge, %preload1003.i.postload1004.i_crit_edge, %for.body.i
  %CurrWI..3.i = phi i64 [ %CurrWI..1.i, %for.body.i ], [ 0, %preload1003.i.postload1004.i_crit_edge ], [ %"CurrWI++2717.i", %thenBB2713.i.postload1004.i_crit_edge ], [ %"CurrWI++.i", %thenBB.i.postload1004.i_crit_edge ]
  %CurrSBIndex..3.i = phi i64 [ %CurrSBIndex..1.i, %for.body.i ], [ 0, %preload1003.i.postload1004.i_crit_edge ], [ %"loadedCurrSB+Stride2719.i", %thenBB2713.i.postload1004.i_crit_edge ], [ %"loadedCurrSB+Stride.i", %thenBB.i.postload1004.i_crit_edge ]
  %currBarrier.3.i = phi i32 [ %currBarrier.1.i, %for.body.i ], [ 8, %preload1003.i.postload1004.i_crit_edge ], [ %currBarrier.1.i, %thenBB2713.i.postload1004.i_crit_edge ], [ %currBarrier.4.i, %thenBB.i.postload1004.i_crit_edge ]
  %"&(pSB[currWI].offset)1711.i" = add nuw i64 %CurrSBIndex..3.i, 256
  %"&pSB[currWI].offset1712.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1711.i"
  %CastToValueType1713.i = bitcast i8* %"&pSB[currWI].offset1712.i" to <16 x i32>*
  %loadedValue1714.i = load <16 x i32>* %CastToValueType1713.i, align 64
  %"&(pSB[currWI].offset)2612.i" = add nuw i64 %CurrSBIndex..3.i, 1024
  %"&pSB[currWI].offset2613.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2612.i"
  %CastToValueType2614.i = bitcast i8* %"&pSB[currWI].offset2613.i" to <16 x i32>*
  %loadedValue2615.i = load <16 x i32>* %CastToValueType2614.i, align 64
  %cmp29.i = icmp slt <16 x i32> %loadedValue1714.i, %loadedValue2615.i
  %"&(pSB[currWI].offset)2566.i" = add nuw i64 %CurrSBIndex..3.i, 896
  %"&pSB[currWI].offset2567.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2566.i"
  %CastToValueType2568.i = bitcast i8* %"&pSB[currWI].offset2567.i" to <16 x i1>*
  %loadedValue2569.i = load <16 x i1>* %CastToValueType2568.i, align 16
  %for.body_to_if.then31264.i = and <16 x i1> %loadedValue2569.i, %cmp29.i
  %extract289.i = extractelement <16 x i1> %for.body_to_if.then31264.i, i32 0
  %extract290.i = extractelement <16 x i1> %for.body_to_if.then31264.i, i32 1
  %extract291.i = extractelement <16 x i1> %for.body_to_if.then31264.i, i32 2
  %extract292.i = extractelement <16 x i1> %for.body_to_if.then31264.i, i32 3
  %extract293.i = extractelement <16 x i1> %for.body_to_if.then31264.i, i32 4
  %extract294.i = extractelement <16 x i1> %for.body_to_if.then31264.i, i32 5
  %extract295.i = extractelement <16 x i1> %for.body_to_if.then31264.i, i32 6
  %extract296.i = extractelement <16 x i1> %for.body_to_if.then31264.i, i32 7
  %extract297.i = extractelement <16 x i1> %for.body_to_if.then31264.i, i32 8
  %extract298.i = extractelement <16 x i1> %for.body_to_if.then31264.i, i32 9
  %extract299.i = extractelement <16 x i1> %for.body_to_if.then31264.i, i32 10
  %extract300.i = extractelement <16 x i1> %for.body_to_if.then31264.i, i32 11
  %extract301.i = extractelement <16 x i1> %for.body_to_if.then31264.i, i32 12
  %extract302.i = extractelement <16 x i1> %for.body_to_if.then31264.i, i32 13
  %extract303.i = extractelement <16 x i1> %for.body_to_if.then31264.i, i32 14
  %extract304.i = extractelement <16 x i1> %for.body_to_if.then31264.i, i32 15
  %"&(pSB[currWI].offset)2524.i" = add nuw i64 %CurrSBIndex..3.i, 704
  %"&pSB[currWI].offset2525.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2524.i"
  %CastToValueType2526.i = bitcast i8* %"&pSB[currWI].offset2525.i" to <16 x i32>*
  %loadedValue2527.i = load <16 x i32>* %CastToValueType2526.i, align 64
  %"&(pSB[currWI].offset)2603.i" = add nuw i64 %CurrSBIndex..3.i, 960
  %"&pSB[currWI].offset2604.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2603.i"
  %CastToValueType2605.i = bitcast i8* %"&pSB[currWI].offset2604.i" to <16 x i32>*
  %loadedValue2606.i = load <16 x i32>* %CastToValueType2605.i, align 64
  %mul33267.i = mul <16 x i32> %loadedValue2527.i, %loadedValue2606.i
  %"&(pSB[currWI].offset)2589.i" = add nuw i64 %CurrSBIndex..3.i, 904
  %"&pSB[currWI].offset2590.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2589.i"
  %CastToValueType2591.i = bitcast i8* %"&pSB[currWI].offset2590.i" to i32*
  %loadedValue2592.i = load i32* %CastToValueType2591.i, align 4
  %add34.i = add i32 %loadedValue2592.i, -1
  %temp268.i = insertelement <16 x i32> undef, i32 %add34.i, i32 0
  %vector269.i = shufflevector <16 x i32> %temp268.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %sub270.i = add <16 x i32> %vector269.i, %mul33267.i
  %add35271.i = add <16 x i32> %sub270.i, %loadedValue2606.i
  %idxprom36272.i = sext <16 x i32> %sub270.i to <16 x i64>
  %extract274.i = extractelement <16 x i64> %idxprom36272.i, i32 1
  %extract275.i = extractelement <16 x i64> %idxprom36272.i, i32 2
  %extract276.i = extractelement <16 x i64> %idxprom36272.i, i32 3
  %extract277.i = extractelement <16 x i64> %idxprom36272.i, i32 4
  %extract278.i = extractelement <16 x i64> %idxprom36272.i, i32 5
  %extract279.i = extractelement <16 x i64> %idxprom36272.i, i32 6
  %extract280.i = extractelement <16 x i64> %idxprom36272.i, i32 7
  %extract281.i = extractelement <16 x i64> %idxprom36272.i, i32 8
  %extract282.i = extractelement <16 x i64> %idxprom36272.i, i32 9
  %extract283.i = extractelement <16 x i64> %idxprom36272.i, i32 10
  %extract284.i = extractelement <16 x i64> %idxprom36272.i, i32 11
  %extract285.i = extractelement <16 x i64> %idxprom36272.i, i32 12
  %extract286.i = extractelement <16 x i64> %idxprom36272.i, i32 13
  %extract287.i = extractelement <16 x i64> %idxprom36272.i, i32 14
  %extract288.i = extractelement <16 x i64> %idxprom36272.i, i32 15
  %99 = getelementptr inbounds float addrspace(3)* %22, i64 %extract274.i
  %100 = getelementptr inbounds float addrspace(3)* %22, i64 %extract275.i
  %101 = getelementptr inbounds float addrspace(3)* %22, i64 %extract276.i
  %102 = getelementptr inbounds float addrspace(3)* %22, i64 %extract277.i
  %103 = getelementptr inbounds float addrspace(3)* %22, i64 %extract278.i
  %104 = getelementptr inbounds float addrspace(3)* %22, i64 %extract279.i
  %105 = getelementptr inbounds float addrspace(3)* %22, i64 %extract280.i
  %106 = getelementptr inbounds float addrspace(3)* %22, i64 %extract281.i
  %107 = getelementptr inbounds float addrspace(3)* %22, i64 %extract282.i
  %108 = getelementptr inbounds float addrspace(3)* %22, i64 %extract283.i
  %109 = getelementptr inbounds float addrspace(3)* %22, i64 %extract284.i
  %110 = getelementptr inbounds float addrspace(3)* %22, i64 %extract285.i
  %111 = getelementptr inbounds float addrspace(3)* %22, i64 %extract286.i
  %112 = getelementptr inbounds float addrspace(3)* %22, i64 %extract287.i
  %113 = getelementptr inbounds float addrspace(3)* %22, i64 %extract288.i
  br i1 %extract289.i, label %preload1021.i, label %postload1022.i

preload1021.i:                                    ; preds = %postload1004.i
  %extract273.i = extractelement <16 x i64> %idxprom36272.i, i32 0
  %114 = getelementptr inbounds float addrspace(3)* %22, i64 %extract273.i
  %masked_load781.i = load float addrspace(3)* %114, align 4
  br label %postload1022.i

postload1022.i:                                   ; preds = %preload1021.i, %postload1004.i
  %phi1023.i = phi float [ undef, %postload1004.i ], [ %masked_load781.i, %preload1021.i ]
  br i1 %extract290.i, label %preload1029.i, label %postload1030.i

preload1029.i:                                    ; preds = %postload1022.i
  %masked_load782.i = load float addrspace(3)* %99, align 4
  br label %postload1030.i

postload1030.i:                                   ; preds = %preload1029.i, %postload1022.i
  %phi1031.i = phi float [ undef, %postload1022.i ], [ %masked_load782.i, %preload1029.i ]
  br i1 %extract291.i, label %preload1037.i, label %postload1038.i

preload1037.i:                                    ; preds = %postload1030.i
  %masked_load783.i = load float addrspace(3)* %100, align 4
  br label %postload1038.i

postload1038.i:                                   ; preds = %preload1037.i, %postload1030.i
  %phi1039.i = phi float [ undef, %postload1030.i ], [ %masked_load783.i, %preload1037.i ]
  br i1 %extract292.i, label %preload1045.i, label %postload1046.i

preload1045.i:                                    ; preds = %postload1038.i
  %masked_load784.i = load float addrspace(3)* %101, align 4
  br label %postload1046.i

postload1046.i:                                   ; preds = %preload1045.i, %postload1038.i
  %phi1047.i = phi float [ undef, %postload1038.i ], [ %masked_load784.i, %preload1045.i ]
  br i1 %extract293.i, label %preload1053.i, label %postload1054.i

preload1053.i:                                    ; preds = %postload1046.i
  %masked_load785.i = load float addrspace(3)* %102, align 4
  br label %postload1054.i

postload1054.i:                                   ; preds = %preload1053.i, %postload1046.i
  %phi1055.i = phi float [ undef, %postload1046.i ], [ %masked_load785.i, %preload1053.i ]
  br i1 %extract294.i, label %preload1061.i, label %postload1062.i

preload1061.i:                                    ; preds = %postload1054.i
  %masked_load786.i = load float addrspace(3)* %103, align 4
  br label %postload1062.i

postload1062.i:                                   ; preds = %preload1061.i, %postload1054.i
  %phi1063.i = phi float [ undef, %postload1054.i ], [ %masked_load786.i, %preload1061.i ]
  br i1 %extract295.i, label %preload1069.i, label %postload1070.i

preload1069.i:                                    ; preds = %postload1062.i
  %masked_load787.i = load float addrspace(3)* %104, align 4
  br label %postload1070.i

postload1070.i:                                   ; preds = %preload1069.i, %postload1062.i
  %phi1071.i = phi float [ undef, %postload1062.i ], [ %masked_load787.i, %preload1069.i ]
  br i1 %extract296.i, label %preload1077.i, label %postload1078.i

preload1077.i:                                    ; preds = %postload1070.i
  %masked_load788.i = load float addrspace(3)* %105, align 4
  br label %postload1078.i

postload1078.i:                                   ; preds = %preload1077.i, %postload1070.i
  %phi1079.i = phi float [ undef, %postload1070.i ], [ %masked_load788.i, %preload1077.i ]
  br i1 %extract297.i, label %preload1085.i, label %postload1086.i

preload1085.i:                                    ; preds = %postload1078.i
  %masked_load789.i = load float addrspace(3)* %106, align 4
  br label %postload1086.i

postload1086.i:                                   ; preds = %preload1085.i, %postload1078.i
  %phi1087.i = phi float [ undef, %postload1078.i ], [ %masked_load789.i, %preload1085.i ]
  br i1 %extract298.i, label %preload1093.i, label %postload1094.i

preload1093.i:                                    ; preds = %postload1086.i
  %masked_load790.i = load float addrspace(3)* %107, align 4
  br label %postload1094.i

postload1094.i:                                   ; preds = %preload1093.i, %postload1086.i
  %phi1095.i = phi float [ undef, %postload1086.i ], [ %masked_load790.i, %preload1093.i ]
  br i1 %extract299.i, label %preload1101.i, label %postload1102.i

preload1101.i:                                    ; preds = %postload1094.i
  %masked_load791.i = load float addrspace(3)* %108, align 4
  br label %postload1102.i

postload1102.i:                                   ; preds = %preload1101.i, %postload1094.i
  %phi1103.i = phi float [ undef, %postload1094.i ], [ %masked_load791.i, %preload1101.i ]
  br i1 %extract300.i, label %preload1109.i, label %postload1110.i

preload1109.i:                                    ; preds = %postload1102.i
  %masked_load792.i = load float addrspace(3)* %109, align 4
  br label %postload1110.i

postload1110.i:                                   ; preds = %preload1109.i, %postload1102.i
  %phi1111.i = phi float [ undef, %postload1102.i ], [ %masked_load792.i, %preload1109.i ]
  br i1 %extract301.i, label %preload1117.i, label %postload1118.i

preload1117.i:                                    ; preds = %postload1110.i
  %masked_load793.i = load float addrspace(3)* %110, align 4
  br label %postload1118.i

postload1118.i:                                   ; preds = %preload1117.i, %postload1110.i
  %phi1119.i = phi float [ undef, %postload1110.i ], [ %masked_load793.i, %preload1117.i ]
  br i1 %extract302.i, label %preload1242.i, label %postload1243.i

preload1242.i:                                    ; preds = %postload1118.i
  %masked_load794.i = load float addrspace(3)* %111, align 4
  br label %postload1243.i

postload1243.i:                                   ; preds = %preload1242.i, %postload1118.i
  %phi1244.i = phi float [ undef, %postload1118.i ], [ %masked_load794.i, %preload1242.i ]
  br i1 %extract303.i, label %preload1250.i, label %postload1251.i

preload1250.i:                                    ; preds = %postload1243.i
  %masked_load795.i = load float addrspace(3)* %112, align 4
  br label %postload1251.i

postload1251.i:                                   ; preds = %preload1250.i, %postload1243.i
  %phi1252.i = phi float [ undef, %postload1243.i ], [ %masked_load795.i, %preload1250.i ]
  br i1 %extract304.i, label %preload1258.i, label %postload1259.i

preload1258.i:                                    ; preds = %postload1251.i
  %masked_load796.i = load float addrspace(3)* %113, align 4
  br label %postload1259.i

postload1259.i:                                   ; preds = %preload1258.i, %postload1251.i
  %phi1260.i = phi float [ undef, %postload1251.i ], [ %masked_load796.i, %preload1258.i ]
  %temp.vect338.i = insertelement <16 x float> undef, float %phi1023.i, i32 0
  %temp.vect339.i = insertelement <16 x float> %temp.vect338.i, float %phi1031.i, i32 1
  %temp.vect340.i = insertelement <16 x float> %temp.vect339.i, float %phi1039.i, i32 2
  %temp.vect341.i = insertelement <16 x float> %temp.vect340.i, float %phi1047.i, i32 3
  %temp.vect342.i = insertelement <16 x float> %temp.vect341.i, float %phi1055.i, i32 4
  %temp.vect343.i = insertelement <16 x float> %temp.vect342.i, float %phi1063.i, i32 5
  %temp.vect344.i = insertelement <16 x float> %temp.vect343.i, float %phi1071.i, i32 6
  %temp.vect345.i = insertelement <16 x float> %temp.vect344.i, float %phi1079.i, i32 7
  %temp.vect346.i = insertelement <16 x float> %temp.vect345.i, float %phi1087.i, i32 8
  %temp.vect347.i = insertelement <16 x float> %temp.vect346.i, float %phi1095.i, i32 9
  %temp.vect348.i = insertelement <16 x float> %temp.vect347.i, float %phi1103.i, i32 10
  %temp.vect349.i = insertelement <16 x float> %temp.vect348.i, float %phi1111.i, i32 11
  %temp.vect350.i = insertelement <16 x float> %temp.vect349.i, float %phi1119.i, i32 12
  %temp.vect351.i = insertelement <16 x float> %temp.vect350.i, float %phi1244.i, i32 13
  %temp.vect352.i = insertelement <16 x float> %temp.vect351.i, float %phi1252.i, i32 14
  %temp.vect353.i = insertelement <16 x float> %temp.vect352.i, float %phi1260.i, i32 15
  %idxprom38305.i = sext <16 x i32> %add35271.i to <16 x i64>
  %extract306.i = extractelement <16 x i64> %idxprom38305.i, i32 0
  %extract307.i = extractelement <16 x i64> %idxprom38305.i, i32 1
  %extract308.i = extractelement <16 x i64> %idxprom38305.i, i32 2
  %extract309.i = extractelement <16 x i64> %idxprom38305.i, i32 3
  %extract310.i = extractelement <16 x i64> %idxprom38305.i, i32 4
  %extract311.i = extractelement <16 x i64> %idxprom38305.i, i32 5
  %extract312.i = extractelement <16 x i64> %idxprom38305.i, i32 6
  %extract313.i = extractelement <16 x i64> %idxprom38305.i, i32 7
  %extract314.i = extractelement <16 x i64> %idxprom38305.i, i32 8
  %extract315.i = extractelement <16 x i64> %idxprom38305.i, i32 9
  %extract316.i = extractelement <16 x i64> %idxprom38305.i, i32 10
  %extract317.i = extractelement <16 x i64> %idxprom38305.i, i32 11
  %extract318.i = extractelement <16 x i64> %idxprom38305.i, i32 12
  %extract319.i = extractelement <16 x i64> %idxprom38305.i, i32 13
  %extract320.i = extractelement <16 x i64> %idxprom38305.i, i32 14
  %extract321.i = extractelement <16 x i64> %idxprom38305.i, i32 15
  %115 = getelementptr inbounds float addrspace(3)* %22, i64 %extract306.i
  %116 = getelementptr inbounds float addrspace(3)* %22, i64 %extract307.i
  %117 = getelementptr inbounds float addrspace(3)* %22, i64 %extract308.i
  %118 = getelementptr inbounds float addrspace(3)* %22, i64 %extract309.i
  %119 = getelementptr inbounds float addrspace(3)* %22, i64 %extract310.i
  %120 = getelementptr inbounds float addrspace(3)* %22, i64 %extract311.i
  %121 = getelementptr inbounds float addrspace(3)* %22, i64 %extract312.i
  %122 = getelementptr inbounds float addrspace(3)* %22, i64 %extract313.i
  %123 = getelementptr inbounds float addrspace(3)* %22, i64 %extract314.i
  %124 = getelementptr inbounds float addrspace(3)* %22, i64 %extract315.i
  %125 = getelementptr inbounds float addrspace(3)* %22, i64 %extract316.i
  %126 = getelementptr inbounds float addrspace(3)* %22, i64 %extract317.i
  %127 = getelementptr inbounds float addrspace(3)* %22, i64 %extract318.i
  %128 = getelementptr inbounds float addrspace(3)* %22, i64 %extract319.i
  %129 = getelementptr inbounds float addrspace(3)* %22, i64 %extract320.i
  %130 = getelementptr inbounds float addrspace(3)* %22, i64 %extract321.i
  br i1 %extract289.i, label %preload1024.i, label %postload1025.i

preload1024.i:                                    ; preds = %postload1259.i
  %masked_load797.i = load float addrspace(3)* %115, align 4
  br label %postload1025.i

postload1025.i:                                   ; preds = %preload1024.i, %postload1259.i
  %phi1026.i = phi float [ undef, %postload1259.i ], [ %masked_load797.i, %preload1024.i ]
  br i1 %extract290.i, label %preload1032.i, label %postload1033.i

preload1032.i:                                    ; preds = %postload1025.i
  %masked_load798.i = load float addrspace(3)* %116, align 4
  br label %postload1033.i

postload1033.i:                                   ; preds = %preload1032.i, %postload1025.i
  %phi1034.i = phi float [ undef, %postload1025.i ], [ %masked_load798.i, %preload1032.i ]
  br i1 %extract291.i, label %preload1040.i, label %postload1041.i

preload1040.i:                                    ; preds = %postload1033.i
  %masked_load799.i = load float addrspace(3)* %117, align 4
  br label %postload1041.i

postload1041.i:                                   ; preds = %preload1040.i, %postload1033.i
  %phi1042.i = phi float [ undef, %postload1033.i ], [ %masked_load799.i, %preload1040.i ]
  br i1 %extract292.i, label %preload1048.i, label %postload1049.i

preload1048.i:                                    ; preds = %postload1041.i
  %masked_load800.i = load float addrspace(3)* %118, align 4
  br label %postload1049.i

postload1049.i:                                   ; preds = %preload1048.i, %postload1041.i
  %phi1050.i = phi float [ undef, %postload1041.i ], [ %masked_load800.i, %preload1048.i ]
  br i1 %extract293.i, label %preload1056.i, label %postload1057.i

preload1056.i:                                    ; preds = %postload1049.i
  %masked_load801.i = load float addrspace(3)* %119, align 4
  br label %postload1057.i

postload1057.i:                                   ; preds = %preload1056.i, %postload1049.i
  %phi1058.i = phi float [ undef, %postload1049.i ], [ %masked_load801.i, %preload1056.i ]
  br i1 %extract294.i, label %preload1064.i, label %postload1065.i

preload1064.i:                                    ; preds = %postload1057.i
  %masked_load802.i = load float addrspace(3)* %120, align 4
  br label %postload1065.i

postload1065.i:                                   ; preds = %preload1064.i, %postload1057.i
  %phi1066.i = phi float [ undef, %postload1057.i ], [ %masked_load802.i, %preload1064.i ]
  br i1 %extract295.i, label %preload1072.i, label %postload1073.i

preload1072.i:                                    ; preds = %postload1065.i
  %masked_load803.i = load float addrspace(3)* %121, align 4
  br label %postload1073.i

postload1073.i:                                   ; preds = %preload1072.i, %postload1065.i
  %phi1074.i = phi float [ undef, %postload1065.i ], [ %masked_load803.i, %preload1072.i ]
  br i1 %extract296.i, label %preload1080.i, label %postload1081.i

preload1080.i:                                    ; preds = %postload1073.i
  %masked_load804.i = load float addrspace(3)* %122, align 4
  br label %postload1081.i

postload1081.i:                                   ; preds = %preload1080.i, %postload1073.i
  %phi1082.i = phi float [ undef, %postload1073.i ], [ %masked_load804.i, %preload1080.i ]
  br i1 %extract297.i, label %preload1088.i, label %postload1089.i

preload1088.i:                                    ; preds = %postload1081.i
  %masked_load805.i = load float addrspace(3)* %123, align 4
  br label %postload1089.i

postload1089.i:                                   ; preds = %preload1088.i, %postload1081.i
  %phi1090.i = phi float [ undef, %postload1081.i ], [ %masked_load805.i, %preload1088.i ]
  br i1 %extract298.i, label %preload1096.i, label %postload1097.i

preload1096.i:                                    ; preds = %postload1089.i
  %masked_load806.i = load float addrspace(3)* %124, align 4
  br label %postload1097.i

postload1097.i:                                   ; preds = %preload1096.i, %postload1089.i
  %phi1098.i = phi float [ undef, %postload1089.i ], [ %masked_load806.i, %preload1096.i ]
  br i1 %extract299.i, label %preload1104.i, label %postload1105.i

preload1104.i:                                    ; preds = %postload1097.i
  %masked_load807.i = load float addrspace(3)* %125, align 4
  br label %postload1105.i

postload1105.i:                                   ; preds = %preload1104.i, %postload1097.i
  %phi1106.i = phi float [ undef, %postload1097.i ], [ %masked_load807.i, %preload1104.i ]
  br i1 %extract300.i, label %preload1112.i, label %postload1113.i

preload1112.i:                                    ; preds = %postload1105.i
  %masked_load808.i = load float addrspace(3)* %126, align 4
  br label %postload1113.i

postload1113.i:                                   ; preds = %preload1112.i, %postload1105.i
  %phi1114.i = phi float [ undef, %postload1105.i ], [ %masked_load808.i, %preload1112.i ]
  br i1 %extract301.i, label %preload1120.i, label %postload1121.i

preload1120.i:                                    ; preds = %postload1113.i
  %masked_load809.i = load float addrspace(3)* %127, align 4
  br label %postload1121.i

postload1121.i:                                   ; preds = %preload1120.i, %postload1113.i
  %phi1122.i = phi float [ undef, %postload1113.i ], [ %masked_load809.i, %preload1120.i ]
  br i1 %extract302.i, label %preload1245.i, label %postload1246.i

preload1245.i:                                    ; preds = %postload1121.i
  %masked_load810.i = load float addrspace(3)* %128, align 4
  br label %postload1246.i

postload1246.i:                                   ; preds = %preload1245.i, %postload1121.i
  %phi1247.i = phi float [ undef, %postload1121.i ], [ %masked_load810.i, %preload1245.i ]
  br i1 %extract303.i, label %preload1253.i, label %postload1254.i

preload1253.i:                                    ; preds = %postload1246.i
  %masked_load811.i = load float addrspace(3)* %129, align 4
  br label %postload1254.i

postload1254.i:                                   ; preds = %preload1253.i, %postload1246.i
  %phi1255.i = phi float [ undef, %postload1246.i ], [ %masked_load811.i, %preload1253.i ]
  br i1 %extract304.i, label %preload1261.i, label %postload1262.i

preload1261.i:                                    ; preds = %postload1254.i
  %masked_load812.i = load float addrspace(3)* %130, align 4
  br label %postload1262.i

postload1262.i:                                   ; preds = %preload1261.i, %postload1254.i
  %phi1263.i = phi float [ undef, %postload1254.i ], [ %masked_load812.i, %preload1261.i ]
  %temp.vect322.i = insertelement <16 x float> undef, float %phi1026.i, i32 0
  %temp.vect323.i = insertelement <16 x float> %temp.vect322.i, float %phi1034.i, i32 1
  %temp.vect324.i = insertelement <16 x float> %temp.vect323.i, float %phi1042.i, i32 2
  %temp.vect325.i = insertelement <16 x float> %temp.vect324.i, float %phi1050.i, i32 3
  %temp.vect326.i = insertelement <16 x float> %temp.vect325.i, float %phi1058.i, i32 4
  %temp.vect327.i = insertelement <16 x float> %temp.vect326.i, float %phi1066.i, i32 5
  %temp.vect328.i = insertelement <16 x float> %temp.vect327.i, float %phi1074.i, i32 6
  %temp.vect329.i = insertelement <16 x float> %temp.vect328.i, float %phi1082.i, i32 7
  %temp.vect330.i = insertelement <16 x float> %temp.vect329.i, float %phi1090.i, i32 8
  %temp.vect331.i = insertelement <16 x float> %temp.vect330.i, float %phi1098.i, i32 9
  %temp.vect332.i = insertelement <16 x float> %temp.vect331.i, float %phi1106.i, i32 10
  %temp.vect333.i = insertelement <16 x float> %temp.vect332.i, float %phi1114.i, i32 11
  %temp.vect334.i = insertelement <16 x float> %temp.vect333.i, float %phi1122.i, i32 12
  %temp.vect335.i = insertelement <16 x float> %temp.vect334.i, float %phi1247.i, i32 13
  %temp.vect336.i = insertelement <16 x float> %temp.vect335.i, float %phi1255.i, i32 14
  %temp.vect337.i = insertelement <16 x float> %temp.vect336.i, float %phi1263.i, i32 15
  %add40354.i = fadd <16 x float> %temp.vect337.i, %temp.vect353.i
  %extract356.i = extractelement <16 x float> %add40354.i, i32 1
  %extract357.i = extractelement <16 x float> %add40354.i, i32 2
  %extract358.i = extractelement <16 x float> %add40354.i, i32 3
  %extract359.i = extractelement <16 x float> %add40354.i, i32 4
  %extract360.i = extractelement <16 x float> %add40354.i, i32 5
  %extract361.i = extractelement <16 x float> %add40354.i, i32 6
  %extract362.i = extractelement <16 x float> %add40354.i, i32 7
  %extract363.i = extractelement <16 x float> %add40354.i, i32 8
  %extract364.i = extractelement <16 x float> %add40354.i, i32 9
  %extract365.i = extractelement <16 x float> %add40354.i, i32 10
  %extract366.i = extractelement <16 x float> %add40354.i, i32 11
  %extract367.i = extractelement <16 x float> %add40354.i, i32 12
  %extract368.i = extractelement <16 x float> %add40354.i, i32 13
  %extract369.i = extractelement <16 x float> %add40354.i, i32 14
  %extract370.i = extractelement <16 x float> %add40354.i, i32 15
  br i1 %extract289.i, label %preload1027.i, label %postload1028.i

preload1027.i:                                    ; preds = %postload1262.i
  %extract355.i = extractelement <16 x float> %add40354.i, i32 0
  store float %extract355.i, float addrspace(3)* %115, align 4
  br label %postload1028.i

postload1028.i:                                   ; preds = %preload1027.i, %postload1262.i
  br i1 %extract290.i, label %preload1035.i, label %postload1036.i

preload1035.i:                                    ; preds = %postload1028.i
  store float %extract356.i, float addrspace(3)* %116, align 4
  br label %postload1036.i

postload1036.i:                                   ; preds = %preload1035.i, %postload1028.i
  br i1 %extract291.i, label %preload1043.i, label %postload1044.i

preload1043.i:                                    ; preds = %postload1036.i
  store float %extract357.i, float addrspace(3)* %117, align 4
  br label %postload1044.i

postload1044.i:                                   ; preds = %preload1043.i, %postload1036.i
  br i1 %extract292.i, label %preload1051.i, label %postload1052.i

preload1051.i:                                    ; preds = %postload1044.i
  store float %extract358.i, float addrspace(3)* %118, align 4
  br label %postload1052.i

postload1052.i:                                   ; preds = %preload1051.i, %postload1044.i
  br i1 %extract293.i, label %preload1059.i, label %postload1060.i

preload1059.i:                                    ; preds = %postload1052.i
  store float %extract359.i, float addrspace(3)* %119, align 4
  br label %postload1060.i

postload1060.i:                                   ; preds = %preload1059.i, %postload1052.i
  br i1 %extract294.i, label %preload1067.i, label %postload1068.i

preload1067.i:                                    ; preds = %postload1060.i
  store float %extract360.i, float addrspace(3)* %120, align 4
  br label %postload1068.i

postload1068.i:                                   ; preds = %preload1067.i, %postload1060.i
  br i1 %extract295.i, label %preload1075.i, label %postload1076.i

preload1075.i:                                    ; preds = %postload1068.i
  store float %extract361.i, float addrspace(3)* %121, align 4
  br label %postload1076.i

postload1076.i:                                   ; preds = %preload1075.i, %postload1068.i
  br i1 %extract296.i, label %preload1083.i, label %postload1084.i

preload1083.i:                                    ; preds = %postload1076.i
  store float %extract362.i, float addrspace(3)* %122, align 4
  br label %postload1084.i

postload1084.i:                                   ; preds = %preload1083.i, %postload1076.i
  br i1 %extract297.i, label %preload1091.i, label %postload1092.i

preload1091.i:                                    ; preds = %postload1084.i
  store float %extract363.i, float addrspace(3)* %123, align 4
  br label %postload1092.i

postload1092.i:                                   ; preds = %preload1091.i, %postload1084.i
  br i1 %extract298.i, label %preload1099.i, label %postload1100.i

preload1099.i:                                    ; preds = %postload1092.i
  store float %extract364.i, float addrspace(3)* %124, align 4
  br label %postload1100.i

postload1100.i:                                   ; preds = %preload1099.i, %postload1092.i
  br i1 %extract299.i, label %preload1107.i, label %postload1108.i

preload1107.i:                                    ; preds = %postload1100.i
  store float %extract365.i, float addrspace(3)* %125, align 4
  br label %postload1108.i

postload1108.i:                                   ; preds = %preload1107.i, %postload1100.i
  br i1 %extract300.i, label %preload1115.i, label %postload1116.i

preload1115.i:                                    ; preds = %postload1108.i
  store float %extract366.i, float addrspace(3)* %126, align 4
  br label %postload1116.i

postload1116.i:                                   ; preds = %preload1115.i, %postload1108.i
  br i1 %extract301.i, label %preload1123.i, label %postload1124.i

preload1123.i:                                    ; preds = %postload1116.i
  store float %extract367.i, float addrspace(3)* %127, align 4
  br label %postload1124.i

postload1124.i:                                   ; preds = %preload1123.i, %postload1116.i
  br i1 %extract302.i, label %preload1248.i, label %postload1249.i

preload1248.i:                                    ; preds = %postload1124.i
  store float %extract368.i, float addrspace(3)* %128, align 4
  br label %postload1249.i

postload1249.i:                                   ; preds = %preload1248.i, %postload1124.i
  br i1 %extract303.i, label %preload1256.i, label %postload1257.i

preload1256.i:                                    ; preds = %postload1249.i
  store float %extract369.i, float addrspace(3)* %129, align 4
  br label %postload1257.i

postload1257.i:                                   ; preds = %preload1256.i, %postload1249.i
  br i1 %extract304.i, label %preload1264.i, label %postload1257.i.if.end41.i_crit_edge

postload1257.i.if.end41.i_crit_edge:              ; preds = %postload1257.i
  br label %if.end41.i

preload1264.i:                                    ; preds = %postload1257.i
  store float %extract370.i, float addrspace(3)* %130, align 4
  br label %if.end41.i

if.end41.i:                                       ; preds = %postload1257.i.if.end41.i_crit_edge, %preload1264.i
  %loadedValue2587.i = load i32* %CastToValueType2591.i, align 4
  %mul42.i = shl i32 %loadedValue2587.i, 1
  %temp373.i = insertelement <16 x i32> undef, i32 %mul42.i, i32 0
  %vector374.i = shufflevector <16 x i32> %temp373.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)2542.i" = add nuw i64 %CurrSBIndex..3.i, 832
  %"&pSB[currWI].offset2543.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2542.i"
  %CastToValueType2544.i = bitcast i8* %"&pSB[currWI].offset2543.i" to <16 x i32>*
  %loadedValue2545.i = load <16 x i32>* %CastToValueType2544.i, align 64
  %loadedValue2554.i = load <16 x i1>* %CastToValueType2568.i, align 16
  %out_sel375.i = select <16 x i1> %loadedValue2554.i, <16 x i32> %vector374.i, <16 x i32> %loadedValue2545.i
  %"&(pSB[currWI].offset)2575.i" = add nuw i64 %CurrSBIndex..3.i, 900
  %"&pSB[currWI].offset2576.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2575.i"
  %CastToValueType2577.i = bitcast i8* %"&pSB[currWI].offset2576.i" to i32*
  %loadedValue2578.i = load i32* %CastToValueType2577.i, align 4
  %shr.i = ashr i32 %loadedValue2578.i, 1
  %cmp27.i = icmp sgt i32 %shr.i, 0
  %temp399.i = insertelement <16 x i1> undef, i1 %cmp27.i, i32 0
  %vector400.i = shufflevector <16 x i1> %temp399.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %notCond.i = xor i1 %cmp27.i, true
  %temp376.i = insertelement <16 x i1> undef, i1 %notCond.i, i32 0
  %vector377.i = shufflevector <16 x i1> %temp376.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %who_left_tr378.i = and <16 x i1> %loadedValue2554.i, %vector377.i
  %"&(pSB[currWI].offset)2533.i" = add nuw i64 %CurrSBIndex..3.i, 768
  %"&pSB[currWI].offset2534.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2533.i"
  %CastToValueType2535.i = bitcast i8* %"&pSB[currWI].offset2534.i" to <16 x i1>*
  %loadedValue2536.i = load <16 x i1>* %CastToValueType2535.i, align 16
  %loop_mask7380.i = or <16 x i1> %loadedValue2536.i, %who_left_tr378.i
  %ipred.i5.i = bitcast <16 x i1> %loop_mask7380.i to i16
  %val.i6.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i5.i, i16 %ipred.i5.i) nounwind
  %131 = and i32 %val.i6.i, 1
  %res.i7.i = icmp eq i32 %131, 0
  %local_edge401.i = and <16 x i1> %loadedValue2554.i, %vector400.i
  br i1 %res.i7.i, label %for.body.i, label %if.end41.i.for.end.i_crit_edge

if.end41.i.for.end.i_crit_edge:                   ; preds = %if.end41.i
  br label %for.end.i

for.end.i:                                        ; preds = %if.end41.i.for.end.i_crit_edge, %if.end24.i
  %CurrWI..4.i = phi i64 [ %CurrWI..0.i, %if.end24.i ], [ %CurrWI..3.i, %if.end41.i.for.end.i_crit_edge ]
  %CurrSBIndex..4.i = phi i64 [ %CurrSBIndex..0.i, %if.end24.i ], [ %CurrSBIndex..3.i, %if.end41.i.for.end.i_crit_edge ]
  %currBarrier.4.i = phi i32 [ %currBarrier.0.i, %if.end24.i ], [ %currBarrier.3.i, %if.end41.i.for.end.i_crit_edge ]
  %vectorPHI418.i = phi <16 x i32> [ undef, %if.end24.i ], [ %out_sel375.i, %if.end41.i.for.end.i_crit_edge ]
  %"&(pSB[currWI].offset)2515.i" = add nuw i64 %CurrSBIndex..4.i, 640
  %"&pSB[currWI].offset2516.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2515.i"
  %CastToValueType2517.i = bitcast i8* %"&pSB[currWI].offset2516.i" to i1*
  %loadedValue2518.i = load i1* %CastToValueType2517.i, align 1
  %merge67419.i = select i1 %loadedValue2518.i, <16 x i32> %vectorPHI418.i, <16 x i32> <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %"&(pSB[currWI].offset)2617.i" = add nuw i64 %CurrSBIndex..4.i, 1088
  %"&pSB[currWI].offset2618.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2617.i"
  %CastToValueType2619.i = bitcast i8* %"&pSB[currWI].offset2618.i" to <16 x i32>*
  store <16 x i32> %merge67419.i, <16 x i32>* %CastToValueType2619.i, align 64
  br i1 %cmp43.i, label %preload1140.i, label %cond.end.i

preload1140.i:                                    ; preds = %for.end.i
  %132 = load i64* %28, align 8
  br label %cond.end.i

cond.end.i:                                       ; preds = %preload1140.i, %for.end.i
  %phi1142.i = phi i64 [ undef, %for.end.i ], [ %132, %preload1140.i ]
  %merge69.i = select i1 %cmp43.i, i64 %phi1142.i, i64 %conv461623.i
  %"&(pSB[currWI].offset)1692.i" = add nuw i64 %CurrSBIndex..4.i, 128
  %"&pSB[currWI].offset1693.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1692.i"
  %CastToValueType1694.i = bitcast i8* %"&pSB[currWI].offset1693.i" to <16 x i64>*
  %loadedValue1695.i = load <16 x i64>* %CastToValueType1694.i, align 128
  %cmp49.i = icmp eq <16 x i64> %loadedValue1695.i, zeroinitializer
  %extract440.i = extractelement <16 x i1> %cmp49.i, i32 0
  %extract441.i = extractelement <16 x i1> %cmp49.i, i32 1
  %extract442.i = extractelement <16 x i1> %cmp49.i, i32 2
  %extract443.i = extractelement <16 x i1> %cmp49.i, i32 3
  %extract444.i = extractelement <16 x i1> %cmp49.i, i32 4
  %extract445.i = extractelement <16 x i1> %cmp49.i, i32 5
  %extract446.i = extractelement <16 x i1> %cmp49.i, i32 6
  %extract447.i = extractelement <16 x i1> %cmp49.i, i32 7
  %extract448.i = extractelement <16 x i1> %cmp49.i, i32 8
  %extract449.i = extractelement <16 x i1> %cmp49.i, i32 9
  %extract450.i = extractelement <16 x i1> %cmp49.i, i32 10
  %extract451.i = extractelement <16 x i1> %cmp49.i, i32 11
  %extract452.i = extractelement <16 x i1> %cmp49.i, i32 12
  %extract453.i = extractelement <16 x i1> %cmp49.i, i32 13
  %extract454.i = extractelement <16 x i1> %cmp49.i, i32 14
  %extract455.i = extractelement <16 x i1> %cmp49.i, i32 15
  %ipred.i2.i = bitcast <16 x i1> %cmp49.i to i16
  %val.i3.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i2.i, i16 %ipred.i2.i) nounwind
  %133 = and i32 %val.i3.i, 1
  %res.i4.i = icmp eq i32 %133, 0
  br i1 %res.i4.i, label %if.then51.i, label %if.end66.i

if.then51.i:                                      ; preds = %cond.end.i
  %"&pSB[currWI].offset1679.i" = getelementptr inbounds i8* %37, i64 %CurrSBIndex..4.i
  %CastToValueType1680.i = bitcast i8* %"&pSB[currWI].offset1679.i" to i64*
  %loadedValue1681.i = load i64* %CastToValueType1680.i, align 8
  %shl53.i = shl i64 %loadedValue1681.i, 33
  %sext9.i = add i64 %shl53.i, -4294967296
  %idxprom59.i = ashr exact i64 %sext9.i, 32
  %arrayidx60.i = getelementptr inbounds float addrspace(3)* %22, i64 %idxprom59.i
  %if.then51_to_if.then58423.i = and <16 x i1> %cmp49.i, %vector422.i
  %extract424.i = extractelement <16 x i1> %if.then51_to_if.then58423.i, i32 0
  %extract425.i = extractelement <16 x i1> %if.then51_to_if.then58423.i, i32 1
  %extract426.i = extractelement <16 x i1> %if.then51_to_if.then58423.i, i32 2
  %extract427.i = extractelement <16 x i1> %if.then51_to_if.then58423.i, i32 3
  %extract428.i = extractelement <16 x i1> %if.then51_to_if.then58423.i, i32 4
  %extract429.i = extractelement <16 x i1> %if.then51_to_if.then58423.i, i32 5
  %extract430.i = extractelement <16 x i1> %if.then51_to_if.then58423.i, i32 6
  %extract431.i = extractelement <16 x i1> %if.then51_to_if.then58423.i, i32 7
  %extract432.i = extractelement <16 x i1> %if.then51_to_if.then58423.i, i32 8
  %extract433.i = extractelement <16 x i1> %if.then51_to_if.then58423.i, i32 9
  %extract434.i = extractelement <16 x i1> %if.then51_to_if.then58423.i, i32 10
  %extract435.i = extractelement <16 x i1> %if.then51_to_if.then58423.i, i32 11
  %extract436.i = extractelement <16 x i1> %if.then51_to_if.then58423.i, i32 12
  %extract437.i = extractelement <16 x i1> %if.then51_to_if.then58423.i, i32 13
  %extract438.i = extractelement <16 x i1> %if.then51_to_if.then58423.i, i32 14
  %extract439.i = extractelement <16 x i1> %if.then51_to_if.then58423.i, i32 15
  br i1 %extract424.i, label %preload1266.i, label %postload1267.i

preload1266.i:                                    ; preds = %if.then51.i
  %masked_load813.i = load float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1267.i

postload1267.i:                                   ; preds = %preload1266.i, %if.then51.i
  %phi1268.i = phi float [ undef, %if.then51.i ], [ %masked_load813.i, %preload1266.i ]
  br i1 %extract425.i, label %preload1271.i, label %postload1272.i

preload1271.i:                                    ; preds = %postload1267.i
  %masked_load814.i = load float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1272.i

postload1272.i:                                   ; preds = %preload1271.i, %postload1267.i
  %phi1273.i = phi float [ undef, %postload1267.i ], [ %masked_load814.i, %preload1271.i ]
  br i1 %extract426.i, label %preload1276.i, label %postload1277.i

preload1276.i:                                    ; preds = %postload1272.i
  %masked_load815.i = load float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1277.i

postload1277.i:                                   ; preds = %preload1276.i, %postload1272.i
  %phi1278.i = phi float [ undef, %postload1272.i ], [ %masked_load815.i, %preload1276.i ]
  br i1 %extract427.i, label %preload1281.i, label %postload1282.i

preload1281.i:                                    ; preds = %postload1277.i
  %masked_load816.i = load float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1282.i

postload1282.i:                                   ; preds = %preload1281.i, %postload1277.i
  %phi1283.i = phi float [ undef, %postload1277.i ], [ %masked_load816.i, %preload1281.i ]
  br i1 %extract428.i, label %preload1286.i, label %postload1287.i

preload1286.i:                                    ; preds = %postload1282.i
  %masked_load817.i = load float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1287.i

postload1287.i:                                   ; preds = %preload1286.i, %postload1282.i
  %phi1288.i = phi float [ undef, %postload1282.i ], [ %masked_load817.i, %preload1286.i ]
  br i1 %extract429.i, label %preload1291.i, label %postload1292.i

preload1291.i:                                    ; preds = %postload1287.i
  %masked_load818.i = load float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1292.i

postload1292.i:                                   ; preds = %preload1291.i, %postload1287.i
  %phi1293.i = phi float [ undef, %postload1287.i ], [ %masked_load818.i, %preload1291.i ]
  br i1 %extract430.i, label %preload1296.i, label %postload1297.i

preload1296.i:                                    ; preds = %postload1292.i
  %masked_load819.i = load float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1297.i

postload1297.i:                                   ; preds = %preload1296.i, %postload1292.i
  %phi1298.i = phi float [ undef, %postload1292.i ], [ %masked_load819.i, %preload1296.i ]
  br i1 %extract431.i, label %preload1301.i, label %postload1302.i

preload1301.i:                                    ; preds = %postload1297.i
  %masked_load820.i = load float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1302.i

postload1302.i:                                   ; preds = %preload1301.i, %postload1297.i
  %phi1303.i = phi float [ undef, %postload1297.i ], [ %masked_load820.i, %preload1301.i ]
  br i1 %extract432.i, label %preload1306.i, label %postload1307.i

preload1306.i:                                    ; preds = %postload1302.i
  %masked_load821.i = load float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1307.i

postload1307.i:                                   ; preds = %preload1306.i, %postload1302.i
  %phi1308.i = phi float [ undef, %postload1302.i ], [ %masked_load821.i, %preload1306.i ]
  br i1 %extract433.i, label %preload1311.i, label %postload1312.i

preload1311.i:                                    ; preds = %postload1307.i
  %masked_load822.i = load float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1312.i

postload1312.i:                                   ; preds = %preload1311.i, %postload1307.i
  %phi1313.i = phi float [ undef, %postload1307.i ], [ %masked_load822.i, %preload1311.i ]
  br i1 %extract434.i, label %preload1316.i, label %postload1317.i

preload1316.i:                                    ; preds = %postload1312.i
  %masked_load823.i = load float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1317.i

postload1317.i:                                   ; preds = %preload1316.i, %postload1312.i
  %phi1318.i = phi float [ undef, %postload1312.i ], [ %masked_load823.i, %preload1316.i ]
  br i1 %extract435.i, label %preload1321.i, label %postload1322.i

preload1321.i:                                    ; preds = %postload1317.i
  %masked_load824.i = load float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1322.i

postload1322.i:                                   ; preds = %preload1321.i, %postload1317.i
  %phi1323.i = phi float [ undef, %postload1317.i ], [ %masked_load824.i, %preload1321.i ]
  br i1 %extract436.i, label %preload1326.i, label %postload1327.i

preload1326.i:                                    ; preds = %postload1322.i
  %masked_load825.i = load float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1327.i

postload1327.i:                                   ; preds = %preload1326.i, %postload1322.i
  %phi1328.i = phi float [ undef, %postload1322.i ], [ %masked_load825.i, %preload1326.i ]
  br i1 %extract437.i, label %preload1331.i, label %postload1332.i

preload1331.i:                                    ; preds = %postload1327.i
  %masked_load826.i = load float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1332.i

postload1332.i:                                   ; preds = %preload1331.i, %postload1327.i
  %phi1333.i = phi float [ undef, %postload1327.i ], [ %masked_load826.i, %preload1331.i ]
  br i1 %extract438.i, label %preload1336.i, label %postload1337.i

preload1336.i:                                    ; preds = %postload1332.i
  %masked_load827.i = load float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1337.i

postload1337.i:                                   ; preds = %preload1336.i, %postload1332.i
  %phi1338.i = phi float [ undef, %postload1332.i ], [ %masked_load827.i, %preload1336.i ]
  br i1 %extract439.i, label %preload1341.i, label %postload1342.i

preload1341.i:                                    ; preds = %postload1337.i
  %masked_load828.i = load float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1342.i

postload1342.i:                                   ; preds = %preload1341.i, %postload1337.i
  %phi1343.i = phi float [ undef, %postload1337.i ], [ %masked_load828.i, %preload1341.i ]
  %sext.i = shl i64 %merge69.i, 32
  %idxprom61.i = ashr exact i64 %sext.i, 32
  %arrayidx62.i = getelementptr inbounds float addrspace(1)* %7, i64 %idxprom61.i
  br i1 %extract424.i, label %preload1269.i, label %postload1270.i

preload1269.i:                                    ; preds = %postload1342.i
  store float %phi1268.i, float addrspace(1)* %arrayidx62.i, align 4
  br label %postload1270.i

postload1270.i:                                   ; preds = %preload1269.i, %postload1342.i
  br i1 %extract425.i, label %preload1274.i, label %postload1275.i

preload1274.i:                                    ; preds = %postload1270.i
  store float %phi1273.i, float addrspace(1)* %arrayidx62.i, align 4
  br label %postload1275.i

postload1275.i:                                   ; preds = %preload1274.i, %postload1270.i
  br i1 %extract426.i, label %preload1279.i, label %postload1280.i

preload1279.i:                                    ; preds = %postload1275.i
  store float %phi1278.i, float addrspace(1)* %arrayidx62.i, align 4
  br label %postload1280.i

postload1280.i:                                   ; preds = %preload1279.i, %postload1275.i
  br i1 %extract427.i, label %preload1284.i, label %postload1285.i

preload1284.i:                                    ; preds = %postload1280.i
  store float %phi1283.i, float addrspace(1)* %arrayidx62.i, align 4
  br label %postload1285.i

postload1285.i:                                   ; preds = %preload1284.i, %postload1280.i
  br i1 %extract428.i, label %preload1289.i, label %postload1290.i

preload1289.i:                                    ; preds = %postload1285.i
  store float %phi1288.i, float addrspace(1)* %arrayidx62.i, align 4
  br label %postload1290.i

postload1290.i:                                   ; preds = %preload1289.i, %postload1285.i
  br i1 %extract429.i, label %preload1294.i, label %postload1295.i

preload1294.i:                                    ; preds = %postload1290.i
  store float %phi1293.i, float addrspace(1)* %arrayidx62.i, align 4
  br label %postload1295.i

postload1295.i:                                   ; preds = %preload1294.i, %postload1290.i
  br i1 %extract430.i, label %preload1299.i, label %postload1300.i

preload1299.i:                                    ; preds = %postload1295.i
  store float %phi1298.i, float addrspace(1)* %arrayidx62.i, align 4
  br label %postload1300.i

postload1300.i:                                   ; preds = %preload1299.i, %postload1295.i
  br i1 %extract431.i, label %preload1304.i, label %postload1305.i

preload1304.i:                                    ; preds = %postload1300.i
  store float %phi1303.i, float addrspace(1)* %arrayidx62.i, align 4
  br label %postload1305.i

postload1305.i:                                   ; preds = %preload1304.i, %postload1300.i
  br i1 %extract432.i, label %preload1309.i, label %postload1310.i

preload1309.i:                                    ; preds = %postload1305.i
  store float %phi1308.i, float addrspace(1)* %arrayidx62.i, align 4
  br label %postload1310.i

postload1310.i:                                   ; preds = %preload1309.i, %postload1305.i
  br i1 %extract433.i, label %preload1314.i, label %postload1315.i

preload1314.i:                                    ; preds = %postload1310.i
  store float %phi1313.i, float addrspace(1)* %arrayidx62.i, align 4
  br label %postload1315.i

postload1315.i:                                   ; preds = %preload1314.i, %postload1310.i
  br i1 %extract434.i, label %preload1319.i, label %postload1320.i

preload1319.i:                                    ; preds = %postload1315.i
  store float %phi1318.i, float addrspace(1)* %arrayidx62.i, align 4
  br label %postload1320.i

postload1320.i:                                   ; preds = %preload1319.i, %postload1315.i
  br i1 %extract435.i, label %preload1324.i, label %postload1325.i

preload1324.i:                                    ; preds = %postload1320.i
  store float %phi1323.i, float addrspace(1)* %arrayidx62.i, align 4
  br label %postload1325.i

postload1325.i:                                   ; preds = %preload1324.i, %postload1320.i
  br i1 %extract436.i, label %preload1329.i, label %postload1330.i

preload1329.i:                                    ; preds = %postload1325.i
  store float %phi1328.i, float addrspace(1)* %arrayidx62.i, align 4
  br label %postload1330.i

postload1330.i:                                   ; preds = %preload1329.i, %postload1325.i
  br i1 %extract437.i, label %preload1334.i, label %postload1335.i

preload1334.i:                                    ; preds = %postload1330.i
  store float %phi1333.i, float addrspace(1)* %arrayidx62.i, align 4
  br label %postload1335.i

postload1335.i:                                   ; preds = %preload1334.i, %postload1330.i
  br i1 %extract438.i, label %preload1339.i, label %postload1340.i

preload1339.i:                                    ; preds = %postload1335.i
  store float %phi1338.i, float addrspace(1)* %arrayidx62.i, align 4
  br label %postload1340.i

postload1340.i:                                   ; preds = %preload1339.i, %postload1335.i
  br i1 %extract439.i, label %preload1344.i, label %if.end63.i

preload1344.i:                                    ; preds = %postload1340.i
  store float %phi1343.i, float addrspace(1)* %arrayidx62.i, align 4
  br label %if.end63.i

if.end63.i:                                       ; preds = %preload1344.i, %postload1340.i
  br i1 %extract440.i, label %preload1346.i, label %postload1347.i

preload1346.i:                                    ; preds = %if.end63.i
  store float 0.000000e+00, float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1347.i

postload1347.i:                                   ; preds = %preload1346.i, %if.end63.i
  br i1 %extract441.i, label %preload1348.i, label %postload1349.i

preload1348.i:                                    ; preds = %postload1347.i
  store float 0.000000e+00, float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1349.i

postload1349.i:                                   ; preds = %preload1348.i, %postload1347.i
  br i1 %extract442.i, label %preload1350.i, label %postload1351.i

preload1350.i:                                    ; preds = %postload1349.i
  store float 0.000000e+00, float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1351.i

postload1351.i:                                   ; preds = %preload1350.i, %postload1349.i
  br i1 %extract443.i, label %preload1352.i, label %postload1353.i

preload1352.i:                                    ; preds = %postload1351.i
  store float 0.000000e+00, float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1353.i

postload1353.i:                                   ; preds = %preload1352.i, %postload1351.i
  br i1 %extract444.i, label %preload1354.i, label %postload1355.i

preload1354.i:                                    ; preds = %postload1353.i
  store float 0.000000e+00, float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1355.i

postload1355.i:                                   ; preds = %preload1354.i, %postload1353.i
  br i1 %extract445.i, label %preload1356.i, label %postload1357.i

preload1356.i:                                    ; preds = %postload1355.i
  store float 0.000000e+00, float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1357.i

postload1357.i:                                   ; preds = %preload1356.i, %postload1355.i
  br i1 %extract446.i, label %preload1358.i, label %postload1359.i

preload1358.i:                                    ; preds = %postload1357.i
  store float 0.000000e+00, float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1359.i

postload1359.i:                                   ; preds = %preload1358.i, %postload1357.i
  br i1 %extract447.i, label %preload1360.i, label %postload1361.i

preload1360.i:                                    ; preds = %postload1359.i
  store float 0.000000e+00, float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1361.i

postload1361.i:                                   ; preds = %preload1360.i, %postload1359.i
  br i1 %extract448.i, label %preload1362.i, label %postload1363.i

preload1362.i:                                    ; preds = %postload1361.i
  store float 0.000000e+00, float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1363.i

postload1363.i:                                   ; preds = %preload1362.i, %postload1361.i
  br i1 %extract449.i, label %preload1364.i, label %postload1365.i

preload1364.i:                                    ; preds = %postload1363.i
  store float 0.000000e+00, float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1365.i

postload1365.i:                                   ; preds = %preload1364.i, %postload1363.i
  br i1 %extract450.i, label %preload1366.i, label %postload1367.i

preload1366.i:                                    ; preds = %postload1365.i
  store float 0.000000e+00, float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1367.i

postload1367.i:                                   ; preds = %preload1366.i, %postload1365.i
  br i1 %extract451.i, label %preload1368.i, label %postload1369.i

preload1368.i:                                    ; preds = %postload1367.i
  store float 0.000000e+00, float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1369.i

postload1369.i:                                   ; preds = %preload1368.i, %postload1367.i
  br i1 %extract452.i, label %preload1370.i, label %postload1371.i

preload1370.i:                                    ; preds = %postload1369.i
  store float 0.000000e+00, float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1371.i

postload1371.i:                                   ; preds = %preload1370.i, %postload1369.i
  br i1 %extract453.i, label %preload1372.i, label %postload1373.i

preload1372.i:                                    ; preds = %postload1371.i
  store float 0.000000e+00, float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1373.i

postload1373.i:                                   ; preds = %preload1372.i, %postload1371.i
  br i1 %extract454.i, label %preload1374.i, label %postload1375.i

preload1374.i:                                    ; preds = %postload1373.i
  store float 0.000000e+00, float addrspace(3)* %arrayidx60.i, align 4
  br label %postload1375.i

postload1375.i:                                   ; preds = %preload1374.i, %postload1373.i
  br i1 %extract455.i, label %preload1376.i, label %if.end66.i

preload1376.i:                                    ; preds = %postload1375.i
  store float 0.000000e+00, float addrspace(3)* %arrayidx60.i, align 4
  br label %if.end66.i

if.end66.i:                                       ; preds = %preload1376.i, %postload1375.i, %cond.end.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..4.i, %34
  br i1 %check.WI.iter.i, label %thenBB.i, label %SyncBB.i

thenBB.i:                                         ; preds = %if.end66.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..4.i, 1408
  %cond10.i = icmp eq i32 %currBarrier.4.i, 17
  br i1 %cond10.i, label %SyncBB2701.i, label %thenBB.i.postload1004.i_crit_edge

thenBB.i.postload1004.i_crit_edge:                ; preds = %thenBB.i
  br label %postload1004.i

SyncBB.i:                                         ; preds = %thenBB2705.i, %thenBB2728.i, %if.end66.i
  %CurrWI..5.i = phi i64 [ 0, %if.end66.i ], [ %"CurrWI++2709.i", %thenBB2705.i ], [ %"CurrWI++2732.i", %thenBB2728.i ]
  %CurrSBIndex..5.i = phi i64 [ 0, %if.end66.i ], [ %"loadedCurrSB+Stride2711.i", %thenBB2705.i ], [ %"loadedCurrSB+Stride2734.i", %thenBB2728.i ]
  %currBarrier.5.i = phi i32 [ 6, %if.end66.i ], [ %currBarrier.9.i, %thenBB2705.i ], [ %currBarrier.6.i, %thenBB2728.i ]
  %"&pSB[currWI].offset1670.i" = getelementptr inbounds i8* %37, i64 %CurrSBIndex..5.i
  %CastToValueType1671.i = bitcast i8* %"&pSB[currWI].offset1670.i" to i64*
  %loadedValue.i = load i64* %CastToValueType1671.i, align 8
  %Mneg15.i = icmp ne i64 %loadedValue.i, 0
  %temp461.i = insertelement <16 x i1> undef, i1 %Mneg15.i, i32 0
  %vector462.i = shufflevector <16 x i1> %temp461.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %negIncomingLoopMask54.i = xor i1 %Mneg15.i, true
  %temp458.i = insertelement <16 x i1> undef, i1 %negIncomingLoopMask54.i, i32 0
  %vector459.i = shufflevector <16 x i1> %temp458.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)2621.i" = add nuw i64 %CurrSBIndex..5.i, 1088
  br i1 %Mneg15.i, label %SyncBB.i.for.body73.i_crit_edge, label %for.end98.i

SyncBB.i.for.body73.i_crit_edge:                  ; preds = %SyncBB.i
  br label %for.body73.i

for.body73.i:                                     ; preds = %for.inc96.i.for.body73.i_crit_edge, %SyncBB.i.for.body73.i_crit_edge
  %CurrWI..6.i = phi i64 [ %CurrWI..5.i, %SyncBB.i.for.body73.i_crit_edge ], [ %CurrWI..8.i, %for.inc96.i.for.body73.i_crit_edge ]
  %CurrSBIndex..6.i = phi i64 [ %CurrSBIndex..5.i, %SyncBB.i.for.body73.i_crit_edge ], [ %CurrSBIndex..8.i, %for.inc96.i.for.body73.i_crit_edge ]
  %currBarrier.6.i = phi i32 [ %currBarrier.5.i, %SyncBB.i.for.body73.i_crit_edge ], [ %currBarrier.8.i, %for.inc96.i.for.body73.i_crit_edge ]
  %vectorPHI457.i = phi <16 x i1> [ %vector459.i, %SyncBB.i.for.body73.i_crit_edge ], [ %loop_mask24596.i, %for.inc96.i.for.body73.i_crit_edge ]
  %vectorPHI460.i = phi <16 x i1> [ %vector462.i, %SyncBB.i.for.body73.i_crit_edge ], [ %local_edge29600.i, %for.inc96.i.for.body73.i_crit_edge ]
  %d67.05.i = phi i32 [ 1, %SyncBB.i.for.body73.i_crit_edge ], [ %mul97.i, %for.inc96.i.for.body73.i_crit_edge ]
  %"&(pSB[currWI].offset)2685.pn.i" = phi i64 [ %"&(pSB[currWI].offset)2621.i", %SyncBB.i.for.body73.i_crit_edge ], [ %"&(pSB[currWI].offset)2695.i", %for.inc96.i.for.body73.i_crit_edge ]
  %vectorPHI463.in.in.i = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2685.pn.i"
  %vectorPHI463.in.i = bitcast i8* %vectorPHI463.in.in.i to <16 x i32>*
  %vectorPHI463.i = load <16 x i32>* %vectorPHI463.in.i, align 64
  %"&(pSB[currWI].offset)2663.i" = add nuw i64 %CurrSBIndex..6.i, 1216
  %"&pSB[currWI].offset2664.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2663.i"
  %CastToValueType2665.i = bitcast i8* %"&pSB[currWI].offset2664.i" to <16 x i32>*
  store <16 x i32> %vectorPHI463.i, <16 x i32>* %CastToValueType2665.i, align 64
  %"&(pSB[currWI].offset)2654.i" = add nuw i64 %CurrSBIndex..6.i, 1172
  %"&pSB[currWI].offset2655.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2654.i"
  %CastToValueType2656.i = bitcast i8* %"&pSB[currWI].offset2655.i" to i32*
  store i32 %d67.05.i, i32* %CastToValueType2656.i, align 4
  %"&(pSB[currWI].offset)2635.i" = add nuw i64 %CurrSBIndex..6.i, 1168
  %"&pSB[currWI].offset2636.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2635.i"
  %CastToValueType2637.i = bitcast i8* %"&pSB[currWI].offset2636.i" to <16 x i1>*
  store <16 x i1> %vectorPHI460.i, <16 x i1>* %CastToValueType2637.i, align 16
  %"&(pSB[currWI].offset)2626.i" = add nuw i64 %CurrSBIndex..6.i, 1152
  %"&pSB[currWI].offset2627.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2626.i"
  %CastToValueType2628.i = bitcast i8* %"&pSB[currWI].offset2627.i" to <16 x i1>*
  store <16 x i1> %vectorPHI457.i, <16 x i1>* %CastToValueType2628.i, align 16
  %temp481.i = insertelement <16 x i32> undef, i32 %d67.05.i, i32 0
  %vector482.i = shufflevector <16 x i32> %temp481.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)2672.i" = add nuw i64 %CurrSBIndex..6.i, 1280
  %"&pSB[currWI].offset2673.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2672.i"
  %CastToValueType2674.i = bitcast i8* %"&pSB[currWI].offset2673.i" to <16 x i32>*
  store <16 x i32> %vector482.i, <16 x i32>* %CastToValueType2674.i, align 64
  %extract465.i = extractelement <16 x i1> %vectorPHI460.i, i32 0
  %shr74464.i = lshr <16 x i32> %vectorPHI463.i, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %"&(pSB[currWI].offset)2681.i" = add nuw i64 %CurrSBIndex..6.i, 1344
  %"&pSB[currWI].offset2682.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2681.i"
  %CastToValueType2683.i = bitcast i8* %"&pSB[currWI].offset2682.i" to <16 x i32>*
  store <16 x i32> %shr74464.i, <16 x i32>* %CastToValueType2683.i, align 64
  br i1 %extract465.i, label %preload1378.i, label %postload1379.i

preload1378.i:                                    ; preds = %for.body73.i
  %check.WI.iter2731.i = icmp ult i64 %CurrWI..6.i, %34
  br i1 %check.WI.iter2731.i, label %thenBB2728.i, label %preload1378.i.postload1379.i_crit_edge

preload1378.i.postload1379.i_crit_edge:           ; preds = %preload1378.i
  br label %postload1379.i

thenBB2728.i:                                     ; preds = %preload1378.i
  %"CurrWI++2732.i" = add nuw i64 %CurrWI..6.i, 1
  %"loadedCurrSB+Stride2734.i" = add nuw i64 %CurrSBIndex..6.i, 1408
  %cond9.i = icmp eq i32 %currBarrier.6.i, 13
  br i1 %cond9.i, label %thenBB2728.i.postload1379.i_crit_edge, label %SyncBB.i

thenBB2728.i.postload1379.i_crit_edge:            ; preds = %thenBB2728.i
  br label %postload1379.i

postload1379.i:                                   ; preds = %thenBB2705.i.postload1379.i_crit_edge, %thenBB2728.i.postload1379.i_crit_edge, %preload1378.i.postload1379.i_crit_edge, %for.body73.i
  %CurrWI..8.i = phi i64 [ %CurrWI..6.i, %for.body73.i ], [ 0, %preload1378.i.postload1379.i_crit_edge ], [ %"CurrWI++2732.i", %thenBB2728.i.postload1379.i_crit_edge ], [ %"CurrWI++2709.i", %thenBB2705.i.postload1379.i_crit_edge ]
  %CurrSBIndex..8.i = phi i64 [ %CurrSBIndex..6.i, %for.body73.i ], [ 0, %preload1378.i.postload1379.i_crit_edge ], [ %"loadedCurrSB+Stride2734.i", %thenBB2728.i.postload1379.i_crit_edge ], [ %"loadedCurrSB+Stride2711.i", %thenBB2705.i.postload1379.i_crit_edge ]
  %currBarrier.8.i = phi i32 [ %currBarrier.6.i, %for.body73.i ], [ 13, %preload1378.i.postload1379.i_crit_edge ], [ %currBarrier.6.i, %thenBB2728.i.postload1379.i_crit_edge ], [ %currBarrier.9.i, %thenBB2705.i.postload1379.i_crit_edge ]
  %"&(pSB[currWI].offset)1706.i" = add nuw i64 %CurrSBIndex..8.i, 256
  %"&pSB[currWI].offset1707.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1706.i"
  %CastToValueType1708.i = bitcast i8* %"&pSB[currWI].offset1707.i" to <16 x i32>*
  %loadedValue1709.i = load <16 x i32>* %CastToValueType1708.i, align 64
  %"&(pSB[currWI].offset)2676.i" = add nuw i64 %CurrSBIndex..8.i, 1280
  %"&pSB[currWI].offset2677.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2676.i"
  %CastToValueType2678.i = bitcast i8* %"&pSB[currWI].offset2677.i" to <16 x i32>*
  %loadedValue2679.i = load <16 x i32>* %CastToValueType2678.i, align 64
  %cmp75.i = icmp slt <16 x i32> %loadedValue1709.i, %loadedValue2679.i
  %"&(pSB[currWI].offset)2649.i" = add nuw i64 %CurrSBIndex..8.i, 1168
  %"&pSB[currWI].offset2650.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2649.i"
  %CastToValueType2651.i = bitcast i8* %"&pSB[currWI].offset2650.i" to <16 x i1>*
  %loadedValue2652.i = load <16 x i1>* %CastToValueType2651.i, align 16
  %for.body73_to_if.then77485.i = and <16 x i1> %loadedValue2652.i, %cmp75.i
  %extract508.i = extractelement <16 x i1> %for.body73_to_if.then77485.i, i32 0
  %extract509.i = extractelement <16 x i1> %for.body73_to_if.then77485.i, i32 1
  %extract510.i = extractelement <16 x i1> %for.body73_to_if.then77485.i, i32 2
  %extract511.i = extractelement <16 x i1> %for.body73_to_if.then77485.i, i32 3
  %extract512.i = extractelement <16 x i1> %for.body73_to_if.then77485.i, i32 4
  %extract513.i = extractelement <16 x i1> %for.body73_to_if.then77485.i, i32 5
  %extract514.i = extractelement <16 x i1> %for.body73_to_if.then77485.i, i32 6
  %extract515.i = extractelement <16 x i1> %for.body73_to_if.then77485.i, i32 7
  %extract516.i = extractelement <16 x i1> %for.body73_to_if.then77485.i, i32 8
  %extract517.i = extractelement <16 x i1> %for.body73_to_if.then77485.i, i32 9
  %extract518.i = extractelement <16 x i1> %for.body73_to_if.then77485.i, i32 10
  %extract519.i = extractelement <16 x i1> %for.body73_to_if.then77485.i, i32 11
  %extract520.i = extractelement <16 x i1> %for.body73_to_if.then77485.i, i32 12
  %extract521.i = extractelement <16 x i1> %for.body73_to_if.then77485.i, i32 13
  %extract522.i = extractelement <16 x i1> %for.body73_to_if.then77485.i, i32 14
  %extract523.i = extractelement <16 x i1> %for.body73_to_if.then77485.i, i32 15
  %"&(pSB[currWI].offset)2667.i" = add nuw i64 %CurrSBIndex..8.i, 1216
  %"&pSB[currWI].offset2668.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2667.i"
  %CastToValueType2669.i = bitcast i8* %"&pSB[currWI].offset2668.i" to <16 x i32>*
  %loadedValue2670.i = load <16 x i32>* %CastToValueType2669.i, align 64
  %mul79486.i = and <16 x i32> %loadedValue2670.i, <i32 -2, i32 -2, i32 -2, i32 -2, i32 -2, i32 -2, i32 -2, i32 -2, i32 -2, i32 -2, i32 -2, i32 -2, i32 -2, i32 -2, i32 -2, i32 -2>
  %mul80487.i = mul <16 x i32> %mul79486.i, %loadedValue1709.i
  %"&(pSB[currWI].offset)2695.i" = add nuw i64 %CurrSBIndex..8.i, 1344
  %"&pSB[currWI].offset2696.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2695.i"
  %CastToValueType2697.i = bitcast i8* %"&pSB[currWI].offset2696.i" to <16 x i32>*
  %loadedValue2698.i = load <16 x i32>* %CastToValueType2697.i, align 64
  %add82488.i = add <16 x i32> %loadedValue2698.i, <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>
  %sub83489.i = add <16 x i32> %add82488.i, %mul80487.i
  %add85490.i = add <16 x i32> %sub83489.i, %loadedValue2698.i
  %idxprom86491.i = sext <16 x i32> %sub83489.i to <16 x i64>
  %extract492.i = extractelement <16 x i64> %idxprom86491.i, i32 0
  %extract493.i = extractelement <16 x i64> %idxprom86491.i, i32 1
  %extract494.i = extractelement <16 x i64> %idxprom86491.i, i32 2
  %extract495.i = extractelement <16 x i64> %idxprom86491.i, i32 3
  %extract496.i = extractelement <16 x i64> %idxprom86491.i, i32 4
  %extract497.i = extractelement <16 x i64> %idxprom86491.i, i32 5
  %extract498.i = extractelement <16 x i64> %idxprom86491.i, i32 6
  %extract499.i = extractelement <16 x i64> %idxprom86491.i, i32 7
  %extract500.i = extractelement <16 x i64> %idxprom86491.i, i32 8
  %extract501.i = extractelement <16 x i64> %idxprom86491.i, i32 9
  %extract502.i = extractelement <16 x i64> %idxprom86491.i, i32 10
  %extract503.i = extractelement <16 x i64> %idxprom86491.i, i32 11
  %extract504.i = extractelement <16 x i64> %idxprom86491.i, i32 12
  %extract505.i = extractelement <16 x i64> %idxprom86491.i, i32 13
  %extract506.i = extractelement <16 x i64> %idxprom86491.i, i32 14
  %extract507.i = extractelement <16 x i64> %idxprom86491.i, i32 15
  %134 = getelementptr inbounds float addrspace(3)* %22, i64 %extract492.i
  %135 = getelementptr inbounds float addrspace(3)* %22, i64 %extract493.i
  %136 = getelementptr inbounds float addrspace(3)* %22, i64 %extract494.i
  %137 = getelementptr inbounds float addrspace(3)* %22, i64 %extract495.i
  %138 = getelementptr inbounds float addrspace(3)* %22, i64 %extract496.i
  %139 = getelementptr inbounds float addrspace(3)* %22, i64 %extract497.i
  %140 = getelementptr inbounds float addrspace(3)* %22, i64 %extract498.i
  %141 = getelementptr inbounds float addrspace(3)* %22, i64 %extract499.i
  %142 = getelementptr inbounds float addrspace(3)* %22, i64 %extract500.i
  %143 = getelementptr inbounds float addrspace(3)* %22, i64 %extract501.i
  %144 = getelementptr inbounds float addrspace(3)* %22, i64 %extract502.i
  %145 = getelementptr inbounds float addrspace(3)* %22, i64 %extract503.i
  %146 = getelementptr inbounds float addrspace(3)* %22, i64 %extract504.i
  %147 = getelementptr inbounds float addrspace(3)* %22, i64 %extract505.i
  %148 = getelementptr inbounds float addrspace(3)* %22, i64 %extract506.i
  %149 = getelementptr inbounds float addrspace(3)* %22, i64 %extract507.i
  br i1 %extract508.i, label %preload1380.i, label %postload1381.i

preload1380.i:                                    ; preds = %postload1379.i
  %masked_load829.i = load float addrspace(3)* %134, align 4
  br label %postload1381.i

postload1381.i:                                   ; preds = %preload1380.i, %postload1379.i
  %phi1382.i = phi float [ undef, %postload1379.i ], [ %masked_load829.i, %preload1380.i ]
  br i1 %extract509.i, label %preload1393.i, label %postload1394.i

preload1393.i:                                    ; preds = %postload1381.i
  %masked_load830.i = load float addrspace(3)* %135, align 4
  br label %postload1394.i

postload1394.i:                                   ; preds = %preload1393.i, %postload1381.i
  %phi1395.i = phi float [ undef, %postload1381.i ], [ %masked_load830.i, %preload1393.i ]
  br i1 %extract510.i, label %preload1406.i, label %postload1407.i

preload1406.i:                                    ; preds = %postload1394.i
  %masked_load831.i = load float addrspace(3)* %136, align 4
  br label %postload1407.i

postload1407.i:                                   ; preds = %preload1406.i, %postload1394.i
  %phi1408.i = phi float [ undef, %postload1394.i ], [ %masked_load831.i, %preload1406.i ]
  br i1 %extract511.i, label %preload1419.i, label %postload1420.i

preload1419.i:                                    ; preds = %postload1407.i
  %masked_load832.i = load float addrspace(3)* %137, align 4
  br label %postload1420.i

postload1420.i:                                   ; preds = %preload1419.i, %postload1407.i
  %phi1421.i = phi float [ undef, %postload1407.i ], [ %masked_load832.i, %preload1419.i ]
  br i1 %extract512.i, label %preload1432.i, label %postload1433.i

preload1432.i:                                    ; preds = %postload1420.i
  %masked_load833.i = load float addrspace(3)* %138, align 4
  br label %postload1433.i

postload1433.i:                                   ; preds = %preload1432.i, %postload1420.i
  %phi1434.i = phi float [ undef, %postload1420.i ], [ %masked_load833.i, %preload1432.i ]
  br i1 %extract513.i, label %preload1445.i, label %postload1446.i

preload1445.i:                                    ; preds = %postload1433.i
  %masked_load834.i = load float addrspace(3)* %139, align 4
  br label %postload1446.i

postload1446.i:                                   ; preds = %preload1445.i, %postload1433.i
  %phi1447.i = phi float [ undef, %postload1433.i ], [ %masked_load834.i, %preload1445.i ]
  br i1 %extract514.i, label %preload1458.i, label %postload1459.i

preload1458.i:                                    ; preds = %postload1446.i
  %masked_load835.i = load float addrspace(3)* %140, align 4
  br label %postload1459.i

postload1459.i:                                   ; preds = %preload1458.i, %postload1446.i
  %phi1460.i = phi float [ undef, %postload1446.i ], [ %masked_load835.i, %preload1458.i ]
  br i1 %extract515.i, label %preload1471.i, label %postload1472.i

preload1471.i:                                    ; preds = %postload1459.i
  %masked_load836.i = load float addrspace(3)* %141, align 4
  br label %postload1472.i

postload1472.i:                                   ; preds = %preload1471.i, %postload1459.i
  %phi1473.i = phi float [ undef, %postload1459.i ], [ %masked_load836.i, %preload1471.i ]
  br i1 %extract516.i, label %preload1484.i, label %postload1485.i

preload1484.i:                                    ; preds = %postload1472.i
  %masked_load837.i = load float addrspace(3)* %142, align 4
  br label %postload1485.i

postload1485.i:                                   ; preds = %preload1484.i, %postload1472.i
  %phi1486.i = phi float [ undef, %postload1472.i ], [ %masked_load837.i, %preload1484.i ]
  br i1 %extract517.i, label %preload1497.i, label %postload1498.i

preload1497.i:                                    ; preds = %postload1485.i
  %masked_load838.i = load float addrspace(3)* %143, align 4
  br label %postload1498.i

postload1498.i:                                   ; preds = %preload1497.i, %postload1485.i
  %phi1499.i = phi float [ undef, %postload1485.i ], [ %masked_load838.i, %preload1497.i ]
  br i1 %extract518.i, label %preload1510.i, label %postload1511.i

preload1510.i:                                    ; preds = %postload1498.i
  %masked_load839.i = load float addrspace(3)* %144, align 4
  br label %postload1511.i

postload1511.i:                                   ; preds = %preload1510.i, %postload1498.i
  %phi1512.i = phi float [ undef, %postload1498.i ], [ %masked_load839.i, %preload1510.i ]
  br i1 %extract519.i, label %preload1523.i, label %postload1524.i

preload1523.i:                                    ; preds = %postload1511.i
  %masked_load840.i = load float addrspace(3)* %145, align 4
  br label %postload1524.i

postload1524.i:                                   ; preds = %preload1523.i, %postload1511.i
  %phi1525.i = phi float [ undef, %postload1511.i ], [ %masked_load840.i, %preload1523.i ]
  br i1 %extract520.i, label %preload1536.i, label %postload1537.i

preload1536.i:                                    ; preds = %postload1524.i
  %masked_load841.i = load float addrspace(3)* %146, align 4
  br label %postload1537.i

postload1537.i:                                   ; preds = %preload1536.i, %postload1524.i
  %phi1538.i = phi float [ undef, %postload1524.i ], [ %masked_load841.i, %preload1536.i ]
  br i1 %extract521.i, label %preload1549.i, label %postload1550.i

preload1549.i:                                    ; preds = %postload1537.i
  %masked_load842.i = load float addrspace(3)* %147, align 4
  br label %postload1550.i

postload1550.i:                                   ; preds = %preload1549.i, %postload1537.i
  %phi1551.i = phi float [ undef, %postload1537.i ], [ %masked_load842.i, %preload1549.i ]
  br i1 %extract522.i, label %preload1562.i, label %postload1563.i

preload1562.i:                                    ; preds = %postload1550.i
  %masked_load843.i = load float addrspace(3)* %148, align 4
  br label %postload1563.i

postload1563.i:                                   ; preds = %preload1562.i, %postload1550.i
  %phi1564.i = phi float [ undef, %postload1550.i ], [ %masked_load843.i, %preload1562.i ]
  br i1 %extract523.i, label %preload1575.i, label %postload1576.i

preload1575.i:                                    ; preds = %postload1563.i
  %masked_load844.i = load float addrspace(3)* %149, align 4
  br label %postload1576.i

postload1576.i:                                   ; preds = %preload1575.i, %postload1563.i
  %phi1577.i = phi float [ undef, %postload1563.i ], [ %masked_load844.i, %preload1575.i ]
  %temp.vect557.i = insertelement <16 x float> undef, float %phi1382.i, i32 0
  %temp.vect558.i = insertelement <16 x float> %temp.vect557.i, float %phi1395.i, i32 1
  %temp.vect559.i = insertelement <16 x float> %temp.vect558.i, float %phi1408.i, i32 2
  %temp.vect560.i = insertelement <16 x float> %temp.vect559.i, float %phi1421.i, i32 3
  %temp.vect561.i = insertelement <16 x float> %temp.vect560.i, float %phi1434.i, i32 4
  %temp.vect562.i = insertelement <16 x float> %temp.vect561.i, float %phi1447.i, i32 5
  %temp.vect563.i = insertelement <16 x float> %temp.vect562.i, float %phi1460.i, i32 6
  %temp.vect564.i = insertelement <16 x float> %temp.vect563.i, float %phi1473.i, i32 7
  %temp.vect565.i = insertelement <16 x float> %temp.vect564.i, float %phi1486.i, i32 8
  %temp.vect566.i = insertelement <16 x float> %temp.vect565.i, float %phi1499.i, i32 9
  %temp.vect567.i = insertelement <16 x float> %temp.vect566.i, float %phi1512.i, i32 10
  %temp.vect568.i = insertelement <16 x float> %temp.vect567.i, float %phi1525.i, i32 11
  %temp.vect569.i = insertelement <16 x float> %temp.vect568.i, float %phi1538.i, i32 12
  %temp.vect570.i = insertelement <16 x float> %temp.vect569.i, float %phi1551.i, i32 13
  %temp.vect571.i = insertelement <16 x float> %temp.vect570.i, float %phi1564.i, i32 14
  %temp.vect572.i = insertelement <16 x float> %temp.vect571.i, float %phi1577.i, i32 15
  %idxprom88524.i = sext <16 x i32> %add85490.i to <16 x i64>
  %extract525.i = extractelement <16 x i64> %idxprom88524.i, i32 0
  %extract526.i = extractelement <16 x i64> %idxprom88524.i, i32 1
  %extract527.i = extractelement <16 x i64> %idxprom88524.i, i32 2
  %extract528.i = extractelement <16 x i64> %idxprom88524.i, i32 3
  %extract529.i = extractelement <16 x i64> %idxprom88524.i, i32 4
  %extract530.i = extractelement <16 x i64> %idxprom88524.i, i32 5
  %extract531.i = extractelement <16 x i64> %idxprom88524.i, i32 6
  %extract532.i = extractelement <16 x i64> %idxprom88524.i, i32 7
  %extract533.i = extractelement <16 x i64> %idxprom88524.i, i32 8
  %extract534.i = extractelement <16 x i64> %idxprom88524.i, i32 9
  %extract535.i = extractelement <16 x i64> %idxprom88524.i, i32 10
  %extract536.i = extractelement <16 x i64> %idxprom88524.i, i32 11
  %extract537.i = extractelement <16 x i64> %idxprom88524.i, i32 12
  %extract538.i = extractelement <16 x i64> %idxprom88524.i, i32 13
  %extract539.i = extractelement <16 x i64> %idxprom88524.i, i32 14
  %extract540.i = extractelement <16 x i64> %idxprom88524.i, i32 15
  %150 = getelementptr inbounds float addrspace(3)* %22, i64 %extract525.i
  %151 = getelementptr inbounds float addrspace(3)* %22, i64 %extract526.i
  %152 = getelementptr inbounds float addrspace(3)* %22, i64 %extract527.i
  %153 = getelementptr inbounds float addrspace(3)* %22, i64 %extract528.i
  %154 = getelementptr inbounds float addrspace(3)* %22, i64 %extract529.i
  %155 = getelementptr inbounds float addrspace(3)* %22, i64 %extract530.i
  %156 = getelementptr inbounds float addrspace(3)* %22, i64 %extract531.i
  %157 = getelementptr inbounds float addrspace(3)* %22, i64 %extract532.i
  %158 = getelementptr inbounds float addrspace(3)* %22, i64 %extract533.i
  %159 = getelementptr inbounds float addrspace(3)* %22, i64 %extract534.i
  %160 = getelementptr inbounds float addrspace(3)* %22, i64 %extract535.i
  %161 = getelementptr inbounds float addrspace(3)* %22, i64 %extract536.i
  %162 = getelementptr inbounds float addrspace(3)* %22, i64 %extract537.i
  %163 = getelementptr inbounds float addrspace(3)* %22, i64 %extract538.i
  %164 = getelementptr inbounds float addrspace(3)* %22, i64 %extract539.i
  %165 = getelementptr inbounds float addrspace(3)* %22, i64 %extract540.i
  br i1 %extract508.i, label %preload1383.i, label %postload1384.i

preload1383.i:                                    ; preds = %postload1576.i
  %masked_load845.i = load float addrspace(3)* %150, align 4
  br label %postload1384.i

postload1384.i:                                   ; preds = %preload1383.i, %postload1576.i
  %phi1385.i = phi float [ undef, %postload1576.i ], [ %masked_load845.i, %preload1383.i ]
  br i1 %extract509.i, label %preload1396.i, label %postload1397.i

preload1396.i:                                    ; preds = %postload1384.i
  %masked_load846.i = load float addrspace(3)* %151, align 4
  br label %postload1397.i

postload1397.i:                                   ; preds = %preload1396.i, %postload1384.i
  %phi1398.i = phi float [ undef, %postload1384.i ], [ %masked_load846.i, %preload1396.i ]
  br i1 %extract510.i, label %preload1409.i, label %postload1410.i

preload1409.i:                                    ; preds = %postload1397.i
  %masked_load847.i = load float addrspace(3)* %152, align 4
  br label %postload1410.i

postload1410.i:                                   ; preds = %preload1409.i, %postload1397.i
  %phi1411.i = phi float [ undef, %postload1397.i ], [ %masked_load847.i, %preload1409.i ]
  br i1 %extract511.i, label %preload1422.i, label %postload1423.i

preload1422.i:                                    ; preds = %postload1410.i
  %masked_load848.i = load float addrspace(3)* %153, align 4
  br label %postload1423.i

postload1423.i:                                   ; preds = %preload1422.i, %postload1410.i
  %phi1424.i = phi float [ undef, %postload1410.i ], [ %masked_load848.i, %preload1422.i ]
  br i1 %extract512.i, label %preload1435.i, label %postload1436.i

preload1435.i:                                    ; preds = %postload1423.i
  %masked_load849.i = load float addrspace(3)* %154, align 4
  br label %postload1436.i

postload1436.i:                                   ; preds = %preload1435.i, %postload1423.i
  %phi1437.i = phi float [ undef, %postload1423.i ], [ %masked_load849.i, %preload1435.i ]
  br i1 %extract513.i, label %preload1448.i, label %postload1449.i

preload1448.i:                                    ; preds = %postload1436.i
  %masked_load850.i = load float addrspace(3)* %155, align 4
  br label %postload1449.i

postload1449.i:                                   ; preds = %preload1448.i, %postload1436.i
  %phi1450.i = phi float [ undef, %postload1436.i ], [ %masked_load850.i, %preload1448.i ]
  br i1 %extract514.i, label %preload1461.i, label %postload1462.i

preload1461.i:                                    ; preds = %postload1449.i
  %masked_load851.i = load float addrspace(3)* %156, align 4
  br label %postload1462.i

postload1462.i:                                   ; preds = %preload1461.i, %postload1449.i
  %phi1463.i = phi float [ undef, %postload1449.i ], [ %masked_load851.i, %preload1461.i ]
  br i1 %extract515.i, label %preload1474.i, label %postload1475.i

preload1474.i:                                    ; preds = %postload1462.i
  %masked_load852.i = load float addrspace(3)* %157, align 4
  br label %postload1475.i

postload1475.i:                                   ; preds = %preload1474.i, %postload1462.i
  %phi1476.i = phi float [ undef, %postload1462.i ], [ %masked_load852.i, %preload1474.i ]
  br i1 %extract516.i, label %preload1487.i, label %postload1488.i

preload1487.i:                                    ; preds = %postload1475.i
  %masked_load853.i = load float addrspace(3)* %158, align 4
  br label %postload1488.i

postload1488.i:                                   ; preds = %preload1487.i, %postload1475.i
  %phi1489.i = phi float [ undef, %postload1475.i ], [ %masked_load853.i, %preload1487.i ]
  br i1 %extract517.i, label %preload1500.i, label %postload1501.i

preload1500.i:                                    ; preds = %postload1488.i
  %masked_load854.i = load float addrspace(3)* %159, align 4
  br label %postload1501.i

postload1501.i:                                   ; preds = %preload1500.i, %postload1488.i
  %phi1502.i = phi float [ undef, %postload1488.i ], [ %masked_load854.i, %preload1500.i ]
  br i1 %extract518.i, label %preload1513.i, label %postload1514.i

preload1513.i:                                    ; preds = %postload1501.i
  %masked_load855.i = load float addrspace(3)* %160, align 4
  br label %postload1514.i

postload1514.i:                                   ; preds = %preload1513.i, %postload1501.i
  %phi1515.i = phi float [ undef, %postload1501.i ], [ %masked_load855.i, %preload1513.i ]
  br i1 %extract519.i, label %preload1526.i, label %postload1527.i

preload1526.i:                                    ; preds = %postload1514.i
  %masked_load856.i = load float addrspace(3)* %161, align 4
  br label %postload1527.i

postload1527.i:                                   ; preds = %preload1526.i, %postload1514.i
  %phi1528.i = phi float [ undef, %postload1514.i ], [ %masked_load856.i, %preload1526.i ]
  br i1 %extract520.i, label %preload1539.i, label %postload1540.i

preload1539.i:                                    ; preds = %postload1527.i
  %masked_load857.i = load float addrspace(3)* %162, align 4
  br label %postload1540.i

postload1540.i:                                   ; preds = %preload1539.i, %postload1527.i
  %phi1541.i = phi float [ undef, %postload1527.i ], [ %masked_load857.i, %preload1539.i ]
  br i1 %extract521.i, label %preload1552.i, label %postload1553.i

preload1552.i:                                    ; preds = %postload1540.i
  %masked_load858.i = load float addrspace(3)* %163, align 4
  br label %postload1553.i

postload1553.i:                                   ; preds = %preload1552.i, %postload1540.i
  %phi1554.i = phi float [ undef, %postload1540.i ], [ %masked_load858.i, %preload1552.i ]
  br i1 %extract522.i, label %preload1565.i, label %postload1566.i

preload1565.i:                                    ; preds = %postload1553.i
  %masked_load859.i = load float addrspace(3)* %164, align 4
  br label %postload1566.i

postload1566.i:                                   ; preds = %preload1565.i, %postload1553.i
  %phi1567.i = phi float [ undef, %postload1553.i ], [ %masked_load859.i, %preload1565.i ]
  br i1 %extract523.i, label %preload1578.i, label %postload1579.i

preload1578.i:                                    ; preds = %postload1566.i
  %masked_load860.i = load float addrspace(3)* %165, align 4
  br label %postload1579.i

postload1579.i:                                   ; preds = %preload1578.i, %postload1566.i
  %phi1580.i = phi float [ undef, %postload1566.i ], [ %masked_load860.i, %preload1578.i ]
  br i1 %extract508.i, label %preload1386.i, label %postload1387.i

preload1386.i:                                    ; preds = %postload1579.i
  store float %phi1385.i, float addrspace(3)* %134, align 4
  br label %postload1387.i

postload1387.i:                                   ; preds = %preload1386.i, %postload1579.i
  br i1 %extract509.i, label %preload1399.i, label %postload1400.i

preload1399.i:                                    ; preds = %postload1387.i
  store float %phi1398.i, float addrspace(3)* %135, align 4
  br label %postload1400.i

postload1400.i:                                   ; preds = %preload1399.i, %postload1387.i
  br i1 %extract510.i, label %preload1412.i, label %postload1413.i

preload1412.i:                                    ; preds = %postload1400.i
  store float %phi1411.i, float addrspace(3)* %136, align 4
  br label %postload1413.i

postload1413.i:                                   ; preds = %preload1412.i, %postload1400.i
  br i1 %extract511.i, label %preload1425.i, label %postload1426.i

preload1425.i:                                    ; preds = %postload1413.i
  store float %phi1424.i, float addrspace(3)* %137, align 4
  br label %postload1426.i

postload1426.i:                                   ; preds = %preload1425.i, %postload1413.i
  br i1 %extract512.i, label %preload1438.i, label %postload1439.i

preload1438.i:                                    ; preds = %postload1426.i
  store float %phi1437.i, float addrspace(3)* %138, align 4
  br label %postload1439.i

postload1439.i:                                   ; preds = %preload1438.i, %postload1426.i
  br i1 %extract513.i, label %preload1451.i, label %postload1452.i

preload1451.i:                                    ; preds = %postload1439.i
  store float %phi1450.i, float addrspace(3)* %139, align 4
  br label %postload1452.i

postload1452.i:                                   ; preds = %preload1451.i, %postload1439.i
  br i1 %extract514.i, label %preload1464.i, label %postload1465.i

preload1464.i:                                    ; preds = %postload1452.i
  store float %phi1463.i, float addrspace(3)* %140, align 4
  br label %postload1465.i

postload1465.i:                                   ; preds = %preload1464.i, %postload1452.i
  br i1 %extract515.i, label %preload1477.i, label %postload1478.i

preload1477.i:                                    ; preds = %postload1465.i
  store float %phi1476.i, float addrspace(3)* %141, align 4
  br label %postload1478.i

postload1478.i:                                   ; preds = %preload1477.i, %postload1465.i
  br i1 %extract516.i, label %preload1490.i, label %postload1491.i

preload1490.i:                                    ; preds = %postload1478.i
  store float %phi1489.i, float addrspace(3)* %142, align 4
  br label %postload1491.i

postload1491.i:                                   ; preds = %preload1490.i, %postload1478.i
  br i1 %extract517.i, label %preload1503.i, label %postload1504.i

preload1503.i:                                    ; preds = %postload1491.i
  store float %phi1502.i, float addrspace(3)* %143, align 4
  br label %postload1504.i

postload1504.i:                                   ; preds = %preload1503.i, %postload1491.i
  br i1 %extract518.i, label %preload1516.i, label %postload1517.i

preload1516.i:                                    ; preds = %postload1504.i
  store float %phi1515.i, float addrspace(3)* %144, align 4
  br label %postload1517.i

postload1517.i:                                   ; preds = %preload1516.i, %postload1504.i
  br i1 %extract519.i, label %preload1529.i, label %postload1530.i

preload1529.i:                                    ; preds = %postload1517.i
  store float %phi1528.i, float addrspace(3)* %145, align 4
  br label %postload1530.i

postload1530.i:                                   ; preds = %preload1529.i, %postload1517.i
  br i1 %extract520.i, label %preload1542.i, label %postload1543.i

preload1542.i:                                    ; preds = %postload1530.i
  store float %phi1541.i, float addrspace(3)* %146, align 4
  br label %postload1543.i

postload1543.i:                                   ; preds = %preload1542.i, %postload1530.i
  br i1 %extract521.i, label %preload1555.i, label %postload1556.i

preload1555.i:                                    ; preds = %postload1543.i
  store float %phi1554.i, float addrspace(3)* %147, align 4
  br label %postload1556.i

postload1556.i:                                   ; preds = %preload1555.i, %postload1543.i
  br i1 %extract522.i, label %preload1568.i, label %postload1569.i

preload1568.i:                                    ; preds = %postload1556.i
  store float %phi1567.i, float addrspace(3)* %148, align 4
  br label %postload1569.i

postload1569.i:                                   ; preds = %preload1568.i, %postload1556.i
  br i1 %extract523.i, label %preload1581.i, label %postload1582.i

preload1581.i:                                    ; preds = %postload1569.i
  store float %phi1580.i, float addrspace(3)* %149, align 4
  br label %postload1582.i

postload1582.i:                                   ; preds = %preload1581.i, %postload1569.i
  br i1 %extract508.i, label %preload1388.i, label %postload1389.i

preload1388.i:                                    ; preds = %postload1582.i
  %masked_load861.i = load float addrspace(3)* %150, align 4
  br label %postload1389.i

postload1389.i:                                   ; preds = %preload1388.i, %postload1582.i
  %phi1390.i = phi float [ undef, %postload1582.i ], [ %masked_load861.i, %preload1388.i ]
  br i1 %extract509.i, label %preload1401.i, label %postload1402.i

preload1401.i:                                    ; preds = %postload1389.i
  %masked_load862.i = load float addrspace(3)* %151, align 4
  br label %postload1402.i

postload1402.i:                                   ; preds = %preload1401.i, %postload1389.i
  %phi1403.i = phi float [ undef, %postload1389.i ], [ %masked_load862.i, %preload1401.i ]
  br i1 %extract510.i, label %preload1414.i, label %postload1415.i

preload1414.i:                                    ; preds = %postload1402.i
  %masked_load863.i = load float addrspace(3)* %152, align 4
  br label %postload1415.i

postload1415.i:                                   ; preds = %preload1414.i, %postload1402.i
  %phi1416.i = phi float [ undef, %postload1402.i ], [ %masked_load863.i, %preload1414.i ]
  br i1 %extract511.i, label %preload1427.i, label %postload1428.i

preload1427.i:                                    ; preds = %postload1415.i
  %masked_load864.i = load float addrspace(3)* %153, align 4
  br label %postload1428.i

postload1428.i:                                   ; preds = %preload1427.i, %postload1415.i
  %phi1429.i = phi float [ undef, %postload1415.i ], [ %masked_load864.i, %preload1427.i ]
  br i1 %extract512.i, label %preload1440.i, label %postload1441.i

preload1440.i:                                    ; preds = %postload1428.i
  %masked_load865.i = load float addrspace(3)* %154, align 4
  br label %postload1441.i

postload1441.i:                                   ; preds = %preload1440.i, %postload1428.i
  %phi1442.i = phi float [ undef, %postload1428.i ], [ %masked_load865.i, %preload1440.i ]
  br i1 %extract513.i, label %preload1453.i, label %postload1454.i

preload1453.i:                                    ; preds = %postload1441.i
  %masked_load866.i = load float addrspace(3)* %155, align 4
  br label %postload1454.i

postload1454.i:                                   ; preds = %preload1453.i, %postload1441.i
  %phi1455.i = phi float [ undef, %postload1441.i ], [ %masked_load866.i, %preload1453.i ]
  br i1 %extract514.i, label %preload1466.i, label %postload1467.i

preload1466.i:                                    ; preds = %postload1454.i
  %masked_load867.i = load float addrspace(3)* %156, align 4
  br label %postload1467.i

postload1467.i:                                   ; preds = %preload1466.i, %postload1454.i
  %phi1468.i = phi float [ undef, %postload1454.i ], [ %masked_load867.i, %preload1466.i ]
  br i1 %extract515.i, label %preload1479.i, label %postload1480.i

preload1479.i:                                    ; preds = %postload1467.i
  %masked_load868.i = load float addrspace(3)* %157, align 4
  br label %postload1480.i

postload1480.i:                                   ; preds = %preload1479.i, %postload1467.i
  %phi1481.i = phi float [ undef, %postload1467.i ], [ %masked_load868.i, %preload1479.i ]
  br i1 %extract516.i, label %preload1492.i, label %postload1493.i

preload1492.i:                                    ; preds = %postload1480.i
  %masked_load869.i = load float addrspace(3)* %158, align 4
  br label %postload1493.i

postload1493.i:                                   ; preds = %preload1492.i, %postload1480.i
  %phi1494.i = phi float [ undef, %postload1480.i ], [ %masked_load869.i, %preload1492.i ]
  br i1 %extract517.i, label %preload1505.i, label %postload1506.i

preload1505.i:                                    ; preds = %postload1493.i
  %masked_load870.i = load float addrspace(3)* %159, align 4
  br label %postload1506.i

postload1506.i:                                   ; preds = %preload1505.i, %postload1493.i
  %phi1507.i = phi float [ undef, %postload1493.i ], [ %masked_load870.i, %preload1505.i ]
  br i1 %extract518.i, label %preload1518.i, label %postload1519.i

preload1518.i:                                    ; preds = %postload1506.i
  %masked_load871.i = load float addrspace(3)* %160, align 4
  br label %postload1519.i

postload1519.i:                                   ; preds = %preload1518.i, %postload1506.i
  %phi1520.i = phi float [ undef, %postload1506.i ], [ %masked_load871.i, %preload1518.i ]
  br i1 %extract519.i, label %preload1531.i, label %postload1532.i

preload1531.i:                                    ; preds = %postload1519.i
  %masked_load872.i = load float addrspace(3)* %161, align 4
  br label %postload1532.i

postload1532.i:                                   ; preds = %preload1531.i, %postload1519.i
  %phi1533.i = phi float [ undef, %postload1519.i ], [ %masked_load872.i, %preload1531.i ]
  br i1 %extract520.i, label %preload1544.i, label %postload1545.i

preload1544.i:                                    ; preds = %postload1532.i
  %masked_load873.i = load float addrspace(3)* %162, align 4
  br label %postload1545.i

postload1545.i:                                   ; preds = %preload1544.i, %postload1532.i
  %phi1546.i = phi float [ undef, %postload1532.i ], [ %masked_load873.i, %preload1544.i ]
  br i1 %extract521.i, label %preload1557.i, label %postload1558.i

preload1557.i:                                    ; preds = %postload1545.i
  %masked_load874.i = load float addrspace(3)* %163, align 4
  br label %postload1558.i

postload1558.i:                                   ; preds = %preload1557.i, %postload1545.i
  %phi1559.i = phi float [ undef, %postload1545.i ], [ %masked_load874.i, %preload1557.i ]
  br i1 %extract522.i, label %preload1570.i, label %postload1571.i

preload1570.i:                                    ; preds = %postload1558.i
  %masked_load875.i = load float addrspace(3)* %164, align 4
  br label %postload1571.i

postload1571.i:                                   ; preds = %preload1570.i, %postload1558.i
  %phi1572.i = phi float [ undef, %postload1558.i ], [ %masked_load875.i, %preload1570.i ]
  br i1 %extract523.i, label %preload1583.i, label %postload1584.i

preload1583.i:                                    ; preds = %postload1571.i
  %masked_load876.i = load float addrspace(3)* %165, align 4
  br label %postload1584.i

postload1584.i:                                   ; preds = %preload1583.i, %postload1571.i
  %phi1585.i = phi float [ undef, %postload1571.i ], [ %masked_load876.i, %preload1583.i ]
  %temp.vect541.i = insertelement <16 x float> undef, float %phi1390.i, i32 0
  %temp.vect542.i = insertelement <16 x float> %temp.vect541.i, float %phi1403.i, i32 1
  %temp.vect543.i = insertelement <16 x float> %temp.vect542.i, float %phi1416.i, i32 2
  %temp.vect544.i = insertelement <16 x float> %temp.vect543.i, float %phi1429.i, i32 3
  %temp.vect545.i = insertelement <16 x float> %temp.vect544.i, float %phi1442.i, i32 4
  %temp.vect546.i = insertelement <16 x float> %temp.vect545.i, float %phi1455.i, i32 5
  %temp.vect547.i = insertelement <16 x float> %temp.vect546.i, float %phi1468.i, i32 6
  %temp.vect548.i = insertelement <16 x float> %temp.vect547.i, float %phi1481.i, i32 7
  %temp.vect549.i = insertelement <16 x float> %temp.vect548.i, float %phi1494.i, i32 8
  %temp.vect550.i = insertelement <16 x float> %temp.vect549.i, float %phi1507.i, i32 9
  %temp.vect551.i = insertelement <16 x float> %temp.vect550.i, float %phi1520.i, i32 10
  %temp.vect552.i = insertelement <16 x float> %temp.vect551.i, float %phi1533.i, i32 11
  %temp.vect553.i = insertelement <16 x float> %temp.vect552.i, float %phi1546.i, i32 12
  %temp.vect554.i = insertelement <16 x float> %temp.vect553.i, float %phi1559.i, i32 13
  %temp.vect555.i = insertelement <16 x float> %temp.vect554.i, float %phi1572.i, i32 14
  %temp.vect556.i = insertelement <16 x float> %temp.vect555.i, float %phi1585.i, i32 15
  %add94573.i = fadd <16 x float> %temp.vect556.i, %temp.vect572.i
  %extract575.i = extractelement <16 x float> %add94573.i, i32 1
  %extract576.i = extractelement <16 x float> %add94573.i, i32 2
  %extract577.i = extractelement <16 x float> %add94573.i, i32 3
  %extract578.i = extractelement <16 x float> %add94573.i, i32 4
  %extract579.i = extractelement <16 x float> %add94573.i, i32 5
  %extract580.i = extractelement <16 x float> %add94573.i, i32 6
  %extract581.i = extractelement <16 x float> %add94573.i, i32 7
  %extract582.i = extractelement <16 x float> %add94573.i, i32 8
  %extract583.i = extractelement <16 x float> %add94573.i, i32 9
  %extract584.i = extractelement <16 x float> %add94573.i, i32 10
  %extract585.i = extractelement <16 x float> %add94573.i, i32 11
  %extract586.i = extractelement <16 x float> %add94573.i, i32 12
  %extract587.i = extractelement <16 x float> %add94573.i, i32 13
  %extract588.i = extractelement <16 x float> %add94573.i, i32 14
  %extract589.i = extractelement <16 x float> %add94573.i, i32 15
  br i1 %extract508.i, label %preload1391.i, label %postload1392.i

preload1391.i:                                    ; preds = %postload1584.i
  %extract574.i = extractelement <16 x float> %add94573.i, i32 0
  store float %extract574.i, float addrspace(3)* %150, align 4
  br label %postload1392.i

postload1392.i:                                   ; preds = %preload1391.i, %postload1584.i
  br i1 %extract509.i, label %preload1404.i, label %postload1405.i

preload1404.i:                                    ; preds = %postload1392.i
  store float %extract575.i, float addrspace(3)* %151, align 4
  br label %postload1405.i

postload1405.i:                                   ; preds = %preload1404.i, %postload1392.i
  br i1 %extract510.i, label %preload1417.i, label %postload1418.i

preload1417.i:                                    ; preds = %postload1405.i
  store float %extract576.i, float addrspace(3)* %152, align 4
  br label %postload1418.i

postload1418.i:                                   ; preds = %preload1417.i, %postload1405.i
  br i1 %extract511.i, label %preload1430.i, label %postload1431.i

preload1430.i:                                    ; preds = %postload1418.i
  store float %extract577.i, float addrspace(3)* %153, align 4
  br label %postload1431.i

postload1431.i:                                   ; preds = %preload1430.i, %postload1418.i
  br i1 %extract512.i, label %preload1443.i, label %postload1444.i

preload1443.i:                                    ; preds = %postload1431.i
  store float %extract578.i, float addrspace(3)* %154, align 4
  br label %postload1444.i

postload1444.i:                                   ; preds = %preload1443.i, %postload1431.i
  br i1 %extract513.i, label %preload1456.i, label %postload1457.i

preload1456.i:                                    ; preds = %postload1444.i
  store float %extract579.i, float addrspace(3)* %155, align 4
  br label %postload1457.i

postload1457.i:                                   ; preds = %preload1456.i, %postload1444.i
  br i1 %extract514.i, label %preload1469.i, label %postload1470.i

preload1469.i:                                    ; preds = %postload1457.i
  store float %extract580.i, float addrspace(3)* %156, align 4
  br label %postload1470.i

postload1470.i:                                   ; preds = %preload1469.i, %postload1457.i
  br i1 %extract515.i, label %preload1482.i, label %postload1483.i

preload1482.i:                                    ; preds = %postload1470.i
  store float %extract581.i, float addrspace(3)* %157, align 4
  br label %postload1483.i

postload1483.i:                                   ; preds = %preload1482.i, %postload1470.i
  br i1 %extract516.i, label %preload1495.i, label %postload1496.i

preload1495.i:                                    ; preds = %postload1483.i
  store float %extract582.i, float addrspace(3)* %158, align 4
  br label %postload1496.i

postload1496.i:                                   ; preds = %preload1495.i, %postload1483.i
  br i1 %extract517.i, label %preload1508.i, label %postload1509.i

preload1508.i:                                    ; preds = %postload1496.i
  store float %extract583.i, float addrspace(3)* %159, align 4
  br label %postload1509.i

postload1509.i:                                   ; preds = %preload1508.i, %postload1496.i
  br i1 %extract518.i, label %preload1521.i, label %postload1522.i

preload1521.i:                                    ; preds = %postload1509.i
  store float %extract584.i, float addrspace(3)* %160, align 4
  br label %postload1522.i

postload1522.i:                                   ; preds = %preload1521.i, %postload1509.i
  br i1 %extract519.i, label %preload1534.i, label %postload1535.i

preload1534.i:                                    ; preds = %postload1522.i
  store float %extract585.i, float addrspace(3)* %161, align 4
  br label %postload1535.i

postload1535.i:                                   ; preds = %preload1534.i, %postload1522.i
  br i1 %extract520.i, label %preload1547.i, label %postload1548.i

preload1547.i:                                    ; preds = %postload1535.i
  store float %extract586.i, float addrspace(3)* %162, align 4
  br label %postload1548.i

postload1548.i:                                   ; preds = %preload1547.i, %postload1535.i
  br i1 %extract521.i, label %preload1560.i, label %postload1561.i

preload1560.i:                                    ; preds = %postload1548.i
  store float %extract587.i, float addrspace(3)* %163, align 4
  br label %postload1561.i

postload1561.i:                                   ; preds = %preload1560.i, %postload1548.i
  br i1 %extract522.i, label %preload1573.i, label %postload1574.i

preload1573.i:                                    ; preds = %postload1561.i
  store float %extract588.i, float addrspace(3)* %164, align 4
  br label %postload1574.i

postload1574.i:                                   ; preds = %preload1573.i, %postload1561.i
  br i1 %extract523.i, label %preload1586.i, label %postload1574.i.for.inc96.i_crit_edge

postload1574.i.for.inc96.i_crit_edge:             ; preds = %postload1574.i
  br label %for.inc96.i

preload1586.i:                                    ; preds = %postload1574.i
  store float %extract589.i, float addrspace(3)* %165, align 4
  br label %for.inc96.i

for.inc96.i:                                      ; preds = %postload1574.i.for.inc96.i_crit_edge, %preload1586.i
  %"&(pSB[currWI].offset)2658.i" = add nuw i64 %CurrSBIndex..8.i, 1172
  %"&pSB[currWI].offset2659.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2658.i"
  %CastToValueType2660.i = bitcast i8* %"&pSB[currWI].offset2659.i" to i32*
  %loadedValue2661.i = load i32* %CastToValueType2660.i, align 4
  %mul97.i = shl nsw i32 %loadedValue2661.i, 1
  %conv69.i = sext i32 %mul97.i to i64
  %"&pSB[currWI].offset1684.i" = getelementptr inbounds i8* %37, i64 %CurrSBIndex..8.i
  %CastToValueType1685.i = bitcast i8* %"&pSB[currWI].offset1684.i" to i64*
  %loadedValue1686.i = load i64* %CastToValueType1685.i, align 8
  %cmp71.i = icmp ugt i64 %conv69.i, %loadedValue1686.i
  %temp592.i = insertelement <16 x i1> undef, i1 %cmp71.i, i32 0
  %vector593.i = shufflevector <16 x i1> %temp592.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %notCond20.i = xor i1 %cmp71.i, true
  %temp598.i = insertelement <16 x i1> undef, i1 %notCond20.i, i32 0
  %vector599.i = shufflevector <16 x i1> %temp598.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %loadedValue2642.i = load <16 x i1>* %CastToValueType2651.i, align 16
  %who_left_tr21594.i = and <16 x i1> %loadedValue2642.i, %vector593.i
  %"&(pSB[currWI].offset)2630.i" = add nuw i64 %CurrSBIndex..8.i, 1152
  %"&pSB[currWI].offset2631.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2630.i"
  %CastToValueType2632.i = bitcast i8* %"&pSB[currWI].offset2631.i" to <16 x i1>*
  %loadedValue2633.i = load <16 x i1>* %CastToValueType2632.i, align 16
  %loop_mask24596.i = or <16 x i1> %loadedValue2633.i, %who_left_tr21594.i
  %ipred.i.i = bitcast <16 x i1> %loop_mask24596.i to i16
  %val.i.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i.i, i16 %ipred.i.i) nounwind
  %166 = and i32 %val.i.i, 1
  %res.i.i = icmp eq i32 %166, 0
  %local_edge29600.i = and <16 x i1> %loadedValue2642.i, %vector599.i
  br i1 %res.i.i, label %for.inc96.i.for.body73.i_crit_edge, label %for.end98.i

for.inc96.i.for.body73.i_crit_edge:               ; preds = %for.inc96.i
  br label %for.body73.i

for.end98.i:                                      ; preds = %for.inc96.i, %SyncBB.i
  %CurrWI..9.i = phi i64 [ %CurrWI..5.i, %SyncBB.i ], [ %CurrWI..8.i, %for.inc96.i ]
  %CurrSBIndex..9.i = phi i64 [ %CurrSBIndex..5.i, %SyncBB.i ], [ %CurrSBIndex..8.i, %for.inc96.i ]
  %currBarrier.9.i = phi i32 [ %currBarrier.5.i, %SyncBB.i ], [ %currBarrier.8.i, %for.inc96.i ]
  %check.WI.iter2708.i = icmp ult i64 %CurrWI..9.i, %34
  br i1 %check.WI.iter2708.i, label %thenBB2705.i, label %SyncBB2699.i

thenBB2705.i:                                     ; preds = %for.end98.i
  %"CurrWI++2709.i" = add nuw i64 %CurrWI..9.i, 1
  %"loadedCurrSB+Stride2711.i" = add nuw i64 %CurrSBIndex..9.i, 1408
  %cond.i = icmp eq i32 %currBarrier.9.i, 13
  br i1 %cond.i, label %thenBB2705.i.postload1379.i_crit_edge, label %SyncBB.i

thenBB2705.i.postload1379.i_crit_edge:            ; preds = %thenBB2705.i
  br label %postload1379.i

SyncBB2699.i:                                     ; preds = %thenBB2721.i, %for.end98.i
  %CurrWI..10.i = phi i64 [ %"CurrWI++2725.i", %thenBB2721.i ], [ 0, %for.end98.i ]
  %CurrSBIndex..10.i = phi i64 [ %"loadedCurrSB+Stride2727.i", %thenBB2721.i ], [ 0, %for.end98.i ]
  %"&(pSB[currWI].offset)1734.i" = add nuw i64 %CurrSBIndex..10.i, 328
  %"&pSB[currWI].offset1735.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1734.i"
  %CastToValueType1736.i = bitcast i8* %"&pSB[currWI].offset1735.i" to <16 x float> addrspace(3)**
  %loadedValue1737.i = load <16 x float> addrspace(3)** %CastToValueType1736.i, align 8
  %167 = load <16 x float> addrspace(3)* %loadedValue1737.i, align 4
  %"&(pSB[currWI].offset)1725.i" = add nuw i64 %CurrSBIndex..10.i, 320
  %"&pSB[currWI].offset1726.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1725.i"
  %CastToValueType1727.i = bitcast i8* %"&pSB[currWI].offset1726.i" to i64*
  %loadedValue1728.i = load i64* %CastToValueType1727.i, align 8
  %168 = getelementptr inbounds float addrspace(1)* %1, i64 %loadedValue1728.i
  %ptrTypeCast602.i = bitcast float addrspace(1)* %168 to <16 x float> addrspace(1)*
  store <16 x float> %167, <16 x float> addrspace(1)* %ptrTypeCast602.i, align 4
  %"&(pSB[currWI].offset)2103.i" = add nuw i64 %CurrSBIndex..10.i, 351
  %"&pSB[currWI].offset2104.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2103.i"
  %CastToValueType2105.i = bitcast i8* %"&pSB[currWI].offset2104.i" to i1*
  %loadedValue2106.i = load i1* %CastToValueType2105.i, align 1
  br i1 %loadedValue2106.i, label %preload1239.i, label %postload1240.i

preload1239.i:                                    ; preds = %SyncBB2699.i
  %"&(pSB[currWI].offset)2271.i" = add nuw i64 %CurrSBIndex..10.i, 632
  %"&pSB[currWI].offset2272.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2271.i"
  %CastToValueType2273.i = bitcast i8* %"&pSB[currWI].offset2272.i" to i64*
  %loadedValue2274.i = load i64* %CastToValueType2273.i, align 8
  %169 = getelementptr inbounds float addrspace(3)* %22, i64 %loadedValue2274.i
  %vload879.i = load float addrspace(3)* %169, align 4
  br label %postload1240.i

postload1240.i:                                   ; preds = %preload1239.i, %SyncBB2699.i
  %phi1241.i = phi float [ undef, %SyncBB2699.i ], [ %vload879.i, %preload1239.i ]
  %"&(pSB[currWI].offset)1743.i" = add nuw i64 %CurrSBIndex..10.i, 336
  %"&pSB[currWI].offset1744.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1743.i"
  %CastToValueType1745.i = bitcast i8* %"&pSB[currWI].offset1744.i" to i1*
  %loadedValue1746.i = load i1* %CastToValueType1745.i, align 1
  br i1 %loadedValue1746.i, label %preload1005.i, label %postload1006.i

preload1005.i:                                    ; preds = %postload1240.i
  %"&(pSB[currWI].offset)2346.i" = add nuw i64 %CurrSBIndex..10.i, 632
  %"&pSB[currWI].offset2347.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2346.i"
  %CastToValueType2348.i = bitcast i8* %"&pSB[currWI].offset2347.i" to i64*
  %loadedValue2349.i = load i64* %CastToValueType2348.i, align 8
  %.sum1637.i = add i64 %loadedValue2349.i, 1
  %170 = getelementptr float addrspace(3)* %22, i64 %.sum1637.i
  %vload882.i = load float addrspace(3)* %170, align 4
  br label %postload1006.i

postload1006.i:                                   ; preds = %preload1005.i, %postload1240.i
  %phi1007.i = phi float [ undef, %postload1240.i ], [ %vload882.i, %preload1005.i ]
  %"&(pSB[currWI].offset)1767.i" = add nuw i64 %CurrSBIndex..10.i, 337
  %"&pSB[currWI].offset1768.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1767.i"
  %CastToValueType1769.i = bitcast i8* %"&pSB[currWI].offset1768.i" to i1*
  %loadedValue1770.i = load i1* %CastToValueType1769.i, align 1
  br i1 %loadedValue1770.i, label %preload1008.i, label %postload1009.i

preload1008.i:                                    ; preds = %postload1006.i
  %"&(pSB[currWI].offset)2341.i" = add nuw i64 %CurrSBIndex..10.i, 632
  %"&pSB[currWI].offset2342.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2341.i"
  %CastToValueType2343.i = bitcast i8* %"&pSB[currWI].offset2342.i" to i64*
  %loadedValue2344.i = load i64* %CastToValueType2343.i, align 8
  %.sum1636.i = add i64 %loadedValue2344.i, 2
  %171 = getelementptr float addrspace(3)* %22, i64 %.sum1636.i
  %vload886.i = load float addrspace(3)* %171, align 4
  br label %postload1009.i

postload1009.i:                                   ; preds = %preload1008.i, %postload1006.i
  %phi1010.i = phi float [ undef, %postload1006.i ], [ %vload886.i, %preload1008.i ]
  %"&(pSB[currWI].offset)1791.i" = add nuw i64 %CurrSBIndex..10.i, 338
  %"&pSB[currWI].offset1792.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1791.i"
  %CastToValueType1793.i = bitcast i8* %"&pSB[currWI].offset1792.i" to i1*
  %loadedValue1794.i = load i1* %CastToValueType1793.i, align 1
  br i1 %loadedValue1794.i, label %preload1146.i, label %postload1147.i

preload1146.i:                                    ; preds = %postload1009.i
  %"&(pSB[currWI].offset)2336.i" = add nuw i64 %CurrSBIndex..10.i, 632
  %"&pSB[currWI].offset2337.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2336.i"
  %CastToValueType2338.i = bitcast i8* %"&pSB[currWI].offset2337.i" to i64*
  %loadedValue2339.i = load i64* %CastToValueType2338.i, align 8
  %.sum1635.i = add i64 %loadedValue2339.i, 3
  %172 = getelementptr float addrspace(3)* %22, i64 %.sum1635.i
  %vload890.i = load float addrspace(3)* %172, align 4
  br label %postload1147.i

postload1147.i:                                   ; preds = %preload1146.i, %postload1009.i
  %phi1148.i = phi float [ undef, %postload1009.i ], [ %vload890.i, %preload1146.i ]
  %"&(pSB[currWI].offset)1815.i" = add nuw i64 %CurrSBIndex..10.i, 339
  %"&pSB[currWI].offset1816.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1815.i"
  %CastToValueType1817.i = bitcast i8* %"&pSB[currWI].offset1816.i" to i1*
  %loadedValue1818.i = load i1* %CastToValueType1817.i, align 1
  br i1 %loadedValue1818.i, label %preload1149.i, label %postload1150.i

preload1149.i:                                    ; preds = %postload1147.i
  %"&(pSB[currWI].offset)2331.i" = add nuw i64 %CurrSBIndex..10.i, 632
  %"&pSB[currWI].offset2332.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2331.i"
  %CastToValueType2333.i = bitcast i8* %"&pSB[currWI].offset2332.i" to i64*
  %loadedValue2334.i = load i64* %CastToValueType2333.i, align 8
  %.sum1634.i = add i64 %loadedValue2334.i, 4
  %173 = getelementptr float addrspace(3)* %22, i64 %.sum1634.i
  %vload894.i = load float addrspace(3)* %173, align 4
  br label %postload1150.i

postload1150.i:                                   ; preds = %preload1149.i, %postload1147.i
  %phi1151.i = phi float [ undef, %postload1147.i ], [ %vload894.i, %preload1149.i ]
  %"&(pSB[currWI].offset)1839.i" = add nuw i64 %CurrSBIndex..10.i, 340
  %"&pSB[currWI].offset1840.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1839.i"
  %CastToValueType1841.i = bitcast i8* %"&pSB[currWI].offset1840.i" to i1*
  %loadedValue1842.i = load i1* %CastToValueType1841.i, align 1
  br i1 %loadedValue1842.i, label %preload1194.i, label %postload1195.i

preload1194.i:                                    ; preds = %postload1150.i
  %"&(pSB[currWI].offset)2326.i" = add nuw i64 %CurrSBIndex..10.i, 632
  %"&pSB[currWI].offset2327.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2326.i"
  %CastToValueType2328.i = bitcast i8* %"&pSB[currWI].offset2327.i" to i64*
  %loadedValue2329.i = load i64* %CastToValueType2328.i, align 8
  %.sum1633.i = add i64 %loadedValue2329.i, 5
  %174 = getelementptr float addrspace(3)* %22, i64 %.sum1633.i
  %vload898.i = load float addrspace(3)* %174, align 4
  br label %postload1195.i

postload1195.i:                                   ; preds = %preload1194.i, %postload1150.i
  %phi1196.i = phi float [ undef, %postload1150.i ], [ %vload898.i, %preload1194.i ]
  %"&(pSB[currWI].offset)1863.i" = add nuw i64 %CurrSBIndex..10.i, 341
  %"&pSB[currWI].offset1864.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1863.i"
  %CastToValueType1865.i = bitcast i8* %"&pSB[currWI].offset1864.i" to i1*
  %loadedValue1866.i = load i1* %CastToValueType1865.i, align 1
  br i1 %loadedValue1866.i, label %preload1197.i, label %postload1198.i

preload1197.i:                                    ; preds = %postload1195.i
  %"&(pSB[currWI].offset)2321.i" = add nuw i64 %CurrSBIndex..10.i, 632
  %"&pSB[currWI].offset2322.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2321.i"
  %CastToValueType2323.i = bitcast i8* %"&pSB[currWI].offset2322.i" to i64*
  %loadedValue2324.i = load i64* %CastToValueType2323.i, align 8
  %.sum1632.i = add i64 %loadedValue2324.i, 6
  %175 = getelementptr float addrspace(3)* %22, i64 %.sum1632.i
  %vload902.i = load float addrspace(3)* %175, align 4
  br label %postload1198.i

postload1198.i:                                   ; preds = %preload1197.i, %postload1195.i
  %phi1199.i = phi float [ undef, %postload1195.i ], [ %vload902.i, %preload1197.i ]
  %"&(pSB[currWI].offset)1887.i" = add nuw i64 %CurrSBIndex..10.i, 342
  %"&pSB[currWI].offset1888.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1887.i"
  %CastToValueType1889.i = bitcast i8* %"&pSB[currWI].offset1888.i" to i1*
  %loadedValue1890.i = load i1* %CastToValueType1889.i, align 1
  br i1 %loadedValue1890.i, label %preload1233.i, label %postload1234.i

preload1233.i:                                    ; preds = %postload1198.i
  %"&(pSB[currWI].offset)2316.i" = add nuw i64 %CurrSBIndex..10.i, 632
  %"&pSB[currWI].offset2317.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2316.i"
  %CastToValueType2318.i = bitcast i8* %"&pSB[currWI].offset2317.i" to i64*
  %loadedValue2319.i = load i64* %CastToValueType2318.i, align 8
  %.sum1631.i = add i64 %loadedValue2319.i, 7
  %176 = getelementptr float addrspace(3)* %22, i64 %.sum1631.i
  %vload906.i = load float addrspace(3)* %176, align 4
  br label %postload1234.i

postload1234.i:                                   ; preds = %preload1233.i, %postload1198.i
  %phi1235.i = phi float [ undef, %postload1198.i ], [ %vload906.i, %preload1233.i ]
  %"&(pSB[currWI].offset)1911.i" = add nuw i64 %CurrSBIndex..10.i, 343
  %"&pSB[currWI].offset1912.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1911.i"
  %CastToValueType1913.i = bitcast i8* %"&pSB[currWI].offset1912.i" to i1*
  %loadedValue1914.i = load i1* %CastToValueType1913.i, align 1
  br i1 %loadedValue1914.i, label %preload1236.i, label %postload1237.i

preload1236.i:                                    ; preds = %postload1234.i
  %"&(pSB[currWI].offset)2311.i" = add nuw i64 %CurrSBIndex..10.i, 632
  %"&pSB[currWI].offset2312.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2311.i"
  %CastToValueType2313.i = bitcast i8* %"&pSB[currWI].offset2312.i" to i64*
  %loadedValue2314.i = load i64* %CastToValueType2313.i, align 8
  %.sum1630.i = add i64 %loadedValue2314.i, 8
  %177 = getelementptr float addrspace(3)* %22, i64 %.sum1630.i
  %vload910.i = load float addrspace(3)* %177, align 4
  br label %postload1237.i

postload1237.i:                                   ; preds = %preload1236.i, %postload1234.i
  %phi1238.i = phi float [ undef, %postload1234.i ], [ %vload910.i, %preload1236.i ]
  %"&(pSB[currWI].offset)1935.i" = add nuw i64 %CurrSBIndex..10.i, 344
  %"&pSB[currWI].offset1936.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1935.i"
  %CastToValueType1937.i = bitcast i8* %"&pSB[currWI].offset1936.i" to i1*
  %loadedValue1938.i = load i1* %CastToValueType1937.i, align 1
  br i1 %loadedValue1938.i, label %preload1218.i, label %postload1219.i

preload1218.i:                                    ; preds = %postload1237.i
  %"&(pSB[currWI].offset)2306.i" = add nuw i64 %CurrSBIndex..10.i, 632
  %"&pSB[currWI].offset2307.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2306.i"
  %CastToValueType2308.i = bitcast i8* %"&pSB[currWI].offset2307.i" to i64*
  %loadedValue2309.i = load i64* %CastToValueType2308.i, align 8
  %.sum1629.i = add i64 %loadedValue2309.i, 9
  %178 = getelementptr float addrspace(3)* %22, i64 %.sum1629.i
  %vload914.i = load float addrspace(3)* %178, align 4
  br label %postload1219.i

postload1219.i:                                   ; preds = %preload1218.i, %postload1237.i
  %phi1220.i = phi float [ undef, %postload1237.i ], [ %vload914.i, %preload1218.i ]
  %"&(pSB[currWI].offset)1959.i" = add nuw i64 %CurrSBIndex..10.i, 345
  %"&pSB[currWI].offset1960.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1959.i"
  %CastToValueType1961.i = bitcast i8* %"&pSB[currWI].offset1960.i" to i1*
  %loadedValue1962.i = load i1* %CastToValueType1961.i, align 1
  br i1 %loadedValue1962.i, label %preload1221.i, label %postload1222.i

preload1221.i:                                    ; preds = %postload1219.i
  %"&(pSB[currWI].offset)2301.i" = add nuw i64 %CurrSBIndex..10.i, 632
  %"&pSB[currWI].offset2302.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2301.i"
  %CastToValueType2303.i = bitcast i8* %"&pSB[currWI].offset2302.i" to i64*
  %loadedValue2304.i = load i64* %CastToValueType2303.i, align 8
  %.sum1628.i = add i64 %loadedValue2304.i, 10
  %179 = getelementptr float addrspace(3)* %22, i64 %.sum1628.i
  %vload918.i = load float addrspace(3)* %179, align 4
  br label %postload1222.i

postload1222.i:                                   ; preds = %preload1221.i, %postload1219.i
  %phi1223.i = phi float [ undef, %postload1219.i ], [ %vload918.i, %preload1221.i ]
  %"&(pSB[currWI].offset)1983.i" = add nuw i64 %CurrSBIndex..10.i, 346
  %"&pSB[currWI].offset1984.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1983.i"
  %CastToValueType1985.i = bitcast i8* %"&pSB[currWI].offset1984.i" to i1*
  %loadedValue1986.i = load i1* %CastToValueType1985.i, align 1
  br i1 %loadedValue1986.i, label %preload1227.i, label %postload1228.i

preload1227.i:                                    ; preds = %postload1222.i
  %"&(pSB[currWI].offset)2296.i" = add nuw i64 %CurrSBIndex..10.i, 632
  %"&pSB[currWI].offset2297.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2296.i"
  %CastToValueType2298.i = bitcast i8* %"&pSB[currWI].offset2297.i" to i64*
  %loadedValue2299.i = load i64* %CastToValueType2298.i, align 8
  %.sum1627.i = add i64 %loadedValue2299.i, 11
  %180 = getelementptr float addrspace(3)* %22, i64 %.sum1627.i
  %vload922.i = load float addrspace(3)* %180, align 4
  br label %postload1228.i

postload1228.i:                                   ; preds = %preload1227.i, %postload1222.i
  %phi1229.i = phi float [ undef, %postload1222.i ], [ %vload922.i, %preload1227.i ]
  %"&(pSB[currWI].offset)2007.i" = add nuw i64 %CurrSBIndex..10.i, 347
  %"&pSB[currWI].offset2008.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2007.i"
  %CastToValueType2009.i = bitcast i8* %"&pSB[currWI].offset2008.i" to i1*
  %loadedValue2010.i = load i1* %CastToValueType2009.i, align 1
  br i1 %loadedValue2010.i, label %preload1230.i, label %postload1231.i

preload1230.i:                                    ; preds = %postload1228.i
  %"&(pSB[currWI].offset)2291.i" = add nuw i64 %CurrSBIndex..10.i, 632
  %"&pSB[currWI].offset2292.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2291.i"
  %CastToValueType2293.i = bitcast i8* %"&pSB[currWI].offset2292.i" to i64*
  %loadedValue2294.i = load i64* %CastToValueType2293.i, align 8
  %.sum1626.i = add i64 %loadedValue2294.i, 12
  %181 = getelementptr float addrspace(3)* %22, i64 %.sum1626.i
  %vload926.i = load float addrspace(3)* %181, align 4
  br label %postload1231.i

postload1231.i:                                   ; preds = %preload1230.i, %postload1228.i
  %phi1232.i = phi float [ undef, %postload1228.i ], [ %vload926.i, %preload1230.i ]
  %"&(pSB[currWI].offset)2031.i" = add nuw i64 %CurrSBIndex..10.i, 348
  %"&pSB[currWI].offset2032.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2031.i"
  %CastToValueType2033.i = bitcast i8* %"&pSB[currWI].offset2032.i" to i1*
  %loadedValue2034.i = load i1* %CastToValueType2033.i, align 1
  br i1 %loadedValue2034.i, label %preload976.i, label %postload977.i

preload976.i:                                     ; preds = %postload1231.i
  %"&(pSB[currWI].offset)2286.i" = add nuw i64 %CurrSBIndex..10.i, 632
  %"&pSB[currWI].offset2287.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2286.i"
  %CastToValueType2288.i = bitcast i8* %"&pSB[currWI].offset2287.i" to i64*
  %loadedValue2289.i = load i64* %CastToValueType2288.i, align 8
  %.sum1625.i = add i64 %loadedValue2289.i, 13
  %182 = getelementptr float addrspace(3)* %22, i64 %.sum1625.i
  %vload930.i = load float addrspace(3)* %182, align 4
  br label %postload977.i

postload977.i:                                    ; preds = %preload976.i, %postload1231.i
  %phi978.i = phi float [ undef, %postload1231.i ], [ %vload930.i, %preload976.i ]
  %"&(pSB[currWI].offset)2055.i" = add nuw i64 %CurrSBIndex..10.i, 349
  %"&pSB[currWI].offset2056.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2055.i"
  %CastToValueType2057.i = bitcast i8* %"&pSB[currWI].offset2056.i" to i1*
  %loadedValue2058.i = load i1* %CastToValueType2057.i, align 1
  br i1 %loadedValue2058.i, label %preload979.i, label %postload980.i

preload979.i:                                     ; preds = %postload977.i
  %"&(pSB[currWI].offset)2281.i" = add nuw i64 %CurrSBIndex..10.i, 632
  %"&pSB[currWI].offset2282.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2281.i"
  %CastToValueType2283.i = bitcast i8* %"&pSB[currWI].offset2282.i" to i64*
  %loadedValue2284.i = load i64* %CastToValueType2283.i, align 8
  %.sum1624.i = add i64 %loadedValue2284.i, 14
  %183 = getelementptr float addrspace(3)* %22, i64 %.sum1624.i
  %vload934.i = load float addrspace(3)* %183, align 4
  br label %postload980.i

postload980.i:                                    ; preds = %preload979.i, %postload977.i
  %phi981.i = phi float [ undef, %postload977.i ], [ %vload934.i, %preload979.i ]
  %"&(pSB[currWI].offset)2079.i" = add nuw i64 %CurrSBIndex..10.i, 350
  %"&pSB[currWI].offset2080.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2079.i"
  %CastToValueType2081.i = bitcast i8* %"&pSB[currWI].offset2080.i" to i1*
  %loadedValue2082.i = load i1* %CastToValueType2081.i, align 1
  br i1 %loadedValue2082.i, label %preload1588.i, label %postload1589.i

preload1588.i:                                    ; preds = %postload980.i
  %"&(pSB[currWI].offset)2276.i" = add nuw i64 %CurrSBIndex..10.i, 632
  %"&pSB[currWI].offset2277.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2276.i"
  %CastToValueType2278.i = bitcast i8* %"&pSB[currWI].offset2277.i" to i64*
  %loadedValue2279.i = load i64* %CastToValueType2278.i, align 8
  %.sum.i = add i64 %loadedValue2279.i, 15
  %184 = getelementptr float addrspace(3)* %22, i64 %.sum.i
  %vload938.i = load float addrspace(3)* %184, align 4
  br label %postload1589.i

postload1589.i:                                   ; preds = %preload1588.i, %postload980.i
  %phi1590.i = phi float [ undef, %postload980.i ], [ %vload938.i, %preload1588.i ]
  %"&(pSB[currWI].offset)2136.i" = add nuw i64 %CurrSBIndex..10.i, 512
  %"&pSB[currWI].offset2137.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2136.i"
  %CastToValueType2138.i = bitcast i8* %"&pSB[currWI].offset2137.i" to i64*
  %loadedValue2139.i = load i64* %CastToValueType2138.i, align 8
  %185 = getelementptr inbounds float addrspace(1)* %1, i64 %loadedValue2139.i
  %"&(pSB[currWI].offset)2145.i" = add nuw i64 %CurrSBIndex..10.i, 520
  %"&pSB[currWI].offset2146.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2145.i"
  %CastToValueType2147.i = bitcast i8* %"&pSB[currWI].offset2146.i" to i64*
  %loadedValue2148.i = load i64* %CastToValueType2147.i, align 8
  %186 = getelementptr inbounds float addrspace(1)* %1, i64 %loadedValue2148.i
  %"&(pSB[currWI].offset)2154.i" = add nuw i64 %CurrSBIndex..10.i, 528
  %"&pSB[currWI].offset2155.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2154.i"
  %CastToValueType2156.i = bitcast i8* %"&pSB[currWI].offset2155.i" to i64*
  %loadedValue2157.i = load i64* %CastToValueType2156.i, align 8
  %187 = getelementptr inbounds float addrspace(1)* %1, i64 %loadedValue2157.i
  %"&(pSB[currWI].offset)2163.i" = add nuw i64 %CurrSBIndex..10.i, 536
  %"&pSB[currWI].offset2164.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2163.i"
  %CastToValueType2165.i = bitcast i8* %"&pSB[currWI].offset2164.i" to i64*
  %loadedValue2166.i = load i64* %CastToValueType2165.i, align 8
  %188 = getelementptr inbounds float addrspace(1)* %1, i64 %loadedValue2166.i
  %"&(pSB[currWI].offset)2172.i" = add nuw i64 %CurrSBIndex..10.i, 544
  %"&pSB[currWI].offset2173.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2172.i"
  %CastToValueType2174.i = bitcast i8* %"&pSB[currWI].offset2173.i" to i64*
  %loadedValue2175.i = load i64* %CastToValueType2174.i, align 8
  %189 = getelementptr inbounds float addrspace(1)* %1, i64 %loadedValue2175.i
  %"&(pSB[currWI].offset)2181.i" = add nuw i64 %CurrSBIndex..10.i, 552
  %"&pSB[currWI].offset2182.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2181.i"
  %CastToValueType2183.i = bitcast i8* %"&pSB[currWI].offset2182.i" to i64*
  %loadedValue2184.i = load i64* %CastToValueType2183.i, align 8
  %190 = getelementptr inbounds float addrspace(1)* %1, i64 %loadedValue2184.i
  %"&(pSB[currWI].offset)2190.i" = add nuw i64 %CurrSBIndex..10.i, 560
  %"&pSB[currWI].offset2191.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2190.i"
  %CastToValueType2192.i = bitcast i8* %"&pSB[currWI].offset2191.i" to i64*
  %loadedValue2193.i = load i64* %CastToValueType2192.i, align 8
  %191 = getelementptr inbounds float addrspace(1)* %1, i64 %loadedValue2193.i
  %"&(pSB[currWI].offset)2199.i" = add nuw i64 %CurrSBIndex..10.i, 568
  %"&pSB[currWI].offset2200.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2199.i"
  %CastToValueType2201.i = bitcast i8* %"&pSB[currWI].offset2200.i" to i64*
  %loadedValue2202.i = load i64* %CastToValueType2201.i, align 8
  %192 = getelementptr inbounds float addrspace(1)* %1, i64 %loadedValue2202.i
  %"&(pSB[currWI].offset)2208.i" = add nuw i64 %CurrSBIndex..10.i, 576
  %"&pSB[currWI].offset2209.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2208.i"
  %CastToValueType2210.i = bitcast i8* %"&pSB[currWI].offset2209.i" to i64*
  %loadedValue2211.i = load i64* %CastToValueType2210.i, align 8
  %193 = getelementptr inbounds float addrspace(1)* %1, i64 %loadedValue2211.i
  %"&(pSB[currWI].offset)2217.i" = add nuw i64 %CurrSBIndex..10.i, 584
  %"&pSB[currWI].offset2218.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2217.i"
  %CastToValueType2219.i = bitcast i8* %"&pSB[currWI].offset2218.i" to i64*
  %loadedValue2220.i = load i64* %CastToValueType2219.i, align 8
  %194 = getelementptr inbounds float addrspace(1)* %1, i64 %loadedValue2220.i
  %"&(pSB[currWI].offset)2226.i" = add nuw i64 %CurrSBIndex..10.i, 592
  %"&pSB[currWI].offset2227.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2226.i"
  %CastToValueType2228.i = bitcast i8* %"&pSB[currWI].offset2227.i" to i64*
  %loadedValue2229.i = load i64* %CastToValueType2228.i, align 8
  %195 = getelementptr inbounds float addrspace(1)* %1, i64 %loadedValue2229.i
  %"&(pSB[currWI].offset)2235.i" = add nuw i64 %CurrSBIndex..10.i, 600
  %"&pSB[currWI].offset2236.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2235.i"
  %CastToValueType2237.i = bitcast i8* %"&pSB[currWI].offset2236.i" to i64*
  %loadedValue2238.i = load i64* %CastToValueType2237.i, align 8
  %196 = getelementptr inbounds float addrspace(1)* %1, i64 %loadedValue2238.i
  %"&(pSB[currWI].offset)2244.i" = add nuw i64 %CurrSBIndex..10.i, 608
  %"&pSB[currWI].offset2245.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2244.i"
  %CastToValueType2246.i = bitcast i8* %"&pSB[currWI].offset2245.i" to i64*
  %loadedValue2247.i = load i64* %CastToValueType2246.i, align 8
  %197 = getelementptr inbounds float addrspace(1)* %1, i64 %loadedValue2247.i
  %"&(pSB[currWI].offset)2253.i" = add nuw i64 %CurrSBIndex..10.i, 616
  %"&pSB[currWI].offset2254.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2253.i"
  %CastToValueType2255.i = bitcast i8* %"&pSB[currWI].offset2254.i" to i64*
  %loadedValue2256.i = load i64* %CastToValueType2255.i, align 8
  %198 = getelementptr inbounds float addrspace(1)* %1, i64 %loadedValue2256.i
  %"&(pSB[currWI].offset)2262.i" = add nuw i64 %CurrSBIndex..10.i, 624
  %"&pSB[currWI].offset2263.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2262.i"
  %CastToValueType2264.i = bitcast i8* %"&pSB[currWI].offset2263.i" to i64*
  %loadedValue2265.i = load i64* %CastToValueType2264.i, align 8
  %199 = getelementptr inbounds float addrspace(1)* %1, i64 %loadedValue2265.i
  br i1 %loadedValue2106.i, label %preload1591.i, label %postload1592.i

preload1591.i:                                    ; preds = %postload1589.i
  %"&(pSB[currWI].offset)2122.i" = add nuw i64 %CurrSBIndex..10.i, 384
  %"&pSB[currWI].offset2123.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2122.i"
  %CastToValueType2124.i = bitcast i8* %"&pSB[currWI].offset2123.i" to <16 x i64>*
  %loadedValue2125.i = load <16 x i64>* %CastToValueType2124.i, align 128
  %extract623.i = extractelement <16 x i64> %loadedValue2125.i, i32 0
  %200 = getelementptr inbounds float addrspace(1)* %1, i64 %extract623.i
  store float %phi1241.i, float addrspace(1)* %200, align 4
  %loadedValue1751.pre.i = load i1* %CastToValueType1745.i, align 1
  br label %postload1592.i

postload1592.i:                                   ; preds = %preload1591.i, %postload1589.i
  %loadedValue1751.i = phi i1 [ %loadedValue1751.pre.i, %preload1591.i ], [ %loadedValue1746.i, %postload1589.i ]
  br i1 %loadedValue1751.i, label %preload1593.i, label %postload1592.i.postload1594.i_crit_edge

postload1592.i.postload1594.i_crit_edge:          ; preds = %postload1592.i
  br label %postload1594.i

preload1593.i:                                    ; preds = %postload1592.i
  store float %phi1007.i, float addrspace(1)* %185, align 4
  br label %postload1594.i

postload1594.i:                                   ; preds = %postload1592.i.postload1594.i_crit_edge, %preload1593.i
  %loadedValue1775.i = load i1* %CastToValueType1769.i, align 1
  br i1 %loadedValue1775.i, label %preload1595.i, label %postload1594.i.postload1596.i_crit_edge

postload1594.i.postload1596.i_crit_edge:          ; preds = %postload1594.i
  br label %postload1596.i

preload1595.i:                                    ; preds = %postload1594.i
  store float %phi1010.i, float addrspace(1)* %186, align 4
  br label %postload1596.i

postload1596.i:                                   ; preds = %postload1594.i.postload1596.i_crit_edge, %preload1595.i
  %loadedValue1799.i = load i1* %CastToValueType1793.i, align 1
  br i1 %loadedValue1799.i, label %preload1597.i, label %postload1596.i.postload1598.i_crit_edge

postload1596.i.postload1598.i_crit_edge:          ; preds = %postload1596.i
  br label %postload1598.i

preload1597.i:                                    ; preds = %postload1596.i
  store float %phi1148.i, float addrspace(1)* %187, align 4
  br label %postload1598.i

postload1598.i:                                   ; preds = %postload1596.i.postload1598.i_crit_edge, %preload1597.i
  %loadedValue1823.i = load i1* %CastToValueType1817.i, align 1
  br i1 %loadedValue1823.i, label %preload1599.i, label %postload1598.i.postload1600.i_crit_edge

postload1598.i.postload1600.i_crit_edge:          ; preds = %postload1598.i
  br label %postload1600.i

preload1599.i:                                    ; preds = %postload1598.i
  store float %phi1151.i, float addrspace(1)* %188, align 4
  br label %postload1600.i

postload1600.i:                                   ; preds = %postload1598.i.postload1600.i_crit_edge, %preload1599.i
  %loadedValue1847.i = load i1* %CastToValueType1841.i, align 1
  br i1 %loadedValue1847.i, label %preload1601.i, label %postload1600.i.postload1602.i_crit_edge

postload1600.i.postload1602.i_crit_edge:          ; preds = %postload1600.i
  br label %postload1602.i

preload1601.i:                                    ; preds = %postload1600.i
  store float %phi1196.i, float addrspace(1)* %189, align 4
  br label %postload1602.i

postload1602.i:                                   ; preds = %postload1600.i.postload1602.i_crit_edge, %preload1601.i
  %loadedValue1871.i = load i1* %CastToValueType1865.i, align 1
  br i1 %loadedValue1871.i, label %preload1603.i, label %postload1602.i.postload1604.i_crit_edge

postload1602.i.postload1604.i_crit_edge:          ; preds = %postload1602.i
  br label %postload1604.i

preload1603.i:                                    ; preds = %postload1602.i
  store float %phi1199.i, float addrspace(1)* %190, align 4
  br label %postload1604.i

postload1604.i:                                   ; preds = %postload1602.i.postload1604.i_crit_edge, %preload1603.i
  %loadedValue1895.i = load i1* %CastToValueType1889.i, align 1
  br i1 %loadedValue1895.i, label %preload1605.i, label %postload1604.i.postload1606.i_crit_edge

postload1604.i.postload1606.i_crit_edge:          ; preds = %postload1604.i
  br label %postload1606.i

preload1605.i:                                    ; preds = %postload1604.i
  store float %phi1235.i, float addrspace(1)* %191, align 4
  br label %postload1606.i

postload1606.i:                                   ; preds = %postload1604.i.postload1606.i_crit_edge, %preload1605.i
  %loadedValue1919.i = load i1* %CastToValueType1913.i, align 1
  br i1 %loadedValue1919.i, label %preload1607.i, label %postload1606.i.postload1608.i_crit_edge

postload1606.i.postload1608.i_crit_edge:          ; preds = %postload1606.i
  br label %postload1608.i

preload1607.i:                                    ; preds = %postload1606.i
  store float %phi1238.i, float addrspace(1)* %192, align 4
  br label %postload1608.i

postload1608.i:                                   ; preds = %postload1606.i.postload1608.i_crit_edge, %preload1607.i
  %loadedValue1943.i = load i1* %CastToValueType1937.i, align 1
  br i1 %loadedValue1943.i, label %preload1609.i, label %postload1608.i.postload1610.i_crit_edge

postload1608.i.postload1610.i_crit_edge:          ; preds = %postload1608.i
  br label %postload1610.i

preload1609.i:                                    ; preds = %postload1608.i
  store float %phi1220.i, float addrspace(1)* %193, align 4
  br label %postload1610.i

postload1610.i:                                   ; preds = %postload1608.i.postload1610.i_crit_edge, %preload1609.i
  %loadedValue1967.i = load i1* %CastToValueType1961.i, align 1
  br i1 %loadedValue1967.i, label %preload1611.i, label %postload1610.i.postload1612.i_crit_edge

postload1610.i.postload1612.i_crit_edge:          ; preds = %postload1610.i
  br label %postload1612.i

preload1611.i:                                    ; preds = %postload1610.i
  store float %phi1223.i, float addrspace(1)* %194, align 4
  br label %postload1612.i

postload1612.i:                                   ; preds = %postload1610.i.postload1612.i_crit_edge, %preload1611.i
  %loadedValue1991.i = load i1* %CastToValueType1985.i, align 1
  br i1 %loadedValue1991.i, label %preload1613.i, label %postload1612.i.postload1614.i_crit_edge

postload1612.i.postload1614.i_crit_edge:          ; preds = %postload1612.i
  br label %postload1614.i

preload1613.i:                                    ; preds = %postload1612.i
  store float %phi1229.i, float addrspace(1)* %195, align 4
  br label %postload1614.i

postload1614.i:                                   ; preds = %postload1612.i.postload1614.i_crit_edge, %preload1613.i
  %loadedValue2015.i = load i1* %CastToValueType2009.i, align 1
  br i1 %loadedValue2015.i, label %preload1615.i, label %postload1614.i.postload1616.i_crit_edge

postload1614.i.postload1616.i_crit_edge:          ; preds = %postload1614.i
  br label %postload1616.i

preload1615.i:                                    ; preds = %postload1614.i
  store float %phi1232.i, float addrspace(1)* %196, align 4
  br label %postload1616.i

postload1616.i:                                   ; preds = %postload1614.i.postload1616.i_crit_edge, %preload1615.i
  %loadedValue2039.i = load i1* %CastToValueType2033.i, align 1
  br i1 %loadedValue2039.i, label %preload1617.i, label %postload1616.i.postload1618.i_crit_edge

postload1616.i.postload1618.i_crit_edge:          ; preds = %postload1616.i
  br label %postload1618.i

preload1617.i:                                    ; preds = %postload1616.i
  store float %phi978.i, float addrspace(1)* %197, align 4
  br label %postload1618.i

postload1618.i:                                   ; preds = %postload1616.i.postload1618.i_crit_edge, %preload1617.i
  %loadedValue2063.i = load i1* %CastToValueType2057.i, align 1
  br i1 %loadedValue2063.i, label %preload1619.i, label %postload1618.i.postload1620.i_crit_edge

postload1618.i.postload1620.i_crit_edge:          ; preds = %postload1618.i
  br label %postload1620.i

preload1619.i:                                    ; preds = %postload1618.i
  store float %phi981.i, float addrspace(1)* %198, align 4
  br label %postload1620.i

postload1620.i:                                   ; preds = %postload1618.i.postload1620.i_crit_edge, %preload1619.i
  %loadedValue2087.i = load i1* %CastToValueType2081.i, align 1
  br i1 %loadedValue2087.i, label %preload1621.i, label %if.end110.i

preload1621.i:                                    ; preds = %postload1620.i
  store float %phi1590.i, float addrspace(1)* %199, align 4
  br label %if.end110.i

if.end110.i:                                      ; preds = %preload1621.i, %postload1620.i
  %check.WI.iter2724.i = icmp ult i64 %CurrWI..10.i, %34
  br i1 %check.WI.iter2724.i, label %thenBB2721.i, label %____Vectorized_.scan_separated_args.exit

thenBB2721.i:                                     ; preds = %if.end110.i
  %"CurrWI++2725.i" = add nuw i64 %CurrWI..10.i, 1
  %"loadedCurrSB+Stride2727.i" = add nuw i64 %CurrSBIndex..10.i, 1408
  br label %SyncBB2699.i

____Vectorized_.scan_separated_args.exit:         ; preds = %if.end110.i
  ret void
}

!opencl.kernels = !{!0, !2}
!opencl.build.options = !{!4}
!cl.noBarrierPath.kernels = !{!4}
!opencl.wrappers = !{!5, !6}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32, i32, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__uniformAdd_separated_args, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3, i32 3}
!2 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32, i32, i32, i32, float addrspace(3)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__scan_separated_args, metadata !3}
!3 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3}
!4 = metadata !{}
!5 = metadata !{void (i8*)* @uniformAdd}
!6 = metadata !{void (i8*)* @scan}
