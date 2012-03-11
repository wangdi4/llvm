; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@StencilKernel.sh = internal addrspace(3) unnamed_addr global [18 x [18 x double]] zeroinitializer, align 16

declare void @__CopyRect_original(double addrspace(1)* nocapture, i32, i32, double addrspace(1)* nocapture, i32, i32, i32, i32) nounwind

declare i64 @get_group_id(i32) nounwind readnone

declare i64 @get_local_id(i32) nounwind readnone

declare i64 @get_local_size(i32) nounwind readnone

declare void @__StencilKernel_original(double addrspace(1)* nocapture, double addrspace(1)* nocapture, double, double, double) nounwind

declare i64 @get_num_groups(i32) nounwind readnone

declare void @barrier(i64)

declare [7 x i64] @__WG.boundaries.CopyRect_original(double addrspace(1)*, i32, i32, double addrspace(1)*, i32, i32, i32, i32)

declare i64 @get_base_global_id.(i32)

declare void @____Vectorized_.StencilKernel_original(double addrspace(1)* nocapture, double addrspace(1)* nocapture, double, double, double) nounwind

declare i1 @allOne(i1)

declare i1 @allZero(i1)

declare double @masked_load_align8_18(i1, double addrspace(1)*)

declare void @masked_store_align8_18(i1, double, double addrspace(1)*)

declare i1 @allZero_v16(<16 x i1>)

declare i1 @allOne_v16(<16 x i1>)

declare double @masked_load_align8_20(i1, double addrspace(1)*)

declare void @masked_store_align8_20(i1, double, double addrspace(3)*)

declare double @masked_load_align8_21(i1, double addrspace(1)*)

declare void @masked_store_align8_21(i1, double, double addrspace(3)*)

declare double @masked_load_align8_22(i1, double addrspace(1)*)

declare void @masked_store_align16_22(i1, double, double addrspace(3)*)

declare double @masked_load_align8_23(i1, double addrspace(1)*)

declare void @masked_store_align8_23(i1, double, double addrspace(3)*)

declare double @masked_load_align8_24(i1, double addrspace(1)*)

declare void @masked_store_align16_24(i1, double, double addrspace(3)*)

declare double @masked_load_align8_25(i1, double addrspace(1)*)

declare void @masked_store_align16_25(i1, double, double addrspace(3)*)

declare double @masked_load_align8_26(i1, double addrspace(1)*)

declare void @masked_store_align8_26(i1, double, double addrspace(3)*)

declare double @masked_load_align8_27(i1, double addrspace(1)*)

declare void @masked_store_align8_27(i1, double, double addrspace(3)*)

declare <16 x double> @masked_load_align8_28(i1, <16 x double> addrspace(1)*)

declare void @masked_store_align8_28(i1, <16 x double>, <16 x double> addrspace(3)*)

declare <16 x double> @masked_load_align8_29(i1, <16 x double> addrspace(1)*)

declare void @masked_store_align8_29(i1, <16 x double>, <16 x double> addrspace(3)*)

declare <16 x double> @masked_load_align8_30(<16 x i1>, <16 x double> addrspace(1)*)

declare <16 x double> @masked_load_align8_31(<16 x i1>, <16 x double> addrspace(1)*)

declare <16 x double> @masked_load_align8_32(<16 x i1>, <16 x double> addrspace(1)*)

declare <16 x double> @masked_load_align8_33(<16 x i1>, <16 x double> addrspace(1)*)

declare <16 x double> @masked_load_align8_34(<16 x i1>, <16 x double> addrspace(1)*)

declare <16 x double> @masked_load_align8_35(<16 x i1>, <16 x double> addrspace(1)*)

declare void @dummybarrier.()

declare i8* @get_special_buffer.() nounwind readnone

declare i64 @get_iter_count.() nounwind readnone

declare i64 @get_new_local_id.(i32, i64) nounwind readnone

declare void @__CopyRect_separated_args(double addrspace(1)* nocapture, i32, i32, double addrspace(1)* nocapture, i32, i32, i32, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare void @__StencilKernel_separated_args(double addrspace(1)* nocapture, double addrspace(1)* nocapture, double, double, double, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare [7 x i64] @WG.boundaries.CopyRect(double addrspace(1)*, i32, i32, double addrspace(1)*, i32, i32, i32, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)

declare void @____Vectorized_.StencilKernel_separated_args(double addrspace(1)* nocapture, double addrspace(1)* nocapture, double, double, double, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare i32 @llvm.x86.mic.kortestc(i16, i16) nounwind readnone

declare i32 @llvm.x86.mic.kortestz(i16, i16) nounwind readnone

define void @StencilKernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to double*
  %7 = load double* %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to double*
  %10 = load double* %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to double*
  %13 = load double* %12, align 8
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
  %32 = bitcast i8 addrspace(3)* %16 to [18 x [18 x double]] addrspace(3)*
  %33 = bitcast i8 addrspace(3)* %16 to double addrspace(3)*
  %34 = getelementptr i8 addrspace(3)* %16, i64 2448
  %35 = bitcast i8 addrspace(3)* %34 to double addrspace(3)*
  %36 = getelementptr i8 addrspace(3)* %16, i64 136
  %37 = bitcast i8 addrspace(3)* %36 to double addrspace(3)*
  %38 = getelementptr i8 addrspace(3)* %16, i64 2584
  %39 = bitcast i8 addrspace(3)* %38 to double addrspace(3)*
  br label %SyncBB148.i

SyncBB148.i:                                      ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %40 = getelementptr i64* %22, i64 1
  %41 = load i64* %40, align 8
  %conv.i = trunc i64 %41 to i32
  %42 = load i64* %22, align 8
  %conv2.i = trunc i64 %42 to i32
  %43 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 4, i64 0
  %44 = load i64* %43, align 8
  %conv6.i = trunc i64 %44 to i32
  %45 = getelementptr <{ [4 x i64] }>* %25, i64 %CurrWI..0.i, i32 0, i64 1
  %46 = load i64* %45, align 8
  %conv8.i = trunc i64 %46 to i32
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %31, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %conv8.i, i32* %CastToValueType.i, align 4
  %47 = getelementptr <{ [4 x i64] }>* %25, i64 %CurrWI..0.i, i32 0, i64 0
  %48 = load i64* %47, align 8
  %conv10.i = trunc i64 %48 to i32
  %"&(pSB[currWI].offset)641.i" = or i64 %CurrSBIndex..0.i, 4
  %"&pSB[currWI].offset65.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)641.i"
  %CastToValueType66.i = bitcast i8* %"&pSB[currWI].offset65.i" to i32*
  store i32 %conv10.i, i32* %CastToValueType66.i, align 4
  %mul.i.i = shl i32 %conv.i, 4
  %add.i.i = add nsw i32 %mul.i.i, %conv8.i
  %mul.i43.i = shl i32 %conv2.i, 4
  %add.i44.i = add nsw i32 %mul.i43.i, %conv10.i
  %mul.i = shl nsw i32 %conv6.i, 4
  %add.i38.i = add nsw i32 %add.i.i, 1
  %add1.i3945.i = or i32 %mul.i, 2
  %mul.i40.i = mul nsw i32 %add1.i3945.i, %add.i38.i
  %add2.i41.i = add i32 %add.i44.i, 1
  %add3.i42.i = add i32 %add2.i41.i, %mul.i40.i
  %idxprom.i = sext i32 %add3.i42.i to i64
  %"&(pSB[currWI].offset)83.i" = add nuw i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset84.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)83.i"
  %CastToValueType85.i = bitcast i8* %"&pSB[currWI].offset84.i" to i64*
  store i64 %idxprom.i, i64* %CastToValueType85.i, align 8
  %arrayidx.i = getelementptr inbounds double addrspace(1)* %1, i64 %idxprom.i
  %49 = load double addrspace(1)* %arrayidx.i, align 8
  %add.i = add nsw i32 %conv10.i, 1
  %idxprom14.i = sext i32 %add.i to i64
  %"&(pSB[currWI].offset)92.i" = add nuw i64 %CurrSBIndex..0.i, 16
  %"&pSB[currWI].offset93.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)92.i"
  %CastToValueType94.i = bitcast i8* %"&pSB[currWI].offset93.i" to i64*
  store i64 %idxprom14.i, i64* %CastToValueType94.i, align 8
  %add15.i = add nsw i32 %conv8.i, 1
  %idxprom16.i = sext i32 %add15.i to i64
  %"&(pSB[currWI].offset)116.i" = add nuw i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset117.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)116.i"
  %CastToValueType118.i = bitcast i8* %"&pSB[currWI].offset117.i" to i64*
  store i64 %idxprom16.i, i64* %CastToValueType118.i, align 8
  %arrayidx18.i = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %idxprom16.i, i64 %idxprom14.i
  %"&(pSB[currWI].offset)140.i" = add nuw i64 %CurrSBIndex..0.i, 32
  %"&pSB[currWI].offset141.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)140.i"
  %CastToValueType142.i = bitcast i8* %"&pSB[currWI].offset141.i" to double addrspace(3)**
  store double addrspace(3)* %arrayidx18.i, double addrspace(3)** %CastToValueType142.i, align 8
  store double %49, double addrspace(3)* %arrayidx18.i, align 8
  %cmp.i = icmp eq i32 %conv8.i, 0
  br i1 %cmp.i, label %if.then.i, label %if.end.i

if.then.i:                                        ; preds = %SyncBB148.i
  %mul.i35.i = mul nsw i32 %add1.i3945.i, %mul.i.i
  %add3.i37.i = add i32 %add2.i41.i, %mul.i35.i
  %idxprom21.i = sext i32 %add3.i37.i to i64
  %arrayidx22.i = getelementptr inbounds double addrspace(1)* %1, i64 %idxprom21.i
  %50 = load double addrspace(1)* %arrayidx22.i, align 8
  %loadedValue114.i = load i64* %CastToValueType94.i, align 8
  %arrayidx25.i = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 0, i64 %loadedValue114.i
  store double %50, double addrspace(3)* %arrayidx25.i, align 8
  %add.i28.i = add nsw i32 %add.i.i, 17
  %mul.i30.i = mul nsw i32 %add1.i3945.i, %add.i28.i
  %add3.i32.i = add i32 %add2.i41.i, %mul.i30.i
  %idxprom28.i = sext i32 %add3.i32.i to i64
  %arrayidx29.i = getelementptr inbounds double addrspace(1)* %1, i64 %idxprom28.i
  %51 = load double addrspace(1)* %arrayidx29.i, align 8
  %loadedValue109.i = load i64* %CastToValueType94.i, align 8
  %arrayidx32.i = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 17, i64 %loadedValue109.i
  store double %51, double addrspace(3)* %arrayidx32.i, align 8
  br label %if.end.i

if.end.i:                                         ; preds = %if.then.i, %SyncBB148.i
  %loadedValue76.i = load i32* %CastToValueType66.i, align 4
  %cmp33.i = icmp eq i32 %loadedValue76.i, 0
  br i1 %cmp33.i, label %if.then35.i, label %if.end52.i

if.then35.i:                                      ; preds = %if.end.i
  %add3.i27.i = add i32 %mul.i43.i, %mul.i40.i
  %idxprom38.i = sext i32 %add3.i27.i to i64
  %arrayidx39.i = getelementptr inbounds double addrspace(1)* %1, i64 %idxprom38.i
  %52 = load double addrspace(1)* %arrayidx39.i, align 8
  %loadedValue138.i = load i64* %CastToValueType118.i, align 8
  %arrayidx43.i = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %loadedValue138.i, i64 0
  store double %52, double addrspace(3)* %arrayidx43.i, align 16
  %add2.i21.i = add i32 %add.i44.i, 17
  %add3.i22.i = add i32 %add2.i21.i, %mul.i40.i
  %idxprom46.i = sext i32 %add3.i22.i to i64
  %arrayidx47.i = getelementptr inbounds double addrspace(1)* %1, i64 %idxprom46.i
  %53 = load double addrspace(1)* %arrayidx47.i, align 8
  %loadedValue133.i = load i64* %CastToValueType118.i, align 8
  %arrayidx51.i = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %loadedValue133.i, i64 17
  store double %53, double addrspace(3)* %arrayidx51.i, align 8
  br label %if.end52.i

if.end52.i:                                       ; preds = %if.then35.i, %if.end.i
  %54 = or i64 %48, %46
  %55 = trunc i64 %54 to i32
  %56 = icmp eq i32 %55, 0
  br i1 %56, label %if.then57.i, label %if.end78.i

if.then57.i:                                      ; preds = %if.end52.i
  %mul.i15.i = mul nsw i32 %add1.i3945.i, %add.i.i
  %add3.i17.i = add i32 %add.i44.i, %mul.i15.i
  %idxprom61.i = sext i32 %add3.i17.i to i64
  %arrayidx62.i = getelementptr inbounds double addrspace(1)* %1, i64 %idxprom61.i
  %57 = load double addrspace(1)* %arrayidx62.i, align 8
  store double %57, double addrspace(3)* %33, align 16
  %add.i8.i = add nsw i32 %add.i.i, 17
  %mul.i10.i = mul nsw i32 %add1.i3945.i, %add.i8.i
  %add3.i12.i = add i32 %add.i44.i, %mul.i10.i
  %idxprom66.i = sext i32 %add3.i12.i to i64
  %arrayidx67.i = getelementptr inbounds double addrspace(1)* %1, i64 %idxprom66.i
  %58 = load double addrspace(1)* %arrayidx67.i, align 8
  store double %58, double addrspace(3)* %35, align 16
  %add2.i6.i = add i32 %add.i44.i, 17
  %add3.i7.i = add i32 %add2.i6.i, %mul.i15.i
  %idxprom71.i = sext i32 %add3.i7.i to i64
  %arrayidx72.i = getelementptr inbounds double addrspace(1)* %1, i64 %idxprom71.i
  %59 = load double addrspace(1)* %arrayidx72.i, align 8
  store double %59, double addrspace(3)* %37, align 8
  %add3.i.i = add i32 %add2.i6.i, %mul.i10.i
  %idxprom76.i = sext i32 %add3.i.i to i64
  %arrayidx77.i = getelementptr inbounds double addrspace(1)* %1, i64 %idxprom76.i
  %60 = load double addrspace(1)* %arrayidx77.i, align 8
  store double %60, double addrspace(3)* %39, align 8
  br label %if.end78.i

if.end78.i:                                       ; preds = %if.then57.i, %if.end52.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %28
  br i1 %check.WI.iter.i, label %thenBB.i, label %if.end78.i.SyncBB.i_crit_edge

if.end78.i.SyncBB.i_crit_edge:                    ; preds = %if.end78.i
  br label %SyncBB.i

thenBB.i:                                         ; preds = %if.end78.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 40
  br label %SyncBB148.i

SyncBB.i:                                         ; preds = %if.end78.i.SyncBB.i_crit_edge, %thenBB151.i
  %CurrWI..1.i = phi i64 [ %"CurrWI++155.i", %thenBB151.i ], [ 0, %if.end78.i.SyncBB.i_crit_edge ]
  %CurrSBIndex..1.i = phi i64 [ %"loadedCurrSB+Stride157.i", %thenBB151.i ], [ 0, %if.end78.i.SyncBB.i_crit_edge ]
  %"&(pSB[currWI].offset)144.i" = add nuw i64 %CurrSBIndex..1.i, 32
  %"&pSB[currWI].offset145.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)144.i"
  %CastToValueType146.i = bitcast i8* %"&pSB[currWI].offset145.i" to double addrspace(3)**
  %loadedValue147.i = load double addrspace(3)** %CastToValueType146.i, align 8
  %61 = load double addrspace(3)* %loadedValue147.i, align 8
  %"&pSB[currWI].offset60.i" = getelementptr inbounds i8* %31, i64 %CurrSBIndex..1.i
  %CastToValueType61.i = bitcast i8* %"&pSB[currWI].offset60.i" to i32*
  %loadedValue62.i = load i32* %CastToValueType61.i, align 4
  %idxprom87.i = sext i32 %loadedValue62.i to i64
  %"&(pSB[currWI].offset)101.i" = add nuw i64 %CurrSBIndex..1.i, 16
  %"&pSB[currWI].offset102.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)101.i"
  %CastToValueType103.i = bitcast i8* %"&pSB[currWI].offset102.i" to i64*
  %loadedValue104.i = load i64* %CastToValueType103.i, align 8
  %arrayidx89.i = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %idxprom87.i, i64 %loadedValue104.i
  %62 = load double addrspace(3)* %arrayidx89.i, align 8
  %add92.i = add nsw i32 %loadedValue62.i, 2
  %idxprom93.i = sext i32 %add92.i to i64
  %arrayidx95.i = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %idxprom93.i, i64 %loadedValue104.i
  %63 = load double addrspace(3)* %arrayidx95.i, align 8
  %add96.i = fadd double %62, %63
  %"&(pSB[currWI].offset)783.i" = or i64 %CurrSBIndex..1.i, 4
  %"&pSB[currWI].offset79.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)783.i"
  %CastToValueType80.i = bitcast i8* %"&pSB[currWI].offset79.i" to i32*
  %loadedValue81.i = load i32* %CastToValueType80.i, align 4
  %idxprom97.i = sext i32 %loadedValue81.i to i64
  %"&(pSB[currWI].offset)125.i" = add nuw i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset126.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)125.i"
  %CastToValueType127.i = bitcast i8* %"&pSB[currWI].offset126.i" to i64*
  %loadedValue128.i = load i64* %CastToValueType127.i, align 8
  %arrayidx101.i = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %loadedValue128.i, i64 %idxprom97.i
  %64 = load double addrspace(3)* %arrayidx101.i, align 8
  %add102.i = fadd double %add96.i, %64
  %add103.i = add nsw i32 %loadedValue81.i, 2
  %idxprom104.i = sext i32 %add103.i to i64
  %arrayidx108.i = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %loadedValue128.i, i64 %idxprom104.i
  %65 = load double addrspace(3)* %arrayidx108.i, align 8
  %add109.i = fadd double %add102.i, %65
  %arrayidx113.i = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %idxprom87.i, i64 %idxprom97.i
  %66 = load double addrspace(3)* %arrayidx113.i, align 8
  %arrayidx118.i = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %idxprom87.i, i64 %idxprom104.i
  %67 = load double addrspace(3)* %arrayidx118.i, align 8
  %add119.i = fadd double %66, %67
  %arrayidx124.i = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %idxprom93.i, i64 %idxprom97.i
  %68 = load double addrspace(3)* %arrayidx124.i, align 8
  %add125.i = fadd double %add119.i, %68
  %arrayidx131.i = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %idxprom93.i, i64 %idxprom104.i
  %69 = load double addrspace(3)* %arrayidx131.i, align 8
  %add132.i = fadd double %add125.i, %69
  %mul133.i = fmul double %61, %7
  %mul134.i = fmul double %add109.i, %10
  %add135.i = fadd double %mul133.i, %mul134.i
  %mul136.i = fmul double %add132.i, %13
  %add137.i = fadd double %add135.i, %mul136.i
  %"&(pSB[currWI].offset)87.i" = add nuw i64 %CurrSBIndex..1.i, 8
  %"&pSB[currWI].offset88.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)87.i"
  %CastToValueType89.i = bitcast i8* %"&pSB[currWI].offset88.i" to i64*
  %loadedValue90.i = load i64* %CastToValueType89.i, align 8
  %arrayidx139.i = getelementptr inbounds double addrspace(1)* %4, i64 %loadedValue90.i
  store double %add137.i, double addrspace(1)* %arrayidx139.i, align 8
  %check.WI.iter154.i = icmp ult i64 %CurrWI..1.i, %28
  br i1 %check.WI.iter154.i, label %thenBB151.i, label %__StencilKernel_separated_args.exit

thenBB151.i:                                      ; preds = %SyncBB.i
  %"CurrWI++155.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride157.i" = add nuw i64 %CurrSBIndex..1.i, 40
  br label %SyncBB.i

__StencilKernel_separated_args.exit:              ; preds = %SyncBB.i
  ret void
}

