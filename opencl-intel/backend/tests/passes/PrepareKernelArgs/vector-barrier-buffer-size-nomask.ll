; Check that vectorized kernel's barrier special buffer size is not round up
; to multiple of VF if vectorized masked kernel doesn't exist.
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
define void @test(i32 addrspace(1)* noalias %dst, i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* noalias %pWorkDim, i64* noalias %pWGId, [4 x i64] %BaseGlbId, i8* noalias %pSpecialBuf, {}* noalias %RuntimeHandle) local_unnamed_addr #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 !kernel_arg_host_accessible !8 !kernel_arg_pipe_depth !9 !kernel_arg_pipe_io !7 !kernel_arg_buffer_location !7 !kernel_arg_name !10 !vectorized_kernel !11 !no_barrier_path !8 !kernel_has_sub_groups !8 !scalarized_kernel !12 !vectorized_width !4 !kernel_execution_length !13 !kernel_has_barrier !14 !kernel_has_global_sync !8 !max_wg_dimensions !4 !barrier_buffer_size !15 !private_memory_size !15 !local_buffer_size !9 {
entry:
; CHECK-LABEL: @test
; CHECK: [[MUL01:%[0-9]+]] = mul nuw nsw i64 %LocalSize_0, %LocalSize_1
; CHECK-NEXT: %LocalSizeProd = mul nuw nsw i64 [[MUL01]], %LocalSize_2
; CHECK-NEXT: %BarrierBufferSize = mul nuw nsw i64 8, %LocalSizeProd
  %BaseGlobalID_0 = extractvalue [4 x i64] %BaseGlbId, 0
  %0 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 3, i64 0, i64 0
  %LocalSize_01 = load i64, i64* %0, align 1
  br label %SyncBB2

SyncBB2:                                          ; preds = %Dispatch7, %entry
  %LocalId_01113 = phi i64 [ 0, %entry ], [ %2, %Dispatch7 ]
  %pCurrSBIndex.0 = phi i64 [ 0, %entry ], [ %4, %Dispatch7 ]
  %GlobalID_0 = add i64 %LocalId_01113, %BaseGlobalID_0
  %1 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %pCurrSBIndex.0
  %pSB_LocalId = bitcast i8* %1 to i64*
  store i64 %GlobalID_0, i64* %pSB_LocalId, align 8
  %2 = add nuw i64 %LocalId_01113, 1
  %3 = icmp ult i64 %2, %LocalSize_01
  br i1 %3, label %Dispatch7, label %SyncBB1.preheader

SyncBB1.preheader:                                ; preds = %SyncBB2
  br label %SyncBB1

Dispatch7:                                        ; preds = %SyncBB2
  %4 = add nuw i64 %pCurrSBIndex.0, 8
  br label %SyncBB2

SyncBB1:                                          ; preds = %SyncBB1.preheader, %Dispatch
  %5 = phi i64 [ %7, %Dispatch ], [ 0, %SyncBB1.preheader ]
  %pCurrSBIndex.1 = phi i64 [ %9, %Dispatch ], [ 0, %SyncBB1.preheader ]
  %6 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %pCurrSBIndex.1
  %pSB_LocalId5 = bitcast i8* %6 to i64*
  %loadedValue = load i64, i64* %pSB_LocalId5, align 8
  %ptridx = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %loadedValue
  store i32 0, i32 addrspace(1)* %ptridx, align 4, !tbaa !16
  %7 = add nuw i64 %5, 1
  %8 = icmp ult i64 %7, %LocalSize_01
  br i1 %8, label %Dispatch, label %SyncBB0

Dispatch:                                         ; preds = %SyncBB1
  %9 = add nuw i64 %pCurrSBIndex.1, 8
  br label %SyncBB1

SyncBB0:                                          ; preds = %SyncBB1
  ret void
}

