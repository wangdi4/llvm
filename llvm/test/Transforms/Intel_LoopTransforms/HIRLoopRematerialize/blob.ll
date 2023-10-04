; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-rematerialize,print<hir>" -aa-pipeline="basic-aa" -hir-loop-rematerialize-tc-lb=1 -hir-allow-loop-materialization-regions=true -disable-output < %s 2>&1 | FileCheck %s
;
; CHECK:Function: Vsub

; CHECK:         BEGIN REGION { }
; CHECK:               %mul = %n  *  %n;
; CHECK:               %mul1 = %mul  *  (%b)[0];
; CHECK:               (%a)[0] = %mul1;
; CHECK:               %mul5 = %mul  *  (%b)[1];
; CHECK:               (%a)[1] = %mul5;
; CHECK:               %mul9 = %mul  *  (%b)[2];
; CHECK:               (%a)[2] = %mul9;
; CHECK:               ret ;
; CHECK:         END REGION

; CHECK:Function: Vsub

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK:              |   %mul = %n  *  %n;
; CHECK:              |   %mul1 = %mul  *  (%b)[i1];
; CHECK:              |   (%a)[i1] = %mul1;
; CHECK:              + END LOOP
; CHECK-NOT:               %mul5 = %mul  *  (%b)[1];
; CHECK-NOT:               (%a)[1] = %mul5;
; CHECK-NOT:               %mul9 = %mul  *  (%b)[2];
; CHECK-NOT:               (%a)[2] = %mul9;
; CHECK:              ret ;
; CHECK:        END REGION

; ModuleID = 'blob.c'
source_filename = "blob.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @Vsub(ptr noalias nocapture %a, ptr noalias nocapture readonly %b, double %n) local_unnamed_addr #0 {
entry:
  %mul = fmul double %n, %n
  %0 = load double, ptr %b, align 8, !tbaa !2
  %mul1 = fmul double %mul, %0
  store double %mul1, ptr %a, align 8, !tbaa !2
  %arrayidx4 = getelementptr inbounds double, ptr %b, i64 1
  %1 = load double, ptr %arrayidx4, align 8, !tbaa !2
  %mul5 = fmul double %mul, %1
  %arrayidx6 = getelementptr inbounds double, ptr %a, i64 1
  store double %mul5, ptr %arrayidx6, align 8, !tbaa !2
  %arrayidx8 = getelementptr inbounds double, ptr %b, i64 2
  %2 = load double, ptr %arrayidx8, align 8, !tbaa !2
  %mul9 = fmul double %mul, %2
  %arrayidx10 = getelementptr inbounds double, ptr %a, i64 2
  store double %mul9, ptr %arrayidx10, align 8, !tbaa !2
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 59f0a4f3baa133224b131c19ed0a4a18f74d9a89) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 8e385d7010b2f29a6fe276330aec211a94986c31)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"double", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
