; Test checks that PrepareKernelArgs Pass correctly replaces function pointers
; to block invoke kernels in enqueue kernel and kernel query built-in functions
; by pointers to wrapper kernels.

; LLVM IR is optained by dumping 'enqueue_simple_block' test (from
; Conformance 2.0 Device Execution suite) in the beginning of the
; PrepareKernelArgs Pass.

; RUN: %oclopt -prepare-kernel-args -S < %s | FileCheck %s

; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.ndrange_t.1 = type { i32, [3 x i64], [3 x i64], [3 x i64] }
%opencl.queue_t.0 = type opaque

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #0

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1) #0

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #0

; Function Attrs: convergent nounwind
define void @enqueue_simple_block(i32 addrspace(1)* noalias %res, i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* noalias %pWorkDim, i64* noalias %pWGId, [4 x i64] %BaseGlbId, i8* noalias %pSpecialBuf, {}* noalias %RuntimeHandle) local_unnamed_addr #1 !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_base_type !9 !kernel_arg_type_qual !10 !kernel_arg_host_accessible !11 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !10 !kernel_arg_buffer_location !10 !kernel_arg_name !12 !no_barrier_path !11 !kernel_execution_length !13 !kernel_has_barrier !11 !kernel_has_global_sync !11 !barrier_buffer_size !14 !private_memory_size !14 !local_buffer_size !6 {
entry:
  %0 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 5
  %1 = bitcast {}** %0 to i8**
  %RuntimeInterface9 = load i8*, i8** %1, align 8
  %2 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 6
  %3 = bitcast {}** %2 to i8**
  %Block2KernelMapper10 = load i8*, i8** %3, align 8
  %pSB_LocalId334 = alloca %struct.ndrange_t.1, align 8
  %BaseGlobalID_0 = extractvalue [4 x i64] %BaseGlbId, 0
  %4 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 4, i64 0
  %NumGroups_0 = load i64, i64* %4, align 8
  %GroupID_0 = load i64, i64* %pWGId, align 8
  %5 = add nsw i64 %GroupID_0, 1
  %6 = icmp eq i64 %NumGroups_0, %5
  %7 = zext i1 %6 to i64
  %8 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 3, i64 %7, i64 0
  %LocalSize_01 = load i64, i64* %8, align 8
  %9 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 4, i64 1
  %NumGroups_1 = load i64, i64* %9, align 8
  %10 = getelementptr i64, i64* %pWGId, i64 1
  %GroupID_1 = load i64, i64* %10, align 8
  %11 = add nsw i64 %GroupID_1, 1
  %12 = icmp eq i64 %NumGroups_1, %11
  %13 = zext i1 %12 to i64
  %14 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 3, i64 %13, i64 1
  %LocalSize_12 = load i64, i64* %14, align 8
  %15 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 4, i64 2
  %NumGroups_2 = load i64, i64* %15, align 8
  %16 = getelementptr i64, i64* %pWGId, i64 2
  %GroupID_2 = load i64, i64* %16, align 8
  %17 = add nsw i64 %GroupID_2, 1
  %18 = icmp eq i64 %NumGroups_2, %17
  %19 = zext i1 %18 to i64
  %20 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 3, i64 %19, i64 2
  %LocalSize_23 = load i64, i64* %20, align 8
  %21 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 5
  %22 = bitcast {}** %21 to i8**
  %RuntimeInterface78 = load i8*, i8** %22, align 8
  %23 = addrspacecast i8* %RuntimeInterface78 to i8 addrspace(4)*
  %call1.i = tail call %opencl.queue_t.0* @ocl20_get_default_queue(i8 addrspace(4)* %23) #5
  br label %SyncBB1

