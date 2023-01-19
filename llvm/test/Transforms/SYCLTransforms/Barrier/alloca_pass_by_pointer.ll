; This test checks cross-barrier function argument is correctly handled when
; callee function has a barrier and is not inlined.
; Before this patch, only dst[0] has correct result in -g -cl-opt-disable mode.
;
; The IR is dumped at the beginning of BarrierPass::runOnModule() from source:
;
; void process(global int *dst, local int *x, int *lid) {
;   x[*lid] = 1;
;   barrier(CLK_LOCAL_MEM_FENCE);
;   dst[*lid] = x[0];
; }
; kernel void test(global int *dst, local int *x) {
;   int lid;
;   lid = get_local_id(0);
;   process(dst, x, &lid);
; }
;
; RUN: opt -enable-native-debug=true  -passes=dpcpp-kernel-barrier %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-barrier %s -S | FileCheck %s
;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind
define void @process(i32 addrspace(1)* noalias %dst, i32 addrspace(3)* noalias %x, i32* noalias %lid) #0 {
entry:
; CHECK-LABEL: entry:
; CHECK: %dst.addr.addr = alloca i32 addrspace(1)**
; CHECK: %x.addr.addr = alloca i32 addrspace(3)**
; CHECK: %lid.addr.addr = alloca i32**
  call void @dummy_barrier.()
  %dst.addr = alloca i32 addrspace(1)*, align 8
  %x.addr = alloca i32 addrspace(3)*, align 8
  %lid.addr = alloca i32*, align 8
  store i32 addrspace(1)* %dst, i32 addrspace(1)** %dst.addr, align 8
  store i32 addrspace(3)* %x, i32 addrspace(3)** %x.addr, align 8
  store i32* %lid, i32** %lid.addr, align 8
  %0 = load i32 addrspace(3)*, i32 addrspace(3)** %x.addr, align 8
  %1 = load i32*, i32** %lid.addr, align 8
  %2 = load i32, i32* %1, align 4
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds i32, i32 addrspace(3)* %0, i64 %idxprom
  store i32 1, i32 addrspace(3)* %arrayidx, align 4
  br label %"Barrier BB1"

; CHECK-LABEL: SyncBB2:
; CHECK: [[SBIndex0:%SBIndex[0-9]+]] = load i64, i64* %pCurrSBIndex
; CHECK-NEXT: [[Offset0:%SB_LocalId_Offset[0-9]+]] = add nuw i64 [[SBIndex0]], {{[0-9]+}}
; CHECK-NEXT: [[GEP0:%[a-zA-Z0-9]+]] = getelementptr inbounds i8, i8* %pSB, i64 [[Offset0]]
; CHECK-NEXT: [[LocalId0:%pSB_LocalId[0-9]+]] = bitcast i8* [[GEP0]] to i32**
; CHECK-NEXT: store i32** [[LocalId0]], i32*** %lid.addr.addr
; CHECK-NEXT: [[L0:%[0-9]+]] = load i32**, i32*** %lid.addr.addr
; CHECK: store i32* %loadedValue{{[0-9]+}}, i32** [[L0]], align 8
; CHECK: %{{[0-9]+}} = load i32*, i32** [[L0]], align 8

"Barrier BB1":                                    ; preds = %entry
  call void @_Z18work_group_barrierj(i32 1) #4
  %3 = load i32 addrspace(3)*, i32 addrspace(3)** %x.addr, align 8
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(3)* %3, i64 0
  %4 = load i32, i32 addrspace(3)* %arrayidx1, align 4
  %5 = load i32 addrspace(1)*, i32 addrspace(1)** %dst.addr, align 8
  %6 = load i32*, i32** %lid.addr, align 8
  %7 = load i32, i32* %6, align 4
  %idxprom2 = sext i32 %7 to i64
  %arrayidx3 = getelementptr inbounds i32, i32 addrspace(1)* %5, i64 %idxprom2
  store i32 %4, i32 addrspace(1)* %arrayidx3, align 4
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %"Barrier BB1"
  call void @_Z18work_group_barrierj(i32 1)
  ret void
}

; Function Attrs: convergent
declare void @_Z18work_group_barrierj(i32) #1

