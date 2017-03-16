; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-complete-unroll-loop-trip-threshold=30 -hir-lmm -print-before=hir-lmm -print-after=hir-lmm < %s 2>&1 | FileCheck %s
; (This test is based on CompleteUnroll/canon-def-level.ll)
;
;  for(i=0; i<60; i++) {
;    a = A[i];
;    for(j=0; j<40; j++) {
;      b = A[j];
;      for(k=0; k<6; k++) {
;        A[k] = i + k*b;
;        A[2*k] = a + k*b;
;        A[3*k] = b;
;      }
;    }
;  }
;
; INPUT HIR:
;
;<48>      
;<48>      + DO i2 = 0, 39, 1   <DO_LOOP>
;<9>       |   %2 = (%A)[i2];
;<50>      |   (%A)[0] = i1;
;<51>      |   (%A)[0] = zext.i32.i64(%0);
;<52>      |   (%A)[0] = %2;
;<53>      |   (%A)[1] = i1 + sext.i32.i64(%2);
;<54>      |   (%A)[2] = sext.i32.i64(%2) + zext.i32.i64(%0);
;<55>      |   (%A)[3] = %2;
;<56>      |   (%A)[2] = i1 + 2 * sext.i32.i64(%2);
;<57>      |   (%A)[4] = 2 * sext.i32.i64(%2) + zext.i32.i64(%0);
;<58>      |   (%A)[6] = %2;
;<59>      |   (%A)[3] = i1 + 3 * sext.i32.i64(%2);
;<60>      |   (%A)[6] = 3 * sext.i32.i64(%2) + zext.i32.i64(%0);
;<61>      |   (%A)[9] = %2;
;<62>      |   (%A)[4] = i1 + 4 * sext.i32.i64(%2);
;<63>      |   (%A)[8] = 4 * sext.i32.i64(%2) + zext.i32.i64(%0);
;<64>      |   (%A)[12] = %2;
;<65>      |   (%A)[5] = i1 + 5 * sext.i32.i64(%2);
;<66>      |   (%A)[10] = 5 * sext.i32.i64(%2) + zext.i32.i64(%0);
;<67>      |   (%A)[15] = %2;
;<48>      + END LOOP
;
; [LIMM Analysis]
;MemRefCollection, entries: 12
;  (%A)[0] {  W  W  W  } 3W : 0R  illegal 
;  (%A)[1] {  W  } 1W : 0R  illegal 
;  (%A)[2] {  W  W  } 2W : 0R  illegal 
;  (%A)[3] {  W  W  } 2W : 0R  illegal 
;  (%A)[4] {  W  W  } 2W : 0R  illegal 
;  (%A)[6] {  W  W  } 2W : 0R  illegal 
;  (%A)[9] {  W  } 1W : 0R  illegal 
;  (%A)[8] {  W  } 1W : 0R  illegal 
;  (%A)[12] {  W  } 1W : 0R  illegal 
;  (%A)[5] {  W  } 1W : 0R  illegal 
;  (%A)[10] {  W  } 1W : 0R  illegal 
;  (%A)[15] {  W  } 1W : 0R  illegal 
;
; LIMM's Opportunities:
; - LILH: (0)
; - LISS: (0)
; - LILHSS:(0)
;  
; CHECK: IR Dump Before HIR Loop Memory Motion
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 59, 1   <DO_LOOP>
; CHECK:        |   %0 = (%A)[i1];
; CHECK:        |
; CHECK:        |   + DO i2 = 0, 39, 1   <DO_LOOP>
; CHECK:        |   |   %2 = (%A)[i2];
; CHECK:        |   |   (%A)[0] = i1;
; CHECK:        |   |   (%A)[0] = zext.i32.i64(%0);
; CHECK:        |   |   (%A)[0] = %2;
; CHECK:        |   |   (%A)[1] = i1 + sext.i32.i64(%2);
; CHECK:        |   |   (%A)[2] = sext.i32.i64(%2) + zext.i32.i64(%0);
; CHECK:        |   |   (%A)[3] = %2;
; CHECK:        |   |   (%A)[2] = i1 + 2 * sext.i32.i64(%2);
; CHECK:        |   |   (%A)[4] = 2 * sext.i32.i64(%2) + zext.i32.i64(%0);
; CHECK:        |   |   (%A)[6] = %2;
; CHECK:        |   |   (%A)[3] = i1 + 3 * sext.i32.i64(%2);
; CHECK:        |   |   (%A)[6] = 3 * sext.i32.i64(%2) + zext.i32.i64(%0);
; CHECK:        |   |   (%A)[9] = %2;
; CHECK:        |   |   (%A)[4] = i1 + 4 * sext.i32.i64(%2);
; CHECK:        |   |   (%A)[8] = 4 * sext.i32.i64(%2) + zext.i32.i64(%0);
; CHECK:        |   |   (%A)[12] = %2;
; CHECK:        |   |   (%A)[5] = i1 + 5 * sext.i32.i64(%2);
; CHECK:        |   |   (%A)[10] = 5 * sext.i32.i64(%2) + zext.i32.i64(%0);
; CHECK:        |   |   (%A)[15] = %2;
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
; *** 
;          
;
; CHECK: IR Dump After HIR Loop Memory Motion
;  
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 59, 1   <DO_LOOP>
; CHECK:        |   %0 = (%A)[i1];
; CHECK:        |   
; CHECK:        |   + DO i2 = 0, 39, 1   <DO_LOOP>
; CHECK:        |   |   %2 = (%A)[i2];
; CHECK:        |   |   (%A)[0] = i1;
; CHECK:        |   |   (%A)[0] = zext.i32.i64(%0);
; CHECK:        |   |   (%A)[0] = %2;
; CHECK:        |   |   (%A)[1] = i1 + sext.i32.i64(%2);
; CHECK:        |   |   (%A)[2] = sext.i32.i64(%2) + zext.i32.i64(%0);
; CHECK:        |   |   (%A)[3] = %2;
; CHECK:        |   |   (%A)[2] = i1 + 2 * sext.i32.i64(%2);
; CHECK:        |   |   (%A)[4] = 2 * sext.i32.i64(%2) + zext.i32.i64(%0);
; CHECK:        |   |   (%A)[6] = %2;
; CHECK:        |   |   (%A)[3] = i1 + 3 * sext.i32.i64(%2);
; CHECK:        |   |   (%A)[6] = 3 * sext.i32.i64(%2) + zext.i32.i64(%0);
; CHECK:        |   |   (%A)[9] = %2;
; CHECK:        |   |   (%A)[4] = i1 + 4 * sext.i32.i64(%2);
; CHECK:        |   |   (%A)[8] = 4 * sext.i32.i64(%2) + zext.i32.i64(%0);
; CHECK:        |   |   (%A)[12] = %2;
; CHECK:        |   |   (%A)[5] = i1 + 5 * sext.i32.i64(%2);
; CHECK:        |   |   (%A)[10] = 5 * sext.i32.i64(%2) + zext.i32.i64(%0);
; CHECK:        |   |   (%A)[15] = %2;
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
;
; ModuleID = 'canon-def-level1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %A) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc.22, %entry
  %indvars.iv58 = phi i64 [ 0, %entry ], [ %indvars.iv.next59, %for.inc.22 ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv58
  %0 = load i32, i32* %arrayidx, align 4
  %1 = zext i32 %0 to i64
  br label %for.body.3

for.body.3:                                       ; preds = %for.inc.19, %for.body
  %indvars.iv55 = phi i64 [ 0, %for.body ], [ %indvars.iv.next56, %for.inc.19 ]
  %arrayidx5 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv55
  %2 = load i32, i32* %arrayidx5, align 4
  %3 = sext i32 %2 to i64
  br label %for.body.8

for.body.8:                                       ; preds = %for.body.8, %for.body.3
  %indvars.iv = phi i64 [ 0, %for.body.3 ], [ %indvars.iv.next, %for.body.8 ]
  %4 = mul nsw i64 %indvars.iv, %3
  %5 = add nsw i64 %4, %indvars.iv58
  %arrayidx10 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %6 = trunc i64 %5 to i32
  store i32 %6, i32* %arrayidx10, align 4
  %7 = add i64 %4, %1
  %8 = shl nsw i64 %indvars.iv, 1
  %arrayidx15 = getelementptr inbounds i32, i32* %A, i64 %8
  %9 = trunc i64 %7 to i32
  store i32 %9, i32* %arrayidx15, align 4
  %10 = mul nuw nsw i64 %indvars.iv, 3
  %arrayidx18 = getelementptr inbounds i32, i32* %A, i64 %10
  store i32 %2, i32* %arrayidx18, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 6
  br i1 %exitcond, label %for.inc.19, label %for.body.8

for.inc.19:                                       ; preds = %for.body.8
  %indvars.iv.next56 = add nuw nsw i64 %indvars.iv55, 1
  %exitcond57 = icmp eq i64 %indvars.iv.next56, 40
  br i1 %exitcond57, label %for.inc.22, label %for.body.3

for.inc.22:                                       ; preds = %for.inc.19
  %indvars.iv.next59 = add nuw nsw i64 %indvars.iv58, 1
  %exitcond60 = icmp eq i64 %indvars.iv.next59, 60
  br i1 %exitcond60, label %for.end.24, label %for.body

for.end.24:                                       ; preds = %for.inc.22
  ret void
}

