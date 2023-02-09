; RUN: opt -opaque-pointers=0 < %s -passes="hir-ssa-deconstruction,print<hir-framework>,hir-temp-cleanup,print<hir-framework>" 2>&1 | FileCheck %s

; Check that &((i8*)(%0)[0] USE will not be replaced.
; It seems to be legal to replace it by (i8**)(%a)[i1], but
; performance-wise consequences are not very clear.

; Before:
; CHECK: Function
;
;  BEGIN REGION { }
;        + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK: |   %0 = (%a)[i1];
; CHECK: |   if (&((i8*)(%0)[0]) == &((%orig)[0]))
;        |   {
;        |      (%a)[i1] = null;
;        |   }
;        + END LOOP
;  END REGION
;
; After:
; CHECK: Function
;
;  BEGIN REGION { }
;        + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK: |   %0 = (%a)[i1];
; CHECK: |   if (&((i8*)(%0)[0]) == &((%orig)[0]))
;        |   {
;        |      (%a)[i1] = null;
;        |   }
;        + END LOOP
;  END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i8* readnone %orig, i32** nocapture %a, i32 %n) {
entry:
  %cmp10 = icmp sgt i32 %n, 0
  br i1 %cmp10, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:
  br label %for.cond.cleanup

for.cond.cleanup:
  ret void

for.body:
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %if.end ]
  %arrayidx = getelementptr inbounds i32*, i32** %a, i64 %indvars.iv
  %0 = load i32*, i32** %arrayidx, align 8
  %c = bitcast i32* %0 to i8*
  %cmp1 = icmp eq i8* %c, %orig
  br i1 %cmp1, label %if.then, label %if.end

if.then:
  store i32* null, i32** %arrayidx, align 8
  br label %if.end

if.end:
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

