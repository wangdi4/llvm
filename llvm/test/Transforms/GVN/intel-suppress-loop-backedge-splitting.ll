; RUN: opt < %s -gvn -S | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Verify that gvn doesnt't break the loop backedge in @foo() to hoist the load %0 when "pre_loopopt" attribute is present. It does hoist it for @bar() since the attribute is not present.

; CHECK: @foo
; CHECK-NOT: crit_edge
; CHECK: ret void


@A = common local_unnamed_addr global [10 x double] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo(double %n, double %m) local_unnamed_addr "pre_loopopt" {
entry:
  store double %m, double* getelementptr inbounds ([10 x double], [10 x double]* @A, i64 0, i64 1), align 8, !tbaa !1
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 3, %entry ], [ %indvars.iv.next, %for.body ]
  %n.addr.05 = phi double [ %n, %entry ], [ %mul, %for.body ]
  %0 = load double, double* getelementptr inbounds ([10 x double], [10 x double]* @A, i64 0, i64 1), align 8, !tbaa !1
  %mul = fmul fast double %0, %n.addr.05
  %arrayidx = getelementptr inbounds [10 x double], [10 x double]* @A, i64 0, i64 %indvars.iv
  store double %mul, double* %arrayidx, align 8, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 9
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

; CHECK: @bar
; CHECK: crit_edge:
; CHECK: %.pre = load double, double* getelementptr inbounds ([10 x double], [10 x double]* @A, i64 0, i64 1)
; CHECK-NEXT: br label %for.body
; CHECK: ret void

; Function Attrs: norecurse nounwind uwtable
define void @bar(double %n, double %m) local_unnamed_addr {
entry:
  store double %m, double* getelementptr inbounds ([10 x double], [10 x double]* @A, i64 0, i64 1), align 8, !tbaa !1
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 3, %entry ], [ %indvars.iv.next, %for.body ]
  %n.addr.05 = phi double [ %n, %entry ], [ %mul, %for.body ]
  %0 = load double, double* getelementptr inbounds ([10 x double], [10 x double]* @A, i64 0, i64 1), align 8, !tbaa !1
  %mul = fmul fast double %0, %n.addr.05
  %arrayidx = getelementptr inbounds [10 x double], [10 x double]* @A, i64 0, i64 %indvars.iv
  store double %mul, double* %arrayidx, align 8, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 9
  br i1 %exitcond, label %for.end, label %for.body

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

