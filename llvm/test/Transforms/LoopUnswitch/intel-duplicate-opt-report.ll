; RUN: opt < %s -enable-new-pm=0 -loop-unswitch -S < %s 2>&1 | FileCheck %s

; Verify that we do not duplicate loop opt report when cloning loops (during unswitching or otherwise).

; CHECK: for.body{{.*}}:

; CHECK-NOT: !llvm.loop

; CHECK: for.body{{.*}}:

; CHECK: !llvm.loop

define dso_local void @foo(i32 %t, i32* nocapture %A, i32* nocapture %B) {
entry:
  %cmp1 = icmp sgt i32 %t, 0
  br label %for.body

for.body:                                         ; preds = %entry, %if.end
  %i.010 = phi i32 [ 0, %entry ], [ %inc5, %if.end ]
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %idxprom = zext i32 %i.010 to i64
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %idxprom2 = zext i32 %i.010 to i64
  %arrayidx3 = getelementptr inbounds i32, i32* %B, i64 %idxprom2
  %1 = load i32, i32* %arrayidx3, align 4
  %inc4 = add nsw i32 %1, 1
  store i32 %inc4, i32* %arrayidx3, align 4
  %inc5 = add nuw nsw i32 %i.010, 1
  %cmp = icmp ult i32 %inc5, 4
  br i1 %cmp, label %for.body, label %for.end, !llvm.loop !0

for.end:                                          ; preds = %if.end
  ret void
}

!0 = distinct !{!0, !1}
!1 = distinct !{!"intel.optreport.rootnode", !2}
!2 = distinct !{!"intel.optreport", !3}
!3 = !{!"intel.optreport.remarks", !4}
!4 = !{!"intel.optreport.remark", !"dummy opt report"}

