; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate" -print-changed -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED

; A simple opt-predicate case without an else output loop

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   if (%n == 8)
;       |   {
;       |     (%p)[0] = 1;
;       |   }
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK:       if (%n == 8)
; CHECK:       {
; CHECK:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:          |   (%p)[0] = 1;
; CHECK:          + END LOOP
; CHECK:       }
; CHECK: END REGION

; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,hir-cg,simplifycfg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -intel-opt-report=low -disable-output 2>&1 < %s | FileCheck %s -check-prefix=OPTREPORT

; OPTREPORT:       Global optimization report for : foo
; OPTREPORT:       LOOP BEGIN
; OPTREPORT-NOT:   <Predicate Optimized v1>
; OPTREPORT:           remark #25423: Invariant If condition at line 0 hoisted out of this loop
; OPTREPORT:       LOOP END
; OPTREPORT-NOT:   LOOP BEGIN
; OPTREPORT-NOT:   <Predicate Optimized v2>
; OPTREPORT-NOT:   LOOP END

; Verify that pass is dumped with print-changed when it triggers.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED: Dump After HIROptPredicate

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32 %n, ptr %p) #0 {
entry:
  br label %for.body

for.body:
  %i = phi i32 [ 0, %entry ], [ %ip, %for.inc ]
  %cmp1 = icmp eq i32 %n, 8
  br i1 %cmp1, label %if.then, label %for.inc

if.then:
  store i32 1, ptr %p
  br label %for.inc

for.inc:
  %ip = add nsw i32 %i, 1
  %cmp = icmp slt i32 %i, 99
  br i1 %cmp, label %for.body, label %for.end

for.end:
  ret void
}
