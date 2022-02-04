; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -disable-output -hir-loop-fusion -hir-loop-fusion-skip-vec-prof-check -print-after=hir-loop-fusion < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -hir-loop-fusion-skip-vec-prof-check -disable-output < %s 2>&1 | FileCheck %s

; Check that i2 loops are all fused completely.

; int foo(int p[restrict][1000], int q[restrict][1000], int n, int *a1) {
;   int acc1 = 0;
;   int acc2 = 0;
;
;   for (size_t i=0;i<n;++i) {
;     for (size_t j=0;j<n;++j) { // Node 0
;       p[i][j] = i+j + a1[j];
;     }
;     for (size_t j=0;j<n;++j) { // Node 1
;       q[i][j] = i+j + a1[j];
;     }
;     for (size_t j=0;j<n;++j) { // Node 2
;       acc1 += i+j + p[i][j];
;     }
;     for (size_t j=0;j<n;++j) { // Node 3
;       acc2 += i+j + q[i][j] + p[i][j];
;     }
;   }
;
;   return acc1 + acc2;
;}

; Fusion Graph:
; 0--1
; 0->2
; 0->3
; 1->3
; 2--3

; BEGIN REGION { }
;       + DO i1 = 0, umax(1, sext.i32.i64(%n)) + -1, 1   <DO_LOOP>
;       |   + DO i2 = 0, umax(1, sext.i32.i64(%n)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
;       |   |   %0 = (%a1)[i2];
;       |   |   (%p)[i1][i2] = i1 + i2 + %0;
;       |   + END LOOP
;       |
;       |
;       |   + DO i2 = 0, umax(1, sext.i32.i64(%n)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
;       |   |   %2 = (%a1)[i2];
;       |   |   (%q)[i1][i2] = i1 + i2 + %2;
;       |   + END LOOP
;       |
;       |
;       |   + DO i2 = 0, umax(1, sext.i32.i64(%n)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
;       |   |   %acc1.0135 = i1 + i2 + %acc1.0135  +  (%p)[i1][i2];
;       |   + END LOOP
;       |
;       |
;       |   + DO i2 = 0, umax(1, sext.i32.i64(%n)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
;       |   |   %7 = (%q)[i1][i2];
;       |   |   %acc2.0136 = i1 + i2 + %7 + %acc2.0136  +  (%p)[i1][i2];
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION
; CHECK-SAME: modified
; CHECK:      DO i1
; CHECK-NEXT:   DO i2
; CHECK:        END LOOP
; CHECK-NOT:    DO i2
; CHECK:      END LOOP
; CHECK-NEXT: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo([1000 x i32]* noalias nocapture %p, [1000 x i32]* noalias nocapture %q, i32 %n, i32* nocapture readonly %a1) local_unnamed_addr #0 {
entry:
  %conv = sext i32 %n to i64
  %cmp134 = icmp eq i32 %n, 0
  br i1 %cmp134, label %for.cond.cleanup, label %for.body7.preheader.preheader

for.body7.preheader.preheader:                    ; preds = %entry
  br label %for.body7.preheader

for.body7.preheader:                              ; preds = %for.body7.preheader.preheader, %for.cond.cleanup53
  %i.0137 = phi i64 [ %inc71, %for.cond.cleanup53 ], [ 0, %for.body7.preheader.preheader ]
  %acc2.0136 = phi i32 [ %conv66.lcssa, %for.cond.cleanup53 ], [ 0, %for.body7.preheader.preheader ]
  %acc1.0135 = phi i32 [ %conv44.lcssa, %for.cond.cleanup53 ], [ 0, %for.body7.preheader.preheader ]
  br label %for.body7

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup53
  %conv66.lcssa.lcssa = phi i32 [ %conv66.lcssa, %for.cond.cleanup53 ]
  %conv44.lcssa.lcssa = phi i32 [ %conv44.lcssa, %for.cond.cleanup53 ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %acc1.0.lcssa = phi i32 [ 0, %entry ], [ %conv44.lcssa.lcssa, %for.cond.cleanup.loopexit ]
  %acc2.0.lcssa = phi i32 [ 0, %entry ], [ %conv66.lcssa.lcssa, %for.cond.cleanup.loopexit ]
  %add73 = add nsw i32 %acc2.0.lcssa, %acc1.0.lcssa
  ret i32 %add73

for.body7:                                        ; preds = %for.body7.preheader, %for.body7
  %j.0124 = phi i64 [ %inc, %for.body7 ], [ 0, %for.body7.preheader ]
  %add = add i64 %j.0124, %i.0137
  %arrayidx = getelementptr inbounds i32, i32* %a1, i64 %j.0124
  %0 = load i32, i32* %arrayidx, align 4
  %1 = trunc i64 %add to i32
  %conv10 = add i32 %0, %1
  %arrayidx12 = getelementptr inbounds [1000 x i32], [1000 x i32]* %p, i64 %i.0137, i64 %j.0124
  store i32 %conv10, i32* %arrayidx12, align 4
  %inc = add nuw i64 %j.0124, 1
  %cmp4 = icmp ult i64 %inc, %conv
  br i1 %cmp4, label %for.body7, label %for.body19.preheader

for.body19.preheader:                             ; preds = %for.body7
  br label %for.body19

for.body19:                                       ; preds = %for.body19.preheader, %for.body19
  %j13.0126 = phi i64 [ %inc28, %for.body19 ], [ 0, %for.body19.preheader ]
  %add20 = add i64 %j13.0126, %i.0137
  %arrayidx21 = getelementptr inbounds i32, i32* %a1, i64 %j13.0126
  %2 = load i32, i32* %arrayidx21, align 4
  %3 = trunc i64 %add20 to i32
  %conv24 = add i32 %2, %3
  %arrayidx26 = getelementptr inbounds [1000 x i32], [1000 x i32]* %q, i64 %i.0137, i64 %j13.0126
  store i32 %conv24, i32* %arrayidx26, align 4
  %inc28 = add nuw i64 %j13.0126, 1
  %cmp16 = icmp ult i64 %inc28, %conv
  br i1 %cmp16, label %for.body19, label %for.body36.preheader

for.body36.preheader:                             ; preds = %for.body19
  br label %for.body36

for.body54.preheader:                             ; preds = %for.body36
  %conv44.lcssa = phi i32 [ %conv44, %for.body36 ]
  br label %for.body54

for.body36:                                       ; preds = %for.body36.preheader, %for.body36
  %j30.0129 = phi i64 [ %inc46, %for.body36 ], [ 0, %for.body36.preheader ]
  %acc1.1128 = phi i32 [ %conv44, %for.body36 ], [ %acc1.0135, %for.body36.preheader ]
  %add37 = add i64 %j30.0129, %i.0137
  %arrayidx39 = getelementptr inbounds [1000 x i32], [1000 x i32]* %p, i64 %i.0137, i64 %j30.0129
  %4 = load i32, i32* %arrayidx39, align 4
  %5 = trunc i64 %add37 to i32
  %6 = add i32 %acc1.1128, %5
  %conv44 = add i32 %6, %4
  %inc46 = add nuw i64 %j30.0129, 1
  %cmp33 = icmp ult i64 %inc46, %conv
  br i1 %cmp33, label %for.body36, label %for.body54.preheader

for.cond.cleanup53:                               ; preds = %for.body54
  %conv66.lcssa = phi i32 [ %conv66, %for.body54 ]
  %inc71 = add nuw i64 %i.0137, 1
  %cmp = icmp ult i64 %inc71, %conv
  br i1 %cmp, label %for.body7.preheader, label %for.cond.cleanup.loopexit

for.body54:                                       ; preds = %for.body54.preheader, %for.body54
  %j48.0132 = phi i64 [ %inc68, %for.body54 ], [ 0, %for.body54.preheader ]
  %acc2.1131 = phi i32 [ %conv66, %for.body54 ], [ %acc2.0136, %for.body54.preheader ]
  %add55 = add i64 %j48.0132, %i.0137
  %arrayidx57 = getelementptr inbounds [1000 x i32], [1000 x i32]* %q, i64 %i.0137, i64 %j48.0132
  %7 = load i32, i32* %arrayidx57, align 4
  %arrayidx61 = getelementptr inbounds [1000 x i32], [1000 x i32]* %p, i64 %i.0137, i64 %j48.0132
  %8 = load i32, i32* %arrayidx61, align 4
  %9 = trunc i64 %add55 to i32
  %10 = add i32 %acc2.1131, %9
  %11 = add i32 %10, %7
  %conv66 = add i32 %11, %8
  %inc68 = add nuw i64 %j48.0132, 1
  %cmp51 = icmp ult i64 %inc68, %conv
  br i1 %cmp51, label %for.body54, label %for.cond.cleanup53
}

