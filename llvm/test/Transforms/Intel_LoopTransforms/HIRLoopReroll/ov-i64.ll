; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" -hir-verify-cf-def-level < %s 2>&1 | FileCheck %s

; If it were rerolled by 8, the new TC overflows 64-bit range.

; CHECK:Function: foo

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, 2305843009213693951, 1   <DO_LOOP>
; CHECK:              |   (%a)[8 * i1] = (%b)[8 * i1];
; CHECK:              |   (%a)[8 * i1 + 1] = (%b)[8 * i1 + 1];
; CHECK:              |   (%a)[8 * i1 + 2] = (%b)[8 * i1 + 2];
; CHECK:              |   (%a)[8 * i1 + 3] = (%b)[8 * i1 + 3];
; CHECK:              |   (%a)[8 * i1 + 4] = (%b)[8 * i1 + 4];
; CHECK:              |   (%a)[8 * i1 + 5] = (%b)[8 * i1 + 5];
; CHECK:              |   (%a)[8 * i1 + 6] = (%b)[8 * i1 + 6];
; CHECK:              |   (%a)[8 * i1 + 7] = (%b)[8 * i1 + 7];
; CHECK:              + END LOOP
; CHECK:        END REGION

; CHECK:Function: foo

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, 2305843009213693951, 1   <DO_LOOP>
; CHECK:              |   (%a)[8 * i1] = (%b)[8 * i1];
; CHECK:              |   (%a)[8 * i1 + 1] = (%b)[8 * i1 + 1];
; CHECK:              |   (%a)[8 * i1 + 2] = (%b)[8 * i1 + 2];
; CHECK:              |   (%a)[8 * i1 + 3] = (%b)[8 * i1 + 3];
; CHECK:              |   (%a)[8 * i1 + 4] = (%b)[8 * i1 + 4];
; CHECK:              |   (%a)[8 * i1 + 5] = (%b)[8 * i1 + 5];
; CHECK:              |   (%a)[8 * i1 + 6] = (%b)[8 * i1 + 6];
; CHECK:              |   (%a)[8 * i1 + 7] = (%b)[8 * i1 + 7];
; CHECK:              + END LOOP
; CHECK:        END REGION

;Module Before HIR
; ModuleID = 'cmplr-8109.c'
source_filename = "cmplr-8109.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(ptr nocapture %a, ptr nocapture readonly %b) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %i.078 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %mul = shl nsw i64 %i.078, 3
  %arrayidx = getelementptr inbounds i32, ptr %b, i64 %mul
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !2
  %arrayidx3 = getelementptr inbounds i32, ptr %a, i64 %mul
  store i32 %0, ptr %arrayidx3, align 4, !tbaa !2
  %add5 = or i64 %mul, 1
  %arrayidx6 = getelementptr inbounds i32, ptr %b, i64 %add5
  %1 = load i32, ptr %arrayidx6, align 4, !tbaa !2
  %arrayidx9 = getelementptr inbounds i32, ptr %a, i64 %add5
  store i32 %1, ptr %arrayidx9, align 4, !tbaa !2
  %add11 = or i64 %mul, 2
  %arrayidx12 = getelementptr inbounds i32, ptr %b, i64 %add11
  %2 = load i32, ptr %arrayidx12, align 4, !tbaa !2
  %arrayidx15 = getelementptr inbounds i32, ptr %a, i64 %add11
  store i32 %2, ptr %arrayidx15, align 4, !tbaa !2
  %add17 = or i64 %mul, 3
  %arrayidx18 = getelementptr inbounds i32, ptr %b, i64 %add17
  %3 = load i32, ptr %arrayidx18, align 4, !tbaa !2
  %arrayidx21 = getelementptr inbounds i32, ptr %a, i64 %add17
  store i32 %3, ptr %arrayidx21, align 4, !tbaa !2
  %add23 = or i64 %mul, 4
  %arrayidx24 = getelementptr inbounds i32, ptr %b, i64 %add23
  %4 = load i32, ptr %arrayidx24, align 4, !tbaa !2
  %arrayidx27 = getelementptr inbounds i32, ptr %a, i64 %add23
  store i32 %4, ptr %arrayidx27, align 4, !tbaa !2
  %add29 = or i64 %mul, 5
  %arrayidx30 = getelementptr inbounds i32, ptr %b, i64 %add29
  %5 = load i32, ptr %arrayidx30, align 4, !tbaa !2
  %arrayidx33 = getelementptr inbounds i32, ptr %a, i64 %add29
  store i32 %5, ptr %arrayidx33, align 4, !tbaa !2
  %add35 = or i64 %mul, 6
  %arrayidx36 = getelementptr inbounds i32, ptr %b, i64 %add35
  %6 = load i32, ptr %arrayidx36, align 4, !tbaa !2
  %arrayidx39 = getelementptr inbounds i32, ptr %a, i64 %add35
  store i32 %6, ptr %arrayidx39, align 4, !tbaa !2
  %add41 = or i64 %mul, 7
  %arrayidx42 = getelementptr inbounds i32, ptr %b, i64 %add41
  %7 = load i32, ptr %arrayidx42, align 4, !tbaa !2
  %arrayidx45 = getelementptr inbounds i32, ptr %a, i64 %add41
  store i32 %7, ptr %arrayidx45, align 4, !tbaa !2
  %inc = add nuw nsw i64 %i.078, 1
  %exitcond = icmp eq i64 %inc, 2305843009213693952
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57c33ad4c535ae8b183eddbaecd43589a368ac99) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm f7aeee7c72dd637d050296f8a574f7ca3ab83646)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
