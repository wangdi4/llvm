; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; HIR Loop Collapse Sanity Test: access a variable array with possible disjoint point(s)
;
; [Analysis]
; Applicable: YES
; Legal:      NO
; Profitable: N/A
; Suitable:   N/A
;
;
; *** Source Code ***
;
;int foo(unsigned O, unsigned P, unsigned Q) {
;  int A[O][P][Q];       // Declare a variable length array
;  int B[100][100][100]; // Declare a fix-length array
;  unsigned i, j, k;
;
;  for (i = 0; i < O; ++i) {
;    for (j = 0; j < P; ++j) {
;      for (k = 0; k < Q; ++k) {
;        A[i][5][k] = 1; // can't collapse, disjoint between i and k level
;      }
;    }
;  }
;
;  for (i = 0; i < O; ++i) {
;    for (j = 0; j < P; ++j) {
;      for (k = 0; k < Q; ++k) {
;        A[2][j][k] = 1; // can collapse j and k level
;      }
;    }
;  }
;
;  for (i = 0; i < O; ++i) {
;    for (j = 0; j < P; ++j) {
;      for (k = 0; k < Q; ++k) {
;        A[i][j][2] = 1; // can collapse, i-j levels are continuous, but leave it out for now: not starting from Innermost level
;      }
;    }
;  }
;
;  return A[0][0][0] + B[1][1][1] + 1;
;}
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, zext.i32.i64(%O) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK:        |   + DO i2 = 0, %P + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK:        |   |   + DO i3 = 0, zext.i32.i64(%Q) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK:        |   |   |   (%vla)[(zext.i32.i64(%P) * zext.i32.i64(%Q)) * i1 + i3 + 5 * zext.i32.i64(%Q)] = 1;
; CHECK:        |   |   + END LOOP
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, %O + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK:        |   + DO i2 = 0, zext.i32.i64(%P) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK:        |   |   + DO i3 = 0, zext.i32.i64(%Q) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK:        |   |   |   (%vla)[zext.i32.i64(%Q) * i2 + i3 + 2 * (zext.i32.i64(%P) * zext.i32.i64(%Q))] = 1;
; CHECK:        |   |   + END LOOP
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, zext.i32.i64(%O) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK:        |   + DO i2 = 0, zext.i32.i64(%P) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK:        |   |   if (%Q != 0)
; CHECK:        |   |   {
; CHECK:        |   |      (%vla)[(zext.i32.i64(%P) * zext.i32.i64(%Q)) * i1 + zext.i32.i64(%Q) * i2 + 2] = 1;
; CHECK:        |   |   }
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, zext.i32.i64(%O) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK:        |   + DO i2 = 0, %P + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK:        |   |   + DO i3 = 0, zext.i32.i64(%Q) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK:        |   |   |   (%vla)[(zext.i32.i64(%P) * zext.i32.i64(%Q)) * i1 + i3 + 5 * zext.i32.i64(%Q)] = 1;
; CHECK:        |   |   + END LOOP
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
; CHECK:    BEGIN REGION { modified }
; CHECK:          + DO i1 = 0, %O + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK:          |   + DO i2 = 0, (zext.i32.i64(%P) * zext.i32.i64(%Q)) + -1, 1   <DO_LOOP>
; CHECK:          |   |   (%vla)[i2 + 2 * (zext.i32.i64(%P) * zext.i32.i64(%Q))] = 1;
; CHECK:          |   + END LOOP
; CHECK:          + END LOOP
; CHECK:    END REGION
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, zext.i32.i64(%O) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK:        |   + DO i2 = 0, zext.i32.i64(%P) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK:        |   |   if (%Q != 0)
; CHECK:        |   |   {
; CHECK:        |   |      (%vla)[(zext.i32.i64(%P) * zext.i32.i64(%Q)) * i1 + zext.i32.i64(%Q) * i2 + 2] = 1;
; CHECK:        |   |   }
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

; Function Attrs: norecurse nounwind readnone uwtable
; Function Attrs: norecurse nounwind readnone uwtable
define i32 @foo(i32 %O, i32 %P, i32 %Q) local_unnamed_addr #0 {
entry:
  %0 = zext i32 %O to i64
  %1 = zext i32 %P to i64
  %2 = zext i32 %Q to i64
  %3 = mul nuw i64 %1, %0
  %4 = mul nuw i64 %3, %2
  %vla = alloca i32, i64 %4, align 16
  %cmp115 = icmp eq i32 %O, 0
  br i1 %cmp115, label %for.end61, label %for.cond1.preheader.lr.ph

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp2113 = icmp eq i32 %P, 0
  %cmp5111 = icmp eq i32 %Q, 0
  %5 = mul nuw i64 %2, %1
  %6 = mul nuw nsw i64 %2, 5
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc13, %for.cond1.preheader.lr.ph
  %indvars.iv135 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next136, %for.inc13 ]
  br i1 %cmp2113, label %for.inc13, label %for.cond4.preheader.lr.ph

