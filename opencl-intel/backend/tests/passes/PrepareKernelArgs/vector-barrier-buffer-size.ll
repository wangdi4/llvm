; Check that barrier special buffer size is rounded up to multiple of VF.
; This IR is dumped before prepare-kernel-args pass from kernel:
; kernel void test(global int *dst) {
;   size_t g = get_global_id(0);
;   barrier(CLK_GLOBAL_MEM_FENCE);
;   dst[g] = 0;
; }
;
; RUN: %oclopt -prepare-kernel-args -S < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define void @test(i32 addrspace(1)* noalias %dst, i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* noalias %pWorkDim, i64* noalias %pWGId, [4 x i64] %BaseGlbId, i8* noalias %pSpecialBuf, {}* noalias %RuntimeHandle) local_unnamed_addr #0 !kernel_arg_addr_space !11 !kernel_arg_access_qual !12 !kernel_arg_type !13 !kernel_arg_base_type !13 !kernel_arg_type_qual !14 !kernel_arg_host_accessible !15 !kernel_arg_pipe_depth !16 !kernel_arg_pipe_io !14 !kernel_arg_buffer_location !14 !kernel_arg_name !17 !vectorized_kernel !18 !no_barrier_path !15 !kernel_has_sub_groups !15 !scalarized_kernel !19 !vectorized_width !11 !kernel_execution_length !20 !kernel_has_barrier !21 !kernel_has_global_sync !15 !max_wg_dimensions !11 !barrier_buffer_size !22 !private_memory_size !22 !local_buffer_size !16 {
entry:
  %BaseGlobalID_0 = extractvalue [4 x i64] %BaseGlbId, 0
  %0 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 4, i64 0
  %NumGroups_0 = load i64, i64* %0, align 1
  %GroupID_0 = load i64, i64* %pWGId, align 1
  %1 = add nsw i64 %GroupID_0, 1
  %2 = icmp eq i64 %NumGroups_0, %1
  %3 = zext i1 %2 to i64
  %4 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 3, i64 %3, i64 0
  %LocalSize_01 = load i64, i64* %4, align 1
  br label %SyncBB2

SyncBB2:                                          ; preds = %Dispatch7, %entry
  %LocalId_01113 = phi i64 [ 0, %entry ], [ %6, %Dispatch7 ]
  %pCurrSBIndex.0 = phi i64 [ 0, %entry ], [ %8, %Dispatch7 ]
  %GlobalID_0 = add i64 %LocalId_01113, %BaseGlobalID_0
  %5 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %pCurrSBIndex.0
  %pSB_LocalId = bitcast i8* %5 to i64*
  store i64 %GlobalID_0, i64* %pSB_LocalId, align 8
  %6 = add nuw i64 %LocalId_01113, 1
  %7 = icmp ult i64 %6, %LocalSize_01
  br i1 %7, label %Dispatch7, label %SyncBB1.preheader

SyncBB1.preheader:                                ; preds = %SyncBB2
  br label %SyncBB1

Dispatch7:                                        ; preds = %SyncBB2
  %8 = add nuw i64 %pCurrSBIndex.0, 8
  br label %SyncBB2

SyncBB1:                                          ; preds = %SyncBB1.preheader, %Dispatch
  %9 = phi i64 [ %11, %Dispatch ], [ 0, %SyncBB1.preheader ]
  %pCurrSBIndex.1 = phi i64 [ %13, %Dispatch ], [ 0, %SyncBB1.preheader ]
  %10 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %pCurrSBIndex.1
  %pSB_LocalId5 = bitcast i8* %10 to i64*
  %loadedValue = load i64, i64* %pSB_LocalId5, align 8
  %ptridx = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %loadedValue
  store i32 0, i32 addrspace(1)* %ptridx, align 4, !tbaa !23
  %11 = add nuw i64 %9, 1
  %12 = icmp ult i64 %11, %LocalSize_01
  br i1 %12, label %Dispatch, label %SyncBB0

Dispatch:                                         ; preds = %SyncBB1
  %13 = add nuw i64 %pCurrSBIndex.1, 8
  br label %SyncBB1

SyncBB0:                                          ; preds = %SyncBB1
  ret void
}

