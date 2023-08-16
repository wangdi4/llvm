; Source code
;
;float A[100][100];
;void foo(){
;  int i, j;
;  for(i = 0; i < 100; ++i){
;    for(j = 0; j < 100; ++j){
;      if(i == j){
;        A[i][j] = 1.0;
;      }else{
;        A[i][j] = 0.0;
;      }
;    }
;  }
;}
;
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-identity-matrix-idiom-recognition,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s -disable-output | FileCheck %s

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-identity-matrix-idiom-recognition" -print-changed -disable-output  2>&1 < %s -disable-output | FileCheck %s --check-prefix=CHECK-CHANGED
;
;*** IR Dump Before HIR Identity Matrix Idiom Recognition ***
;Function: foo
;
;<0>          BEGIN REGION { }
;<22>               + DO i1 = 0, 99, 1   <DO_LOOP>
;<23>               |   + DO i2 = 0, 99, 1   <DO_LOOP>
;<7>                |   |   %. = (i1 == i2) ? 1.000000e+00 : 0.000000e+00;
;<8>                |   |   (@A)[0][i1][i2] = %.;
;<23>               |   + END LOOP
;<22>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Identity Matrix Idiom Recognition ***
;Function: foo
;
; CHECK:    BEGIN REGION { }
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   (@A)[0][i1][i2] = 0.000000e+00;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
;
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   (@A)[0][i1][i1] = 1.000000e+00;
; CHECK:           + END LOOP
; CHECK:     END REGION
;

; Verify that pass is dumped with print-changed when it triggers.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED: Dump After HIRIdentityMatrixIdiomRecognition

; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x [100 x float]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc11, %entry
  %indvars.iv23 = phi i64 [ 0, %entry ], [ %indvars.iv.next24, %for.inc11 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %cmp4 = icmp eq i64 %indvars.iv23, %indvars.iv
  %arrayidx6 = getelementptr inbounds [100 x [100 x float]], ptr @A, i64 0, i64 %indvars.iv23, i64 %indvars.iv
  %. = select i1 %cmp4, float 1.000000e+00, float 0.000000e+00
  store float %., ptr %arrayidx6, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.inc11, label %for.body3

for.inc11:                                        ; preds = %for.body3
  %indvars.iv.next24 = add nuw nsw i64 %indvars.iv23, 1
  %exitcond25 = icmp eq i64 %indvars.iv.next24, 100
  br i1 %exitcond25, label %for.end13, label %for.cond1.preheader

for.end13:                                        ; preds = %for.inc11
  ret void
}

attributes #0 = { norecurse nounwind uwtable writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) 2019.8.1.0"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA100_A100_f", !4, i64 0}
!4 = !{!"array@_ZTSA100_f", !5, i64 0}
!5 = !{!"float", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
