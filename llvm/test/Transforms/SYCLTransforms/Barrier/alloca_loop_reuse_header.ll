; This test checks that loop header (for.cond)'s successor (for.end) which is
; not in the loop can reuse address loaded in loop header even if the loop has
; a barrier.
;
; The IR is dumped at the beginning of BarrierPass::runOnModule() from source:
;
; kernel void test() {
;   int i = 170;
;   for (; i < 200;) {
;     i += 30;
;     barrier(CLK_LOCAL_MEM_FENCE);
;   }
;   i = 0;
; }
;
; RUN: opt -passes=sycl-kernel-barrier %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier %s -S | FileCheck %s
;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind
define void @test() #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !1 !kernel_arg_type !1 !kernel_arg_base_type !1 !kernel_arg_type_qual !1 !kernel_arg_host_accessible !1 !kernel_arg_pipe_depth !1 !kernel_arg_pipe_io !1 !kernel_arg_buffer_location !1 !kernel_arg_name !1 !kernel_execution_length !5 !no_barrier_path !6 !kernel_has_global_sync !6 {
entry:
; CHECK-LABEL: entry:
; CHECK: %i.addr = alloca ptr
; CHECK-NOT: %i = alloca i32
  call void @dummy_barrier.()
  %i = alloca i32, align 4
  store i32 170, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %"Barrier BB1", %entry
  %0 = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %0, 200
; CHECK-LABEL: for.cond:
; CHECK-NEXT: [[Index:%SBIndex[0-9]*]] = load i64, ptr %pCurrSBIndex
; CHECK-NEXT: [[Offset:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[Index]], {{[0-9]+}}
; CHECK-NEXT: [[GEP:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[Offset]]
; CHECK-NEXT: store ptr [[GEP]], ptr %i.addr
; CHECK-NEXT: [[I:%[0-9]+]] = load ptr, ptr %i.addr
; CHECK-NEXT: load i32, ptr [[I]], align 4
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, ptr %i, align 4
  %add = add nsw i32 %1, 30
  store i32 %add, ptr %i, align 4
  br label %"Barrier BB1"

"Barrier BB1":                                    ; preds = %for.body
  call void @_Z18work_group_barrierj(i32 1) #2
  br label %for.cond

for.end:                                          ; preds = %for.cond
; CHECK-LABEL: for.end:
; CHECK-NEXT: store i32 0, ptr [[I]], align 4
  store i32 0, ptr %i, align 4
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %for.end
  call void @_Z18work_group_barrierj(i32 1)
  ret void
}

; Function Attrs: convergent
declare void @_Z18work_group_barrierj(i32) #1

declare void @dummy_barrier.()

attributes #0 = { convergent noinline norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent "kernel-call-once" "kernel-convergent-call" }

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
!5 = !{i32 85}
!6 = !{i1 false}

;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pCurrBarrier = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pCurrSBIndex = alloca i64, align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pLocalIds = alloca [3 x i64], align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pSB = call ptr @get_special_buffer.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %LocalSize_0 = call i64 @_Z14get_local_sizej(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %LocalSize_1 = call i64 @_Z14get_local_sizej(i32 1)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %LocalSize_2 = call i64 @_Z14get_local_sizej(i32 2)

; DEBUGIFY-NOT: WARNING
