; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-rematerialize,print<hir>" -hir-loop-rematerialize-tc-lb=1 -aa-pipeline="basic-aa" -hir-allow-loop-materialization-regions=true -disable-output < %s 2>&1 | FileCheck %s


; CHECK:Function: Vsub

; CHECK:         BEGIN REGION { }
; CHECK:               %0 = (%b)[0];
; CHECK:               (%a)[0] = (%n * %n * %0);
; CHECK:               %1 = (%b)[1];
; CHECK:               (%a)[1] = (%n * %n * %1);
; CHECK:               %2 = (%b)[2];
; CHECK:               (%a)[2] = (%n * %n * %2);
; CHECK:               ret ;
; CHECK:         END REGION

; CHECK:Function: Vsub

; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK:               |   %0 = (%b)[i1];
; CHECK:               |   (%a)[i1] = (%n * %n * %0);
; CHECK:               + END LOOP
; CHECK-NOT:               %1 = (%b)[1];
; CHECK-NOT:               (%a)[1] = (%n * %n * %1);
; CHECK-NOT:               %2 = (%b)[2];
; CHECK-NOT:               (%a)[2] = (%n * %n * %2);
; CHECK:               ret ;
; CHECK:         END REGION



;Module Before HIR
; ModuleID = 'blob-int.c'
source_filename = "blob-int.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @Vsub(ptr noalias nocapture %a, ptr noalias nocapture readonly %b, i32 %n) local_unnamed_addr #0 {
entry:
  %mul = mul nsw i32 %n, %n
  %0 = load i32, ptr %b, align 4, !tbaa !2
  %mul1 = mul nsw i32 %0, %mul
  store i32 %mul1, ptr %a, align 4, !tbaa !2
  %arrayidx4 = getelementptr inbounds i32, ptr %b, i64 1
  %1 = load i32, ptr %arrayidx4, align 4, !tbaa !2
  %mul5 = mul nsw i32 %1, %mul
  %arrayidx6 = getelementptr inbounds i32, ptr %a, i64 1
  store i32 %mul5, ptr %arrayidx6, align 4, !tbaa !2
  %arrayidx8 = getelementptr inbounds i32, ptr %b, i64 2
  %2 = load i32, ptr %arrayidx8, align 4, !tbaa !2
  %mul9 = mul nsw i32 %2, %mul
  %arrayidx10 = getelementptr inbounds i32, ptr %a, i64 2
  store i32 %mul9, ptr %arrayidx10, align 4, !tbaa !2
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 59f0a4f3baa133224b131c19ed0a4a18f74d9a89) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 8e385d7010b2f29a6fe276330aec211a94986c31)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
