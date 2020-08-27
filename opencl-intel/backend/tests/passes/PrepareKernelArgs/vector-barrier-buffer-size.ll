; Check that barrier special buffer size is rounded up to multiple of VF in
; vectorized masked kernel.
; This IR is dumped before prepare-kernel-args pass from kernel:
;   #pragma OPENCL EXTENSION cl_intel_subgroups: enable
;   kernel void test(global int* src)
;   {
;     size_t s = get_sub_group_size();
;     barrier(CLK_GLOBAL_MEM_FENCE);
;     src[0] = s;
;   }
;
; RUN: %oclopt -prepare-kernel-args -S < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: nounwind willreturn
declare void @llvm.masked.scatter.v16i32.v16p1i32(<16 x i32>, <16 x i32 addrspace(1)*>, i32 immarg, <16 x i1>) #0

; Function Attrs: convergent norecurse nounwind
define void @test(i32 addrspace(1)* noalias %src, i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* noalias %pWorkDim, i64* noalias %pWGId, [4 x i64] %BaseGlbId, i8* noalias %pSpecialBuf, {}* noalias %RuntimeHandle) local_unnamed_addr #1 !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_base_type !9 !kernel_arg_type_qual !10 !kernel_arg_host_accessible !11 !kernel_arg_pipe_depth !12 !kernel_arg_pipe_io !10 !kernel_arg_buffer_location !10 !kernel_arg_name !13 !vectorized_kernel !14 !vectorized_masked_kernel !15 !no_barrier_path !11 !kernel_has_sub_groups !16 !opencl.stats.Vectorizer.CanVect !7 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !12 !vectorized_width !17 !vectorization_dimension !12 !scalarized_kernel !18 !can_unite_workgroups !11 !kernel_execution_length !19 !kernel_has_barrier !16 !kernel_has_global_sync !11 !barrier_buffer_size !20 !private_memory_size !20 !local_buffer_size !12 {
FirstBB:
; CHECK-LABEL: @test
; CHECK: [[MUL01:%[0-9]+]] = mul nuw nsw i64 %LocalSize_0, %LocalSize_1
; CHECK-NEXT: %LocalSizeProd = mul nuw nsw i64 [[MUL01]], %LocalSize_2
; CHECK-NEXT: [[ADD0:%[0-9]+]] = add nuw nsw i64 %LocalSizeProd, 15
; CHECK-NEXT: %RoundUpToMultipleVF = and i64 [[ADD0]], -16
; CHECK-NEXT: %BarrierBufferSize = mul nuw nsw i64 8, %RoundUpToMultipleVF
  %0 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 4, i64 0
  %NumGroups_0 = load i64, i64* %0, align 1
  %GroupID_0 = load i64, i64* %pWGId, align 1
  %1 = add nsw i64 %GroupID_0, 1
  %2 = icmp eq i64 %NumGroups_0, %1
  %3 = zext i1 %2 to i64
  %4 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 3, i64 %3, i64 0
  %LocalSize_01 = load i64, i64* %4, align 1
  %5 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 4, i64 1
  %NumGroups_1 = load i64, i64* %5, align 1
  %6 = getelementptr i64, i64* %pWGId, i64 1
  %GroupID_1 = load i64, i64* %6, align 1
  %7 = add nsw i64 %GroupID_1, 1
  %8 = icmp eq i64 %NumGroups_1, %7
  %9 = zext i1 %8 to i64
  %10 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 3, i64 %9, i64 1
  %LocalSize_12 = load i64, i64* %10, align 1
  %11 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 4, i64 2
  %NumGroups_2 = load i64, i64* %11, align 1
  %12 = getelementptr i64, i64* %pWGId, i64 2
  %GroupID_2 = load i64, i64* %12, align 1
  %13 = add nsw i64 %GroupID_2, 1
  %14 = icmp eq i64 %NumGroups_2, %13
  %15 = zext i1 %14 to i64
  %16 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 3, i64 %15, i64 2
  %LocalSize_23 = load i64, i64* %16, align 1
  %broadcast.splatinsert2.i = insertelement <16 x i32 addrspace(1)*> undef, i32 addrspace(1)* %src, i32 0
  %broadcast.splat3.i = shufflevector <16 x i32 addrspace(1)*> %broadcast.splatinsert2.i, <16 x i32 addrspace(1)*> undef, <16 x i32> zeroinitializer
  %17 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 4, i64 0
  %NumGroups_04 = load i64, i64* %17, align 1
  %GroupID_05 = load i64, i64* %pWGId, align 1
  %18 = add nsw i64 %GroupID_05, 1
  %19 = icmp eq i64 %NumGroups_04, %18
  %20 = zext i1 %19 to i64
  %21 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 3, i64 %20, i64 0
  %LocalSize_06 = load i64, i64* %21, align 1
  %uniform.id.max = and i64 %LocalSize_06, -16
  %22 = and i64 %LocalSize_06, 15
  %23 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 4, i64 0
  %NumGroups_07 = load i64, i64* %23, align 1
  %GroupID_08 = load i64, i64* %pWGId, align 1
  %24 = add nsw i64 %GroupID_08, 1
  %25 = icmp eq i64 %NumGroups_07, %24
  %26 = zext i1 %25 to i64
  %27 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 3, i64 %26, i64 0
  %LocalSize_09 = load i64, i64* %27, align 1
  br label %SyncBB2.outer

