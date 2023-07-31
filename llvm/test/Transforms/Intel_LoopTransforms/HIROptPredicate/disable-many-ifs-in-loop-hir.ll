; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -hir-opt-predicate-max-ifs-in-loop-threshold=2 -aa-pipeline="basic-aa" -disable-output -debug-only=hir-opt-predicate < %s 2>&1 | FileCheck %s

; This test case checks that opt-predicate wasn't applied because the number of
; conditionals inside the loopnest after applying the optimization for the
; first is larger that the threshold set by
; hir-opt-predicate-max-ifs-in-loop-threshold (default is 8, test case use 2).

; Original code in C/C++

; void foo(int *a, int n, int m, int q, int p) {
;   for(int i = 0; i < n; i++) {
;     if (p < 10) {
;       for (int j= 0; j < m; j++) {
;         for (int k = 0; k < q; k++) {
;           if (q < 50)
;             a[k]+= j;
;           else if (q < 60)
;             a[k]+= j + 2;
;           else if (q < 10)
;             a[k]+= i + 3;
;           else if (q < 70)
;             a[k]+= k + 4;
;           else
;             a[k]+= i;
;         }
;       }
;     } else {
;       for (int j= 0; j < q; j++) {
;         for (int k = 0; k < m; k++) {
;           if (m < 50)
;             a[k]+= j;
;           else if (m < 60)
;             a[k]+= j + 2;
;           else if (m < 10)
;             a[k]+= i + 3;
;           else if (m < 70)
;             a[k]+= k + 4;
;           else
;             a[k]+= i;
;         }
;       }
;     }
;   }
; }

; HIR before optimization is applied

; BEGIN REGION { }
;       + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   if (%p < 10)
;       |   {
;       |      + DO i2 = 0, %m + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |      |   + DO i3 = 0, zext.i32.i64(%q) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |      |   |   if (%q < 50)
;       |      |   |   {
;       |      |   |      %0 = (%a)[i3];
;       |      |   |      (%a)[i3] = i2 + %0;
;       |      |   |   }
;       |      |   |   else
;       |      |   |   {
;       |      |   |      if (%q <u 60)
;       |      |   |      {
;       |      |   |         %1 = (%a)[i3];
;       |      |   |         (%a)[i3] = i2 + %1 + 2;
;       |      |   |      }
;       |      |   |      else
;       |      |   |      {
;       |      |   |         if (%q <u 70)
;       |      |   |         {
;       |      |   |            %2 = (%a)[i3];
;       |      |   |            (%a)[i3] = i3 + %2 + 4;
;       |      |   |         }
;       |      |   |         else
;       |      |   |         {
;       |      |   |            %5 = (%a)[i3];
;       |      |   |            (%a)[i3] = i1 + %5;
;       |      |   |         }
;       |      |   |      }
;       |      |   |   }
;       |      |   + END LOOP
;       |      + END LOOP
;       |   }
;       |   else
;       |   {
;       |      + DO i2 = 0, %q + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |      |   + DO i3 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |      |   |   if (%m < 50)
;       |      |   |   {
;       |      |   |      %6 = (%a)[i3];
;       |      |   |      (%a)[i3] = i2 + %6;
;       |      |   |   }
;       |      |   |   else
;       |      |   |   {
;       |      |   |      if (%m <u 60)
;       |      |   |      {
;       |      |   |         %7 = (%a)[i3];
;       |      |   |         (%a)[i3] = i2 + %7 + 2;
;       |      |   |      }
;       |      |   |      else
;       |      |   |      {
;       |      |   |         if (%m <u 70)
;       |      |   |         {
;       |      |   |            %8 = (%a)[i3];
;       |      |   |            (%a)[i3] = i3 + %8 + 4;
;       |      |   |         }
;       |      |   |         else
;       |      |   |         {
;       |      |   |            %11 = (%a)[i3];
;       |      |   |            (%a)[i3] = i1 + %11;
;       |      |   |         }
;       |      |   |      }
;       |      |   |   }
;       |      |   + END LOOP
;       |      + END LOOP
;       |   }
;       + END LOOP
; END REGION

; Check that only the first conditional (%p < 10) was hoisted


; CHECK: Unswitching loop <154>:
; CHECK: H: {<2>          if (%p < 10), L: 0, PU: [ F/F ]}