define void @CopyRect(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32*
  %4 = load i32* %3, align 4
  %5 = getelementptr i8* %pBuffer, i64 12
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 16
  %9 = bitcast i8* %8 to double addrspace(1)**
  %10 = load double addrspace(1)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 24
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 28
  %15 = bitcast i8* %14 to i32*
  %16 = load i32* %15, align 4
  %17 = getelementptr i8* %pBuffer, i64 32
  %18 = bitcast i8* %17 to i32*
  %19 = load i32* %18, align 4
  %20 = getelementptr i8* %pBuffer, i64 36
  %21 = bitcast i8* %20 to i32*
  %22 = load i32* %21, align 4
  %23 = getelementptr i8* %pBuffer, i64 48
  %24 = bitcast i8* %23 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %25 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 56
  %27 = bitcast i8* %26 to i64**
  %28 = load i64** %27, align 8
  %29 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %25, i64 0, i32 3, i64 0
  %30 = load i64* %29, align 8
  %31 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %25, i64 0, i32 3, i64 1
  %32 = load i64* %31, align 8
  %33 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %25, i64 0, i32 3, i64 2
  %34 = load i64* %33, align 8
  %vector.size.i = ashr i64 %30, 4
  %num.vector.wi.i = and i64 %30, -16
  %scalar.size.i = sub i64 %30, %num.vector.wi.i
  %35 = icmp eq i64 %vector.size.i, 0
  br i1 %35, label %scalarIf.i, label %dim_2_vector_pre_head.i

dim_2_vector_pre_head.i:                          ; preds = %entry
  %temp59vector_func.i = insertelement <16 x i32> undef, i32 %7, i32 0
  %vector60vector_func.i = shufflevector <16 x i32> %temp59vector_func.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %temp18vector_func.i = insertelement <16 x i32> undef, i32 %16, i32 0
  %vector19vector_func.i = shufflevector <16 x i32> %temp18vector_func.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %temp9vector_func.i = insertelement <16 x i32> undef, i32 %22, i32 0
  %vector10vector_func.i = shufflevector <16 x i32> %temp9vector_func.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %cmp81vector_func.i = icmp sgt i32 %19, 0
  %temp11vector_func.i = insertelement <16 x i1> undef, i1 %cmp81vector_func.i, i32 0
  %vector12vector_func.i = shufflevector <16 x i1> %temp11vector_func.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %idx.extvector_func.i = sext i32 %13 to i64
  %temp25vector_func.i = insertelement <16 x i64> undef, i64 %idx.extvector_func.i, i32 0
  %vector26vector_func.i = shufflevector <16 x i64> %temp25vector_func.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %idx.ext13vector_func.i = sext i32 %4 to i64
  %temp64vector_func.i = insertelement <16 x i64> undef, i64 %idx.ext13vector_func.i, i32 0
  %vector65vector_func.i = shufflevector <16 x i64> %temp64vector_func.i, <16 x i64> undef, <16 x i32> zeroinitializer
  br label %dim_1_vector_pre_head.i

dim_1_vector_pre_head.i:                          ; preds = %dim_1_vector_exit.i, %dim_2_vector_pre_head.i
  %dim_2_vector_ind_var.i = phi i64 [ 0, %dim_2_vector_pre_head.i ], [ %dim_2_vector_inc_ind_var.i, %dim_1_vector_exit.i ]
  br label %dim_0_vector_pre_head.i

dim_0_vector_pre_head.i:                          ; preds = %dim_0_vector_exit.i, %dim_1_vector_pre_head.i
  %dim_1_vector_ind_var.i = phi i64 [ 0, %dim_1_vector_pre_head.i ], [ %dim_1_vector_inc_ind_var.i, %dim_0_vector_exit.i ]
  br label %entryvector_func.i

entryvector_func.i:                               ; preds = %if.endvector_func.i.entryvector_func.i_crit_edge, %dim_0_vector_pre_head.i
  %dim_0_vector_ind_var.i = phi i64 [ 0, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_ind_var.i, %if.endvector_func.i.entryvector_func.i_crit_edge ]
  %dim_0_vector_tid.i = phi i64 [ 0, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_tid.i, %if.endvector_func.i.entryvector_func.i_crit_edge ]
  %36 = load i64* %28, align 8
  %convvector_func.i = trunc i64 %36 to i32
  %broadcast1vector_func.i = insertelement <16 x i64> undef, i64 %dim_0_vector_tid.i, i32 0
  %broadcast2vector_func.i = shufflevector <16 x i64> %broadcast1vector_func.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %37 = add <16 x i64> %broadcast2vector_func.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %conv27vector_func.i = trunc <16 x i64> %37 to <16 x i32>
  %38 = load i64* %29, align 8
  %conv6vector_func.i = trunc i64 %38 to i32
  %mulvector_func.i = mul nsw i32 %conv6vector_func.i, %convvector_func.i
  %tempvector_func.i = insertelement <16 x i32> undef, i32 %mulvector_func.i, i32 0
  %vectorvector_func.i = shufflevector <16 x i32> %tempvector_func.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %add8vector_func.i = add nsw <16 x i32> %vectorvector_func.i, %conv27vector_func.i
  %cmpvector_func.i = icmp slt <16 x i32> %add8vector_func.i, %vector10vector_func.i
  %entry_to_for.body.lr.ph14vector_func.i = and <16 x i1> %cmpvector_func.i, %vector12vector_func.i
  %ipred.i1.i = bitcast <16 x i1> %entry_to_for.body.lr.ph14vector_func.i to i16
  %val.i2.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i1.i, i16 %ipred.i1.i) nounwind
  %39 = and i32 %val.i2.i, 1
  %res.i3.i = icmp eq i32 %39, 0
  br i1 %res.i3.i, label %for.bodyvector_func.preheader.i, label %if.endvector_func.i

for.bodyvector_func.preheader.i:                  ; preds = %entryvector_func.i
  %negIncomingLoopMask15vector_func.i = xor <16 x i1> %entry_to_for.body.lr.ph14vector_func.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %mul.i20vector_func.i = mul nsw <16 x i32> %add8vector_func.i, %vector19vector_func.i
  %mul.i161vector_func.i = mul nsw <16 x i32> %add8vector_func.i, %vector60vector_func.i
  br label %for.bodyvector_func.i

for.bodyvector_func.i:                            ; preds = %postload183vector_func.i, %for.bodyvector_func.preheader.i
  %vectorPHIvector_func.i = phi <16 x i1> [ %loop_mask387vector_func.i, %postload183vector_func.i ], [ %negIncomingLoopMask15vector_func.i, %for.bodyvector_func.preheader.i ]
  %vectorPHI17vector_func.i = phi <16 x i1> [ %local_edge91vector_func.i, %postload183vector_func.i ], [ %entry_to_for.body.lr.ph14vector_func.i, %for.bodyvector_func.preheader.i ]
  %c.02vector_func.i = phi i32 [ %incvector_func.i, %postload183vector_func.i ], [ 0, %for.bodyvector_func.preheader.i ]
  %extract43vector_func.i = extractelement <16 x i1> %vectorPHI17vector_func.i, i32 0
  %extract44vector_func.i = extractelement <16 x i1> %vectorPHI17vector_func.i, i32 1
  %extract45vector_func.i = extractelement <16 x i1> %vectorPHI17vector_func.i, i32 2
  %extract46vector_func.i = extractelement <16 x i1> %vectorPHI17vector_func.i, i32 3
  %extract47vector_func.i = extractelement <16 x i1> %vectorPHI17vector_func.i, i32 4
  %extract48vector_func.i = extractelement <16 x i1> %vectorPHI17vector_func.i, i32 5
  %extract49vector_func.i = extractelement <16 x i1> %vectorPHI17vector_func.i, i32 6
  %extract50vector_func.i = extractelement <16 x i1> %vectorPHI17vector_func.i, i32 7
  %extract51vector_func.i = extractelement <16 x i1> %vectorPHI17vector_func.i, i32 8
  %extract52vector_func.i = extractelement <16 x i1> %vectorPHI17vector_func.i, i32 9
  %extract53vector_func.i = extractelement <16 x i1> %vectorPHI17vector_func.i, i32 10
  %extract54vector_func.i = extractelement <16 x i1> %vectorPHI17vector_func.i, i32 11
  %extract55vector_func.i = extractelement <16 x i1> %vectorPHI17vector_func.i, i32 12
  %extract56vector_func.i = extractelement <16 x i1> %vectorPHI17vector_func.i, i32 13
  %extract57vector_func.i = extractelement <16 x i1> %vectorPHI17vector_func.i, i32 14
  %extract58vector_func.i = extractelement <16 x i1> %vectorPHI17vector_func.i, i32 15
  %temp21vector_func.i = insertelement <16 x i32> undef, i32 %c.02vector_func.i, i32 0
  %vector22vector_func.i = shufflevector <16 x i32> %temp21vector_func.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %add.i23vector_func.i = add nsw <16 x i32> %mul.i20vector_func.i, %vector22vector_func.i
  %idxprom24vector_func.i = sext <16 x i32> %add.i23vector_func.i to <16 x i64>
  %add.ptr.sum27vector_func.i = add <16 x i64> %idxprom24vector_func.i, %vector26vector_func.i
  %extract28vector_func.i = extractelement <16 x i64> %add.ptr.sum27vector_func.i, i32 1
  %extract29vector_func.i = extractelement <16 x i64> %add.ptr.sum27vector_func.i, i32 2
  %extract30vector_func.i = extractelement <16 x i64> %add.ptr.sum27vector_func.i, i32 3
  %extract31vector_func.i = extractelement <16 x i64> %add.ptr.sum27vector_func.i, i32 4
  %extract32vector_func.i = extractelement <16 x i64> %add.ptr.sum27vector_func.i, i32 5
  %extract33vector_func.i = extractelement <16 x i64> %add.ptr.sum27vector_func.i, i32 6
  %extract34vector_func.i = extractelement <16 x i64> %add.ptr.sum27vector_func.i, i32 7
  %extract35vector_func.i = extractelement <16 x i64> %add.ptr.sum27vector_func.i, i32 8
  %extract36vector_func.i = extractelement <16 x i64> %add.ptr.sum27vector_func.i, i32 9
  %extract37vector_func.i = extractelement <16 x i64> %add.ptr.sum27vector_func.i, i32 10
  %extract38vector_func.i = extractelement <16 x i64> %add.ptr.sum27vector_func.i, i32 11
  %extract39vector_func.i = extractelement <16 x i64> %add.ptr.sum27vector_func.i, i32 12
  %extract40vector_func.i = extractelement <16 x i64> %add.ptr.sum27vector_func.i, i32 13
  %extract41vector_func.i = extractelement <16 x i64> %add.ptr.sum27vector_func.i, i32 14
  %extract42vector_func.i = extractelement <16 x i64> %add.ptr.sum27vector_func.i, i32 15
  %40 = getelementptr inbounds double addrspace(1)* %10, i64 %extract28vector_func.i
  %41 = getelementptr inbounds double addrspace(1)* %10, i64 %extract29vector_func.i
  %42 = getelementptr inbounds double addrspace(1)* %10, i64 %extract30vector_func.i
  %43 = getelementptr inbounds double addrspace(1)* %10, i64 %extract31vector_func.i
  %44 = getelementptr inbounds double addrspace(1)* %10, i64 %extract32vector_func.i
  %45 = getelementptr inbounds double addrspace(1)* %10, i64 %extract33vector_func.i
  %46 = getelementptr inbounds double addrspace(1)* %10, i64 %extract34vector_func.i
  %47 = getelementptr inbounds double addrspace(1)* %10, i64 %extract35vector_func.i
  %48 = getelementptr inbounds double addrspace(1)* %10, i64 %extract36vector_func.i
  %49 = getelementptr inbounds double addrspace(1)* %10, i64 %extract37vector_func.i
  %50 = getelementptr inbounds double addrspace(1)* %10, i64 %extract38vector_func.i
  %51 = getelementptr inbounds double addrspace(1)* %10, i64 %extract39vector_func.i
  %52 = getelementptr inbounds double addrspace(1)* %10, i64 %extract40vector_func.i
  %53 = getelementptr inbounds double addrspace(1)* %10, i64 %extract41vector_func.i
  %54 = getelementptr inbounds double addrspace(1)* %10, i64 %extract42vector_func.i
  br i1 %extract43vector_func.i, label %preloadvector_func.i, label %postloadvector_func.i

preloadvector_func.i:                             ; preds = %for.bodyvector_func.i
  %extractvector_func.i = extractelement <16 x i64> %add.ptr.sum27vector_func.i, i32 0
  %55 = getelementptr inbounds double addrspace(1)* %10, i64 %extractvector_func.i
  %masked_loadvector_func.i = load double addrspace(1)* %55, align 8
  br label %postloadvector_func.i

postloadvector_func.i:                            ; preds = %preloadvector_func.i, %for.bodyvector_func.i
  %phivector_func.i = phi double [ undef, %for.bodyvector_func.i ], [ %masked_loadvector_func.i, %preloadvector_func.i ]
  br i1 %extract44vector_func.i, label %preload109vector_func.i, label %postload110vector_func.i

preload109vector_func.i:                          ; preds = %postloadvector_func.i
  %masked_load92vector_func.i = load double addrspace(1)* %40, align 8
  br label %postload110vector_func.i

postload110vector_func.i:                         ; preds = %preload109vector_func.i, %postloadvector_func.i
  %phi111vector_func.i = phi double [ undef, %postloadvector_func.i ], [ %masked_load92vector_func.i, %preload109vector_func.i ]
  br i1 %extract45vector_func.i, label %preload114vector_func.i, label %postload115vector_func.i

preload114vector_func.i:                          ; preds = %postload110vector_func.i
  %masked_load93vector_func.i = load double addrspace(1)* %41, align 8
  br label %postload115vector_func.i

postload115vector_func.i:                         ; preds = %preload114vector_func.i, %postload110vector_func.i
  %phi116vector_func.i = phi double [ undef, %postload110vector_func.i ], [ %masked_load93vector_func.i, %preload114vector_func.i ]
  br i1 %extract46vector_func.i, label %preload119vector_func.i, label %postload120vector_func.i

preload119vector_func.i:                          ; preds = %postload115vector_func.i
  %masked_load94vector_func.i = load double addrspace(1)* %42, align 8
  br label %postload120vector_func.i

postload120vector_func.i:                         ; preds = %preload119vector_func.i, %postload115vector_func.i
  %phi121vector_func.i = phi double [ undef, %postload115vector_func.i ], [ %masked_load94vector_func.i, %preload119vector_func.i ]
  br i1 %extract47vector_func.i, label %preload124vector_func.i, label %postload125vector_func.i

preload124vector_func.i:                          ; preds = %postload120vector_func.i
  %masked_load95vector_func.i = load double addrspace(1)* %43, align 8
  br label %postload125vector_func.i

postload125vector_func.i:                         ; preds = %preload124vector_func.i, %postload120vector_func.i
  %phi126vector_func.i = phi double [ undef, %postload120vector_func.i ], [ %masked_load95vector_func.i, %preload124vector_func.i ]
  br i1 %extract48vector_func.i, label %preload129vector_func.i, label %postload130vector_func.i

preload129vector_func.i:                          ; preds = %postload125vector_func.i
  %masked_load96vector_func.i = load double addrspace(1)* %44, align 8
  br label %postload130vector_func.i

postload130vector_func.i:                         ; preds = %preload129vector_func.i, %postload125vector_func.i
  %phi131vector_func.i = phi double [ undef, %postload125vector_func.i ], [ %masked_load96vector_func.i, %preload129vector_func.i ]
  br i1 %extract49vector_func.i, label %preload134vector_func.i, label %postload135vector_func.i

preload134vector_func.i:                          ; preds = %postload130vector_func.i
  %masked_load97vector_func.i = load double addrspace(1)* %45, align 8
  br label %postload135vector_func.i

postload135vector_func.i:                         ; preds = %preload134vector_func.i, %postload130vector_func.i
  %phi136vector_func.i = phi double [ undef, %postload130vector_func.i ], [ %masked_load97vector_func.i, %preload134vector_func.i ]
  br i1 %extract50vector_func.i, label %preload139vector_func.i, label %postload140vector_func.i

preload139vector_func.i:                          ; preds = %postload135vector_func.i
  %masked_load98vector_func.i = load double addrspace(1)* %46, align 8
  br label %postload140vector_func.i

postload140vector_func.i:                         ; preds = %preload139vector_func.i, %postload135vector_func.i
  %phi141vector_func.i = phi double [ undef, %postload135vector_func.i ], [ %masked_load98vector_func.i, %preload139vector_func.i ]
  br i1 %extract51vector_func.i, label %preload144vector_func.i, label %postload145vector_func.i

preload144vector_func.i:                          ; preds = %postload140vector_func.i
  %masked_load99vector_func.i = load double addrspace(1)* %47, align 8
  br label %postload145vector_func.i

postload145vector_func.i:                         ; preds = %preload144vector_func.i, %postload140vector_func.i
  %phi146vector_func.i = phi double [ undef, %postload140vector_func.i ], [ %masked_load99vector_func.i, %preload144vector_func.i ]
  br i1 %extract52vector_func.i, label %preload149vector_func.i, label %postload150vector_func.i

preload149vector_func.i:                          ; preds = %postload145vector_func.i
  %masked_load100vector_func.i = load double addrspace(1)* %48, align 8
  br label %postload150vector_func.i

postload150vector_func.i:                         ; preds = %preload149vector_func.i, %postload145vector_func.i
  %phi151vector_func.i = phi double [ undef, %postload145vector_func.i ], [ %masked_load100vector_func.i, %preload149vector_func.i ]
  br i1 %extract53vector_func.i, label %preload154vector_func.i, label %postload155vector_func.i

preload154vector_func.i:                          ; preds = %postload150vector_func.i
  %masked_load101vector_func.i = load double addrspace(1)* %49, align 8
  br label %postload155vector_func.i

postload155vector_func.i:                         ; preds = %preload154vector_func.i, %postload150vector_func.i
  %phi156vector_func.i = phi double [ undef, %postload150vector_func.i ], [ %masked_load101vector_func.i, %preload154vector_func.i ]
  br i1 %extract54vector_func.i, label %preload159vector_func.i, label %postload160vector_func.i

preload159vector_func.i:                          ; preds = %postload155vector_func.i
  %masked_load102vector_func.i = load double addrspace(1)* %50, align 8
  br label %postload160vector_func.i

postload160vector_func.i:                         ; preds = %preload159vector_func.i, %postload155vector_func.i
  %phi161vector_func.i = phi double [ undef, %postload155vector_func.i ], [ %masked_load102vector_func.i, %preload159vector_func.i ]
  br i1 %extract55vector_func.i, label %preload164vector_func.i, label %postload165vector_func.i

preload164vector_func.i:                          ; preds = %postload160vector_func.i
  %masked_load103vector_func.i = load double addrspace(1)* %51, align 8
  br label %postload165vector_func.i

postload165vector_func.i:                         ; preds = %preload164vector_func.i, %postload160vector_func.i
  %phi166vector_func.i = phi double [ undef, %postload160vector_func.i ], [ %masked_load103vector_func.i, %preload164vector_func.i ]
  br i1 %extract56vector_func.i, label %preload169vector_func.i, label %postload170vector_func.i

preload169vector_func.i:                          ; preds = %postload165vector_func.i
  %masked_load104vector_func.i = load double addrspace(1)* %52, align 8
  br label %postload170vector_func.i

postload170vector_func.i:                         ; preds = %preload169vector_func.i, %postload165vector_func.i
  %phi171vector_func.i = phi double [ undef, %postload165vector_func.i ], [ %masked_load104vector_func.i, %preload169vector_func.i ]
  br i1 %extract57vector_func.i, label %preload174vector_func.i, label %postload175vector_func.i

preload174vector_func.i:                          ; preds = %postload170vector_func.i
  %masked_load105vector_func.i = load double addrspace(1)* %53, align 8
  br label %postload175vector_func.i

