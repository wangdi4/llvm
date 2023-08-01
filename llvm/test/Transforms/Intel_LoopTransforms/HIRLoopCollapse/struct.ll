; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; check if the given code collaped in presence of references to struct.
;
; struct my_struct {
;   int b[10][20];
; };
;
; struct my_struct A[5];
;
; void foo() {
;   int i, j;
;
;   for (i = 0; i < 10; i++) {
;     for (j = 0; j < 20; j++) {
;       A[1].b[i][j] += 1; // 1st and 2nd dimensions still collapsable.
;     }
;   }
; }
;
; CHECK: Function
;
; CHECK:      BEGIN REGION { }
; CHECK:            + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:            |   + DO i2 = 0, 19, 1   <DO_LOOP>
; CHECK:            |   |   %0 = (@A)[0][1].0[i1][i2];
; CHECK:            |   |   (@A)[0][1].0[i1][i2] = %0 + 1;
; CHECK:            |   + END LOOP
; CHECK:            + END LOOP
; CHECK:      END REGION
;
; CHECK: Function
;
; CHECK:      BEGIN REGION { modified }
; CHECK:            + DO i1 = 0, 199, 1   <DO_LOOP>
; CHECK:            |   %0 = (@A)[0][1].0[0][i1];
; CHECK:            |   (@A)[0][1].0[0][i1] = %0 + 1;
; CHECK:            + END LOOP
; CHECK:      END REGION


source_filename = "struct.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.my_struct = type { [10 x [20 x i32]] }

@A = common local_unnamed_addr global [5 x %struct.my_struct] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc6, %entry
  %indvars.iv16 = phi i64 [ 0, %entry ], [ %indvars.iv.next17, %for.inc6 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [5 x %struct.my_struct], ptr @A, i64 0, i64 1, i32 0, i64 %indvars.iv16, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx5, align 4, !tbaa !2
  %add = add nsw i32 %0, 1
  store i32 %add, ptr %arrayidx5, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 20
  br i1 %exitcond, label %for.inc6, label %for.body3

for.inc6:                                         ; preds = %for.body3
  %indvars.iv.next17 = add nuw nsw i64 %indvars.iv16, 1
  %exitcond18 = icmp eq i64 %indvars.iv.next17, 10
  br i1 %exitcond18, label %for.end8, label %for.body

for.end8:                                         ; preds = %for.inc6
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 756622f1dabd3965f80d20e0d52127a138802cbd) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm d205c8c7c2af5a23339f593c96cf0533b55521d4)"}
!2 = !{!3, !6, i64 0}
!3 = !{!"struct@my_struct", !4, i64 0}
!4 = !{!"array@_ZTSA10_A20_i", !5, i64 0}
!5 = !{!"array@_ZTSA20_i", !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
