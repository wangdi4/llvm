; XFAIL: *
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-rematerialize,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; CHECK:Function: _Z4VdotRdPKdS1_

; CHECK:         BEGIN REGION { }
; CHECK:               %mul = (%b)[0]  *  (%c)[0];
; CHECK:               %mul4 = (%b)[1]  *  (%c)[1];
; CHECK:               %add = %mul  +  %mul4;
; CHECK:               %mul7 = (%b)[2]  *  (%c)[2];
; CHECK:               %add8 = %add  +  %mul7;
; CHECK:               (%a)[0] = %add8;
; CHECK:               ret ;
; CHECK:         END REGION

; CHECK:Function: _Z4VdotRdPKdS1_

; CHECK:         DO i1 =
; CHECK:         END LOOP


;Module Before HIR
; ModuleID = 'vdot.cpp'
source_filename = "vdot.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @_Z4VdotRdPKdS1_(ptr nocapture dereferenceable(8) %a, ptr nocapture readonly %b, ptr nocapture readonly %c) local_unnamed_addr #0 {
entry:
  %0 = load double, ptr %b, align 8, !tbaa !2
  %1 = load double, ptr %c, align 8, !tbaa !2
  %mul = fmul double %0, %1
  %arrayidx2 = getelementptr inbounds double, ptr %b, i64 1
  %2 = load double, ptr %arrayidx2, align 8, !tbaa !2
  %arrayidx3 = getelementptr inbounds double, ptr %c, i64 1
  %3 = load double, ptr %arrayidx3, align 8, !tbaa !2
  %mul4 = fmul double %2, %3
  %add = fadd double %mul, %mul4
  %arrayidx5 = getelementptr inbounds double, ptr %b, i64 2
  %4 = load double, ptr %arrayidx5, align 8, !tbaa !2
  %arrayidx6 = getelementptr inbounds double, ptr %c, i64 2
  %5 = load double, ptr %arrayidx6, align 8, !tbaa !2
  %mul7 = fmul double %4, %5
  %add8 = fadd double %add, %mul7
  store double %add8, ptr %a, align 8, !tbaa !2
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
!5 = !{!"Simple C++ TBAA"}