postload175vector_func.i:                         ; preds = %preload174vector_func.i, %postload170vector_func.i
  %phi176vector_func.i = phi double [ undef, %postload170vector_func.i ], [ %masked_load105vector_func.i, %preload174vector_func.i ]
  br i1 %extract58vector_func.i, label %preload179vector_func.i, label %postload180vector_func.i

preload179vector_func.i:                          ; preds = %postload175vector_func.i
  %masked_load106vector_func.i = load double addrspace(1)* %54, align 8
  br label %postload180vector_func.i

postload180vector_func.i:                         ; preds = %preload179vector_func.i, %postload175vector_func.i
  %phi181vector_func.i = phi double [ undef, %postload175vector_func.i ], [ %masked_load106vector_func.i, %preload179vector_func.i ]
  %add.i262vector_func.i = add nsw <16 x i32> %mul.i161vector_func.i, %vector22vector_func.i
  %idxprom1263vector_func.i = sext <16 x i32> %add.i262vector_func.i to <16 x i64>
  %add.ptr14.sum66vector_func.i = add <16 x i64> %idxprom1263vector_func.i, %vector65vector_func.i
  %extract68vector_func.i = extractelement <16 x i64> %add.ptr14.sum66vector_func.i, i32 1
  %extract69vector_func.i = extractelement <16 x i64> %add.ptr14.sum66vector_func.i, i32 2
  %extract70vector_func.i = extractelement <16 x i64> %add.ptr14.sum66vector_func.i, i32 3
  %extract71vector_func.i = extractelement <16 x i64> %add.ptr14.sum66vector_func.i, i32 4
  %extract72vector_func.i = extractelement <16 x i64> %add.ptr14.sum66vector_func.i, i32 5
  %extract73vector_func.i = extractelement <16 x i64> %add.ptr14.sum66vector_func.i, i32 6
  %extract74vector_func.i = extractelement <16 x i64> %add.ptr14.sum66vector_func.i, i32 7
  %extract75vector_func.i = extractelement <16 x i64> %add.ptr14.sum66vector_func.i, i32 8
  %extract76vector_func.i = extractelement <16 x i64> %add.ptr14.sum66vector_func.i, i32 9
  %extract77vector_func.i = extractelement <16 x i64> %add.ptr14.sum66vector_func.i, i32 10
  %extract78vector_func.i = extractelement <16 x i64> %add.ptr14.sum66vector_func.i, i32 11
  %extract79vector_func.i = extractelement <16 x i64> %add.ptr14.sum66vector_func.i, i32 12
  %extract80vector_func.i = extractelement <16 x i64> %add.ptr14.sum66vector_func.i, i32 13
  %extract81vector_func.i = extractelement <16 x i64> %add.ptr14.sum66vector_func.i, i32 14
  %extract82vector_func.i = extractelement <16 x i64> %add.ptr14.sum66vector_func.i, i32 15
  %56 = getelementptr inbounds double addrspace(1)* %1, i64 %extract68vector_func.i
  %57 = getelementptr inbounds double addrspace(1)* %1, i64 %extract69vector_func.i
  %58 = getelementptr inbounds double addrspace(1)* %1, i64 %extract70vector_func.i
  %59 = getelementptr inbounds double addrspace(1)* %1, i64 %extract71vector_func.i
  %60 = getelementptr inbounds double addrspace(1)* %1, i64 %extract72vector_func.i
  %61 = getelementptr inbounds double addrspace(1)* %1, i64 %extract73vector_func.i
  %62 = getelementptr inbounds double addrspace(1)* %1, i64 %extract74vector_func.i
  %63 = getelementptr inbounds double addrspace(1)* %1, i64 %extract75vector_func.i
  %64 = getelementptr inbounds double addrspace(1)* %1, i64 %extract76vector_func.i
  %65 = getelementptr inbounds double addrspace(1)* %1, i64 %extract77vector_func.i
  %66 = getelementptr inbounds double addrspace(1)* %1, i64 %extract78vector_func.i
  %67 = getelementptr inbounds double addrspace(1)* %1, i64 %extract79vector_func.i
  %68 = getelementptr inbounds double addrspace(1)* %1, i64 %extract80vector_func.i
  %69 = getelementptr inbounds double addrspace(1)* %1, i64 %extract81vector_func.i
  %70 = getelementptr inbounds double addrspace(1)* %1, i64 %extract82vector_func.i
  br i1 %extract43vector_func.i, label %preload107vector_func.i, label %postload108vector_func.i

preload107vector_func.i:                          ; preds = %postload180vector_func.i
  %extract67vector_func.i = extractelement <16 x i64> %add.ptr14.sum66vector_func.i, i32 0
  %71 = getelementptr inbounds double addrspace(1)* %1, i64 %extract67vector_func.i
  store double %phivector_func.i, double addrspace(1)* %71, align 8
  br label %postload108vector_func.i

postload108vector_func.i:                         ; preds = %preload107vector_func.i, %postload180vector_func.i
  br i1 %extract44vector_func.i, label %preload112vector_func.i, label %postload113vector_func.i

preload112vector_func.i:                          ; preds = %postload108vector_func.i
  store double %phi111vector_func.i, double addrspace(1)* %56, align 8
  br label %postload113vector_func.i

postload113vector_func.i:                         ; preds = %preload112vector_func.i, %postload108vector_func.i
  br i1 %extract45vector_func.i, label %preload117vector_func.i, label %postload118vector_func.i

preload117vector_func.i:                          ; preds = %postload113vector_func.i
  store double %phi116vector_func.i, double addrspace(1)* %57, align 8
  br label %postload118vector_func.i

postload118vector_func.i:                         ; preds = %preload117vector_func.i, %postload113vector_func.i
  br i1 %extract46vector_func.i, label %preload122vector_func.i, label %postload123vector_func.i

preload122vector_func.i:                          ; preds = %postload118vector_func.i
  store double %phi121vector_func.i, double addrspace(1)* %58, align 8
  br label %postload123vector_func.i

postload123vector_func.i:                         ; preds = %preload122vector_func.i, %postload118vector_func.i
  br i1 %extract47vector_func.i, label %preload127vector_func.i, label %postload128vector_func.i

preload127vector_func.i:                          ; preds = %postload123vector_func.i
  store double %phi126vector_func.i, double addrspace(1)* %59, align 8
  br label %postload128vector_func.i

postload128vector_func.i:                         ; preds = %preload127vector_func.i, %postload123vector_func.i
  br i1 %extract48vector_func.i, label %preload132vector_func.i, label %postload133vector_func.i

preload132vector_func.i:                          ; preds = %postload128vector_func.i
  store double %phi131vector_func.i, double addrspace(1)* %60, align 8
  br label %postload133vector_func.i

postload133vector_func.i:                         ; preds = %preload132vector_func.i, %postload128vector_func.i
  br i1 %extract49vector_func.i, label %preload137vector_func.i, label %postload138vector_func.i

preload137vector_func.i:                          ; preds = %postload133vector_func.i
  store double %phi136vector_func.i, double addrspace(1)* %61, align 8
  br label %postload138vector_func.i

postload138vector_func.i:                         ; preds = %preload137vector_func.i, %postload133vector_func.i
  br i1 %extract50vector_func.i, label %preload142vector_func.i, label %postload143vector_func.i

preload142vector_func.i:                          ; preds = %postload138vector_func.i
  store double %phi141vector_func.i, double addrspace(1)* %62, align 8
  br label %postload143vector_func.i

postload143vector_func.i:                         ; preds = %preload142vector_func.i, %postload138vector_func.i
  br i1 %extract51vector_func.i, label %preload147vector_func.i, label %postload148vector_func.i

preload147vector_func.i:                          ; preds = %postload143vector_func.i
  store double %phi146vector_func.i, double addrspace(1)* %63, align 8
  br label %postload148vector_func.i

postload148vector_func.i:                         ; preds = %preload147vector_func.i, %postload143vector_func.i
  br i1 %extract52vector_func.i, label %preload152vector_func.i, label %postload153vector_func.i

preload152vector_func.i:                          ; preds = %postload148vector_func.i
  store double %phi151vector_func.i, double addrspace(1)* %64, align 8
  br label %postload153vector_func.i

postload153vector_func.i:                         ; preds = %preload152vector_func.i, %postload148vector_func.i
  br i1 %extract53vector_func.i, label %preload157vector_func.i, label %postload158vector_func.i

preload157vector_func.i:                          ; preds = %postload153vector_func.i
  store double %phi156vector_func.i, double addrspace(1)* %65, align 8
  br label %postload158vector_func.i

postload158vector_func.i:                         ; preds = %preload157vector_func.i, %postload153vector_func.i
  br i1 %extract54vector_func.i, label %preload162vector_func.i, label %postload163vector_func.i

preload162vector_func.i:                          ; preds = %postload158vector_func.i
  store double %phi161vector_func.i, double addrspace(1)* %66, align 8
  br label %postload163vector_func.i

postload163vector_func.i:                         ; preds = %preload162vector_func.i, %postload158vector_func.i
  br i1 %extract55vector_func.i, label %preload167vector_func.i, label %postload168vector_func.i

preload167vector_func.i:                          ; preds = %postload163vector_func.i
  store double %phi166vector_func.i, double addrspace(1)* %67, align 8
  br label %postload168vector_func.i

postload168vector_func.i:                         ; preds = %preload167vector_func.i, %postload163vector_func.i
  br i1 %extract56vector_func.i, label %preload172vector_func.i, label %postload173vector_func.i

preload172vector_func.i:                          ; preds = %postload168vector_func.i
  store double %phi171vector_func.i, double addrspace(1)* %68, align 8
  br label %postload173vector_func.i

postload173vector_func.i:                         ; preds = %preload172vector_func.i, %postload168vector_func.i
  br i1 %extract57vector_func.i, label %preload177vector_func.i, label %postload178vector_func.i

preload177vector_func.i:                          ; preds = %postload173vector_func.i
  store double %phi176vector_func.i, double addrspace(1)* %69, align 8
  br label %postload178vector_func.i

postload178vector_func.i:                         ; preds = %preload177vector_func.i, %postload173vector_func.i
  br i1 %extract58vector_func.i, label %preload182vector_func.i, label %postload183vector_func.i

preload182vector_func.i:                          ; preds = %postload178vector_func.i
  store double %phi181vector_func.i, double addrspace(1)* %70, align 8
  br label %postload183vector_func.i

postload183vector_func.i:                         ; preds = %preload182vector_func.i, %postload178vector_func.i
  %incvector_func.i = add nsw i32 %c.02vector_func.i, 1
  %exitcondvector_func.i = icmp eq i32 %incvector_func.i, %19
  %temp83vector_func.i = insertelement <16 x i1> undef, i1 %exitcondvector_func.i, i32 0
  %vector84vector_func.i = shufflevector <16 x i1> %temp83vector_func.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %notCondvector_func.i = xor i1 %exitcondvector_func.i, true
  %temp89vector_func.i = insertelement <16 x i1> undef, i1 %notCondvector_func.i, i32 0
  %vector90vector_func.i = shufflevector <16 x i1> %temp89vector_func.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %who_left_tr85vector_func.i = and <16 x i1> %vectorPHI17vector_func.i, %vector84vector_func.i
  %loop_mask387vector_func.i = or <16 x i1> %vectorPHIvector_func.i, %who_left_tr85vector_func.i
  %ipred.i.i = bitcast <16 x i1> %loop_mask387vector_func.i to i16
  %val.i.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i.i, i16 %ipred.i.i) nounwind
  %72 = and i32 %val.i.i, 1
  %res.i.i = icmp eq i32 %72, 0
  %local_edge91vector_func.i = and <16 x i1> %vectorPHI17vector_func.i, %vector90vector_func.i
  br i1 %res.i.i, label %for.bodyvector_func.i, label %if.endvector_func.i

if.endvector_func.i:                              ; preds = %postload183vector_func.i, %entryvector_func.i
  %dim_0_vector_inc_ind_var.i = add i64 %dim_0_vector_ind_var.i, 1
  %dim_0_vector_cmp.to.max.i = icmp eq i64 %dim_0_vector_inc_ind_var.i, %vector.size.i
  %dim_0_vector_inc_tid.i = add i64 %dim_0_vector_tid.i, 16
  br i1 %dim_0_vector_cmp.to.max.i, label %dim_0_vector_exit.i, label %if.endvector_func.i.entryvector_func.i_crit_edge

if.endvector_func.i.entryvector_func.i_crit_edge: ; preds = %if.endvector_func.i
  br label %entryvector_func.i

dim_0_vector_exit.i:                              ; preds = %if.endvector_func.i
  %dim_1_vector_inc_ind_var.i = add i64 %dim_1_vector_ind_var.i, 1
  %dim_1_vector_cmp.to.max.i = icmp eq i64 %dim_1_vector_inc_ind_var.i, %32
  br i1 %dim_1_vector_cmp.to.max.i, label %dim_1_vector_exit.i, label %dim_0_vector_pre_head.i

dim_1_vector_exit.i:                              ; preds = %dim_0_vector_exit.i
  %dim_2_vector_inc_ind_var.i = add i64 %dim_2_vector_ind_var.i, 1
  %dim_2_vector_cmp.to.max.i = icmp eq i64 %dim_2_vector_inc_ind_var.i, %34
  br i1 %dim_2_vector_cmp.to.max.i, label %scalarIf.i, label %dim_1_vector_pre_head.i

scalarIf.i:                                       ; preds = %dim_1_vector_exit.i, %entry
  %73 = icmp eq i64 %30, %num.vector.wi.i
  br i1 %73, label %__CopyRect_separated_args.exit, label %dim_2_pre_head.i

dim_2_pre_head.i:                                 ; preds = %scalarIf.i
  %cmp81.i = icmp sgt i32 %19, 0
  %idx.ext.i = sext i32 %13 to i64
  %idx.ext13.i = sext i32 %4 to i64
  br label %dim_1_pre_head.i

dim_1_pre_head.i:                                 ; preds = %dim_1_exit.i, %dim_2_pre_head.i
  %dim_2_ind_var.i = phi i64 [ 0, %dim_2_pre_head.i ], [ %dim_2_inc_ind_var.i, %dim_1_exit.i ]
  br label %dim_0_pre_head.i

dim_0_pre_head.i:                                 ; preds = %dim_0_exit.i, %dim_1_pre_head.i
  %dim_1_ind_var.i = phi i64 [ 0, %dim_1_pre_head.i ], [ %dim_1_inc_ind_var.i, %dim_0_exit.i ]
  br label %scalar_kernel_entry.i

scalar_kernel_entry.i:                            ; preds = %if.end.i.scalar_kernel_entry.i_crit_edge, %dim_0_pre_head.i
  %dim_0_ind_var.i = phi i64 [ 0, %dim_0_pre_head.i ], [ %dim_0_inc_ind_var.i, %if.end.i.scalar_kernel_entry.i_crit_edge ]
  %dim_0_tid.i = phi i64 [ %num.vector.wi.i, %dim_0_pre_head.i ], [ %dim_0_inc_tid.i, %if.end.i.scalar_kernel_entry.i_crit_edge ]
  %74 = load i64* %28, align 8
  %conv.i = trunc i64 %74 to i32
  %conv2.i = trunc i64 %dim_0_tid.i to i32
  %75 = load i64* %29, align 8
  %conv6.i = trunc i64 %75 to i32
  %mul.i = mul nsw i32 %conv6.i, %conv.i
  %add.i = add nsw i32 %mul.i, %conv2.i
  %cmp.i = icmp slt i32 %add.i, %22
  %or.cond.i = and i1 %cmp.i, %cmp81.i
  br i1 %or.cond.i, label %for.body.lr.ph.i, label %if.end.i

for.body.lr.ph.i:                                 ; preds = %scalar_kernel_entry.i
  %mul.i.i = mul nsw i32 %add.i, %16
  %mul.i3.i = mul nsw i32 %add.i, %7
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %for.body.lr.ph.i
  %c.02.i = phi i32 [ 0, %for.body.lr.ph.i ], [ %inc.i, %for.body.i ]
  %add.i.i = add nsw i32 %mul.i.i, %c.02.i
  %idxprom.i = sext i32 %add.i.i to i64
  %add.ptr.sum.i = add i64 %idxprom.i, %idx.ext.i
  %arrayidx.i = getelementptr inbounds double addrspace(1)* %10, i64 %add.ptr.sum.i
  %76 = load double addrspace(1)* %arrayidx.i, align 8
  %add.i4.i = add nsw i32 %mul.i3.i, %c.02.i
  %idxprom12.i = sext i32 %add.i4.i to i64
  %add.ptr14.sum.i = add i64 %idxprom12.i, %idx.ext13.i
  %arrayidx15.i = getelementptr inbounds double addrspace(1)* %1, i64 %add.ptr14.sum.i
  store double %76, double addrspace(1)* %arrayidx15.i, align 8
  %inc.i = add nsw i32 %c.02.i, 1
  %exitcond.i = icmp eq i32 %inc.i, %19
  br i1 %exitcond.i, label %if.end.i, label %for.body.i

if.end.i:                                         ; preds = %for.body.i, %scalar_kernel_entry.i
  %dim_0_inc_ind_var.i = add i64 %dim_0_ind_var.i, 1
  %dim_0_cmp.to.max.i = icmp eq i64 %dim_0_inc_ind_var.i, %scalar.size.i
  %dim_0_inc_tid.i = add i64 %dim_0_tid.i, 1
  br i1 %dim_0_cmp.to.max.i, label %dim_0_exit.i, label %if.end.i.scalar_kernel_entry.i_crit_edge

if.end.i.scalar_kernel_entry.i_crit_edge:         ; preds = %if.end.i
  br label %scalar_kernel_entry.i

dim_0_exit.i:                                     ; preds = %if.end.i
  %dim_1_inc_ind_var.i = add i64 %dim_1_ind_var.i, 1
  %dim_1_cmp.to.max.i = icmp eq i64 %dim_1_inc_ind_var.i, %32
  br i1 %dim_1_cmp.to.max.i, label %dim_1_exit.i, label %dim_0_pre_head.i

dim_1_exit.i:                                     ; preds = %dim_0_exit.i
  %dim_2_inc_ind_var.i = add i64 %dim_2_ind_var.i, 1
  %dim_2_cmp.to.max.i = icmp eq i64 %dim_2_inc_ind_var.i, %34
  br i1 %dim_2_cmp.to.max.i, label %__CopyRect_separated_args.exit, label %dim_1_pre_head.i

__CopyRect_separated_args.exit:                   ; preds = %scalarIf.i, %dim_1_exit.i
  ret void
}

