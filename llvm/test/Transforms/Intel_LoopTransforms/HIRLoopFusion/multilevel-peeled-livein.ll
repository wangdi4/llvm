; RUN: opt -hir-details -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-create-function-level-region < %s 2>&1 | FileCheck %s

; Check that "%n {sb:3}" is added as live-in for i2/i3 peeled iterations.
; This should happend bacause of normalization.

; BEGIN REGION { }
;       + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
;       |   + DO i2 = 0, 99, 1   <DO_LOOP>
;       |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
;       |   |   |   (@A)[0][i1][i2][i3] = i1 + i2 + i3;
;       |   |   + END LOOP
;       |   + END LOOP
;       + END LOOP
;
;
;       + DO i1 = 0, %n + -2, 1   <DO_LOOP>  <MAX_TC_EST = 100>
;       |   + DO i2 = 0, 99, 1   <DO_LOOP>
;       |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
;       |   |   |   %0 = (@A)[0][i1][i2][i3];
;       |   |   |   (@B)[0][i1][i2][i3] = i1 + i2 + i3 + %0;
;       |   |   + END LOOP
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i32 i1 = 0, %n + -2, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK:           | <BLOB> LINEAR i32 %n {sb:[[SB:.*]]}
; CHECK:           |   + DO i32 i2 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   + DO i32 i3 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   |   (@A)[0][i1][i2][i3] = i1 + i2 + i3;
; CHECK:           |   |   |   %0 = (@A)[0][i1][i2][i3];
; CHECK:           |   |   |   (@B)[0][i1][i2][i3] = i1 + i2 + i3 + %0;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP

; CHECK:           + LiveIn symbases: [[SB]]
; CHECK:           + DO i32 i1 = 0, 0, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK:           |   + LiveIn symbases: [[SB]]
; CHECK:           |   + DO i32 i2 = 0, 99, 1   <DO_LOOP>
; CHECK:           |       + LiveIn symbases: [[SB]]
; CHECK:           |   |   + DO i32 i3 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   |   (@A)[0][i1 + %n + -1][i2][i3] = i1 + i2 + i3 + %n + -1;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION

target datalayout = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x [100 x [100 x i32]]] zeroinitializer, align 4
@B = dso_local local_unnamed_addr global [100 x [100 x [100 x i32]]] zeroinitializer, align 4

; Function Attrs: nofree norecurse nounwind
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader.preheader

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.cond.cleanup3
  %i.086 = phi i32 [ %inc16, %for.cond.cleanup3 ], [ 0, %for.cond1.preheader.preheader ]
  br label %for.cond5.preheader

for.cond19.preheader:                             ; preds = %for.cond.cleanup3
  %sub = add nsw i32 %n, -1
  br label %for.cond24.preheader.preheader

for.cond24.preheader.preheader:                   ; preds = %for.cond19.preheader
  br label %for.cond24.preheader

for.cond5.preheader:                              ; preds = %for.cond1.preheader, %for.cond.cleanup7
  %j.084 = phi i32 [ 0, %for.cond1.preheader ], [ %inc13, %for.cond.cleanup7 ]
  %add = add nuw nsw i32 %j.084, %i.086
  br label %for.body8

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %inc16 = add nuw nsw i32 %i.086, 1
  %exitcond91 = icmp eq i32 %inc16, %n
  br i1 %exitcond91, label %for.cond19.preheader, label %for.cond1.preheader

for.cond.cleanup7:                                ; preds = %for.body8
  %inc13 = add nuw nsw i32 %j.084, 1
  %exitcond90 = icmp eq i32 %inc13, 100
  br i1 %exitcond90, label %for.cond.cleanup3, label %for.cond5.preheader

for.body8:                                        ; preds = %for.cond5.preheader, %for.body8
  %k.083 = phi i32 [ 0, %for.cond5.preheader ], [ %inc, %for.body8 ]
  %add9 = add nuw nsw i32 %add, %k.083
  %arrayidx11 = getelementptr inbounds [100 x [100 x [100 x i32]]], ptr @A, i32 0, i32 %i.086, i32 %j.084, i32 %k.083
  store i32 %add9, ptr %arrayidx11, align 4
  %inc = add nuw nsw i32 %k.083, 1
  %exitcond89 = icmp eq i32 %inc, 100
  br i1 %exitcond89, label %for.cond.cleanup7, label %for.body8

for.cond24.preheader:                             ; preds = %for.cond24.preheader.preheader, %for.cond.cleanup26
  %i18.082 = phi i32 [ %inc49, %for.cond.cleanup26 ], [ 0, %for.cond24.preheader.preheader ]
  br label %for.cond29.preheader

for.cond.cleanup21.loopexit:                      ; preds = %for.cond.cleanup26
  br label %for.cond.cleanup21

for.cond.cleanup21:                               ; preds = %for.cond.cleanup21.loopexit, %entry, %for.cond19.preheader
  ret void

for.cond29.preheader:                             ; preds = %for.cond24.preheader, %for.cond.cleanup31
  %j23.080 = phi i32 [ 0, %for.cond24.preheader ], [ %inc46, %for.cond.cleanup31 ]
  %add36 = add nuw i32 %j23.080, %i18.082
  br label %for.body32

for.cond.cleanup26:                               ; preds = %for.cond.cleanup31
  %inc49 = add nuw nsw i32 %i18.082, 1
  %exitcond88 = icmp eq i32 %inc49, %sub
  br i1 %exitcond88, label %for.cond.cleanup21.loopexit, label %for.cond24.preheader

for.cond.cleanup31:                               ; preds = %for.body32
  %inc46 = add nuw nsw i32 %j23.080, 1
  %exitcond87 = icmp eq i32 %inc46, 100
  br i1 %exitcond87, label %for.cond.cleanup26, label %for.cond29.preheader

for.body32:                                       ; preds = %for.cond29.preheader, %for.body32
  %k28.079 = phi i32 [ 0, %for.cond29.preheader ], [ %inc43, %for.body32 ]
  %arrayidx35 = getelementptr inbounds [100 x [100 x [100 x i32]]], ptr @A, i32 0, i32 %i18.082, i32 %j23.080, i32 %k28.079
  %0 = load i32, ptr %arrayidx35, align 4
  %add37 = add nuw i32 %add36, %k28.079
  %add38 = add i32 %add37, %0
  %arrayidx41 = getelementptr inbounds [100 x [100 x [100 x i32]]], ptr @B, i32 0, i32 %i18.082, i32 %j23.080, i32 %k28.079
  store i32 %add38, ptr %arrayidx41, align 4
  %inc43 = add nuw nsw i32 %k28.079, 1
  %exitcond = icmp eq i32 %inc43, 100
  br i1 %exitcond, label %for.cond.cleanup31, label %for.body32
}

