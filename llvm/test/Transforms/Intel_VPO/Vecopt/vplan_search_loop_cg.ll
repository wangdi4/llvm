; RUN: opt < %s -S -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation \
; RUN:     -hir-vec-dir-insert -VPlanDriverHIR -allow-memory-speculation -hir-cg \
; RUN:     2>&1 | FileCheck %s
;
; Check that search loop idiom was vectorized.

; CHECK: @llvm.cttz

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @_Z3fooiPKaPaa(i32 %n, i8* nocapture readonly %a, i8* nocapture readnone %b, i8 signext %val) local_unnamed_addr #0 {
entry:
  %cmp8 = icmp sgt i32 %n, 0
  br i1 %cmp8, label %for.body.preheader, label %cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i8, i8* %a, i64 %indvars.iv
  %1 = load i8, i8* %arrayidx, align 1, !tbaa !2
  %cmp2 = icmp eq i8 %1, %val
  br i1 %cmp2, label %for.inc, label %cleanup.loopexit.split.loop.exit

for.inc:                                          ; preds = %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %cleanup.loopexit, !llvm.loop !5

cleanup.loopexit.split.loop.exit:                 ; preds = %for.body
  %indvars.iv.lcssa = phi i64 [ %indvars.iv, %for.body ]
  %2 = trunc i64 %indvars.iv.lcssa to i32
  br label %cleanup

cleanup.loopexit:                                 ; preds = %for.inc
  br label %cleanup

cleanup:                                          ; preds = %cleanup.loopexit, %cleanup.loopexit.split.loop.exit, %entry
  %index.0 = phi i32 [ -1, %entry ], [ %2, %cleanup.loopexit.split.loop.exit ], [ -1, %cleanup.loopexit ]
  ret i32 %index.0
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 6d93f34e605c44d05e5c49346cf267f862c04f87) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 89e0ade2ec1aea25dcf4f481a86a58a1ce934c50)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C++ TBAA"}
!5 = distinct !{!5, !6, !7}
!6 = !{!"llvm.loop.vectorize.ignore_profitability"}
!7 = !{!"llvm.loop.vectorize.enable", i1 true}
