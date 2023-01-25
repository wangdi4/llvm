; sycl-kernel-wg-loop-bound pass hung on following code.
; RUN: opt -passes="sycl-kernel-analysis,sycl-kernel-wg-loop-bound" %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes="sycl-kernel-analysis,sycl-kernel-wg-loop-bound" %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

;; The IR is dumped at the beginning of WGLoopBoundaries::runOnModule()
;; when calling clBuildProgram from the following source.
;; void __kernel test(int a) {
;;   bool b = false;
;;   while (!b) {
;;     if (a)
;;       b = true;
;;   }
;; }

; CHECK: WGLoopBoundaries
; CHECK: found 0 early exit boundaries
; CHECK: found 1 uniform early exit conditions

; Function Attrs: convergent nounwind
define void @test(i32 %a) !no_barrier_path !11 {
entry:
  %tobool1 = icmp eq i32 %a, 0
  br i1 %tobool1, label %while.body.us, label %while.end

while.body.us:                                    ; preds = %entry, %while.body.us
  br label %while.body.us

while.end:                                        ; preds = %entry
  ret void
}


!sycl.kernels = !{!4}

!4 = !{void (i32)* @test}
!11 = !{i1 true}

; DEBUGIFY-COUNT-1: Instruction with empty DebugLoc in function test
; DEBUGIFY-COUNT-16: Instruction with empty DebugLoc in function WG.boundaries.test
; DEBUGIFY-COUNT-1: Missing line
; DEBUGIFY-NOT: WARNING