SyncBB1:                                          ; preds = %Dispatch, %entry
  %24 = phi i64 [ 0, %entry ], [ %60, %Dispatch ]
  %25 = phi i64 [ 0, %entry ], [ %61, %Dispatch ]
  %LocalId_03839 = phi i64 [ 0, %entry ], [ %LocalId_03840, %Dispatch ]
  %pCurrSBIndex.0 = phi i64 [ 0, %entry ], [ %62, %Dispatch ]
  %GlobalID_0 = add i64 %LocalId_03839, %BaseGlobalID_0
  %26 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %pCurrSBIndex.0
  %block.size = bitcast i8* %26 to i32*
  store i32 28, i32* %block.size, align 8
  %27 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %pCurrSBIndex.0
  %block.align = getelementptr inbounds i8, i8* %27, i64 4
  %28 = bitcast i8* %block.align to i32*
  store i32 8, i32* %28, align 4
  %29 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %pCurrSBIndex.0
  %block.captured = getelementptr inbounds i8, i8* %29, i64 8
  %30 = bitcast i8* %block.captured to i64*
  store i64 %GlobalID_0, i64* %30, align 8, !tbaa !15
  %31 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %pCurrSBIndex.0
  %block.captured1 = getelementptr inbounds i8, i8* %31, i64 24
  %32 = bitcast i8* %block.captured1 to i32*
  store i32 3, i32* %32, align 8, !tbaa !19
  %33 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %pCurrSBIndex.0
  %block.captured2 = getelementptr inbounds i8, i8* %33, i64 16
  %34 = bitcast i8* %block.captured2 to i32 addrspace(1)**
  store i32 addrspace(1)* %res, i32 addrspace(1)** %34, align 8, !tbaa !21
  %35 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %pCurrSBIndex.0
  %36 = bitcast i8* %35 to void ()*
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %GlobalID_0
  store i32 -1, i32 addrspace(1)* %arrayidx, align 4, !tbaa !19
  %SB_LocalId_Offset29 = or i64 %pCurrSBIndex.0, 32
  %37 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %SB_LocalId_Offset29
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %37) #2
  %SB_LocalId_Offset26 = or i64 %pCurrSBIndex.0, 32
  %38 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %SB_LocalId_Offset26
  %T.sroa.0.0..sroa_idx.i.i = bitcast i8* %38 to i32*
  store i32 1, i32* %T.sroa.0.0..sroa_idx.i.i, align 8, !alias.scope !23
  %T.sroa.46.0..sroa_idx7.i.i = getelementptr inbounds i8, i8* %38, i64 8
  %39 = bitcast i8* %T.sroa.46.0..sroa_idx7.i.i to i64*
  store i64 0, i64* %39, align 8, !alias.scope !23
  %T.sroa.512.0..sroa_idx13.i.i = getelementptr inbounds i8, i8* %38, i64 32
  %40 = bitcast i8* %T.sroa.512.0..sroa_idx13.i.i to i64*
  store i64 1, i64* %40, align 8, !alias.scope !23
  %T.sroa.618.0..sroa_idx19.i.i = getelementptr inbounds i8, i8* %38, i64 56
  %41 = bitcast i8* %T.sroa.618.0..sroa_idx19.i.i to i64*
  store i64 0, i64* %41, align 8, !alias.scope !23
  %SB_LocalId_Offset35 = add nuw i64 %pCurrSBIndex.0, 112
  %42 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %SB_LocalId_Offset35
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 8 %42, i8* nonnull align 8 %37, i64 80, i1 false), !tbaa.struct !28
  %43 = bitcast void ()* %36 to i8*
  %44 = addrspacecast i8* %43 to i8 addrspace(4)*
  %SB_LocalId_Offset32 = add nuw i64 %pCurrSBIndex.0, 112
  %45 = getelementptr inbounds i8, i8* %pSpecialBuf, i64 %SB_LocalId_Offset32
  %46 = bitcast %struct.ndrange_t.1* %pSB_LocalId334 to i8*
  call void @llvm.lifetime.start.p0i8(i64 80, i8* %46)
  %47 = bitcast %struct.ndrange_t.1* %pSB_LocalId334 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %47, i8* nonnull align 1 %45, i64 80, i1 false) #2
  %48 = addrspacecast i8* %RuntimeInterface9 to i8 addrspace(4)*
  %49 = addrspacecast i8* %Block2KernelMapper10 to i8 addrspace(4)*
  %50 = bitcast {}* %RuntimeHandle to i8*
  %51 = addrspacecast i8* %50 to i8 addrspace(4)*
  %52 = addrspacecast %struct.ndrange_t.1* %pSB_LocalId334 to %struct.ndrange_t.1 addrspace(4)*

