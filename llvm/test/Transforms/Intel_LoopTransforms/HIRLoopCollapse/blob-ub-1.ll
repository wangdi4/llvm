; RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; [Analysis]
; Applicable: Y
; Legal:      Y
; Profitable: Y
; Suitable:   Y
; Note: only the relevant part of A[] is transformed (on i2-i3 level)
;
; *** Source Code ***
;void foo(int *A, int N) {
;  int i1, i2, i3;
;  int B = 0;
;
;  //try to collapse the i2..i3 loop?
;  for (i1 = 0; i1 < N; i1++) {
;    B++;
;    if (B % 2) {
;      for (i2 = 0; i2 < N; i2++) {
;        for (i3 = 0; i3 < N; i3++) {
;          A[i1 + N * i2 + i3] = N * i2 + i3;
;        }
;      }
;    }
;  }
;}
;
;
; CHECK: Function
;
; CHECK:    BEGIN REGION { }
; CHECK:        + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>
; CHECK:        |   if (-1 * i1 + 1 != 0)
; CHECK:        |   {
; CHECK:        |      + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>
; CHECK:        |      |   + DO i3 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>
; CHECK:        |      |   |   (%A)[i1 + sext.i32.i64(%N) * i2 + i3] = sext.i32.i64(%N) * i2 + i3;
; CHECK:        |      |   + END LOOP
; CHECK:        |      + END LOOP
; CHECK:        |   }
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>
; CHECK:        |   if (-1 * i1 + 1 != 0)
; CHECK:        |   {
; CHECK:        |      + DO i2 = 0, (sext.i32.i64(%N) * sext.i32.i64(%N)) + -1, 1   <DO_LOOP>
; CHECK:        |      |   (%A)[i1 + i2] = i2;
; CHECK:        |      + END LOOP
; CHECK:        |   }
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr nocapture %A, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp39 = icmp sgt i32 %N, 0
  br i1 %cmp39, label %for.body.lr.ph, label %for.end16

for.body.lr.ph:                                   ; preds = %entry
  %0 = sext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.inc14, %for.body.lr.ph
  %indvars.iv52 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next53, %for.inc14 ]
  %B.043 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.inc14 ]
  %inc = add nuw nsw i32 %B.043, 1
  %rem34 = and i32 %inc, 1
  %tobool = icmp eq i32 %rem34, 0
  br i1 %tobool, label %for.inc14, label %for.body6.lr.ph

for.body6.lr.ph:                                  ; preds = %for.body, %for.inc11
  %indvars.iv46 = phi i64 [ %indvars.iv.next47, %for.inc11 ], [ 0, %for.body ]
  %1 = mul nsw i64 %indvars.iv46, %0
  %2 = add nsw i64 %1, %indvars.iv52
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.body6.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body6.lr.ph ], [ %indvars.iv.next, %for.body6 ]
  %3 = add nsw i64 %indvars.iv, %1
  %4 = add nsw i64 %2, %indvars.iv
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %4
  %5 = trunc i64 %3 to i32
  store i32 %5, ptr %arrayidx, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %0
  br i1 %exitcond, label %for.inc11, label %for.body6

for.inc11:                                        ; preds = %for.body6
  %indvars.iv.next47 = add nuw nsw i64 %indvars.iv46, 1
  %exitcond51 = icmp eq i64 %indvars.iv.next47, %0
  br i1 %exitcond51, label %for.inc14, label %for.body6.lr.ph

for.inc14:                                        ; preds = %for.inc11, %for.body
  %indvars.iv.next53 = add nuw nsw i64 %indvars.iv52, 1
  %exitcond55 = icmp eq i64 %indvars.iv.next53, %0
  br i1 %exitcond55, label %for.end16, label %for.body

for.end16:                                        ; preds = %for.inc14, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (cfe/trunk)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
