; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-rematerialize,print<hir>" -aa-pipeline="basic-aa" -hir-allow-loop-materialization-regions=true -disable-output < %s 2>&1 | FileCheck %s


; CHECK:Function: set
; CHECK:         BEGIN REGION { }
; CHECK:               (%knobs)[0] = 1.000000e+01;
; CHECK:               (%knobs)[1] = 1.000000e+01;
; CHECK:               (%knobs)[2] = 1.000000e+00;
; CHECK:               ret ;
; CHECK:         END REGION

; CHECK:Function: set
; CHECK-NOT: END LOOP

;Module Before HIR
; ModuleID = 'int-to-dobule.c'
source_filename = "int-to-dobule.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @set(ptr nocapture %knobs) local_unnamed_addr #0 {
entry:
  store double 1.000000e+01, ptr %knobs, align 8, !tbaa !2
  %arrayidx1 = getelementptr inbounds double, ptr %knobs, i64 1
  store double 1.000000e+01, ptr %arrayidx1, align 8, !tbaa !2
  %arrayidx2 = getelementptr inbounds double, ptr %knobs, i64 2
  store double 1.000000e+00, ptr %arrayidx2, align 8, !tbaa !2
  ret void
}

attributes #0 = { norecurse nounwind uwtable writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) 2019.8.1.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"double", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