; Function Attrs: convergent norecurse nounwind
define void @__Vectorized_.test(i32 addrspace(1)* noalias %dst, i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* noalias %pWorkDim, i64* noalias %pWGId, [4 x i64] %BaseGlbId, i8* noalias %pSpecialBuf, {}* noalias %RuntimeHandle) local_unnamed_addr #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 !kernel_arg_host_accessible !8 !kernel_arg_pipe_depth !9 !kernel_arg_pipe_io !7 !kernel_arg_buffer_location !7 !kernel_arg_name !10 !vectorized_kernel !12 !no_barrier_path !8 !kernel_has_sub_groups !8 !scalarized_kernel !20 !vectorized_width !21 !vectorization_dimension !9 !can_unite_workgroups !8 !kernel_execution_length !22 !kernel_has_barrier !14 !kernel_has_global_sync !8 !max_wg_dimensions !4 !barrier_buffer_size !4 !private_memory_size !4 !local_buffer_size !9 {
entry:
; CHECK-LABEL: @__Vectorized_.test
; CHECK: [[MUL01:%[0-9]+]] = mul nuw nsw i64 %LocalSize_0, %LocalSize_1
; CHECK-NEXT: %LocalSizeProd = mul nuw nsw i64 [[MUL01]], %LocalSize_2
; CHECK-NEXT: %BarrierBufferSize = mul nuw nsw i64 1, %LocalSizeProd
  %BaseGlobalID_0 = extractvalue [4 x i64] %BaseGlbId, 0
  %0 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 3, i64 0, i64 0
  %LocalSize_01 = load i64, i64* %0, align 1
  br label %SyncBB2

SyncBB2:                                          ; preds = %Dispatch21, %entry
  %LocalId_02527 = phi i64 [ 0, %entry ], [ %2, %Dispatch21 ]
  %pCurrSBIndex.0 = phi i64 [ 0, %entry ], [ %4, %Dispatch21 ]
  %GlobalID_0 = add i64 %LocalId_02527, %BaseGlobalID_0
  %1 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %pCurrSBIndex.0
  %pSB_LocalId = bitcast i8* %1 to i64*
  store i64 %GlobalID_0, i64* %pSB_LocalId, align 8
  %2 = add nuw i64 %LocalId_02527, 16
  %3 = icmp ult i64 %2, %LocalSize_01
  br i1 %3, label %Dispatch21, label %SyncBB1.preheader

SyncBB1.preheader:                                ; preds = %SyncBB2
  br label %SyncBB1

Dispatch21:                                       ; preds = %SyncBB2
  %4 = add nuw i64 %pCurrSBIndex.0, 8
  br label %SyncBB2

SyncBB1:                                          ; preds = %SyncBB1.preheader, %Dispatch
  %5 = phi i64 [ %8, %Dispatch ], [ 0, %SyncBB1.preheader ]
  %pCurrSBIndex.1 = phi i64 [ %10, %Dispatch ], [ 0, %SyncBB1.preheader ]
  %6 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %pCurrSBIndex.1
  %pSB_LocalId19 = bitcast i8* %6 to i64*
  %loadedValue = load i64, i64* %pSB_LocalId19, align 8
  %7 = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %loadedValue
  %ptrTypeCast = bitcast i32 addrspace(1)* %7 to <16 x i32> addrspace(1)*
  store <16 x i32> zeroinitializer, <16 x i32> addrspace(1)* %ptrTypeCast, align 4
  %8 = add nuw i64 %5, 16
  %9 = icmp ult i64 %8, %LocalSize_01
  br i1 %9, label %Dispatch, label %SyncBB0

Dispatch:                                         ; preds = %SyncBB1
  %10 = add nuw i64 %pCurrSBIndex.1, 8
  br label %SyncBB1

SyncBB0:                                          ; preds = %SyncBB1
  ret void
}

attributes #0 = { convergent norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!1}
!llvm.ident = !{!2}
!opencl.kernels = !{!3}

!0 = !{i32 1, i32 2}
!1 = !{}
!2 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!3 = !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @test}
!4 = !{i32 1}
!5 = !{!"none"}
!6 = !{!"int*"}
!7 = !{!""}
!8 = !{i1 false}
!9 = !{i32 0}
!10 = !{!"dst"}
!11 = !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @__Vectorized_.test}
!12 = !{null}
!13 = !{i32 5}
!14 = !{i1 true}
!15 = !{i32 8}
!16 = !{!17, !17, i64 0}
!17 = !{!"int", !18, i64 0}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C/C++ TBAA"}
!20 = distinct !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @test}
!21 = !{i32 16}
!22 = !{i32 6}