SyncBB2.outer:                                    ; preds = %Dispatch, %FirstBB
  %.lcssa3236 = phi i64 [ %.lcssa3239, %Dispatch ], [ 0, %FirstBB ]
  %28 = phi i64 [ %64, %Dispatch ], [ 0, %FirstBB ]
  %29 = phi i64 [ %65, %Dispatch ], [ 0, %FirstBB ]
  %pCurrBarrier.0.ph = phi i32 [ %pCurrBarrier.2, %Dispatch ], [ 4, %FirstBB ]
  %pCurrSBIndex.0.ph = phi i64 [ %66, %Dispatch ], [ 0, %FirstBB ]
  br label %SyncBB2

SyncBB2:                                          ; preds = %SyncBB2.outer, %Dispatch16
  %30 = phi i64 [ %46, %Dispatch16 ], [ %.lcssa3236, %SyncBB2.outer ]
  %31 = phi i64 [ %47, %Dispatch16 ], [ %28, %SyncBB2.outer ]
  %LocalId_02528 = phi i64 [ %LocalId_02529, %Dispatch16 ], [ %29, %SyncBB2.outer ]
  %pCurrSBIndex.0 = phi i64 [ %48, %Dispatch16 ], [ %pCurrSBIndex.0.ph, %SyncBB2.outer ]
  %32 = icmp ult i64 %LocalId_02528, %uniform.id.max
  %33 = select i1 %32, i64 16, i64 %22
  %34 = insertelement <16 x i64> undef, i64 %33, i32 0
  %35 = shufflevector <16 x i64> %34, <16 x i64> undef, <16 x i32> zeroinitializer
  %mask.bool.i = icmp ugt <16 x i64> %35, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %36 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %pCurrSBIndex.0
  %pSB_LocalId = bitcast i8* %36 to i64*
  store i64 %LocalId_02528, i64* %pSB_LocalId, align 8
  %SB_LocalId_Offset7 = add nuw i64 %pCurrSBIndex.0, 64
  %37 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %SB_LocalId_Offset7
  %pSB_LocalId8 = bitcast i8* %37 to <16 x i32>*
  %ZEXT-i1Toi32 = zext <16 x i1> %mask.bool.i to <16 x i32>
  store <16 x i32> %ZEXT-i1Toi32, <16 x i32>* %pSB_LocalId8, align 64
  %38 = bitcast <16 x i1> %mask.bool.i to i16
  %39 = icmp eq i16 %38, 0
  br i1 %39, label %_ZGVeM16u_test.exit.loopexit, label %pred.call.if.i

pred.call.if.i:                                   ; preds = %SyncBB2
  %40 = add nuw i64 %LocalId_02528, 16
  %41 = icmp ult i64 %40, %LocalSize_01
  br i1 %41, label %Dispatch16, label %LoopEnd_017

LoopEnd_017:                                      ; preds = %pred.call.if.i
  %42 = add nuw i64 %31, 1
  %43 = icmp ult i64 %42, %LocalSize_12
  br i1 %43, label %Dispatch16, label %LoopEnd_119

LoopEnd_119:                                      ; preds = %LoopEnd_017
  %44 = add nuw i64 %30, 1
  %45 = icmp ult i64 %44, %LocalSize_23
  br i1 %45, label %Dispatch16, label %SyncBB1.loopexit