; CHECK: [[InvokeKer:%[0-9]+]] = addrspacecast void (i8*, i64*, {}*)* @__enqueue_simple_block_block_invoke_kernel to i8 addrspace(4)*
; CHECK: call i32 @ocl20_enqueue_kernel_basic(%opencl.queue_t.0* {{.*}}, i32 1, %struct.ndrange_t.1 addrspace(4)* {{.*}}, i8 addrspace(4)* [[InvokeKer]]

  %call3.i = call i32 @ocl20_enqueue_kernel_basic(%opencl.queue_t.0* %call1.i, i32 1, %struct.ndrange_t.1 addrspace(4)* %52, i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @__enqueue_simple_block_block_invoke_kernel to i8*) to i8 addrspace(4)*), i8 addrspace(4)* %44, i8 addrspace(4)* %48, i8 addrspace(4)* %49, i8 addrspace(4)* %51) #6
  %53 = bitcast %struct.ndrange_t.1* %pSB_LocalId334 to i8*
  call void @llvm.lifetime.end.p0i8(i64 80, i8* %53)
  %cmp = icmp eq i32 %call3.i, 0
  br i1 %cmp, label %cleanup, label %if.then

if.then:                                          ; preds = %SyncBB1
  store i32 -1, i32 addrspace(1)* %arrayidx, align 4, !tbaa !19
  br label %cleanup

cleanup:                                          ; preds = %SyncBB1, %if.then
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %37) #2
  %54 = add nuw i64 %LocalId_03839, 1
  %55 = icmp ult i64 %54, %LocalSize_01
  br i1 %55, label %Dispatch, label %LoopEnd_0

LoopEnd_0:                                        ; preds = %cleanup
  %56 = add nuw i64 %25, 1
  %57 = icmp ult i64 %56, %LocalSize_12
  br i1 %57, label %Dispatch, label %LoopEnd_1

LoopEnd_1:                                        ; preds = %LoopEnd_0
  %58 = add nuw i64 %24, 1
  %59 = icmp ult i64 %58, %LocalSize_23
  br i1 %59, label %Dispatch, label %SyncBB0

Dispatch:                                         ; preds = %LoopEnd_1, %LoopEnd_0, %cleanup
  %60 = phi i64 [ %58, %LoopEnd_1 ], [ %24, %LoopEnd_0 ], [ %24, %cleanup ]
  %61 = phi i64 [ 0, %LoopEnd_1 ], [ %56, %LoopEnd_0 ], [ %25, %cleanup ]
  %LocalId_03840 = phi i64 [ 0, %LoopEnd_1 ], [ 0, %LoopEnd_0 ], [ %54, %cleanup ]
  %62 = add nuw i64 %pCurrSBIndex.0, 192
  br label %SyncBB1

SyncBB0:                                          ; preds = %LoopEnd_1
  ret void
}

; CHECK: define void @__enqueue_simple_block_block_invoke_kernel
; CHECK: call void @____enqueue_simple_block_block_invoke_kernel_separated_args