; CHECK: Skipping opportunity because the number of Ifs in loop is larger than max allowed (3 > 2)
; CHECK: Unswitching loop <156>:
; CHECK: Skipping opportunity because the number of Ifs in loop is larger than max allowed (3 > 2)
; CHECK: Unswitching loop <156>:
; CHECK: Skipping opportunity because the number of Ifs in loop is larger than max allowed (3 > 2)
; CHECK: Unswitching loop <158>:
; CHECK: Skipping opportunity because the number of Ifs in loop is larger than max allowed (3 > 2)
; CHECK: Unswitching loop <158>:
; CHECK: Skipping opportunity because the number of Ifs in loop is larger than max allowed (3 > 2)
; CHECK: Unswitching loop <158>:
; CHECK: Skipping opportunity because the number of Ifs in loop is larger than max allowed (3 > 2)

; CHECK:    BEGIN REGION { modified }
; CHECK:          if (%p < 10)
; CHECK:          {
; CHECK:             + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:             |   + DO i2 = 0, %m + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:             |   |   + DO i3 = 0, zext.i32.i64(%q) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:             |   |   |   if (%q < 50) <no_unswitch>
; CHECK:             |   |   |   {
; CHECK:             |   |   |      %0 = (%a)[i3];
; CHECK:             |   |   |      (%a)[i3] = i2 + %0;
; CHECK:             |   |   |   }
; CHECK:             |   |   |   else
; CHECK:             |   |   |   {
; CHECK:             |   |   |      if (%q <u 60) <no_unswitch>
; CHECK:             |   |   |      {
; CHECK:             |   |   |         %1 = (%a)[i3];
; CHECK:             |   |   |         (%a)[i3] = i2 + %1 + 2;
; CHECK:             |   |   |      }
; CHECK:             |   |   |      else
; CHECK:             |   |   |      {
; CHECK:             |   |   |         if (%q <u 70) <no_unswitch>
; CHECK:             |   |   |         {
; CHECK:             |   |   |            %2 = (%a)[i3];
; CHECK:             |   |   |            (%a)[i3] = i3 + %2 + 4;
; CHECK:             |   |   |         }
; CHECK:             |   |   |         else
; CHECK:             |   |   |         {
; CHECK:             |   |   |            %5 = (%a)[i3];
; CHECK:             |   |   |            (%a)[i3] = i1 + %5;
; CHECK:             |   |   |         }
; CHECK:             |   |   |      }
; CHECK:             |   |   |   }
; CHECK:             |   |   + END LOOP
; CHECK:             |   + END LOOP
; CHECK:             + END LOOP
; CHECK:          }
; CHECK:          else
; CHECK:          {
; CHECK:             + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:             |   + DO i2 = 0, %q + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:             |   |   + DO i3 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:             |   |   |   if (%m < 50) <no_unswitch>
; CHECK:             |   |   |   {
; CHECK:             |   |   |      %6 = (%a)[i3];
; CHECK:             |   |   |      (%a)[i3] = i2 + %6;
; CHECK:             |   |   |   }
; CHECK:             |   |   |   else
; CHECK:             |   |   |   {
; CHECK:             |   |   |      if (%m <u 60) <no_unswitch>
; CHECK:             |   |   |      {
; CHECK:             |   |   |         %7 = (%a)[i3];
; CHECK:             |   |   |         (%a)[i3] = i2 + %7 + 2;
; CHECK:             |   |   |      }
; CHECK:             |   |   |      else
; CHECK:             |   |   |      {
; CHECK:             |   |   |         if (%m <u 70) <no_unswitch>
; CHECK:             |   |   |         {
; CHECK:             |   |   |            %8 = (%a)[i3];
; CHECK:             |   |   |            (%a)[i3] = i3 + %8 + 4;
; CHECK:             |   |   |         }
; CHECK:             |   |   |         else
; CHECK:             |   |   |         {
; CHECK:             |   |   |            %11 = (%a)[i3];
; CHECK:             |   |   |            (%a)[i3] = i1 + %11;
; CHECK:             |   |   |         }
; CHECK:             |   |   |      }
; CHECK:             |   |   |   }
; CHECK:             |   |   + END LOOP
; CHECK:             |   + END LOOP
; CHECK:             + END LOOP
; CHECK:          }
; CHECK:    END REGION