; Function Attrs: convergent norecurse nounwind
define void @__Vectorized_.test(i32 addrspace(1)* noalias %dst, i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* noalias %pWorkDim, i64* noalias %pWGId, [4 x i64] %BaseGlbId, i8* noalias %pSpecialBuf, {}* noalias %RuntimeHandle) local_unnamed_addr #0 !kernel_arg_addr_space !11 !kernel_arg_access_qual !12 !kernel_arg_type !13 !kernel_arg_base_type !13 !kernel_arg_type_qual !14 !kernel_arg_host_accessible !15 !kernel_arg_pipe_depth !16 !kernel_arg_pipe_io !14 !kernel_arg_buffer_location !14 !kernel_arg_name !17 !vectorized_kernel !19 !no_barrier_path !15 !kernel_has_sub_groups !15 !scalarized_kernel !27 !opencl.stats.Vectorizer.CanVect !11 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !16 !opencl.stats.predicate.Predicated !16 !opencl.stats.Vectorizer.Wide_Unmasked_Memory_Operation_Created !11 !opencl.stats.Vectorizer.Blocks_That_Are_Better_Vectorized !11 !opencl.stats.Vectorizer.Blocks_That_Are_Better_Scalarized !16 !vectorized_width !28 !vectorization_dimension !16 !can_unite_workgroups !15 !kernel_execution_length !29 !kernel_has_barrier !21 !kernel_has_global_sync !15 !max_wg_dimensions !11 !barrier_buffer_size !11 !private_memory_size !11 !local_buffer_size !16 {
; CHECK: @__Vectorized_.test
; CHECK: [[MUL01:%[0-9]+]] = mul i64 %LocalSize_0, %LocalSize_1
; CHECK-NEXT: %LocalSizeProd = mul i64 [[MUL01]], %LocalSize_2
; CHECK-NEXT: [[ADD0:%[0-9]+]] = add i64 %LocalSizeProd, 15
; CHECK-NEXT: %RoundUpToMultipleVF = and i64 [[ADD0]], -16
; CHECK-NEXT: %BarrierBufferSize = mul i64 1, %RoundUpToMultipleVF
entry:
  %BaseGlobalID_0 = extractvalue [4 x i64] %BaseGlbId, 0
  %0 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 4, i64 0
  %NumGroups_0 = load i64, i64* %0, align 1
  %GroupID_0 = load i64, i64* %pWGId, align 1
  %1 = add nsw i64 %GroupID_0, 1
  %2 = icmp eq i64 %NumGroups_0, %1
  %3 = zext i1 %2 to i64
  %4 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 3, i64 %3, i64 0
  %LocalSize_01 = load i64, i64* %4, align 1
  br label %SyncBB2

SyncBB2:                                          ; preds = %Dispatch21, %entry
  %LocalId_02527 = phi i64 [ 0, %entry ], [ %6, %Dispatch21 ]
  %pCurrSBIndex.0 = phi i64 [ 0, %entry ], [ %8, %Dispatch21 ]
  %GlobalID_0 = add i64 %LocalId_02527, %BaseGlobalID_0
  %5 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %pCurrSBIndex.0
  %pSB_LocalId = bitcast i8* %5 to i64*
  store i64 %GlobalID_0, i64* %pSB_LocalId, align 8
  %6 = add nuw i64 %LocalId_02527, 16
  %7 = icmp ult i64 %6, %LocalSize_01
  br i1 %7, label %Dispatch21, label %SyncBB1.preheader

SyncBB1.preheader:                                ; preds = %SyncBB2
  br label %SyncBB1

Dispatch21:                                       ; preds = %SyncBB2
  %8 = add nuw i64 %pCurrSBIndex.0, 8
  br label %SyncBB2

SyncBB1:                                          ; preds = %SyncBB1.preheader, %Dispatch
  %9 = phi i64 [ %12, %Dispatch ], [ 0, %SyncBB1.preheader ]
  %pCurrSBIndex.1 = phi i64 [ %14, %Dispatch ], [ 0, %SyncBB1.preheader ]
  %10 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %pCurrSBIndex.1
  %pSB_LocalId19 = bitcast i8* %10 to i64*
  %loadedValue = load i64, i64* %pSB_LocalId19, align 8
  %11 = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %loadedValue
  %ptrTypeCast = bitcast i32 addrspace(1)* %11 to <16 x i32> addrspace(1)*
  store <16 x i32> zeroinitializer, <16 x i32> addrspace(1)* %ptrTypeCast, align 4
  %12 = add nuw i64 %9, 16
  %13 = icmp ult i64 %12, %LocalSize_01
  br i1 %13, label %Dispatch, label %SyncBB0

Dispatch:                                         ; preds = %SyncBB1
  %14 = add nuw i64 %pCurrSBIndex.1, 8
  br label %SyncBB1

SyncBB0:                                          ; preds = %SyncBB1
  ret void
}

attributes #0 = { convergent norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }

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
!opencl.stats.predicate.Predicated = !{!6}
!opencl.stats.Vectorizer.Wide_Unmasked_Memory_Operation_Created = !{!7}
!opencl.stats.Vectorizer.Blocks_That_Are_Better_Vectorized = !{!8}
!opencl.stats.Vectorizer.Blocks_That_Are_Better_Scalarized = !{!9}
!opencl.kernels = !{!10}

!0 = !{i32 2, i32 0}
!1 = !{}
!2 = !{!"-cl-std=CL2.0"}
!3 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!4 = !{!"Code is vectorizable"}
!5 = !{!"The chosen vectorization dimension"}
!6 = !{!"one if the function is predicated, zero otherwise"}
!7 = !{!"Created a wide (vector) load / store."}
!8 = !{!"blocks for which the heuristics says it is better to vectorize"}
!9 = !{!"blocks for which the heuristics says it is better to leave scalar version"}
!10 = !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @test}
!11 = !{i32 1}
!12 = !{!"none"}
!13 = !{!"int*"}
!14 = !{!""}
!15 = !{i1 false}
!16 = !{i32 0}
!17 = !{!"dst"}
!18 = !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @__Vectorized_.test}
!19 = !{null}
!20 = !{i32 5}
!21 = !{i1 true}
!22 = !{i32 8}
!23 = !{!24, !24, i64 0}
!24 = !{!"int", !25, i64 0}
!25 = !{!"omnipotent char", !26, i64 0}
!26 = !{!"Simple C/C++ TBAA"}
!27 = distinct !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @test}
!28 = !{i32 16}
!29 = !{i32 6}

