; RUN: opt  -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-framework -hir-details -hir-cost-model-throttling=0 <%s  2>&1 | FileCheck %s
; RUN: opt  -passes="hir-ssa-deconstruction,print<hir>" -hir-details -hir-cost-model-throttling=0 <%s 2>&1 | FileCheck %s

; Check that we recognize 'byval' attribute in HIR Framework.

; Function: _Z4copyPiS_
;        BEGIN REGION { }
;            + DO i1 = 0, 19, 1   <DO_LOOP>
;            |   %call = @_Z4div3Pi(&((%str)[2 * i1]));
;            |   (%ptr)[2 * i1] = %call;
;            + END LOOP
;        END REGION


; CHECK:   BEGIN REGION { }
; CHECK:      + DO i64 i1 = 0, 19, 1   <DO_LOOP>
; CHECK:      |   %call = @_Z4div3Pi(&((%str)[2 * i1]));
; CHECK:      |   <LVAL-REG> NON-LINEAR i1 %call {sb:6}
; CHECK:      |   <RVAL-REG> &((LINEAR i32* %str)[LINEAR i64 2 * i1]) inbounds  {sb:12}
; CHECK:      |      <BLOB> LINEAR i32* %str {sb:5}
; CHECK:      |   <FAKE-RVAL-REG> {canUsePointeeSize}(LINEAR i32* %str)[LINEAR i64 2 * i1] inbounds  {sb:12}
; CHECK:      |      <BLOB> LINEAR i32* %str {sb:5}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress uwtable
define dso_local void @_Z4copyPiS_(i32* %str, i32* nocapture %ptr) local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %str, i64 %indvars.iv
  %call = tail call zeroext i1 @_Z4div3Pi(i32* %arrayidx)
  %conv = zext i1 %call to i32
  %arrayidx2 = getelementptr inbounds i32, i32* %ptr, i64 %indvars.iv
  store i32 %conv, i32* %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp = icmp ult i64 %indvars.iv, 38
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  ret void
}


; Check that we recognize 'byval' attribute and set corresponding type
; in HIR Framework for self mem ref arguments.

; CHECK:           + DO i64 i1 = 0, 19, 1   <DO_LOOP>
; CHECK:           |   %call = @_Z4div3Pi(&((%str)[0]));
; CHECK:           |   <LVAL-REG> NON-LINEAR i1 %call {sb:5}
; CHECK:           |   <RVAL-REG> &((LINEAR i32* %str)[i64 0]) inbounds  {sb:11}
; CHECK:           |      <BLOB> LINEAR i32* %str {sb:4}
; CHECK:           |   <FAKE-RVAL-REG> {canUsePointeeSize}(LINEAR i32* %str)[i64 0] inbounds  {sb:11}
; CHECK:           |      <BLOB> LINEAR i32* %str {sb:4}
; CHECK:           |
; CHECK:           |   (%ptr)[2 * i1] = %call;
; CHECK:           |   <LVAL-REG> {al:4}(LINEAR i32* %ptr)[LINEAR i64 2 * i1] inbounds  {sb:11}
; CHECK:           |      <BLOB> LINEAR i32* %ptr {sb:8}
; CHECK:           |   <RVAL-REG> NON-LINEAR zext.i1.i32(%call) {sb:2}
; CHECK:           |      <BLOB> NON-LINEAR i1 %call {sb:5}
; CHECK:           |
; CHECK:           + END LOOP

; Function Attrs: mustprogress uwtable
define dso_local void @_Z4copy2PiS_(i32* %str, i32* nocapture %ptr) local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %call = tail call zeroext i1 @_Z4div3Pi(i32* %str)
  %conv = zext i1 %call to i32
  %arrayidx = getelementptr inbounds i32, i32* %ptr, i64 %indvars.iv
  store i32 %conv, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp = icmp ult i64 %indvars.iv, 38
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  ret void
}

; Function Attrs: noinline
declare dso_local zeroext i1 @_Z4div3Pi(i32* byval(i32)) local_unnamed_addr


