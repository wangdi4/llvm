; RUN: opt -passes=sycl-kernel-barrier -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the Barrier pass
;; The case:
;;     Functions are not inlined. Cross-barrier return value should be saved to
;;     special buffer and caller will load from special buffer.
;;     IR is dumped before BarrierPass from following source with build option
;;     "-cl-opt-disable":
;;         size_t calc() {
;;           size_t gid = get_global_id(0);
;;           barrier(CLK_LOCAL_MEM_FENCE);
;;           return gid;
;;         }
;;         size_t inclusive() {
;;           return calc();
;;         }
;;         kernel void test(global size_t *dst) {
;;           size_t gid = get_global_id(0);
;;           dst[gid] = inclusive();
;;         }
;; The expected result:
;;     * All functions contain no more barrier/dummy_barrier. instructions.
;;     * Function "calc" stores "%w" to offset 24 in the special buffer.
;;     * Function "inclusive" loads from offset 24 in the special buffer to "%z".
;;     * Function "inclusive" stores "%z" to offset 16 in the special buffer.
;;     * Kernel "test" stores "%x" to offset 0 in the special buffer.
;;     * Kernel "test" loads from offset 0 in the special buffer in second loop.
;;     * Kernel "test" loads from offset 16 in the special buffer to "%y".
;;*****************************************************************************

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind
define internal fastcc i64 @calc() unnamed_addr #0 {
entry:
; CHECK: calc
; CHECK-NOT: call void @dummy_barrier.
; CHECK: %SBIndex = load i64, ptr %pCurrSBIndex, align 8
; CHECK-NEXT: %SB_LocalId_Offset = add nuw i64 %SBIndex, 24
; CHECK-NEXT: [[GEP0:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 %SB_LocalId_Offset
; CHECK-NEXT: store i64 %GlobalID_0, ptr [[GEP0]], align 8
; CHECK-NOT: call void @_Z18work_group_barrierj
  call void @dummy_barrier.()
  %w = tail call i64 @_Z13get_global_idj(i32 0) #4
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %entry
  tail call void @_Z18work_group_barrierj(i32 1) #5
  ret i64 %w
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent
declare void @_Z18work_group_barrierj(i32) local_unnamed_addr #2

; Function Attrs: convergent noinline norecurse nounwind
define internal fastcc i64 @inclusive() unnamed_addr #0 {
entry:
; CHECK: inclusive
; CHECK-NOT: call void @dummy_barrier.
; CHECK: %SBIndex = load i64, ptr %pCurrSBIndex, align 8
; CHECK-NEXT: %SB_LocalId_Offset = add nuw i64 %SBIndex, 24
; CHECK-NEXT: [[GEP1:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 %SB_LocalId_Offset
; CHECK-NEXT: %loadedValue = load i64, ptr [[GEP1]], align 8
; CHECK-NEXT: %SB_LocalId_Offset2 = add nuw i64 %SBIndex, 16
; CHECK-NEXT: [[GEP2:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 %SB_LocalId_Offset2
; CHECK-NEXT: store i64 %loadedValue, ptr [[GEP2]], align 8
; CHECK-NOT: call void @_Z18work_group_barrierj
  call void @dummy_barrier.()
  br label %"Barrier BB1"

"Barrier BB1":                                    ; preds = %entry
  call void @_Z18work_group_barrierj(i32 1)
  %z = tail call fastcc i64 @calc() #6
  br label %"Barrier BB2"

"Barrier BB2":                                    ; preds = %"Barrier BB1"
  call void @dummy_barrier.()
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %"Barrier BB2"
  call void @_Z18work_group_barrierj(i32 1)
  ret i64 %z
}

; Function Attrs: convergent noinline norecurse nounwind
define void @test(ptr addrspace(1) noalias %dst) local_unnamed_addr #3 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !12 !kernel_has_sub_groups !10 !kernel_execution_length !13 !no_barrier_path !10 !kernel_has_global_sync !10 {
entry:
; CHECK-LABEL: define void @test
; CHECK-NOT: call void @dummy_barrier.
; CHECK: %SBIndex = load i64, ptr %pCurrSBIndex, align 8
; CHECK-NEXT: %SB_LocalId_Offset = add nuw i64 %SBIndex, 16
; CHECK-NEXT: [[GEP0:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 %SB_LocalId_Offset
; CHECK-NEXT: %loadedValue = load i64, ptr [[GEP0]], align 8
; CHECK-NEXT: %SB_LocalId_Offset2 = add nuw i64 %SBIndex, 8
; CHECK-NEXT: [[GEP1:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 %SB_LocalId_Offset2
; CHECK-NEXT: store i64 %loadedValue, ptr [[GEP1]], align 8
; CHECK-NEXT: %SB_LocalId_Offset8 = add nuw i64 %SBIndex, 0
; CHECK-NEXT: [[GEP2:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 %SB_LocalId_Offset8
; CHECK-NEXT: %loadedValue10 = load i64, ptr [[GEP2]], align 8
; CHECK-NEXT: %ptridx = getelementptr inbounds i64, ptr addrspace(1) %dst, i64 %loadedValue10
; CHECK-NEXT: %SB_LocalId_Offset12 = add nuw i64 %SBIndex, 8
; CHECK-NEXT: [[GEP3:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 %SB_LocalId_Offset12
; CHECK-NEXT: %loadedValue14 = load i64, ptr [[GEP3]], align 8
; CHECK-NEXT: store i64 %loadedValue14, ptr addrspace(1) %ptridx, align 8
; CHECK-NOT: call void @_Z18work_group_barrierj
  call void @dummy_barrier.()
  %x = tail call i64 @_Z13get_global_idj(i32 0) #4
  br label %"Barrier BB1"

"Barrier BB1":                                    ; preds = %entry
  call void @_Z18work_group_barrierj(i32 1)
  %y = tail call fastcc i64 @inclusive() #6
  br label %"Barrier BB2"

"Barrier BB2":                                    ; preds = %"Barrier BB1"
  call void @dummy_barrier.()
  %ptridx = getelementptr inbounds i64, ptr addrspace(1) %dst, i64 %x
  store i64 %y, ptr addrspace(1) %ptridx, align 8
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %"Barrier BB2"
  call void @_Z18work_group_barrierj(i32 1)
  ret void
}

declare void @dummy_barrier.()

attributes #0 = { convergent noinline norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent noinline norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { convergent nounwind readnone }
attributes #5 = { convergent nounwind "kernel-call-once" "kernel-convergent-call" }
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

!0 = !{i32 2, i32 0}
!1 = !{}
!2 = !{!"-cl-std=CL2.0", !"-cl-opt-disable"}
!3 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!4 = !{ptr @test}
!5 = !{i32 1}
!6 = !{!"none"}
!7 = !{!"size_t*"}
!8 = !{!"ulong*"}
!9 = !{!""}
!10 = !{i1 false}
!11 = !{i32 0}
!12 = !{!"dst"}
!13 = !{i32 5}

;; barrier key values
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function calc -- %BaseGlobalId_0 = call i64 @get_base_global_id.(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function calc -- %pCurrBarrier = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function calc -- %pCurrSBIndex = alloca i64, align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function calc -- %pLocalIds = alloca [3 x i64], align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function calc -- %pSB = call ptr @get_special_buffer.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function calc -- %LocalSize_0 = call i64 @_Z14get_local_sizej(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function calc -- %LocalSize_1 = call i64 @_Z14get_local_sizej(i32 1)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function calc -- %LocalSize_2 = call i64 @_Z14get_local_sizej(i32 2)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pCurrBarrier = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pCurrSBIndex = alloca i64, align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pLocalIds = alloca [3 x i64], align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pSB = call ptr @get_special_buffer.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %LocalSize_0 = call i64 @_Z14get_local_sizej(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %LocalSize_1 = call i64 @_Z14get_local_sizej(i32 1)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %LocalSize_2 = call i64 @_Z14get_local_sizej(i32 2)

; DEBUGIFY-NOT: WARNING
