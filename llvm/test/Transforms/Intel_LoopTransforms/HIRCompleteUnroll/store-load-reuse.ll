; REQUIRES: asserts

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -tbaa -hir-pre-vec-complete-unroll -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll" -aa-pipeline="tbaa" -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s

; RUN: opt -convert-to-subscript < %s | opt -hir-ssa-deconstruction -hir-temp-cleanup -tbaa -hir-pre-vec-complete-unroll -debug-only=hir-complete-unroll 2>&1 | FileCheck %s
; RUN: opt -convert-to-subscript < %s | opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll" -aa-pipeline="tbaa" -debug-only=hir-complete-unroll 2>&1 | FileCheck %s

; Verify that we have GEP savings of 18 (9 * 2) due to reuse between (%A)[i1] and (%A)[i1 + -1].

; + DO i1 = 0, 9, 1   <DO_LOOP>
; |   (%A)[i1] = i1;
; |   if (%k > 5)
; |   {
; |      %t.013 = (%A)[i1 + -1]  +  %t.013;
; |   }
; + END LOOP

; CHECK: GEPSavings: 18

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i32 @foo(i32* nocapture %A, i32 %k) local_unnamed_addr #0 {
entry:
  %cmp1 = icmp sgt i32 %k, 5
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %t.013 = phi i32 [ 0, %entry ], [ %t.1, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4, !tbaa !2
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %1 = add nsw i64 %indvars.iv, -1
  %arrayidx3 = getelementptr inbounds i32, i32* %A, i64 %1
  %2 = load i32, i32* %arrayidx3, align 4, !tbaa !2
  %add = add nsw i32 %2, %t.013
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %t.1 = phi i32 [ %add, %if.then ], [ %t.013, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  %t.1.lcssa = phi i32 [ %t.1, %for.inc ]
  ret i32 %t.1.lcssa
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 6a026a7944d2244cc728fa0e8328c8ce3bc0d72c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 33ca482a1b7f763cfc3c669b2ba2ff2ebbb4b09e)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