; Function Attrs: convergent noinline norecurse nounwind
define void @test(i32 addrspace(1)* noalias %dst, i32 addrspace(3)* noalias %x) #2 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 !kernel_arg_name !11 !kernel_execution_length !12 !no_barrier_path !13 !kernel_has_global_sync !13 {
entry:
; CHECK-LABEL: entry:
; CHECK: %dst.addr.addr = alloca i32 addrspace(1)**
; CHECK: %x.addr.addr = alloca i32 addrspace(3)**
; CHECK: %lid.addr = alloca i32*
  call void @dummy_barrier.()
  %dst.addr = alloca i32 addrspace(1)*, align 8
  %x.addr = alloca i32 addrspace(3)*, align 8
  %lid = alloca i32, align 4
  store i32 addrspace(1)* %dst, i32 addrspace(1)** %dst.addr, align 8
  store i32 addrspace(3)* %x, i32 addrspace(3)** %x.addr, align 8
  %call = call i64 @_Z12get_local_idj(i32 0) #5
  %conv = trunc i64 %call to i32
  store i32 %conv, i32* %lid, align 4
  %0 = load i32 addrspace(1)*, i32 addrspace(1)** %dst.addr, align 8
  %1 = load i32 addrspace(3)*, i32 addrspace(3)** %x.addr, align 8
  br label %"Barrier BB1"

; CHECK-LABEL: SyncBB2:
; CHECK: [[L1:%[0-9]+]] = load i32*, i32** %lid.addr
; CHECK: [[LocalId0:%LocalId_[0-9]+]] = load i64, i64* %pLocalId_0
; CHECK-NEXT: %conv = trunc i64 [[LocalId0]] to i32
; CHECK-NEXT: store i32 %conv, i32* [[L1]], align 4

"Barrier BB1":                                    ; preds = %entry
  call void @_Z18work_group_barrierj(i32 1)

; CHECK-LABEL: SyncBB1:
; CHECK: store i32* %pSB_LocalId{{[0-9]+}}, i32** %lid.addr
; CHECK-NEXT: [[L2:%[0-9]+]] = load i32*, i32** %lid.addr
; CHECK: call void @process(i32 addrspace(1)* %loadedValue{{[0-9]+}}, i32 addrspace(3)* %loadedValue{{[0-9]+}}, i32* [[L2]]) #5

  call void @process(i32 addrspace(1)* %0, i32 addrspace(3)* %1, i32* %lid) #6
  br label %"Barrier BB2"

"Barrier BB2":                                    ; preds = %"Barrier BB1"
  call void @dummy_barrier.()
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %"Barrier BB2"
  call void @_Z18work_group_barrierj(i32 1)
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z12get_local_idj(i32) #3

declare void @dummy_barrier.()

attributes #0 = { convergent noinline norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent noinline norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { convergent "kernel-call-once" "kernel-convergent-call" }
attributes #5 = { convergent nounwind readnone }
attributes #6 = { convergent }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!sycl.kernels = !{!4}

!0 = !{i32 1, i32 2}
!1 = !{}
!2 = !{!"-cl-opt-disable"}
!3 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!4 = !{void (i32 addrspace(1)*, i32 addrspace(3)*)* @test}
!5 = !{i32 1, i32 3}
!6 = !{!"none", !"none"}
!7 = !{!"int*", !"int*"}
!8 = !{!"", !""}
!9 = !{i1 false, i1 false}
!10 = !{i32 0, i32 0}
!11 = !{!"dst", !"x"}
!12 = !{i32 12}
!13 = !{i1 false}

;; barrier key values
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function process -- %pCurrBarrier = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function process -- %pCurrSBIndex = alloca i64, align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function process -- %pLocalIds = alloca [3 x i64], align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function process -- %pSB = call i8* @get_special_buffer.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function process -- %LocalSize_0 = call i64 @_Z14get_local_sizej(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function process -- %LocalSize_1 = call i64 @_Z14get_local_sizej(i32 1)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function process -- %LocalSize_2 = call i64 @_Z14get_local_sizej(i32 2)
;; arguments
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function process -- %loadedValue = load i32 addrspace(1)*, i32 addrspace(1)** %pSB_LocalId, align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function process -- %loadedValue4 = load i32 addrspace(3)*, i32 addrspace(3)** %pSB_LocalId3, align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function process -- %loadedValue8 = load i32*, i32** %pSB_LocalId7, align 8
;; barrier key values
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pCurrBarrier = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pCurrSBIndex = alloca i64, align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pLocalIds = alloca [3 x i64], align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pSB = call i8* @get_special_buffer.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %LocalSize_0 = call i64 @_Z14get_local_sizej(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %LocalSize_1 = call i64 @_Z14get_local_sizej(i32 1)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %LocalSize_2 = call i64 @_Z14get_local_sizej(i32 2)

; DEBUGIFY-NOT: WARNING
