; RUN: opt -passes="gvn" < %s -enable-split-backedge-in-load-pre -S | FileCheck %s

; Verify that loop id gets duplicated to the new basic block when the backedge
; is split by GVN.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global [10 x double] zeroinitializer, align 16

; CHECK: @bar
; CHECK: crit_edge, !llvm.loop
; CHECK: %.pre = load double, ptr getelementptr inbounds ([10 x double], ptr @A, i64 0, i64 1)
; CHECK-NEXT: br label %for.body, !llvm.loop
; CHECK: ret void

define void @bar(double %n, double %m) {
entry:
  store double %m, ptr getelementptr inbounds ([10 x double], ptr @A, i64 0, i64 1), align 8, !tbaa !1
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 3, %entry ], [ %indvars.iv.next, %for.body ]
  %n.addr.05 = phi double [ %n, %entry ], [ %mul, %for.body ]
  %0 = load double, ptr getelementptr inbounds ([10 x double], ptr @A, i64 0, i64 1), align 8, !tbaa !1
  %mul = fmul fast double %0, %n.addr.05
  %arrayidx = getelementptr inbounds [10 x double], ptr @A, i64 0, i64 %indvars.iv
  store double %mul, ptr %arrayidx, align 8, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 9
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !6

for.end:                                          ; preds = %for.body
  ret void
}

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (trunk 21198) (llvm/branches/loopopt 21242)"}
!1 = !{!2, !3, i64 0}
!2 = !{!"array@_ZTSA10_d", !3, i64 0}
!3 = !{!"double", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.unroll.disable"}
