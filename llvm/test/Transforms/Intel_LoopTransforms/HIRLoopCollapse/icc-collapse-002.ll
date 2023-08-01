; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; HIR Loop Collapse Sanity Test: testcase is simplified from icc's collaps_002.c test file.
; This testcase won't trigger loop collapse.
;
; *** Source Code ***
;int a[20][20][50];
;
;void foo(int n){
;  int i, j, k;
;
;  for(i=0; i < 20; i++){
;    for(j=0; j < 20; j++){
;      for(k=0; k < 50; k++){
;        a[i][j][k] = k - i + 1;
;      }
;    }
;  }
;
;}
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 19, 1   <DO_LOOP>
; CHECK:        |   |   + DO i3 = 0, 49, 1   <DO_LOOP>
; CHECK:        |   |   |   (@a)[0][i1][i2][i3] = -1 * i1 + i3 + 1;
; CHECK:        |   |   + END LOOP
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 19, 1   <DO_LOOP>
; CHECK:        |   |   + DO i3 = 0, 49, 1   <DO_LOOP>
; CHECK:        |   |   |   (@a)[0][i1][i2][i3] = -1 * i1 + i3 + 1;
; CHECK:        |   |   + END LOOP
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common local_unnamed_addr global [20 x [20 x [50 x i32]]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc14, %entry
  %indvars.iv35 = phi i64 [ 0, %entry ], [ %indvars.iv.next36, %for.inc14 ]
  %0 = sub nsw i64 1, %indvars.iv35
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc11, %for.cond1.preheader
  %indvars.iv32 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next33, %for.inc11 ]
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %1 = add nsw i64 %0, %indvars.iv
  %arrayidx10 = getelementptr inbounds [20 x [20 x [50 x i32]]], ptr @a, i64 0, i64 %indvars.iv35, i64 %indvars.iv32, i64 %indvars.iv
  %2 = trunc i64 %1 to i32
  store i32 %2, ptr %arrayidx10, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 50
  br i1 %exitcond, label %for.inc11, label %for.body6

for.inc11:                                        ; preds = %for.body6
  %indvars.iv.next33 = add nuw nsw i64 %indvars.iv32, 1
  %exitcond34 = icmp eq i64 %indvars.iv.next33, 20
  br i1 %exitcond34, label %for.inc14, label %for.cond4.preheader

for.inc14:                                        ; preds = %for.inc11
  %indvars.iv.next36 = add nuw nsw i64 %indvars.iv35, 1
  %exitcond38 = icmp eq i64 %indvars.iv.next36, 20
  br i1 %exitcond38, label %for.end16, label %for.cond1.preheader

for.end16:                                        ; preds = %for.inc14
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (trunk 21400) (llvm/branches/loopopt 21436)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
