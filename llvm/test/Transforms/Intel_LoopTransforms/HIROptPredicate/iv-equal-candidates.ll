; RUN: opt -hir-ssa-deconstruction -hir-opt-var-predicate -S -print-after=hir-opt-var-predicate -disable-output  < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -aa-pipeline="basic-aa" -S -disable-output  < %s 2>&1 | FileCheck %s

; Check that both equal candidates (i1 == 20) will be handled at once.

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1
;       |   if (i1 == 20)
;       |   {
;       |      (%p)[20] = 21;
;       |   }
;       |   else
;       |   {
;       |      (%p)[i1 + 1] = i1 + -1;
;       |   }
;       |   %3 = (%a)[i1];
;       |   (%a)[i1] = %3 + 1;
;       |   %4 = (%q)[i1];
;       |   if (i1 == 20)
;       |   {
;       |      (%q)[i1] = %4 + 1;
;       |   }
;       |   else
;       |   {
;       |      (%q)[i1 + 1] = %4 + -1;
;       |   }
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, 19, 1
; CHECK:       |   (%p)[i1 + 1] = i1 + -1;
; CHECK:       |   %3 = (%a)[i1];
; CHECK:       |   (%a)[i1] = %3 + 1;
; CHECK:       |   %4 = (%q)[i1];
; CHECK:       |   (%q)[i1 + 1] = %4 + -1;
; CHECK:       + END LOOP
;
; CHECK:       (%p)[20] = 21;
; CHECK:       %3 = (%a)[20];
; CHECK:       (%a)[20] = %3 + 1;
; CHECK:       %4 = (%q)[20];
; CHECK:       (%q)[20] = %4 + 1;
;
; CHECK:       + DO i1 = 0, 78, 1
; CHECK:       |   (%p)[i1 + 22] = i1 + 20;
; CHECK:       |   %3 = (%a)[i1 + 21];
; CHECK:       |   (%a)[i1 + 21] = %3 + 1;
; CHECK:       |   %4 = (%q)[i1 + 21];
; CHECK:       |   (%q)[i1 + 22] = %4 + -1;
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* nocapture %p, i32* nocapture %q, i32* nocapture %a) local_unnamed_addr #0 {
entry:
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 20
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  ret void

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %cmp1 = icmp eq i64 %indvars.iv, 20
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  store i32 21, i32* %arrayidx, align 4
  br label %if.end

if.else:                                          ; preds = %for.body
  %0 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx4 = getelementptr inbounds i32, i32* %p, i64 %0
  %1 = trunc i64 %indvars.iv to i32
  %2 = add i32 %1, -1
  store i32 %2, i32* %arrayidx4, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %arrayidx6 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx6, align 4
  %inc = add nsw i32 %3, 1
  store i32 %inc, i32* %arrayidx6, align 4
  %arrayidx10 = getelementptr inbounds i32, i32* %q, i64 %indvars.iv
  %4 = load i32, i32* %arrayidx10, align 4
  br i1 %cmp1, label %if.then8, label %if.else14

if.then8:                                         ; preds = %if.end
  %add11 = add nsw i32 %4, 1
  store i32 %add11, i32* %arrayidx10, align 4
  br label %for.inc

if.else14:                                        ; preds = %if.end
  %sub17 = add nsw i32 %4, -1
  %5 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx20 = getelementptr inbounds i32, i32* %q, i64 %5
  store i32 %sub17, i32* %arrayidx20, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then8, %if.else14
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

