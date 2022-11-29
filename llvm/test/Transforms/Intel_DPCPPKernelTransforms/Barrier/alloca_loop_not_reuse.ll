; This test checks that variable j's address loaded in loop header (for.cond1)
; is not reused in for.inc because there is a barrier ("Barrier BB") between them.
;
; The IR is dumped at the beginning of BarrierPass::runOnModule() from source:
;
; kernel void test() {
;   for (int i = 0; i < 10; i++) {
;     for (int j = 0; j < 5; j++) {
;       if (j < 3) {
;         int pp = 24;
;         barrier(CLK_LOCAL_MEM_FENCE);
;         while (pp > 20) ;
;       }
;     }
;   }
; }
;
; RUN: opt -enable-native-debug=true -passes=dpcpp-kernel-barrier %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-barrier %s -S | FileCheck %s
;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind
define void @test() #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !1 !kernel_arg_type !1 !kernel_arg_base_type !1 !kernel_arg_type_qual !1 !kernel_arg_host_accessible !1 !kernel_arg_pipe_depth !1 !kernel_arg_pipe_io !1 !kernel_arg_buffer_location !1 !kernel_arg_name !1 !kernel_execution_length !5 !kernel_has_barrier !6 !kernel_has_global_sync !7 {
entry:
; CHECK-LABEL: entry:
; CHECK: %j.addr = alloca i32*
; CHECK-NOT: %j = alloca i32
  call void @dummy_barrier.()
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %pp = alloca i32, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc6, %entry
; CHECK-LABEL: for.cond1:
; CHECK: [[Index:%SBIndex[0-9]*]] = load i64, i64* %pCurrSBIndex
; CHECK-NEXT: [[Offset:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[Index]], {{[0-9]+}}
; CHECK-NEXT: [[GEP:%[0-9]+]] = getelementptr inbounds i8, i8* %pSB, i64 [[Offset]]
; CHECK-NEXT: [[LocalId:%pSB_LocalId[0-9]*]] = bitcast i8* [[GEP]] to i32*
; CHECK-NEXT: store i32* [[LocalId]], i32** %j.addr
; CHECK-NEXT: [[J:%[0-9]+]] = load i32*, i32** %j.addr
; CHECK-NEXT: load i32, i32* [[J]], align 4
  %0 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %0, 10
  br i1 %cmp, label %for.body, label %for.end8

for.body:                                         ; preds = %for.cond
  store i32 0, i32* %j, align 4
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc, %for.body
  %1 = load i32, i32* %j, align 4
  %cmp2 = icmp slt i32 %1, 5
  br i1 %cmp2, label %for.body3, label %for.end

for.body3:                                        ; preds = %for.cond1
  %2 = load i32, i32* %j, align 4
  %cmp4 = icmp slt i32 %2, 3
  br i1 %cmp4, label %if.then, label %if.end

if.then:                                          ; preds = %for.body3
  store i32 24, i32* %pp, align 4
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %if.then
  call void @_Z18work_group_barrierj(i32 1) #2
  br label %while.cond

while.cond:                                       ; preds = %while.body, %"Barrier BB"
  %3 = load i32, i32* %pp, align 4
  %cmp5 = icmp sgt i32 %3, 20
  br i1 %cmp5, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  br label %while.cond

while.end:                                        ; preds = %while.cond
  br label %if.end

if.end:                                           ; preds = %while.end, %for.body3
  br label %for.inc

for.inc:                                          ; preds = %if.end
; CHECK-LABEL: for.inc:
; CHECK: [[Index1:%SBIndex[0-9]*]] = load i64, i64* %pCurrSBIndex
; CHECK-NEXT: [[Offset1:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[Index1]], {{[0-9]+}}
; CHECK-NEXT: [[GEP1:%[0-9]+]] = getelementptr inbounds i8, i8* %pSB, i64 [[Offset1]]
; CHECK-NEXT: [[LocalId1:%pSB_LocalId[0-9]*]] = bitcast i8* [[GEP1]] to i32*
; CHECK-NEXT: store i32* [[LocalId1]], i32** %j.addr
; CHECK-NEXT: [[J1:%[0-9]+]] = load i32*, i32** %j.addr
; CHECK-NEXT: load i32, i32* [[J1]], align 4
  %4 = load i32, i32* %j, align 4
  %inc = add nsw i32 %4, 1
  store i32 %inc, i32* %j, align 4
  br label %for.cond1

for.end:                                          ; preds = %for.cond1
  br label %for.inc6

for.inc6:                                         ; preds = %for.end
  %5 = load i32, i32* %i, align 4
  %inc7 = add nsw i32 %5, 1
  store i32 %inc7, i32* %i, align 4
  br label %for.cond

for.end8:                                         ; preds = %for.cond
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
!4 = !{void ()* @test}
!5 = !{i32 5606}
!6 = !{i1 true}
!7 = !{i1 false}

;; barrier key values
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pCurrBarrier = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pCurrSBIndex = alloca i64, align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pLocalIds = alloca [3 x i64], align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pSB = call i8* @get_special_buffer.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %LocalSize_0 = call i64 @_Z14get_local_sizej(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %LocalSize_1 = call i64 @_Z14get_local_sizej(i32 1)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %LocalSize_2 = call i64 @_Z14get_local_sizej(i32 2)

; DEBUGIFY-NOT: WARNING