define void @__Vectorized_.StencilKernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to double*
  %7 = load double* %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to double*
  %10 = load double* %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to double*
  %13 = load double* %12, align 8
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
  %32 = bitcast i8 addrspace(3)* %16 to [18 x [18 x double]] addrspace(3)*
  %33 = bitcast i8 addrspace(3)* %16 to double addrspace(3)*
  %34 = getelementptr i8 addrspace(3)* %16, i64 2448
  %35 = bitcast i8 addrspace(3)* %34 to double addrspace(3)*
  %36 = getelementptr i8 addrspace(3)* %16, i64 136
  %37 = bitcast i8 addrspace(3)* %36 to double addrspace(3)*
  %38 = getelementptr i8 addrspace(3)* %16, i64 2584
  %39 = bitcast i8 addrspace(3)* %38 to double addrspace(3)*
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB1571.i, %entry
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride1577.i", %thenBB1571.i ]
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++1575.i", %thenBB1571.i ]
  %40 = getelementptr i64* %22, i64 1
  %41 = load i64* %40, align 8
  %conv.i = trunc i64 %41 to i32
  %42 = load i64* %22, align 8
  %conv2.i = trunc i64 %42 to i32
  %43 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 4, i64 0
  %44 = load i64* %43, align 8
  %conv6.i = trunc i64 %44 to i32
  %45 = getelementptr <{ [4 x i64] }>* %25, i64 %CurrWI..0.i, i32 0, i64 1
  %46 = load i64* %45, align 8
  %temp264.i = insertelement <16 x i64> undef, i64 %46, i32 0
  %vector265.i = shufflevector <16 x i64> %temp264.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %conv8.i = trunc i64 %46 to i32
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %31, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %conv8.i, i32* %CastToValueType.i, align 4
  %47 = getelementptr <{ [4 x i64] }>* %25, i64 %CurrWI..0.i, i32 0, i64 0
  %48 = load i64* %47, align 8
  %broadcast1.i = insertelement <16 x i64> undef, i64 %48, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %49 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %conv1084.i = trunc <16 x i64> %49 to <16 x i32>
  %"&(pSB[currWI].offset)1478.i" = add nuw i64 %CurrSBIndex..0.i, 64
  %"&pSB[currWI].offset1479.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1478.i"
  %CastToValueType1480.i = bitcast i8* %"&pSB[currWI].offset1479.i" to <16 x i32>*
  store <16 x i32> %conv1084.i, <16 x i32>* %CastToValueType1480.i, align 64
  %mul.i.i = shl i32 %conv.i, 4
  %add.i.i = add nsw i32 %mul.i.i, %conv8.i
  %mul.i43.i = shl i32 %conv2.i, 4
  %temp.i = insertelement <16 x i32> undef, i32 %mul.i43.i, i32 0
  %vector.i = shufflevector <16 x i32> %temp.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %add.i4485.i = add nsw <16 x i32> %vector.i, %conv1084.i
  %mul.i = shl nsw i32 %conv6.i, 4
  %add.i38.i = add nsw i32 %add.i.i, 1
  %add1.i3945.i = or i32 %mul.i, 2
  %mul.i40.i = mul nsw i32 %add1.i3945.i, %add.i38.i
  %temp87.i = insertelement <16 x i32> undef, i32 %mul.i40.i, i32 0
  %vector88.i = shufflevector <16 x i32> %temp87.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %add2.i4186.i = add <16 x i32> %add.i4485.i, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %add3.i4289.i = add <16 x i32> %add2.i4186.i, %vector88.i
  %idxprom90.i = sext <16 x i32> %add3.i4289.i to <16 x i64>
  %extract.i = extractelement <16 x i64> %idxprom90.i, i32 0
  %"&(pSB[currWI].offset)1497.i" = add nuw i64 %CurrSBIndex..0.i, 128
  %"&pSB[currWI].offset1498.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1497.i"
  %CastToValueType1499.i = bitcast i8* %"&pSB[currWI].offset1498.i" to i64*
  store i64 %extract.i, i64* %CastToValueType1499.i, align 8
  %50 = getelementptr inbounds double addrspace(1)* %1, i64 %extract.i
  %ptrTypeCast.i = bitcast double addrspace(1)* %50 to <16 x double> addrspace(1)*
  %51 = load <16 x double> addrspace(1)* %ptrTypeCast.i, align 8
  %add106.i = add nsw <16 x i32> %conv1084.i, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %idxprom14107.i = sext <16 x i32> %add106.i to <16 x i64>
  %extract108.i = extractelement <16 x i64> %idxprom14107.i, i32 0
  %"&(pSB[currWI].offset)1506.i" = add nuw i64 %CurrSBIndex..0.i, 136
  %"&pSB[currWI].offset1507.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1506.i"
  %CastToValueType1508.i = bitcast i8* %"&pSB[currWI].offset1507.i" to i64*
  store i64 %extract108.i, i64* %CastToValueType1508.i, align 8
  %add15.i = add nsw i32 %conv8.i, 1
  %idxprom16.i = sext i32 %add15.i to i64
  %"&(pSB[currWI].offset)1530.i" = add nuw i64 %CurrSBIndex..0.i, 144
  %"&pSB[currWI].offset1531.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1530.i"
  %CastToValueType1532.i = bitcast i8* %"&pSB[currWI].offset1531.i" to i64*
  store i64 %idxprom16.i, i64* %CastToValueType1532.i, align 8
  %52 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %idxprom16.i, i64 %extract108.i
  %ptrTypeCast124.i = bitcast double addrspace(3)* %52 to <16 x double> addrspace(3)*
  %"&(pSB[currWI].offset)1554.i" = add nuw i64 %CurrSBIndex..0.i, 152
  %"&pSB[currWI].offset1555.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1554.i"
  %CastToValueType1556.i = bitcast i8* %"&pSB[currWI].offset1555.i" to <16 x double> addrspace(3)**
  store <16 x double> addrspace(3)* %ptrTypeCast124.i, <16 x double> addrspace(3)** %CastToValueType1556.i, align 8
  store <16 x double> %51, <16 x double> addrspace(3)* %ptrTypeCast124.i, align 8
  %cmp.i = icmp eq i32 %conv8.i, 0
  br i1 %cmp.i, label %preload1185.i, label %postload1186.i

preload1185.i:                                    ; preds = %SyncBB.i
  %.lhs.lhs.i = extractelement <16 x i32> %add.i4485.i, i32 0
  %.lhs.i = add i32 %.lhs.lhs.i, 1
  %mul.i35.i = mul nsw i32 %add1.i3945.i, %mul.i.i
  %53 = add i32 %.lhs.i, %mul.i35.i
  %extract130.i = sext i32 %53 to i64
  %54 = getelementptr inbounds double addrspace(1)* %1, i64 %extract130.i
  %ptrTypeCast146.i = bitcast double addrspace(1)* %54 to <16 x double> addrspace(1)*
  %masked_load.i = load <16 x double> addrspace(1)* %ptrTypeCast146.i, align 8
  br label %postload1186.i

postload1186.i:                                   ; preds = %preload1185.i, %SyncBB.i
  %phi1187.i = phi <16 x double> [ undef, %SyncBB.i ], [ %masked_load.i, %preload1185.i ]
  br i1 %cmp.i, label %preload1188.i, label %postload1191.i

preload1188.i:                                    ; preds = %postload1186.i
  %loadedValue1528.i = load i64* %CastToValueType1508.i, align 8
  %55 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 0, i64 %loadedValue1528.i
  %ptrTypeCast147.i = bitcast double addrspace(3)* %55 to <16 x double> addrspace(3)*
  store <16 x double> %phi1187.i, <16 x double> addrspace(3)* %ptrTypeCast147.i, align 8
  %.lhs1372.lhs.i = extractelement <16 x i32> %add.i4485.i, i32 0
  %add.i28.i = add nsw i32 %add.i.i, 17
  %.lhs1372.i = add i32 %.lhs1372.lhs.i, 1
  %mul.i30.i = mul nsw i32 %add1.i3945.i, %add.i28.i
  %56 = add i32 %.lhs1372.i, %mul.i30.i
  %extract153.i = sext i32 %56 to i64
  %57 = getelementptr inbounds double addrspace(1)* %1, i64 %extract153.i
  %ptrTypeCast169.i = bitcast double addrspace(1)* %57 to <16 x double> addrspace(1)*
  %masked_load497.i = load <16 x double> addrspace(1)* %ptrTypeCast169.i, align 8
  br label %postload1191.i

postload1191.i:                                   ; preds = %preload1188.i, %postload1186.i
  %phi1192.i = phi <16 x double> [ %masked_load497.i, %preload1188.i ], [ undef, %postload1186.i ]
  br i1 %cmp.i, label %preload1193.i, label %if.end.i

preload1193.i:                                    ; preds = %postload1191.i
  %loadedValue1523.i = load i64* %CastToValueType1508.i, align 8
  %58 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 17, i64 %loadedValue1523.i
  %ptrTypeCast170.i = bitcast double addrspace(3)* %58 to <16 x double> addrspace(3)*
  store <16 x double> %phi1192.i, <16 x double> addrspace(3)* %ptrTypeCast170.i, align 8
  br label %if.end.i

if.end.i:                                         ; preds = %preload1193.i, %postload1191.i
  %loadedValue1495.i = load <16 x i32>* %CastToValueType1480.i, align 64
  %cmp33.i = icmp eq <16 x i32> %loadedValue1495.i, zeroinitializer
  %extract194.i = extractelement <16 x i1> %cmp33.i, i32 0
  %extract195.i = extractelement <16 x i1> %cmp33.i, i32 1
  %extract196.i = extractelement <16 x i1> %cmp33.i, i32 2
  %extract197.i = extractelement <16 x i1> %cmp33.i, i32 3
  %extract198.i = extractelement <16 x i1> %cmp33.i, i32 4
  %extract199.i = extractelement <16 x i1> %cmp33.i, i32 5
  %extract200.i = extractelement <16 x i1> %cmp33.i, i32 6
  %extract201.i = extractelement <16 x i1> %cmp33.i, i32 7
  %extract202.i = extractelement <16 x i1> %cmp33.i, i32 8
  %extract203.i = extractelement <16 x i1> %cmp33.i, i32 9
  %extract204.i = extractelement <16 x i1> %cmp33.i, i32 10
  %extract205.i = extractelement <16 x i1> %cmp33.i, i32 11
  %extract206.i = extractelement <16 x i1> %cmp33.i, i32 12
  %extract207.i = extractelement <16 x i1> %cmp33.i, i32 13
  %extract208.i = extractelement <16 x i1> %cmp33.i, i32 14
  %extract209.i = extractelement <16 x i1> %cmp33.i, i32 15
  %.lhs1376.i = extractelement <16 x i32> %add.i4485.i, i32 0
  %59 = add i32 %.lhs1376.i, %mul.i40.i
  %extract177.i = sext i32 %59 to i64
  br i1 %extract194.i, label %preload894.i, label %postload895.i

preload894.i:                                     ; preds = %if.end.i
  %60 = getelementptr inbounds double addrspace(1)* %1, i64 %extract177.i
  %vload499.i = load double addrspace(1)* %60, align 8
  br label %postload895.i

postload895.i:                                    ; preds = %preload894.i, %if.end.i
  %phi896.i = phi double [ undef, %if.end.i ], [ %vload499.i, %preload894.i ]
  br i1 %extract195.i, label %preload885.i, label %postload886.i

preload885.i:                                     ; preds = %postload895.i
  %.sum1467.i = add i64 %extract177.i, 1
  %61 = getelementptr double addrspace(1)* %1, i64 %.sum1467.i
  %vload502.i = load double addrspace(1)* %61, align 8
  br label %postload886.i

postload886.i:                                    ; preds = %preload885.i, %postload895.i
  %phi887.i = phi double [ undef, %postload895.i ], [ %vload502.i, %preload885.i ]
  br i1 %extract196.i, label %preload897.i, label %postload898.i

preload897.i:                                     ; preds = %postload886.i
  %.sum1466.i = add i64 %extract177.i, 2
  %62 = getelementptr double addrspace(1)* %1, i64 %.sum1466.i
  %vload506.i = load double addrspace(1)* %62, align 8
  br label %postload898.i

postload898.i:                                    ; preds = %preload897.i, %postload886.i
  %phi899.i = phi double [ undef, %postload886.i ], [ %vload506.i, %preload897.i ]
  br i1 %extract197.i, label %preload888.i, label %postload889.i

preload888.i:                                     ; preds = %postload898.i
  %.sum1465.i = add i64 %extract177.i, 3
  %63 = getelementptr double addrspace(1)* %1, i64 %.sum1465.i
  %vload510.i = load double addrspace(1)* %63, align 8
  br label %postload889.i

postload889.i:                                    ; preds = %preload888.i, %postload898.i
  %phi890.i = phi double [ undef, %postload898.i ], [ %vload510.i, %preload888.i ]
  br i1 %extract198.i, label %preload1176.i, label %postload1177.i

preload1176.i:                                    ; preds = %postload889.i
  %.sum1464.i = add i64 %extract177.i, 4
  %64 = getelementptr double addrspace(1)* %1, i64 %.sum1464.i
  %vload514.i = load double addrspace(1)* %64, align 8
  br label %postload1177.i

postload1177.i:                                   ; preds = %preload1176.i, %postload889.i
  %phi1178.i = phi double [ undef, %postload889.i ], [ %vload514.i, %preload1176.i ]
  br i1 %extract199.i, label %preload1210.i, label %postload1211.i

preload1210.i:                                    ; preds = %postload1177.i
  %.sum1463.i = add i64 %extract177.i, 5
  %65 = getelementptr double addrspace(1)* %1, i64 %.sum1463.i
  %vload518.i = load double addrspace(1)* %65, align 8
  br label %postload1211.i

postload1211.i:                                   ; preds = %preload1210.i, %postload1177.i
  %phi1212.i = phi double [ undef, %postload1177.i ], [ %vload518.i, %preload1210.i ]
  br i1 %extract200.i, label %preload1273.i, label %postload1274.i

preload1273.i:                                    ; preds = %postload1211.i
  %.sum1462.i = add i64 %extract177.i, 6
  %66 = getelementptr double addrspace(1)* %1, i64 %.sum1462.i
  %vload522.i = load double addrspace(1)* %66, align 8
  br label %postload1274.i

postload1274.i:                                   ; preds = %preload1273.i, %postload1211.i
  %phi1275.i = phi double [ undef, %postload1211.i ], [ %vload522.i, %preload1273.i ]
  br i1 %extract201.i, label %preload.i, label %postload.i

preload.i:                                        ; preds = %postload1274.i
  %.sum1461.i = add i64 %extract177.i, 7
  %67 = getelementptr double addrspace(1)* %1, i64 %.sum1461.i
  %vload526.i = load double addrspace(1)* %67, align 8
  br label %postload.i

postload.i:                                       ; preds = %preload.i, %postload1274.i
  %phi.i = phi double [ undef, %postload1274.i ], [ %vload526.i, %preload.i ]
  br i1 %extract202.i, label %preload1207.i, label %postload1208.i

preload1207.i:                                    ; preds = %postload.i
  %.sum1460.i = add i64 %extract177.i, 8
  %68 = getelementptr double addrspace(1)* %1, i64 %.sum1460.i
  %vload530.i = load double addrspace(1)* %68, align 8
  br label %postload1208.i

postload1208.i:                                   ; preds = %preload1207.i, %postload.i
  %phi1209.i = phi double [ undef, %postload.i ], [ %vload530.i, %preload1207.i ]
  br i1 %extract203.i, label %preload1213.i, label %postload1214.i

preload1213.i:                                    ; preds = %postload1208.i
  %.sum1459.i = add i64 %extract177.i, 9
  %69 = getelementptr double addrspace(1)* %1, i64 %.sum1459.i
  %vload534.i = load double addrspace(1)* %69, align 8
  br label %postload1214.i

postload1214.i:                                   ; preds = %preload1213.i, %postload1208.i
  %phi1215.i = phi double [ undef, %postload1208.i ], [ %vload534.i, %preload1213.i ]
  br i1 %extract204.i, label %preload942.i, label %postload943.i

preload942.i:                                     ; preds = %postload1214.i
  %.sum1458.i = add i64 %extract177.i, 10
  %70 = getelementptr double addrspace(1)* %1, i64 %.sum1458.i
  %vload538.i = load double addrspace(1)* %70, align 8
  br label %postload943.i

postload943.i:                                    ; preds = %preload942.i, %postload1214.i
  %phi944.i = phi double [ undef, %postload1214.i ], [ %vload538.i, %preload942.i ]
  br i1 %extract205.i, label %preload891.i, label %postload892.i

preload891.i:                                     ; preds = %postload943.i
  %.sum1457.i = add i64 %extract177.i, 11
  %71 = getelementptr double addrspace(1)* %1, i64 %.sum1457.i
  %vload542.i = load double addrspace(1)* %71, align 8
  br label %postload892.i

postload892.i:                                    ; preds = %preload891.i, %postload943.i
  %phi893.i = phi double [ undef, %postload943.i ], [ %vload542.i, %preload891.i ]
  br i1 %extract206.i, label %preload1369.i, label %postload1370.i

preload1369.i:                                    ; preds = %postload892.i
  %.sum1456.i = add i64 %extract177.i, 12
  %72 = getelementptr double addrspace(1)* %1, i64 %.sum1456.i
  %vload546.i = load double addrspace(1)* %72, align 8
  br label %postload1370.i

postload1370.i:                                   ; preds = %preload1369.i, %postload892.i
  %phi1371.i = phi double [ undef, %postload892.i ], [ %vload546.i, %preload1369.i ]
  br i1 %extract207.i, label %preload903.i, label %postload904.i

preload903.i:                                     ; preds = %postload1370.i
  %.sum1455.i = add i64 %extract177.i, 13
  %73 = getelementptr double addrspace(1)* %1, i64 %.sum1455.i
  %vload550.i = load double addrspace(1)* %73, align 8
  br label %postload904.i

postload904.i:                                    ; preds = %preload903.i, %postload1370.i
  %phi905.i = phi double [ undef, %postload1370.i ], [ %vload550.i, %preload903.i ]
  br i1 %extract208.i, label %preload1240.i, label %postload1241.i

preload1240.i:                                    ; preds = %postload904.i
  %.sum1454.i = add i64 %extract177.i, 14
  %74 = getelementptr double addrspace(1)* %1, i64 %.sum1454.i
  %vload554.i = load double addrspace(1)* %74, align 8
  br label %postload1241.i

postload1241.i:                                   ; preds = %preload1240.i, %postload904.i
  %phi1242.i = phi double [ undef, %postload904.i ], [ %vload554.i, %preload1240.i ]
  br i1 %extract209.i, label %preload1204.i, label %postload1205.i

preload1204.i:                                    ; preds = %postload1241.i
  %.sum1453.i = add i64 %extract177.i, 15
  %75 = getelementptr double addrspace(1)* %1, i64 %.sum1453.i
  %vload558.i = load double addrspace(1)* %75, align 8
  br label %postload1205.i

postload1205.i:                                   ; preds = %preload1204.i, %postload1241.i
  %phi1206.i = phi double [ undef, %postload1241.i ], [ %vload558.i, %preload1204.i ]
  %loadedValue1552.i = load i64* %CastToValueType1532.i, align 8
  %arrayidx43.i = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %loadedValue1552.i, i64 0
  br i1 %extract194.i, label %preload960.i, label %postload961.i

preload960.i:                                     ; preds = %postload1205.i
  store double %phi896.i, double addrspace(3)* %arrayidx43.i, align 16
  br label %postload961.i

postload961.i:                                    ; preds = %preload960.i, %postload1205.i
  br i1 %extract195.i, label %preload964.i, label %postload965.i

preload964.i:                                     ; preds = %postload961.i
  store double %phi887.i, double addrspace(3)* %arrayidx43.i, align 16
  br label %postload965.i

postload965.i:                                    ; preds = %preload964.i, %postload961.i
  br i1 %extract196.i, label %preload968.i, label %postload969.i

preload968.i:                                     ; preds = %postload965.i
  store double %phi899.i, double addrspace(3)* %arrayidx43.i, align 16
  br label %postload969.i

postload969.i:                                    ; preds = %preload968.i, %postload965.i
  br i1 %extract197.i, label %preload972.i, label %postload973.i

preload972.i:                                     ; preds = %postload969.i
  store double %phi890.i, double addrspace(3)* %arrayidx43.i, align 16
  br label %postload973.i

postload973.i:                                    ; preds = %preload972.i, %postload969.i
  br i1 %extract198.i, label %preload976.i, label %postload977.i

preload976.i:                                     ; preds = %postload973.i
  store double %phi1178.i, double addrspace(3)* %arrayidx43.i, align 16
  br label %postload977.i

postload977.i:                                    ; preds = %preload976.i, %postload973.i
  br i1 %extract199.i, label %preload980.i, label %postload981.i

preload980.i:                                     ; preds = %postload977.i
  store double %phi1212.i, double addrspace(3)* %arrayidx43.i, align 16
  br label %postload981.i

postload981.i:                                    ; preds = %preload980.i, %postload977.i
  br i1 %extract200.i, label %preload984.i, label %postload985.i

preload984.i:                                     ; preds = %postload981.i
  store double %phi1275.i, double addrspace(3)* %arrayidx43.i, align 16
  br label %postload985.i

postload985.i:                                    ; preds = %preload984.i, %postload981.i
  br i1 %extract201.i, label %preload988.i, label %postload989.i

