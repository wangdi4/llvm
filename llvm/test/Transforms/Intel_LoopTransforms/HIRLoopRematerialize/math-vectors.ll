; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-rematerialize,print<hir>" -aa-pipeline="basic-aa" -hir-allow-loop-materialization-regions=true -disable-output < %s 2>&1 | FileCheck %s

;#if 0
;void project_v3_plane(float v[3], const float n[3], const float p[3])
;{
;  float vector[3];
;  float mul;
;
;  sub_v3_v3v3(vector, v, p);
;  mul = dot_v3v3(vector, n) / len_squared_v3(n);
;
;  mul_v3_v3fl(vector, n, mul);
;
;  sub_v3_v3(v, vector);
;}
;#endif
;
; Inlined into:
;
;void project_v3_plane(float v[3], const float n[3], const float p[3])
;{
;  float vector[3];
;  float mul;
;
;  vector[0] = v[0] - p[0];
;  vector[1] = v[1] - p[1];
;  vector[2] = v[2] - p[2];
;
;  mul = (vector[0]*n[0] +
;        vector[1]*n[1] +
;        vector[2]*n[2]) /
;        (n[0] * n[0] +
;        n[1] * n[1] +
;        n[2] * n[2]);
;
;  vector[0] = n[0] - mul;
;  vector[1] = n[1] - mul;
;  vector[2] = n[2] - mul;
;
;  v[0] -= vector[0];
;  v[1] -= vector[1];
;  v[2] -= vector[2];
;}

;CHECK:Function: project_v3_plane
;CHECK:         BEGIN REGION { }
;CHECK:               %0 = (%v)[0];
;CHECK:               %sub = %0  -  (%p)[0];
;CHECK:               %2 = (%v)[1];
;CHECK:               %sub5 = %2  -  (%p)[1];
;CHECK:               %4 = (%v)[2];
;CHECK:               %sub9 = %4  -  (%p)[2];
;CHECK:               %6 = (%n)[0];
;CHECK:               %mul13 = %sub  *  %6;
;CHECK:               %7 = (%n)[1];
;CHECK:               %mul16 = %sub5  *  %7;
;CHECK:               %add = %mul13  +  %mul16;
;CHECK:               %8 = (%n)[2];
;CHECK:               %mul19 = %sub9  *  %8;
;CHECK:               %add20 = %add  +  %mul19;
;CHECK:               %mul23 = %6  *  %6;
;CHECK:               %mul26 = %7  *  %7;
;CHECK:               %add27 = %mul23  +  %mul26;
;CHECK:               %mul30 = %8  *  %8;
;CHECK:               %add31 = %add27  +  %mul30;
;CHECK:               %div = %add20  /  %add31;
;CHECK:               %sub33 = %6  -  %div;
;CHECK:               %sub36 = %7  -  %div;
;CHECK:               %sub39 = %8  -  %div;
;CHECK:               %sub43 = %0  -  %sub33;
;CHECK:               (%v)[0] = %sub43;
;CHECK:               %sub46 = %2  -  %sub36;
;CHECK:               (%v)[1] = %sub46;
;CHECK:               %sub49 = %4  -  %sub39;
;CHECK:               (%v)[2] = %sub49;
;CHECK:               ret ;
;CHECK:         END REGION

; The whole region should not be materialized into one loop. It may be materialized in TWO loops.
; One before %div = %add20 / %add31 and after. Or only one loop after %div =.
; But these needs quite a sophistication of rematerialization algorithm.
; Currently, we bail out.

;CHECK:Function: project_v3_plane
;CHECK-NOT: DO_LOOP
;CHECK-NOT: END LOOP


;Module Before HIR
; ModuleID = 'math_vecs.c'
source_filename = "math_vecs.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @project_v3_plane(ptr nocapture %v, ptr nocapture readonly %n, ptr nocapture readonly %p) local_unnamed_addr #0 {
entry:
  %0 = load float, ptr %v, align 4, !tbaa !2
  %1 = load float, ptr %p, align 4, !tbaa !2
  %sub = fsub float %0, %1
  %arrayidx3 = getelementptr inbounds float, ptr %v, i64 1
  %2 = load float, ptr %arrayidx3, align 4, !tbaa !2
  %arrayidx4 = getelementptr inbounds float, ptr %p, i64 1
  %3 = load float, ptr %arrayidx4, align 4, !tbaa !2
  %sub5 = fsub float %2, %3
  %arrayidx7 = getelementptr inbounds float, ptr %v, i64 2
  %4 = load float, ptr %arrayidx7, align 4, !tbaa !2
  %arrayidx8 = getelementptr inbounds float, ptr %p, i64 2
  %5 = load float, ptr %arrayidx8, align 4, !tbaa !2
  %sub9 = fsub float %4, %5
  %6 = load float, ptr %n, align 4, !tbaa !2
  %mul13 = fmul float %sub, %6
  %arrayidx15 = getelementptr inbounds float, ptr %n, i64 1
  %7 = load float, ptr %arrayidx15, align 4, !tbaa !2
  %mul16 = fmul float %sub5, %7
  %add = fadd float %mul13, %mul16
  %arrayidx18 = getelementptr inbounds float, ptr %n, i64 2
  %8 = load float, ptr %arrayidx18, align 4, !tbaa !2
  %mul19 = fmul float %sub9, %8
  %add20 = fadd float %add, %mul19
  %mul23 = fmul float %6, %6
  %mul26 = fmul float %7, %7
  %add27 = fadd float %mul23, %mul26
  %mul30 = fmul float %8, %8
  %add31 = fadd float %add27, %mul30
  %div = fdiv float %add20, %add31
  %sub33 = fsub float %6, %div
  %sub36 = fsub float %7, %div
  %sub39 = fsub float %8, %div
  %sub43 = fsub float %0, %sub33
  store float %sub43, ptr %v, align 4, !tbaa !2
  %sub46 = fsub float %2, %sub36
  store float %sub46, ptr %arrayidx3, align 4, !tbaa !2
  %sub49 = fsub float %4, %sub39
  store float %sub49, ptr %arrayidx7, align 4, !tbaa !2
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) 2019.8.1.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
