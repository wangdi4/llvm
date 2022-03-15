; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -analyze -hir-dd-analysis -hir-dd-analysis-verify=Region 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 < %s | FileCheck %s

;  Test checks that we create (=) flow edge for non live-in temps (%x.0).

; <0>          BEGIN REGION { }
; <43>               + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 99>  <LEGAL_MAX_TC = 2147483647>
; <5>                |   if ((@b)[0][i1][i1] > %n)
; <5>                |   {
; <11>               |      %2 = (@b)[0][i1 + -1][i1];
; <14>               |      %x.0 = i1 + %2;
; <5>                |   }
; <5>                |   else
; <5>                |   {
; <19>               |      %5 = (@b)[0][i1 + 1][i1];
; <22>               |      %x.0 = -1 * i1 + %5;
; <5>                |   }
; <44>               |
; <44>               |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
; <29>               |   |   (@b)[0][i1][i2] = %x.0;
; <44>               |   + END LOOP
; <43>               + END LOOP
; <0>          END REGION

; CHECK-DAG:  %x.0 --> %x.0 FLOW (=)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@b = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local i32 @foo(i32 noundef %n) local_unnamed_addr {
entry:
  %cmp1550 = icmp sgt i32 %n, 0
  br i1 %cmp1550, label %for.body.preheader, label %entry.for.end23_crit_edge

entry.for.end23_crit_edge:                        ; preds = %entry
  %.pre = sext i32 %n to i64
  br label %for.end23

for.body.preheader:                               ; preds = %entry
  %wide.trip.count5961 = zext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc21
  %indvars.iv55 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next56, %for.inc21 ]
  %arrayidx2 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @b, i64 0, i64 %indvars.iv55, i64 %indvars.iv55
  %0 = load i32, i32* %arrayidx2, align 4
  %cmp3 = icmp sgt i32 %0, %n
  br i1 %cmp3, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %1 = add nsw i64 %indvars.iv55, -1
  %arrayidx7 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @b, i64 0, i64 %1, i64 %indvars.iv55
  %2 = load i32, i32* %arrayidx7, align 4
  %3 = trunc i64 %indvars.iv55 to i32
  %add = add nsw i32 %2, %3
  br label %for.body16.preheader

if.else:                                          ; preds = %for.body
  %4 = add nuw nsw i64 %indvars.iv55, 1
  %arrayidx12 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @b, i64 0, i64 %4, i64 %indvars.iv55
  %5 = load i32, i32* %arrayidx12, align 4
  %6 = trunc i64 %indvars.iv55 to i32
  %sub13 = sub nsw i32 %5, %6
  br label %for.body16.preheader

for.body16.preheader:                             ; preds = %if.then, %if.else
  %x.0 = phi i32 [ %add, %if.then ], [ %sub13, %if.else ]
  br label %for.body16

for.body16:                                       ; preds = %for.body16.preheader, %for.body16
  %indvars.iv = phi i64 [ 0, %for.body16.preheader ], [ %indvars.iv.next, %for.body16 ]
  %arrayidx20 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @b, i64 0, i64 %indvars.iv55, i64 %indvars.iv
  store i32 %x.0, i32* %arrayidx20, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count5961
  br i1 %exitcond.not, label %for.inc21, label %for.body16

for.inc21:                                        ; preds = %for.body16
  %indvars.iv.next56 = add nuw nsw i64 %indvars.iv55, 1
  %exitcond60.not = icmp eq i64 %indvars.iv.next56, %wide.trip.count5961
  br i1 %exitcond60.not, label %for.end23.loopexit, label %for.body

for.end23.loopexit:                               ; preds = %for.inc21
  br label %for.end23

for.end23:                                        ; preds = %for.end23.loopexit, %entry.for.end23_crit_edge
  %idxprom24.pre-phi = phi i64 [ %.pre, %entry.for.end23_crit_edge ], [ %wide.trip.count5961, %for.end23.loopexit ]
  %sub26 = add nsw i32 %n, -1
  %idxprom27 = sext i32 %sub26 to i64
  %arrayidx28 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @b, i64 0, i64 %idxprom24.pre-phi, i64 %idxprom27
  %7 = load i32, i32* %arrayidx28, align 4
  ret i32 %7
}

