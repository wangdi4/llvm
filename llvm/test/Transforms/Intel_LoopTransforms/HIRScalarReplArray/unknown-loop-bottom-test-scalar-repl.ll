; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-scalarrepl-array,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Verify that we are able to scalar replace load-only groups used in unknown loop bottom test.

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   do.body:
; CHECK: |   %indvars.iv.next = i1  +  1;
; CHECK: |   if ((@A)[0][i1 + 1] != (@A)[0][i1 + 2])
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto do.body;
; CHECK: |   }
; CHECK: + END LOOP
; CHECK: END REGION

; CHECK: Function: foo

; CHECK: %scalarepl = (@A)[0][1];
; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   do.body:
; CHECK: |   %indvars.iv.next = i1  +  1;
; CHECK: |   %scalarepl1 = (@A)[0][i1 + 2];
; CHECK: |   if (%scalarepl != %scalarepl1)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      %scalarepl = %scalarepl1;
; CHECK: |      goto do.body;
; CHECK: |   }
; CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @foo() local_unnamed_addr #0 {
entry:
  br label %do.body

do.body:                                          ; preds = %do.body, %entry
  %indvars.iv = phi i64 [ %indvars.iv.next, %do.body ], [ 0, %entry ]
  %indvars.iv.next = add nuw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv.next
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %1 = add nuw nsw i64 %indvars.iv, 2
  %arrayidx2 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %1
  %2 = load i32, i32* %arrayidx2, align 4, !tbaa !2
  %cmp = icmp eq i32 %0, %2
  br i1 %cmp, label %do.end, label %do.body

do.end:                                           ; preds = %do.body
  %indvars.iv.next.lcssa = phi i64 [ %indvars.iv.next, %do.body ]
  %3 = trunc i64 %indvars.iv.next.lcssa to i32
  ret i32 %3
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 8917aed201bae2133195342c218e4ac7e137d59c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm fedd2ae89cd17a6c09f52df573addd6e4c0e785b)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
