; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc
;
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

@opencl_radixSortBlocks_local_numtrue.0 = internal addrspace(3) global i32 0
@opencl_findRadixOffsets_local_sStartPointers = internal addrspace(3) global [16 x i32] zeroinitializer, align 16
@opencl_reorderData_local_sKeys2 = internal addrspace(3) global [256 x <2 x i32>] zeroinitializer, align 16
@opencl_reorderData_local_sValues2 = internal addrspace(3) global [256 x <2 x i32>] zeroinitializer, align 16
@opencl_reorderData_local_sOffsets = internal addrspace(3) global [16 x i32] zeroinitializer, align 16
@opencl_reorderData_local_sBlockOffsets = internal addrspace(3) global [16 x i32] zeroinitializer, align 16
@opencl_addUniform_local_uni.0 = internal addrspace(3) global i32 0
@opencl_scan_local_s_data = internal addrspace(3) global [512 x i32] zeroinitializer, align 16

declare void @__radixSortBlocks_original(i32, i32, <4 x i32> addrspace(1)* nocapture, <4 x i32> addrspace(1)* nocapture, <4 x i32> addrspace(1)* nocapture, <4 x i32> addrspace(1)* nocapture, i32 addrspace(3)* nocapture) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_local_id(i32)

declare i64 @get_local_size(i32)

declare fastcc <4 x i32> @__scan4_original(<4 x i32>, i32 addrspace(3)* nocapture) nounwind inlinehint

declare void @barrier(i64)

declare void @__findRadixOffsets_original(<2 x i32> addrspace(1)* nocapture, i32 addrspace(1)* nocapture, i32 addrspace(1)* nocapture, i32, i32, i32, i32 addrspace(3)* nocapture) nounwind

declare i64 @get_group_id(i32)

declare void @__reorderData_original(i32, i32 addrspace(1)* nocapture, i32 addrspace(1)* nocapture, <2 x i32> addrspace(1)* nocapture, <2 x i32> addrspace(1)* nocapture, i32 addrspace(1)* nocapture, i32 addrspace(1)* nocapture, i32 addrspace(1)* nocapture, i32) nounwind

declare void @__addUniform_original(i32 addrspace(1)* nocapture, i32 addrspace(1)* nocapture, i32) nounwind

declare void @__scan_original(i32 addrspace(1)* nocapture, i32 addrspace(1)* nocapture, i32 addrspace(1)* nocapture, i32, i32, i32) nounwind

declare fastcc i32 @__scanLocalMem_original(i32) nounwind inlinehint

declare fastcc i32 @__scanLSB_original(i32, i32 addrspace(3)* nocapture) nounwind inlinehint

declare void @dummybarrier.()

declare i64* @get_curr_wi.()

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_local_id.(i32, i64)

declare i64 @get_new_global_id.(i32, i64)

declare fastcc i32 @__scanLocalMem_New_original(i32, i64, i64) nounwind inlinehint

declare fastcc i32 @__scanLSB_New_original(i32, i32 addrspace(3)* nocapture, i64) nounwind inlinehint

declare fastcc <4 x i32> @__scan4_New_original(<4 x i32>, i32 addrspace(3)* nocapture, i64, i64) nounwind inlinehint

define void @__radixSortBlocks_separated_args(i32 %nbits, i32 %startbit, <4 x i32> addrspace(1)* nocapture %keysOut, <4 x i32> addrspace(1)* nocapture %valuesOut, <4 x i32> addrspace(1)* nocapture %keysIn, <4 x i32> addrspace(1)* nocapture %valuesIn, i32 addrspace(3)* nocapture %sMem, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = bitcast i8 addrspace(3)* %pLocalMem to i32 addrspace(3)*
  store i64 0, i64* %pCurrWI, align 8
  %1 = add i32 %startbit, %nbits
  %2 = icmp ugt i32 %1, %startbit
  br label %SyncBB471

SyncBB471:                                        ; preds = %thenBB502, %thenBB509, %FirstBB
  %currBarrier.3 = phi i32 [ 65, %FirstBB ], [ %currBarrier.2, %thenBB502 ], [ %currBarrier.1, %thenBB509 ]
  %CurrSBIndex..4 = phi i64 [ 0, %FirstBB ], [ %"loadedCurrSB+Stride508", %thenBB502 ], [ %"loadedCurrSB+Stride515", %thenBB509 ]
  %currWI517 = load i64* %pCurrWI, align 8
  %3 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %currWI517, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %6 = load i64* %5, align 8
  %7 = add i64 %4, %6
  %8 = trunc i64 %4 to i32
  %9 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %10 = load i64* %9, align 8
  %11 = trunc i64 %10 to i32
  %12 = and i64 %7, 4294967295
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..4
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i64*
  store i64 %12, i64* %CastToValueType, align 8
  %13 = getelementptr inbounds <4 x i32> addrspace(1)* %keysIn, i64 %12
  %14 = load <4 x i32> addrspace(1)* %13, align 16
  %"&(pSB[currWI].offset)29" = add nuw i64 %CurrSBIndex..4, 16
  %"&pSB[currWI].offset30" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)29"
  %CastToValueType31 = bitcast i8* %"&pSB[currWI].offset30" to <4 x i32>*
  store <4 x i32> %14, <4 x i32>* %CastToValueType31, align 16
  %15 = getelementptr inbounds <4 x i32> addrspace(1)* %valuesIn, i64 %12
  %16 = load <4 x i32> addrspace(1)* %15, align 16
  %"&(pSB[currWI].offset)38" = add nuw i64 %CurrSBIndex..4, 32
  %"&pSB[currWI].offset39" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)38"
  %CastToValueType40 = bitcast i8* %"&pSB[currWI].offset39" to <4 x i32>*
  store <4 x i32> %16, <4 x i32>* %CastToValueType40, align 16
  br i1 %2, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB471
  %17 = add i32 %11, -1
  %18 = icmp eq i32 %8, %17
  %"&(pSB[currWI].offset)47" = add nuw i64 %CurrSBIndex..4, 48
  %"&pSB[currWI].offset48" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)47"
  %CastToValueType49 = bitcast i8* %"&pSB[currWI].offset48" to i1*
  store i1 %18, i1* %CastToValueType49, align 1
  %19 = shl i32 %8, 2
  %"&(pSB[currWI].offset)56" = add nuw i64 %CurrSBIndex..4, 52
  %"&pSB[currWI].offset57" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)56"
  %CastToValueType58 = bitcast i8* %"&pSB[currWI].offset57" to i32*
  store i32 %19, i32* %CastToValueType58, align 4
  %20 = and i64 %4, 4294967295
  %21 = getelementptr inbounds i32 addrspace(3)* %sMem, i64 %20
  %"&(pSB[currWI].offset)65" = add nuw i64 %CurrSBIndex..4, 56
  %"&pSB[currWI].offset66" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)65"
  %CastToValueType67 = bitcast i8* %"&pSB[currWI].offset66" to i32 addrspace(3)**
  store i32 addrspace(3)* %21, i32 addrspace(3)** %CastToValueType67, align 8
  %22 = add i32 %11, %8
  %23 = zext i32 %22 to i64
  %24 = getelementptr inbounds i32 addrspace(3)* %sMem, i64 %23
  %"&(pSB[currWI].offset)79" = add nuw i64 %CurrSBIndex..4, 64
  %"&pSB[currWI].offset80" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)79"
  %CastToValueType81 = bitcast i8* %"&pSB[currWI].offset80" to i32 addrspace(3)**
  store i32 addrspace(3)* %24, i32 addrspace(3)** %CastToValueType81, align 8
  %25 = shl i32 %11, 1
  %26 = add i32 %25, %8
  %27 = zext i32 %26 to i64
  %28 = getelementptr inbounds i32 addrspace(3)* %sMem, i64 %27
  %"&(pSB[currWI].offset)93" = add nuw i64 %CurrSBIndex..4, 72
  %"&pSB[currWI].offset94" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)93"
  %CastToValueType95 = bitcast i8* %"&pSB[currWI].offset94" to i32 addrspace(3)**
  store i32 addrspace(3)* %28, i32 addrspace(3)** %CastToValueType95, align 8
  %29 = mul i32 %11, 3
  %30 = add i32 %29, %8
  %31 = zext i32 %30 to i64
  %32 = getelementptr inbounds i32 addrspace(3)* %sMem, i64 %31
  %"&(pSB[currWI].offset)107" = add nuw i64 %CurrSBIndex..4, 80
  %"&pSB[currWI].offset108" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)107"
  %CastToValueType109 = bitcast i8* %"&pSB[currWI].offset108" to i32 addrspace(3)**
  store i32 addrspace(3)* %32, i32 addrspace(3)** %CastToValueType109, align 8
  %33 = or i32 %19, 3
  %"&(pSB[currWI].offset)121" = add nuw i64 %CurrSBIndex..4, 88
  %"&pSB[currWI].offset122" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)121"
  %CastToValueType123 = bitcast i8* %"&pSB[currWI].offset122" to i32*
  store i32 %33, i32* %CastToValueType123, align 4
  %34 = or i32 %19, 2
  %"&(pSB[currWI].offset)130" = add nuw i64 %CurrSBIndex..4, 92
  %"&pSB[currWI].offset131" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)130"
  %CastToValueType132 = bitcast i8* %"&pSB[currWI].offset131" to i32*
  store i32 %34, i32* %CastToValueType132, align 4
  %35 = or i32 %19, 1
  %"&(pSB[currWI].offset)139" = add nuw i64 %CurrSBIndex..4, 96
  %"&pSB[currWI].offset140" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)139"
  %CastToValueType141 = bitcast i8* %"&pSB[currWI].offset140" to i32*
  store i32 %35, i32* %CastToValueType141, align 4
  %"&(pSB[currWI].offset)33" = add nuw i64 %CurrSBIndex..4, 16
  %"&(pSB[currWI].offset)42" = add nuw i64 %CurrSBIndex..4, 32
  br label %"Barrier BB17"

"Barrier BB17":                                   ; preds = %SyncBB468, %bb.nph
  %currBarrier.1 = phi i32 [ %currBarrier.3, %bb.nph ], [ %currBarrier.0, %SyncBB468 ]
  %CurrSBIndex..2 = phi i64 [ %CurrSBIndex..4, %bb.nph ], [ %CurrSBIndex..1, %SyncBB468 ]
  %indvar = phi i32 [ 0, %bb.nph ], [ %indvar.next, %SyncBB468 ]
  %"&(pSB[currWI].offset)42.pn" = phi i64 [ %"&(pSB[currWI].offset)42", %bb.nph ], [ %"&(pSB[currWI].offset)449", %SyncBB468 ]
  %"&(pSB[currWI].offset)33.pn" = phi i64 [ %"&(pSB[currWI].offset)33", %bb.nph ], [ %"&(pSB[currWI].offset)435", %SyncBB468 ]
  %key.07.in.in = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)33.pn"
  %value.08.in.in = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)42.pn"
  %key.07.in = bitcast i8* %key.07.in.in to <4 x i32>*
  %value.08.in = bitcast i8* %value.08.in.in to <4 x i32>*
  %key.07 = load <4 x i32>* %key.07.in, align 16
  %value.08 = load <4 x i32>* %value.08.in, align 16
  %"&(pSB[currWI].offset)157" = add nuw i64 %CurrSBIndex..2, 112
  %"&pSB[currWI].offset158" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)157"
  %CastToValueType159 = bitcast i8* %"&pSB[currWI].offset158" to <4 x i32>*
  store <4 x i32> %value.08, <4 x i32>* %CastToValueType159, align 16
  %"&(pSB[currWI].offset)148" = add nuw i64 %CurrSBIndex..2, 100
  %"&pSB[currWI].offset149" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)148"
  %CastToValueType150 = bitcast i8* %"&pSB[currWI].offset149" to i32*
  store i32 %indvar, i32* %CastToValueType150, align 4
  %shift.09 = add i32 %indvar, %startbit
  %36 = extractelement <4 x i32> %key.07, i32 0
  %"&(pSB[currWI].offset)181" = add nuw i64 %CurrSBIndex..2, 128
  %"&pSB[currWI].offset182" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)181"
  %CastToValueType183 = bitcast i8* %"&pSB[currWI].offset182" to i32*
  store i32 %36, i32* %CastToValueType183, align 4
  %tmp = shl i32 1, %shift.09
  %37 = and i32 %tmp, %36
  %38 = icmp eq i32 %37, 0
  %"&(pSB[currWI].offset)190" = add nuw i64 %CurrSBIndex..2, 132
  %"&pSB[currWI].offset191" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)190"
  %CastToValueType192 = bitcast i8* %"&pSB[currWI].offset191" to i1*
  store i1 %38, i1* %CastToValueType192, align 1
  %39 = zext i1 %38 to i32
  %40 = insertelement <4 x i32> undef, i32 %39, i32 0
  %41 = extractelement <4 x i32> %key.07, i32 1
  %"&(pSB[currWI].offset)199" = add nuw i64 %CurrSBIndex..2, 136
  %"&pSB[currWI].offset200" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)199"
  %CastToValueType201 = bitcast i8* %"&pSB[currWI].offset200" to i32*
  store i32 %41, i32* %CastToValueType201, align 4
  %42 = and i32 %tmp, %41
  %43 = icmp eq i32 %42, 0
  %"&(pSB[currWI].offset)208" = add nuw i64 %CurrSBIndex..2, 140
  %"&pSB[currWI].offset209" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)208"
  %CastToValueType210 = bitcast i8* %"&pSB[currWI].offset209" to i1*
  store i1 %43, i1* %CastToValueType210, align 1
  %44 = zext i1 %43 to i32
  %45 = insertelement <4 x i32> %40, i32 %44, i32 1
  %46 = extractelement <4 x i32> %key.07, i32 2
  %"&(pSB[currWI].offset)217" = add nuw i64 %CurrSBIndex..2, 144
  %"&pSB[currWI].offset218" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)217"
  %CastToValueType219 = bitcast i8* %"&pSB[currWI].offset218" to i32*
  store i32 %46, i32* %CastToValueType219, align 4
  %47 = and i32 %tmp, %46
  %48 = icmp eq i32 %47, 0
  %"&(pSB[currWI].offset)226" = add nuw i64 %CurrSBIndex..2, 148
  %"&pSB[currWI].offset227" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)226"
  %CastToValueType228 = bitcast i8* %"&pSB[currWI].offset227" to i1*
  store i1 %48, i1* %CastToValueType228, align 1
  %49 = zext i1 %48 to i32
  %50 = insertelement <4 x i32> %45, i32 %49, i32 2
  %51 = extractelement <4 x i32> %key.07, i32 3
  %"&(pSB[currWI].offset)235" = add nuw i64 %CurrSBIndex..2, 152
  %"&pSB[currWI].offset236" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)235"
  %CastToValueType237 = bitcast i8* %"&pSB[currWI].offset236" to i32*
  store i32 %51, i32* %CastToValueType237, align 4
  %52 = and i32 %tmp, %51
  %53 = icmp eq i32 %52, 0
  %"&(pSB[currWI].offset)244" = add nuw i64 %CurrSBIndex..2, 156
  %"&pSB[currWI].offset245" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)244"
  %CastToValueType246 = bitcast i8* %"&pSB[currWI].offset245" to i1*
  store i1 %53, i1* %CastToValueType246, align 1
  %54 = zext i1 %53 to i32
  %"&(pSB[currWI].offset)253" = add nuw i64 %CurrSBIndex..2, 160
  %"&pSB[currWI].offset254" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)253"
  %CastToValueType255 = bitcast i8* %"&pSB[currWI].offset254" to i32*
  store i32 %54, i32* %CastToValueType255, align 4
  %55 = insertelement <4 x i32> %50, i32 %54, i32 3
  %"&(pSB[currWI].offset)262" = add nuw i64 %CurrSBIndex..2, 176
  %"&pSB[currWI].offset263" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)262"
  %CastToValueType264 = bitcast i8* %"&pSB[currWI].offset263" to <4 x i32>*
  store <4 x i32> %55, <4 x i32>* %CastToValueType264, align 16
  %loadedCurrWI511 = load i64* %pCurrWI, align 8
  %check.WI.iter512 = icmp ult i64 %loadedCurrWI511, %iterCount
  br i1 %check.WI.iter512, label %thenBB509, label %elseBB510

thenBB509:                                        ; preds = %"Barrier BB17"
  %"CurrWI++513" = add nuw i64 %loadedCurrWI511, 1
  store i64 %"CurrWI++513", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride515" = add nuw i64 %CurrSBIndex..2, 560
  %cond2 = icmp eq i32 %currBarrier.1, 65
  br i1 %cond2, label %SyncBB471, label %SyncBB468

elseBB510:                                        ; preds = %"Barrier BB17"
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB22.i

SyncBB22.i:                                       ; preds = %thenBB26.i, %elseBB510
  %CurrSBIndex..0.i = phi i64 [ 0, %elseBB510 ], [ %"loadedCurrSB+Stride32.i", %thenBB26.i ]
  %loadedCurrWI28.i = load i64* %pCurrWI, align 8
  %check.WI.iter29.i = icmp ult i64 %loadedCurrWI28.i, %iterCount
  br i1 %check.WI.iter29.i, label %thenBB26.i, label %elseBB27.i

thenBB26.i:                                       ; preds = %SyncBB22.i
  %"CurrWI++30.i" = add nuw i64 %loadedCurrWI28.i, 1
  store i64 %"CurrWI++30.i", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride32.i" = add nuw i64 %CurrSBIndex..0.i, 560
  br label %SyncBB22.i

elseBB27.i:                                       ; preds = %SyncBB22.i
  %"&(pSB[currWI].offset)5.i" = add nuw i64 %CurrSBIndex..0.i, 176
  %"&(pSB[currWI].offset)10.i" = add nuw i64 %CurrSBIndex..0.i, 176
  %"&(pSB[currWI].offset)15.i" = add nuw i64 %CurrSBIndex..0.i, 176
  %"&(pSB[currWI].offset)24.i" = add nuw i64 %CurrSBIndex..0.i, 176
  %"&pSB[currWI].offset6.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)5.i"
  %"&pSB[currWI].offset11.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)10.i"
  %"&pSB[currWI].offset16.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)15.i"
  %"&pSB[currWI].offset25.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)24.i"
  %CastToArgType.i = bitcast i8* %"&pSB[currWI].offset6.i" to <4 x i32>*
  %CastToArgType12.i = bitcast i8* %"&pSB[currWI].offset11.i" to <4 x i32>*
  %CastToArgType17.i = bitcast i8* %"&pSB[currWI].offset16.i" to <4 x i32>*
  %CastToArgType26.i = bitcast i8* %"&pSB[currWI].offset25.i" to <4 x i32>*
  %loadedValue8.i = load <4 x i32>* %CastToArgType.i, align 16
  %loadedValue13.i = load <4 x i32>* %CastToArgType12.i, align 16
  %loadedValue22.i = load <4 x i32>* %CastToArgType17.i, align 16
  %loadedValue27.i = load <4 x i32>* %CastToArgType26.i, align 16
  %56 = extractelement <4 x i32> %loadedValue27.i, i32 1
  %57 = extractelement <4 x i32> %loadedValue22.i, i32 0
  %58 = add i32 %56, %57
  %59 = extractelement <4 x i32> %loadedValue13.i, i32 2
  %60 = add i32 %58, %59
  store i64 0, i64* %pCurrWI, align 8
  %61 = extractelement <4 x i32> %loadedValue8.i, i32 3
  %62 = add i32 %60, %61
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB220.i, %elseBB27.i
  %CurrWI..0.i = phi i64 [ %"CurrWI++224.i", %thenBB220.i ], [ 0, %elseBB27.i ]
  %CurrSBIndex..0.i3 = phi i64 [ %"loadedCurrSB+Stride226.i", %thenBB220.i ], [ 0, %elseBB27.i ]
  %63 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0.i, i32 0, i64 0
  %64 = load i64* %63, align 8
  %"&(pSB[currWI].offset).i4" = add nuw i64 %CurrSBIndex..0.i3, 504
  %"&pSB[currWI].offset.i5" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset).i4"
  %CastToValueType.i6 = bitcast i8* %"&pSB[currWI].offset.i5" to i64*
  store i64 %64, i64* %CastToValueType.i6, align 8
  %sext.i = shl i64 %64, 32
  %65 = ashr i64 %sext.i, 32
  %66 = getelementptr inbounds i32 addrspace(3)* %sMem, i64 %65
  store i32 0, i32 addrspace(3)* %66, align 4
  %check.WI.iter223.i = icmp ult i64 %CurrWI..0.i, %iterCount
  br i1 %check.WI.iter223.i, label %thenBB220.i, label %SyncBB203.i

thenBB220.i:                                      ; preds = %SyncBB.i
  %"CurrWI++224.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride226.i" = add nuw i64 %CurrSBIndex..0.i3, 560
  br label %SyncBB.i

SyncBB203.i:                                      ; preds = %thenBB227.i, %SyncBB.i
  %CurrWI..1.i = phi i64 [ %"CurrWI++231.i", %thenBB227.i ], [ 0, %SyncBB.i ]
  %CurrSBIndex..1.i7 = phi i64 [ %"loadedCurrSB+Stride233.i", %thenBB227.i ], [ 0, %SyncBB.i ]
  %67 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %68 = load i64* %67, align 8
  %"&(pSB[currWI].offset)18.i" = add nuw i64 %CurrSBIndex..1.i7, 504
  %"&pSB[currWI].offset19.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)18.i"
  %CastToValueType20.i = bitcast i8* %"&pSB[currWI].offset19.i" to i64*
  %loadedValue.i8 = load i64* %CastToValueType20.i, align 8
  %69 = add i64 %68, %loadedValue.i8
  %70 = trunc i64 %69 to i32
  %"&(pSB[currWI].offset)22.i" = add nuw i64 %CurrSBIndex..1.i7, 512
  %"&pSB[currWI].offset23.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)22.i"
  %CastToValueType24.i = bitcast i8* %"&pSB[currWI].offset23.i" to i32*
  store i32 %70, i32* %CastToValueType24.i, align 4
  %71 = sext i32 %70 to i64
  %72 = getelementptr inbounds i32 addrspace(3)* %sMem, i64 %71
  %"&(pSB[currWI].offset)61.i" = add nuw i64 %CurrSBIndex..1.i7, 520
  %"&pSB[currWI].offset62.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)61.i"
  %CastToValueType63.i = bitcast i8* %"&pSB[currWI].offset62.i" to i32 addrspace(3)**
  store i32 addrspace(3)* %72, i32 addrspace(3)** %CastToValueType63.i, align 8
  store i32 %62, i32 addrspace(3)* %72, align 4
  %check.WI.iter230.i = icmp ult i64 %CurrWI..1.i, %iterCount
  br i1 %check.WI.iter230.i, label %thenBB227.i, label %SyncBB204.i

thenBB227.i:                                      ; preds = %SyncBB203.i
  %"CurrWI++231.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride233.i" = add nuw i64 %CurrSBIndex..1.i7, 560
  br label %SyncBB203.i

SyncBB204.i:                                      ; preds = %thenBB234.i, %SyncBB203.i
  %CurrWI..2.i = phi i64 [ %"CurrWI++238.i", %thenBB234.i ], [ 0, %SyncBB203.i ]
  %CurrSBIndex..2.i = phi i64 [ %"loadedCurrSB+Stride240.i", %thenBB234.i ], [ 0, %SyncBB203.i ]
  %"&(pSB[currWI].offset)56.i" = add nuw i64 %CurrSBIndex..2.i, 512
  %"&pSB[currWI].offset57.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)56.i"
  %CastToValueType58.i = bitcast i8* %"&pSB[currWI].offset57.i" to i32*
  %loadedValue59.i = load i32* %CastToValueType58.i, align 4
  %73 = add nsw i32 %loadedValue59.i, -1
  %74 = sext i32 %73 to i64
  %75 = getelementptr inbounds i32 addrspace(3)* %sMem, i64 %74
  %76 = load i32 addrspace(3)* %75, align 4
  %"&(pSB[currWI].offset)140.i" = add nuw i64 %CurrSBIndex..2.i, 528
  %"&pSB[currWI].offset141.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)140.i"
  %CastToValueType142.i = bitcast i8* %"&pSB[currWI].offset141.i" to i32*
  store i32 %76, i32* %CastToValueType142.i, align 4
  %check.WI.iter237.i = icmp ult i64 %CurrWI..2.i, %iterCount
  br i1 %check.WI.iter237.i, label %thenBB234.i, label %SyncBB205.i

thenBB234.i:                                      ; preds = %SyncBB204.i
  %"CurrWI++238.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride240.i" = add nuw i64 %CurrSBIndex..2.i, 560
  br label %SyncBB204.i

SyncBB205.i:                                      ; preds = %thenBB241.i, %SyncBB204.i
  %CurrWI..3.i = phi i64 [ %"CurrWI++245.i", %thenBB241.i ], [ 0, %SyncBB204.i ]
  %CurrSBIndex..3.i = phi i64 [ %"loadedCurrSB+Stride247.i", %thenBB241.i ], [ 0, %SyncBB204.i ]
  %"&(pSB[currWI].offset)130.i" = add nuw i64 %CurrSBIndex..3.i, 520
  %"&pSB[currWI].offset131.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)130.i"
  %CastToValueType132.i = bitcast i8* %"&pSB[currWI].offset131.i" to i32 addrspace(3)**
  %loadedValue133.i = load i32 addrspace(3)** %CastToValueType132.i, align 8
  %77 = load i32 addrspace(3)* %loadedValue133.i, align 4
  %"&(pSB[currWI].offset)144.i" = add nuw i64 %CurrSBIndex..3.i, 528
  %"&pSB[currWI].offset145.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)144.i"
  %CastToValueType146.i = bitcast i8* %"&pSB[currWI].offset145.i" to i32*
  %loadedValue147.i = load i32* %CastToValueType146.i, align 4
  %78 = add i32 %77, %loadedValue147.i
  %"&(pSB[currWI].offset)135.i" = add nuw i64 %CurrSBIndex..3.i, 520
  %"&pSB[currWI].offset136.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)135.i"
  %CastToValueType137.i = bitcast i8* %"&pSB[currWI].offset136.i" to i32 addrspace(3)**
  %loadedValue138.i = load i32 addrspace(3)** %CastToValueType137.i, align 8
  store i32 %78, i32 addrspace(3)* %loadedValue138.i, align 4
  %check.WI.iter244.i = icmp ult i64 %CurrWI..3.i, %iterCount
  br i1 %check.WI.iter244.i, label %thenBB241.i, label %SyncBB206.i

thenBB241.i:                                      ; preds = %SyncBB205.i
  %"CurrWI++245.i" = add nuw i64 %CurrWI..3.i, 1
  %"loadedCurrSB+Stride247.i" = add nuw i64 %CurrSBIndex..3.i, 560
  br label %SyncBB205.i

SyncBB206.i:                                      ; preds = %thenBB248.i, %SyncBB205.i
  %CurrWI..4.i = phi i64 [ %"CurrWI++252.i", %thenBB248.i ], [ 0, %SyncBB205.i ]
  %CurrSBIndex..4.i = phi i64 [ %"loadedCurrSB+Stride254.i", %thenBB248.i ], [ 0, %SyncBB205.i ]
  %"&(pSB[currWI].offset)51.i" = add nuw i64 %CurrSBIndex..4.i, 512
  %"&pSB[currWI].offset52.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)51.i"
  %CastToValueType53.i = bitcast i8* %"&pSB[currWI].offset52.i" to i32*
  %loadedValue54.i = load i32* %CastToValueType53.i, align 4
  %79 = add nsw i32 %loadedValue54.i, -2
  %80 = sext i32 %79 to i64
  %81 = getelementptr inbounds i32 addrspace(3)* %sMem, i64 %80
  %82 = load i32 addrspace(3)* %81, align 4
  %"&(pSB[currWI].offset)149.i" = add nuw i64 %CurrSBIndex..4.i, 532
  %"&pSB[currWI].offset150.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)149.i"
  %CastToValueType151.i = bitcast i8* %"&pSB[currWI].offset150.i" to i32*
  store i32 %82, i32* %CastToValueType151.i, align 4
  %check.WI.iter251.i = icmp ult i64 %CurrWI..4.i, %iterCount
  br i1 %check.WI.iter251.i, label %thenBB248.i, label %SyncBB207.i

thenBB248.i:                                      ; preds = %SyncBB206.i
  %"CurrWI++252.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride254.i" = add nuw i64 %CurrSBIndex..4.i, 560
  br label %SyncBB206.i

SyncBB207.i:                                      ; preds = %thenBB255.i, %SyncBB206.i
  %CurrWI..5.i = phi i64 [ %"CurrWI++259.i", %thenBB255.i ], [ 0, %SyncBB206.i ]
  %CurrSBIndex..5.i = phi i64 [ %"loadedCurrSB+Stride261.i", %thenBB255.i ], [ 0, %SyncBB206.i ]
  %"&(pSB[currWI].offset)120.i" = add nuw i64 %CurrSBIndex..5.i, 520
  %"&pSB[currWI].offset121.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)120.i"
  %CastToValueType122.i = bitcast i8* %"&pSB[currWI].offset121.i" to i32 addrspace(3)**
  %loadedValue123.i = load i32 addrspace(3)** %CastToValueType122.i, align 8
  %83 = load i32 addrspace(3)* %loadedValue123.i, align 4
  %"&(pSB[currWI].offset)153.i" = add nuw i64 %CurrSBIndex..5.i, 532
  %"&pSB[currWI].offset154.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)153.i"
  %CastToValueType155.i = bitcast i8* %"&pSB[currWI].offset154.i" to i32*
  %loadedValue156.i = load i32* %CastToValueType155.i, align 4
  %84 = add i32 %83, %loadedValue156.i
  %"&(pSB[currWI].offset)125.i" = add nuw i64 %CurrSBIndex..5.i, 520
  %"&pSB[currWI].offset126.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)125.i"
  %CastToValueType127.i = bitcast i8* %"&pSB[currWI].offset126.i" to i32 addrspace(3)**
  %loadedValue128.i = load i32 addrspace(3)** %CastToValueType127.i, align 8
  store i32 %84, i32 addrspace(3)* %loadedValue128.i, align 4
  %check.WI.iter258.i = icmp ult i64 %CurrWI..5.i, %iterCount
  br i1 %check.WI.iter258.i, label %thenBB255.i, label %SyncBB208.i

thenBB255.i:                                      ; preds = %SyncBB207.i
  %"CurrWI++259.i" = add nuw i64 %CurrWI..5.i, 1
  %"loadedCurrSB+Stride261.i" = add nuw i64 %CurrSBIndex..5.i, 560
  br label %SyncBB207.i

SyncBB208.i:                                      ; preds = %thenBB262.i, %SyncBB207.i
  %CurrWI..6.i = phi i64 [ %"CurrWI++266.i", %thenBB262.i ], [ 0, %SyncBB207.i ]
  %CurrSBIndex..6.i = phi i64 [ %"loadedCurrSB+Stride268.i", %thenBB262.i ], [ 0, %SyncBB207.i ]
  %"&(pSB[currWI].offset)46.i" = add nuw i64 %CurrSBIndex..6.i, 512
  %"&pSB[currWI].offset47.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)46.i"
  %CastToValueType48.i = bitcast i8* %"&pSB[currWI].offset47.i" to i32*
  %loadedValue49.i = load i32* %CastToValueType48.i, align 4
  %85 = add nsw i32 %loadedValue49.i, -4
  %86 = sext i32 %85 to i64
  %87 = getelementptr inbounds i32 addrspace(3)* %sMem, i64 %86
  %88 = load i32 addrspace(3)* %87, align 4
  %"&(pSB[currWI].offset)158.i" = add nuw i64 %CurrSBIndex..6.i, 536
  %"&pSB[currWI].offset159.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)158.i"
  %CastToValueType160.i = bitcast i8* %"&pSB[currWI].offset159.i" to i32*
  store i32 %88, i32* %CastToValueType160.i, align 4
  %check.WI.iter265.i = icmp ult i64 %CurrWI..6.i, %iterCount
  br i1 %check.WI.iter265.i, label %thenBB262.i, label %SyncBB209.i

thenBB262.i:                                      ; preds = %SyncBB208.i
  %"CurrWI++266.i" = add nuw i64 %CurrWI..6.i, 1
  %"loadedCurrSB+Stride268.i" = add nuw i64 %CurrSBIndex..6.i, 560
  br label %SyncBB208.i

SyncBB209.i:                                      ; preds = %thenBB269.i, %SyncBB208.i
  %CurrWI..7.i = phi i64 [ %"CurrWI++273.i", %thenBB269.i ], [ 0, %SyncBB208.i ]
  %CurrSBIndex..7.i = phi i64 [ %"loadedCurrSB+Stride275.i", %thenBB269.i ], [ 0, %SyncBB208.i ]
  %"&(pSB[currWI].offset)110.i" = add nuw i64 %CurrSBIndex..7.i, 520
  %"&pSB[currWI].offset111.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)110.i"
  %CastToValueType112.i = bitcast i8* %"&pSB[currWI].offset111.i" to i32 addrspace(3)**
  %loadedValue113.i = load i32 addrspace(3)** %CastToValueType112.i, align 8
  %89 = load i32 addrspace(3)* %loadedValue113.i, align 4
  %"&(pSB[currWI].offset)162.i" = add nuw i64 %CurrSBIndex..7.i, 536
  %"&pSB[currWI].offset163.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)162.i"
  %CastToValueType164.i = bitcast i8* %"&pSB[currWI].offset163.i" to i32*
  %loadedValue165.i = load i32* %CastToValueType164.i, align 4
  %90 = add i32 %89, %loadedValue165.i
  %"&(pSB[currWI].offset)115.i" = add nuw i64 %CurrSBIndex..7.i, 520
  %"&pSB[currWI].offset116.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)115.i"
  %CastToValueType117.i = bitcast i8* %"&pSB[currWI].offset116.i" to i32 addrspace(3)**
  %loadedValue118.i = load i32 addrspace(3)** %CastToValueType117.i, align 8
  store i32 %90, i32 addrspace(3)* %loadedValue118.i, align 4
  %check.WI.iter272.i = icmp ult i64 %CurrWI..7.i, %iterCount
  br i1 %check.WI.iter272.i, label %thenBB269.i, label %SyncBB210.i

thenBB269.i:                                      ; preds = %SyncBB209.i
  %"CurrWI++273.i" = add nuw i64 %CurrWI..7.i, 1
  %"loadedCurrSB+Stride275.i" = add nuw i64 %CurrSBIndex..7.i, 560
  br label %SyncBB209.i

SyncBB210.i:                                      ; preds = %thenBB276.i, %SyncBB209.i
  %CurrWI..8.i = phi i64 [ %"CurrWI++280.i", %thenBB276.i ], [ 0, %SyncBB209.i ]
  %CurrSBIndex..8.i = phi i64 [ %"loadedCurrSB+Stride282.i", %thenBB276.i ], [ 0, %SyncBB209.i ]
  %"&(pSB[currWI].offset)41.i" = add nuw i64 %CurrSBIndex..8.i, 512
  %"&pSB[currWI].offset42.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)41.i"
  %CastToValueType43.i = bitcast i8* %"&pSB[currWI].offset42.i" to i32*
  %loadedValue44.i = load i32* %CastToValueType43.i, align 4
  %91 = add nsw i32 %loadedValue44.i, -8
  %92 = sext i32 %91 to i64
  %93 = getelementptr inbounds i32 addrspace(3)* %sMem, i64 %92
  %94 = load i32 addrspace(3)* %93, align 4
  %"&(pSB[currWI].offset)167.i" = add nuw i64 %CurrSBIndex..8.i, 540
  %"&pSB[currWI].offset168.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)167.i"
  %CastToValueType169.i = bitcast i8* %"&pSB[currWI].offset168.i" to i32*
  store i32 %94, i32* %CastToValueType169.i, align 4
  %check.WI.iter279.i = icmp ult i64 %CurrWI..8.i, %iterCount
  br i1 %check.WI.iter279.i, label %thenBB276.i, label %SyncBB211.i

thenBB276.i:                                      ; preds = %SyncBB210.i
  %"CurrWI++280.i" = add nuw i64 %CurrWI..8.i, 1
  %"loadedCurrSB+Stride282.i" = add nuw i64 %CurrSBIndex..8.i, 560
  br label %SyncBB210.i

SyncBB211.i:                                      ; preds = %thenBB283.i, %SyncBB210.i
  %CurrWI..9.i = phi i64 [ %"CurrWI++287.i", %thenBB283.i ], [ 0, %SyncBB210.i ]
  %CurrSBIndex..9.i = phi i64 [ %"loadedCurrSB+Stride289.i", %thenBB283.i ], [ 0, %SyncBB210.i ]
  %"&(pSB[currWI].offset)100.i" = add nuw i64 %CurrSBIndex..9.i, 520
  %"&pSB[currWI].offset101.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)100.i"
  %CastToValueType102.i = bitcast i8* %"&pSB[currWI].offset101.i" to i32 addrspace(3)**
  %loadedValue103.i = load i32 addrspace(3)** %CastToValueType102.i, align 8
  %95 = load i32 addrspace(3)* %loadedValue103.i, align 4
  %"&(pSB[currWI].offset)171.i" = add nuw i64 %CurrSBIndex..9.i, 540
  %"&pSB[currWI].offset172.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)171.i"
  %CastToValueType173.i = bitcast i8* %"&pSB[currWI].offset172.i" to i32*
  %loadedValue174.i = load i32* %CastToValueType173.i, align 4
  %96 = add i32 %95, %loadedValue174.i
  %"&(pSB[currWI].offset)105.i" = add nuw i64 %CurrSBIndex..9.i, 520
  %"&pSB[currWI].offset106.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)105.i"
  %CastToValueType107.i = bitcast i8* %"&pSB[currWI].offset106.i" to i32 addrspace(3)**
  %loadedValue108.i = load i32 addrspace(3)** %CastToValueType107.i, align 8
  store i32 %96, i32 addrspace(3)* %loadedValue108.i, align 4
  %check.WI.iter286.i = icmp ult i64 %CurrWI..9.i, %iterCount
  br i1 %check.WI.iter286.i, label %thenBB283.i, label %SyncBB212.i

thenBB283.i:                                      ; preds = %SyncBB211.i
  %"CurrWI++287.i" = add nuw i64 %CurrWI..9.i, 1
  %"loadedCurrSB+Stride289.i" = add nuw i64 %CurrSBIndex..9.i, 560
  br label %SyncBB211.i

SyncBB212.i:                                      ; preds = %thenBB290.i, %SyncBB211.i
  %CurrWI..10.i = phi i64 [ %"CurrWI++294.i", %thenBB290.i ], [ 0, %SyncBB211.i ]
  %CurrSBIndex..10.i = phi i64 [ %"loadedCurrSB+Stride296.i", %thenBB290.i ], [ 0, %SyncBB211.i ]
  %"&(pSB[currWI].offset)36.i" = add nuw i64 %CurrSBIndex..10.i, 512
  %"&pSB[currWI].offset37.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)36.i"
  %CastToValueType38.i = bitcast i8* %"&pSB[currWI].offset37.i" to i32*
  %loadedValue39.i = load i32* %CastToValueType38.i, align 4
  %97 = add nsw i32 %loadedValue39.i, -16
  %98 = sext i32 %97 to i64
  %99 = getelementptr inbounds i32 addrspace(3)* %sMem, i64 %98
  %100 = load i32 addrspace(3)* %99, align 4
  %"&(pSB[currWI].offset)176.i" = add nuw i64 %CurrSBIndex..10.i, 544
  %"&pSB[currWI].offset177.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)176.i"
  %CastToValueType178.i = bitcast i8* %"&pSB[currWI].offset177.i" to i32*
  store i32 %100, i32* %CastToValueType178.i, align 4
  %check.WI.iter293.i = icmp ult i64 %CurrWI..10.i, %iterCount
  br i1 %check.WI.iter293.i, label %thenBB290.i, label %SyncBB213.i

thenBB290.i:                                      ; preds = %SyncBB212.i
  %"CurrWI++294.i" = add nuw i64 %CurrWI..10.i, 1
  %"loadedCurrSB+Stride296.i" = add nuw i64 %CurrSBIndex..10.i, 560
  br label %SyncBB212.i

SyncBB213.i:                                      ; preds = %thenBB297.i, %SyncBB212.i
  %CurrWI..11.i = phi i64 [ %"CurrWI++301.i", %thenBB297.i ], [ 0, %SyncBB212.i ]
  %CurrSBIndex..11.i = phi i64 [ %"loadedCurrSB+Stride303.i", %thenBB297.i ], [ 0, %SyncBB212.i ]
  %"&(pSB[currWI].offset)90.i" = add nuw i64 %CurrSBIndex..11.i, 520
  %"&pSB[currWI].offset91.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)90.i"
  %CastToValueType92.i = bitcast i8* %"&pSB[currWI].offset91.i" to i32 addrspace(3)**
  %loadedValue93.i = load i32 addrspace(3)** %CastToValueType92.i, align 8
  %101 = load i32 addrspace(3)* %loadedValue93.i, align 4
  %"&(pSB[currWI].offset)180.i" = add nuw i64 %CurrSBIndex..11.i, 544
  %"&pSB[currWI].offset181.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)180.i"
  %CastToValueType182.i = bitcast i8* %"&pSB[currWI].offset181.i" to i32*
  %loadedValue183.i = load i32* %CastToValueType182.i, align 4
  %102 = add i32 %101, %loadedValue183.i
  %"&(pSB[currWI].offset)95.i" = add nuw i64 %CurrSBIndex..11.i, 520
  %"&pSB[currWI].offset96.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)95.i"
  %CastToValueType97.i = bitcast i8* %"&pSB[currWI].offset96.i" to i32 addrspace(3)**
  %loadedValue98.i = load i32 addrspace(3)** %CastToValueType97.i, align 8
  store i32 %102, i32 addrspace(3)* %loadedValue98.i, align 4
  %check.WI.iter300.i = icmp ult i64 %CurrWI..11.i, %iterCount
  br i1 %check.WI.iter300.i, label %thenBB297.i, label %SyncBB214.i

thenBB297.i:                                      ; preds = %SyncBB213.i
  %"CurrWI++301.i" = add nuw i64 %CurrWI..11.i, 1
  %"loadedCurrSB+Stride303.i" = add nuw i64 %CurrSBIndex..11.i, 560
  br label %SyncBB213.i

SyncBB214.i:                                      ; preds = %thenBB304.i, %SyncBB213.i
  %CurrWI..12.i = phi i64 [ %"CurrWI++308.i", %thenBB304.i ], [ 0, %SyncBB213.i ]
  %CurrSBIndex..12.i = phi i64 [ %"loadedCurrSB+Stride310.i", %thenBB304.i ], [ 0, %SyncBB213.i ]
  %"&(pSB[currWI].offset)31.i" = add nuw i64 %CurrSBIndex..12.i, 512
  %"&pSB[currWI].offset32.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)31.i"
  %CastToValueType33.i = bitcast i8* %"&pSB[currWI].offset32.i" to i32*
  %loadedValue34.i = load i32* %CastToValueType33.i, align 4
  %103 = add nsw i32 %loadedValue34.i, -32
  %104 = sext i32 %103 to i64
  %105 = getelementptr inbounds i32 addrspace(3)* %sMem, i64 %104
  %106 = load i32 addrspace(3)* %105, align 4
  %"&(pSB[currWI].offset)185.i" = add nuw i64 %CurrSBIndex..12.i, 548
  %"&pSB[currWI].offset186.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)185.i"
  %CastToValueType187.i = bitcast i8* %"&pSB[currWI].offset186.i" to i32*
  store i32 %106, i32* %CastToValueType187.i, align 4
  %check.WI.iter307.i = icmp ult i64 %CurrWI..12.i, %iterCount
  br i1 %check.WI.iter307.i, label %thenBB304.i, label %SyncBB215.i

thenBB304.i:                                      ; preds = %SyncBB214.i
  %"CurrWI++308.i" = add nuw i64 %CurrWI..12.i, 1
  %"loadedCurrSB+Stride310.i" = add nuw i64 %CurrSBIndex..12.i, 560
  br label %SyncBB214.i

SyncBB215.i:                                      ; preds = %thenBB311.i, %SyncBB214.i
  %CurrWI..13.i = phi i64 [ %"CurrWI++315.i", %thenBB311.i ], [ 0, %SyncBB214.i ]
  %CurrSBIndex..13.i = phi i64 [ %"loadedCurrSB+Stride317.i", %thenBB311.i ], [ 0, %SyncBB214.i ]
  %"&(pSB[currWI].offset)80.i" = add nuw i64 %CurrSBIndex..13.i, 520
  %"&pSB[currWI].offset81.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)80.i"
  %CastToValueType82.i = bitcast i8* %"&pSB[currWI].offset81.i" to i32 addrspace(3)**
  %loadedValue83.i = load i32 addrspace(3)** %CastToValueType82.i, align 8
  %107 = load i32 addrspace(3)* %loadedValue83.i, align 4
  %"&(pSB[currWI].offset)189.i" = add nuw i64 %CurrSBIndex..13.i, 548
  %"&pSB[currWI].offset190.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)189.i"
  %CastToValueType191.i = bitcast i8* %"&pSB[currWI].offset190.i" to i32*
  %loadedValue192.i = load i32* %CastToValueType191.i, align 4
  %108 = add i32 %107, %loadedValue192.i
  %"&(pSB[currWI].offset)85.i" = add nuw i64 %CurrSBIndex..13.i, 520
  %"&pSB[currWI].offset86.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)85.i"
  %CastToValueType87.i = bitcast i8* %"&pSB[currWI].offset86.i" to i32 addrspace(3)**
  %loadedValue88.i = load i32 addrspace(3)** %CastToValueType87.i, align 8
  store i32 %108, i32 addrspace(3)* %loadedValue88.i, align 4
  %check.WI.iter314.i = icmp ult i64 %CurrWI..13.i, %iterCount
  br i1 %check.WI.iter314.i, label %thenBB311.i, label %SyncBB216.i

thenBB311.i:                                      ; preds = %SyncBB215.i
  %"CurrWI++315.i" = add nuw i64 %CurrWI..13.i, 1
  %"loadedCurrSB+Stride317.i" = add nuw i64 %CurrSBIndex..13.i, 560
  br label %SyncBB215.i

SyncBB216.i:                                      ; preds = %thenBB318.i, %SyncBB215.i
  %CurrWI..14.i = phi i64 [ %"CurrWI++322.i", %thenBB318.i ], [ 0, %SyncBB215.i ]
  %CurrSBIndex..14.i = phi i64 [ %"loadedCurrSB+Stride324.i", %thenBB318.i ], [ 0, %SyncBB215.i ]
  %"&(pSB[currWI].offset)26.i" = add nuw i64 %CurrSBIndex..14.i, 512
  %"&pSB[currWI].offset27.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)26.i"
  %CastToValueType28.i = bitcast i8* %"&pSB[currWI].offset27.i" to i32*
  %loadedValue29.i = load i32* %CastToValueType28.i, align 4
  %109 = add nsw i32 %loadedValue29.i, -64
  %110 = sext i32 %109 to i64
  %111 = getelementptr inbounds i32 addrspace(3)* %sMem, i64 %110
  %112 = load i32 addrspace(3)* %111, align 4
  %"&(pSB[currWI].offset)194.i" = add nuw i64 %CurrSBIndex..14.i, 552
  %"&pSB[currWI].offset195.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)194.i"
  %CastToValueType196.i = bitcast i8* %"&pSB[currWI].offset195.i" to i32*
  store i32 %112, i32* %CastToValueType196.i, align 4
  %check.WI.iter321.i = icmp ult i64 %CurrWI..14.i, %iterCount
  br i1 %check.WI.iter321.i, label %thenBB318.i, label %SyncBB217.i

thenBB318.i:                                      ; preds = %SyncBB216.i
  %"CurrWI++322.i" = add nuw i64 %CurrWI..14.i, 1
  %"loadedCurrSB+Stride324.i" = add nuw i64 %CurrSBIndex..14.i, 560
  br label %SyncBB216.i

SyncBB217.i:                                      ; preds = %thenBB325.i, %SyncBB216.i
  %CurrWI..15.i = phi i64 [ %"CurrWI++329.i", %thenBB325.i ], [ 0, %SyncBB216.i ]
  %CurrSBIndex..15.i = phi i64 [ %"loadedCurrSB+Stride331.i", %thenBB325.i ], [ 0, %SyncBB216.i ]
  %"&(pSB[currWI].offset)70.i" = add nuw i64 %CurrSBIndex..15.i, 520
  %"&pSB[currWI].offset71.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)70.i"
  %CastToValueType72.i = bitcast i8* %"&pSB[currWI].offset71.i" to i32 addrspace(3)**
  %loadedValue73.i = load i32 addrspace(3)** %CastToValueType72.i, align 8
  %113 = load i32 addrspace(3)* %loadedValue73.i, align 4
  %"&(pSB[currWI].offset)198.i" = add nuw i64 %CurrSBIndex..15.i, 552
  %"&pSB[currWI].offset199.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)198.i"
  %CastToValueType200.i = bitcast i8* %"&pSB[currWI].offset199.i" to i32*
  %loadedValue201.i = load i32* %CastToValueType200.i, align 4
  %114 = add i32 %113, %loadedValue201.i
  %"&(pSB[currWI].offset)75.i" = add nuw i64 %CurrSBIndex..15.i, 520
  %"&pSB[currWI].offset76.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)75.i"
  %CastToValueType77.i = bitcast i8* %"&pSB[currWI].offset76.i" to i32 addrspace(3)**
  %loadedValue78.i = load i32 addrspace(3)** %CastToValueType77.i, align 8
  store i32 %114, i32 addrspace(3)* %loadedValue78.i, align 4
  %check.WI.iter328.i = icmp ult i64 %CurrWI..15.i, %iterCount
  br i1 %check.WI.iter328.i, label %thenBB325.i, label %SyncBB218.i

thenBB325.i:                                      ; preds = %SyncBB217.i
  %"CurrWI++329.i" = add nuw i64 %CurrWI..15.i, 1
  %"loadedCurrSB+Stride331.i" = add nuw i64 %CurrSBIndex..15.i, 560
  br label %SyncBB217.i

SyncBB218.i:                                      ; preds = %thenBB.i13, %SyncBB217.i
  %CurrWI..16.i = phi i64 [ %"CurrWI++.i11", %thenBB.i13 ], [ 0, %SyncBB217.i ]
  %CurrSBIndex..16.i = phi i64 [ %"loadedCurrSB+Stride.i12", %thenBB.i13 ], [ 0, %SyncBB217.i ]
  %"&(pSB[currWI].offset)65.i" = add nuw i64 %CurrSBIndex..16.i, 520
  %"&pSB[currWI].offset66.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)65.i"
  %CastToValueType67.i = bitcast i8* %"&pSB[currWI].offset66.i" to i32 addrspace(3)**
  %loadedValue68.i = load i32 addrspace(3)** %CastToValueType67.i, align 8
  %115 = load i32 addrspace(3)* %loadedValue68.i, align 4
  %116 = sub i32 %115, %62
  %"&(pSB[currWI].offset)3.i" = add nuw i64 %CurrSBIndex..16.i, 324
  %"&pSB[currWI].offset4.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)3.i"
  %CastToArgType.i9 = bitcast i8* %"&pSB[currWI].offset4.i" to i32*
  store i32 %116, i32* %CastToArgType.i9, align 4
  %check.WI.iter.i10 = icmp ult i64 %CurrWI..16.i, %iterCount
  br i1 %check.WI.iter.i10, label %thenBB.i13, label %scanLSB_New.exit

thenBB.i13:                                       ; preds = %SyncBB218.i
  %"CurrWI++.i11" = add nuw i64 %CurrWI..16.i, 1
  %"loadedCurrSB+Stride.i12" = add nuw i64 %CurrSBIndex..16.i, 560
  br label %SyncBB218.i

scanLSB_New.exit:                                 ; preds = %SyncBB218.i
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB23.i

SyncBB23.i:                                       ; preds = %thenBB.i, %scanLSB_New.exit
  %CurrSBIndex..1.i = phi i64 [ 0, %scanLSB_New.exit ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %"&(pSB[currWI].offset)14.i" = add nuw i64 %CurrSBIndex..1.i, 324
  %"&pSB[currWI].offset15.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)14.i"
  %CastToValueType16.i = bitcast i8* %"&pSB[currWI].offset15.i" to i32*
  %loadedValue17.i = load i32* %CastToValueType16.i, align 4
  %117 = insertelement <4 x i32> undef, i32 %loadedValue17.i, i32 0
  %"&(pSB[currWI].offset)9.i" = add nuw i64 %CurrSBIndex..1.i, 324
  %"&pSB[currWI].offset10.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)9.i"
  %CastToValueType11.i = bitcast i8* %"&pSB[currWI].offset10.i" to i32*
  %loadedValue12.i = load i32* %CastToValueType11.i, align 4
  %118 = add i32 %loadedValue12.i, %57
  %119 = insertelement <4 x i32> %117, i32 %118, i32 1
  %"&(pSB[currWI].offset)4.i" = add nuw i64 %CurrSBIndex..1.i, 324
  %"&pSB[currWI].offset5.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)4.i"
  %CastToValueType6.i = bitcast i8* %"&pSB[currWI].offset5.i" to i32*
  %loadedValue7.i = load i32* %CastToValueType6.i, align 4
  %120 = add i32 %loadedValue7.i, %58
  %121 = insertelement <4 x i32> %119, i32 %120, i32 2
  %"&(pSB[currWI].offset).i" = add nuw i64 %CurrSBIndex..1.i, 324
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset).i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  %loadedValue.i = load i32* %CastToValueType.i, align 4
  %122 = add i32 %loadedValue.i, %60
  %123 = insertelement <4 x i32> %121, i32 %122, i32 3
  %"&(pSB[currWI].offset)29.i" = add nuw i64 %CurrSBIndex..1.i, 192
  %"&pSB[currWI].offset30.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)29.i"
  %CastToArgType31.i = bitcast i8* %"&pSB[currWI].offset30.i" to <4 x i32>*
  store <4 x i32> %123, <4 x i32>* %CastToArgType31.i, align 16
  %loadedCurrWI.i = load i64* %pCurrWI, align 8
  %check.WI.iter.i = icmp ult i64 %loadedCurrWI.i, %iterCount
  br i1 %check.WI.iter.i, label %thenBB.i, label %scan4_New.exit

thenBB.i:                                         ; preds = %SyncBB23.i
  %"CurrWI++.i" = add nuw i64 %loadedCurrWI.i, 1
  store i64 %"CurrWI++.i", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..1.i, 560
  br label %SyncBB23.i

scan4_New.exit:                                   ; preds = %SyncBB23.i
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB470

SyncBB470:                                        ; preds = %thenBB, %scan4_New.exit
  %CurrSBIndex..0 = phi i64 [ 0, %scan4_New.exit ], [ %"loadedCurrSB+Stride", %thenBB ]
  %"&(pSB[currWI].offset)51" = add nuw i64 %CurrSBIndex..0, 48
  %"&pSB[currWI].offset52" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)51"
  %CastToValueType53 = bitcast i8* %"&pSB[currWI].offset52" to i1*
  %loadedValue54 = load i1* %CastToValueType53, align 1
  br i1 %loadedValue54, label %124, label %127

; <label>:124                                     ; preds = %SyncBB470
  %"&(pSB[currWI].offset)271" = add nuw i64 %CurrSBIndex..0, 192
  %"&pSB[currWI].offset272" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)271"
  %CastToValueType273 = bitcast i8* %"&pSB[currWI].offset272" to <4 x i32>*
  %loadedValue274 = load <4 x i32>* %CastToValueType273, align 16
  %125 = extractelement <4 x i32> %loadedValue274, i32 3
  %"&(pSB[currWI].offset)257" = add nuw i64 %CurrSBIndex..0, 160
  %"&pSB[currWI].offset258" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)257"
  %CastToValueType259 = bitcast i8* %"&pSB[currWI].offset258" to i32*
  %loadedValue260 = load i32* %CastToValueType259, align 4
  %126 = add i32 %125, %loadedValue260
  store i32 %126, i32 addrspace(3)* %0, align 4
  br label %127

; <label>:127                                     ; preds = %124, %SyncBB470
  %loadedCurrWI = load i64* %pCurrWI, align 8
  %check.WI.iter = icmp ult i64 %loadedCurrWI, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %127
  %"CurrWI++" = add nuw i64 %loadedCurrWI, 1
  store i64 %"CurrWI++", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 560
  br label %SyncBB470

elseBB:                                           ; preds = %127
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB481, %elseBB
  %CurrSBIndex..6 = phi i64 [ 0, %elseBB ], [ %"loadedCurrSB+Stride487", %thenBB481 ]
  %"&(pSB[currWI].offset)194" = add nuw i64 %CurrSBIndex..6, 132
  %"&pSB[currWI].offset195" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)194"
  %CastToValueType196 = bitcast i8* %"&pSB[currWI].offset195" to i1*
  %loadedValue197 = load i1* %CastToValueType196, align 1
  br i1 %loadedValue197, label %128, label %130

; <label>:128                                     ; preds = %SyncBB
  %"&(pSB[currWI].offset)276" = add nuw i64 %CurrSBIndex..6, 192
  %"&pSB[currWI].offset277" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)276"
  %CastToValueType278 = bitcast i8* %"&pSB[currWI].offset277" to <4 x i32>*
  %loadedValue279 = load <4 x i32>* %CastToValueType278, align 16
  %129 = extractelement <4 x i32> %loadedValue279, i32 0
  %"&(pSB[currWI].offset)316" = add nuw i64 %CurrSBIndex..6, 208
  %"&pSB[currWI].offset317" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)316"
  %CastToValueType318 = bitcast i8* %"&pSB[currWI].offset317" to i32*
  store i32 %129, i32* %CastToValueType318, align 4
  br label %135

; <label>:130                                     ; preds = %SyncBB
  %131 = load i32 addrspace(3)* %0, align 4
  %"&(pSB[currWI].offset)281" = add nuw i64 %CurrSBIndex..6, 192
  %"&pSB[currWI].offset282" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)281"
  %CastToValueType283 = bitcast i8* %"&pSB[currWI].offset282" to <4 x i32>*
  %loadedValue284 = load <4 x i32>* %CastToValueType283, align 16
  %132 = extractelement <4 x i32> %loadedValue284, i32 0
  %"&(pSB[currWI].offset)60" = add nuw i64 %CurrSBIndex..6, 52
  %"&pSB[currWI].offset61" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)60"
  %CastToValueType62 = bitcast i8* %"&pSB[currWI].offset61" to i32*
  %loadedValue63 = load i32* %CastToValueType62, align 4
  %133 = sub i32 %loadedValue63, %132
  %134 = add i32 %133, %131
  %"&(pSB[currWI].offset)320" = add nuw i64 %CurrSBIndex..6, 212
  %"&pSB[currWI].offset321" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)320"
  %CastToValueType322 = bitcast i8* %"&pSB[currWI].offset321" to i32*
  store i32 %134, i32* %CastToValueType322, align 4
  br label %135

; <label>:135                                     ; preds = %130, %128
  %136 = phi i32 [ %129, %128 ], [ %134, %130 ]
  %"&(pSB[currWI].offset)324" = add nuw i64 %CurrSBIndex..6, 216
  %"&pSB[currWI].offset325" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)324"
  %CastToValueType326 = bitcast i8* %"&pSB[currWI].offset325" to i32*
  store i32 %136, i32* %CastToValueType326, align 4
  %"&(pSB[currWI].offset)212" = add nuw i64 %CurrSBIndex..6, 140
  %"&pSB[currWI].offset213" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)212"
  %CastToValueType214 = bitcast i8* %"&pSB[currWI].offset213" to i1*
  %loadedValue215 = load i1* %CastToValueType214, align 1
  br i1 %loadedValue215, label %137, label %139

; <label>:137                                     ; preds = %135
  %"&(pSB[currWI].offset)286" = add nuw i64 %CurrSBIndex..6, 192
  %"&pSB[currWI].offset287" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)286"
  %CastToValueType288 = bitcast i8* %"&pSB[currWI].offset287" to <4 x i32>*
  %loadedValue289 = load <4 x i32>* %CastToValueType288, align 16
  %138 = extractelement <4 x i32> %loadedValue289, i32 1
  %"&(pSB[currWI].offset)338" = add nuw i64 %CurrSBIndex..6, 220
  %"&pSB[currWI].offset339" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)338"
  %CastToValueType340 = bitcast i8* %"&pSB[currWI].offset339" to i32*
  store i32 %138, i32* %CastToValueType340, align 4
  br label %144

; <label>:139                                     ; preds = %135
  %140 = load i32 addrspace(3)* %0, align 4
  %"&(pSB[currWI].offset)291" = add nuw i64 %CurrSBIndex..6, 192
  %"&pSB[currWI].offset292" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)291"
  %CastToValueType293 = bitcast i8* %"&pSB[currWI].offset292" to <4 x i32>*
  %loadedValue294 = load <4 x i32>* %CastToValueType293, align 16
  %141 = extractelement <4 x i32> %loadedValue294, i32 1
  %"&(pSB[currWI].offset)143" = add nuw i64 %CurrSBIndex..6, 96
  %"&pSB[currWI].offset144" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)143"
  %CastToValueType145 = bitcast i8* %"&pSB[currWI].offset144" to i32*
  %loadedValue146 = load i32* %CastToValueType145, align 4
  %142 = sub i32 %loadedValue146, %141
  %143 = add i32 %142, %140
  %"&(pSB[currWI].offset)342" = add nuw i64 %CurrSBIndex..6, 224
  %"&pSB[currWI].offset343" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)342"
  %CastToValueType344 = bitcast i8* %"&pSB[currWI].offset343" to i32*
  store i32 %143, i32* %CastToValueType344, align 4
  br label %144

; <label>:144                                     ; preds = %139, %137
  %145 = phi i32 [ %138, %137 ], [ %143, %139 ]
  %"&(pSB[currWI].offset)346" = add nuw i64 %CurrSBIndex..6, 228
  %"&pSB[currWI].offset347" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)346"
  %CastToValueType348 = bitcast i8* %"&pSB[currWI].offset347" to i32*
  store i32 %145, i32* %CastToValueType348, align 4
  %"&(pSB[currWI].offset)230" = add nuw i64 %CurrSBIndex..6, 148
  %"&pSB[currWI].offset231" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)230"
  %CastToValueType232 = bitcast i8* %"&pSB[currWI].offset231" to i1*
  %loadedValue233 = load i1* %CastToValueType232, align 1
  br i1 %loadedValue233, label %146, label %148

; <label>:146                                     ; preds = %144
  %"&(pSB[currWI].offset)296" = add nuw i64 %CurrSBIndex..6, 192
  %"&pSB[currWI].offset297" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)296"
  %CastToValueType298 = bitcast i8* %"&pSB[currWI].offset297" to <4 x i32>*
  %loadedValue299 = load <4 x i32>* %CastToValueType298, align 16
  %147 = extractelement <4 x i32> %loadedValue299, i32 2
  %"&(pSB[currWI].offset)360" = add nuw i64 %CurrSBIndex..6, 232
  %"&pSB[currWI].offset361" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)360"
  %CastToValueType362 = bitcast i8* %"&pSB[currWI].offset361" to i32*
  store i32 %147, i32* %CastToValueType362, align 4
  br label %153

; <label>:148                                     ; preds = %144
  %149 = load i32 addrspace(3)* %0, align 4
  %"&(pSB[currWI].offset)301" = add nuw i64 %CurrSBIndex..6, 192
  %"&pSB[currWI].offset302" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)301"
  %CastToValueType303 = bitcast i8* %"&pSB[currWI].offset302" to <4 x i32>*
  %loadedValue304 = load <4 x i32>* %CastToValueType303, align 16
  %150 = extractelement <4 x i32> %loadedValue304, i32 2
  %"&(pSB[currWI].offset)134" = add nuw i64 %CurrSBIndex..6, 92
  %"&pSB[currWI].offset135" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)134"
  %CastToValueType136 = bitcast i8* %"&pSB[currWI].offset135" to i32*
  %loadedValue137 = load i32* %CastToValueType136, align 4
  %151 = sub i32 %loadedValue137, %150
  %152 = add i32 %151, %149
  %"&(pSB[currWI].offset)364" = add nuw i64 %CurrSBIndex..6, 236
  %"&pSB[currWI].offset365" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)364"
  %CastToValueType366 = bitcast i8* %"&pSB[currWI].offset365" to i32*
  store i32 %152, i32* %CastToValueType366, align 4
  br label %153

; <label>:153                                     ; preds = %148, %146
  %154 = phi i32 [ %147, %146 ], [ %152, %148 ]
  %"&(pSB[currWI].offset)368" = add nuw i64 %CurrSBIndex..6, 240
  %"&pSB[currWI].offset369" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)368"
  %CastToValueType370 = bitcast i8* %"&pSB[currWI].offset369" to i32*
  store i32 %154, i32* %CastToValueType370, align 4
  %"&(pSB[currWI].offset)248" = add nuw i64 %CurrSBIndex..6, 156
  %"&pSB[currWI].offset249" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)248"
  %CastToValueType250 = bitcast i8* %"&pSB[currWI].offset249" to i1*
  %loadedValue251 = load i1* %CastToValueType250, align 1
  br i1 %loadedValue251, label %155, label %157

; <label>:155                                     ; preds = %153
  %"&(pSB[currWI].offset)306" = add nuw i64 %CurrSBIndex..6, 192
  %"&pSB[currWI].offset307" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)306"
  %CastToValueType308 = bitcast i8* %"&pSB[currWI].offset307" to <4 x i32>*
  %loadedValue309 = load <4 x i32>* %CastToValueType308, align 16
  %156 = extractelement <4 x i32> %loadedValue309, i32 3
  %"&(pSB[currWI].offset)382" = add nuw i64 %CurrSBIndex..6, 244
  %"&pSB[currWI].offset383" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)382"
  %CastToValueType384 = bitcast i8* %"&pSB[currWI].offset383" to i32*
  store i32 %156, i32* %CastToValueType384, align 4
  br label %"Barrier BB13"

; <label>:157                                     ; preds = %153
  %158 = load i32 addrspace(3)* %0, align 4
  %"&(pSB[currWI].offset)311" = add nuw i64 %CurrSBIndex..6, 192
  %"&pSB[currWI].offset312" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)311"
  %CastToValueType313 = bitcast i8* %"&pSB[currWI].offset312" to <4 x i32>*
  %loadedValue314 = load <4 x i32>* %CastToValueType313, align 16
  %159 = extractelement <4 x i32> %loadedValue314, i32 3
  %"&(pSB[currWI].offset)125" = add nuw i64 %CurrSBIndex..6, 88
  %"&pSB[currWI].offset126" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)125"
  %CastToValueType127 = bitcast i8* %"&pSB[currWI].offset126" to i32*
  %loadedValue128 = load i32* %CastToValueType127, align 4
  %160 = sub i32 %loadedValue128, %159
  %161 = add i32 %160, %158
  %"&(pSB[currWI].offset)386" = add nuw i64 %CurrSBIndex..6, 248
  %"&pSB[currWI].offset387" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)386"
  %CastToValueType388 = bitcast i8* %"&pSB[currWI].offset387" to i32*
  store i32 %161, i32* %CastToValueType388, align 4
  br label %"Barrier BB13"

"Barrier BB13":                                   ; preds = %157, %155
  %162 = phi i32 [ %156, %155 ], [ %161, %157 ]
  %"&(pSB[currWI].offset)328" = add nuw i64 %CurrSBIndex..6, 216
  %"&pSB[currWI].offset329" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)328"
  %CastToValueType330 = bitcast i8* %"&pSB[currWI].offset329" to i32*
  %loadedValue331 = load i32* %CastToValueType330, align 4
  %163 = and i32 %loadedValue331, 3
  %164 = mul i32 %163, %11
  %"&(pSB[currWI].offset)333" = add nuw i64 %CurrSBIndex..6, 216
  %"&pSB[currWI].offset334" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)333"
  %CastToValueType335 = bitcast i8* %"&pSB[currWI].offset334" to i32*
  %loadedValue336 = load i32* %CastToValueType335, align 4
  %165 = lshr i32 %loadedValue336, 2
  %166 = add i32 %164, %165
  %167 = zext i32 %166 to i64
  %168 = getelementptr inbounds i32 addrspace(3)* %sMem, i64 %167
  %"&(pSB[currWI].offset)390" = add nuw i64 %CurrSBIndex..6, 256
  %"&pSB[currWI].offset391" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)390"
  %CastToValueType392 = bitcast i8* %"&pSB[currWI].offset391" to i32 addrspace(3)**
  store i32 addrspace(3)* %168, i32 addrspace(3)** %CastToValueType392, align 8
  %"&(pSB[currWI].offset)185" = add nuw i64 %CurrSBIndex..6, 128
  %"&pSB[currWI].offset186" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)185"
  %CastToValueType187 = bitcast i8* %"&pSB[currWI].offset186" to i32*
  %loadedValue188 = load i32* %CastToValueType187, align 4
  store i32 %loadedValue188, i32 addrspace(3)* %168, align 4
  %"&(pSB[currWI].offset)350" = add nuw i64 %CurrSBIndex..6, 228
  %"&pSB[currWI].offset351" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)350"
  %CastToValueType352 = bitcast i8* %"&pSB[currWI].offset351" to i32*
  %loadedValue353 = load i32* %CastToValueType352, align 4
  %169 = and i32 %loadedValue353, 3
  %170 = mul i32 %169, %11
  %"&(pSB[currWI].offset)355" = add nuw i64 %CurrSBIndex..6, 228
  %"&pSB[currWI].offset356" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)355"
  %CastToValueType357 = bitcast i8* %"&pSB[currWI].offset356" to i32*
  %loadedValue358 = load i32* %CastToValueType357, align 4
  %171 = lshr i32 %loadedValue358, 2
  %172 = add i32 %170, %171
  %173 = zext i32 %172 to i64
  %174 = getelementptr inbounds i32 addrspace(3)* %sMem, i64 %173
  %"&(pSB[currWI].offset)399" = add nuw i64 %CurrSBIndex..6, 264
  %"&pSB[currWI].offset400" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)399"
  %CastToValueType401 = bitcast i8* %"&pSB[currWI].offset400" to i32 addrspace(3)**
  store i32 addrspace(3)* %174, i32 addrspace(3)** %CastToValueType401, align 8
  %"&(pSB[currWI].offset)203" = add nuw i64 %CurrSBIndex..6, 136
  %"&pSB[currWI].offset204" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)203"
  %CastToValueType205 = bitcast i8* %"&pSB[currWI].offset204" to i32*
  %loadedValue206 = load i32* %CastToValueType205, align 4
  store i32 %loadedValue206, i32 addrspace(3)* %174, align 4
  %"&(pSB[currWI].offset)372" = add nuw i64 %CurrSBIndex..6, 240
  %"&pSB[currWI].offset373" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)372"
  %CastToValueType374 = bitcast i8* %"&pSB[currWI].offset373" to i32*
  %loadedValue375 = load i32* %CastToValueType374, align 4
  %175 = and i32 %loadedValue375, 3
  %176 = mul i32 %175, %11
  %"&(pSB[currWI].offset)377" = add nuw i64 %CurrSBIndex..6, 240
  %"&pSB[currWI].offset378" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)377"
  %CastToValueType379 = bitcast i8* %"&pSB[currWI].offset378" to i32*
  %loadedValue380 = load i32* %CastToValueType379, align 4
  %177 = lshr i32 %loadedValue380, 2
  %178 = add i32 %176, %177
  %179 = zext i32 %178 to i64
  %180 = getelementptr inbounds i32 addrspace(3)* %sMem, i64 %179
  %"&(pSB[currWI].offset)408" = add nuw i64 %CurrSBIndex..6, 272
  %"&pSB[currWI].offset409" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)408"
  %CastToValueType410 = bitcast i8* %"&pSB[currWI].offset409" to i32 addrspace(3)**
  store i32 addrspace(3)* %180, i32 addrspace(3)** %CastToValueType410, align 8
  %"&(pSB[currWI].offset)221" = add nuw i64 %CurrSBIndex..6, 144
  %"&pSB[currWI].offset222" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)221"
  %CastToValueType223 = bitcast i8* %"&pSB[currWI].offset222" to i32*
  %loadedValue224 = load i32* %CastToValueType223, align 4
  store i32 %loadedValue224, i32 addrspace(3)* %180, align 4
  %181 = and i32 %162, 3
  %182 = mul i32 %181, %11
  %183 = lshr i32 %162, 2
  %184 = add i32 %182, %183
  %185 = zext i32 %184 to i64
  %186 = getelementptr inbounds i32 addrspace(3)* %sMem, i64 %185
  %"&(pSB[currWI].offset)417" = add nuw i64 %CurrSBIndex..6, 280
  %"&pSB[currWI].offset418" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)417"
  %CastToValueType419 = bitcast i8* %"&pSB[currWI].offset418" to i32 addrspace(3)**
  store i32 addrspace(3)* %186, i32 addrspace(3)** %CastToValueType419, align 8
  %"&(pSB[currWI].offset)239" = add nuw i64 %CurrSBIndex..6, 152
  %"&pSB[currWI].offset240" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)239"
  %CastToValueType241 = bitcast i8* %"&pSB[currWI].offset240" to i32*
  %loadedValue242 = load i32* %CastToValueType241, align 4
  store i32 %loadedValue242, i32 addrspace(3)* %186, align 4
  %loadedCurrWI483 = load i64* %pCurrWI, align 8
  %check.WI.iter484 = icmp ult i64 %loadedCurrWI483, %iterCount
  br i1 %check.WI.iter484, label %thenBB481, label %elseBB482

thenBB481:                                        ; preds = %"Barrier BB13"
  %"CurrWI++485" = add nuw i64 %loadedCurrWI483, 1
  store i64 %"CurrWI++485", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride487" = add nuw i64 %CurrSBIndex..6, 560
  br label %SyncBB

elseBB482:                                        ; preds = %"Barrier BB13"
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB466

SyncBB466:                                        ; preds = %thenBB474, %elseBB482
  %CurrSBIndex..5 = phi i64 [ 0, %elseBB482 ], [ %"loadedCurrSB+Stride480", %thenBB474 ]
  %"&(pSB[currWI].offset)74" = add nuw i64 %CurrSBIndex..5, 56
  %"&pSB[currWI].offset75" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)74"
  %CastToValueType76 = bitcast i8* %"&pSB[currWI].offset75" to i32 addrspace(3)**
  %loadedValue77 = load i32 addrspace(3)** %CastToValueType76, align 8
  %187 = load i32 addrspace(3)* %loadedValue77, align 4
  %188 = insertelement <4 x i32> undef, i32 %187, i32 0
  %"&(pSB[currWI].offset)88" = add nuw i64 %CurrSBIndex..5, 64
  %"&pSB[currWI].offset89" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)88"
  %CastToValueType90 = bitcast i8* %"&pSB[currWI].offset89" to i32 addrspace(3)**
  %loadedValue91 = load i32 addrspace(3)** %CastToValueType90, align 8
  %189 = load i32 addrspace(3)* %loadedValue91, align 4
  %190 = insertelement <4 x i32> %188, i32 %189, i32 1
  %"&(pSB[currWI].offset)102" = add nuw i64 %CurrSBIndex..5, 72
  %"&pSB[currWI].offset103" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)102"
  %CastToValueType104 = bitcast i8* %"&pSB[currWI].offset103" to i32 addrspace(3)**
  %loadedValue105 = load i32 addrspace(3)** %CastToValueType104, align 8
  %191 = load i32 addrspace(3)* %loadedValue105, align 4
  %192 = insertelement <4 x i32> %190, i32 %191, i32 2
  %"&(pSB[currWI].offset)116" = add nuw i64 %CurrSBIndex..5, 80
  %"&pSB[currWI].offset117" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)116"
  %CastToValueType118 = bitcast i8* %"&pSB[currWI].offset117" to i32 addrspace(3)**
  %loadedValue119 = load i32 addrspace(3)** %CastToValueType118, align 8
  %193 = load i32 addrspace(3)* %loadedValue119, align 4
  %194 = insertelement <4 x i32> %192, i32 %193, i32 3
  %"&(pSB[currWI].offset)426" = add nuw i64 %CurrSBIndex..5, 288
  %"&pSB[currWI].offset427" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)426"
  %CastToValueType428 = bitcast i8* %"&pSB[currWI].offset427" to <4 x i32>*
  store <4 x i32> %194, <4 x i32>* %CastToValueType428, align 16
  %loadedCurrWI476 = load i64* %pCurrWI, align 8
  %check.WI.iter477 = icmp ult i64 %loadedCurrWI476, %iterCount
  br i1 %check.WI.iter477, label %thenBB474, label %elseBB475

thenBB474:                                        ; preds = %SyncBB466
  %"CurrWI++478" = add nuw i64 %loadedCurrWI476, 1
  store i64 %"CurrWI++478", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride480" = add nuw i64 %CurrSBIndex..5, 560
  br label %SyncBB466

elseBB475:                                        ; preds = %SyncBB466
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB465

SyncBB465:                                        ; preds = %thenBB488, %elseBB475
  %CurrSBIndex..7 = phi i64 [ 0, %elseBB475 ], [ %"loadedCurrSB+Stride494", %thenBB488 ]
  %"&(pSB[currWI].offset)161" = add nuw i64 %CurrSBIndex..7, 112
  %"&pSB[currWI].offset162" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)161"
  %CastToValueType163 = bitcast i8* %"&pSB[currWI].offset162" to <4 x i32>*
  %loadedValue164 = load <4 x i32>* %CastToValueType163, align 16
  %195 = extractelement <4 x i32> %loadedValue164, i32 0
  %"&(pSB[currWI].offset)394" = add nuw i64 %CurrSBIndex..7, 256
  %"&pSB[currWI].offset395" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)394"
  %CastToValueType396 = bitcast i8* %"&pSB[currWI].offset395" to i32 addrspace(3)**
  %loadedValue397 = load i32 addrspace(3)** %CastToValueType396, align 8
  store i32 %195, i32 addrspace(3)* %loadedValue397, align 4
  %"&(pSB[currWI].offset)166" = add nuw i64 %CurrSBIndex..7, 112
  %"&pSB[currWI].offset167" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)166"
  %CastToValueType168 = bitcast i8* %"&pSB[currWI].offset167" to <4 x i32>*
  %loadedValue169 = load <4 x i32>* %CastToValueType168, align 16
  %196 = extractelement <4 x i32> %loadedValue169, i32 1
  %"&(pSB[currWI].offset)403" = add nuw i64 %CurrSBIndex..7, 264
  %"&pSB[currWI].offset404" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)403"
  %CastToValueType405 = bitcast i8* %"&pSB[currWI].offset404" to i32 addrspace(3)**
  %loadedValue406 = load i32 addrspace(3)** %CastToValueType405, align 8
  store i32 %196, i32 addrspace(3)* %loadedValue406, align 4
  %"&(pSB[currWI].offset)171" = add nuw i64 %CurrSBIndex..7, 112
  %"&pSB[currWI].offset172" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)171"
  %CastToValueType173 = bitcast i8* %"&pSB[currWI].offset172" to <4 x i32>*
  %loadedValue174 = load <4 x i32>* %CastToValueType173, align 16
  %197 = extractelement <4 x i32> %loadedValue174, i32 2
  %"&(pSB[currWI].offset)412" = add nuw i64 %CurrSBIndex..7, 272
  %"&pSB[currWI].offset413" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)412"
  %CastToValueType414 = bitcast i8* %"&pSB[currWI].offset413" to i32 addrspace(3)**
  %loadedValue415 = load i32 addrspace(3)** %CastToValueType414, align 8
  store i32 %197, i32 addrspace(3)* %loadedValue415, align 4
  %"&(pSB[currWI].offset)176" = add nuw i64 %CurrSBIndex..7, 112
  %"&pSB[currWI].offset177" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)176"
  %CastToValueType178 = bitcast i8* %"&pSB[currWI].offset177" to <4 x i32>*
  %loadedValue179 = load <4 x i32>* %CastToValueType178, align 16
  %198 = extractelement <4 x i32> %loadedValue179, i32 3
  %"&(pSB[currWI].offset)421" = add nuw i64 %CurrSBIndex..7, 280
  %"&pSB[currWI].offset422" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)421"
  %CastToValueType423 = bitcast i8* %"&pSB[currWI].offset422" to i32 addrspace(3)**
  %loadedValue424 = load i32 addrspace(3)** %CastToValueType423, align 8
  store i32 %198, i32 addrspace(3)* %loadedValue424, align 4
  %loadedCurrWI490 = load i64* %pCurrWI, align 8
  %check.WI.iter491 = icmp ult i64 %loadedCurrWI490, %iterCount
  br i1 %check.WI.iter491, label %thenBB488, label %elseBB489

thenBB488:                                        ; preds = %SyncBB465
  %"CurrWI++492" = add nuw i64 %loadedCurrWI490, 1
  store i64 %"CurrWI++492", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride494" = add nuw i64 %CurrSBIndex..7, 560
  br label %SyncBB465

elseBB489:                                        ; preds = %SyncBB465
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB467

SyncBB467:                                        ; preds = %thenBB495, %elseBB489
  %CurrSBIndex..8 = phi i64 [ 0, %elseBB489 ], [ %"loadedCurrSB+Stride501", %thenBB495 ]
  %"&(pSB[currWI].offset)69" = add nuw i64 %CurrSBIndex..8, 56
  %"&pSB[currWI].offset70" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)69"
  %CastToValueType71 = bitcast i8* %"&pSB[currWI].offset70" to i32 addrspace(3)**
  %loadedValue72 = load i32 addrspace(3)** %CastToValueType71, align 8
  %199 = load i32 addrspace(3)* %loadedValue72, align 4
  %200 = insertelement <4 x i32> undef, i32 %199, i32 0
  %"&(pSB[currWI].offset)83" = add nuw i64 %CurrSBIndex..8, 64
  %"&pSB[currWI].offset84" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)83"
  %CastToValueType85 = bitcast i8* %"&pSB[currWI].offset84" to i32 addrspace(3)**
  %loadedValue86 = load i32 addrspace(3)** %CastToValueType85, align 8
  %201 = load i32 addrspace(3)* %loadedValue86, align 4
  %202 = insertelement <4 x i32> %200, i32 %201, i32 1
  %"&(pSB[currWI].offset)97" = add nuw i64 %CurrSBIndex..8, 72
  %"&pSB[currWI].offset98" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)97"
  %CastToValueType99 = bitcast i8* %"&pSB[currWI].offset98" to i32 addrspace(3)**
  %loadedValue100 = load i32 addrspace(3)** %CastToValueType99, align 8
  %203 = load i32 addrspace(3)* %loadedValue100, align 4
  %204 = insertelement <4 x i32> %202, i32 %203, i32 2
  %"&(pSB[currWI].offset)111" = add nuw i64 %CurrSBIndex..8, 80
  %"&pSB[currWI].offset112" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)111"
  %CastToValueType113 = bitcast i8* %"&pSB[currWI].offset112" to i32 addrspace(3)**
  %loadedValue114 = load i32 addrspace(3)** %CastToValueType113, align 8
  %205 = load i32 addrspace(3)* %loadedValue114, align 4
  %206 = insertelement <4 x i32> %204, i32 %205, i32 3
  %"&(pSB[currWI].offset)440" = add nuw i64 %CurrSBIndex..8, 304
  %"&pSB[currWI].offset441" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)440"
  %CastToValueType442 = bitcast i8* %"&pSB[currWI].offset441" to <4 x i32>*
  store <4 x i32> %206, <4 x i32>* %CastToValueType442, align 16
  %loadedCurrWI497 = load i64* %pCurrWI, align 8
  %check.WI.iter498 = icmp ult i64 %loadedCurrWI497, %iterCount
  br i1 %check.WI.iter498, label %thenBB495, label %elseBB496

thenBB495:                                        ; preds = %SyncBB467
  %"CurrWI++499" = add nuw i64 %loadedCurrWI497, 1
  store i64 %"CurrWI++499", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride501" = add nuw i64 %CurrSBIndex..8, 560
  br label %SyncBB467

elseBB496:                                        ; preds = %SyncBB467
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB468

SyncBB468:                                        ; preds = %thenBB509, %thenBB502, %elseBB496
  %currBarrier.0 = phi i32 [ %currBarrier.1, %thenBB509 ], [ %currBarrier.2, %thenBB502 ], [ 7, %elseBB496 ]
  %CurrSBIndex..1 = phi i64 [ %"loadedCurrSB+Stride515", %thenBB509 ], [ %"loadedCurrSB+Stride508", %thenBB502 ], [ 0, %elseBB496 ]
  %"&(pSB[currWI].offset)152" = add nuw i64 %CurrSBIndex..1, 100
  %"&pSB[currWI].offset153" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)152"
  %CastToValueType154 = bitcast i8* %"&pSB[currWI].offset153" to i32*
  %loadedValue155 = load i32* %CastToValueType154, align 4
  %indvar.next = add i32 %loadedValue155, 1
  %"&(pSB[currWI].offset)454" = add nuw i64 %CurrSBIndex..1, 320
  %"&pSB[currWI].offset455" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)454"
  %CastToValueType456 = bitcast i8* %"&pSB[currWI].offset455" to i32*
  store i32 %indvar.next, i32* %CastToValueType456, align 4
  %exitcond = icmp eq i32 %indvar.next, %nbits
  %"&(pSB[currWI].offset)430" = add nuw i64 %CurrSBIndex..1, 288
  %"&pSB[currWI].offset431" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)430"
  %CastToValueType432 = bitcast i8* %"&pSB[currWI].offset431" to <4 x i32>*
  %loadedValue433 = load <4 x i32>* %CastToValueType432, align 16
  %"&(pSB[currWI].offset)435" = add nuw i64 %CurrSBIndex..1, 288
  %"&(pSB[currWI].offset)444" = add nuw i64 %CurrSBIndex..1, 304
  %"&pSB[currWI].offset445" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)444"
  %CastToValueType446 = bitcast i8* %"&pSB[currWI].offset445" to <4 x i32>*
  %loadedValue447 = load <4 x i32>* %CastToValueType446, align 16
  %"&(pSB[currWI].offset)449" = add nuw i64 %CurrSBIndex..1, 304
  br i1 %exitcond, label %._crit_edge, label %"Barrier BB17"

._crit_edge:                                      ; preds = %SyncBB468, %SyncBB471
  %currBarrier.2 = phi i32 [ %currBarrier.0, %SyncBB468 ], [ %currBarrier.3, %SyncBB471 ]
  %CurrSBIndex..3 = phi i64 [ %CurrSBIndex..1, %SyncBB468 ], [ %CurrSBIndex..4, %SyncBB471 ]
  %value.0.lcssa = phi <4 x i32> [ %loadedValue447, %SyncBB468 ], [ %16, %SyncBB471 ]
  %key.0.lcssa = phi <4 x i32> [ %loadedValue433, %SyncBB468 ], [ %14, %SyncBB471 ]
  %"&pSB[currWI].offset25" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..3
  %CastToValueType26 = bitcast i8* %"&pSB[currWI].offset25" to i64*
  %loadedValue27 = load i64* %CastToValueType26, align 8
  %207 = getelementptr inbounds <4 x i32> addrspace(1)* %keysOut, i64 %loadedValue27
  store <4 x i32> %key.0.lcssa, <4 x i32> addrspace(1)* %207, align 16
  %"&pSB[currWI].offset21" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..3
  %CastToValueType22 = bitcast i8* %"&pSB[currWI].offset21" to i64*
  %loadedValue = load i64* %CastToValueType22, align 8
  %208 = getelementptr inbounds <4 x i32> addrspace(1)* %valuesOut, i64 %loadedValue
  store <4 x i32> %value.0.lcssa, <4 x i32> addrspace(1)* %208, align 16
  %loadedCurrWI504 = load i64* %pCurrWI, align 8
  %check.WI.iter505 = icmp ult i64 %loadedCurrWI504, %iterCount
  br i1 %check.WI.iter505, label %thenBB502, label %elseBB503

thenBB502:                                        ; preds = %._crit_edge
  %"CurrWI++506" = add nuw i64 %loadedCurrWI504, 1
  store i64 %"CurrWI++506", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride508" = add nuw i64 %CurrSBIndex..3, 560
  %cond = icmp eq i32 %currBarrier.2, 65
  br i1 %cond, label %SyncBB471, label %SyncBB468

elseBB503:                                        ; preds = %._crit_edge
  store i64 0, i64* %pCurrWI, align 8
  ret void
}

define void @__findRadixOffsets_separated_args(<2 x i32> addrspace(1)* nocapture %keys, i32 addrspace(1)* nocapture %counters, i32 addrspace(1)* nocapture %blockOffsets, i32 %startbit, i32 %numElements, i32 %totalBlocks, i32 addrspace(3)* nocapture %sRadix1, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
; <label>:0
  %1 = bitcast i8 addrspace(3)* %pLocalMem to [16 x i32] addrspace(3)*
  br label %SyncBB155

SyncBB155:                                        ; preds = %0, %thenBB
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ 0, %0 ]
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %0 ]
  %2 = load i64* %pWGId, align 8
  %3 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %"&(pSB[currWI].offset)" = add nuw i64 %CurrSBIndex..0, 328
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i64*
  store i64 %4, i64* %CastToValueType, align 8
  %5 = trunc i64 %4 to i32
  %"&(pSB[currWI].offset)27" = add nuw i64 %CurrSBIndex..0, 336
  %"&pSB[currWI].offset28" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)27"
  %CastToValueType29 = bitcast i8* %"&pSB[currWI].offset28" to i32*
  store i32 %5, i32* %CastToValueType29, align 4
  %6 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %7 = load i64* %6, align 8
  %8 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %9 = load i64* %8, align 8
  %10 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %11 = load i64* %10, align 8
  %12 = add i64 %9, %11
  %13 = getelementptr inbounds <2 x i32> addrspace(1)* %keys, i64 %12
  %14 = load <2 x i32> addrspace(1)* %13, align 8
  %15 = extractelement <2 x i32> %14, i32 0
  %16 = lshr i32 %15, %startbit
  %17 = and i32 %16, 15
  %18 = shl i32 %5, 1
  %19 = zext i32 %18 to i64
  %20 = getelementptr inbounds i32 addrspace(3)* %sRadix1, i64 %19
  store i32 %17, i32 addrspace(3)* %20, align 4
  %21 = extractelement <2 x i32> %14, i32 1
  %22 = lshr i32 %21, %startbit
  %23 = and i32 %22, 15
  %24 = or i32 %18, 1
  %25 = zext i32 %24 to i64
  %26 = getelementptr inbounds i32 addrspace(3)* %sRadix1, i64 %25
  store i32 %23, i32 addrspace(3)* %26, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %SyncBB155
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 560
  br label %SyncBB155

elseBB:                                           ; preds = %SyncBB155
  %27 = trunc i64 %2 to i32
  %28 = trunc i64 %7 to i32
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB157, %elseBB
  %CurrSBIndex..1 = phi i64 [ 0, %elseBB ], [ %"loadedCurrSB+Stride163", %thenBB157 ]
  %CurrWI..1 = phi i64 [ 0, %elseBB ], [ %"CurrWI++161", %thenBB157 ]
  %"&(pSB[currWI].offset)61" = add nuw i64 %CurrSBIndex..1, 336
  %"&pSB[currWI].offset62" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)61"
  %CastToValueType63 = bitcast i8* %"&pSB[currWI].offset62" to i32*
  %loadedValue64 = load i32* %CastToValueType63, align 4
  %29 = icmp ult i32 %loadedValue64, 16
  %"&(pSB[currWI].offset)81" = add nuw i64 %CurrSBIndex..1, 340
  %"&pSB[currWI].offset82" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)81"
  %CastToValueType83 = bitcast i8* %"&pSB[currWI].offset82" to i1*
  store i1 %29, i1* %CastToValueType83, align 1
  br i1 %29, label %30, label %33

; <label>:30                                      ; preds = %SyncBB
  %"&(pSB[currWI].offset)22" = add nuw i64 %CurrSBIndex..1, 328
  %"&pSB[currWI].offset23" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)22"
  %CastToValueType24 = bitcast i8* %"&pSB[currWI].offset23" to i64*
  %loadedValue25 = load i64* %CastToValueType24, align 8
  %31 = and i64 %loadedValue25, 4294967295
  %32 = getelementptr inbounds [16 x i32] addrspace(3)* %1, i64 0, i64 %31
  store i32 0, i32 addrspace(3)* %32, align 4
  br label %33

; <label>:33                                      ; preds = %30, %SyncBB
  %check.WI.iter160 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter160, label %thenBB157, label %SyncBB150

thenBB157:                                        ; preds = %33
  %"CurrWI++161" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride163" = add nuw i64 %CurrSBIndex..1, 560
  br label %SyncBB

SyncBB150:                                        ; preds = %33, %thenBB164
  %CurrSBIndex..2 = phi i64 [ %"loadedCurrSB+Stride170", %thenBB164 ], [ 0, %33 ]
  %CurrWI..2 = phi i64 [ %"CurrWI++168", %thenBB164 ], [ 0, %33 ]
  %"&(pSB[currWI].offset)56" = add nuw i64 %CurrSBIndex..2, 336
  %"&pSB[currWI].offset57" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)56"
  %CastToValueType58 = bitcast i8* %"&pSB[currWI].offset57" to i32*
  %loadedValue59 = load i32* %CastToValueType58, align 4
  %34 = icmp eq i32 %loadedValue59, 0
  %"&(pSB[currWI].offset)95" = add nuw i64 %CurrSBIndex..2, 341
  %"&pSB[currWI].offset96" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)95"
  %CastToValueType97 = bitcast i8* %"&pSB[currWI].offset96" to i1*
  store i1 %34, i1* %CastToValueType97, align 1
  br i1 %34, label %phi-split-bb, label %35

; <label>:35                                      ; preds = %SyncBB150
  %"&(pSB[currWI].offset)17" = add nuw i64 %CurrSBIndex..2, 328
  %"&pSB[currWI].offset18" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)17"
  %CastToValueType19 = bitcast i8* %"&pSB[currWI].offset18" to i64*
  %loadedValue20 = load i64* %CastToValueType19, align 8
  %36 = and i64 %loadedValue20, 4294967295
  %37 = getelementptr inbounds i32 addrspace(3)* %sRadix1, i64 %36
  %38 = load i32 addrspace(3)* %37, align 4
  %"&(pSB[currWI].offset)51" = add nuw i64 %CurrSBIndex..2, 336
  %"&pSB[currWI].offset52" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)51"
  %CastToValueType53 = bitcast i8* %"&pSB[currWI].offset52" to i32*
  %loadedValue54 = load i32* %CastToValueType53, align 4
  %39 = add i32 %loadedValue54, -1
  %40 = zext i32 %39 to i64
  %41 = getelementptr inbounds i32 addrspace(3)* %sRadix1, i64 %40
  %42 = load i32 addrspace(3)* %41, align 4
  %43 = icmp eq i32 %38, %42
  br i1 %43, label %phi-split-bb, label %44

; <label>:44                                      ; preds = %35
  %45 = zext i32 %38 to i64
  %46 = getelementptr inbounds [16 x i32] addrspace(3)* %1, i64 0, i64 %45
  %"&(pSB[currWI].offset)66" = add nuw i64 %CurrSBIndex..2, 336
  %"&pSB[currWI].offset67" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)66"
  %CastToValueType68 = bitcast i8* %"&pSB[currWI].offset67" to i32*
  %loadedValue69 = load i32* %CastToValueType68, align 4
  store i32 %loadedValue69, i32 addrspace(3)* %46, align 4
  br label %phi-split-bb

phi-split-bb:                                     ; preds = %SyncBB150, %35, %44
  %"&(pSB[currWI].offset)46" = add nuw i64 %CurrSBIndex..2, 336
  %"&pSB[currWI].offset47" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)46"
  %CastToValueType48 = bitcast i8* %"&pSB[currWI].offset47" to i32*
  %loadedValue49 = load i32* %CastToValueType48, align 4
  %47 = add i32 %28, %loadedValue49
  %"&(pSB[currWI].offset)104" = add nuw i64 %CurrSBIndex..2, 344
  %"&pSB[currWI].offset105" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)104"
  %CastToValueType106 = bitcast i8* %"&pSB[currWI].offset105" to i32*
  store i32 %47, i32* %CastToValueType106, align 4
  %48 = zext i32 %47 to i64
  %49 = getelementptr inbounds i32 addrspace(3)* %sRadix1, i64 %48
  %"&(pSB[currWI].offset)118" = add nuw i64 %CurrSBIndex..2, 352
  %"&pSB[currWI].offset119" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)118"
  %CastToValueType120 = bitcast i8* %"&pSB[currWI].offset119" to i32 addrspace(3)**
  store i32 addrspace(3)* %49, i32 addrspace(3)** %CastToValueType120, align 8
  %50 = load i32 addrspace(3)* %49, align 4
  %"&(pSB[currWI].offset)41" = add nuw i64 %CurrSBIndex..2, 336
  %"&pSB[currWI].offset42" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)41"
  %CastToValueType43 = bitcast i8* %"&pSB[currWI].offset42" to i32*
  %loadedValue44 = load i32* %CastToValueType43, align 4
  %51 = add i32 %loadedValue44, -1
  %"&(pSB[currWI].offset)127" = add nuw i64 %CurrSBIndex..2, 360
  %"&pSB[currWI].offset128" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)127"
  %CastToValueType129 = bitcast i8* %"&pSB[currWI].offset128" to i32*
  store i32 %51, i32* %CastToValueType129, align 4
  %52 = add i32 %51, %28
  %53 = zext i32 %52 to i64
  %54 = getelementptr inbounds i32 addrspace(3)* %sRadix1, i64 %53
  %"&(pSB[currWI].offset)136" = add nuw i64 %CurrSBIndex..2, 368
  %"&pSB[currWI].offset137" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)136"
  %CastToValueType138 = bitcast i8* %"&pSB[currWI].offset137" to i32 addrspace(3)**
  store i32 addrspace(3)* %54, i32 addrspace(3)** %CastToValueType138, align 8
  %55 = load i32 addrspace(3)* %54, align 4
  %56 = icmp eq i32 %50, %55
  br i1 %56, label %60, label %57

; <label>:57                                      ; preds = %phi-split-bb
  %58 = zext i32 %50 to i64
  %59 = getelementptr inbounds [16 x i32] addrspace(3)* %1, i64 0, i64 %58
  %"&(pSB[currWI].offset)108" = add nuw i64 %CurrSBIndex..2, 344
  %"&pSB[currWI].offset109" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)108"
  %CastToValueType110 = bitcast i8* %"&pSB[currWI].offset109" to i32*
  %loadedValue111 = load i32* %CastToValueType110, align 4
  store i32 %loadedValue111, i32 addrspace(3)* %59, align 4
  br label %60

; <label>:60                                      ; preds = %57, %phi-split-bb
  %check.WI.iter167 = icmp ult i64 %CurrWI..2, %iterCount
  br i1 %check.WI.iter167, label %thenBB164, label %elseBB165

thenBB164:                                        ; preds = %60
  %"CurrWI++168" = add nuw i64 %CurrWI..2, 1
  %"loadedCurrSB+Stride170" = add nuw i64 %CurrSBIndex..2, 560
  br label %SyncBB150

elseBB165:                                        ; preds = %60
  %61 = shl i32 %27, 4
  br label %SyncBB151

SyncBB151:                                        ; preds = %thenBB171, %elseBB165
  %CurrSBIndex..3 = phi i64 [ 0, %elseBB165 ], [ %"loadedCurrSB+Stride177", %thenBB171 ]
  %CurrWI..3 = phi i64 [ 0, %elseBB165 ], [ %"CurrWI++175", %thenBB171 ]
  %"&(pSB[currWI].offset)90" = add nuw i64 %CurrSBIndex..3, 340
  %"&pSB[currWI].offset91" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)90"
  %CastToValueType92 = bitcast i8* %"&pSB[currWI].offset91" to i1*
  %loadedValue93 = load i1* %CastToValueType92, align 1
  br i1 %loadedValue93, label %62, label %69

; <label>:62                                      ; preds = %SyncBB151
  %"&(pSB[currWI].offset)12" = add nuw i64 %CurrSBIndex..3, 328
  %"&pSB[currWI].offset13" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)12"
  %CastToValueType14 = bitcast i8* %"&pSB[currWI].offset13" to i64*
  %loadedValue15 = load i64* %CastToValueType14, align 8
  %63 = and i64 %loadedValue15, 4294967295
  %64 = getelementptr inbounds [16 x i32] addrspace(3)* %1, i64 0, i64 %63
  %65 = load i32 addrspace(3)* %64, align 4
  %"&(pSB[currWI].offset)36" = add nuw i64 %CurrSBIndex..3, 336
  %"&pSB[currWI].offset37" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)36"
  %CastToValueType38 = bitcast i8* %"&pSB[currWI].offset37" to i32*
  %loadedValue39 = load i32* %CastToValueType38, align 4
  %66 = add i32 %61, %loadedValue39
  %67 = zext i32 %66 to i64
  %68 = getelementptr inbounds i32 addrspace(1)* %blockOffsets, i64 %67
  store i32 %65, i32 addrspace(1)* %68, align 4
  br label %69

; <label>:69                                      ; preds = %62, %SyncBB151
  %check.WI.iter174 = icmp ult i64 %CurrWI..3, %iterCount
  br i1 %check.WI.iter174, label %thenBB171, label %elseBB172

thenBB171:                                        ; preds = %69
  %"CurrWI++175" = add nuw i64 %CurrWI..3, 1
  %"loadedCurrSB+Stride177" = add nuw i64 %CurrSBIndex..3, 560
  br label %SyncBB151

elseBB172:                                        ; preds = %69
  %70 = add i32 %28, -1
  %71 = shl i32 %28, 1
  %72 = add i32 %71, -1
  %73 = zext i32 %72 to i64
  %74 = getelementptr inbounds i32 addrspace(3)* %sRadix1, i64 %73
  br label %SyncBB152

SyncBB152:                                        ; preds = %thenBB185, %elseBB172
  %CurrSBIndex..5 = phi i64 [ 0, %elseBB172 ], [ %"loadedCurrSB+Stride191", %thenBB185 ]
  %CurrWI..5 = phi i64 [ 0, %elseBB172 ], [ %"CurrWI++189", %thenBB185 ]
  %"&(pSB[currWI].offset)99" = add nuw i64 %CurrSBIndex..5, 341
  %"&pSB[currWI].offset100" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)99"
  %CastToValueType101 = bitcast i8* %"&pSB[currWI].offset100" to i1*
  %loadedValue102 = load i1* %CastToValueType101, align 1
  br i1 %loadedValue102, label %phi-split-bb1, label %75

; <label>:75                                      ; preds = %SyncBB152
  %"&(pSB[currWI].offset)7" = add nuw i64 %CurrSBIndex..5, 328
  %"&pSB[currWI].offset8" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)7"
  %CastToValueType9 = bitcast i8* %"&pSB[currWI].offset8" to i64*
  %loadedValue10 = load i64* %CastToValueType9, align 8
  %76 = and i64 %loadedValue10, 4294967295
  %77 = getelementptr inbounds i32 addrspace(3)* %sRadix1, i64 %76
  %78 = load i32 addrspace(3)* %77, align 4
  %"&(pSB[currWI].offset)131" = add nuw i64 %CurrSBIndex..5, 360
  %"&pSB[currWI].offset132" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)131"
  %CastToValueType133 = bitcast i8* %"&pSB[currWI].offset132" to i32*
  %loadedValue134 = load i32* %CastToValueType133, align 4
  %79 = zext i32 %loadedValue134 to i64
  %80 = getelementptr inbounds i32 addrspace(3)* %sRadix1, i64 %79
  %81 = load i32 addrspace(3)* %80, align 4
  %82 = icmp eq i32 %78, %81
  br i1 %82, label %phi-split-bb1, label %83

; <label>:83                                      ; preds = %75
  %84 = zext i32 %81 to i64
  %85 = getelementptr inbounds [16 x i32] addrspace(3)* %1, i64 0, i64 %84
  %86 = load i32 addrspace(3)* %85, align 4
  %"&(pSB[currWI].offset)71" = add nuw i64 %CurrSBIndex..5, 336
  %"&pSB[currWI].offset72" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)71"
  %CastToValueType73 = bitcast i8* %"&pSB[currWI].offset72" to i32*
  %loadedValue74 = load i32* %CastToValueType73, align 4
  %87 = sub i32 %loadedValue74, %86
  store i32 %87, i32 addrspace(3)* %85, align 4
  br label %phi-split-bb1

phi-split-bb1:                                    ; preds = %SyncBB152, %75, %83
  %"&(pSB[currWI].offset)122" = add nuw i64 %CurrSBIndex..5, 352
  %"&pSB[currWI].offset123" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)122"
  %CastToValueType124 = bitcast i8* %"&pSB[currWI].offset123" to i32 addrspace(3)**
  %loadedValue125 = load i32 addrspace(3)** %CastToValueType124, align 8
  %88 = load i32 addrspace(3)* %loadedValue125, align 4
  %"&(pSB[currWI].offset)140" = add nuw i64 %CurrSBIndex..5, 368
  %"&pSB[currWI].offset141" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)140"
  %CastToValueType142 = bitcast i8* %"&pSB[currWI].offset141" to i32 addrspace(3)**
  %loadedValue143 = load i32 addrspace(3)** %CastToValueType142, align 8
  %89 = load i32 addrspace(3)* %loadedValue143, align 4
  %90 = icmp eq i32 %88, %89
  br i1 %90, label %96, label %91

; <label>:91                                      ; preds = %phi-split-bb1
  %92 = zext i32 %89 to i64
  %93 = getelementptr inbounds [16 x i32] addrspace(3)* %1, i64 0, i64 %92
  %94 = load i32 addrspace(3)* %93, align 4
  %"&(pSB[currWI].offset)113" = add nuw i64 %CurrSBIndex..5, 344
  %"&pSB[currWI].offset114" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)113"
  %CastToValueType115 = bitcast i8* %"&pSB[currWI].offset114" to i32*
  %loadedValue116 = load i32* %CastToValueType115, align 4
  %95 = sub i32 %loadedValue116, %94
  store i32 %95, i32 addrspace(3)* %93, align 4
  br label %96

; <label>:96                                      ; preds = %91, %phi-split-bb1
  %"&(pSB[currWI].offset)76" = add nuw i64 %CurrSBIndex..5, 336
  %"&pSB[currWI].offset77" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)76"
  %CastToValueType78 = bitcast i8* %"&pSB[currWI].offset77" to i32*
  %loadedValue79 = load i32* %CastToValueType78, align 4
  %97 = icmp eq i32 %loadedValue79, %70
  br i1 %97, label %98, label %104

; <label>:98                                      ; preds = %96
  %99 = load i32 addrspace(3)* %74, align 4
  %100 = zext i32 %99 to i64
  %101 = getelementptr inbounds [16 x i32] addrspace(3)* %1, i64 0, i64 %100
  %102 = load i32 addrspace(3)* %101, align 4
  %103 = sub i32 %71, %102
  store i32 %103, i32 addrspace(3)* %101, align 4
  br label %104

; <label>:104                                     ; preds = %98, %96
  %check.WI.iter188 = icmp ult i64 %CurrWI..5, %iterCount
  br i1 %check.WI.iter188, label %thenBB185, label %SyncBB154

thenBB185:                                        ; preds = %104
  %"CurrWI++189" = add nuw i64 %CurrWI..5, 1
  %"loadedCurrSB+Stride191" = add nuw i64 %CurrSBIndex..5, 560
  br label %SyncBB152

SyncBB154:                                        ; preds = %104, %thenBB178
  %CurrSBIndex..4 = phi i64 [ %"loadedCurrSB+Stride184", %thenBB178 ], [ 0, %104 ]
  %CurrWI..4 = phi i64 [ %"CurrWI++182", %thenBB178 ], [ 0, %104 ]
  %"&(pSB[currWI].offset)85" = add nuw i64 %CurrSBIndex..4, 340
  %"&pSB[currWI].offset86" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)85"
  %CastToValueType87 = bitcast i8* %"&pSB[currWI].offset86" to i1*
  %loadedValue88 = load i1* %CastToValueType87, align 1
  br i1 %loadedValue88, label %105, label %UnifiedReturnBlock

; <label>:105                                     ; preds = %SyncBB154
  %"&(pSB[currWI].offset)3" = add nuw i64 %CurrSBIndex..4, 328
  %"&pSB[currWI].offset4" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)3"
  %CastToValueType5 = bitcast i8* %"&pSB[currWI].offset4" to i64*
  %loadedValue = load i64* %CastToValueType5, align 8
  %106 = and i64 %loadedValue, 4294967295
  %107 = getelementptr inbounds [16 x i32] addrspace(3)* %1, i64 0, i64 %106
  %108 = load i32 addrspace(3)* %107, align 4
  %"&(pSB[currWI].offset)31" = add nuw i64 %CurrSBIndex..4, 336
  %"&pSB[currWI].offset32" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)31"
  %CastToValueType33 = bitcast i8* %"&pSB[currWI].offset32" to i32*
  %loadedValue34 = load i32* %CastToValueType33, align 4
  %109 = mul i32 %loadedValue34, %totalBlocks
  %110 = add i32 %109, %27
  %111 = zext i32 %110 to i64
  %112 = getelementptr inbounds i32 addrspace(1)* %counters, i64 %111
  store i32 %108, i32 addrspace(1)* %112, align 4
  br label %UnifiedReturnBlock

UnifiedReturnBlock:                               ; preds = %SyncBB154, %105
  %check.WI.iter181 = icmp ult i64 %CurrWI..4, %iterCount
  br i1 %check.WI.iter181, label %thenBB178, label %SyncBB153

thenBB178:                                        ; preds = %UnifiedReturnBlock
  %"CurrWI++182" = add nuw i64 %CurrWI..4, 1
  %"loadedCurrSB+Stride184" = add nuw i64 %CurrSBIndex..4, 560
  br label %SyncBB154

SyncBB153:                                        ; preds = %UnifiedReturnBlock
  ret void
}

define void @__reorderData_separated_args(i32 %startbit, i32 addrspace(1)* nocapture %outKeys, i32 addrspace(1)* nocapture %outValues, <2 x i32> addrspace(1)* nocapture %keys, <2 x i32> addrspace(1)* nocapture %values, i32 addrspace(1)* nocapture %blockOffsets, i32 addrspace(1)* nocapture %offsets, i32 addrspace(1)* nocapture %sizes, i32 %totalBlocks, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = bitcast i8 addrspace(3)* %pLocalMem to [256 x <2 x i32>] addrspace(3)*
  %1 = getelementptr i8 addrspace(3)* %pLocalMem, i64 2048
  %2 = bitcast i8 addrspace(3)* %1 to [256 x <2 x i32>] addrspace(3)*
  %3 = getelementptr i8 addrspace(3)* %pLocalMem, i64 4096
  %4 = bitcast i8 addrspace(3)* %3 to [16 x i32] addrspace(3)*
  %5 = getelementptr i8 addrspace(3)* %pLocalMem, i64 4224
  %6 = bitcast i8 addrspace(3)* %5 to [16 x i32] addrspace(3)*
  %7 = zext i32 %totalBlocks to i64
  br label %SyncBB3

SyncBB3:                                          ; preds = %thenBB5, %FirstBB
  %CurrWI..1 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++9", %thenBB5 ]
  %8 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %9 = load i64* %8, align 8
  %10 = load i64* %pWGId, align 8
  %11 = and i64 %10, 4294967295
  %12 = mul i64 %11, %9
  %13 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..1, i32 0, i64 0
  %14 = load i64* %13, align 8
  %15 = add i64 %12, %14
  %16 = and i64 %15, 4294967295
  %17 = getelementptr inbounds <2 x i32> addrspace(1)* %keys, i64 %16
  %18 = load <2 x i32> addrspace(1)* %17, align 8
  %19 = getelementptr inbounds [256 x <2 x i32>] addrspace(3)* %0, i64 0, i64 %14
  store <2 x i32> %18, <2 x i32> addrspace(3)* %19, align 8
  %20 = getelementptr inbounds <2 x i32> addrspace(1)* %values, i64 %16
  %21 = load <2 x i32> addrspace(1)* %20, align 8
  %22 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..1, i32 0, i64 0
  %23 = load i64* %22, align 8
  %24 = getelementptr inbounds [256 x <2 x i32>] addrspace(3)* %2, i64 0, i64 %23
  store <2 x i32> %21, <2 x i32> addrspace(3)* %24, align 8
  %25 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..1, i32 0, i64 0
  %26 = load i64* %25, align 8
  %27 = icmp ult i64 %26, 16
  br i1 %27, label %28, label %44

; <label>:28                                      ; preds = %SyncBB3
  %29 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..1, i32 0, i64 0
  %30 = load i64* %29, align 8
  %31 = mul i64 %30, %7
  %32 = add i64 %31, %11
  %33 = getelementptr inbounds i32 addrspace(1)* %offsets, i64 %32
  %34 = load i32 addrspace(1)* %33, align 4
  %35 = getelementptr inbounds [16 x i32] addrspace(3)* %4, i64 0, i64 %30
  store i32 %34, i32 addrspace(3)* %35, align 4
  %36 = shl i64 %10, 4
  %37 = and i64 %36, 4294967280
  %38 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..1, i32 0, i64 0
  %39 = load i64* %38, align 8
  %40 = add i64 %39, %37
  %41 = getelementptr inbounds i32 addrspace(1)* %blockOffsets, i64 %40
  %42 = load i32 addrspace(1)* %41, align 4
  %43 = getelementptr inbounds [16 x i32] addrspace(3)* %6, i64 0, i64 %39
  store i32 %42, i32 addrspace(3)* %43, align 4
  br label %44

; <label>:44                                      ; preds = %28, %SyncBB3
  %check.WI.iter8 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter8, label %thenBB5, label %elseBB6

thenBB5:                                          ; preds = %44
  %"CurrWI++9" = add nuw i64 %CurrWI..1, 1
  br label %SyncBB3

elseBB6:                                          ; preds = %44
  %45 = and i64 %9, 4294967295
  br label %SyncBB4

SyncBB4:                                          ; preds = %thenBB, %elseBB6
  %CurrWI..0 = phi i64 [ 0, %elseBB6 ], [ %"CurrWI++", %thenBB ]
  %46 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %47 = load i64* %46, align 8
  %48 = getelementptr inbounds [256 x <2 x i32>] addrspace(3)* %0, i64 0, i64 0, i64 %47
  %49 = load i32 addrspace(3)* %48, align 4
  %50 = lshr i32 %49, %startbit
  %51 = and i32 %50, 15
  %52 = zext i32 %51 to i64
  %53 = getelementptr inbounds [16 x i32] addrspace(3)* %4, i64 0, i64 %52
  %54 = load i32 addrspace(3)* %53, align 4
  %55 = zext i32 %54 to i64
  %56 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %57 = load i64* %56, align 8
  %58 = getelementptr inbounds [16 x i32] addrspace(3)* %6, i64 0, i64 %52
  %59 = load i32 addrspace(3)* %58, align 4
  %60 = zext i32 %59 to i64
  %61 = add i64 %55, %57
  %62 = sub i64 %61, %60
  %63 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %64 = load i64* %63, align 8
  %65 = getelementptr inbounds [256 x <2 x i32>] addrspace(3)* %0, i64 0, i64 0, i64 %64
  %66 = load i32 addrspace(3)* %65, align 4
  %67 = and i64 %62, 4294967295
  %68 = getelementptr inbounds i32 addrspace(1)* %outKeys, i64 %67
  store i32 %66, i32 addrspace(1)* %68, align 4
  %69 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %70 = load i64* %69, align 8
  %71 = getelementptr inbounds [256 x <2 x i32>] addrspace(3)* %2, i64 0, i64 0, i64 %70
  %72 = load i32 addrspace(3)* %71, align 4
  %73 = getelementptr inbounds i32 addrspace(1)* %outValues, i64 %67
  store i32 %72, i32 addrspace(1)* %73, align 4
  %74 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %75 = load i64* %74, align 8
  %76 = add i64 %75, %45
  %77 = getelementptr inbounds [256 x <2 x i32>] addrspace(3)* %0, i64 0, i64 0, i64 %76
  %78 = load i32 addrspace(3)* %77, align 4
  %79 = lshr i32 %78, %startbit
  %80 = and i32 %79, 15
  %81 = zext i32 %80 to i64
  %82 = getelementptr inbounds [16 x i32] addrspace(3)* %4, i64 0, i64 %81
  %83 = load i32 addrspace(3)* %82, align 4
  %84 = zext i32 %83 to i64
  %85 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %86 = load i64* %85, align 8
  %87 = getelementptr inbounds [16 x i32] addrspace(3)* %6, i64 0, i64 %81
  %88 = load i32 addrspace(3)* %87, align 4
  %89 = zext i32 %88 to i64
  %90 = add i64 %86, %9
  %91 = add i64 %90, %84
  %92 = sub i64 %91, %89
  %93 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %94 = load i64* %93, align 8
  %95 = add i64 %94, %45
  %96 = getelementptr inbounds [256 x <2 x i32>] addrspace(3)* %0, i64 0, i64 0, i64 %95
  %97 = load i32 addrspace(3)* %96, align 4
  %98 = and i64 %92, 4294967295
  %99 = getelementptr inbounds i32 addrspace(1)* %outKeys, i64 %98
  store i32 %97, i32 addrspace(1)* %99, align 4
  %100 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %101 = load i64* %100, align 8
  %102 = add i64 %101, %45
  %103 = getelementptr inbounds [256 x <2 x i32>] addrspace(3)* %2, i64 0, i64 0, i64 %102
  %104 = load i32 addrspace(3)* %103, align 4
  %105 = getelementptr inbounds i32 addrspace(1)* %outValues, i64 %98
  store i32 %104, i32 addrspace(1)* %105, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %SyncBB4
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB4

SyncBB:                                           ; preds = %SyncBB4
  ret void
}

define void @__addUniform_separated_args(i32 addrspace(1)* nocapture %d_vector, i32 addrspace(1)* nocapture %d_uniforms, i32 %n, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
; <label>:0
  %1 = bitcast i8 addrspace(3)* %pLocalMem to i32 addrspace(3)*
  br label %SyncBB

SyncBB:                                           ; preds = %0, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %0 ]
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ 0, %0 ]
  %2 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = icmp eq i64 %3, 0
  br i1 %4, label %5, label %"Barrier BB"

; <label>:5                                       ; preds = %SyncBB
  %6 = load i64* %pWGId, align 8
  %7 = getelementptr inbounds i32 addrspace(1)* %d_uniforms, i64 %6
  %8 = load i32 addrspace(1)* %7, align 4
  store i32 %8, i32 addrspace(3)* %1, align 4
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %5, %SyncBB
  %9 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %10 = load i64* %9, align 8
  %11 = load i64* %pWGId, align 8
  %12 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %13 = load i64* %12, align 8
  %14 = shl i64 %11, 2
  %15 = mul i64 %14, %13
  %16 = add i64 %15, %10
  %"&(pSB[currWI].offset)" = add nuw i64 %CurrSBIndex..0, 376
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i64*
  store i64 %16, i64* %CastToValueType, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB5

thenBB:                                           ; preds = %"Barrier BB"
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 560
  br label %SyncBB

SyncBB5:                                          ; preds = %"Barrier BB", %thenBB8
  %CurrWI..1 = phi i64 [ %"CurrWI++12", %thenBB8 ], [ 0, %"Barrier BB" ]
  %CurrSBIndex..1 = phi i64 [ %"loadedCurrSB+Stride14", %thenBB8 ], [ 0, %"Barrier BB" ]
  %"&(pSB[currWI].offset)2" = add nuw i64 %CurrSBIndex..1, 376
  %"&pSB[currWI].offset3" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2"
  %CastToValueType4 = bitcast i8* %"&pSB[currWI].offset3" to i64*
  %loadedValue = load i64* %CastToValueType4, align 8
  br label %17

; <label>:17                                      ; preds = %22, %SyncBB5
  %address.0.in = phi i64 [ %loadedValue, %SyncBB5 ], [ %30, %22 ]
  %18 = phi i32 [ 0, %SyncBB5 ], [ %31, %22 ]
  %19 = icmp slt i32 %18, 4
  br i1 %19, label %20, label %.critedge

; <label>:20                                      ; preds = %17
  %address.0 = trunc i64 %address.0.in to i32
  %21 = icmp ult i32 %address.0, %n
  br i1 %21, label %22, label %.critedge

; <label>:22                                      ; preds = %20
  %23 = load i32 addrspace(3)* %1, align 4
  %24 = and i64 %address.0.in, 4294967295
  %25 = getelementptr inbounds i32 addrspace(1)* %d_vector, i64 %24
  %26 = load i32 addrspace(1)* %25, align 4
  %27 = add i32 %26, %23
  store i32 %27, i32 addrspace(1)* %25, align 4
  %28 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %29 = load i64* %28, align 8
  %30 = add i64 %29, %24
  %31 = add nsw i32 %18, 1
  br label %17

.critedge:                                        ; preds = %20, %17
  %check.WI.iter11 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter11, label %thenBB8, label %SyncBB6

thenBB8:                                          ; preds = %.critedge
  %"CurrWI++12" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride14" = add nuw i64 %CurrSBIndex..1, 560
  br label %SyncBB5

SyncBB6:                                          ; preds = %.critedge
  ret void
}

define void @__scan_separated_args(i32 addrspace(1)* nocapture %g_odata, i32 addrspace(1)* nocapture %g_idata, i32 addrspace(1)* nocapture %g_blockSums, i32 %n, i32 %fullBlock, i32 %storeSum, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  store i64 0, i64* %pCurrWI, align 8
  %0 = bitcast i32 addrspace(1)* %g_idata to <4 x i32> addrspace(1)*
  %1 = icmp eq i32 %fullBlock, 0
  br label %SyncBB141

SyncBB141:                                        ; preds = %thenBB144, %FirstBB
  %CurrSBIndex..1 = phi i64 [ 0, %FirstBB ], [ %"loadedCurrSB+Stride150", %thenBB144 ]
  %currWI151 = load i64* %pCurrWI, align 8
  %2 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %currWI151, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %5 = load i64* %4, align 8
  %6 = add i64 %3, %5
  %7 = trunc i64 %6 to i32
  %"&(pSB[currWI].offset)" = add nuw i64 %CurrSBIndex..1, 384
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i32*
  store i32 %7, i32* %CastToValueType, align 4
  %currWI = load i64* %pCurrWI, align 8
  %8 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %currWI, i32 0, i64 0
  %9 = load i64* %8, align 8
  %"&(pSB[currWI].offset)14" = add nuw i64 %CurrSBIndex..1, 392
  %"&pSB[currWI].offset15" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)14"
  %CastToValueType16 = bitcast i8* %"&pSB[currWI].offset15" to i64*
  store i64 %9, i64* %CastToValueType16, align 8
  %10 = shl i32 %7, 2
  %"&(pSB[currWI].offset)23" = add nuw i64 %CurrSBIndex..1, 400
  %"&pSB[currWI].offset24" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)23"
  %CastToValueType25 = bitcast i8* %"&pSB[currWI].offset24" to i32*
  store i32 %10, i32* %CastToValueType25, align 4
  br i1 %1, label %11, label %14

; <label>:11                                      ; preds = %SyncBB141
  %"&(pSB[currWI].offset)52" = add nuw i64 %CurrSBIndex..1, 400
  %"&pSB[currWI].offset53" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)52"
  %CastToValueType54 = bitcast i8* %"&pSB[currWI].offset53" to i32*
  %loadedValue55 = load i32* %CastToValueType54, align 4
  %12 = or i32 %loadedValue55, 3
  %13 = icmp slt i32 %12, %n
  br i1 %13, label %14, label %28

; <label>:14                                      ; preds = %11, %SyncBB141
  %"&(pSB[currWI].offset)5" = add nuw i64 %CurrSBIndex..1, 384
  %"&pSB[currWI].offset6" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)5"
  %CastToValueType7 = bitcast i8* %"&pSB[currWI].offset6" to i32*
  %loadedValue = load i32* %CastToValueType7, align 4
  %15 = sext i32 %loadedValue to i64
  %16 = getelementptr inbounds <4 x i32> addrspace(1)* %0, i64 %15
  %17 = load <4 x i32> addrspace(1)* %16, align 16
  %18 = extractelement <4 x i32> %17, i32 1
  %19 = extractelement <4 x i32> %17, i32 0
  %20 = add i32 %18, %19
  %21 = insertelement <4 x i32> %17, i32 %20, i32 1
  %22 = extractelement <4 x i32> %17, i32 2
  %23 = add i32 %20, %22
  %24 = insertelement <4 x i32> %21, i32 %23, i32 2
  %25 = extractelement <4 x i32> %17, i32 3
  %26 = add i32 %23, %25
  %27 = insertelement <4 x i32> %24, i32 %26, i32 3
  br label %"Barrier BB"

; <label>:28                                      ; preds = %11
  %"&(pSB[currWI].offset)72" = add nuw i64 %CurrSBIndex..1, 400
  %"&pSB[currWI].offset73" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)72"
  %CastToValueType74 = bitcast i8* %"&pSB[currWI].offset73" to i32*
  %loadedValue75 = load i32* %CastToValueType74, align 4
  %29 = icmp slt i32 %loadedValue75, %n
  br i1 %29, label %30, label %35

; <label>:30                                      ; preds = %28
  %"&(pSB[currWI].offset)67" = add nuw i64 %CurrSBIndex..1, 400
  %"&pSB[currWI].offset68" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)67"
  %CastToValueType69 = bitcast i8* %"&pSB[currWI].offset68" to i32*
  %loadedValue70 = load i32* %CastToValueType69, align 4
  %31 = sext i32 %loadedValue70 to i64
  %32 = getelementptr inbounds i32 addrspace(1)* %g_idata, i64 %31
  %33 = load i32 addrspace(1)* %32, align 4
  %34 = uitofp i32 %33 to float
  %phitmp = fptoui float %34 to i32
  br label %35

; <label>:35                                      ; preds = %30, %28
  %36 = phi i32 [ %phitmp, %30 ], [ 0, %28 ]
  %37 = insertelement <4 x i32> undef, i32 %36, i32 0
  %"&(pSB[currWI].offset)47" = add nuw i64 %CurrSBIndex..1, 400
  %"&pSB[currWI].offset48" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)47"
  %CastToValueType49 = bitcast i8* %"&pSB[currWI].offset48" to i32*
  %loadedValue50 = load i32* %CastToValueType49, align 4
  %38 = or i32 %loadedValue50, 1
  %39 = icmp slt i32 %38, %n
  br i1 %39, label %40, label %45

; <label>:40                                      ; preds = %35
  %41 = sext i32 %38 to i64
  %42 = getelementptr inbounds i32 addrspace(1)* %g_idata, i64 %41
  %43 = load i32 addrspace(1)* %42, align 4
  %44 = uitofp i32 %43 to float
  br label %45

; <label>:45                                      ; preds = %40, %35
  %46 = phi float [ %44, %40 ], [ 0.000000e+00, %35 ]
  %47 = uitofp i32 %36 to float
  %48 = fadd float %46, %47
  %49 = fptoui float %48 to i32
  %50 = insertelement <4 x i32> %37, i32 %49, i32 1
  %"&(pSB[currWI].offset)42" = add nuw i64 %CurrSBIndex..1, 400
  %"&pSB[currWI].offset43" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)42"
  %CastToValueType44 = bitcast i8* %"&pSB[currWI].offset43" to i32*
  %loadedValue45 = load i32* %CastToValueType44, align 4
  %51 = or i32 %loadedValue45, 2
  %52 = icmp slt i32 %51, %n
  br i1 %52, label %53, label %58

; <label>:53                                      ; preds = %45
  %54 = sext i32 %51 to i64
  %55 = getelementptr inbounds i32 addrspace(1)* %g_idata, i64 %54
  %56 = load i32 addrspace(1)* %55, align 4
  %57 = uitofp i32 %56 to float
  br label %58

; <label>:58                                      ; preds = %53, %45
  %59 = phi float [ %57, %53 ], [ 0.000000e+00, %45 ]
  %60 = uitofp i32 %49 to float
  %61 = fadd float %59, %60
  %62 = fptoui float %61 to i32
  %63 = insertelement <4 x i32> %50, i32 %62, i32 2
  %64 = uitofp i32 %62 to float
  %65 = fptoui float %64 to i32
  %66 = insertelement <4 x i32> %63, i32 %65, i32 3
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %58, %14
  %threadScanT.0 = phi <4 x i32> [ %27, %14 ], [ %66, %58 ]
  %res.0 = phi i32 [ %26, %14 ], [ %65, %58 ]
  %"&(pSB[currWI].offset)101" = add nuw i64 %CurrSBIndex..1, 432
  %"&pSB[currWI].offset102" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)101"
  %CastToValueType103 = bitcast i8* %"&pSB[currWI].offset102" to i32*
  store i32 %res.0, i32* %CastToValueType103, align 4
  %"&(pSB[currWI].offset)77" = add nuw i64 %CurrSBIndex..1, 416
  %"&pSB[currWI].offset78" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)77"
  %CastToValueType79 = bitcast i8* %"&pSB[currWI].offset78" to <4 x i32>*
  store <4 x i32> %threadScanT.0, <4 x i32>* %CastToValueType79, align 16
  %loadedCurrWI146 = load i64* %pCurrWI, align 8
  %check.WI.iter147 = icmp ult i64 %loadedCurrWI146, %iterCount
  br i1 %check.WI.iter147, label %thenBB144, label %elseBB145

thenBB144:                                        ; preds = %"Barrier BB"
  %"CurrWI++148" = add nuw i64 %loadedCurrWI146, 1
  store i64 %"CurrWI++148", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride150" = add nuw i64 %CurrSBIndex..1, 560
  br label %SyncBB141

elseBB145:                                        ; preds = %"Barrier BB"
  store i64 0, i64* %pCurrWI, align 8
  %67 = bitcast i8 addrspace(3)* %pLocalMem to [512 x i32] addrspace(3)*
  br label %SyncBB232.i

SyncBB232.i:                                      ; preds = %thenBB252.i, %elseBB145
  %CurrWI..0.i = phi i64 [ %"CurrWI++256.i", %thenBB252.i ], [ 0, %elseBB145 ]
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride258.i", %thenBB252.i ], [ 0, %elseBB145 ]
  %68 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0.i, i32 0, i64 0
  %69 = load i64* %68, align 8
  %"&(pSB[currWI].offset).i" = add nuw i64 %CurrSBIndex..0.i, 440
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset).i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i64*
  store i64 %69, i64* %CastToValueType.i, align 8
  %sext.i = shl i64 %69, 32
  %70 = ashr i64 %sext.i, 32
  %71 = getelementptr inbounds [512 x i32] addrspace(3)* %67, i64 0, i64 %70
  store i32 0, i32 addrspace(3)* %71, align 4
  %check.WI.iter255.i = icmp ult i64 %CurrWI..0.i, %iterCount
  br i1 %check.WI.iter255.i, label %thenBB252.i, label %SyncBB233.i

thenBB252.i:                                      ; preds = %SyncBB232.i
  %"CurrWI++256.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride258.i" = add nuw i64 %CurrSBIndex..0.i, 560
  br label %SyncBB232.i

SyncBB233.i:                                      ; preds = %thenBB259.i, %SyncBB232.i
  %CurrWI..1.i = phi i64 [ %"CurrWI++263.i", %thenBB259.i ], [ 0, %SyncBB232.i ]
  %CurrSBIndex..1.i = phi i64 [ %"loadedCurrSB+Stride265.i", %thenBB259.i ], [ 0, %SyncBB232.i ]
  %72 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %73 = load i64* %72, align 8
  %"&(pSB[currWI].offset)20.i" = add nuw i64 %CurrSBIndex..1.i, 440
  %"&pSB[currWI].offset21.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)20.i"
  %CastToValueType22.i = bitcast i8* %"&pSB[currWI].offset21.i" to i64*
  %loadedValue.i = load i64* %CastToValueType22.i, align 8
  %74 = add i64 %73, %loadedValue.i
  %75 = trunc i64 %74 to i32
  %"&(pSB[currWI].offset)24.i" = add nuw i64 %CurrSBIndex..1.i, 448
  %"&pSB[currWI].offset25.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)24.i"
  %CastToValueType26.i = bitcast i8* %"&pSB[currWI].offset25.i" to i32*
  store i32 %75, i32* %CastToValueType26.i, align 4
  %76 = sext i32 %75 to i64
  %77 = getelementptr inbounds [512 x i32] addrspace(3)* %67, i64 0, i64 %76
  %"&(pSB[currWI].offset)68.i" = add nuw i64 %CurrSBIndex..1.i, 456
  %"&pSB[currWI].offset69.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)68.i"
  %CastToValueType70.i = bitcast i8* %"&pSB[currWI].offset69.i" to i32 addrspace(3)**
  store i32 addrspace(3)* %77, i32 addrspace(3)** %CastToValueType70.i, align 8
  %"&(pSB[currWI].offset)4.i" = add nuw i64 %CurrSBIndex..1.i, 432
  %"&pSB[currWI].offset5.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)4.i"
  %CastToArgType.i = bitcast i8* %"&pSB[currWI].offset5.i" to i32*
  %loadedValue6.i = load i32* %CastToArgType.i, align 4
  store i32 %loadedValue6.i, i32 addrspace(3)* %77, align 4
  %check.WI.iter262.i = icmp ult i64 %CurrWI..1.i, %iterCount
  br i1 %check.WI.iter262.i, label %thenBB259.i, label %SyncBB234.i

thenBB259.i:                                      ; preds = %SyncBB233.i
  %"CurrWI++263.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride265.i" = add nuw i64 %CurrSBIndex..1.i, 560
  br label %SyncBB233.i

SyncBB234.i:                                      ; preds = %thenBB266.i, %SyncBB233.i
  %CurrWI..2.i = phi i64 [ %"CurrWI++270.i", %thenBB266.i ], [ 0, %SyncBB233.i ]
  %CurrSBIndex..2.i = phi i64 [ %"loadedCurrSB+Stride272.i", %thenBB266.i ], [ 0, %SyncBB233.i ]
  %"&(pSB[currWI].offset)63.i" = add nuw i64 %CurrSBIndex..2.i, 448
  %"&pSB[currWI].offset64.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)63.i"
  %CastToValueType65.i = bitcast i8* %"&pSB[currWI].offset64.i" to i32*
  %loadedValue66.i = load i32* %CastToValueType65.i, align 4
  %78 = add nsw i32 %loadedValue66.i, -1
  %79 = sext i32 %78 to i64
  %80 = getelementptr inbounds [512 x i32] addrspace(3)* %67, i64 0, i64 %79
  %"&(pSB[currWI].offset)152.i" = add nuw i64 %CurrSBIndex..2.i, 464
  %"&pSB[currWI].offset153.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)152.i"
  %CastToValueType154.i = bitcast i8* %"&pSB[currWI].offset153.i" to i32 addrspace(3)**
  store i32 addrspace(3)* %80, i32 addrspace(3)** %CastToValueType154.i, align 8
  %81 = load i32 addrspace(3)* %80, align 4
  %"&(pSB[currWI].offset)161.i" = add nuw i64 %CurrSBIndex..2.i, 472
  %"&pSB[currWI].offset162.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)161.i"
  %CastToValueType163.i = bitcast i8* %"&pSB[currWI].offset162.i" to i32*
  store i32 %81, i32* %CastToValueType163.i, align 4
  %check.WI.iter269.i = icmp ult i64 %CurrWI..2.i, %iterCount
  br i1 %check.WI.iter269.i, label %thenBB266.i, label %SyncBB235.i

thenBB266.i:                                      ; preds = %SyncBB234.i
  %"CurrWI++270.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride272.i" = add nuw i64 %CurrSBIndex..2.i, 560
  br label %SyncBB234.i

SyncBB235.i:                                      ; preds = %thenBB273.i, %SyncBB234.i
  %CurrWI..3.i = phi i64 [ %"CurrWI++277.i", %thenBB273.i ], [ 0, %SyncBB234.i ]
  %CurrSBIndex..3.i = phi i64 [ %"loadedCurrSB+Stride279.i", %thenBB273.i ], [ 0, %SyncBB234.i ]
  %"&(pSB[currWI].offset)147.i" = add nuw i64 %CurrSBIndex..3.i, 456
  %"&pSB[currWI].offset148.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)147.i"
  %CastToValueType149.i = bitcast i8* %"&pSB[currWI].offset148.i" to i32 addrspace(3)**
  %loadedValue150.i = load i32 addrspace(3)** %CastToValueType149.i, align 8
  %82 = load i32 addrspace(3)* %loadedValue150.i, align 4
  %"&(pSB[currWI].offset)165.i" = add nuw i64 %CurrSBIndex..3.i, 472
  %"&pSB[currWI].offset166.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)165.i"
  %CastToValueType167.i = bitcast i8* %"&pSB[currWI].offset166.i" to i32*
  %loadedValue168.i = load i32* %CastToValueType167.i, align 4
  %83 = add i32 %82, %loadedValue168.i
  %"&(pSB[currWI].offset)142.i" = add nuw i64 %CurrSBIndex..3.i, 456
  %"&pSB[currWI].offset143.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)142.i"
  %CastToValueType144.i = bitcast i8* %"&pSB[currWI].offset143.i" to i32 addrspace(3)**
  %loadedValue145.i = load i32 addrspace(3)** %CastToValueType144.i, align 8
  store i32 %83, i32 addrspace(3)* %loadedValue145.i, align 4
  %check.WI.iter276.i = icmp ult i64 %CurrWI..3.i, %iterCount
  br i1 %check.WI.iter276.i, label %thenBB273.i, label %SyncBB236.i

thenBB273.i:                                      ; preds = %SyncBB235.i
  %"CurrWI++277.i" = add nuw i64 %CurrWI..3.i, 1
  %"loadedCurrSB+Stride279.i" = add nuw i64 %CurrSBIndex..3.i, 560
  br label %SyncBB235.i

SyncBB236.i:                                      ; preds = %thenBB280.i, %SyncBB235.i
  %CurrWI..4.i = phi i64 [ %"CurrWI++284.i", %thenBB280.i ], [ 0, %SyncBB235.i ]
  %CurrSBIndex..4.i = phi i64 [ %"loadedCurrSB+Stride286.i", %thenBB280.i ], [ 0, %SyncBB235.i ]
  %"&(pSB[currWI].offset)58.i" = add nuw i64 %CurrSBIndex..4.i, 448
  %"&pSB[currWI].offset59.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)58.i"
  %CastToValueType60.i = bitcast i8* %"&pSB[currWI].offset59.i" to i32*
  %loadedValue61.i = load i32* %CastToValueType60.i, align 4
  %84 = add nsw i32 %loadedValue61.i, -2
  %85 = sext i32 %84 to i64
  %86 = getelementptr inbounds [512 x i32] addrspace(3)* %67, i64 0, i64 %85
  %87 = load i32 addrspace(3)* %86, align 4
  %"&(pSB[currWI].offset)170.i" = add nuw i64 %CurrSBIndex..4.i, 476
  %"&pSB[currWI].offset171.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)170.i"
  %CastToValueType172.i = bitcast i8* %"&pSB[currWI].offset171.i" to i32*
  store i32 %87, i32* %CastToValueType172.i, align 4
  %check.WI.iter283.i = icmp ult i64 %CurrWI..4.i, %iterCount
  br i1 %check.WI.iter283.i, label %thenBB280.i, label %SyncBB237.i

thenBB280.i:                                      ; preds = %SyncBB236.i
  %"CurrWI++284.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride286.i" = add nuw i64 %CurrSBIndex..4.i, 560
  br label %SyncBB236.i

SyncBB237.i:                                      ; preds = %thenBB287.i, %SyncBB236.i
  %CurrWI..5.i = phi i64 [ %"CurrWI++291.i", %thenBB287.i ], [ 0, %SyncBB236.i ]
  %CurrSBIndex..5.i = phi i64 [ %"loadedCurrSB+Stride293.i", %thenBB287.i ], [ 0, %SyncBB236.i ]
  %"&(pSB[currWI].offset)137.i" = add nuw i64 %CurrSBIndex..5.i, 456
  %"&pSB[currWI].offset138.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)137.i"
  %CastToValueType139.i = bitcast i8* %"&pSB[currWI].offset138.i" to i32 addrspace(3)**
  %loadedValue140.i = load i32 addrspace(3)** %CastToValueType139.i, align 8
  %88 = load i32 addrspace(3)* %loadedValue140.i, align 4
  %"&(pSB[currWI].offset)174.i" = add nuw i64 %CurrSBIndex..5.i, 476
  %"&pSB[currWI].offset175.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)174.i"
  %CastToValueType176.i = bitcast i8* %"&pSB[currWI].offset175.i" to i32*
  %loadedValue177.i = load i32* %CastToValueType176.i, align 4
  %89 = add i32 %88, %loadedValue177.i
  %"&(pSB[currWI].offset)132.i" = add nuw i64 %CurrSBIndex..5.i, 456
  %"&pSB[currWI].offset133.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)132.i"
  %CastToValueType134.i = bitcast i8* %"&pSB[currWI].offset133.i" to i32 addrspace(3)**
  %loadedValue135.i = load i32 addrspace(3)** %CastToValueType134.i, align 8
  store i32 %89, i32 addrspace(3)* %loadedValue135.i, align 4
  %check.WI.iter290.i = icmp ult i64 %CurrWI..5.i, %iterCount
  br i1 %check.WI.iter290.i, label %thenBB287.i, label %SyncBB238.i

thenBB287.i:                                      ; preds = %SyncBB237.i
  %"CurrWI++291.i" = add nuw i64 %CurrWI..5.i, 1
  %"loadedCurrSB+Stride293.i" = add nuw i64 %CurrSBIndex..5.i, 560
  br label %SyncBB237.i

SyncBB238.i:                                      ; preds = %thenBB294.i, %SyncBB237.i
  %CurrWI..6.i = phi i64 [ %"CurrWI++298.i", %thenBB294.i ], [ 0, %SyncBB237.i ]
  %CurrSBIndex..6.i = phi i64 [ %"loadedCurrSB+Stride300.i", %thenBB294.i ], [ 0, %SyncBB237.i ]
  %"&(pSB[currWI].offset)53.i" = add nuw i64 %CurrSBIndex..6.i, 448
  %"&pSB[currWI].offset54.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)53.i"
  %CastToValueType55.i = bitcast i8* %"&pSB[currWI].offset54.i" to i32*
  %loadedValue56.i = load i32* %CastToValueType55.i, align 4
  %90 = add nsw i32 %loadedValue56.i, -4
  %91 = sext i32 %90 to i64
  %92 = getelementptr inbounds [512 x i32] addrspace(3)* %67, i64 0, i64 %91
  %93 = load i32 addrspace(3)* %92, align 4
  %"&(pSB[currWI].offset)179.i" = add nuw i64 %CurrSBIndex..6.i, 480
  %"&pSB[currWI].offset180.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)179.i"
  %CastToValueType181.i = bitcast i8* %"&pSB[currWI].offset180.i" to i32*
  store i32 %93, i32* %CastToValueType181.i, align 4
  %check.WI.iter297.i = icmp ult i64 %CurrWI..6.i, %iterCount
  br i1 %check.WI.iter297.i, label %thenBB294.i, label %SyncBB239.i

thenBB294.i:                                      ; preds = %SyncBB238.i
  %"CurrWI++298.i" = add nuw i64 %CurrWI..6.i, 1
  %"loadedCurrSB+Stride300.i" = add nuw i64 %CurrSBIndex..6.i, 560
  br label %SyncBB238.i

SyncBB239.i:                                      ; preds = %thenBB301.i, %SyncBB238.i
  %CurrWI..7.i = phi i64 [ %"CurrWI++305.i", %thenBB301.i ], [ 0, %SyncBB238.i ]
  %CurrSBIndex..7.i = phi i64 [ %"loadedCurrSB+Stride307.i", %thenBB301.i ], [ 0, %SyncBB238.i ]
  %"&(pSB[currWI].offset)127.i" = add nuw i64 %CurrSBIndex..7.i, 456
  %"&pSB[currWI].offset128.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)127.i"
  %CastToValueType129.i = bitcast i8* %"&pSB[currWI].offset128.i" to i32 addrspace(3)**
  %loadedValue130.i = load i32 addrspace(3)** %CastToValueType129.i, align 8
  %94 = load i32 addrspace(3)* %loadedValue130.i, align 4
  %"&(pSB[currWI].offset)183.i" = add nuw i64 %CurrSBIndex..7.i, 480
  %"&pSB[currWI].offset184.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)183.i"
  %CastToValueType185.i = bitcast i8* %"&pSB[currWI].offset184.i" to i32*
  %loadedValue186.i = load i32* %CastToValueType185.i, align 4
  %95 = add i32 %94, %loadedValue186.i
  %"&(pSB[currWI].offset)122.i" = add nuw i64 %CurrSBIndex..7.i, 456
  %"&pSB[currWI].offset123.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)122.i"
  %CastToValueType124.i = bitcast i8* %"&pSB[currWI].offset123.i" to i32 addrspace(3)**
  %loadedValue125.i = load i32 addrspace(3)** %CastToValueType124.i, align 8
  store i32 %95, i32 addrspace(3)* %loadedValue125.i, align 4
  %check.WI.iter304.i = icmp ult i64 %CurrWI..7.i, %iterCount
  br i1 %check.WI.iter304.i, label %thenBB301.i, label %SyncBB240.i

thenBB301.i:                                      ; preds = %SyncBB239.i
  %"CurrWI++305.i" = add nuw i64 %CurrWI..7.i, 1
  %"loadedCurrSB+Stride307.i" = add nuw i64 %CurrSBIndex..7.i, 560
  br label %SyncBB239.i

SyncBB240.i:                                      ; preds = %thenBB308.i, %SyncBB239.i
  %CurrWI..8.i = phi i64 [ %"CurrWI++312.i", %thenBB308.i ], [ 0, %SyncBB239.i ]
  %CurrSBIndex..8.i = phi i64 [ %"loadedCurrSB+Stride314.i", %thenBB308.i ], [ 0, %SyncBB239.i ]
  %"&(pSB[currWI].offset)48.i" = add nuw i64 %CurrSBIndex..8.i, 448
  %"&pSB[currWI].offset49.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)48.i"
  %CastToValueType50.i = bitcast i8* %"&pSB[currWI].offset49.i" to i32*
  %loadedValue51.i = load i32* %CastToValueType50.i, align 4
  %96 = add nsw i32 %loadedValue51.i, -8
  %97 = sext i32 %96 to i64
  %98 = getelementptr inbounds [512 x i32] addrspace(3)* %67, i64 0, i64 %97
  %99 = load i32 addrspace(3)* %98, align 4
  %"&(pSB[currWI].offset)188.i" = add nuw i64 %CurrSBIndex..8.i, 484
  %"&pSB[currWI].offset189.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)188.i"
  %CastToValueType190.i = bitcast i8* %"&pSB[currWI].offset189.i" to i32*
  store i32 %99, i32* %CastToValueType190.i, align 4
  %check.WI.iter311.i = icmp ult i64 %CurrWI..8.i, %iterCount
  br i1 %check.WI.iter311.i, label %thenBB308.i, label %SyncBB241.i

thenBB308.i:                                      ; preds = %SyncBB240.i
  %"CurrWI++312.i" = add nuw i64 %CurrWI..8.i, 1
  %"loadedCurrSB+Stride314.i" = add nuw i64 %CurrSBIndex..8.i, 560
  br label %SyncBB240.i

SyncBB241.i:                                      ; preds = %thenBB315.i, %SyncBB240.i
  %CurrWI..9.i = phi i64 [ %"CurrWI++319.i", %thenBB315.i ], [ 0, %SyncBB240.i ]
  %CurrSBIndex..9.i = phi i64 [ %"loadedCurrSB+Stride321.i", %thenBB315.i ], [ 0, %SyncBB240.i ]
  %"&(pSB[currWI].offset)117.i" = add nuw i64 %CurrSBIndex..9.i, 456
  %"&pSB[currWI].offset118.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)117.i"
  %CastToValueType119.i = bitcast i8* %"&pSB[currWI].offset118.i" to i32 addrspace(3)**
  %loadedValue120.i = load i32 addrspace(3)** %CastToValueType119.i, align 8
  %100 = load i32 addrspace(3)* %loadedValue120.i, align 4
  %"&(pSB[currWI].offset)192.i" = add nuw i64 %CurrSBIndex..9.i, 484
  %"&pSB[currWI].offset193.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)192.i"
  %CastToValueType194.i = bitcast i8* %"&pSB[currWI].offset193.i" to i32*
  %loadedValue195.i = load i32* %CastToValueType194.i, align 4
  %101 = add i32 %100, %loadedValue195.i
  %"&(pSB[currWI].offset)112.i" = add nuw i64 %CurrSBIndex..9.i, 456
  %"&pSB[currWI].offset113.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)112.i"
  %CastToValueType114.i = bitcast i8* %"&pSB[currWI].offset113.i" to i32 addrspace(3)**
  %loadedValue115.i = load i32 addrspace(3)** %CastToValueType114.i, align 8
  store i32 %101, i32 addrspace(3)* %loadedValue115.i, align 4
  %check.WI.iter318.i = icmp ult i64 %CurrWI..9.i, %iterCount
  br i1 %check.WI.iter318.i, label %thenBB315.i, label %SyncBB242.i

thenBB315.i:                                      ; preds = %SyncBB241.i
  %"CurrWI++319.i" = add nuw i64 %CurrWI..9.i, 1
  %"loadedCurrSB+Stride321.i" = add nuw i64 %CurrSBIndex..9.i, 560
  br label %SyncBB241.i

SyncBB242.i:                                      ; preds = %thenBB322.i, %SyncBB241.i
  %CurrWI..10.i = phi i64 [ %"CurrWI++326.i", %thenBB322.i ], [ 0, %SyncBB241.i ]
  %CurrSBIndex..10.i = phi i64 [ %"loadedCurrSB+Stride328.i", %thenBB322.i ], [ 0, %SyncBB241.i ]
  %"&(pSB[currWI].offset)43.i" = add nuw i64 %CurrSBIndex..10.i, 448
  %"&pSB[currWI].offset44.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)43.i"
  %CastToValueType45.i = bitcast i8* %"&pSB[currWI].offset44.i" to i32*
  %loadedValue46.i = load i32* %CastToValueType45.i, align 4
  %102 = add nsw i32 %loadedValue46.i, -16
  %103 = sext i32 %102 to i64
  %104 = getelementptr inbounds [512 x i32] addrspace(3)* %67, i64 0, i64 %103
  %105 = load i32 addrspace(3)* %104, align 4
  %"&(pSB[currWI].offset)197.i" = add nuw i64 %CurrSBIndex..10.i, 488
  %"&pSB[currWI].offset198.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)197.i"
  %CastToValueType199.i = bitcast i8* %"&pSB[currWI].offset198.i" to i32*
  store i32 %105, i32* %CastToValueType199.i, align 4
  %check.WI.iter325.i = icmp ult i64 %CurrWI..10.i, %iterCount
  br i1 %check.WI.iter325.i, label %thenBB322.i, label %SyncBB243.i

thenBB322.i:                                      ; preds = %SyncBB242.i
  %"CurrWI++326.i" = add nuw i64 %CurrWI..10.i, 1
  %"loadedCurrSB+Stride328.i" = add nuw i64 %CurrSBIndex..10.i, 560
  br label %SyncBB242.i

SyncBB243.i:                                      ; preds = %thenBB329.i, %SyncBB242.i
  %CurrWI..11.i = phi i64 [ %"CurrWI++333.i", %thenBB329.i ], [ 0, %SyncBB242.i ]
  %CurrSBIndex..11.i = phi i64 [ %"loadedCurrSB+Stride335.i", %thenBB329.i ], [ 0, %SyncBB242.i ]
  %"&(pSB[currWI].offset)107.i" = add nuw i64 %CurrSBIndex..11.i, 456
  %"&pSB[currWI].offset108.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)107.i"
  %CastToValueType109.i = bitcast i8* %"&pSB[currWI].offset108.i" to i32 addrspace(3)**
  %loadedValue110.i = load i32 addrspace(3)** %CastToValueType109.i, align 8
  %106 = load i32 addrspace(3)* %loadedValue110.i, align 4
  %"&(pSB[currWI].offset)201.i" = add nuw i64 %CurrSBIndex..11.i, 488
  %"&pSB[currWI].offset202.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)201.i"
  %CastToValueType203.i = bitcast i8* %"&pSB[currWI].offset202.i" to i32*
  %loadedValue204.i = load i32* %CastToValueType203.i, align 4
  %107 = add i32 %106, %loadedValue204.i
  %"&(pSB[currWI].offset)102.i" = add nuw i64 %CurrSBIndex..11.i, 456
  %"&pSB[currWI].offset103.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)102.i"
  %CastToValueType104.i = bitcast i8* %"&pSB[currWI].offset103.i" to i32 addrspace(3)**
  %loadedValue105.i = load i32 addrspace(3)** %CastToValueType104.i, align 8
  store i32 %107, i32 addrspace(3)* %loadedValue105.i, align 4
  %check.WI.iter332.i = icmp ult i64 %CurrWI..11.i, %iterCount
  br i1 %check.WI.iter332.i, label %thenBB329.i, label %SyncBB244.i

thenBB329.i:                                      ; preds = %SyncBB243.i
  %"CurrWI++333.i" = add nuw i64 %CurrWI..11.i, 1
  %"loadedCurrSB+Stride335.i" = add nuw i64 %CurrSBIndex..11.i, 560
  br label %SyncBB243.i

SyncBB244.i:                                      ; preds = %thenBB336.i, %SyncBB243.i
  %CurrWI..12.i = phi i64 [ %"CurrWI++340.i", %thenBB336.i ], [ 0, %SyncBB243.i ]
  %CurrSBIndex..12.i = phi i64 [ %"loadedCurrSB+Stride342.i", %thenBB336.i ], [ 0, %SyncBB243.i ]
  %"&(pSB[currWI].offset)38.i" = add nuw i64 %CurrSBIndex..12.i, 448
  %"&pSB[currWI].offset39.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)38.i"
  %CastToValueType40.i = bitcast i8* %"&pSB[currWI].offset39.i" to i32*
  %loadedValue41.i = load i32* %CastToValueType40.i, align 4
  %108 = add nsw i32 %loadedValue41.i, -32
  %109 = sext i32 %108 to i64
  %110 = getelementptr inbounds [512 x i32] addrspace(3)* %67, i64 0, i64 %109
  %111 = load i32 addrspace(3)* %110, align 4
  %"&(pSB[currWI].offset)206.i" = add nuw i64 %CurrSBIndex..12.i, 492
  %"&pSB[currWI].offset207.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)206.i"
  %CastToValueType208.i = bitcast i8* %"&pSB[currWI].offset207.i" to i32*
  store i32 %111, i32* %CastToValueType208.i, align 4
  %check.WI.iter339.i = icmp ult i64 %CurrWI..12.i, %iterCount
  br i1 %check.WI.iter339.i, label %thenBB336.i, label %SyncBB245.i

thenBB336.i:                                      ; preds = %SyncBB244.i
  %"CurrWI++340.i" = add nuw i64 %CurrWI..12.i, 1
  %"loadedCurrSB+Stride342.i" = add nuw i64 %CurrSBIndex..12.i, 560
  br label %SyncBB244.i

SyncBB245.i:                                      ; preds = %thenBB343.i, %SyncBB244.i
  %CurrWI..13.i = phi i64 [ %"CurrWI++347.i", %thenBB343.i ], [ 0, %SyncBB244.i ]
  %CurrSBIndex..13.i = phi i64 [ %"loadedCurrSB+Stride349.i", %thenBB343.i ], [ 0, %SyncBB244.i ]
  %"&(pSB[currWI].offset)97.i" = add nuw i64 %CurrSBIndex..13.i, 456
  %"&pSB[currWI].offset98.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)97.i"
  %CastToValueType99.i = bitcast i8* %"&pSB[currWI].offset98.i" to i32 addrspace(3)**
  %loadedValue100.i = load i32 addrspace(3)** %CastToValueType99.i, align 8
  %112 = load i32 addrspace(3)* %loadedValue100.i, align 4
  %"&(pSB[currWI].offset)210.i" = add nuw i64 %CurrSBIndex..13.i, 492
  %"&pSB[currWI].offset211.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)210.i"
  %CastToValueType212.i = bitcast i8* %"&pSB[currWI].offset211.i" to i32*
  %loadedValue213.i = load i32* %CastToValueType212.i, align 4
  %113 = add i32 %112, %loadedValue213.i
  %"&(pSB[currWI].offset)92.i" = add nuw i64 %CurrSBIndex..13.i, 456
  %"&pSB[currWI].offset93.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)92.i"
  %CastToValueType94.i = bitcast i8* %"&pSB[currWI].offset93.i" to i32 addrspace(3)**
  %loadedValue95.i = load i32 addrspace(3)** %CastToValueType94.i, align 8
  store i32 %113, i32 addrspace(3)* %loadedValue95.i, align 4
  %check.WI.iter346.i = icmp ult i64 %CurrWI..13.i, %iterCount
  br i1 %check.WI.iter346.i, label %thenBB343.i, label %SyncBB246.i

thenBB343.i:                                      ; preds = %SyncBB245.i
  %"CurrWI++347.i" = add nuw i64 %CurrWI..13.i, 1
  %"loadedCurrSB+Stride349.i" = add nuw i64 %CurrSBIndex..13.i, 560
  br label %SyncBB245.i

SyncBB246.i:                                      ; preds = %thenBB350.i, %SyncBB245.i
  %CurrWI..14.i = phi i64 [ %"CurrWI++354.i", %thenBB350.i ], [ 0, %SyncBB245.i ]
  %CurrSBIndex..14.i = phi i64 [ %"loadedCurrSB+Stride356.i", %thenBB350.i ], [ 0, %SyncBB245.i ]
  %"&(pSB[currWI].offset)33.i" = add nuw i64 %CurrSBIndex..14.i, 448
  %"&pSB[currWI].offset34.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)33.i"
  %CastToValueType35.i = bitcast i8* %"&pSB[currWI].offset34.i" to i32*
  %loadedValue36.i = load i32* %CastToValueType35.i, align 4
  %114 = add nsw i32 %loadedValue36.i, -64
  %115 = sext i32 %114 to i64
  %116 = getelementptr inbounds [512 x i32] addrspace(3)* %67, i64 0, i64 %115
  %117 = load i32 addrspace(3)* %116, align 4
  %"&(pSB[currWI].offset)215.i" = add nuw i64 %CurrSBIndex..14.i, 496
  %"&pSB[currWI].offset216.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)215.i"
  %CastToValueType217.i = bitcast i8* %"&pSB[currWI].offset216.i" to i32*
  store i32 %117, i32* %CastToValueType217.i, align 4
  %check.WI.iter353.i = icmp ult i64 %CurrWI..14.i, %iterCount
  br i1 %check.WI.iter353.i, label %thenBB350.i, label %SyncBB247.i

thenBB350.i:                                      ; preds = %SyncBB246.i
  %"CurrWI++354.i" = add nuw i64 %CurrWI..14.i, 1
  %"loadedCurrSB+Stride356.i" = add nuw i64 %CurrSBIndex..14.i, 560
  br label %SyncBB246.i

SyncBB247.i:                                      ; preds = %thenBB357.i, %SyncBB246.i
  %CurrWI..15.i = phi i64 [ %"CurrWI++361.i", %thenBB357.i ], [ 0, %SyncBB246.i ]
  %CurrSBIndex..15.i = phi i64 [ %"loadedCurrSB+Stride363.i", %thenBB357.i ], [ 0, %SyncBB246.i ]
  %"&(pSB[currWI].offset)87.i" = add nuw i64 %CurrSBIndex..15.i, 456
  %"&pSB[currWI].offset88.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)87.i"
  %CastToValueType89.i = bitcast i8* %"&pSB[currWI].offset88.i" to i32 addrspace(3)**
  %loadedValue90.i = load i32 addrspace(3)** %CastToValueType89.i, align 8
  %118 = load i32 addrspace(3)* %loadedValue90.i, align 4
  %"&(pSB[currWI].offset)219.i" = add nuw i64 %CurrSBIndex..15.i, 496
  %"&pSB[currWI].offset220.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)219.i"
  %CastToValueType221.i = bitcast i8* %"&pSB[currWI].offset220.i" to i32*
  %loadedValue222.i = load i32* %CastToValueType221.i, align 4
  %119 = add i32 %118, %loadedValue222.i
  %"&(pSB[currWI].offset)82.i" = add nuw i64 %CurrSBIndex..15.i, 456
  %"&pSB[currWI].offset83.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)82.i"
  %CastToValueType84.i = bitcast i8* %"&pSB[currWI].offset83.i" to i32 addrspace(3)**
  %loadedValue85.i = load i32 addrspace(3)** %CastToValueType84.i, align 8
  store i32 %119, i32 addrspace(3)* %loadedValue85.i, align 4
  %check.WI.iter360.i = icmp ult i64 %CurrWI..15.i, %iterCount
  br i1 %check.WI.iter360.i, label %thenBB357.i, label %SyncBB248.i

thenBB357.i:                                      ; preds = %SyncBB247.i
  %"CurrWI++361.i" = add nuw i64 %CurrWI..15.i, 1
  %"loadedCurrSB+Stride363.i" = add nuw i64 %CurrSBIndex..15.i, 560
  br label %SyncBB247.i

SyncBB248.i:                                      ; preds = %thenBB364.i, %SyncBB247.i
  %CurrWI..16.i = phi i64 [ %"CurrWI++368.i", %thenBB364.i ], [ 0, %SyncBB247.i ]
  %CurrSBIndex..16.i = phi i64 [ %"loadedCurrSB+Stride370.i", %thenBB364.i ], [ 0, %SyncBB247.i ]
  %"&(pSB[currWI].offset)28.i" = add nuw i64 %CurrSBIndex..16.i, 448
  %"&pSB[currWI].offset29.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)28.i"
  %CastToValueType30.i = bitcast i8* %"&pSB[currWI].offset29.i" to i32*
  %loadedValue31.i = load i32* %CastToValueType30.i, align 4
  %120 = add nsw i32 %loadedValue31.i, -128
  %121 = sext i32 %120 to i64
  %122 = getelementptr inbounds [512 x i32] addrspace(3)* %67, i64 0, i64 %121
  %123 = load i32 addrspace(3)* %122, align 4
  %"&(pSB[currWI].offset)224.i" = add nuw i64 %CurrSBIndex..16.i, 500
  %"&pSB[currWI].offset225.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)224.i"
  %CastToValueType226.i = bitcast i8* %"&pSB[currWI].offset225.i" to i32*
  store i32 %123, i32* %CastToValueType226.i, align 4
  %check.WI.iter367.i = icmp ult i64 %CurrWI..16.i, %iterCount
  br i1 %check.WI.iter367.i, label %thenBB364.i, label %SyncBB249.i

thenBB364.i:                                      ; preds = %SyncBB248.i
  %"CurrWI++368.i" = add nuw i64 %CurrWI..16.i, 1
  %"loadedCurrSB+Stride370.i" = add nuw i64 %CurrSBIndex..16.i, 560
  br label %SyncBB248.i

SyncBB249.i:                                      ; preds = %thenBB371.i, %SyncBB248.i
  %CurrWI..17.i = phi i64 [ %"CurrWI++375.i", %thenBB371.i ], [ 0, %SyncBB248.i ]
  %CurrSBIndex..17.i = phi i64 [ %"loadedCurrSB+Stride377.i", %thenBB371.i ], [ 0, %SyncBB248.i ]
  %"&(pSB[currWI].offset)77.i" = add nuw i64 %CurrSBIndex..17.i, 456
  %"&pSB[currWI].offset78.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)77.i"
  %CastToValueType79.i = bitcast i8* %"&pSB[currWI].offset78.i" to i32 addrspace(3)**
  %loadedValue80.i = load i32 addrspace(3)** %CastToValueType79.i, align 8
  %124 = load i32 addrspace(3)* %loadedValue80.i, align 4
  %"&(pSB[currWI].offset)228.i" = add nuw i64 %CurrSBIndex..17.i, 500
  %"&pSB[currWI].offset229.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)228.i"
  %CastToValueType230.i = bitcast i8* %"&pSB[currWI].offset229.i" to i32*
  %loadedValue231.i = load i32* %CastToValueType230.i, align 4
  %125 = add i32 %124, %loadedValue231.i
  %"&(pSB[currWI].offset)72.i" = add nuw i64 %CurrSBIndex..17.i, 456
  %"&pSB[currWI].offset73.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)72.i"
  %CastToValueType74.i = bitcast i8* %"&pSB[currWI].offset73.i" to i32 addrspace(3)**
  %loadedValue75.i = load i32 addrspace(3)** %CastToValueType74.i, align 8
  store i32 %125, i32 addrspace(3)* %loadedValue75.i, align 4
  %check.WI.iter374.i = icmp ult i64 %CurrWI..17.i, %iterCount
  br i1 %check.WI.iter374.i, label %thenBB371.i, label %SyncBB250.i

thenBB371.i:                                      ; preds = %SyncBB249.i
  %"CurrWI++375.i" = add nuw i64 %CurrWI..17.i, 1
  %"loadedCurrSB+Stride377.i" = add nuw i64 %CurrSBIndex..17.i, 560
  br label %SyncBB249.i

SyncBB250.i:                                      ; preds = %thenBB.i, %SyncBB249.i
  %CurrWI..18.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %SyncBB249.i ]
  %CurrSBIndex..18.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %SyncBB249.i ]
  %"&(pSB[currWI].offset)156.i" = add nuw i64 %CurrSBIndex..18.i, 464
  %"&pSB[currWI].offset157.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)156.i"
  %CastToValueType158.i = bitcast i8* %"&pSB[currWI].offset157.i" to i32 addrspace(3)**
  %loadedValue159.i = load i32 addrspace(3)** %CastToValueType158.i, align 8
  %126 = load i32 addrspace(3)* %loadedValue159.i, align 4
  %"&(pSB[currWI].offset)8.i" = add nuw i64 %CurrSBIndex..18.i, 436
  %"&pSB[currWI].offset9.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)8.i"
  %CastToArgType10.i = bitcast i8* %"&pSB[currWI].offset9.i" to i32*
  store i32 %126, i32* %CastToArgType10.i, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..18.i, %iterCount
  br i1 %check.WI.iter.i, label %thenBB.i, label %"Barrier BB3"

thenBB.i:                                         ; preds = %SyncBB250.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..18.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..18.i, 560
  br label %SyncBB250.i

"Barrier BB3":                                    ; preds = %SyncBB250.i
  store i64 0, i64* %pCurrWI, align 8
  %127 = icmp eq i32 %storeSum, 0
  %128 = bitcast i32 addrspace(1)* %g_odata to <4 x i32> addrspace(1)*
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %"Barrier BB3"
  %CurrSBIndex..0 = phi i64 [ 0, %"Barrier BB3" ], [ %"loadedCurrSB+Stride", %thenBB ]
  br i1 %127, label %phi-split-bb, label %129

; <label>:129                                     ; preds = %SyncBB
  %"&(pSB[currWI].offset)18" = add nuw i64 %CurrSBIndex..0, 392
  %"&pSB[currWI].offset19" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)18"
  %CastToValueType20 = bitcast i8* %"&pSB[currWI].offset19" to i64*
  %loadedValue21 = load i64* %CastToValueType20, align 8
  %sext = shl i64 %loadedValue21, 32
  %130 = ashr i64 %sext, 32
  %131 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %132 = load i64* %131, align 8
  %133 = add i64 %132, -1
  %134 = icmp eq i64 %130, %133
  br i1 %134, label %135, label %phi-split-bb

; <label>:135                                     ; preds = %129
  %"&(pSB[currWI].offset)96" = add nuw i64 %CurrSBIndex..0, 416
  %"&pSB[currWI].offset97" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)96"
  %CastToValueType98 = bitcast i8* %"&pSB[currWI].offset97" to <4 x i32>*
  %loadedValue99 = load <4 x i32>* %CastToValueType98, align 16
  %136 = extractelement <4 x i32> %loadedValue99, i32 3
  %"&(pSB[currWI].offset)125" = add nuw i64 %CurrSBIndex..0, 436
  %"&pSB[currWI].offset126" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)125"
  %CastToValueType127 = bitcast i8* %"&pSB[currWI].offset126" to i32*
  %loadedValue128 = load i32* %CastToValueType127, align 4
  %137 = add i32 %loadedValue128, %136
  %138 = load i64* %pWGId, align 8
  %139 = getelementptr inbounds i32 addrspace(1)* %g_blockSums, i64 %138
  store i32 %137, i32 addrspace(1)* %139, align 4
  br label %phi-split-bb

phi-split-bb:                                     ; preds = %SyncBB, %135, %129
  %"&(pSB[currWI].offset)130" = add nuw i64 %CurrSBIndex..0, 436
  %"&pSB[currWI].offset131" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)130"
  %CastToValueType132 = bitcast i8* %"&pSB[currWI].offset131" to i32*
  %loadedValue133 = load i32* %CastToValueType132, align 4
  %140 = insertelement <4 x i32> undef, i32 %loadedValue133, i32 0
  %"&(pSB[currWI].offset)91" = add nuw i64 %CurrSBIndex..0, 416
  %"&pSB[currWI].offset92" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)91"
  %CastToValueType93 = bitcast i8* %"&pSB[currWI].offset92" to <4 x i32>*
  %loadedValue94 = load <4 x i32>* %CastToValueType93, align 16
  %141 = extractelement <4 x i32> %loadedValue94, i32 0
  %"&(pSB[currWI].offset)120" = add nuw i64 %CurrSBIndex..0, 436
  %"&pSB[currWI].offset121" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)120"
  %CastToValueType122 = bitcast i8* %"&pSB[currWI].offset121" to i32*
  %loadedValue123 = load i32* %CastToValueType122, align 4
  %142 = add i32 %loadedValue123, %141
  %143 = insertelement <4 x i32> %140, i32 %142, i32 1
  %"&(pSB[currWI].offset)86" = add nuw i64 %CurrSBIndex..0, 416
  %"&pSB[currWI].offset87" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)86"
  %CastToValueType88 = bitcast i8* %"&pSB[currWI].offset87" to <4 x i32>*
  %loadedValue89 = load <4 x i32>* %CastToValueType88, align 16
  %144 = extractelement <4 x i32> %loadedValue89, i32 1
  %"&(pSB[currWI].offset)115" = add nuw i64 %CurrSBIndex..0, 436
  %"&pSB[currWI].offset116" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)115"
  %CastToValueType117 = bitcast i8* %"&pSB[currWI].offset116" to i32*
  %loadedValue118 = load i32* %CastToValueType117, align 4
  %145 = add i32 %loadedValue118, %144
  %146 = insertelement <4 x i32> %143, i32 %145, i32 2
  %"&(pSB[currWI].offset)81" = add nuw i64 %CurrSBIndex..0, 416
  %"&pSB[currWI].offset82" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)81"
  %CastToValueType83 = bitcast i8* %"&pSB[currWI].offset82" to <4 x i32>*
  %loadedValue84 = load <4 x i32>* %CastToValueType83, align 16
  %147 = extractelement <4 x i32> %loadedValue84, i32 2
  %"&(pSB[currWI].offset)110" = add nuw i64 %CurrSBIndex..0, 436
  %"&pSB[currWI].offset111" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)110"
  %CastToValueType112 = bitcast i8* %"&pSB[currWI].offset111" to i32*
  %loadedValue113 = load i32* %CastToValueType112, align 4
  %148 = add i32 %loadedValue113, %147
  %149 = insertelement <4 x i32> %146, i32 %148, i32 3
  br i1 %1, label %150, label %153

; <label>:150                                     ; preds = %phi-split-bb
  %"&(pSB[currWI].offset)37" = add nuw i64 %CurrSBIndex..0, 400
  %"&pSB[currWI].offset38" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)37"
  %CastToValueType39 = bitcast i8* %"&pSB[currWI].offset38" to i32*
  %loadedValue40 = load i32* %CastToValueType39, align 4
  %151 = or i32 %loadedValue40, 3
  %152 = icmp slt i32 %151, %n
  br i1 %152, label %153, label %156

; <label>:153                                     ; preds = %150, %phi-split-bb
  %"&(pSB[currWI].offset)9" = add nuw i64 %CurrSBIndex..0, 384
  %"&pSB[currWI].offset10" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)9"
  %CastToValueType11 = bitcast i8* %"&pSB[currWI].offset10" to i32*
  %loadedValue12 = load i32* %CastToValueType11, align 4
  %154 = sext i32 %loadedValue12 to i64
  %155 = getelementptr inbounds <4 x i32> addrspace(1)* %128, i64 %154
  store <4 x i32> %149, <4 x i32> addrspace(1)* %155, align 16
  br label %UnifiedReturnBlock

; <label>:156                                     ; preds = %150
  %"&(pSB[currWI].offset)62" = add nuw i64 %CurrSBIndex..0, 400
  %"&pSB[currWI].offset63" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)62"
  %CastToValueType64 = bitcast i8* %"&pSB[currWI].offset63" to i32*
  %loadedValue65 = load i32* %CastToValueType64, align 4
  %157 = icmp slt i32 %loadedValue65, %n
  br i1 %157, label %158, label %UnifiedReturnBlock

; <label>:158                                     ; preds = %156
  %"&(pSB[currWI].offset)57" = add nuw i64 %CurrSBIndex..0, 400
  %"&pSB[currWI].offset58" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)57"
  %CastToValueType59 = bitcast i8* %"&pSB[currWI].offset58" to i32*
  %loadedValue60 = load i32* %CastToValueType59, align 4
  %159 = sext i32 %loadedValue60 to i64
  %160 = getelementptr inbounds i32 addrspace(1)* %g_odata, i64 %159
  %"&(pSB[currWI].offset)135" = add nuw i64 %CurrSBIndex..0, 436
  %"&pSB[currWI].offset136" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)135"
  %CastToValueType137 = bitcast i8* %"&pSB[currWI].offset136" to i32*
  %loadedValue138 = load i32* %CastToValueType137, align 4
  store i32 %loadedValue138, i32 addrspace(1)* %160, align 4
  %"&(pSB[currWI].offset)32" = add nuw i64 %CurrSBIndex..0, 400
  %"&pSB[currWI].offset33" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)32"
  %CastToValueType34 = bitcast i8* %"&pSB[currWI].offset33" to i32*
  %loadedValue35 = load i32* %CastToValueType34, align 4
  %161 = or i32 %loadedValue35, 1
  %162 = icmp slt i32 %161, %n
  br i1 %162, label %163, label %UnifiedReturnBlock

; <label>:163                                     ; preds = %158
  %164 = sext i32 %161 to i64
  %165 = getelementptr inbounds i32 addrspace(1)* %g_odata, i64 %164
  store i32 %142, i32 addrspace(1)* %165, align 4
  %"&(pSB[currWI].offset)27" = add nuw i64 %CurrSBIndex..0, 400
  %"&pSB[currWI].offset28" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)27"
  %CastToValueType29 = bitcast i8* %"&pSB[currWI].offset28" to i32*
  %loadedValue30 = load i32* %CastToValueType29, align 4
  %166 = or i32 %loadedValue30, 2
  %167 = icmp slt i32 %166, %n
  br i1 %167, label %168, label %UnifiedReturnBlock

; <label>:168                                     ; preds = %163
  %169 = sext i32 %166 to i64
  %170 = getelementptr inbounds i32 addrspace(1)* %g_odata, i64 %169
  store i32 %145, i32 addrspace(1)* %170, align 4
  br label %UnifiedReturnBlock

UnifiedReturnBlock:                               ; preds = %168, %156, %163, %158, %153
  %loadedCurrWI = load i64* %pCurrWI, align 8
  %check.WI.iter = icmp ult i64 %loadedCurrWI, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %UnifiedReturnBlock
  %"CurrWI++" = add nuw i64 %loadedCurrWI, 1
  store i64 %"CurrWI++", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 560
  br label %SyncBB

elseBB:                                           ; preds = %UnifiedReturnBlock
  store i64 0, i64* %pCurrWI, align 8
  ret void
}

define void @reorderData(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i32*
  %1 = load i32* %0, align 4
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32 addrspace(1)**
  %4 = load i32 addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32 addrspace(1)**
  %7 = load i32 addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to <2 x i32> addrspace(1)**
  %10 = load <2 x i32> addrspace(1)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to <2 x i32> addrspace(1)**
  %13 = load <2 x i32> addrspace(1)** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to i32 addrspace(1)**
  %16 = load i32 addrspace(1)** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 48
  %18 = bitcast i8* %17 to i32 addrspace(1)**
  %19 = load i32 addrspace(1)** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 64
  %21 = bitcast i8* %20 to i32*
  %22 = load i32* %21, align 4
  %23 = getelementptr i8* %pBuffer, i64 72
  %24 = bitcast i8* %23 to i8 addrspace(3)**
  %25 = load i8 addrspace(3)** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 80
  %27 = bitcast i8* %26 to %struct.WorkDim**
  %28 = load %struct.WorkDim** %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 88
  %30 = bitcast i8* %29 to i64**
  %31 = load i64** %30, align 8
  %32 = getelementptr i8* %pBuffer, i64 104
  %33 = bitcast i8* %32 to %struct.PaddedDimId**
  %34 = load %struct.PaddedDimId** %33, align 8
  %35 = getelementptr i8* %pBuffer, i64 120
  %36 = bitcast i8* %35 to i64*
  %37 = load i64* %36, align 8
  %38 = bitcast i8 addrspace(3)* %25 to [256 x <2 x i32>] addrspace(3)*
  %39 = getelementptr i8 addrspace(3)* %25, i64 2048
  %40 = bitcast i8 addrspace(3)* %39 to [256 x <2 x i32>] addrspace(3)*
  %41 = getelementptr i8 addrspace(3)* %25, i64 4096
  %42 = bitcast i8 addrspace(3)* %41 to [16 x i32] addrspace(3)*
  %43 = getelementptr i8 addrspace(3)* %25, i64 4224
  %44 = bitcast i8 addrspace(3)* %43 to [16 x i32] addrspace(3)*
  %45 = zext i32 %22 to i64
  br label %SyncBB3.i

SyncBB3.i:                                        ; preds = %thenBB5.i, %entry
  %CurrWI..1.i = phi i64 [ 0, %entry ], [ %"CurrWI++9.i", %thenBB5.i ]
  %46 = getelementptr %struct.WorkDim* %28, i64 0, i32 3, i64 0
  %47 = load i64* %46, align 8
  %48 = load i64* %31, align 8
  %49 = and i64 %48, 4294967295
  %50 = mul i64 %49, %47
  %51 = getelementptr %struct.PaddedDimId* %34, i64 %CurrWI..1.i, i32 0, i64 0
  %52 = load i64* %51, align 8
  %53 = add i64 %50, %52
  %54 = and i64 %53, 4294967295
  %55 = getelementptr inbounds <2 x i32> addrspace(1)* %10, i64 %54
  %56 = load <2 x i32> addrspace(1)* %55, align 8
  %57 = getelementptr inbounds [256 x <2 x i32>] addrspace(3)* %38, i64 0, i64 %52
  store <2 x i32> %56, <2 x i32> addrspace(3)* %57, align 8
  %58 = getelementptr inbounds <2 x i32> addrspace(1)* %13, i64 %54
  %59 = load <2 x i32> addrspace(1)* %58, align 8
  %60 = getelementptr %struct.PaddedDimId* %34, i64 %CurrWI..1.i, i32 0, i64 0
  %61 = load i64* %60, align 8
  %62 = getelementptr inbounds [256 x <2 x i32>] addrspace(3)* %40, i64 0, i64 %61
  store <2 x i32> %59, <2 x i32> addrspace(3)* %62, align 8
  %63 = getelementptr %struct.PaddedDimId* %34, i64 %CurrWI..1.i, i32 0, i64 0
  %64 = load i64* %63, align 8
  %65 = icmp ult i64 %64, 16
  br i1 %65, label %66, label %82

; <label>:66                                      ; preds = %SyncBB3.i
  %67 = getelementptr %struct.PaddedDimId* %34, i64 %CurrWI..1.i, i32 0, i64 0
  %68 = load i64* %67, align 8
  %69 = mul i64 %68, %45
  %70 = add i64 %69, %49
  %71 = getelementptr inbounds i32 addrspace(1)* %19, i64 %70
  %72 = load i32 addrspace(1)* %71, align 4
  %73 = getelementptr inbounds [16 x i32] addrspace(3)* %42, i64 0, i64 %68
  store i32 %72, i32 addrspace(3)* %73, align 4
  %74 = shl i64 %48, 4
  %75 = and i64 %74, 4294967280
  %76 = getelementptr %struct.PaddedDimId* %34, i64 %CurrWI..1.i, i32 0, i64 0
  %77 = load i64* %76, align 8
  %78 = add i64 %77, %75
  %79 = getelementptr inbounds i32 addrspace(1)* %16, i64 %78
  %80 = load i32 addrspace(1)* %79, align 4
  %81 = getelementptr inbounds [16 x i32] addrspace(3)* %44, i64 0, i64 %77
  store i32 %80, i32 addrspace(3)* %81, align 4
  br label %82

; <label>:82                                      ; preds = %66, %SyncBB3.i
  %check.WI.iter8.i = icmp ult i64 %CurrWI..1.i, %37
  br i1 %check.WI.iter8.i, label %thenBB5.i, label %elseBB6.i

thenBB5.i:                                        ; preds = %82
  %"CurrWI++9.i" = add nuw i64 %CurrWI..1.i, 1
  br label %SyncBB3.i

elseBB6.i:                                        ; preds = %82
  %83 = and i64 %47, 4294967295
  br label %SyncBB4.i

SyncBB4.i:                                        ; preds = %thenBB.i, %elseBB6.i
  %CurrWI..0.i = phi i64 [ 0, %elseBB6.i ], [ %"CurrWI++.i", %thenBB.i ]
  %84 = getelementptr %struct.PaddedDimId* %34, i64 %CurrWI..0.i, i32 0, i64 0
  %85 = load i64* %84, align 8
  %86 = getelementptr inbounds [256 x <2 x i32>] addrspace(3)* %38, i64 0, i64 0, i64 %85
  %87 = load i32 addrspace(3)* %86, align 4
  %88 = lshr i32 %87, %1
  %89 = and i32 %88, 15
  %90 = zext i32 %89 to i64
  %91 = getelementptr inbounds [16 x i32] addrspace(3)* %42, i64 0, i64 %90
  %92 = load i32 addrspace(3)* %91, align 4
  %93 = zext i32 %92 to i64
  %94 = getelementptr %struct.PaddedDimId* %34, i64 %CurrWI..0.i, i32 0, i64 0
  %95 = load i64* %94, align 8
  %96 = getelementptr inbounds [16 x i32] addrspace(3)* %44, i64 0, i64 %90
  %97 = load i32 addrspace(3)* %96, align 4
  %98 = zext i32 %97 to i64
  %99 = add i64 %93, %95
  %100 = sub i64 %99, %98
  %101 = getelementptr %struct.PaddedDimId* %34, i64 %CurrWI..0.i, i32 0, i64 0
  %102 = load i64* %101, align 8
  %103 = getelementptr inbounds [256 x <2 x i32>] addrspace(3)* %38, i64 0, i64 0, i64 %102
  %104 = load i32 addrspace(3)* %103, align 4
  %105 = and i64 %100, 4294967295
  %106 = getelementptr inbounds i32 addrspace(1)* %4, i64 %105
  store i32 %104, i32 addrspace(1)* %106, align 4
  %107 = getelementptr %struct.PaddedDimId* %34, i64 %CurrWI..0.i, i32 0, i64 0
  %108 = load i64* %107, align 8
  %109 = getelementptr inbounds [256 x <2 x i32>] addrspace(3)* %40, i64 0, i64 0, i64 %108
  %110 = load i32 addrspace(3)* %109, align 4
  %111 = getelementptr inbounds i32 addrspace(1)* %7, i64 %105
  store i32 %110, i32 addrspace(1)* %111, align 4
  %112 = getelementptr %struct.PaddedDimId* %34, i64 %CurrWI..0.i, i32 0, i64 0
  %113 = load i64* %112, align 8
  %114 = add i64 %113, %83
  %115 = getelementptr inbounds [256 x <2 x i32>] addrspace(3)* %38, i64 0, i64 0, i64 %114
  %116 = load i32 addrspace(3)* %115, align 4
  %117 = lshr i32 %116, %1
  %118 = and i32 %117, 15
  %119 = zext i32 %118 to i64
  %120 = getelementptr inbounds [16 x i32] addrspace(3)* %42, i64 0, i64 %119
  %121 = load i32 addrspace(3)* %120, align 4
  %122 = zext i32 %121 to i64
  %123 = getelementptr %struct.PaddedDimId* %34, i64 %CurrWI..0.i, i32 0, i64 0
  %124 = load i64* %123, align 8
  %125 = getelementptr inbounds [16 x i32] addrspace(3)* %44, i64 0, i64 %119
  %126 = load i32 addrspace(3)* %125, align 4
  %127 = zext i32 %126 to i64
  %128 = add i64 %124, %47
  %129 = add i64 %128, %122
  %130 = sub i64 %129, %127
  %131 = getelementptr %struct.PaddedDimId* %34, i64 %CurrWI..0.i, i32 0, i64 0
  %132 = load i64* %131, align 8
  %133 = add i64 %132, %83
  %134 = getelementptr inbounds [256 x <2 x i32>] addrspace(3)* %38, i64 0, i64 0, i64 %133
  %135 = load i32 addrspace(3)* %134, align 4
  %136 = and i64 %130, 4294967295
  %137 = getelementptr inbounds i32 addrspace(1)* %4, i64 %136
  store i32 %135, i32 addrspace(1)* %137, align 4
  %138 = getelementptr %struct.PaddedDimId* %34, i64 %CurrWI..0.i, i32 0, i64 0
  %139 = load i64* %138, align 8
  %140 = add i64 %139, %83
  %141 = getelementptr inbounds [256 x <2 x i32>] addrspace(3)* %40, i64 0, i64 0, i64 %140
  %142 = load i32 addrspace(3)* %141, align 4
  %143 = getelementptr inbounds i32 addrspace(1)* %7, i64 %136
  store i32 %142, i32 addrspace(1)* %143, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %37
  br i1 %check.WI.iter.i, label %thenBB.i, label %__reorderData_separated_args.exit

thenBB.i:                                         ; preds = %SyncBB4.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB4.i

__reorderData_separated_args.exit:                ; preds = %SyncBB4.i
  ret void
}

define void @addUniform(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i32 addrspace(1)**
  %1 = load i32 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32 addrspace(1)**
  %4 = load i32 addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i8 addrspace(3)**
  %10 = load i8 addrspace(3)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to %struct.WorkDim**
  %13 = load %struct.WorkDim** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to i64**
  %16 = load i64** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 56
  %18 = bitcast i8* %17 to %struct.PaddedDimId**
  %19 = load %struct.PaddedDimId** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 72
  %21 = bitcast i8* %20 to i64*
  %22 = load i64* %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 80
  %24 = bitcast i8* %23 to i8**
  %25 = load i8** %24, align 8
  %26 = bitcast i8 addrspace(3)* %10 to i32 addrspace(3)*
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %entry ]
  %27 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..0.i, i32 0, i64 0
  %28 = load i64* %27, align 8
  %29 = icmp eq i64 %28, 0
  br i1 %29, label %30, label %"Barrier BB.i"

; <label>:30                                      ; preds = %SyncBB.i
  %31 = load i64* %16, align 8
  %32 = getelementptr inbounds i32 addrspace(1)* %4, i64 %31
  %33 = load i32 addrspace(1)* %32, align 4
  store i32 %33, i32 addrspace(3)* %26, align 4
  br label %"Barrier BB.i"

"Barrier BB.i":                                   ; preds = %30, %SyncBB.i
  %34 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..0.i, i32 0, i64 0
  %35 = load i64* %34, align 8
  %36 = load i64* %16, align 8
  %37 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %38 = load i64* %37, align 8
  %39 = shl i64 %36, 2
  %40 = mul i64 %39, %38
  %41 = add i64 %40, %35
  %"&(pSB[currWI].offset).i" = add nuw i64 %CurrSBIndex..0.i, 376
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset).i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i64*
  store i64 %41, i64* %CastToValueType.i, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %22
  br i1 %check.WI.iter.i, label %thenBB.i, label %SyncBB5.i

thenBB.i:                                         ; preds = %"Barrier BB.i"
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 560
  br label %SyncBB.i

SyncBB5.i:                                        ; preds = %thenBB8.i, %"Barrier BB.i"
  %CurrWI..1.i = phi i64 [ %"CurrWI++12.i", %thenBB8.i ], [ 0, %"Barrier BB.i" ]
  %CurrSBIndex..1.i = phi i64 [ %"loadedCurrSB+Stride14.i", %thenBB8.i ], [ 0, %"Barrier BB.i" ]
  %"&(pSB[currWI].offset)2.i" = add nuw i64 %CurrSBIndex..1.i, 376
  %"&pSB[currWI].offset3.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2.i"
  %CastToValueType4.i = bitcast i8* %"&pSB[currWI].offset3.i" to i64*
  %loadedValue.i = load i64* %CastToValueType4.i, align 8
  br label %42

; <label>:42                                      ; preds = %47, %SyncBB5.i
  %address.0.in.i = phi i64 [ %loadedValue.i, %SyncBB5.i ], [ %55, %47 ]
  %43 = phi i32 [ 0, %SyncBB5.i ], [ %56, %47 ]
  %44 = icmp slt i32 %43, 4
  br i1 %44, label %45, label %.critedge.i

; <label>:45                                      ; preds = %42
  %address.0.i = trunc i64 %address.0.in.i to i32
  %46 = icmp ult i32 %address.0.i, %7
  br i1 %46, label %47, label %.critedge.i

; <label>:47                                      ; preds = %45
  %48 = load i32 addrspace(3)* %26, align 4
  %49 = and i64 %address.0.in.i, 4294967295
  %50 = getelementptr inbounds i32 addrspace(1)* %1, i64 %49
  %51 = load i32 addrspace(1)* %50, align 4
  %52 = add i32 %51, %48
  store i32 %52, i32 addrspace(1)* %50, align 4
  %53 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %54 = load i64* %53, align 8
  %55 = add i64 %54, %49
  %56 = add nsw i32 %43, 1
  br label %42

.critedge.i:                                      ; preds = %45, %42
  %check.WI.iter11.i = icmp ult i64 %CurrWI..1.i, %22
  br i1 %check.WI.iter11.i, label %thenBB8.i, label %__addUniform_separated_args.exit

thenBB8.i:                                        ; preds = %.critedge.i
  %"CurrWI++12.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride14.i" = add nuw i64 %CurrSBIndex..1.i, 560
  br label %SyncBB5.i

__addUniform_separated_args.exit:                 ; preds = %.critedge.i
  ret void
}

define void @scan(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i32 addrspace(1)**
  %1 = load i32 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32 addrspace(1)**
  %4 = load i32 addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32 addrspace(1)**
  %7 = load i32 addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 28
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 32
  %15 = bitcast i8* %14 to i32*
  %16 = load i32* %15, align 4
  %17 = getelementptr i8* %pBuffer, i64 40
  %18 = bitcast i8* %17 to i8 addrspace(3)**
  %19 = load i8 addrspace(3)** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 48
  %21 = bitcast i8* %20 to %struct.WorkDim**
  %22 = load %struct.WorkDim** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 56
  %24 = bitcast i8* %23 to i64**
  %25 = load i64** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 64
  %27 = bitcast i8* %26 to %struct.PaddedDimId**
  %28 = load %struct.PaddedDimId** %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 72
  %30 = bitcast i8* %29 to %struct.PaddedDimId**
  %31 = load %struct.PaddedDimId** %30, align 8
  %32 = getelementptr i8* %pBuffer, i64 88
  %33 = bitcast i8* %32 to i64*
  %34 = load i64* %33, align 8
  %35 = getelementptr i8* %pBuffer, i64 96
  %36 = bitcast i8* %35 to i8**
  %37 = load i8** %36, align 8
  %38 = getelementptr i8* %pBuffer, i64 104
  %39 = bitcast i8* %38 to i64**
  %40 = load i64** %39, align 8
  store i64 0, i64* %40, align 8
  %41 = bitcast i32 addrspace(1)* %4 to <4 x i32> addrspace(1)*
  %42 = icmp eq i32 %13, 0
  br label %SyncBB141.i

SyncBB141.i:                                      ; preds = %thenBB144.i, %entry
  %CurrSBIndex..1.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride150.i", %thenBB144.i ]
  %currWI151.i = load i64* %40, align 8
  %43 = getelementptr %struct.PaddedDimId* %31, i64 %currWI151.i, i32 0, i64 0
  %44 = load i64* %43, align 8
  %45 = getelementptr %struct.PaddedDimId* %28, i64 0, i32 0, i64 0
  %46 = load i64* %45, align 8
  %47 = add i64 %44, %46
  %48 = trunc i64 %47 to i32
  %"&(pSB[currWI].offset).i" = add nuw i64 %CurrSBIndex..1.i, 384
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset).i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %48, i32* %CastToValueType.i, align 4
  %currWI.i = load i64* %40, align 8
  %49 = getelementptr %struct.PaddedDimId* %31, i64 %currWI.i, i32 0, i64 0
  %50 = load i64* %49, align 8
  %"&(pSB[currWI].offset)14.i" = add nuw i64 %CurrSBIndex..1.i, 392
  %"&pSB[currWI].offset15.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)14.i"
  %CastToValueType16.i = bitcast i8* %"&pSB[currWI].offset15.i" to i64*
  store i64 %50, i64* %CastToValueType16.i, align 8
  %51 = shl i32 %48, 2
  %"&(pSB[currWI].offset)23.i" = add nuw i64 %CurrSBIndex..1.i, 400
  %"&pSB[currWI].offset24.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)23.i"
  %CastToValueType25.i = bitcast i8* %"&pSB[currWI].offset24.i" to i32*
  store i32 %51, i32* %CastToValueType25.i, align 4
  br i1 %42, label %52, label %55

; <label>:52                                      ; preds = %SyncBB141.i
  %"&(pSB[currWI].offset)52.i" = add nuw i64 %CurrSBIndex..1.i, 400
  %"&pSB[currWI].offset53.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)52.i"
  %CastToValueType54.i = bitcast i8* %"&pSB[currWI].offset53.i" to i32*
  %loadedValue55.i = load i32* %CastToValueType54.i, align 4
  %53 = or i32 %loadedValue55.i, 3
  %54 = icmp slt i32 %53, %10
  br i1 %54, label %55, label %69

; <label>:55                                      ; preds = %52, %SyncBB141.i
  %"&(pSB[currWI].offset)5.i" = add nuw i64 %CurrSBIndex..1.i, 384
  %"&pSB[currWI].offset6.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)5.i"
  %CastToValueType7.i = bitcast i8* %"&pSB[currWI].offset6.i" to i32*
  %loadedValue.i = load i32* %CastToValueType7.i, align 4
  %56 = sext i32 %loadedValue.i to i64
  %57 = getelementptr inbounds <4 x i32> addrspace(1)* %41, i64 %56
  %58 = load <4 x i32> addrspace(1)* %57, align 16
  %59 = extractelement <4 x i32> %58, i32 1
  %60 = extractelement <4 x i32> %58, i32 0
  %61 = add i32 %59, %60
  %62 = insertelement <4 x i32> %58, i32 %61, i32 1
  %63 = extractelement <4 x i32> %58, i32 2
  %64 = add i32 %61, %63
  %65 = insertelement <4 x i32> %62, i32 %64, i32 2
  %66 = extractelement <4 x i32> %58, i32 3
  %67 = add i32 %64, %66
  %68 = insertelement <4 x i32> %65, i32 %67, i32 3
  br label %"Barrier BB.i"

; <label>:69                                      ; preds = %52
  %"&(pSB[currWI].offset)72.i" = add nuw i64 %CurrSBIndex..1.i, 400
  %"&pSB[currWI].offset73.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)72.i"
  %CastToValueType74.i = bitcast i8* %"&pSB[currWI].offset73.i" to i32*
  %loadedValue75.i = load i32* %CastToValueType74.i, align 4
  %70 = icmp slt i32 %loadedValue75.i, %10
  br i1 %70, label %71, label %76

; <label>:71                                      ; preds = %69
  %"&(pSB[currWI].offset)67.i" = add nuw i64 %CurrSBIndex..1.i, 400
  %"&pSB[currWI].offset68.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)67.i"
  %CastToValueType69.i = bitcast i8* %"&pSB[currWI].offset68.i" to i32*
  %loadedValue70.i = load i32* %CastToValueType69.i, align 4
  %72 = sext i32 %loadedValue70.i to i64
  %73 = getelementptr inbounds i32 addrspace(1)* %4, i64 %72
  %74 = load i32 addrspace(1)* %73, align 4
  %75 = uitofp i32 %74 to float
  %phitmp.i = fptoui float %75 to i32
  br label %76

; <label>:76                                      ; preds = %71, %69
  %77 = phi i32 [ %phitmp.i, %71 ], [ 0, %69 ]
  %78 = insertelement <4 x i32> undef, i32 %77, i32 0
  %"&(pSB[currWI].offset)47.i" = add nuw i64 %CurrSBIndex..1.i, 400
  %"&pSB[currWI].offset48.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)47.i"
  %CastToValueType49.i = bitcast i8* %"&pSB[currWI].offset48.i" to i32*
  %loadedValue50.i = load i32* %CastToValueType49.i, align 4
  %79 = or i32 %loadedValue50.i, 1
  %80 = icmp slt i32 %79, %10
  br i1 %80, label %81, label %86

; <label>:81                                      ; preds = %76
  %82 = sext i32 %79 to i64
  %83 = getelementptr inbounds i32 addrspace(1)* %4, i64 %82
  %84 = load i32 addrspace(1)* %83, align 4
  %85 = uitofp i32 %84 to float
  br label %86

; <label>:86                                      ; preds = %81, %76
  %87 = phi float [ %85, %81 ], [ 0.000000e+00, %76 ]
  %88 = uitofp i32 %77 to float
  %89 = fadd float %87, %88
  %90 = fptoui float %89 to i32
  %91 = insertelement <4 x i32> %78, i32 %90, i32 1
  %"&(pSB[currWI].offset)42.i" = add nuw i64 %CurrSBIndex..1.i, 400
  %"&pSB[currWI].offset43.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)42.i"
  %CastToValueType44.i = bitcast i8* %"&pSB[currWI].offset43.i" to i32*
  %loadedValue45.i = load i32* %CastToValueType44.i, align 4
  %92 = or i32 %loadedValue45.i, 2
  %93 = icmp slt i32 %92, %10
  br i1 %93, label %94, label %99

; <label>:94                                      ; preds = %86
  %95 = sext i32 %92 to i64
  %96 = getelementptr inbounds i32 addrspace(1)* %4, i64 %95
  %97 = load i32 addrspace(1)* %96, align 4
  %98 = uitofp i32 %97 to float
  br label %99

; <label>:99                                      ; preds = %94, %86
  %100 = phi float [ %98, %94 ], [ 0.000000e+00, %86 ]
  %101 = uitofp i32 %90 to float
  %102 = fadd float %100, %101
  %103 = fptoui float %102 to i32
  %104 = insertelement <4 x i32> %91, i32 %103, i32 2
  %105 = uitofp i32 %103 to float
  %106 = fptoui float %105 to i32
  %107 = insertelement <4 x i32> %104, i32 %106, i32 3
  br label %"Barrier BB.i"

"Barrier BB.i":                                   ; preds = %99, %55
  %threadScanT.0.i = phi <4 x i32> [ %68, %55 ], [ %107, %99 ]
  %res.0.i = phi i32 [ %67, %55 ], [ %106, %99 ]
  %"&(pSB[currWI].offset)101.i" = add nuw i64 %CurrSBIndex..1.i, 432
  %"&pSB[currWI].offset102.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)101.i"
  %CastToValueType103.i = bitcast i8* %"&pSB[currWI].offset102.i" to i32*
  store i32 %res.0.i, i32* %CastToValueType103.i, align 4
  %"&(pSB[currWI].offset)77.i" = add nuw i64 %CurrSBIndex..1.i, 416
  %"&pSB[currWI].offset78.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)77.i"
  %CastToValueType79.i = bitcast i8* %"&pSB[currWI].offset78.i" to <4 x i32>*
  store <4 x i32> %threadScanT.0.i, <4 x i32>* %CastToValueType79.i, align 16
  %loadedCurrWI146.i = load i64* %40, align 8
  %check.WI.iter147.i = icmp ult i64 %loadedCurrWI146.i, %34
  br i1 %check.WI.iter147.i, label %thenBB144.i, label %elseBB145.i

thenBB144.i:                                      ; preds = %"Barrier BB.i"
  %"CurrWI++148.i" = add nuw i64 %loadedCurrWI146.i, 1
  store i64 %"CurrWI++148.i", i64* %40, align 8
  %"loadedCurrSB+Stride150.i" = add nuw i64 %CurrSBIndex..1.i, 560
  br label %SyncBB141.i

elseBB145.i:                                      ; preds = %"Barrier BB.i"
  store i64 0, i64* %40, align 8
  %108 = bitcast i8 addrspace(3)* %19 to [512 x i32] addrspace(3)*
  br label %SyncBB232.i.i

SyncBB232.i.i:                                    ; preds = %thenBB252.i.i, %elseBB145.i
  %CurrWI..0.i.i = phi i64 [ %"CurrWI++256.i.i", %thenBB252.i.i ], [ 0, %elseBB145.i ]
  %CurrSBIndex..0.i.i = phi i64 [ %"loadedCurrSB+Stride258.i.i", %thenBB252.i.i ], [ 0, %elseBB145.i ]
  %109 = getelementptr %struct.PaddedDimId* %31, i64 %CurrWI..0.i.i, i32 0, i64 0
  %110 = load i64* %109, align 8
  %"&(pSB[currWI].offset).i.i" = add nuw i64 %CurrSBIndex..0.i.i, 440
  %"&pSB[currWI].offset.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset).i.i"
  %CastToValueType.i.i = bitcast i8* %"&pSB[currWI].offset.i.i" to i64*
  store i64 %110, i64* %CastToValueType.i.i, align 8
  %sext.i.i = shl i64 %110, 32
  %111 = ashr i64 %sext.i.i, 32
  %112 = getelementptr inbounds [512 x i32] addrspace(3)* %108, i64 0, i64 %111
  store i32 0, i32 addrspace(3)* %112, align 4
  %check.WI.iter255.i.i = icmp ult i64 %CurrWI..0.i.i, %34
  br i1 %check.WI.iter255.i.i, label %thenBB252.i.i, label %SyncBB233.i.i

thenBB252.i.i:                                    ; preds = %SyncBB232.i.i
  %"CurrWI++256.i.i" = add nuw i64 %CurrWI..0.i.i, 1
  %"loadedCurrSB+Stride258.i.i" = add nuw i64 %CurrSBIndex..0.i.i, 560
  br label %SyncBB232.i.i

SyncBB233.i.i:                                    ; preds = %thenBB259.i.i, %SyncBB232.i.i
  %CurrWI..1.i.i = phi i64 [ %"CurrWI++263.i.i", %thenBB259.i.i ], [ 0, %SyncBB232.i.i ]
  %CurrSBIndex..1.i.i = phi i64 [ %"loadedCurrSB+Stride265.i.i", %thenBB259.i.i ], [ 0, %SyncBB232.i.i ]
  %113 = getelementptr %struct.WorkDim* %22, i64 0, i32 3, i64 0
  %114 = load i64* %113, align 8
  %"&(pSB[currWI].offset)20.i.i" = add nuw i64 %CurrSBIndex..1.i.i, 440
  %"&pSB[currWI].offset21.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)20.i.i"
  %CastToValueType22.i.i = bitcast i8* %"&pSB[currWI].offset21.i.i" to i64*
  %loadedValue.i.i = load i64* %CastToValueType22.i.i, align 8
  %115 = add i64 %114, %loadedValue.i.i
  %116 = trunc i64 %115 to i32
  %"&(pSB[currWI].offset)24.i.i" = add nuw i64 %CurrSBIndex..1.i.i, 448
  %"&pSB[currWI].offset25.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)24.i.i"
  %CastToValueType26.i.i = bitcast i8* %"&pSB[currWI].offset25.i.i" to i32*
  store i32 %116, i32* %CastToValueType26.i.i, align 4
  %117 = sext i32 %116 to i64
  %118 = getelementptr inbounds [512 x i32] addrspace(3)* %108, i64 0, i64 %117
  %"&(pSB[currWI].offset)68.i.i" = add nuw i64 %CurrSBIndex..1.i.i, 456
  %"&pSB[currWI].offset69.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)68.i.i"
  %CastToValueType70.i.i = bitcast i8* %"&pSB[currWI].offset69.i.i" to i32 addrspace(3)**
  store i32 addrspace(3)* %118, i32 addrspace(3)** %CastToValueType70.i.i, align 8
  %"&(pSB[currWI].offset)4.i.i" = add nuw i64 %CurrSBIndex..1.i.i, 432
  %"&pSB[currWI].offset5.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)4.i.i"
  %CastToArgType.i.i = bitcast i8* %"&pSB[currWI].offset5.i.i" to i32*
  %loadedValue6.i.i = load i32* %CastToArgType.i.i, align 4
  store i32 %loadedValue6.i.i, i32 addrspace(3)* %118, align 4
  %check.WI.iter262.i.i = icmp ult i64 %CurrWI..1.i.i, %34
  br i1 %check.WI.iter262.i.i, label %thenBB259.i.i, label %SyncBB234.i.i

thenBB259.i.i:                                    ; preds = %SyncBB233.i.i
  %"CurrWI++263.i.i" = add nuw i64 %CurrWI..1.i.i, 1
  %"loadedCurrSB+Stride265.i.i" = add nuw i64 %CurrSBIndex..1.i.i, 560
  br label %SyncBB233.i.i

SyncBB234.i.i:                                    ; preds = %thenBB266.i.i, %SyncBB233.i.i
  %CurrWI..2.i.i = phi i64 [ %"CurrWI++270.i.i", %thenBB266.i.i ], [ 0, %SyncBB233.i.i ]
  %CurrSBIndex..2.i.i = phi i64 [ %"loadedCurrSB+Stride272.i.i", %thenBB266.i.i ], [ 0, %SyncBB233.i.i ]
  %"&(pSB[currWI].offset)63.i.i" = add nuw i64 %CurrSBIndex..2.i.i, 448
  %"&pSB[currWI].offset64.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)63.i.i"
  %CastToValueType65.i.i = bitcast i8* %"&pSB[currWI].offset64.i.i" to i32*
  %loadedValue66.i.i = load i32* %CastToValueType65.i.i, align 4
  %119 = add nsw i32 %loadedValue66.i.i, -1
  %120 = sext i32 %119 to i64
  %121 = getelementptr inbounds [512 x i32] addrspace(3)* %108, i64 0, i64 %120
  %"&(pSB[currWI].offset)152.i.i" = add nuw i64 %CurrSBIndex..2.i.i, 464
  %"&pSB[currWI].offset153.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)152.i.i"
  %CastToValueType154.i.i = bitcast i8* %"&pSB[currWI].offset153.i.i" to i32 addrspace(3)**
  store i32 addrspace(3)* %121, i32 addrspace(3)** %CastToValueType154.i.i, align 8
  %122 = load i32 addrspace(3)* %121, align 4
  %"&(pSB[currWI].offset)161.i.i" = add nuw i64 %CurrSBIndex..2.i.i, 472
  %"&pSB[currWI].offset162.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)161.i.i"
  %CastToValueType163.i.i = bitcast i8* %"&pSB[currWI].offset162.i.i" to i32*
  store i32 %122, i32* %CastToValueType163.i.i, align 4
  %check.WI.iter269.i.i = icmp ult i64 %CurrWI..2.i.i, %34
  br i1 %check.WI.iter269.i.i, label %thenBB266.i.i, label %SyncBB235.i.i

thenBB266.i.i:                                    ; preds = %SyncBB234.i.i
  %"CurrWI++270.i.i" = add nuw i64 %CurrWI..2.i.i, 1
  %"loadedCurrSB+Stride272.i.i" = add nuw i64 %CurrSBIndex..2.i.i, 560
  br label %SyncBB234.i.i

SyncBB235.i.i:                                    ; preds = %thenBB273.i.i, %SyncBB234.i.i
  %CurrWI..3.i.i = phi i64 [ %"CurrWI++277.i.i", %thenBB273.i.i ], [ 0, %SyncBB234.i.i ]
  %CurrSBIndex..3.i.i = phi i64 [ %"loadedCurrSB+Stride279.i.i", %thenBB273.i.i ], [ 0, %SyncBB234.i.i ]
  %"&(pSB[currWI].offset)147.i.i" = add nuw i64 %CurrSBIndex..3.i.i, 456
  %"&pSB[currWI].offset148.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)147.i.i"
  %CastToValueType149.i.i = bitcast i8* %"&pSB[currWI].offset148.i.i" to i32 addrspace(3)**
  %loadedValue150.i.i = load i32 addrspace(3)** %CastToValueType149.i.i, align 8
  %123 = load i32 addrspace(3)* %loadedValue150.i.i, align 4
  %"&(pSB[currWI].offset)165.i.i" = add nuw i64 %CurrSBIndex..3.i.i, 472
  %"&pSB[currWI].offset166.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)165.i.i"
  %CastToValueType167.i.i = bitcast i8* %"&pSB[currWI].offset166.i.i" to i32*
  %loadedValue168.i.i = load i32* %CastToValueType167.i.i, align 4
  %124 = add i32 %123, %loadedValue168.i.i
  %"&(pSB[currWI].offset)142.i.i" = add nuw i64 %CurrSBIndex..3.i.i, 456
  %"&pSB[currWI].offset143.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)142.i.i"
  %CastToValueType144.i.i = bitcast i8* %"&pSB[currWI].offset143.i.i" to i32 addrspace(3)**
  %loadedValue145.i.i = load i32 addrspace(3)** %CastToValueType144.i.i, align 8
  store i32 %124, i32 addrspace(3)* %loadedValue145.i.i, align 4
  %check.WI.iter276.i.i = icmp ult i64 %CurrWI..3.i.i, %34
  br i1 %check.WI.iter276.i.i, label %thenBB273.i.i, label %SyncBB236.i.i

thenBB273.i.i:                                    ; preds = %SyncBB235.i.i
  %"CurrWI++277.i.i" = add nuw i64 %CurrWI..3.i.i, 1
  %"loadedCurrSB+Stride279.i.i" = add nuw i64 %CurrSBIndex..3.i.i, 560
  br label %SyncBB235.i.i

SyncBB236.i.i:                                    ; preds = %thenBB280.i.i, %SyncBB235.i.i
  %CurrWI..4.i.i = phi i64 [ %"CurrWI++284.i.i", %thenBB280.i.i ], [ 0, %SyncBB235.i.i ]
  %CurrSBIndex..4.i.i = phi i64 [ %"loadedCurrSB+Stride286.i.i", %thenBB280.i.i ], [ 0, %SyncBB235.i.i ]
  %"&(pSB[currWI].offset)58.i.i" = add nuw i64 %CurrSBIndex..4.i.i, 448
  %"&pSB[currWI].offset59.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)58.i.i"
  %CastToValueType60.i.i = bitcast i8* %"&pSB[currWI].offset59.i.i" to i32*
  %loadedValue61.i.i = load i32* %CastToValueType60.i.i, align 4
  %125 = add nsw i32 %loadedValue61.i.i, -2
  %126 = sext i32 %125 to i64
  %127 = getelementptr inbounds [512 x i32] addrspace(3)* %108, i64 0, i64 %126
  %128 = load i32 addrspace(3)* %127, align 4
  %"&(pSB[currWI].offset)170.i.i" = add nuw i64 %CurrSBIndex..4.i.i, 476
  %"&pSB[currWI].offset171.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)170.i.i"
  %CastToValueType172.i.i = bitcast i8* %"&pSB[currWI].offset171.i.i" to i32*
  store i32 %128, i32* %CastToValueType172.i.i, align 4
  %check.WI.iter283.i.i = icmp ult i64 %CurrWI..4.i.i, %34
  br i1 %check.WI.iter283.i.i, label %thenBB280.i.i, label %SyncBB237.i.i

thenBB280.i.i:                                    ; preds = %SyncBB236.i.i
  %"CurrWI++284.i.i" = add nuw i64 %CurrWI..4.i.i, 1
  %"loadedCurrSB+Stride286.i.i" = add nuw i64 %CurrSBIndex..4.i.i, 560
  br label %SyncBB236.i.i

SyncBB237.i.i:                                    ; preds = %thenBB287.i.i, %SyncBB236.i.i
  %CurrWI..5.i.i = phi i64 [ %"CurrWI++291.i.i", %thenBB287.i.i ], [ 0, %SyncBB236.i.i ]
  %CurrSBIndex..5.i.i = phi i64 [ %"loadedCurrSB+Stride293.i.i", %thenBB287.i.i ], [ 0, %SyncBB236.i.i ]
  %"&(pSB[currWI].offset)137.i.i" = add nuw i64 %CurrSBIndex..5.i.i, 456
  %"&pSB[currWI].offset138.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)137.i.i"
  %CastToValueType139.i.i = bitcast i8* %"&pSB[currWI].offset138.i.i" to i32 addrspace(3)**
  %loadedValue140.i.i = load i32 addrspace(3)** %CastToValueType139.i.i, align 8
  %129 = load i32 addrspace(3)* %loadedValue140.i.i, align 4
  %"&(pSB[currWI].offset)174.i.i" = add nuw i64 %CurrSBIndex..5.i.i, 476
  %"&pSB[currWI].offset175.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)174.i.i"
  %CastToValueType176.i.i = bitcast i8* %"&pSB[currWI].offset175.i.i" to i32*
  %loadedValue177.i.i = load i32* %CastToValueType176.i.i, align 4
  %130 = add i32 %129, %loadedValue177.i.i
  %"&(pSB[currWI].offset)132.i.i" = add nuw i64 %CurrSBIndex..5.i.i, 456
  %"&pSB[currWI].offset133.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)132.i.i"
  %CastToValueType134.i.i = bitcast i8* %"&pSB[currWI].offset133.i.i" to i32 addrspace(3)**
  %loadedValue135.i.i = load i32 addrspace(3)** %CastToValueType134.i.i, align 8
  store i32 %130, i32 addrspace(3)* %loadedValue135.i.i, align 4
  %check.WI.iter290.i.i = icmp ult i64 %CurrWI..5.i.i, %34
  br i1 %check.WI.iter290.i.i, label %thenBB287.i.i, label %SyncBB238.i.i

thenBB287.i.i:                                    ; preds = %SyncBB237.i.i
  %"CurrWI++291.i.i" = add nuw i64 %CurrWI..5.i.i, 1
  %"loadedCurrSB+Stride293.i.i" = add nuw i64 %CurrSBIndex..5.i.i, 560
  br label %SyncBB237.i.i

SyncBB238.i.i:                                    ; preds = %thenBB294.i.i, %SyncBB237.i.i
  %CurrWI..6.i.i = phi i64 [ %"CurrWI++298.i.i", %thenBB294.i.i ], [ 0, %SyncBB237.i.i ]
  %CurrSBIndex..6.i.i = phi i64 [ %"loadedCurrSB+Stride300.i.i", %thenBB294.i.i ], [ 0, %SyncBB237.i.i ]
  %"&(pSB[currWI].offset)53.i.i" = add nuw i64 %CurrSBIndex..6.i.i, 448
  %"&pSB[currWI].offset54.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)53.i.i"
  %CastToValueType55.i.i = bitcast i8* %"&pSB[currWI].offset54.i.i" to i32*
  %loadedValue56.i.i = load i32* %CastToValueType55.i.i, align 4
  %131 = add nsw i32 %loadedValue56.i.i, -4
  %132 = sext i32 %131 to i64
  %133 = getelementptr inbounds [512 x i32] addrspace(3)* %108, i64 0, i64 %132
  %134 = load i32 addrspace(3)* %133, align 4
  %"&(pSB[currWI].offset)179.i.i" = add nuw i64 %CurrSBIndex..6.i.i, 480
  %"&pSB[currWI].offset180.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)179.i.i"
  %CastToValueType181.i.i = bitcast i8* %"&pSB[currWI].offset180.i.i" to i32*
  store i32 %134, i32* %CastToValueType181.i.i, align 4
  %check.WI.iter297.i.i = icmp ult i64 %CurrWI..6.i.i, %34
  br i1 %check.WI.iter297.i.i, label %thenBB294.i.i, label %SyncBB239.i.i

thenBB294.i.i:                                    ; preds = %SyncBB238.i.i
  %"CurrWI++298.i.i" = add nuw i64 %CurrWI..6.i.i, 1
  %"loadedCurrSB+Stride300.i.i" = add nuw i64 %CurrSBIndex..6.i.i, 560
  br label %SyncBB238.i.i

SyncBB239.i.i:                                    ; preds = %thenBB301.i.i, %SyncBB238.i.i
  %CurrWI..7.i.i = phi i64 [ %"CurrWI++305.i.i", %thenBB301.i.i ], [ 0, %SyncBB238.i.i ]
  %CurrSBIndex..7.i.i = phi i64 [ %"loadedCurrSB+Stride307.i.i", %thenBB301.i.i ], [ 0, %SyncBB238.i.i ]
  %"&(pSB[currWI].offset)127.i.i" = add nuw i64 %CurrSBIndex..7.i.i, 456
  %"&pSB[currWI].offset128.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)127.i.i"
  %CastToValueType129.i.i = bitcast i8* %"&pSB[currWI].offset128.i.i" to i32 addrspace(3)**
  %loadedValue130.i.i = load i32 addrspace(3)** %CastToValueType129.i.i, align 8
  %135 = load i32 addrspace(3)* %loadedValue130.i.i, align 4
  %"&(pSB[currWI].offset)183.i.i" = add nuw i64 %CurrSBIndex..7.i.i, 480
  %"&pSB[currWI].offset184.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)183.i.i"
  %CastToValueType185.i.i = bitcast i8* %"&pSB[currWI].offset184.i.i" to i32*
  %loadedValue186.i.i = load i32* %CastToValueType185.i.i, align 4
  %136 = add i32 %135, %loadedValue186.i.i
  %"&(pSB[currWI].offset)122.i.i" = add nuw i64 %CurrSBIndex..7.i.i, 456
  %"&pSB[currWI].offset123.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)122.i.i"
  %CastToValueType124.i.i = bitcast i8* %"&pSB[currWI].offset123.i.i" to i32 addrspace(3)**
  %loadedValue125.i.i = load i32 addrspace(3)** %CastToValueType124.i.i, align 8
  store i32 %136, i32 addrspace(3)* %loadedValue125.i.i, align 4
  %check.WI.iter304.i.i = icmp ult i64 %CurrWI..7.i.i, %34
  br i1 %check.WI.iter304.i.i, label %thenBB301.i.i, label %SyncBB240.i.i

thenBB301.i.i:                                    ; preds = %SyncBB239.i.i
  %"CurrWI++305.i.i" = add nuw i64 %CurrWI..7.i.i, 1
  %"loadedCurrSB+Stride307.i.i" = add nuw i64 %CurrSBIndex..7.i.i, 560
  br label %SyncBB239.i.i

SyncBB240.i.i:                                    ; preds = %thenBB308.i.i, %SyncBB239.i.i
  %CurrWI..8.i.i = phi i64 [ %"CurrWI++312.i.i", %thenBB308.i.i ], [ 0, %SyncBB239.i.i ]
  %CurrSBIndex..8.i.i = phi i64 [ %"loadedCurrSB+Stride314.i.i", %thenBB308.i.i ], [ 0, %SyncBB239.i.i ]
  %"&(pSB[currWI].offset)48.i.i" = add nuw i64 %CurrSBIndex..8.i.i, 448
  %"&pSB[currWI].offset49.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)48.i.i"
  %CastToValueType50.i.i = bitcast i8* %"&pSB[currWI].offset49.i.i" to i32*
  %loadedValue51.i.i = load i32* %CastToValueType50.i.i, align 4
  %137 = add nsw i32 %loadedValue51.i.i, -8
  %138 = sext i32 %137 to i64
  %139 = getelementptr inbounds [512 x i32] addrspace(3)* %108, i64 0, i64 %138
  %140 = load i32 addrspace(3)* %139, align 4
  %"&(pSB[currWI].offset)188.i.i" = add nuw i64 %CurrSBIndex..8.i.i, 484
  %"&pSB[currWI].offset189.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)188.i.i"
  %CastToValueType190.i.i = bitcast i8* %"&pSB[currWI].offset189.i.i" to i32*
  store i32 %140, i32* %CastToValueType190.i.i, align 4
  %check.WI.iter311.i.i = icmp ult i64 %CurrWI..8.i.i, %34
  br i1 %check.WI.iter311.i.i, label %thenBB308.i.i, label %SyncBB241.i.i

thenBB308.i.i:                                    ; preds = %SyncBB240.i.i
  %"CurrWI++312.i.i" = add nuw i64 %CurrWI..8.i.i, 1
  %"loadedCurrSB+Stride314.i.i" = add nuw i64 %CurrSBIndex..8.i.i, 560
  br label %SyncBB240.i.i

SyncBB241.i.i:                                    ; preds = %thenBB315.i.i, %SyncBB240.i.i
  %CurrWI..9.i.i = phi i64 [ %"CurrWI++319.i.i", %thenBB315.i.i ], [ 0, %SyncBB240.i.i ]
  %CurrSBIndex..9.i.i = phi i64 [ %"loadedCurrSB+Stride321.i.i", %thenBB315.i.i ], [ 0, %SyncBB240.i.i ]
  %"&(pSB[currWI].offset)117.i.i" = add nuw i64 %CurrSBIndex..9.i.i, 456
  %"&pSB[currWI].offset118.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)117.i.i"
  %CastToValueType119.i.i = bitcast i8* %"&pSB[currWI].offset118.i.i" to i32 addrspace(3)**
  %loadedValue120.i.i = load i32 addrspace(3)** %CastToValueType119.i.i, align 8
  %141 = load i32 addrspace(3)* %loadedValue120.i.i, align 4
  %"&(pSB[currWI].offset)192.i.i" = add nuw i64 %CurrSBIndex..9.i.i, 484
  %"&pSB[currWI].offset193.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)192.i.i"
  %CastToValueType194.i.i = bitcast i8* %"&pSB[currWI].offset193.i.i" to i32*
  %loadedValue195.i.i = load i32* %CastToValueType194.i.i, align 4
  %142 = add i32 %141, %loadedValue195.i.i
  %"&(pSB[currWI].offset)112.i.i" = add nuw i64 %CurrSBIndex..9.i.i, 456
  %"&pSB[currWI].offset113.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)112.i.i"
  %CastToValueType114.i.i = bitcast i8* %"&pSB[currWI].offset113.i.i" to i32 addrspace(3)**
  %loadedValue115.i.i = load i32 addrspace(3)** %CastToValueType114.i.i, align 8
  store i32 %142, i32 addrspace(3)* %loadedValue115.i.i, align 4
  %check.WI.iter318.i.i = icmp ult i64 %CurrWI..9.i.i, %34
  br i1 %check.WI.iter318.i.i, label %thenBB315.i.i, label %SyncBB242.i.i

thenBB315.i.i:                                    ; preds = %SyncBB241.i.i
  %"CurrWI++319.i.i" = add nuw i64 %CurrWI..9.i.i, 1
  %"loadedCurrSB+Stride321.i.i" = add nuw i64 %CurrSBIndex..9.i.i, 560
  br label %SyncBB241.i.i

SyncBB242.i.i:                                    ; preds = %thenBB322.i.i, %SyncBB241.i.i
  %CurrWI..10.i.i = phi i64 [ %"CurrWI++326.i.i", %thenBB322.i.i ], [ 0, %SyncBB241.i.i ]
  %CurrSBIndex..10.i.i = phi i64 [ %"loadedCurrSB+Stride328.i.i", %thenBB322.i.i ], [ 0, %SyncBB241.i.i ]
  %"&(pSB[currWI].offset)43.i.i" = add nuw i64 %CurrSBIndex..10.i.i, 448
  %"&pSB[currWI].offset44.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)43.i.i"
  %CastToValueType45.i.i = bitcast i8* %"&pSB[currWI].offset44.i.i" to i32*
  %loadedValue46.i.i = load i32* %CastToValueType45.i.i, align 4
  %143 = add nsw i32 %loadedValue46.i.i, -16
  %144 = sext i32 %143 to i64
  %145 = getelementptr inbounds [512 x i32] addrspace(3)* %108, i64 0, i64 %144
  %146 = load i32 addrspace(3)* %145, align 4
  %"&(pSB[currWI].offset)197.i.i" = add nuw i64 %CurrSBIndex..10.i.i, 488
  %"&pSB[currWI].offset198.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)197.i.i"
  %CastToValueType199.i.i = bitcast i8* %"&pSB[currWI].offset198.i.i" to i32*
  store i32 %146, i32* %CastToValueType199.i.i, align 4
  %check.WI.iter325.i.i = icmp ult i64 %CurrWI..10.i.i, %34
  br i1 %check.WI.iter325.i.i, label %thenBB322.i.i, label %SyncBB243.i.i

thenBB322.i.i:                                    ; preds = %SyncBB242.i.i
  %"CurrWI++326.i.i" = add nuw i64 %CurrWI..10.i.i, 1
  %"loadedCurrSB+Stride328.i.i" = add nuw i64 %CurrSBIndex..10.i.i, 560
  br label %SyncBB242.i.i

SyncBB243.i.i:                                    ; preds = %thenBB329.i.i, %SyncBB242.i.i
  %CurrWI..11.i.i = phi i64 [ %"CurrWI++333.i.i", %thenBB329.i.i ], [ 0, %SyncBB242.i.i ]
  %CurrSBIndex..11.i.i = phi i64 [ %"loadedCurrSB+Stride335.i.i", %thenBB329.i.i ], [ 0, %SyncBB242.i.i ]
  %"&(pSB[currWI].offset)107.i.i" = add nuw i64 %CurrSBIndex..11.i.i, 456
  %"&pSB[currWI].offset108.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)107.i.i"
  %CastToValueType109.i.i = bitcast i8* %"&pSB[currWI].offset108.i.i" to i32 addrspace(3)**
  %loadedValue110.i.i = load i32 addrspace(3)** %CastToValueType109.i.i, align 8
  %147 = load i32 addrspace(3)* %loadedValue110.i.i, align 4
  %"&(pSB[currWI].offset)201.i.i" = add nuw i64 %CurrSBIndex..11.i.i, 488
  %"&pSB[currWI].offset202.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)201.i.i"
  %CastToValueType203.i.i = bitcast i8* %"&pSB[currWI].offset202.i.i" to i32*
  %loadedValue204.i.i = load i32* %CastToValueType203.i.i, align 4
  %148 = add i32 %147, %loadedValue204.i.i
  %"&(pSB[currWI].offset)102.i.i" = add nuw i64 %CurrSBIndex..11.i.i, 456
  %"&pSB[currWI].offset103.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)102.i.i"
  %CastToValueType104.i.i = bitcast i8* %"&pSB[currWI].offset103.i.i" to i32 addrspace(3)**
  %loadedValue105.i.i = load i32 addrspace(3)** %CastToValueType104.i.i, align 8
  store i32 %148, i32 addrspace(3)* %loadedValue105.i.i, align 4
  %check.WI.iter332.i.i = icmp ult i64 %CurrWI..11.i.i, %34
  br i1 %check.WI.iter332.i.i, label %thenBB329.i.i, label %SyncBB244.i.i

thenBB329.i.i:                                    ; preds = %SyncBB243.i.i
  %"CurrWI++333.i.i" = add nuw i64 %CurrWI..11.i.i, 1
  %"loadedCurrSB+Stride335.i.i" = add nuw i64 %CurrSBIndex..11.i.i, 560
  br label %SyncBB243.i.i

SyncBB244.i.i:                                    ; preds = %thenBB336.i.i, %SyncBB243.i.i
  %CurrWI..12.i.i = phi i64 [ %"CurrWI++340.i.i", %thenBB336.i.i ], [ 0, %SyncBB243.i.i ]
  %CurrSBIndex..12.i.i = phi i64 [ %"loadedCurrSB+Stride342.i.i", %thenBB336.i.i ], [ 0, %SyncBB243.i.i ]
  %"&(pSB[currWI].offset)38.i.i" = add nuw i64 %CurrSBIndex..12.i.i, 448
  %"&pSB[currWI].offset39.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)38.i.i"
  %CastToValueType40.i.i = bitcast i8* %"&pSB[currWI].offset39.i.i" to i32*
  %loadedValue41.i.i = load i32* %CastToValueType40.i.i, align 4
  %149 = add nsw i32 %loadedValue41.i.i, -32
  %150 = sext i32 %149 to i64
  %151 = getelementptr inbounds [512 x i32] addrspace(3)* %108, i64 0, i64 %150
  %152 = load i32 addrspace(3)* %151, align 4
  %"&(pSB[currWI].offset)206.i.i" = add nuw i64 %CurrSBIndex..12.i.i, 492
  %"&pSB[currWI].offset207.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)206.i.i"
  %CastToValueType208.i.i = bitcast i8* %"&pSB[currWI].offset207.i.i" to i32*
  store i32 %152, i32* %CastToValueType208.i.i, align 4
  %check.WI.iter339.i.i = icmp ult i64 %CurrWI..12.i.i, %34
  br i1 %check.WI.iter339.i.i, label %thenBB336.i.i, label %SyncBB245.i.i

thenBB336.i.i:                                    ; preds = %SyncBB244.i.i
  %"CurrWI++340.i.i" = add nuw i64 %CurrWI..12.i.i, 1
  %"loadedCurrSB+Stride342.i.i" = add nuw i64 %CurrSBIndex..12.i.i, 560
  br label %SyncBB244.i.i

SyncBB245.i.i:                                    ; preds = %thenBB343.i.i, %SyncBB244.i.i
  %CurrWI..13.i.i = phi i64 [ %"CurrWI++347.i.i", %thenBB343.i.i ], [ 0, %SyncBB244.i.i ]
  %CurrSBIndex..13.i.i = phi i64 [ %"loadedCurrSB+Stride349.i.i", %thenBB343.i.i ], [ 0, %SyncBB244.i.i ]
  %"&(pSB[currWI].offset)97.i.i" = add nuw i64 %CurrSBIndex..13.i.i, 456
  %"&pSB[currWI].offset98.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)97.i.i"
  %CastToValueType99.i.i = bitcast i8* %"&pSB[currWI].offset98.i.i" to i32 addrspace(3)**
  %loadedValue100.i.i = load i32 addrspace(3)** %CastToValueType99.i.i, align 8
  %153 = load i32 addrspace(3)* %loadedValue100.i.i, align 4
  %"&(pSB[currWI].offset)210.i.i" = add nuw i64 %CurrSBIndex..13.i.i, 492
  %"&pSB[currWI].offset211.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)210.i.i"
  %CastToValueType212.i.i = bitcast i8* %"&pSB[currWI].offset211.i.i" to i32*
  %loadedValue213.i.i = load i32* %CastToValueType212.i.i, align 4
  %154 = add i32 %153, %loadedValue213.i.i
  %"&(pSB[currWI].offset)92.i.i" = add nuw i64 %CurrSBIndex..13.i.i, 456
  %"&pSB[currWI].offset93.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)92.i.i"
  %CastToValueType94.i.i = bitcast i8* %"&pSB[currWI].offset93.i.i" to i32 addrspace(3)**
  %loadedValue95.i.i = load i32 addrspace(3)** %CastToValueType94.i.i, align 8
  store i32 %154, i32 addrspace(3)* %loadedValue95.i.i, align 4
  %check.WI.iter346.i.i = icmp ult i64 %CurrWI..13.i.i, %34
  br i1 %check.WI.iter346.i.i, label %thenBB343.i.i, label %SyncBB246.i.i

thenBB343.i.i:                                    ; preds = %SyncBB245.i.i
  %"CurrWI++347.i.i" = add nuw i64 %CurrWI..13.i.i, 1
  %"loadedCurrSB+Stride349.i.i" = add nuw i64 %CurrSBIndex..13.i.i, 560
  br label %SyncBB245.i.i

SyncBB246.i.i:                                    ; preds = %thenBB350.i.i, %SyncBB245.i.i
  %CurrWI..14.i.i = phi i64 [ %"CurrWI++354.i.i", %thenBB350.i.i ], [ 0, %SyncBB245.i.i ]
  %CurrSBIndex..14.i.i = phi i64 [ %"loadedCurrSB+Stride356.i.i", %thenBB350.i.i ], [ 0, %SyncBB245.i.i ]
  %"&(pSB[currWI].offset)33.i.i" = add nuw i64 %CurrSBIndex..14.i.i, 448
  %"&pSB[currWI].offset34.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)33.i.i"
  %CastToValueType35.i.i = bitcast i8* %"&pSB[currWI].offset34.i.i" to i32*
  %loadedValue36.i.i = load i32* %CastToValueType35.i.i, align 4
  %155 = add nsw i32 %loadedValue36.i.i, -64
  %156 = sext i32 %155 to i64
  %157 = getelementptr inbounds [512 x i32] addrspace(3)* %108, i64 0, i64 %156
  %158 = load i32 addrspace(3)* %157, align 4
  %"&(pSB[currWI].offset)215.i.i" = add nuw i64 %CurrSBIndex..14.i.i, 496
  %"&pSB[currWI].offset216.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)215.i.i"
  %CastToValueType217.i.i = bitcast i8* %"&pSB[currWI].offset216.i.i" to i32*
  store i32 %158, i32* %CastToValueType217.i.i, align 4
  %check.WI.iter353.i.i = icmp ult i64 %CurrWI..14.i.i, %34
  br i1 %check.WI.iter353.i.i, label %thenBB350.i.i, label %SyncBB247.i.i

thenBB350.i.i:                                    ; preds = %SyncBB246.i.i
  %"CurrWI++354.i.i" = add nuw i64 %CurrWI..14.i.i, 1
  %"loadedCurrSB+Stride356.i.i" = add nuw i64 %CurrSBIndex..14.i.i, 560
  br label %SyncBB246.i.i

SyncBB247.i.i:                                    ; preds = %thenBB357.i.i, %SyncBB246.i.i
  %CurrWI..15.i.i = phi i64 [ %"CurrWI++361.i.i", %thenBB357.i.i ], [ 0, %SyncBB246.i.i ]
  %CurrSBIndex..15.i.i = phi i64 [ %"loadedCurrSB+Stride363.i.i", %thenBB357.i.i ], [ 0, %SyncBB246.i.i ]
  %"&(pSB[currWI].offset)87.i.i" = add nuw i64 %CurrSBIndex..15.i.i, 456
  %"&pSB[currWI].offset88.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)87.i.i"
  %CastToValueType89.i.i = bitcast i8* %"&pSB[currWI].offset88.i.i" to i32 addrspace(3)**
  %loadedValue90.i.i = load i32 addrspace(3)** %CastToValueType89.i.i, align 8
  %159 = load i32 addrspace(3)* %loadedValue90.i.i, align 4
  %"&(pSB[currWI].offset)219.i.i" = add nuw i64 %CurrSBIndex..15.i.i, 496
  %"&pSB[currWI].offset220.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)219.i.i"
  %CastToValueType221.i.i = bitcast i8* %"&pSB[currWI].offset220.i.i" to i32*
  %loadedValue222.i.i = load i32* %CastToValueType221.i.i, align 4
  %160 = add i32 %159, %loadedValue222.i.i
  %"&(pSB[currWI].offset)82.i.i" = add nuw i64 %CurrSBIndex..15.i.i, 456
  %"&pSB[currWI].offset83.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)82.i.i"
  %CastToValueType84.i.i = bitcast i8* %"&pSB[currWI].offset83.i.i" to i32 addrspace(3)**
  %loadedValue85.i.i = load i32 addrspace(3)** %CastToValueType84.i.i, align 8
  store i32 %160, i32 addrspace(3)* %loadedValue85.i.i, align 4
  %check.WI.iter360.i.i = icmp ult i64 %CurrWI..15.i.i, %34
  br i1 %check.WI.iter360.i.i, label %thenBB357.i.i, label %SyncBB248.i.i

thenBB357.i.i:                                    ; preds = %SyncBB247.i.i
  %"CurrWI++361.i.i" = add nuw i64 %CurrWI..15.i.i, 1
  %"loadedCurrSB+Stride363.i.i" = add nuw i64 %CurrSBIndex..15.i.i, 560
  br label %SyncBB247.i.i

SyncBB248.i.i:                                    ; preds = %thenBB364.i.i, %SyncBB247.i.i
  %CurrWI..16.i.i = phi i64 [ %"CurrWI++368.i.i", %thenBB364.i.i ], [ 0, %SyncBB247.i.i ]
  %CurrSBIndex..16.i.i = phi i64 [ %"loadedCurrSB+Stride370.i.i", %thenBB364.i.i ], [ 0, %SyncBB247.i.i ]
  %"&(pSB[currWI].offset)28.i.i" = add nuw i64 %CurrSBIndex..16.i.i, 448
  %"&pSB[currWI].offset29.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)28.i.i"
  %CastToValueType30.i.i = bitcast i8* %"&pSB[currWI].offset29.i.i" to i32*
  %loadedValue31.i.i = load i32* %CastToValueType30.i.i, align 4
  %161 = add nsw i32 %loadedValue31.i.i, -128
  %162 = sext i32 %161 to i64
  %163 = getelementptr inbounds [512 x i32] addrspace(3)* %108, i64 0, i64 %162
  %164 = load i32 addrspace(3)* %163, align 4
  %"&(pSB[currWI].offset)224.i.i" = add nuw i64 %CurrSBIndex..16.i.i, 500
  %"&pSB[currWI].offset225.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)224.i.i"
  %CastToValueType226.i.i = bitcast i8* %"&pSB[currWI].offset225.i.i" to i32*
  store i32 %164, i32* %CastToValueType226.i.i, align 4
  %check.WI.iter367.i.i = icmp ult i64 %CurrWI..16.i.i, %34
  br i1 %check.WI.iter367.i.i, label %thenBB364.i.i, label %SyncBB249.i.i

thenBB364.i.i:                                    ; preds = %SyncBB248.i.i
  %"CurrWI++368.i.i" = add nuw i64 %CurrWI..16.i.i, 1
  %"loadedCurrSB+Stride370.i.i" = add nuw i64 %CurrSBIndex..16.i.i, 560
  br label %SyncBB248.i.i

SyncBB249.i.i:                                    ; preds = %thenBB371.i.i, %SyncBB248.i.i
  %CurrWI..17.i.i = phi i64 [ %"CurrWI++375.i.i", %thenBB371.i.i ], [ 0, %SyncBB248.i.i ]
  %CurrSBIndex..17.i.i = phi i64 [ %"loadedCurrSB+Stride377.i.i", %thenBB371.i.i ], [ 0, %SyncBB248.i.i ]
  %"&(pSB[currWI].offset)77.i.i" = add nuw i64 %CurrSBIndex..17.i.i, 456
  %"&pSB[currWI].offset78.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)77.i.i"
  %CastToValueType79.i.i = bitcast i8* %"&pSB[currWI].offset78.i.i" to i32 addrspace(3)**
  %loadedValue80.i.i = load i32 addrspace(3)** %CastToValueType79.i.i, align 8
  %165 = load i32 addrspace(3)* %loadedValue80.i.i, align 4
  %"&(pSB[currWI].offset)228.i.i" = add nuw i64 %CurrSBIndex..17.i.i, 500
  %"&pSB[currWI].offset229.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)228.i.i"
  %CastToValueType230.i.i = bitcast i8* %"&pSB[currWI].offset229.i.i" to i32*
  %loadedValue231.i.i = load i32* %CastToValueType230.i.i, align 4
  %166 = add i32 %165, %loadedValue231.i.i
  %"&(pSB[currWI].offset)72.i.i" = add nuw i64 %CurrSBIndex..17.i.i, 456
  %"&pSB[currWI].offset73.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)72.i.i"
  %CastToValueType74.i.i = bitcast i8* %"&pSB[currWI].offset73.i.i" to i32 addrspace(3)**
  %loadedValue75.i.i = load i32 addrspace(3)** %CastToValueType74.i.i, align 8
  store i32 %166, i32 addrspace(3)* %loadedValue75.i.i, align 4
  %check.WI.iter374.i.i = icmp ult i64 %CurrWI..17.i.i, %34
  br i1 %check.WI.iter374.i.i, label %thenBB371.i.i, label %SyncBB250.i.i

thenBB371.i.i:                                    ; preds = %SyncBB249.i.i
  %"CurrWI++375.i.i" = add nuw i64 %CurrWI..17.i.i, 1
  %"loadedCurrSB+Stride377.i.i" = add nuw i64 %CurrSBIndex..17.i.i, 560
  br label %SyncBB249.i.i

SyncBB250.i.i:                                    ; preds = %thenBB.i.i, %SyncBB249.i.i
  %CurrWI..18.i.i = phi i64 [ %"CurrWI++.i.i", %thenBB.i.i ], [ 0, %SyncBB249.i.i ]
  %CurrSBIndex..18.i.i = phi i64 [ %"loadedCurrSB+Stride.i.i", %thenBB.i.i ], [ 0, %SyncBB249.i.i ]
  %"&(pSB[currWI].offset)156.i.i" = add nuw i64 %CurrSBIndex..18.i.i, 464
  %"&pSB[currWI].offset157.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)156.i.i"
  %CastToValueType158.i.i = bitcast i8* %"&pSB[currWI].offset157.i.i" to i32 addrspace(3)**
  %loadedValue159.i.i = load i32 addrspace(3)** %CastToValueType158.i.i, align 8
  %167 = load i32 addrspace(3)* %loadedValue159.i.i, align 4
  %"&(pSB[currWI].offset)8.i.i" = add nuw i64 %CurrSBIndex..18.i.i, 436
  %"&pSB[currWI].offset9.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)8.i.i"
  %CastToArgType10.i.i = bitcast i8* %"&pSB[currWI].offset9.i.i" to i32*
  store i32 %167, i32* %CastToArgType10.i.i, align 4
  %check.WI.iter.i.i = icmp ult i64 %CurrWI..18.i.i, %34
  br i1 %check.WI.iter.i.i, label %thenBB.i.i, label %"Barrier BB3.i"

thenBB.i.i:                                       ; preds = %SyncBB250.i.i
  %"CurrWI++.i.i" = add nuw i64 %CurrWI..18.i.i, 1
  %"loadedCurrSB+Stride.i.i" = add nuw i64 %CurrSBIndex..18.i.i, 560
  br label %SyncBB250.i.i

"Barrier BB3.i":                                  ; preds = %SyncBB250.i.i
  store i64 0, i64* %40, align 8
  %168 = icmp eq i32 %16, 0
  %169 = bitcast i32 addrspace(1)* %1 to <4 x i32> addrspace(1)*
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %"Barrier BB3.i"
  %CurrSBIndex..0.i = phi i64 [ 0, %"Barrier BB3.i" ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  br i1 %168, label %phi-split-bb.i, label %170

; <label>:170                                     ; preds = %SyncBB.i
  %"&(pSB[currWI].offset)18.i" = add nuw i64 %CurrSBIndex..0.i, 392
  %"&pSB[currWI].offset19.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)18.i"
  %CastToValueType20.i = bitcast i8* %"&pSB[currWI].offset19.i" to i64*
  %loadedValue21.i = load i64* %CastToValueType20.i, align 8
  %sext.i = shl i64 %loadedValue21.i, 32
  %171 = ashr i64 %sext.i, 32
  %172 = getelementptr %struct.WorkDim* %22, i64 0, i32 3, i64 0
  %173 = load i64* %172, align 8
  %174 = add i64 %173, -1
  %175 = icmp eq i64 %171, %174
  br i1 %175, label %176, label %phi-split-bb.i

; <label>:176                                     ; preds = %170
  %"&(pSB[currWI].offset)96.i" = add nuw i64 %CurrSBIndex..0.i, 416
  %"&pSB[currWI].offset97.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)96.i"
  %CastToValueType98.i = bitcast i8* %"&pSB[currWI].offset97.i" to <4 x i32>*
  %loadedValue99.i = load <4 x i32>* %CastToValueType98.i, align 16
  %177 = extractelement <4 x i32> %loadedValue99.i, i32 3
  %"&(pSB[currWI].offset)125.i" = add nuw i64 %CurrSBIndex..0.i, 436
  %"&pSB[currWI].offset126.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)125.i"
  %CastToValueType127.i = bitcast i8* %"&pSB[currWI].offset126.i" to i32*
  %loadedValue128.i = load i32* %CastToValueType127.i, align 4
  %178 = add i32 %loadedValue128.i, %177
  %179 = load i64* %25, align 8
  %180 = getelementptr inbounds i32 addrspace(1)* %7, i64 %179
  store i32 %178, i32 addrspace(1)* %180, align 4
  br label %phi-split-bb.i

phi-split-bb.i:                                   ; preds = %176, %170, %SyncBB.i
  %"&(pSB[currWI].offset)130.i" = add nuw i64 %CurrSBIndex..0.i, 436
  %"&pSB[currWI].offset131.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)130.i"
  %CastToValueType132.i = bitcast i8* %"&pSB[currWI].offset131.i" to i32*
  %loadedValue133.i = load i32* %CastToValueType132.i, align 4
  %181 = insertelement <4 x i32> undef, i32 %loadedValue133.i, i32 0
  %"&(pSB[currWI].offset)91.i" = add nuw i64 %CurrSBIndex..0.i, 416
  %"&pSB[currWI].offset92.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)91.i"
  %CastToValueType93.i = bitcast i8* %"&pSB[currWI].offset92.i" to <4 x i32>*
  %loadedValue94.i = load <4 x i32>* %CastToValueType93.i, align 16
  %182 = extractelement <4 x i32> %loadedValue94.i, i32 0
  %"&(pSB[currWI].offset)120.i" = add nuw i64 %CurrSBIndex..0.i, 436
  %"&pSB[currWI].offset121.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)120.i"
  %CastToValueType122.i = bitcast i8* %"&pSB[currWI].offset121.i" to i32*
  %loadedValue123.i = load i32* %CastToValueType122.i, align 4
  %183 = add i32 %loadedValue123.i, %182
  %184 = insertelement <4 x i32> %181, i32 %183, i32 1
  %"&(pSB[currWI].offset)86.i" = add nuw i64 %CurrSBIndex..0.i, 416
  %"&pSB[currWI].offset87.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)86.i"
  %CastToValueType88.i = bitcast i8* %"&pSB[currWI].offset87.i" to <4 x i32>*
  %loadedValue89.i = load <4 x i32>* %CastToValueType88.i, align 16
  %185 = extractelement <4 x i32> %loadedValue89.i, i32 1
  %"&(pSB[currWI].offset)115.i" = add nuw i64 %CurrSBIndex..0.i, 436
  %"&pSB[currWI].offset116.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)115.i"
  %CastToValueType117.i = bitcast i8* %"&pSB[currWI].offset116.i" to i32*
  %loadedValue118.i = load i32* %CastToValueType117.i, align 4
  %186 = add i32 %loadedValue118.i, %185
  %187 = insertelement <4 x i32> %184, i32 %186, i32 2
  %"&(pSB[currWI].offset)81.i" = add nuw i64 %CurrSBIndex..0.i, 416
  %"&pSB[currWI].offset82.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)81.i"
  %CastToValueType83.i = bitcast i8* %"&pSB[currWI].offset82.i" to <4 x i32>*
  %loadedValue84.i = load <4 x i32>* %CastToValueType83.i, align 16
  %188 = extractelement <4 x i32> %loadedValue84.i, i32 2
  %"&(pSB[currWI].offset)110.i" = add nuw i64 %CurrSBIndex..0.i, 436
  %"&pSB[currWI].offset111.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)110.i"
  %CastToValueType112.i = bitcast i8* %"&pSB[currWI].offset111.i" to i32*
  %loadedValue113.i = load i32* %CastToValueType112.i, align 4
  %189 = add i32 %loadedValue113.i, %188
  %190 = insertelement <4 x i32> %187, i32 %189, i32 3
  br i1 %42, label %191, label %194

; <label>:191                                     ; preds = %phi-split-bb.i
  %"&(pSB[currWI].offset)37.i" = add nuw i64 %CurrSBIndex..0.i, 400
  %"&pSB[currWI].offset38.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)37.i"
  %CastToValueType39.i = bitcast i8* %"&pSB[currWI].offset38.i" to i32*
  %loadedValue40.i = load i32* %CastToValueType39.i, align 4
  %192 = or i32 %loadedValue40.i, 3
  %193 = icmp slt i32 %192, %10
  br i1 %193, label %194, label %197

; <label>:194                                     ; preds = %191, %phi-split-bb.i
  %"&(pSB[currWI].offset)9.i" = add nuw i64 %CurrSBIndex..0.i, 384
  %"&pSB[currWI].offset10.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)9.i"
  %CastToValueType11.i = bitcast i8* %"&pSB[currWI].offset10.i" to i32*
  %loadedValue12.i = load i32* %CastToValueType11.i, align 4
  %195 = sext i32 %loadedValue12.i to i64
  %196 = getelementptr inbounds <4 x i32> addrspace(1)* %169, i64 %195
  store <4 x i32> %190, <4 x i32> addrspace(1)* %196, align 16
  br label %UnifiedReturnBlock.i

; <label>:197                                     ; preds = %191
  %"&(pSB[currWI].offset)62.i" = add nuw i64 %CurrSBIndex..0.i, 400
  %"&pSB[currWI].offset63.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)62.i"
  %CastToValueType64.i = bitcast i8* %"&pSB[currWI].offset63.i" to i32*
  %loadedValue65.i = load i32* %CastToValueType64.i, align 4
  %198 = icmp slt i32 %loadedValue65.i, %10
  br i1 %198, label %199, label %UnifiedReturnBlock.i

; <label>:199                                     ; preds = %197
  %"&(pSB[currWI].offset)57.i" = add nuw i64 %CurrSBIndex..0.i, 400
  %"&pSB[currWI].offset58.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)57.i"
  %CastToValueType59.i = bitcast i8* %"&pSB[currWI].offset58.i" to i32*
  %loadedValue60.i = load i32* %CastToValueType59.i, align 4
  %200 = sext i32 %loadedValue60.i to i64
  %201 = getelementptr inbounds i32 addrspace(1)* %1, i64 %200
  %"&(pSB[currWI].offset)135.i" = add nuw i64 %CurrSBIndex..0.i, 436
  %"&pSB[currWI].offset136.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)135.i"
  %CastToValueType137.i = bitcast i8* %"&pSB[currWI].offset136.i" to i32*
  %loadedValue138.i = load i32* %CastToValueType137.i, align 4
  store i32 %loadedValue138.i, i32 addrspace(1)* %201, align 4
  %"&(pSB[currWI].offset)32.i" = add nuw i64 %CurrSBIndex..0.i, 400
  %"&pSB[currWI].offset33.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)32.i"
  %CastToValueType34.i = bitcast i8* %"&pSB[currWI].offset33.i" to i32*
  %loadedValue35.i = load i32* %CastToValueType34.i, align 4
  %202 = or i32 %loadedValue35.i, 1
  %203 = icmp slt i32 %202, %10
  br i1 %203, label %204, label %UnifiedReturnBlock.i

; <label>:204                                     ; preds = %199
  %205 = sext i32 %202 to i64
  %206 = getelementptr inbounds i32 addrspace(1)* %1, i64 %205
  store i32 %183, i32 addrspace(1)* %206, align 4
  %"&(pSB[currWI].offset)27.i" = add nuw i64 %CurrSBIndex..0.i, 400
  %"&pSB[currWI].offset28.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)27.i"
  %CastToValueType29.i = bitcast i8* %"&pSB[currWI].offset28.i" to i32*
  %loadedValue30.i = load i32* %CastToValueType29.i, align 4
  %207 = or i32 %loadedValue30.i, 2
  %208 = icmp slt i32 %207, %10
  br i1 %208, label %209, label %UnifiedReturnBlock.i

; <label>:209                                     ; preds = %204
  %210 = sext i32 %207 to i64
  %211 = getelementptr inbounds i32 addrspace(1)* %1, i64 %210
  store i32 %186, i32 addrspace(1)* %211, align 4
  br label %UnifiedReturnBlock.i

UnifiedReturnBlock.i:                             ; preds = %209, %204, %199, %197, %194
  %loadedCurrWI.i = load i64* %40, align 8
  %check.WI.iter.i = icmp ult i64 %loadedCurrWI.i, %34
  br i1 %check.WI.iter.i, label %thenBB.i, label %__scan_separated_args.exit

thenBB.i:                                         ; preds = %UnifiedReturnBlock.i
  %"CurrWI++.i" = add nuw i64 %loadedCurrWI.i, 1
  store i64 %"CurrWI++.i", i64* %40, align 8
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 560
  br label %SyncBB.i

__scan_separated_args.exit:                       ; preds = %UnifiedReturnBlock.i
  store i64 0, i64* %40, align 8
  ret void
}

define void @findRadixOffsets(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <2 x i32> addrspace(1)**
  %1 = load <2 x i32> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32 addrspace(1)**
  %4 = load i32 addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32 addrspace(1)**
  %7 = load i32 addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to i32 addrspace(3)**
  %16 = load i32 addrspace(3)** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 48
  %18 = bitcast i8* %17 to i8 addrspace(3)**
  %19 = load i8 addrspace(3)** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 56
  %21 = bitcast i8* %20 to %struct.WorkDim**
  %22 = load %struct.WorkDim** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 64
  %24 = bitcast i8* %23 to i64**
  %25 = load i64** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 72
  %27 = bitcast i8* %26 to %struct.PaddedDimId**
  %28 = load %struct.PaddedDimId** %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 80
  %30 = bitcast i8* %29 to %struct.PaddedDimId**
  %31 = load %struct.PaddedDimId** %30, align 8
  %32 = getelementptr i8* %pBuffer, i64 96
  %33 = bitcast i8* %32 to i64*
  %34 = load i64* %33, align 8
  %35 = getelementptr i8* %pBuffer, i64 104
  %36 = bitcast i8* %35 to i8**
  %37 = load i8** %36, align 8
  %38 = bitcast i8 addrspace(3)* %19 to [16 x i32] addrspace(3)*
  br label %SyncBB155.i

SyncBB155.i:                                      ; preds = %thenBB.i, %entry
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %entry ]
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %39 = load i64* %25, align 8
  %40 = getelementptr %struct.PaddedDimId* %31, i64 %CurrWI..0.i, i32 0, i64 0
  %41 = load i64* %40, align 8
  %"&(pSB[currWI].offset).i" = add nuw i64 %CurrSBIndex..0.i, 328
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset).i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i64*
  store i64 %41, i64* %CastToValueType.i, align 8
  %42 = trunc i64 %41 to i32
  %"&(pSB[currWI].offset)27.i" = add nuw i64 %CurrSBIndex..0.i, 336
  %"&pSB[currWI].offset28.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)27.i"
  %CastToValueType29.i = bitcast i8* %"&pSB[currWI].offset28.i" to i32*
  store i32 %42, i32* %CastToValueType29.i, align 4
  %43 = getelementptr %struct.WorkDim* %22, i64 0, i32 3, i64 0
  %44 = load i64* %43, align 8
  %45 = getelementptr %struct.PaddedDimId* %31, i64 %CurrWI..0.i, i32 0, i64 0
  %46 = load i64* %45, align 8
  %47 = getelementptr %struct.PaddedDimId* %28, i64 0, i32 0, i64 0
  %48 = load i64* %47, align 8
  %49 = add i64 %46, %48
  %50 = getelementptr inbounds <2 x i32> addrspace(1)* %1, i64 %49
  %51 = load <2 x i32> addrspace(1)* %50, align 8
  %52 = extractelement <2 x i32> %51, i32 0
  %53 = lshr i32 %52, %10
  %54 = and i32 %53, 15
  %55 = shl i32 %42, 1
  %56 = zext i32 %55 to i64
  %57 = getelementptr inbounds i32 addrspace(3)* %16, i64 %56
  store i32 %54, i32 addrspace(3)* %57, align 4
  %58 = extractelement <2 x i32> %51, i32 1
  %59 = lshr i32 %58, %10
  %60 = and i32 %59, 15
  %61 = or i32 %55, 1
  %62 = zext i32 %61 to i64
  %63 = getelementptr inbounds i32 addrspace(3)* %16, i64 %62
  store i32 %60, i32 addrspace(3)* %63, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %34
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %SyncBB155.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 560
  br label %SyncBB155.i

elseBB.i:                                         ; preds = %SyncBB155.i
  %64 = trunc i64 %39 to i32
  %65 = trunc i64 %44 to i32
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB157.i, %elseBB.i
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride163.i", %thenBB157.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++161.i", %thenBB157.i ]
  %"&(pSB[currWI].offset)61.i" = add nuw i64 %CurrSBIndex..1.i, 336
  %"&pSB[currWI].offset62.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)61.i"
  %CastToValueType63.i = bitcast i8* %"&pSB[currWI].offset62.i" to i32*
  %loadedValue64.i = load i32* %CastToValueType63.i, align 4
  %66 = icmp ult i32 %loadedValue64.i, 16
  %"&(pSB[currWI].offset)81.i" = add nuw i64 %CurrSBIndex..1.i, 340
  %"&pSB[currWI].offset82.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)81.i"
  %CastToValueType83.i = bitcast i8* %"&pSB[currWI].offset82.i" to i1*
  store i1 %66, i1* %CastToValueType83.i, align 1
  br i1 %66, label %67, label %70

; <label>:67                                      ; preds = %SyncBB.i
  %"&(pSB[currWI].offset)22.i" = add nuw i64 %CurrSBIndex..1.i, 328
  %"&pSB[currWI].offset23.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)22.i"
  %CastToValueType24.i = bitcast i8* %"&pSB[currWI].offset23.i" to i64*
  %loadedValue25.i = load i64* %CastToValueType24.i, align 8
  %68 = and i64 %loadedValue25.i, 4294967295
  %69 = getelementptr inbounds [16 x i32] addrspace(3)* %38, i64 0, i64 %68
  store i32 0, i32 addrspace(3)* %69, align 4
  br label %70

; <label>:70                                      ; preds = %67, %SyncBB.i
  %check.WI.iter160.i = icmp ult i64 %CurrWI..1.i, %34
  br i1 %check.WI.iter160.i, label %thenBB157.i, label %SyncBB150.i

thenBB157.i:                                      ; preds = %70
  %"CurrWI++161.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride163.i" = add nuw i64 %CurrSBIndex..1.i, 560
  br label %SyncBB.i

SyncBB150.i:                                      ; preds = %thenBB164.i, %70
  %CurrSBIndex..2.i = phi i64 [ %"loadedCurrSB+Stride170.i", %thenBB164.i ], [ 0, %70 ]
  %CurrWI..2.i = phi i64 [ %"CurrWI++168.i", %thenBB164.i ], [ 0, %70 ]
  %"&(pSB[currWI].offset)56.i" = add nuw i64 %CurrSBIndex..2.i, 336
  %"&pSB[currWI].offset57.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)56.i"
  %CastToValueType58.i = bitcast i8* %"&pSB[currWI].offset57.i" to i32*
  %loadedValue59.i = load i32* %CastToValueType58.i, align 4
  %71 = icmp eq i32 %loadedValue59.i, 0
  %"&(pSB[currWI].offset)95.i" = add nuw i64 %CurrSBIndex..2.i, 341
  %"&pSB[currWI].offset96.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)95.i"
  %CastToValueType97.i = bitcast i8* %"&pSB[currWI].offset96.i" to i1*
  store i1 %71, i1* %CastToValueType97.i, align 1
  br i1 %71, label %phi-split-bb.i, label %72

; <label>:72                                      ; preds = %SyncBB150.i
  %"&(pSB[currWI].offset)17.i" = add nuw i64 %CurrSBIndex..2.i, 328
  %"&pSB[currWI].offset18.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)17.i"
  %CastToValueType19.i = bitcast i8* %"&pSB[currWI].offset18.i" to i64*
  %loadedValue20.i = load i64* %CastToValueType19.i, align 8
  %73 = and i64 %loadedValue20.i, 4294967295
  %74 = getelementptr inbounds i32 addrspace(3)* %16, i64 %73
  %75 = load i32 addrspace(3)* %74, align 4
  %"&(pSB[currWI].offset)51.i" = add nuw i64 %CurrSBIndex..2.i, 336
  %"&pSB[currWI].offset52.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)51.i"
  %CastToValueType53.i = bitcast i8* %"&pSB[currWI].offset52.i" to i32*
  %loadedValue54.i = load i32* %CastToValueType53.i, align 4
  %76 = add i32 %loadedValue54.i, -1
  %77 = zext i32 %76 to i64
  %78 = getelementptr inbounds i32 addrspace(3)* %16, i64 %77
  %79 = load i32 addrspace(3)* %78, align 4
  %80 = icmp eq i32 %75, %79
  br i1 %80, label %phi-split-bb.i, label %81

; <label>:81                                      ; preds = %72
  %82 = zext i32 %75 to i64
  %83 = getelementptr inbounds [16 x i32] addrspace(3)* %38, i64 0, i64 %82
  %"&(pSB[currWI].offset)66.i" = add nuw i64 %CurrSBIndex..2.i, 336
  %"&pSB[currWI].offset67.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)66.i"
  %CastToValueType68.i = bitcast i8* %"&pSB[currWI].offset67.i" to i32*
  %loadedValue69.i = load i32* %CastToValueType68.i, align 4
  store i32 %loadedValue69.i, i32 addrspace(3)* %83, align 4
  br label %phi-split-bb.i

phi-split-bb.i:                                   ; preds = %81, %72, %SyncBB150.i
  %"&(pSB[currWI].offset)46.i" = add nuw i64 %CurrSBIndex..2.i, 336
  %"&pSB[currWI].offset47.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)46.i"
  %CastToValueType48.i = bitcast i8* %"&pSB[currWI].offset47.i" to i32*
  %loadedValue49.i = load i32* %CastToValueType48.i, align 4
  %84 = add i32 %65, %loadedValue49.i
  %"&(pSB[currWI].offset)104.i" = add nuw i64 %CurrSBIndex..2.i, 344
  %"&pSB[currWI].offset105.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)104.i"
  %CastToValueType106.i = bitcast i8* %"&pSB[currWI].offset105.i" to i32*
  store i32 %84, i32* %CastToValueType106.i, align 4
  %85 = zext i32 %84 to i64
  %86 = getelementptr inbounds i32 addrspace(3)* %16, i64 %85
  %"&(pSB[currWI].offset)118.i" = add nuw i64 %CurrSBIndex..2.i, 352
  %"&pSB[currWI].offset119.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)118.i"
  %CastToValueType120.i = bitcast i8* %"&pSB[currWI].offset119.i" to i32 addrspace(3)**
  store i32 addrspace(3)* %86, i32 addrspace(3)** %CastToValueType120.i, align 8
  %87 = load i32 addrspace(3)* %86, align 4
  %"&(pSB[currWI].offset)41.i" = add nuw i64 %CurrSBIndex..2.i, 336
  %"&pSB[currWI].offset42.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)41.i"
  %CastToValueType43.i = bitcast i8* %"&pSB[currWI].offset42.i" to i32*
  %loadedValue44.i = load i32* %CastToValueType43.i, align 4
  %88 = add i32 %loadedValue44.i, -1
  %"&(pSB[currWI].offset)127.i" = add nuw i64 %CurrSBIndex..2.i, 360
  %"&pSB[currWI].offset128.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)127.i"
  %CastToValueType129.i = bitcast i8* %"&pSB[currWI].offset128.i" to i32*
  store i32 %88, i32* %CastToValueType129.i, align 4
  %89 = add i32 %88, %65
  %90 = zext i32 %89 to i64
  %91 = getelementptr inbounds i32 addrspace(3)* %16, i64 %90
  %"&(pSB[currWI].offset)136.i" = add nuw i64 %CurrSBIndex..2.i, 368
  %"&pSB[currWI].offset137.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)136.i"
  %CastToValueType138.i = bitcast i8* %"&pSB[currWI].offset137.i" to i32 addrspace(3)**
  store i32 addrspace(3)* %91, i32 addrspace(3)** %CastToValueType138.i, align 8
  %92 = load i32 addrspace(3)* %91, align 4
  %93 = icmp eq i32 %87, %92
  br i1 %93, label %97, label %94

; <label>:94                                      ; preds = %phi-split-bb.i
  %95 = zext i32 %87 to i64
  %96 = getelementptr inbounds [16 x i32] addrspace(3)* %38, i64 0, i64 %95
  %"&(pSB[currWI].offset)108.i" = add nuw i64 %CurrSBIndex..2.i, 344
  %"&pSB[currWI].offset109.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)108.i"
  %CastToValueType110.i = bitcast i8* %"&pSB[currWI].offset109.i" to i32*
  %loadedValue111.i = load i32* %CastToValueType110.i, align 4
  store i32 %loadedValue111.i, i32 addrspace(3)* %96, align 4
  br label %97

; <label>:97                                      ; preds = %94, %phi-split-bb.i
  %check.WI.iter167.i = icmp ult i64 %CurrWI..2.i, %34
  br i1 %check.WI.iter167.i, label %thenBB164.i, label %elseBB165.i

thenBB164.i:                                      ; preds = %97
  %"CurrWI++168.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride170.i" = add nuw i64 %CurrSBIndex..2.i, 560
  br label %SyncBB150.i

elseBB165.i:                                      ; preds = %97
  %98 = shl i32 %64, 4
  br label %SyncBB151.i

SyncBB151.i:                                      ; preds = %thenBB171.i, %elseBB165.i
  %CurrSBIndex..3.i = phi i64 [ 0, %elseBB165.i ], [ %"loadedCurrSB+Stride177.i", %thenBB171.i ]
  %CurrWI..3.i = phi i64 [ 0, %elseBB165.i ], [ %"CurrWI++175.i", %thenBB171.i ]
  %"&(pSB[currWI].offset)90.i" = add nuw i64 %CurrSBIndex..3.i, 340
  %"&pSB[currWI].offset91.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)90.i"
  %CastToValueType92.i = bitcast i8* %"&pSB[currWI].offset91.i" to i1*
  %loadedValue93.i = load i1* %CastToValueType92.i, align 1
  br i1 %loadedValue93.i, label %99, label %106

; <label>:99                                      ; preds = %SyncBB151.i
  %"&(pSB[currWI].offset)12.i" = add nuw i64 %CurrSBIndex..3.i, 328
  %"&pSB[currWI].offset13.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)12.i"
  %CastToValueType14.i = bitcast i8* %"&pSB[currWI].offset13.i" to i64*
  %loadedValue15.i = load i64* %CastToValueType14.i, align 8
  %100 = and i64 %loadedValue15.i, 4294967295
  %101 = getelementptr inbounds [16 x i32] addrspace(3)* %38, i64 0, i64 %100
  %102 = load i32 addrspace(3)* %101, align 4
  %"&(pSB[currWI].offset)36.i" = add nuw i64 %CurrSBIndex..3.i, 336
  %"&pSB[currWI].offset37.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)36.i"
  %CastToValueType38.i = bitcast i8* %"&pSB[currWI].offset37.i" to i32*
  %loadedValue39.i = load i32* %CastToValueType38.i, align 4
  %103 = add i32 %98, %loadedValue39.i
  %104 = zext i32 %103 to i64
  %105 = getelementptr inbounds i32 addrspace(1)* %7, i64 %104
  store i32 %102, i32 addrspace(1)* %105, align 4
  br label %106

; <label>:106                                     ; preds = %99, %SyncBB151.i
  %check.WI.iter174.i = icmp ult i64 %CurrWI..3.i, %34
  br i1 %check.WI.iter174.i, label %thenBB171.i, label %elseBB172.i

thenBB171.i:                                      ; preds = %106
  %"CurrWI++175.i" = add nuw i64 %CurrWI..3.i, 1
  %"loadedCurrSB+Stride177.i" = add nuw i64 %CurrSBIndex..3.i, 560
  br label %SyncBB151.i

elseBB172.i:                                      ; preds = %106
  %107 = add i32 %65, -1
  %108 = shl i32 %65, 1
  %109 = add i32 %108, -1
  %110 = zext i32 %109 to i64
  %111 = getelementptr inbounds i32 addrspace(3)* %16, i64 %110
  br label %SyncBB152.i

SyncBB152.i:                                      ; preds = %thenBB185.i, %elseBB172.i
  %CurrSBIndex..5.i = phi i64 [ 0, %elseBB172.i ], [ %"loadedCurrSB+Stride191.i", %thenBB185.i ]
  %CurrWI..5.i = phi i64 [ 0, %elseBB172.i ], [ %"CurrWI++189.i", %thenBB185.i ]
  %"&(pSB[currWI].offset)99.i" = add nuw i64 %CurrSBIndex..5.i, 341
  %"&pSB[currWI].offset100.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)99.i"
  %CastToValueType101.i = bitcast i8* %"&pSB[currWI].offset100.i" to i1*
  %loadedValue102.i = load i1* %CastToValueType101.i, align 1
  br i1 %loadedValue102.i, label %phi-split-bb1.i, label %112

; <label>:112                                     ; preds = %SyncBB152.i
  %"&(pSB[currWI].offset)7.i" = add nuw i64 %CurrSBIndex..5.i, 328
  %"&pSB[currWI].offset8.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)7.i"
  %CastToValueType9.i = bitcast i8* %"&pSB[currWI].offset8.i" to i64*
  %loadedValue10.i = load i64* %CastToValueType9.i, align 8
  %113 = and i64 %loadedValue10.i, 4294967295
  %114 = getelementptr inbounds i32 addrspace(3)* %16, i64 %113
  %115 = load i32 addrspace(3)* %114, align 4
  %"&(pSB[currWI].offset)131.i" = add nuw i64 %CurrSBIndex..5.i, 360
  %"&pSB[currWI].offset132.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)131.i"
  %CastToValueType133.i = bitcast i8* %"&pSB[currWI].offset132.i" to i32*
  %loadedValue134.i = load i32* %CastToValueType133.i, align 4
  %116 = zext i32 %loadedValue134.i to i64
  %117 = getelementptr inbounds i32 addrspace(3)* %16, i64 %116
  %118 = load i32 addrspace(3)* %117, align 4
  %119 = icmp eq i32 %115, %118
  br i1 %119, label %phi-split-bb1.i, label %120

; <label>:120                                     ; preds = %112
  %121 = zext i32 %118 to i64
  %122 = getelementptr inbounds [16 x i32] addrspace(3)* %38, i64 0, i64 %121
  %123 = load i32 addrspace(3)* %122, align 4
  %"&(pSB[currWI].offset)71.i" = add nuw i64 %CurrSBIndex..5.i, 336
  %"&pSB[currWI].offset72.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)71.i"
  %CastToValueType73.i = bitcast i8* %"&pSB[currWI].offset72.i" to i32*
  %loadedValue74.i = load i32* %CastToValueType73.i, align 4
  %124 = sub i32 %loadedValue74.i, %123
  store i32 %124, i32 addrspace(3)* %122, align 4
  br label %phi-split-bb1.i

phi-split-bb1.i:                                  ; preds = %120, %112, %SyncBB152.i
  %"&(pSB[currWI].offset)122.i" = add nuw i64 %CurrSBIndex..5.i, 352
  %"&pSB[currWI].offset123.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)122.i"
  %CastToValueType124.i = bitcast i8* %"&pSB[currWI].offset123.i" to i32 addrspace(3)**
  %loadedValue125.i = load i32 addrspace(3)** %CastToValueType124.i, align 8
  %125 = load i32 addrspace(3)* %loadedValue125.i, align 4
  %"&(pSB[currWI].offset)140.i" = add nuw i64 %CurrSBIndex..5.i, 368
  %"&pSB[currWI].offset141.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)140.i"
  %CastToValueType142.i = bitcast i8* %"&pSB[currWI].offset141.i" to i32 addrspace(3)**
  %loadedValue143.i = load i32 addrspace(3)** %CastToValueType142.i, align 8
  %126 = load i32 addrspace(3)* %loadedValue143.i, align 4
  %127 = icmp eq i32 %125, %126
  br i1 %127, label %133, label %128

; <label>:128                                     ; preds = %phi-split-bb1.i
  %129 = zext i32 %126 to i64
  %130 = getelementptr inbounds [16 x i32] addrspace(3)* %38, i64 0, i64 %129
  %131 = load i32 addrspace(3)* %130, align 4
  %"&(pSB[currWI].offset)113.i" = add nuw i64 %CurrSBIndex..5.i, 344
  %"&pSB[currWI].offset114.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)113.i"
  %CastToValueType115.i = bitcast i8* %"&pSB[currWI].offset114.i" to i32*
  %loadedValue116.i = load i32* %CastToValueType115.i, align 4
  %132 = sub i32 %loadedValue116.i, %131
  store i32 %132, i32 addrspace(3)* %130, align 4
  br label %133

; <label>:133                                     ; preds = %128, %phi-split-bb1.i
  %"&(pSB[currWI].offset)76.i" = add nuw i64 %CurrSBIndex..5.i, 336
  %"&pSB[currWI].offset77.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)76.i"
  %CastToValueType78.i = bitcast i8* %"&pSB[currWI].offset77.i" to i32*
  %loadedValue79.i = load i32* %CastToValueType78.i, align 4
  %134 = icmp eq i32 %loadedValue79.i, %107
  br i1 %134, label %135, label %141

; <label>:135                                     ; preds = %133
  %136 = load i32 addrspace(3)* %111, align 4
  %137 = zext i32 %136 to i64
  %138 = getelementptr inbounds [16 x i32] addrspace(3)* %38, i64 0, i64 %137
  %139 = load i32 addrspace(3)* %138, align 4
  %140 = sub i32 %108, %139
  store i32 %140, i32 addrspace(3)* %138, align 4
  br label %141

; <label>:141                                     ; preds = %135, %133
  %check.WI.iter188.i = icmp ult i64 %CurrWI..5.i, %34
  br i1 %check.WI.iter188.i, label %thenBB185.i, label %SyncBB154.i

thenBB185.i:                                      ; preds = %141
  %"CurrWI++189.i" = add nuw i64 %CurrWI..5.i, 1
  %"loadedCurrSB+Stride191.i" = add nuw i64 %CurrSBIndex..5.i, 560
  br label %SyncBB152.i

SyncBB154.i:                                      ; preds = %thenBB178.i, %141
  %CurrSBIndex..4.i = phi i64 [ %"loadedCurrSB+Stride184.i", %thenBB178.i ], [ 0, %141 ]
  %CurrWI..4.i = phi i64 [ %"CurrWI++182.i", %thenBB178.i ], [ 0, %141 ]
  %"&(pSB[currWI].offset)85.i" = add nuw i64 %CurrSBIndex..4.i, 340
  %"&pSB[currWI].offset86.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)85.i"
  %CastToValueType87.i = bitcast i8* %"&pSB[currWI].offset86.i" to i1*
  %loadedValue88.i = load i1* %CastToValueType87.i, align 1
  br i1 %loadedValue88.i, label %142, label %UnifiedReturnBlock.i

; <label>:142                                     ; preds = %SyncBB154.i
  %"&(pSB[currWI].offset)3.i" = add nuw i64 %CurrSBIndex..4.i, 328
  %"&pSB[currWI].offset4.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)3.i"
  %CastToValueType5.i = bitcast i8* %"&pSB[currWI].offset4.i" to i64*
  %loadedValue.i = load i64* %CastToValueType5.i, align 8
  %143 = and i64 %loadedValue.i, 4294967295
  %144 = getelementptr inbounds [16 x i32] addrspace(3)* %38, i64 0, i64 %143
  %145 = load i32 addrspace(3)* %144, align 4
  %"&(pSB[currWI].offset)31.i" = add nuw i64 %CurrSBIndex..4.i, 336
  %"&pSB[currWI].offset32.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)31.i"
  %CastToValueType33.i = bitcast i8* %"&pSB[currWI].offset32.i" to i32*
  %loadedValue34.i = load i32* %CastToValueType33.i, align 4
  %146 = mul i32 %loadedValue34.i, %13
  %147 = add i32 %146, %64
  %148 = zext i32 %147 to i64
  %149 = getelementptr inbounds i32 addrspace(1)* %4, i64 %148
  store i32 %145, i32 addrspace(1)* %149, align 4
  br label %UnifiedReturnBlock.i

UnifiedReturnBlock.i:                             ; preds = %142, %SyncBB154.i
  %check.WI.iter181.i = icmp ult i64 %CurrWI..4.i, %34
  br i1 %check.WI.iter181.i, label %thenBB178.i, label %__findRadixOffsets_separated_args.exit

thenBB178.i:                                      ; preds = %UnifiedReturnBlock.i
  %"CurrWI++182.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride184.i" = add nuw i64 %CurrSBIndex..4.i, 560
  br label %SyncBB154.i

__findRadixOffsets_separated_args.exit:           ; preds = %UnifiedReturnBlock.i
  ret void
}

define void @radixSortBlocks(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i32*
  %1 = load i32* %0, align 4
  %2 = getelementptr i8* %pBuffer, i64 4
  %3 = bitcast i8* %2 to i32*
  %4 = load i32* %3, align 4
  %5 = getelementptr i8* %pBuffer, i64 8
  %6 = bitcast i8* %5 to <4 x i32> addrspace(1)**
  %7 = load <4 x i32> addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 16
  %9 = bitcast i8* %8 to <4 x i32> addrspace(1)**
  %10 = load <4 x i32> addrspace(1)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 24
  %12 = bitcast i8* %11 to <4 x i32> addrspace(1)**
  %13 = load <4 x i32> addrspace(1)** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 32
  %15 = bitcast i8* %14 to <4 x i32> addrspace(1)**
  %16 = load <4 x i32> addrspace(1)** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 40
  %18 = bitcast i8* %17 to i32 addrspace(3)**
  %19 = load i32 addrspace(3)** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 48
  %21 = bitcast i8* %20 to i8 addrspace(3)**
  %22 = load i8 addrspace(3)** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 56
  %24 = bitcast i8* %23 to %struct.WorkDim**
  %25 = load %struct.WorkDim** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 72
  %27 = bitcast i8* %26 to %struct.PaddedDimId**
  %28 = load %struct.PaddedDimId** %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 80
  %30 = bitcast i8* %29 to %struct.PaddedDimId**
  %31 = load %struct.PaddedDimId** %30, align 8
  %32 = getelementptr i8* %pBuffer, i64 96
  %33 = bitcast i8* %32 to i64*
  %34 = load i64* %33, align 8
  %35 = getelementptr i8* %pBuffer, i64 104
  %36 = bitcast i8* %35 to i8**
  %37 = load i8** %36, align 8
  %38 = getelementptr i8* %pBuffer, i64 112
  %39 = bitcast i8* %38 to i64**
  %40 = load i64** %39, align 8
  %41 = bitcast i8 addrspace(3)* %22 to i32 addrspace(3)*
  store i64 0, i64* %40, align 8
  %42 = add i32 %4, %1
  %43 = icmp ugt i32 %42, %4
  br label %SyncBB471.i

SyncBB471.i:                                      ; preds = %thenBB502.i, %thenBB509.i, %entry
  %currBarrier.3.i = phi i32 [ 65, %entry ], [ %currBarrier.2.i, %thenBB502.i ], [ %currBarrier.1.i, %thenBB509.i ]
  %CurrSBIndex..4.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride508.i", %thenBB502.i ], [ %"loadedCurrSB+Stride515.i", %thenBB509.i ]
  %currWI517.i = load i64* %40, align 8
  %44 = getelementptr %struct.PaddedDimId* %31, i64 %currWI517.i, i32 0, i64 0
  %45 = load i64* %44, align 8
  %46 = getelementptr %struct.PaddedDimId* %28, i64 0, i32 0, i64 0
  %47 = load i64* %46, align 8
  %48 = add i64 %45, %47
  %49 = trunc i64 %45 to i32
  %50 = getelementptr %struct.WorkDim* %25, i64 0, i32 3, i64 0
  %51 = load i64* %50, align 8
  %52 = trunc i64 %51 to i32
  %53 = and i64 %48, 4294967295
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %37, i64 %CurrSBIndex..4.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i64*
  store i64 %53, i64* %CastToValueType.i, align 8
  %54 = getelementptr inbounds <4 x i32> addrspace(1)* %13, i64 %53
  %55 = load <4 x i32> addrspace(1)* %54, align 16
  %"&(pSB[currWI].offset)29.i" = add nuw i64 %CurrSBIndex..4.i, 16
  %"&pSB[currWI].offset30.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)29.i"
  %CastToValueType31.i = bitcast i8* %"&pSB[currWI].offset30.i" to <4 x i32>*
  store <4 x i32> %55, <4 x i32>* %CastToValueType31.i, align 16
  %56 = getelementptr inbounds <4 x i32> addrspace(1)* %16, i64 %53
  %57 = load <4 x i32> addrspace(1)* %56, align 16
  %"&(pSB[currWI].offset)38.i" = add nuw i64 %CurrSBIndex..4.i, 32
  %"&pSB[currWI].offset39.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)38.i"
  %CastToValueType40.i = bitcast i8* %"&pSB[currWI].offset39.i" to <4 x i32>*
  store <4 x i32> %57, <4 x i32>* %CastToValueType40.i, align 16
  br i1 %43, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %SyncBB471.i
  %58 = add i32 %52, -1
  %59 = icmp eq i32 %49, %58
  %"&(pSB[currWI].offset)47.i" = add nuw i64 %CurrSBIndex..4.i, 48
  %"&pSB[currWI].offset48.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)47.i"
  %CastToValueType49.i = bitcast i8* %"&pSB[currWI].offset48.i" to i1*
  store i1 %59, i1* %CastToValueType49.i, align 1
  %60 = shl i32 %49, 2
  %"&(pSB[currWI].offset)56.i" = add nuw i64 %CurrSBIndex..4.i, 52
  %"&pSB[currWI].offset57.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)56.i"
  %CastToValueType58.i = bitcast i8* %"&pSB[currWI].offset57.i" to i32*
  store i32 %60, i32* %CastToValueType58.i, align 4
  %61 = and i64 %45, 4294967295
  %62 = getelementptr inbounds i32 addrspace(3)* %19, i64 %61
  %"&(pSB[currWI].offset)65.i" = add nuw i64 %CurrSBIndex..4.i, 56
  %"&pSB[currWI].offset66.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)65.i"
  %CastToValueType67.i = bitcast i8* %"&pSB[currWI].offset66.i" to i32 addrspace(3)**
  store i32 addrspace(3)* %62, i32 addrspace(3)** %CastToValueType67.i, align 8
  %63 = add i32 %52, %49
  %64 = zext i32 %63 to i64
  %65 = getelementptr inbounds i32 addrspace(3)* %19, i64 %64
  %"&(pSB[currWI].offset)79.i" = add nuw i64 %CurrSBIndex..4.i, 64
  %"&pSB[currWI].offset80.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)79.i"
  %CastToValueType81.i = bitcast i8* %"&pSB[currWI].offset80.i" to i32 addrspace(3)**
  store i32 addrspace(3)* %65, i32 addrspace(3)** %CastToValueType81.i, align 8
  %66 = shl i32 %52, 1
  %67 = add i32 %66, %49
  %68 = zext i32 %67 to i64
  %69 = getelementptr inbounds i32 addrspace(3)* %19, i64 %68
  %"&(pSB[currWI].offset)93.i" = add nuw i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset94.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)93.i"
  %CastToValueType95.i = bitcast i8* %"&pSB[currWI].offset94.i" to i32 addrspace(3)**
  store i32 addrspace(3)* %69, i32 addrspace(3)** %CastToValueType95.i, align 8
  %70 = mul i32 %52, 3
  %71 = add i32 %70, %49
  %72 = zext i32 %71 to i64
  %73 = getelementptr inbounds i32 addrspace(3)* %19, i64 %72
  %"&(pSB[currWI].offset)107.i" = add nuw i64 %CurrSBIndex..4.i, 80
  %"&pSB[currWI].offset108.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)107.i"
  %CastToValueType109.i = bitcast i8* %"&pSB[currWI].offset108.i" to i32 addrspace(3)**
  store i32 addrspace(3)* %73, i32 addrspace(3)** %CastToValueType109.i, align 8
  %74 = or i32 %60, 3
  %"&(pSB[currWI].offset)121.i" = add nuw i64 %CurrSBIndex..4.i, 88
  %"&pSB[currWI].offset122.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)121.i"
  %CastToValueType123.i = bitcast i8* %"&pSB[currWI].offset122.i" to i32*
  store i32 %74, i32* %CastToValueType123.i, align 4
  %75 = or i32 %60, 2
  %"&(pSB[currWI].offset)130.i" = add nuw i64 %CurrSBIndex..4.i, 92
  %"&pSB[currWI].offset131.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)130.i"
  %CastToValueType132.i = bitcast i8* %"&pSB[currWI].offset131.i" to i32*
  store i32 %75, i32* %CastToValueType132.i, align 4
  %76 = or i32 %60, 1
  %"&(pSB[currWI].offset)139.i" = add nuw i64 %CurrSBIndex..4.i, 96
  %"&pSB[currWI].offset140.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)139.i"
  %CastToValueType141.i = bitcast i8* %"&pSB[currWI].offset140.i" to i32*
  store i32 %76, i32* %CastToValueType141.i, align 4
  %"&(pSB[currWI].offset)33.i" = add nuw i64 %CurrSBIndex..4.i, 16
  %"&(pSB[currWI].offset)42.i" = add nuw i64 %CurrSBIndex..4.i, 32
  br label %"Barrier BB17.i"

"Barrier BB17.i":                                 ; preds = %SyncBB468.i, %bb.nph.i
  %currBarrier.1.i = phi i32 [ %currBarrier.3.i, %bb.nph.i ], [ %currBarrier.0.i, %SyncBB468.i ]
  %CurrSBIndex..2.i = phi i64 [ %CurrSBIndex..4.i, %bb.nph.i ], [ %CurrSBIndex..1.i, %SyncBB468.i ]
  %indvar.i = phi i32 [ 0, %bb.nph.i ], [ %indvar.next.i, %SyncBB468.i ]
  %"&(pSB[currWI].offset)42.pn.i" = phi i64 [ %"&(pSB[currWI].offset)42.i", %bb.nph.i ], [ %"&(pSB[currWI].offset)449.i", %SyncBB468.i ]
  %"&(pSB[currWI].offset)33.pn.i" = phi i64 [ %"&(pSB[currWI].offset)33.i", %bb.nph.i ], [ %"&(pSB[currWI].offset)435.i", %SyncBB468.i ]
  %key.07.in.in.i = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)33.pn.i"
  %value.08.in.in.i = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)42.pn.i"
  %key.07.in.i = bitcast i8* %key.07.in.in.i to <4 x i32>*
  %value.08.in.i = bitcast i8* %value.08.in.in.i to <4 x i32>*
  %key.07.i = load <4 x i32>* %key.07.in.i, align 16
  %value.08.i = load <4 x i32>* %value.08.in.i, align 16
  %"&(pSB[currWI].offset)157.i" = add nuw i64 %CurrSBIndex..2.i, 112
  %"&pSB[currWI].offset158.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)157.i"
  %CastToValueType159.i = bitcast i8* %"&pSB[currWI].offset158.i" to <4 x i32>*
  store <4 x i32> %value.08.i, <4 x i32>* %CastToValueType159.i, align 16
  %"&(pSB[currWI].offset)148.i" = add nuw i64 %CurrSBIndex..2.i, 100
  %"&pSB[currWI].offset149.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)148.i"
  %CastToValueType150.i = bitcast i8* %"&pSB[currWI].offset149.i" to i32*
  store i32 %indvar.i, i32* %CastToValueType150.i, align 4
  %shift.09.i = add i32 %indvar.i, %4
  %77 = extractelement <4 x i32> %key.07.i, i32 0
  %"&(pSB[currWI].offset)181.i" = add nuw i64 %CurrSBIndex..2.i, 128
  %"&pSB[currWI].offset182.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)181.i"
  %CastToValueType183.i = bitcast i8* %"&pSB[currWI].offset182.i" to i32*
  store i32 %77, i32* %CastToValueType183.i, align 4
  %tmp.i = shl i32 1, %shift.09.i
  %78 = and i32 %tmp.i, %77
  %79 = icmp eq i32 %78, 0
  %"&(pSB[currWI].offset)190.i" = add nuw i64 %CurrSBIndex..2.i, 132
  %"&pSB[currWI].offset191.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)190.i"
  %CastToValueType192.i = bitcast i8* %"&pSB[currWI].offset191.i" to i1*
  store i1 %79, i1* %CastToValueType192.i, align 1
  %80 = zext i1 %79 to i32
  %81 = insertelement <4 x i32> undef, i32 %80, i32 0
  %82 = extractelement <4 x i32> %key.07.i, i32 1
  %"&(pSB[currWI].offset)199.i" = add nuw i64 %CurrSBIndex..2.i, 136
  %"&pSB[currWI].offset200.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)199.i"
  %CastToValueType201.i = bitcast i8* %"&pSB[currWI].offset200.i" to i32*
  store i32 %82, i32* %CastToValueType201.i, align 4
  %83 = and i32 %tmp.i, %82
  %84 = icmp eq i32 %83, 0
  %"&(pSB[currWI].offset)208.i" = add nuw i64 %CurrSBIndex..2.i, 140
  %"&pSB[currWI].offset209.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)208.i"
  %CastToValueType210.i = bitcast i8* %"&pSB[currWI].offset209.i" to i1*
  store i1 %84, i1* %CastToValueType210.i, align 1
  %85 = zext i1 %84 to i32
  %86 = insertelement <4 x i32> %81, i32 %85, i32 1
  %87 = extractelement <4 x i32> %key.07.i, i32 2
  %"&(pSB[currWI].offset)217.i" = add nuw i64 %CurrSBIndex..2.i, 144
  %"&pSB[currWI].offset218.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)217.i"
  %CastToValueType219.i = bitcast i8* %"&pSB[currWI].offset218.i" to i32*
  store i32 %87, i32* %CastToValueType219.i, align 4
  %88 = and i32 %tmp.i, %87
  %89 = icmp eq i32 %88, 0
  %"&(pSB[currWI].offset)226.i" = add nuw i64 %CurrSBIndex..2.i, 148
  %"&pSB[currWI].offset227.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)226.i"
  %CastToValueType228.i = bitcast i8* %"&pSB[currWI].offset227.i" to i1*
  store i1 %89, i1* %CastToValueType228.i, align 1
  %90 = zext i1 %89 to i32
  %91 = insertelement <4 x i32> %86, i32 %90, i32 2
  %92 = extractelement <4 x i32> %key.07.i, i32 3
  %"&(pSB[currWI].offset)235.i" = add nuw i64 %CurrSBIndex..2.i, 152
  %"&pSB[currWI].offset236.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)235.i"
  %CastToValueType237.i = bitcast i8* %"&pSB[currWI].offset236.i" to i32*
  store i32 %92, i32* %CastToValueType237.i, align 4
  %93 = and i32 %tmp.i, %92
  %94 = icmp eq i32 %93, 0
  %"&(pSB[currWI].offset)244.i" = add nuw i64 %CurrSBIndex..2.i, 156
  %"&pSB[currWI].offset245.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)244.i"
  %CastToValueType246.i = bitcast i8* %"&pSB[currWI].offset245.i" to i1*
  store i1 %94, i1* %CastToValueType246.i, align 1
  %95 = zext i1 %94 to i32
  %"&(pSB[currWI].offset)253.i" = add nuw i64 %CurrSBIndex..2.i, 160
  %"&pSB[currWI].offset254.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)253.i"
  %CastToValueType255.i = bitcast i8* %"&pSB[currWI].offset254.i" to i32*
  store i32 %95, i32* %CastToValueType255.i, align 4
  %96 = insertelement <4 x i32> %91, i32 %95, i32 3
  %"&(pSB[currWI].offset)262.i" = add nuw i64 %CurrSBIndex..2.i, 176
  %"&pSB[currWI].offset263.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)262.i"
  %CastToValueType264.i = bitcast i8* %"&pSB[currWI].offset263.i" to <4 x i32>*
  store <4 x i32> %96, <4 x i32>* %CastToValueType264.i, align 16
  %loadedCurrWI511.i = load i64* %40, align 8
  %check.WI.iter512.i = icmp ult i64 %loadedCurrWI511.i, %34
  br i1 %check.WI.iter512.i, label %thenBB509.i, label %elseBB510.i

thenBB509.i:                                      ; preds = %"Barrier BB17.i"
  %"CurrWI++513.i" = add nuw i64 %loadedCurrWI511.i, 1
  store i64 %"CurrWI++513.i", i64* %40, align 8
  %"loadedCurrSB+Stride515.i" = add nuw i64 %CurrSBIndex..2.i, 560
  %cond2.i = icmp eq i32 %currBarrier.1.i, 65
  br i1 %cond2.i, label %SyncBB471.i, label %SyncBB468.i

elseBB510.i:                                      ; preds = %"Barrier BB17.i"
  store i64 0, i64* %40, align 8
  br label %SyncBB22.i.i

SyncBB22.i.i:                                     ; preds = %thenBB26.i.i, %elseBB510.i
  %CurrSBIndex..0.i.i = phi i64 [ 0, %elseBB510.i ], [ %"loadedCurrSB+Stride32.i.i", %thenBB26.i.i ]
  %loadedCurrWI28.i.i = load i64* %40, align 8
  %check.WI.iter29.i.i = icmp ult i64 %loadedCurrWI28.i.i, %34
  br i1 %check.WI.iter29.i.i, label %thenBB26.i.i, label %elseBB27.i.i

thenBB26.i.i:                                     ; preds = %SyncBB22.i.i
  %"CurrWI++30.i.i" = add nuw i64 %loadedCurrWI28.i.i, 1
  store i64 %"CurrWI++30.i.i", i64* %40, align 8
  %"loadedCurrSB+Stride32.i.i" = add nuw i64 %CurrSBIndex..0.i.i, 560
  br label %SyncBB22.i.i

elseBB27.i.i:                                     ; preds = %SyncBB22.i.i
  %"&(pSB[currWI].offset)5.i.i" = add nuw i64 %CurrSBIndex..0.i.i, 176
  %"&(pSB[currWI].offset)10.i.i" = add nuw i64 %CurrSBIndex..0.i.i, 176
  %"&(pSB[currWI].offset)15.i.i" = add nuw i64 %CurrSBIndex..0.i.i, 176
  %"&(pSB[currWI].offset)24.i.i" = add nuw i64 %CurrSBIndex..0.i.i, 176
  %"&pSB[currWI].offset6.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)5.i.i"
  %"&pSB[currWI].offset11.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)10.i.i"
  %"&pSB[currWI].offset16.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)15.i.i"
  %"&pSB[currWI].offset25.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)24.i.i"
  %CastToArgType.i.i = bitcast i8* %"&pSB[currWI].offset6.i.i" to <4 x i32>*
  %CastToArgType12.i.i = bitcast i8* %"&pSB[currWI].offset11.i.i" to <4 x i32>*
  %CastToArgType17.i.i = bitcast i8* %"&pSB[currWI].offset16.i.i" to <4 x i32>*
  %CastToArgType26.i.i = bitcast i8* %"&pSB[currWI].offset25.i.i" to <4 x i32>*
  %loadedValue8.i.i = load <4 x i32>* %CastToArgType.i.i, align 16
  %loadedValue13.i.i = load <4 x i32>* %CastToArgType12.i.i, align 16
  %loadedValue22.i.i = load <4 x i32>* %CastToArgType17.i.i, align 16
  %loadedValue27.i.i = load <4 x i32>* %CastToArgType26.i.i, align 16
  %97 = extractelement <4 x i32> %loadedValue27.i.i, i32 1
  %98 = extractelement <4 x i32> %loadedValue22.i.i, i32 0
  %99 = add i32 %97, %98
  %100 = extractelement <4 x i32> %loadedValue13.i.i, i32 2
  %101 = add i32 %99, %100
  store i64 0, i64* %40, align 8
  %102 = extractelement <4 x i32> %loadedValue8.i.i, i32 3
  %103 = add i32 %101, %102
  br label %SyncBB.i.i

SyncBB.i.i:                                       ; preds = %thenBB220.i.i, %elseBB27.i.i
  %CurrWI..0.i.i = phi i64 [ %"CurrWI++224.i.i", %thenBB220.i.i ], [ 0, %elseBB27.i.i ]
  %CurrSBIndex..0.i3.i = phi i64 [ %"loadedCurrSB+Stride226.i.i", %thenBB220.i.i ], [ 0, %elseBB27.i.i ]
  %104 = getelementptr %struct.PaddedDimId* %31, i64 %CurrWI..0.i.i, i32 0, i64 0
  %105 = load i64* %104, align 8
  %"&(pSB[currWI].offset).i4.i" = add nuw i64 %CurrSBIndex..0.i3.i, 504
  %"&pSB[currWI].offset.i5.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset).i4.i"
  %CastToValueType.i6.i = bitcast i8* %"&pSB[currWI].offset.i5.i" to i64*
  store i64 %105, i64* %CastToValueType.i6.i, align 8
  %sext.i.i = shl i64 %105, 32
  %106 = ashr i64 %sext.i.i, 32
  %107 = getelementptr inbounds i32 addrspace(3)* %19, i64 %106
  store i32 0, i32 addrspace(3)* %107, align 4
  %check.WI.iter223.i.i = icmp ult i64 %CurrWI..0.i.i, %34
  br i1 %check.WI.iter223.i.i, label %thenBB220.i.i, label %SyncBB203.i.i

thenBB220.i.i:                                    ; preds = %SyncBB.i.i
  %"CurrWI++224.i.i" = add nuw i64 %CurrWI..0.i.i, 1
  %"loadedCurrSB+Stride226.i.i" = add nuw i64 %CurrSBIndex..0.i3.i, 560
  br label %SyncBB.i.i

SyncBB203.i.i:                                    ; preds = %thenBB227.i.i, %SyncBB.i.i
  %CurrWI..1.i.i = phi i64 [ %"CurrWI++231.i.i", %thenBB227.i.i ], [ 0, %SyncBB.i.i ]
  %CurrSBIndex..1.i7.i = phi i64 [ %"loadedCurrSB+Stride233.i.i", %thenBB227.i.i ], [ 0, %SyncBB.i.i ]
  %108 = getelementptr %struct.WorkDim* %25, i64 0, i32 3, i64 0
  %109 = load i64* %108, align 8
  %"&(pSB[currWI].offset)18.i.i" = add nuw i64 %CurrSBIndex..1.i7.i, 504
  %"&pSB[currWI].offset19.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)18.i.i"
  %CastToValueType20.i.i = bitcast i8* %"&pSB[currWI].offset19.i.i" to i64*
  %loadedValue.i8.i = load i64* %CastToValueType20.i.i, align 8
  %110 = add i64 %109, %loadedValue.i8.i
  %111 = trunc i64 %110 to i32
  %"&(pSB[currWI].offset)22.i.i" = add nuw i64 %CurrSBIndex..1.i7.i, 512
  %"&pSB[currWI].offset23.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)22.i.i"
  %CastToValueType24.i.i = bitcast i8* %"&pSB[currWI].offset23.i.i" to i32*
  store i32 %111, i32* %CastToValueType24.i.i, align 4
  %112 = sext i32 %111 to i64
  %113 = getelementptr inbounds i32 addrspace(3)* %19, i64 %112
  %"&(pSB[currWI].offset)61.i.i" = add nuw i64 %CurrSBIndex..1.i7.i, 520
  %"&pSB[currWI].offset62.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)61.i.i"
  %CastToValueType63.i.i = bitcast i8* %"&pSB[currWI].offset62.i.i" to i32 addrspace(3)**
  store i32 addrspace(3)* %113, i32 addrspace(3)** %CastToValueType63.i.i, align 8
  store i32 %103, i32 addrspace(3)* %113, align 4
  %check.WI.iter230.i.i = icmp ult i64 %CurrWI..1.i.i, %34
  br i1 %check.WI.iter230.i.i, label %thenBB227.i.i, label %SyncBB204.i.i

thenBB227.i.i:                                    ; preds = %SyncBB203.i.i
  %"CurrWI++231.i.i" = add nuw i64 %CurrWI..1.i.i, 1
  %"loadedCurrSB+Stride233.i.i" = add nuw i64 %CurrSBIndex..1.i7.i, 560
  br label %SyncBB203.i.i

SyncBB204.i.i:                                    ; preds = %thenBB234.i.i, %SyncBB203.i.i
  %CurrWI..2.i.i = phi i64 [ %"CurrWI++238.i.i", %thenBB234.i.i ], [ 0, %SyncBB203.i.i ]
  %CurrSBIndex..2.i.i = phi i64 [ %"loadedCurrSB+Stride240.i.i", %thenBB234.i.i ], [ 0, %SyncBB203.i.i ]
  %"&(pSB[currWI].offset)56.i.i" = add nuw i64 %CurrSBIndex..2.i.i, 512
  %"&pSB[currWI].offset57.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)56.i.i"
  %CastToValueType58.i.i = bitcast i8* %"&pSB[currWI].offset57.i.i" to i32*
  %loadedValue59.i.i = load i32* %CastToValueType58.i.i, align 4
  %114 = add nsw i32 %loadedValue59.i.i, -1
  %115 = sext i32 %114 to i64
  %116 = getelementptr inbounds i32 addrspace(3)* %19, i64 %115
  %117 = load i32 addrspace(3)* %116, align 4
  %"&(pSB[currWI].offset)140.i.i" = add nuw i64 %CurrSBIndex..2.i.i, 528
  %"&pSB[currWI].offset141.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)140.i.i"
  %CastToValueType142.i.i = bitcast i8* %"&pSB[currWI].offset141.i.i" to i32*
  store i32 %117, i32* %CastToValueType142.i.i, align 4
  %check.WI.iter237.i.i = icmp ult i64 %CurrWI..2.i.i, %34
  br i1 %check.WI.iter237.i.i, label %thenBB234.i.i, label %SyncBB205.i.i

thenBB234.i.i:                                    ; preds = %SyncBB204.i.i
  %"CurrWI++238.i.i" = add nuw i64 %CurrWI..2.i.i, 1
  %"loadedCurrSB+Stride240.i.i" = add nuw i64 %CurrSBIndex..2.i.i, 560
  br label %SyncBB204.i.i

SyncBB205.i.i:                                    ; preds = %thenBB241.i.i, %SyncBB204.i.i
  %CurrWI..3.i.i = phi i64 [ %"CurrWI++245.i.i", %thenBB241.i.i ], [ 0, %SyncBB204.i.i ]
  %CurrSBIndex..3.i.i = phi i64 [ %"loadedCurrSB+Stride247.i.i", %thenBB241.i.i ], [ 0, %SyncBB204.i.i ]
  %"&(pSB[currWI].offset)130.i.i" = add nuw i64 %CurrSBIndex..3.i.i, 520
  %"&pSB[currWI].offset131.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)130.i.i"
  %CastToValueType132.i.i = bitcast i8* %"&pSB[currWI].offset131.i.i" to i32 addrspace(3)**
  %loadedValue133.i.i = load i32 addrspace(3)** %CastToValueType132.i.i, align 8
  %118 = load i32 addrspace(3)* %loadedValue133.i.i, align 4
  %"&(pSB[currWI].offset)144.i.i" = add nuw i64 %CurrSBIndex..3.i.i, 528
  %"&pSB[currWI].offset145.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)144.i.i"
  %CastToValueType146.i.i = bitcast i8* %"&pSB[currWI].offset145.i.i" to i32*
  %loadedValue147.i.i = load i32* %CastToValueType146.i.i, align 4
  %119 = add i32 %118, %loadedValue147.i.i
  %"&(pSB[currWI].offset)135.i.i" = add nuw i64 %CurrSBIndex..3.i.i, 520
  %"&pSB[currWI].offset136.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)135.i.i"
  %CastToValueType137.i.i = bitcast i8* %"&pSB[currWI].offset136.i.i" to i32 addrspace(3)**
  %loadedValue138.i.i = load i32 addrspace(3)** %CastToValueType137.i.i, align 8
  store i32 %119, i32 addrspace(3)* %loadedValue138.i.i, align 4
  %check.WI.iter244.i.i = icmp ult i64 %CurrWI..3.i.i, %34
  br i1 %check.WI.iter244.i.i, label %thenBB241.i.i, label %SyncBB206.i.i

thenBB241.i.i:                                    ; preds = %SyncBB205.i.i
  %"CurrWI++245.i.i" = add nuw i64 %CurrWI..3.i.i, 1
  %"loadedCurrSB+Stride247.i.i" = add nuw i64 %CurrSBIndex..3.i.i, 560
  br label %SyncBB205.i.i

SyncBB206.i.i:                                    ; preds = %thenBB248.i.i, %SyncBB205.i.i
  %CurrWI..4.i.i = phi i64 [ %"CurrWI++252.i.i", %thenBB248.i.i ], [ 0, %SyncBB205.i.i ]
  %CurrSBIndex..4.i.i = phi i64 [ %"loadedCurrSB+Stride254.i.i", %thenBB248.i.i ], [ 0, %SyncBB205.i.i ]
  %"&(pSB[currWI].offset)51.i.i" = add nuw i64 %CurrSBIndex..4.i.i, 512
  %"&pSB[currWI].offset52.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)51.i.i"
  %CastToValueType53.i.i = bitcast i8* %"&pSB[currWI].offset52.i.i" to i32*
  %loadedValue54.i.i = load i32* %CastToValueType53.i.i, align 4
  %120 = add nsw i32 %loadedValue54.i.i, -2
  %121 = sext i32 %120 to i64
  %122 = getelementptr inbounds i32 addrspace(3)* %19, i64 %121
  %123 = load i32 addrspace(3)* %122, align 4
  %"&(pSB[currWI].offset)149.i.i" = add nuw i64 %CurrSBIndex..4.i.i, 532
  %"&pSB[currWI].offset150.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)149.i.i"
  %CastToValueType151.i.i = bitcast i8* %"&pSB[currWI].offset150.i.i" to i32*
  store i32 %123, i32* %CastToValueType151.i.i, align 4
  %check.WI.iter251.i.i = icmp ult i64 %CurrWI..4.i.i, %34
  br i1 %check.WI.iter251.i.i, label %thenBB248.i.i, label %SyncBB207.i.i

thenBB248.i.i:                                    ; preds = %SyncBB206.i.i
  %"CurrWI++252.i.i" = add nuw i64 %CurrWI..4.i.i, 1
  %"loadedCurrSB+Stride254.i.i" = add nuw i64 %CurrSBIndex..4.i.i, 560
  br label %SyncBB206.i.i

SyncBB207.i.i:                                    ; preds = %thenBB255.i.i, %SyncBB206.i.i
  %CurrWI..5.i.i = phi i64 [ %"CurrWI++259.i.i", %thenBB255.i.i ], [ 0, %SyncBB206.i.i ]
  %CurrSBIndex..5.i.i = phi i64 [ %"loadedCurrSB+Stride261.i.i", %thenBB255.i.i ], [ 0, %SyncBB206.i.i ]
  %"&(pSB[currWI].offset)120.i.i" = add nuw i64 %CurrSBIndex..5.i.i, 520
  %"&pSB[currWI].offset121.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)120.i.i"
  %CastToValueType122.i.i = bitcast i8* %"&pSB[currWI].offset121.i.i" to i32 addrspace(3)**
  %loadedValue123.i.i = load i32 addrspace(3)** %CastToValueType122.i.i, align 8
  %124 = load i32 addrspace(3)* %loadedValue123.i.i, align 4
  %"&(pSB[currWI].offset)153.i.i" = add nuw i64 %CurrSBIndex..5.i.i, 532
  %"&pSB[currWI].offset154.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)153.i.i"
  %CastToValueType155.i.i = bitcast i8* %"&pSB[currWI].offset154.i.i" to i32*
  %loadedValue156.i.i = load i32* %CastToValueType155.i.i, align 4
  %125 = add i32 %124, %loadedValue156.i.i
  %"&(pSB[currWI].offset)125.i.i" = add nuw i64 %CurrSBIndex..5.i.i, 520
  %"&pSB[currWI].offset126.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)125.i.i"
  %CastToValueType127.i.i = bitcast i8* %"&pSB[currWI].offset126.i.i" to i32 addrspace(3)**
  %loadedValue128.i.i = load i32 addrspace(3)** %CastToValueType127.i.i, align 8
  store i32 %125, i32 addrspace(3)* %loadedValue128.i.i, align 4
  %check.WI.iter258.i.i = icmp ult i64 %CurrWI..5.i.i, %34
  br i1 %check.WI.iter258.i.i, label %thenBB255.i.i, label %SyncBB208.i.i

thenBB255.i.i:                                    ; preds = %SyncBB207.i.i
  %"CurrWI++259.i.i" = add nuw i64 %CurrWI..5.i.i, 1
  %"loadedCurrSB+Stride261.i.i" = add nuw i64 %CurrSBIndex..5.i.i, 560
  br label %SyncBB207.i.i

SyncBB208.i.i:                                    ; preds = %thenBB262.i.i, %SyncBB207.i.i
  %CurrWI..6.i.i = phi i64 [ %"CurrWI++266.i.i", %thenBB262.i.i ], [ 0, %SyncBB207.i.i ]
  %CurrSBIndex..6.i.i = phi i64 [ %"loadedCurrSB+Stride268.i.i", %thenBB262.i.i ], [ 0, %SyncBB207.i.i ]
  %"&(pSB[currWI].offset)46.i.i" = add nuw i64 %CurrSBIndex..6.i.i, 512
  %"&pSB[currWI].offset47.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)46.i.i"
  %CastToValueType48.i.i = bitcast i8* %"&pSB[currWI].offset47.i.i" to i32*
  %loadedValue49.i.i = load i32* %CastToValueType48.i.i, align 4
  %126 = add nsw i32 %loadedValue49.i.i, -4
  %127 = sext i32 %126 to i64
  %128 = getelementptr inbounds i32 addrspace(3)* %19, i64 %127
  %129 = load i32 addrspace(3)* %128, align 4
  %"&(pSB[currWI].offset)158.i.i" = add nuw i64 %CurrSBIndex..6.i.i, 536
  %"&pSB[currWI].offset159.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)158.i.i"
  %CastToValueType160.i.i = bitcast i8* %"&pSB[currWI].offset159.i.i" to i32*
  store i32 %129, i32* %CastToValueType160.i.i, align 4
  %check.WI.iter265.i.i = icmp ult i64 %CurrWI..6.i.i, %34
  br i1 %check.WI.iter265.i.i, label %thenBB262.i.i, label %SyncBB209.i.i

thenBB262.i.i:                                    ; preds = %SyncBB208.i.i
  %"CurrWI++266.i.i" = add nuw i64 %CurrWI..6.i.i, 1
  %"loadedCurrSB+Stride268.i.i" = add nuw i64 %CurrSBIndex..6.i.i, 560
  br label %SyncBB208.i.i

SyncBB209.i.i:                                    ; preds = %thenBB269.i.i, %SyncBB208.i.i
  %CurrWI..7.i.i = phi i64 [ %"CurrWI++273.i.i", %thenBB269.i.i ], [ 0, %SyncBB208.i.i ]
  %CurrSBIndex..7.i.i = phi i64 [ %"loadedCurrSB+Stride275.i.i", %thenBB269.i.i ], [ 0, %SyncBB208.i.i ]
  %"&(pSB[currWI].offset)110.i.i" = add nuw i64 %CurrSBIndex..7.i.i, 520
  %"&pSB[currWI].offset111.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)110.i.i"
  %CastToValueType112.i.i = bitcast i8* %"&pSB[currWI].offset111.i.i" to i32 addrspace(3)**
  %loadedValue113.i.i = load i32 addrspace(3)** %CastToValueType112.i.i, align 8
  %130 = load i32 addrspace(3)* %loadedValue113.i.i, align 4
  %"&(pSB[currWI].offset)162.i.i" = add nuw i64 %CurrSBIndex..7.i.i, 536
  %"&pSB[currWI].offset163.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)162.i.i"
  %CastToValueType164.i.i = bitcast i8* %"&pSB[currWI].offset163.i.i" to i32*
  %loadedValue165.i.i = load i32* %CastToValueType164.i.i, align 4
  %131 = add i32 %130, %loadedValue165.i.i
  %"&(pSB[currWI].offset)115.i.i" = add nuw i64 %CurrSBIndex..7.i.i, 520
  %"&pSB[currWI].offset116.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)115.i.i"
  %CastToValueType117.i.i = bitcast i8* %"&pSB[currWI].offset116.i.i" to i32 addrspace(3)**
  %loadedValue118.i.i = load i32 addrspace(3)** %CastToValueType117.i.i, align 8
  store i32 %131, i32 addrspace(3)* %loadedValue118.i.i, align 4
  %check.WI.iter272.i.i = icmp ult i64 %CurrWI..7.i.i, %34
  br i1 %check.WI.iter272.i.i, label %thenBB269.i.i, label %SyncBB210.i.i

thenBB269.i.i:                                    ; preds = %SyncBB209.i.i
  %"CurrWI++273.i.i" = add nuw i64 %CurrWI..7.i.i, 1
  %"loadedCurrSB+Stride275.i.i" = add nuw i64 %CurrSBIndex..7.i.i, 560
  br label %SyncBB209.i.i

SyncBB210.i.i:                                    ; preds = %thenBB276.i.i, %SyncBB209.i.i
  %CurrWI..8.i.i = phi i64 [ %"CurrWI++280.i.i", %thenBB276.i.i ], [ 0, %SyncBB209.i.i ]
  %CurrSBIndex..8.i.i = phi i64 [ %"loadedCurrSB+Stride282.i.i", %thenBB276.i.i ], [ 0, %SyncBB209.i.i ]
  %"&(pSB[currWI].offset)41.i.i" = add nuw i64 %CurrSBIndex..8.i.i, 512
  %"&pSB[currWI].offset42.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)41.i.i"
  %CastToValueType43.i.i = bitcast i8* %"&pSB[currWI].offset42.i.i" to i32*
  %loadedValue44.i.i = load i32* %CastToValueType43.i.i, align 4
  %132 = add nsw i32 %loadedValue44.i.i, -8
  %133 = sext i32 %132 to i64
  %134 = getelementptr inbounds i32 addrspace(3)* %19, i64 %133
  %135 = load i32 addrspace(3)* %134, align 4
  %"&(pSB[currWI].offset)167.i.i" = add nuw i64 %CurrSBIndex..8.i.i, 540
  %"&pSB[currWI].offset168.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)167.i.i"
  %CastToValueType169.i.i = bitcast i8* %"&pSB[currWI].offset168.i.i" to i32*
  store i32 %135, i32* %CastToValueType169.i.i, align 4
  %check.WI.iter279.i.i = icmp ult i64 %CurrWI..8.i.i, %34
  br i1 %check.WI.iter279.i.i, label %thenBB276.i.i, label %SyncBB211.i.i

thenBB276.i.i:                                    ; preds = %SyncBB210.i.i
  %"CurrWI++280.i.i" = add nuw i64 %CurrWI..8.i.i, 1
  %"loadedCurrSB+Stride282.i.i" = add nuw i64 %CurrSBIndex..8.i.i, 560
  br label %SyncBB210.i.i

SyncBB211.i.i:                                    ; preds = %thenBB283.i.i, %SyncBB210.i.i
  %CurrWI..9.i.i = phi i64 [ %"CurrWI++287.i.i", %thenBB283.i.i ], [ 0, %SyncBB210.i.i ]
  %CurrSBIndex..9.i.i = phi i64 [ %"loadedCurrSB+Stride289.i.i", %thenBB283.i.i ], [ 0, %SyncBB210.i.i ]
  %"&(pSB[currWI].offset)100.i.i" = add nuw i64 %CurrSBIndex..9.i.i, 520
  %"&pSB[currWI].offset101.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)100.i.i"
  %CastToValueType102.i.i = bitcast i8* %"&pSB[currWI].offset101.i.i" to i32 addrspace(3)**
  %loadedValue103.i.i = load i32 addrspace(3)** %CastToValueType102.i.i, align 8
  %136 = load i32 addrspace(3)* %loadedValue103.i.i, align 4
  %"&(pSB[currWI].offset)171.i.i" = add nuw i64 %CurrSBIndex..9.i.i, 540
  %"&pSB[currWI].offset172.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)171.i.i"
  %CastToValueType173.i.i = bitcast i8* %"&pSB[currWI].offset172.i.i" to i32*
  %loadedValue174.i.i = load i32* %CastToValueType173.i.i, align 4
  %137 = add i32 %136, %loadedValue174.i.i
  %"&(pSB[currWI].offset)105.i.i" = add nuw i64 %CurrSBIndex..9.i.i, 520
  %"&pSB[currWI].offset106.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)105.i.i"
  %CastToValueType107.i.i = bitcast i8* %"&pSB[currWI].offset106.i.i" to i32 addrspace(3)**
  %loadedValue108.i.i = load i32 addrspace(3)** %CastToValueType107.i.i, align 8
  store i32 %137, i32 addrspace(3)* %loadedValue108.i.i, align 4
  %check.WI.iter286.i.i = icmp ult i64 %CurrWI..9.i.i, %34
  br i1 %check.WI.iter286.i.i, label %thenBB283.i.i, label %SyncBB212.i.i

thenBB283.i.i:                                    ; preds = %SyncBB211.i.i
  %"CurrWI++287.i.i" = add nuw i64 %CurrWI..9.i.i, 1
  %"loadedCurrSB+Stride289.i.i" = add nuw i64 %CurrSBIndex..9.i.i, 560
  br label %SyncBB211.i.i

SyncBB212.i.i:                                    ; preds = %thenBB290.i.i, %SyncBB211.i.i
  %CurrWI..10.i.i = phi i64 [ %"CurrWI++294.i.i", %thenBB290.i.i ], [ 0, %SyncBB211.i.i ]
  %CurrSBIndex..10.i.i = phi i64 [ %"loadedCurrSB+Stride296.i.i", %thenBB290.i.i ], [ 0, %SyncBB211.i.i ]
  %"&(pSB[currWI].offset)36.i.i" = add nuw i64 %CurrSBIndex..10.i.i, 512
  %"&pSB[currWI].offset37.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)36.i.i"
  %CastToValueType38.i.i = bitcast i8* %"&pSB[currWI].offset37.i.i" to i32*
  %loadedValue39.i.i = load i32* %CastToValueType38.i.i, align 4
  %138 = add nsw i32 %loadedValue39.i.i, -16
  %139 = sext i32 %138 to i64
  %140 = getelementptr inbounds i32 addrspace(3)* %19, i64 %139
  %141 = load i32 addrspace(3)* %140, align 4
  %"&(pSB[currWI].offset)176.i.i" = add nuw i64 %CurrSBIndex..10.i.i, 544
  %"&pSB[currWI].offset177.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)176.i.i"
  %CastToValueType178.i.i = bitcast i8* %"&pSB[currWI].offset177.i.i" to i32*
  store i32 %141, i32* %CastToValueType178.i.i, align 4
  %check.WI.iter293.i.i = icmp ult i64 %CurrWI..10.i.i, %34
  br i1 %check.WI.iter293.i.i, label %thenBB290.i.i, label %SyncBB213.i.i

thenBB290.i.i:                                    ; preds = %SyncBB212.i.i
  %"CurrWI++294.i.i" = add nuw i64 %CurrWI..10.i.i, 1
  %"loadedCurrSB+Stride296.i.i" = add nuw i64 %CurrSBIndex..10.i.i, 560
  br label %SyncBB212.i.i

SyncBB213.i.i:                                    ; preds = %thenBB297.i.i, %SyncBB212.i.i
  %CurrWI..11.i.i = phi i64 [ %"CurrWI++301.i.i", %thenBB297.i.i ], [ 0, %SyncBB212.i.i ]
  %CurrSBIndex..11.i.i = phi i64 [ %"loadedCurrSB+Stride303.i.i", %thenBB297.i.i ], [ 0, %SyncBB212.i.i ]
  %"&(pSB[currWI].offset)90.i.i" = add nuw i64 %CurrSBIndex..11.i.i, 520
  %"&pSB[currWI].offset91.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)90.i.i"
  %CastToValueType92.i.i = bitcast i8* %"&pSB[currWI].offset91.i.i" to i32 addrspace(3)**
  %loadedValue93.i.i = load i32 addrspace(3)** %CastToValueType92.i.i, align 8
  %142 = load i32 addrspace(3)* %loadedValue93.i.i, align 4
  %"&(pSB[currWI].offset)180.i.i" = add nuw i64 %CurrSBIndex..11.i.i, 544
  %"&pSB[currWI].offset181.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)180.i.i"
  %CastToValueType182.i.i = bitcast i8* %"&pSB[currWI].offset181.i.i" to i32*
  %loadedValue183.i.i = load i32* %CastToValueType182.i.i, align 4
  %143 = add i32 %142, %loadedValue183.i.i
  %"&(pSB[currWI].offset)95.i.i" = add nuw i64 %CurrSBIndex..11.i.i, 520
  %"&pSB[currWI].offset96.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)95.i.i"
  %CastToValueType97.i.i = bitcast i8* %"&pSB[currWI].offset96.i.i" to i32 addrspace(3)**
  %loadedValue98.i.i = load i32 addrspace(3)** %CastToValueType97.i.i, align 8
  store i32 %143, i32 addrspace(3)* %loadedValue98.i.i, align 4
  %check.WI.iter300.i.i = icmp ult i64 %CurrWI..11.i.i, %34
  br i1 %check.WI.iter300.i.i, label %thenBB297.i.i, label %SyncBB214.i.i

thenBB297.i.i:                                    ; preds = %SyncBB213.i.i
  %"CurrWI++301.i.i" = add nuw i64 %CurrWI..11.i.i, 1
  %"loadedCurrSB+Stride303.i.i" = add nuw i64 %CurrSBIndex..11.i.i, 560
  br label %SyncBB213.i.i

SyncBB214.i.i:                                    ; preds = %thenBB304.i.i, %SyncBB213.i.i
  %CurrWI..12.i.i = phi i64 [ %"CurrWI++308.i.i", %thenBB304.i.i ], [ 0, %SyncBB213.i.i ]
  %CurrSBIndex..12.i.i = phi i64 [ %"loadedCurrSB+Stride310.i.i", %thenBB304.i.i ], [ 0, %SyncBB213.i.i ]
  %"&(pSB[currWI].offset)31.i.i" = add nuw i64 %CurrSBIndex..12.i.i, 512
  %"&pSB[currWI].offset32.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)31.i.i"
  %CastToValueType33.i.i = bitcast i8* %"&pSB[currWI].offset32.i.i" to i32*
  %loadedValue34.i.i = load i32* %CastToValueType33.i.i, align 4
  %144 = add nsw i32 %loadedValue34.i.i, -32
  %145 = sext i32 %144 to i64
  %146 = getelementptr inbounds i32 addrspace(3)* %19, i64 %145
  %147 = load i32 addrspace(3)* %146, align 4
  %"&(pSB[currWI].offset)185.i.i" = add nuw i64 %CurrSBIndex..12.i.i, 548
  %"&pSB[currWI].offset186.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)185.i.i"
  %CastToValueType187.i.i = bitcast i8* %"&pSB[currWI].offset186.i.i" to i32*
  store i32 %147, i32* %CastToValueType187.i.i, align 4
  %check.WI.iter307.i.i = icmp ult i64 %CurrWI..12.i.i, %34
  br i1 %check.WI.iter307.i.i, label %thenBB304.i.i, label %SyncBB215.i.i

thenBB304.i.i:                                    ; preds = %SyncBB214.i.i
  %"CurrWI++308.i.i" = add nuw i64 %CurrWI..12.i.i, 1
  %"loadedCurrSB+Stride310.i.i" = add nuw i64 %CurrSBIndex..12.i.i, 560
  br label %SyncBB214.i.i

SyncBB215.i.i:                                    ; preds = %thenBB311.i.i, %SyncBB214.i.i
  %CurrWI..13.i.i = phi i64 [ %"CurrWI++315.i.i", %thenBB311.i.i ], [ 0, %SyncBB214.i.i ]
  %CurrSBIndex..13.i.i = phi i64 [ %"loadedCurrSB+Stride317.i.i", %thenBB311.i.i ], [ 0, %SyncBB214.i.i ]
  %"&(pSB[currWI].offset)80.i.i" = add nuw i64 %CurrSBIndex..13.i.i, 520
  %"&pSB[currWI].offset81.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)80.i.i"
  %CastToValueType82.i.i = bitcast i8* %"&pSB[currWI].offset81.i.i" to i32 addrspace(3)**
  %loadedValue83.i.i = load i32 addrspace(3)** %CastToValueType82.i.i, align 8
  %148 = load i32 addrspace(3)* %loadedValue83.i.i, align 4
  %"&(pSB[currWI].offset)189.i.i" = add nuw i64 %CurrSBIndex..13.i.i, 548
  %"&pSB[currWI].offset190.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)189.i.i"
  %CastToValueType191.i.i = bitcast i8* %"&pSB[currWI].offset190.i.i" to i32*
  %loadedValue192.i.i = load i32* %CastToValueType191.i.i, align 4
  %149 = add i32 %148, %loadedValue192.i.i
  %"&(pSB[currWI].offset)85.i.i" = add nuw i64 %CurrSBIndex..13.i.i, 520
  %"&pSB[currWI].offset86.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)85.i.i"
  %CastToValueType87.i.i = bitcast i8* %"&pSB[currWI].offset86.i.i" to i32 addrspace(3)**
  %loadedValue88.i.i = load i32 addrspace(3)** %CastToValueType87.i.i, align 8
  store i32 %149, i32 addrspace(3)* %loadedValue88.i.i, align 4
  %check.WI.iter314.i.i = icmp ult i64 %CurrWI..13.i.i, %34
  br i1 %check.WI.iter314.i.i, label %thenBB311.i.i, label %SyncBB216.i.i

thenBB311.i.i:                                    ; preds = %SyncBB215.i.i
  %"CurrWI++315.i.i" = add nuw i64 %CurrWI..13.i.i, 1
  %"loadedCurrSB+Stride317.i.i" = add nuw i64 %CurrSBIndex..13.i.i, 560
  br label %SyncBB215.i.i

SyncBB216.i.i:                                    ; preds = %thenBB318.i.i, %SyncBB215.i.i
  %CurrWI..14.i.i = phi i64 [ %"CurrWI++322.i.i", %thenBB318.i.i ], [ 0, %SyncBB215.i.i ]
  %CurrSBIndex..14.i.i = phi i64 [ %"loadedCurrSB+Stride324.i.i", %thenBB318.i.i ], [ 0, %SyncBB215.i.i ]
  %"&(pSB[currWI].offset)26.i.i" = add nuw i64 %CurrSBIndex..14.i.i, 512
  %"&pSB[currWI].offset27.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)26.i.i"
  %CastToValueType28.i.i = bitcast i8* %"&pSB[currWI].offset27.i.i" to i32*
  %loadedValue29.i.i = load i32* %CastToValueType28.i.i, align 4
  %150 = add nsw i32 %loadedValue29.i.i, -64
  %151 = sext i32 %150 to i64
  %152 = getelementptr inbounds i32 addrspace(3)* %19, i64 %151
  %153 = load i32 addrspace(3)* %152, align 4
  %"&(pSB[currWI].offset)194.i.i" = add nuw i64 %CurrSBIndex..14.i.i, 552
  %"&pSB[currWI].offset195.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)194.i.i"
  %CastToValueType196.i.i = bitcast i8* %"&pSB[currWI].offset195.i.i" to i32*
  store i32 %153, i32* %CastToValueType196.i.i, align 4
  %check.WI.iter321.i.i = icmp ult i64 %CurrWI..14.i.i, %34
  br i1 %check.WI.iter321.i.i, label %thenBB318.i.i, label %SyncBB217.i.i

thenBB318.i.i:                                    ; preds = %SyncBB216.i.i
  %"CurrWI++322.i.i" = add nuw i64 %CurrWI..14.i.i, 1
  %"loadedCurrSB+Stride324.i.i" = add nuw i64 %CurrSBIndex..14.i.i, 560
  br label %SyncBB216.i.i

SyncBB217.i.i:                                    ; preds = %thenBB325.i.i, %SyncBB216.i.i
  %CurrWI..15.i.i = phi i64 [ %"CurrWI++329.i.i", %thenBB325.i.i ], [ 0, %SyncBB216.i.i ]
  %CurrSBIndex..15.i.i = phi i64 [ %"loadedCurrSB+Stride331.i.i", %thenBB325.i.i ], [ 0, %SyncBB216.i.i ]
  %"&(pSB[currWI].offset)70.i.i" = add nuw i64 %CurrSBIndex..15.i.i, 520
  %"&pSB[currWI].offset71.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)70.i.i"
  %CastToValueType72.i.i = bitcast i8* %"&pSB[currWI].offset71.i.i" to i32 addrspace(3)**
  %loadedValue73.i.i = load i32 addrspace(3)** %CastToValueType72.i.i, align 8
  %154 = load i32 addrspace(3)* %loadedValue73.i.i, align 4
  %"&(pSB[currWI].offset)198.i.i" = add nuw i64 %CurrSBIndex..15.i.i, 552
  %"&pSB[currWI].offset199.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)198.i.i"
  %CastToValueType200.i.i = bitcast i8* %"&pSB[currWI].offset199.i.i" to i32*
  %loadedValue201.i.i = load i32* %CastToValueType200.i.i, align 4
  %155 = add i32 %154, %loadedValue201.i.i
  %"&(pSB[currWI].offset)75.i.i" = add nuw i64 %CurrSBIndex..15.i.i, 520
  %"&pSB[currWI].offset76.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)75.i.i"
  %CastToValueType77.i.i = bitcast i8* %"&pSB[currWI].offset76.i.i" to i32 addrspace(3)**
  %loadedValue78.i.i = load i32 addrspace(3)** %CastToValueType77.i.i, align 8
  store i32 %155, i32 addrspace(3)* %loadedValue78.i.i, align 4
  %check.WI.iter328.i.i = icmp ult i64 %CurrWI..15.i.i, %34
  br i1 %check.WI.iter328.i.i, label %thenBB325.i.i, label %SyncBB218.i.i

thenBB325.i.i:                                    ; preds = %SyncBB217.i.i
  %"CurrWI++329.i.i" = add nuw i64 %CurrWI..15.i.i, 1
  %"loadedCurrSB+Stride331.i.i" = add nuw i64 %CurrSBIndex..15.i.i, 560
  br label %SyncBB217.i.i

SyncBB218.i.i:                                    ; preds = %thenBB.i13.i, %SyncBB217.i.i
  %CurrWI..16.i.i = phi i64 [ %"CurrWI++.i11.i", %thenBB.i13.i ], [ 0, %SyncBB217.i.i ]
  %CurrSBIndex..16.i.i = phi i64 [ %"loadedCurrSB+Stride.i12.i", %thenBB.i13.i ], [ 0, %SyncBB217.i.i ]
  %"&(pSB[currWI].offset)65.i.i" = add nuw i64 %CurrSBIndex..16.i.i, 520
  %"&pSB[currWI].offset66.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)65.i.i"
  %CastToValueType67.i.i = bitcast i8* %"&pSB[currWI].offset66.i.i" to i32 addrspace(3)**
  %loadedValue68.i.i = load i32 addrspace(3)** %CastToValueType67.i.i, align 8
  %156 = load i32 addrspace(3)* %loadedValue68.i.i, align 4
  %157 = sub i32 %156, %103
  %"&(pSB[currWI].offset)3.i.i" = add nuw i64 %CurrSBIndex..16.i.i, 324
  %"&pSB[currWI].offset4.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)3.i.i"
  %CastToArgType.i9.i = bitcast i8* %"&pSB[currWI].offset4.i.i" to i32*
  store i32 %157, i32* %CastToArgType.i9.i, align 4
  %check.WI.iter.i10.i = icmp ult i64 %CurrWI..16.i.i, %34
  br i1 %check.WI.iter.i10.i, label %thenBB.i13.i, label %scanLSB_New.exit.i

thenBB.i13.i:                                     ; preds = %SyncBB218.i.i
  %"CurrWI++.i11.i" = add nuw i64 %CurrWI..16.i.i, 1
  %"loadedCurrSB+Stride.i12.i" = add nuw i64 %CurrSBIndex..16.i.i, 560
  br label %SyncBB218.i.i

scanLSB_New.exit.i:                               ; preds = %SyncBB218.i.i
  store i64 0, i64* %40, align 8
  br label %SyncBB23.i.i

SyncBB23.i.i:                                     ; preds = %thenBB.i.i, %scanLSB_New.exit.i
  %CurrSBIndex..1.i.i = phi i64 [ 0, %scanLSB_New.exit.i ], [ %"loadedCurrSB+Stride.i.i", %thenBB.i.i ]
  %"&(pSB[currWI].offset)14.i.i" = add nuw i64 %CurrSBIndex..1.i.i, 324
  %"&pSB[currWI].offset15.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)14.i.i"
  %CastToValueType16.i.i = bitcast i8* %"&pSB[currWI].offset15.i.i" to i32*
  %loadedValue17.i.i = load i32* %CastToValueType16.i.i, align 4
  %158 = insertelement <4 x i32> undef, i32 %loadedValue17.i.i, i32 0
  %"&(pSB[currWI].offset)9.i.i" = add nuw i64 %CurrSBIndex..1.i.i, 324
  %"&pSB[currWI].offset10.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)9.i.i"
  %CastToValueType11.i.i = bitcast i8* %"&pSB[currWI].offset10.i.i" to i32*
  %loadedValue12.i.i = load i32* %CastToValueType11.i.i, align 4
  %159 = add i32 %loadedValue12.i.i, %98
  %160 = insertelement <4 x i32> %158, i32 %159, i32 1
  %"&(pSB[currWI].offset)4.i.i" = add nuw i64 %CurrSBIndex..1.i.i, 324
  %"&pSB[currWI].offset5.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)4.i.i"
  %CastToValueType6.i.i = bitcast i8* %"&pSB[currWI].offset5.i.i" to i32*
  %loadedValue7.i.i = load i32* %CastToValueType6.i.i, align 4
  %161 = add i32 %loadedValue7.i.i, %99
  %162 = insertelement <4 x i32> %160, i32 %161, i32 2
  %"&(pSB[currWI].offset).i.i" = add nuw i64 %CurrSBIndex..1.i.i, 324
  %"&pSB[currWI].offset.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset).i.i"
  %CastToValueType.i.i = bitcast i8* %"&pSB[currWI].offset.i.i" to i32*
  %loadedValue.i.i = load i32* %CastToValueType.i.i, align 4
  %163 = add i32 %loadedValue.i.i, %101
  %164 = insertelement <4 x i32> %162, i32 %163, i32 3
  %"&(pSB[currWI].offset)29.i.i" = add nuw i64 %CurrSBIndex..1.i.i, 192
  %"&pSB[currWI].offset30.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)29.i.i"
  %CastToArgType31.i.i = bitcast i8* %"&pSB[currWI].offset30.i.i" to <4 x i32>*
  store <4 x i32> %164, <4 x i32>* %CastToArgType31.i.i, align 16
  %loadedCurrWI.i.i = load i64* %40, align 8
  %check.WI.iter.i.i = icmp ult i64 %loadedCurrWI.i.i, %34
  br i1 %check.WI.iter.i.i, label %thenBB.i.i, label %scan4_New.exit.i

thenBB.i.i:                                       ; preds = %SyncBB23.i.i
  %"CurrWI++.i.i" = add nuw i64 %loadedCurrWI.i.i, 1
  store i64 %"CurrWI++.i.i", i64* %40, align 8
  %"loadedCurrSB+Stride.i.i" = add nuw i64 %CurrSBIndex..1.i.i, 560
  br label %SyncBB23.i.i

scan4_New.exit.i:                                 ; preds = %SyncBB23.i.i
  store i64 0, i64* %40, align 8
  br label %SyncBB470.i

SyncBB470.i:                                      ; preds = %thenBB.i, %scan4_New.exit.i
  %CurrSBIndex..0.i = phi i64 [ 0, %scan4_New.exit.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %"&(pSB[currWI].offset)51.i" = add nuw i64 %CurrSBIndex..0.i, 48
  %"&pSB[currWI].offset52.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)51.i"
  %CastToValueType53.i = bitcast i8* %"&pSB[currWI].offset52.i" to i1*
  %loadedValue54.i = load i1* %CastToValueType53.i, align 1
  br i1 %loadedValue54.i, label %165, label %168

; <label>:165                                     ; preds = %SyncBB470.i
  %"&(pSB[currWI].offset)271.i" = add nuw i64 %CurrSBIndex..0.i, 192
  %"&pSB[currWI].offset272.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)271.i"
  %CastToValueType273.i = bitcast i8* %"&pSB[currWI].offset272.i" to <4 x i32>*
  %loadedValue274.i = load <4 x i32>* %CastToValueType273.i, align 16
  %166 = extractelement <4 x i32> %loadedValue274.i, i32 3
  %"&(pSB[currWI].offset)257.i" = add nuw i64 %CurrSBIndex..0.i, 160
  %"&pSB[currWI].offset258.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)257.i"
  %CastToValueType259.i = bitcast i8* %"&pSB[currWI].offset258.i" to i32*
  %loadedValue260.i = load i32* %CastToValueType259.i, align 4
  %167 = add i32 %166, %loadedValue260.i
  store i32 %167, i32 addrspace(3)* %41, align 4
  br label %168

; <label>:168                                     ; preds = %165, %SyncBB470.i
  %loadedCurrWI.i = load i64* %40, align 8
  %check.WI.iter.i = icmp ult i64 %loadedCurrWI.i, %34
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %168
  %"CurrWI++.i" = add nuw i64 %loadedCurrWI.i, 1
  store i64 %"CurrWI++.i", i64* %40, align 8
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 560
  br label %SyncBB470.i

elseBB.i:                                         ; preds = %168
  store i64 0, i64* %40, align 8
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB481.i, %elseBB.i
  %CurrSBIndex..6.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride487.i", %thenBB481.i ]
  %"&(pSB[currWI].offset)194.i" = add nuw i64 %CurrSBIndex..6.i, 132
  %"&pSB[currWI].offset195.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)194.i"
  %CastToValueType196.i = bitcast i8* %"&pSB[currWI].offset195.i" to i1*
  %loadedValue197.i = load i1* %CastToValueType196.i, align 1
  br i1 %loadedValue197.i, label %169, label %171

; <label>:169                                     ; preds = %SyncBB.i
  %"&(pSB[currWI].offset)276.i" = add nuw i64 %CurrSBIndex..6.i, 192
  %"&pSB[currWI].offset277.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)276.i"
  %CastToValueType278.i = bitcast i8* %"&pSB[currWI].offset277.i" to <4 x i32>*
  %loadedValue279.i = load <4 x i32>* %CastToValueType278.i, align 16
  %170 = extractelement <4 x i32> %loadedValue279.i, i32 0
  %"&(pSB[currWI].offset)316.i" = add nuw i64 %CurrSBIndex..6.i, 208
  %"&pSB[currWI].offset317.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)316.i"
  %CastToValueType318.i = bitcast i8* %"&pSB[currWI].offset317.i" to i32*
  store i32 %170, i32* %CastToValueType318.i, align 4
  br label %176

; <label>:171                                     ; preds = %SyncBB.i
  %172 = load i32 addrspace(3)* %41, align 4
  %"&(pSB[currWI].offset)281.i" = add nuw i64 %CurrSBIndex..6.i, 192
  %"&pSB[currWI].offset282.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)281.i"
  %CastToValueType283.i = bitcast i8* %"&pSB[currWI].offset282.i" to <4 x i32>*
  %loadedValue284.i = load <4 x i32>* %CastToValueType283.i, align 16
  %173 = extractelement <4 x i32> %loadedValue284.i, i32 0
  %"&(pSB[currWI].offset)60.i" = add nuw i64 %CurrSBIndex..6.i, 52
  %"&pSB[currWI].offset61.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)60.i"
  %CastToValueType62.i = bitcast i8* %"&pSB[currWI].offset61.i" to i32*
  %loadedValue63.i = load i32* %CastToValueType62.i, align 4
  %174 = sub i32 %loadedValue63.i, %173
  %175 = add i32 %174, %172
  %"&(pSB[currWI].offset)320.i" = add nuw i64 %CurrSBIndex..6.i, 212
  %"&pSB[currWI].offset321.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)320.i"
  %CastToValueType322.i = bitcast i8* %"&pSB[currWI].offset321.i" to i32*
  store i32 %175, i32* %CastToValueType322.i, align 4
  br label %176

; <label>:176                                     ; preds = %171, %169
  %177 = phi i32 [ %170, %169 ], [ %175, %171 ]
  %"&(pSB[currWI].offset)324.i" = add nuw i64 %CurrSBIndex..6.i, 216
  %"&pSB[currWI].offset325.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)324.i"
  %CastToValueType326.i = bitcast i8* %"&pSB[currWI].offset325.i" to i32*
  store i32 %177, i32* %CastToValueType326.i, align 4
  %"&(pSB[currWI].offset)212.i" = add nuw i64 %CurrSBIndex..6.i, 140
  %"&pSB[currWI].offset213.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)212.i"
  %CastToValueType214.i = bitcast i8* %"&pSB[currWI].offset213.i" to i1*
  %loadedValue215.i = load i1* %CastToValueType214.i, align 1
  br i1 %loadedValue215.i, label %178, label %180

; <label>:178                                     ; preds = %176
  %"&(pSB[currWI].offset)286.i" = add nuw i64 %CurrSBIndex..6.i, 192
  %"&pSB[currWI].offset287.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)286.i"
  %CastToValueType288.i = bitcast i8* %"&pSB[currWI].offset287.i" to <4 x i32>*
  %loadedValue289.i = load <4 x i32>* %CastToValueType288.i, align 16
  %179 = extractelement <4 x i32> %loadedValue289.i, i32 1
  %"&(pSB[currWI].offset)338.i" = add nuw i64 %CurrSBIndex..6.i, 220
  %"&pSB[currWI].offset339.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)338.i"
  %CastToValueType340.i = bitcast i8* %"&pSB[currWI].offset339.i" to i32*
  store i32 %179, i32* %CastToValueType340.i, align 4
  br label %185

; <label>:180                                     ; preds = %176
  %181 = load i32 addrspace(3)* %41, align 4
  %"&(pSB[currWI].offset)291.i" = add nuw i64 %CurrSBIndex..6.i, 192
  %"&pSB[currWI].offset292.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)291.i"
  %CastToValueType293.i = bitcast i8* %"&pSB[currWI].offset292.i" to <4 x i32>*
  %loadedValue294.i = load <4 x i32>* %CastToValueType293.i, align 16
  %182 = extractelement <4 x i32> %loadedValue294.i, i32 1
  %"&(pSB[currWI].offset)143.i" = add nuw i64 %CurrSBIndex..6.i, 96
  %"&pSB[currWI].offset144.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)143.i"
  %CastToValueType145.i = bitcast i8* %"&pSB[currWI].offset144.i" to i32*
  %loadedValue146.i = load i32* %CastToValueType145.i, align 4
  %183 = sub i32 %loadedValue146.i, %182
  %184 = add i32 %183, %181
  %"&(pSB[currWI].offset)342.i" = add nuw i64 %CurrSBIndex..6.i, 224
  %"&pSB[currWI].offset343.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)342.i"
  %CastToValueType344.i = bitcast i8* %"&pSB[currWI].offset343.i" to i32*
  store i32 %184, i32* %CastToValueType344.i, align 4
  br label %185

; <label>:185                                     ; preds = %180, %178
  %186 = phi i32 [ %179, %178 ], [ %184, %180 ]
  %"&(pSB[currWI].offset)346.i" = add nuw i64 %CurrSBIndex..6.i, 228
  %"&pSB[currWI].offset347.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)346.i"
  %CastToValueType348.i = bitcast i8* %"&pSB[currWI].offset347.i" to i32*
  store i32 %186, i32* %CastToValueType348.i, align 4
  %"&(pSB[currWI].offset)230.i" = add nuw i64 %CurrSBIndex..6.i, 148
  %"&pSB[currWI].offset231.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)230.i"
  %CastToValueType232.i = bitcast i8* %"&pSB[currWI].offset231.i" to i1*
  %loadedValue233.i = load i1* %CastToValueType232.i, align 1
  br i1 %loadedValue233.i, label %187, label %189

; <label>:187                                     ; preds = %185
  %"&(pSB[currWI].offset)296.i" = add nuw i64 %CurrSBIndex..6.i, 192
  %"&pSB[currWI].offset297.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)296.i"
  %CastToValueType298.i = bitcast i8* %"&pSB[currWI].offset297.i" to <4 x i32>*
  %loadedValue299.i = load <4 x i32>* %CastToValueType298.i, align 16
  %188 = extractelement <4 x i32> %loadedValue299.i, i32 2
  %"&(pSB[currWI].offset)360.i" = add nuw i64 %CurrSBIndex..6.i, 232
  %"&pSB[currWI].offset361.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)360.i"
  %CastToValueType362.i = bitcast i8* %"&pSB[currWI].offset361.i" to i32*
  store i32 %188, i32* %CastToValueType362.i, align 4
  br label %194

; <label>:189                                     ; preds = %185
  %190 = load i32 addrspace(3)* %41, align 4
  %"&(pSB[currWI].offset)301.i" = add nuw i64 %CurrSBIndex..6.i, 192
  %"&pSB[currWI].offset302.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)301.i"
  %CastToValueType303.i = bitcast i8* %"&pSB[currWI].offset302.i" to <4 x i32>*
  %loadedValue304.i = load <4 x i32>* %CastToValueType303.i, align 16
  %191 = extractelement <4 x i32> %loadedValue304.i, i32 2
  %"&(pSB[currWI].offset)134.i" = add nuw i64 %CurrSBIndex..6.i, 92
  %"&pSB[currWI].offset135.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)134.i"
  %CastToValueType136.i = bitcast i8* %"&pSB[currWI].offset135.i" to i32*
  %loadedValue137.i = load i32* %CastToValueType136.i, align 4
  %192 = sub i32 %loadedValue137.i, %191
  %193 = add i32 %192, %190
  %"&(pSB[currWI].offset)364.i" = add nuw i64 %CurrSBIndex..6.i, 236
  %"&pSB[currWI].offset365.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)364.i"
  %CastToValueType366.i = bitcast i8* %"&pSB[currWI].offset365.i" to i32*
  store i32 %193, i32* %CastToValueType366.i, align 4
  br label %194

; <label>:194                                     ; preds = %189, %187
  %195 = phi i32 [ %188, %187 ], [ %193, %189 ]
  %"&(pSB[currWI].offset)368.i" = add nuw i64 %CurrSBIndex..6.i, 240
  %"&pSB[currWI].offset369.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)368.i"
  %CastToValueType370.i = bitcast i8* %"&pSB[currWI].offset369.i" to i32*
  store i32 %195, i32* %CastToValueType370.i, align 4
  %"&(pSB[currWI].offset)248.i" = add nuw i64 %CurrSBIndex..6.i, 156
  %"&pSB[currWI].offset249.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)248.i"
  %CastToValueType250.i = bitcast i8* %"&pSB[currWI].offset249.i" to i1*
  %loadedValue251.i = load i1* %CastToValueType250.i, align 1
  br i1 %loadedValue251.i, label %196, label %198

; <label>:196                                     ; preds = %194
  %"&(pSB[currWI].offset)306.i" = add nuw i64 %CurrSBIndex..6.i, 192
  %"&pSB[currWI].offset307.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)306.i"
  %CastToValueType308.i = bitcast i8* %"&pSB[currWI].offset307.i" to <4 x i32>*
  %loadedValue309.i = load <4 x i32>* %CastToValueType308.i, align 16
  %197 = extractelement <4 x i32> %loadedValue309.i, i32 3
  %"&(pSB[currWI].offset)382.i" = add nuw i64 %CurrSBIndex..6.i, 244
  %"&pSB[currWI].offset383.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)382.i"
  %CastToValueType384.i = bitcast i8* %"&pSB[currWI].offset383.i" to i32*
  store i32 %197, i32* %CastToValueType384.i, align 4
  br label %"Barrier BB13.i"

; <label>:198                                     ; preds = %194
  %199 = load i32 addrspace(3)* %41, align 4
  %"&(pSB[currWI].offset)311.i" = add nuw i64 %CurrSBIndex..6.i, 192
  %"&pSB[currWI].offset312.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)311.i"
  %CastToValueType313.i = bitcast i8* %"&pSB[currWI].offset312.i" to <4 x i32>*
  %loadedValue314.i = load <4 x i32>* %CastToValueType313.i, align 16
  %200 = extractelement <4 x i32> %loadedValue314.i, i32 3
  %"&(pSB[currWI].offset)125.i" = add nuw i64 %CurrSBIndex..6.i, 88
  %"&pSB[currWI].offset126.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)125.i"
  %CastToValueType127.i = bitcast i8* %"&pSB[currWI].offset126.i" to i32*
  %loadedValue128.i = load i32* %CastToValueType127.i, align 4
  %201 = sub i32 %loadedValue128.i, %200
  %202 = add i32 %201, %199
  %"&(pSB[currWI].offset)386.i" = add nuw i64 %CurrSBIndex..6.i, 248
  %"&pSB[currWI].offset387.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)386.i"
  %CastToValueType388.i = bitcast i8* %"&pSB[currWI].offset387.i" to i32*
  store i32 %202, i32* %CastToValueType388.i, align 4
  br label %"Barrier BB13.i"

"Barrier BB13.i":                                 ; preds = %198, %196
  %203 = phi i32 [ %197, %196 ], [ %202, %198 ]
  %"&(pSB[currWI].offset)328.i" = add nuw i64 %CurrSBIndex..6.i, 216
  %"&pSB[currWI].offset329.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)328.i"
  %CastToValueType330.i = bitcast i8* %"&pSB[currWI].offset329.i" to i32*
  %loadedValue331.i = load i32* %CastToValueType330.i, align 4
  %204 = and i32 %loadedValue331.i, 3
  %205 = mul i32 %204, %52
  %"&(pSB[currWI].offset)333.i" = add nuw i64 %CurrSBIndex..6.i, 216
  %"&pSB[currWI].offset334.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)333.i"
  %CastToValueType335.i = bitcast i8* %"&pSB[currWI].offset334.i" to i32*
  %loadedValue336.i = load i32* %CastToValueType335.i, align 4
  %206 = lshr i32 %loadedValue336.i, 2
  %207 = add i32 %205, %206
  %208 = zext i32 %207 to i64
  %209 = getelementptr inbounds i32 addrspace(3)* %19, i64 %208
  %"&(pSB[currWI].offset)390.i" = add nuw i64 %CurrSBIndex..6.i, 256
  %"&pSB[currWI].offset391.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)390.i"
  %CastToValueType392.i = bitcast i8* %"&pSB[currWI].offset391.i" to i32 addrspace(3)**
  store i32 addrspace(3)* %209, i32 addrspace(3)** %CastToValueType392.i, align 8
  %"&(pSB[currWI].offset)185.i" = add nuw i64 %CurrSBIndex..6.i, 128
  %"&pSB[currWI].offset186.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)185.i"
  %CastToValueType187.i = bitcast i8* %"&pSB[currWI].offset186.i" to i32*
  %loadedValue188.i = load i32* %CastToValueType187.i, align 4
  store i32 %loadedValue188.i, i32 addrspace(3)* %209, align 4
  %"&(pSB[currWI].offset)350.i" = add nuw i64 %CurrSBIndex..6.i, 228
  %"&pSB[currWI].offset351.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)350.i"
  %CastToValueType352.i = bitcast i8* %"&pSB[currWI].offset351.i" to i32*
  %loadedValue353.i = load i32* %CastToValueType352.i, align 4
  %210 = and i32 %loadedValue353.i, 3
  %211 = mul i32 %210, %52
  %"&(pSB[currWI].offset)355.i" = add nuw i64 %CurrSBIndex..6.i, 228
  %"&pSB[currWI].offset356.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)355.i"
  %CastToValueType357.i = bitcast i8* %"&pSB[currWI].offset356.i" to i32*
  %loadedValue358.i = load i32* %CastToValueType357.i, align 4
  %212 = lshr i32 %loadedValue358.i, 2
  %213 = add i32 %211, %212
  %214 = zext i32 %213 to i64
  %215 = getelementptr inbounds i32 addrspace(3)* %19, i64 %214
  %"&(pSB[currWI].offset)399.i" = add nuw i64 %CurrSBIndex..6.i, 264
  %"&pSB[currWI].offset400.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)399.i"
  %CastToValueType401.i = bitcast i8* %"&pSB[currWI].offset400.i" to i32 addrspace(3)**
  store i32 addrspace(3)* %215, i32 addrspace(3)** %CastToValueType401.i, align 8
  %"&(pSB[currWI].offset)203.i" = add nuw i64 %CurrSBIndex..6.i, 136
  %"&pSB[currWI].offset204.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)203.i"
  %CastToValueType205.i = bitcast i8* %"&pSB[currWI].offset204.i" to i32*
  %loadedValue206.i = load i32* %CastToValueType205.i, align 4
  store i32 %loadedValue206.i, i32 addrspace(3)* %215, align 4
  %"&(pSB[currWI].offset)372.i" = add nuw i64 %CurrSBIndex..6.i, 240
  %"&pSB[currWI].offset373.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)372.i"
  %CastToValueType374.i = bitcast i8* %"&pSB[currWI].offset373.i" to i32*
  %loadedValue375.i = load i32* %CastToValueType374.i, align 4
  %216 = and i32 %loadedValue375.i, 3
  %217 = mul i32 %216, %52
  %"&(pSB[currWI].offset)377.i" = add nuw i64 %CurrSBIndex..6.i, 240
  %"&pSB[currWI].offset378.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)377.i"
  %CastToValueType379.i = bitcast i8* %"&pSB[currWI].offset378.i" to i32*
  %loadedValue380.i = load i32* %CastToValueType379.i, align 4
  %218 = lshr i32 %loadedValue380.i, 2
  %219 = add i32 %217, %218
  %220 = zext i32 %219 to i64
  %221 = getelementptr inbounds i32 addrspace(3)* %19, i64 %220
  %"&(pSB[currWI].offset)408.i" = add nuw i64 %CurrSBIndex..6.i, 272
  %"&pSB[currWI].offset409.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)408.i"
  %CastToValueType410.i = bitcast i8* %"&pSB[currWI].offset409.i" to i32 addrspace(3)**
  store i32 addrspace(3)* %221, i32 addrspace(3)** %CastToValueType410.i, align 8
  %"&(pSB[currWI].offset)221.i" = add nuw i64 %CurrSBIndex..6.i, 144
  %"&pSB[currWI].offset222.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)221.i"
  %CastToValueType223.i = bitcast i8* %"&pSB[currWI].offset222.i" to i32*
  %loadedValue224.i = load i32* %CastToValueType223.i, align 4
  store i32 %loadedValue224.i, i32 addrspace(3)* %221, align 4
  %222 = and i32 %203, 3
  %223 = mul i32 %222, %52
  %224 = lshr i32 %203, 2
  %225 = add i32 %223, %224
  %226 = zext i32 %225 to i64
  %227 = getelementptr inbounds i32 addrspace(3)* %19, i64 %226
  %"&(pSB[currWI].offset)417.i" = add nuw i64 %CurrSBIndex..6.i, 280
  %"&pSB[currWI].offset418.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)417.i"
  %CastToValueType419.i = bitcast i8* %"&pSB[currWI].offset418.i" to i32 addrspace(3)**
  store i32 addrspace(3)* %227, i32 addrspace(3)** %CastToValueType419.i, align 8
  %"&(pSB[currWI].offset)239.i" = add nuw i64 %CurrSBIndex..6.i, 152
  %"&pSB[currWI].offset240.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)239.i"
  %CastToValueType241.i = bitcast i8* %"&pSB[currWI].offset240.i" to i32*
  %loadedValue242.i = load i32* %CastToValueType241.i, align 4
  store i32 %loadedValue242.i, i32 addrspace(3)* %227, align 4
  %loadedCurrWI483.i = load i64* %40, align 8
  %check.WI.iter484.i = icmp ult i64 %loadedCurrWI483.i, %34
  br i1 %check.WI.iter484.i, label %thenBB481.i, label %elseBB482.i

thenBB481.i:                                      ; preds = %"Barrier BB13.i"
  %"CurrWI++485.i" = add nuw i64 %loadedCurrWI483.i, 1
  store i64 %"CurrWI++485.i", i64* %40, align 8
  %"loadedCurrSB+Stride487.i" = add nuw i64 %CurrSBIndex..6.i, 560
  br label %SyncBB.i

elseBB482.i:                                      ; preds = %"Barrier BB13.i"
  store i64 0, i64* %40, align 8
  br label %SyncBB466.i

SyncBB466.i:                                      ; preds = %thenBB474.i, %elseBB482.i
  %CurrSBIndex..5.i = phi i64 [ 0, %elseBB482.i ], [ %"loadedCurrSB+Stride480.i", %thenBB474.i ]
  %"&(pSB[currWI].offset)74.i" = add nuw i64 %CurrSBIndex..5.i, 56
  %"&pSB[currWI].offset75.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)74.i"
  %CastToValueType76.i = bitcast i8* %"&pSB[currWI].offset75.i" to i32 addrspace(3)**
  %loadedValue77.i = load i32 addrspace(3)** %CastToValueType76.i, align 8
  %228 = load i32 addrspace(3)* %loadedValue77.i, align 4
  %229 = insertelement <4 x i32> undef, i32 %228, i32 0
  %"&(pSB[currWI].offset)88.i" = add nuw i64 %CurrSBIndex..5.i, 64
  %"&pSB[currWI].offset89.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)88.i"
  %CastToValueType90.i = bitcast i8* %"&pSB[currWI].offset89.i" to i32 addrspace(3)**
  %loadedValue91.i = load i32 addrspace(3)** %CastToValueType90.i, align 8
  %230 = load i32 addrspace(3)* %loadedValue91.i, align 4
  %231 = insertelement <4 x i32> %229, i32 %230, i32 1
  %"&(pSB[currWI].offset)102.i" = add nuw i64 %CurrSBIndex..5.i, 72
  %"&pSB[currWI].offset103.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)102.i"
  %CastToValueType104.i = bitcast i8* %"&pSB[currWI].offset103.i" to i32 addrspace(3)**
  %loadedValue105.i = load i32 addrspace(3)** %CastToValueType104.i, align 8
  %232 = load i32 addrspace(3)* %loadedValue105.i, align 4
  %233 = insertelement <4 x i32> %231, i32 %232, i32 2
  %"&(pSB[currWI].offset)116.i" = add nuw i64 %CurrSBIndex..5.i, 80
  %"&pSB[currWI].offset117.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)116.i"
  %CastToValueType118.i = bitcast i8* %"&pSB[currWI].offset117.i" to i32 addrspace(3)**
  %loadedValue119.i = load i32 addrspace(3)** %CastToValueType118.i, align 8
  %234 = load i32 addrspace(3)* %loadedValue119.i, align 4
  %235 = insertelement <4 x i32> %233, i32 %234, i32 3
  %"&(pSB[currWI].offset)426.i" = add nuw i64 %CurrSBIndex..5.i, 288
  %"&pSB[currWI].offset427.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)426.i"
  %CastToValueType428.i = bitcast i8* %"&pSB[currWI].offset427.i" to <4 x i32>*
  store <4 x i32> %235, <4 x i32>* %CastToValueType428.i, align 16
  %loadedCurrWI476.i = load i64* %40, align 8
  %check.WI.iter477.i = icmp ult i64 %loadedCurrWI476.i, %34
  br i1 %check.WI.iter477.i, label %thenBB474.i, label %elseBB475.i

thenBB474.i:                                      ; preds = %SyncBB466.i
  %"CurrWI++478.i" = add nuw i64 %loadedCurrWI476.i, 1
  store i64 %"CurrWI++478.i", i64* %40, align 8
  %"loadedCurrSB+Stride480.i" = add nuw i64 %CurrSBIndex..5.i, 560
  br label %SyncBB466.i

elseBB475.i:                                      ; preds = %SyncBB466.i
  store i64 0, i64* %40, align 8
  br label %SyncBB465.i

SyncBB465.i:                                      ; preds = %thenBB488.i, %elseBB475.i
  %CurrSBIndex..7.i = phi i64 [ 0, %elseBB475.i ], [ %"loadedCurrSB+Stride494.i", %thenBB488.i ]
  %"&(pSB[currWI].offset)161.i" = add nuw i64 %CurrSBIndex..7.i, 112
  %"&pSB[currWI].offset162.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)161.i"
  %CastToValueType163.i = bitcast i8* %"&pSB[currWI].offset162.i" to <4 x i32>*
  %loadedValue164.i = load <4 x i32>* %CastToValueType163.i, align 16
  %236 = extractelement <4 x i32> %loadedValue164.i, i32 0
  %"&(pSB[currWI].offset)394.i" = add nuw i64 %CurrSBIndex..7.i, 256
  %"&pSB[currWI].offset395.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)394.i"
  %CastToValueType396.i = bitcast i8* %"&pSB[currWI].offset395.i" to i32 addrspace(3)**
  %loadedValue397.i = load i32 addrspace(3)** %CastToValueType396.i, align 8
  store i32 %236, i32 addrspace(3)* %loadedValue397.i, align 4
  %"&(pSB[currWI].offset)166.i" = add nuw i64 %CurrSBIndex..7.i, 112
  %"&pSB[currWI].offset167.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)166.i"
  %CastToValueType168.i = bitcast i8* %"&pSB[currWI].offset167.i" to <4 x i32>*
  %loadedValue169.i = load <4 x i32>* %CastToValueType168.i, align 16
  %237 = extractelement <4 x i32> %loadedValue169.i, i32 1
  %"&(pSB[currWI].offset)403.i" = add nuw i64 %CurrSBIndex..7.i, 264
  %"&pSB[currWI].offset404.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)403.i"
  %CastToValueType405.i = bitcast i8* %"&pSB[currWI].offset404.i" to i32 addrspace(3)**
  %loadedValue406.i = load i32 addrspace(3)** %CastToValueType405.i, align 8
  store i32 %237, i32 addrspace(3)* %loadedValue406.i, align 4
  %"&(pSB[currWI].offset)171.i" = add nuw i64 %CurrSBIndex..7.i, 112
  %"&pSB[currWI].offset172.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)171.i"
  %CastToValueType173.i = bitcast i8* %"&pSB[currWI].offset172.i" to <4 x i32>*
  %loadedValue174.i = load <4 x i32>* %CastToValueType173.i, align 16
  %238 = extractelement <4 x i32> %loadedValue174.i, i32 2
  %"&(pSB[currWI].offset)412.i" = add nuw i64 %CurrSBIndex..7.i, 272
  %"&pSB[currWI].offset413.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)412.i"
  %CastToValueType414.i = bitcast i8* %"&pSB[currWI].offset413.i" to i32 addrspace(3)**
  %loadedValue415.i = load i32 addrspace(3)** %CastToValueType414.i, align 8
  store i32 %238, i32 addrspace(3)* %loadedValue415.i, align 4
  %"&(pSB[currWI].offset)176.i" = add nuw i64 %CurrSBIndex..7.i, 112
  %"&pSB[currWI].offset177.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)176.i"
  %CastToValueType178.i = bitcast i8* %"&pSB[currWI].offset177.i" to <4 x i32>*
  %loadedValue179.i = load <4 x i32>* %CastToValueType178.i, align 16
  %239 = extractelement <4 x i32> %loadedValue179.i, i32 3
  %"&(pSB[currWI].offset)421.i" = add nuw i64 %CurrSBIndex..7.i, 280
  %"&pSB[currWI].offset422.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)421.i"
  %CastToValueType423.i = bitcast i8* %"&pSB[currWI].offset422.i" to i32 addrspace(3)**
  %loadedValue424.i = load i32 addrspace(3)** %CastToValueType423.i, align 8
  store i32 %239, i32 addrspace(3)* %loadedValue424.i, align 4
  %loadedCurrWI490.i = load i64* %40, align 8
  %check.WI.iter491.i = icmp ult i64 %loadedCurrWI490.i, %34
  br i1 %check.WI.iter491.i, label %thenBB488.i, label %elseBB489.i

thenBB488.i:                                      ; preds = %SyncBB465.i
  %"CurrWI++492.i" = add nuw i64 %loadedCurrWI490.i, 1
  store i64 %"CurrWI++492.i", i64* %40, align 8
  %"loadedCurrSB+Stride494.i" = add nuw i64 %CurrSBIndex..7.i, 560
  br label %SyncBB465.i

elseBB489.i:                                      ; preds = %SyncBB465.i
  store i64 0, i64* %40, align 8
  br label %SyncBB467.i

SyncBB467.i:                                      ; preds = %thenBB495.i, %elseBB489.i
  %CurrSBIndex..8.i = phi i64 [ 0, %elseBB489.i ], [ %"loadedCurrSB+Stride501.i", %thenBB495.i ]
  %"&(pSB[currWI].offset)69.i" = add nuw i64 %CurrSBIndex..8.i, 56
  %"&pSB[currWI].offset70.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)69.i"
  %CastToValueType71.i = bitcast i8* %"&pSB[currWI].offset70.i" to i32 addrspace(3)**
  %loadedValue72.i = load i32 addrspace(3)** %CastToValueType71.i, align 8
  %240 = load i32 addrspace(3)* %loadedValue72.i, align 4
  %241 = insertelement <4 x i32> undef, i32 %240, i32 0
  %"&(pSB[currWI].offset)83.i" = add nuw i64 %CurrSBIndex..8.i, 64
  %"&pSB[currWI].offset84.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)83.i"
  %CastToValueType85.i = bitcast i8* %"&pSB[currWI].offset84.i" to i32 addrspace(3)**
  %loadedValue86.i = load i32 addrspace(3)** %CastToValueType85.i, align 8
  %242 = load i32 addrspace(3)* %loadedValue86.i, align 4
  %243 = insertelement <4 x i32> %241, i32 %242, i32 1
  %"&(pSB[currWI].offset)97.i" = add nuw i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset98.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)97.i"
  %CastToValueType99.i = bitcast i8* %"&pSB[currWI].offset98.i" to i32 addrspace(3)**
  %loadedValue100.i = load i32 addrspace(3)** %CastToValueType99.i, align 8
  %244 = load i32 addrspace(3)* %loadedValue100.i, align 4
  %245 = insertelement <4 x i32> %243, i32 %244, i32 2
  %"&(pSB[currWI].offset)111.i" = add nuw i64 %CurrSBIndex..8.i, 80
  %"&pSB[currWI].offset112.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)111.i"
  %CastToValueType113.i = bitcast i8* %"&pSB[currWI].offset112.i" to i32 addrspace(3)**
  %loadedValue114.i = load i32 addrspace(3)** %CastToValueType113.i, align 8
  %246 = load i32 addrspace(3)* %loadedValue114.i, align 4
  %247 = insertelement <4 x i32> %245, i32 %246, i32 3
  %"&(pSB[currWI].offset)440.i" = add nuw i64 %CurrSBIndex..8.i, 304
  %"&pSB[currWI].offset441.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)440.i"
  %CastToValueType442.i = bitcast i8* %"&pSB[currWI].offset441.i" to <4 x i32>*
  store <4 x i32> %247, <4 x i32>* %CastToValueType442.i, align 16
  %loadedCurrWI497.i = load i64* %40, align 8
  %check.WI.iter498.i = icmp ult i64 %loadedCurrWI497.i, %34
  br i1 %check.WI.iter498.i, label %thenBB495.i, label %elseBB496.i

thenBB495.i:                                      ; preds = %SyncBB467.i
  %"CurrWI++499.i" = add nuw i64 %loadedCurrWI497.i, 1
  store i64 %"CurrWI++499.i", i64* %40, align 8
  %"loadedCurrSB+Stride501.i" = add nuw i64 %CurrSBIndex..8.i, 560
  br label %SyncBB467.i

elseBB496.i:                                      ; preds = %SyncBB467.i
  store i64 0, i64* %40, align 8
  br label %SyncBB468.i

SyncBB468.i:                                      ; preds = %thenBB502.i, %elseBB496.i, %thenBB509.i
  %currBarrier.0.i = phi i32 [ %currBarrier.1.i, %thenBB509.i ], [ %currBarrier.2.i, %thenBB502.i ], [ 7, %elseBB496.i ]
  %CurrSBIndex..1.i = phi i64 [ %"loadedCurrSB+Stride515.i", %thenBB509.i ], [ %"loadedCurrSB+Stride508.i", %thenBB502.i ], [ 0, %elseBB496.i ]
  %"&(pSB[currWI].offset)152.i" = add nuw i64 %CurrSBIndex..1.i, 100
  %"&pSB[currWI].offset153.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)152.i"
  %CastToValueType154.i = bitcast i8* %"&pSB[currWI].offset153.i" to i32*
  %loadedValue155.i = load i32* %CastToValueType154.i, align 4
  %indvar.next.i = add i32 %loadedValue155.i, 1
  %"&(pSB[currWI].offset)454.i" = add nuw i64 %CurrSBIndex..1.i, 320
  %"&pSB[currWI].offset455.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)454.i"
  %CastToValueType456.i = bitcast i8* %"&pSB[currWI].offset455.i" to i32*
  store i32 %indvar.next.i, i32* %CastToValueType456.i, align 4
  %exitcond.i = icmp eq i32 %indvar.next.i, %1
  %"&(pSB[currWI].offset)430.i" = add nuw i64 %CurrSBIndex..1.i, 288
  %"&pSB[currWI].offset431.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)430.i"
  %CastToValueType432.i = bitcast i8* %"&pSB[currWI].offset431.i" to <4 x i32>*
  %loadedValue433.i = load <4 x i32>* %CastToValueType432.i, align 16
  %"&(pSB[currWI].offset)435.i" = add nuw i64 %CurrSBIndex..1.i, 288
  %"&(pSB[currWI].offset)444.i" = add nuw i64 %CurrSBIndex..1.i, 304
  %"&pSB[currWI].offset445.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)444.i"
  %CastToValueType446.i = bitcast i8* %"&pSB[currWI].offset445.i" to <4 x i32>*
  %loadedValue447.i = load <4 x i32>* %CastToValueType446.i, align 16
  %"&(pSB[currWI].offset)449.i" = add nuw i64 %CurrSBIndex..1.i, 304
  br i1 %exitcond.i, label %._crit_edge.i, label %"Barrier BB17.i"

._crit_edge.i:                                    ; preds = %SyncBB468.i, %SyncBB471.i
  %currBarrier.2.i = phi i32 [ %currBarrier.0.i, %SyncBB468.i ], [ %currBarrier.3.i, %SyncBB471.i ]
  %CurrSBIndex..3.i = phi i64 [ %CurrSBIndex..1.i, %SyncBB468.i ], [ %CurrSBIndex..4.i, %SyncBB471.i ]
  %value.0.lcssa.i = phi <4 x i32> [ %loadedValue447.i, %SyncBB468.i ], [ %57, %SyncBB471.i ]
  %key.0.lcssa.i = phi <4 x i32> [ %loadedValue433.i, %SyncBB468.i ], [ %55, %SyncBB471.i ]
  %"&pSB[currWI].offset25.i" = getelementptr inbounds i8* %37, i64 %CurrSBIndex..3.i
  %CastToValueType26.i = bitcast i8* %"&pSB[currWI].offset25.i" to i64*
  %loadedValue27.i = load i64* %CastToValueType26.i, align 8
  %248 = getelementptr inbounds <4 x i32> addrspace(1)* %7, i64 %loadedValue27.i
  store <4 x i32> %key.0.lcssa.i, <4 x i32> addrspace(1)* %248, align 16
  %"&pSB[currWI].offset21.i" = getelementptr inbounds i8* %37, i64 %CurrSBIndex..3.i
  %CastToValueType22.i = bitcast i8* %"&pSB[currWI].offset21.i" to i64*
  %loadedValue.i = load i64* %CastToValueType22.i, align 8
  %249 = getelementptr inbounds <4 x i32> addrspace(1)* %10, i64 %loadedValue.i
  store <4 x i32> %value.0.lcssa.i, <4 x i32> addrspace(1)* %249, align 16
  %loadedCurrWI504.i = load i64* %40, align 8
  %check.WI.iter505.i = icmp ult i64 %loadedCurrWI504.i, %34
  br i1 %check.WI.iter505.i, label %thenBB502.i, label %__radixSortBlocks_separated_args.exit

thenBB502.i:                                      ; preds = %._crit_edge.i
  %"CurrWI++506.i" = add nuw i64 %loadedCurrWI504.i, 1
  store i64 %"CurrWI++506.i", i64* %40, align 8
  %"loadedCurrSB+Stride508.i" = add nuw i64 %CurrSBIndex..3.i, 560
  %cond.i = icmp eq i32 %currBarrier.2.i, 65
  br i1 %cond.i, label %SyncBB471.i, label %SyncBB468.i

__radixSortBlocks_separated_args.exit:            ; preds = %._crit_edge.i
  store i64 0, i64* %40, align 8
  ret void
}

!opencl.kernels = !{!0, !2, !3, !4, !5}
!opencl_radixSortBlocks_locals_anchor = !{!6}
!opencl_findRadixOffsets_locals_anchor = !{!7}
!opencl_reorderData_locals_anchor = !{!8, !9, !10, !11}
!opencl_addUniform_locals_anchor = !{!12}
!opencl_scan_locals_anchor = !{!13}

!0 = metadata !{void (i32, i32, <4 x i32> addrspace(1)*, <4 x i32> addrspace(1)*, <4 x i32> addrspace(1)*, <4 x i32> addrspace(1)*, i32 addrspace(3)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__radixSortBlocks_separated_args, metadata !1, metadata !1, metadata !"", metadata !"uint, uint, uint4 __attribute__((address_space(1))) *, uint4 __attribute__((address_space(1))) *, uint4 __attribute__((address_space(1))) *, uint4 __attribute__((address_space(1))) *, uint __attribute__((address_space(3))) *", metadata !"opencl_radixSortBlocks_locals_anchor", void (i8*)* @radixSortBlocks}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (<2 x i32> addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, i32, i32, i32 addrspace(3)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__findRadixOffsets_separated_args, metadata !1, metadata !1, metadata !"", metadata !"uint2 __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, uint, uint, uint, uint __attribute__((address_space(3))) *", metadata !"opencl_findRadixOffsets_locals_anchor", void (i8*)* @findRadixOffsets}
!3 = metadata !{void (i32, i32 addrspace(1)*, i32 addrspace(1)*, <2 x i32> addrspace(1)*, <2 x i32> addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__reorderData_separated_args, metadata !1, metadata !1, metadata !"", metadata !"uint, uint __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, uint2 __attribute__((address_space(1))) *, uint2 __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, uint", metadata !"opencl_reorderData_locals_anchor", void (i8*)* @reorderData}
!4 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__addUniform_separated_args, metadata !1, metadata !1, metadata !"", metadata !"uint __attribute__((address_space(1))) *, uint const __attribute__((address_space(1))) *, int const", metadata !"opencl_addUniform_locals_anchor", void (i8*)* @addUniform}
!5 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, i32, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__scan_separated_args, metadata !1, metadata !1, metadata !"", metadata !"uint __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, int const, int const, int const", metadata !"opencl_scan_locals_anchor", void (i8*)* @scan}
!6 = metadata !{metadata !"opencl_radixSortBlocks_local_numtrue"}
!7 = metadata !{metadata !"opencl_findRadixOffsets_local_sStartPointers"}
!8 = metadata !{metadata !"opencl_reorderData_local_sKeys2"}
!9 = metadata !{metadata !"opencl_reorderData_local_sValues2"}
!10 = metadata !{metadata !"opencl_reorderData_local_sOffsets"}
!11 = metadata !{metadata !"opencl_reorderData_local_sBlockOffsets"}
!12 = metadata !{metadata !"opencl_addUniform_local_uni"}
!13 = metadata !{metadata !"opencl_scan_local_s_data"}
