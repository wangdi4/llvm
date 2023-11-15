; INTEL_FEATURE_SW_ADVANCED
; Checks that when functions are cloned opt-report messages are propagated to
; cloned functions. This test is based on ip_cloning_2.ll.

; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -passes='module(post-inline-ip-cloning),intel-ir-optreport-emitter' -ip-cloning-loop-heuristic -intel-opt-report-file=stdout -disable-output | FileCheck %s

@F_1 = external local_unnamed_addr global [100 x i32], align 16

define void @foo() local_unnamed_addr {
entry:
  tail call fastcc void @bar(i32 10)
  tail call fastcc void @bar(i32 20)
  ret void
}

; CHECK: Global optimization report for : bar

; CHECK: LOOP BEGIN
; CHECK:     remark #99999: Dummy remark for testing
; CHECK: LOOP END

; CHECK: Global optimization report for : bar.1

; CHECK: LOOP BEGIN
; CHECK:     remark #99999: Dummy remark for testing
; CHECK: LOOP END

; CHECK: Global optimization report for : bar.2

; CHECK: LOOP BEGIN
; CHECK:     remark #99999: Dummy remark for testing
; CHECK: LOOP END

define internal fastcc void @bar(i32 %ub) unnamed_addr {
entry:
  %add = add i32 %ub, 20
  %cmp6 = icmp eq i32 %add, 0
  br i1 %cmp6, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i32 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @F_1, i32 0, i32 %indvars.iv
  %i = load i32, ptr %arrayidx, align 4
  %add1 = add i32 %i, %indvars.iv
  store i32 %add1, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond = icmp eq i32 %indvars.iv.next, %add
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !0

for.end:                                          ; preds = %for.body, %entry
  ret void
}

!0 = distinct !{!0, !1}
!1 = distinct !{!"intel.optreport", !2}
!2 = !{!"intel.optreport.remarks", !3}
!3 = !{!"intel.optreport.remark", i32 99999}
; end INTEL_FEATURE_SW_ADVANCED
