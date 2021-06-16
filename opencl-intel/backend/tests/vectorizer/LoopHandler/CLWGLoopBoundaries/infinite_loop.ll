; cl-loop-bound pass hung on following code.
; RUN: %oclopt -analyze -kernel-analysis -cl-loop-bound -verify %s -S -o - | FileCheck %s

;; The IR is dumped at the beginning of CLWGLoopBoundaries::runOnModule()
;; when calling clBuildProgram from the following source.
;; void __kernel test(int a) {
;;   bool b = false;
;;   while (!b) {
;;     if (a)
;;       b = true;
;;   }
;; }

; CHECK: CLWGLoopBoundaries
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


!opencl.kernels = !{!4}

!4 = !{void (i32)* @test}
!11 = !{i1 true}