for.cond4.preheader.lr.ph:                        ; preds = %for.cond1.preheader
  %7 = mul nsw i64 %5, %indvars.iv135
  %arrayidx = getelementptr inbounds i32, ptr %vla, i64 %7
  %arrayidx7 = getelementptr inbounds i32, ptr %arrayidx, i64 %6
  br label %for.cond4.preheader

for.cond16.preheader:                             ; preds = %for.inc13
  br i1 %cmp115, label %for.end61, label %for.cond19.preheader.lr.ph

for.cond19.preheader.lr.ph:                       ; preds = %for.cond16.preheader
  %cmp20107 = icmp eq i32 %P, 0
  %cmp23105 = icmp eq i32 %Q, 0
  %8 = shl nuw nsw i64 %1, 1
  %9 = mul i64 %8, %2
  %arrayidx25 = getelementptr inbounds i32, ptr %vla, i64 %9
  br label %for.cond19.preheader

for.cond4.preheader:                              ; preds = %for.inc10, %for.cond4.preheader.lr.ph
  %j.0114 = phi i32 [ 0, %for.cond4.preheader.lr.ph ], [ %inc11, %for.inc10 ]
  br i1 %cmp5111, label %for.inc10, label %for.body6.preheader

for.body6.preheader:                              ; preds = %for.cond4.preheader
  br label %for.body6

for.body6:                                        ; preds = %for.body6.preheader, %for.body6
  %indvars.iv130 = phi i64 [ %indvars.iv.next131, %for.body6 ], [ 0, %for.body6.preheader ]
  %arrayidx9 = getelementptr inbounds i32, ptr %arrayidx7, i64 %indvars.iv130
  store i32 1, ptr %arrayidx9, align 4, !tbaa !1
  %indvars.iv.next131 = add nuw nsw i64 %indvars.iv130, 1
  %exitcond133 = icmp eq i64 %indvars.iv.next131, %2
  br i1 %exitcond133, label %for.inc10.loopexit, label %for.body6

for.inc10.loopexit:                               ; preds = %for.body6
  br label %for.inc10

for.inc10:                                        ; preds = %for.inc10.loopexit, %for.cond4.preheader
  %inc11 = add nuw i32 %j.0114, 1
  %exitcond134 = icmp eq i32 %inc11, %P
  br i1 %exitcond134, label %for.inc13.loopexit, label %for.cond4.preheader

for.inc13.loopexit:                               ; preds = %for.inc10
  br label %for.inc13

for.inc13:                                        ; preds = %for.inc13.loopexit, %for.cond1.preheader
  %indvars.iv.next136 = add nuw nsw i64 %indvars.iv135, 1
  %exitcond138 = icmp eq i64 %indvars.iv.next136, %0
  br i1 %exitcond138, label %for.cond16.preheader, label %for.cond1.preheader

for.cond19.preheader:                             ; preds = %for.inc36, %for.cond19.preheader.lr.ph
  %i.1110 = phi i32 [ 0, %for.cond19.preheader.lr.ph ], [ %inc37, %for.inc36 ]
  br i1 %cmp20107, label %for.inc36, label %for.cond22.preheader.preheader

for.cond22.preheader.preheader:                   ; preds = %for.cond19.preheader
  br label %for.cond22.preheader

for.cond39.preheader:                             ; preds = %for.inc36
  br i1 %cmp115, label %for.end61, label %for.cond42.preheader.lr.ph

for.cond42.preheader.lr.ph:                       ; preds = %for.cond39.preheader
  %cmp43101 = icmp eq i32 %P, 0
  %cmp4699 = icmp eq i32 %Q, 0
  %10 = mul nuw i64 %2, %1
  br label %for.cond42.preheader

for.cond22.preheader:                             ; preds = %for.cond22.preheader.preheader, %for.inc33
  %indvars.iv125 = phi i64 [ %indvars.iv.next126, %for.inc33 ], [ 0, %for.cond22.preheader.preheader ]
  br i1 %cmp23105, label %for.inc33, label %for.body24.lr.ph

