; RUN: opt -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-dd-analysis -hir-dd-analysis-verify=Region -hir-cost-model-throttling=0 < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -hir-cost-model-throttling=0 -disable-output < %s 2>&1 | FileCheck %s

; Test checks that DD analysis does refine (*) edges between real and fake DDRefs when 'canUsePointeeSize' is set.

;            BEGIN REGION { }
;                  + DO i1 = 0, 39, 1   <DO_LOOP>
;                  |   (%str)[i1] = (%ptr)[i1 + -1];
;                  |   %call = @_Z4div3Pi(&((%str)[i1]));
;                  |   (%ptr)[i1] = %call;
;                  + END LOOP
;            END REGION


; CHECK: copy1
; CHECK: (%str)[i1] --> (%str)[i1] FLOW (=) (0)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: uwtable
define dso_local void @copy1(i32* %str, i32* nocapture %ptr, i32 %n) local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = add nsw i64 %indvars.iv, -1
  %arrayidx = getelementptr inbounds i32, i32* %ptr, i64 %0
  %1 = load i32, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, i32* %str, i64 %indvars.iv
  store i32 %1, i32* %arrayidx2, align 4
  %call = tail call zeroext i1 @_Z4div3Pi(i32* nonnull %arrayidx2)
  %conv = zext i1 %call to i32
  %arrayidx6 = getelementptr inbounds i32, i32* %ptr, i64 %indvars.iv
  store i32 %conv, i32* %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 40
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

;           BEGIN REGION { }
;                  + DO i1 = 0, 39, 1   <DO_LOOP>
;                  |   (%str)[i1] = (%ptr)[i1 + -1];
;                  |   %call = @_Z4div3Pi(&((%str)[i1 + 5]));
;                  |   (%ptr)[i1] = %call;
;                  + END LOOP
;            END REGION

; CHECK: copy2
; CHECK: (%str)[i1 + 5] --> (%str)[i1] ANTI (<) (5)

; Function Attrs: uwtable
define dso_local void @copy2(i32* %str, i32* nocapture %ptr, i32 %n) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = add nsw i64 %indvars.iv, -1
  %arrayidx = getelementptr inbounds i32, i32* %ptr, i64 %0
  %1 = load i32, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, i32* %str, i64 %indvars.iv
  store i32 %1, i32* %arrayidx2, align 4
  %2 = add nuw nsw i64 %indvars.iv, 5
  %arrayidx4 = getelementptr inbounds i32, i32* %str, i64 %2
  %call = tail call zeroext i1 @_Z4div3Pi(i32* nonnull %arrayidx4)
  %conv = zext i1 %call to i32
  %arrayidx6 = getelementptr inbounds i32, i32* %ptr, i64 %indvars.iv
  store i32 %conv, i32* %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 40
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

; Function Attrs: noinline
declare dso_local zeroext i1 @_Z4div3Pi(i32* byval(i32)) local_unnamed_addr

