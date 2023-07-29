; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-rematerialize,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; HIR framework does not create region for structure offset accesses so the
; checks are expected to fail.
; It can be extended if required.
; XFAIL:*

;
;struct S {
;  double X;
;  double Y;
;};
;void Vsub(double a[3], const struct S b[3], double n) {
;  a[0] = n * n * b[0].X;
;  a[1] = n * n * b[1].Y;
;  a[2] = n * n * b[2].X;
;}

; CHECK: Function: Vsub
; CHECK:         BEGIN REGION { }
; CHECK:               %mul = %n  *  %n;
; CHECK:               %mul1 = %mul  *  (%b)[0].0;
; CHECK:               (%a)[0] = %mul1;
; CHECK:               %mul5 = %mul  *  (%b)[1].1;
; CHECK:               (%a)[1] = %mul5;
; CHECK:               %mul10 = %mul  *  (%b)[2].0;
; CHECK:               (%a)[2] = %mul10;
; CHECK:               ret ;
; CHECK:         END REGION

; CHECK: Function: Vsub
; CHECK-NOT: DO_LOOP
; CHECK-NOT: END LOOP

;Module Before HIR
; ModuleID = 'struct.c'
source_filename = "struct.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S = type { double, double }

; Function Attrs: norecurse nounwind uwtable
define dso_local void @Vsub(ptr nocapture %a, ptr nocapture readonly %b, double %n) local_unnamed_addr #0 {
entry:
  %mul = fmul double %n, %n
  %0 = load double, ptr %b, align 8, !tbaa !2
  %mul1 = fmul double %mul, %0
  store double %mul1, ptr %a, align 8, !tbaa !7
  %Y = getelementptr inbounds %struct.S, ptr %b, i64 1, i32 1
  %1 = load double, ptr %Y, align 8, !tbaa !8
  %mul5 = fmul double %mul, %1
  %arrayidx6 = getelementptr inbounds double, ptr %a, i64 1
  store double %mul5, ptr %arrayidx6, align 8, !tbaa !7
  %X9 = getelementptr inbounds %struct.S, ptr %b, i64 2, i32 0
  %2 = load double, ptr %X9, align 8, !tbaa !2
  %mul10 = fmul double %mul, %2
  %arrayidx11 = getelementptr inbounds double, ptr %a, i64 2
  store double %mul10, ptr %arrayidx11, align 8, !tbaa !7
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) 2019.8.1.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"struct@S", !4, i64 0, !4, i64 8}
!4 = !{!"double", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!4, !4, i64 0}
!8 = !{!3, !4, i64 8}
