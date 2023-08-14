; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-mv-const-ub,hir-loop-collapse,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-loop-fusion-skip-vec-prof-check  < %s 2>&1 | FileCheck %s

; Test checks that DD edges are computed correctly after collapsing and fusion doesn't happen on i3 level.

; HIR before collapsing and fusion
;                  if (%n == 3)
;                  {
;                     + DO i1 = 0, 2, 1   <DO_LOOP>
;                     |   + DO i2 = 0, 2, 1   <DO_LOOP>  <MAX_TC_EST = 3>
;                     |   |   + DO i3 = 0, -1 * %m + 2, 1   <DO_LOOP>  <MAX_TC_EST = 4>
;                     |   |   |   + DO i4 = 0, 2, 1   <DO_LOOP>  <MAX_TC_EST = 3>
;                     |   |   |   |   + DO i5 = 0, 2, 1   <DO_LOOP>  <MAX_TC_EST = 3>
;                     |   |   |   |   |   + DO i6 = 0, 2, 1   <DO_LOOP>  <MAX_TC_EST = 3>
;                     |   |   |   |   |   |   + DO i7 = 0, 2, 1   <DO_LOOP>  <MAX_TC_EST = 3>
;                     |   |   |   |   |   |   |   %0 = (@b)[0][i1][i2][i3 + %k + -1][i4][i5][i6][i7];
;                     |   |   |   |   |   |   |   (@a)[0][i1][i2][i3][i4][i5][i6][i7] = %0 + 1;
;                     |   |   |   |   |   |   + END LOOP
;                     |   |   |   |   |   + END LOOP
;                     |   |   |   |   + END LOOP
;                     |   |   |   + END LOOP
;                     |   |   + END LOOP
;                     |   + END LOOP
;                     + END LOOP
;                     + DO i1 = 0, 2, 1   <DO_LOOP>
;                     |   + DO i2 = 0, 2, 1   <DO_LOOP>  <MAX_TC_EST = 3>
;                     |   |   + DO i3 = 0, -1 * %m + 2, 1   <DO_LOOP>  <MAX_TC_EST = 4>
;                     |   |   |   + DO i4 = 0, 2, 1   <DO_LOOP>  <MAX_TC_EST = 3>
;                     |   |   |   |   + DO i5 = 0, 2, 1   <DO_LOOP>  <MAX_TC_EST = 3>
;                     |   |   |   |   |   + DO i6 = 0, 2, 1   <DO_LOOP>  <MAX_TC_EST = 3>
;                     |   |   |   |   |   |   + DO i7 = 0, 2, 1   <DO_LOOP>  <MAX_TC_EST = 3>
;                     |   |   |   |   |   |   |   %1 = (@a)[0][i1][i2][i3][i4][i5][i6][i7];
;                     |   |   |   |   |   |   |   (@b)[0][i1][i2][i3 + %m + -1][i4][i5][i6][i7] = %1 + 1;
;                     |   |   |   |   |   |   + END LOOP
;                     |   |   |   |   |   + END LOOP
;                     |   |   |   |   + END LOOP
;                     |   |   |   + END LOOP
;                     |   |   + END LOOP
;                     |   + END LOOP
;                     + END LOOP
;                  }
;                  else