preload988.i:                                     ; preds = %postload985.i
  store double %phi.i, double addrspace(3)* %arrayidx43.i, align 16
  br label %postload989.i

postload989.i:                                    ; preds = %preload988.i, %postload985.i
  br i1 %extract202.i, label %preload992.i, label %postload993.i

preload992.i:                                     ; preds = %postload989.i
  store double %phi1209.i, double addrspace(3)* %arrayidx43.i, align 16
  br label %postload993.i

postload993.i:                                    ; preds = %preload992.i, %postload989.i
  br i1 %extract203.i, label %preload996.i, label %postload997.i

preload996.i:                                     ; preds = %postload993.i
  store double %phi1215.i, double addrspace(3)* %arrayidx43.i, align 16
  br label %postload997.i

postload997.i:                                    ; preds = %preload996.i, %postload993.i
  br i1 %extract204.i, label %preload1000.i, label %postload1001.i

preload1000.i:                                    ; preds = %postload997.i
  store double %phi944.i, double addrspace(3)* %arrayidx43.i, align 16
  br label %postload1001.i

postload1001.i:                                   ; preds = %preload1000.i, %postload997.i
  br i1 %extract205.i, label %preload1004.i, label %postload1005.i

preload1004.i:                                    ; preds = %postload1001.i
  store double %phi893.i, double addrspace(3)* %arrayidx43.i, align 16
  br label %postload1005.i

postload1005.i:                                   ; preds = %preload1004.i, %postload1001.i
  br i1 %extract206.i, label %preload1008.i, label %postload1009.i

preload1008.i:                                    ; preds = %postload1005.i
  store double %phi1371.i, double addrspace(3)* %arrayidx43.i, align 16
  br label %postload1009.i

postload1009.i:                                   ; preds = %preload1008.i, %postload1005.i
  br i1 %extract207.i, label %preload1012.i, label %postload1013.i

preload1012.i:                                    ; preds = %postload1009.i
  store double %phi905.i, double addrspace(3)* %arrayidx43.i, align 16
  br label %postload1013.i

postload1013.i:                                   ; preds = %preload1012.i, %postload1009.i
  br i1 %extract208.i, label %preload1016.i, label %postload1017.i

preload1016.i:                                    ; preds = %postload1013.i
  store double %phi1242.i, double addrspace(3)* %arrayidx43.i, align 16
  br label %postload1017.i

postload1017.i:                                   ; preds = %preload1016.i, %postload1013.i
  br i1 %extract209.i, label %preload1020.i, label %postload1021.i

preload1020.i:                                    ; preds = %postload1017.i
  store double %phi1206.i, double addrspace(3)* %arrayidx43.i, align 16
  br label %postload1021.i

postload1021.i:                                   ; preds = %preload1020.i, %postload1017.i
  %.lhs1373.i = add i32 %.lhs1376.i, 17
  %76 = add i32 %.lhs1373.i, %mul.i40.i
  %extract231.i = sext i32 %76 to i64
  br i1 %extract194.i, label %preload1179.i, label %postload1180.i

preload1179.i:                                    ; preds = %postload1021.i
  %77 = getelementptr inbounds double addrspace(1)* %1, i64 %extract231.i
  %vload563.i = load double addrspace(1)* %77, align 8
  br label %postload1180.i

postload1180.i:                                   ; preds = %preload1179.i, %postload1021.i
  %phi1181.i = phi double [ undef, %postload1021.i ], [ %vload563.i, %preload1179.i ]
  br i1 %extract195.i, label %preload915.i, label %postload916.i

preload915.i:                                     ; preds = %postload1180.i
  %.sum1452.i = add i64 %extract231.i, 1
  %78 = getelementptr double addrspace(1)* %1, i64 %.sum1452.i
  %vload567.i = load double addrspace(1)* %78, align 8
  br label %postload916.i

postload916.i:                                    ; preds = %preload915.i, %postload1180.i
  %phi917.i = phi double [ undef, %postload1180.i ], [ %vload567.i, %preload915.i ]
  br i1 %extract196.i, label %preload900.i, label %postload901.i

preload900.i:                                     ; preds = %postload916.i
  %.sum1451.i = add i64 %extract231.i, 2
  %79 = getelementptr double addrspace(1)* %1, i64 %.sum1451.i
  %vload571.i = load double addrspace(1)* %79, align 8
  br label %postload901.i

postload901.i:                                    ; preds = %preload900.i, %postload916.i
  %phi902.i = phi double [ undef, %postload916.i ], [ %vload571.i, %preload900.i ]
  br i1 %extract197.i, label %preload1182.i, label %postload1183.i

preload1182.i:                                    ; preds = %postload901.i
  %.sum1450.i = add i64 %extract231.i, 3
  %80 = getelementptr double addrspace(1)* %1, i64 %.sum1450.i
  %vload575.i = load double addrspace(1)* %80, align 8
  br label %postload1183.i

postload1183.i:                                   ; preds = %preload1182.i, %postload901.i
  %phi1184.i = phi double [ undef, %postload901.i ], [ %vload575.i, %preload1182.i ]
  br i1 %extract198.i, label %preload1173.i, label %postload1174.i

preload1173.i:                                    ; preds = %postload1183.i
  %.sum1449.i = add i64 %extract231.i, 4
  %81 = getelementptr double addrspace(1)* %1, i64 %.sum1449.i
  %vload579.i = load double addrspace(1)* %81, align 8
  br label %postload1174.i

postload1174.i:                                   ; preds = %preload1173.i, %postload1183.i
  %phi1175.i = phi double [ undef, %postload1183.i ], [ %vload579.i, %preload1173.i ]
  br i1 %extract199.i, label %preload921.i, label %postload922.i

preload921.i:                                     ; preds = %postload1174.i
  %.sum1448.i = add i64 %extract231.i, 5
  %82 = getelementptr double addrspace(1)* %1, i64 %.sum1448.i
  %vload583.i = load double addrspace(1)* %82, align 8
  br label %postload922.i

postload922.i:                                    ; preds = %preload921.i, %postload1174.i
  %phi923.i = phi double [ undef, %postload1174.i ], [ %vload583.i, %preload921.i ]
  br i1 %extract200.i, label %preload924.i, label %postload925.i

preload924.i:                                     ; preds = %postload922.i
  %.sum1447.i = add i64 %extract231.i, 6
  %83 = getelementptr double addrspace(1)* %1, i64 %.sum1447.i
  %vload587.i = load double addrspace(1)* %83, align 8
  br label %postload925.i

postload925.i:                                    ; preds = %preload924.i, %postload922.i
  %phi926.i = phi double [ undef, %postload922.i ], [ %vload587.i, %preload924.i ]
  br i1 %extract201.i, label %preload1201.i, label %postload1202.i

preload1201.i:                                    ; preds = %postload925.i
  %.sum1446.i = add i64 %extract231.i, 7
  %84 = getelementptr double addrspace(1)* %1, i64 %.sum1446.i
  %vload591.i = load double addrspace(1)* %84, align 8
  br label %postload1202.i

postload1202.i:                                   ; preds = %preload1201.i, %postload925.i
  %phi1203.i = phi double [ undef, %postload925.i ], [ %vload591.i, %preload1201.i ]
  br i1 %extract202.i, label %preload918.i, label %postload919.i

preload918.i:                                     ; preds = %postload1202.i
  %.sum1445.i = add i64 %extract231.i, 8
  %85 = getelementptr double addrspace(1)* %1, i64 %.sum1445.i
  %vload595.i = load double addrspace(1)* %85, align 8
  br label %postload919.i

postload919.i:                                    ; preds = %preload918.i, %postload1202.i
  %phi920.i = phi double [ undef, %postload1202.i ], [ %vload595.i, %preload918.i ]
  br i1 %extract203.i, label %preload1195.i, label %postload1196.i

preload1195.i:                                    ; preds = %postload919.i
  %.sum1444.i = add i64 %extract231.i, 9
  %86 = getelementptr double addrspace(1)* %1, i64 %.sum1444.i
  %vload599.i = load double addrspace(1)* %86, align 8
  br label %postload1196.i

postload1196.i:                                   ; preds = %preload1195.i, %postload919.i
  %phi1197.i = phi double [ undef, %postload919.i ], [ %vload599.i, %preload1195.i ]
  br i1 %extract204.i, label %preload1198.i, label %postload1199.i

preload1198.i:                                    ; preds = %postload1196.i
  %.sum1443.i = add i64 %extract231.i, 10
  %87 = getelementptr double addrspace(1)* %1, i64 %.sum1443.i
  %vload603.i = load double addrspace(1)* %87, align 8
  br label %postload1199.i

postload1199.i:                                   ; preds = %preload1198.i, %postload1196.i
  %phi1200.i = phi double [ undef, %postload1196.i ], [ %vload603.i, %preload1198.i ]
  br i1 %extract205.i, label %preload912.i, label %postload913.i

preload912.i:                                     ; preds = %postload1199.i
  %.sum1442.i = add i64 %extract231.i, 11
  %88 = getelementptr double addrspace(1)* %1, i64 %.sum1442.i
  %vload607.i = load double addrspace(1)* %88, align 8
  br label %postload913.i

postload913.i:                                    ; preds = %preload912.i, %postload1199.i
  %phi914.i = phi double [ undef, %postload1199.i ], [ %vload607.i, %preload912.i ]
  br i1 %extract206.i, label %preload927.i, label %postload928.i

preload927.i:                                     ; preds = %postload913.i
  %.sum1441.i = add i64 %extract231.i, 12
  %89 = getelementptr double addrspace(1)* %1, i64 %.sum1441.i
  %vload611.i = load double addrspace(1)* %89, align 8
  br label %postload928.i

postload928.i:                                    ; preds = %preload927.i, %postload913.i
  %phi929.i = phi double [ undef, %postload913.i ], [ %vload611.i, %preload927.i ]
  br i1 %extract207.i, label %preload930.i, label %postload931.i

preload930.i:                                     ; preds = %postload928.i
  %.sum1440.i = add i64 %extract231.i, 13
  %90 = getelementptr double addrspace(1)* %1, i64 %.sum1440.i
  %vload615.i = load double addrspace(1)* %90, align 8
  br label %postload931.i

postload931.i:                                    ; preds = %preload930.i, %postload928.i
  %phi932.i = phi double [ undef, %postload928.i ], [ %vload615.i, %preload930.i ]
  br i1 %extract208.i, label %preload906.i, label %postload907.i

preload906.i:                                     ; preds = %postload931.i
  %.sum1439.i = add i64 %extract231.i, 14
  %91 = getelementptr double addrspace(1)* %1, i64 %.sum1439.i
  %vload619.i = load double addrspace(1)* %91, align 8
  br label %postload907.i

postload907.i:                                    ; preds = %preload906.i, %postload931.i
  %phi908.i = phi double [ undef, %postload931.i ], [ %vload619.i, %preload906.i ]
  br i1 %extract209.i, label %preload909.i, label %postload907.i.postload910.i_crit_edge

postload907.i.postload910.i_crit_edge:            ; preds = %postload907.i
  br label %postload910.i

preload909.i:                                     ; preds = %postload907.i
  %.sum1438.i = add i64 %extract231.i, 15
  %92 = getelementptr double addrspace(1)* %1, i64 %.sum1438.i
  %vload623.i = load double addrspace(1)* %92, align 8
  br label %postload910.i

postload910.i:                                    ; preds = %postload907.i.postload910.i_crit_edge, %preload909.i
  %phi911.i = phi double [ %vload623.i, %preload909.i ], [ undef, %postload907.i.postload910.i_crit_edge ]
  %loadedValue1547.i = load i64* %CastToValueType1532.i, align 8
  %arrayidx51.i = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %loadedValue1547.i, i64 17
  br i1 %extract194.i, label %preload962.i, label %postload963.i

preload962.i:                                     ; preds = %postload910.i
  store double %phi1181.i, double addrspace(3)* %arrayidx51.i, align 8
  br label %postload963.i

postload963.i:                                    ; preds = %preload962.i, %postload910.i
  br i1 %extract195.i, label %preload966.i, label %postload967.i

preload966.i:                                     ; preds = %postload963.i
  store double %phi917.i, double addrspace(3)* %arrayidx51.i, align 8
  br label %postload967.i

postload967.i:                                    ; preds = %preload966.i, %postload963.i
  br i1 %extract196.i, label %preload970.i, label %postload971.i

preload970.i:                                     ; preds = %postload967.i
  store double %phi902.i, double addrspace(3)* %arrayidx51.i, align 8
  br label %postload971.i

postload971.i:                                    ; preds = %preload970.i, %postload967.i
  br i1 %extract197.i, label %preload974.i, label %postload975.i

preload974.i:                                     ; preds = %postload971.i
  store double %phi1184.i, double addrspace(3)* %arrayidx51.i, align 8
  br label %postload975.i

postload975.i:                                    ; preds = %preload974.i, %postload971.i
  br i1 %extract198.i, label %preload978.i, label %postload979.i

preload978.i:                                     ; preds = %postload975.i
  store double %phi1175.i, double addrspace(3)* %arrayidx51.i, align 8
  br label %postload979.i

postload979.i:                                    ; preds = %preload978.i, %postload975.i
  br i1 %extract199.i, label %preload982.i, label %postload983.i

preload982.i:                                     ; preds = %postload979.i
  store double %phi923.i, double addrspace(3)* %arrayidx51.i, align 8
  br label %postload983.i

postload983.i:                                    ; preds = %preload982.i, %postload979.i
  br i1 %extract200.i, label %preload986.i, label %postload987.i

preload986.i:                                     ; preds = %postload983.i
  store double %phi926.i, double addrspace(3)* %arrayidx51.i, align 8
  br label %postload987.i

postload987.i:                                    ; preds = %preload986.i, %postload983.i
  br i1 %extract201.i, label %preload990.i, label %postload991.i

preload990.i:                                     ; preds = %postload987.i
  store double %phi1203.i, double addrspace(3)* %arrayidx51.i, align 8
  br label %postload991.i

postload991.i:                                    ; preds = %preload990.i, %postload987.i
  br i1 %extract202.i, label %preload994.i, label %postload995.i

preload994.i:                                     ; preds = %postload991.i
  store double %phi920.i, double addrspace(3)* %arrayidx51.i, align 8
  br label %postload995.i

postload995.i:                                    ; preds = %preload994.i, %postload991.i
  br i1 %extract203.i, label %preload998.i, label %postload999.i

preload998.i:                                     ; preds = %postload995.i
  store double %phi1197.i, double addrspace(3)* %arrayidx51.i, align 8
  br label %postload999.i

postload999.i:                                    ; preds = %preload998.i, %postload995.i
  br i1 %extract204.i, label %preload1002.i, label %postload1003.i

preload1002.i:                                    ; preds = %postload999.i
  store double %phi1200.i, double addrspace(3)* %arrayidx51.i, align 8
  br label %postload1003.i

postload1003.i:                                   ; preds = %preload1002.i, %postload999.i
  br i1 %extract205.i, label %preload1006.i, label %postload1007.i

preload1006.i:                                    ; preds = %postload1003.i
  store double %phi914.i, double addrspace(3)* %arrayidx51.i, align 8
  br label %postload1007.i

postload1007.i:                                   ; preds = %preload1006.i, %postload1003.i
  br i1 %extract206.i, label %preload1010.i, label %postload1011.i

preload1010.i:                                    ; preds = %postload1007.i
  store double %phi929.i, double addrspace(3)* %arrayidx51.i, align 8
  br label %postload1011.i

postload1011.i:                                   ; preds = %preload1010.i, %postload1007.i
  br i1 %extract207.i, label %preload1014.i, label %postload1015.i

preload1014.i:                                    ; preds = %postload1011.i
  store double %phi932.i, double addrspace(3)* %arrayidx51.i, align 8
  br label %postload1015.i

postload1015.i:                                   ; preds = %preload1014.i, %postload1011.i
  br i1 %extract208.i, label %preload1018.i, label %postload1019.i

preload1018.i:                                    ; preds = %postload1015.i
  store double %phi908.i, double addrspace(3)* %arrayidx51.i, align 8
  br label %postload1019.i

postload1019.i:                                   ; preds = %preload1018.i, %postload1015.i
  br i1 %extract209.i, label %preload1022.i, label %if.end52.i

preload1022.i:                                    ; preds = %postload1019.i
  store double %phi911.i, double addrspace(3)* %arrayidx51.i, align 8
  br label %if.end52.i

if.end52.i:                                       ; preds = %preload1022.i, %postload1019.i
  %93 = or <16 x i64> %49, %vector265.i
  %94 = trunc <16 x i64> %93 to <16 x i32>
  %95 = icmp eq <16 x i32> %94, zeroinitializer
  %extract289.i = extractelement <16 x i1> %95, i32 0
  %extract290.i = extractelement <16 x i1> %95, i32 1
  %extract291.i = extractelement <16 x i1> %95, i32 2
  %extract292.i = extractelement <16 x i1> %95, i32 3
  %extract293.i = extractelement <16 x i1> %95, i32 4
  %extract294.i = extractelement <16 x i1> %95, i32 5
  %extract295.i = extractelement <16 x i1> %95, i32 6
  %extract296.i = extractelement <16 x i1> %95, i32 7
  %extract297.i = extractelement <16 x i1> %95, i32 8
  %extract298.i = extractelement <16 x i1> %95, i32 9
  %extract299.i = extractelement <16 x i1> %95, i32 10
  %extract300.i = extractelement <16 x i1> %95, i32 11
  %extract301.i = extractelement <16 x i1> %95, i32 12
  %extract302.i = extractelement <16 x i1> %95, i32 13
  %extract303.i = extractelement <16 x i1> %95, i32 14
  %extract304.i = extractelement <16 x i1> %95, i32 15
  %mul.i15.i = mul nsw i32 %add1.i3945.i, %add.i.i
  %96 = add i32 %.lhs1376.i, %mul.i15.i
  %extract272.i = sext i32 %96 to i64
  br i1 %extract289.i, label %preload1036.i, label %postload1037.i

preload1036.i:                                    ; preds = %if.end52.i
  %97 = getelementptr inbounds double addrspace(1)* %1, i64 %extract272.i
  %vload628.i = load double addrspace(1)* %97, align 8
  br label %postload1037.i

postload1037.i:                                   ; preds = %preload1036.i, %if.end52.i
  %phi1038.i = phi double [ undef, %if.end52.i ], [ %vload628.i, %preload1036.i ]
  br i1 %extract290.i, label %preload1033.i, label %postload1034.i

preload1033.i:                                    ; preds = %postload1037.i
  %.sum1437.i = add i64 %extract272.i, 1
  %98 = getelementptr double addrspace(1)* %1, i64 %.sum1437.i
  %vload632.i = load double addrspace(1)* %98, align 8
  br label %postload1034.i

postload1034.i:                                   ; preds = %preload1033.i, %postload1037.i
  %phi1035.i = phi double [ undef, %postload1037.i ], [ %vload632.i, %preload1033.i ]
  br i1 %extract291.i, label %preload1216.i, label %postload1217.i

preload1216.i:                                    ; preds = %postload1034.i
  %.sum1436.i = add i64 %extract272.i, 2
  %99 = getelementptr double addrspace(1)* %1, i64 %.sum1436.i
  %vload636.i = load double addrspace(1)* %99, align 8
  br label %postload1217.i

postload1217.i:                                   ; preds = %preload1216.i, %postload1034.i
  %phi1218.i = phi double [ undef, %postload1034.i ], [ %vload636.i, %preload1216.i ]
  br i1 %extract292.i, label %preload1219.i, label %postload1220.i

preload1219.i:                                    ; preds = %postload1217.i
  %.sum1435.i = add i64 %extract272.i, 3
  %100 = getelementptr double addrspace(1)* %1, i64 %.sum1435.i
  %vload640.i = load double addrspace(1)* %100, align 8
  br label %postload1220.i

postload1220.i:                                   ; preds = %preload1219.i, %postload1217.i
  %phi1221.i = phi double [ undef, %postload1217.i ], [ %vload640.i, %preload1219.i ]
  br i1 %extract293.i, label %preload1288.i, label %postload1289.i

