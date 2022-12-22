; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; *** Source Code ***
; int A[10][20];
; void foo() {
;   int i, j;
;
;   for (i = 0; i < 10; i++) {
;     for (j = 0; j < 20; j++) {
;       A[i][j & 7] += 1;  // & 7 makes j casted into "i3" type
;     }
;   }
; }
;
; IV at level 2 (i.e. i2) has llvm dest type "i3" dut to & 7.
; Its src type is i32 or i64. Src and dest types are different.
; Collapsing is not done as a result.
;
;
; CHECK: Function
; CHECK:       BEGIN REGION { }
; CHECK:             + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:             |   + DO i2 = 0, 19, 1   <DO_LOOP>
; CHECK:             |   |   %1 = (@A)[0][i1][i2];
; CHECK:             |   |   (@A)[0][i1][i2] = %1 + 1;
; CHECK:             |   + END LOOP
; CHECK:             + END LOOP
; CHECK:       END REGION
;
; CHECK: Function
; CHECK:        BEGIN REGION { }
; CHECK:             + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:             |   + DO i2 = 0, 19, 1   <DO_LOOP>
; CHECK:             |   |   %1 = (@A)[0][i1][i2];
; CHECK:             |   |   (@A)[0][i1][i2] = %1 + 1;
; CHECK:             |   + END LOOP
; CHECK:             + END LOOP
; CHECK:        END REGION
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "new.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global [10 x [20 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc6, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc6 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body
  %j.014 = phi i32 [ 0, %for.body ], [ %inc, %for.body3 ]
  %and = and i32 %j.014, 7
  %0 = zext i32 %and to i64
  %arrayidx5 = getelementptr inbounds [10 x [20 x i32]], [10 x [20 x i32]]* @A, i64 0, i64 %indvars.iv, i64 %0
  %1 = load i32, i32* %arrayidx5, align 4, !tbaa !2
  %add = add nsw i32 %1, 1
  store i32 %add, i32* %arrayidx5, align 4, !tbaa !2
  %inc = add nuw nsw i32 %j.014, 1
  %exitcond = icmp eq i32 %inc, 20
  br i1 %exitcond, label %for.inc6, label %for.body3

for.inc6:                                         ; preds = %for.body3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond16 = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond16, label %for.end8, label %for.body

for.end8:                                         ; preds = %for.inc6
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 756622f1dabd3965f80d20e0d52127a138802cbd) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm d205c8c7c2af5a23339f593c96cf0533b55521d4)"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA10_A20_i", !4, i64 0}
!4 = !{!"array@_ZTSA20_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
