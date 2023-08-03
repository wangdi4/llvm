; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,print<hir>" -aa-pipeline="basic-aa" -hir-details 2>&1 < %s | FileCheck %s

; Verify that complete unroll is able to successfully handle negative trip
;  counts and gives up if loops with negative trip count don't have NSW flag.

; Note that the MAX_TC_EST of i2 loop is too big. This is being set by replacing
; i1 with 2 and assuming -6 as a positive value. The loop is actually dead as
; the ZTT is never true.

; CHECK: + DO i64 i1 = 0, 2, 1   <DO_LOOP>

; CHECK: |   + Ztt: if (i1 + 1 >=u 9)
; CHECK: |   + HasSignedIV: No
; CHECK: |   + DO i64 i2 = 0, i1 + -8, 1   <DO_LOOP>  <MAX_TC_EST = 18446744073709551611>

; CHECK:         + HasSignedIV: No
; CHECK: |   |   + DO i64 i3 = 0, i2 + 9, 1   <DO_LOOP>
; CHECK: |   |   |   if ((@t)[0][i3] == 0)
; CHECK: |   |   |   {
; CHECK: |   |   |      (@ym)[0][i2 + 15] = (@i)[0][i2 + 16];
; CHECK: |   |   |   }
; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@b = dso_local local_unnamed_addr global i8 0, align 1
@k = dso_local local_unnamed_addr global i64 0, align 8
@i = dso_local local_unnamed_addr global [20 x i32] zeroinitializer, align 16
@ym = dso_local local_unnamed_addr global [20 x i32] zeroinitializer, align 16
@t = dso_local local_unnamed_addr global [20 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %0 = load i32, ptr @ym, align 16
  %1 = load i8, ptr @b, align 1
  %2 = trunc i32 %0 to i8
  %conv1 = add i8 %1, %2
  store i8 %conv1, ptr @b, align 1
  store i64 1, ptr @k, align 8
  br label %for.cond3.preheader

for.cond3.preheader:                              ; preds = %for.end20, %entry
  %indvar40 = phi i64 [ 0, %entry ], [ %indvar.next41, %for.end20 ]
  %storemerge38 = phi i64 [ 1, %entry ], [ %inc22, %for.end20 ]
  %3 = add nuw nsw i64 %indvar40, -7
  %cmp436 = icmp ult i64 %storemerge38, 9
  br i1 %cmp436, label %for.end20, label %for.cond8.preheader.preheader

for.cond8.preheader.preheader:                    ; preds = %for.cond3.preheader
  br label %for.cond8.preheader

for.cond8.preheader:                              ; preds = %for.cond8.preheader.preheader, %for.end
  %indvar = phi i64 [ %indvar.next, %for.end ], [ 0, %for.cond8.preheader.preheader ]
  %j.037 = phi i64 [ %inc19, %for.end ], [ 9, %for.cond8.preheader.preheader ]
  %4 = add i64 %indvar, 10
  %add12 = add i64 %j.037, 6
  %arrayidx13 = getelementptr inbounds [20 x i32], ptr @ym, i64 0, i64 %add12
  %add14 = add i64 %j.037, 7
  %arrayidx15 = getelementptr inbounds [20 x i32], ptr @i, i64 0, i64 %add14
  br label %for.body11

for.body11:                                       ; preds = %for.cond8.preheader, %for.inc
  %z.035 = phi i64 [ 0, %for.cond8.preheader ], [ %inc, %for.inc ]
  %arrayidx = getelementptr inbounds [20 x i32], ptr @t, i64 0, i64 %z.035
  %5 = load i32, ptr %arrayidx, align 4
  %tobool = icmp eq i32 %5, 0
  br i1 %tobool, label %if.else, label %for.inc

if.else:                                          ; preds = %for.body11
  %6 = load i32, ptr %arrayidx15, align 4
  store i32 %6, ptr %arrayidx13, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body11, %if.else
  %inc = add i64 %z.035, 1
  %exitcond = icmp eq i64 %inc, %4
  br i1 %exitcond, label %for.end, label %for.body11

for.end:                                          ; preds = %for.inc
  %inc19 = add nuw i64 %j.037, 1
  %indvar.next = add i64 %indvar, 1
  %exitcond42 = icmp eq i64 %indvar.next, %3
  br i1 %exitcond42, label %for.end20.loopexit, label %for.cond8.preheader

for.end20.loopexit:                               ; preds = %for.end
  br label %for.end20

for.end20:                                        ; preds = %for.end20.loopexit, %for.cond3.preheader
  %inc22 = add nuw i64 %storemerge38, 1
  %indvar.next41 = add i64 %indvar40, 1
  %exitcond43 = icmp eq i64 %indvar.next41, 3
  br i1 %exitcond43, label %for.end23, label %for.cond3.preheader

for.end23:                                        ; preds = %for.end20
  store i64 4, ptr @k, align 8
  ret i32 0
}

