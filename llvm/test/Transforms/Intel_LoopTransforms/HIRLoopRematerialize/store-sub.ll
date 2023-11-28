; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-rematerialize,print<hir>" -aa-pipeline="basic-aa" -hir-loop-rematerialize-tc-lb=1 -hir-allow-loop-materialization-regions=true -disable-output < %s 2>&1 | FileCheck %s

; CHECK:Function: Vsub

; CHECK:         BEGIN REGION { }
; CHECK:               %sub = (%b)[0]  -  (%c)[0];
; CHECK:               (%a)[0] = %sub;
; CHECK:               %sub5 = (%b)[1]  -  (%c)[1];
; CHECK:               (%a)[1] = %sub5;
; CHECK:               %sub9 = (%b)[2]  -  (%c)[2];
; CHECK:               (%a)[2] = %sub9;
; CHECK:               ret ;
; CHECK:         END REGION

; CHECK:Function: Vsub

; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK:               |   %sub = (%b)[i1]  -  (%c)[i1];
; CHECK:               |   (%a)[i1] = %sub;
; CHECK:               + END LOOP

; CHECK-NOT:               %sub5 = (%b)[1]  -  (%c)[1];
; CHECK-NOT:               (%a)[1] = %sub5;
; CHECK-NOT:               %sub9 = (%b)[2]  -  (%c)[2];
; CHECK-NOT:               (%a)[2] = %sub9;

; CHECK:               ret ;
; CHECK:         END REGION


;Module Before HIR
; ModuleID = 'simple.c'
source_filename = "simple.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @Vsub(ptr noalias nocapture %a, ptr noalias nocapture readonly %b, ptr noalias nocapture readonly %c) local_unnamed_addr #0 {
entry:
  %0 = load double, ptr %b, align 8, !tbaa !2
  %1 = load double, ptr %c, align 8, !tbaa !2
  %sub = fsub double %0, %1
  store double %sub, ptr %a, align 8, !tbaa !2
  %arrayidx3 = getelementptr inbounds double, ptr %b, i64 1
  %2 = load double, ptr %arrayidx3, align 8, !tbaa !2
  %arrayidx4 = getelementptr inbounds double, ptr %c, i64 1
  %3 = load double, ptr %arrayidx4, align 8, !tbaa !2
  %sub5 = fsub double %2, %3
  %arrayidx6 = getelementptr inbounds double, ptr %a, i64 1
  store double %sub5, ptr %arrayidx6, align 8, !tbaa !2
  %arrayidx7 = getelementptr inbounds double, ptr %b, i64 2
  %4 = load double, ptr %arrayidx7, align 8, !tbaa !2
  %arrayidx8 = getelementptr inbounds double, ptr %c, i64 2
  %5 = load double, ptr %arrayidx8, align 8, !tbaa !2
  %sub9 = fsub double %4, %5
  %arrayidx10 = getelementptr inbounds double, ptr %a, i64 2
  store double %sub9, ptr %arrayidx10, align 8, !tbaa !2
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 5bcc07a3062bc172ccee604ad6c78f4be2ff02df) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm a8d6867359b5a7e34274495337d251a2bf4516e3)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"double", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
