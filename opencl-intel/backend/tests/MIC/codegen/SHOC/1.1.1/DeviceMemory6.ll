; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@writeLocalMemory.lbuf = internal addrspace(3) unnamed_addr global [4096 x float] zeroinitializer, align 16

declare void @__writeLocalMemory_original(float addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32) nounwind readnone

declare i64 @get_local_id(i32) nounwind readnone

declare i64 @get_local_size(i32) nounwind readnone

declare void @barrier(i64)

declare void @____Vectorized_.writeLocalMemory_original(float addrspace(1)* nocapture, i32) nounwind

declare i1 @allOne(i1)

declare i1 @allZero(i1)

declare void @dummybarrier.()

declare i8* @get_special_buffer.() nounwind readnone

declare i64 @get_iter_count.() nounwind readnone

declare i64 @get_new_local_id.(i32, i64) nounwind readnone

declare i64 @get_new_global_id.(i32, i64) nounwind readnone

declare void @__writeLocalMemory_separated_args(float addrspace(1)* nocapture, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare void @____Vectorized_.writeLocalMemory_separated_args(float addrspace(1)* nocapture, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

define void @writeLocalMemory(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 16
  %3 = bitcast i8* %2 to i8 addrspace(3)**
  %4 = load i8 addrspace(3)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 24
  %6 = bitcast i8* %5 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %7 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 40
  %9 = bitcast i8* %8 to <{ [4 x i64] }>**
  %10 = load <{ [4 x i64] }>** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 48
  %12 = bitcast i8* %11 to <{ [4 x i64] }>**
  %13 = load <{ [4 x i64] }>** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 64
  %15 = bitcast i8* %14 to i64*
  %16 = load i64* %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 72
  %18 = bitcast i8* %17 to i8**
  %19 = load i8** %18, align 8
  %20 = bitcast i8 addrspace(3)* %4 to [4096 x float] addrspace(3)*
  br label %SyncBB39.i

SyncBB39.i:                                       ; preds = %thenBB.i, %entry
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %21 = getelementptr <{ [4 x i64] }>* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %22 = load i64* %21, align 8
  %23 = getelementptr <{ [4 x i64] }>* %10, i64 0, i32 0, i64 0
  %24 = load i64* %23, align 8
  %25 = add i64 %22, %24
  %conv.i = trunc i64 %25 to i32
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %conv.i, i32* %CastToValueType.i, align 4
  %26 = load i64* %21, align 8
  %conv6.i = trunc i64 %26 to i32
  %"&(pSB[currWI].offset)271.i" = or i64 %CurrSBIndex..0.i, 4
  %"&pSB[currWI].offset28.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)271.i"
  %CastToValueType29.i = bitcast i8* %"&pSB[currWI].offset28.i" to i32*
  store i32 %conv6.i, i32* %CastToValueType29.i, align 4
  %27 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %7, i64 0, i32 3, i64 0
  %28 = load i64* %27, align 8
  %conv11.i = sitofp i32 %conv.i to float
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %SyncBB39.i
  %s.020.i = phi i32 [ %conv6.i, %SyncBB39.i ], [ %and89.i, %for.body.i ]
  %j.019.i = phi i32 [ 0, %SyncBB39.i ], [ %inc.i, %for.body.i ]
  %and.i = and i32 %s.020.i, 4095
  %idxprom1.i = zext i32 %and.i to i64
  %arrayidx.i = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %idxprom1.i
  store float %conv11.i, float addrspace(3)* %arrayidx.i, align 4
  %add14.i = add nsw i32 %s.020.i, 1
  %and15.i = and i32 %add14.i, 4095
  %idxprom162.i = zext i32 %and15.i to i64
  %arrayidx17.i = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %idxprom162.i
  store float %conv11.i, float addrspace(3)* %arrayidx17.i, align 4
  %add19.i = add nsw i32 %s.020.i, 2
  %and20.i = and i32 %add19.i, 4095
  %idxprom213.i = zext i32 %and20.i to i64
  %arrayidx22.i = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %idxprom213.i
  store float %conv11.i, float addrspace(3)* %arrayidx22.i, align 4
  %add24.i = add nsw i32 %s.020.i, 3
  %and25.i = and i32 %add24.i, 4095
  %idxprom264.i = zext i32 %and25.i to i64
  %arrayidx27.i = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %idxprom264.i
  store float %conv11.i, float addrspace(3)* %arrayidx27.i, align 4
  %add29.i = add nsw i32 %s.020.i, 4
  %and30.i = and i32 %add29.i, 4095
  %idxprom315.i = zext i32 %and30.i to i64
  %arrayidx32.i = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %idxprom315.i
  store float %conv11.i, float addrspace(3)* %arrayidx32.i, align 4
  %add34.i = add nsw i32 %s.020.i, 5
  %and35.i = and i32 %add34.i, 4095
  %idxprom366.i = zext i32 %and35.i to i64
  %arrayidx37.i = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %idxprom366.i
  store float %conv11.i, float addrspace(3)* %arrayidx37.i, align 4
  %add39.i = add nsw i32 %s.020.i, 6
  %and40.i = and i32 %add39.i, 4095
  %idxprom417.i = zext i32 %and40.i to i64
  %arrayidx42.i = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %idxprom417.i
  store float %conv11.i, float addrspace(3)* %arrayidx42.i, align 4
  %add44.i = add nsw i32 %s.020.i, 7
  %and45.i = and i32 %add44.i, 4095
  %idxprom468.i = zext i32 %and45.i to i64
  %arrayidx47.i = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %idxprom468.i
  store float %conv11.i, float addrspace(3)* %arrayidx47.i, align 4
  %add49.i = add nsw i32 %s.020.i, 8
  %and50.i = and i32 %add49.i, 4095
  %idxprom519.i = zext i32 %and50.i to i64
  %arrayidx52.i = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %idxprom519.i
  store float %conv11.i, float addrspace(3)* %arrayidx52.i, align 4
  %add54.i = add nsw i32 %s.020.i, 9
  %and55.i = and i32 %add54.i, 4095
  %idxprom5610.i = zext i32 %and55.i to i64
  %arrayidx57.i = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %idxprom5610.i
  store float %conv11.i, float addrspace(3)* %arrayidx57.i, align 4
  %add59.i = add nsw i32 %s.020.i, 10
  %and60.i = and i32 %add59.i, 4095
  %idxprom6111.i = zext i32 %and60.i to i64
  %arrayidx62.i = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %idxprom6111.i
  store float %conv11.i, float addrspace(3)* %arrayidx62.i, align 4
  %add64.i = add nsw i32 %s.020.i, 11
  %and65.i = and i32 %add64.i, 4095
  %idxprom6612.i = zext i32 %and65.i to i64
  %arrayidx67.i = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %idxprom6612.i
  store float %conv11.i, float addrspace(3)* %arrayidx67.i, align 4
  %add69.i = add nsw i32 %s.020.i, 12
  %and70.i = and i32 %add69.i, 4095
  %idxprom7113.i = zext i32 %and70.i to i64
  %arrayidx72.i = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %idxprom7113.i
  store float %conv11.i, float addrspace(3)* %arrayidx72.i, align 4
  %add74.i = add nsw i32 %s.020.i, 13
  %and75.i = and i32 %add74.i, 4095
  %idxprom7614.i = zext i32 %and75.i to i64
  %arrayidx77.i = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %idxprom7614.i
  store float %conv11.i, float addrspace(3)* %arrayidx77.i, align 4
  %add79.i = add nsw i32 %s.020.i, 14
  %and80.i = and i32 %add79.i, 4095
  %idxprom8115.i = zext i32 %and80.i to i64
  %arrayidx82.i = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %idxprom8115.i
  store float %conv11.i, float addrspace(3)* %arrayidx82.i, align 4
  %add84.i = add nsw i32 %s.020.i, 15
  %and85.i = and i32 %add84.i, 4095
  %idxprom8616.i = zext i32 %and85.i to i64
  %arrayidx87.i = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %idxprom8616.i
  store float %conv11.i, float addrspace(3)* %arrayidx87.i, align 4
  %add88.i = add nsw i32 %s.020.i, 16
  %and89.i = and i32 %add88.i, 4095
  %inc.i = add nsw i32 %j.019.i, 1
  %exitcond21.i = icmp eq i32 %inc.i, 3000
  br i1 %exitcond21.i, label %"Barrier BB.i", label %for.body.i

"Barrier BB.i":                                   ; preds = %for.body.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %"Barrier BB.i"
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 8
  br label %SyncBB39.i

elseBB.i:                                         ; preds = %"Barrier BB.i"
  %conv8.i = trunc i64 %28 to i32
  %isDivisorZero.i = icmp eq i32 %conv8.i, 0
  %newiDvisor.i = select i1 %isDivisorZero.i, i32 1, i32 %conv8.i
  %div.i = sdiv i32 4096, %newiDvisor.i
  %cmp9117.i = icmp sgt i32 %div.i, 0
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB41.i, %elseBB.i
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride47.i", %thenBB41.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++45.i", %thenBB41.i ]
  br i1 %cmp9117.i, label %for.body93.lr.ph.i, label %for.end100.i

for.body93.lr.ph.i:                               ; preds = %SyncBB.i
  %"&(pSB[currWI].offset)312.i" = or i64 %CurrSBIndex..1.i, 4
  %"&pSB[currWI].offset32.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)312.i"
  %CastToValueType33.i = bitcast i8* %"&pSB[currWI].offset32.i" to i32*
  %loadedValue34.i = load i32* %CastToValueType33.i, align 4
  %idxprom94.i = sext i32 %loadedValue34.i to i64
  %arrayidx95.i = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %idxprom94.i
  %"&pSB[currWI].offset24.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..1.i
  %CastToValueType25.i = bitcast i8* %"&pSB[currWI].offset24.i" to i32*
  %loadedValue.i = load i32* %CastToValueType25.i, align 4
  %idxprom96.i = sext i32 %loadedValue.i to i64
  %arrayidx97.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom96.i
  br label %for.body93.i

for.body93.i:                                     ; preds = %for.body93.i, %for.body93.lr.ph.i
  %j.118.i = phi i32 [ 0, %for.body93.lr.ph.i ], [ %inc99.i, %for.body93.i ]
  %29 = load float addrspace(3)* %arrayidx95.i, align 4
  store float %29, float addrspace(1)* %arrayidx97.i, align 4
  %inc99.i = add nsw i32 %j.118.i, 1
  %exitcond.i = icmp eq i32 %inc99.i, %div.i
  br i1 %exitcond.i, label %for.end100.i, label %for.body93.i

for.end100.i:                                     ; preds = %for.body93.i, %SyncBB.i
  %check.WI.iter44.i = icmp ult i64 %CurrWI..1.i, %16
  br i1 %check.WI.iter44.i, label %thenBB41.i, label %__writeLocalMemory_separated_args.exit

thenBB41.i:                                       ; preds = %for.end100.i
  %"CurrWI++45.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride47.i" = add nuw i64 %CurrSBIndex..1.i, 8
  br label %SyncBB.i

__writeLocalMemory_separated_args.exit:           ; preds = %for.end100.i
  ret void
}

define void @__Vectorized_.writeLocalMemory(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 16
  %3 = bitcast i8* %2 to i8 addrspace(3)**
  %4 = load i8 addrspace(3)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 24
  %6 = bitcast i8* %5 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %7 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 40
  %9 = bitcast i8* %8 to <{ [4 x i64] }>**
  %10 = load <{ [4 x i64] }>** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 48
  %12 = bitcast i8* %11 to <{ [4 x i64] }>**
  %13 = load <{ [4 x i64] }>** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 64
  %15 = bitcast i8* %14 to i64*
  %16 = load i64* %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 72
  %18 = bitcast i8* %17 to i8**
  %19 = load i8** %18, align 8
  %20 = bitcast i8 addrspace(3)* %4 to [4096 x float] addrspace(3)*
  br label %SyncBB377.i

SyncBB377.i:                                      ; preds = %thenBB380.i, %entry
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride386.i", %thenBB380.i ]
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++384.i", %thenBB380.i ]
  %21 = getelementptr <{ [4 x i64] }>* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %22 = load i64* %21, align 8
  %23 = getelementptr <{ [4 x i64] }>* %10, i64 0, i32 0, i64 0
  %24 = load i64* %23, align 8
  %25 = add i64 %22, %24
  %broadcast1.i = insertelement <16 x i64> undef, i64 %25, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %26 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %conv1.i = trunc <16 x i64> %26 to <16 x i32>
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to <16 x i32>*
  store <16 x i32> %conv1.i, <16 x i32>* %CastToValueType.i, align 64
  %27 = load i64* %21, align 8
  %broadcast12.i = insertelement <16 x i64> undef, i64 %27, i32 0
  %broadcast23.i = shufflevector <16 x i64> %broadcast12.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %28 = add <16 x i64> %broadcast23.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %conv64.i = trunc <16 x i64> %28 to <16 x i32>
  %"&(pSB[currWI].offset)3661.i" = or i64 %CurrSBIndex..0.i, 64
  %"&pSB[currWI].offset367.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)3661.i"
  %CastToValueType368.i = bitcast i8* %"&pSB[currWI].offset367.i" to <16 x i32>*
  store <16 x i32> %conv64.i, <16 x i32>* %CastToValueType368.i, align 64
  %29 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %7, i64 0, i32 3, i64 0
  %30 = load i64* %29, align 8
  %conv115.i = sitofp <16 x i32> %conv1.i to <16 x float>
  %extract23.i = extractelement <16 x float> %conv115.i, i32 0
  %extract24.i = extractelement <16 x float> %conv115.i, i32 1
  %extract25.i = extractelement <16 x float> %conv115.i, i32 2
  %extract26.i = extractelement <16 x float> %conv115.i, i32 3
  %extract27.i = extractelement <16 x float> %conv115.i, i32 4
  %extract28.i = extractelement <16 x float> %conv115.i, i32 5
  %extract29.i = extractelement <16 x float> %conv115.i, i32 6
  %extract30.i = extractelement <16 x float> %conv115.i, i32 7
  %extract31.i = extractelement <16 x float> %conv115.i, i32 8
  %extract32.i = extractelement <16 x float> %conv115.i, i32 9
  %extract33.i = extractelement <16 x float> %conv115.i, i32 10
  %extract34.i = extractelement <16 x float> %conv115.i, i32 11
  %extract35.i = extractelement <16 x float> %conv115.i, i32 12
  %extract36.i = extractelement <16 x float> %conv115.i, i32 13
  %extract37.i = extractelement <16 x float> %conv115.i, i32 14
  %extract38.i = extractelement <16 x float> %conv115.i, i32 15
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %SyncBB377.i
  %vectorPHI.i = phi <16 x i32> [ %conv64.i, %SyncBB377.i ], [ %and89325.i, %for.body.i ]
  %j.019.i = phi i32 [ 0, %SyncBB377.i ], [ %inc.i, %for.body.i ]
  %and6.i = and <16 x i32> %vectorPHI.i, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %idxprom17.i = zext <16 x i32> %and6.i to <16 x i64>
  %extract.i = extractelement <16 x i64> %idxprom17.i, i32 0
  %extract8.i = extractelement <16 x i64> %idxprom17.i, i32 1
  %extract9.i = extractelement <16 x i64> %idxprom17.i, i32 2
  %extract10.i = extractelement <16 x i64> %idxprom17.i, i32 3
  %extract11.i = extractelement <16 x i64> %idxprom17.i, i32 4
  %extract12.i = extractelement <16 x i64> %idxprom17.i, i32 5
  %extract13.i = extractelement <16 x i64> %idxprom17.i, i32 6
  %extract14.i = extractelement <16 x i64> %idxprom17.i, i32 7
  %extract15.i = extractelement <16 x i64> %idxprom17.i, i32 8
  %extract16.i = extractelement <16 x i64> %idxprom17.i, i32 9
  %extract17.i = extractelement <16 x i64> %idxprom17.i, i32 10
  %extract18.i = extractelement <16 x i64> %idxprom17.i, i32 11
  %extract19.i = extractelement <16 x i64> %idxprom17.i, i32 12
  %extract20.i = extractelement <16 x i64> %idxprom17.i, i32 13
  %extract21.i = extractelement <16 x i64> %idxprom17.i, i32 14
  %extract22.i = extractelement <16 x i64> %idxprom17.i, i32 15
  %31 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract.i
  %32 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract8.i
  %33 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract9.i
  %34 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract10.i
  %35 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract11.i
  %36 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract12.i
  %37 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract13.i
  %38 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract14.i
  %39 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract15.i
  %40 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract16.i
  %41 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract17.i
  %42 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract18.i
  %43 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract19.i
  %44 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract20.i
  %45 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract21.i
  %46 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract22.i
  store float %extract23.i, float addrspace(3)* %31, align 4
  store float %extract24.i, float addrspace(3)* %32, align 4
  store float %extract25.i, float addrspace(3)* %33, align 4
  store float %extract26.i, float addrspace(3)* %34, align 4
  store float %extract27.i, float addrspace(3)* %35, align 4
  store float %extract28.i, float addrspace(3)* %36, align 4
  store float %extract29.i, float addrspace(3)* %37, align 4
  store float %extract30.i, float addrspace(3)* %38, align 4
  store float %extract31.i, float addrspace(3)* %39, align 4
  store float %extract32.i, float addrspace(3)* %40, align 4
  store float %extract33.i, float addrspace(3)* %41, align 4
  store float %extract34.i, float addrspace(3)* %42, align 4
  store float %extract35.i, float addrspace(3)* %43, align 4
  store float %extract36.i, float addrspace(3)* %44, align 4
  store float %extract37.i, float addrspace(3)* %45, align 4
  store float %extract38.i, float addrspace(3)* %46, align 4
  %add1439.i = add nsw <16 x i32> %vectorPHI.i, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %and1540.i = and <16 x i32> %add1439.i, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %idxprom16241.i = zext <16 x i32> %and1540.i to <16 x i64>
  %extract42.i = extractelement <16 x i64> %idxprom16241.i, i32 0
  %extract43.i = extractelement <16 x i64> %idxprom16241.i, i32 1
  %extract44.i = extractelement <16 x i64> %idxprom16241.i, i32 2
  %extract45.i = extractelement <16 x i64> %idxprom16241.i, i32 3
  %extract46.i = extractelement <16 x i64> %idxprom16241.i, i32 4
  %extract47.i = extractelement <16 x i64> %idxprom16241.i, i32 5
  %extract48.i = extractelement <16 x i64> %idxprom16241.i, i32 6
  %extract49.i = extractelement <16 x i64> %idxprom16241.i, i32 7
  %extract50.i = extractelement <16 x i64> %idxprom16241.i, i32 8
  %extract51.i = extractelement <16 x i64> %idxprom16241.i, i32 9
  %extract52.i = extractelement <16 x i64> %idxprom16241.i, i32 10
  %extract53.i = extractelement <16 x i64> %idxprom16241.i, i32 11
  %extract54.i = extractelement <16 x i64> %idxprom16241.i, i32 12
  %extract55.i = extractelement <16 x i64> %idxprom16241.i, i32 13
  %extract56.i = extractelement <16 x i64> %idxprom16241.i, i32 14
  %extract57.i = extractelement <16 x i64> %idxprom16241.i, i32 15
  %47 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract42.i
  %48 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract43.i
  %49 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract44.i
  %50 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract45.i
  %51 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract46.i
  %52 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract47.i
  %53 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract48.i
  %54 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract49.i
  %55 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract50.i
  %56 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract51.i
  %57 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract52.i
  %58 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract53.i
  %59 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract54.i
  %60 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract55.i
  %61 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract56.i
  %62 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract57.i
  store float %extract23.i, float addrspace(3)* %47, align 4
  store float %extract24.i, float addrspace(3)* %48, align 4
  store float %extract25.i, float addrspace(3)* %49, align 4
  store float %extract26.i, float addrspace(3)* %50, align 4
  store float %extract27.i, float addrspace(3)* %51, align 4
  store float %extract28.i, float addrspace(3)* %52, align 4
  store float %extract29.i, float addrspace(3)* %53, align 4
  store float %extract30.i, float addrspace(3)* %54, align 4
  store float %extract31.i, float addrspace(3)* %55, align 4
  store float %extract32.i, float addrspace(3)* %56, align 4
  store float %extract33.i, float addrspace(3)* %57, align 4
  store float %extract34.i, float addrspace(3)* %58, align 4
  store float %extract35.i, float addrspace(3)* %59, align 4
  store float %extract36.i, float addrspace(3)* %60, align 4
  store float %extract37.i, float addrspace(3)* %61, align 4
  store float %extract38.i, float addrspace(3)* %62, align 4
  %add1958.i = add nsw <16 x i32> %vectorPHI.i, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %and2059.i = and <16 x i32> %add1958.i, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %idxprom21360.i = zext <16 x i32> %and2059.i to <16 x i64>
  %extract61.i = extractelement <16 x i64> %idxprom21360.i, i32 0
  %extract62.i = extractelement <16 x i64> %idxprom21360.i, i32 1
  %extract63.i = extractelement <16 x i64> %idxprom21360.i, i32 2
  %extract64.i = extractelement <16 x i64> %idxprom21360.i, i32 3
  %extract65.i = extractelement <16 x i64> %idxprom21360.i, i32 4
  %extract66.i = extractelement <16 x i64> %idxprom21360.i, i32 5
  %extract67.i = extractelement <16 x i64> %idxprom21360.i, i32 6
  %extract68.i = extractelement <16 x i64> %idxprom21360.i, i32 7
  %extract69.i = extractelement <16 x i64> %idxprom21360.i, i32 8
  %extract70.i = extractelement <16 x i64> %idxprom21360.i, i32 9
  %extract71.i = extractelement <16 x i64> %idxprom21360.i, i32 10
  %extract72.i = extractelement <16 x i64> %idxprom21360.i, i32 11
  %extract73.i = extractelement <16 x i64> %idxprom21360.i, i32 12
  %extract74.i = extractelement <16 x i64> %idxprom21360.i, i32 13
  %extract75.i = extractelement <16 x i64> %idxprom21360.i, i32 14
  %extract76.i = extractelement <16 x i64> %idxprom21360.i, i32 15
  %63 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract61.i
  %64 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract62.i
  %65 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract63.i
  %66 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract64.i
  %67 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract65.i
  %68 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract66.i
  %69 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract67.i
  %70 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract68.i
  %71 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract69.i
  %72 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract70.i
  %73 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract71.i
  %74 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract72.i
  %75 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract73.i
  %76 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract74.i
  %77 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract75.i
  %78 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract76.i
  store float %extract23.i, float addrspace(3)* %63, align 4
  store float %extract24.i, float addrspace(3)* %64, align 4
  store float %extract25.i, float addrspace(3)* %65, align 4
  store float %extract26.i, float addrspace(3)* %66, align 4
  store float %extract27.i, float addrspace(3)* %67, align 4
  store float %extract28.i, float addrspace(3)* %68, align 4
  store float %extract29.i, float addrspace(3)* %69, align 4
  store float %extract30.i, float addrspace(3)* %70, align 4
  store float %extract31.i, float addrspace(3)* %71, align 4
  store float %extract32.i, float addrspace(3)* %72, align 4
  store float %extract33.i, float addrspace(3)* %73, align 4
  store float %extract34.i, float addrspace(3)* %74, align 4
  store float %extract35.i, float addrspace(3)* %75, align 4
  store float %extract36.i, float addrspace(3)* %76, align 4
  store float %extract37.i, float addrspace(3)* %77, align 4
  store float %extract38.i, float addrspace(3)* %78, align 4
  %add2477.i = add nsw <16 x i32> %vectorPHI.i, <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
  %and2578.i = and <16 x i32> %add2477.i, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %idxprom26479.i = zext <16 x i32> %and2578.i to <16 x i64>
  %extract80.i = extractelement <16 x i64> %idxprom26479.i, i32 0
  %extract81.i = extractelement <16 x i64> %idxprom26479.i, i32 1
  %extract82.i = extractelement <16 x i64> %idxprom26479.i, i32 2
  %extract83.i = extractelement <16 x i64> %idxprom26479.i, i32 3
  %extract84.i = extractelement <16 x i64> %idxprom26479.i, i32 4
  %extract85.i = extractelement <16 x i64> %idxprom26479.i, i32 5
  %extract86.i = extractelement <16 x i64> %idxprom26479.i, i32 6
  %extract87.i = extractelement <16 x i64> %idxprom26479.i, i32 7
  %extract88.i = extractelement <16 x i64> %idxprom26479.i, i32 8
  %extract89.i = extractelement <16 x i64> %idxprom26479.i, i32 9
  %extract90.i = extractelement <16 x i64> %idxprom26479.i, i32 10
  %extract91.i = extractelement <16 x i64> %idxprom26479.i, i32 11
  %extract92.i = extractelement <16 x i64> %idxprom26479.i, i32 12
  %extract93.i = extractelement <16 x i64> %idxprom26479.i, i32 13
  %extract94.i = extractelement <16 x i64> %idxprom26479.i, i32 14
  %extract95.i = extractelement <16 x i64> %idxprom26479.i, i32 15
  %79 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract80.i
  %80 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract81.i
  %81 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract82.i
  %82 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract83.i
  %83 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract84.i
  %84 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract85.i
  %85 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract86.i
  %86 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract87.i
  %87 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract88.i
  %88 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract89.i
  %89 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract90.i
  %90 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract91.i
  %91 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract92.i
  %92 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract93.i
  %93 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract94.i
  %94 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract95.i
  store float %extract23.i, float addrspace(3)* %79, align 4
  store float %extract24.i, float addrspace(3)* %80, align 4
  store float %extract25.i, float addrspace(3)* %81, align 4
  store float %extract26.i, float addrspace(3)* %82, align 4
  store float %extract27.i, float addrspace(3)* %83, align 4
  store float %extract28.i, float addrspace(3)* %84, align 4
  store float %extract29.i, float addrspace(3)* %85, align 4
  store float %extract30.i, float addrspace(3)* %86, align 4
  store float %extract31.i, float addrspace(3)* %87, align 4
  store float %extract32.i, float addrspace(3)* %88, align 4
  store float %extract33.i, float addrspace(3)* %89, align 4
  store float %extract34.i, float addrspace(3)* %90, align 4
  store float %extract35.i, float addrspace(3)* %91, align 4
  store float %extract36.i, float addrspace(3)* %92, align 4
  store float %extract37.i, float addrspace(3)* %93, align 4
  store float %extract38.i, float addrspace(3)* %94, align 4
  %add2996.i = add nsw <16 x i32> %vectorPHI.i, <i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4>
  %and3097.i = and <16 x i32> %add2996.i, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %idxprom31598.i = zext <16 x i32> %and3097.i to <16 x i64>
  %extract99.i = extractelement <16 x i64> %idxprom31598.i, i32 0
  %extract100.i = extractelement <16 x i64> %idxprom31598.i, i32 1
  %extract101.i = extractelement <16 x i64> %idxprom31598.i, i32 2
  %extract102.i = extractelement <16 x i64> %idxprom31598.i, i32 3
  %extract103.i = extractelement <16 x i64> %idxprom31598.i, i32 4
  %extract104.i = extractelement <16 x i64> %idxprom31598.i, i32 5
  %extract105.i = extractelement <16 x i64> %idxprom31598.i, i32 6
  %extract106.i = extractelement <16 x i64> %idxprom31598.i, i32 7
  %extract107.i = extractelement <16 x i64> %idxprom31598.i, i32 8
  %extract108.i = extractelement <16 x i64> %idxprom31598.i, i32 9
  %extract109.i = extractelement <16 x i64> %idxprom31598.i, i32 10
  %extract110.i = extractelement <16 x i64> %idxprom31598.i, i32 11
  %extract111.i = extractelement <16 x i64> %idxprom31598.i, i32 12
  %extract112.i = extractelement <16 x i64> %idxprom31598.i, i32 13
  %extract113.i = extractelement <16 x i64> %idxprom31598.i, i32 14
  %extract114.i = extractelement <16 x i64> %idxprom31598.i, i32 15
  %95 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract99.i
  %96 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract100.i
  %97 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract101.i
  %98 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract102.i
  %99 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract103.i
  %100 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract104.i
  %101 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract105.i
  %102 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract106.i
  %103 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract107.i
  %104 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract108.i
  %105 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract109.i
  %106 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract110.i
  %107 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract111.i
  %108 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract112.i
  %109 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract113.i
  %110 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract114.i
  store float %extract23.i, float addrspace(3)* %95, align 4
  store float %extract24.i, float addrspace(3)* %96, align 4
  store float %extract25.i, float addrspace(3)* %97, align 4
  store float %extract26.i, float addrspace(3)* %98, align 4
  store float %extract27.i, float addrspace(3)* %99, align 4
  store float %extract28.i, float addrspace(3)* %100, align 4
  store float %extract29.i, float addrspace(3)* %101, align 4
  store float %extract30.i, float addrspace(3)* %102, align 4
  store float %extract31.i, float addrspace(3)* %103, align 4
  store float %extract32.i, float addrspace(3)* %104, align 4
  store float %extract33.i, float addrspace(3)* %105, align 4
  store float %extract34.i, float addrspace(3)* %106, align 4
  store float %extract35.i, float addrspace(3)* %107, align 4
  store float %extract36.i, float addrspace(3)* %108, align 4
  store float %extract37.i, float addrspace(3)* %109, align 4
  store float %extract38.i, float addrspace(3)* %110, align 4
  %add34115.i = add nsw <16 x i32> %vectorPHI.i, <i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5>
  %and35116.i = and <16 x i32> %add34115.i, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %idxprom366117.i = zext <16 x i32> %and35116.i to <16 x i64>
  %extract118.i = extractelement <16 x i64> %idxprom366117.i, i32 0
  %extract119.i = extractelement <16 x i64> %idxprom366117.i, i32 1
  %extract120.i = extractelement <16 x i64> %idxprom366117.i, i32 2
  %extract121.i = extractelement <16 x i64> %idxprom366117.i, i32 3
  %extract122.i = extractelement <16 x i64> %idxprom366117.i, i32 4
  %extract123.i = extractelement <16 x i64> %idxprom366117.i, i32 5
  %extract124.i = extractelement <16 x i64> %idxprom366117.i, i32 6
  %extract125.i = extractelement <16 x i64> %idxprom366117.i, i32 7
  %extract126.i = extractelement <16 x i64> %idxprom366117.i, i32 8
  %extract127.i = extractelement <16 x i64> %idxprom366117.i, i32 9
  %extract128.i = extractelement <16 x i64> %idxprom366117.i, i32 10
  %extract129.i = extractelement <16 x i64> %idxprom366117.i, i32 11
  %extract130.i = extractelement <16 x i64> %idxprom366117.i, i32 12
  %extract131.i = extractelement <16 x i64> %idxprom366117.i, i32 13
  %extract132.i = extractelement <16 x i64> %idxprom366117.i, i32 14
  %extract133.i = extractelement <16 x i64> %idxprom366117.i, i32 15
  %111 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract118.i
  %112 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract119.i
  %113 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract120.i
  %114 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract121.i
  %115 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract122.i
  %116 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract123.i
  %117 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract124.i
  %118 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract125.i
  %119 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract126.i
  %120 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract127.i
  %121 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract128.i
  %122 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract129.i
  %123 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract130.i
  %124 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract131.i
  %125 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract132.i
  %126 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract133.i
  store float %extract23.i, float addrspace(3)* %111, align 4
  store float %extract24.i, float addrspace(3)* %112, align 4
  store float %extract25.i, float addrspace(3)* %113, align 4
  store float %extract26.i, float addrspace(3)* %114, align 4
  store float %extract27.i, float addrspace(3)* %115, align 4
  store float %extract28.i, float addrspace(3)* %116, align 4
  store float %extract29.i, float addrspace(3)* %117, align 4
  store float %extract30.i, float addrspace(3)* %118, align 4
  store float %extract31.i, float addrspace(3)* %119, align 4
  store float %extract32.i, float addrspace(3)* %120, align 4
  store float %extract33.i, float addrspace(3)* %121, align 4
  store float %extract34.i, float addrspace(3)* %122, align 4
  store float %extract35.i, float addrspace(3)* %123, align 4
  store float %extract36.i, float addrspace(3)* %124, align 4
  store float %extract37.i, float addrspace(3)* %125, align 4
  store float %extract38.i, float addrspace(3)* %126, align 4
  %add39134.i = add nsw <16 x i32> %vectorPHI.i, <i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6>
  %and40135.i = and <16 x i32> %add39134.i, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %idxprom417136.i = zext <16 x i32> %and40135.i to <16 x i64>
  %extract137.i = extractelement <16 x i64> %idxprom417136.i, i32 0
  %extract138.i = extractelement <16 x i64> %idxprom417136.i, i32 1
  %extract139.i = extractelement <16 x i64> %idxprom417136.i, i32 2
  %extract140.i = extractelement <16 x i64> %idxprom417136.i, i32 3
  %extract141.i = extractelement <16 x i64> %idxprom417136.i, i32 4
  %extract142.i = extractelement <16 x i64> %idxprom417136.i, i32 5
  %extract143.i = extractelement <16 x i64> %idxprom417136.i, i32 6
  %extract144.i = extractelement <16 x i64> %idxprom417136.i, i32 7
  %extract145.i = extractelement <16 x i64> %idxprom417136.i, i32 8
  %extract146.i = extractelement <16 x i64> %idxprom417136.i, i32 9
  %extract147.i = extractelement <16 x i64> %idxprom417136.i, i32 10
  %extract148.i = extractelement <16 x i64> %idxprom417136.i, i32 11
  %extract149.i = extractelement <16 x i64> %idxprom417136.i, i32 12
  %extract150.i = extractelement <16 x i64> %idxprom417136.i, i32 13
  %extract151.i = extractelement <16 x i64> %idxprom417136.i, i32 14
  %extract152.i = extractelement <16 x i64> %idxprom417136.i, i32 15
  %127 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract137.i
  %128 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract138.i
  %129 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract139.i
  %130 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract140.i
  %131 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract141.i
  %132 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract142.i
  %133 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract143.i
  %134 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract144.i
  %135 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract145.i
  %136 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract146.i
  %137 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract147.i
  %138 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract148.i
  %139 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract149.i
  %140 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract150.i
  %141 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract151.i
  %142 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract152.i
  store float %extract23.i, float addrspace(3)* %127, align 4
  store float %extract24.i, float addrspace(3)* %128, align 4
  store float %extract25.i, float addrspace(3)* %129, align 4
  store float %extract26.i, float addrspace(3)* %130, align 4
  store float %extract27.i, float addrspace(3)* %131, align 4
  store float %extract28.i, float addrspace(3)* %132, align 4
  store float %extract29.i, float addrspace(3)* %133, align 4
  store float %extract30.i, float addrspace(3)* %134, align 4
  store float %extract31.i, float addrspace(3)* %135, align 4
  store float %extract32.i, float addrspace(3)* %136, align 4
  store float %extract33.i, float addrspace(3)* %137, align 4
  store float %extract34.i, float addrspace(3)* %138, align 4
  store float %extract35.i, float addrspace(3)* %139, align 4
  store float %extract36.i, float addrspace(3)* %140, align 4
  store float %extract37.i, float addrspace(3)* %141, align 4
  store float %extract38.i, float addrspace(3)* %142, align 4
  %add44153.i = add nsw <16 x i32> %vectorPHI.i, <i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7>
  %and45154.i = and <16 x i32> %add44153.i, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %idxprom468155.i = zext <16 x i32> %and45154.i to <16 x i64>
  %extract156.i = extractelement <16 x i64> %idxprom468155.i, i32 0
  %extract157.i = extractelement <16 x i64> %idxprom468155.i, i32 1
  %extract158.i = extractelement <16 x i64> %idxprom468155.i, i32 2
  %extract159.i = extractelement <16 x i64> %idxprom468155.i, i32 3
  %extract160.i = extractelement <16 x i64> %idxprom468155.i, i32 4
  %extract161.i = extractelement <16 x i64> %idxprom468155.i, i32 5
  %extract162.i = extractelement <16 x i64> %idxprom468155.i, i32 6
  %extract163.i = extractelement <16 x i64> %idxprom468155.i, i32 7
  %extract164.i = extractelement <16 x i64> %idxprom468155.i, i32 8
  %extract165.i = extractelement <16 x i64> %idxprom468155.i, i32 9
  %extract166.i = extractelement <16 x i64> %idxprom468155.i, i32 10
  %extract167.i = extractelement <16 x i64> %idxprom468155.i, i32 11
  %extract168.i = extractelement <16 x i64> %idxprom468155.i, i32 12
  %extract169.i = extractelement <16 x i64> %idxprom468155.i, i32 13
  %extract170.i = extractelement <16 x i64> %idxprom468155.i, i32 14
  %extract171.i = extractelement <16 x i64> %idxprom468155.i, i32 15
  %143 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract156.i
  %144 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract157.i
  %145 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract158.i
  %146 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract159.i
  %147 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract160.i
  %148 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract161.i
  %149 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract162.i
  %150 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract163.i
  %151 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract164.i
  %152 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract165.i
  %153 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract166.i
  %154 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract167.i
  %155 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract168.i
  %156 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract169.i
  %157 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract170.i
  %158 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract171.i
  store float %extract23.i, float addrspace(3)* %143, align 4
  store float %extract24.i, float addrspace(3)* %144, align 4
  store float %extract25.i, float addrspace(3)* %145, align 4
  store float %extract26.i, float addrspace(3)* %146, align 4
  store float %extract27.i, float addrspace(3)* %147, align 4
  store float %extract28.i, float addrspace(3)* %148, align 4
  store float %extract29.i, float addrspace(3)* %149, align 4
  store float %extract30.i, float addrspace(3)* %150, align 4
  store float %extract31.i, float addrspace(3)* %151, align 4
  store float %extract32.i, float addrspace(3)* %152, align 4
  store float %extract33.i, float addrspace(3)* %153, align 4
  store float %extract34.i, float addrspace(3)* %154, align 4
  store float %extract35.i, float addrspace(3)* %155, align 4
  store float %extract36.i, float addrspace(3)* %156, align 4
  store float %extract37.i, float addrspace(3)* %157, align 4
  store float %extract38.i, float addrspace(3)* %158, align 4
  %add49172.i = add nsw <16 x i32> %vectorPHI.i, <i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8>
  %and50173.i = and <16 x i32> %add49172.i, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %idxprom519174.i = zext <16 x i32> %and50173.i to <16 x i64>
  %extract175.i = extractelement <16 x i64> %idxprom519174.i, i32 0
  %extract176.i = extractelement <16 x i64> %idxprom519174.i, i32 1
  %extract177.i = extractelement <16 x i64> %idxprom519174.i, i32 2
  %extract178.i = extractelement <16 x i64> %idxprom519174.i, i32 3
  %extract179.i = extractelement <16 x i64> %idxprom519174.i, i32 4
  %extract180.i = extractelement <16 x i64> %idxprom519174.i, i32 5
  %extract181.i = extractelement <16 x i64> %idxprom519174.i, i32 6
  %extract182.i = extractelement <16 x i64> %idxprom519174.i, i32 7
  %extract183.i = extractelement <16 x i64> %idxprom519174.i, i32 8
  %extract184.i = extractelement <16 x i64> %idxprom519174.i, i32 9
  %extract185.i = extractelement <16 x i64> %idxprom519174.i, i32 10
  %extract186.i = extractelement <16 x i64> %idxprom519174.i, i32 11
  %extract187.i = extractelement <16 x i64> %idxprom519174.i, i32 12
  %extract188.i = extractelement <16 x i64> %idxprom519174.i, i32 13
  %extract189.i = extractelement <16 x i64> %idxprom519174.i, i32 14
  %extract190.i = extractelement <16 x i64> %idxprom519174.i, i32 15
  %159 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract175.i
  %160 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract176.i
  %161 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract177.i
  %162 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract178.i
  %163 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract179.i
  %164 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract180.i
  %165 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract181.i
  %166 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract182.i
  %167 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract183.i
  %168 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract184.i
  %169 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract185.i
  %170 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract186.i
  %171 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract187.i
  %172 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract188.i
  %173 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract189.i
  %174 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract190.i
  store float %extract23.i, float addrspace(3)* %159, align 4
  store float %extract24.i, float addrspace(3)* %160, align 4
  store float %extract25.i, float addrspace(3)* %161, align 4
  store float %extract26.i, float addrspace(3)* %162, align 4
  store float %extract27.i, float addrspace(3)* %163, align 4
  store float %extract28.i, float addrspace(3)* %164, align 4
  store float %extract29.i, float addrspace(3)* %165, align 4
  store float %extract30.i, float addrspace(3)* %166, align 4
  store float %extract31.i, float addrspace(3)* %167, align 4
  store float %extract32.i, float addrspace(3)* %168, align 4
  store float %extract33.i, float addrspace(3)* %169, align 4
  store float %extract34.i, float addrspace(3)* %170, align 4
  store float %extract35.i, float addrspace(3)* %171, align 4
  store float %extract36.i, float addrspace(3)* %172, align 4
  store float %extract37.i, float addrspace(3)* %173, align 4
  store float %extract38.i, float addrspace(3)* %174, align 4
  %add54191.i = add nsw <16 x i32> %vectorPHI.i, <i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9>
  %and55192.i = and <16 x i32> %add54191.i, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %idxprom5610193.i = zext <16 x i32> %and55192.i to <16 x i64>
  %extract194.i = extractelement <16 x i64> %idxprom5610193.i, i32 0
  %extract195.i = extractelement <16 x i64> %idxprom5610193.i, i32 1
  %extract196.i = extractelement <16 x i64> %idxprom5610193.i, i32 2
  %extract197.i = extractelement <16 x i64> %idxprom5610193.i, i32 3
  %extract198.i = extractelement <16 x i64> %idxprom5610193.i, i32 4
  %extract199.i = extractelement <16 x i64> %idxprom5610193.i, i32 5
  %extract200.i = extractelement <16 x i64> %idxprom5610193.i, i32 6
  %extract201.i = extractelement <16 x i64> %idxprom5610193.i, i32 7
  %extract202.i = extractelement <16 x i64> %idxprom5610193.i, i32 8
  %extract203.i = extractelement <16 x i64> %idxprom5610193.i, i32 9
  %extract204.i = extractelement <16 x i64> %idxprom5610193.i, i32 10
  %extract205.i = extractelement <16 x i64> %idxprom5610193.i, i32 11
  %extract206.i = extractelement <16 x i64> %idxprom5610193.i, i32 12
  %extract207.i = extractelement <16 x i64> %idxprom5610193.i, i32 13
  %extract208.i = extractelement <16 x i64> %idxprom5610193.i, i32 14
  %extract209.i = extractelement <16 x i64> %idxprom5610193.i, i32 15
  %175 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract194.i
  %176 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract195.i
  %177 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract196.i
  %178 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract197.i
  %179 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract198.i
  %180 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract199.i
  %181 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract200.i
  %182 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract201.i
  %183 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract202.i
  %184 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract203.i
  %185 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract204.i
  %186 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract205.i
  %187 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract206.i
  %188 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract207.i
  %189 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract208.i
  %190 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract209.i
  store float %extract23.i, float addrspace(3)* %175, align 4
  store float %extract24.i, float addrspace(3)* %176, align 4
  store float %extract25.i, float addrspace(3)* %177, align 4
  store float %extract26.i, float addrspace(3)* %178, align 4
  store float %extract27.i, float addrspace(3)* %179, align 4
  store float %extract28.i, float addrspace(3)* %180, align 4
  store float %extract29.i, float addrspace(3)* %181, align 4
  store float %extract30.i, float addrspace(3)* %182, align 4
  store float %extract31.i, float addrspace(3)* %183, align 4
  store float %extract32.i, float addrspace(3)* %184, align 4
  store float %extract33.i, float addrspace(3)* %185, align 4
  store float %extract34.i, float addrspace(3)* %186, align 4
  store float %extract35.i, float addrspace(3)* %187, align 4
  store float %extract36.i, float addrspace(3)* %188, align 4
  store float %extract37.i, float addrspace(3)* %189, align 4
  store float %extract38.i, float addrspace(3)* %190, align 4
  %add59210.i = add nsw <16 x i32> %vectorPHI.i, <i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10>
  %and60211.i = and <16 x i32> %add59210.i, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %idxprom6111212.i = zext <16 x i32> %and60211.i to <16 x i64>
  %extract213.i = extractelement <16 x i64> %idxprom6111212.i, i32 0
  %extract214.i = extractelement <16 x i64> %idxprom6111212.i, i32 1
  %extract215.i = extractelement <16 x i64> %idxprom6111212.i, i32 2
  %extract216.i = extractelement <16 x i64> %idxprom6111212.i, i32 3
  %extract217.i = extractelement <16 x i64> %idxprom6111212.i, i32 4
  %extract218.i = extractelement <16 x i64> %idxprom6111212.i, i32 5
  %extract219.i = extractelement <16 x i64> %idxprom6111212.i, i32 6
  %extract220.i = extractelement <16 x i64> %idxprom6111212.i, i32 7
  %extract221.i = extractelement <16 x i64> %idxprom6111212.i, i32 8
  %extract222.i = extractelement <16 x i64> %idxprom6111212.i, i32 9
  %extract223.i = extractelement <16 x i64> %idxprom6111212.i, i32 10
  %extract224.i = extractelement <16 x i64> %idxprom6111212.i, i32 11
  %extract225.i = extractelement <16 x i64> %idxprom6111212.i, i32 12
  %extract226.i = extractelement <16 x i64> %idxprom6111212.i, i32 13
  %extract227.i = extractelement <16 x i64> %idxprom6111212.i, i32 14
  %extract228.i = extractelement <16 x i64> %idxprom6111212.i, i32 15
  %191 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract213.i
  %192 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract214.i
  %193 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract215.i
  %194 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract216.i
  %195 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract217.i
  %196 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract218.i
  %197 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract219.i
  %198 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract220.i
  %199 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract221.i
  %200 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract222.i
  %201 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract223.i
  %202 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract224.i
  %203 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract225.i
  %204 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract226.i
  %205 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract227.i
  %206 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract228.i
  store float %extract23.i, float addrspace(3)* %191, align 4
  store float %extract24.i, float addrspace(3)* %192, align 4
  store float %extract25.i, float addrspace(3)* %193, align 4
  store float %extract26.i, float addrspace(3)* %194, align 4
  store float %extract27.i, float addrspace(3)* %195, align 4
  store float %extract28.i, float addrspace(3)* %196, align 4
  store float %extract29.i, float addrspace(3)* %197, align 4
  store float %extract30.i, float addrspace(3)* %198, align 4
  store float %extract31.i, float addrspace(3)* %199, align 4
  store float %extract32.i, float addrspace(3)* %200, align 4
  store float %extract33.i, float addrspace(3)* %201, align 4
  store float %extract34.i, float addrspace(3)* %202, align 4
  store float %extract35.i, float addrspace(3)* %203, align 4
  store float %extract36.i, float addrspace(3)* %204, align 4
  store float %extract37.i, float addrspace(3)* %205, align 4
  store float %extract38.i, float addrspace(3)* %206, align 4
  %add64229.i = add nsw <16 x i32> %vectorPHI.i, <i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11>
  %and65230.i = and <16 x i32> %add64229.i, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %idxprom6612231.i = zext <16 x i32> %and65230.i to <16 x i64>
  %extract232.i = extractelement <16 x i64> %idxprom6612231.i, i32 0
  %extract233.i = extractelement <16 x i64> %idxprom6612231.i, i32 1
  %extract234.i = extractelement <16 x i64> %idxprom6612231.i, i32 2
  %extract235.i = extractelement <16 x i64> %idxprom6612231.i, i32 3
  %extract236.i = extractelement <16 x i64> %idxprom6612231.i, i32 4
  %extract237.i = extractelement <16 x i64> %idxprom6612231.i, i32 5
  %extract238.i = extractelement <16 x i64> %idxprom6612231.i, i32 6
  %extract239.i = extractelement <16 x i64> %idxprom6612231.i, i32 7
  %extract240.i = extractelement <16 x i64> %idxprom6612231.i, i32 8
  %extract241.i = extractelement <16 x i64> %idxprom6612231.i, i32 9
  %extract242.i = extractelement <16 x i64> %idxprom6612231.i, i32 10
  %extract243.i = extractelement <16 x i64> %idxprom6612231.i, i32 11
  %extract244.i = extractelement <16 x i64> %idxprom6612231.i, i32 12
  %extract245.i = extractelement <16 x i64> %idxprom6612231.i, i32 13
  %extract246.i = extractelement <16 x i64> %idxprom6612231.i, i32 14
  %extract247.i = extractelement <16 x i64> %idxprom6612231.i, i32 15
  %207 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract232.i
  %208 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract233.i
  %209 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract234.i
  %210 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract235.i
  %211 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract236.i
  %212 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract237.i
  %213 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract238.i
  %214 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract239.i
  %215 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract240.i
  %216 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract241.i
  %217 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract242.i
  %218 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract243.i
  %219 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract244.i
  %220 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract245.i
  %221 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract246.i
  %222 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract247.i
  store float %extract23.i, float addrspace(3)* %207, align 4
  store float %extract24.i, float addrspace(3)* %208, align 4
  store float %extract25.i, float addrspace(3)* %209, align 4
  store float %extract26.i, float addrspace(3)* %210, align 4
  store float %extract27.i, float addrspace(3)* %211, align 4
  store float %extract28.i, float addrspace(3)* %212, align 4
  store float %extract29.i, float addrspace(3)* %213, align 4
  store float %extract30.i, float addrspace(3)* %214, align 4
  store float %extract31.i, float addrspace(3)* %215, align 4
  store float %extract32.i, float addrspace(3)* %216, align 4
  store float %extract33.i, float addrspace(3)* %217, align 4
  store float %extract34.i, float addrspace(3)* %218, align 4
  store float %extract35.i, float addrspace(3)* %219, align 4
  store float %extract36.i, float addrspace(3)* %220, align 4
  store float %extract37.i, float addrspace(3)* %221, align 4
  store float %extract38.i, float addrspace(3)* %222, align 4
  %add69248.i = add nsw <16 x i32> %vectorPHI.i, <i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12>
  %and70249.i = and <16 x i32> %add69248.i, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %idxprom7113250.i = zext <16 x i32> %and70249.i to <16 x i64>
  %extract251.i = extractelement <16 x i64> %idxprom7113250.i, i32 0
  %extract252.i = extractelement <16 x i64> %idxprom7113250.i, i32 1
  %extract253.i = extractelement <16 x i64> %idxprom7113250.i, i32 2
  %extract254.i = extractelement <16 x i64> %idxprom7113250.i, i32 3
  %extract255.i = extractelement <16 x i64> %idxprom7113250.i, i32 4
  %extract256.i = extractelement <16 x i64> %idxprom7113250.i, i32 5
  %extract257.i = extractelement <16 x i64> %idxprom7113250.i, i32 6
  %extract258.i = extractelement <16 x i64> %idxprom7113250.i, i32 7
  %extract259.i = extractelement <16 x i64> %idxprom7113250.i, i32 8
  %extract260.i = extractelement <16 x i64> %idxprom7113250.i, i32 9
  %extract261.i = extractelement <16 x i64> %idxprom7113250.i, i32 10
  %extract262.i = extractelement <16 x i64> %idxprom7113250.i, i32 11
  %extract263.i = extractelement <16 x i64> %idxprom7113250.i, i32 12
  %extract264.i = extractelement <16 x i64> %idxprom7113250.i, i32 13
  %extract265.i = extractelement <16 x i64> %idxprom7113250.i, i32 14
  %extract266.i = extractelement <16 x i64> %idxprom7113250.i, i32 15
  %223 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract251.i
  %224 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract252.i
  %225 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract253.i
  %226 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract254.i
  %227 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract255.i
  %228 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract256.i
  %229 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract257.i
  %230 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract258.i
  %231 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract259.i
  %232 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract260.i
  %233 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract261.i
  %234 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract262.i
  %235 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract263.i
  %236 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract264.i
  %237 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract265.i
  %238 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract266.i
  store float %extract23.i, float addrspace(3)* %223, align 4
  store float %extract24.i, float addrspace(3)* %224, align 4
  store float %extract25.i, float addrspace(3)* %225, align 4
  store float %extract26.i, float addrspace(3)* %226, align 4
  store float %extract27.i, float addrspace(3)* %227, align 4
  store float %extract28.i, float addrspace(3)* %228, align 4
  store float %extract29.i, float addrspace(3)* %229, align 4
  store float %extract30.i, float addrspace(3)* %230, align 4
  store float %extract31.i, float addrspace(3)* %231, align 4
  store float %extract32.i, float addrspace(3)* %232, align 4
  store float %extract33.i, float addrspace(3)* %233, align 4
  store float %extract34.i, float addrspace(3)* %234, align 4
  store float %extract35.i, float addrspace(3)* %235, align 4
  store float %extract36.i, float addrspace(3)* %236, align 4
  store float %extract37.i, float addrspace(3)* %237, align 4
  store float %extract38.i, float addrspace(3)* %238, align 4
  %add74267.i = add nsw <16 x i32> %vectorPHI.i, <i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13>
  %and75268.i = and <16 x i32> %add74267.i, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %idxprom7614269.i = zext <16 x i32> %and75268.i to <16 x i64>
  %extract270.i = extractelement <16 x i64> %idxprom7614269.i, i32 0
  %extract271.i = extractelement <16 x i64> %idxprom7614269.i, i32 1
  %extract272.i = extractelement <16 x i64> %idxprom7614269.i, i32 2
  %extract273.i = extractelement <16 x i64> %idxprom7614269.i, i32 3
  %extract274.i = extractelement <16 x i64> %idxprom7614269.i, i32 4
  %extract275.i = extractelement <16 x i64> %idxprom7614269.i, i32 5
  %extract276.i = extractelement <16 x i64> %idxprom7614269.i, i32 6
  %extract277.i = extractelement <16 x i64> %idxprom7614269.i, i32 7
  %extract278.i = extractelement <16 x i64> %idxprom7614269.i, i32 8
  %extract279.i = extractelement <16 x i64> %idxprom7614269.i, i32 9
  %extract280.i = extractelement <16 x i64> %idxprom7614269.i, i32 10
  %extract281.i = extractelement <16 x i64> %idxprom7614269.i, i32 11
  %extract282.i = extractelement <16 x i64> %idxprom7614269.i, i32 12
  %extract283.i = extractelement <16 x i64> %idxprom7614269.i, i32 13
  %extract284.i = extractelement <16 x i64> %idxprom7614269.i, i32 14
  %extract285.i = extractelement <16 x i64> %idxprom7614269.i, i32 15
  %239 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract270.i
  %240 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract271.i
  %241 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract272.i
  %242 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract273.i
  %243 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract274.i
  %244 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract275.i
  %245 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract276.i
  %246 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract277.i
  %247 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract278.i
  %248 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract279.i
  %249 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract280.i
  %250 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract281.i
  %251 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract282.i
  %252 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract283.i
  %253 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract284.i
  %254 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract285.i
  store float %extract23.i, float addrspace(3)* %239, align 4
  store float %extract24.i, float addrspace(3)* %240, align 4
  store float %extract25.i, float addrspace(3)* %241, align 4
  store float %extract26.i, float addrspace(3)* %242, align 4
  store float %extract27.i, float addrspace(3)* %243, align 4
  store float %extract28.i, float addrspace(3)* %244, align 4
  store float %extract29.i, float addrspace(3)* %245, align 4
  store float %extract30.i, float addrspace(3)* %246, align 4
  store float %extract31.i, float addrspace(3)* %247, align 4
  store float %extract32.i, float addrspace(3)* %248, align 4
  store float %extract33.i, float addrspace(3)* %249, align 4
  store float %extract34.i, float addrspace(3)* %250, align 4
  store float %extract35.i, float addrspace(3)* %251, align 4
  store float %extract36.i, float addrspace(3)* %252, align 4
  store float %extract37.i, float addrspace(3)* %253, align 4
  store float %extract38.i, float addrspace(3)* %254, align 4
  %add79286.i = add nsw <16 x i32> %vectorPHI.i, <i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14>
  %and80287.i = and <16 x i32> %add79286.i, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %idxprom8115288.i = zext <16 x i32> %and80287.i to <16 x i64>
  %extract289.i = extractelement <16 x i64> %idxprom8115288.i, i32 0
  %extract290.i = extractelement <16 x i64> %idxprom8115288.i, i32 1
  %extract291.i = extractelement <16 x i64> %idxprom8115288.i, i32 2
  %extract292.i = extractelement <16 x i64> %idxprom8115288.i, i32 3
  %extract293.i = extractelement <16 x i64> %idxprom8115288.i, i32 4
  %extract294.i = extractelement <16 x i64> %idxprom8115288.i, i32 5
  %extract295.i = extractelement <16 x i64> %idxprom8115288.i, i32 6
  %extract296.i = extractelement <16 x i64> %idxprom8115288.i, i32 7
  %extract297.i = extractelement <16 x i64> %idxprom8115288.i, i32 8
  %extract298.i = extractelement <16 x i64> %idxprom8115288.i, i32 9
  %extract299.i = extractelement <16 x i64> %idxprom8115288.i, i32 10
  %extract300.i = extractelement <16 x i64> %idxprom8115288.i, i32 11
  %extract301.i = extractelement <16 x i64> %idxprom8115288.i, i32 12
  %extract302.i = extractelement <16 x i64> %idxprom8115288.i, i32 13
  %extract303.i = extractelement <16 x i64> %idxprom8115288.i, i32 14
  %extract304.i = extractelement <16 x i64> %idxprom8115288.i, i32 15
  %255 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract289.i
  %256 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract290.i
  %257 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract291.i
  %258 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract292.i
  %259 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract293.i
  %260 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract294.i
  %261 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract295.i
  %262 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract296.i
  %263 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract297.i
  %264 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract298.i
  %265 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract299.i
  %266 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract300.i
  %267 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract301.i
  %268 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract302.i
  %269 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract303.i
  %270 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract304.i
  store float %extract23.i, float addrspace(3)* %255, align 4
  store float %extract24.i, float addrspace(3)* %256, align 4
  store float %extract25.i, float addrspace(3)* %257, align 4
  store float %extract26.i, float addrspace(3)* %258, align 4
  store float %extract27.i, float addrspace(3)* %259, align 4
  store float %extract28.i, float addrspace(3)* %260, align 4
  store float %extract29.i, float addrspace(3)* %261, align 4
  store float %extract30.i, float addrspace(3)* %262, align 4
  store float %extract31.i, float addrspace(3)* %263, align 4
  store float %extract32.i, float addrspace(3)* %264, align 4
  store float %extract33.i, float addrspace(3)* %265, align 4
  store float %extract34.i, float addrspace(3)* %266, align 4
  store float %extract35.i, float addrspace(3)* %267, align 4
  store float %extract36.i, float addrspace(3)* %268, align 4
  store float %extract37.i, float addrspace(3)* %269, align 4
  store float %extract38.i, float addrspace(3)* %270, align 4
  %add84305.i = add nsw <16 x i32> %vectorPHI.i, <i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15>
  %and85306.i = and <16 x i32> %add84305.i, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %idxprom8616307.i = zext <16 x i32> %and85306.i to <16 x i64>
  %extract308.i = extractelement <16 x i64> %idxprom8616307.i, i32 0
  %extract309.i = extractelement <16 x i64> %idxprom8616307.i, i32 1
  %extract310.i = extractelement <16 x i64> %idxprom8616307.i, i32 2
  %extract311.i = extractelement <16 x i64> %idxprom8616307.i, i32 3
  %extract312.i = extractelement <16 x i64> %idxprom8616307.i, i32 4
  %extract313.i = extractelement <16 x i64> %idxprom8616307.i, i32 5
  %extract314.i = extractelement <16 x i64> %idxprom8616307.i, i32 6
  %extract315.i = extractelement <16 x i64> %idxprom8616307.i, i32 7
  %extract316.i = extractelement <16 x i64> %idxprom8616307.i, i32 8
  %extract317.i = extractelement <16 x i64> %idxprom8616307.i, i32 9
  %extract318.i = extractelement <16 x i64> %idxprom8616307.i, i32 10
  %extract319.i = extractelement <16 x i64> %idxprom8616307.i, i32 11
  %extract320.i = extractelement <16 x i64> %idxprom8616307.i, i32 12
  %extract321.i = extractelement <16 x i64> %idxprom8616307.i, i32 13
  %extract322.i = extractelement <16 x i64> %idxprom8616307.i, i32 14
  %extract323.i = extractelement <16 x i64> %idxprom8616307.i, i32 15
  %271 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract308.i
  %272 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract309.i
  %273 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract310.i
  %274 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract311.i
  %275 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract312.i
  %276 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract313.i
  %277 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract314.i
  %278 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract315.i
  %279 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract316.i
  %280 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract317.i
  %281 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract318.i
  %282 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract319.i
  %283 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract320.i
  %284 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract321.i
  %285 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract322.i
  %286 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract323.i
  store float %extract23.i, float addrspace(3)* %271, align 4
  store float %extract24.i, float addrspace(3)* %272, align 4
  store float %extract25.i, float addrspace(3)* %273, align 4
  store float %extract26.i, float addrspace(3)* %274, align 4
  store float %extract27.i, float addrspace(3)* %275, align 4
  store float %extract28.i, float addrspace(3)* %276, align 4
  store float %extract29.i, float addrspace(3)* %277, align 4
  store float %extract30.i, float addrspace(3)* %278, align 4
  store float %extract31.i, float addrspace(3)* %279, align 4
  store float %extract32.i, float addrspace(3)* %280, align 4
  store float %extract33.i, float addrspace(3)* %281, align 4
  store float %extract34.i, float addrspace(3)* %282, align 4
  store float %extract35.i, float addrspace(3)* %283, align 4
  store float %extract36.i, float addrspace(3)* %284, align 4
  store float %extract37.i, float addrspace(3)* %285, align 4
  store float %extract38.i, float addrspace(3)* %286, align 4
  %add88324.i = add nsw <16 x i32> %vectorPHI.i, <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>
  %and89325.i = and <16 x i32> %add88324.i, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %inc.i = add nsw i32 %j.019.i, 1
  %exitcond21.i = icmp eq i32 %inc.i, 3000
  br i1 %exitcond21.i, label %"Barrier BB.i", label %for.body.i

"Barrier BB.i":                                   ; preds = %for.body.i
  %check.WI.iter383.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter383.i, label %thenBB380.i, label %elseBB381.i

thenBB380.i:                                      ; preds = %"Barrier BB.i"
  %"CurrWI++384.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride386.i" = add nuw i64 %CurrSBIndex..0.i, 128
  br label %SyncBB377.i

elseBB381.i:                                      ; preds = %"Barrier BB.i"
  %conv8.i = trunc i64 %30 to i32
  %isDivisorZero.i = icmp eq i32 %conv8.i, 0
  %newiDvisor.i = select i1 %isDivisorZero.i, i32 1, i32 %conv8.i
  %div.i = sdiv i32 4096, %newiDvisor.i
  %cmp9117.i = icmp sgt i32 %div.i, 0
  br label %SyncBB378.i

SyncBB378.i:                                      ; preds = %thenBB.i, %elseBB381.i
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB381.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB381.i ], [ %"CurrWI++.i", %thenBB.i ]
  br i1 %cmp9117.i, label %for.body93.lr.ph.i, label %for.end100.i

for.body93.lr.ph.i:                               ; preds = %SyncBB378.i
  %"&(pSB[currWI].offset)3702.i" = or i64 %CurrSBIndex..1.i, 64
  %"&pSB[currWI].offset371.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)3702.i"
  %CastToValueType372.i = bitcast i8* %"&pSB[currWI].offset371.i" to <16 x i32>*
  %loadedValue373.i = load <16 x i32>* %CastToValueType372.i, align 64
  %287 = extractelement <16 x i32> %loadedValue373.i, i32 0
  %extract327.i = sext i32 %287 to i64
  %288 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract327.i
  %"&pSB[currWI].offset363.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..1.i
  %CastToValueType364.i = bitcast i8* %"&pSB[currWI].offset363.i" to <16 x i32>*
  %loadedValue.i = load <16 x i32>* %CastToValueType364.i, align 64
  %289 = extractelement <16 x i32> %loadedValue.i, i32 0
  %extract344.i = sext i32 %289 to i64
  %290 = getelementptr inbounds float addrspace(1)* %1, i64 %extract344.i
  %ptrTypeCast.i = bitcast float addrspace(3)* %288 to <16 x float> addrspace(3)*
  %ptrTypeCast360.i = bitcast float addrspace(1)* %290 to <16 x float> addrspace(1)*
  br label %for.body93.i

for.body93.i:                                     ; preds = %for.body93.i, %for.body93.lr.ph.i
  %j.118.i = phi i32 [ 0, %for.body93.lr.ph.i ], [ %inc99.i, %for.body93.i ]
  %291 = load <16 x float> addrspace(3)* %ptrTypeCast.i, align 4
  store <16 x float> %291, <16 x float> addrspace(1)* %ptrTypeCast360.i, align 4
  %inc99.i = add nsw i32 %j.118.i, 1
  %exitcond.i = icmp eq i32 %inc99.i, %div.i
  br i1 %exitcond.i, label %for.end100.i, label %for.body93.i

for.end100.i:                                     ; preds = %for.body93.i, %SyncBB378.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..1.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.writeLocalMemory_separated_args.exit

thenBB.i:                                         ; preds = %for.end100.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..1.i, 128
  br label %SyncBB378.i

____Vectorized_.writeLocalMemory_separated_args.exit: ; preds = %for.end100.i
  ret void
}

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}
!cl.noBarrierPath.kernels = !{!2}
!opencl.wrappers = !{!3}

!0 = metadata !{void (float addrspace(1)*, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__writeLocalMemory_separated_args, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3}
!2 = metadata !{}
!3 = metadata !{void (i8*)* @writeLocalMemory}