; Function Attrs: nounwind
define dso_local void @__enqueue_simple_block_block_invoke_kernel(i8 addrspace(4)*, i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* noalias %pWorkDim, i64* noalias %pWGId, [4 x i64] %BaseGlbId, i8* noalias %pSpecialBuf, {}* noalias %RuntimeHandle) #2 !block_literal_size !30 !no_barrier_path !11 !kernel_execution_length !31 !kernel_has_barrier !11 !kernel_has_global_sync !11 !max_wg_dimensions !6 !barrier_buffer_size !6 !private_memory_size !6 !local_buffer_size !6 {
entry:
  %block.capture.addr.i = getelementptr inbounds i8, i8 addrspace(4)* %0, i64 8
  %1 = bitcast i8 addrspace(4)* %block.capture.addr.i to i64 addrspace(4)*
  %2 = load i64, i64 addrspace(4)* %1, align 8, !tbaa !15
  %block.capture.addr1.i = getelementptr inbounds i8, i8 addrspace(4)* %0, i64 24
  %3 = bitcast i8 addrspace(4)* %block.capture.addr1.i to i32 addrspace(4)*
  %4 = load i32, i32 addrspace(4)* %3, align 8, !tbaa !19
  %block.capture.addr2.i = getelementptr inbounds i8, i8 addrspace(4)* %0, i64 16
  %5 = bitcast i8 addrspace(4)* %block.capture.addr2.i to i32 addrspace(1)* addrspace(4)*
  %6 = load i32 addrspace(1)*, i32 addrspace(1)* addrspace(4)* %5, align 8, !tbaa !21
  %mul1.i.i = mul nsw i32 %4, 7
  %sub.i.i = add nsw i32 %mul1.i.i, -21
  %arrayidx.i.i = getelementptr inbounds i32, i32 addrspace(1)* %6, i64 %2
  store i32 %sub.i.i, i32 addrspace(1)* %arrayidx.i.i, align 4, !tbaa !19
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i8 addrspace(4)* @__get_device_command_manager() local_unnamed_addr #3

; Function Attrs: convergent nounwind readnone
declare %opencl.queue_t.0* @ocl20_get_default_queue(i8 addrspace(4)*) local_unnamed_addr #3

; Function Attrs: convergent nounwind readnone
declare i8 addrspace(4)* @__get_block_to_kernel_mapper() local_unnamed_addr #3

; Function Attrs: convergent nounwind readnone
declare i8 addrspace(4)* @__get_runtime_handle() local_unnamed_addr #3

; Function Attrs: convergent
declare i32 @ocl20_enqueue_kernel_basic(%opencl.queue_t.0*, i32, %struct.ndrange_t.1 addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)*) local_unnamed_addr #4

attributes #0 = { argmemonly nounwind }
attributes #1 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { convergent nounwind readnone }
attributes #6 = { convergent nounwind }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!opencl.kernels = !{!5}
!opencl.gen_addr_space_pointer_counter = !{!6}
!opencl.global_variable_total_size = !{!6}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!3 = !{!"-cl-std=CL2.0"}
!4 = !{!"clang version 7.0.0"}
!5 = !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @enqueue_simple_block, void (i8 addrspace(4)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64],
[2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)* @__enqueue_simple_block_block_invoke_kernel}
!6 = !{i32 0}
!7 = !{i32 1}
!8 = !{!"none"}
!9 = !{!"int*"}
!10 = !{!""}
!11 = !{i1 false}
!12 = !{!"res"}
!13 = !{i32 32}
!14 = !{i32 192}
!15 = !{!16, !16, i64 0}
!16 = !{!"long", !17, i64 0}
!17 = !{!"omnipotent char", !18, i64 0}
!18 = !{!"Simple C/C++ TBAA"}
!19 = !{!20, !20, i64 0}
!20 = !{!"int", !17, i64 0}
!21 = !{!22, !22, i64 0}
!22 = !{!"any pointer", !17, i64 0}
!23 = !{!24, !26}
!24 = distinct !{!24, !25, !"_Z10ndrange_1Dmmm: %agg.result"}
!25 = distinct !{!25, !"_Z10ndrange_1Dmmm"}
!26 = distinct !{!26, !27, !"_Z10ndrange_1Dm: %agg.result"}
!27 = distinct !{!27, !"_Z10ndrange_1Dm"}
!28 = !{i64 0, i64 4, !19, i64 8, i64 24, !29, i64 32, i64 24, !29, i64 56, i64 24, !29}
!29 = !{!17, !17, i64 0}
!30 = !{i32 28}
!31 = !{i32 14}
