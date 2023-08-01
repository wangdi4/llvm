; RUN: opt -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -hir-create-function-level-region -disable-output < %s 2>&1 | FileCheck %s

; Check parsing output for the loop verifying that we are able to build lexical links successfully.


; CHECK: BEGIN REGION

; Verify that function entry block is split and the new block is set as the region entry block.
; CHECK: EntryBB: %entry.split

; CHECK: + DO i1 = 0, 1, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   %conv = sitofp.i32.float(2 * i2);
; CHECK: |   |   (%b)[i2] = %conv;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %0 = (%b)[-1 * i1 + 1];
; CHECK: |   if (%0 > 5.000000e+01)
; CHECK: |   {
; CHECK: |      goto while.end;
; CHECK: |   }
; CHECK: |
; CHECK: |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   %2 = (%b)[i2];
; CHECK: |   |   %conv13 = fptosi.float.i32(%2);
; CHECK: |   |   (%a)[i2] = %conv13;
; CHECK: |   + END LOOP
; CHECK: + END LOOP

; CHECK: while.end:
; CHECK: ret 0;
; CHECK: END REGION


;Module Before HIR; ModuleID = 'goto-issue.c'
source_filename = "goto-issue.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i32 @foo(ptr nocapture %a, ptr nocapture %b) local_unnamed_addr #0 {
entry:
  br label %while.body

while.body:                                       ; preds = %entry, %for.cond.cleanup9
  %indvars.iv40 = phi i64 [ 1, %entry ], [ %indvars.iv.next41, %for.cond.cleanup9 ]
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %arrayidx2 = getelementptr inbounds float, ptr %b, i64 %indvars.iv40
  %0 = load float, ptr %arrayidx2, align 4, !tbaa !2
  %cmp3 = fcmp ogt float %0, 5.000000e+01
  br i1 %cmp3, label %while.end, label %if.end

for.body:                                         ; preds = %for.body, %while.body
  %indvars.iv = phi i64 [ 0, %while.body ], [ %indvars.iv.next, %for.body ]
  %indvars.iv.tr = trunc i64 %indvars.iv to i32
  %1 = shl i32 %indvars.iv.tr, 1
  %conv = sitofp i32 %1 to float
  %arrayidx = getelementptr inbounds float, ptr %b, i64 %indvars.iv
  store float %conv, ptr %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

if.end:                                           ; preds = %for.cond.cleanup
  br label %for.body10

for.cond.cleanup9:                                ; preds = %for.body10
  %indvars.iv.next41 = add nsw i64 %indvars.iv40, -1
  %tobool = icmp eq i64 %indvars.iv40, 0
  br i1 %tobool, label %while.end, label %while.body

for.body10:                                       ; preds = %for.body10, %if.end
  %indvars.iv37 = phi i64 [ 0, %if.end ], [ %indvars.iv.next38, %for.body10 ]
  %arrayidx12 = getelementptr inbounds float, ptr %b, i64 %indvars.iv37
  %2 = load float, ptr %arrayidx12, align 4, !tbaa !2
  %conv13 = fptosi float %2 to i32
  %arrayidx15 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv37
  store i32 %conv13, ptr %arrayidx15, align 4, !tbaa !6
  %indvars.iv.next38 = add nuw nsw i64 %indvars.iv37, 1
  %exitcond39 = icmp eq i64 %indvars.iv.next38, 100
  br i1 %exitcond39, label %for.cond.cleanup9, label %for.body10

while.end:                                        ; preds = %for.cond.cleanup9, %for.cond.cleanup
  ret i32 0
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 (cfe/trunk)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