Dispatch16:                                       ; preds = %LoopEnd_119, %LoopEnd_017, %pred.call.if.i
  %46 = phi i64 [ %44, %LoopEnd_119 ], [ %30, %LoopEnd_017 ], [ %30, %pred.call.if.i ]
  %47 = phi i64 [ 0, %LoopEnd_119 ], [ %42, %LoopEnd_017 ], [ %31, %pred.call.if.i ]
  %LocalId_02529 = phi i64 [ 0, %LoopEnd_119 ], [ 0, %LoopEnd_017 ], [ %40, %pred.call.if.i ]
  %48 = add nuw i64 %pCurrSBIndex.0, 128
  br label %SyncBB2

SyncBB1.loopexit:                                 ; preds = %LoopEnd_119
  %LocalSize_09.lcssa10 = phi i64 [ %LocalSize_09, %LoopEnd_119 ]
  br label %SyncBB1

SyncBB1:                                          ; preds = %SyncBB1.loopexit, %Dispatch
  %LocalSize_0911 = phi i64 [ %LocalSize_0912, %Dispatch ], [ %LocalSize_09.lcssa10, %SyncBB1.loopexit ]
  %.lcssa3238 = phi i64 [ %.lcssa3239, %Dispatch ], [ 0, %SyncBB1.loopexit ]
  %49 = phi i64 [ %64, %Dispatch ], [ 0, %SyncBB1.loopexit ]
  %50 = phi i64 [ %65, %Dispatch ], [ 0, %SyncBB1.loopexit ]
  %pCurrBarrier.1 = phi i32 [ %pCurrBarrier.2, %Dispatch ], [ 2, %SyncBB1.loopexit ]
  %pCurrSBIndex.1 = phi i64 [ %66, %Dispatch ], [ 0, %SyncBB1.loopexit ]
  br label %_ZGVeM16u_test.exit

_ZGVeM16u_test.exit.loopexit:                     ; preds = %SyncBB2
  %.lcssa16 = phi i64 [ %30, %SyncBB2 ]
  %.lcssa = phi i64 [ %31, %SyncBB2 ]
  %LocalId_02528.lcssa = phi i64 [ %LocalId_02528, %SyncBB2 ]
  %pCurrSBIndex.0.lcssa = phi i64 [ %pCurrSBIndex.0, %SyncBB2 ]
  %LocalSize_09.lcssa = phi i64 [ %LocalSize_09, %SyncBB2 ]
  br label %_ZGVeM16u_test.exit

_ZGVeM16u_test.exit:                              ; preds = %_ZGVeM16u_test.exit.loopexit, %SyncBB1
  %LocalSize_0912 = phi i64 [ %LocalSize_0911, %SyncBB1 ], [ %LocalSize_09.lcssa, %_ZGVeM16u_test.exit.loopexit ]
  %.lcssa3237 = phi i64 [ %.lcssa3238, %SyncBB1 ], [ %.lcssa16, %_ZGVeM16u_test.exit.loopexit ]
  %51 = phi i64 [ %49, %SyncBB1 ], [ %.lcssa, %_ZGVeM16u_test.exit.loopexit ]
  %52 = phi i64 [ %50, %SyncBB1 ], [ %LocalId_02528.lcssa, %_ZGVeM16u_test.exit.loopexit ]
  %pCurrBarrier.2 = phi i32 [ %pCurrBarrier.1, %SyncBB1 ], [ %pCurrBarrier.0.ph, %_ZGVeM16u_test.exit.loopexit ]
  %pCurrSBIndex.2 = phi i64 [ %pCurrSBIndex.1, %SyncBB1 ], [ %pCurrSBIndex.0.lcssa, %_ZGVeM16u_test.exit.loopexit ]
  %uniform.id.max.i = and i64 %LocalSize_0912, -16
  %53 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %pCurrSBIndex.2
  %pSB_LocalId5 = bitcast i8* %53 to i64*
  %loadedValue = load i64, i64* %pSB_LocalId5, align 8
  %54 = icmp ult i64 %loadedValue, %uniform.id.max.i
  %55 = trunc i64 %LocalSize_0912 to i32
  %56 = and i32 %55, 15
  %subgroup.size.i = select i1 %54, i32 16, i32 %56
  %broadcast.splatinsert.i = insertelement <16 x i32> undef, i32 %subgroup.size.i, i32 0
  %broadcast.splat.i = shufflevector <16 x i32> %broadcast.splatinsert.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %SB_LocalId_Offset10 = add nuw i64 %pCurrSBIndex.2, 64
  %57 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %SB_LocalId_Offset10
  %pSB_LocalId11 = bitcast i8* %57 to <16 x i32>*
  %loadedValue12 = load <16 x i32>, <16 x i32>* %pSB_LocalId11, align 64
  %Trunc-i1Toi32 = trunc <16 x i32> %loadedValue12 to <16 x i1>
  call void @llvm.masked.scatter.v16i32.v16p1i32(<16 x i32> %broadcast.splat.i, <16 x i32 addrspace(1)*> %broadcast.splat3.i, i32 4, <16 x i1> %Trunc-i1Toi32) #2
  %58 = add nuw i64 %52, 16
  %59 = icmp ult i64 %58, %LocalSize_01
  br i1 %59, label %Dispatch, label %LoopEnd_0

