; RUN: opt -passes="hir-ssa-deconstruction,hir-post-vec-complete-unroll,hir-lmm,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s
;
; *** IR Dump Before HIR Loop Memory Motion ***
;
;<0>          BEGIN REGION { modified }
;<30>               + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
;<33>               |   %0 = (@A)[0][0];
;<7>                |   %0 = (@A)[0][1];
;<14>               |   (@B)[0][i1] = %0 + 5;
;<30>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Loop Memory Motion ***
;
;CHECK:       BEGIN REGION { modified }
;CHECK:               %limm = (@A)[0][0];
;CHECK:               %limm2 = (@A)[0][1];
;CHECK:            + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
;CHECK:            |   %0 = %limm;
;CHECK:            |   %0 = %limm2;
;CHECK:            |   (@B)[0][i1] = %0 + 5;
;CHECK:            + END LOOP
;CHECK:      END REGION
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp20 = icmp sgt i32 %n, 0
  br i1 %cmp20, label %for.cond1.preheader.preheader, label %for.end9

for.cond1.preheader.preheader:                    ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc7, %for.cond1.preheader.preheader
  %indvars.iv22 = phi i64 [ 0, %for.cond1.preheader.preheader ], [ %indvars.iv.next23, %for.inc7 ]
  %arrayidx6 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %indvars.iv22
  br label %for.body3

for.body3:                                        ; preds = %for.inc, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %cmp4 = icmp eq i64 %indvars.iv, 0
  br i1 %cmp4, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body3
  %add = add nsw i32 %0, 5
  store i32 %add, ptr %arrayidx6, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 2
  br i1 %exitcond, label %for.inc7, label %for.body3

for.inc7:                                         ; preds = %for.inc
  %.lcssa = phi i32 [ %0, %for.inc ]
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond24 = icmp eq i64 %indvars.iv.next23, %wide.trip.count
  br i1 %exitcond24, label %for.end9.loopexit, label %for.cond1.preheader

for.end9.loopexit:                                ; preds = %for.inc7
  %.lcssa.lcssa = phi i32 [ %.lcssa, %for.inc7 ]
  br label %for.end9

for.end9:                                         ; preds = %for.end9.loopexit, %entry
  %t.0.lcssa = phi i32 [ undef, %entry ], [ %.lcssa.lcssa, %for.end9.loopexit ]
  ret i32 %t.0.lcssa
}