preload1288.i:                                    ; preds = %postload1220.i
  %.sum1434.i = add i64 %extract272.i, 4
  %101 = getelementptr double addrspace(1)* %1, i64 %.sum1434.i
  %vload644.i = load double addrspace(1)* %101, align 8
  br label %postload1289.i

postload1289.i:                                   ; preds = %preload1288.i, %postload1220.i
  %phi1290.i = phi double [ undef, %postload1220.i ], [ %vload644.i, %preload1288.i ]
  br i1 %extract294.i, label %preload1291.i, label %postload1292.i

preload1291.i:                                    ; preds = %postload1289.i
  %.sum1433.i = add i64 %extract272.i, 5
  %102 = getelementptr double addrspace(1)* %1, i64 %.sum1433.i
  %vload648.i = load double addrspace(1)* %102, align 8
  br label %postload1292.i

postload1292.i:                                   ; preds = %preload1291.i, %postload1289.i
  %phi1293.i = phi double [ undef, %postload1289.i ], [ %vload648.i, %preload1291.i ]
  br i1 %extract295.i, label %preload1279.i, label %postload1280.i

preload1279.i:                                    ; preds = %postload1292.i
  %.sum1432.i = add i64 %extract272.i, 6
  %103 = getelementptr double addrspace(1)* %1, i64 %.sum1432.i
  %vload652.i = load double addrspace(1)* %103, align 8
  br label %postload1280.i

postload1280.i:                                   ; preds = %preload1279.i, %postload1292.i
  %phi1281.i = phi double [ undef, %postload1292.i ], [ %vload652.i, %preload1279.i ]
  br i1 %extract296.i, label %preload1282.i, label %postload1283.i

preload1282.i:                                    ; preds = %postload1280.i
  %.sum1431.i = add i64 %extract272.i, 7
  %104 = getelementptr double addrspace(1)* %1, i64 %.sum1431.i
  %vload656.i = load double addrspace(1)* %104, align 8
  br label %postload1283.i

postload1283.i:                                   ; preds = %preload1282.i, %postload1280.i
  %phi1284.i = phi double [ undef, %postload1280.i ], [ %vload656.i, %preload1282.i ]
  br i1 %extract297.i, label %preload1285.i, label %postload1286.i

preload1285.i:                                    ; preds = %postload1283.i
  %.sum1430.i = add i64 %extract272.i, 8
  %105 = getelementptr double addrspace(1)* %1, i64 %.sum1430.i
  %vload660.i = load double addrspace(1)* %105, align 8
  br label %postload1286.i

postload1286.i:                                   ; preds = %preload1285.i, %postload1283.i
  %phi1287.i = phi double [ undef, %postload1283.i ], [ %vload660.i, %preload1285.i ]
  br i1 %extract298.i, label %preload1039.i, label %postload1040.i

preload1039.i:                                    ; preds = %postload1286.i
  %.sum1429.i = add i64 %extract272.i, 9
  %106 = getelementptr double addrspace(1)* %1, i64 %.sum1429.i
  %vload664.i = load double addrspace(1)* %106, align 8
  br label %postload1040.i

postload1040.i:                                   ; preds = %preload1039.i, %postload1286.i
  %phi1041.i = phi double [ undef, %postload1286.i ], [ %vload664.i, %preload1039.i ]
  br i1 %extract299.i, label %preload1042.i, label %postload1043.i

preload1042.i:                                    ; preds = %postload1040.i
  %.sum1428.i = add i64 %extract272.i, 10
  %107 = getelementptr double addrspace(1)* %1, i64 %.sum1428.i
  %vload668.i = load double addrspace(1)* %107, align 8
  br label %postload1043.i

postload1043.i:                                   ; preds = %preload1042.i, %postload1040.i
  %phi1044.i = phi double [ undef, %postload1040.i ], [ %vload668.i, %preload1042.i ]
  br i1 %extract300.i, label %preload1024.i, label %postload1025.i

preload1024.i:                                    ; preds = %postload1043.i
  %.sum1427.i = add i64 %extract272.i, 11
  %108 = getelementptr double addrspace(1)* %1, i64 %.sum1427.i
  %vload672.i = load double addrspace(1)* %108, align 8
  br label %postload1025.i

postload1025.i:                                   ; preds = %preload1024.i, %postload1043.i
  %phi1026.i = phi double [ undef, %postload1043.i ], [ %vload672.i, %preload1024.i ]
  br i1 %extract301.i, label %preload1027.i, label %postload1028.i

preload1027.i:                                    ; preds = %postload1025.i
  %.sum1426.i = add i64 %extract272.i, 12
  %109 = getelementptr double addrspace(1)* %1, i64 %.sum1426.i
  %vload676.i = load double addrspace(1)* %109, align 8
  br label %postload1028.i

postload1028.i:                                   ; preds = %preload1027.i, %postload1025.i
  %phi1029.i = phi double [ undef, %postload1025.i ], [ %vload676.i, %preload1027.i ]
  br i1 %extract302.i, label %preload1030.i, label %postload1031.i

preload1030.i:                                    ; preds = %postload1028.i
  %.sum1425.i = add i64 %extract272.i, 13
  %110 = getelementptr double addrspace(1)* %1, i64 %.sum1425.i
  %vload680.i = load double addrspace(1)* %110, align 8
  br label %postload1031.i

postload1031.i:                                   ; preds = %preload1030.i, %postload1028.i
  %phi1032.i = phi double [ undef, %postload1028.i ], [ %vload680.i, %preload1030.i ]
  br i1 %extract303.i, label %preload945.i, label %postload946.i

preload945.i:                                     ; preds = %postload1031.i
  %.sum1424.i = add i64 %extract272.i, 14
  %111 = getelementptr double addrspace(1)* %1, i64 %.sum1424.i
  %vload684.i = load double addrspace(1)* %111, align 8
  br label %postload946.i

postload946.i:                                    ; preds = %preload945.i, %postload1031.i
  %phi947.i = phi double [ undef, %postload1031.i ], [ %vload684.i, %preload945.i ]
  br i1 %extract304.i, label %preload948.i, label %postload949.i

preload948.i:                                     ; preds = %postload946.i
  %.sum1423.i = add i64 %extract272.i, 15
  %112 = getelementptr double addrspace(1)* %1, i64 %.sum1423.i
  %vload688.i = load double addrspace(1)* %112, align 8
  br label %postload949.i

postload949.i:                                    ; preds = %preload948.i, %postload946.i
  %phi950.i = phi double [ undef, %postload946.i ], [ %vload688.i, %preload948.i ]
  br i1 %extract289.i, label %preload1045.i, label %postload1046.i

preload1045.i:                                    ; preds = %postload949.i
  store double %phi1038.i, double addrspace(3)* %33, align 16
  br label %postload1046.i

postload1046.i:                                   ; preds = %preload1045.i, %postload949.i
  br i1 %extract290.i, label %preload1053.i, label %postload1054.i

preload1053.i:                                    ; preds = %postload1046.i
  store double %phi1035.i, double addrspace(3)* %33, align 16
  br label %postload1054.i

postload1054.i:                                   ; preds = %preload1053.i, %postload1046.i
  br i1 %extract291.i, label %preload1061.i, label %postload1062.i

preload1061.i:                                    ; preds = %postload1054.i
  store double %phi1218.i, double addrspace(3)* %33, align 16
  br label %postload1062.i

postload1062.i:                                   ; preds = %preload1061.i, %postload1054.i
  br i1 %extract292.i, label %preload1069.i, label %postload1070.i

preload1069.i:                                    ; preds = %postload1062.i
  store double %phi1221.i, double addrspace(3)* %33, align 16
  br label %postload1070.i

postload1070.i:                                   ; preds = %preload1069.i, %postload1062.i
  br i1 %extract293.i, label %preload1077.i, label %postload1078.i

preload1077.i:                                    ; preds = %postload1070.i
  store double %phi1290.i, double addrspace(3)* %33, align 16
  br label %postload1078.i

postload1078.i:                                   ; preds = %preload1077.i, %postload1070.i
  br i1 %extract294.i, label %preload1085.i, label %postload1086.i

preload1085.i:                                    ; preds = %postload1078.i
  store double %phi1293.i, double addrspace(3)* %33, align 16
  br label %postload1086.i

postload1086.i:                                   ; preds = %preload1085.i, %postload1078.i
  br i1 %extract295.i, label %preload1093.i, label %postload1094.i

preload1093.i:                                    ; preds = %postload1086.i
  store double %phi1281.i, double addrspace(3)* %33, align 16
  br label %postload1094.i

postload1094.i:                                   ; preds = %preload1093.i, %postload1086.i
  br i1 %extract296.i, label %preload1101.i, label %postload1102.i

preload1101.i:                                    ; preds = %postload1094.i
  store double %phi1284.i, double addrspace(3)* %33, align 16
  br label %postload1102.i

postload1102.i:                                   ; preds = %preload1101.i, %postload1094.i
  br i1 %extract297.i, label %preload1109.i, label %postload1110.i

preload1109.i:                                    ; preds = %postload1102.i
  store double %phi1287.i, double addrspace(3)* %33, align 16
  br label %postload1110.i

postload1110.i:                                   ; preds = %preload1109.i, %postload1102.i
  br i1 %extract298.i, label %preload1117.i, label %postload1118.i

preload1117.i:                                    ; preds = %postload1110.i
  store double %phi1041.i, double addrspace(3)* %33, align 16
  br label %postload1118.i

postload1118.i:                                   ; preds = %preload1117.i, %postload1110.i
  br i1 %extract299.i, label %preload1125.i, label %postload1126.i

preload1125.i:                                    ; preds = %postload1118.i
  store double %phi1044.i, double addrspace(3)* %33, align 16
  br label %postload1126.i

postload1126.i:                                   ; preds = %preload1125.i, %postload1118.i
  br i1 %extract300.i, label %preload1133.i, label %postload1134.i

preload1133.i:                                    ; preds = %postload1126.i
  store double %phi1026.i, double addrspace(3)* %33, align 16
  br label %postload1134.i

postload1134.i:                                   ; preds = %preload1133.i, %postload1126.i
  br i1 %extract301.i, label %preload1141.i, label %postload1142.i

preload1141.i:                                    ; preds = %postload1134.i
  store double %phi1029.i, double addrspace(3)* %33, align 16
  br label %postload1142.i

postload1142.i:                                   ; preds = %preload1141.i, %postload1134.i
  br i1 %extract302.i, label %preload1149.i, label %postload1150.i

preload1149.i:                                    ; preds = %postload1142.i
  store double %phi1032.i, double addrspace(3)* %33, align 16
  br label %postload1150.i

postload1150.i:                                   ; preds = %preload1149.i, %postload1142.i
  br i1 %extract303.i, label %preload1157.i, label %postload1158.i

preload1157.i:                                    ; preds = %postload1150.i
  store double %phi947.i, double addrspace(3)* %33, align 16
  br label %postload1158.i

postload1158.i:                                   ; preds = %preload1157.i, %postload1150.i
  br i1 %extract304.i, label %preload1165.i, label %postload1166.i

preload1165.i:                                    ; preds = %postload1158.i
  store double %phi950.i, double addrspace(3)* %33, align 16
  br label %postload1166.i

postload1166.i:                                   ; preds = %preload1165.i, %postload1158.i
  %add.i8.i = add nsw i32 %add.i.i, 17
  %mul.i10.i = mul nsw i32 %add1.i3945.i, %add.i8.i
  %113 = add i32 %.lhs1376.i, %mul.i10.i
  %extract326.i = sext i32 %113 to i64
  br i1 %extract289.i, label %preload1258.i, label %postload1259.i

preload1258.i:                                    ; preds = %postload1166.i
  %114 = getelementptr inbounds double addrspace(1)* %1, i64 %extract326.i
  %vload693.i = load double addrspace(1)* %114, align 8
  br label %postload1259.i

postload1259.i:                                   ; preds = %preload1258.i, %postload1166.i
  %phi1260.i = phi double [ undef, %postload1166.i ], [ %vload693.i, %preload1258.i ]
  br i1 %extract290.i, label %preload1231.i, label %postload1232.i

preload1231.i:                                    ; preds = %postload1259.i
  %.sum1422.i = add i64 %extract326.i, 1
  %115 = getelementptr double addrspace(1)* %1, i64 %.sum1422.i
  %vload697.i = load double addrspace(1)* %115, align 8
  br label %postload1232.i

postload1232.i:                                   ; preds = %preload1231.i, %postload1259.i
  %phi1233.i = phi double [ undef, %postload1259.i ], [ %vload697.i, %preload1231.i ]
  br i1 %extract291.i, label %preload1234.i, label %postload1235.i

preload1234.i:                                    ; preds = %postload1232.i
  %.sum1421.i = add i64 %extract326.i, 2
  %116 = getelementptr double addrspace(1)* %1, i64 %.sum1421.i
  %vload701.i = load double addrspace(1)* %116, align 8
  br label %postload1235.i

postload1235.i:                                   ; preds = %preload1234.i, %postload1232.i
  %phi1236.i = phi double [ undef, %postload1232.i ], [ %vload701.i, %preload1234.i ]
  br i1 %extract292.i, label %preload1237.i, label %postload1238.i

preload1237.i:                                    ; preds = %postload1235.i
  %.sum1420.i = add i64 %extract326.i, 3
  %117 = getelementptr double addrspace(1)* %1, i64 %.sum1420.i
  %vload705.i = load double addrspace(1)* %117, align 8
  br label %postload1238.i

postload1238.i:                                   ; preds = %preload1237.i, %postload1235.i
  %phi1239.i = phi double [ undef, %postload1235.i ], [ %vload705.i, %preload1237.i ]
  br i1 %extract293.i, label %preload1363.i, label %postload1364.i

preload1363.i:                                    ; preds = %postload1238.i
  %.sum1419.i = add i64 %extract326.i, 4
  %118 = getelementptr double addrspace(1)* %1, i64 %.sum1419.i
  %vload709.i = load double addrspace(1)* %118, align 8
  br label %postload1364.i

postload1364.i:                                   ; preds = %preload1363.i, %postload1238.i
  %phi1365.i = phi double [ undef, %postload1238.i ], [ %vload709.i, %preload1363.i ]
  br i1 %extract294.i, label %preload1366.i, label %postload1367.i

preload1366.i:                                    ; preds = %postload1364.i
  %.sum1418.i = add i64 %extract326.i, 5
  %119 = getelementptr double addrspace(1)* %1, i64 %.sum1418.i
  %vload713.i = load double addrspace(1)* %119, align 8
  br label %postload1367.i

postload1367.i:                                   ; preds = %preload1366.i, %postload1364.i
  %phi1368.i = phi double [ undef, %postload1364.i ], [ %vload713.i, %preload1366.i ]
  br i1 %extract295.i, label %preload1243.i, label %postload1244.i

preload1243.i:                                    ; preds = %postload1367.i
  %.sum1417.i = add i64 %extract326.i, 6
  %120 = getelementptr double addrspace(1)* %1, i64 %.sum1417.i
  %vload717.i = load double addrspace(1)* %120, align 8
  br label %postload1244.i

postload1244.i:                                   ; preds = %preload1243.i, %postload1367.i
  %phi1245.i = phi double [ undef, %postload1367.i ], [ %vload717.i, %preload1243.i ]
  br i1 %extract296.i, label %preload1246.i, label %postload1247.i

preload1246.i:                                    ; preds = %postload1244.i
  %.sum1416.i = add i64 %extract326.i, 7
  %121 = getelementptr double addrspace(1)* %1, i64 %.sum1416.i
  %vload721.i = load double addrspace(1)* %121, align 8
  br label %postload1247.i

postload1247.i:                                   ; preds = %preload1246.i, %postload1244.i
  %phi1248.i = phi double [ undef, %postload1244.i ], [ %vload721.i, %preload1246.i ]
  br i1 %extract297.i, label %preload1249.i, label %postload1250.i

preload1249.i:                                    ; preds = %postload1247.i
  %.sum1415.i = add i64 %extract326.i, 8
  %122 = getelementptr double addrspace(1)* %1, i64 %.sum1415.i
  %vload725.i = load double addrspace(1)* %122, align 8
  br label %postload1250.i

postload1250.i:                                   ; preds = %preload1249.i, %postload1247.i
  %phi1251.i = phi double [ undef, %postload1247.i ], [ %vload725.i, %preload1249.i ]
  br i1 %extract298.i, label %preload1252.i, label %postload1253.i

preload1252.i:                                    ; preds = %postload1250.i
  %.sum1414.i = add i64 %extract326.i, 9
  %123 = getelementptr double addrspace(1)* %1, i64 %.sum1414.i
  %vload729.i = load double addrspace(1)* %123, align 8
  br label %postload1253.i

postload1253.i:                                   ; preds = %preload1252.i, %postload1250.i
  %phi1254.i = phi double [ undef, %postload1250.i ], [ %vload729.i, %preload1252.i ]
  br i1 %extract299.i, label %preload1255.i, label %postload1256.i

preload1255.i:                                    ; preds = %postload1253.i
  %.sum1413.i = add i64 %extract326.i, 10
  %124 = getelementptr double addrspace(1)* %1, i64 %.sum1413.i
  %vload733.i = load double addrspace(1)* %124, align 8
  br label %postload1256.i

postload1256.i:                                   ; preds = %preload1255.i, %postload1253.i
  %phi1257.i = phi double [ undef, %postload1253.i ], [ %vload733.i, %preload1255.i ]
  br i1 %extract300.i, label %preload1261.i, label %postload1262.i

preload1261.i:                                    ; preds = %postload1256.i
  %.sum1412.i = add i64 %extract326.i, 11
  %125 = getelementptr double addrspace(1)* %1, i64 %.sum1412.i
  %vload737.i = load double addrspace(1)* %125, align 8
  br label %postload1262.i

postload1262.i:                                   ; preds = %preload1261.i, %postload1256.i
  %phi1263.i = phi double [ undef, %postload1256.i ], [ %vload737.i, %preload1261.i ]
  br i1 %extract301.i, label %preload1264.i, label %postload1265.i

preload1264.i:                                    ; preds = %postload1262.i
  %.sum1411.i = add i64 %extract326.i, 12
  %126 = getelementptr double addrspace(1)* %1, i64 %.sum1411.i
  %vload741.i = load double addrspace(1)* %126, align 8
  br label %postload1265.i

postload1265.i:                                   ; preds = %preload1264.i, %postload1262.i
  %phi1266.i = phi double [ undef, %postload1262.i ], [ %vload741.i, %preload1264.i ]
  br i1 %extract302.i, label %preload1267.i, label %postload1268.i

preload1267.i:                                    ; preds = %postload1265.i
  %.sum1410.i = add i64 %extract326.i, 13
  %127 = getelementptr double addrspace(1)* %1, i64 %.sum1410.i
  %vload745.i = load double addrspace(1)* %127, align 8
  br label %postload1268.i

postload1268.i:                                   ; preds = %preload1267.i, %postload1265.i
  %phi1269.i = phi double [ undef, %postload1265.i ], [ %vload745.i, %preload1267.i ]
  br i1 %extract303.i, label %preload1270.i, label %postload1271.i

preload1270.i:                                    ; preds = %postload1268.i
  %.sum1409.i = add i64 %extract326.i, 14
  %128 = getelementptr double addrspace(1)* %1, i64 %.sum1409.i
  %vload749.i = load double addrspace(1)* %128, align 8
  br label %postload1271.i

postload1271.i:                                   ; preds = %preload1270.i, %postload1268.i
  %phi1272.i = phi double [ undef, %postload1268.i ], [ %vload749.i, %preload1270.i ]
  br i1 %extract304.i, label %preload1354.i, label %postload1355.i

preload1354.i:                                    ; preds = %postload1271.i
  %.sum1408.i = add i64 %extract326.i, 15
  %129 = getelementptr double addrspace(1)* %1, i64 %.sum1408.i
  %vload753.i = load double addrspace(1)* %129, align 8
  br label %postload1355.i

