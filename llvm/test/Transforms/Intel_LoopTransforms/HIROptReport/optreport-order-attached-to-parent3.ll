; Check the proper optreport order (structure and metadata) for loop interchange with manually attached optreport as a next_sibling (Complete Unrolled loop) of inner loop.

;void foo(int **restrict A, int N) {
;
;  for (int j = 0; j < N; ++j){
;    for (int i = 0; i < N; ++i) {
;      A[i][j] = j+i;
;    }
;  }
;  return;
;}

; RUN: opt -hir-ssa-deconstruction -hir-loop-interchange -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace

; OPTREPORT: LOOP BEGIN
; OPTREPORT-NEXT:     remark #25444: Loopnest Interchanged: ( 1 2 ) --> ( 2 1 ){{[[:space:]]}}
; OPTREPORT-NEXT:     LOOP BEGIN
; OPTREPORT-NEXT:     LOOP END{{[[:space:]]}}
; OPTREPORT-NEXT:     LOOP BEGIN
; OPTREPORT-NEXT:         remark #25436: Loop completely unrolled by 6
; OPTREPORT-NEXT:     LOOP END
; OPTREPORT-NEXT: LOOP END

; RUN: opt -hir-ssa-deconstruction -hir-loop-interchange -hir-cg -intel-opt-report=low -simplifycfg < %s -S | FileCheck %s

; CHECK: [[M1:!.*]] = distinct !{[[M1]], [[M2:!.*]]}
; CHECK: [[M2]] = distinct !{!"intel.optreport.rootnode", [[M3:!.*]]}
; CHECK: [[M3]] = distinct !{!"intel.optreport", [[M4:!.*]]}
; CHECK: [[M4]] = !{!"intel.optreport.next_sibling", [[M5:!.*]]}
; CHECK: [[M5]] = distinct !{!"intel.optreport.rootnode", [[M6:!.*]]}
; CHECK: [[M6]] = distinct !{!"intel.optreport", [[M7:!.*]]}
; CHECK: [[M7]] = !{!"intel.optreport.remarks", [[M8:!.*]]}
; CHECK: [[M8]] = !{!"intel.optreport.remark", i32 25436, !"Loop completely unrolled by %d", i32 6}
; CHECK: [[M9:!.*]] = distinct !{[[M9]], [[M10:!.*]]}
; CHECK: [[M10]] = distinct !{!"intel.optreport.rootnode", [[M11:!.*]]}
; CHECK: [[M11]] = distinct !{!"intel.optreport", [[M12:!.*]]}
; CHECK: [[M12]] = !{!"intel.optreport.remarks", [[M13:!.*]]}
; CHECK: [[M13]] = !{!"intel.optreport.remark", i32 25444, !"Loopnest Interchanged: %s", !"( 1 2 ) --> ( 2 1 )"}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32** noalias nocapture readonly %A, i32 %N) local_unnamed_addr #0 {
  %cmp12 = icmp sgt i32 %N, 0
  br i1 %cmp12, label %.lr.ph16.split.us, label %._crit_edge17

.lr.ph16.split.us:                                ; preds = %0
  %wide.trip.count = sext i32 %N to i64
  br label %.lr.ph.us

.lr.ph.us:                                        ; preds = %._crit_edge.us, %.lr.ph16.split.us
  %indvars.iv19 = phi i64 [ %indvars.iv.next20, %._crit_edge.us ], [ 0, %.lr.ph16.split.us ]
  br label %1

; <label>:1:                                      ; preds = %1, %.lr.ph.us
  %indvars.iv = phi i64 [ 0, %.lr.ph.us ], [ %indvars.iv.next, %1 ]
  %2 = add nuw nsw i64 %indvars.iv, %indvars.iv19
  %arrayidx.us = getelementptr inbounds i32*, i32** %A, i64 %indvars.iv
  %3 = load i32*, i32** %arrayidx.us, align 8, !tbaa !2
  %arrayidx3.us = getelementptr inbounds i32, i32* %3, i64 %indvars.iv19
  %4 = trunc i64 %2 to i32
  store i32 %4, i32* %arrayidx3.us, align 4, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %._crit_edge.us, label %1, !llvm.loop !8

._crit_edge.us:                                   ; preds = %1
  %indvars.iv.next.lcssa = phi i64 [ %indvars.iv.next, %1 ]
  %indvars.iv.next20 = add nuw nsw i64 %indvars.iv19, 1
  %exitcond22 = icmp eq i64 %indvars.iv.next20, %wide.trip.count
  br i1 %exitcond22, label %._crit_edge17.loopexit, label %.lr.ph.us

._crit_edge17.loopexit:                           ; preds = %._crit_edge.us
  br label %._crit_edge17

._crit_edge17:                                    ; preds = %._crit_edge17.loopexit, %0
  ret void
}

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPi", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
!8 = distinct !{!8, !9}
!9 = distinct !{!"intel.optreport.rootnode", !10}
!10 = distinct !{!"intel.optreport", !11}
!11 = !{!"intel.optreport.next_sibling", !12}
!12 = distinct !{!"intel.optreport.rootnode", !13}
!13 = distinct !{!"intel.optreport", !14}
!14 = !{!"intel.optreport.remarks", !15}
!15 = !{!"intel.optreport.remark", i32 25436, !"Loop completely unrolled by %d", i32 6}
