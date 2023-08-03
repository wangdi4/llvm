; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-collapse" -print-changed -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED

;
; *** Source Code ***
; int A[10][20][3];
; void foo() {
;   int i, j, k;
;
;   for (i = 0; i < 10; i++) {
;     for (j = 0; j < 20; j++) {
;      for (k = 0; k < 3; k++) {
;       A[i & 7][j][k] += 1;  // & 7 makes i casted into "i3" type,
;                              // only i2-i2 are collapsable
;      }
;     }
;   }
; }
;
; CHECK: Function
;
; CHECK:       BEGIN REGION { }
; CHECK:             + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:             |   + DO i2 = 0, 19, 1   <DO_LOOP>
; CHECK:             |   |   + DO i3 = 0, 2, 1   <DO_LOOP>
; CHECK:             |   |   |   %1 = (@A)[0][i1][i2][i3];
; CHECK:             |   |   |   (@A)[0][i1][i2][i3] = %1 + 1;
; CHECK:             |   |   + END LOOP
; CHECK:             |   + END LOOP
; CHECK:             + END LOOP
; CHECK:       END REGION
;
; CHECK: Function
;
; CHECK:      BEGIN REGION { modified }
; CHECK:            + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:            |   + DO i2 = 0, 59, 1   <DO_LOOP>
; CHECK:            |   |   %1 = (@A)[0][i1][0][i2];
; CHECK:            |   |   (@A)[0][i1][0][i2] = %1 + 1;
; CHECK:            |   + END LOOP
; CHECK:            + END LOOP
; CHECK:      END REGION

; Verify that pass is dumped with print-changed when it triggers.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED: Dump After HIRLoopCollapse

source_filename = "type-valid.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global [10 x [20 x [3 x i32]]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc14, %entry
  %i.028 = phi i32 [ 0, %entry ], [ %inc15, %for.inc14 ]
  %and = and i32 %i.028, 7
  %0 = zext i32 %and to i64
  br label %for.body3

for.body3:                                        ; preds = %for.inc11, %for.body
  %indvars.iv29 = phi i64 [ 0, %for.body ], [ %indvars.iv.next30, %for.inc11 ]
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.body3
  %indvars.iv = phi i64 [ 0, %for.body3 ], [ %indvars.iv.next, %for.body6 ]
  %arrayidx10 = getelementptr inbounds [10 x [20 x [3 x i32]]], ptr @A, i64 0, i64 %0, i64 %indvars.iv29, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx10, align 4, !tbaa !2
  %add = add nsw i32 %1, 1
  store i32 %add, ptr %arrayidx10, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 3
  br i1 %exitcond, label %for.inc11, label %for.body6

for.inc11:                                        ; preds = %for.body6
  %indvars.iv.next30 = add nuw nsw i64 %indvars.iv29, 1
  %exitcond31 = icmp eq i64 %indvars.iv.next30, 20
  br i1 %exitcond31, label %for.inc14, label %for.body3

for.inc14:                                        ; preds = %for.inc11
  %inc15 = add nuw nsw i32 %i.028, 1
  %exitcond32 = icmp eq i32 %inc15, 10
  br i1 %exitcond32, label %for.end16, label %for.body

for.end16:                                        ; preds = %for.inc14
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 756622f1dabd3965f80d20e0d52127a138802cbd) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm d205c8c7c2af5a23339f593c96cf0533b55521d4)"}
!2 = !{!3, !6, i64 0}
!3 = !{!"array@_ZTSA10_A20_A3_i", !4, i64 0}
!4 = !{!"array@_ZTSA20_A3_i", !5, i64 0}
!5 = !{!"array@_ZTSA3_i", !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