postload1355.i:                                   ; preds = %preload1354.i, %postload1271.i
  %phi1356.i = phi double [ undef, %postload1271.i ], [ %vload753.i, %preload1354.i ]
  br i1 %extract289.i, label %preload1047.i, label %postload1048.i

preload1047.i:                                    ; preds = %postload1355.i
  store double %phi1260.i, double addrspace(3)* %35, align 16
  br label %postload1048.i

postload1048.i:                                   ; preds = %preload1047.i, %postload1355.i
  br i1 %extract290.i, label %preload1055.i, label %postload1056.i

preload1055.i:                                    ; preds = %postload1048.i
  store double %phi1233.i, double addrspace(3)* %35, align 16
  br label %postload1056.i

postload1056.i:                                   ; preds = %preload1055.i, %postload1048.i
  br i1 %extract291.i, label %preload1063.i, label %postload1064.i

preload1063.i:                                    ; preds = %postload1056.i
  store double %phi1236.i, double addrspace(3)* %35, align 16
  br label %postload1064.i

postload1064.i:                                   ; preds = %preload1063.i, %postload1056.i
  br i1 %extract292.i, label %preload1071.i, label %postload1072.i

preload1071.i:                                    ; preds = %postload1064.i
  store double %phi1239.i, double addrspace(3)* %35, align 16
  br label %postload1072.i

postload1072.i:                                   ; preds = %preload1071.i, %postload1064.i
  br i1 %extract293.i, label %preload1079.i, label %postload1080.i

preload1079.i:                                    ; preds = %postload1072.i
  store double %phi1365.i, double addrspace(3)* %35, align 16
  br label %postload1080.i

postload1080.i:                                   ; preds = %preload1079.i, %postload1072.i
  br i1 %extract294.i, label %preload1087.i, label %postload1088.i

preload1087.i:                                    ; preds = %postload1080.i
  store double %phi1368.i, double addrspace(3)* %35, align 16
  br label %postload1088.i

postload1088.i:                                   ; preds = %preload1087.i, %postload1080.i
  br i1 %extract295.i, label %preload1095.i, label %postload1096.i

preload1095.i:                                    ; preds = %postload1088.i
  store double %phi1245.i, double addrspace(3)* %35, align 16
  br label %postload1096.i

postload1096.i:                                   ; preds = %preload1095.i, %postload1088.i
  br i1 %extract296.i, label %preload1103.i, label %postload1104.i

preload1103.i:                                    ; preds = %postload1096.i
  store double %phi1248.i, double addrspace(3)* %35, align 16
  br label %postload1104.i

postload1104.i:                                   ; preds = %preload1103.i, %postload1096.i
  br i1 %extract297.i, label %preload1111.i, label %postload1112.i

preload1111.i:                                    ; preds = %postload1104.i
  store double %phi1251.i, double addrspace(3)* %35, align 16
  br label %postload1112.i

postload1112.i:                                   ; preds = %preload1111.i, %postload1104.i
  br i1 %extract298.i, label %preload1119.i, label %postload1120.i

preload1119.i:                                    ; preds = %postload1112.i
  store double %phi1254.i, double addrspace(3)* %35, align 16
  br label %postload1120.i

postload1120.i:                                   ; preds = %preload1119.i, %postload1112.i
  br i1 %extract299.i, label %preload1127.i, label %postload1128.i

preload1127.i:                                    ; preds = %postload1120.i
  store double %phi1257.i, double addrspace(3)* %35, align 16
  br label %postload1128.i

postload1128.i:                                   ; preds = %preload1127.i, %postload1120.i
  br i1 %extract300.i, label %preload1135.i, label %postload1136.i

preload1135.i:                                    ; preds = %postload1128.i
  store double %phi1263.i, double addrspace(3)* %35, align 16
  br label %postload1136.i

postload1136.i:                                   ; preds = %preload1135.i, %postload1128.i
  br i1 %extract301.i, label %preload1143.i, label %postload1144.i

preload1143.i:                                    ; preds = %postload1136.i
  store double %phi1266.i, double addrspace(3)* %35, align 16
  br label %postload1144.i

postload1144.i:                                   ; preds = %preload1143.i, %postload1136.i
  br i1 %extract302.i, label %preload1151.i, label %postload1152.i

preload1151.i:                                    ; preds = %postload1144.i
  store double %phi1269.i, double addrspace(3)* %35, align 16
  br label %postload1152.i

postload1152.i:                                   ; preds = %preload1151.i, %postload1144.i
  br i1 %extract303.i, label %preload1159.i, label %postload1160.i

preload1159.i:                                    ; preds = %postload1152.i
  store double %phi1272.i, double addrspace(3)* %35, align 16
  br label %postload1160.i

postload1160.i:                                   ; preds = %preload1159.i, %postload1152.i
  br i1 %extract304.i, label %preload1167.i, label %postload1168.i

preload1167.i:                                    ; preds = %postload1160.i
  store double %phi1356.i, double addrspace(3)* %35, align 16
  br label %postload1168.i

postload1168.i:                                   ; preds = %preload1167.i, %postload1160.i
  %130 = add i32 %.lhs1373.i, %mul.i15.i
  %extract364.i = sext i32 %130 to i64
  br i1 %extract289.i, label %preload1276.i, label %postload1277.i

preload1276.i:                                    ; preds = %postload1168.i
  %131 = getelementptr inbounds double addrspace(1)* %1, i64 %extract364.i
  %vload758.i = load double addrspace(1)* %131, align 8
  br label %postload1277.i

postload1277.i:                                   ; preds = %preload1276.i, %postload1168.i
  %phi1278.i = phi double [ undef, %postload1168.i ], [ %vload758.i, %preload1276.i ]
  br i1 %extract290.i, label %preload1357.i, label %postload1358.i

preload1357.i:                                    ; preds = %postload1277.i
  %.sum1407.i = add i64 %extract364.i, 1
  %132 = getelementptr double addrspace(1)* %1, i64 %.sum1407.i
  %vload762.i = load double addrspace(1)* %132, align 8
  br label %postload1358.i

postload1358.i:                                   ; preds = %preload1357.i, %postload1277.i
  %phi1359.i = phi double [ undef, %postload1277.i ], [ %vload762.i, %preload1357.i ]
  br i1 %extract291.i, label %preload1360.i, label %postload1361.i

preload1360.i:                                    ; preds = %postload1358.i
  %.sum1406.i = add i64 %extract364.i, 2
  %133 = getelementptr double addrspace(1)* %1, i64 %.sum1406.i
  %vload766.i = load double addrspace(1)* %133, align 8
  br label %postload1361.i

postload1361.i:                                   ; preds = %preload1360.i, %postload1358.i
  %phi1362.i = phi double [ undef, %postload1358.i ], [ %vload766.i, %preload1360.i ]
  br i1 %extract292.i, label %preload1345.i, label %postload1346.i

preload1345.i:                                    ; preds = %postload1361.i
  %.sum1405.i = add i64 %extract364.i, 3
  %134 = getelementptr double addrspace(1)* %1, i64 %.sum1405.i
  %vload770.i = load double addrspace(1)* %134, align 8
  br label %postload1346.i

postload1346.i:                                   ; preds = %preload1345.i, %postload1361.i
  %phi1347.i = phi double [ undef, %postload1361.i ], [ %vload770.i, %preload1345.i ]
  br i1 %extract293.i, label %preload1348.i, label %postload1349.i

preload1348.i:                                    ; preds = %postload1346.i
  %.sum1404.i = add i64 %extract364.i, 4
  %135 = getelementptr double addrspace(1)* %1, i64 %.sum1404.i
  %vload774.i = load double addrspace(1)* %135, align 8
  br label %postload1349.i

postload1349.i:                                   ; preds = %preload1348.i, %postload1346.i
  %phi1350.i = phi double [ undef, %postload1346.i ], [ %vload774.i, %preload1348.i ]
  br i1 %extract294.i, label %preload1351.i, label %postload1352.i

preload1351.i:                                    ; preds = %postload1349.i
  %.sum1403.i = add i64 %extract364.i, 5
  %136 = getelementptr double addrspace(1)* %1, i64 %.sum1403.i
  %vload778.i = load double addrspace(1)* %136, align 8
  br label %postload1352.i

postload1352.i:                                   ; preds = %preload1351.i, %postload1349.i
  %phi1353.i = phi double [ undef, %postload1349.i ], [ %vload778.i, %preload1351.i ]
  br i1 %extract295.i, label %preload1333.i, label %postload1334.i

preload1333.i:                                    ; preds = %postload1352.i
  %.sum1402.i = add i64 %extract364.i, 6
  %137 = getelementptr double addrspace(1)* %1, i64 %.sum1402.i
  %vload782.i = load double addrspace(1)* %137, align 8
  br label %postload1334.i

postload1334.i:                                   ; preds = %preload1333.i, %postload1352.i
  %phi1335.i = phi double [ undef, %postload1352.i ], [ %vload782.i, %preload1333.i ]
  br i1 %extract296.i, label %preload1336.i, label %postload1337.i

preload1336.i:                                    ; preds = %postload1334.i
  %.sum1401.i = add i64 %extract364.i, 7
  %138 = getelementptr double addrspace(1)* %1, i64 %.sum1401.i
  %vload786.i = load double addrspace(1)* %138, align 8
  br label %postload1337.i

postload1337.i:                                   ; preds = %preload1336.i, %postload1334.i
  %phi1338.i = phi double [ undef, %postload1334.i ], [ %vload786.i, %preload1336.i ]
  br i1 %extract297.i, label %preload1339.i, label %postload1340.i

preload1339.i:                                    ; preds = %postload1337.i
  %.sum1400.i = add i64 %extract364.i, 8
  %139 = getelementptr double addrspace(1)* %1, i64 %.sum1400.i
  %vload790.i = load double addrspace(1)* %139, align 8
  br label %postload1340.i

postload1340.i:                                   ; preds = %preload1339.i, %postload1337.i
  %phi1341.i = phi double [ undef, %postload1337.i ], [ %vload790.i, %preload1339.i ]
  br i1 %extract298.i, label %preload1342.i, label %postload1343.i

preload1342.i:                                    ; preds = %postload1340.i
  %.sum1399.i = add i64 %extract364.i, 9
  %140 = getelementptr double addrspace(1)* %1, i64 %.sum1399.i
  %vload794.i = load double addrspace(1)* %140, align 8
  br label %postload1343.i

postload1343.i:                                   ; preds = %preload1342.i, %postload1340.i
  %phi1344.i = phi double [ undef, %postload1340.i ], [ %vload794.i, %preload1342.i ]
  br i1 %extract299.i, label %preload1324.i, label %postload1325.i

preload1324.i:                                    ; preds = %postload1343.i
  %.sum1398.i = add i64 %extract364.i, 10
  %141 = getelementptr double addrspace(1)* %1, i64 %.sum1398.i
  %vload798.i = load double addrspace(1)* %141, align 8
  br label %postload1325.i

postload1325.i:                                   ; preds = %preload1324.i, %postload1343.i
  %phi1326.i = phi double [ undef, %postload1343.i ], [ %vload798.i, %preload1324.i ]
  br i1 %extract300.i, label %preload1327.i, label %postload1328.i

preload1327.i:                                    ; preds = %postload1325.i
  %.sum1397.i = add i64 %extract364.i, 11
  %142 = getelementptr double addrspace(1)* %1, i64 %.sum1397.i
  %vload802.i = load double addrspace(1)* %142, align 8
  br label %postload1328.i

postload1328.i:                                   ; preds = %preload1327.i, %postload1325.i
  %phi1329.i = phi double [ undef, %postload1325.i ], [ %vload802.i, %preload1327.i ]
  br i1 %extract301.i, label %preload1330.i, label %postload1331.i

preload1330.i:                                    ; preds = %postload1328.i
  %.sum1396.i = add i64 %extract364.i, 12
  %143 = getelementptr double addrspace(1)* %1, i64 %.sum1396.i
  %vload806.i = load double addrspace(1)* %143, align 8
  br label %postload1331.i

postload1331.i:                                   ; preds = %preload1330.i, %postload1328.i
  %phi1332.i = phi double [ undef, %postload1328.i ], [ %vload806.i, %preload1330.i ]
  br i1 %extract302.i, label %preload1315.i, label %postload1316.i

preload1315.i:                                    ; preds = %postload1331.i
  %.sum1395.i = add i64 %extract364.i, 13
  %144 = getelementptr double addrspace(1)* %1, i64 %.sum1395.i
  %vload810.i = load double addrspace(1)* %144, align 8
  br label %postload1316.i

postload1316.i:                                   ; preds = %preload1315.i, %postload1331.i
  %phi1317.i = phi double [ undef, %postload1331.i ], [ %vload810.i, %preload1315.i ]
  br i1 %extract303.i, label %preload1318.i, label %postload1319.i

preload1318.i:                                    ; preds = %postload1316.i
  %.sum1394.i = add i64 %extract364.i, 14
  %145 = getelementptr double addrspace(1)* %1, i64 %.sum1394.i
  %vload814.i = load double addrspace(1)* %145, align 8
  br label %postload1319.i

postload1319.i:                                   ; preds = %preload1318.i, %postload1316.i
  %phi1320.i = phi double [ undef, %postload1316.i ], [ %vload814.i, %preload1318.i ]
  br i1 %extract304.i, label %preload1321.i, label %postload1322.i

preload1321.i:                                    ; preds = %postload1319.i
  %.sum1393.i = add i64 %extract364.i, 15
  %146 = getelementptr double addrspace(1)* %1, i64 %.sum1393.i
  %vload818.i = load double addrspace(1)* %146, align 8
  br label %postload1322.i

postload1322.i:                                   ; preds = %preload1321.i, %postload1319.i
  %phi1323.i = phi double [ undef, %postload1319.i ], [ %vload818.i, %preload1321.i ]
  br i1 %extract289.i, label %preload1049.i, label %postload1050.i

preload1049.i:                                    ; preds = %postload1322.i
  store double %phi1278.i, double addrspace(3)* %37, align 8
  br label %postload1050.i

postload1050.i:                                   ; preds = %preload1049.i, %postload1322.i
  br i1 %extract290.i, label %preload1057.i, label %postload1058.i

preload1057.i:                                    ; preds = %postload1050.i
  store double %phi1359.i, double addrspace(3)* %37, align 8
  br label %postload1058.i

postload1058.i:                                   ; preds = %preload1057.i, %postload1050.i
  br i1 %extract291.i, label %preload1065.i, label %postload1066.i

preload1065.i:                                    ; preds = %postload1058.i
  store double %phi1362.i, double addrspace(3)* %37, align 8
  br label %postload1066.i

postload1066.i:                                   ; preds = %preload1065.i, %postload1058.i
  br i1 %extract292.i, label %preload1073.i, label %postload1074.i

preload1073.i:                                    ; preds = %postload1066.i
  store double %phi1347.i, double addrspace(3)* %37, align 8
  br label %postload1074.i

postload1074.i:                                   ; preds = %preload1073.i, %postload1066.i
  br i1 %extract293.i, label %preload1081.i, label %postload1082.i

preload1081.i:                                    ; preds = %postload1074.i
  store double %phi1350.i, double addrspace(3)* %37, align 8
  br label %postload1082.i

postload1082.i:                                   ; preds = %preload1081.i, %postload1074.i
  br i1 %extract294.i, label %preload1089.i, label %postload1090.i

preload1089.i:                                    ; preds = %postload1082.i
  store double %phi1353.i, double addrspace(3)* %37, align 8
  br label %postload1090.i

postload1090.i:                                   ; preds = %preload1089.i, %postload1082.i
  br i1 %extract295.i, label %preload1097.i, label %postload1098.i

preload1097.i:                                    ; preds = %postload1090.i
  store double %phi1335.i, double addrspace(3)* %37, align 8
  br label %postload1098.i

postload1098.i:                                   ; preds = %preload1097.i, %postload1090.i
  br i1 %extract296.i, label %preload1105.i, label %postload1106.i

preload1105.i:                                    ; preds = %postload1098.i
  store double %phi1338.i, double addrspace(3)* %37, align 8
  br label %postload1106.i

postload1106.i:                                   ; preds = %preload1105.i, %postload1098.i
  br i1 %extract297.i, label %preload1113.i, label %postload1114.i

preload1113.i:                                    ; preds = %postload1106.i
  store double %phi1341.i, double addrspace(3)* %37, align 8
  br label %postload1114.i

postload1114.i:                                   ; preds = %preload1113.i, %postload1106.i
  br i1 %extract298.i, label %preload1121.i, label %postload1122.i

preload1121.i:                                    ; preds = %postload1114.i
  store double %phi1344.i, double addrspace(3)* %37, align 8
  br label %postload1122.i

postload1122.i:                                   ; preds = %preload1121.i, %postload1114.i
  br i1 %extract299.i, label %preload1129.i, label %postload1130.i

preload1129.i:                                    ; preds = %postload1122.i
  store double %phi1326.i, double addrspace(3)* %37, align 8
  br label %postload1130.i

postload1130.i:                                   ; preds = %preload1129.i, %postload1122.i
  br i1 %extract300.i, label %preload1137.i, label %postload1138.i

preload1137.i:                                    ; preds = %postload1130.i
  store double %phi1329.i, double addrspace(3)* %37, align 8
  br label %postload1138.i

postload1138.i:                                   ; preds = %preload1137.i, %postload1130.i
  br i1 %extract301.i, label %preload1145.i, label %postload1146.i

preload1145.i:                                    ; preds = %postload1138.i
  store double %phi1332.i, double addrspace(3)* %37, align 8
  br label %postload1146.i

postload1146.i:                                   ; preds = %preload1145.i, %postload1138.i
  br i1 %extract302.i, label %preload1153.i, label %postload1154.i

preload1153.i:                                    ; preds = %postload1146.i
  store double %phi1317.i, double addrspace(3)* %37, align 8
  br label %postload1154.i

postload1154.i:                                   ; preds = %preload1153.i, %postload1146.i
  br i1 %extract303.i, label %preload1161.i, label %postload1162.i

preload1161.i:                                    ; preds = %postload1154.i
  store double %phi1320.i, double addrspace(3)* %37, align 8
  br label %postload1162.i

postload1162.i:                                   ; preds = %preload1161.i, %postload1154.i
  br i1 %extract304.i, label %preload1169.i, label %postload1170.i

preload1169.i:                                    ; preds = %postload1162.i
  store double %phi1323.i, double addrspace(3)* %37, align 8
  br label %postload1170.i

postload1170.i:                                   ; preds = %preload1169.i, %postload1162.i
  %147 = add i32 %.lhs1373.i, %mul.i10.i
  %extract402.i = sext i32 %147 to i64
  br i1 %extract289.i, label %preload1306.i, label %postload1307.i

preload1306.i:                                    ; preds = %postload1170.i
  %148 = getelementptr inbounds double addrspace(1)* %1, i64 %extract402.i
  %vload823.i = load double addrspace(1)* %148, align 8
  br label %postload1307.i

postload1307.i:                                   ; preds = %preload1306.i, %postload1170.i
  %phi1308.i = phi double [ undef, %postload1170.i ], [ %vload823.i, %preload1306.i ]
  br i1 %extract290.i, label %preload1309.i, label %postload1310.i

preload1309.i:                                    ; preds = %postload1307.i
  %.sum1392.i = add i64 %extract402.i, 1
  %149 = getelementptr double addrspace(1)* %1, i64 %.sum1392.i
  %vload827.i = load double addrspace(1)* %149, align 8
  br label %postload1310.i

postload1310.i:                                   ; preds = %preload1309.i, %postload1307.i
  %phi1311.i = phi double [ undef, %postload1307.i ], [ %vload827.i, %preload1309.i ]
  br i1 %extract291.i, label %preload1312.i, label %postload1313.i

preload1312.i:                                    ; preds = %postload1310.i
  %.sum1391.i = add i64 %extract402.i, 2
  %150 = getelementptr double addrspace(1)* %1, i64 %.sum1391.i
  %vload831.i = load double addrspace(1)* %150, align 8
  br label %postload1313.i