LoopEnd_0:                                        ; preds = %_ZGVeM16u_test.exit
  %60 = add nuw i64 %51, 1
  %61 = icmp ult i64 %60, %LocalSize_12
  br i1 %61, label %Dispatch, label %LoopEnd_1

LoopEnd_1:                                        ; preds = %LoopEnd_0
  %62 = add nuw i64 %.lcssa3237, 1
  %63 = icmp ult i64 %62, %LocalSize_23
  br i1 %63, label %Dispatch, label %SyncBB0

Dispatch:                                         ; preds = %LoopEnd_1, %LoopEnd_0, %_ZGVeM16u_test.exit
  %.lcssa3239 = phi i64 [ %62, %LoopEnd_1 ], [ %.lcssa3237, %LoopEnd_0 ], [ %.lcssa3237, %_ZGVeM16u_test.exit ]
  %64 = phi i64 [ 0, %LoopEnd_1 ], [ %60, %LoopEnd_0 ], [ %51, %_ZGVeM16u_test.exit ]
  %65 = phi i64 [ 0, %LoopEnd_1 ], [ 0, %LoopEnd_0 ], [ %58, %_ZGVeM16u_test.exit ]
  %66 = add nuw i64 %pCurrSBIndex.2, 128
  %cond = icmp eq i32 %pCurrBarrier.2, 2
  br i1 %cond, label %SyncBB1, label %SyncBB2.outer

SyncBB0:                                          ; preds = %LoopEnd_1
  ret void
}