; HIR after collapsing and fusion
; CHECK:     BEGIN REGION { modified }
; CHECK:           if (%n == 3)
; CHECK:           {
; CHECK:              + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, 2, 1   <DO_LOOP>  <MAX_TC_EST = 3>
; CHECK:              |   |   + DO i3 = 0, 81 * (3 + (-1 * %m)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 324>
;                     |   |   |   %0 = (@b)[0][i1][i2][%k + -1][0][0][0][i3];
;                     |   |   |   (@a)[0][i1][i2][0][0][0][0][i3] = %0 + 1;
; CHECK:              |   |   + END LOOP
;                     |   |
;                     |   |
; CHECK:              |   |   + DO i3 = 0, 81 * (3 + (-1 * %m)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 324>
;                     |   |   |   %1 = (@a)[0][i1][i2][0][0][0][0][i3];
;                     |   |   |   (@b)[0][i1][i2][%m + -1][0][0][0][i3] = %1 + 1;
; CHECK:              |   |   + END LOOP
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:           }
; CHECK:           else


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@b = dso_local local_unnamed_addr global [3 x [3 x [4 x [3 x [3 x [3 x [3 x i32]]]]]]] zeroinitializer, align 16
@a = dso_local local_unnamed_addr global [3 x [3 x [4 x [3 x [3 x [3 x [3 x i32]]]]]]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @foo(i64 noundef %n, i64 noundef %m, i64 noundef %k) local_unnamed_addr {
entry:
  %cmp202 = icmp sgt i64 %n, 0
  br i1 %cmp202, label %for.cond1.preheader.lr.ph, label %for.end111

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %sub = sub nsw i64 %n, %m
  %cmp5198 = icmp sgt i64 %sub, 0
  %add = add i64 %k, -1
  br label %for.cond4.preheader.preheader

for.cond4.preheader.preheader:                    ; preds = %for.inc49, %for.cond1.preheader.lr.ph
  %i1.0203 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %inc50, %for.inc49 ]
  br label %for.cond4.preheader

for.cond52.preheader:                             ; preds = %for.inc49
  br label %for.cond55.preheader.lr.ph

for.cond55.preheader.lr.ph:                       ; preds = %for.cond52.preheader
  %add84 = add i64 %m, -1
  br label %for.cond58.preheader.preheader

for.cond4.preheader:                              ; preds = %for.cond4.preheader.preheader, %for.inc46
  %i2.0201 = phi i64 [ %inc47, %for.inc46 ], [ 0, %for.cond4.preheader.preheader ]
  br i1 %cmp5198, label %for.cond7.preheader.preheader, label %for.inc46

for.cond7.preheader.preheader:                    ; preds = %for.cond4.preheader
  br label %for.cond7.preheader

for.cond7.preheader:                              ; preds = %for.cond7.preheader.preheader, %for.inc43
  %i3.0199 = phi i64 [ %inc44, %for.inc43 ], [ 0, %for.cond7.preheader.preheader ]
  %sub20 = add i64 %add, %i3.0199
  br label %for.cond10.preheader

for.cond10.preheader:                             ; preds = %for.cond7.preheader, %for.inc40
  %i4.0197 = phi i64 [ 0, %for.cond7.preheader ], [ %inc41, %for.inc40 ]
  br label %for.cond13.preheader

for.cond13.preheader:                             ; preds = %for.cond10.preheader, %for.inc37
  %i5.0195 = phi i64 [ 0, %for.cond10.preheader ], [ %inc38, %for.inc37 ]
  br label %for.cond16.preheader

for.cond16.preheader:                             ; preds = %for.cond13.preheader, %for.inc34
  %i6.0193 = phi i64 [ 0, %for.cond13.preheader ], [ %inc35, %for.inc34 ]
  br label %for.body18

for.body18:                                       ; preds = %for.cond16.preheader, %for.body18
  %i7.0191 = phi i64 [ 0, %for.cond16.preheader ], [ %inc, %for.body18 ]
  %arrayidx25 = getelementptr inbounds [3 x [3 x [4 x [3 x [3 x [3 x [3 x i32]]]]]]], ptr @b, i64 0, i64 %i1.0203, i64 %i2.0201, i64 %sub20, i64 %i4.0197, i64 %i5.0195, i64 %i6.0193, i64 %i7.0191
  %0 = load i32, ptr %arrayidx25, align 4
  %add26 = add nsw i32 %0, 1
  %arrayidx33 = getelementptr inbounds [3 x [3 x [4 x [3 x [3 x [3 x [3 x i32]]]]]]], ptr @a, i64 0, i64 %i1.0203, i64 %i2.0201, i64 %i3.0199, i64 %i4.0197, i64 %i5.0195, i64 %i6.0193, i64 %i7.0191
  store i32 %add26, ptr %arrayidx33, align 4
  %inc = add nuw nsw i64 %i7.0191, 1
  %exitcond.not = icmp eq i64 %inc, %n
  br i1 %exitcond.not, label %for.inc34, label %for.body18

for.inc34:                                        ; preds = %for.body18
  %inc35 = add nuw nsw i64 %i6.0193, 1
  %exitcond218.not = icmp eq i64 %inc35, %n
  br i1 %exitcond218.not, label %for.inc37, label %for.cond16.preheader

for.inc37:                                        ; preds = %for.inc34
  %inc38 = add nuw nsw i64 %i5.0195, 1
  %exitcond219.not = icmp eq i64 %inc38, %n
  br i1 %exitcond219.not, label %for.inc40, label %for.cond13.preheader

for.inc40:                                        ; preds = %for.inc37
  %inc41 = add nuw nsw i64 %i4.0197, 1
  %exitcond220.not = icmp eq i64 %inc41, %n
  br i1 %exitcond220.not, label %for.inc43, label %for.cond10.preheader

for.inc43:                                        ; preds = %for.inc40
  %inc44 = add nuw nsw i64 %i3.0199, 1
  %exitcond221.not = icmp eq i64 %inc44, %sub
  br i1 %exitcond221.not, label %for.inc46.loopexit, label %for.cond7.preheader

for.inc46.loopexit:                               ; preds = %for.inc43
  br label %for.inc46

for.inc46:                                        ; preds = %for.inc46.loopexit, %for.cond4.preheader
  %inc47 = add nuw nsw i64 %i2.0201, 1
  %exitcond222.not = icmp eq i64 %inc47, %n
  br i1 %exitcond222.not, label %for.inc49, label %for.cond4.preheader

for.inc49:                                        ; preds = %for.inc46
  %inc50 = add nuw nsw i64 %i1.0203, 1
  %exitcond223.not = icmp eq i64 %inc50, %n
  br i1 %exitcond223.not, label %for.cond52.preheader, label %for.cond4.preheader.preheader

for.cond58.preheader.preheader:                   ; preds = %for.inc109, %for.cond55.preheader.lr.ph
  %i1.1217 = phi i64 [ 0, %for.cond55.preheader.lr.ph ], [ %inc110, %for.inc109 ]
  br label %for.cond58.preheader

for.cond58.preheader:                             ; preds = %for.cond58.preheader.preheader, %for.inc106
  %i2.1215 = phi i64 [ %inc107, %for.inc106 ], [ 0, %for.cond58.preheader.preheader ]
  br i1 %cmp5198, label %for.cond62.preheader.preheader, label %for.inc106

for.cond62.preheader.preheader:                   ; preds = %for.cond58.preheader
  br label %for.cond62.preheader

for.cond62.preheader:                             ; preds = %for.cond62.preheader.preheader, %for.inc103
  %i3.1213 = phi i64 [ %inc104, %for.inc103 ], [ 0, %for.cond62.preheader.preheader ]
  %sub85 = add i64 %add84, %i3.1213
  br label %for.cond65.preheader

for.cond65.preheader:                             ; preds = %for.cond62.preheader, %for.inc100
  %i4.1211 = phi i64 [ 0, %for.cond62.preheader ], [ %inc101, %for.inc100 ]
  br label %for.cond68.preheader

for.cond68.preheader:                             ; preds = %for.cond65.preheader, %for.inc97
  %i5.1209 = phi i64 [ 0, %for.cond65.preheader ], [ %inc98, %for.inc97 ]
  br label %for.cond71.preheader

for.cond71.preheader:                             ; preds = %for.cond68.preheader, %for.inc94
  %i6.1207 = phi i64 [ 0, %for.cond68.preheader ], [ %inc95, %for.inc94 ]
  br label %for.body73

for.body73:                                       ; preds = %for.cond71.preheader, %for.body73
  %i7.1205 = phi i64 [ 0, %for.cond71.preheader ], [ %inc92, %for.body73 ]
  %arrayidx80 = getelementptr inbounds [3 x [3 x [4 x [3 x [3 x [3 x [3 x i32]]]]]]], ptr @a, i64 0, i64 %i1.1217, i64 %i2.1215, i64 %i3.1213, i64 %i4.1211, i64 %i5.1209, i64 %i6.1207, i64 %i7.1205
  %1 = load i32, ptr %arrayidx80, align 4
  %add81 = add nsw i32 %1, 1
  %arrayidx90 = getelementptr inbounds [3 x [3 x [4 x [3 x [3 x [3 x [3 x i32]]]]]]], ptr @b, i64 0, i64 %i1.1217, i64 %i2.1215, i64 %sub85, i64 %i4.1211, i64 %i5.1209, i64 %i6.1207, i64 %i7.1205
  store i32 %add81, ptr %arrayidx90, align 4
  %inc92 = add nuw nsw i64 %i7.1205, 1
  %exitcond224.not = icmp eq i64 %inc92, %n
  br i1 %exitcond224.not, label %for.inc94, label %for.body73

for.inc94:                                        ; preds = %for.body73
  %inc95 = add nuw nsw i64 %i6.1207, 1
  %exitcond225.not = icmp eq i64 %inc95, %n
  br i1 %exitcond225.not, label %for.inc97, label %for.cond71.preheader

for.inc97:                                        ; preds = %for.inc94
  %inc98 = add nuw nsw i64 %i5.1209, 1
  %exitcond226.not = icmp eq i64 %inc98, %n
  br i1 %exitcond226.not, label %for.inc100, label %for.cond68.preheader

for.inc100:                                       ; preds = %for.inc97
  %inc101 = add nuw nsw i64 %i4.1211, 1
  %exitcond227.not = icmp eq i64 %inc101, %n
  br i1 %exitcond227.not, label %for.inc103, label %for.cond65.preheader

for.inc103:                                       ; preds = %for.inc100
  %inc104 = add nuw nsw i64 %i3.1213, 1
  %exitcond228.not = icmp eq i64 %inc104, %sub
  br i1 %exitcond228.not, label %for.inc106.loopexit, label %for.cond62.preheader

for.inc106.loopexit:                              ; preds = %for.inc103
  br label %for.inc106

for.inc106:                                       ; preds = %for.inc106.loopexit, %for.cond58.preheader
  %inc107 = add nuw nsw i64 %i2.1215, 1
  %exitcond229.not = icmp eq i64 %inc107, %n
  br i1 %exitcond229.not, label %for.inc109, label %for.cond58.preheader

for.inc109:                                       ; preds = %for.inc106
  %inc110 = add nuw nsw i64 %i1.1217, 1
  %exitcond230.not = icmp eq i64 %inc110, %n
  br i1 %exitcond230.not, label %for.end111.loopexit, label %for.cond58.preheader.preheader

for.end111.loopexit:                              ; preds = %for.inc109
  br label %for.end111

for.end111:                                       ; preds = %for.end111.loopexit, %entry, %for.cond52.preheader
  %arrayidx118 = getelementptr inbounds [3 x [3 x [4 x [3 x [3 x [3 x [3 x i32]]]]]]], ptr @a, i64 0, i64 %n, i64 %m, i64 %k, i64 %n, i64 %m, i64 %k, i64 %k
  %2 = load i32, ptr %arrayidx118, align 4
  ret i32 %2
}