postload1313.i:                                   ; preds = %preload1312.i, %postload1310.i
  %phi1314.i = phi double [ undef, %postload1310.i ], [ %vload831.i, %preload1312.i ]
  br i1 %extract292.i, label %preload1294.i, label %postload1295.i

preload1294.i:                                    ; preds = %postload1313.i
  %.sum1390.i = add i64 %extract402.i, 3
  %151 = getelementptr double addrspace(1)* %1, i64 %.sum1390.i
  %vload835.i = load double addrspace(1)* %151, align 8
  br label %postload1295.i

postload1295.i:                                   ; preds = %preload1294.i, %postload1313.i
  %phi1296.i = phi double [ undef, %postload1313.i ], [ %vload835.i, %preload1294.i ]
  br i1 %extract293.i, label %preload1297.i, label %postload1298.i

preload1297.i:                                    ; preds = %postload1295.i
  %.sum1389.i = add i64 %extract402.i, 4
  %152 = getelementptr double addrspace(1)* %1, i64 %.sum1389.i
  %vload839.i = load double addrspace(1)* %152, align 8
  br label %postload1298.i

postload1298.i:                                   ; preds = %preload1297.i, %postload1295.i
  %phi1299.i = phi double [ undef, %postload1295.i ], [ %vload839.i, %preload1297.i ]
  br i1 %extract294.i, label %preload1300.i, label %postload1301.i

preload1300.i:                                    ; preds = %postload1298.i
  %.sum1388.i = add i64 %extract402.i, 5
  %153 = getelementptr double addrspace(1)* %1, i64 %.sum1388.i
  %vload843.i = load double addrspace(1)* %153, align 8
  br label %postload1301.i

postload1301.i:                                   ; preds = %preload1300.i, %postload1298.i
  %phi1302.i = phi double [ undef, %postload1298.i ], [ %vload843.i, %preload1300.i ]
  br i1 %extract295.i, label %preload1303.i, label %postload1304.i

preload1303.i:                                    ; preds = %postload1301.i
  %.sum1387.i = add i64 %extract402.i, 6
  %154 = getelementptr double addrspace(1)* %1, i64 %.sum1387.i
  %vload847.i = load double addrspace(1)* %154, align 8
  br label %postload1304.i

postload1304.i:                                   ; preds = %preload1303.i, %postload1301.i
  %phi1305.i = phi double [ undef, %postload1301.i ], [ %vload847.i, %preload1303.i ]
  br i1 %extract296.i, label %preload933.i, label %postload934.i

preload933.i:                                     ; preds = %postload1304.i
  %.sum1386.i = add i64 %extract402.i, 7
  %155 = getelementptr double addrspace(1)* %1, i64 %.sum1386.i
  %vload851.i = load double addrspace(1)* %155, align 8
  br label %postload934.i

postload934.i:                                    ; preds = %preload933.i, %postload1304.i
  %phi935.i = phi double [ undef, %postload1304.i ], [ %vload851.i, %preload933.i ]
  br i1 %extract297.i, label %preload936.i, label %postload937.i

preload936.i:                                     ; preds = %postload934.i
  %.sum1385.i = add i64 %extract402.i, 8
  %156 = getelementptr double addrspace(1)* %1, i64 %.sum1385.i
  %vload855.i = load double addrspace(1)* %156, align 8
  br label %postload937.i

postload937.i:                                    ; preds = %preload936.i, %postload934.i
  %phi938.i = phi double [ undef, %postload934.i ], [ %vload855.i, %preload936.i ]
  br i1 %extract298.i, label %preload939.i, label %postload940.i

preload939.i:                                     ; preds = %postload937.i
  %.sum1384.i = add i64 %extract402.i, 9
  %157 = getelementptr double addrspace(1)* %1, i64 %.sum1384.i
  %vload859.i = load double addrspace(1)* %157, align 8
  br label %postload940.i

postload940.i:                                    ; preds = %preload939.i, %postload937.i
  %phi941.i = phi double [ undef, %postload937.i ], [ %vload859.i, %preload939.i ]
  br i1 %extract299.i, label %preload1222.i, label %postload1223.i

preload1222.i:                                    ; preds = %postload940.i
  %.sum1383.i = add i64 %extract402.i, 10
  %158 = getelementptr double addrspace(1)* %1, i64 %.sum1383.i
  %vload863.i = load double addrspace(1)* %158, align 8
  br label %postload1223.i

postload1223.i:                                   ; preds = %preload1222.i, %postload940.i
  %phi1224.i = phi double [ undef, %postload940.i ], [ %vload863.i, %preload1222.i ]
  br i1 %extract300.i, label %preload1225.i, label %postload1226.i

preload1225.i:                                    ; preds = %postload1223.i
  %.sum1382.i = add i64 %extract402.i, 11
  %159 = getelementptr double addrspace(1)* %1, i64 %.sum1382.i
  %vload867.i = load double addrspace(1)* %159, align 8
  br label %postload1226.i

postload1226.i:                                   ; preds = %preload1225.i, %postload1223.i
  %phi1227.i = phi double [ undef, %postload1223.i ], [ %vload867.i, %preload1225.i ]
  br i1 %extract301.i, label %preload1228.i, label %postload1229.i

preload1228.i:                                    ; preds = %postload1226.i
  %.sum1381.i = add i64 %extract402.i, 12
  %160 = getelementptr double addrspace(1)* %1, i64 %.sum1381.i
  %vload871.i = load double addrspace(1)* %160, align 8
  br label %postload1229.i

postload1229.i:                                   ; preds = %preload1228.i, %postload1226.i
  %phi1230.i = phi double [ undef, %postload1226.i ], [ %vload871.i, %preload1228.i ]
  br i1 %extract302.i, label %preload951.i, label %postload952.i

preload951.i:                                     ; preds = %postload1229.i
  %.sum1380.i = add i64 %extract402.i, 13
  %161 = getelementptr double addrspace(1)* %1, i64 %.sum1380.i
  %vload875.i = load double addrspace(1)* %161, align 8
  br label %postload952.i

postload952.i:                                    ; preds = %preload951.i, %postload1229.i
  %phi953.i = phi double [ undef, %postload1229.i ], [ %vload875.i, %preload951.i ]
  br i1 %extract303.i, label %preload954.i, label %postload955.i

preload954.i:                                     ; preds = %postload952.i
  %.sum1379.i = add i64 %extract402.i, 14
  %162 = getelementptr double addrspace(1)* %1, i64 %.sum1379.i
  %vload879.i = load double addrspace(1)* %162, align 8
  br label %postload955.i

postload955.i:                                    ; preds = %preload954.i, %postload952.i
  %phi956.i = phi double [ undef, %postload952.i ], [ %vload879.i, %preload954.i ]
  br i1 %extract304.i, label %preload957.i, label %postload958.i

preload957.i:                                     ; preds = %postload955.i
  %.sum.i = add i64 %extract402.i, 15
  %163 = getelementptr double addrspace(1)* %1, i64 %.sum.i
  %vload883.i = load double addrspace(1)* %163, align 8
  br label %postload958.i

postload958.i:                                    ; preds = %preload957.i, %postload955.i
  %phi959.i = phi double [ undef, %postload955.i ], [ %vload883.i, %preload957.i ]
  br i1 %extract289.i, label %preload1051.i, label %postload1052.i

preload1051.i:                                    ; preds = %postload958.i
  store double %phi1308.i, double addrspace(3)* %39, align 8
  br label %postload1052.i

postload1052.i:                                   ; preds = %preload1051.i, %postload958.i
  br i1 %extract290.i, label %preload1059.i, label %postload1060.i

preload1059.i:                                    ; preds = %postload1052.i
  store double %phi1311.i, double addrspace(3)* %39, align 8
  br label %postload1060.i

postload1060.i:                                   ; preds = %preload1059.i, %postload1052.i
  br i1 %extract291.i, label %preload1067.i, label %postload1068.i

preload1067.i:                                    ; preds = %postload1060.i
  store double %phi1314.i, double addrspace(3)* %39, align 8
  br label %postload1068.i

postload1068.i:                                   ; preds = %preload1067.i, %postload1060.i
  br i1 %extract292.i, label %preload1075.i, label %postload1076.i

preload1075.i:                                    ; preds = %postload1068.i
  store double %phi1296.i, double addrspace(3)* %39, align 8
  br label %postload1076.i

postload1076.i:                                   ; preds = %preload1075.i, %postload1068.i
  br i1 %extract293.i, label %preload1083.i, label %postload1084.i

preload1083.i:                                    ; preds = %postload1076.i
  store double %phi1299.i, double addrspace(3)* %39, align 8
  br label %postload1084.i

postload1084.i:                                   ; preds = %preload1083.i, %postload1076.i
  br i1 %extract294.i, label %preload1091.i, label %postload1092.i

preload1091.i:                                    ; preds = %postload1084.i
  store double %phi1302.i, double addrspace(3)* %39, align 8
  br label %postload1092.i

postload1092.i:                                   ; preds = %preload1091.i, %postload1084.i
  br i1 %extract295.i, label %preload1099.i, label %postload1100.i

preload1099.i:                                    ; preds = %postload1092.i
  store double %phi1305.i, double addrspace(3)* %39, align 8
  br label %postload1100.i

postload1100.i:                                   ; preds = %preload1099.i, %postload1092.i
  br i1 %extract296.i, label %preload1107.i, label %postload1108.i

preload1107.i:                                    ; preds = %postload1100.i
  store double %phi935.i, double addrspace(3)* %39, align 8
  br label %postload1108.i

postload1108.i:                                   ; preds = %preload1107.i, %postload1100.i
  br i1 %extract297.i, label %preload1115.i, label %postload1116.i

preload1115.i:                                    ; preds = %postload1108.i
  store double %phi938.i, double addrspace(3)* %39, align 8
  br label %postload1116.i

postload1116.i:                                   ; preds = %preload1115.i, %postload1108.i
  br i1 %extract298.i, label %preload1123.i, label %postload1124.i

preload1123.i:                                    ; preds = %postload1116.i
  store double %phi941.i, double addrspace(3)* %39, align 8
  br label %postload1124.i

postload1124.i:                                   ; preds = %preload1123.i, %postload1116.i
  br i1 %extract299.i, label %preload1131.i, label %postload1132.i

preload1131.i:                                    ; preds = %postload1124.i
  store double %phi1224.i, double addrspace(3)* %39, align 8
  br label %postload1132.i

postload1132.i:                                   ; preds = %preload1131.i, %postload1124.i
  br i1 %extract300.i, label %preload1139.i, label %postload1140.i

preload1139.i:                                    ; preds = %postload1132.i
  store double %phi1227.i, double addrspace(3)* %39, align 8
  br label %postload1140.i

postload1140.i:                                   ; preds = %preload1139.i, %postload1132.i
  br i1 %extract301.i, label %preload1147.i, label %postload1148.i

preload1147.i:                                    ; preds = %postload1140.i
  store double %phi1230.i, double addrspace(3)* %39, align 8
  br label %postload1148.i

postload1148.i:                                   ; preds = %preload1147.i, %postload1140.i
  br i1 %extract302.i, label %preload1155.i, label %postload1156.i

preload1155.i:                                    ; preds = %postload1148.i
  store double %phi953.i, double addrspace(3)* %39, align 8
  br label %postload1156.i

postload1156.i:                                   ; preds = %preload1155.i, %postload1148.i
  br i1 %extract303.i, label %preload1163.i, label %postload1164.i

preload1163.i:                                    ; preds = %postload1156.i
  store double %phi956.i, double addrspace(3)* %39, align 8
  br label %postload1164.i

postload1164.i:                                   ; preds = %preload1163.i, %postload1156.i
  br i1 %extract304.i, label %preload1171.i, label %if.end78.i

preload1171.i:                                    ; preds = %postload1164.i
  store double %phi959.i, double addrspace(3)* %39, align 8
  br label %if.end78.i

if.end78.i:                                       ; preds = %preload1171.i, %postload1164.i
  %check.WI.iter1574.i = icmp ult i64 %CurrWI..0.i, %28
  br i1 %check.WI.iter1574.i, label %thenBB1571.i, label %elseBB1572.i

thenBB1571.i:                                     ; preds = %if.end78.i
  %"CurrWI++1575.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride1577.i" = add nuw i64 %CurrSBIndex..0.i, 192
  br label %SyncBB.i

elseBB1572.i:                                     ; preds = %if.end78.i
  %temp492.i = insertelement <16 x double> undef, double %13, i32 0
  %vector493.i = shufflevector <16 x double> %temp492.i, <16 x double> undef, <16 x i32> zeroinitializer
  %temp488.i = insertelement <16 x double> undef, double %10, i32 0
  %vector489.i = shufflevector <16 x double> %temp488.i, <16 x double> undef, <16 x i32> zeroinitializer
  %temp485.i = insertelement <16 x double> undef, double %7, i32 0
  %vector486.i = shufflevector <16 x double> %temp485.i, <16 x double> undef, <16 x i32> zeroinitializer
  br label %SyncBB1569.i

SyncBB1569.i:                                     ; preds = %thenBB.i, %elseBB1572.i
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB1572.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB1572.i ], [ %"CurrWI++.i", %thenBB.i ]
  %"&(pSB[currWI].offset)1558.i" = add nuw i64 %CurrSBIndex..1.i, 152
  %"&pSB[currWI].offset1559.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1558.i"
  %CastToValueType1560.i = bitcast i8* %"&pSB[currWI].offset1559.i" to <16 x double> addrspace(3)**
  %loadedValue1561.i = load <16 x double> addrspace(3)** %CastToValueType1560.i, align 8
  %164 = load <16 x double> addrspace(3)* %loadedValue1561.i, align 8
  %"&pSB[currWI].offset1474.i" = getelementptr inbounds i8* %31, i64 %CurrSBIndex..1.i
  %CastToValueType1475.i = bitcast i8* %"&pSB[currWI].offset1474.i" to i32*
  %loadedValue1476.i = load i32* %CastToValueType1475.i, align 4
  %idxprom87.i = sext i32 %loadedValue1476.i to i64
  %"&(pSB[currWI].offset)1515.i" = add nuw i64 %CurrSBIndex..1.i, 136
  %"&pSB[currWI].offset1516.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1515.i"
  %CastToValueType1517.i = bitcast i8* %"&pSB[currWI].offset1516.i" to i64*
  %loadedValue1518.i = load i64* %CastToValueType1517.i, align 8
  %165 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %idxprom87.i, i64 %loadedValue1518.i
  %ptrTypeCast436.i = bitcast double addrspace(3)* %165 to <16 x double> addrspace(3)*
  %166 = load <16 x double> addrspace(3)* %ptrTypeCast436.i, align 8
  %add92.i = add nsw i32 %loadedValue1476.i, 2
  %idxprom93.i = sext i32 %add92.i to i64
  %167 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %idxprom93.i, i64 %loadedValue1518.i
  %ptrTypeCast437.i = bitcast double addrspace(3)* %167 to <16 x double> addrspace(3)*
  %168 = load <16 x double> addrspace(3)* %ptrTypeCast437.i, align 8
  %add96438.i = fadd <16 x double> %166, %168
  %"&(pSB[currWI].offset)1487.i" = add nuw i64 %CurrSBIndex..1.i, 64
  %"&pSB[currWI].offset1488.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1487.i"
  %CastToValueType1489.i = bitcast i8* %"&pSB[currWI].offset1488.i" to <16 x i32>*
  %loadedValue1490.i = load <16 x i32>* %CastToValueType1489.i, align 64
  %idxprom97439.i = sext <16 x i32> %loadedValue1490.i to <16 x i64>
  %extract440.i = extractelement <16 x i64> %idxprom97439.i, i32 0
  %"&(pSB[currWI].offset)1539.i" = add nuw i64 %CurrSBIndex..1.i, 144
  %"&pSB[currWI].offset1540.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1539.i"
  %CastToValueType1541.i = bitcast i8* %"&pSB[currWI].offset1540.i" to i64*
  %loadedValue1542.i = load i64* %CastToValueType1541.i, align 8
  %169 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %loadedValue1542.i, i64 %extract440.i
  %ptrTypeCast456.i = bitcast double addrspace(3)* %169 to <16 x double> addrspace(3)*
  %170 = load <16 x double> addrspace(3)* %ptrTypeCast456.i, align 8
  %add102457.i = fadd <16 x double> %add96438.i, %170
  %add103458.i = add nsw <16 x i32> %loadedValue1490.i, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %idxprom104459.i = sext <16 x i32> %add103458.i to <16 x i64>
  %extract460.i = extractelement <16 x i64> %idxprom104459.i, i32 0
  %171 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %loadedValue1542.i, i64 %extract460.i
  %ptrTypeCast476.i = bitcast double addrspace(3)* %171 to <16 x double> addrspace(3)*
  %172 = load <16 x double> addrspace(3)* %ptrTypeCast476.i, align 8
  %add109477.i = fadd <16 x double> %add102457.i, %172
  %173 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %idxprom87.i, i64 %extract440.i
  %ptrTypeCast478.i = bitcast double addrspace(3)* %173 to <16 x double> addrspace(3)*
  %174 = load <16 x double> addrspace(3)* %ptrTypeCast478.i, align 8
  %175 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %idxprom87.i, i64 %extract460.i
  %ptrTypeCast479.i = bitcast double addrspace(3)* %175 to <16 x double> addrspace(3)*
  %176 = load <16 x double> addrspace(3)* %ptrTypeCast479.i, align 8
  %add119480.i = fadd <16 x double> %174, %176
  %177 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %idxprom93.i, i64 %extract440.i
  %ptrTypeCast481.i = bitcast double addrspace(3)* %177 to <16 x double> addrspace(3)*
  %178 = load <16 x double> addrspace(3)* %ptrTypeCast481.i, align 8
  %add125482.i = fadd <16 x double> %add119480.i, %178
  %179 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %idxprom93.i, i64 %extract460.i
  %ptrTypeCast483.i = bitcast double addrspace(3)* %179 to <16 x double> addrspace(3)*
  %180 = load <16 x double> addrspace(3)* %ptrTypeCast483.i, align 8
  %add132484.i = fadd <16 x double> %add125482.i, %180
  %mul133487.i = fmul <16 x double> %164, %vector486.i
  %mul134490.i = fmul <16 x double> %add109477.i, %vector489.i
  %add135491.i = fadd <16 x double> %mul133487.i, %mul134490.i
  %mul136494.i = fmul <16 x double> %add132484.i, %vector493.i
  %add137495.i = fadd <16 x double> %add135491.i, %mul136494.i
  %"&(pSB[currWI].offset)1501.i" = add nuw i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset1502.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1501.i"
  %CastToValueType1503.i = bitcast i8* %"&pSB[currWI].offset1502.i" to i64*
  %loadedValue1504.i = load i64* %CastToValueType1503.i, align 8
  %181 = getelementptr inbounds double addrspace(1)* %4, i64 %loadedValue1504.i
  %ptrTypeCast496.i = bitcast double addrspace(1)* %181 to <16 x double> addrspace(1)*
  store <16 x double> %add137495.i, <16 x double> addrspace(1)* %ptrTypeCast496.i, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..1.i, %28
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.StencilKernel_separated_args.exit

thenBB.i:                                         ; preds = %SyncBB1569.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..1.i, 192
  br label %SyncBB1569.i

____Vectorized_.StencilKernel_separated_args.exit: ; preds = %SyncBB1569.i
  ret void
}

!opencl.kernels = !{!0, !2}
!opencl.build.options = !{!4}
!cl.noBarrierPath.kernels = !{!5}
!opencl.wrappers = !{!6, !7}

!0 = metadata !{void (double addrspace(1)*, i32, i32, double addrspace(1)*, i32, i32, i32, i32, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__CopyRect_separated_args, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3}
!2 = metadata !{void (double addrspace(1)*, double addrspace(1)*, double, double, double, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__StencilKernel_separated_args, metadata !3}
!3 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3, i32 3}
!4 = metadata !{}
!5 = metadata !{metadata !"CopyRect"}
!6 = metadata !{void (i8*)* @CopyRect}
!7 = metadata !{void (i8*)* @StencilKernel}