; Function Attrs: convergent norecurse nounwind
define void @_ZGVeN16u_test(i32 addrspace(1)* noalias %src, i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* noalias %pWorkDim, i64* noalias %pWGId, [4 x i64] %BaseGlbId, i8* noalias %pSpecialBuf, {}* noalias %RuntimeHandle) local_unnamed_addr #1 !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_base_type !9 !kernel_arg_type_qual !10 !kernel_arg_host_accessible !11 !kernel_arg_pipe_depth !12 !kernel_arg_pipe_io !10 !kernel_arg_buffer_location !10 !kernel_arg_name !13 !vectorized_kernel !18 !no_barrier_path !11 !kernel_has_sub_groups !16 !ocl_recommended_vector_length !17 !opencl.stats.Vectorizer.CanVect !7 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !12 !vectorized_width !17 !vectorization_dimension !12 !scalarized_kernel !15 !can_unite_workgroups !11 !kernel_execution_length !21 !kernel_has_barrier !16 !kernel_has_global_sync !11 !barrier_buffer_size !7 !private_memory_size !7 !local_buffer_size !12 {
entry:
; CHECK-LABEL: @_ZGVeN16u_test
; CHECK: [[MUL01:%[0-9]+]] = mul nuw nsw i64 %LocalSize_0, %LocalSize_1
; CHECK-NEXT: %LocalSizeProd = mul nuw nsw i64 [[MUL01]], %LocalSize_2
; CHECK-NEXT: %BarrierBufferSize = mul nuw nsw i64 1, %LocalSizeProd
  %pLocalIds = alloca [3 x i64], align 8
  %0 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 4, i64 0
  %NumGroups_0 = load i64, i64* %0, align 1
  %GroupID_0 = load i64, i64* %pWGId, align 1
  %1 = add nsw i64 %GroupID_0, 1
  %2 = icmp eq i64 %NumGroups_0, %1
  %3 = zext i1 %2 to i64
  %4 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 3, i64 %3, i64 0
  %LocalSize_01 = load i64, i64* %4, align 1
  %5 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 4, i64 1
  %NumGroups_1 = load i64, i64* %5, align 1
  %6 = getelementptr i64, i64* %pWGId, i64 1
  %GroupID_1 = load i64, i64* %6, align 1
  %7 = add nsw i64 %GroupID_1, 1
  %8 = icmp eq i64 %NumGroups_1, %7
  %9 = zext i1 %8 to i64
  %10 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 3, i64 %9, i64 1
  %LocalSize_12 = load i64, i64* %10, align 1
  %11 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 4, i64 2
  %NumGroups_2 = load i64, i64* %11, align 1
  %12 = getelementptr i64, i64* %pWGId, i64 2
  %GroupID_2 = load i64, i64* %12, align 1
  %13 = add nsw i64 %GroupID_2, 1
  %14 = icmp eq i64 %NumGroups_2, %13
  %15 = zext i1 %14 to i64
  %16 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 3, i64 %15, i64 2
  %LocalSize_23 = load i64, i64* %16, align 1
  %pLocalId_0 = getelementptr inbounds [3 x i64], [3 x i64]* %pLocalIds, i64 0, i64 0
  %pLocalId_1 = getelementptr inbounds [3 x i64], [3 x i64]* %pLocalIds, i64 0, i64 1
  %pLocalId_2 = getelementptr inbounds [3 x i64], [3 x i64]* %pLocalIds, i64 0, i64 2
  store i64 0, i64* %pLocalId_0, align 8
  store i64 0, i64* %pLocalId_1, align 8
  store i64 0, i64* %pLocalId_2, align 8
  %17 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 4, i64 0
  %NumGroups_04 = load i64, i64* %17, align 1
  %GroupID_05 = load i64, i64* %pWGId, align 1
  %18 = add nsw i64 %GroupID_05, 1
  %19 = icmp eq i64 %NumGroups_04, %18
  %20 = zext i1 %19 to i64
  %21 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 3, i64 %20, i64 0
  %LocalSize_06 = load i64, i64* %21, align 1
  %uniform.id.max = and i64 %LocalSize_06, -16
  %22 = trunc i64 %LocalSize_06 to i32
  %23 = and i32 %22, 15
  %pLocalId_0.promoted16 = load i64, i64* %pLocalId_0, align 8
  %pLocalId_1.promoted19 = load i64, i64* %pLocalId_1, align 8
  %pLocalId_2.promoted20 = load i64, i64* %pLocalId_2, align 8
  br label %SyncBB2

SyncBB2:                                          ; preds = %Dispatch7, %entry
  %24 = phi i64 [ %pLocalId_2.promoted20, %entry ], [ %34, %Dispatch7 ]
  %25 = phi i64 [ %pLocalId_1.promoted19, %entry ], [ %35, %Dispatch7 ]
  %LocalId_01517 = phi i64 [ %pLocalId_0.promoted16, %entry ], [ %LocalId_01518, %Dispatch7 ]
  %pCurrSBIndex.0 = phi i64 [ 0, %entry ], [ %36, %Dispatch7 ]
  %26 = icmp ult i64 %LocalId_01517, %uniform.id.max
  %subgroup.size = select i1 %26, i32 16, i32 %23
  %27 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %pCurrSBIndex.0
  %pSB_LocalId = bitcast i8* %27 to i32*
  store i32 %subgroup.size, i32* %pSB_LocalId, align 4
  %28 = add nuw i64 %LocalId_01517, 16
  %29 = icmp ult i64 %28, %LocalSize_01
  br i1 %29, label %Dispatch7, label %LoopEnd_08

LoopEnd_08:                                       ; preds = %SyncBB2
  %30 = add nuw i64 %25, 1
  %31 = icmp ult i64 %30, %LocalSize_12
  br i1 %31, label %Dispatch7, label %LoopEnd_110

LoopEnd_110:                                      ; preds = %LoopEnd_08
  %32 = add nuw i64 %24, 1
  %33 = icmp ult i64 %32, %LocalSize_23
  br i1 %33, label %Dispatch7, label %LoopEnd_212

LoopEnd_212:                                      ; preds = %LoopEnd_110
  store i64 0, i64* %pLocalId_0, align 8
  store i64 0, i64* %pLocalId_1, align 8
  store i64 0, i64* %pLocalId_2, align 8
  br label %SyncBB1

