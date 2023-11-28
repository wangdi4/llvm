; RUN: opt -passes="loop-unroll,intel-ir-optreport-emitter" -intel-opt-report=low -intel-opt-report-file=stdout -disable-output < %s | FileCheck %s

; Verify that partial unrolling adds the expected opt report remark and does not
; drop any existing remarks.

; CHECK: Global optimization report for : unroll_pragma

; CHECK: LOOP BEGIN
; CHECK:     remark #99999: Dummy remark for testing
; CHECK:     remark #25604: Loop has been partially unrolled with factor 8 by LLVM LoopUnroll
; CHECK: LOOP END

; CHECK: Global optimization report for : unroll_heuristics

; CHECK: LOOP BEGIN
; CHECK:     remark #99999: Dummy remark for testing
; CHECK:     remark #25604: Loop has been partially unrolled with factor 8 by LLVM LoopUnroll
; CHECK: LOOP END

target triple = "x86_64-unknown-linux-gnu"

define void @unroll_pragma(double* %A) {
entry:
  br label %L1

L1:
  %i = phi i64 [ 0, %entry ], [ %i.next, %L1 ]
  %Ai = getelementptr inbounds double, double* %A, i64 %i
  %idouble = uitofp i64 %i to double
  store double %idouble, double* %Ai, align 8
  %i.next = add nuw nsw i64 %i, 1
  %cond = icmp ne i64 %i.next, 1048576
  br i1 %cond, label %L1, label %L1.end, !llvm.loop !0

L1.end:
  ret void
}

define void @unroll_heuristics(double* %A) #0 {
entry:
  br label %L1

L1:
  %i = phi i64 [ 0, %entry ], [ %i.next, %L1 ]
  %Ai = getelementptr inbounds double, double* %A, i64 %i
  %idouble = uitofp i64 %i to double
  store double %idouble, double* %Ai, align 8
  %i.next = add nuw nsw i64 %i, 1
  %cond = icmp ne i64 %i.next, 1048576
  br i1 %cond, label %L1, label %L1.end, !llvm.loop !6

L1.end:
  ret void
}

attributes #0 = { "target-cpu"="skylake-avx512" }

!0 = distinct !{!0, !1, !2}
!1 = !{!"llvm.loop.unroll.count", i32 8}
!2 = distinct !{!"intel.optreport", !4}
!4 = !{!"intel.optreport.remarks", !5}
!5 = !{!"intel.optreport.remark", i32 99999}

!6 = distinct !{!6, !7}
!7 = distinct !{!"intel.optreport", !4}
