; RUN: opt -passes="simplifycfg" -S < %s | FileCheck %s

; Verify that SimplifyCFG preserves tbaa metadata on speculated stores.

; CHECK: %spec.store.select = select i1 %cmp3, i32 0, i32 5
; CHECK: store i32 %spec.store.select, ptr %A, align 4, !tbaa !0

define dso_local void @foo(ptr noundef %A, i32 %t) {
entry:
 br label %bb

bb:                                         ; preds = %for.cond
  store i32 5, ptr %A, align 4, !tbaa !3
  %cmp3 = icmp slt i32 %t, 5
  br i1 %cmp3, label %if.then, label %exit

if.then:                                          ; preds = %bb
  store i32 0, ptr %A, align 4, !tbaa !3
  br label %exit

exit:                                          ; preds = %for.cond
  ret void
}

!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}

