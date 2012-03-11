; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@bottom_scan.s_seed = internal addrspace(3) unnamed_addr global double 0.000000e+00, align 8

declare void @__reduce_original(double addrspace(1)* nocapture, double addrspace(1)* nocapture, i32, double addrspace(3)* nocapture) nounwind

declare i64 @get_num_groups(i32) nounwind readnone

declare i64 @get_group_id(i32) nounwind readnone

declare i64 @get_local_id(i32) nounwind readnone

declare i64 @get_local_size(i32) nounwind readnone

declare void @barrier(i64)

declare void @__top_scan_original(double addrspace(1)* nocapture, i32, double addrspace(3)* nocapture) nounwind

declare void @__bottom_scan_original(double addrspace(1)* nocapture, double addrspace(1)* nocapture, double addrspace(1)* nocapture, i32, double addrspace(3)* nocapture) nounwind

declare void @____Vectorized_.reduce_original(double addrspace(1)* nocapture, double addrspace(1)* nocapture, i32, double addrspace(3)* nocapture) nounwind

declare void @____Vectorized_.top_scan_original(double addrspace(1)* nocapture, i32, double addrspace(3)* nocapture) nounwind

declare void @____Vectorized_.bottom_scan_original(double addrspace(1)* nocapture, double addrspace(1)* nocapture, double addrspace(1)* nocapture, i32, double addrspace(3)* nocapture) nounwind

declare i1 @allOne(i1)

declare i1 @allZero(i1)

declare double @masked_load_align8_26(i1, double addrspace(1)*)

declare double @masked_load_align8_27(i1, double addrspace(3)*)

declare double @masked_load_align8_28(i1, double addrspace(3)*)

declare void @masked_store_align8_17(i1, double, double addrspace(3)*)

declare void @maskedf_10_barrier(i1, i64)

declare double @masked_load_align8_29(i1, double addrspace(3)*)

declare void @masked_store_align8_18(i1, double, double addrspace(1)*)

declare i1 @allZero_v16(<16 x i1>)

declare i1 @allOne_v16(<16 x i1>)

declare <16 x double> @masked_load_align8_32(<16 x i1>, <16 x double> addrspace(3)*)

declare void @masked_store_align8_19(<16 x i1>, <16 x double>, <16 x double> addrspace(3)*)

declare double @masked_load_align8_34(i1, double addrspace(1)*)

declare double @masked_load_align8_35(i1, double addrspace(3)*)

declare void @maskedf_11_barrier(i1, i64)

declare double @masked_load_align8_36(i1, double addrspace(3)*)

declare void @masked_store_align8_21(i1, double, double addrspace(3)*)

declare void @maskedf_12_barrier(i1, i64)

declare double @masked_load_align8_37(i1, double addrspace(3)*)

declare void @masked_store_align8_22(i1, double, double addrspace(1)*)

declare <16 x double> @masked_load_align8_38(<16 x i1>, <16 x double> addrspace(1)*)

declare <16 x double> @masked_load_align8_39(i1, <16 x double> addrspace(3)*)

declare <16 x double> @masked_load_align8_40(i1, <16 x double> addrspace(3)*)

declare void @masked_store_align8_23(i1, <16 x double>, <16 x double> addrspace(3)*)

declare <16 x double> @masked_load_align8_41(<16 x i1>, <16 x double> addrspace(3)*)

declare void @masked_store_align8_24(<16 x i1>, <16 x double>, <16 x double> addrspace(1)*)

declare double @masked_load_align8_42(i1, double addrspace(1)*)

declare i64 @maskedf_13_get_local_size(i1, i32)

declare <4 x double> @masked_load_align32_43(i1, <4 x double> addrspace(1)*)

declare i64 @maskedf_14_get_local_id(i1, i32)

declare void @masked_store_align8_25(i1, double, double addrspace(3)*)

declare i64 @maskedf_15_get_local_size(i1, i32)

declare void @masked_store_align8_26(i1, double, double addrspace(3)*)

declare void @maskedf_16_barrier(i1, i64)

declare double @masked_load_align8_44(i1, double addrspace(3)*)

declare void @maskedf_17_barrier(i1, i64)

declare double @masked_load_align8_45(i1, double addrspace(3)*)

declare void @masked_store_align8_27(i1, double, double addrspace(3)*)

declare void @maskedf_18_barrier(i1, i64)

declare double @masked_load_align8_46(i1, double addrspace(3)*)

declare void @masked_store_align32_28(i1, <4 x double>, <4 x double> addrspace(1)*)

declare void @masked_store_align8_29(i1, double, double addrspace(3)*)

declare void @maskedf_19_barrier(i1, i64)

declare double @masked_load_align8_47(i1, double addrspace(3)*)

declare void @masked_store_align8_30(<16 x i1>, <16 x double>, <16 x double> addrspace(3)*)

declare void @dummybarrier.()

declare i8* @get_special_buffer.() nounwind readnone

declare i64 @get_iter_count.() nounwind readnone

declare i64 @get_new_local_id.(i32, i64) nounwind readnone

declare void @__reduce_separated_args(double addrspace(1)* nocapture, double addrspace(1)* nocapture, i32, double addrspace(3)* nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare void @__top_scan_separated_args(double addrspace(1)* nocapture, i32, double addrspace(3)* nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare void @__bottom_scan_separated_args(double addrspace(1)* nocapture, double addrspace(1)* nocapture, double addrspace(1)* nocapture, i32, double addrspace(3)* nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare void @____Vectorized_.reduce_separated_args(double addrspace(1)* nocapture, double addrspace(1)* nocapture, i32, double addrspace(3)* nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare void @____Vectorized_.top_scan_separated_args(double addrspace(1)* nocapture, i32, double addrspace(3)* nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare void @____Vectorized_.bottom_scan_separated_args(double addrspace(1)* nocapture, double addrspace(1)* nocapture, double addrspace(1)* nocapture, i32, double addrspace(3)* nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare i32 @llvm.x86.mic.kortestc(i16, i16) nounwind readnone

declare i32 @llvm.x86.mic.kortestz(i16, i16) nounwind readnone

define void @top_scan(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32*
  %4 = load i32* %3, align 4
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to double addrspace(3)**
  %7 = load double addrspace(3)** %6, align 8
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
  br label %SyncBB81.i

SyncBB81.i:                                       ; preds = %thenBB85.i, %entry
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride91.i", %thenBB85.i ]
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++89.i", %thenBB85.i ]
  %20 = getelementptr <{ [4 x i64] }>* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %21 = load i64* %20, align 8
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i64*
  store i64 %21, i64* %CastToValueType.i, align 8
  %cmp.i = icmp ult i64 %21, %conv.i
  %"&(pSB[currWI].offset)23.i" = add nuw i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset24.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)23.i"
  %CastToValueType25.i = bitcast i8* %"&pSB[currWI].offset24.i" to i1*
  store i1 %cmp.i, i1* %CastToValueType25.i, align 1
  br i1 %cmp.i, label %if.then.i, label %if.end.i

if.then.i:                                        ; preds = %SyncBB81.i
  %arrayidx.i = getelementptr inbounds double addrspace(1)* %1, i64 %21
  %22 = load double addrspace(1)* %arrayidx.i, align 8
  br label %if.end.i

if.end.i:                                         ; preds = %if.then.i, %SyncBB81.i
  %val.0.i = phi double [ %22, %if.then.i ], [ 0.000000e+00, %SyncBB81.i ]
  %sext.i.i = shl i64 %21, 32
  %idxprom.i.i = ashr exact i64 %sext.i.i, 32
  %arrayidx.i.i = getelementptr inbounds double addrspace(3)* %7, i64 %idxprom.i.i
  store double 0.000000e+00, double addrspace(3)* %arrayidx.i.i, align 8
  %23 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %10, i64 0, i32 3, i64 0
  %24 = load i64* %23, align 8
  %loadedValue11.i = load i64* %CastToValueType.i, align 8
  %add.i.i = add i64 %24, %loadedValue11.i
  %conv3.i.i = trunc i64 %add.i.i to i32
  %"&(pSB[currWI].offset)32.i" = add nuw i64 %CurrSBIndex..0.i, 12
  %"&pSB[currWI].offset33.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)32.i"
  %CastToValueType34.i = bitcast i8* %"&pSB[currWI].offset33.i" to i32*
  store i32 %conv3.i.i, i32* %CastToValueType34.i, align 4
  %idxprom4.i.i = sext i32 %conv3.i.i to i64
  %arrayidx5.i.i = getelementptr inbounds double addrspace(3)* %7, i64 %idxprom4.i.i
  %"&(pSB[currWI].offset)46.i" = add nuw i64 %CurrSBIndex..0.i, 16
  %"&pSB[currWI].offset47.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)46.i"
  %CastToValueType48.i = bitcast i8* %"&pSB[currWI].offset47.i" to double addrspace(3)**
  store double addrspace(3)* %arrayidx5.i.i, double addrspace(3)** %CastToValueType48.i, align 8
  store double %val.0.i, double addrspace(3)* %arrayidx5.i.i, align 8
  %check.WI.iter88.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter88.i, label %thenBB85.i, label %elseBB86.i

thenBB85.i:                                       ; preds = %if.end.i
  %"CurrWI++89.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride91.i" = add nuw i64 %CurrSBIndex..0.i, 40
  br label %SyncBB81.i

elseBB86.i:                                       ; preds = %if.end.i
  %cmp1.i.i = icmp ugt i64 %24, 1
  br label %SyncBB80.i

SyncBB80.i:                                       ; preds = %thenBB92.i, %thenBB.i, %elseBB86.i
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB86.i ], [ %"loadedCurrSB+Stride98.i", %thenBB92.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %currBarrier.0.i = phi i32 [ 8, %elseBB86.i ], [ %currBarrier.3.i, %thenBB92.i ], [ %currBarrier.1.i, %thenBB.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB86.i ], [ %"CurrWI++96.i", %thenBB92.i ], [ %"CurrWI++.i", %thenBB.i ]
  br i1 %cmp1.i.i, label %for.body.i.i, label %scanLocalMem.exit.i

for.body.i.i:                                     ; preds = %SyncBB83.i, %SyncBB80.i
  %CurrSBIndex..2.i = phi i64 [ %CurrSBIndex..4.i, %SyncBB83.i ], [ %CurrSBIndex..1.i, %SyncBB80.i ]
  %currBarrier.1.i = phi i32 [ %currBarrier.2.i, %SyncBB83.i ], [ %currBarrier.0.i, %SyncBB80.i ]
  %CurrWI..2.i = phi i64 [ %CurrWI..4.i, %SyncBB83.i ], [ %CurrWI..1.i, %SyncBB80.i ]
  %i.02.i.i = phi i32 [ %mul.i.i, %SyncBB83.i ], [ 1, %SyncBB80.i ]
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
  %arrayidx10.i.i = getelementptr inbounds double addrspace(3)* %7, i64 %idxprom9.i.i
  %25 = load double addrspace(3)* %arrayidx10.i.i, align 8
  %"&(pSB[currWI].offset)69.i" = add nuw i64 %CurrSBIndex..2.i, 32
  %"&pSB[currWI].offset70.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)69.i"
  %CastToValueType71.i = bitcast i8* %"&pSB[currWI].offset70.i" to double*
  store double %25, double* %CastToValueType71.i, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..2.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %for.body.i.i.SyncBB.i_crit_edge

for.body.i.i.SyncBB.i_crit_edge:                  ; preds = %for.body.i.i
  br label %SyncBB.i

thenBB.i:                                         ; preds = %for.body.i.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..2.i, 40
  %cond1.i = icmp eq i32 %currBarrier.1.i, 8
  br i1 %cond1.i, label %SyncBB80.i, label %SyncBB83.i

SyncBB.i:                                         ; preds = %for.body.i.i.SyncBB.i_crit_edge, %thenBB100.i
  %CurrSBIndex..3.i = phi i64 [ %"loadedCurrSB+Stride106.i", %thenBB100.i ], [ 0, %for.body.i.i.SyncBB.i_crit_edge ]
  %CurrWI..3.i = phi i64 [ %"CurrWI++104.i", %thenBB100.i ], [ 0, %for.body.i.i.SyncBB.i_crit_edge ]
  %"&(pSB[currWI].offset)55.i" = add nuw i64 %CurrSBIndex..3.i, 16
  %"&pSB[currWI].offset56.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)55.i"
  %CastToValueType57.i = bitcast i8* %"&pSB[currWI].offset56.i" to double addrspace(3)**
  %loadedValue58.i = load double addrspace(3)** %CastToValueType57.i, align 8
  %26 = load double addrspace(3)* %loadedValue58.i, align 8
  %"&(pSB[currWI].offset)73.i" = add nuw i64 %CurrSBIndex..3.i, 32
  %"&pSB[currWI].offset74.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)73.i"
  %CastToValueType75.i = bitcast i8* %"&pSB[currWI].offset74.i" to double*
  %loadedValue76.i = load double* %CastToValueType75.i, align 8
  %add13.i.i = fadd double %26, %loadedValue76.i
  store double %add13.i.i, double addrspace(3)* %loadedValue58.i, align 8
  %check.WI.iter103.i = icmp ult i64 %CurrWI..3.i, %16
  br i1 %check.WI.iter103.i, label %thenBB100.i, label %SyncBB83.i

thenBB100.i:                                      ; preds = %SyncBB.i
  %"CurrWI++104.i" = add nuw i64 %CurrWI..3.i, 1
  %"loadedCurrSB+Stride106.i" = add nuw i64 %CurrSBIndex..3.i, 40
  br label %SyncBB.i

SyncBB83.i:                                       ; preds = %thenBB92.i, %SyncBB.i, %thenBB.i
  %CurrSBIndex..4.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ %"loadedCurrSB+Stride98.i", %thenBB92.i ], [ 0, %SyncBB.i ]
  %currBarrier.2.i = phi i32 [ %currBarrier.1.i, %thenBB.i ], [ %currBarrier.3.i, %thenBB92.i ], [ 16, %SyncBB.i ]
  %CurrWI..4.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ %"CurrWI++96.i", %thenBB92.i ], [ 0, %SyncBB.i ]
  %"&(pSB[currWI].offset)64.i" = add nuw i64 %CurrSBIndex..4.i, 24
  %"&pSB[currWI].offset65.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)64.i"
  %CastToValueType66.i = bitcast i8* %"&pSB[currWI].offset65.i" to i32*
  %loadedValue67.i = load i32* %CastToValueType66.i, align 4
  %mul.i.i = shl nsw i32 %loadedValue67.i, 1
  %conv6.i.i = sext i32 %mul.i.i to i64
  %cmp.i.i = icmp ult i64 %conv6.i.i, %24
  br i1 %cmp.i.i, label %for.body.i.i, label %scanLocalMem.exit.i

scanLocalMem.exit.i:                              ; preds = %SyncBB83.i, %SyncBB80.i
  %CurrSBIndex..5.i = phi i64 [ %CurrSBIndex..1.i, %SyncBB80.i ], [ %CurrSBIndex..4.i, %SyncBB83.i ]
  %currBarrier.3.i = phi i32 [ %currBarrier.0.i, %SyncBB80.i ], [ %currBarrier.2.i, %SyncBB83.i ]
  %CurrWI..5.i = phi i64 [ %CurrWI..1.i, %SyncBB80.i ], [ %CurrWI..4.i, %SyncBB83.i ]
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
  %arrayidx16.i.i = getelementptr inbounds double addrspace(3)* %7, i64 %idxprom15.i.i
  %27 = load double addrspace(3)* %arrayidx16.i.i, align 8
  %"&pSB[currWI].offset14.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..5.i
  %CastToValueType15.i = bitcast i8* %"&pSB[currWI].offset14.i" to i64*
  %loadedValue16.i = load i64* %CastToValueType15.i, align 8
  %arrayidx10.i = getelementptr inbounds double addrspace(1)* %1, i64 %loadedValue16.i
  store double %27, double addrspace(1)* %arrayidx10.i, align 8
  br label %if.end11.i

if.end11.i:                                       ; preds = %if.then8.i, %scanLocalMem.exit.i
  %check.WI.iter95.i = icmp ult i64 %CurrWI..5.i, %16
  br i1 %check.WI.iter95.i, label %thenBB92.i, label %__top_scan_separated_args.exit

thenBB92.i:                                       ; preds = %if.end11.i
  %"CurrWI++96.i" = add nuw i64 %CurrWI..5.i, 1
  %"loadedCurrSB+Stride98.i" = add nuw i64 %CurrSBIndex..5.i, 40
  %cond.i = icmp eq i32 %currBarrier.3.i, 16
  br i1 %cond.i, label %SyncBB83.i, label %SyncBB80.i

__top_scan_separated_args.exit:                   ; preds = %if.end11.i
  ret void
}

define void @reduce(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to double addrspace(3)**
  %10 = load double addrspace(3)** %9, align 8
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
  br label %SyncBB62.i

SyncBB62.i:                                       ; preds = %thenBB.i, %entry
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

while.body.i:                                     ; preds = %while.body.i, %SyncBB62.i
  %sum.06.i = phi double [ %add15.i, %while.body.i ], [ 0.000000e+00, %SyncBB62.i ]
  %i.05.i = phi i32 [ %conv19.i, %while.body.i ], [ %add12.i, %SyncBB62.i ]
  %idxprom.i = sext i32 %i.05.i to i64
  %arrayidx.i = getelementptr inbounds double addrspace(1)* %1, i64 %idxprom.i
  %33 = load double addrspace(1)* %arrayidx.i, align 8
  %add15.i = fadd double %sum.06.i, %33
  %conv171.i = zext i32 %i.05.i to i64
  %add18.i = add i64 %32, %conv171.i
  %conv19.i = trunc i64 %add18.i to i32
  %cmp13.i = icmp slt i32 %conv19.i, %cond.i
  br i1 %cmp13.i, label %while.body.i, label %while.end.i

while.end.i:                                      ; preds = %while.body.i, %SyncBB62.i
  %sum.0.lcssa.i = phi double [ 0.000000e+00, %SyncBB62.i ], [ %add15.i, %while.body.i ]
  %idxprom20.i = sext i32 %conv11.i to i64
  %arrayidx21.i = getelementptr inbounds double addrspace(3)* %10, i64 %idxprom20.i
  %"&(pSB[currWI].offset)30.i" = add nuw i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset31.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)30.i"
  %CastToValueType32.i = bitcast i8* %"&pSB[currWI].offset31.i" to double addrspace(3)**
  store double addrspace(3)* %arrayidx21.i, double addrspace(3)** %CastToValueType32.i, align 8
  store double %sum.0.lcssa.i, double addrspace(3)* %arrayidx21.i, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %22
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %while.end.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 24
  br label %SyncBB62.i

elseBB.i:                                         ; preds = %while.end.i
  %div23.i = lshr i64 %32, 1
  %conv24.i = trunc i64 %div23.i to i32
  %cmp252.i = icmp eq i32 %conv24.i, 0
  %arrayidx40.i = getelementptr inbounds double addrspace(1)* %4, i64 %28
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB66.i, %thenBB73.i, %elseBB.i
  %currBarrier.0.i = phi i32 [ 1, %elseBB.i ], [ %currBarrier.3.i, %thenBB66.i ], [ %currBarrier.1.i, %thenBB73.i ]
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride72.i", %thenBB66.i ], [ %"loadedCurrSB+Stride79.i", %thenBB73.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++70.i", %thenBB66.i ], [ %"CurrWI++77.i", %thenBB73.i ]
  br i1 %cmp252.i, label %for.end.i, label %for.body.i

for.body.i:                                       ; preds = %SyncBB64.i, %SyncBB.i
  %currBarrier.1.i = phi i32 [ %currBarrier.2.i, %SyncBB64.i ], [ %currBarrier.0.i, %SyncBB.i ]
  %CurrSBIndex..2.i = phi i64 [ %CurrSBIndex..3.i, %SyncBB64.i ], [ %CurrSBIndex..1.i, %SyncBB.i ]
  %CurrWI..2.i = phi i64 [ %CurrWI..3.i, %SyncBB64.i ], [ %CurrWI..1.i, %SyncBB.i ]
  %s.03.i = phi i32 [ %shr.i, %SyncBB64.i ], [ %conv24.i, %SyncBB.i ]
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
  %arrayidx31.i = getelementptr inbounds double addrspace(3)* %10, i64 %idxprom30.i
  %34 = load double addrspace(3)* %arrayidx31.i, align 8
  %"&(pSB[currWI].offset)34.i" = add nuw i64 %CurrSBIndex..2.i, 8
  %"&pSB[currWI].offset35.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)34.i"
  %CastToValueType36.i = bitcast i8* %"&pSB[currWI].offset35.i" to double addrspace(3)**
  %loadedValue37.i = load double addrspace(3)** %CastToValueType36.i, align 8
  %35 = load double addrspace(3)* %loadedValue37.i, align 8
  %add34.i = fadd double %35, %34
  store double %add34.i, double addrspace(3)* %loadedValue37.i, align 8
  br label %if.end.i

if.end.i:                                         ; preds = %if.then.i, %for.body.i
  %check.WI.iter76.i = icmp ult i64 %CurrWI..2.i, %22
  br i1 %check.WI.iter76.i, label %thenBB73.i, label %SyncBB64.i

thenBB73.i:                                       ; preds = %if.end.i
  %"CurrWI++77.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride79.i" = add nuw i64 %CurrSBIndex..2.i, 24
  %cond2.i = icmp eq i32 %currBarrier.1.i, 1
  br i1 %cond2.i, label %SyncBB.i, label %SyncBB64.i

SyncBB64.i:                                       ; preds = %thenBB66.i, %thenBB73.i, %if.end.i
  %currBarrier.2.i = phi i32 [ %currBarrier.3.i, %thenBB66.i ], [ %currBarrier.1.i, %thenBB73.i ], [ 23, %if.end.i ]
  %CurrSBIndex..3.i = phi i64 [ %"loadedCurrSB+Stride72.i", %thenBB66.i ], [ %"loadedCurrSB+Stride79.i", %thenBB73.i ], [ 0, %if.end.i ]
  %CurrWI..3.i = phi i64 [ %"CurrWI++70.i", %thenBB66.i ], [ %"CurrWI++77.i", %thenBB73.i ], [ 0, %if.end.i ]
  %"&(pSB[currWI].offset)48.i" = add nuw i64 %CurrSBIndex..3.i, 16
  %"&pSB[currWI].offset49.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)48.i"
  %CastToValueType50.i = bitcast i8* %"&pSB[currWI].offset49.i" to i32*
  %loadedValue51.i = load i32* %CastToValueType50.i, align 4
  %shr.i = lshr i32 %loadedValue51.i, 1
  %cmp25.i = icmp eq i32 %shr.i, 0
  br i1 %cmp25.i, label %for.end.i, label %for.body.i

for.end.i:                                        ; preds = %SyncBB64.i, %SyncBB.i
  %currBarrier.3.i = phi i32 [ %currBarrier.0.i, %SyncBB.i ], [ %currBarrier.2.i, %SyncBB64.i ]
  %CurrSBIndex..4.i = phi i64 [ %CurrSBIndex..1.i, %SyncBB.i ], [ %CurrSBIndex..3.i, %SyncBB64.i ]
  %CurrWI..4.i = phi i64 [ %CurrWI..1.i, %SyncBB.i ], [ %CurrWI..3.i, %SyncBB64.i ]
  %"&pSB[currWI].offset26.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..4.i
  %CastToValueType27.i = bitcast i8* %"&pSB[currWI].offset26.i" to i32*
  %loadedValue28.i = load i32* %CastToValueType27.i, align 4
  %cmp35.i = icmp eq i32 %loadedValue28.i, 0
  br i1 %cmp35.i, label %if.then37.i, label %if.end41.i

if.then37.i:                                      ; preds = %for.end.i
  %36 = load double addrspace(3)* %10, align 8
  store double %36, double addrspace(1)* %arrayidx40.i, align 8
  br label %if.end41.i

if.end41.i:                                       ; preds = %if.then37.i, %for.end.i
  %check.WI.iter69.i = icmp ult i64 %CurrWI..4.i, %22
  br i1 %check.WI.iter69.i, label %thenBB66.i, label %__reduce_separated_args.exit

thenBB66.i:                                       ; preds = %if.end41.i
  %"CurrWI++70.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride72.i" = add nuw i64 %CurrSBIndex..4.i, 24
  %cond1.i = icmp eq i32 %currBarrier.3.i, 23
  br i1 %cond1.i, label %SyncBB64.i, label %SyncBB.i

__reduce_separated_args.exit:                     ; preds = %if.end41.i
  ret void
}

define void @bottom_scan(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to double addrspace(1)**
  %7 = load double addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to double addrspace(3)**
  %13 = load double addrspace(3)** %12, align 8
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
  %32 = bitcast i8 addrspace(3)* %16 to double addrspace(3)*
  %33 = bitcast double addrspace(1)* %1 to <4 x double> addrspace(1)*
  %34 = bitcast double addrspace(1)* %7 to <4 x double> addrspace(1)*
  %div.i = sdiv i32 %10, 4
  %conv.i = sext i32 %div.i to i64
  br label %SyncBB182.i

SyncBB182.i:                                      ; preds = %thenBB201.i, %thenBB.i, %entry
  %call38174.0.i = phi i64 [ undef, %entry ], [ %call38174.3.i, %thenBB201.i ], [ %call38174.1.i, %thenBB.i ]
  %currBarrier.0.i = phi i32 [ 24, %entry ], [ %currBarrier.7.i, %thenBB201.i ], [ %currBarrier.1.i, %thenBB.i ]
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride207.i", %thenBB201.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++205.i", %thenBB201.i ], [ %"CurrWI++.i", %thenBB.i ]
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

while.body.lr.ph.i:                               ; preds = %SyncBB182.i
  %arrayidx.i = getelementptr inbounds double addrspace(1)* %4, i64 %37
  %add11.i = add i64 %mul.i, %39
  %40 = load double addrspace(1)* %arrayidx.i, align 8
  %41 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %42 = load i64* %41, align 8
  %sub39.i = add i64 %42, -1
  %cmp40.i = icmp eq i64 %39, %sub39.i
  %"&(pSB[currWI].offset)33.i" = add nuw i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset34.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)33.i"
  %CastToValueType35.i = bitcast i8* %"&pSB[currWI].offset34.i" to i1*
  store i1 %cmp40.i, i1* %CastToValueType35.i, align 1
  br label %while.body.i

while.body.i:                                     ; preds = %SyncBB184.i, %while.body.lr.ph.i
  %call38174.1.i = phi i64 [ %42, %while.body.lr.ph.i ], [ %call38174.2.i, %SyncBB184.i ]
  %currBarrier.1.i = phi i32 [ %currBarrier.0.i, %while.body.lr.ph.i ], [ %currBarrier.6.i, %SyncBB184.i ]
  %CurrSBIndex..1.i = phi i64 [ %CurrSBIndex..0.i, %while.body.lr.ph.i ], [ %CurrSBIndex..7.i, %SyncBB184.i ]
  %CurrWI..1.i = phi i64 [ %CurrWI..0.i, %while.body.lr.ph.i ], [ %CurrWI..7.i, %SyncBB184.i ]
  %i.07.in.i = phi i64 [ %add11.i, %while.body.lr.ph.i ], [ %add50.i, %SyncBB184.i ]
  %seed.06.i = phi double [ %40, %while.body.lr.ph.i ], [ %55, %SyncBB184.i ]
  %window.05.i = phi i32 [ %conv5.i, %while.body.lr.ph.i ], [ %conv47.i, %SyncBB184.i ]
  %"&(pSB[currWI].offset)51.i" = add nuw i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset52.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)51.i"
  %CastToValueType53.i = bitcast i8* %"&pSB[currWI].offset52.i" to i32*
  store i32 %window.05.i, i32* %CastToValueType53.i, align 4
  %"&(pSB[currWI].offset)42.i" = add nuw i64 %CurrSBIndex..1.i, 16
  %"&pSB[currWI].offset43.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)42.i"
  %CastToValueType44.i = bitcast i8* %"&pSB[currWI].offset43.i" to double*
  store double %seed.06.i, double* %CastToValueType44.i, align 8
  %i.07.i = trunc i64 %i.07.in.i to i32
  %"&(pSB[currWI].offset)60.i" = add nuw i64 %CurrSBIndex..1.i, 28
  %"&pSB[currWI].offset61.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)60.i"
  %CastToValueType62.i = bitcast i8* %"&pSB[currWI].offset61.i" to i32*
  store i32 %i.07.i, i32* %CastToValueType62.i, align 4
  %cmp16.i = icmp slt i32 %i.07.i, %cond.i
  %"&(pSB[currWI].offset)79.i" = add nuw i64 %CurrSBIndex..1.i, 32
  %"&pSB[currWI].offset80.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)79.i"
  %CastToValueType81.i = bitcast i8* %"&pSB[currWI].offset80.i" to i1*
  store i1 %cmp16.i, i1* %CastToValueType81.i, align 1
  br i1 %cmp16.i, label %if.then.i, label %while.body.i.if.end.i_crit_edge

while.body.i.if.end.i_crit_edge:                  ; preds = %while.body.i
  br label %if.end.i

if.then.i:                                        ; preds = %while.body.i
  %idxprom.i = sext i32 %i.07.i to i64
  %arrayidx18.i = getelementptr inbounds <4 x double> addrspace(1)* %33, i64 %idxprom.i
  %43 = load <4 x double> addrspace(1)* %arrayidx18.i, align 32
  br label %if.end.i

if.end.i:                                         ; preds = %while.body.i.if.end.i_crit_edge, %if.then.i
  %val_4.1.i = phi <4 x double> [ %43, %if.then.i ], [ zeroinitializer, %while.body.i.if.end.i_crit_edge ]
  %44 = extractelement <4 x double> %val_4.1.i, i32 0
  %"&(pSB[currWI].offset)88.i" = add nuw i64 %CurrSBIndex..1.i, 40
  %"&pSB[currWI].offset89.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)88.i"
  %CastToValueType90.i = bitcast i8* %"&pSB[currWI].offset89.i" to double*
  store double %44, double* %CastToValueType90.i, align 8
  %45 = extractelement <4 x double> %val_4.1.i, i32 1
  %add19.i = fadd double %45, %44
  %"&(pSB[currWI].offset)97.i" = add nuw i64 %CurrSBIndex..1.i, 48
  %"&pSB[currWI].offset98.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)97.i"
  %CastToValueType99.i = bitcast i8* %"&pSB[currWI].offset98.i" to double*
  store double %add19.i, double* %CastToValueType99.i, align 8
  %46 = extractelement <4 x double> %val_4.1.i, i32 2
  %add20.i = fadd double %46, %add19.i
  %"&(pSB[currWI].offset)106.i" = add nuw i64 %CurrSBIndex..1.i, 56
  %"&pSB[currWI].offset107.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)106.i"
  %CastToValueType108.i = bitcast i8* %"&pSB[currWI].offset107.i" to double*
  store double %add20.i, double* %CastToValueType108.i, align 8
  %47 = extractelement <4 x double> %val_4.1.i, i32 3
  %add21.i = fadd double %47, %add20.i
  %"&(pSB[currWI].offset)115.i" = add nuw i64 %CurrSBIndex..1.i, 64
  %"&pSB[currWI].offset116.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)115.i"
  %CastToValueType117.i = bitcast i8* %"&pSB[currWI].offset116.i" to double*
  store double %add21.i, double* %CastToValueType117.i, align 8
  %"&pSB[currWI].offset15.i" = getelementptr inbounds i8* %31, i64 %CurrSBIndex..1.i
  %CastToValueType16.i = bitcast i8* %"&pSB[currWI].offset15.i" to i64*
  %loadedValue.i = load i64* %CastToValueType16.i, align 8
  %sext.i.i = shl i64 %loadedValue.i, 32
  %idxprom.i.i = ashr exact i64 %sext.i.i, 32
  %arrayidx.i.i = getelementptr inbounds double addrspace(3)* %13, i64 %idxprom.i.i
  store double 0.000000e+00, double addrspace(3)* %arrayidx.i.i, align 8
  %loadedValue21.i = load i64* %CastToValueType16.i, align 8
  %add.i.i = add i64 %call38174.1.i, %loadedValue21.i
  %conv3.i.i = trunc i64 %add.i.i to i32
  %"&(pSB[currWI].offset)124.i" = add nuw i64 %CurrSBIndex..1.i, 72
  %"&pSB[currWI].offset125.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)124.i"
  %CastToValueType126.i = bitcast i8* %"&pSB[currWI].offset125.i" to i32*
  store i32 %conv3.i.i, i32* %CastToValueType126.i, align 4
  %idxprom4.i.i = sext i32 %conv3.i.i to i64
  %arrayidx5.i.i = getelementptr inbounds double addrspace(3)* %13, i64 %idxprom4.i.i
  %"&(pSB[currWI].offset)138.i" = add nuw i64 %CurrSBIndex..1.i, 80
  %"&pSB[currWI].offset139.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)138.i"
  %CastToValueType140.i = bitcast i8* %"&pSB[currWI].offset139.i" to double addrspace(3)**
  store double addrspace(3)* %arrayidx5.i.i, double addrspace(3)** %CastToValueType140.i, align 8
  store double %add21.i, double addrspace(3)* %arrayidx5.i.i, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..1.i, %28
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %if.end.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..1.i, 104
  %cond4.i = icmp eq i32 %currBarrier.1.i, 24
  br i1 %cond4.i, label %SyncBB182.i, label %thenBB.i.SyncBB184.i_crit_edge

thenBB.i.SyncBB184.i_crit_edge:                   ; preds = %thenBB.i
  br label %SyncBB184.i

elseBB.i:                                         ; preds = %if.end.i
  %cmp1.i.i = icmp ugt i64 %call38174.1.i, 1
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB209.i, %thenBB186.i, %elseBB.i
  %currBarrier.2.i = phi i32 [ 9, %elseBB.i ], [ %currBarrier.5.i, %thenBB209.i ], [ %currBarrier.3.i, %thenBB186.i ]
  %CurrSBIndex..2.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride215.i", %thenBB209.i ], [ %"loadedCurrSB+Stride192.i", %thenBB186.i ]
  %CurrWI..2.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++213.i", %thenBB209.i ], [ %"CurrWI++190.i", %thenBB186.i ]
  br i1 %cmp1.i.i, label %for.body.i.i, label %scanLocalMem.exit.i

for.body.i.i:                                     ; preds = %SyncBB181.i, %SyncBB.i
  %currBarrier.3.i = phi i32 [ %currBarrier.4.i, %SyncBB181.i ], [ %currBarrier.2.i, %SyncBB.i ]
  %CurrSBIndex..3.i = phi i64 [ %CurrSBIndex..5.i, %SyncBB181.i ], [ %CurrSBIndex..2.i, %SyncBB.i ]
  %CurrWI..3.i = phi i64 [ %CurrWI..5.i, %SyncBB181.i ], [ %CurrWI..2.i, %SyncBB.i ]
  %i.02.i.i = phi i32 [ %mul.i.i, %SyncBB181.i ], [ 1, %SyncBB.i ]
  %"&(pSB[currWI].offset)152.i" = add nuw i64 %CurrSBIndex..3.i, 88
  %"&pSB[currWI].offset153.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)152.i"
  %CastToValueType154.i = bitcast i8* %"&pSB[currWI].offset153.i" to i32*
  store i32 %i.02.i.i, i32* %CastToValueType154.i, align 4
  %"&(pSB[currWI].offset)133.i" = add nuw i64 %CurrSBIndex..3.i, 72
  %"&pSB[currWI].offset134.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)133.i"
  %CastToValueType135.i = bitcast i8* %"&pSB[currWI].offset134.i" to i32*
  %loadedValue136.i = load i32* %CastToValueType135.i, align 4
  %sub.i.i = sub nsw i32 %loadedValue136.i, %i.02.i.i
  %idxprom9.i.i = sext i32 %sub.i.i to i64
  %arrayidx10.i.i = getelementptr inbounds double addrspace(3)* %13, i64 %idxprom9.i.i
  %48 = load double addrspace(3)* %arrayidx10.i.i, align 8
  %"&(pSB[currWI].offset)161.i" = add nuw i64 %CurrSBIndex..3.i, 96
  %"&pSB[currWI].offset162.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)161.i"
  %CastToValueType163.i = bitcast i8* %"&pSB[currWI].offset162.i" to double*
  store double %48, double* %CastToValueType163.i, align 8
  %check.WI.iter189.i = icmp ult i64 %CurrWI..3.i, %28
  br i1 %check.WI.iter189.i, label %thenBB186.i, label %for.body.i.i.SyncBB180.i_crit_edge

for.body.i.i.SyncBB180.i_crit_edge:               ; preds = %for.body.i.i
  br label %SyncBB180.i

thenBB186.i:                                      ; preds = %for.body.i.i
  %"CurrWI++190.i" = add nuw i64 %CurrWI..3.i, 1
  %"loadedCurrSB+Stride192.i" = add nuw i64 %CurrSBIndex..3.i, 104
  %cond3.i = icmp eq i32 %currBarrier.3.i, 9
  br i1 %cond3.i, label %SyncBB.i, label %SyncBB181.i

SyncBB180.i:                                      ; preds = %for.body.i.i.SyncBB180.i_crit_edge, %thenBB194.i
  %CurrSBIndex..4.i = phi i64 [ %"loadedCurrSB+Stride200.i", %thenBB194.i ], [ 0, %for.body.i.i.SyncBB180.i_crit_edge ]
  %CurrWI..4.i = phi i64 [ %"CurrWI++198.i", %thenBB194.i ], [ 0, %for.body.i.i.SyncBB180.i_crit_edge ]
  %"&(pSB[currWI].offset)147.i" = add nuw i64 %CurrSBIndex..4.i, 80
  %"&pSB[currWI].offset148.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)147.i"
  %CastToValueType149.i = bitcast i8* %"&pSB[currWI].offset148.i" to double addrspace(3)**
  %loadedValue150.i = load double addrspace(3)** %CastToValueType149.i, align 8
  %49 = load double addrspace(3)* %loadedValue150.i, align 8
  %"&(pSB[currWI].offset)165.i" = add nuw i64 %CurrSBIndex..4.i, 96
  %"&pSB[currWI].offset166.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)165.i"
  %CastToValueType167.i = bitcast i8* %"&pSB[currWI].offset166.i" to double*
  %loadedValue168.i = load double* %CastToValueType167.i, align 8
  %add13.i.i = fadd double %49, %loadedValue168.i
  store double %add13.i.i, double addrspace(3)* %loadedValue150.i, align 8
  %check.WI.iter197.i = icmp ult i64 %CurrWI..4.i, %28
  br i1 %check.WI.iter197.i, label %thenBB194.i, label %SyncBB181.i

thenBB194.i:                                      ; preds = %SyncBB180.i
  %"CurrWI++198.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride200.i" = add nuw i64 %CurrSBIndex..4.i, 104
  br label %SyncBB180.i

SyncBB181.i:                                      ; preds = %thenBB209.i, %SyncBB180.i, %thenBB186.i
  %currBarrier.4.i = phi i32 [ %currBarrier.3.i, %thenBB186.i ], [ %currBarrier.5.i, %thenBB209.i ], [ 11, %SyncBB180.i ]
  %CurrSBIndex..5.i = phi i64 [ %"loadedCurrSB+Stride192.i", %thenBB186.i ], [ %"loadedCurrSB+Stride215.i", %thenBB209.i ], [ 0, %SyncBB180.i ]
  %CurrWI..5.i = phi i64 [ %"CurrWI++190.i", %thenBB186.i ], [ %"CurrWI++213.i", %thenBB209.i ], [ 0, %SyncBB180.i ]
  %"&(pSB[currWI].offset)156.i" = add nuw i64 %CurrSBIndex..5.i, 88
  %"&pSB[currWI].offset157.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)156.i"
  %CastToValueType158.i = bitcast i8* %"&pSB[currWI].offset157.i" to i32*
  %loadedValue159.i = load i32* %CastToValueType158.i, align 4
  %mul.i.i = shl nsw i32 %loadedValue159.i, 1
  %conv6.i.i = sext i32 %mul.i.i to i64
  %cmp.i.i = icmp ult i64 %conv6.i.i, %call38174.1.i
  br i1 %cmp.i.i, label %for.body.i.i, label %scanLocalMem.exit.i

scanLocalMem.exit.i:                              ; preds = %SyncBB181.i, %SyncBB.i
  %currBarrier.5.i = phi i32 [ %currBarrier.2.i, %SyncBB.i ], [ %currBarrier.4.i, %SyncBB181.i ]
  %CurrSBIndex..6.i = phi i64 [ %CurrSBIndex..2.i, %SyncBB.i ], [ %CurrSBIndex..5.i, %SyncBB181.i ]
  %CurrWI..6.i = phi i64 [ %CurrWI..2.i, %SyncBB.i ], [ %CurrWI..5.i, %SyncBB181.i ]
  %"&(pSB[currWI].offset)128.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset129.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)128.i"
  %CastToValueType130.i = bitcast i8* %"&pSB[currWI].offset129.i" to i32*
  %loadedValue131.i = load i32* %CastToValueType130.i, align 4
  %sub14.i.i = add nsw i32 %loadedValue131.i, -1
  %idxprom15.i.i = sext i32 %sub14.i.i to i64
  %arrayidx16.i.i = getelementptr inbounds double addrspace(3)* %13, i64 %idxprom15.i.i
  %50 = load double addrspace(3)* %arrayidx16.i.i, align 8
  %"&(pSB[currWI].offset)46.i" = add nuw i64 %CurrSBIndex..6.i, 16
  %"&pSB[currWI].offset47.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)46.i"
  %CastToValueType48.i = bitcast i8* %"&pSB[currWI].offset47.i" to double*
  %loadedValue49.i = load double* %CastToValueType48.i, align 8
  %add29.i = fadd double %50, %loadedValue49.i
  %"&(pSB[currWI].offset)119.i" = add nuw i64 %CurrSBIndex..6.i, 64
  %"&pSB[currWI].offset120.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)119.i"
  %CastToValueType121.i = bitcast i8* %"&pSB[currWI].offset120.i" to double*
  %loadedValue122.i = load double* %CastToValueType121.i, align 8
  %add30.i = fadd double %loadedValue122.i, %add29.i
  %"&(pSB[currWI].offset)83.i" = add nuw i64 %CurrSBIndex..6.i, 32
  %"&pSB[currWI].offset84.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)83.i"
  %CastToValueType85.i = bitcast i8* %"&pSB[currWI].offset84.i" to i1*
  %loadedValue86.i = load i1* %CastToValueType85.i, align 1
  br i1 %loadedValue86.i, label %if.then33.i, label %if.end36.i

if.then33.i:                                      ; preds = %scanLocalMem.exit.i
  %"&(pSB[currWI].offset)92.i" = add nuw i64 %CurrSBIndex..6.i, 40
  %"&pSB[currWI].offset93.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)92.i"
  %CastToValueType94.i = bitcast i8* %"&pSB[currWI].offset93.i" to double*
  %loadedValue95.i = load double* %CastToValueType94.i, align 8
  %add24.i = fadd double %loadedValue95.i, %add29.i
  %"&(pSB[currWI].offset)101.i" = add nuw i64 %CurrSBIndex..6.i, 48
  %"&pSB[currWI].offset102.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)101.i"
  %CastToValueType103.i = bitcast i8* %"&pSB[currWI].offset102.i" to double*
  %loadedValue104.i = load double* %CastToValueType103.i, align 8
  %add26.i = fadd double %loadedValue104.i, %add29.i
  %51 = insertelement <4 x double> undef, double %add24.i, i32 0
  %"&(pSB[currWI].offset)110.i" = add nuw i64 %CurrSBIndex..6.i, 56
  %"&pSB[currWI].offset111.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)110.i"
  %CastToValueType112.i = bitcast i8* %"&pSB[currWI].offset111.i" to double*
  %loadedValue113.i = load double* %CastToValueType112.i, align 8
  %add28.i = fadd double %loadedValue113.i, %add29.i
  %52 = insertelement <4 x double> %51, double %add26.i, i32 1
  %53 = insertelement <4 x double> %52, double %add28.i, i32 2
  %54 = insertelement <4 x double> %53, double %add30.i, i32 3
  %"&(pSB[currWI].offset)69.i" = add nuw i64 %CurrSBIndex..6.i, 28
  %"&pSB[currWI].offset70.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)69.i"
  %CastToValueType71.i = bitcast i8* %"&pSB[currWI].offset70.i" to i32*
  %loadedValue72.i = load i32* %CastToValueType71.i, align 4
  %idxprom34.i = sext i32 %loadedValue72.i to i64
  %arrayidx35.i = getelementptr inbounds <4 x double> addrspace(1)* %34, i64 %idxprom34.i
  store <4 x double> %54, <4 x double> addrspace(1)* %arrayidx35.i, align 32
  br label %if.end36.i

if.end36.i:                                       ; preds = %if.then33.i, %scanLocalMem.exit.i
  %"&(pSB[currWI].offset)37.i" = add nuw i64 %CurrSBIndex..6.i, 8
  %"&pSB[currWI].offset38.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)37.i"
  %CastToValueType39.i = bitcast i8* %"&pSB[currWI].offset38.i" to i1*
  %loadedValue40.i = load i1* %CastToValueType39.i, align 1
  br i1 %loadedValue40.i, label %if.then42.i, label %if.end43.i

if.then42.i:                                      ; preds = %if.end36.i
  store double %add30.i, double addrspace(3)* %32, align 8
  br label %if.end43.i

if.end43.i:                                       ; preds = %if.then42.i, %if.end36.i
  %check.WI.iter212.i = icmp ult i64 %CurrWI..6.i, %28
  br i1 %check.WI.iter212.i, label %thenBB209.i, label %if.end43.i.SyncBB184.i_crit_edge

if.end43.i.SyncBB184.i_crit_edge:                 ; preds = %if.end43.i
  br label %SyncBB184.i

thenBB209.i:                                      ; preds = %if.end43.i
  %"CurrWI++213.i" = add nuw i64 %CurrWI..6.i, 1
  %"loadedCurrSB+Stride215.i" = add nuw i64 %CurrSBIndex..6.i, 104
  %cond2.i = icmp eq i32 %currBarrier.5.i, 11
  br i1 %cond2.i, label %SyncBB181.i, label %SyncBB.i

SyncBB184.i:                                      ; preds = %thenBB201.i.SyncBB184.i_crit_edge, %if.end43.i.SyncBB184.i_crit_edge, %thenBB.i.SyncBB184.i_crit_edge
  %call38174.2.i = phi i64 [ %call38174.1.i, %thenBB.i.SyncBB184.i_crit_edge ], [ %call38174.1.i, %if.end43.i.SyncBB184.i_crit_edge ], [ %call38174.3.i, %thenBB201.i.SyncBB184.i_crit_edge ]
  %currBarrier.6.i = phi i32 [ %currBarrier.1.i, %thenBB.i.SyncBB184.i_crit_edge ], [ 21, %if.end43.i.SyncBB184.i_crit_edge ], [ %currBarrier.7.i, %thenBB201.i.SyncBB184.i_crit_edge ]
  %CurrSBIndex..7.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i.SyncBB184.i_crit_edge ], [ 0, %if.end43.i.SyncBB184.i_crit_edge ], [ %"loadedCurrSB+Stride207.i", %thenBB201.i.SyncBB184.i_crit_edge ]
  %CurrWI..7.i = phi i64 [ %"CurrWI++.i", %thenBB.i.SyncBB184.i_crit_edge ], [ 0, %if.end43.i.SyncBB184.i_crit_edge ], [ %"CurrWI++205.i", %thenBB201.i.SyncBB184.i_crit_edge ]
  %55 = load double addrspace(3)* %32, align 8
  %"&(pSB[currWI].offset)55.i" = add nuw i64 %CurrSBIndex..7.i, 24
  %"&pSB[currWI].offset56.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)55.i"
  %CastToValueType57.i = bitcast i8* %"&pSB[currWI].offset56.i" to i32*
  %loadedValue58.i = load i32* %CastToValueType57.i, align 4
  %conv452.i = zext i32 %loadedValue58.i to i64
  %add46.i = add i64 %call38174.2.i, %conv452.i
  %conv47.i = trunc i64 %add46.i to i32
  %"&(pSB[currWI].offset)74.i" = add nuw i64 %CurrSBIndex..7.i, 28
  %"&pSB[currWI].offset75.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)74.i"
  %CastToValueType76.i = bitcast i8* %"&pSB[currWI].offset75.i" to i32*
  %loadedValue77.i = load i32* %CastToValueType76.i, align 4
  %conv49.i = sext i32 %loadedValue77.i to i64
  %add50.i = add i64 %call38174.2.i, %conv49.i
  %cmp14.i = icmp slt i32 %conv47.i, %cond.i
  br i1 %cmp14.i, label %while.body.i, label %while.end.i

while.end.i:                                      ; preds = %SyncBB184.i, %SyncBB182.i
  %call38174.3.i = phi i64 [ %call38174.0.i, %SyncBB182.i ], [ %call38174.2.i, %SyncBB184.i ]
  %currBarrier.7.i = phi i32 [ %currBarrier.0.i, %SyncBB182.i ], [ %currBarrier.6.i, %SyncBB184.i ]
  %CurrSBIndex..8.i = phi i64 [ %CurrSBIndex..0.i, %SyncBB182.i ], [ %CurrSBIndex..7.i, %SyncBB184.i ]
  %CurrWI..8.i = phi i64 [ %CurrWI..0.i, %SyncBB182.i ], [ %CurrWI..7.i, %SyncBB184.i ]
  %check.WI.iter204.i = icmp ult i64 %CurrWI..8.i, %28
  br i1 %check.WI.iter204.i, label %thenBB201.i, label %__bottom_scan_separated_args.exit

thenBB201.i:                                      ; preds = %while.end.i
  %"CurrWI++205.i" = add nuw i64 %CurrWI..8.i, 1
  %"loadedCurrSB+Stride207.i" = add nuw i64 %CurrSBIndex..8.i, 104
  %cond1.i = icmp eq i32 %currBarrier.7.i, 21
  br i1 %cond1.i, label %thenBB201.i.SyncBB184.i_crit_edge, label %SyncBB182.i

thenBB201.i.SyncBB184.i_crit_edge:                ; preds = %thenBB201.i
  br label %SyncBB184.i

__bottom_scan_separated_args.exit:                ; preds = %while.end.i
  ret void
}

define void @__Vectorized_.bottom_scan(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to double addrspace(1)**
  %7 = load double addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to double addrspace(3)**
  %13 = load double addrspace(3)** %12, align 8
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
  %32 = bitcast i8 addrspace(3)* %16 to double addrspace(3)*
  %33 = bitcast double addrspace(1)* %1 to <4 x double> addrspace(1)*
  %34 = bitcast double addrspace(1)* %7 to <4 x double> addrspace(1)*
  %div.i = sdiv i32 %10, 4
  %conv.i = sext i32 %div.i to i64
  br label %SyncBB2941.i

SyncBB2941.i:                                     ; preds = %thenBB2961.i, %thenBB2969.i, %thenBB2953.i, %thenBB2945.i, %thenBB.i, %entry
  %currBarrier.0.i = phi i32 [ 29, %entry ], [ %currBarrier.1.i, %thenBB.i ], [ %currBarrier.6.i, %thenBB2953.i ], [ %currBarrier.4.i, %thenBB2945.i ], [ %currBarrier.9.i, %thenBB2969.i ], [ %currBarrier.12.i, %thenBB2961.i ]
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ %"loadedCurrSB+Stride2959.i", %thenBB2953.i ], [ %"loadedCurrSB+Stride2951.i", %thenBB2945.i ], [ %"loadedCurrSB+Stride2975.i", %thenBB2969.i ], [ %"loadedCurrSB+Stride2967.i", %thenBB2961.i ]
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ], [ %"CurrWI++2957.i", %thenBB2953.i ], [ %"CurrWI++2949.i", %thenBB2945.i ], [ %"CurrWI++2973.i", %thenBB2969.i ], [ %"CurrWI++2965.i", %thenBB2961.i ]
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
  br i1 %cmp144.i, label %preload819.i, label %postload820.i

preload819.i:                                     ; preds = %SyncBB2941.i
  %arrayidx.i = getelementptr inbounds double addrspace(1)* %4, i64 %37
  %masked_load.i = load double addrspace(1)* %arrayidx.i, align 8
  br label %postload820.i

postload820.i:                                    ; preds = %preload819.i, %SyncBB2941.i
  %phi821.i = phi double [ undef, %SyncBB2941.i ], [ %masked_load.i, %preload819.i ]
  %temp94.i = insertelement <16 x double> undef, double %phi821.i, i32 0
  %vector95.i = shufflevector <16 x double> %temp94.i, <16 x double> undef, <16 x i32> zeroinitializer
  br i1 %cmp144.i, label %preload822.i, label %postload823.i

preload822.i:                                     ; preds = %postload820.i
  %41 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %42 = load i64* %41, align 8
  br label %postload823.i

postload823.i:                                    ; preds = %preload822.i, %postload820.i
  %phi824.i = phi i64 [ undef, %postload820.i ], [ %42, %preload822.i ]
  %"&(pSB[currWI].offset)1271.i" = add nuw i64 %CurrSBIndex..0.i, 128
  %"&pSB[currWI].offset1272.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1271.i"
  %CastToValueType1273.i = bitcast i8* %"&pSB[currWI].offset1272.i" to i64*
  store i64 %phi824.i, i64* %CastToValueType1273.i, align 8
  %temp580.i = insertelement <16 x i64> undef, i64 %phi824.i, i32 0
  %vector581.i = shufflevector <16 x i64> %temp580.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)1280.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1281.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1280.i"
  %CastToValueType1282.i = bitcast i8* %"&pSB[currWI].offset1281.i" to <16 x i64>*
  store <16 x i64> %vector581.i, <16 x i64>* %CastToValueType1282.i, align 128
  %sub39.i = add i64 %phi824.i, -1
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

while.body.i:                                     ; preds = %postload1030.i, %postload823.i
  %currBarrier.1.i = phi i32 [ %currBarrier.11.i, %postload1030.i ], [ %currBarrier.0.i, %postload823.i ]
  %CurrSBIndex..1.i = phi i64 [ %CurrSBIndex..11.i, %postload1030.i ], [ %CurrSBIndex..0.i, %postload823.i ]
  %CurrWI..1.i = phi i64 [ %CurrWI..11.i, %postload1030.i ], [ %CurrWI..0.i, %postload823.i ]
  %vectorPHI86.i = phi <16 x i1> [ %loop_mask26587.i, %postload1030.i ], [ %vector88.i, %postload823.i ]
  %loadedValue1389.i = phi <16 x i1> [ %local_edge31608.i, %postload1030.i ], [ %vector91.i, %postload823.i ]
  %vectorPHI92.i = phi <16 x i64> [ %add50582.i, %postload1030.i ], [ %add1183.i, %postload823.i ]
  %vectorPHI93.i = phi <16 x double> [ %temp.vect640.i, %postload1030.i ], [ %vector95.i, %postload823.i ]
  %window.05.i = phi i32 [ %conv47.i, %postload1030.i ], [ %conv5.i, %postload823.i ]
  %"&(pSB[currWI].offset)1420.i" = add nuw i64 %CurrSBIndex..1.i, 640
  %"&pSB[currWI].offset1421.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1420.i"
  %CastToValueType1422.i = bitcast i8* %"&pSB[currWI].offset1421.i" to i32*
  store i32 %window.05.i, i32* %CastToValueType1422.i, align 4
  %"&(pSB[currWI].offset)1411.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1412.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1411.i"
  %CastToValueType1413.i = bitcast i8* %"&pSB[currWI].offset1412.i" to <16 x double>*
  store <16 x double> %vectorPHI93.i, <16 x double>* %CastToValueType1413.i, align 128
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
  %"&(pSB[currWI].offset)1429.i" = add nuw i64 %CurrSBIndex..1.i, 644
  %"&pSB[currWI].offset1430.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1429.i"
  %CastToValueType1431.i = bitcast i8* %"&pSB[currWI].offset1430.i" to i1*
  store i1 %extract119.i, i1* %CastToValueType1431.i, align 1
  %extract120.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 2
  %"&(pSB[currWI].offset)1443.i" = add nuw i64 %CurrSBIndex..1.i, 645
  %"&pSB[currWI].offset1444.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1443.i"
  %CastToValueType1445.i = bitcast i8* %"&pSB[currWI].offset1444.i" to i1*
  store i1 %extract120.i, i1* %CastToValueType1445.i, align 1
  %extract121.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 3
  %"&(pSB[currWI].offset)1457.i" = add nuw i64 %CurrSBIndex..1.i, 646
  %"&pSB[currWI].offset1458.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1457.i"
  %CastToValueType1459.i = bitcast i8* %"&pSB[currWI].offset1458.i" to i1*
  store i1 %extract121.i, i1* %CastToValueType1459.i, align 1
  %extract122.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 4
  %"&(pSB[currWI].offset)1471.i" = add nuw i64 %CurrSBIndex..1.i, 647
  %"&pSB[currWI].offset1472.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1471.i"
  %CastToValueType1473.i = bitcast i8* %"&pSB[currWI].offset1472.i" to i1*
  store i1 %extract122.i, i1* %CastToValueType1473.i, align 1
  %extract123.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 5
  %"&(pSB[currWI].offset)1485.i" = add nuw i64 %CurrSBIndex..1.i, 648
  %"&pSB[currWI].offset1486.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1485.i"
  %CastToValueType1487.i = bitcast i8* %"&pSB[currWI].offset1486.i" to i1*
  store i1 %extract123.i, i1* %CastToValueType1487.i, align 1
  %extract124.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 6
  %"&(pSB[currWI].offset)1499.i" = add nuw i64 %CurrSBIndex..1.i, 649
  %"&pSB[currWI].offset1500.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1499.i"
  %CastToValueType1501.i = bitcast i8* %"&pSB[currWI].offset1500.i" to i1*
  store i1 %extract124.i, i1* %CastToValueType1501.i, align 1
  %extract125.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 7
  %"&(pSB[currWI].offset)1513.i" = add nuw i64 %CurrSBIndex..1.i, 650
  %"&pSB[currWI].offset1514.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1513.i"
  %CastToValueType1515.i = bitcast i8* %"&pSB[currWI].offset1514.i" to i1*
  store i1 %extract125.i, i1* %CastToValueType1515.i, align 1
  %extract126.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 8
  %"&(pSB[currWI].offset)1527.i" = add nuw i64 %CurrSBIndex..1.i, 651
  %"&pSB[currWI].offset1528.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1527.i"
  %CastToValueType1529.i = bitcast i8* %"&pSB[currWI].offset1528.i" to i1*
  store i1 %extract126.i, i1* %CastToValueType1529.i, align 1
  %extract127.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 9
  %"&(pSB[currWI].offset)1541.i" = add nuw i64 %CurrSBIndex..1.i, 652
  %"&pSB[currWI].offset1542.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1541.i"
  %CastToValueType1543.i = bitcast i8* %"&pSB[currWI].offset1542.i" to i1*
  store i1 %extract127.i, i1* %CastToValueType1543.i, align 1
  %extract128.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 10
  %"&(pSB[currWI].offset)1555.i" = add nuw i64 %CurrSBIndex..1.i, 653
  %"&pSB[currWI].offset1556.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1555.i"
  %CastToValueType1557.i = bitcast i8* %"&pSB[currWI].offset1556.i" to i1*
  store i1 %extract128.i, i1* %CastToValueType1557.i, align 1
  %extract129.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 11
  %"&(pSB[currWI].offset)1569.i" = add nuw i64 %CurrSBIndex..1.i, 654
  %"&pSB[currWI].offset1570.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1569.i"
  %CastToValueType1571.i = bitcast i8* %"&pSB[currWI].offset1570.i" to i1*
  store i1 %extract129.i, i1* %CastToValueType1571.i, align 1
  %extract130.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 12
  %"&(pSB[currWI].offset)1583.i" = add nuw i64 %CurrSBIndex..1.i, 655
  %"&pSB[currWI].offset1584.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1583.i"
  %CastToValueType1585.i = bitcast i8* %"&pSB[currWI].offset1584.i" to i1*
  store i1 %extract130.i, i1* %CastToValueType1585.i, align 1
  %extract131.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 13
  %"&(pSB[currWI].offset)1597.i" = add nuw i64 %CurrSBIndex..1.i, 656
  %"&pSB[currWI].offset1598.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1597.i"
  %CastToValueType1599.i = bitcast i8* %"&pSB[currWI].offset1598.i" to i1*
  store i1 %extract131.i, i1* %CastToValueType1599.i, align 1
  %extract132.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 14
  %"&(pSB[currWI].offset)1611.i" = add nuw i64 %CurrSBIndex..1.i, 657
  %"&pSB[currWI].offset1612.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1611.i"
  %CastToValueType1613.i = bitcast i8* %"&pSB[currWI].offset1612.i" to i1*
  store i1 %extract132.i, i1* %CastToValueType1613.i, align 1
  %extract133.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 15
  %"&(pSB[currWI].offset)1625.i" = add nuw i64 %CurrSBIndex..1.i, 658
  %"&pSB[currWI].offset1626.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1625.i"
  %CastToValueType1627.i = bitcast i8* %"&pSB[currWI].offset1626.i" to i1*
  store i1 %extract133.i, i1* %CastToValueType1627.i, align 1
  %extract118.i = extractelement <16 x i1> %while.body_to_if.then101.i, i32 0
  %"&(pSB[currWI].offset)1639.i" = add nuw i64 %CurrSBIndex..1.i, 659
  %"&pSB[currWI].offset1640.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1639.i"
  %CastToValueType1641.i = bitcast i8* %"&pSB[currWI].offset1640.i" to i1*
  store i1 %extract118.i, i1* %CastToValueType1641.i, align 1
  %idxprom102.i = sext <16 x i32> %i.0796.i to <16 x i64>
  %"&(pSB[currWI].offset)1648.i" = add nuw i64 %CurrSBIndex..1.i, 768
  %"&pSB[currWI].offset1649.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1648.i"
  %CastToValueType1650.i = bitcast i8* %"&pSB[currWI].offset1649.i" to <16 x i64>*
  store <16 x i64> %idxprom102.i, <16 x i64>* %CastToValueType1650.i, align 128
  %extract103.i = extractelement <16 x i64> %idxprom102.i, i32 1
  %"&(pSB[currWI].offset)1667.i" = add nuw i64 %CurrSBIndex..1.i, 896
  %"&pSB[currWI].offset1668.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1667.i"
  %CastToValueType1669.i = bitcast i8* %"&pSB[currWI].offset1668.i" to i64*
  store i64 %extract103.i, i64* %CastToValueType1669.i, align 8
  %extract104.i = extractelement <16 x i64> %idxprom102.i, i32 2
  %"&(pSB[currWI].offset)1676.i" = add nuw i64 %CurrSBIndex..1.i, 904
  %"&pSB[currWI].offset1677.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1676.i"
  %CastToValueType1678.i = bitcast i8* %"&pSB[currWI].offset1677.i" to i64*
  store i64 %extract104.i, i64* %CastToValueType1678.i, align 8
  %extract105.i = extractelement <16 x i64> %idxprom102.i, i32 3
  %"&(pSB[currWI].offset)1685.i" = add nuw i64 %CurrSBIndex..1.i, 912
  %"&pSB[currWI].offset1686.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1685.i"
  %CastToValueType1687.i = bitcast i8* %"&pSB[currWI].offset1686.i" to i64*
  store i64 %extract105.i, i64* %CastToValueType1687.i, align 8
  %extract106.i = extractelement <16 x i64> %idxprom102.i, i32 4
  %"&(pSB[currWI].offset)1694.i" = add nuw i64 %CurrSBIndex..1.i, 920
  %"&pSB[currWI].offset1695.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1694.i"
  %CastToValueType1696.i = bitcast i8* %"&pSB[currWI].offset1695.i" to i64*
  store i64 %extract106.i, i64* %CastToValueType1696.i, align 8
  %extract107.i = extractelement <16 x i64> %idxprom102.i, i32 5
  %"&(pSB[currWI].offset)1703.i" = add nuw i64 %CurrSBIndex..1.i, 928
  %"&pSB[currWI].offset1704.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1703.i"
  %CastToValueType1705.i = bitcast i8* %"&pSB[currWI].offset1704.i" to i64*
  store i64 %extract107.i, i64* %CastToValueType1705.i, align 8
  %extract108.i = extractelement <16 x i64> %idxprom102.i, i32 6
  %"&(pSB[currWI].offset)1712.i" = add nuw i64 %CurrSBIndex..1.i, 936
  %"&pSB[currWI].offset1713.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1712.i"
  %CastToValueType1714.i = bitcast i8* %"&pSB[currWI].offset1713.i" to i64*
  store i64 %extract108.i, i64* %CastToValueType1714.i, align 8
  %extract109.i = extractelement <16 x i64> %idxprom102.i, i32 7
  %"&(pSB[currWI].offset)1721.i" = add nuw i64 %CurrSBIndex..1.i, 944
  %"&pSB[currWI].offset1722.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1721.i"
  %CastToValueType1723.i = bitcast i8* %"&pSB[currWI].offset1722.i" to i64*
  store i64 %extract109.i, i64* %CastToValueType1723.i, align 8
  %extract110.i = extractelement <16 x i64> %idxprom102.i, i32 8
  %"&(pSB[currWI].offset)1730.i" = add nuw i64 %CurrSBIndex..1.i, 952
  %"&pSB[currWI].offset1731.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1730.i"
  %CastToValueType1732.i = bitcast i8* %"&pSB[currWI].offset1731.i" to i64*
  store i64 %extract110.i, i64* %CastToValueType1732.i, align 8
  %extract111.i = extractelement <16 x i64> %idxprom102.i, i32 9
  %"&(pSB[currWI].offset)1739.i" = add nuw i64 %CurrSBIndex..1.i, 960
  %"&pSB[currWI].offset1740.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1739.i"
  %CastToValueType1741.i = bitcast i8* %"&pSB[currWI].offset1740.i" to i64*
  store i64 %extract111.i, i64* %CastToValueType1741.i, align 8
  %extract112.i = extractelement <16 x i64> %idxprom102.i, i32 10
  %"&(pSB[currWI].offset)1748.i" = add nuw i64 %CurrSBIndex..1.i, 968
  %"&pSB[currWI].offset1749.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1748.i"
  %CastToValueType1750.i = bitcast i8* %"&pSB[currWI].offset1749.i" to i64*
  store i64 %extract112.i, i64* %CastToValueType1750.i, align 8
  %extract113.i = extractelement <16 x i64> %idxprom102.i, i32 11
  %"&(pSB[currWI].offset)1757.i" = add nuw i64 %CurrSBIndex..1.i, 976
  %"&pSB[currWI].offset1758.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1757.i"
  %CastToValueType1759.i = bitcast i8* %"&pSB[currWI].offset1758.i" to i64*
  store i64 %extract113.i, i64* %CastToValueType1759.i, align 8
  %extract114.i = extractelement <16 x i64> %idxprom102.i, i32 12
  %"&(pSB[currWI].offset)1766.i" = add nuw i64 %CurrSBIndex..1.i, 984
  %"&pSB[currWI].offset1767.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1766.i"
  %CastToValueType1768.i = bitcast i8* %"&pSB[currWI].offset1767.i" to i64*
  store i64 %extract114.i, i64* %CastToValueType1768.i, align 8
  %extract115.i = extractelement <16 x i64> %idxprom102.i, i32 13
  %"&(pSB[currWI].offset)1775.i" = add nuw i64 %CurrSBIndex..1.i, 992
  %"&pSB[currWI].offset1776.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1775.i"
  %CastToValueType1777.i = bitcast i8* %"&pSB[currWI].offset1776.i" to i64*
  store i64 %extract115.i, i64* %CastToValueType1777.i, align 8
  %extract116.i = extractelement <16 x i64> %idxprom102.i, i32 14
  %"&(pSB[currWI].offset)1784.i" = add nuw i64 %CurrSBIndex..1.i, 1000
  %"&pSB[currWI].offset1785.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1784.i"
  %CastToValueType1786.i = bitcast i8* %"&pSB[currWI].offset1785.i" to i64*
  store i64 %extract116.i, i64* %CastToValueType1786.i, align 8
  %extract117.i = extractelement <16 x i64> %idxprom102.i, i32 15
  %"&(pSB[currWI].offset)1793.i" = add nuw i64 %CurrSBIndex..1.i, 1008
  %"&pSB[currWI].offset1794.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1793.i"
  %CastToValueType1795.i = bitcast i8* %"&pSB[currWI].offset1794.i" to i64*
  store i64 %extract117.i, i64* %CastToValueType1795.i, align 8
  %43 = getelementptr inbounds <4 x double> addrspace(1)* %33, i64 %extract103.i
  %44 = getelementptr inbounds <4 x double> addrspace(1)* %33, i64 %extract104.i
  %45 = getelementptr inbounds <4 x double> addrspace(1)* %33, i64 %extract105.i
  %46 = getelementptr inbounds <4 x double> addrspace(1)* %33, i64 %extract106.i
  %47 = getelementptr inbounds <4 x double> addrspace(1)* %33, i64 %extract107.i
  %48 = getelementptr inbounds <4 x double> addrspace(1)* %33, i64 %extract108.i
  %49 = getelementptr inbounds <4 x double> addrspace(1)* %33, i64 %extract109.i
  %50 = getelementptr inbounds <4 x double> addrspace(1)* %33, i64 %extract110.i
  %51 = getelementptr inbounds <4 x double> addrspace(1)* %33, i64 %extract111.i
  %52 = getelementptr inbounds <4 x double> addrspace(1)* %33, i64 %extract112.i
  %53 = getelementptr inbounds <4 x double> addrspace(1)* %33, i64 %extract113.i
  %54 = getelementptr inbounds <4 x double> addrspace(1)* %33, i64 %extract114.i
  %55 = getelementptr inbounds <4 x double> addrspace(1)* %33, i64 %extract115.i
  %56 = getelementptr inbounds <4 x double> addrspace(1)* %33, i64 %extract116.i
  %57 = getelementptr inbounds <4 x double> addrspace(1)* %33, i64 %extract117.i
  br i1 %extract118.i, label %preload807.i, label %postload808.i

preload807.i:                                     ; preds = %while.body.i
  %extract.i = extractelement <16 x i64> %idxprom102.i, i32 0
  %58 = getelementptr inbounds <4 x double> addrspace(1)* %33, i64 %extract.i
  %masked_load641.i = load <4 x double> addrspace(1)* %58, align 32
  br label %postload808.i

postload808.i:                                    ; preds = %preload807.i, %while.body.i
  %phi809.i = phi <4 x double> [ undef, %while.body.i ], [ %masked_load641.i, %preload807.i ]
  br i1 %extract119.i, label %preload1007.i, label %postload1008.i

preload1007.i:                                    ; preds = %postload808.i
  %masked_load642.i = load <4 x double> addrspace(1)* %43, align 32
  br label %postload1008.i

postload1008.i:                                   ; preds = %preload1007.i, %postload808.i
  %phi1009.i = phi <4 x double> [ undef, %postload808.i ], [ %masked_load642.i, %preload1007.i ]
  br i1 %extract120.i, label %preload1197.i, label %postload1198.i

preload1197.i:                                    ; preds = %postload1008.i
  %masked_load643.i = load <4 x double> addrspace(1)* %44, align 32
  br label %postload1198.i

postload1198.i:                                   ; preds = %preload1197.i, %postload1008.i
  %phi1199.i = phi <4 x double> [ undef, %postload1008.i ], [ %masked_load643.i, %preload1197.i ]
  br i1 %extract121.i, label %preload816.i, label %postload817.i

preload816.i:                                     ; preds = %postload1198.i
  %masked_load644.i = load <4 x double> addrspace(1)* %45, align 32
  br label %postload817.i

postload817.i:                                    ; preds = %preload816.i, %postload1198.i
  %phi818.i = phi <4 x double> [ undef, %postload1198.i ], [ %masked_load644.i, %preload816.i ]
  br i1 %extract122.i, label %preload813.i, label %postload814.i

preload813.i:                                     ; preds = %postload817.i
  %masked_load645.i = load <4 x double> addrspace(1)* %46, align 32
  br label %postload814.i

postload814.i:                                    ; preds = %preload813.i, %postload817.i
  %phi815.i = phi <4 x double> [ undef, %postload817.i ], [ %masked_load645.i, %preload813.i ]
  br i1 %extract123.i, label %preload1232.i, label %postload1233.i

preload1232.i:                                    ; preds = %postload814.i
  %masked_load646.i = load <4 x double> addrspace(1)* %47, align 32
  br label %postload1233.i

postload1233.i:                                   ; preds = %preload1232.i, %postload814.i
  %phi1234.i = phi <4 x double> [ undef, %postload814.i ], [ %masked_load646.i, %preload1232.i ]
  br i1 %extract124.i, label %preload1203.i, label %postload1204.i

preload1203.i:                                    ; preds = %postload1233.i
  %masked_load647.i = load <4 x double> addrspace(1)* %48, align 32
  br label %postload1204.i

postload1204.i:                                   ; preds = %preload1203.i, %postload1233.i
  %phi1205.i = phi <4 x double> [ undef, %postload1233.i ], [ %masked_load647.i, %preload1203.i ]
  br i1 %extract125.i, label %preload1206.i, label %postload1207.i

preload1206.i:                                    ; preds = %postload1204.i
  %masked_load648.i = load <4 x double> addrspace(1)* %49, align 32
  br label %postload1207.i

postload1207.i:                                   ; preds = %preload1206.i, %postload1204.i
  %phi1208.i = phi <4 x double> [ undef, %postload1204.i ], [ %masked_load648.i, %preload1206.i ]
  br i1 %extract126.i, label %preload828.i, label %postload829.i

preload828.i:                                     ; preds = %postload1207.i
  %masked_load649.i = load <4 x double> addrspace(1)* %50, align 32
  br label %postload829.i

postload829.i:                                    ; preds = %preload828.i, %postload1207.i
  %phi830.i = phi <4 x double> [ undef, %postload1207.i ], [ %masked_load649.i, %preload828.i ]
  br i1 %extract127.i, label %preload831.i, label %postload832.i

preload831.i:                                     ; preds = %postload829.i
  %masked_load650.i = load <4 x double> addrspace(1)* %51, align 32
  br label %postload832.i

postload832.i:                                    ; preds = %preload831.i, %postload829.i
  %phi833.i = phi <4 x double> [ undef, %postload829.i ], [ %masked_load650.i, %preload831.i ]
  br i1 %extract128.i, label %preload834.i, label %postload835.i

preload834.i:                                     ; preds = %postload832.i
  %masked_load651.i = load <4 x double> addrspace(1)* %52, align 32
  br label %postload835.i

postload835.i:                                    ; preds = %preload834.i, %postload832.i
  %phi836.i = phi <4 x double> [ undef, %postload832.i ], [ %masked_load651.i, %preload834.i ]
  br i1 %extract129.i, label %preload1010.i, label %postload1011.i

preload1010.i:                                    ; preds = %postload835.i
  %masked_load652.i = load <4 x double> addrspace(1)* %53, align 32
  br label %postload1011.i

postload1011.i:                                   ; preds = %preload1010.i, %postload835.i
  %phi1012.i = phi <4 x double> [ undef, %postload835.i ], [ %masked_load652.i, %preload1010.i ]
  br i1 %extract130.i, label %preload1013.i, label %postload1014.i

preload1013.i:                                    ; preds = %postload1011.i
  %masked_load653.i = load <4 x double> addrspace(1)* %54, align 32
  br label %postload1014.i

postload1014.i:                                   ; preds = %preload1013.i, %postload1011.i
  %phi1015.i = phi <4 x double> [ undef, %postload1011.i ], [ %masked_load653.i, %preload1013.i ]
  br i1 %extract131.i, label %preload1016.i, label %postload1017.i

preload1016.i:                                    ; preds = %postload1014.i
  %masked_load654.i = load <4 x double> addrspace(1)* %55, align 32
  br label %postload1017.i

postload1017.i:                                   ; preds = %preload1016.i, %postload1014.i
  %phi1018.i = phi <4 x double> [ undef, %postload1014.i ], [ %masked_load654.i, %preload1016.i ]
  br i1 %extract132.i, label %preload948.i, label %postload949.i

preload948.i:                                     ; preds = %postload1017.i
  %masked_load655.i = load <4 x double> addrspace(1)* %56, align 32
  br label %postload949.i

postload949.i:                                    ; preds = %preload948.i, %postload1017.i
  %phi950.i = phi <4 x double> [ undef, %postload1017.i ], [ %masked_load655.i, %preload948.i ]
  br i1 %extract133.i, label %preload951.i, label %postload949.i.if.end.i_crit_edge

postload949.i.if.end.i_crit_edge:                 ; preds = %postload949.i
  br label %if.end.i

preload951.i:                                     ; preds = %postload949.i
  %masked_load656.i = load <4 x double> addrspace(1)* %57, align 32
  br label %if.end.i

if.end.i:                                         ; preds = %postload949.i.if.end.i_crit_edge, %preload951.i
  %phi953.i = phi <4 x double> [ %masked_load656.i, %preload951.i ], [ undef, %postload949.i.if.end.i_crit_edge ]
  %59 = extractelement <4 x double> %phi809.i, i32 3
  %60 = extractelement <4 x double> %phi809.i, i32 2
  %61 = extractelement <4 x double> %phi809.i, i32 1
  %62 = extractelement <4 x double> %phi809.i, i32 0
  %temp.vect136.i = insertelement <16 x double> undef, double %59, i32 0
  %63 = extractelement <4 x double> %phi1009.i, i32 3
  %temp.vect153.i = insertelement <16 x double> undef, double %60, i32 0
  %64 = extractelement <4 x double> %phi1009.i, i32 2
  %temp.vect170.i = insertelement <16 x double> undef, double %61, i32 0
  %65 = extractelement <4 x double> %phi1009.i, i32 1
  %temp.vect187.i = insertelement <16 x double> undef, double %62, i32 0
  %66 = extractelement <4 x double> %phi1009.i, i32 0
  %temp.vect137.i = insertelement <16 x double> %temp.vect136.i, double %63, i32 1
  %67 = extractelement <4 x double> %phi1199.i, i32 3
  %temp.vect154.i = insertelement <16 x double> %temp.vect153.i, double %64, i32 1
  %68 = extractelement <4 x double> %phi1199.i, i32 2
  %temp.vect171.i = insertelement <16 x double> %temp.vect170.i, double %65, i32 1
  %69 = extractelement <4 x double> %phi1199.i, i32 1
  %temp.vect188.i = insertelement <16 x double> %temp.vect187.i, double %66, i32 1
  %70 = extractelement <4 x double> %phi1199.i, i32 0
  %temp.vect138.i = insertelement <16 x double> %temp.vect137.i, double %67, i32 2
  %71 = extractelement <4 x double> %phi818.i, i32 3
  %temp.vect155.i = insertelement <16 x double> %temp.vect154.i, double %68, i32 2
  %72 = extractelement <4 x double> %phi818.i, i32 2
  %temp.vect172.i = insertelement <16 x double> %temp.vect171.i, double %69, i32 2
  %73 = extractelement <4 x double> %phi818.i, i32 1
  %temp.vect189.i = insertelement <16 x double> %temp.vect188.i, double %70, i32 2
  %74 = extractelement <4 x double> %phi818.i, i32 0
  %temp.vect139.i = insertelement <16 x double> %temp.vect138.i, double %71, i32 3
  %75 = extractelement <4 x double> %phi815.i, i32 3
  %temp.vect156.i = insertelement <16 x double> %temp.vect155.i, double %72, i32 3
  %76 = extractelement <4 x double> %phi815.i, i32 2
  %temp.vect173.i = insertelement <16 x double> %temp.vect172.i, double %73, i32 3
  %77 = extractelement <4 x double> %phi815.i, i32 1
  %temp.vect190.i = insertelement <16 x double> %temp.vect189.i, double %74, i32 3
  %78 = extractelement <4 x double> %phi815.i, i32 0
  %temp.vect140.i = insertelement <16 x double> %temp.vect139.i, double %75, i32 4
  %79 = extractelement <4 x double> %phi1234.i, i32 3
  %temp.vect157.i = insertelement <16 x double> %temp.vect156.i, double %76, i32 4
  %80 = extractelement <4 x double> %phi1234.i, i32 2
  %temp.vect174.i = insertelement <16 x double> %temp.vect173.i, double %77, i32 4
  %81 = extractelement <4 x double> %phi1234.i, i32 1
  %temp.vect191.i = insertelement <16 x double> %temp.vect190.i, double %78, i32 4
  %82 = extractelement <4 x double> %phi1234.i, i32 0
  %temp.vect141.i = insertelement <16 x double> %temp.vect140.i, double %79, i32 5
  %83 = extractelement <4 x double> %phi1205.i, i32 3
  %temp.vect158.i = insertelement <16 x double> %temp.vect157.i, double %80, i32 5
  %84 = extractelement <4 x double> %phi1205.i, i32 2
  %temp.vect175.i = insertelement <16 x double> %temp.vect174.i, double %81, i32 5
  %85 = extractelement <4 x double> %phi1205.i, i32 1
  %temp.vect192.i = insertelement <16 x double> %temp.vect191.i, double %82, i32 5
  %86 = extractelement <4 x double> %phi1205.i, i32 0
  %temp.vect142.i = insertelement <16 x double> %temp.vect141.i, double %83, i32 6
  %87 = extractelement <4 x double> %phi1208.i, i32 3
  %temp.vect159.i = insertelement <16 x double> %temp.vect158.i, double %84, i32 6
  %88 = extractelement <4 x double> %phi1208.i, i32 2
  %temp.vect176.i = insertelement <16 x double> %temp.vect175.i, double %85, i32 6
  %89 = extractelement <4 x double> %phi1208.i, i32 1
  %temp.vect193.i = insertelement <16 x double> %temp.vect192.i, double %86, i32 6
  %90 = extractelement <4 x double> %phi1208.i, i32 0
  %temp.vect143.i = insertelement <16 x double> %temp.vect142.i, double %87, i32 7
  %91 = extractelement <4 x double> %phi830.i, i32 3
  %temp.vect160.i = insertelement <16 x double> %temp.vect159.i, double %88, i32 7
  %92 = extractelement <4 x double> %phi830.i, i32 2
  %temp.vect177.i = insertelement <16 x double> %temp.vect176.i, double %89, i32 7
  %93 = extractelement <4 x double> %phi830.i, i32 1
  %temp.vect194.i = insertelement <16 x double> %temp.vect193.i, double %90, i32 7
  %94 = extractelement <4 x double> %phi830.i, i32 0
  %temp.vect144.i = insertelement <16 x double> %temp.vect143.i, double %91, i32 8
  %95 = extractelement <4 x double> %phi833.i, i32 3
  %temp.vect161.i = insertelement <16 x double> %temp.vect160.i, double %92, i32 8
  %96 = extractelement <4 x double> %phi833.i, i32 2
  %temp.vect178.i = insertelement <16 x double> %temp.vect177.i, double %93, i32 8
  %97 = extractelement <4 x double> %phi833.i, i32 1
  %temp.vect195.i = insertelement <16 x double> %temp.vect194.i, double %94, i32 8
  %98 = extractelement <4 x double> %phi833.i, i32 0
  %temp.vect145.i = insertelement <16 x double> %temp.vect144.i, double %95, i32 9
  %99 = extractelement <4 x double> %phi836.i, i32 3
  %temp.vect162.i = insertelement <16 x double> %temp.vect161.i, double %96, i32 9
  %100 = extractelement <4 x double> %phi836.i, i32 2
  %temp.vect179.i = insertelement <16 x double> %temp.vect178.i, double %97, i32 9
  %101 = extractelement <4 x double> %phi836.i, i32 1
  %temp.vect196.i = insertelement <16 x double> %temp.vect195.i, double %98, i32 9
  %102 = extractelement <4 x double> %phi836.i, i32 0
  %temp.vect146.i = insertelement <16 x double> %temp.vect145.i, double %99, i32 10
  %103 = extractelement <4 x double> %phi1012.i, i32 3
  %temp.vect163.i = insertelement <16 x double> %temp.vect162.i, double %100, i32 10
  %104 = extractelement <4 x double> %phi1012.i, i32 2
  %temp.vect180.i = insertelement <16 x double> %temp.vect179.i, double %101, i32 10
  %105 = extractelement <4 x double> %phi1012.i, i32 1
  %temp.vect197.i = insertelement <16 x double> %temp.vect196.i, double %102, i32 10
  %106 = extractelement <4 x double> %phi1012.i, i32 0
  %temp.vect147.i = insertelement <16 x double> %temp.vect146.i, double %103, i32 11
  %107 = extractelement <4 x double> %phi1015.i, i32 3
  %temp.vect164.i = insertelement <16 x double> %temp.vect163.i, double %104, i32 11
  %108 = extractelement <4 x double> %phi1015.i, i32 2
  %temp.vect181.i = insertelement <16 x double> %temp.vect180.i, double %105, i32 11
  %109 = extractelement <4 x double> %phi1015.i, i32 1
  %temp.vect198.i = insertelement <16 x double> %temp.vect197.i, double %106, i32 11
  %110 = extractelement <4 x double> %phi1015.i, i32 0
  %temp.vect148.i = insertelement <16 x double> %temp.vect147.i, double %107, i32 12
  %111 = extractelement <4 x double> %phi1018.i, i32 3
  %temp.vect165.i = insertelement <16 x double> %temp.vect164.i, double %108, i32 12
  %112 = extractelement <4 x double> %phi1018.i, i32 2
  %temp.vect182.i = insertelement <16 x double> %temp.vect181.i, double %109, i32 12
  %113 = extractelement <4 x double> %phi1018.i, i32 1
  %temp.vect199.i = insertelement <16 x double> %temp.vect198.i, double %110, i32 12
  %114 = extractelement <4 x double> %phi1018.i, i32 0
  %temp.vect149.i = insertelement <16 x double> %temp.vect148.i, double %111, i32 13
  %115 = extractelement <4 x double> %phi950.i, i32 3
  %temp.vect166.i = insertelement <16 x double> %temp.vect165.i, double %112, i32 13
  %116 = extractelement <4 x double> %phi950.i, i32 2
  %temp.vect183.i = insertelement <16 x double> %temp.vect182.i, double %113, i32 13
  %117 = extractelement <4 x double> %phi950.i, i32 1
  %temp.vect200.i = insertelement <16 x double> %temp.vect199.i, double %114, i32 13
  %118 = extractelement <4 x double> %phi950.i, i32 0
  %temp.vect150.i = insertelement <16 x double> %temp.vect149.i, double %115, i32 14
  %119 = extractelement <4 x double> %phi953.i, i32 3
  %temp.vect167.i = insertelement <16 x double> %temp.vect166.i, double %116, i32 14
  %120 = extractelement <4 x double> %phi953.i, i32 2
  %temp.vect184.i = insertelement <16 x double> %temp.vect183.i, double %117, i32 14
  %121 = extractelement <4 x double> %phi953.i, i32 1
  %temp.vect201.i = insertelement <16 x double> %temp.vect200.i, double %118, i32 14
  %122 = extractelement <4 x double> %phi953.i, i32 0
  %temp.vect151.i = insertelement <16 x double> %temp.vect150.i, double %119, i32 15
  %temp.vect168.i = insertelement <16 x double> %temp.vect167.i, double %120, i32 15
  %temp.vect185.i = insertelement <16 x double> %temp.vect184.i, double %121, i32 15
  %temp.vect202.i = insertelement <16 x double> %temp.vect201.i, double %122, i32 15
  %extract227.i = extractelement <16 x i1> %loadedValue1389.i, i32 0
  %"&(pSB[currWI].offset)1802.i" = add nuw i64 %CurrSBIndex..1.i, 1016
  %"&pSB[currWI].offset1803.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1802.i"
  %CastToValueType1804.i = bitcast i8* %"&pSB[currWI].offset1803.i" to i1*
  store i1 %extract227.i, i1* %CastToValueType1804.i, align 1
  %extract228.i = extractelement <16 x i1> %loadedValue1389.i, i32 1
  %"&(pSB[currWI].offset)1831.i" = add nuw i64 %CurrSBIndex..1.i, 1017
  %"&pSB[currWI].offset1832.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1831.i"
  %CastToValueType1833.i = bitcast i8* %"&pSB[currWI].offset1832.i" to i1*
  store i1 %extract228.i, i1* %CastToValueType1833.i, align 1
  %extract229.i = extractelement <16 x i1> %loadedValue1389.i, i32 2
  %"&(pSB[currWI].offset)1855.i" = add nuw i64 %CurrSBIndex..1.i, 1018
  %"&pSB[currWI].offset1856.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1855.i"
  %CastToValueType1857.i = bitcast i8* %"&pSB[currWI].offset1856.i" to i1*
  store i1 %extract229.i, i1* %CastToValueType1857.i, align 1
  %extract230.i = extractelement <16 x i1> %loadedValue1389.i, i32 3
  %"&(pSB[currWI].offset)1879.i" = add nuw i64 %CurrSBIndex..1.i, 1019
  %"&pSB[currWI].offset1880.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1879.i"
  %CastToValueType1881.i = bitcast i8* %"&pSB[currWI].offset1880.i" to i1*
  store i1 %extract230.i, i1* %CastToValueType1881.i, align 1
  %extract231.i = extractelement <16 x i1> %loadedValue1389.i, i32 4
  %"&(pSB[currWI].offset)1903.i" = add nuw i64 %CurrSBIndex..1.i, 1020
  %"&pSB[currWI].offset1904.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1903.i"
  %CastToValueType1905.i = bitcast i8* %"&pSB[currWI].offset1904.i" to i1*
  store i1 %extract231.i, i1* %CastToValueType1905.i, align 1
  %extract232.i = extractelement <16 x i1> %loadedValue1389.i, i32 5
  %"&(pSB[currWI].offset)1927.i" = add nuw i64 %CurrSBIndex..1.i, 1021
  %"&pSB[currWI].offset1928.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1927.i"
  %CastToValueType1929.i = bitcast i8* %"&pSB[currWI].offset1928.i" to i1*
  store i1 %extract232.i, i1* %CastToValueType1929.i, align 1
  %extract233.i = extractelement <16 x i1> %loadedValue1389.i, i32 6
  %"&(pSB[currWI].offset)1951.i" = add nuw i64 %CurrSBIndex..1.i, 1022
  %"&pSB[currWI].offset1952.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1951.i"
  %CastToValueType1953.i = bitcast i8* %"&pSB[currWI].offset1952.i" to i1*
  store i1 %extract233.i, i1* %CastToValueType1953.i, align 1
  %extract234.i = extractelement <16 x i1> %loadedValue1389.i, i32 7
  %"&(pSB[currWI].offset)1975.i" = add nuw i64 %CurrSBIndex..1.i, 1023
  %"&pSB[currWI].offset1976.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1975.i"
  %CastToValueType1977.i = bitcast i8* %"&pSB[currWI].offset1976.i" to i1*
  store i1 %extract234.i, i1* %CastToValueType1977.i, align 1
  %extract235.i = extractelement <16 x i1> %loadedValue1389.i, i32 8
  %"&(pSB[currWI].offset)1999.i" = add nuw i64 %CurrSBIndex..1.i, 1024
  %"&pSB[currWI].offset2000.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1999.i"
  %CastToValueType2001.i = bitcast i8* %"&pSB[currWI].offset2000.i" to i1*
  store i1 %extract235.i, i1* %CastToValueType2001.i, align 1
  %extract236.i = extractelement <16 x i1> %loadedValue1389.i, i32 9
  %"&(pSB[currWI].offset)2023.i" = add nuw i64 %CurrSBIndex..1.i, 1025
  %"&pSB[currWI].offset2024.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2023.i"
  %CastToValueType2025.i = bitcast i8* %"&pSB[currWI].offset2024.i" to i1*
  store i1 %extract236.i, i1* %CastToValueType2025.i, align 1
  %extract237.i = extractelement <16 x i1> %loadedValue1389.i, i32 10
  %"&(pSB[currWI].offset)2047.i" = add nuw i64 %CurrSBIndex..1.i, 1026
  %"&pSB[currWI].offset2048.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2047.i"
  %CastToValueType2049.i = bitcast i8* %"&pSB[currWI].offset2048.i" to i1*
  store i1 %extract237.i, i1* %CastToValueType2049.i, align 1
  %extract238.i = extractelement <16 x i1> %loadedValue1389.i, i32 11
  %"&(pSB[currWI].offset)2071.i" = add nuw i64 %CurrSBIndex..1.i, 1027
  %"&pSB[currWI].offset2072.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2071.i"
  %CastToValueType2073.i = bitcast i8* %"&pSB[currWI].offset2072.i" to i1*
  store i1 %extract238.i, i1* %CastToValueType2073.i, align 1
  %extract239.i = extractelement <16 x i1> %loadedValue1389.i, i32 12
  %"&(pSB[currWI].offset)2095.i" = add nuw i64 %CurrSBIndex..1.i, 1028
  %"&pSB[currWI].offset2096.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2095.i"
  %CastToValueType2097.i = bitcast i8* %"&pSB[currWI].offset2096.i" to i1*
  store i1 %extract239.i, i1* %CastToValueType2097.i, align 1
  %extract240.i = extractelement <16 x i1> %loadedValue1389.i, i32 13
  %"&(pSB[currWI].offset)2119.i" = add nuw i64 %CurrSBIndex..1.i, 1029
  %"&pSB[currWI].offset2120.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2119.i"
  %CastToValueType2121.i = bitcast i8* %"&pSB[currWI].offset2120.i" to i1*
  store i1 %extract240.i, i1* %CastToValueType2121.i, align 1
  %extract241.i = extractelement <16 x i1> %loadedValue1389.i, i32 14
  %"&(pSB[currWI].offset)2143.i" = add nuw i64 %CurrSBIndex..1.i, 1030
  %"&pSB[currWI].offset2144.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2143.i"
  %CastToValueType2145.i = bitcast i8* %"&pSB[currWI].offset2144.i" to i1*
  store i1 %extract241.i, i1* %CastToValueType2145.i, align 1
  %extract242.i = extractelement <16 x i1> %loadedValue1389.i, i32 15
  %"&(pSB[currWI].offset)2167.i" = add nuw i64 %CurrSBIndex..1.i, 1031
  %"&pSB[currWI].offset2168.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2167.i"
  %CastToValueType2169.i = bitcast i8* %"&pSB[currWI].offset2168.i" to i1*
  store i1 %extract242.i, i1* %CastToValueType2169.i, align 1
  %merge53152.i = select <16 x i1> %while.body_to_if.then101.i, <16 x double> %temp.vect151.i, <16 x double> zeroinitializer
  %merge51169.i = select <16 x i1> %while.body_to_if.then101.i, <16 x double> %temp.vect168.i, <16 x double> zeroinitializer
  %merge49186.i = select <16 x i1> %while.body_to_if.then101.i, <16 x double> %temp.vect185.i, <16 x double> zeroinitializer
  %merge203.i = select <16 x i1> %while.body_to_if.then101.i, <16 x double> %temp.vect202.i, <16 x double> zeroinitializer
  %"&(pSB[currWI].offset)2191.i" = add nuw i64 %CurrSBIndex..1.i, 1152
  %"&pSB[currWI].offset2192.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2191.i"
  %CastToValueType2193.i = bitcast i8* %"&pSB[currWI].offset2192.i" to <16 x double>*
  store <16 x double> %merge203.i, <16 x double>* %CastToValueType2193.i, align 128
  %add19204.i = fadd <16 x double> %merge49186.i, %merge203.i
  %"&(pSB[currWI].offset)2200.i" = add nuw i64 %CurrSBIndex..1.i, 1280
  %"&pSB[currWI].offset2201.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2200.i"
  %CastToValueType2202.i = bitcast i8* %"&pSB[currWI].offset2201.i" to <16 x double>*
  store <16 x double> %add19204.i, <16 x double>* %CastToValueType2202.i, align 128
  %add20205.i = fadd <16 x double> %merge51169.i, %add19204.i
  %"&(pSB[currWI].offset)2209.i" = add nuw i64 %CurrSBIndex..1.i, 1408
  %"&pSB[currWI].offset2210.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2209.i"
  %CastToValueType2211.i = bitcast i8* %"&pSB[currWI].offset2210.i" to <16 x double>*
  store <16 x double> %add20205.i, <16 x double>* %CastToValueType2211.i, align 128
  %add21206.i = fadd <16 x double> %merge53152.i, %add20205.i
  %"&(pSB[currWI].offset)2218.i" = add nuw i64 %CurrSBIndex..1.i, 1536
  %"&pSB[currWI].offset2219.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2218.i"
  %CastToValueType2220.i = bitcast i8* %"&pSB[currWI].offset2219.i" to <16 x double>*
  store <16 x double> %add21206.i, <16 x double>* %CastToValueType2220.i, align 128
  %extract278.i = extractelement <16 x double> %add21206.i, i32 0
  %extract279.i = extractelement <16 x double> %add21206.i, i32 1
  %extract280.i = extractelement <16 x double> %add21206.i, i32 2
  %extract281.i = extractelement <16 x double> %add21206.i, i32 3
  %extract282.i = extractelement <16 x double> %add21206.i, i32 4
  %extract283.i = extractelement <16 x double> %add21206.i, i32 5
  %extract284.i = extractelement <16 x double> %add21206.i, i32 6
  %extract285.i = extractelement <16 x double> %add21206.i, i32 7
  %extract286.i = extractelement <16 x double> %add21206.i, i32 8
  %extract287.i = extractelement <16 x double> %add21206.i, i32 9
  %extract288.i = extractelement <16 x double> %add21206.i, i32 10
  %extract289.i = extractelement <16 x double> %add21206.i, i32 11
  %extract290.i = extractelement <16 x double> %add21206.i, i32 12
  %extract291.i = extractelement <16 x double> %add21206.i, i32 13
  %extract292.i = extractelement <16 x double> %add21206.i, i32 14
  %extract293.i = extractelement <16 x double> %add21206.i, i32 15
  %"&pSB[currWI].offset1262.i" = getelementptr inbounds i8* %31, i64 %CurrSBIndex..1.i
  %CastToValueType1263.i = bitcast i8* %"&pSB[currWI].offset1262.i" to <16 x i64>*
  %loadedValue1264.i = load <16 x i64>* %CastToValueType1263.i, align 128
  %extract211.lhs.lhs.i = extractelement <16 x i64> %loadedValue1264.i, i32 0
  %extract211.lhs.i = shl i64 %extract211.lhs.lhs.i, 32
  %extract211.i = ashr exact i64 %extract211.lhs.i, 32
  br i1 %extract227.i, label %preload.i, label %postload.i

preload.i:                                        ; preds = %if.end.i
  %123 = getelementptr inbounds double addrspace(3)* %13, i64 %extract211.i
  store double 0.000000e+00, double addrspace(3)* %123, align 8
  %loadedValue1838.pre.i = load i1* %CastToValueType1833.i, align 1
  br label %postload.i

postload.i:                                       ; preds = %preload.i, %if.end.i
  %loadedValue1838.i = phi i1 [ %loadedValue1838.pre.i, %preload.i ], [ %extract228.i, %if.end.i ]
  br i1 %loadedValue1838.i, label %preload801.i, label %postload.i.postload802.i_crit_edge

postload.i.postload802.i_crit_edge:               ; preds = %postload.i
  br label %postload802.i

preload801.i:                                     ; preds = %postload.i
  %.sum1254.i = add i64 %extract211.i, 1
  %124 = getelementptr double addrspace(3)* %13, i64 %.sum1254.i
  store double 0.000000e+00, double addrspace(3)* %124, align 8
  br label %postload802.i

postload802.i:                                    ; preds = %postload.i.postload802.i_crit_edge, %preload801.i
  %loadedValue1862.i = load i1* %CastToValueType1857.i, align 1
  br i1 %loadedValue1862.i, label %preload939.i, label %postload802.i.postload940.i_crit_edge

postload802.i.postload940.i_crit_edge:            ; preds = %postload802.i
  br label %postload940.i

preload939.i:                                     ; preds = %postload802.i
  %.sum1253.i = add i64 %extract211.i, 2
  %125 = getelementptr double addrspace(3)* %13, i64 %.sum1253.i
  store double 0.000000e+00, double addrspace(3)* %125, align 8
  br label %postload940.i

postload940.i:                                    ; preds = %postload802.i.postload940.i_crit_edge, %preload939.i
  %loadedValue1886.i = load i1* %CastToValueType1881.i, align 1
  br i1 %loadedValue1886.i, label %preload945.i, label %postload940.i.postload946.i_crit_edge

postload940.i.postload946.i_crit_edge:            ; preds = %postload940.i
  br label %postload946.i

preload945.i:                                     ; preds = %postload940.i
  %.sum1252.i = add i64 %extract211.i, 3
  %126 = getelementptr double addrspace(3)* %13, i64 %.sum1252.i
  store double 0.000000e+00, double addrspace(3)* %126, align 8
  br label %postload946.i

postload946.i:                                    ; preds = %postload940.i.postload946.i_crit_edge, %preload945.i
  %loadedValue1910.i = load i1* %CastToValueType1905.i, align 1
  br i1 %loadedValue1910.i, label %preload825.i, label %postload946.i.postload826.i_crit_edge

postload946.i.postload826.i_crit_edge:            ; preds = %postload946.i
  br label %postload826.i

preload825.i:                                     ; preds = %postload946.i
  %.sum1251.i = add i64 %extract211.i, 4
  %127 = getelementptr double addrspace(3)* %13, i64 %.sum1251.i
  store double 0.000000e+00, double addrspace(3)* %127, align 8
  br label %postload826.i

postload826.i:                                    ; preds = %postload946.i.postload826.i_crit_edge, %preload825.i
  %loadedValue1934.i = load i1* %CastToValueType1929.i, align 1
  br i1 %loadedValue1934.i, label %preload954.i, label %postload826.i.postload955.i_crit_edge

postload826.i.postload955.i_crit_edge:            ; preds = %postload826.i
  br label %postload955.i

preload954.i:                                     ; preds = %postload826.i
  %.sum1250.i = add i64 %extract211.i, 5
  %128 = getelementptr double addrspace(3)* %13, i64 %.sum1250.i
  store double 0.000000e+00, double addrspace(3)* %128, align 8
  br label %postload955.i

postload955.i:                                    ; preds = %postload826.i.postload955.i_crit_edge, %preload954.i
  %loadedValue1958.i = load i1* %CastToValueType1953.i, align 1
  br i1 %loadedValue1958.i, label %preload1200.i, label %postload955.i.postload1201.i_crit_edge

postload955.i.postload1201.i_crit_edge:           ; preds = %postload955.i
  br label %postload1201.i

preload1200.i:                                    ; preds = %postload955.i
  %.sum1249.i = add i64 %extract211.i, 6
  %129 = getelementptr double addrspace(3)* %13, i64 %.sum1249.i
  store double 0.000000e+00, double addrspace(3)* %129, align 8
  br label %postload1201.i

postload1201.i:                                   ; preds = %postload955.i.postload1201.i_crit_edge, %preload1200.i
  %loadedValue1982.i = load i1* %CastToValueType1977.i, align 1
  br i1 %loadedValue1982.i, label %preload837.i, label %postload1201.i.postload838.i_crit_edge

postload1201.i.postload838.i_crit_edge:           ; preds = %postload1201.i
  br label %postload838.i

preload837.i:                                     ; preds = %postload1201.i
  %.sum1248.i = add i64 %extract211.i, 7
  %130 = getelementptr double addrspace(3)* %13, i64 %.sum1248.i
  store double 0.000000e+00, double addrspace(3)* %130, align 8
  br label %postload838.i

postload838.i:                                    ; preds = %postload1201.i.postload838.i_crit_edge, %preload837.i
  %loadedValue2006.i = load i1* %CastToValueType2001.i, align 1
  br i1 %loadedValue2006.i, label %preload1235.i, label %postload838.i.postload1236.i_crit_edge

postload838.i.postload1236.i_crit_edge:           ; preds = %postload838.i
  br label %postload1236.i

preload1235.i:                                    ; preds = %postload838.i
  %.sum1247.i = add i64 %extract211.i, 8
  %131 = getelementptr double addrspace(3)* %13, i64 %.sum1247.i
  store double 0.000000e+00, double addrspace(3)* %131, align 8
  br label %postload1236.i

postload1236.i:                                   ; preds = %postload838.i.postload1236.i_crit_edge, %preload1235.i
  %loadedValue2030.i = load i1* %CastToValueType2025.i, align 1
  br i1 %loadedValue2030.i, label %preload804.i, label %postload1236.i.postload805.i_crit_edge

postload1236.i.postload805.i_crit_edge:           ; preds = %postload1236.i
  br label %postload805.i

preload804.i:                                     ; preds = %postload1236.i
  %.sum1246.i = add i64 %extract211.i, 9
  %132 = getelementptr double addrspace(3)* %13, i64 %.sum1246.i
  store double 0.000000e+00, double addrspace(3)* %132, align 8
  br label %postload805.i

postload805.i:                                    ; preds = %postload1236.i.postload805.i_crit_edge, %preload804.i
  %loadedValue2054.i = load i1* %CastToValueType2049.i, align 1
  br i1 %loadedValue2054.i, label %preload942.i, label %postload805.i.postload943.i_crit_edge

postload805.i.postload943.i_crit_edge:            ; preds = %postload805.i
  br label %postload943.i

preload942.i:                                     ; preds = %postload805.i
  %.sum1245.i = add i64 %extract211.i, 10
  %133 = getelementptr double addrspace(3)* %13, i64 %.sum1245.i
  store double 0.000000e+00, double addrspace(3)* %133, align 8
  br label %postload943.i

postload943.i:                                    ; preds = %postload805.i.postload943.i_crit_edge, %preload942.i
  %loadedValue2078.i = load i1* %CastToValueType2073.i, align 1
  br i1 %loadedValue2078.i, label %preload1001.i, label %postload943.i.postload1002.i_crit_edge

postload943.i.postload1002.i_crit_edge:           ; preds = %postload943.i
  br label %postload1002.i

preload1001.i:                                    ; preds = %postload943.i
  %.sum1244.i = add i64 %extract211.i, 11
  %134 = getelementptr double addrspace(3)* %13, i64 %.sum1244.i
  store double 0.000000e+00, double addrspace(3)* %134, align 8
  br label %postload1002.i

postload1002.i:                                   ; preds = %postload943.i.postload1002.i_crit_edge, %preload1001.i
  %loadedValue2102.i = load i1* %CastToValueType2097.i, align 1
  br i1 %loadedValue2102.i, label %preload1004.i, label %postload1002.i.postload1005.i_crit_edge

postload1002.i.postload1005.i_crit_edge:          ; preds = %postload1002.i
  br label %postload1005.i

preload1004.i:                                    ; preds = %postload1002.i
  %.sum1243.i = add i64 %extract211.i, 12
  %135 = getelementptr double addrspace(3)* %13, i64 %.sum1243.i
  store double 0.000000e+00, double addrspace(3)* %135, align 8
  br label %postload1005.i

postload1005.i:                                   ; preds = %postload1002.i.postload1005.i_crit_edge, %preload1004.i
  %loadedValue2126.i = load i1* %CastToValueType2121.i, align 1
  br i1 %loadedValue2126.i, label %preload810.i, label %postload1005.i.postload811.i_crit_edge

postload1005.i.postload811.i_crit_edge:           ; preds = %postload1005.i
  br label %postload811.i

preload810.i:                                     ; preds = %postload1005.i
  %.sum1242.i = add i64 %extract211.i, 13
  %136 = getelementptr double addrspace(3)* %13, i64 %.sum1242.i
  store double 0.000000e+00, double addrspace(3)* %136, align 8
  br label %postload811.i

postload811.i:                                    ; preds = %postload1005.i.postload811.i_crit_edge, %preload810.i
  %loadedValue2150.i = load i1* %CastToValueType2145.i, align 1
  br i1 %loadedValue2150.i, label %preload936.i, label %postload811.i.postload937.i_crit_edge

postload811.i.postload937.i_crit_edge:            ; preds = %postload811.i
  br label %postload937.i

preload936.i:                                     ; preds = %postload811.i
  %.sum1241.i = add i64 %extract211.i, 14
  %137 = getelementptr double addrspace(3)* %13, i64 %.sum1241.i
  store double 0.000000e+00, double addrspace(3)* %137, align 8
  br label %postload937.i

postload937.i:                                    ; preds = %postload811.i.postload937.i_crit_edge, %preload936.i
  %loadedValue2174.i = load i1* %CastToValueType2169.i, align 1
  br i1 %loadedValue2174.i, label %preload1209.i, label %postload937.i.postload1210.i_crit_edge

postload937.i.postload1210.i_crit_edge:           ; preds = %postload937.i
  br label %postload1210.i

preload1209.i:                                    ; preds = %postload937.i
  %.sum.i = add i64 %extract211.i, 15
  %138 = getelementptr double addrspace(3)* %13, i64 %.sum.i
  store double 0.000000e+00, double addrspace(3)* %138, align 8
  br label %postload1210.i

postload1210.i:                                   ; preds = %postload937.i.postload1210.i_crit_edge, %preload1209.i
  %loadedValue1829.i = load i1* %CastToValueType1804.i, align 1
  br i1 %loadedValue1829.i, label %preload1019.i, label %postload1210.i.postload1020.i_crit_edge

postload1210.i.postload1020.i_crit_edge:          ; preds = %postload1210.i
  br label %postload1020.i

preload1019.i:                                    ; preds = %postload1210.i
  %139 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %140 = load i64* %139, align 8
  br label %postload1020.i

postload1020.i:                                   ; preds = %postload1210.i.postload1020.i_crit_edge, %preload1019.i
  %phi1021.i = phi i64 [ %140, %preload1019.i ], [ undef, %postload1210.i.postload1020.i_crit_edge ]
  %loadedValue1853.i = load i1* %CastToValueType1833.i, align 1
  br i1 %loadedValue1853.i, label %preload1032.i, label %postload1020.i.postload1033.i_crit_edge

postload1020.i.postload1033.i_crit_edge:          ; preds = %postload1020.i
  br label %postload1033.i

preload1032.i:                                    ; preds = %postload1020.i
  %141 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %142 = load i64* %141, align 8
  br label %postload1033.i

postload1033.i:                                   ; preds = %postload1020.i.postload1033.i_crit_edge, %preload1032.i
  %phi1034.i = phi i64 [ %142, %preload1032.i ], [ undef, %postload1020.i.postload1033.i_crit_edge ]
  %loadedValue1877.i = load i1* %CastToValueType1857.i, align 1
  br i1 %loadedValue1877.i, label %preload1043.i, label %postload1033.i.postload1044.i_crit_edge

postload1033.i.postload1044.i_crit_edge:          ; preds = %postload1033.i
  br label %postload1044.i

preload1043.i:                                    ; preds = %postload1033.i
  %143 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %144 = load i64* %143, align 8
  br label %postload1044.i

postload1044.i:                                   ; preds = %postload1033.i.postload1044.i_crit_edge, %preload1043.i
  %phi1045.i = phi i64 [ %144, %preload1043.i ], [ undef, %postload1033.i.postload1044.i_crit_edge ]
  %loadedValue1901.i = load i1* %CastToValueType1881.i, align 1
  br i1 %loadedValue1901.i, label %preload1054.i, label %postload1044.i.postload1055.i_crit_edge

postload1044.i.postload1055.i_crit_edge:          ; preds = %postload1044.i
  br label %postload1055.i

preload1054.i:                                    ; preds = %postload1044.i
  %145 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %146 = load i64* %145, align 8
  br label %postload1055.i

postload1055.i:                                   ; preds = %postload1044.i.postload1055.i_crit_edge, %preload1054.i
  %phi1056.i = phi i64 [ %146, %preload1054.i ], [ undef, %postload1044.i.postload1055.i_crit_edge ]
  %loadedValue1925.i = load i1* %CastToValueType1905.i, align 1
  br i1 %loadedValue1925.i, label %preload1065.i, label %postload1055.i.postload1066.i_crit_edge

postload1055.i.postload1066.i_crit_edge:          ; preds = %postload1055.i
  br label %postload1066.i

preload1065.i:                                    ; preds = %postload1055.i
  %147 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %148 = load i64* %147, align 8
  br label %postload1066.i

postload1066.i:                                   ; preds = %postload1055.i.postload1066.i_crit_edge, %preload1065.i
  %phi1067.i = phi i64 [ %148, %preload1065.i ], [ undef, %postload1055.i.postload1066.i_crit_edge ]
  %loadedValue1949.i = load i1* %CastToValueType1929.i, align 1
  br i1 %loadedValue1949.i, label %preload1076.i, label %postload1066.i.postload1077.i_crit_edge

postload1066.i.postload1077.i_crit_edge:          ; preds = %postload1066.i
  br label %postload1077.i

preload1076.i:                                    ; preds = %postload1066.i
  %149 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %150 = load i64* %149, align 8
  br label %postload1077.i

postload1077.i:                                   ; preds = %postload1066.i.postload1077.i_crit_edge, %preload1076.i
  %phi1078.i = phi i64 [ %150, %preload1076.i ], [ undef, %postload1066.i.postload1077.i_crit_edge ]
  %loadedValue1973.i = load i1* %CastToValueType1953.i, align 1
  br i1 %loadedValue1973.i, label %preload1087.i, label %postload1077.i.postload1088.i_crit_edge

postload1077.i.postload1088.i_crit_edge:          ; preds = %postload1077.i
  br label %postload1088.i

preload1087.i:                                    ; preds = %postload1077.i
  %151 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %152 = load i64* %151, align 8
  br label %postload1088.i

postload1088.i:                                   ; preds = %postload1077.i.postload1088.i_crit_edge, %preload1087.i
  %phi1089.i = phi i64 [ %152, %preload1087.i ], [ undef, %postload1077.i.postload1088.i_crit_edge ]
  %loadedValue1997.i = load i1* %CastToValueType1977.i, align 1
  br i1 %loadedValue1997.i, label %preload1098.i, label %postload1088.i.postload1099.i_crit_edge

postload1088.i.postload1099.i_crit_edge:          ; preds = %postload1088.i
  br label %postload1099.i

preload1098.i:                                    ; preds = %postload1088.i
  %153 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %154 = load i64* %153, align 8
  br label %postload1099.i

postload1099.i:                                   ; preds = %postload1088.i.postload1099.i_crit_edge, %preload1098.i
  %phi1100.i = phi i64 [ %154, %preload1098.i ], [ undef, %postload1088.i.postload1099.i_crit_edge ]
  %loadedValue2021.i = load i1* %CastToValueType2001.i, align 1
  br i1 %loadedValue2021.i, label %preload1109.i, label %postload1099.i.postload1110.i_crit_edge

postload1099.i.postload1110.i_crit_edge:          ; preds = %postload1099.i
  br label %postload1110.i

preload1109.i:                                    ; preds = %postload1099.i
  %155 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %156 = load i64* %155, align 8
  br label %postload1110.i

postload1110.i:                                   ; preds = %postload1099.i.postload1110.i_crit_edge, %preload1109.i
  %phi1111.i = phi i64 [ %156, %preload1109.i ], [ undef, %postload1099.i.postload1110.i_crit_edge ]
  %loadedValue2045.i = load i1* %CastToValueType2025.i, align 1
  br i1 %loadedValue2045.i, label %preload1120.i, label %postload1110.i.postload1121.i_crit_edge

postload1110.i.postload1121.i_crit_edge:          ; preds = %postload1110.i
  br label %postload1121.i

preload1120.i:                                    ; preds = %postload1110.i
  %157 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %158 = load i64* %157, align 8
  br label %postload1121.i

postload1121.i:                                   ; preds = %postload1110.i.postload1121.i_crit_edge, %preload1120.i
  %phi1122.i = phi i64 [ %158, %preload1120.i ], [ undef, %postload1110.i.postload1121.i_crit_edge ]
  %loadedValue2069.i = load i1* %CastToValueType2049.i, align 1
  br i1 %loadedValue2069.i, label %preload1131.i, label %postload1121.i.postload1132.i_crit_edge

postload1121.i.postload1132.i_crit_edge:          ; preds = %postload1121.i
  br label %postload1132.i

preload1131.i:                                    ; preds = %postload1121.i
  %159 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %160 = load i64* %159, align 8
  br label %postload1132.i

postload1132.i:                                   ; preds = %postload1121.i.postload1132.i_crit_edge, %preload1131.i
  %phi1133.i = phi i64 [ %160, %preload1131.i ], [ undef, %postload1121.i.postload1132.i_crit_edge ]
  %loadedValue2093.i = load i1* %CastToValueType2073.i, align 1
  br i1 %loadedValue2093.i, label %preload1142.i, label %postload1132.i.postload1143.i_crit_edge

postload1132.i.postload1143.i_crit_edge:          ; preds = %postload1132.i
  br label %postload1143.i

preload1142.i:                                    ; preds = %postload1132.i
  %161 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %162 = load i64* %161, align 8
  br label %postload1143.i

postload1143.i:                                   ; preds = %postload1132.i.postload1143.i_crit_edge, %preload1142.i
  %phi1144.i = phi i64 [ %162, %preload1142.i ], [ undef, %postload1132.i.postload1143.i_crit_edge ]
  %loadedValue2117.i = load i1* %CastToValueType2097.i, align 1
  br i1 %loadedValue2117.i, label %preload1153.i, label %postload1143.i.postload1154.i_crit_edge

postload1143.i.postload1154.i_crit_edge:          ; preds = %postload1143.i
  br label %postload1154.i

preload1153.i:                                    ; preds = %postload1143.i
  %163 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %164 = load i64* %163, align 8
  br label %postload1154.i

postload1154.i:                                   ; preds = %postload1143.i.postload1154.i_crit_edge, %preload1153.i
  %phi1155.i = phi i64 [ %164, %preload1153.i ], [ undef, %postload1143.i.postload1154.i_crit_edge ]
  %loadedValue2141.i = load i1* %CastToValueType2121.i, align 1
  br i1 %loadedValue2141.i, label %preload1164.i, label %postload1154.i.postload1165.i_crit_edge

postload1154.i.postload1165.i_crit_edge:          ; preds = %postload1154.i
  br label %postload1165.i

preload1164.i:                                    ; preds = %postload1154.i
  %165 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %166 = load i64* %165, align 8
  br label %postload1165.i

postload1165.i:                                   ; preds = %postload1154.i.postload1165.i_crit_edge, %preload1164.i
  %phi1166.i = phi i64 [ %166, %preload1164.i ], [ undef, %postload1154.i.postload1165.i_crit_edge ]
  %loadedValue2165.i = load i1* %CastToValueType2145.i, align 1
  br i1 %loadedValue2165.i, label %preload1175.i, label %postload1165.i.postload1176.i_crit_edge

postload1165.i.postload1176.i_crit_edge:          ; preds = %postload1165.i
  br label %postload1176.i

preload1175.i:                                    ; preds = %postload1165.i
  %167 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %168 = load i64* %167, align 8
  br label %postload1176.i

postload1176.i:                                   ; preds = %postload1165.i.postload1176.i_crit_edge, %preload1175.i
  %phi1177.i = phi i64 [ %168, %preload1175.i ], [ undef, %postload1165.i.postload1176.i_crit_edge ]
  %loadedValue2189.i = load i1* %CastToValueType2169.i, align 1
  br i1 %loadedValue2189.i, label %preload1186.i, label %postload1176.i.postload1187.i_crit_edge

postload1176.i.postload1187.i_crit_edge:          ; preds = %postload1176.i
  br label %postload1187.i

preload1186.i:                                    ; preds = %postload1176.i
  %169 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %170 = load i64* %169, align 8
  br label %postload1187.i

postload1187.i:                                   ; preds = %postload1176.i.postload1187.i_crit_edge, %preload1186.i
  %phi1188.i = phi i64 [ %170, %preload1186.i ], [ undef, %postload1176.i.postload1187.i_crit_edge ]
  %temp.vect243.i = insertelement <16 x i64> undef, i64 %phi1021.i, i32 0
  %temp.vect244.i = insertelement <16 x i64> %temp.vect243.i, i64 %phi1034.i, i32 1
  %temp.vect245.i = insertelement <16 x i64> %temp.vect244.i, i64 %phi1045.i, i32 2
  %temp.vect246.i = insertelement <16 x i64> %temp.vect245.i, i64 %phi1056.i, i32 3
  %temp.vect247.i = insertelement <16 x i64> %temp.vect246.i, i64 %phi1067.i, i32 4
  %temp.vect248.i = insertelement <16 x i64> %temp.vect247.i, i64 %phi1078.i, i32 5
  %temp.vect249.i = insertelement <16 x i64> %temp.vect248.i, i64 %phi1089.i, i32 6
  %temp.vect250.i = insertelement <16 x i64> %temp.vect249.i, i64 %phi1100.i, i32 7
  %temp.vect251.i = insertelement <16 x i64> %temp.vect250.i, i64 %phi1111.i, i32 8
  %temp.vect252.i = insertelement <16 x i64> %temp.vect251.i, i64 %phi1122.i, i32 9
  %temp.vect253.i = insertelement <16 x i64> %temp.vect252.i, i64 %phi1133.i, i32 10
  %temp.vect254.i = insertelement <16 x i64> %temp.vect253.i, i64 %phi1144.i, i32 11
  %temp.vect255.i = insertelement <16 x i64> %temp.vect254.i, i64 %phi1155.i, i32 12
  %temp.vect256.i = insertelement <16 x i64> %temp.vect255.i, i64 %phi1166.i, i32 13
  %temp.vect257.i = insertelement <16 x i64> %temp.vect256.i, i64 %phi1177.i, i32 14
  %temp.vect258.i = insertelement <16 x i64> %temp.vect257.i, i64 %phi1188.i, i32 15
  %"&(pSB[currWI].offset)2227.i" = add nuw i64 %CurrSBIndex..1.i, 1664
  %"&pSB[currWI].offset2228.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2227.i"
  %CastToValueType2229.i = bitcast i8* %"&pSB[currWI].offset2228.i" to <16 x i64>*
  store <16 x i64> %temp.vect258.i, <16 x i64>* %CastToValueType2229.i, align 128
  %loadedValue.i = load <16 x i64>* %CastToValueType1263.i, align 128
  %add.i259.i = add <16 x i64> %temp.vect258.i, %loadedValue.i
  %conv3.i260.i = trunc <16 x i64> %add.i259.i to <16 x i32>
  %"&(pSB[currWI].offset)2241.i" = add nuw i64 %CurrSBIndex..1.i, 1792
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
  %171 = getelementptr inbounds double addrspace(3)* %13, i64 %extract262.i
  %"&(pSB[currWI].offset)2255.i" = add nuw i64 %CurrSBIndex..1.i, 1856
  %"&pSB[currWI].offset2256.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2255.i"
  %CastToValueType2257.i = bitcast i8* %"&pSB[currWI].offset2256.i" to double addrspace(3)**
  store double addrspace(3)* %171, double addrspace(3)** %CastToValueType2257.i, align 8
  %172 = getelementptr inbounds double addrspace(3)* %13, i64 %extract263.i
  %"&(pSB[currWI].offset)2274.i" = add nuw i64 %CurrSBIndex..1.i, 1864
  %"&pSB[currWI].offset2275.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2274.i"
  %CastToValueType2276.i = bitcast i8* %"&pSB[currWI].offset2275.i" to double addrspace(3)**
  store double addrspace(3)* %172, double addrspace(3)** %CastToValueType2276.i, align 8
  %173 = getelementptr inbounds double addrspace(3)* %13, i64 %extract264.i
  %"&(pSB[currWI].offset)2293.i" = add nuw i64 %CurrSBIndex..1.i, 1872
  %"&pSB[currWI].offset2294.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2293.i"
  %CastToValueType2295.i = bitcast i8* %"&pSB[currWI].offset2294.i" to double addrspace(3)**
  store double addrspace(3)* %173, double addrspace(3)** %CastToValueType2295.i, align 8
  %174 = getelementptr inbounds double addrspace(3)* %13, i64 %extract265.i
  %"&(pSB[currWI].offset)2312.i" = add nuw i64 %CurrSBIndex..1.i, 1880
  %"&pSB[currWI].offset2313.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2312.i"
  %CastToValueType2314.i = bitcast i8* %"&pSB[currWI].offset2313.i" to double addrspace(3)**
  store double addrspace(3)* %174, double addrspace(3)** %CastToValueType2314.i, align 8
  %175 = getelementptr inbounds double addrspace(3)* %13, i64 %extract266.i
  %"&(pSB[currWI].offset)2331.i" = add nuw i64 %CurrSBIndex..1.i, 1888
  %"&pSB[currWI].offset2332.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2331.i"
  %CastToValueType2333.i = bitcast i8* %"&pSB[currWI].offset2332.i" to double addrspace(3)**
  store double addrspace(3)* %175, double addrspace(3)** %CastToValueType2333.i, align 8
  %176 = getelementptr inbounds double addrspace(3)* %13, i64 %extract267.i
  %"&(pSB[currWI].offset)2350.i" = add nuw i64 %CurrSBIndex..1.i, 1896
  %"&pSB[currWI].offset2351.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2350.i"
  %CastToValueType2352.i = bitcast i8* %"&pSB[currWI].offset2351.i" to double addrspace(3)**
  store double addrspace(3)* %176, double addrspace(3)** %CastToValueType2352.i, align 8
  %177 = getelementptr inbounds double addrspace(3)* %13, i64 %extract268.i
  %"&(pSB[currWI].offset)2369.i" = add nuw i64 %CurrSBIndex..1.i, 1904
  %"&pSB[currWI].offset2370.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2369.i"
  %CastToValueType2371.i = bitcast i8* %"&pSB[currWI].offset2370.i" to double addrspace(3)**
  store double addrspace(3)* %177, double addrspace(3)** %CastToValueType2371.i, align 8
  %178 = getelementptr inbounds double addrspace(3)* %13, i64 %extract269.i
  %"&(pSB[currWI].offset)2388.i" = add nuw i64 %CurrSBIndex..1.i, 1912
  %"&pSB[currWI].offset2389.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2388.i"
  %CastToValueType2390.i = bitcast i8* %"&pSB[currWI].offset2389.i" to double addrspace(3)**
  store double addrspace(3)* %178, double addrspace(3)** %CastToValueType2390.i, align 8
  %179 = getelementptr inbounds double addrspace(3)* %13, i64 %extract270.i
  %"&(pSB[currWI].offset)2407.i" = add nuw i64 %CurrSBIndex..1.i, 1920
  %"&pSB[currWI].offset2408.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2407.i"
  %CastToValueType2409.i = bitcast i8* %"&pSB[currWI].offset2408.i" to double addrspace(3)**
  store double addrspace(3)* %179, double addrspace(3)** %CastToValueType2409.i, align 8
  %180 = getelementptr inbounds double addrspace(3)* %13, i64 %extract271.i
  %"&(pSB[currWI].offset)2426.i" = add nuw i64 %CurrSBIndex..1.i, 1928
  %"&pSB[currWI].offset2427.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2426.i"
  %CastToValueType2428.i = bitcast i8* %"&pSB[currWI].offset2427.i" to double addrspace(3)**
  store double addrspace(3)* %180, double addrspace(3)** %CastToValueType2428.i, align 8
  %181 = getelementptr inbounds double addrspace(3)* %13, i64 %extract272.i
  %"&(pSB[currWI].offset)2445.i" = add nuw i64 %CurrSBIndex..1.i, 1936
  %"&pSB[currWI].offset2446.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2445.i"
  %CastToValueType2447.i = bitcast i8* %"&pSB[currWI].offset2446.i" to double addrspace(3)**
  store double addrspace(3)* %181, double addrspace(3)** %CastToValueType2447.i, align 8
  %182 = getelementptr inbounds double addrspace(3)* %13, i64 %extract273.i
  %"&(pSB[currWI].offset)2464.i" = add nuw i64 %CurrSBIndex..1.i, 1944
  %"&pSB[currWI].offset2465.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2464.i"
  %CastToValueType2466.i = bitcast i8* %"&pSB[currWI].offset2465.i" to double addrspace(3)**
  store double addrspace(3)* %182, double addrspace(3)** %CastToValueType2466.i, align 8
  %183 = getelementptr inbounds double addrspace(3)* %13, i64 %extract274.i
  %"&(pSB[currWI].offset)2483.i" = add nuw i64 %CurrSBIndex..1.i, 1952
  %"&pSB[currWI].offset2484.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2483.i"
  %CastToValueType2485.i = bitcast i8* %"&pSB[currWI].offset2484.i" to double addrspace(3)**
  store double addrspace(3)* %183, double addrspace(3)** %CastToValueType2485.i, align 8
  %184 = getelementptr inbounds double addrspace(3)* %13, i64 %extract275.i
  %"&(pSB[currWI].offset)2502.i" = add nuw i64 %CurrSBIndex..1.i, 1960
  %"&pSB[currWI].offset2503.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2502.i"
  %CastToValueType2504.i = bitcast i8* %"&pSB[currWI].offset2503.i" to double addrspace(3)**
  store double addrspace(3)* %184, double addrspace(3)** %CastToValueType2504.i, align 8
  %185 = getelementptr inbounds double addrspace(3)* %13, i64 %extract276.i
  %"&(pSB[currWI].offset)2521.i" = add nuw i64 %CurrSBIndex..1.i, 1968
  %"&pSB[currWI].offset2522.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2521.i"
  %CastToValueType2523.i = bitcast i8* %"&pSB[currWI].offset2522.i" to double addrspace(3)**
  store double addrspace(3)* %185, double addrspace(3)** %CastToValueType2523.i, align 8
  %186 = getelementptr inbounds double addrspace(3)* %13, i64 %extract277.i
  %"&(pSB[currWI].offset)2540.i" = add nuw i64 %CurrSBIndex..1.i, 1976
  %"&pSB[currWI].offset2541.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2540.i"
  %CastToValueType2542.i = bitcast i8* %"&pSB[currWI].offset2541.i" to double addrspace(3)**
  store double addrspace(3)* %186, double addrspace(3)** %CastToValueType2542.i, align 8
  br i1 %loadedValue1829.i, label %preload1022.i, label %postload1023.i

preload1022.i:                                    ; preds = %postload1187.i
  store double %extract278.i, double addrspace(3)* %171, align 8
  %loadedValue1848.pre.i = load i1* %CastToValueType1833.i, align 1
  br label %postload1023.i

postload1023.i:                                   ; preds = %preload1022.i, %postload1187.i
  %loadedValue1848.i = phi i1 [ %loadedValue1848.pre.i, %preload1022.i ], [ %loadedValue1853.i, %postload1187.i ]
  br i1 %loadedValue1848.i, label %preload1035.i, label %postload1023.i.postload1036.i_crit_edge

postload1023.i.postload1036.i_crit_edge:          ; preds = %postload1023.i
  br label %postload1036.i

preload1035.i:                                    ; preds = %postload1023.i
  %loadedValue2291.i = load double addrspace(3)** %CastToValueType2276.i, align 8
  store double %extract279.i, double addrspace(3)* %loadedValue2291.i, align 8
  br label %postload1036.i

postload1036.i:                                   ; preds = %postload1023.i.postload1036.i_crit_edge, %preload1035.i
  %loadedValue1872.i = load i1* %CastToValueType1857.i, align 1
  br i1 %loadedValue1872.i, label %preload1046.i, label %postload1036.i.postload1047.i_crit_edge

postload1036.i.postload1047.i_crit_edge:          ; preds = %postload1036.i
  br label %postload1047.i

preload1046.i:                                    ; preds = %postload1036.i
  %loadedValue2310.i = load double addrspace(3)** %CastToValueType2295.i, align 8
  store double %extract280.i, double addrspace(3)* %loadedValue2310.i, align 8
  br label %postload1047.i

postload1047.i:                                   ; preds = %postload1036.i.postload1047.i_crit_edge, %preload1046.i
  %loadedValue1896.i = load i1* %CastToValueType1881.i, align 1
  br i1 %loadedValue1896.i, label %preload1057.i, label %postload1047.i.postload1058.i_crit_edge

postload1047.i.postload1058.i_crit_edge:          ; preds = %postload1047.i
  br label %postload1058.i

preload1057.i:                                    ; preds = %postload1047.i
  %loadedValue2329.i = load double addrspace(3)** %CastToValueType2314.i, align 8
  store double %extract281.i, double addrspace(3)* %loadedValue2329.i, align 8
  br label %postload1058.i

postload1058.i:                                   ; preds = %postload1047.i.postload1058.i_crit_edge, %preload1057.i
  %loadedValue1920.i = load i1* %CastToValueType1905.i, align 1
  br i1 %loadedValue1920.i, label %preload1068.i, label %postload1058.i.postload1069.i_crit_edge

postload1058.i.postload1069.i_crit_edge:          ; preds = %postload1058.i
  br label %postload1069.i

preload1068.i:                                    ; preds = %postload1058.i
  %loadedValue2348.i = load double addrspace(3)** %CastToValueType2333.i, align 8
  store double %extract282.i, double addrspace(3)* %loadedValue2348.i, align 8
  br label %postload1069.i

postload1069.i:                                   ; preds = %postload1058.i.postload1069.i_crit_edge, %preload1068.i
  %loadedValue1944.i = load i1* %CastToValueType1929.i, align 1
  br i1 %loadedValue1944.i, label %preload1079.i, label %postload1069.i.postload1080.i_crit_edge

postload1069.i.postload1080.i_crit_edge:          ; preds = %postload1069.i
  br label %postload1080.i

preload1079.i:                                    ; preds = %postload1069.i
  %loadedValue2367.i = load double addrspace(3)** %CastToValueType2352.i, align 8
  store double %extract283.i, double addrspace(3)* %loadedValue2367.i, align 8
  br label %postload1080.i

postload1080.i:                                   ; preds = %postload1069.i.postload1080.i_crit_edge, %preload1079.i
  %loadedValue1968.i = load i1* %CastToValueType1953.i, align 1
  br i1 %loadedValue1968.i, label %preload1090.i, label %postload1080.i.postload1091.i_crit_edge

postload1080.i.postload1091.i_crit_edge:          ; preds = %postload1080.i
  br label %postload1091.i

preload1090.i:                                    ; preds = %postload1080.i
  %loadedValue2386.i = load double addrspace(3)** %CastToValueType2371.i, align 8
  store double %extract284.i, double addrspace(3)* %loadedValue2386.i, align 8
  br label %postload1091.i

postload1091.i:                                   ; preds = %postload1080.i.postload1091.i_crit_edge, %preload1090.i
  %loadedValue1992.i = load i1* %CastToValueType1977.i, align 1
  br i1 %loadedValue1992.i, label %preload1101.i, label %postload1091.i.postload1102.i_crit_edge

postload1091.i.postload1102.i_crit_edge:          ; preds = %postload1091.i
  br label %postload1102.i

preload1101.i:                                    ; preds = %postload1091.i
  %loadedValue2405.i = load double addrspace(3)** %CastToValueType2390.i, align 8
  store double %extract285.i, double addrspace(3)* %loadedValue2405.i, align 8
  br label %postload1102.i

postload1102.i:                                   ; preds = %postload1091.i.postload1102.i_crit_edge, %preload1101.i
  %loadedValue2016.i = load i1* %CastToValueType2001.i, align 1
  br i1 %loadedValue2016.i, label %preload1112.i, label %postload1102.i.postload1113.i_crit_edge

postload1102.i.postload1113.i_crit_edge:          ; preds = %postload1102.i
  br label %postload1113.i

preload1112.i:                                    ; preds = %postload1102.i
  %loadedValue2424.i = load double addrspace(3)** %CastToValueType2409.i, align 8
  store double %extract286.i, double addrspace(3)* %loadedValue2424.i, align 8
  br label %postload1113.i

postload1113.i:                                   ; preds = %postload1102.i.postload1113.i_crit_edge, %preload1112.i
  %loadedValue2040.i = load i1* %CastToValueType2025.i, align 1
  br i1 %loadedValue2040.i, label %preload1123.i, label %postload1113.i.postload1124.i_crit_edge

postload1113.i.postload1124.i_crit_edge:          ; preds = %postload1113.i
  br label %postload1124.i

preload1123.i:                                    ; preds = %postload1113.i
  %loadedValue2443.i = load double addrspace(3)** %CastToValueType2428.i, align 8
  store double %extract287.i, double addrspace(3)* %loadedValue2443.i, align 8
  br label %postload1124.i

postload1124.i:                                   ; preds = %postload1113.i.postload1124.i_crit_edge, %preload1123.i
  %loadedValue2064.i = load i1* %CastToValueType2049.i, align 1
  br i1 %loadedValue2064.i, label %preload1134.i, label %postload1124.i.postload1135.i_crit_edge

postload1124.i.postload1135.i_crit_edge:          ; preds = %postload1124.i
  br label %postload1135.i

preload1134.i:                                    ; preds = %postload1124.i
  %loadedValue2462.i = load double addrspace(3)** %CastToValueType2447.i, align 8
  store double %extract288.i, double addrspace(3)* %loadedValue2462.i, align 8
  br label %postload1135.i

postload1135.i:                                   ; preds = %postload1124.i.postload1135.i_crit_edge, %preload1134.i
  %loadedValue2088.i = load i1* %CastToValueType2073.i, align 1
  br i1 %loadedValue2088.i, label %preload1145.i, label %postload1135.i.postload1146.i_crit_edge

postload1135.i.postload1146.i_crit_edge:          ; preds = %postload1135.i
  br label %postload1146.i

preload1145.i:                                    ; preds = %postload1135.i
  %loadedValue2481.i = load double addrspace(3)** %CastToValueType2466.i, align 8
  store double %extract289.i, double addrspace(3)* %loadedValue2481.i, align 8
  br label %postload1146.i

postload1146.i:                                   ; preds = %postload1135.i.postload1146.i_crit_edge, %preload1145.i
  %loadedValue2112.i = load i1* %CastToValueType2097.i, align 1
  br i1 %loadedValue2112.i, label %preload1156.i, label %postload1146.i.postload1157.i_crit_edge

postload1146.i.postload1157.i_crit_edge:          ; preds = %postload1146.i
  br label %postload1157.i

preload1156.i:                                    ; preds = %postload1146.i
  %loadedValue2500.i = load double addrspace(3)** %CastToValueType2485.i, align 8
  store double %extract290.i, double addrspace(3)* %loadedValue2500.i, align 8
  br label %postload1157.i

postload1157.i:                                   ; preds = %postload1146.i.postload1157.i_crit_edge, %preload1156.i
  %loadedValue2136.i = load i1* %CastToValueType2121.i, align 1
  br i1 %loadedValue2136.i, label %preload1167.i, label %postload1157.i.postload1168.i_crit_edge

postload1157.i.postload1168.i_crit_edge:          ; preds = %postload1157.i
  br label %postload1168.i

preload1167.i:                                    ; preds = %postload1157.i
  %loadedValue2519.i = load double addrspace(3)** %CastToValueType2504.i, align 8
  store double %extract291.i, double addrspace(3)* %loadedValue2519.i, align 8
  br label %postload1168.i

postload1168.i:                                   ; preds = %postload1157.i.postload1168.i_crit_edge, %preload1167.i
  %loadedValue2160.i = load i1* %CastToValueType2145.i, align 1
  br i1 %loadedValue2160.i, label %preload1178.i, label %postload1168.i.postload1179.i_crit_edge

postload1168.i.postload1179.i_crit_edge:          ; preds = %postload1168.i
  br label %postload1179.i

preload1178.i:                                    ; preds = %postload1168.i
  %loadedValue2538.i = load double addrspace(3)** %CastToValueType2523.i, align 8
  store double %extract292.i, double addrspace(3)* %loadedValue2538.i, align 8
  br label %postload1179.i

postload1179.i:                                   ; preds = %postload1168.i.postload1179.i_crit_edge, %preload1178.i
  %loadedValue2184.i = load i1* %CastToValueType2169.i, align 1
  br i1 %loadedValue2184.i, label %preload1189.i, label %postload1179.i.postload1190.i_crit_edge

postload1179.i.postload1190.i_crit_edge:          ; preds = %postload1179.i
  br label %postload1190.i

preload1189.i:                                    ; preds = %postload1179.i
  %loadedValue2557.i = load double addrspace(3)** %CastToValueType2542.i, align 8
  store double %extract293.i, double addrspace(3)* %loadedValue2557.i, align 8
  br label %postload1190.i

postload1190.i:                                   ; preds = %postload1179.i.postload1190.i_crit_edge, %preload1189.i
  %loadedValue1819.i = load i1* %CastToValueType1804.i, align 1
  br i1 %loadedValue1819.i, label %preload1024.i, label %postload1190.i.postload1025.i_crit_edge

postload1190.i.postload1025.i_crit_edge:          ; preds = %postload1190.i
  br label %postload1025.i

preload1024.i:                                    ; preds = %postload1190.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..1.i, %28
  br i1 %check.WI.iter.i, label %thenBB.i, label %preload1024.i.postload1025.i_crit_edge

preload1024.i.postload1025.i_crit_edge:           ; preds = %preload1024.i
  br label %postload1025.i

thenBB.i:                                         ; preds = %preload1024.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..1.i, 2176
  switch i32 %currBarrier.1.i, label %SyncBB2941.i [
    i32 20, label %thenBB.i.SyncBB2943.i_crit_edge
    i32 7, label %postload776.i
    i32 6, label %SyncBB2939.i
    i32 5, label %thenBB.i.postload1025.i_crit_edge
  ]

thenBB.i.SyncBB2943.i_crit_edge:                  ; preds = %thenBB.i
  br label %SyncBB2943.i

thenBB.i.postload1025.i_crit_edge:                ; preds = %thenBB.i
  br label %postload1025.i

postload1025.i:                                   ; preds = %thenBB2961.i.postload1025.i_crit_edge, %thenBB2969.i.postload1025.i_crit_edge, %thenBB2953.i.postload1025.i_crit_edge, %thenBB2945.i.postload1025.i_crit_edge, %thenBB.i.postload1025.i_crit_edge, %preload1024.i.postload1025.i_crit_edge, %postload1190.i.postload1025.i_crit_edge
  %currBarrier.3.i = phi i32 [ %currBarrier.1.i, %postload1190.i.postload1025.i_crit_edge ], [ 5, %preload1024.i.postload1025.i_crit_edge ], [ %currBarrier.1.i, %thenBB.i.postload1025.i_crit_edge ], [ %currBarrier.4.i, %thenBB2945.i.postload1025.i_crit_edge ], [ %currBarrier.6.i, %thenBB2953.i.postload1025.i_crit_edge ], [ %currBarrier.9.i, %thenBB2969.i.postload1025.i_crit_edge ], [ %currBarrier.12.i, %thenBB2961.i.postload1025.i_crit_edge ]
  %CurrSBIndex..3.i = phi i64 [ %CurrSBIndex..1.i, %postload1190.i.postload1025.i_crit_edge ], [ 0, %preload1024.i.postload1025.i_crit_edge ], [ %"loadedCurrSB+Stride.i", %thenBB.i.postload1025.i_crit_edge ], [ %"loadedCurrSB+Stride2951.i", %thenBB2945.i.postload1025.i_crit_edge ], [ %"loadedCurrSB+Stride2959.i", %thenBB2953.i.postload1025.i_crit_edge ], [ %"loadedCurrSB+Stride2975.i", %thenBB2969.i.postload1025.i_crit_edge ], [ %"loadedCurrSB+Stride2967.i", %thenBB2961.i.postload1025.i_crit_edge ]
  %CurrWI..3.i = phi i64 [ %CurrWI..1.i, %postload1190.i.postload1025.i_crit_edge ], [ 0, %preload1024.i.postload1025.i_crit_edge ], [ %"CurrWI++.i", %thenBB.i.postload1025.i_crit_edge ], [ %"CurrWI++2949.i", %thenBB2945.i.postload1025.i_crit_edge ], [ %"CurrWI++2957.i", %thenBB2953.i.postload1025.i_crit_edge ], [ %"CurrWI++2973.i", %thenBB2969.i.postload1025.i_crit_edge ], [ %"CurrWI++2965.i", %thenBB2961.i.postload1025.i_crit_edge ]
  %"&(pSB[currWI].offset)2236.i" = add nuw i64 %CurrSBIndex..3.i, 1664
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

for.body.i.i:                                     ; preds = %postload776.i, %postload1025.i
  %currBarrier.4.i = phi i32 [ %currBarrier.8.i, %postload776.i ], [ %currBarrier.3.i, %postload1025.i ]
  %CurrSBIndex..4.i = phi i64 [ %CurrSBIndex..8.i, %postload776.i ], [ %CurrSBIndex..3.i, %postload1025.i ]
  %CurrWI..4.i = phi i64 [ %CurrWI..8.i, %postload776.i ], [ %CurrWI..3.i, %postload1025.i ]
  %vectorPHI296.i = phi <16 x i1> [ %loop_mask15389.i, %postload776.i ], [ %negIncomingLoopMask40295.i, %postload1025.i ]
  %vectorPHI298.i = phi <16 x i1> [ %local_edge408.i, %postload776.i ], [ %if.end_to_for.body.i.preheader294.i, %postload1025.i ]
  %i.02.i.i = phi i32 [ %mul.i.i, %postload776.i ], [ 1, %postload1025.i ]
  %"&(pSB[currWI].offset)2582.i" = add nuw i64 %CurrSBIndex..4.i, 2004
  %"&pSB[currWI].offset2583.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2582.i"
  %CastToValueType2584.i = bitcast i8* %"&pSB[currWI].offset2583.i" to i32*
  store i32 %i.02.i.i, i32* %CastToValueType2584.i, align 4
  %"&(pSB[currWI].offset)2568.i" = add nuw i64 %CurrSBIndex..4.i, 2000
  %"&pSB[currWI].offset2569.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2568.i"
  %CastToValueType2570.i = bitcast i8* %"&pSB[currWI].offset2569.i" to <16 x i1>*
  store <16 x i1> %vectorPHI298.i, <16 x i1>* %CastToValueType2570.i, align 16
  %"&(pSB[currWI].offset)2559.i" = add nuw i64 %CurrSBIndex..4.i, 1984
  %"&pSB[currWI].offset2560.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2559.i"
  %CastToValueType2561.i = bitcast i8* %"&pSB[currWI].offset2560.i" to <16 x i1>*
  store <16 x i1> %vectorPHI296.i, <16 x i1>* %CastToValueType2561.i, align 16
  %extract319.i = extractelement <16 x i1> %vectorPHI298.i, i32 0
  %"&(pSB[currWI].offset)2591.i" = add nuw i64 %CurrSBIndex..4.i, 2008
  %"&pSB[currWI].offset2592.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2591.i"
  %CastToValueType2593.i = bitcast i8* %"&pSB[currWI].offset2592.i" to i1*
  store i1 %extract319.i, i1* %CastToValueType2593.i, align 1
  %extract320.i = extractelement <16 x i1> %vectorPHI298.i, i32 1
  %"&(pSB[currWI].offset)2610.i" = add nuw i64 %CurrSBIndex..4.i, 2009
  %"&pSB[currWI].offset2611.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2610.i"
  %CastToValueType2612.i = bitcast i8* %"&pSB[currWI].offset2611.i" to i1*
  store i1 %extract320.i, i1* %CastToValueType2612.i, align 1
  %extract321.i = extractelement <16 x i1> %vectorPHI298.i, i32 2
  %"&(pSB[currWI].offset)2629.i" = add nuw i64 %CurrSBIndex..4.i, 2010
  %"&pSB[currWI].offset2630.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2629.i"
  %CastToValueType2631.i = bitcast i8* %"&pSB[currWI].offset2630.i" to i1*
  store i1 %extract321.i, i1* %CastToValueType2631.i, align 1
  %extract322.i = extractelement <16 x i1> %vectorPHI298.i, i32 3
  %"&(pSB[currWI].offset)2648.i" = add nuw i64 %CurrSBIndex..4.i, 2011
  %"&pSB[currWI].offset2649.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2648.i"
  %CastToValueType2650.i = bitcast i8* %"&pSB[currWI].offset2649.i" to i1*
  store i1 %extract322.i, i1* %CastToValueType2650.i, align 1
  %extract323.i = extractelement <16 x i1> %vectorPHI298.i, i32 4
  %"&(pSB[currWI].offset)2667.i" = add nuw i64 %CurrSBIndex..4.i, 2012
  %"&pSB[currWI].offset2668.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2667.i"
  %CastToValueType2669.i = bitcast i8* %"&pSB[currWI].offset2668.i" to i1*
  store i1 %extract323.i, i1* %CastToValueType2669.i, align 1
  %extract324.i = extractelement <16 x i1> %vectorPHI298.i, i32 5
  %"&(pSB[currWI].offset)2686.i" = add nuw i64 %CurrSBIndex..4.i, 2013
  %"&pSB[currWI].offset2687.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2686.i"
  %CastToValueType2688.i = bitcast i8* %"&pSB[currWI].offset2687.i" to i1*
  store i1 %extract324.i, i1* %CastToValueType2688.i, align 1
  %extract325.i = extractelement <16 x i1> %vectorPHI298.i, i32 6
  %"&(pSB[currWI].offset)2705.i" = add nuw i64 %CurrSBIndex..4.i, 2014
  %"&pSB[currWI].offset2706.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2705.i"
  %CastToValueType2707.i = bitcast i8* %"&pSB[currWI].offset2706.i" to i1*
  store i1 %extract325.i, i1* %CastToValueType2707.i, align 1
  %extract326.i = extractelement <16 x i1> %vectorPHI298.i, i32 7
  %"&(pSB[currWI].offset)2724.i" = add nuw i64 %CurrSBIndex..4.i, 2015
  %"&pSB[currWI].offset2725.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2724.i"
  %CastToValueType2726.i = bitcast i8* %"&pSB[currWI].offset2725.i" to i1*
  store i1 %extract326.i, i1* %CastToValueType2726.i, align 1
  %extract327.i = extractelement <16 x i1> %vectorPHI298.i, i32 8
  %"&(pSB[currWI].offset)2743.i" = add nuw i64 %CurrSBIndex..4.i, 2016
  %"&pSB[currWI].offset2744.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2743.i"
  %CastToValueType2745.i = bitcast i8* %"&pSB[currWI].offset2744.i" to i1*
  store i1 %extract327.i, i1* %CastToValueType2745.i, align 1
  %extract328.i = extractelement <16 x i1> %vectorPHI298.i, i32 9
  %"&(pSB[currWI].offset)2762.i" = add nuw i64 %CurrSBIndex..4.i, 2017
  %"&pSB[currWI].offset2763.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2762.i"
  %CastToValueType2764.i = bitcast i8* %"&pSB[currWI].offset2763.i" to i1*
  store i1 %extract328.i, i1* %CastToValueType2764.i, align 1
  %extract329.i = extractelement <16 x i1> %vectorPHI298.i, i32 10
  %"&(pSB[currWI].offset)2781.i" = add nuw i64 %CurrSBIndex..4.i, 2018
  %"&pSB[currWI].offset2782.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2781.i"
  %CastToValueType2783.i = bitcast i8* %"&pSB[currWI].offset2782.i" to i1*
  store i1 %extract329.i, i1* %CastToValueType2783.i, align 1
  %extract330.i = extractelement <16 x i1> %vectorPHI298.i, i32 11
  %"&(pSB[currWI].offset)2800.i" = add nuw i64 %CurrSBIndex..4.i, 2019
  %"&pSB[currWI].offset2801.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2800.i"
  %CastToValueType2802.i = bitcast i8* %"&pSB[currWI].offset2801.i" to i1*
  store i1 %extract330.i, i1* %CastToValueType2802.i, align 1
  %extract331.i = extractelement <16 x i1> %vectorPHI298.i, i32 12
  %"&(pSB[currWI].offset)2819.i" = add nuw i64 %CurrSBIndex..4.i, 2020
  %"&pSB[currWI].offset2820.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2819.i"
  %CastToValueType2821.i = bitcast i8* %"&pSB[currWI].offset2820.i" to i1*
  store i1 %extract331.i, i1* %CastToValueType2821.i, align 1
  %extract332.i = extractelement <16 x i1> %vectorPHI298.i, i32 13
  %"&(pSB[currWI].offset)2838.i" = add nuw i64 %CurrSBIndex..4.i, 2021
  %"&pSB[currWI].offset2839.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2838.i"
  %CastToValueType2840.i = bitcast i8* %"&pSB[currWI].offset2839.i" to i1*
  store i1 %extract332.i, i1* %CastToValueType2840.i, align 1
  %extract333.i = extractelement <16 x i1> %vectorPHI298.i, i32 14
  %"&(pSB[currWI].offset)2857.i" = add nuw i64 %CurrSBIndex..4.i, 2022
  %"&pSB[currWI].offset2858.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2857.i"
  %CastToValueType2859.i = bitcast i8* %"&pSB[currWI].offset2858.i" to i1*
  store i1 %extract333.i, i1* %CastToValueType2859.i, align 1
  %extract334.i = extractelement <16 x i1> %vectorPHI298.i, i32 15
  %"&(pSB[currWI].offset)2876.i" = add nuw i64 %CurrSBIndex..4.i, 2023
  %"&pSB[currWI].offset2877.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2876.i"
  %CastToValueType2878.i = bitcast i8* %"&pSB[currWI].offset2877.i" to i1*
  store i1 %extract334.i, i1* %CastToValueType2878.i, align 1
  %temp299.i = insertelement <16 x i32> undef, i32 %i.02.i.i, i32 0
  %vector300.i = shufflevector <16 x i32> %temp299.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)2250.i" = add nuw i64 %CurrSBIndex..4.i, 1792
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
  %188 = getelementptr inbounds double addrspace(3)* %13, i64 %extract304.i
  %189 = getelementptr inbounds double addrspace(3)* %13, i64 %extract305.i
  %190 = getelementptr inbounds double addrspace(3)* %13, i64 %extract306.i
  %191 = getelementptr inbounds double addrspace(3)* %13, i64 %extract307.i
  %192 = getelementptr inbounds double addrspace(3)* %13, i64 %extract308.i
  %193 = getelementptr inbounds double addrspace(3)* %13, i64 %extract309.i
  %194 = getelementptr inbounds double addrspace(3)* %13, i64 %extract310.i
  %195 = getelementptr inbounds double addrspace(3)* %13, i64 %extract311.i
  %196 = getelementptr inbounds double addrspace(3)* %13, i64 %extract312.i
  %197 = getelementptr inbounds double addrspace(3)* %13, i64 %extract313.i
  %198 = getelementptr inbounds double addrspace(3)* %13, i64 %extract314.i
  %199 = getelementptr inbounds double addrspace(3)* %13, i64 %extract315.i
  %200 = getelementptr inbounds double addrspace(3)* %13, i64 %extract316.i
  %201 = getelementptr inbounds double addrspace(3)* %13, i64 %extract317.i
  %202 = getelementptr inbounds double addrspace(3)* %13, i64 %extract318.i
  br i1 %extract319.i, label %preload767.i, label %postload768.i

preload767.i:                                     ; preds = %for.body.i.i
  %extract303.i = extractelement <16 x i64> %idxprom9.i302.i, i32 0
  %203 = getelementptr inbounds double addrspace(3)* %13, i64 %extract303.i
  %masked_load703.i = load double addrspace(3)* %203, align 8
  br label %postload768.i

postload768.i:                                    ; preds = %preload767.i, %for.body.i.i
  %phi769.i = phi double [ undef, %for.body.i.i ], [ %masked_load703.i, %preload767.i ]
  br i1 %extract320.i, label %preload777.i, label %postload778.i

preload777.i:                                     ; preds = %postload768.i
  %masked_load704.i = load double addrspace(3)* %188, align 8
  br label %postload778.i

postload778.i:                                    ; preds = %preload777.i, %postload768.i
  %phi779.i = phi double [ undef, %postload768.i ], [ %masked_load704.i, %preload777.i ]
  br i1 %extract321.i, label %preload785.i, label %postload786.i

preload785.i:                                     ; preds = %postload778.i
  %masked_load705.i = load double addrspace(3)* %189, align 8
  br label %postload786.i

postload786.i:                                    ; preds = %preload785.i, %postload778.i
  %phi787.i = phi double [ undef, %postload778.i ], [ %masked_load705.i, %preload785.i ]
  br i1 %extract322.i, label %preload793.i, label %postload794.i

preload793.i:                                     ; preds = %postload786.i
  %masked_load706.i = load double addrspace(3)* %190, align 8
  br label %postload794.i

postload794.i:                                    ; preds = %preload793.i, %postload786.i
  %phi795.i = phi double [ undef, %postload786.i ], [ %masked_load706.i, %preload793.i ]
  br i1 %extract323.i, label %preload840.i, label %postload841.i

preload840.i:                                     ; preds = %postload794.i
  %masked_load707.i = load double addrspace(3)* %191, align 8
  br label %postload841.i

postload841.i:                                    ; preds = %preload840.i, %postload794.i
  %phi842.i = phi double [ undef, %postload794.i ], [ %masked_load707.i, %preload840.i ]
  br i1 %extract324.i, label %preload848.i, label %postload849.i

preload848.i:                                     ; preds = %postload841.i
  %masked_load708.i = load double addrspace(3)* %192, align 8
  br label %postload849.i

postload849.i:                                    ; preds = %preload848.i, %postload841.i
  %phi850.i = phi double [ undef, %postload841.i ], [ %masked_load708.i, %preload848.i ]
  br i1 %extract325.i, label %preload856.i, label %postload857.i

preload856.i:                                     ; preds = %postload849.i
  %masked_load709.i = load double addrspace(3)* %193, align 8
  br label %postload857.i

postload857.i:                                    ; preds = %preload856.i, %postload849.i
  %phi858.i = phi double [ undef, %postload849.i ], [ %masked_load709.i, %preload856.i ]
  br i1 %extract326.i, label %preload864.i, label %postload865.i

preload864.i:                                     ; preds = %postload857.i
  %masked_load710.i = load double addrspace(3)* %194, align 8
  br label %postload865.i

postload865.i:                                    ; preds = %preload864.i, %postload857.i
  %phi866.i = phi double [ undef, %postload857.i ], [ %masked_load710.i, %preload864.i ]
  br i1 %extract327.i, label %preload872.i, label %postload873.i

preload872.i:                                     ; preds = %postload865.i
  %masked_load711.i = load double addrspace(3)* %195, align 8
  br label %postload873.i

postload873.i:                                    ; preds = %preload872.i, %postload865.i
  %phi874.i = phi double [ undef, %postload865.i ], [ %masked_load711.i, %preload872.i ]
  br i1 %extract328.i, label %preload880.i, label %postload881.i

preload880.i:                                     ; preds = %postload873.i
  %masked_load712.i = load double addrspace(3)* %196, align 8
  br label %postload881.i

postload881.i:                                    ; preds = %preload880.i, %postload873.i
  %phi882.i = phi double [ undef, %postload873.i ], [ %masked_load712.i, %preload880.i ]
  br i1 %extract329.i, label %preload888.i, label %postload889.i

preload888.i:                                     ; preds = %postload881.i
  %masked_load713.i = load double addrspace(3)* %197, align 8
  br label %postload889.i

postload889.i:                                    ; preds = %preload888.i, %postload881.i
  %phi890.i = phi double [ undef, %postload881.i ], [ %masked_load713.i, %preload888.i ]
  br i1 %extract330.i, label %preload896.i, label %postload897.i

preload896.i:                                     ; preds = %postload889.i
  %masked_load714.i = load double addrspace(3)* %198, align 8
  br label %postload897.i

postload897.i:                                    ; preds = %preload896.i, %postload889.i
  %phi898.i = phi double [ undef, %postload889.i ], [ %masked_load714.i, %preload896.i ]
  br i1 %extract331.i, label %preload904.i, label %postload905.i

preload904.i:                                     ; preds = %postload897.i
  %masked_load715.i = load double addrspace(3)* %199, align 8
  br label %postload905.i

postload905.i:                                    ; preds = %preload904.i, %postload897.i
  %phi906.i = phi double [ undef, %postload897.i ], [ %masked_load715.i, %preload904.i ]
  br i1 %extract332.i, label %preload912.i, label %postload913.i

preload912.i:                                     ; preds = %postload905.i
  %masked_load716.i = load double addrspace(3)* %200, align 8
  br label %postload913.i

postload913.i:                                    ; preds = %preload912.i, %postload905.i
  %phi914.i = phi double [ undef, %postload905.i ], [ %masked_load716.i, %preload912.i ]
  br i1 %extract333.i, label %preload920.i, label %postload921.i

preload920.i:                                     ; preds = %postload913.i
  %masked_load717.i = load double addrspace(3)* %201, align 8
  br label %postload921.i

postload921.i:                                    ; preds = %preload920.i, %postload913.i
  %phi922.i = phi double [ undef, %postload913.i ], [ %masked_load717.i, %preload920.i ]
  br i1 %extract334.i, label %preload928.i, label %postload929.i

preload928.i:                                     ; preds = %postload921.i
  %masked_load718.i = load double addrspace(3)* %202, align 8
  br label %postload929.i

postload929.i:                                    ; preds = %preload928.i, %postload921.i
  %phi930.i = phi double [ undef, %postload921.i ], [ %masked_load718.i, %preload928.i ]
  %temp.vect351.i = insertelement <16 x double> undef, double %phi769.i, i32 0
  %temp.vect352.i = insertelement <16 x double> %temp.vect351.i, double %phi779.i, i32 1
  %temp.vect353.i = insertelement <16 x double> %temp.vect352.i, double %phi787.i, i32 2
  %temp.vect354.i = insertelement <16 x double> %temp.vect353.i, double %phi795.i, i32 3
  %temp.vect355.i = insertelement <16 x double> %temp.vect354.i, double %phi842.i, i32 4
  %temp.vect356.i = insertelement <16 x double> %temp.vect355.i, double %phi850.i, i32 5
  %temp.vect357.i = insertelement <16 x double> %temp.vect356.i, double %phi858.i, i32 6
  %temp.vect358.i = insertelement <16 x double> %temp.vect357.i, double %phi866.i, i32 7
  %temp.vect359.i = insertelement <16 x double> %temp.vect358.i, double %phi874.i, i32 8
  %temp.vect360.i = insertelement <16 x double> %temp.vect359.i, double %phi882.i, i32 9
  %temp.vect361.i = insertelement <16 x double> %temp.vect360.i, double %phi890.i, i32 10
  %temp.vect362.i = insertelement <16 x double> %temp.vect361.i, double %phi898.i, i32 11
  %temp.vect363.i = insertelement <16 x double> %temp.vect362.i, double %phi906.i, i32 12
  %temp.vect364.i = insertelement <16 x double> %temp.vect363.i, double %phi914.i, i32 13
  %temp.vect365.i = insertelement <16 x double> %temp.vect364.i, double %phi922.i, i32 14
  %temp.vect366.i = insertelement <16 x double> %temp.vect365.i, double %phi930.i, i32 15
  %"&(pSB[currWI].offset)2895.i" = add nuw i64 %CurrSBIndex..4.i, 2048
  %"&pSB[currWI].offset2896.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2895.i"
  %CastToValueType2897.i = bitcast i8* %"&pSB[currWI].offset2896.i" to <16 x double>*
  store <16 x double> %temp.vect366.i, <16 x double>* %CastToValueType2897.i, align 128
  br i1 %extract319.i, label %preload770.i, label %postload771.i

preload770.i:                                     ; preds = %postload929.i
  %check.WI.iter2948.i = icmp ult i64 %CurrWI..4.i, %28
  br i1 %check.WI.iter2948.i, label %thenBB2945.i, label %SyncBB2939.i

thenBB2945.i:                                     ; preds = %preload770.i
  %"CurrWI++2949.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride2951.i" = add nuw i64 %CurrSBIndex..4.i, 2176
  switch i32 %currBarrier.4.i, label %thenBB2945.i.postload1025.i_crit_edge [
    i32 29, label %SyncBB2941.i
    i32 20, label %thenBB2945.i.SyncBB2943.i_crit_edge
    i32 7, label %postload776.i
    i32 6, label %SyncBB2939.i
  ]

thenBB2945.i.SyncBB2943.i_crit_edge:              ; preds = %thenBB2945.i
  br label %SyncBB2943.i

thenBB2945.i.postload1025.i_crit_edge:            ; preds = %thenBB2945.i
  br label %postload1025.i

SyncBB2939.i:                                     ; preds = %thenBB2961.i, %thenBB2969.i, %thenBB2953.i, %thenBB2945.i, %preload770.i, %thenBB.i
  %currBarrier.5.i = phi i32 [ %currBarrier.4.i, %thenBB2945.i ], [ %currBarrier.6.i, %thenBB2953.i ], [ %currBarrier.9.i, %thenBB2969.i ], [ %currBarrier.1.i, %thenBB.i ], [ %currBarrier.12.i, %thenBB2961.i ], [ 6, %preload770.i ]
  %CurrSBIndex..5.i = phi i64 [ %"loadedCurrSB+Stride2951.i", %thenBB2945.i ], [ %"loadedCurrSB+Stride2959.i", %thenBB2953.i ], [ %"loadedCurrSB+Stride2975.i", %thenBB2969.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ %"loadedCurrSB+Stride2967.i", %thenBB2961.i ], [ 0, %preload770.i ]
  %CurrWI..5.i = phi i64 [ %"CurrWI++2949.i", %thenBB2945.i ], [ %"CurrWI++2957.i", %thenBB2953.i ], [ %"CurrWI++2973.i", %thenBB2969.i ], [ %"CurrWI++.i", %thenBB.i ], [ %"CurrWI++2965.i", %thenBB2961.i ], [ 0, %preload770.i ]
  %"&(pSB[currWI].offset)2264.i" = add nuw i64 %CurrSBIndex..5.i, 1856
  %"&pSB[currWI].offset2265.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2264.i"
  %CastToValueType2266.i = bitcast i8* %"&pSB[currWI].offset2265.i" to double addrspace(3)**
  %loadedValue2267.i = load double addrspace(3)** %CastToValueType2266.i, align 8
  %masked_load719.i = load double addrspace(3)* %loadedValue2267.i, align 8
  br label %postload771.i

postload771.i:                                    ; preds = %SyncBB2939.i, %postload929.i
  %currBarrier.6.i = phi i32 [ %currBarrier.5.i, %SyncBB2939.i ], [ %currBarrier.4.i, %postload929.i ]
  %CurrSBIndex..6.i = phi i64 [ %CurrSBIndex..5.i, %SyncBB2939.i ], [ %CurrSBIndex..4.i, %postload929.i ]
  %CurrWI..6.i = phi i64 [ %CurrWI..5.i, %SyncBB2939.i ], [ %CurrWI..4.i, %postload929.i ]
  %phi772.i = phi double [ %masked_load719.i, %SyncBB2939.i ], [ undef, %postload929.i ]
  %"&(pSB[currWI].offset)2619.i" = add nuw i64 %CurrSBIndex..6.i, 2009
  %"&pSB[currWI].offset2620.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2619.i"
  %CastToValueType2621.i = bitcast i8* %"&pSB[currWI].offset2620.i" to i1*
  %loadedValue2622.i = load i1* %CastToValueType2621.i, align 1
  br i1 %loadedValue2622.i, label %preload780.i, label %postload771.i.postload781.i_crit_edge

postload771.i.postload781.i_crit_edge:            ; preds = %postload771.i
  br label %postload781.i

preload780.i:                                     ; preds = %postload771.i
  %"&(pSB[currWI].offset)2283.i" = add nuw i64 %CurrSBIndex..6.i, 1864
  %"&pSB[currWI].offset2284.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2283.i"
  %CastToValueType2285.i = bitcast i8* %"&pSB[currWI].offset2284.i" to double addrspace(3)**
  %loadedValue2286.i = load double addrspace(3)** %CastToValueType2285.i, align 8
  %masked_load720.i = load double addrspace(3)* %loadedValue2286.i, align 8
  br label %postload781.i

postload781.i:                                    ; preds = %postload771.i.postload781.i_crit_edge, %preload780.i
  %phi782.i = phi double [ %masked_load720.i, %preload780.i ], [ undef, %postload771.i.postload781.i_crit_edge ]
  %"&(pSB[currWI].offset)2638.i" = add nuw i64 %CurrSBIndex..6.i, 2010
  %"&pSB[currWI].offset2639.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2638.i"
  %CastToValueType2640.i = bitcast i8* %"&pSB[currWI].offset2639.i" to i1*
  %loadedValue2641.i = load i1* %CastToValueType2640.i, align 1
  br i1 %loadedValue2641.i, label %preload788.i, label %postload781.i.postload789.i_crit_edge

postload781.i.postload789.i_crit_edge:            ; preds = %postload781.i
  br label %postload789.i

preload788.i:                                     ; preds = %postload781.i
  %"&(pSB[currWI].offset)2302.i" = add nuw i64 %CurrSBIndex..6.i, 1872
  %"&pSB[currWI].offset2303.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2302.i"
  %CastToValueType2304.i = bitcast i8* %"&pSB[currWI].offset2303.i" to double addrspace(3)**
  %loadedValue2305.i = load double addrspace(3)** %CastToValueType2304.i, align 8
  %masked_load721.i = load double addrspace(3)* %loadedValue2305.i, align 8
  br label %postload789.i

postload789.i:                                    ; preds = %postload781.i.postload789.i_crit_edge, %preload788.i
  %phi790.i = phi double [ %masked_load721.i, %preload788.i ], [ undef, %postload781.i.postload789.i_crit_edge ]
  %"&(pSB[currWI].offset)2657.i" = add nuw i64 %CurrSBIndex..6.i, 2011
  %"&pSB[currWI].offset2658.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2657.i"
  %CastToValueType2659.i = bitcast i8* %"&pSB[currWI].offset2658.i" to i1*
  %loadedValue2660.i = load i1* %CastToValueType2659.i, align 1
  br i1 %loadedValue2660.i, label %preload796.i, label %postload789.i.postload797.i_crit_edge

postload789.i.postload797.i_crit_edge:            ; preds = %postload789.i
  br label %postload797.i

preload796.i:                                     ; preds = %postload789.i
  %"&(pSB[currWI].offset)2321.i" = add nuw i64 %CurrSBIndex..6.i, 1880
  %"&pSB[currWI].offset2322.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2321.i"
  %CastToValueType2323.i = bitcast i8* %"&pSB[currWI].offset2322.i" to double addrspace(3)**
  %loadedValue2324.i = load double addrspace(3)** %CastToValueType2323.i, align 8
  %masked_load722.i = load double addrspace(3)* %loadedValue2324.i, align 8
  br label %postload797.i

postload797.i:                                    ; preds = %postload789.i.postload797.i_crit_edge, %preload796.i
  %phi798.i = phi double [ %masked_load722.i, %preload796.i ], [ undef, %postload789.i.postload797.i_crit_edge ]
  %"&(pSB[currWI].offset)2676.i" = add nuw i64 %CurrSBIndex..6.i, 2012
  %"&pSB[currWI].offset2677.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2676.i"
  %CastToValueType2678.i = bitcast i8* %"&pSB[currWI].offset2677.i" to i1*
  %loadedValue2679.i = load i1* %CastToValueType2678.i, align 1
  br i1 %loadedValue2679.i, label %preload843.i, label %postload797.i.postload844.i_crit_edge

postload797.i.postload844.i_crit_edge:            ; preds = %postload797.i
  br label %postload844.i

preload843.i:                                     ; preds = %postload797.i
  %"&(pSB[currWI].offset)2340.i" = add nuw i64 %CurrSBIndex..6.i, 1888
  %"&pSB[currWI].offset2341.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2340.i"
  %CastToValueType2342.i = bitcast i8* %"&pSB[currWI].offset2341.i" to double addrspace(3)**
  %loadedValue2343.i = load double addrspace(3)** %CastToValueType2342.i, align 8
  %masked_load723.i = load double addrspace(3)* %loadedValue2343.i, align 8
  br label %postload844.i

postload844.i:                                    ; preds = %postload797.i.postload844.i_crit_edge, %preload843.i
  %phi845.i = phi double [ %masked_load723.i, %preload843.i ], [ undef, %postload797.i.postload844.i_crit_edge ]
  %"&(pSB[currWI].offset)2695.i" = add nuw i64 %CurrSBIndex..6.i, 2013
  %"&pSB[currWI].offset2696.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2695.i"
  %CastToValueType2697.i = bitcast i8* %"&pSB[currWI].offset2696.i" to i1*
  %loadedValue2698.i = load i1* %CastToValueType2697.i, align 1
  br i1 %loadedValue2698.i, label %preload851.i, label %postload844.i.postload852.i_crit_edge

postload844.i.postload852.i_crit_edge:            ; preds = %postload844.i
  br label %postload852.i

preload851.i:                                     ; preds = %postload844.i
  %"&(pSB[currWI].offset)2359.i" = add nuw i64 %CurrSBIndex..6.i, 1896
  %"&pSB[currWI].offset2360.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2359.i"
  %CastToValueType2361.i = bitcast i8* %"&pSB[currWI].offset2360.i" to double addrspace(3)**
  %loadedValue2362.i = load double addrspace(3)** %CastToValueType2361.i, align 8
  %masked_load724.i = load double addrspace(3)* %loadedValue2362.i, align 8
  br label %postload852.i

postload852.i:                                    ; preds = %postload844.i.postload852.i_crit_edge, %preload851.i
  %phi853.i = phi double [ %masked_load724.i, %preload851.i ], [ undef, %postload844.i.postload852.i_crit_edge ]
  %"&(pSB[currWI].offset)2714.i" = add nuw i64 %CurrSBIndex..6.i, 2014
  %"&pSB[currWI].offset2715.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2714.i"
  %CastToValueType2716.i = bitcast i8* %"&pSB[currWI].offset2715.i" to i1*
  %loadedValue2717.i = load i1* %CastToValueType2716.i, align 1
  br i1 %loadedValue2717.i, label %preload859.i, label %postload852.i.postload860.i_crit_edge

postload852.i.postload860.i_crit_edge:            ; preds = %postload852.i
  br label %postload860.i

preload859.i:                                     ; preds = %postload852.i
  %"&(pSB[currWI].offset)2378.i" = add nuw i64 %CurrSBIndex..6.i, 1904
  %"&pSB[currWI].offset2379.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2378.i"
  %CastToValueType2380.i = bitcast i8* %"&pSB[currWI].offset2379.i" to double addrspace(3)**
  %loadedValue2381.i = load double addrspace(3)** %CastToValueType2380.i, align 8
  %masked_load725.i = load double addrspace(3)* %loadedValue2381.i, align 8
  br label %postload860.i

postload860.i:                                    ; preds = %postload852.i.postload860.i_crit_edge, %preload859.i
  %phi861.i = phi double [ %masked_load725.i, %preload859.i ], [ undef, %postload852.i.postload860.i_crit_edge ]
  %"&(pSB[currWI].offset)2733.i" = add nuw i64 %CurrSBIndex..6.i, 2015
  %"&pSB[currWI].offset2734.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2733.i"
  %CastToValueType2735.i = bitcast i8* %"&pSB[currWI].offset2734.i" to i1*
  %loadedValue2736.i = load i1* %CastToValueType2735.i, align 1
  br i1 %loadedValue2736.i, label %preload867.i, label %postload860.i.postload868.i_crit_edge

postload860.i.postload868.i_crit_edge:            ; preds = %postload860.i
  br label %postload868.i

preload867.i:                                     ; preds = %postload860.i
  %"&(pSB[currWI].offset)2397.i" = add nuw i64 %CurrSBIndex..6.i, 1912
  %"&pSB[currWI].offset2398.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2397.i"
  %CastToValueType2399.i = bitcast i8* %"&pSB[currWI].offset2398.i" to double addrspace(3)**
  %loadedValue2400.i = load double addrspace(3)** %CastToValueType2399.i, align 8
  %masked_load726.i = load double addrspace(3)* %loadedValue2400.i, align 8
  br label %postload868.i

postload868.i:                                    ; preds = %postload860.i.postload868.i_crit_edge, %preload867.i
  %phi869.i = phi double [ %masked_load726.i, %preload867.i ], [ undef, %postload860.i.postload868.i_crit_edge ]
  %"&(pSB[currWI].offset)2752.i" = add nuw i64 %CurrSBIndex..6.i, 2016
  %"&pSB[currWI].offset2753.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2752.i"
  %CastToValueType2754.i = bitcast i8* %"&pSB[currWI].offset2753.i" to i1*
  %loadedValue2755.i = load i1* %CastToValueType2754.i, align 1
  br i1 %loadedValue2755.i, label %preload875.i, label %postload868.i.postload876.i_crit_edge

postload868.i.postload876.i_crit_edge:            ; preds = %postload868.i
  br label %postload876.i

preload875.i:                                     ; preds = %postload868.i
  %"&(pSB[currWI].offset)2416.i" = add nuw i64 %CurrSBIndex..6.i, 1920
  %"&pSB[currWI].offset2417.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2416.i"
  %CastToValueType2418.i = bitcast i8* %"&pSB[currWI].offset2417.i" to double addrspace(3)**
  %loadedValue2419.i = load double addrspace(3)** %CastToValueType2418.i, align 8
  %masked_load727.i = load double addrspace(3)* %loadedValue2419.i, align 8
  br label %postload876.i

postload876.i:                                    ; preds = %postload868.i.postload876.i_crit_edge, %preload875.i
  %phi877.i = phi double [ %masked_load727.i, %preload875.i ], [ undef, %postload868.i.postload876.i_crit_edge ]
  %"&(pSB[currWI].offset)2771.i" = add nuw i64 %CurrSBIndex..6.i, 2017
  %"&pSB[currWI].offset2772.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2771.i"
  %CastToValueType2773.i = bitcast i8* %"&pSB[currWI].offset2772.i" to i1*
  %loadedValue2774.i = load i1* %CastToValueType2773.i, align 1
  br i1 %loadedValue2774.i, label %preload883.i, label %postload876.i.postload884.i_crit_edge

postload876.i.postload884.i_crit_edge:            ; preds = %postload876.i
  br label %postload884.i

preload883.i:                                     ; preds = %postload876.i
  %"&(pSB[currWI].offset)2435.i" = add nuw i64 %CurrSBIndex..6.i, 1928
  %"&pSB[currWI].offset2436.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2435.i"
  %CastToValueType2437.i = bitcast i8* %"&pSB[currWI].offset2436.i" to double addrspace(3)**
  %loadedValue2438.i = load double addrspace(3)** %CastToValueType2437.i, align 8
  %masked_load728.i = load double addrspace(3)* %loadedValue2438.i, align 8
  br label %postload884.i

postload884.i:                                    ; preds = %postload876.i.postload884.i_crit_edge, %preload883.i
  %phi885.i = phi double [ %masked_load728.i, %preload883.i ], [ undef, %postload876.i.postload884.i_crit_edge ]
  %"&(pSB[currWI].offset)2790.i" = add nuw i64 %CurrSBIndex..6.i, 2018
  %"&pSB[currWI].offset2791.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2790.i"
  %CastToValueType2792.i = bitcast i8* %"&pSB[currWI].offset2791.i" to i1*
  %loadedValue2793.i = load i1* %CastToValueType2792.i, align 1
  br i1 %loadedValue2793.i, label %preload891.i, label %postload884.i.postload892.i_crit_edge

postload884.i.postload892.i_crit_edge:            ; preds = %postload884.i
  br label %postload892.i

preload891.i:                                     ; preds = %postload884.i
  %"&(pSB[currWI].offset)2454.i" = add nuw i64 %CurrSBIndex..6.i, 1936
  %"&pSB[currWI].offset2455.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2454.i"
  %CastToValueType2456.i = bitcast i8* %"&pSB[currWI].offset2455.i" to double addrspace(3)**
  %loadedValue2457.i = load double addrspace(3)** %CastToValueType2456.i, align 8
  %masked_load729.i = load double addrspace(3)* %loadedValue2457.i, align 8
  br label %postload892.i

postload892.i:                                    ; preds = %postload884.i.postload892.i_crit_edge, %preload891.i
  %phi893.i = phi double [ %masked_load729.i, %preload891.i ], [ undef, %postload884.i.postload892.i_crit_edge ]
  %"&(pSB[currWI].offset)2809.i" = add nuw i64 %CurrSBIndex..6.i, 2019
  %"&pSB[currWI].offset2810.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2809.i"
  %CastToValueType2811.i = bitcast i8* %"&pSB[currWI].offset2810.i" to i1*
  %loadedValue2812.i = load i1* %CastToValueType2811.i, align 1
  br i1 %loadedValue2812.i, label %preload899.i, label %postload892.i.postload900.i_crit_edge

postload892.i.postload900.i_crit_edge:            ; preds = %postload892.i
  br label %postload900.i

preload899.i:                                     ; preds = %postload892.i
  %"&(pSB[currWI].offset)2473.i" = add nuw i64 %CurrSBIndex..6.i, 1944
  %"&pSB[currWI].offset2474.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2473.i"
  %CastToValueType2475.i = bitcast i8* %"&pSB[currWI].offset2474.i" to double addrspace(3)**
  %loadedValue2476.i = load double addrspace(3)** %CastToValueType2475.i, align 8
  %masked_load730.i = load double addrspace(3)* %loadedValue2476.i, align 8
  br label %postload900.i

postload900.i:                                    ; preds = %postload892.i.postload900.i_crit_edge, %preload899.i
  %phi901.i = phi double [ %masked_load730.i, %preload899.i ], [ undef, %postload892.i.postload900.i_crit_edge ]
  %"&(pSB[currWI].offset)2828.i" = add nuw i64 %CurrSBIndex..6.i, 2020
  %"&pSB[currWI].offset2829.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2828.i"
  %CastToValueType2830.i = bitcast i8* %"&pSB[currWI].offset2829.i" to i1*
  %loadedValue2831.i = load i1* %CastToValueType2830.i, align 1
  br i1 %loadedValue2831.i, label %preload907.i, label %postload900.i.postload908.i_crit_edge

postload900.i.postload908.i_crit_edge:            ; preds = %postload900.i
  br label %postload908.i

preload907.i:                                     ; preds = %postload900.i
  %"&(pSB[currWI].offset)2492.i" = add nuw i64 %CurrSBIndex..6.i, 1952
  %"&pSB[currWI].offset2493.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2492.i"
  %CastToValueType2494.i = bitcast i8* %"&pSB[currWI].offset2493.i" to double addrspace(3)**
  %loadedValue2495.i = load double addrspace(3)** %CastToValueType2494.i, align 8
  %masked_load731.i = load double addrspace(3)* %loadedValue2495.i, align 8
  br label %postload908.i

postload908.i:                                    ; preds = %postload900.i.postload908.i_crit_edge, %preload907.i
  %phi909.i = phi double [ %masked_load731.i, %preload907.i ], [ undef, %postload900.i.postload908.i_crit_edge ]
  %"&(pSB[currWI].offset)2847.i" = add nuw i64 %CurrSBIndex..6.i, 2021
  %"&pSB[currWI].offset2848.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2847.i"
  %CastToValueType2849.i = bitcast i8* %"&pSB[currWI].offset2848.i" to i1*
  %loadedValue2850.i = load i1* %CastToValueType2849.i, align 1
  br i1 %loadedValue2850.i, label %preload915.i, label %postload908.i.postload916.i_crit_edge

postload908.i.postload916.i_crit_edge:            ; preds = %postload908.i
  br label %postload916.i

preload915.i:                                     ; preds = %postload908.i
  %"&(pSB[currWI].offset)2511.i" = add nuw i64 %CurrSBIndex..6.i, 1960
  %"&pSB[currWI].offset2512.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2511.i"
  %CastToValueType2513.i = bitcast i8* %"&pSB[currWI].offset2512.i" to double addrspace(3)**
  %loadedValue2514.i = load double addrspace(3)** %CastToValueType2513.i, align 8
  %masked_load732.i = load double addrspace(3)* %loadedValue2514.i, align 8
  br label %postload916.i

postload916.i:                                    ; preds = %postload908.i.postload916.i_crit_edge, %preload915.i
  %phi917.i = phi double [ %masked_load732.i, %preload915.i ], [ undef, %postload908.i.postload916.i_crit_edge ]
  %"&(pSB[currWI].offset)2866.i" = add nuw i64 %CurrSBIndex..6.i, 2022
  %"&pSB[currWI].offset2867.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2866.i"
  %CastToValueType2868.i = bitcast i8* %"&pSB[currWI].offset2867.i" to i1*
  %loadedValue2869.i = load i1* %CastToValueType2868.i, align 1
  br i1 %loadedValue2869.i, label %preload923.i, label %postload916.i.postload924.i_crit_edge

postload916.i.postload924.i_crit_edge:            ; preds = %postload916.i
  br label %postload924.i

preload923.i:                                     ; preds = %postload916.i
  %"&(pSB[currWI].offset)2530.i" = add nuw i64 %CurrSBIndex..6.i, 1968
  %"&pSB[currWI].offset2531.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2530.i"
  %CastToValueType2532.i = bitcast i8* %"&pSB[currWI].offset2531.i" to double addrspace(3)**
  %loadedValue2533.i = load double addrspace(3)** %CastToValueType2532.i, align 8
  %masked_load733.i = load double addrspace(3)* %loadedValue2533.i, align 8
  br label %postload924.i

postload924.i:                                    ; preds = %postload916.i.postload924.i_crit_edge, %preload923.i
  %phi925.i = phi double [ %masked_load733.i, %preload923.i ], [ undef, %postload916.i.postload924.i_crit_edge ]
  %"&(pSB[currWI].offset)2885.i" = add nuw i64 %CurrSBIndex..6.i, 2023
  %"&pSB[currWI].offset2886.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2885.i"
  %CastToValueType2887.i = bitcast i8* %"&pSB[currWI].offset2886.i" to i1*
  %loadedValue2888.i = load i1* %CastToValueType2887.i, align 1
  br i1 %loadedValue2888.i, label %preload931.i, label %postload924.i.postload932.i_crit_edge

postload924.i.postload932.i_crit_edge:            ; preds = %postload924.i
  br label %postload932.i

preload931.i:                                     ; preds = %postload924.i
  %"&(pSB[currWI].offset)2549.i" = add nuw i64 %CurrSBIndex..6.i, 1976
  %"&pSB[currWI].offset2550.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2549.i"
  %CastToValueType2551.i = bitcast i8* %"&pSB[currWI].offset2550.i" to double addrspace(3)**
  %loadedValue2552.i = load double addrspace(3)** %CastToValueType2551.i, align 8
  %masked_load734.i = load double addrspace(3)* %loadedValue2552.i, align 8
  br label %postload932.i

postload932.i:                                    ; preds = %postload924.i.postload932.i_crit_edge, %preload931.i
  %phi933.i = phi double [ %masked_load734.i, %preload931.i ], [ undef, %postload924.i.postload932.i_crit_edge ]
  %temp.vect335.i = insertelement <16 x double> undef, double %phi772.i, i32 0
  %temp.vect336.i = insertelement <16 x double> %temp.vect335.i, double %phi782.i, i32 1
  %temp.vect337.i = insertelement <16 x double> %temp.vect336.i, double %phi790.i, i32 2
  %temp.vect338.i = insertelement <16 x double> %temp.vect337.i, double %phi798.i, i32 3
  %temp.vect339.i = insertelement <16 x double> %temp.vect338.i, double %phi845.i, i32 4
  %temp.vect340.i = insertelement <16 x double> %temp.vect339.i, double %phi853.i, i32 5
  %temp.vect341.i = insertelement <16 x double> %temp.vect340.i, double %phi861.i, i32 6
  %temp.vect342.i = insertelement <16 x double> %temp.vect341.i, double %phi869.i, i32 7
  %temp.vect343.i = insertelement <16 x double> %temp.vect342.i, double %phi877.i, i32 8
  %temp.vect344.i = insertelement <16 x double> %temp.vect343.i, double %phi885.i, i32 9
  %temp.vect345.i = insertelement <16 x double> %temp.vect344.i, double %phi893.i, i32 10
  %temp.vect346.i = insertelement <16 x double> %temp.vect345.i, double %phi901.i, i32 11
  %temp.vect347.i = insertelement <16 x double> %temp.vect346.i, double %phi909.i, i32 12
  %temp.vect348.i = insertelement <16 x double> %temp.vect347.i, double %phi917.i, i32 13
  %temp.vect349.i = insertelement <16 x double> %temp.vect348.i, double %phi925.i, i32 14
  %temp.vect350.i = insertelement <16 x double> %temp.vect349.i, double %phi933.i, i32 15
  %"&(pSB[currWI].offset)2899.i" = add nuw i64 %CurrSBIndex..6.i, 2048
  %"&pSB[currWI].offset2900.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2899.i"
  %CastToValueType2901.i = bitcast i8* %"&pSB[currWI].offset2900.i" to <16 x double>*
  %loadedValue2902.i = load <16 x double>* %CastToValueType2901.i, align 128
  %add13.i367.i = fadd <16 x double> %temp.vect350.i, %loadedValue2902.i
  %extract369.i = extractelement <16 x double> %add13.i367.i, i32 1
  %extract370.i = extractelement <16 x double> %add13.i367.i, i32 2
  %extract371.i = extractelement <16 x double> %add13.i367.i, i32 3
  %extract372.i = extractelement <16 x double> %add13.i367.i, i32 4
  %extract373.i = extractelement <16 x double> %add13.i367.i, i32 5
  %extract374.i = extractelement <16 x double> %add13.i367.i, i32 6
  %extract375.i = extractelement <16 x double> %add13.i367.i, i32 7
  %extract376.i = extractelement <16 x double> %add13.i367.i, i32 8
  %extract377.i = extractelement <16 x double> %add13.i367.i, i32 9
  %extract378.i = extractelement <16 x double> %add13.i367.i, i32 10
  %extract379.i = extractelement <16 x double> %add13.i367.i, i32 11
  %extract380.i = extractelement <16 x double> %add13.i367.i, i32 12
  %extract381.i = extractelement <16 x double> %add13.i367.i, i32 13
  %extract382.i = extractelement <16 x double> %add13.i367.i, i32 14
  %extract383.i = extractelement <16 x double> %add13.i367.i, i32 15
  %"&(pSB[currWI].offset)2600.i" = add nuw i64 %CurrSBIndex..6.i, 2008
  %"&pSB[currWI].offset2601.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2600.i"
  %CastToValueType2602.i = bitcast i8* %"&pSB[currWI].offset2601.i" to i1*
  %loadedValue2603.i = load i1* %CastToValueType2602.i, align 1
  br i1 %loadedValue2603.i, label %preload773.i, label %postload774.i

preload773.i:                                     ; preds = %postload932.i
  %extract368.i = extractelement <16 x double> %add13.i367.i, i32 0
  %"&(pSB[currWI].offset)2259.i" = add nuw i64 %CurrSBIndex..6.i, 1856
  %"&pSB[currWI].offset2260.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2259.i"
  %CastToValueType2261.i = bitcast i8* %"&pSB[currWI].offset2260.i" to double addrspace(3)**
  %loadedValue2262.i = load double addrspace(3)** %CastToValueType2261.i, align 8
  store double %extract368.i, double addrspace(3)* %loadedValue2262.i, align 8
  %loadedValue2617.pre.i = load i1* %CastToValueType2621.i, align 1
  br label %postload774.i

postload774.i:                                    ; preds = %preload773.i, %postload932.i
  %loadedValue2617.i = phi i1 [ %loadedValue2617.pre.i, %preload773.i ], [ %loadedValue2622.i, %postload932.i ]
  br i1 %loadedValue2617.i, label %preload783.i, label %postload774.i.postload784.i_crit_edge

postload774.i.postload784.i_crit_edge:            ; preds = %postload774.i
  br label %postload784.i

preload783.i:                                     ; preds = %postload774.i
  %"&(pSB[currWI].offset)2278.i" = add nuw i64 %CurrSBIndex..6.i, 1864
  %"&pSB[currWI].offset2279.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2278.i"
  %CastToValueType2280.i = bitcast i8* %"&pSB[currWI].offset2279.i" to double addrspace(3)**
  %loadedValue2281.i = load double addrspace(3)** %CastToValueType2280.i, align 8
  store double %extract369.i, double addrspace(3)* %loadedValue2281.i, align 8
  br label %postload784.i

postload784.i:                                    ; preds = %postload774.i.postload784.i_crit_edge, %preload783.i
  %loadedValue2636.i = load i1* %CastToValueType2640.i, align 1
  br i1 %loadedValue2636.i, label %preload791.i, label %postload784.i.postload792.i_crit_edge

postload784.i.postload792.i_crit_edge:            ; preds = %postload784.i
  br label %postload792.i

preload791.i:                                     ; preds = %postload784.i
  %"&(pSB[currWI].offset)2297.i" = add nuw i64 %CurrSBIndex..6.i, 1872
  %"&pSB[currWI].offset2298.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2297.i"
  %CastToValueType2299.i = bitcast i8* %"&pSB[currWI].offset2298.i" to double addrspace(3)**
  %loadedValue2300.i = load double addrspace(3)** %CastToValueType2299.i, align 8
  store double %extract370.i, double addrspace(3)* %loadedValue2300.i, align 8
  br label %postload792.i

postload792.i:                                    ; preds = %postload784.i.postload792.i_crit_edge, %preload791.i
  %loadedValue2655.i = load i1* %CastToValueType2659.i, align 1
  br i1 %loadedValue2655.i, label %preload799.i, label %postload792.i.postload800.i_crit_edge

postload792.i.postload800.i_crit_edge:            ; preds = %postload792.i
  br label %postload800.i

preload799.i:                                     ; preds = %postload792.i
  %"&(pSB[currWI].offset)2316.i" = add nuw i64 %CurrSBIndex..6.i, 1880
  %"&pSB[currWI].offset2317.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2316.i"
  %CastToValueType2318.i = bitcast i8* %"&pSB[currWI].offset2317.i" to double addrspace(3)**
  %loadedValue2319.i = load double addrspace(3)** %CastToValueType2318.i, align 8
  store double %extract371.i, double addrspace(3)* %loadedValue2319.i, align 8
  br label %postload800.i

postload800.i:                                    ; preds = %postload792.i.postload800.i_crit_edge, %preload799.i
  %loadedValue2674.i = load i1* %CastToValueType2678.i, align 1
  br i1 %loadedValue2674.i, label %preload846.i, label %postload800.i.postload847.i_crit_edge

postload800.i.postload847.i_crit_edge:            ; preds = %postload800.i
  br label %postload847.i

preload846.i:                                     ; preds = %postload800.i
  %"&(pSB[currWI].offset)2335.i" = add nuw i64 %CurrSBIndex..6.i, 1888
  %"&pSB[currWI].offset2336.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2335.i"
  %CastToValueType2337.i = bitcast i8* %"&pSB[currWI].offset2336.i" to double addrspace(3)**
  %loadedValue2338.i = load double addrspace(3)** %CastToValueType2337.i, align 8
  store double %extract372.i, double addrspace(3)* %loadedValue2338.i, align 8
  br label %postload847.i

postload847.i:                                    ; preds = %postload800.i.postload847.i_crit_edge, %preload846.i
  %loadedValue2693.i = load i1* %CastToValueType2697.i, align 1
  br i1 %loadedValue2693.i, label %preload854.i, label %postload847.i.postload855.i_crit_edge

postload847.i.postload855.i_crit_edge:            ; preds = %postload847.i
  br label %postload855.i

preload854.i:                                     ; preds = %postload847.i
  %"&(pSB[currWI].offset)2354.i" = add nuw i64 %CurrSBIndex..6.i, 1896
  %"&pSB[currWI].offset2355.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2354.i"
  %CastToValueType2356.i = bitcast i8* %"&pSB[currWI].offset2355.i" to double addrspace(3)**
  %loadedValue2357.i = load double addrspace(3)** %CastToValueType2356.i, align 8
  store double %extract373.i, double addrspace(3)* %loadedValue2357.i, align 8
  br label %postload855.i

postload855.i:                                    ; preds = %postload847.i.postload855.i_crit_edge, %preload854.i
  %loadedValue2712.i = load i1* %CastToValueType2716.i, align 1
  br i1 %loadedValue2712.i, label %preload862.i, label %postload855.i.postload863.i_crit_edge

postload855.i.postload863.i_crit_edge:            ; preds = %postload855.i
  br label %postload863.i

preload862.i:                                     ; preds = %postload855.i
  %"&(pSB[currWI].offset)2373.i" = add nuw i64 %CurrSBIndex..6.i, 1904
  %"&pSB[currWI].offset2374.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2373.i"
  %CastToValueType2375.i = bitcast i8* %"&pSB[currWI].offset2374.i" to double addrspace(3)**
  %loadedValue2376.i = load double addrspace(3)** %CastToValueType2375.i, align 8
  store double %extract374.i, double addrspace(3)* %loadedValue2376.i, align 8
  br label %postload863.i

postload863.i:                                    ; preds = %postload855.i.postload863.i_crit_edge, %preload862.i
  %loadedValue2731.i = load i1* %CastToValueType2735.i, align 1
  br i1 %loadedValue2731.i, label %preload870.i, label %postload863.i.postload871.i_crit_edge

postload863.i.postload871.i_crit_edge:            ; preds = %postload863.i
  br label %postload871.i

preload870.i:                                     ; preds = %postload863.i
  %"&(pSB[currWI].offset)2392.i" = add nuw i64 %CurrSBIndex..6.i, 1912
  %"&pSB[currWI].offset2393.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2392.i"
  %CastToValueType2394.i = bitcast i8* %"&pSB[currWI].offset2393.i" to double addrspace(3)**
  %loadedValue2395.i = load double addrspace(3)** %CastToValueType2394.i, align 8
  store double %extract375.i, double addrspace(3)* %loadedValue2395.i, align 8
  br label %postload871.i

postload871.i:                                    ; preds = %postload863.i.postload871.i_crit_edge, %preload870.i
  %loadedValue2750.i = load i1* %CastToValueType2754.i, align 1
  br i1 %loadedValue2750.i, label %preload878.i, label %postload871.i.postload879.i_crit_edge

postload871.i.postload879.i_crit_edge:            ; preds = %postload871.i
  br label %postload879.i

preload878.i:                                     ; preds = %postload871.i
  %"&(pSB[currWI].offset)2411.i" = add nuw i64 %CurrSBIndex..6.i, 1920
  %"&pSB[currWI].offset2412.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2411.i"
  %CastToValueType2413.i = bitcast i8* %"&pSB[currWI].offset2412.i" to double addrspace(3)**
  %loadedValue2414.i = load double addrspace(3)** %CastToValueType2413.i, align 8
  store double %extract376.i, double addrspace(3)* %loadedValue2414.i, align 8
  br label %postload879.i

postload879.i:                                    ; preds = %postload871.i.postload879.i_crit_edge, %preload878.i
  %loadedValue2769.i = load i1* %CastToValueType2773.i, align 1
  br i1 %loadedValue2769.i, label %preload886.i, label %postload879.i.postload887.i_crit_edge

postload879.i.postload887.i_crit_edge:            ; preds = %postload879.i
  br label %postload887.i

preload886.i:                                     ; preds = %postload879.i
  %"&(pSB[currWI].offset)2430.i" = add nuw i64 %CurrSBIndex..6.i, 1928
  %"&pSB[currWI].offset2431.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2430.i"
  %CastToValueType2432.i = bitcast i8* %"&pSB[currWI].offset2431.i" to double addrspace(3)**
  %loadedValue2433.i = load double addrspace(3)** %CastToValueType2432.i, align 8
  store double %extract377.i, double addrspace(3)* %loadedValue2433.i, align 8
  br label %postload887.i

postload887.i:                                    ; preds = %postload879.i.postload887.i_crit_edge, %preload886.i
  %loadedValue2788.i = load i1* %CastToValueType2792.i, align 1
  br i1 %loadedValue2788.i, label %preload894.i, label %postload887.i.postload895.i_crit_edge

postload887.i.postload895.i_crit_edge:            ; preds = %postload887.i
  br label %postload895.i

preload894.i:                                     ; preds = %postload887.i
  %"&(pSB[currWI].offset)2449.i" = add nuw i64 %CurrSBIndex..6.i, 1936
  %"&pSB[currWI].offset2450.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2449.i"
  %CastToValueType2451.i = bitcast i8* %"&pSB[currWI].offset2450.i" to double addrspace(3)**
  %loadedValue2452.i = load double addrspace(3)** %CastToValueType2451.i, align 8
  store double %extract378.i, double addrspace(3)* %loadedValue2452.i, align 8
  br label %postload895.i

postload895.i:                                    ; preds = %postload887.i.postload895.i_crit_edge, %preload894.i
  %loadedValue2807.i = load i1* %CastToValueType2811.i, align 1
  br i1 %loadedValue2807.i, label %preload902.i, label %postload895.i.postload903.i_crit_edge

postload895.i.postload903.i_crit_edge:            ; preds = %postload895.i
  br label %postload903.i

preload902.i:                                     ; preds = %postload895.i
  %"&(pSB[currWI].offset)2468.i" = add nuw i64 %CurrSBIndex..6.i, 1944
  %"&pSB[currWI].offset2469.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2468.i"
  %CastToValueType2470.i = bitcast i8* %"&pSB[currWI].offset2469.i" to double addrspace(3)**
  %loadedValue2471.i = load double addrspace(3)** %CastToValueType2470.i, align 8
  store double %extract379.i, double addrspace(3)* %loadedValue2471.i, align 8
  br label %postload903.i

postload903.i:                                    ; preds = %postload895.i.postload903.i_crit_edge, %preload902.i
  %loadedValue2826.i = load i1* %CastToValueType2830.i, align 1
  br i1 %loadedValue2826.i, label %preload910.i, label %postload903.i.postload911.i_crit_edge

postload903.i.postload911.i_crit_edge:            ; preds = %postload903.i
  br label %postload911.i

preload910.i:                                     ; preds = %postload903.i
  %"&(pSB[currWI].offset)2487.i" = add nuw i64 %CurrSBIndex..6.i, 1952
  %"&pSB[currWI].offset2488.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2487.i"
  %CastToValueType2489.i = bitcast i8* %"&pSB[currWI].offset2488.i" to double addrspace(3)**
  %loadedValue2490.i = load double addrspace(3)** %CastToValueType2489.i, align 8
  store double %extract380.i, double addrspace(3)* %loadedValue2490.i, align 8
  br label %postload911.i

postload911.i:                                    ; preds = %postload903.i.postload911.i_crit_edge, %preload910.i
  %loadedValue2845.i = load i1* %CastToValueType2849.i, align 1
  br i1 %loadedValue2845.i, label %preload918.i, label %postload911.i.postload919.i_crit_edge

postload911.i.postload919.i_crit_edge:            ; preds = %postload911.i
  br label %postload919.i

preload918.i:                                     ; preds = %postload911.i
  %"&(pSB[currWI].offset)2506.i" = add nuw i64 %CurrSBIndex..6.i, 1960
  %"&pSB[currWI].offset2507.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2506.i"
  %CastToValueType2508.i = bitcast i8* %"&pSB[currWI].offset2507.i" to double addrspace(3)**
  %loadedValue2509.i = load double addrspace(3)** %CastToValueType2508.i, align 8
  store double %extract381.i, double addrspace(3)* %loadedValue2509.i, align 8
  br label %postload919.i

postload919.i:                                    ; preds = %postload911.i.postload919.i_crit_edge, %preload918.i
  %loadedValue2864.i = load i1* %CastToValueType2868.i, align 1
  br i1 %loadedValue2864.i, label %preload926.i, label %postload919.i.postload927.i_crit_edge

postload919.i.postload927.i_crit_edge:            ; preds = %postload919.i
  br label %postload927.i

preload926.i:                                     ; preds = %postload919.i
  %"&(pSB[currWI].offset)2525.i" = add nuw i64 %CurrSBIndex..6.i, 1968
  %"&pSB[currWI].offset2526.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2525.i"
  %CastToValueType2527.i = bitcast i8* %"&pSB[currWI].offset2526.i" to double addrspace(3)**
  %loadedValue2528.i = load double addrspace(3)** %CastToValueType2527.i, align 8
  store double %extract382.i, double addrspace(3)* %loadedValue2528.i, align 8
  br label %postload927.i

postload927.i:                                    ; preds = %postload919.i.postload927.i_crit_edge, %preload926.i
  %loadedValue2883.i = load i1* %CastToValueType2887.i, align 1
  br i1 %loadedValue2883.i, label %preload934.i, label %postload927.i.postload935.i_crit_edge

postload927.i.postload935.i_crit_edge:            ; preds = %postload927.i
  br label %postload935.i

preload934.i:                                     ; preds = %postload927.i
  %"&(pSB[currWI].offset)2544.i" = add nuw i64 %CurrSBIndex..6.i, 1976
  %"&pSB[currWI].offset2545.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2544.i"
  %CastToValueType2546.i = bitcast i8* %"&pSB[currWI].offset2545.i" to double addrspace(3)**
  %loadedValue2547.i = load double addrspace(3)** %CastToValueType2546.i, align 8
  store double %extract383.i, double addrspace(3)* %loadedValue2547.i, align 8
  br label %postload935.i

postload935.i:                                    ; preds = %postload927.i.postload935.i_crit_edge, %preload934.i
  %loadedValue2598.i = load i1* %CastToValueType2602.i, align 1
  br i1 %loadedValue2598.i, label %preload775.i, label %postload776.i

preload775.i:                                     ; preds = %postload935.i
  %check.WI.iter2956.i = icmp ult i64 %CurrWI..6.i, %28
  br i1 %check.WI.iter2956.i, label %thenBB2953.i, label %postload776.i

thenBB2953.i:                                     ; preds = %preload775.i
  %"CurrWI++2957.i" = add nuw i64 %CurrWI..6.i, 1
  %"loadedCurrSB+Stride2959.i" = add nuw i64 %CurrSBIndex..6.i, 2176
  switch i32 %currBarrier.6.i, label %SyncBB2939.i [
    i32 5, label %thenBB2953.i.postload1025.i_crit_edge
    i32 29, label %SyncBB2941.i
    i32 20, label %thenBB2953.i.SyncBB2943.i_crit_edge
    i32 7, label %postload776.i
  ]

thenBB2953.i.SyncBB2943.i_crit_edge:              ; preds = %thenBB2953.i
  br label %SyncBB2943.i

thenBB2953.i.postload1025.i_crit_edge:            ; preds = %thenBB2953.i
  br label %postload1025.i

postload776.i:                                    ; preds = %thenBB2961.i, %thenBB2969.i, %thenBB2953.i, %preload775.i, %postload935.i, %thenBB2945.i, %thenBB.i
  %currBarrier.8.i = phi i32 [ %currBarrier.6.i, %postload935.i ], [ %currBarrier.9.i, %thenBB2969.i ], [ %currBarrier.6.i, %thenBB2953.i ], [ %currBarrier.4.i, %thenBB2945.i ], [ %currBarrier.1.i, %thenBB.i ], [ %currBarrier.12.i, %thenBB2961.i ], [ 7, %preload775.i ]
  %CurrSBIndex..8.i = phi i64 [ %CurrSBIndex..6.i, %postload935.i ], [ %"loadedCurrSB+Stride2975.i", %thenBB2969.i ], [ %"loadedCurrSB+Stride2959.i", %thenBB2953.i ], [ %"loadedCurrSB+Stride2951.i", %thenBB2945.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ %"loadedCurrSB+Stride2967.i", %thenBB2961.i ], [ 0, %preload775.i ]
  %CurrWI..8.i = phi i64 [ %CurrWI..6.i, %postload935.i ], [ %"CurrWI++2973.i", %thenBB2969.i ], [ %"CurrWI++2957.i", %thenBB2953.i ], [ %"CurrWI++2949.i", %thenBB2945.i ], [ %"CurrWI++.i", %thenBB.i ], [ %"CurrWI++2965.i", %thenBB2961.i ], [ 0, %preload775.i ]
  %"&(pSB[currWI].offset)2586.i" = add nuw i64 %CurrSBIndex..8.i, 2004
  %"&pSB[currWI].offset2587.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2586.i"
  %CastToValueType2588.i = bitcast i8* %"&pSB[currWI].offset2587.i" to i32*
  %loadedValue2589.i = load i32* %CastToValueType2588.i, align 4
  %mul.i.i = shl nsw i32 %loadedValue2589.i, 1
  %conv6.i.i = sext i32 %mul.i.i to i64
  %temp384.i = insertelement <16 x i64> undef, i64 %conv6.i.i, i32 0
  %vector385.i = shufflevector <16 x i64> %temp384.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)2231.i" = add nuw i64 %CurrSBIndex..8.i, 1664
  %"&pSB[currWI].offset2232.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2231.i"
  %CastToValueType2233.i = bitcast i8* %"&pSB[currWI].offset2232.i" to <16 x i64>*
  %loadedValue2234.i = load <16 x i64>* %CastToValueType2233.i, align 128
  %cmp.i.i = icmp ult <16 x i64> %vector385.i, %loadedValue2234.i
  %notCond386.i = xor <16 x i1> %cmp.i.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %"&(pSB[currWI].offset)2577.i" = add nuw i64 %CurrSBIndex..8.i, 2000
  %"&pSB[currWI].offset2578.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2577.i"
  %CastToValueType2579.i = bitcast i8* %"&pSB[currWI].offset2578.i" to <16 x i1>*
  %loadedValue2580.i = load <16 x i1>* %CastToValueType2579.i, align 16
  %who_left_tr387.i = and <16 x i1> %loadedValue2580.i, %notCond386.i
  %"&(pSB[currWI].offset)2563.i" = add nuw i64 %CurrSBIndex..8.i, 1984
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

scanLocalMem.exit.i:                              ; preds = %postload776.i, %postload1025.i
  %currBarrier.9.i = phi i32 [ %currBarrier.3.i, %postload1025.i ], [ %currBarrier.8.i, %postload776.i ]
  %CurrSBIndex..9.i = phi i64 [ %CurrSBIndex..3.i, %postload1025.i ], [ %CurrSBIndex..8.i, %postload776.i ]
  %CurrWI..9.i = phi i64 [ %CurrWI..3.i, %postload1025.i ], [ %CurrWI..8.i, %postload776.i ]
  %"&(pSB[currWI].offset)2245.i" = add nuw i64 %CurrSBIndex..9.i, 1792
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
  %205 = getelementptr inbounds double addrspace(3)* %13, i64 %extract428.i
  %206 = getelementptr inbounds double addrspace(3)* %13, i64 %extract429.i
  %207 = getelementptr inbounds double addrspace(3)* %13, i64 %extract430.i
  %208 = getelementptr inbounds double addrspace(3)* %13, i64 %extract431.i
  %209 = getelementptr inbounds double addrspace(3)* %13, i64 %extract432.i
  %210 = getelementptr inbounds double addrspace(3)* %13, i64 %extract433.i
  %211 = getelementptr inbounds double addrspace(3)* %13, i64 %extract434.i
  %212 = getelementptr inbounds double addrspace(3)* %13, i64 %extract435.i
  %213 = getelementptr inbounds double addrspace(3)* %13, i64 %extract436.i
  %214 = getelementptr inbounds double addrspace(3)* %13, i64 %extract437.i
  %215 = getelementptr inbounds double addrspace(3)* %13, i64 %extract438.i
  %216 = getelementptr inbounds double addrspace(3)* %13, i64 %extract439.i
  %217 = getelementptr inbounds double addrspace(3)* %13, i64 %extract440.i
  %218 = getelementptr inbounds double addrspace(3)* %13, i64 %extract441.i
  %219 = getelementptr inbounds double addrspace(3)* %13, i64 %extract442.i
  %"&(pSB[currWI].offset)1811.i" = add nuw i64 %CurrSBIndex..9.i, 1016
  %"&pSB[currWI].offset1812.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1811.i"
  %CastToValueType1813.i = bitcast i8* %"&pSB[currWI].offset1812.i" to i1*
  %loadedValue1814.i = load i1* %CastToValueType1813.i, align 1
  br i1 %loadedValue1814.i, label %preload1026.i, label %postload1027.i

preload1026.i:                                    ; preds = %scanLocalMem.exit.i
  %extract427.i = extractelement <16 x i64> %idxprom15.i426.i, i32 0
  %220 = getelementptr inbounds double addrspace(3)* %13, i64 %extract427.i
  %masked_load735.i = load double addrspace(3)* %220, align 8
  br label %postload1027.i

postload1027.i:                                   ; preds = %preload1026.i, %scanLocalMem.exit.i
  %phi1028.i = phi double [ undef, %scanLocalMem.exit.i ], [ %masked_load735.i, %preload1026.i ]
  %"&(pSB[currWI].offset)1840.i" = add nuw i64 %CurrSBIndex..9.i, 1017
  %"&pSB[currWI].offset1841.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1840.i"
  %CastToValueType1842.i = bitcast i8* %"&pSB[currWI].offset1841.i" to i1*
  %loadedValue1843.i = load i1* %CastToValueType1842.i, align 1
  br i1 %loadedValue1843.i, label %preload1037.i, label %postload1038.i

preload1037.i:                                    ; preds = %postload1027.i
  %masked_load736.i = load double addrspace(3)* %205, align 8
  br label %postload1038.i

postload1038.i:                                   ; preds = %preload1037.i, %postload1027.i
  %phi1039.i = phi double [ undef, %postload1027.i ], [ %masked_load736.i, %preload1037.i ]
  %"&(pSB[currWI].offset)1864.i" = add nuw i64 %CurrSBIndex..9.i, 1018
  %"&pSB[currWI].offset1865.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1864.i"
  %CastToValueType1866.i = bitcast i8* %"&pSB[currWI].offset1865.i" to i1*
  %loadedValue1867.i = load i1* %CastToValueType1866.i, align 1
  br i1 %loadedValue1867.i, label %preload1048.i, label %postload1049.i

preload1048.i:                                    ; preds = %postload1038.i
  %masked_load737.i = load double addrspace(3)* %206, align 8
  br label %postload1049.i

postload1049.i:                                   ; preds = %preload1048.i, %postload1038.i
  %phi1050.i = phi double [ undef, %postload1038.i ], [ %masked_load737.i, %preload1048.i ]
  %"&(pSB[currWI].offset)1888.i" = add nuw i64 %CurrSBIndex..9.i, 1019
  %"&pSB[currWI].offset1889.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1888.i"
  %CastToValueType1890.i = bitcast i8* %"&pSB[currWI].offset1889.i" to i1*
  %loadedValue1891.i = load i1* %CastToValueType1890.i, align 1
  br i1 %loadedValue1891.i, label %preload1059.i, label %postload1060.i

preload1059.i:                                    ; preds = %postload1049.i
  %masked_load738.i = load double addrspace(3)* %207, align 8
  br label %postload1060.i

postload1060.i:                                   ; preds = %preload1059.i, %postload1049.i
  %phi1061.i = phi double [ undef, %postload1049.i ], [ %masked_load738.i, %preload1059.i ]
  %"&(pSB[currWI].offset)1912.i" = add nuw i64 %CurrSBIndex..9.i, 1020
  %"&pSB[currWI].offset1913.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1912.i"
  %CastToValueType1914.i = bitcast i8* %"&pSB[currWI].offset1913.i" to i1*
  %loadedValue1915.i = load i1* %CastToValueType1914.i, align 1
  br i1 %loadedValue1915.i, label %preload1070.i, label %postload1071.i

preload1070.i:                                    ; preds = %postload1060.i
  %masked_load739.i = load double addrspace(3)* %208, align 8
  br label %postload1071.i

postload1071.i:                                   ; preds = %preload1070.i, %postload1060.i
  %phi1072.i = phi double [ undef, %postload1060.i ], [ %masked_load739.i, %preload1070.i ]
  %"&(pSB[currWI].offset)1936.i" = add nuw i64 %CurrSBIndex..9.i, 1021
  %"&pSB[currWI].offset1937.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1936.i"
  %CastToValueType1938.i = bitcast i8* %"&pSB[currWI].offset1937.i" to i1*
  %loadedValue1939.i = load i1* %CastToValueType1938.i, align 1
  br i1 %loadedValue1939.i, label %preload1081.i, label %postload1082.i

preload1081.i:                                    ; preds = %postload1071.i
  %masked_load740.i = load double addrspace(3)* %209, align 8
  br label %postload1082.i

postload1082.i:                                   ; preds = %preload1081.i, %postload1071.i
  %phi1083.i = phi double [ undef, %postload1071.i ], [ %masked_load740.i, %preload1081.i ]
  %"&(pSB[currWI].offset)1960.i" = add nuw i64 %CurrSBIndex..9.i, 1022
  %"&pSB[currWI].offset1961.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1960.i"
  %CastToValueType1962.i = bitcast i8* %"&pSB[currWI].offset1961.i" to i1*
  %loadedValue1963.i = load i1* %CastToValueType1962.i, align 1
  br i1 %loadedValue1963.i, label %preload1092.i, label %postload1093.i

preload1092.i:                                    ; preds = %postload1082.i
  %masked_load741.i = load double addrspace(3)* %210, align 8
  br label %postload1093.i

postload1093.i:                                   ; preds = %preload1092.i, %postload1082.i
  %phi1094.i = phi double [ undef, %postload1082.i ], [ %masked_load741.i, %preload1092.i ]
  %"&(pSB[currWI].offset)1984.i" = add nuw i64 %CurrSBIndex..9.i, 1023
  %"&pSB[currWI].offset1985.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1984.i"
  %CastToValueType1986.i = bitcast i8* %"&pSB[currWI].offset1985.i" to i1*
  %loadedValue1987.i = load i1* %CastToValueType1986.i, align 1
  br i1 %loadedValue1987.i, label %preload1103.i, label %postload1104.i

preload1103.i:                                    ; preds = %postload1093.i
  %masked_load742.i = load double addrspace(3)* %211, align 8
  br label %postload1104.i

postload1104.i:                                   ; preds = %preload1103.i, %postload1093.i
  %phi1105.i = phi double [ undef, %postload1093.i ], [ %masked_load742.i, %preload1103.i ]
  %"&(pSB[currWI].offset)2008.i" = add nuw i64 %CurrSBIndex..9.i, 1024
  %"&pSB[currWI].offset2009.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2008.i"
  %CastToValueType2010.i = bitcast i8* %"&pSB[currWI].offset2009.i" to i1*
  %loadedValue2011.i = load i1* %CastToValueType2010.i, align 1
  br i1 %loadedValue2011.i, label %preload1114.i, label %postload1115.i

preload1114.i:                                    ; preds = %postload1104.i
  %masked_load743.i = load double addrspace(3)* %212, align 8
  br label %postload1115.i

postload1115.i:                                   ; preds = %preload1114.i, %postload1104.i
  %phi1116.i = phi double [ undef, %postload1104.i ], [ %masked_load743.i, %preload1114.i ]
  %"&(pSB[currWI].offset)2032.i" = add nuw i64 %CurrSBIndex..9.i, 1025
  %"&pSB[currWI].offset2033.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2032.i"
  %CastToValueType2034.i = bitcast i8* %"&pSB[currWI].offset2033.i" to i1*
  %loadedValue2035.i = load i1* %CastToValueType2034.i, align 1
  br i1 %loadedValue2035.i, label %preload1125.i, label %postload1126.i

preload1125.i:                                    ; preds = %postload1115.i
  %masked_load744.i = load double addrspace(3)* %213, align 8
  br label %postload1126.i

postload1126.i:                                   ; preds = %preload1125.i, %postload1115.i
  %phi1127.i = phi double [ undef, %postload1115.i ], [ %masked_load744.i, %preload1125.i ]
  %"&(pSB[currWI].offset)2056.i" = add nuw i64 %CurrSBIndex..9.i, 1026
  %"&pSB[currWI].offset2057.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2056.i"
  %CastToValueType2058.i = bitcast i8* %"&pSB[currWI].offset2057.i" to i1*
  %loadedValue2059.i = load i1* %CastToValueType2058.i, align 1
  br i1 %loadedValue2059.i, label %preload1136.i, label %postload1137.i

preload1136.i:                                    ; preds = %postload1126.i
  %masked_load745.i = load double addrspace(3)* %214, align 8
  br label %postload1137.i

postload1137.i:                                   ; preds = %preload1136.i, %postload1126.i
  %phi1138.i = phi double [ undef, %postload1126.i ], [ %masked_load745.i, %preload1136.i ]
  %"&(pSB[currWI].offset)2080.i" = add nuw i64 %CurrSBIndex..9.i, 1027
  %"&pSB[currWI].offset2081.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2080.i"
  %CastToValueType2082.i = bitcast i8* %"&pSB[currWI].offset2081.i" to i1*
  %loadedValue2083.i = load i1* %CastToValueType2082.i, align 1
  br i1 %loadedValue2083.i, label %preload1147.i, label %postload1148.i

preload1147.i:                                    ; preds = %postload1137.i
  %masked_load746.i = load double addrspace(3)* %215, align 8
  br label %postload1148.i

postload1148.i:                                   ; preds = %preload1147.i, %postload1137.i
  %phi1149.i = phi double [ undef, %postload1137.i ], [ %masked_load746.i, %preload1147.i ]
  %"&(pSB[currWI].offset)2104.i" = add nuw i64 %CurrSBIndex..9.i, 1028
  %"&pSB[currWI].offset2105.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2104.i"
  %CastToValueType2106.i = bitcast i8* %"&pSB[currWI].offset2105.i" to i1*
  %loadedValue2107.i = load i1* %CastToValueType2106.i, align 1
  br i1 %loadedValue2107.i, label %preload1158.i, label %postload1159.i

preload1158.i:                                    ; preds = %postload1148.i
  %masked_load747.i = load double addrspace(3)* %216, align 8
  br label %postload1159.i

postload1159.i:                                   ; preds = %preload1158.i, %postload1148.i
  %phi1160.i = phi double [ undef, %postload1148.i ], [ %masked_load747.i, %preload1158.i ]
  %"&(pSB[currWI].offset)2128.i" = add nuw i64 %CurrSBIndex..9.i, 1029
  %"&pSB[currWI].offset2129.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2128.i"
  %CastToValueType2130.i = bitcast i8* %"&pSB[currWI].offset2129.i" to i1*
  %loadedValue2131.i = load i1* %CastToValueType2130.i, align 1
  br i1 %loadedValue2131.i, label %preload1169.i, label %postload1170.i

preload1169.i:                                    ; preds = %postload1159.i
  %masked_load748.i = load double addrspace(3)* %217, align 8
  br label %postload1170.i

postload1170.i:                                   ; preds = %preload1169.i, %postload1159.i
  %phi1171.i = phi double [ undef, %postload1159.i ], [ %masked_load748.i, %preload1169.i ]
  %"&(pSB[currWI].offset)2152.i" = add nuw i64 %CurrSBIndex..9.i, 1030
  %"&pSB[currWI].offset2153.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2152.i"
  %CastToValueType2154.i = bitcast i8* %"&pSB[currWI].offset2153.i" to i1*
  %loadedValue2155.i = load i1* %CastToValueType2154.i, align 1
  br i1 %loadedValue2155.i, label %preload1180.i, label %postload1181.i

preload1180.i:                                    ; preds = %postload1170.i
  %masked_load749.i = load double addrspace(3)* %218, align 8
  br label %postload1181.i

postload1181.i:                                   ; preds = %preload1180.i, %postload1170.i
  %phi1182.i = phi double [ undef, %postload1170.i ], [ %masked_load749.i, %preload1180.i ]
  %"&(pSB[currWI].offset)2176.i" = add nuw i64 %CurrSBIndex..9.i, 1031
  %"&pSB[currWI].offset2177.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2176.i"
  %CastToValueType2178.i = bitcast i8* %"&pSB[currWI].offset2177.i" to i1*
  %loadedValue2179.i = load i1* %CastToValueType2178.i, align 1
  br i1 %loadedValue2179.i, label %preload1191.i, label %postload1192.i

preload1191.i:                                    ; preds = %postload1181.i
  %masked_load750.i = load double addrspace(3)* %219, align 8
  br label %postload1192.i

postload1192.i:                                   ; preds = %preload1191.i, %postload1181.i
  %phi1193.i = phi double [ undef, %postload1181.i ], [ %masked_load750.i, %preload1191.i ]
  %temp.vect443.i = insertelement <16 x double> undef, double %phi1028.i, i32 0
  %temp.vect444.i = insertelement <16 x double> %temp.vect443.i, double %phi1039.i, i32 1
  %temp.vect445.i = insertelement <16 x double> %temp.vect444.i, double %phi1050.i, i32 2
  %temp.vect446.i = insertelement <16 x double> %temp.vect445.i, double %phi1061.i, i32 3
  %temp.vect447.i = insertelement <16 x double> %temp.vect446.i, double %phi1072.i, i32 4
  %temp.vect448.i = insertelement <16 x double> %temp.vect447.i, double %phi1083.i, i32 5
  %temp.vect449.i = insertelement <16 x double> %temp.vect448.i, double %phi1094.i, i32 6
  %temp.vect450.i = insertelement <16 x double> %temp.vect449.i, double %phi1105.i, i32 7
  %temp.vect451.i = insertelement <16 x double> %temp.vect450.i, double %phi1116.i, i32 8
  %temp.vect452.i = insertelement <16 x double> %temp.vect451.i, double %phi1127.i, i32 9
  %temp.vect453.i = insertelement <16 x double> %temp.vect452.i, double %phi1138.i, i32 10
  %temp.vect454.i = insertelement <16 x double> %temp.vect453.i, double %phi1149.i, i32 11
  %temp.vect455.i = insertelement <16 x double> %temp.vect454.i, double %phi1160.i, i32 12
  %temp.vect456.i = insertelement <16 x double> %temp.vect455.i, double %phi1171.i, i32 13
  %temp.vect457.i = insertelement <16 x double> %temp.vect456.i, double %phi1182.i, i32 14
  %temp.vect458.i = insertelement <16 x double> %temp.vect457.i, double %phi1193.i, i32 15
  %"&(pSB[currWI].offset)1415.i" = add nuw i64 %CurrSBIndex..9.i, 512
  %"&pSB[currWI].offset1416.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1415.i"
  %CastToValueType1417.i = bitcast i8* %"&pSB[currWI].offset1416.i" to <16 x double>*
  %loadedValue1418.i = load <16 x double>* %CastToValueType1417.i, align 128
  %add29459.i = fadd <16 x double> %temp.vect458.i, %loadedValue1418.i
  %"&(pSB[currWI].offset)2222.i" = add nuw i64 %CurrSBIndex..9.i, 1536
  %"&pSB[currWI].offset2223.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2222.i"
  %CastToValueType2224.i = bitcast i8* %"&pSB[currWI].offset2223.i" to <16 x double>*
  %loadedValue2225.i = load <16 x double>* %CastToValueType2224.i, align 128
  %add30460.i = fadd <16 x double> %loadedValue2225.i, %add29459.i
  %extract513.i = extractelement <16 x double> %add30460.i, i32 0
  %extract514.i = extractelement <16 x double> %add30460.i, i32 1
  %extract515.i = extractelement <16 x double> %add30460.i, i32 2
  %extract516.i = extractelement <16 x double> %add30460.i, i32 3
  %extract517.i = extractelement <16 x double> %add30460.i, i32 4
  %extract518.i = extractelement <16 x double> %add30460.i, i32 5
  %extract519.i = extractelement <16 x double> %add30460.i, i32 6
  %extract520.i = extractelement <16 x double> %add30460.i, i32 7
  %extract521.i = extractelement <16 x double> %add30460.i, i32 8
  %extract522.i = extractelement <16 x double> %add30460.i, i32 9
  %extract523.i = extractelement <16 x double> %add30460.i, i32 10
  %extract524.i = extractelement <16 x double> %add30460.i, i32 11
  %extract525.i = extractelement <16 x double> %add30460.i, i32 12
  %extract526.i = extractelement <16 x double> %add30460.i, i32 13
  %extract527.i = extractelement <16 x double> %add30460.i, i32 14
  %extract528.i = extractelement <16 x double> %add30460.i, i32 15
  %"&(pSB[currWI].offset)2195.i" = add nuw i64 %CurrSBIndex..9.i, 1152
  %"&pSB[currWI].offset2196.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2195.i"
  %CastToValueType2197.i = bitcast i8* %"&pSB[currWI].offset2196.i" to <16 x double>*
  %loadedValue2198.i = load <16 x double>* %CastToValueType2197.i, align 128
  %add24462.i = fadd <16 x double> %loadedValue2198.i, %add29459.i
  %extract466.i = extractelement <16 x double> %add24462.i, i32 1
  %extract467.i = extractelement <16 x double> %add24462.i, i32 2
  %extract468.i = extractelement <16 x double> %add24462.i, i32 3
  %extract469.i = extractelement <16 x double> %add24462.i, i32 4
  %extract470.i = extractelement <16 x double> %add24462.i, i32 5
  %extract471.i = extractelement <16 x double> %add24462.i, i32 6
  %extract472.i = extractelement <16 x double> %add24462.i, i32 7
  %extract473.i = extractelement <16 x double> %add24462.i, i32 8
  %extract474.i = extractelement <16 x double> %add24462.i, i32 9
  %extract475.i = extractelement <16 x double> %add24462.i, i32 10
  %extract476.i = extractelement <16 x double> %add24462.i, i32 11
  %extract477.i = extractelement <16 x double> %add24462.i, i32 12
  %extract478.i = extractelement <16 x double> %add24462.i, i32 13
  %extract479.i = extractelement <16 x double> %add24462.i, i32 14
  %extract480.i = extractelement <16 x double> %add24462.i, i32 15
  %"&(pSB[currWI].offset)2204.i" = add nuw i64 %CurrSBIndex..9.i, 1280
  %"&pSB[currWI].offset2205.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2204.i"
  %CastToValueType2206.i = bitcast i8* %"&pSB[currWI].offset2205.i" to <16 x double>*
  %loadedValue2207.i = load <16 x double>* %CastToValueType2206.i, align 128
  %add26463.i = fadd <16 x double> %loadedValue2207.i, %add29459.i
  %extract482.i = extractelement <16 x double> %add26463.i, i32 1
  %extract483.i = extractelement <16 x double> %add26463.i, i32 2
  %extract484.i = extractelement <16 x double> %add26463.i, i32 3
  %extract485.i = extractelement <16 x double> %add26463.i, i32 4
  %extract486.i = extractelement <16 x double> %add26463.i, i32 5
  %extract487.i = extractelement <16 x double> %add26463.i, i32 6
  %extract488.i = extractelement <16 x double> %add26463.i, i32 7
  %extract489.i = extractelement <16 x double> %add26463.i, i32 8
  %extract490.i = extractelement <16 x double> %add26463.i, i32 9
  %extract491.i = extractelement <16 x double> %add26463.i, i32 10
  %extract492.i = extractelement <16 x double> %add26463.i, i32 11
  %extract493.i = extractelement <16 x double> %add26463.i, i32 12
  %extract494.i = extractelement <16 x double> %add26463.i, i32 13
  %extract495.i = extractelement <16 x double> %add26463.i, i32 14
  %extract496.i = extractelement <16 x double> %add26463.i, i32 15
  %"&(pSB[currWI].offset)2213.i" = add nuw i64 %CurrSBIndex..9.i, 1408
  %"&pSB[currWI].offset2214.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2213.i"
  %CastToValueType2215.i = bitcast i8* %"&pSB[currWI].offset2214.i" to <16 x double>*
  %loadedValue2216.i = load <16 x double>* %CastToValueType2215.i, align 128
  %add28464.i = fadd <16 x double> %loadedValue2216.i, %add29459.i
  %extract498.i = extractelement <16 x double> %add28464.i, i32 1
  %extract499.i = extractelement <16 x double> %add28464.i, i32 2
  %extract500.i = extractelement <16 x double> %add28464.i, i32 3
  %extract501.i = extractelement <16 x double> %add28464.i, i32 4
  %extract502.i = extractelement <16 x double> %add28464.i, i32 5
  %extract503.i = extractelement <16 x double> %add28464.i, i32 6
  %extract504.i = extractelement <16 x double> %add28464.i, i32 7
  %extract505.i = extractelement <16 x double> %add28464.i, i32 8
  %extract506.i = extractelement <16 x double> %add28464.i, i32 9
  %extract507.i = extractelement <16 x double> %add28464.i, i32 10
  %extract508.i = extractelement <16 x double> %add28464.i, i32 11
  %extract509.i = extractelement <16 x double> %add28464.i, i32 12
  %extract510.i = extractelement <16 x double> %add28464.i, i32 13
  %extract511.i = extractelement <16 x double> %add28464.i, i32 14
  %extract512.i = extractelement <16 x double> %add28464.i, i32 15
  %221 = insertelement <4 x double> undef, double %extract466.i, i32 0
  %222 = insertelement <4 x double> undef, double %extract467.i, i32 0
  %223 = insertelement <4 x double> undef, double %extract468.i, i32 0
  %224 = insertelement <4 x double> undef, double %extract469.i, i32 0
  %225 = insertelement <4 x double> undef, double %extract470.i, i32 0
  %226 = insertelement <4 x double> undef, double %extract471.i, i32 0
  %227 = insertelement <4 x double> undef, double %extract472.i, i32 0
  %228 = insertelement <4 x double> undef, double %extract473.i, i32 0
  %229 = insertelement <4 x double> undef, double %extract474.i, i32 0
  %230 = insertelement <4 x double> undef, double %extract475.i, i32 0
  %231 = insertelement <4 x double> undef, double %extract476.i, i32 0
  %232 = insertelement <4 x double> undef, double %extract477.i, i32 0
  %233 = insertelement <4 x double> undef, double %extract478.i, i32 0
  %234 = insertelement <4 x double> undef, double %extract479.i, i32 0
  %235 = insertelement <4 x double> undef, double %extract480.i, i32 0
  %236 = insertelement <4 x double> %221, double %extract482.i, i32 1
  %237 = insertelement <4 x double> %222, double %extract483.i, i32 1
  %238 = insertelement <4 x double> %223, double %extract484.i, i32 1
  %239 = insertelement <4 x double> %224, double %extract485.i, i32 1
  %240 = insertelement <4 x double> %225, double %extract486.i, i32 1
  %241 = insertelement <4 x double> %226, double %extract487.i, i32 1
  %242 = insertelement <4 x double> %227, double %extract488.i, i32 1
  %243 = insertelement <4 x double> %228, double %extract489.i, i32 1
  %244 = insertelement <4 x double> %229, double %extract490.i, i32 1
  %245 = insertelement <4 x double> %230, double %extract491.i, i32 1
  %246 = insertelement <4 x double> %231, double %extract492.i, i32 1
  %247 = insertelement <4 x double> %232, double %extract493.i, i32 1
  %248 = insertelement <4 x double> %233, double %extract494.i, i32 1
  %249 = insertelement <4 x double> %234, double %extract495.i, i32 1
  %250 = insertelement <4 x double> %235, double %extract496.i, i32 1
  %251 = insertelement <4 x double> %236, double %extract498.i, i32 2
  %252 = insertelement <4 x double> %237, double %extract499.i, i32 2
  %253 = insertelement <4 x double> %238, double %extract500.i, i32 2
  %254 = insertelement <4 x double> %239, double %extract501.i, i32 2
  %255 = insertelement <4 x double> %240, double %extract502.i, i32 2
  %256 = insertelement <4 x double> %241, double %extract503.i, i32 2
  %257 = insertelement <4 x double> %242, double %extract504.i, i32 2
  %258 = insertelement <4 x double> %243, double %extract505.i, i32 2
  %259 = insertelement <4 x double> %244, double %extract506.i, i32 2
  %260 = insertelement <4 x double> %245, double %extract507.i, i32 2
  %261 = insertelement <4 x double> %246, double %extract508.i, i32 2
  %262 = insertelement <4 x double> %247, double %extract509.i, i32 2
  %263 = insertelement <4 x double> %248, double %extract510.i, i32 2
  %264 = insertelement <4 x double> %249, double %extract511.i, i32 2
  %265 = insertelement <4 x double> %250, double %extract512.i, i32 2
  %266 = insertelement <4 x double> %251, double %extract514.i, i32 3
  %267 = insertelement <4 x double> %252, double %extract515.i, i32 3
  %268 = insertelement <4 x double> %253, double %extract516.i, i32 3
  %269 = insertelement <4 x double> %254, double %extract517.i, i32 3
  %270 = insertelement <4 x double> %255, double %extract518.i, i32 3
  %271 = insertelement <4 x double> %256, double %extract519.i, i32 3
  %272 = insertelement <4 x double> %257, double %extract520.i, i32 3
  %273 = insertelement <4 x double> %258, double %extract521.i, i32 3
  %274 = insertelement <4 x double> %259, double %extract522.i, i32 3
  %275 = insertelement <4 x double> %260, double %extract523.i, i32 3
  %276 = insertelement <4 x double> %261, double %extract524.i, i32 3
  %277 = insertelement <4 x double> %262, double %extract525.i, i32 3
  %278 = insertelement <4 x double> %263, double %extract526.i, i32 3
  %279 = insertelement <4 x double> %264, double %extract527.i, i32 3
  %280 = insertelement <4 x double> %265, double %extract528.i, i32 3
  %"&(pSB[currWI].offset)1671.i" = add nuw i64 %CurrSBIndex..9.i, 896
  %"&pSB[currWI].offset1672.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1671.i"
  %CastToValueType1673.i = bitcast i8* %"&pSB[currWI].offset1672.i" to i64*
  %loadedValue1674.i = load i64* %CastToValueType1673.i, align 8
  %281 = getelementptr inbounds <4 x double> addrspace(1)* %34, i64 %loadedValue1674.i
  %"&(pSB[currWI].offset)1680.i" = add nuw i64 %CurrSBIndex..9.i, 904
  %"&pSB[currWI].offset1681.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1680.i"
  %CastToValueType1682.i = bitcast i8* %"&pSB[currWI].offset1681.i" to i64*
  %loadedValue1683.i = load i64* %CastToValueType1682.i, align 8
  %282 = getelementptr inbounds <4 x double> addrspace(1)* %34, i64 %loadedValue1683.i
  %"&(pSB[currWI].offset)1689.i" = add nuw i64 %CurrSBIndex..9.i, 912
  %"&pSB[currWI].offset1690.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1689.i"
  %CastToValueType1691.i = bitcast i8* %"&pSB[currWI].offset1690.i" to i64*
  %loadedValue1692.i = load i64* %CastToValueType1691.i, align 8
  %283 = getelementptr inbounds <4 x double> addrspace(1)* %34, i64 %loadedValue1692.i
  %"&(pSB[currWI].offset)1698.i" = add nuw i64 %CurrSBIndex..9.i, 920
  %"&pSB[currWI].offset1699.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1698.i"
  %CastToValueType1700.i = bitcast i8* %"&pSB[currWI].offset1699.i" to i64*
  %loadedValue1701.i = load i64* %CastToValueType1700.i, align 8
  %284 = getelementptr inbounds <4 x double> addrspace(1)* %34, i64 %loadedValue1701.i
  %"&(pSB[currWI].offset)1707.i" = add nuw i64 %CurrSBIndex..9.i, 928
  %"&pSB[currWI].offset1708.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1707.i"
  %CastToValueType1709.i = bitcast i8* %"&pSB[currWI].offset1708.i" to i64*
  %loadedValue1710.i = load i64* %CastToValueType1709.i, align 8
  %285 = getelementptr inbounds <4 x double> addrspace(1)* %34, i64 %loadedValue1710.i
  %"&(pSB[currWI].offset)1716.i" = add nuw i64 %CurrSBIndex..9.i, 936
  %"&pSB[currWI].offset1717.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1716.i"
  %CastToValueType1718.i = bitcast i8* %"&pSB[currWI].offset1717.i" to i64*
  %loadedValue1719.i = load i64* %CastToValueType1718.i, align 8
  %286 = getelementptr inbounds <4 x double> addrspace(1)* %34, i64 %loadedValue1719.i
  %"&(pSB[currWI].offset)1725.i" = add nuw i64 %CurrSBIndex..9.i, 944
  %"&pSB[currWI].offset1726.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1725.i"
  %CastToValueType1727.i = bitcast i8* %"&pSB[currWI].offset1726.i" to i64*
  %loadedValue1728.i = load i64* %CastToValueType1727.i, align 8
  %287 = getelementptr inbounds <4 x double> addrspace(1)* %34, i64 %loadedValue1728.i
  %"&(pSB[currWI].offset)1734.i" = add nuw i64 %CurrSBIndex..9.i, 952
  %"&pSB[currWI].offset1735.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1734.i"
  %CastToValueType1736.i = bitcast i8* %"&pSB[currWI].offset1735.i" to i64*
  %loadedValue1737.i = load i64* %CastToValueType1736.i, align 8
  %288 = getelementptr inbounds <4 x double> addrspace(1)* %34, i64 %loadedValue1737.i
  %"&(pSB[currWI].offset)1743.i" = add nuw i64 %CurrSBIndex..9.i, 960
  %"&pSB[currWI].offset1744.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1743.i"
  %CastToValueType1745.i = bitcast i8* %"&pSB[currWI].offset1744.i" to i64*
  %loadedValue1746.i = load i64* %CastToValueType1745.i, align 8
  %289 = getelementptr inbounds <4 x double> addrspace(1)* %34, i64 %loadedValue1746.i
  %"&(pSB[currWI].offset)1752.i" = add nuw i64 %CurrSBIndex..9.i, 968
  %"&pSB[currWI].offset1753.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1752.i"
  %CastToValueType1754.i = bitcast i8* %"&pSB[currWI].offset1753.i" to i64*
  %loadedValue1755.i = load i64* %CastToValueType1754.i, align 8
  %290 = getelementptr inbounds <4 x double> addrspace(1)* %34, i64 %loadedValue1755.i
  %"&(pSB[currWI].offset)1761.i" = add nuw i64 %CurrSBIndex..9.i, 976
  %"&pSB[currWI].offset1762.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1761.i"
  %CastToValueType1763.i = bitcast i8* %"&pSB[currWI].offset1762.i" to i64*
  %loadedValue1764.i = load i64* %CastToValueType1763.i, align 8
  %291 = getelementptr inbounds <4 x double> addrspace(1)* %34, i64 %loadedValue1764.i
  %"&(pSB[currWI].offset)1770.i" = add nuw i64 %CurrSBIndex..9.i, 984
  %"&pSB[currWI].offset1771.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1770.i"
  %CastToValueType1772.i = bitcast i8* %"&pSB[currWI].offset1771.i" to i64*
  %loadedValue1773.i = load i64* %CastToValueType1772.i, align 8
  %292 = getelementptr inbounds <4 x double> addrspace(1)* %34, i64 %loadedValue1773.i
  %"&(pSB[currWI].offset)1779.i" = add nuw i64 %CurrSBIndex..9.i, 992
  %"&pSB[currWI].offset1780.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1779.i"
  %CastToValueType1781.i = bitcast i8* %"&pSB[currWI].offset1780.i" to i64*
  %loadedValue1782.i = load i64* %CastToValueType1781.i, align 8
  %293 = getelementptr inbounds <4 x double> addrspace(1)* %34, i64 %loadedValue1782.i
  %"&(pSB[currWI].offset)1788.i" = add nuw i64 %CurrSBIndex..9.i, 1000
  %"&pSB[currWI].offset1789.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1788.i"
  %CastToValueType1790.i = bitcast i8* %"&pSB[currWI].offset1789.i" to i64*
  %loadedValue1791.i = load i64* %CastToValueType1790.i, align 8
  %294 = getelementptr inbounds <4 x double> addrspace(1)* %34, i64 %loadedValue1791.i
  %"&(pSB[currWI].offset)1797.i" = add nuw i64 %CurrSBIndex..9.i, 1008
  %"&pSB[currWI].offset1798.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1797.i"
  %CastToValueType1799.i = bitcast i8* %"&pSB[currWI].offset1798.i" to i64*
  %loadedValue1800.i = load i64* %CastToValueType1799.i, align 8
  %295 = getelementptr inbounds <4 x double> addrspace(1)* %34, i64 %loadedValue1800.i
  %"&(pSB[currWI].offset)1643.i" = add nuw i64 %CurrSBIndex..9.i, 659
  %"&pSB[currWI].offset1644.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1643.i"
  %CastToValueType1645.i = bitcast i8* %"&pSB[currWI].offset1644.i" to i1*
  %loadedValue1646.i = load i1* %CastToValueType1645.i, align 1
  br i1 %loadedValue1646.i, label %preload957.i, label %postload958.i

preload957.i:                                     ; preds = %postload1192.i
  %extract465.i = extractelement <16 x double> %add24462.i, i32 0
  %296 = insertelement <4 x double> undef, double %extract465.i, i32 0
  %extract481.i = extractelement <16 x double> %add26463.i, i32 0
  %297 = insertelement <4 x double> %296, double %extract481.i, i32 1
  %extract497.i = extractelement <16 x double> %add28464.i, i32 0
  %"&(pSB[currWI].offset)1657.i" = add nuw i64 %CurrSBIndex..9.i, 768
  %"&pSB[currWI].offset1658.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1657.i"
  %CastToValueType1659.i = bitcast i8* %"&pSB[currWI].offset1658.i" to <16 x i64>*
  %loadedValue1660.i = load <16 x i64>* %CastToValueType1659.i, align 128
  %extract530.i = extractelement <16 x i64> %loadedValue1660.i, i32 0
  %298 = insertelement <4 x double> %297, double %extract497.i, i32 2
  %299 = getelementptr inbounds <4 x double> addrspace(1)* %34, i64 %extract530.i
  %300 = insertelement <4 x double> %298, double %extract513.i, i32 3
  store <4 x double> %300, <4 x double> addrspace(1)* %299, align 32
  br label %postload958.i

postload958.i:                                    ; preds = %preload957.i, %postload1192.i
  %"&(pSB[currWI].offset)1433.i" = add nuw i64 %CurrSBIndex..9.i, 644
  %"&pSB[currWI].offset1434.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1433.i"
  %CastToValueType1435.i = bitcast i8* %"&pSB[currWI].offset1434.i" to i1*
  %loadedValue1436.i = load i1* %CastToValueType1435.i, align 1
  br i1 %loadedValue1436.i, label %preload959.i, label %postload960.i

preload959.i:                                     ; preds = %postload958.i
  store <4 x double> %266, <4 x double> addrspace(1)* %281, align 32
  br label %postload960.i

postload960.i:                                    ; preds = %preload959.i, %postload958.i
  %"&(pSB[currWI].offset)1447.i" = add nuw i64 %CurrSBIndex..9.i, 645
  %"&pSB[currWI].offset1448.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1447.i"
  %CastToValueType1449.i = bitcast i8* %"&pSB[currWI].offset1448.i" to i1*
  %loadedValue1450.i = load i1* %CastToValueType1449.i, align 1
  br i1 %loadedValue1450.i, label %preload961.i, label %postload962.i

preload961.i:                                     ; preds = %postload960.i
  store <4 x double> %267, <4 x double> addrspace(1)* %282, align 32
  br label %postload962.i

postload962.i:                                    ; preds = %preload961.i, %postload960.i
  %"&(pSB[currWI].offset)1461.i" = add nuw i64 %CurrSBIndex..9.i, 646
  %"&pSB[currWI].offset1462.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1461.i"
  %CastToValueType1463.i = bitcast i8* %"&pSB[currWI].offset1462.i" to i1*
  %loadedValue1464.i = load i1* %CastToValueType1463.i, align 1
  br i1 %loadedValue1464.i, label %preload963.i, label %postload964.i

preload963.i:                                     ; preds = %postload962.i
  store <4 x double> %268, <4 x double> addrspace(1)* %283, align 32
  br label %postload964.i

postload964.i:                                    ; preds = %preload963.i, %postload962.i
  %"&(pSB[currWI].offset)1475.i" = add nuw i64 %CurrSBIndex..9.i, 647
  %"&pSB[currWI].offset1476.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1475.i"
  %CastToValueType1477.i = bitcast i8* %"&pSB[currWI].offset1476.i" to i1*
  %loadedValue1478.i = load i1* %CastToValueType1477.i, align 1
  br i1 %loadedValue1478.i, label %preload965.i, label %postload966.i

preload965.i:                                     ; preds = %postload964.i
  store <4 x double> %269, <4 x double> addrspace(1)* %284, align 32
  br label %postload966.i

postload966.i:                                    ; preds = %preload965.i, %postload964.i
  %"&(pSB[currWI].offset)1489.i" = add nuw i64 %CurrSBIndex..9.i, 648
  %"&pSB[currWI].offset1490.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1489.i"
  %CastToValueType1491.i = bitcast i8* %"&pSB[currWI].offset1490.i" to i1*
  %loadedValue1492.i = load i1* %CastToValueType1491.i, align 1
  br i1 %loadedValue1492.i, label %preload967.i, label %postload968.i

preload967.i:                                     ; preds = %postload966.i
  store <4 x double> %270, <4 x double> addrspace(1)* %285, align 32
  br label %postload968.i

postload968.i:                                    ; preds = %preload967.i, %postload966.i
  %"&(pSB[currWI].offset)1503.i" = add nuw i64 %CurrSBIndex..9.i, 649
  %"&pSB[currWI].offset1504.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1503.i"
  %CastToValueType1505.i = bitcast i8* %"&pSB[currWI].offset1504.i" to i1*
  %loadedValue1506.i = load i1* %CastToValueType1505.i, align 1
  br i1 %loadedValue1506.i, label %preload969.i, label %postload970.i

preload969.i:                                     ; preds = %postload968.i
  store <4 x double> %271, <4 x double> addrspace(1)* %286, align 32
  br label %postload970.i

postload970.i:                                    ; preds = %preload969.i, %postload968.i
  %"&(pSB[currWI].offset)1517.i" = add nuw i64 %CurrSBIndex..9.i, 650
  %"&pSB[currWI].offset1518.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1517.i"
  %CastToValueType1519.i = bitcast i8* %"&pSB[currWI].offset1518.i" to i1*
  %loadedValue1520.i = load i1* %CastToValueType1519.i, align 1
  br i1 %loadedValue1520.i, label %preload971.i, label %postload972.i

preload971.i:                                     ; preds = %postload970.i
  store <4 x double> %272, <4 x double> addrspace(1)* %287, align 32
  br label %postload972.i

postload972.i:                                    ; preds = %preload971.i, %postload970.i
  %"&(pSB[currWI].offset)1531.i" = add nuw i64 %CurrSBIndex..9.i, 651
  %"&pSB[currWI].offset1532.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1531.i"
  %CastToValueType1533.i = bitcast i8* %"&pSB[currWI].offset1532.i" to i1*
  %loadedValue1534.i = load i1* %CastToValueType1533.i, align 1
  br i1 %loadedValue1534.i, label %preload973.i, label %postload974.i

preload973.i:                                     ; preds = %postload972.i
  store <4 x double> %273, <4 x double> addrspace(1)* %288, align 32
  br label %postload974.i

postload974.i:                                    ; preds = %preload973.i, %postload972.i
  %"&(pSB[currWI].offset)1545.i" = add nuw i64 %CurrSBIndex..9.i, 652
  %"&pSB[currWI].offset1546.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1545.i"
  %CastToValueType1547.i = bitcast i8* %"&pSB[currWI].offset1546.i" to i1*
  %loadedValue1548.i = load i1* %CastToValueType1547.i, align 1
  br i1 %loadedValue1548.i, label %preload975.i, label %postload976.i

preload975.i:                                     ; preds = %postload974.i
  store <4 x double> %274, <4 x double> addrspace(1)* %289, align 32
  br label %postload976.i

postload976.i:                                    ; preds = %preload975.i, %postload974.i
  %"&(pSB[currWI].offset)1559.i" = add nuw i64 %CurrSBIndex..9.i, 653
  %"&pSB[currWI].offset1560.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1559.i"
  %CastToValueType1561.i = bitcast i8* %"&pSB[currWI].offset1560.i" to i1*
  %loadedValue1562.i = load i1* %CastToValueType1561.i, align 1
  br i1 %loadedValue1562.i, label %preload977.i, label %postload978.i

preload977.i:                                     ; preds = %postload976.i
  store <4 x double> %275, <4 x double> addrspace(1)* %290, align 32
  br label %postload978.i

postload978.i:                                    ; preds = %preload977.i, %postload976.i
  %"&(pSB[currWI].offset)1573.i" = add nuw i64 %CurrSBIndex..9.i, 654
  %"&pSB[currWI].offset1574.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1573.i"
  %CastToValueType1575.i = bitcast i8* %"&pSB[currWI].offset1574.i" to i1*
  %loadedValue1576.i = load i1* %CastToValueType1575.i, align 1
  br i1 %loadedValue1576.i, label %preload979.i, label %postload980.i

preload979.i:                                     ; preds = %postload978.i
  store <4 x double> %276, <4 x double> addrspace(1)* %291, align 32
  br label %postload980.i

postload980.i:                                    ; preds = %preload979.i, %postload978.i
  %"&(pSB[currWI].offset)1587.i" = add nuw i64 %CurrSBIndex..9.i, 655
  %"&pSB[currWI].offset1588.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1587.i"
  %CastToValueType1589.i = bitcast i8* %"&pSB[currWI].offset1588.i" to i1*
  %loadedValue1590.i = load i1* %CastToValueType1589.i, align 1
  br i1 %loadedValue1590.i, label %preload981.i, label %postload982.i

preload981.i:                                     ; preds = %postload980.i
  store <4 x double> %277, <4 x double> addrspace(1)* %292, align 32
  br label %postload982.i

postload982.i:                                    ; preds = %preload981.i, %postload980.i
  %"&(pSB[currWI].offset)1601.i" = add nuw i64 %CurrSBIndex..9.i, 656
  %"&pSB[currWI].offset1602.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1601.i"
  %CastToValueType1603.i = bitcast i8* %"&pSB[currWI].offset1602.i" to i1*
  %loadedValue1604.i = load i1* %CastToValueType1603.i, align 1
  br i1 %loadedValue1604.i, label %preload983.i, label %postload984.i

preload983.i:                                     ; preds = %postload982.i
  store <4 x double> %278, <4 x double> addrspace(1)* %293, align 32
  br label %postload984.i

postload984.i:                                    ; preds = %preload983.i, %postload982.i
  %"&(pSB[currWI].offset)1615.i" = add nuw i64 %CurrSBIndex..9.i, 657
  %"&pSB[currWI].offset1616.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1615.i"
  %CastToValueType1617.i = bitcast i8* %"&pSB[currWI].offset1616.i" to i1*
  %loadedValue1618.i = load i1* %CastToValueType1617.i, align 1
  br i1 %loadedValue1618.i, label %preload985.i, label %postload986.i

preload985.i:                                     ; preds = %postload984.i
  store <4 x double> %279, <4 x double> addrspace(1)* %294, align 32
  br label %postload986.i

postload986.i:                                    ; preds = %preload985.i, %postload984.i
  %"&(pSB[currWI].offset)1629.i" = add nuw i64 %CurrSBIndex..9.i, 658
  %"&pSB[currWI].offset1630.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1629.i"
  %CastToValueType1631.i = bitcast i8* %"&pSB[currWI].offset1630.i" to i1*
  %loadedValue1632.i = load i1* %CastToValueType1631.i, align 1
  br i1 %loadedValue1632.i, label %preload987.i, label %postload986.i.if.end36.i_crit_edge

postload986.i.if.end36.i_crit_edge:               ; preds = %postload986.i
  br label %if.end36.i

preload987.i:                                     ; preds = %postload986.i
  store <4 x double> %280, <4 x double> addrspace(1)* %295, align 32
  br label %if.end36.i

if.end36.i:                                       ; preds = %postload986.i.if.end36.i_crit_edge, %preload987.i
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
  br i1 %extract563.i, label %preload989.i, label %postload990.i

preload989.i:                                     ; preds = %if.end36.i
  store double %extract513.i, double addrspace(3)* %32, align 8
  br label %postload990.i

postload990.i:                                    ; preds = %preload989.i, %if.end36.i
  br i1 %extract564.i, label %preload991.i, label %postload992.i

preload991.i:                                     ; preds = %postload990.i
  store double %extract514.i, double addrspace(3)* %32, align 8
  br label %postload992.i

postload992.i:                                    ; preds = %preload991.i, %postload990.i
  br i1 %extract565.i, label %preload993.i, label %postload994.i

preload993.i:                                     ; preds = %postload992.i
  store double %extract515.i, double addrspace(3)* %32, align 8
  br label %postload994.i

postload994.i:                                    ; preds = %preload993.i, %postload992.i
  br i1 %extract566.i, label %preload995.i, label %postload996.i

preload995.i:                                     ; preds = %postload994.i
  store double %extract516.i, double addrspace(3)* %32, align 8
  br label %postload996.i

postload996.i:                                    ; preds = %preload995.i, %postload994.i
  br i1 %extract567.i, label %preload997.i, label %postload998.i

preload997.i:                                     ; preds = %postload996.i
  store double %extract517.i, double addrspace(3)* %32, align 8
  br label %postload998.i

postload998.i:                                    ; preds = %preload997.i, %postload996.i
  br i1 %extract568.i, label %preload999.i, label %postload1000.i

preload999.i:                                     ; preds = %postload998.i
  store double %extract518.i, double addrspace(3)* %32, align 8
  br label %postload1000.i

postload1000.i:                                   ; preds = %preload999.i, %postload998.i
  br i1 %extract569.i, label %preload1212.i, label %postload1213.i

preload1212.i:                                    ; preds = %postload1000.i
  store double %extract519.i, double addrspace(3)* %32, align 8
  br label %postload1213.i

postload1213.i:                                   ; preds = %preload1212.i, %postload1000.i
  br i1 %extract570.i, label %preload1214.i, label %postload1215.i

preload1214.i:                                    ; preds = %postload1213.i
  store double %extract520.i, double addrspace(3)* %32, align 8
  br label %postload1215.i

postload1215.i:                                   ; preds = %preload1214.i, %postload1213.i
  br i1 %extract571.i, label %preload1216.i, label %postload1217.i

preload1216.i:                                    ; preds = %postload1215.i
  store double %extract521.i, double addrspace(3)* %32, align 8
  br label %postload1217.i

postload1217.i:                                   ; preds = %preload1216.i, %postload1215.i
  br i1 %extract572.i, label %preload1218.i, label %postload1219.i

preload1218.i:                                    ; preds = %postload1217.i
  store double %extract522.i, double addrspace(3)* %32, align 8
  br label %postload1219.i

postload1219.i:                                   ; preds = %preload1218.i, %postload1217.i
  br i1 %extract573.i, label %preload1220.i, label %postload1221.i

preload1220.i:                                    ; preds = %postload1219.i
  store double %extract523.i, double addrspace(3)* %32, align 8
  br label %postload1221.i

postload1221.i:                                   ; preds = %preload1220.i, %postload1219.i
  br i1 %extract574.i, label %preload1222.i, label %postload1223.i

preload1222.i:                                    ; preds = %postload1221.i
  store double %extract524.i, double addrspace(3)* %32, align 8
  br label %postload1223.i

postload1223.i:                                   ; preds = %preload1222.i, %postload1221.i
  br i1 %extract575.i, label %preload1224.i, label %postload1225.i

preload1224.i:                                    ; preds = %postload1223.i
  store double %extract525.i, double addrspace(3)* %32, align 8
  br label %postload1225.i

postload1225.i:                                   ; preds = %preload1224.i, %postload1223.i
  br i1 %extract576.i, label %preload1226.i, label %postload1227.i

preload1226.i:                                    ; preds = %postload1225.i
  store double %extract526.i, double addrspace(3)* %32, align 8
  br label %postload1227.i

postload1227.i:                                   ; preds = %preload1226.i, %postload1225.i
  br i1 %extract577.i, label %preload1228.i, label %postload1229.i

preload1228.i:                                    ; preds = %postload1227.i
  store double %extract527.i, double addrspace(3)* %32, align 8
  br label %postload1229.i

postload1229.i:                                   ; preds = %preload1228.i, %postload1227.i
  br i1 %extract578.i, label %preload1230.i, label %postload1229.i.if.end43.i_crit_edge

postload1229.i.if.end43.i_crit_edge:              ; preds = %postload1229.i
  br label %if.end43.i

preload1230.i:                                    ; preds = %postload1229.i
  store double %extract528.i, double addrspace(3)* %32, align 8
  br label %if.end43.i

if.end43.i:                                       ; preds = %postload1229.i.if.end43.i_crit_edge, %preload1230.i
  %loadedValue1809.i = load i1* %CastToValueType1813.i, align 1
  br i1 %loadedValue1809.i, label %preload1029.i, label %if.end43.postload1030_crit_edge.i

if.end43.postload1030_crit_edge.i:                ; preds = %if.end43.i
  %masked_load752.pre.i = load double addrspace(3)* %32, align 8
  br label %postload1030.i

preload1029.i:                                    ; preds = %if.end43.i
  %check.WI.iter2972.i = icmp ult i64 %CurrWI..9.i, %28
  br i1 %check.WI.iter2972.i, label %thenBB2969.i, label %preload1029.i.SyncBB2943.i_crit_edge

preload1029.i.SyncBB2943.i_crit_edge:             ; preds = %preload1029.i
  br label %SyncBB2943.i

thenBB2969.i:                                     ; preds = %preload1029.i
  %"CurrWI++2973.i" = add nuw i64 %CurrWI..9.i, 1
  %"loadedCurrSB+Stride2975.i" = add nuw i64 %CurrSBIndex..9.i, 2176
  switch i32 %currBarrier.9.i, label %postload776.i [
    i32 6, label %SyncBB2939.i
    i32 5, label %thenBB2969.i.postload1025.i_crit_edge
    i32 29, label %SyncBB2941.i
    i32 20, label %thenBB2969.i.SyncBB2943.i_crit_edge
  ]

thenBB2969.i.SyncBB2943.i_crit_edge:              ; preds = %thenBB2969.i
  br label %SyncBB2943.i

thenBB2969.i.postload1025.i_crit_edge:            ; preds = %thenBB2969.i
  br label %postload1025.i

SyncBB2943.i:                                     ; preds = %thenBB2961.i.SyncBB2943.i_crit_edge, %thenBB2969.i.SyncBB2943.i_crit_edge, %preload1029.i.SyncBB2943.i_crit_edge, %thenBB2953.i.SyncBB2943.i_crit_edge, %thenBB2945.i.SyncBB2943.i_crit_edge, %thenBB.i.SyncBB2943.i_crit_edge
  %currBarrier.10.i = phi i32 [ %currBarrier.1.i, %thenBB.i.SyncBB2943.i_crit_edge ], [ %currBarrier.4.i, %thenBB2945.i.SyncBB2943.i_crit_edge ], [ %currBarrier.6.i, %thenBB2953.i.SyncBB2943.i_crit_edge ], [ 20, %preload1029.i.SyncBB2943.i_crit_edge ], [ %currBarrier.9.i, %thenBB2969.i.SyncBB2943.i_crit_edge ], [ %currBarrier.12.i, %thenBB2961.i.SyncBB2943.i_crit_edge ]
  %CurrSBIndex..10.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i.SyncBB2943.i_crit_edge ], [ %"loadedCurrSB+Stride2951.i", %thenBB2945.i.SyncBB2943.i_crit_edge ], [ %"loadedCurrSB+Stride2959.i", %thenBB2953.i.SyncBB2943.i_crit_edge ], [ 0, %preload1029.i.SyncBB2943.i_crit_edge ], [ %"loadedCurrSB+Stride2975.i", %thenBB2969.i.SyncBB2943.i_crit_edge ], [ %"loadedCurrSB+Stride2967.i", %thenBB2961.i.SyncBB2943.i_crit_edge ]
  %CurrWI..10.i = phi i64 [ %"CurrWI++.i", %thenBB.i.SyncBB2943.i_crit_edge ], [ %"CurrWI++2949.i", %thenBB2945.i.SyncBB2943.i_crit_edge ], [ %"CurrWI++2957.i", %thenBB2953.i.SyncBB2943.i_crit_edge ], [ 0, %preload1029.i.SyncBB2943.i_crit_edge ], [ %"CurrWI++2973.i", %thenBB2969.i.SyncBB2943.i_crit_edge ], [ %"CurrWI++2965.i", %thenBB2961.i.SyncBB2943.i_crit_edge ]
  %masked_load751.i = load double addrspace(3)* %32, align 8
  br label %postload1030.i

postload1030.i:                                   ; preds = %SyncBB2943.i, %if.end43.postload1030_crit_edge.i
  %currBarrier.11.i = phi i32 [ %currBarrier.10.i, %SyncBB2943.i ], [ %currBarrier.9.i, %if.end43.postload1030_crit_edge.i ]
  %CurrSBIndex..11.i = phi i64 [ %CurrSBIndex..10.i, %SyncBB2943.i ], [ %CurrSBIndex..9.i, %if.end43.postload1030_crit_edge.i ]
  %CurrWI..11.i = phi i64 [ %CurrWI..10.i, %SyncBB2943.i ], [ %CurrWI..9.i, %if.end43.postload1030_crit_edge.i ]
  %masked_load752.i = phi double [ %masked_load751.i, %SyncBB2943.i ], [ %masked_load752.pre.i, %if.end43.postload1030_crit_edge.i ]
  %phi1031.i = phi double [ %masked_load751.i, %SyncBB2943.i ], [ undef, %if.end43.postload1030_crit_edge.i ]
  %temp.vect625.i = insertelement <16 x double> undef, double %phi1031.i, i32 0
  %temp.vect626.i = insertelement <16 x double> %temp.vect625.i, double %masked_load752.i, i32 1
  %temp.vect627.i = insertelement <16 x double> %temp.vect626.i, double %masked_load752.i, i32 2
  %temp.vect628.i = insertelement <16 x double> %temp.vect627.i, double %masked_load752.i, i32 3
  %temp.vect629.i = insertelement <16 x double> %temp.vect628.i, double %masked_load752.i, i32 4
  %temp.vect630.i = insertelement <16 x double> %temp.vect629.i, double %masked_load752.i, i32 5
  %temp.vect631.i = insertelement <16 x double> %temp.vect630.i, double %masked_load752.i, i32 6
  %temp.vect632.i = insertelement <16 x double> %temp.vect631.i, double %masked_load752.i, i32 7
  %temp.vect633.i = insertelement <16 x double> %temp.vect632.i, double %masked_load752.i, i32 8
  %temp.vect634.i = insertelement <16 x double> %temp.vect633.i, double %masked_load752.i, i32 9
  %temp.vect635.i = insertelement <16 x double> %temp.vect634.i, double %masked_load752.i, i32 10
  %temp.vect636.i = insertelement <16 x double> %temp.vect635.i, double %masked_load752.i, i32 11
  %temp.vect637.i = insertelement <16 x double> %temp.vect636.i, double %masked_load752.i, i32 12
  %temp.vect638.i = insertelement <16 x double> %temp.vect637.i, double %masked_load752.i, i32 13
  %temp.vect639.i = insertelement <16 x double> %temp.vect638.i, double %masked_load752.i, i32 14
  %temp.vect640.i = insertelement <16 x double> %temp.vect639.i, double %masked_load752.i, i32 15
  %"&(pSB[currWI].offset)1424.i" = add nuw i64 %CurrSBIndex..11.i, 640
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
  %"&(pSB[currWI].offset)1652.i" = add nuw i64 %CurrSBIndex..11.i, 768
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

while.end.i:                                      ; preds = %postload1030.i, %postload823.i
  %currBarrier.12.i = phi i32 [ %currBarrier.0.i, %postload823.i ], [ %currBarrier.11.i, %postload1030.i ]
  %CurrSBIndex..12.i = phi i64 [ %CurrSBIndex..0.i, %postload823.i ], [ %CurrSBIndex..11.i, %postload1030.i ]
  %CurrWI..12.i = phi i64 [ %CurrWI..0.i, %postload823.i ], [ %CurrWI..11.i, %postload1030.i ]
  %check.WI.iter2964.i = icmp ult i64 %CurrWI..12.i, %28
  br i1 %check.WI.iter2964.i, label %thenBB2961.i, label %____Vectorized_.bottom_scan_separated_args.exit

thenBB2961.i:                                     ; preds = %while.end.i
  %"CurrWI++2965.i" = add nuw i64 %CurrWI..12.i, 1
  %"loadedCurrSB+Stride2967.i" = add nuw i64 %CurrSBIndex..12.i, 2176
  switch i32 %currBarrier.12.i, label %thenBB2961.i.SyncBB2943.i_crit_edge [
    i32 7, label %postload776.i
    i32 6, label %SyncBB2939.i
    i32 5, label %thenBB2961.i.postload1025.i_crit_edge
    i32 29, label %SyncBB2941.i
  ]

thenBB2961.i.SyncBB2943.i_crit_edge:              ; preds = %thenBB2961.i
  br label %SyncBB2943.i

thenBB2961.i.postload1025.i_crit_edge:            ; preds = %thenBB2961.i
  br label %postload1025.i

____Vectorized_.bottom_scan_separated_args.exit:  ; preds = %while.end.i
  ret void
}

define void @__Vectorized_.reduce(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to double addrspace(3)**
  %10 = load double addrspace(3)** %9, align 8
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

while.body.i:                                     ; preds = %postload559.i, %while.body.preheader.i
  %vectorPHI.i = phi <16 x i1> [ %loop_mask1116.i, %postload559.i ], [ %negIncomingLoopMask53.i, %while.body.preheader.i ]
  %vectorPHI55.i = phi <16 x double> [ %out_sel107.i, %postload559.i ], [ undef, %while.body.preheader.i ]
  %vectorPHI56.i = phi <16 x i1> [ %local_edge135.i, %postload559.i ], [ %cmp134.i, %while.body.preheader.i ]
  %vectorPHI57.i = phi <16 x double> [ %add15106.i, %postload559.i ], [ zeroinitializer, %while.body.preheader.i ]
  %vectorPHI58.i = phi <16 x i32> [ %conv19112.i, %postload559.i ], [ %add1247.i, %while.body.preheader.i ]
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
  %35 = getelementptr inbounds double addrspace(1)* %1, i64 %extract60.i
  %36 = getelementptr inbounds double addrspace(1)* %1, i64 %extract61.i
  %37 = getelementptr inbounds double addrspace(1)* %1, i64 %extract62.i
  %38 = getelementptr inbounds double addrspace(1)* %1, i64 %extract63.i
  %39 = getelementptr inbounds double addrspace(1)* %1, i64 %extract64.i
  %40 = getelementptr inbounds double addrspace(1)* %1, i64 %extract65.i
  %41 = getelementptr inbounds double addrspace(1)* %1, i64 %extract66.i
  %42 = getelementptr inbounds double addrspace(1)* %1, i64 %extract67.i
  %43 = getelementptr inbounds double addrspace(1)* %1, i64 %extract68.i
  %44 = getelementptr inbounds double addrspace(1)* %1, i64 %extract69.i
  %45 = getelementptr inbounds double addrspace(1)* %1, i64 %extract70.i
  %46 = getelementptr inbounds double addrspace(1)* %1, i64 %extract71.i
  %47 = getelementptr inbounds double addrspace(1)* %1, i64 %extract72.i
  %48 = getelementptr inbounds double addrspace(1)* %1, i64 %extract73.i
  %49 = getelementptr inbounds double addrspace(1)* %1, i64 %extract74.i
  br i1 %extract75.i, label %preload677.i, label %postload678.i

preload677.i:                                     ; preds = %while.body.i
  %extract.i = extractelement <16 x i64> %idxprom59.i, i32 0
  %50 = getelementptr inbounds double addrspace(1)* %1, i64 %extract.i
  %masked_load.i = load double addrspace(1)* %50, align 8
  br label %postload678.i

postload678.i:                                    ; preds = %preload677.i, %while.body.i
  %phi679.i = phi double [ undef, %while.body.i ], [ %masked_load.i, %preload677.i ]
  br i1 %extract76.i, label %preload474.i, label %postload475.i

preload474.i:                                     ; preds = %postload678.i
  %masked_load280.i = load double addrspace(1)* %35, align 8
  br label %postload475.i

postload475.i:                                    ; preds = %preload474.i, %postload678.i
  %phi476.i = phi double [ undef, %postload678.i ], [ %masked_load280.i, %preload474.i ]
  br i1 %extract77.i, label %preload561.i, label %postload562.i

preload561.i:                                     ; preds = %postload475.i
  %masked_load281.i = load double addrspace(1)* %36, align 8
  br label %postload562.i

postload562.i:                                    ; preds = %preload561.i, %postload475.i
  %phi563.i = phi double [ undef, %postload475.i ], [ %masked_load281.i, %preload561.i ]
  br i1 %extract78.i, label %preload564.i, label %postload565.i

preload564.i:                                     ; preds = %postload562.i
  %masked_load282.i = load double addrspace(1)* %37, align 8
  br label %postload565.i

postload565.i:                                    ; preds = %preload564.i, %postload562.i
  %phi566.i = phi double [ undef, %postload562.i ], [ %masked_load282.i, %preload564.i ]
  br i1 %extract79.i, label %preload567.i, label %postload568.i

preload567.i:                                     ; preds = %postload565.i
  %masked_load283.i = load double addrspace(1)* %38, align 8
  br label %postload568.i

postload568.i:                                    ; preds = %preload567.i, %postload565.i
  %phi569.i = phi double [ undef, %postload565.i ], [ %masked_load283.i, %preload567.i ]
  br i1 %extract80.i, label %preload540.i, label %postload541.i

preload540.i:                                     ; preds = %postload568.i
  %masked_load284.i = load double addrspace(1)* %39, align 8
  br label %postload541.i

postload541.i:                                    ; preds = %preload540.i, %postload568.i
  %phi542.i = phi double [ undef, %postload568.i ], [ %masked_load284.i, %preload540.i ]
  br i1 %extract81.i, label %preload543.i, label %postload544.i

preload543.i:                                     ; preds = %postload541.i
  %masked_load285.i = load double addrspace(1)* %40, align 8
  br label %postload544.i

postload544.i:                                    ; preds = %preload543.i, %postload541.i
  %phi545.i = phi double [ undef, %postload541.i ], [ %masked_load285.i, %preload543.i ]
  br i1 %extract82.i, label %preload546.i, label %postload547.i

preload546.i:                                     ; preds = %postload544.i
  %masked_load286.i = load double addrspace(1)* %41, align 8
  br label %postload547.i

postload547.i:                                    ; preds = %preload546.i, %postload544.i
  %phi548.i = phi double [ undef, %postload544.i ], [ %masked_load286.i, %preload546.i ]
  br i1 %extract83.i, label %preload549.i, label %postload550.i

preload549.i:                                     ; preds = %postload547.i
  %masked_load287.i = load double addrspace(1)* %42, align 8
  br label %postload550.i

postload550.i:                                    ; preds = %preload549.i, %postload547.i
  %phi551.i = phi double [ undef, %postload547.i ], [ %masked_load287.i, %preload549.i ]
  br i1 %extract84.i, label %preload477.i, label %postload478.i

preload477.i:                                     ; preds = %postload550.i
  %masked_load288.i = load double addrspace(1)* %43, align 8
  br label %postload478.i

postload478.i:                                    ; preds = %preload477.i, %postload550.i
  %phi479.i = phi double [ undef, %postload550.i ], [ %masked_load288.i, %preload477.i ]
  br i1 %extract85.i, label %preload480.i, label %postload481.i

preload480.i:                                     ; preds = %postload478.i
  %masked_load289.i = load double addrspace(1)* %44, align 8
  br label %postload481.i

postload481.i:                                    ; preds = %preload480.i, %postload478.i
  %phi482.i = phi double [ undef, %postload478.i ], [ %masked_load289.i, %preload480.i ]
  br i1 %extract86.i, label %preload483.i, label %postload484.i

preload483.i:                                     ; preds = %postload481.i
  %masked_load290.i = load double addrspace(1)* %45, align 8
  br label %postload484.i

postload484.i:                                    ; preds = %preload483.i, %postload481.i
  %phi485.i = phi double [ undef, %postload481.i ], [ %masked_load290.i, %preload483.i ]
  br i1 %extract87.i, label %preload486.i, label %postload487.i

preload486.i:                                     ; preds = %postload484.i
  %masked_load291.i = load double addrspace(1)* %46, align 8
  br label %postload487.i

postload487.i:                                    ; preds = %preload486.i, %postload484.i
  %phi488.i = phi double [ undef, %postload484.i ], [ %masked_load291.i, %preload486.i ]
  br i1 %extract88.i, label %preload552.i, label %postload553.i

preload552.i:                                     ; preds = %postload487.i
  %masked_load292.i = load double addrspace(1)* %47, align 8
  br label %postload553.i

postload553.i:                                    ; preds = %preload552.i, %postload487.i
  %phi554.i = phi double [ undef, %postload487.i ], [ %masked_load292.i, %preload552.i ]
  br i1 %extract89.i, label %preload555.i, label %postload556.i

preload555.i:                                     ; preds = %postload553.i
  %masked_load293.i = load double addrspace(1)* %48, align 8
  br label %postload556.i

postload556.i:                                    ; preds = %preload555.i, %postload553.i
  %phi557.i = phi double [ undef, %postload553.i ], [ %masked_load293.i, %preload555.i ]
  br i1 %extract90.i, label %preload558.i, label %postload559.i

preload558.i:                                     ; preds = %postload556.i
  %masked_load294.i = load double addrspace(1)* %49, align 8
  br label %postload559.i

postload559.i:                                    ; preds = %preload558.i, %postload556.i
  %phi560.i = phi double [ undef, %postload556.i ], [ %masked_load294.i, %preload558.i ]
  %temp.vect.i = insertelement <16 x double> undef, double %phi679.i, i32 0
  %temp.vect91.i = insertelement <16 x double> %temp.vect.i, double %phi476.i, i32 1
  %temp.vect92.i = insertelement <16 x double> %temp.vect91.i, double %phi563.i, i32 2
  %temp.vect93.i = insertelement <16 x double> %temp.vect92.i, double %phi566.i, i32 3
  %temp.vect94.i = insertelement <16 x double> %temp.vect93.i, double %phi569.i, i32 4
  %temp.vect95.i = insertelement <16 x double> %temp.vect94.i, double %phi542.i, i32 5
  %temp.vect96.i = insertelement <16 x double> %temp.vect95.i, double %phi545.i, i32 6
  %temp.vect97.i = insertelement <16 x double> %temp.vect96.i, double %phi548.i, i32 7
  %temp.vect98.i = insertelement <16 x double> %temp.vect97.i, double %phi551.i, i32 8
  %temp.vect99.i = insertelement <16 x double> %temp.vect98.i, double %phi479.i, i32 9
  %temp.vect100.i = insertelement <16 x double> %temp.vect99.i, double %phi482.i, i32 10
  %temp.vect101.i = insertelement <16 x double> %temp.vect100.i, double %phi485.i, i32 11
  %temp.vect102.i = insertelement <16 x double> %temp.vect101.i, double %phi488.i, i32 12
  %temp.vect103.i = insertelement <16 x double> %temp.vect102.i, double %phi554.i, i32 13
  %temp.vect104.i = insertelement <16 x double> %temp.vect103.i, double %phi557.i, i32 14
  %temp.vect105.i = insertelement <16 x double> %temp.vect104.i, double %phi560.i, i32 15
  %add15106.i = fadd <16 x double> %vectorPHI57.i, %temp.vect105.i
  %out_sel107.i = select <16 x i1> %vectorPHI56.i, <16 x double> %add15106.i, <16 x double> %vectorPHI55.i
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

while.end.i:                                      ; preds = %postload559.i, %SyncBB968.i
  %vectorPHI152.i = phi <16 x double> [ undef, %SyncBB968.i ], [ %out_sel107.i, %postload559.i ]
  %merge153.i = select <16 x i1> %cmp134.i, <16 x double> %vectorPHI152.i, <16 x double> zeroinitializer
  %52 = extractelement <16 x i32> %conv1146.i, i32 0
  %extract155.i = sext i32 %52 to i64
  %"&(pSB[currWI].offset)7597.i" = or i64 %CurrSBIndex..0.i, 64
  %"&pSB[currWI].offset760.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)7597.i"
  %CastToValueType761.i = bitcast i8* %"&pSB[currWI].offset760.i" to i64*
  store i64 %extract155.i, i64* %CastToValueType761.i, align 8
  %53 = getelementptr inbounds double addrspace(3)* %10, i64 %extract155.i
  %"&(pSB[currWI].offset)9138.i" = or i64 %CurrSBIndex..0.i, 72
  %"&pSB[currWI].offset914.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)9138.i"
  %CastToValueType915.i = bitcast i8* %"&pSB[currWI].offset914.i" to double addrspace(3)**
  store double addrspace(3)* %53, double addrspace(3)** %CastToValueType915.i, align 8
  %ptrTypeCast.i = bitcast double addrspace(3)* %53 to <16 x double> addrspace(3)*
  store <16 x double> %merge153.i, <16 x double> addrspace(3)* %ptrTypeCast.i, align 8
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
  %arrayidx40.i = getelementptr inbounds double addrspace(1)* %4, i64 %28
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB978.i, %thenBB971.i, %elseBB.i
  %currBarrier.0.i = phi i32 [ 0, %elseBB.i ], [ %currBarrier.4.i, %thenBB978.i ], [ %currBarrier.1.i, %thenBB971.i ]
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride984.i", %thenBB978.i ], [ %"loadedCurrSB+Stride977.i", %thenBB971.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++982.i", %thenBB978.i ], [ %"CurrWI++975.i", %thenBB971.i ]
  br i1 %Mneg3.i, label %for.body.i, label %for.end.i

for.body.i:                                       ; preds = %postload464.i, %SyncBB.i
  %currBarrier.1.i = phi i32 [ %currBarrier.3.i, %postload464.i ], [ %currBarrier.0.i, %SyncBB.i ]
  %CurrSBIndex..2.i = phi i64 [ %CurrSBIndex..4.i, %postload464.i ], [ %CurrSBIndex..1.i, %SyncBB.i ]
  %CurrWI..2.i = phi i64 [ %CurrWI..4.i, %postload464.i ], [ %CurrWI..1.i, %SyncBB.i ]
  %vectorPHI172.i = phi <16 x i1> [ %loop_mask12258.i, %postload464.i ], [ %vector174.i, %SyncBB.i ]
  %vectorPHI175.i = phi <16 x i1> [ %local_edge17262.i, %postload464.i ], [ %vector177.i, %SyncBB.i ]
  %s.03.i = phi i32 [ %shr.i, %postload464.i ], [ %conv24.i, %SyncBB.i ]
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
  %54 = getelementptr inbounds double addrspace(3)* %10, i64 %extract186.i
  %55 = getelementptr inbounds double addrspace(3)* %10, i64 %extract187.i
  %56 = getelementptr inbounds double addrspace(3)* %10, i64 %extract188.i
  %57 = getelementptr inbounds double addrspace(3)* %10, i64 %extract189.i
  %58 = getelementptr inbounds double addrspace(3)* %10, i64 %extract190.i
  %59 = getelementptr inbounds double addrspace(3)* %10, i64 %extract191.i
  %60 = getelementptr inbounds double addrspace(3)* %10, i64 %extract192.i
  %61 = getelementptr inbounds double addrspace(3)* %10, i64 %extract193.i
  %62 = getelementptr inbounds double addrspace(3)* %10, i64 %extract194.i
  %63 = getelementptr inbounds double addrspace(3)* %10, i64 %extract195.i
  %64 = getelementptr inbounds double addrspace(3)* %10, i64 %extract196.i
  %65 = getelementptr inbounds double addrspace(3)* %10, i64 %extract197.i
  %66 = getelementptr inbounds double addrspace(3)* %10, i64 %extract198.i
  %67 = getelementptr inbounds double addrspace(3)* %10, i64 %extract199.i
  %68 = getelementptr inbounds double addrspace(3)* %10, i64 %extract200.i
  br i1 %extract201.i, label %preload516.i, label %postload517.i

preload516.i:                                     ; preds = %for.body.i
  %extract185.i = extractelement <16 x i64> %idxprom30184.i, i32 0
  %69 = getelementptr inbounds double addrspace(3)* %10, i64 %extract185.i
  %masked_load295.i = load double addrspace(3)* %69, align 8
  br label %postload517.i

postload517.i:                                    ; preds = %preload516.i, %for.body.i
  %phi518.i = phi double [ undef, %for.body.i ], [ %masked_load295.i, %preload516.i ]
  br i1 %extract202.i, label %preload519.i, label %postload520.i

preload519.i:                                     ; preds = %postload517.i
  %masked_load296.i = load double addrspace(3)* %54, align 8
  br label %postload520.i

postload520.i:                                    ; preds = %preload519.i, %postload517.i
  %phi521.i = phi double [ undef, %postload517.i ], [ %masked_load296.i, %preload519.i ]
  br i1 %extract203.i, label %preload522.i, label %postload523.i

preload522.i:                                     ; preds = %postload520.i
  %masked_load297.i = load double addrspace(3)* %55, align 8
  br label %postload523.i

postload523.i:                                    ; preds = %preload522.i, %postload520.i
  %phi524.i = phi double [ undef, %postload520.i ], [ %masked_load297.i, %preload522.i ]
  br i1 %extract204.i, label %preload525.i, label %postload526.i

preload525.i:                                     ; preds = %postload523.i
  %masked_load298.i = load double addrspace(3)* %56, align 8
  br label %postload526.i

postload526.i:                                    ; preds = %preload525.i, %postload523.i
  %phi527.i = phi double [ undef, %postload523.i ], [ %masked_load298.i, %preload525.i ]
  br i1 %extract205.i, label %preload528.i, label %postload529.i

preload528.i:                                     ; preds = %postload526.i
  %masked_load299.i = load double addrspace(3)* %57, align 8
  br label %postload529.i

postload529.i:                                    ; preds = %preload528.i, %postload526.i
  %phi530.i = phi double [ undef, %postload526.i ], [ %masked_load299.i, %preload528.i ]
  br i1 %extract206.i, label %preload531.i, label %postload532.i

preload531.i:                                     ; preds = %postload529.i
  %masked_load300.i = load double addrspace(3)* %58, align 8
  br label %postload532.i

postload532.i:                                    ; preds = %preload531.i, %postload529.i
  %phi533.i = phi double [ undef, %postload529.i ], [ %masked_load300.i, %preload531.i ]
  br i1 %extract207.i, label %preload.i, label %postload.i

preload.i:                                        ; preds = %postload532.i
  %masked_load301.i = load double addrspace(3)* %59, align 8
  br label %postload.i

postload.i:                                       ; preds = %preload.i, %postload532.i
  %phi.i = phi double [ undef, %postload532.i ], [ %masked_load301.i, %preload.i ]
  br i1 %extract208.i, label %preload436.i, label %postload437.i

preload436.i:                                     ; preds = %postload.i
  %masked_load302.i = load double addrspace(3)* %60, align 8
  br label %postload437.i

postload437.i:                                    ; preds = %preload436.i, %postload.i
  %phi438.i = phi double [ undef, %postload.i ], [ %masked_load302.i, %preload436.i ]
  br i1 %extract209.i, label %preload439.i, label %postload440.i

preload439.i:                                     ; preds = %postload437.i
  %masked_load303.i = load double addrspace(3)* %61, align 8
  br label %postload440.i

postload440.i:                                    ; preds = %preload439.i, %postload437.i
  %phi441.i = phi double [ undef, %postload437.i ], [ %masked_load303.i, %preload439.i ]
  br i1 %extract210.i, label %preload442.i, label %postload443.i

preload442.i:                                     ; preds = %postload440.i
  %masked_load304.i = load double addrspace(3)* %62, align 8
  br label %postload443.i

postload443.i:                                    ; preds = %preload442.i, %postload440.i
  %phi444.i = phi double [ undef, %postload440.i ], [ %masked_load304.i, %preload442.i ]
  br i1 %extract211.i, label %preload445.i, label %postload446.i

preload445.i:                                     ; preds = %postload443.i
  %masked_load305.i = load double addrspace(3)* %63, align 8
  br label %postload446.i

postload446.i:                                    ; preds = %preload445.i, %postload443.i
  %phi447.i = phi double [ undef, %postload443.i ], [ %masked_load305.i, %preload445.i ]
  br i1 %extract212.i, label %preload448.i, label %postload449.i

preload448.i:                                     ; preds = %postload446.i
  %masked_load306.i = load double addrspace(3)* %64, align 8
  br label %postload449.i

postload449.i:                                    ; preds = %preload448.i, %postload446.i
  %phi450.i = phi double [ undef, %postload446.i ], [ %masked_load306.i, %preload448.i ]
  br i1 %extract213.i, label %preload451.i, label %postload452.i

preload451.i:                                     ; preds = %postload449.i
  %masked_load307.i = load double addrspace(3)* %65, align 8
  br label %postload452.i

postload452.i:                                    ; preds = %preload451.i, %postload449.i
  %phi453.i = phi double [ undef, %postload449.i ], [ %masked_load307.i, %preload451.i ]
  br i1 %extract214.i, label %preload454.i, label %postload455.i

preload454.i:                                     ; preds = %postload452.i
  %masked_load308.i = load double addrspace(3)* %66, align 8
  br label %postload455.i

postload455.i:                                    ; preds = %preload454.i, %postload452.i
  %phi456.i = phi double [ undef, %postload452.i ], [ %masked_load308.i, %preload454.i ]
  br i1 %extract215.i, label %preload457.i, label %postload458.i

preload457.i:                                     ; preds = %postload455.i
  %masked_load309.i = load double addrspace(3)* %67, align 8
  br label %postload458.i

postload458.i:                                    ; preds = %preload457.i, %postload455.i
  %phi459.i = phi double [ undef, %postload455.i ], [ %masked_load309.i, %preload457.i ]
  br i1 %extract216.i, label %preload460.i, label %postload461.i

preload460.i:                                     ; preds = %postload458.i
  %masked_load310.i = load double addrspace(3)* %68, align 8
  br label %postload461.i

postload461.i:                                    ; preds = %preload460.i, %postload458.i
  %phi462.i = phi double [ undef, %postload458.i ], [ %masked_load310.i, %preload460.i ]
  %temp.vect218.i = insertelement <16 x double> undef, double %phi518.i, i32 0
  %temp.vect219.i = insertelement <16 x double> %temp.vect218.i, double %phi521.i, i32 1
  %temp.vect220.i = insertelement <16 x double> %temp.vect219.i, double %phi524.i, i32 2
  %temp.vect221.i = insertelement <16 x double> %temp.vect220.i, double %phi527.i, i32 3
  %temp.vect222.i = insertelement <16 x double> %temp.vect221.i, double %phi530.i, i32 4
  %temp.vect223.i = insertelement <16 x double> %temp.vect222.i, double %phi533.i, i32 5
  %temp.vect224.i = insertelement <16 x double> %temp.vect223.i, double %phi.i, i32 6
  %temp.vect225.i = insertelement <16 x double> %temp.vect224.i, double %phi438.i, i32 7
  %temp.vect226.i = insertelement <16 x double> %temp.vect225.i, double %phi441.i, i32 8
  %temp.vect227.i = insertelement <16 x double> %temp.vect226.i, double %phi444.i, i32 9
  %temp.vect228.i = insertelement <16 x double> %temp.vect227.i, double %phi447.i, i32 10
  %temp.vect229.i = insertelement <16 x double> %temp.vect228.i, double %phi450.i, i32 11
  %temp.vect230.i = insertelement <16 x double> %temp.vect229.i, double %phi453.i, i32 12
  %temp.vect231.i = insertelement <16 x double> %temp.vect230.i, double %phi456.i, i32 13
  %temp.vect232.i = insertelement <16 x double> %temp.vect231.i, double %phi459.i, i32 14
  %temp.vect233.i = insertelement <16 x double> %temp.vect232.i, double %phi462.i, i32 15
  br i1 %extract201.i, label %preload537.i, label %postload538.i

preload537.i:                                     ; preds = %postload461.i
  %"&(pSB[currWI].offset)917.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset918.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)917.i"
  %CastToValueType919.i = bitcast i8* %"&pSB[currWI].offset918.i" to double addrspace(3)**
  %loadedValue920.i = load double addrspace(3)** %CastToValueType919.i, align 8
  %vload312.i = load double addrspace(3)* %loadedValue920.i, align 8
  br label %postload538.i

postload538.i:                                    ; preds = %preload537.i, %postload461.i
  %phi539.i = phi double [ undef, %postload461.i ], [ %vload312.i, %preload537.i ]
  %vpack.i = insertelement <16 x double> undef, double %phi539.i, i32 0
  br i1 %extract202.i, label %preload534.i, label %postload535.i

preload534.i:                                     ; preds = %postload538.i
  %"&(pSB[currWI].offset)763.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset764.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)763.i"
  %CastToValueType765.i = bitcast i8* %"&pSB[currWI].offset764.i" to i64*
  %loadedValue766.i = load i64* %CastToValueType765.i, align 8
  %.sum738.i = add i64 %loadedValue766.i, 1
  %70 = getelementptr double addrspace(3)* %10, i64 %.sum738.i
  %vload315.i = load double addrspace(3)* %70, align 8
  br label %postload535.i

postload535.i:                                    ; preds = %preload534.i, %postload538.i
  %phi536.i = phi double [ undef, %postload538.i ], [ %vload315.i, %preload534.i ]
  %vpack316.i = insertelement <16 x double> %vpack.i, double %phi536.i, i32 1
  br i1 %extract203.i, label %preload680.i, label %postload681.i

preload680.i:                                     ; preds = %postload535.i
  %"&(pSB[currWI].offset)768.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset769.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)768.i"
  %CastToValueType770.i = bitcast i8* %"&pSB[currWI].offset769.i" to i64*
  %loadedValue771.i = load i64* %CastToValueType770.i, align 8
  %.sum737.i = add i64 %loadedValue771.i, 2
  %71 = getelementptr double addrspace(3)* %10, i64 %.sum737.i
  %vload319.i = load double addrspace(3)* %71, align 8
  br label %postload681.i

postload681.i:                                    ; preds = %preload680.i, %postload535.i
  %phi682.i = phi double [ undef, %postload535.i ], [ %vload319.i, %preload680.i ]
  %vpack320.i = insertelement <16 x double> %vpack316.i, double %phi682.i, i32 2
  br i1 %extract204.i, label %preload683.i, label %postload684.i

preload683.i:                                     ; preds = %postload681.i
  %"&(pSB[currWI].offset)773.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset774.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)773.i"
  %CastToValueType775.i = bitcast i8* %"&pSB[currWI].offset774.i" to i64*
  %loadedValue776.i = load i64* %CastToValueType775.i, align 8
  %.sum736.i = add i64 %loadedValue776.i, 3
  %72 = getelementptr double addrspace(3)* %10, i64 %.sum736.i
  %vload323.i = load double addrspace(3)* %72, align 8
  br label %postload684.i

postload684.i:                                    ; preds = %preload683.i, %postload681.i
  %phi685.i = phi double [ undef, %postload681.i ], [ %vload323.i, %preload683.i ]
  %vpack324.i = insertelement <16 x double> %vpack320.i, double %phi685.i, i32 3
  br i1 %extract205.i, label %preload686.i, label %postload687.i

preload686.i:                                     ; preds = %postload684.i
  %"&(pSB[currWI].offset)778.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset779.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)778.i"
  %CastToValueType780.i = bitcast i8* %"&pSB[currWI].offset779.i" to i64*
  %loadedValue781.i = load i64* %CastToValueType780.i, align 8
  %.sum735.i = add i64 %loadedValue781.i, 4
  %73 = getelementptr double addrspace(3)* %10, i64 %.sum735.i
  %vload327.i = load double addrspace(3)* %73, align 8
  br label %postload687.i

postload687.i:                                    ; preds = %preload686.i, %postload684.i
  %phi688.i = phi double [ undef, %postload684.i ], [ %vload327.i, %preload686.i ]
  %vpack328.i = insertelement <16 x double> %vpack324.i, double %phi688.i, i32 4
  br i1 %extract206.i, label %preload501.i, label %postload502.i

preload501.i:                                     ; preds = %postload687.i
  %"&(pSB[currWI].offset)783.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset784.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)783.i"
  %CastToValueType785.i = bitcast i8* %"&pSB[currWI].offset784.i" to i64*
  %loadedValue786.i = load i64* %CastToValueType785.i, align 8
  %.sum734.i = add i64 %loadedValue786.i, 5
  %74 = getelementptr double addrspace(3)* %10, i64 %.sum734.i
  %vload331.i = load double addrspace(3)* %74, align 8
  br label %postload502.i

postload502.i:                                    ; preds = %preload501.i, %postload687.i
  %phi503.i = phi double [ undef, %postload687.i ], [ %vload331.i, %preload501.i ]
  %vpack332.i = insertelement <16 x double> %vpack328.i, double %phi503.i, i32 5
  br i1 %extract207.i, label %preload504.i, label %postload505.i

preload504.i:                                     ; preds = %postload502.i
  %"&(pSB[currWI].offset)788.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset789.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)788.i"
  %CastToValueType790.i = bitcast i8* %"&pSB[currWI].offset789.i" to i64*
  %loadedValue791.i = load i64* %CastToValueType790.i, align 8
  %.sum733.i = add i64 %loadedValue791.i, 6
  %75 = getelementptr double addrspace(3)* %10, i64 %.sum733.i
  %vload335.i = load double addrspace(3)* %75, align 8
  br label %postload505.i

postload505.i:                                    ; preds = %preload504.i, %postload502.i
  %phi506.i = phi double [ undef, %postload502.i ], [ %vload335.i, %preload504.i ]
  %vpack336.i = insertelement <16 x double> %vpack332.i, double %phi506.i, i32 6
  br i1 %extract208.i, label %preload689.i, label %postload690.i

preload689.i:                                     ; preds = %postload505.i
  %"&(pSB[currWI].offset)793.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset794.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)793.i"
  %CastToValueType795.i = bitcast i8* %"&pSB[currWI].offset794.i" to i64*
  %loadedValue796.i = load i64* %CastToValueType795.i, align 8
  %.sum732.i = add i64 %loadedValue796.i, 7
  %76 = getelementptr double addrspace(3)* %10, i64 %.sum732.i
  %vload339.i = load double addrspace(3)* %76, align 8
  br label %postload690.i

postload690.i:                                    ; preds = %preload689.i, %postload505.i
  %phi691.i = phi double [ undef, %postload505.i ], [ %vload339.i, %preload689.i ]
  %vpack340.i = insertelement <16 x double> %vpack336.i, double %phi691.i, i32 7
  br i1 %extract209.i, label %preload570.i, label %postload571.i

preload570.i:                                     ; preds = %postload690.i
  %"&(pSB[currWI].offset)798.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset799.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)798.i"
  %CastToValueType800.i = bitcast i8* %"&pSB[currWI].offset799.i" to i64*
  %loadedValue801.i = load i64* %CastToValueType800.i, align 8
  %.sum731.i = add i64 %loadedValue801.i, 8
  %77 = getelementptr double addrspace(3)* %10, i64 %.sum731.i
  %vload343.i = load double addrspace(3)* %77, align 8
  br label %postload571.i

postload571.i:                                    ; preds = %preload570.i, %postload690.i
  %phi572.i = phi double [ undef, %postload690.i ], [ %vload343.i, %preload570.i ]
  %vpack344.i = insertelement <16 x double> %vpack340.i, double %phi572.i, i32 8
  br i1 %extract210.i, label %preload573.i, label %postload574.i

preload573.i:                                     ; preds = %postload571.i
  %"&(pSB[currWI].offset)803.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset804.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)803.i"
  %CastToValueType805.i = bitcast i8* %"&pSB[currWI].offset804.i" to i64*
  %loadedValue806.i = load i64* %CastToValueType805.i, align 8
  %.sum730.i = add i64 %loadedValue806.i, 9
  %78 = getelementptr double addrspace(3)* %10, i64 %.sum730.i
  %vload347.i = load double addrspace(3)* %78, align 8
  br label %postload574.i

postload574.i:                                    ; preds = %preload573.i, %postload571.i
  %phi575.i = phi double [ undef, %postload571.i ], [ %vload347.i, %preload573.i ]
  %vpack348.i = insertelement <16 x double> %vpack344.i, double %phi575.i, i32 9
  br i1 %extract211.i, label %preload507.i, label %postload508.i

preload507.i:                                     ; preds = %postload574.i
  %"&(pSB[currWI].offset)808.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset809.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)808.i"
  %CastToValueType810.i = bitcast i8* %"&pSB[currWI].offset809.i" to i64*
  %loadedValue811.i = load i64* %CastToValueType810.i, align 8
  %.sum729.i = add i64 %loadedValue811.i, 10
  %79 = getelementptr double addrspace(3)* %10, i64 %.sum729.i
  %vload351.i = load double addrspace(3)* %79, align 8
  br label %postload508.i

postload508.i:                                    ; preds = %preload507.i, %postload574.i
  %phi509.i = phi double [ undef, %postload574.i ], [ %vload351.i, %preload507.i ]
  %vpack352.i = insertelement <16 x double> %vpack348.i, double %phi509.i, i32 10
  br i1 %extract212.i, label %preload510.i, label %postload511.i

preload510.i:                                     ; preds = %postload508.i
  %"&(pSB[currWI].offset)813.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset814.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)813.i"
  %CastToValueType815.i = bitcast i8* %"&pSB[currWI].offset814.i" to i64*
  %loadedValue816.i = load i64* %CastToValueType815.i, align 8
  %.sum728.i = add i64 %loadedValue816.i, 11
  %80 = getelementptr double addrspace(3)* %10, i64 %.sum728.i
  %vload355.i = load double addrspace(3)* %80, align 8
  br label %postload511.i

postload511.i:                                    ; preds = %preload510.i, %postload508.i
  %phi512.i = phi double [ undef, %postload508.i ], [ %vload355.i, %preload510.i ]
  %vpack356.i = insertelement <16 x double> %vpack352.i, double %phi512.i, i32 11
  br i1 %extract213.i, label %preload513.i, label %postload514.i

preload513.i:                                     ; preds = %postload511.i
  %"&(pSB[currWI].offset)818.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset819.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)818.i"
  %CastToValueType820.i = bitcast i8* %"&pSB[currWI].offset819.i" to i64*
  %loadedValue821.i = load i64* %CastToValueType820.i, align 8
  %.sum727.i = add i64 %loadedValue821.i, 12
  %81 = getelementptr double addrspace(3)* %10, i64 %.sum727.i
  %vload359.i = load double addrspace(3)* %81, align 8
  br label %postload514.i

postload514.i:                                    ; preds = %preload513.i, %postload511.i
  %phi515.i = phi double [ undef, %postload511.i ], [ %vload359.i, %preload513.i ]
  %vpack360.i = insertelement <16 x double> %vpack356.i, double %phi515.i, i32 12
  br i1 %extract214.i, label %preload668.i, label %postload669.i

preload668.i:                                     ; preds = %postload514.i
  %"&(pSB[currWI].offset)823.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset824.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)823.i"
  %CastToValueType825.i = bitcast i8* %"&pSB[currWI].offset824.i" to i64*
  %loadedValue826.i = load i64* %CastToValueType825.i, align 8
  %.sum726.i = add i64 %loadedValue826.i, 13
  %82 = getelementptr double addrspace(3)* %10, i64 %.sum726.i
  %vload363.i = load double addrspace(3)* %82, align 8
  br label %postload669.i

postload669.i:                                    ; preds = %preload668.i, %postload514.i
  %phi670.i = phi double [ undef, %postload514.i ], [ %vload363.i, %preload668.i ]
  %vpack364.i = insertelement <16 x double> %vpack360.i, double %phi670.i, i32 13
  br i1 %extract215.i, label %preload671.i, label %postload672.i

preload671.i:                                     ; preds = %postload669.i
  %"&(pSB[currWI].offset)828.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset829.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)828.i"
  %CastToValueType830.i = bitcast i8* %"&pSB[currWI].offset829.i" to i64*
  %loadedValue831.i = load i64* %CastToValueType830.i, align 8
  %.sum725.i = add i64 %loadedValue831.i, 14
  %83 = getelementptr double addrspace(3)* %10, i64 %.sum725.i
  %vload367.i = load double addrspace(3)* %83, align 8
  br label %postload672.i

postload672.i:                                    ; preds = %preload671.i, %postload669.i
  %phi673.i = phi double [ undef, %postload669.i ], [ %vload367.i, %preload671.i ]
  %vpack368.i = insertelement <16 x double> %vpack364.i, double %phi673.i, i32 14
  br i1 %extract216.i, label %preload674.i, label %postload675.i

preload674.i:                                     ; preds = %postload672.i
  %"&(pSB[currWI].offset)833.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset834.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)833.i"
  %CastToValueType835.i = bitcast i8* %"&pSB[currWI].offset834.i" to i64*
  %loadedValue836.i = load i64* %CastToValueType835.i, align 8
  %.sum724.i = add i64 %loadedValue836.i, 15
  %84 = getelementptr double addrspace(3)* %10, i64 %.sum724.i
  %vload371.i = load double addrspace(3)* %84, align 8
  br label %postload675.i

postload675.i:                                    ; preds = %preload674.i, %postload672.i
  %phi676.i = phi double [ undef, %postload672.i ], [ %vload371.i, %preload674.i ]
  %vpack372.i = insertelement <16 x double> %vpack368.i, double %phi676.i, i32 15
  %add34234.i = fadd <16 x double> %vpack372.i, %temp.vect233.i
  br i1 %extract201.i, label %preload465.i, label %postload466.i

preload465.i:                                     ; preds = %postload675.i
  %exData.i = extractelement <16 x double> %add34234.i, i32 0
  %"&(pSB[currWI].offset)922.i" = add nuw i64 %CurrSBIndex..2.i, 72
  %"&pSB[currWI].offset923.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)922.i"
  %CastToValueType924.i = bitcast i8* %"&pSB[currWI].offset923.i" to double addrspace(3)**
  %loadedValue925.i = load double addrspace(3)** %CastToValueType924.i, align 8
  store double %exData.i, double addrspace(3)* %loadedValue925.i, align 8
  br label %postload466.i

postload466.i:                                    ; preds = %preload465.i, %postload675.i
  br i1 %extract202.i, label %preload468.i, label %postload469.i

preload468.i:                                     ; preds = %postload466.i
  %"&(pSB[currWI].offset)838.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset839.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)838.i"
  %CastToValueType840.i = bitcast i8* %"&pSB[currWI].offset839.i" to i64*
  %loadedValue841.i = load i64* %CastToValueType840.i, align 8
  %.sum723.i = add i64 %loadedValue841.i, 1
  %85 = getelementptr double addrspace(3)* %10, i64 %.sum723.i
  %exData377.i = extractelement <16 x double> %add34234.i, i32 1
  store double %exData377.i, double addrspace(3)* %85, align 8
  br label %postload469.i

postload469.i:                                    ; preds = %preload468.i, %postload466.i
  br i1 %extract203.i, label %preload471.i, label %postload472.i

preload471.i:                                     ; preds = %postload469.i
  %"&(pSB[currWI].offset)843.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset844.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)843.i"
  %CastToValueType845.i = bitcast i8* %"&pSB[currWI].offset844.i" to i64*
  %loadedValue846.i = load i64* %CastToValueType845.i, align 8
  %.sum722.i = add i64 %loadedValue846.i, 2
  %86 = getelementptr double addrspace(3)* %10, i64 %.sum722.i
  %exData380.i = extractelement <16 x double> %add34234.i, i32 2
  store double %exData380.i, double addrspace(3)* %86, align 8
  br label %postload472.i

postload472.i:                                    ; preds = %preload471.i, %postload469.i
  br i1 %extract204.i, label %preload576.i, label %postload577.i

preload576.i:                                     ; preds = %postload472.i
  %"&(pSB[currWI].offset)848.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset849.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)848.i"
  %CastToValueType850.i = bitcast i8* %"&pSB[currWI].offset849.i" to i64*
  %loadedValue851.i = load i64* %CastToValueType850.i, align 8
  %.sum721.i = add i64 %loadedValue851.i, 3
  %87 = getelementptr double addrspace(3)* %10, i64 %.sum721.i
  %exData383.i = extractelement <16 x double> %add34234.i, i32 3
  store double %exData383.i, double addrspace(3)* %87, align 8
  br label %postload577.i

postload577.i:                                    ; preds = %preload576.i, %postload472.i
  br i1 %extract205.i, label %preload579.i, label %postload580.i

preload579.i:                                     ; preds = %postload577.i
  %"&(pSB[currWI].offset)853.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset854.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)853.i"
  %CastToValueType855.i = bitcast i8* %"&pSB[currWI].offset854.i" to i64*
  %loadedValue856.i = load i64* %CastToValueType855.i, align 8
  %.sum720.i = add i64 %loadedValue856.i, 4
  %88 = getelementptr double addrspace(3)* %10, i64 %.sum720.i
  %exData386.i = extractelement <16 x double> %add34234.i, i32 4
  store double %exData386.i, double addrspace(3)* %88, align 8
  br label %postload580.i

postload580.i:                                    ; preds = %preload579.i, %postload577.i
  br i1 %extract206.i, label %preload582.i, label %postload583.i

preload582.i:                                     ; preds = %postload580.i
  %"&(pSB[currWI].offset)858.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset859.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)858.i"
  %CastToValueType860.i = bitcast i8* %"&pSB[currWI].offset859.i" to i64*
  %loadedValue861.i = load i64* %CastToValueType860.i, align 8
  %.sum719.i = add i64 %loadedValue861.i, 5
  %89 = getelementptr double addrspace(3)* %10, i64 %.sum719.i
  %exData389.i = extractelement <16 x double> %add34234.i, i32 5
  store double %exData389.i, double addrspace(3)* %89, align 8
  br label %postload583.i

postload583.i:                                    ; preds = %preload582.i, %postload580.i
  br i1 %extract207.i, label %preload585.i, label %postload586.i

preload585.i:                                     ; preds = %postload583.i
  %"&(pSB[currWI].offset)863.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset864.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)863.i"
  %CastToValueType865.i = bitcast i8* %"&pSB[currWI].offset864.i" to i64*
  %loadedValue866.i = load i64* %CastToValueType865.i, align 8
  %.sum718.i = add i64 %loadedValue866.i, 6
  %90 = getelementptr double addrspace(3)* %10, i64 %.sum718.i
  %exData392.i = extractelement <16 x double> %add34234.i, i32 6
  store double %exData392.i, double addrspace(3)* %90, align 8
  br label %postload586.i

postload586.i:                                    ; preds = %preload585.i, %postload583.i
  br i1 %extract208.i, label %preload692.i, label %postload693.i

preload692.i:                                     ; preds = %postload586.i
  %"&(pSB[currWI].offset)868.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset869.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)868.i"
  %CastToValueType870.i = bitcast i8* %"&pSB[currWI].offset869.i" to i64*
  %loadedValue871.i = load i64* %CastToValueType870.i, align 8
  %.sum717.i = add i64 %loadedValue871.i, 7
  %91 = getelementptr double addrspace(3)* %10, i64 %.sum717.i
  %exData395.i = extractelement <16 x double> %add34234.i, i32 7
  store double %exData395.i, double addrspace(3)* %91, align 8
  br label %postload693.i

postload693.i:                                    ; preds = %preload692.i, %postload586.i
  br i1 %extract209.i, label %preload695.i, label %postload696.i

preload695.i:                                     ; preds = %postload693.i
  %"&(pSB[currWI].offset)873.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset874.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)873.i"
  %CastToValueType875.i = bitcast i8* %"&pSB[currWI].offset874.i" to i64*
  %loadedValue876.i = load i64* %CastToValueType875.i, align 8
  %.sum716.i = add i64 %loadedValue876.i, 8
  %92 = getelementptr double addrspace(3)* %10, i64 %.sum716.i
  %exData398.i = extractelement <16 x double> %add34234.i, i32 8
  store double %exData398.i, double addrspace(3)* %92, align 8
  br label %postload696.i

postload696.i:                                    ; preds = %preload695.i, %postload693.i
  br i1 %extract210.i, label %preload698.i, label %postload699.i

preload698.i:                                     ; preds = %postload696.i
  %"&(pSB[currWI].offset)878.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset879.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)878.i"
  %CastToValueType880.i = bitcast i8* %"&pSB[currWI].offset879.i" to i64*
  %loadedValue881.i = load i64* %CastToValueType880.i, align 8
  %.sum715.i = add i64 %loadedValue881.i, 9
  %93 = getelementptr double addrspace(3)* %10, i64 %.sum715.i
  %exData401.i = extractelement <16 x double> %add34234.i, i32 9
  store double %exData401.i, double addrspace(3)* %93, align 8
  br label %postload699.i

postload699.i:                                    ; preds = %preload698.i, %postload696.i
  br i1 %extract211.i, label %preload701.i, label %postload702.i

preload701.i:                                     ; preds = %postload699.i
  %"&(pSB[currWI].offset)883.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset884.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)883.i"
  %CastToValueType885.i = bitcast i8* %"&pSB[currWI].offset884.i" to i64*
  %loadedValue886.i = load i64* %CastToValueType885.i, align 8
  %.sum714.i = add i64 %loadedValue886.i, 10
  %94 = getelementptr double addrspace(3)* %10, i64 %.sum714.i
  %exData404.i = extractelement <16 x double> %add34234.i, i32 10
  store double %exData404.i, double addrspace(3)* %94, align 8
  br label %postload702.i

postload702.i:                                    ; preds = %preload701.i, %postload699.i
  br i1 %extract212.i, label %preload704.i, label %postload705.i

preload704.i:                                     ; preds = %postload702.i
  %"&(pSB[currWI].offset)888.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset889.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)888.i"
  %CastToValueType890.i = bitcast i8* %"&pSB[currWI].offset889.i" to i64*
  %loadedValue891.i = load i64* %CastToValueType890.i, align 8
  %.sum713.i = add i64 %loadedValue891.i, 11
  %95 = getelementptr double addrspace(3)* %10, i64 %.sum713.i
  %exData407.i = extractelement <16 x double> %add34234.i, i32 11
  store double %exData407.i, double addrspace(3)* %95, align 8
  br label %postload705.i

postload705.i:                                    ; preds = %preload704.i, %postload702.i
  br i1 %extract213.i, label %preload489.i, label %postload490.i

preload489.i:                                     ; preds = %postload705.i
  %"&(pSB[currWI].offset)893.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset894.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)893.i"
  %CastToValueType895.i = bitcast i8* %"&pSB[currWI].offset894.i" to i64*
  %loadedValue896.i = load i64* %CastToValueType895.i, align 8
  %.sum712.i = add i64 %loadedValue896.i, 12
  %96 = getelementptr double addrspace(3)* %10, i64 %.sum712.i
  %exData410.i = extractelement <16 x double> %add34234.i, i32 12
  store double %exData410.i, double addrspace(3)* %96, align 8
  br label %postload490.i

postload490.i:                                    ; preds = %preload489.i, %postload705.i
  br i1 %extract214.i, label %preload492.i, label %postload493.i

preload492.i:                                     ; preds = %postload490.i
  %"&(pSB[currWI].offset)898.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset899.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)898.i"
  %CastToValueType900.i = bitcast i8* %"&pSB[currWI].offset899.i" to i64*
  %loadedValue901.i = load i64* %CastToValueType900.i, align 8
  %.sum711.i = add i64 %loadedValue901.i, 13
  %97 = getelementptr double addrspace(3)* %10, i64 %.sum711.i
  %exData413.i = extractelement <16 x double> %add34234.i, i32 13
  store double %exData413.i, double addrspace(3)* %97, align 8
  br label %postload493.i

postload493.i:                                    ; preds = %preload492.i, %postload490.i
  br i1 %extract215.i, label %preload495.i, label %postload496.i

preload495.i:                                     ; preds = %postload493.i
  %"&(pSB[currWI].offset)903.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset904.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)903.i"
  %CastToValueType905.i = bitcast i8* %"&pSB[currWI].offset904.i" to i64*
  %loadedValue906.i = load i64* %CastToValueType905.i, align 8
  %.sum710.i = add i64 %loadedValue906.i, 14
  %98 = getelementptr double addrspace(3)* %10, i64 %.sum710.i
  %exData416.i = extractelement <16 x double> %add34234.i, i32 14
  store double %exData416.i, double addrspace(3)* %98, align 8
  br label %postload496.i

postload496.i:                                    ; preds = %preload495.i, %postload493.i
  br i1 %extract216.i, label %preload498.i, label %postload496.i.if.end.i_crit_edge

postload496.i.if.end.i_crit_edge:                 ; preds = %postload496.i
  br label %if.end.i

preload498.i:                                     ; preds = %postload496.i
  %"&(pSB[currWI].offset)908.i" = add nuw i64 %CurrSBIndex..2.i, 64
  %"&pSB[currWI].offset909.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)908.i"
  %CastToValueType910.i = bitcast i8* %"&pSB[currWI].offset909.i" to i64*
  %loadedValue911.i = load i64* %CastToValueType910.i, align 8
  %.sum.i = add i64 %loadedValue911.i, 15
  %99 = getelementptr double addrspace(3)* %10, i64 %.sum.i
  %exData419.i = extractelement <16 x double> %add34234.i, i32 15
  store double %exData419.i, double addrspace(3)* %99, align 8
  br label %if.end.i

if.end.i:                                         ; preds = %postload496.i.if.end.i_crit_edge, %preload498.i
  %loadedValue943.i = load <16 x i1>* %CastToValueType938.i, align 16
  %extract238.i = extractelement <16 x i1> %loadedValue943.i, i32 0
  br i1 %extract238.i, label %preload463.i, label %if.end.i.postload464.i_crit_edge

if.end.i.postload464.i_crit_edge:                 ; preds = %if.end.i
  br label %postload464.i

preload463.i:                                     ; preds = %if.end.i
  %check.WI.iter974.i = icmp ult i64 %CurrWI..2.i, %22
  br i1 %check.WI.iter974.i, label %thenBB971.i, label %preload463.i.postload464.i_crit_edge

preload463.i.postload464.i_crit_edge:             ; preds = %preload463.i
  br label %postload464.i

thenBB971.i:                                      ; preds = %preload463.i
  %"CurrWI++975.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride977.i" = add nuw i64 %CurrSBIndex..2.i, 128
  %cond10.i = icmp eq i32 %currBarrier.1.i, 3
  br i1 %cond10.i, label %thenBB971.i.postload464.i_crit_edge, label %SyncBB.i

thenBB971.i.postload464.i_crit_edge:              ; preds = %thenBB971.i
  br label %postload464.i

postload464.i:                                    ; preds = %thenBB978.i.postload464.i_crit_edge, %thenBB971.i.postload464.i_crit_edge, %preload463.i.postload464.i_crit_edge, %if.end.i.postload464.i_crit_edge
  %currBarrier.3.i = phi i32 [ %currBarrier.1.i, %if.end.i.postload464.i_crit_edge ], [ 3, %preload463.i.postload464.i_crit_edge ], [ %currBarrier.1.i, %thenBB971.i.postload464.i_crit_edge ], [ %currBarrier.4.i, %thenBB978.i.postload464.i_crit_edge ]
  %CurrSBIndex..4.i = phi i64 [ %CurrSBIndex..2.i, %if.end.i.postload464.i_crit_edge ], [ 0, %preload463.i.postload464.i_crit_edge ], [ %"loadedCurrSB+Stride977.i", %thenBB971.i.postload464.i_crit_edge ], [ %"loadedCurrSB+Stride984.i", %thenBB978.i.postload464.i_crit_edge ]
  %CurrWI..4.i = phi i64 [ %CurrWI..2.i, %if.end.i.postload464.i_crit_edge ], [ 0, %preload463.i.postload464.i_crit_edge ], [ %"CurrWI++975.i", %thenBB971.i.postload464.i_crit_edge ], [ %"CurrWI++982.i", %thenBB978.i.postload464.i_crit_edge ]
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

for.end.i:                                        ; preds = %postload464.i, %SyncBB.i
  %currBarrier.4.i = phi i32 [ %currBarrier.0.i, %SyncBB.i ], [ %currBarrier.3.i, %postload464.i ]
  %CurrSBIndex..5.i = phi i64 [ %CurrSBIndex..1.i, %SyncBB.i ], [ %CurrSBIndex..4.i, %postload464.i ]
  %CurrWI..5.i = phi i64 [ %CurrWI..1.i, %SyncBB.i ], [ %CurrWI..4.i, %postload464.i ]
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
  br i1 %extract264.i, label %preload588.i, label %postload589.i

preload588.i:                                     ; preds = %for.end.i
  %masked_load420.i = load double addrspace(3)* %10, align 8
  br label %postload589.i

postload589.i:                                    ; preds = %preload588.i, %for.end.i
  %phi590.i = phi double [ undef, %for.end.i ], [ %masked_load420.i, %preload588.i ]
  br i1 %extract265.i, label %preload593.i, label %postload594.i

preload593.i:                                     ; preds = %postload589.i
  %masked_load421.i = load double addrspace(3)* %10, align 8
  br label %postload594.i

postload594.i:                                    ; preds = %preload593.i, %postload589.i
  %phi595.i = phi double [ undef, %postload589.i ], [ %masked_load421.i, %preload593.i ]
  br i1 %extract266.i, label %preload598.i, label %postload599.i

preload598.i:                                     ; preds = %postload594.i
  %masked_load422.i = load double addrspace(3)* %10, align 8
  br label %postload599.i

postload599.i:                                    ; preds = %preload598.i, %postload594.i
  %phi600.i = phi double [ undef, %postload594.i ], [ %masked_load422.i, %preload598.i ]
  br i1 %extract267.i, label %preload603.i, label %postload604.i

preload603.i:                                     ; preds = %postload599.i
  %masked_load423.i = load double addrspace(3)* %10, align 8
  br label %postload604.i

postload604.i:                                    ; preds = %preload603.i, %postload599.i
  %phi605.i = phi double [ undef, %postload599.i ], [ %masked_load423.i, %preload603.i ]
  br i1 %extract268.i, label %preload608.i, label %postload609.i

preload608.i:                                     ; preds = %postload604.i
  %masked_load424.i = load double addrspace(3)* %10, align 8
  br label %postload609.i

postload609.i:                                    ; preds = %preload608.i, %postload604.i
  %phi610.i = phi double [ undef, %postload604.i ], [ %masked_load424.i, %preload608.i ]
  br i1 %extract269.i, label %preload613.i, label %postload614.i

preload613.i:                                     ; preds = %postload609.i
  %masked_load425.i = load double addrspace(3)* %10, align 8
  br label %postload614.i

postload614.i:                                    ; preds = %preload613.i, %postload609.i
  %phi615.i = phi double [ undef, %postload609.i ], [ %masked_load425.i, %preload613.i ]
  br i1 %extract270.i, label %preload618.i, label %postload619.i

preload618.i:                                     ; preds = %postload614.i
  %masked_load426.i = load double addrspace(3)* %10, align 8
  br label %postload619.i

postload619.i:                                    ; preds = %preload618.i, %postload614.i
  %phi620.i = phi double [ undef, %postload614.i ], [ %masked_load426.i, %preload618.i ]
  br i1 %extract271.i, label %preload623.i, label %postload624.i

preload623.i:                                     ; preds = %postload619.i
  %masked_load427.i = load double addrspace(3)* %10, align 8
  br label %postload624.i

postload624.i:                                    ; preds = %preload623.i, %postload619.i
  %phi625.i = phi double [ undef, %postload619.i ], [ %masked_load427.i, %preload623.i ]
  br i1 %extract272.i, label %preload628.i, label %postload629.i

preload628.i:                                     ; preds = %postload624.i
  %masked_load428.i = load double addrspace(3)* %10, align 8
  br label %postload629.i

postload629.i:                                    ; preds = %preload628.i, %postload624.i
  %phi630.i = phi double [ undef, %postload624.i ], [ %masked_load428.i, %preload628.i ]
  br i1 %extract273.i, label %preload633.i, label %postload634.i

preload633.i:                                     ; preds = %postload629.i
  %masked_load429.i = load double addrspace(3)* %10, align 8
  br label %postload634.i

postload634.i:                                    ; preds = %preload633.i, %postload629.i
  %phi635.i = phi double [ undef, %postload629.i ], [ %masked_load429.i, %preload633.i ]
  br i1 %extract274.i, label %preload638.i, label %postload639.i

preload638.i:                                     ; preds = %postload634.i
  %masked_load430.i = load double addrspace(3)* %10, align 8
  br label %postload639.i

postload639.i:                                    ; preds = %preload638.i, %postload634.i
  %phi640.i = phi double [ undef, %postload634.i ], [ %masked_load430.i, %preload638.i ]
  br i1 %extract275.i, label %preload643.i, label %postload644.i

preload643.i:                                     ; preds = %postload639.i
  %masked_load431.i = load double addrspace(3)* %10, align 8
  br label %postload644.i

postload644.i:                                    ; preds = %preload643.i, %postload639.i
  %phi645.i = phi double [ undef, %postload639.i ], [ %masked_load431.i, %preload643.i ]
  br i1 %extract276.i, label %preload648.i, label %postload649.i

preload648.i:                                     ; preds = %postload644.i
  %masked_load432.i = load double addrspace(3)* %10, align 8
  br label %postload649.i

postload649.i:                                    ; preds = %preload648.i, %postload644.i
  %phi650.i = phi double [ undef, %postload644.i ], [ %masked_load432.i, %preload648.i ]
  br i1 %extract277.i, label %preload653.i, label %postload654.i

preload653.i:                                     ; preds = %postload649.i
  %masked_load433.i = load double addrspace(3)* %10, align 8
  br label %postload654.i

postload654.i:                                    ; preds = %preload653.i, %postload649.i
  %phi655.i = phi double [ undef, %postload649.i ], [ %masked_load433.i, %preload653.i ]
  br i1 %extract278.i, label %preload658.i, label %postload659.i

preload658.i:                                     ; preds = %postload654.i
  %masked_load434.i = load double addrspace(3)* %10, align 8
  br label %postload659.i

postload659.i:                                    ; preds = %preload658.i, %postload654.i
  %phi660.i = phi double [ undef, %postload654.i ], [ %masked_load434.i, %preload658.i ]
  br i1 %extract279.i, label %preload663.i, label %postload664.i

preload663.i:                                     ; preds = %postload659.i
  %masked_load435.i = load double addrspace(3)* %10, align 8
  br label %postload664.i

postload664.i:                                    ; preds = %preload663.i, %postload659.i
  %phi665.i = phi double [ undef, %postload659.i ], [ %masked_load435.i, %preload663.i ]
  br i1 %extract264.i, label %preload591.i, label %postload592.i

preload591.i:                                     ; preds = %postload664.i
  store double %phi590.i, double addrspace(1)* %arrayidx40.i, align 8
  br label %postload592.i

postload592.i:                                    ; preds = %preload591.i, %postload664.i
  br i1 %extract265.i, label %preload596.i, label %postload597.i

preload596.i:                                     ; preds = %postload592.i
  store double %phi595.i, double addrspace(1)* %arrayidx40.i, align 8
  br label %postload597.i

postload597.i:                                    ; preds = %preload596.i, %postload592.i
  br i1 %extract266.i, label %preload601.i, label %postload602.i

preload601.i:                                     ; preds = %postload597.i
  store double %phi600.i, double addrspace(1)* %arrayidx40.i, align 8
  br label %postload602.i

postload602.i:                                    ; preds = %preload601.i, %postload597.i
  br i1 %extract267.i, label %preload606.i, label %postload607.i

preload606.i:                                     ; preds = %postload602.i
  store double %phi605.i, double addrspace(1)* %arrayidx40.i, align 8
  br label %postload607.i

postload607.i:                                    ; preds = %preload606.i, %postload602.i
  br i1 %extract268.i, label %preload611.i, label %postload612.i

preload611.i:                                     ; preds = %postload607.i
  store double %phi610.i, double addrspace(1)* %arrayidx40.i, align 8
  br label %postload612.i

postload612.i:                                    ; preds = %preload611.i, %postload607.i
  br i1 %extract269.i, label %preload616.i, label %postload617.i

preload616.i:                                     ; preds = %postload612.i
  store double %phi615.i, double addrspace(1)* %arrayidx40.i, align 8
  br label %postload617.i

postload617.i:                                    ; preds = %preload616.i, %postload612.i
  br i1 %extract270.i, label %preload621.i, label %postload622.i

preload621.i:                                     ; preds = %postload617.i
  store double %phi620.i, double addrspace(1)* %arrayidx40.i, align 8
  br label %postload622.i

postload622.i:                                    ; preds = %preload621.i, %postload617.i
  br i1 %extract271.i, label %preload626.i, label %postload627.i

preload626.i:                                     ; preds = %postload622.i
  store double %phi625.i, double addrspace(1)* %arrayidx40.i, align 8
  br label %postload627.i

postload627.i:                                    ; preds = %preload626.i, %postload622.i
  br i1 %extract272.i, label %preload631.i, label %postload632.i

preload631.i:                                     ; preds = %postload627.i
  store double %phi630.i, double addrspace(1)* %arrayidx40.i, align 8
  br label %postload632.i

postload632.i:                                    ; preds = %preload631.i, %postload627.i
  br i1 %extract273.i, label %preload636.i, label %postload637.i

preload636.i:                                     ; preds = %postload632.i
  store double %phi635.i, double addrspace(1)* %arrayidx40.i, align 8
  br label %postload637.i

postload637.i:                                    ; preds = %preload636.i, %postload632.i
  br i1 %extract274.i, label %preload641.i, label %postload642.i

preload641.i:                                     ; preds = %postload637.i
  store double %phi640.i, double addrspace(1)* %arrayidx40.i, align 8
  br label %postload642.i

postload642.i:                                    ; preds = %preload641.i, %postload637.i
  br i1 %extract275.i, label %preload646.i, label %postload647.i

preload646.i:                                     ; preds = %postload642.i
  store double %phi645.i, double addrspace(1)* %arrayidx40.i, align 8
  br label %postload647.i

postload647.i:                                    ; preds = %preload646.i, %postload642.i
  br i1 %extract276.i, label %preload651.i, label %postload652.i

preload651.i:                                     ; preds = %postload647.i
  store double %phi650.i, double addrspace(1)* %arrayidx40.i, align 8
  br label %postload652.i

postload652.i:                                    ; preds = %preload651.i, %postload647.i
  br i1 %extract277.i, label %preload656.i, label %postload657.i

preload656.i:                                     ; preds = %postload652.i
  store double %phi655.i, double addrspace(1)* %arrayidx40.i, align 8
  br label %postload657.i

postload657.i:                                    ; preds = %preload656.i, %postload652.i
  br i1 %extract278.i, label %preload661.i, label %postload662.i

preload661.i:                                     ; preds = %postload657.i
  store double %phi660.i, double addrspace(1)* %arrayidx40.i, align 8
  br label %postload662.i

postload662.i:                                    ; preds = %preload661.i, %postload657.i
  br i1 %extract279.i, label %preload666.i, label %if.end41.i

preload666.i:                                     ; preds = %postload662.i
  store double %phi665.i, double addrspace(1)* %arrayidx40.i, align 8
  br label %if.end41.i

if.end41.i:                                       ; preds = %preload666.i, %postload662.i
  %check.WI.iter981.i = icmp ult i64 %CurrWI..5.i, %22
  br i1 %check.WI.iter981.i, label %thenBB978.i, label %____Vectorized_.reduce_separated_args.exit

thenBB978.i:                                      ; preds = %if.end41.i
  %"CurrWI++982.i" = add nuw i64 %CurrWI..5.i, 1
  %"loadedCurrSB+Stride984.i" = add nuw i64 %CurrSBIndex..5.i, 128
  %cond9.i = icmp eq i32 %currBarrier.4.i, 3
  br i1 %cond9.i, label %thenBB978.i.postload464.i_crit_edge, label %SyncBB.i

thenBB978.i.postload464.i_crit_edge:              ; preds = %thenBB978.i
  br label %postload464.i

____Vectorized_.reduce_separated_args.exit:       ; preds = %if.end41.i
  ret void
}

define void @__Vectorized_.top_scan(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32*
  %4 = load i32* %3, align 4
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to double addrspace(3)**
  %7 = load double addrspace(3)** %6, align 8
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
  br label %SyncBB974.i

SyncBB974.i:                                      ; preds = %thenBB978.i, %entry
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride984.i", %thenBB978.i ]
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++982.i", %thenBB978.i ]
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
  br i1 %exmask.i, label %preload305.i, label %postload306.i

preload305.i:                                     ; preds = %SyncBB974.i
  %23 = getelementptr inbounds double addrspace(1)* %1, i64 %extract.i
  %vload128.i = load double addrspace(1)* %23, align 8
  br label %postload306.i

postload306.i:                                    ; preds = %preload305.i, %SyncBB974.i
  %phi307.i = phi double [ undef, %SyncBB974.i ], [ %vload128.i, %preload305.i ]
  %vpack.i = insertelement <16 x double> undef, double %phi307.i, i32 0
  %exmask130.i = extractelement <16 x i1> %cmp.i, i32 1
  %"&(pSB[currWI].offset)6772.i" = or i64 %CurrSBIndex..0.i, 9
  %"&pSB[currWI].offset678.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)6772.i"
  %CastToValueType679.i = bitcast i8* %"&pSB[currWI].offset678.i" to i1*
  store i1 %exmask130.i, i1* %CastToValueType679.i, align 1
  br i1 %exmask130.i, label %preload430.i, label %postload431.i

preload430.i:                                     ; preds = %postload306.i
  %.sum495.i = add i64 %extract.i, 1
  %24 = getelementptr double addrspace(1)* %1, i64 %.sum495.i
  %vload131.i = load double addrspace(1)* %24, align 8
  br label %postload431.i

postload431.i:                                    ; preds = %preload430.i, %postload306.i
  %phi432.i = phi double [ undef, %postload306.i ], [ %vload131.i, %preload430.i ]
  %vpack132.i = insertelement <16 x double> %vpack.i, double %phi432.i, i32 1
  %exmask134.i = extractelement <16 x i1> %cmp.i, i32 2
  %"&(pSB[currWI].offset)6913.i" = or i64 %CurrSBIndex..0.i, 10
  %"&pSB[currWI].offset692.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)6913.i"
  %CastToValueType693.i = bitcast i8* %"&pSB[currWI].offset692.i" to i1*
  store i1 %exmask134.i, i1* %CastToValueType693.i, align 1
  br i1 %exmask134.i, label %preload427.i, label %postload428.i

preload427.i:                                     ; preds = %postload431.i
  %.sum494.i = add i64 %extract.i, 2
  %25 = getelementptr double addrspace(1)* %1, i64 %.sum494.i
  %vload135.i = load double addrspace(1)* %25, align 8
  br label %postload428.i

postload428.i:                                    ; preds = %preload427.i, %postload431.i
  %phi429.i = phi double [ undef, %postload431.i ], [ %vload135.i, %preload427.i ]
  %vpack136.i = insertelement <16 x double> %vpack132.i, double %phi429.i, i32 2
  %exmask138.i = extractelement <16 x i1> %cmp.i, i32 3
  %"&(pSB[currWI].offset)7054.i" = or i64 %CurrSBIndex..0.i, 11
  %"&pSB[currWI].offset706.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)7054.i"
  %CastToValueType707.i = bitcast i8* %"&pSB[currWI].offset706.i" to i1*
  store i1 %exmask138.i, i1* %CastToValueType707.i, align 1
  br i1 %exmask138.i, label %preload424.i, label %postload425.i

preload424.i:                                     ; preds = %postload428.i
  %.sum493.i = add i64 %extract.i, 3
  %26 = getelementptr double addrspace(1)* %1, i64 %.sum493.i
  %vload139.i = load double addrspace(1)* %26, align 8
  br label %postload425.i

postload425.i:                                    ; preds = %preload424.i, %postload428.i
  %phi426.i = phi double [ undef, %postload428.i ], [ %vload139.i, %preload424.i ]
  %vpack140.i = insertelement <16 x double> %vpack136.i, double %phi426.i, i32 3
  %exmask142.i = extractelement <16 x i1> %cmp.i, i32 4
  %"&(pSB[currWI].offset)7195.i" = or i64 %CurrSBIndex..0.i, 12
  %"&pSB[currWI].offset720.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)7195.i"
  %CastToValueType721.i = bitcast i8* %"&pSB[currWI].offset720.i" to i1*
  store i1 %exmask142.i, i1* %CastToValueType721.i, align 1
  br i1 %exmask142.i, label %preload421.i, label %postload422.i

preload421.i:                                     ; preds = %postload425.i
  %.sum492.i = add i64 %extract.i, 4
  %27 = getelementptr double addrspace(1)* %1, i64 %.sum492.i
  %vload143.i = load double addrspace(1)* %27, align 8
  br label %postload422.i

postload422.i:                                    ; preds = %preload421.i, %postload425.i
  %phi423.i = phi double [ undef, %postload425.i ], [ %vload143.i, %preload421.i ]
  %vpack144.i = insertelement <16 x double> %vpack140.i, double %phi423.i, i32 4
  %exmask146.i = extractelement <16 x i1> %cmp.i, i32 5
  %"&(pSB[currWI].offset)7336.i" = or i64 %CurrSBIndex..0.i, 13
  %"&pSB[currWI].offset734.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)7336.i"
  %CastToValueType735.i = bitcast i8* %"&pSB[currWI].offset734.i" to i1*
  store i1 %exmask146.i, i1* %CastToValueType735.i, align 1
  br i1 %exmask146.i, label %preload418.i, label %postload419.i

preload418.i:                                     ; preds = %postload422.i
  %.sum491.i = add i64 %extract.i, 5
  %28 = getelementptr double addrspace(1)* %1, i64 %.sum491.i
  %vload147.i = load double addrspace(1)* %28, align 8
  br label %postload419.i

postload419.i:                                    ; preds = %preload418.i, %postload422.i
  %phi420.i = phi double [ undef, %postload422.i ], [ %vload147.i, %preload418.i ]
  %vpack148.i = insertelement <16 x double> %vpack144.i, double %phi420.i, i32 5
  %exmask150.i = extractelement <16 x i1> %cmp.i, i32 6
  %"&(pSB[currWI].offset)7477.i" = or i64 %CurrSBIndex..0.i, 14
  %"&pSB[currWI].offset748.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)7477.i"
  %CastToValueType749.i = bitcast i8* %"&pSB[currWI].offset748.i" to i1*
  store i1 %exmask150.i, i1* %CastToValueType749.i, align 1
  br i1 %exmask150.i, label %preload415.i, label %postload416.i

preload415.i:                                     ; preds = %postload419.i
  %.sum490.i = add i64 %extract.i, 6
  %29 = getelementptr double addrspace(1)* %1, i64 %.sum490.i
  %vload151.i = load double addrspace(1)* %29, align 8
  br label %postload416.i

postload416.i:                                    ; preds = %preload415.i, %postload419.i
  %phi417.i = phi double [ undef, %postload419.i ], [ %vload151.i, %preload415.i ]
  %vpack152.i = insertelement <16 x double> %vpack148.i, double %phi417.i, i32 6
  %exmask154.i = extractelement <16 x i1> %cmp.i, i32 7
  %"&(pSB[currWI].offset)7618.i" = or i64 %CurrSBIndex..0.i, 15
  %"&pSB[currWI].offset762.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)7618.i"
  %CastToValueType763.i = bitcast i8* %"&pSB[currWI].offset762.i" to i1*
  store i1 %exmask154.i, i1* %CastToValueType763.i, align 1
  br i1 %exmask154.i, label %preload406.i, label %postload407.i

preload406.i:                                     ; preds = %postload416.i
  %.sum489.i = add i64 %extract.i, 7
  %30 = getelementptr double addrspace(1)* %1, i64 %.sum489.i
  %vload155.i = load double addrspace(1)* %30, align 8
  br label %postload407.i

postload407.i:                                    ; preds = %preload406.i, %postload416.i
  %phi408.i = phi double [ undef, %postload416.i ], [ %vload155.i, %preload406.i ]
  %vpack156.i = insertelement <16 x double> %vpack152.i, double %phi408.i, i32 7
  %exmask158.i = extractelement <16 x i1> %cmp.i, i32 8
  %"&(pSB[currWI].offset)7759.i" = or i64 %CurrSBIndex..0.i, 16
  %"&pSB[currWI].offset776.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)7759.i"
  %CastToValueType777.i = bitcast i8* %"&pSB[currWI].offset776.i" to i1*
  store i1 %exmask158.i, i1* %CastToValueType777.i, align 1
  br i1 %exmask158.i, label %preload403.i, label %postload404.i

preload403.i:                                     ; preds = %postload407.i
  %.sum488.i = add i64 %extract.i, 8
  %31 = getelementptr double addrspace(1)* %1, i64 %.sum488.i
  %vload159.i = load double addrspace(1)* %31, align 8
  br label %postload404.i

postload404.i:                                    ; preds = %preload403.i, %postload407.i
  %phi405.i = phi double [ undef, %postload407.i ], [ %vload159.i, %preload403.i ]
  %vpack160.i = insertelement <16 x double> %vpack156.i, double %phi405.i, i32 8
  %exmask162.i = extractelement <16 x i1> %cmp.i, i32 9
  %"&(pSB[currWI].offset)78910.i" = or i64 %CurrSBIndex..0.i, 17
  %"&pSB[currWI].offset790.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)78910.i"
  %CastToValueType791.i = bitcast i8* %"&pSB[currWI].offset790.i" to i1*
  store i1 %exmask162.i, i1* %CastToValueType791.i, align 1
  br i1 %exmask162.i, label %preload400.i, label %postload401.i

preload400.i:                                     ; preds = %postload404.i
  %.sum487.i = add i64 %extract.i, 9
  %32 = getelementptr double addrspace(1)* %1, i64 %.sum487.i
  %vload163.i = load double addrspace(1)* %32, align 8
  br label %postload401.i

postload401.i:                                    ; preds = %preload400.i, %postload404.i
  %phi402.i = phi double [ undef, %postload404.i ], [ %vload163.i, %preload400.i ]
  %vpack164.i = insertelement <16 x double> %vpack160.i, double %phi402.i, i32 9
  %exmask166.i = extractelement <16 x i1> %cmp.i, i32 10
  %"&(pSB[currWI].offset)80311.i" = or i64 %CurrSBIndex..0.i, 18
  %"&pSB[currWI].offset804.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)80311.i"
  %CastToValueType805.i = bitcast i8* %"&pSB[currWI].offset804.i" to i1*
  store i1 %exmask166.i, i1* %CastToValueType805.i, align 1
  br i1 %exmask166.i, label %preload397.i, label %postload398.i

preload397.i:                                     ; preds = %postload401.i
  %.sum486.i = add i64 %extract.i, 10
  %33 = getelementptr double addrspace(1)* %1, i64 %.sum486.i
  %vload167.i = load double addrspace(1)* %33, align 8
  br label %postload398.i

postload398.i:                                    ; preds = %preload397.i, %postload401.i
  %phi399.i = phi double [ undef, %postload401.i ], [ %vload167.i, %preload397.i ]
  %vpack168.i = insertelement <16 x double> %vpack164.i, double %phi399.i, i32 10
  %exmask170.i = extractelement <16 x i1> %cmp.i, i32 11
  %"&(pSB[currWI].offset)81712.i" = or i64 %CurrSBIndex..0.i, 19
  %"&pSB[currWI].offset818.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)81712.i"
  %CastToValueType819.i = bitcast i8* %"&pSB[currWI].offset818.i" to i1*
  store i1 %exmask170.i, i1* %CastToValueType819.i, align 1
  br i1 %exmask170.i, label %preload394.i, label %postload395.i

preload394.i:                                     ; preds = %postload398.i
  %.sum485.i = add i64 %extract.i, 11
  %34 = getelementptr double addrspace(1)* %1, i64 %.sum485.i
  %vload171.i = load double addrspace(1)* %34, align 8
  br label %postload395.i

postload395.i:                                    ; preds = %preload394.i, %postload398.i
  %phi396.i = phi double [ undef, %postload398.i ], [ %vload171.i, %preload394.i ]
  %vpack172.i = insertelement <16 x double> %vpack168.i, double %phi396.i, i32 11
  %exmask174.i = extractelement <16 x i1> %cmp.i, i32 12
  %"&(pSB[currWI].offset)83113.i" = or i64 %CurrSBIndex..0.i, 20
  %"&pSB[currWI].offset832.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)83113.i"
  %CastToValueType833.i = bitcast i8* %"&pSB[currWI].offset832.i" to i1*
  store i1 %exmask174.i, i1* %CastToValueType833.i, align 1
  br i1 %exmask174.i, label %preload332.i, label %postload333.i

preload332.i:                                     ; preds = %postload395.i
  %.sum484.i = add i64 %extract.i, 12
  %35 = getelementptr double addrspace(1)* %1, i64 %.sum484.i
  %vload175.i = load double addrspace(1)* %35, align 8
  br label %postload333.i

postload333.i:                                    ; preds = %preload332.i, %postload395.i
  %phi334.i = phi double [ undef, %postload395.i ], [ %vload175.i, %preload332.i ]
  %vpack176.i = insertelement <16 x double> %vpack172.i, double %phi334.i, i32 12
  %exmask178.i = extractelement <16 x i1> %cmp.i, i32 13
  %"&(pSB[currWI].offset)84514.i" = or i64 %CurrSBIndex..0.i, 21
  %"&pSB[currWI].offset846.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)84514.i"
  %CastToValueType847.i = bitcast i8* %"&pSB[currWI].offset846.i" to i1*
  store i1 %exmask178.i, i1* %CastToValueType847.i, align 1
  br i1 %exmask178.i, label %preload329.i, label %postload330.i

preload329.i:                                     ; preds = %postload333.i
  %.sum483.i = add i64 %extract.i, 13
  %36 = getelementptr double addrspace(1)* %1, i64 %.sum483.i
  %vload179.i = load double addrspace(1)* %36, align 8
  br label %postload330.i

postload330.i:                                    ; preds = %preload329.i, %postload333.i
  %phi331.i = phi double [ undef, %postload333.i ], [ %vload179.i, %preload329.i ]
  %vpack180.i = insertelement <16 x double> %vpack176.i, double %phi331.i, i32 13
  %exmask182.i = extractelement <16 x i1> %cmp.i, i32 14
  %"&(pSB[currWI].offset)85915.i" = or i64 %CurrSBIndex..0.i, 22
  %"&pSB[currWI].offset860.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)85915.i"
  %CastToValueType861.i = bitcast i8* %"&pSB[currWI].offset860.i" to i1*
  store i1 %exmask182.i, i1* %CastToValueType861.i, align 1
  br i1 %exmask182.i, label %preload326.i, label %postload327.i

preload326.i:                                     ; preds = %postload330.i
  %.sum482.i = add i64 %extract.i, 14
  %37 = getelementptr double addrspace(1)* %1, i64 %.sum482.i
  %vload183.i = load double addrspace(1)* %37, align 8
  br label %postload327.i

postload327.i:                                    ; preds = %preload326.i, %postload330.i
  %phi328.i = phi double [ undef, %postload330.i ], [ %vload183.i, %preload326.i ]
  %vpack184.i = insertelement <16 x double> %vpack180.i, double %phi328.i, i32 14
  %exmask186.i = extractelement <16 x i1> %cmp.i, i32 15
  %"&(pSB[currWI].offset)87316.i" = or i64 %CurrSBIndex..0.i, 23
  %"&pSB[currWI].offset874.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)87316.i"
  %CastToValueType875.i = bitcast i8* %"&pSB[currWI].offset874.i" to i1*
  store i1 %exmask186.i, i1* %CastToValueType875.i, align 1
  br i1 %exmask186.i, label %preload323.i, label %if.end.i

preload323.i:                                     ; preds = %postload327.i
  %.sum481.i = add i64 %extract.i, 15
  %38 = getelementptr double addrspace(1)* %1, i64 %.sum481.i
  %vload187.i = load double addrspace(1)* %38, align 8
  br label %if.end.i

if.end.i:                                         ; preds = %preload323.i, %postload327.i
  %phi325.i = phi double [ undef, %postload327.i ], [ %vload187.i, %preload323.i ]
  %vpack188.i = insertelement <16 x double> %vpack184.i, double %phi325.i, i32 15
  %merge38.i = select <16 x i1> %cmp.i, <16 x double> %vpack188.i, <16 x double> zeroinitializer
  %extract43.lhs.i = shl i64 %extract.i, 32
  %extract43.i = ashr exact i64 %extract43.lhs.i, 32
  %39 = getelementptr inbounds double addrspace(3)* %7, i64 %extract43.i
  %ptrTypeCast59.i = bitcast double addrspace(3)* %39 to <16 x double> addrspace(3)*
  store <16 x double> zeroinitializer, <16 x double> addrspace(3)* %ptrTypeCast59.i, align 8
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
  %43 = getelementptr inbounds double addrspace(3)* %7, i64 %extract65.i
  %ptrTypeCast81.i = bitcast double addrspace(3)* %43 to <16 x double> addrspace(3)*
  %"&(pSB[currWI].offset)90118.i" = or i64 %CurrSBIndex..0.i, 32
  %"&pSB[currWI].offset902.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)90118.i"
  %CastToValueType903.i = bitcast i8* %"&pSB[currWI].offset902.i" to <16 x double> addrspace(3)**
  store <16 x double> addrspace(3)* %ptrTypeCast81.i, <16 x double> addrspace(3)** %CastToValueType903.i, align 8
  store <16 x double> %merge38.i, <16 x double> addrspace(3)* %ptrTypeCast81.i, align 8
  %check.WI.iter981.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter981.i, label %thenBB978.i, label %elseBB979.i

thenBB978.i:                                      ; preds = %if.end.i
  %"CurrWI++982.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride984.i" = add nuw i64 %CurrSBIndex..0.i, 256
  br label %SyncBB974.i

elseBB979.i:                                      ; preds = %if.end.i
  %cmp1.i.i = icmp ugt i64 %41, 1
  %negIncomingLoopMask.i = xor i1 %cmp1.i.i, true
  br label %SyncBB973.i

SyncBB973.i:                                      ; preds = %thenBB985.i, %thenBB993.i, %thenBB.i, %elseBB979.i
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB979.i ], [ %"loadedCurrSB+Stride991.i", %thenBB985.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ %"loadedCurrSB+Stride999.i", %thenBB993.i ]
  %currBarrier.0.i = phi i32 [ 12, %elseBB979.i ], [ %currBarrier.6.i, %thenBB985.i ], [ %currBarrier.1.i, %thenBB.i ], [ %currBarrier.3.i, %thenBB993.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB979.i ], [ %"CurrWI++989.i", %thenBB985.i ], [ %"CurrWI++.i", %thenBB.i ], [ %"CurrWI++997.i", %thenBB993.i ]
  br i1 %cmp1.i.i, label %for.body.i.i, label %if.then8.i

for.body.i.i:                                     ; preds = %postload360.i, %SyncBB973.i
  %CurrSBIndex..2.i = phi i64 [ %CurrSBIndex..6.i, %postload360.i ], [ %CurrSBIndex..1.i, %SyncBB973.i ]
  %currBarrier.1.i = phi i32 [ %currBarrier.5.i, %postload360.i ], [ %currBarrier.0.i, %SyncBB973.i ]
  %CurrWI..2.i = phi i64 [ %CurrWI..6.i, %postload360.i ], [ %CurrWI..1.i, %SyncBB973.i ]
  %for.body.i_loop_mask.0.i = phi i1 [ %loop_mask3.i, %postload360.i ], [ %negIncomingLoopMask.i, %SyncBB973.i ]
  %for.body.i_Min.i = phi i1 [ %local_edge.i, %postload360.i ], [ %cmp1.i.i, %SyncBB973.i ]
  %loadedValue955.i = phi i32 [ %mul.i.i, %postload360.i ], [ 1, %SyncBB973.i ]
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
  br i1 %for.body.i_Min.i, label %preload353.i, label %postload354.i

preload353.i:                                     ; preds = %for.body.i.i
  %"&(pSB[currWI].offset)891.i" = add nuw i64 %CurrSBIndex..2.i, 24
  %"&pSB[currWI].offset892.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)891.i"
  %CastToValueType893.i = bitcast i8* %"&pSB[currWI].offset892.i" to i32*
  %loadedValue894.i = load i32* %CastToValueType893.i, align 4
  %44 = sub i32 %loadedValue894.i, %loadedValue955.i
  %extract86.i = sext i32 %44 to i64
  %45 = getelementptr inbounds double addrspace(3)* %7, i64 %extract86.i
  %ptrTypeCast102.i = bitcast double addrspace(3)* %45 to <16 x double> addrspace(3)*
  %masked_load.i = load <16 x double> addrspace(3)* %ptrTypeCast102.i, align 8
  %"&(pSB[currWI].offset)962.i" = add nuw i64 %CurrSBIndex..2.i, 128
  %"&pSB[currWI].offset963.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)962.i"
  %CastToValueType964.i = bitcast i8* %"&pSB[currWI].offset963.i" to <16 x double>*
  store <16 x double> %masked_load.i, <16 x double>* %CastToValueType964.i, align 128
  %check.WI.iter.i = icmp ult i64 %CurrWI..2.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %SyncBB.i

thenBB.i:                                         ; preds = %preload353.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..2.i, 256
  switch i32 %currBarrier.1.i, label %SyncBB973.i [
    i32 22, label %thenBB.i.postload360.i_crit_edge
    i32 4, label %SyncBB.i
  ]

thenBB.i.postload360.i_crit_edge:                 ; preds = %thenBB.i
  br label %postload360.i

SyncBB.i:                                         ; preds = %thenBB985.i, %thenBB993.i, %thenBB.i, %preload353.i
  %CurrSBIndex..3.i = phi i64 [ %"loadedCurrSB+Stride991.i", %thenBB985.i ], [ %"loadedCurrSB+Stride999.i", %thenBB993.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %preload353.i ]
  %currBarrier.2.i = phi i32 [ %currBarrier.6.i, %thenBB985.i ], [ %currBarrier.3.i, %thenBB993.i ], [ %currBarrier.1.i, %thenBB.i ], [ 4, %preload353.i ]
  %CurrWI..3.i = phi i64 [ %"CurrWI++989.i", %thenBB985.i ], [ %"CurrWI++997.i", %thenBB993.i ], [ %"CurrWI++.i", %thenBB.i ], [ 0, %preload353.i ]
  %"&(pSB[currWI].offset)966.i" = add nuw i64 %CurrSBIndex..3.i, 128
  %"&pSB[currWI].offset967.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)966.i"
  %CastToValueType968.i = bitcast i8* %"&pSB[currWI].offset967.i" to <16 x double>*
  %loadedValue969.i = load <16 x double>* %CastToValueType968.i, align 128
  br label %postload354.i

postload354.i:                                    ; preds = %SyncBB.i, %for.body.i.i
  %CurrSBIndex..4.i = phi i64 [ %CurrSBIndex..3.i, %SyncBB.i ], [ %CurrSBIndex..2.i, %for.body.i.i ]
  %currBarrier.3.i = phi i32 [ %currBarrier.2.i, %SyncBB.i ], [ %currBarrier.1.i, %for.body.i.i ]
  %CurrWI..4.i = phi i64 [ %CurrWI..3.i, %SyncBB.i ], [ %CurrWI..2.i, %for.body.i.i ]
  %phi355.i = phi <16 x double> [ %loadedValue969.i, %SyncBB.i ], [ undef, %for.body.i.i ]
  %"&(pSB[currWI].offset)933.i" = add nuw i64 %CurrSBIndex..4.i, 41
  %"&pSB[currWI].offset934.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)933.i"
  %CastToValueType935.i = bitcast i8* %"&pSB[currWI].offset934.i" to i1*
  %loadedValue936.i = load i1* %CastToValueType935.i, align 1
  br i1 %loadedValue936.i, label %preload356.i, label %postload357.i

preload356.i:                                     ; preds = %postload354.i
  %"&(pSB[currWI].offset)910.i" = add nuw i64 %CurrSBIndex..4.i, 32
  %"&pSB[currWI].offset911.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)910.i"
  %CastToValueType912.i = bitcast i8* %"&pSB[currWI].offset911.i" to <16 x double> addrspace(3)**
  %loadedValue913.i = load <16 x double> addrspace(3)** %CastToValueType912.i, align 8
  %masked_load189.i = load <16 x double> addrspace(3)* %loadedValue913.i, align 8
  br label %postload357.i

postload357.i:                                    ; preds = %preload356.i, %postload354.i
  %phi358.i = phi <16 x double> [ undef, %postload354.i ], [ %masked_load189.i, %preload356.i ]
  br i1 %loadedValue936.i, label %preload359.i, label %postload360.i

preload359.i:                                     ; preds = %postload357.i
  %add13.i104.i = fadd <16 x double> %phi358.i, %phi355.i
  %"&(pSB[currWI].offset)905.i" = add nuw i64 %CurrSBIndex..4.i, 32
  %"&pSB[currWI].offset906.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)905.i"
  %CastToValueType907.i = bitcast i8* %"&pSB[currWI].offset906.i" to <16 x double> addrspace(3)**
  %loadedValue908.i = load <16 x double> addrspace(3)** %CastToValueType907.i, align 8
  store <16 x double> %add13.i104.i, <16 x double> addrspace(3)* %loadedValue908.i, align 8
  %check.WI.iter996.i = icmp ult i64 %CurrWI..4.i, %16
  br i1 %check.WI.iter996.i, label %thenBB993.i, label %preload359.i.postload360.i_crit_edge

preload359.i.postload360.i_crit_edge:             ; preds = %preload359.i
  br label %postload360.i

thenBB993.i:                                      ; preds = %preload359.i
  %"CurrWI++997.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride999.i" = add nuw i64 %CurrSBIndex..4.i, 256
  switch i32 %currBarrier.3.i, label %SyncBB.i [
    i32 12, label %SyncBB973.i
    i32 22, label %thenBB993.i.postload360.i_crit_edge
  ]

thenBB993.i.postload360.i_crit_edge:              ; preds = %thenBB993.i
  br label %postload360.i

postload360.i:                                    ; preds = %thenBB985.i.postload360.i_crit_edge, %thenBB993.i.postload360.i_crit_edge, %preload359.i.postload360.i_crit_edge, %thenBB.i.postload360.i_crit_edge, %postload357.i
  %CurrSBIndex..6.i = phi i64 [ %CurrSBIndex..4.i, %postload357.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i.postload360.i_crit_edge ], [ 0, %preload359.i.postload360.i_crit_edge ], [ %"loadedCurrSB+Stride999.i", %thenBB993.i.postload360.i_crit_edge ], [ %"loadedCurrSB+Stride991.i", %thenBB985.i.postload360.i_crit_edge ]
  %currBarrier.5.i = phi i32 [ %currBarrier.3.i, %postload357.i ], [ %currBarrier.1.i, %thenBB.i.postload360.i_crit_edge ], [ 22, %preload359.i.postload360.i_crit_edge ], [ %currBarrier.3.i, %thenBB993.i.postload360.i_crit_edge ], [ %currBarrier.6.i, %thenBB985.i.postload360.i_crit_edge ]
  %CurrWI..6.i = phi i64 [ %CurrWI..4.i, %postload357.i ], [ %"CurrWI++.i", %thenBB.i.postload360.i_crit_edge ], [ 0, %preload359.i.postload360.i_crit_edge ], [ %"CurrWI++997.i", %thenBB993.i.postload360.i_crit_edge ], [ %"CurrWI++989.i", %thenBB985.i.postload360.i_crit_edge ]
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

if.then8.i:                                       ; preds = %postload360.i, %SyncBB973.i
  %CurrSBIndex..7.i = phi i64 [ %CurrSBIndex..1.i, %SyncBB973.i ], [ %CurrSBIndex..6.i, %postload360.i ]
  %currBarrier.6.i = phi i32 [ %currBarrier.0.i, %SyncBB973.i ], [ %currBarrier.5.i, %postload360.i ]
  %CurrWI..7.i = phi i64 [ %CurrWI..1.i, %SyncBB973.i ], [ %CurrWI..6.i, %postload360.i ]
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
  br i1 %loadedValue675.i, label %preload320.i, label %postload321.i

preload320.i:                                     ; preds = %if.then8.i
  %47 = getelementptr inbounds double addrspace(3)* %7, i64 %extract109.i
  %vload193.i = load double addrspace(3)* %47, align 8
  br label %postload321.i

postload321.i:                                    ; preds = %preload320.i, %if.then8.i
  %phi322.i = phi double [ undef, %if.then8.i ], [ %vload193.i, %preload320.i ]
  %"&(pSB[currWI].offset)686.i" = add nuw i64 %CurrSBIndex..7.i, 9
  %"&pSB[currWI].offset687.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)686.i"
  %CastToValueType688.i = bitcast i8* %"&pSB[currWI].offset687.i" to i1*
  %loadedValue689.i = load i1* %CastToValueType688.i, align 1
  br i1 %loadedValue689.i, label %preload344.i, label %postload345.i

preload344.i:                                     ; preds = %postload321.i
  %.sum480.i = add i64 %extract109.i, 1
  %48 = getelementptr double addrspace(3)* %7, i64 %.sum480.i
  %vload197.i = load double addrspace(3)* %48, align 8
  br label %postload345.i

postload345.i:                                    ; preds = %preload344.i, %postload321.i
  %phi346.i = phi double [ undef, %postload321.i ], [ %vload197.i, %preload344.i ]
  %"&(pSB[currWI].offset)700.i" = add nuw i64 %CurrSBIndex..7.i, 10
  %"&pSB[currWI].offset701.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)700.i"
  %CastToValueType702.i = bitcast i8* %"&pSB[currWI].offset701.i" to i1*
  %loadedValue703.i = load i1* %CastToValueType702.i, align 1
  br i1 %loadedValue703.i, label %preload341.i, label %postload342.i

preload341.i:                                     ; preds = %postload345.i
  %.sum479.i = add i64 %extract109.i, 2
  %49 = getelementptr double addrspace(3)* %7, i64 %.sum479.i
  %vload201.i = load double addrspace(3)* %49, align 8
  br label %postload342.i

postload342.i:                                    ; preds = %preload341.i, %postload345.i
  %phi343.i = phi double [ undef, %postload345.i ], [ %vload201.i, %preload341.i ]
  %"&(pSB[currWI].offset)714.i" = add nuw i64 %CurrSBIndex..7.i, 11
  %"&pSB[currWI].offset715.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)714.i"
  %CastToValueType716.i = bitcast i8* %"&pSB[currWI].offset715.i" to i1*
  %loadedValue717.i = load i1* %CastToValueType716.i, align 1
  br i1 %loadedValue717.i, label %preload338.i, label %postload339.i

preload338.i:                                     ; preds = %postload342.i
  %.sum478.i = add i64 %extract109.i, 3
  %50 = getelementptr double addrspace(3)* %7, i64 %.sum478.i
  %vload205.i = load double addrspace(3)* %50, align 8
  br label %postload339.i

postload339.i:                                    ; preds = %preload338.i, %postload342.i
  %phi340.i = phi double [ undef, %postload342.i ], [ %vload205.i, %preload338.i ]
  %"&(pSB[currWI].offset)728.i" = add nuw i64 %CurrSBIndex..7.i, 12
  %"&pSB[currWI].offset729.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)728.i"
  %CastToValueType730.i = bitcast i8* %"&pSB[currWI].offset729.i" to i1*
  %loadedValue731.i = load i1* %CastToValueType730.i, align 1
  br i1 %loadedValue731.i, label %preload335.i, label %postload336.i

preload335.i:                                     ; preds = %postload339.i
  %.sum477.i = add i64 %extract109.i, 4
  %51 = getelementptr double addrspace(3)* %7, i64 %.sum477.i
  %vload209.i = load double addrspace(3)* %51, align 8
  br label %postload336.i

postload336.i:                                    ; preds = %preload335.i, %postload339.i
  %phi337.i = phi double [ undef, %postload339.i ], [ %vload209.i, %preload335.i ]
  %"&(pSB[currWI].offset)742.i" = add nuw i64 %CurrSBIndex..7.i, 13
  %"&pSB[currWI].offset743.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)742.i"
  %CastToValueType744.i = bitcast i8* %"&pSB[currWI].offset743.i" to i1*
  %loadedValue745.i = load i1* %CastToValueType744.i, align 1
  br i1 %loadedValue745.i, label %preload317.i, label %postload318.i

preload317.i:                                     ; preds = %postload336.i
  %.sum476.i = add i64 %extract109.i, 5
  %52 = getelementptr double addrspace(3)* %7, i64 %.sum476.i
  %vload213.i = load double addrspace(3)* %52, align 8
  br label %postload318.i

postload318.i:                                    ; preds = %preload317.i, %postload336.i
  %phi319.i = phi double [ undef, %postload336.i ], [ %vload213.i, %preload317.i ]
  %"&(pSB[currWI].offset)756.i" = add nuw i64 %CurrSBIndex..7.i, 14
  %"&pSB[currWI].offset757.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)756.i"
  %CastToValueType758.i = bitcast i8* %"&pSB[currWI].offset757.i" to i1*
  %loadedValue759.i = load i1* %CastToValueType758.i, align 1
  br i1 %loadedValue759.i, label %preload350.i, label %postload351.i

preload350.i:                                     ; preds = %postload318.i
  %.sum475.i = add i64 %extract109.i, 6
  %53 = getelementptr double addrspace(3)* %7, i64 %.sum475.i
  %vload217.i = load double addrspace(3)* %53, align 8
  br label %postload351.i

postload351.i:                                    ; preds = %preload350.i, %postload318.i
  %phi352.i = phi double [ undef, %postload318.i ], [ %vload217.i, %preload350.i ]
  %"&(pSB[currWI].offset)770.i" = add nuw i64 %CurrSBIndex..7.i, 15
  %"&pSB[currWI].offset771.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)770.i"
  %CastToValueType772.i = bitcast i8* %"&pSB[currWI].offset771.i" to i1*
  %loadedValue773.i = load i1* %CastToValueType772.i, align 1
  br i1 %loadedValue773.i, label %preload376.i, label %postload377.i

preload376.i:                                     ; preds = %postload351.i
  %.sum474.i = add i64 %extract109.i, 7
  %54 = getelementptr double addrspace(3)* %7, i64 %.sum474.i
  %vload221.i = load double addrspace(3)* %54, align 8
  br label %postload377.i

postload377.i:                                    ; preds = %preload376.i, %postload351.i
  %phi378.i = phi double [ undef, %postload351.i ], [ %vload221.i, %preload376.i ]
  %"&(pSB[currWI].offset)784.i" = add nuw i64 %CurrSBIndex..7.i, 16
  %"&pSB[currWI].offset785.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)784.i"
  %CastToValueType786.i = bitcast i8* %"&pSB[currWI].offset785.i" to i1*
  %loadedValue787.i = load i1* %CastToValueType786.i, align 1
  br i1 %loadedValue787.i, label %preload314.i, label %postload315.i

preload314.i:                                     ; preds = %postload377.i
  %.sum473.i = add i64 %extract109.i, 8
  %55 = getelementptr double addrspace(3)* %7, i64 %.sum473.i
  %vload225.i = load double addrspace(3)* %55, align 8
  br label %postload315.i

postload315.i:                                    ; preds = %preload314.i, %postload377.i
  %phi316.i = phi double [ undef, %postload377.i ], [ %vload225.i, %preload314.i ]
  %"&(pSB[currWI].offset)798.i" = add nuw i64 %CurrSBIndex..7.i, 17
  %"&pSB[currWI].offset799.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)798.i"
  %CastToValueType800.i = bitcast i8* %"&pSB[currWI].offset799.i" to i1*
  %loadedValue801.i = load i1* %CastToValueType800.i, align 1
  br i1 %loadedValue801.i, label %preload382.i, label %postload383.i

preload382.i:                                     ; preds = %postload315.i
  %.sum472.i = add i64 %extract109.i, 9
  %56 = getelementptr double addrspace(3)* %7, i64 %.sum472.i
  %vload229.i = load double addrspace(3)* %56, align 8
  br label %postload383.i

postload383.i:                                    ; preds = %preload382.i, %postload315.i
  %phi384.i = phi double [ undef, %postload315.i ], [ %vload229.i, %preload382.i ]
  %"&(pSB[currWI].offset)812.i" = add nuw i64 %CurrSBIndex..7.i, 18
  %"&pSB[currWI].offset813.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)812.i"
  %CastToValueType814.i = bitcast i8* %"&pSB[currWI].offset813.i" to i1*
  %loadedValue815.i = load i1* %CastToValueType814.i, align 1
  br i1 %loadedValue815.i, label %preload302.i, label %postload303.i

preload302.i:                                     ; preds = %postload383.i
  %.sum471.i = add i64 %extract109.i, 10
  %57 = getelementptr double addrspace(3)* %7, i64 %.sum471.i
  %vload233.i = load double addrspace(3)* %57, align 8
  br label %postload303.i

postload303.i:                                    ; preds = %preload302.i, %postload383.i
  %phi304.i = phi double [ undef, %postload383.i ], [ %vload233.i, %preload302.i ]
  %"&(pSB[currWI].offset)826.i" = add nuw i64 %CurrSBIndex..7.i, 19
  %"&pSB[currWI].offset827.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)826.i"
  %CastToValueType828.i = bitcast i8* %"&pSB[currWI].offset827.i" to i1*
  %loadedValue829.i = load i1* %CastToValueType828.i, align 1
  br i1 %loadedValue829.i, label %preload347.i, label %postload348.i

preload347.i:                                     ; preds = %postload303.i
  %.sum470.i = add i64 %extract109.i, 11
  %58 = getelementptr double addrspace(3)* %7, i64 %.sum470.i
  %vload237.i = load double addrspace(3)* %58, align 8
  br label %postload348.i

postload348.i:                                    ; preds = %preload347.i, %postload303.i
  %phi349.i = phi double [ undef, %postload303.i ], [ %vload237.i, %preload347.i ]
  %"&(pSB[currWI].offset)840.i" = add nuw i64 %CurrSBIndex..7.i, 20
  %"&pSB[currWI].offset841.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)840.i"
  %CastToValueType842.i = bitcast i8* %"&pSB[currWI].offset841.i" to i1*
  %loadedValue843.i = load i1* %CastToValueType842.i, align 1
  br i1 %loadedValue843.i, label %preload379.i, label %postload380.i

preload379.i:                                     ; preds = %postload348.i
  %.sum469.i = add i64 %extract109.i, 12
  %59 = getelementptr double addrspace(3)* %7, i64 %.sum469.i
  %vload241.i = load double addrspace(3)* %59, align 8
  br label %postload380.i

postload380.i:                                    ; preds = %preload379.i, %postload348.i
  %phi381.i = phi double [ undef, %postload348.i ], [ %vload241.i, %preload379.i ]
  %"&(pSB[currWI].offset)854.i" = add nuw i64 %CurrSBIndex..7.i, 21
  %"&pSB[currWI].offset855.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)854.i"
  %CastToValueType856.i = bitcast i8* %"&pSB[currWI].offset855.i" to i1*
  %loadedValue857.i = load i1* %CastToValueType856.i, align 1
  br i1 %loadedValue857.i, label %preload.i, label %postload.i

preload.i:                                        ; preds = %postload380.i
  %.sum468.i = add i64 %extract109.i, 13
  %60 = getelementptr double addrspace(3)* %7, i64 %.sum468.i
  %vload245.i = load double addrspace(3)* %60, align 8
  br label %postload.i

postload.i:                                       ; preds = %preload.i, %postload380.i
  %phi.i = phi double [ undef, %postload380.i ], [ %vload245.i, %preload.i ]
  %"&(pSB[currWI].offset)868.i" = add nuw i64 %CurrSBIndex..7.i, 22
  %"&pSB[currWI].offset869.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)868.i"
  %CastToValueType870.i = bitcast i8* %"&pSB[currWI].offset869.i" to i1*
  %loadedValue871.i = load i1* %CastToValueType870.i, align 1
  br i1 %loadedValue871.i, label %preload308.i, label %postload309.i

preload308.i:                                     ; preds = %postload.i
  %.sum467.i = add i64 %extract109.i, 14
  %61 = getelementptr double addrspace(3)* %7, i64 %.sum467.i
  %vload249.i = load double addrspace(3)* %61, align 8
  br label %postload309.i

postload309.i:                                    ; preds = %preload308.i, %postload.i
  %phi310.i = phi double [ undef, %postload.i ], [ %vload249.i, %preload308.i ]
  %"&(pSB[currWI].offset)882.i" = add nuw i64 %CurrSBIndex..7.i, 23
  %"&pSB[currWI].offset883.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)882.i"
  %CastToValueType884.i = bitcast i8* %"&pSB[currWI].offset883.i" to i1*
  %loadedValue885.i = load i1* %CastToValueType884.i, align 1
  br i1 %loadedValue885.i, label %preload385.i, label %postload386.i

preload385.i:                                     ; preds = %postload309.i
  %.sum466.i = add i64 %extract109.i, 15
  %62 = getelementptr double addrspace(3)* %7, i64 %.sum466.i
  %vload253.i = load double addrspace(3)* %62, align 8
  br label %postload386.i

postload386.i:                                    ; preds = %preload385.i, %postload309.i
  %phi387.i = phi double [ undef, %postload309.i ], [ %vload253.i, %preload385.i ]
  br i1 %loadedValue675.i, label %preload311.i, label %postload312.i

preload311.i:                                     ; preds = %postload386.i
  %"&pSB[currWI].offset654.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType655.i = bitcast i8* %"&pSB[currWI].offset654.i" to i64*
  %loadedValue656.i = load i64* %CastToValueType655.i, align 8
  %63 = getelementptr inbounds double addrspace(1)* %1, i64 %loadedValue656.i
  store double %phi322.i, double addrspace(1)* %63, align 8
  %loadedValue684.pre.i = load i1* %CastToValueType688.i, align 1
  br label %postload312.i

postload312.i:                                    ; preds = %preload311.i, %postload386.i
  %loadedValue684.i = phi i1 [ %loadedValue684.pre.i, %preload311.i ], [ %loadedValue689.i, %postload386.i ]
  br i1 %loadedValue684.i, label %preload388.i, label %postload312.i.postload389.i_crit_edge

postload312.i.postload389.i_crit_edge:            ; preds = %postload312.i
  br label %postload389.i

preload388.i:                                     ; preds = %postload312.i
  %"&pSB[currWI].offset579.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType580.i = bitcast i8* %"&pSB[currWI].offset579.i" to i64*
  %loadedValue581.i = load i64* %CastToValueType580.i, align 8
  %.sum465.i = add i64 %loadedValue581.i, 1
  %64 = getelementptr double addrspace(1)* %1, i64 %.sum465.i
  store double %phi346.i, double addrspace(1)* %64, align 8
  br label %postload389.i

postload389.i:                                    ; preds = %postload312.i.postload389.i_crit_edge, %preload388.i
  %loadedValue698.i = load i1* %CastToValueType702.i, align 1
  br i1 %loadedValue698.i, label %preload391.i, label %postload389.i.postload392.i_crit_edge

postload389.i.postload392.i_crit_edge:            ; preds = %postload389.i
  br label %postload392.i

preload391.i:                                     ; preds = %postload389.i
  %"&pSB[currWI].offset584.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType585.i = bitcast i8* %"&pSB[currWI].offset584.i" to i64*
  %loadedValue586.i = load i64* %CastToValueType585.i, align 8
  %.sum464.i = add i64 %loadedValue586.i, 2
  %65 = getelementptr double addrspace(1)* %1, i64 %.sum464.i
  store double %phi343.i, double addrspace(1)* %65, align 8
  br label %postload392.i

postload392.i:                                    ; preds = %postload389.i.postload392.i_crit_edge, %preload391.i
  %loadedValue712.i = load i1* %CastToValueType716.i, align 1
  br i1 %loadedValue712.i, label %preload409.i, label %postload392.i.postload410.i_crit_edge

postload392.i.postload410.i_crit_edge:            ; preds = %postload392.i
  br label %postload410.i

preload409.i:                                     ; preds = %postload392.i
  %"&pSB[currWI].offset589.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType590.i = bitcast i8* %"&pSB[currWI].offset589.i" to i64*
  %loadedValue591.i = load i64* %CastToValueType590.i, align 8
  %.sum463.i = add i64 %loadedValue591.i, 3
  %66 = getelementptr double addrspace(1)* %1, i64 %.sum463.i
  store double %phi340.i, double addrspace(1)* %66, align 8
  br label %postload410.i

postload410.i:                                    ; preds = %postload392.i.postload410.i_crit_edge, %preload409.i
  %loadedValue726.i = load i1* %CastToValueType730.i, align 1
  br i1 %loadedValue726.i, label %preload412.i, label %postload410.i.postload413.i_crit_edge

postload410.i.postload413.i_crit_edge:            ; preds = %postload410.i
  br label %postload413.i

preload412.i:                                     ; preds = %postload410.i
  %"&pSB[currWI].offset594.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType595.i = bitcast i8* %"&pSB[currWI].offset594.i" to i64*
  %loadedValue596.i = load i64* %CastToValueType595.i, align 8
  %.sum462.i = add i64 %loadedValue596.i, 4
  %67 = getelementptr double addrspace(1)* %1, i64 %.sum462.i
  store double %phi337.i, double addrspace(1)* %67, align 8
  br label %postload413.i

postload413.i:                                    ; preds = %postload410.i.postload413.i_crit_edge, %preload412.i
  %loadedValue740.i = load i1* %CastToValueType744.i, align 1
  br i1 %loadedValue740.i, label %preload433.i, label %postload413.i.postload434.i_crit_edge

postload413.i.postload434.i_crit_edge:            ; preds = %postload413.i
  br label %postload434.i

preload433.i:                                     ; preds = %postload413.i
  %"&pSB[currWI].offset599.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType600.i = bitcast i8* %"&pSB[currWI].offset599.i" to i64*
  %loadedValue601.i = load i64* %CastToValueType600.i, align 8
  %.sum461.i = add i64 %loadedValue601.i, 5
  %68 = getelementptr double addrspace(1)* %1, i64 %.sum461.i
  store double %phi319.i, double addrspace(1)* %68, align 8
  br label %postload434.i

postload434.i:                                    ; preds = %postload413.i.postload434.i_crit_edge, %preload433.i
  %loadedValue754.i = load i1* %CastToValueType758.i, align 1
  br i1 %loadedValue754.i, label %preload436.i, label %postload434.i.postload437.i_crit_edge

postload434.i.postload437.i_crit_edge:            ; preds = %postload434.i
  br label %postload437.i

preload436.i:                                     ; preds = %postload434.i
  %"&pSB[currWI].offset604.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType605.i = bitcast i8* %"&pSB[currWI].offset604.i" to i64*
  %loadedValue606.i = load i64* %CastToValueType605.i, align 8
  %.sum460.i = add i64 %loadedValue606.i, 6
  %69 = getelementptr double addrspace(1)* %1, i64 %.sum460.i
  store double %phi352.i, double addrspace(1)* %69, align 8
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
  %70 = getelementptr double addrspace(1)* %1, i64 %.sum459.i
  store double %phi378.i, double addrspace(1)* %70, align 8
  br label %postload440.i

postload440.i:                                    ; preds = %postload437.i.postload440.i_crit_edge, %preload439.i
  %loadedValue782.i = load i1* %CastToValueType786.i, align 1
  br i1 %loadedValue782.i, label %preload361.i, label %postload440.i.postload362.i_crit_edge

postload440.i.postload362.i_crit_edge:            ; preds = %postload440.i
  br label %postload362.i

preload361.i:                                     ; preds = %postload440.i
  %"&pSB[currWI].offset614.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType615.i = bitcast i8* %"&pSB[currWI].offset614.i" to i64*
  %loadedValue616.i = load i64* %CastToValueType615.i, align 8
  %.sum458.i = add i64 %loadedValue616.i, 8
  %71 = getelementptr double addrspace(1)* %1, i64 %.sum458.i
  store double %phi316.i, double addrspace(1)* %71, align 8
  br label %postload362.i

postload362.i:                                    ; preds = %postload440.i.postload362.i_crit_edge, %preload361.i
  %loadedValue796.i = load i1* %CastToValueType800.i, align 1
  br i1 %loadedValue796.i, label %preload364.i, label %postload362.i.postload365.i_crit_edge

postload362.i.postload365.i_crit_edge:            ; preds = %postload362.i
  br label %postload365.i

preload364.i:                                     ; preds = %postload362.i
  %"&pSB[currWI].offset619.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType620.i = bitcast i8* %"&pSB[currWI].offset619.i" to i64*
  %loadedValue621.i = load i64* %CastToValueType620.i, align 8
  %.sum457.i = add i64 %loadedValue621.i, 9
  %72 = getelementptr double addrspace(1)* %1, i64 %.sum457.i
  store double %phi384.i, double addrspace(1)* %72, align 8
  br label %postload365.i

postload365.i:                                    ; preds = %postload362.i.postload365.i_crit_edge, %preload364.i
  %loadedValue810.i = load i1* %CastToValueType814.i, align 1
  br i1 %loadedValue810.i, label %preload367.i, label %postload365.i.postload368.i_crit_edge

postload365.i.postload368.i_crit_edge:            ; preds = %postload365.i
  br label %postload368.i

preload367.i:                                     ; preds = %postload365.i
  %"&pSB[currWI].offset624.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType625.i = bitcast i8* %"&pSB[currWI].offset624.i" to i64*
  %loadedValue626.i = load i64* %CastToValueType625.i, align 8
  %.sum456.i = add i64 %loadedValue626.i, 10
  %73 = getelementptr double addrspace(1)* %1, i64 %.sum456.i
  store double %phi304.i, double addrspace(1)* %73, align 8
  br label %postload368.i

postload368.i:                                    ; preds = %postload365.i.postload368.i_crit_edge, %preload367.i
  %loadedValue824.i = load i1* %CastToValueType828.i, align 1
  br i1 %loadedValue824.i, label %preload370.i, label %postload368.i.postload371.i_crit_edge

postload368.i.postload371.i_crit_edge:            ; preds = %postload368.i
  br label %postload371.i

preload370.i:                                     ; preds = %postload368.i
  %"&pSB[currWI].offset629.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType630.i = bitcast i8* %"&pSB[currWI].offset629.i" to i64*
  %loadedValue631.i = load i64* %CastToValueType630.i, align 8
  %.sum455.i = add i64 %loadedValue631.i, 11
  %74 = getelementptr double addrspace(1)* %1, i64 %.sum455.i
  store double %phi349.i, double addrspace(1)* %74, align 8
  br label %postload371.i

postload371.i:                                    ; preds = %postload368.i.postload371.i_crit_edge, %preload370.i
  %loadedValue838.i = load i1* %CastToValueType842.i, align 1
  br i1 %loadedValue838.i, label %preload373.i, label %postload371.i.postload374.i_crit_edge

postload371.i.postload374.i_crit_edge:            ; preds = %postload371.i
  br label %postload374.i

preload373.i:                                     ; preds = %postload371.i
  %"&pSB[currWI].offset634.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType635.i = bitcast i8* %"&pSB[currWI].offset634.i" to i64*
  %loadedValue636.i = load i64* %CastToValueType635.i, align 8
  %.sum454.i = add i64 %loadedValue636.i, 12
  %75 = getelementptr double addrspace(1)* %1, i64 %.sum454.i
  store double %phi381.i, double addrspace(1)* %75, align 8
  br label %postload374.i

postload374.i:                                    ; preds = %postload371.i.postload374.i_crit_edge, %preload373.i
  %loadedValue852.i = load i1* %CastToValueType856.i, align 1
  br i1 %loadedValue852.i, label %preload442.i, label %postload374.i.postload443.i_crit_edge

postload374.i.postload443.i_crit_edge:            ; preds = %postload374.i
  br label %postload443.i

preload442.i:                                     ; preds = %postload374.i
  %"&pSB[currWI].offset639.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType640.i = bitcast i8* %"&pSB[currWI].offset639.i" to i64*
  %loadedValue641.i = load i64* %CastToValueType640.i, align 8
  %.sum453.i = add i64 %loadedValue641.i, 13
  %76 = getelementptr double addrspace(1)* %1, i64 %.sum453.i
  store double %phi.i, double addrspace(1)* %76, align 8
  br label %postload443.i

postload443.i:                                    ; preds = %postload374.i.postload443.i_crit_edge, %preload442.i
  %loadedValue866.i = load i1* %CastToValueType870.i, align 1
  br i1 %loadedValue866.i, label %preload445.i, label %postload443.i.postload446.i_crit_edge

postload443.i.postload446.i_crit_edge:            ; preds = %postload443.i
  br label %postload446.i

preload445.i:                                     ; preds = %postload443.i
  %"&pSB[currWI].offset644.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType645.i = bitcast i8* %"&pSB[currWI].offset644.i" to i64*
  %loadedValue646.i = load i64* %CastToValueType645.i, align 8
  %.sum452.i = add i64 %loadedValue646.i, 14
  %77 = getelementptr double addrspace(1)* %1, i64 %.sum452.i
  store double %phi310.i, double addrspace(1)* %77, align 8
  br label %postload446.i

postload446.i:                                    ; preds = %postload443.i.postload446.i_crit_edge, %preload445.i
  %loadedValue880.i = load i1* %CastToValueType884.i, align 1
  br i1 %loadedValue880.i, label %preload448.i, label %if.end11.i

preload448.i:                                     ; preds = %postload446.i
  %"&pSB[currWI].offset649.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..7.i
  %CastToValueType650.i = bitcast i8* %"&pSB[currWI].offset649.i" to i64*
  %loadedValue651.i = load i64* %CastToValueType650.i, align 8
  %.sum.i = add i64 %loadedValue651.i, 15
  %78 = getelementptr double addrspace(1)* %1, i64 %.sum.i
  store double %phi387.i, double addrspace(1)* %78, align 8
  br label %if.end11.i

if.end11.i:                                       ; preds = %preload448.i, %postload446.i
  %check.WI.iter988.i = icmp ult i64 %CurrWI..7.i, %16
  br i1 %check.WI.iter988.i, label %thenBB985.i, label %____Vectorized_.top_scan_separated_args.exit

thenBB985.i:                                      ; preds = %if.end11.i
  %"CurrWI++989.i" = add nuw i64 %CurrWI..7.i, 1
  %"loadedCurrSB+Stride991.i" = add nuw i64 %CurrSBIndex..7.i, 256
  switch i32 %currBarrier.6.i, label %SyncBB973.i [
    i32 22, label %thenBB985.i.postload360.i_crit_edge
    i32 4, label %SyncBB.i
  ]

thenBB985.i.postload360.i_crit_edge:              ; preds = %thenBB985.i
  br label %postload360.i

____Vectorized_.top_scan_separated_args.exit:     ; preds = %if.end11.i
  ret void
}

!opencl.kernels = !{!0, !2, !4}
!opencl.build.options = !{!6}
!cl.noBarrierPath.kernels = !{!6}
!opencl.wrappers = !{!7, !8, !9}

!0 = metadata !{void (double addrspace(1)*, double addrspace(1)*, i32, double addrspace(3)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__reduce_separated_args, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3}
!2 = metadata !{void (double addrspace(1)*, i32, double addrspace(3)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__top_scan_separated_args, metadata !3}
!3 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3}
!4 = metadata !{void (double addrspace(1)*, double addrspace(1)*, double addrspace(1)*, i32, double addrspace(3)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__bottom_scan_separated_args, metadata !5}
!5 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3, i32 3}
!6 = metadata !{}
!7 = metadata !{void (i8*)* @reduce}
!8 = metadata !{void (i8*)* @top_scan}
!9 = metadata !{void (i8*)* @bottom_scan}
