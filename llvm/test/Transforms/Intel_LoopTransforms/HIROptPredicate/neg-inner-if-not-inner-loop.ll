; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>" -S < %s 2>&1 | FileCheck %s

; This test checks that the inner If condition wasn't hoisted out
; since the nested Ifs aren't inside the innermost loop.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   if ((%p)[i1] == 8)
;       |   {
;       |      if (%n == 5)
;       |      {
;       |         (%p)[i1] = 1;
;       |      }
;       |   }
;       |   
;       |   + DO i2 = 0, 98, 1   <DO_LOOP>
;       |   |   (%p)[i2] = 3;
;       |   + END LOOP
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   if ((%p)[i1] == 8)
; CHECK:       |   {
; CHECK:       |      if (%n == 5)
; CHECK:       |      {
; CHECK:       |         (%p)[i1] = 1;
; CHECK:       |      }
; CHECK:       |   }
; CHECK:       |   
; CHECK:       |   + DO i2 = 0, 98, 1   <DO_LOOP>
; CHECK:       |   |   (%p)[i2] = 3;
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i64 %n,ptr %p) {
entry:
  %0 = trunc i64 %n to i32
  br label %for.body

for.body:
  %i = phi i32 [ 0, %entry ], [ %ip, %for.inc ]
  %idxprom = sext i32 %i to i64
  %arrayidx2 = getelementptr inbounds i32, ptr %p, i64 %idxprom
  %1 = load i32, ptr %arrayidx2, align 4
  %cmp1 = icmp eq i32 %1, 8
  br i1 %cmp1, label %if.then, label %cont.branch

if.then:
  %cmp2 = icmp eq i32 %0, 5
  br i1 %cmp2, label %if.inner, label %cont.branch

if.inner:
  store i32 1, ptr %arrayidx2
  br label %cont.branch

cont.branch:
  br label %for.inner.body

for.inner.body:
  %j = phi i32 [ 0, %cont.branch ], [ %jp, %for.inner.inc ]
  %idxprom2 = sext i32 %j to i64
  %arrayidx3 = getelementptr inbounds i32, ptr %p, i64 %idxprom2
  store i32 3, ptr %arrayidx3
  br label %for.inner.inc

for.inner.inc:
  %jp = add nsw i32 %j, 1
  %cmpj = icmp slt i32 %jp, 99
  br i1 %cmpj, label %for.inner.body, label %for.inc

for.inc:
  %ip = add nsw i32 %i, 1
  %cmp = icmp slt i32 %i, 99
  br i1 %cmp, label %for.body, label %for.end

for.end:
  ret void
}