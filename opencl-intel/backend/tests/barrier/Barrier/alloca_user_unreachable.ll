; This test checks alloca' user is replaced with address in special buffer
; if there is unreachable in the user's basic block.
;
; The IR is dumped at the beginning of BarrierPass::runOnModule() from source:
;
; void foo(int gf) {}
; __kernel void main_kernel()
; {
;   int g = 1;
;   while (false)
;     foo(g);
; }
;
; RUN: %oclopt -B-ValueAnalysis -B-BarrierAnalysis -B-Barrier %s -S | FileCheck %s
;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind
define void @foo(i32 %gf) #0 {
entry:
  %gf.addr = alloca i32, align 4
  store i32 %gf, i32* %gf.addr, align 4
  ret void
}

; Function Attrs: convergent noinline norecurse nounwind
define void @main_kernel() #1 !kernel_arg_addr_space !1 !kernel_arg_access_qual !1 !kernel_arg_type !1 !kernel_arg_base_type !1 !kernel_arg_type_qual !1 !kernel_arg_host_accessible !1 !kernel_arg_pipe_depth !1 !kernel_arg_pipe_io !1 !kernel_arg_buffer_location !1 !kernel_arg_name !1 !kernel_execution_length !5 !kernel_has_barrier !6 !kernel_has_global_sync !6 {
entry:
; CHECK-LABEL: entry:
; CHECK: %g.addr = alloca i32*
; CHECK-NOT: %g = alloca i32

; CHECK-LABEL: SyncBB1:
; CHECK-NEXT: [[SBIndex0:%SBIndex]] = load i64, i64* %pCurrSBIndex
; CHECK-NEXT: [[Offset0:%SB_LocalId_Offset]] = add nuw i64 [[SBIndex0]], {{[0-9]+}}
; CHECK-NEXT: [[GEP0:%[0-9]+]] = getelementptr inbounds i8, i8* %pSB, i64 [[Offset0]]
; CHECK-NEXT: [[LocalId0:%pSB_LocalId]] = bitcast i8* [[GEP0]] to i32*
; CHECK-NEXT: store i32* [[LocalId0]], i32** %g.addr
; CHECK-NEXT: [[L0:%[0-9]+]] = load i32*, i32** %g.addr, align 8
; CHECK-NEXT: store i32 1, i32* [[L0]], align 4

  call void @dummybarrier.()
  %g = alloca i32, align 4
  store i32 1, i32* %g, align 4
  br label %while.cond

while.cond:                                       ; preds = %entry
  br i1 false, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
; CHECK-LABEL: while.body:
; CHECK-NEXT: [[L1:%[0-9]+]] = load i32, i32* [[L0]], align 4
; CHECK-NEXT: call void @foo(i32 [[L1]]) #2

  %0 = load i32, i32* %g, align 4
  call void @foo(i32 %0) #2
  unreachable

while.end:                                        ; preds = %while.cond
  call void @_Z7barrierj(i32 1)
  ret void
}

declare void @dummybarrier.()

; Function Attrs: convergent
declare void @_Z7barrierj(i32) #2

attributes #0 = { convergent noinline norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent noinline norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!opencl.kernels = !{!4}

!0 = !{i32 1, i32 2}
!1 = !{}
!2 = !{!"-cl-opt-disable"}
!3 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!4 = !{void ()* @main_kernel}
!5 = !{i32 8}
!6 = !{i1 false}
