; RUN: llvm-as %p/WGBuiltins64.ll -o %t.WGBuiltins64.bc
; RUN: opt -runtimelib=%t.WGBuiltins64.bc -B-ValueAnalysis -B-BarrierAnalysis -B-SplitOnBarrier -B-Barrier -verify -S < %s | FileCheck %s

;;*****************************************************************************
; This test checks the Barrier pass
;;  - The case is a kernel with a work_group_barrier(uint, int) call
;;  - It checks that this call is removed after the Barrier pass
;;*****************************************************************************
;; __kernel void build_hash_table() {
;;   int done = 0;
;;   while(!work_group_all(done)) {
;;     done = 1;
;;     work_group_barrier(CLK_LOCAL_MEM_FENCE, memory_scope_work_group);
;;   }
;; }

;; CHECK-NOT: call {{.*}}work_group_barrier

; ModuleID = 'work_group_barrier2.cl'
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

; Function Attrs: nounwind
define spir_kernel void @build_hash_table() #0 {
entry:
  %done = alloca i32, align 4
  store i32 0, i32* %done, align 4
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %0 = load i32, i32* %done, align 4
  %call = call spir_func i32 @_Z14work_group_alli(i32 %0)
  %tobool = icmp ne i32 %call, 0
  %lnot = xor i1 %tobool, true
  br i1 %lnot, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  store i32 1, i32* %done, align 4
  call spir_func void @_Z18work_group_barrierji(i32 1, i32 1)
  br label %while.cond

while.end:                                        ; preds = %while.cond
  ret void
}

declare spir_func i32 @_Z14work_group_alli(i32) #1

declare spir_func void @_Z18work_group_barrierji(i32, i32) #1

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}
!llvm.ident = !{!9}

!0 = !{void ()* @build_hash_table}
!6 = !{i32 1, i32 2}
!7 = !{i32 2, i32 0}
!8 = !{}
!9 = !{!"clang version 3.6.2 "}
