; Test to verify that IR OptReportEmitter is able to handle opt-report
; emitted at function-level. For example, remarks added in the format
; ORBuilder(*F).addRemark(Verbosity, RemarkID);

; RUN: opt -passes="intel-ir-optreport-emitter" -disable-output 2>&1 < %s | FileCheck %s --strict-whitespace

; CHECK-LABEL: Global optimization report for : foo
; CHECK-EMPTY:
; CHECK-NEXT: FUNCTION REPORT BEGIN
; CHECK-NEXT:     remark #99999: Dummy remark for testing
; CHECK-NEXT: FUNCTION REPORT END
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN
; CHECK-NEXT:     remark #15301: SIMD LOOP WAS VECTORIZED
; CHECK-NEXT:     remark #15305: vectorization support: vector length 4
; CHECK-NEXT: LOOP END
; CHECK-NEXT: =================================================================

define void @foo(i64* %A) local_unnamed_addr !intel.optreport !0 {
entry:
  br label %VPlannedBB3

VPlannedBB3:                                      ; preds = %entry
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %VPlannedBB3
  %uni.phi = phi i64 [ 0, %VPlannedBB3 ], [ %uni.phi.add, %vector.body ]
  %vec.phi = phi <4 x i64> [ <i64 0, i64 1, i64 2, i64 3>, %VPlannedBB3 ], [ %vec.phi.add, %vector.body ]
  %ptr = getelementptr inbounds i64, i64* %A, i64 %uni.phi
  %bc = bitcast i64* %ptr to <4 x i64>*
  store <4 x i64> %vec.phi, <4 x i64>* %bc
  %vec.phi.add = add nuw nsw <4 x i64> %vec.phi, <i64 4, i64 4, i64 4, i64 4>
  %uni.phi.add = add nuw nsw i64 %uni.phi, 4
  %cmp = icmp uge i64 %uni.phi.add, 300
  br i1 %cmp, label %loop.exit, label %vector.body, !llvm.loop !5

loop.exit:                                        ; preds = %final.merge
  ret void
}

!0 = distinct !{!"intel.optreport", !2, !3}
!2 = !{!"intel.optreport.title", !"FUNCTION REPORT"}
!3 = !{!"intel.optreport.remarks", !4}
!4 = !{!"intel.optreport.remark", i32 99999}
!5 = distinct !{!5, !6, !11}
!6 = distinct !{!"intel.optreport", !8}
!8 = !{!"intel.optreport.remarks", !9, !10}
!9 = !{!"intel.optreport.remark", i32 15301}
!10 = !{!"intel.optreport.remark", i32 15305, !"4"}
!11 = !{!"llvm.loop.isvectorized", i32 1}
