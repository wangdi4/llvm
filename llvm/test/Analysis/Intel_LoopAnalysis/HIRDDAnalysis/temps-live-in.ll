; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -analyze -hir-dd-analysis -hir-dd-analysis-verify=Region 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 < %s | FileCheck %s

;  Test checks that we create (=) flow edge for non live-in temps (%x.0).

; <0>          BEGIN REGION { }
; <29>               + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
; <3>                |   %0 = (@b)[0][i1];
; <5>                |   if (%0 > %n)
; <5>                |   {
; <13>               |      %x.0 = i1 + 3;
; <5>                |   }
; <5>                |   else
; <5>                |   {
; <20>               |      %x.0 = i1 + -2;
; <5>                |   }
; <23>               |   (@b)[0][i1] = %x.0;
; <29>               + END LOOP
; <0>          END REGION

; CHECK-DAG:  %x.0 --> %x.0 FLOW (=)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@b = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local i32 @foo(i32 noundef %n) local_unnamed_addr {
entry:
  %cmp26 = icmp sgt i32 %n, 0
  br i1 %cmp26, label %for.body.preheader, label %entry.for.end_crit_edge

entry.for.end_crit_edge:                          ; preds = %entry
  %.pre31 = sext i32 %n to i64
  br label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count32 = zext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %if.end
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next.pre-phi, %if.end ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @b, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp sgt i32 %0, %n
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %iv1 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %iv1, 3
  %if.pre = add nuw nsw i64 %indvars.iv, 1
  br label %if.end

if.else:                                          ; preds = %for.body
  %iv2 = trunc i64 %indvars.iv to i32
  %sub7 = sub nsw i32 %iv2, 2
  %else.pre = add nuw nsw i64 %indvars.iv, 1
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %indvars.iv.next.pre-phi = phi i64 [ %else.pre, %if.else ], [ %if.pre, %if.then ]
  %x.0 = phi i32 [ %sub7, %if.else ], [ %add, %if.then ]
  store i32 %x.0, i32* %arrayidx, align 4
  %exitcond.not = icmp eq i64 %indvars.iv.next.pre-phi, %wide.trip.count32
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %if.end
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry.for.end_crit_edge
  %idxprom10.pre-phi = phi i64 [ %.pre31, %entry.for.end_crit_edge ], [ %wide.trip.count32, %for.end.loopexit ]
  %arrayidx11 = getelementptr inbounds [100 x i32], [100 x i32]* @b, i64 0, i64 %idxprom10.pre-phi
  %ld = load i32, i32* %arrayidx11, align 4
  ret i32 %ld
}