Dispatch7:                                        ; preds = %LoopEnd_110, %LoopEnd_08, %SyncBB2
  %34 = phi i64 [ %32, %LoopEnd_110 ], [ %24, %LoopEnd_08 ], [ %24, %SyncBB2 ]
  %35 = phi i64 [ 0, %LoopEnd_110 ], [ %30, %LoopEnd_08 ], [ %25, %SyncBB2 ]
  %LocalId_01518 = phi i64 [ 0, %LoopEnd_110 ], [ 0, %LoopEnd_08 ], [ %28, %SyncBB2 ]
  %36 = add nuw i64 %pCurrSBIndex.0, 4
  br label %SyncBB2

SyncBB1:                                          ; preds = %LoopEnd_212, %Dispatch
  %37 = phi i64 [ 0, %LoopEnd_212 ], [ %47, %Dispatch ]
  %38 = phi i64 [ 0, %LoopEnd_212 ], [ %48, %Dispatch ]
  %39 = phi i64 [ 0, %LoopEnd_212 ], [ %49, %Dispatch ]
  %pCurrSBIndex.1 = phi i64 [ 0, %LoopEnd_212 ], [ %50, %Dispatch ]
  %40 = add nuw i64 %39, 16
  %41 = icmp ult i64 %40, %LocalSize_01
  br i1 %41, label %Dispatch, label %LoopEnd_0

LoopEnd_0:                                        ; preds = %SyncBB1
  %42 = add nuw i64 %38, 1
  %43 = icmp ult i64 %42, %LocalSize_12
  br i1 %43, label %Dispatch, label %LoopEnd_1

LoopEnd_1:                                        ; preds = %LoopEnd_0
  %44 = add nuw i64 %37, 1
  %45 = icmp ult i64 %44, %LocalSize_23
  br i1 %45, label %Dispatch, label %LoopEnd_2

LoopEnd_2:                                        ; preds = %LoopEnd_1
  %pCurrSBIndex.1.lcssa = phi i64 [ %pCurrSBIndex.1, %LoopEnd_1 ]
  %46 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %pCurrSBIndex.1.lcssa
  %pSB_LocalId5.le = bitcast i8* %46 to i32*
  %loadedValue.le = load i32, i32* %pSB_LocalId5.le, align 4
  store i32 %loadedValue.le, i32 addrspace(1)* %src, align 4
  store i64 0, i64* %pLocalId_0, align 8
  store i64 0, i64* %pLocalId_1, align 8
  store i64 0, i64* %pLocalId_2, align 8
  ret void

Dispatch:                                         ; preds = %LoopEnd_1, %LoopEnd_0, %SyncBB1
  %47 = phi i64 [ %44, %LoopEnd_1 ], [ %37, %LoopEnd_0 ], [ %37, %SyncBB1 ]
  %48 = phi i64 [ 0, %LoopEnd_1 ], [ %42, %LoopEnd_0 ], [ %38, %SyncBB1 ]
  %49 = phi i64 [ 0, %LoopEnd_1 ], [ 0, %LoopEnd_0 ], [ %40, %SyncBB1 ]
  %50 = add nuw i64 %pCurrSBIndex.1, 4
  br label %SyncBB1
}

attributes #0 = { nounwind willreturn }
attributes #1 = { convergent norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVeN16u_test,_ZGVeM16u_test" }
attributes #2 = { nounwind }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!opencl.stats.Vectorizer.CanVect = !{!4}
!opencl.stats.Vectorizer.Chosen_Vectorization_Dim = !{!5}
!opencl.kernels = !{!6}

!0 = !{i32 2, i32 0}
!1 = !{}
!2 = !{!"-cl-std=CL2.0"}
!3 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!4 = !{!"Code is vectorizable"}
!5 = !{!"The chosen vectorization dimension"}
!6 = !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @test}
!7 = !{i32 1}
!8 = !{!"none"}
!9 = !{!"int*"}
!10 = !{!""}
!11 = !{i1 false}
!12 = !{i32 0}
!13 = !{!"src"}
!14 = !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @_ZGVeN16u_test}
!15 = distinct !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @test}
!16 = !{i1 true}
!17 = !{i32 16}
!18 = !{null}
!19 = !{i32 7}
!20 = !{i32 8}
!21 = !{i32 10}
