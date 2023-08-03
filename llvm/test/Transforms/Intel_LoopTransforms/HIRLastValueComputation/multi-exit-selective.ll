; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,print<hir>" -disable-output 2>&1 < %s | FileCheck %s

; Verify that liveout value %indvars.iv.out is only added to the exit it is really liveout from (%loopexit).
; It is not added to the other early exit (%cleanup.loopexit) or to the postexit of the loop.

; Incoming HIR-
; + DO i1 = 0, 9, 1   <DO_MULTI_EXIT_LOOP>
; |   %indvars.iv.out = i1;
; |   %0 = (%A)[i1];
; |   if (%0 > 0)
; |   {
; |      goto loopexit;
; |   }
; |   if (%0 < 0)
; |   {
; |      goto cleanup.loopexit;
; |   }
; + END LOOP

; CHECK:      + DO i1 = 0, 9, 1   <DO_MULTI_EXIT_LOOP>
; CHECK-NEXT: |   %0 = (%A)[i1];
; CHECK-NEXT: |   if (%0 > 0)
; CHECK-NEXT: |   {
; CHECK-NEXT: |      %indvars.iv.out = i1;
; CHECK-NEXT: |      goto loopexit;
; CHECK-NEXT: |   }
; CHECK-NEXT: |   if (%0 < 0)
; CHECK-NEXT: |   {
; CHECK-NEXT: |      goto cleanup.loopexit;
; CHECK-NEXT: |   }
; CHECK-NEXT: + END LOOP
; CHECK-NEXT: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @foo(ptr nocapture readonly %A) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp sgt i32 %0, 0
  br i1 %cmp1, label %loopexit, label %if.end

if.end:                                           ; preds = %for.body
  %cmp4 = icmp slt i32 %0, 0
  br i1 %cmp4, label %cleanup.loopexit, label %for.inc

for.inc:                                          ; preds = %if.end
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv.next, 10
  br i1 %cmp, label %for.body, label %cleanup.loopexit

loopexit:                        ; preds = %for.body
  %indvars.iv.lcssa = phi i64 [ %indvars.iv, %for.body ]
  %1 = trunc i64 %indvars.iv.lcssa to i32
  br label %cleanup

cleanup.loopexit:                                 ; preds = %for.inc, %if.end
  %retval.0.ph = phi i32 [ -1, %if.end ], [ 0, %for.inc ]
  br label %cleanup

cleanup:                                          ; preds = %cleanup.loopexit, %loopexit
  %retval.0 = phi i32 [ %1, %loopexit ], [ %retval.0.ph, %cleanup.loopexit ]
  ret i32 %retval.0
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 81a88af0f8e5fbc44460e0e3d157b6ba6d246190) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 1ada5f21c0882b409a9afdfc920562a0a53135fd)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