source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind uwtable
define dso_local void @_Z3fooPiiiii(ptr nocapture noundef %a, i32 noundef %n, i32 noundef %m, i32 noundef %q, i32 noundef %p) local_unnamed_addr #0 {
entry:
  %cmp155 = icmp sgt i32 %n, 0
  br i1 %cmp155, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %cmp1 = icmp slt i32 %p, 10
  %cmp45149 = icmp sgt i32 %q, 0
  %cmp50147 = icmp sgt i32 %m, 0
  %cmp53 = icmp slt i32 %m, 50
  %cmp59 = icmp ult i32 %m, 60
  %cmp73 = icmp ult i32 %m, 70
  %cmp10 = icmp slt i32 %q, 50
  %cmp12 = icmp ult i32 %q, 60
  %cmp26 = icmp ult i32 %q, 70
  %wide.trip.count = zext i32 %m to i64
  %wide.trip.count164 = zext i32 %q to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc94
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.lr.ph, %for.inc94
  %i.0156 = phi i32 [ 0, %for.body.lr.ph ], [ %inc95, %for.inc94 ]
  br i1 %cmp1, label %for.cond2.preheader, label %for.cond44.preheader

for.cond44.preheader:                             ; preds = %for.body
  br i1 %cmp45149, label %for.cond49.preheader.preheader, label %for.inc94

for.cond49.preheader.preheader:                   ; preds = %for.cond44.preheader
  br label %for.cond49.preheader

for.cond2.preheader:                              ; preds = %for.body
  br i1 %cmp50147, label %for.cond6.preheader.preheader, label %for.inc94

for.cond6.preheader.preheader:                    ; preds = %for.cond2.preheader
  br label %for.cond6.preheader

for.cond6.preheader:                              ; preds = %for.cond6.preheader.preheader, %for.cond.cleanup8
  %j.0154 = phi i32 [ %inc40, %for.cond.cleanup8 ], [ 0, %for.cond6.preheader.preheader ]
  br i1 %cmp45149, label %for.body9.lr.ph, label %for.cond.cleanup8

for.body9.lr.ph:                                  ; preds = %for.cond6.preheader
  %add14 = add nuw nsw i32 %j.0154, 2
  br label %for.body9

for.cond.cleanup8.loopexit:                       ; preds = %for.inc
  br label %for.cond.cleanup8

for.cond.cleanup8:                                ; preds = %for.cond.cleanup8.loopexit, %for.cond6.preheader
  %inc40 = add nuw nsw i32 %j.0154, 1
  %exitcond166.not = icmp eq i32 %inc40, %m
  br i1 %exitcond166.not, label %for.inc94.loopexit, label %for.cond6.preheader, !llvm.loop !3

for.body9:                                        ; preds = %for.body9.lr.ph, %for.inc
  %indvars.iv161 = phi i64 [ 0, %for.body9.lr.ph ], [ %indvars.iv.next162, %for.inc ]
  br i1 %cmp10, label %if.then11, label %if.else

if.then11:                                        ; preds = %for.body9
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv161
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !5
  %add = add nsw i32 %0, %j.0154
  store i32 %add, ptr %arrayidx, align 4, !tbaa !5
  br label %for.inc

if.else:                                          ; preds = %for.body9
  br i1 %cmp12, label %if.then13, label %if.else25

if.then13:                                        ; preds = %if.else
  %arrayidx16 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv161
  %1 = load i32, ptr %arrayidx16, align 4, !tbaa !5
  %add17 = add nsw i32 %add14, %1
  store i32 %add17, ptr %arrayidx16, align 4, !tbaa !5
  br label %for.inc

if.else25:                                        ; preds = %if.else
  br i1 %cmp26, label %if.then27, label %if.else32

if.then27:                                        ; preds = %if.else25
  %arrayidx30 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv161
  %2 = load i32, ptr %arrayidx30, align 4, !tbaa !5
  %3 = trunc i64 %indvars.iv161 to i32
  %4 = add i32 %3, 4
  %add31 = add nsw i32 %4, %2
  store i32 %add31, ptr %arrayidx30, align 4, !tbaa !5
  br label %for.inc

if.else32:                                        ; preds = %if.else25
  %arrayidx34 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv161
  %5 = load i32, ptr %arrayidx34, align 4, !tbaa !5
  %add35 = add nsw i32 %5, %i.0156
  store i32 %add35, ptr %arrayidx34, align 4, !tbaa !5
  br label %for.inc

