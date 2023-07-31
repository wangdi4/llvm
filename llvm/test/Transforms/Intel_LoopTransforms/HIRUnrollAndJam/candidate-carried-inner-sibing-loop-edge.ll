; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-unroll-and-jam" -debug-only=hir-unroll-and-jam -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that we give up on unroll & jam of i1 loop on encountering this edge-
; (@A)[0][i1 + 1][i2][i3] --> (@A)[0][i1][i2][i3] FLOW (< =)

; This dependency is carried by i1 loop and edge is between sibling loops so
; it isn't an innermost loop edge. Such edges cannot be correctly handled by
; unroll & jam.

; Input HIR-
; + DO i1 = 0, 30, 1   <DO_LOOP>
; |   + DO i2 = 0, 30, 1   <DO_LOOP>
; |   |   + DO i3 = 0, 30, 1   <DO_LOOP>
; |   |   |   (@B)[0][i1][i2][i3] = (@A)[0][i1][i2][i3];
; |   |   |   %1 = (@C)[0][i2][i3];
; |   |   |   (@C)[0][i2][i3] = %1 + 1;
; |   |   + END LOOP
; |   |
; |   |
; |   |   + DO i3 = 0, 30, 1   <DO_LOOP>
; |   |   |   (@A)[0][i1 + 1][i2][i3] = (@D)[0][i1][i2][i3];
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP


; CHECK: Illegal edge found: 
; CHECK-SAME: (@A)[0][i1 + 1][i2][i3] --> (@A)[0][i1][i2][i3] FLOW (< =)


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [32 x [32 x [32 x i32]]] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [32 x [32 x [32 x i32]]] zeroinitializer, align 16
@C = dso_local local_unnamed_addr global [32 x [32 x i32]] zeroinitializer, align 16
@D = dso_local local_unnamed_addr global [32 x [32 x [32 x i32]]] zeroinitializer, align 16

define dso_local void @foo() {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc43
  %indvars.iv78 = phi i64 [ 0, %entry ], [ %indvars.iv.next79, %for.inc43 ]
  %indvars.iv.next79 = add nuw nsw i64 %indvars.iv78, 1
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond1.preheader, %for.inc40
  %indvars.iv75 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next76, %for.inc40 ]
  br label %for.body6

for.body6:                                        ; preds = %for.cond4.preheader, %for.body6
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %arrayidx10 = getelementptr inbounds [32 x [32 x [32 x i32]]], ptr @A, i64 0, i64 %indvars.iv78, i64 %indvars.iv75, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx10, align 4
  %arrayidx16 = getelementptr inbounds [32 x [32 x [32 x i32]]], ptr @B, i64 0, i64 %indvars.iv78, i64 %indvars.iv75, i64 %indvars.iv
  store i32 %0, ptr %arrayidx16, align 4
  %arrayidx20 = getelementptr inbounds [32 x [32 x i32]], ptr @C, i64 0, i64 %indvars.iv75, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx20, align 4
  %inc = add nsw i32 %1, 1
  store i32 %inc, ptr %arrayidx20, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 31
  br i1 %exitcond.not, label %for.body24.preheader, label %for.body6

for.body24.preheader:                             ; preds = %for.body6
  br label %for.body24

for.body24:                                       ; preds = %for.body24.preheader, %for.body24
  %indvars.iv72 = phi i64 [ %indvars.iv.next73, %for.body24 ], [ 0, %for.body24.preheader ]
  %arrayidx30 = getelementptr inbounds [32 x [32 x [32 x i32]]], ptr @D, i64 0, i64 %indvars.iv78, i64 %indvars.iv75, i64 %indvars.iv72
  %2 = load i32, ptr %arrayidx30, align 4
  %arrayidx36 = getelementptr inbounds [32 x [32 x [32 x i32]]], ptr @A, i64 0, i64 %indvars.iv.next79, i64 %indvars.iv75, i64 %indvars.iv72
  store i32 %2, ptr %arrayidx36, align 4
  %indvars.iv.next73 = add nuw nsw i64 %indvars.iv72, 1
  %exitcond74.not = icmp eq i64 %indvars.iv.next73, 31
  br i1 %exitcond74.not, label %for.inc40, label %for.body24

for.inc40:                                        ; preds = %for.body24
  %indvars.iv.next76 = add nuw nsw i64 %indvars.iv75, 1
  %exitcond77.not = icmp eq i64 %indvars.iv.next76, 31
  br i1 %exitcond77.not, label %for.inc43, label %for.cond4.preheader

for.inc43:                                        ; preds = %for.inc40
  %exitcond80.not = icmp eq i64 %indvars.iv.next79, 31
  br i1 %exitcond80.not, label %for.end45, label %for.cond1.preheader

for.end45:                                        ; preds = %for.inc43
  ret void
}

