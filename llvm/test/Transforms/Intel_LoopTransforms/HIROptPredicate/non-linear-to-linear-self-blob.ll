; RUN: opt -hir-details -disable-output -hir-ssa-deconstruction -hir-opt-predicate -print-after=hir-opt-predicate -S < %s 2>&1 | FileCheck %s
; RUN: opt -hir-details -disable-output -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   if (%n == 8)
;       |   {
;       |      %x.01 = %x.01  +  1;
;       |   }
;       |   (%p)[i1] = %x.01;
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK:       if (%n == 8)
; CHECK:       {
; CHECK:         + DO i32 i1 = 0, 99, 1
; CHECK:         |   %x.01 = %x.01  +  1;
; CHECK:         |   (%p)[i1] = %x.01;
; CHECK:         + END LOOP
; CHECK:       }
; CHECK:       else
; CHECK:       {
; CHECK:         + DO i32 i1 = 0, 99, 1
; CHECK:         |   (%p)[i1] = %x.01;
; CHECK:         |   <RVAL-REG> LINEAR i32 %x.01 {sb:3}
; CHECK:         + END LOOP
; CHECK:       }
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32 %n, i32* %p) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.02 = phi i32 [ 0, %entry ], [ %inc2, %for.inc ]
  %x.01 = phi i32 [ 0, %entry ], [ %x.1, %for.inc ]
  %cmp1 = icmp eq i32 %n, 8
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %inc = add nsw i32 %x.01, 1
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %x.1 = phi i32 [ %inc, %if.then ], [ %x.01, %for.body ]
  %idxprom = sext i32 %i.02 to i64
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %idxprom
  store i32 %x.1, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %inc2 = add nsw i32 %i.02, 1
  %cmp = icmp slt i32 %inc2, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  ret void
}

