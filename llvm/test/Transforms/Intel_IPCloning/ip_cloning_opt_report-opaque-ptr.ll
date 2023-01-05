; INTEL_FEATURE_SW_ADVANCED
; Checks that when functions are cloned opt-report messages are propagated to
; cloned functions. This test is based on ip_cloning_2-opaque-ptr.ll.

; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers < %s -passes='module(post-inline-ip-cloning)' -ip-cloning-loop-heuristic -S 2>&1 | FileCheck %s

@F_1 = external local_unnamed_addr global [100 x i32], align 16

define void @foo() local_unnamed_addr {
entry:
  tail call fastcc void @bar(i32 10)
  tail call fastcc void @bar(i32 20)
  ret void
}

; CHECK: define internal fastcc void @bar(i32 %ub) unnamed_addr
; CHECK: br i1 %exitcond, label %for.end, label %for.body, !llvm.loop ![[LOOP:[0-9]+]]

; CHECK: define internal fastcc void @bar.1(i32 %ub) unnamed_addr
; CHECK: br i1 %exitcond, label %for.end, label %for.body, !llvm.loop ![[LOOP1:[0-9]+]]

; CHECK: define internal fastcc void @bar.2(i32 %ub) unnamed_addr
; CHECK: br i1 %exitcond, label %for.end, label %for.body, !llvm.loop ![[LOOP2:[0-9]+]]

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

; CHECK-DAG: ![[LOOP]] = distinct !{![[LOOP]], ![[ROOT:[0-9]+]]}
; CHECK-DAG: ![[ROOT]] = distinct !{!"intel.optreport.rootnode", ![[REPORT:[0-9]+]]}
; CHECK-DAG: ![[REPORT]] = distinct !{!"intel.optreport", ![[REMARKS:[0-9]+]]}
; CHECK-DAG: ![[REMARKS]] = !{!"intel.optreport.remarks", ![[DUMMY:[0-9]+]]}
; CHECK-DAG: ![[DUMMY]] = !{!"intel.optreport.remark", i32 0, !"Dummy remark"}

; CHECK-DAG: ![[LOOP1]] = distinct !{![[LOOP1]], ![[ROOT1:[0-9]+]]}
; CHECK-DAG: ![[ROOT1]] = distinct !{!"intel.optreport.rootnode", ![[REPORT1:[0-9]+]]}
; CHECK-DAG: ![[REPORT1]] = distinct !{!"intel.optreport", ![[REMARKS]]}

; CHECK-DAG: ![[LOOP2]] = distinct !{![[LOOP2]], ![[ROOT2:[0-9]+]]}
; CHECK-DAG: ![[ROOT2]] = distinct !{!"intel.optreport.rootnode", ![[REPORT2:[0-9]+]]}
; CHECK-DAG: ![[REPORT2]] = distinct !{!"intel.optreport", ![[REMARKS]]}

!0 = distinct !{!0, !1}
!1 = distinct !{!"intel.optreport.rootnode", !2}
!2 = distinct !{!"intel.optreport", !3}
!3 = !{!"intel.optreport.remarks", !4}
!4 = !{!"intel.optreport.remark", i32 0, !"Dummy remark"}
; end INTEL_FEATURE_SW_ADVANCED
