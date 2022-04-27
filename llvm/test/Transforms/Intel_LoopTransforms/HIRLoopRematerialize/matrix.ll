; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-rematerialize -print-before=hir-loop-rematerialize -print-after=hir-loop-rematerialize < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-rematerialize,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

;#if 0
;void bone_matrix_translate_y(float mat[4][4], float y)
;{
;  float trans[3];
;
;  copy_v3_v3(trans, mat[1]);
;  mul_v3_fl(trans, y);
;  add_v3_v3(mat[3], trans);
;}
;#endif
;
;void bone_matrix_translate_y(float mat[4][4], float y) {
;  float trans[3];
;  trans[0] = mat[1][0];
;  trans[1] = mat[1][1];
;  trans[2] = mat[1][2];
;
;  trans[0] *= y;
;  trans[1] *= y;
;  trans[2] *= y;
;
;  mat[3][0] += trans[0];
;  mat[3][1] += trans[1];
;  mat[3][2] += trans[2];
;}

; CHECK:Function: bone_matrix_translate_y
; CHECK:         BEGIN REGION { }
; CHECK:               %mul = (%mat)[1][0]  *  %y;
; CHECK:               %mul11 = (%mat)[1][1]  *  %y;
; CHECK:               %mul13 = (%mat)[1][2]  *  %y;
; CHECK:               %add = %mul  +  (%mat)[3][0];
; CHECK:               (%mat)[3][0] = %add;
; CHECK:               %add20 = %mul11  +  (%mat)[3][1];
; CHECK:               (%mat)[3][1] = %add20;
; CHECK:               %add24 = %mul13  +  (%mat)[3][2];
; CHECK:               (%mat)[3][2] = %add24;
; CHECK:               ret ;
; CHECK:         END REGION

; CHECK:Function: bone_matrix_translate_y
; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK:               |   %mul = (%mat)[1][i1]  *  %y;
; CHECK:               |   %add = %mul  +  (%mat)[3][i1];
; CHECK:               |   (%mat)[3][i1] = %add;
; CHECK:               + END LOOP

; CHECK:               ret ;
; CHECK:         END REGION

; Check the opt report remarks of loop rematerialization.

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-rematerialize -hir-cg -intel-opt-report=low -intel-ir-optreport-emitter -simplifycfg -force-hir-cg 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-rematerialize,hir-cg,simplifycfg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -intel-opt-report=low -force-hir-cg  < %s 2>&1 | FileCheck %s -check-prefix=OPTREPORT

; OPTREPORT: LOOP BEGIN
; OPTREPORT:    remark #25397: Materialized a loop with a trip count of 3
; OPTREPORT: LOOP END

;Module Before HIR
; ModuleID = 'matrix.c'
source_filename = "matrix.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @bone_matrix_translate_y([4 x float]* noalias nocapture %mat, float %y) local_unnamed_addr #0 {
entry:
  %arrayidx1 = getelementptr inbounds [4 x float], [4 x float]* %mat, i64 1, i64 0
  %0 = load float, float* %arrayidx1, align 4, !tbaa !2
  %arrayidx4 = getelementptr inbounds [4 x float], [4 x float]* %mat, i64 1, i64 1
  %1 = load float, float* %arrayidx4, align 4, !tbaa !2
  %arrayidx7 = getelementptr inbounds [4 x float], [4 x float]* %mat, i64 1, i64 2
  %2 = load float, float* %arrayidx7, align 4, !tbaa !2
  %mul = fmul float %0, %y
  %mul11 = fmul float %1, %y
  %mul13 = fmul float %2, %y
  %arrayidx16 = getelementptr inbounds [4 x float], [4 x float]* %mat, i64 3, i64 0
  %3 = load float, float* %arrayidx16, align 4, !tbaa !2
  %add = fadd float %mul, %3
  store float %add, float* %arrayidx16, align 4, !tbaa !2
  %arrayidx19 = getelementptr inbounds [4 x float], [4 x float]* %mat, i64 3, i64 1
  %4 = load float, float* %arrayidx19, align 4, !tbaa !2
  %add20 = fadd float %mul11, %4
  store float %add20, float* %arrayidx19, align 4, !tbaa !2
  %arrayidx23 = getelementptr inbounds [4 x float], [4 x float]* %mat, i64 3, i64 2
  %5 = load float, float* %arrayidx23, align 4, !tbaa !2
  %add24 = fadd float %mul13, %5
  store float %add24, float* %arrayidx23, align 4, !tbaa !2
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) 2019.8.1.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA4_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