for.inc:                                          ; preds = %if.then11, %if.else32, %if.then27, %if.then13
  %indvars.iv.next162 = add nuw nsw i64 %indvars.iv161, 1
  %exitcond165.not = icmp eq i64 %indvars.iv.next162, %wide.trip.count164
  br i1 %exitcond165.not, label %for.cond.cleanup8.loopexit, label %for.body9, !llvm.loop !9

for.cond49.preheader:                             ; preds = %for.cond49.preheader.preheader, %for.cond.cleanup51
  %j43.0150 = phi i32 [ %inc91, %for.cond.cleanup51 ], [ 0, %for.cond49.preheader.preheader ]
  br i1 %cmp50147, label %for.body52.lr.ph, label %for.cond.cleanup51

for.body52.lr.ph:                                 ; preds = %for.cond49.preheader
  %add61 = add nuw nsw i32 %j43.0150, 2
  br label %for.body52

for.cond.cleanup51.loopexit:                      ; preds = %for.inc87
  br label %for.cond.cleanup51

for.cond.cleanup51:                               ; preds = %for.cond.cleanup51.loopexit, %for.cond49.preheader
  %inc91 = add nuw nsw i32 %j43.0150, 1
  %exitcond160.not = icmp eq i32 %inc91, %q
  br i1 %exitcond160.not, label %for.inc94.loopexit169, label %for.cond49.preheader, !llvm.loop !10

for.body52:                                       ; preds = %for.body52.lr.ph, %for.inc87
  %indvars.iv = phi i64 [ 0, %for.body52.lr.ph ], [ %indvars.iv.next, %for.inc87 ]
  br i1 %cmp53, label %if.then54, label %if.else58

if.then54:                                        ; preds = %for.body52
  %arrayidx56 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %6 = load i32, ptr %arrayidx56, align 4, !tbaa !5
  %add57 = add nsw i32 %6, %j43.0150
  store i32 %add57, ptr %arrayidx56, align 4, !tbaa !5
  br label %for.inc87

if.else58:                                        ; preds = %for.body52
  br i1 %cmp59, label %if.then60, label %if.else72

if.then60:                                        ; preds = %if.else58
  %arrayidx63 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %7 = load i32, ptr %arrayidx63, align 4, !tbaa !5
  %add64 = add nsw i32 %add61, %7
  store i32 %add64, ptr %arrayidx63, align 4, !tbaa !5
  br label %for.inc87

if.else72:                                        ; preds = %if.else58
  br i1 %cmp73, label %if.then74, label %if.else79

if.then74:                                        ; preds = %if.else72
  %arrayidx77 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %8 = load i32, ptr %arrayidx77, align 4, !tbaa !5
  %9 = trunc i64 %indvars.iv to i32
  %10 = add i32 %9, 4
  %add78 = add nsw i32 %10, %8
  store i32 %add78, ptr %arrayidx77, align 4, !tbaa !5
  br label %for.inc87

if.else79:                                        ; preds = %if.else72
  %arrayidx81 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %11 = load i32, ptr %arrayidx81, align 4, !tbaa !5
  %add82 = add nsw i32 %11, %i.0156
  store i32 %add82, ptr %arrayidx81, align 4, !tbaa !5
  br label %for.inc87

for.inc87:                                        ; preds = %if.then54, %if.else79, %if.then74, %if.then60
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup51.loopexit, label %for.body52, !llvm.loop !11

for.inc94.loopexit:                               ; preds = %for.cond.cleanup8
  br label %for.inc94

for.inc94.loopexit169:                            ; preds = %for.cond.cleanup51
  br label %for.inc94

for.inc94:                                        ; preds = %for.inc94.loopexit169, %for.inc94.loopexit, %for.cond44.preheader, %for.cond2.preheader
  %inc95 = add nuw nsw i32 %i.0156, 1
  %exitcond167.not = icmp eq i32 %inc95, %n
  br i1 %exitcond167.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !12
}

attributes #0 = { argmemonly mustprogress nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.mustprogress"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
!9 = distinct !{!9, !4}
!10 = distinct !{!10, !4}
!11 = distinct !{!11, !4}
!12 = distinct !{!12, !4}