for.body24.lr.ph:                                 ; preds = %for.cond22.preheader
  %11 = mul nuw nsw i64 %indvars.iv125, %2
  %arrayidx27 = getelementptr inbounds i32, ptr %arrayidx25, i64 %11
  br label %for.body24

for.body24:                                       ; preds = %for.body24, %for.body24.lr.ph
  %indvars.iv121 = phi i64 [ 0, %for.body24.lr.ph ], [ %indvars.iv.next122, %for.body24 ]
  %arrayidx29 = getelementptr inbounds i32, ptr %arrayidx27, i64 %indvars.iv121
  store i32 1, ptr %arrayidx29, align 4, !tbaa !1
  %indvars.iv.next122 = add nuw nsw i64 %indvars.iv121, 1
  %exitcond124 = icmp eq i64 %indvars.iv.next122, %2
  br i1 %exitcond124, label %for.inc33.loopexit, label %for.body24

for.inc33.loopexit:                               ; preds = %for.body24
  br label %for.inc33

for.inc33:                                        ; preds = %for.inc33.loopexit, %for.cond22.preheader
  %indvars.iv.next126 = add nuw nsw i64 %indvars.iv125, 1
  %exitcond128 = icmp eq i64 %indvars.iv.next126, %1
  br i1 %exitcond128, label %for.inc36.loopexit, label %for.cond22.preheader

for.inc36.loopexit:                               ; preds = %for.inc33
  br label %for.inc36

for.inc36:                                        ; preds = %for.inc36.loopexit, %for.cond19.preheader
  %inc37 = add nuw i32 %i.1110, 1
  %exitcond129 = icmp eq i32 %inc37, %O
  br i1 %exitcond129, label %for.cond39.preheader, label %for.cond19.preheader

for.cond42.preheader:                             ; preds = %for.inc59, %for.cond42.preheader.lr.ph
  %indvars.iv117 = phi i64 [ 0, %for.cond42.preheader.lr.ph ], [ %indvars.iv.next118, %for.inc59 ]
  br i1 %cmp43101, label %for.inc59, label %for.cond45.preheader.lr.ph

for.cond45.preheader.lr.ph:                       ; preds = %for.cond42.preheader
  %12 = mul nsw i64 %10, %indvars.iv117
  %arrayidx49 = getelementptr inbounds i32, ptr %vla, i64 %12
  br label %for.cond45.preheader

for.cond45.preheader:                             ; preds = %for.inc56, %for.cond45.preheader.lr.ph
  %indvars.iv = phi i64 [ 0, %for.cond45.preheader.lr.ph ], [ %indvars.iv.next, %for.inc56 ]
  br i1 %cmp4699, label %for.inc56, label %for.body47.lr.ph

for.body47.lr.ph:                                 ; preds = %for.cond45.preheader
  %13 = mul nuw nsw i64 %indvars.iv, %2
  %arrayidx51 = getelementptr inbounds i32, ptr %arrayidx49, i64 %13
  %arrayidx52 = getelementptr inbounds i32, ptr %arrayidx51, i64 2
  store i32 1, ptr %arrayidx52, align 4, !tbaa !1
  br label %for.inc56

for.inc56:                                        ; preds = %for.cond45.preheader, %for.body47.lr.ph
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %1
  br i1 %exitcond, label %for.inc59.loopexit, label %for.cond45.preheader

for.inc59.loopexit:                               ; preds = %for.inc56
  br label %for.inc59

for.inc59:                                        ; preds = %for.inc59.loopexit, %for.cond42.preheader
  %indvars.iv.next118 = add nuw nsw i64 %indvars.iv117, 1
  %exitcond120 = icmp eq i64 %indvars.iv.next118, %0
  br i1 %exitcond120, label %for.end61.loopexit, label %for.cond42.preheader

for.end61.loopexit:                               ; preds = %for.inc59
  br label %for.end61

for.end61:                                        ; preds = %for.end61.loopexit, %entry, %for.cond16.preheader, %for.cond39.preheader
  %14 = load i32, ptr %vla, align 16, !tbaa !1
  %add = add nsw i32 %14, 1
  ret i32 %add
}

; Function Attrs: nounwind
declare ptr @llvm.stacksave() #1

; Function Attrs: nounwind
declare void @llvm.stackrestore(ptr) #1

attributes #0 = { norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (trunk 21400) (llvm/branches/loopopt 21452)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
