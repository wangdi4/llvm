; RUN: opt -passes="loop-vectorize,intel-ir-optreport-emitter" -intel-opt-report=high -intel-opt-report-file=stdout -disable-output < %s | FileCheck %s
; Check that optreport metadata node is not propagated to remainder loop

; CHECK:      Global optimization report for : vect

; CHECK:      LOOP BEGIN
; CHECK-NEXT:     remark #99999: Dummy remark for testing
; CHECK-NEXT: LOOP END

; CHECK:      LOOP BEGIN
; CHECK-NEXT: LOOP END

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = external global [255 x i32]

; Function Attrs: nounwind readonly uwtable
define i32 @vect() {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %red.05 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds [255 x i32], ptr @a, i64 0, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %0, %red.05
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 255
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !1

for.end:                                          ; preds = %for.body
  ret i32 %add
}

!1 = distinct !{!1, !2, !3, !4}
!2 = !{!"DummyMetadata1"}
!3 = !{!"DummyMetadata2"}
!4 = distinct !{!"intel.optreport", !6}
!6 = !{!"intel.optreport.remarks", !7}
!7 = !{!"intel.optreport.remark", i32 99999}

