; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-opt-predicate,print<hir>" -debug-only=hir-opt-predicate -disable-output  < %s 2>&1 | FileCheck %s
;
; This test case checks that the number of exists in an empty multi-exit loop
; won't be computed during remove reduntant nodes, and an assertion is not
; triggered.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, 3, 1   <DO_MULTI_EXIT_LOOP>
;       |   switch(%n)
;       |   {
;       |   case 2:
;       |      if (%t == 0)
;       |      {
;       |         goto cleanup;
;       |      }
;       |      break;
;       |   case 1:
;       |      if (%t == 0)
;       |      {
;       |         goto for.check;
;       |      }
;       |      break;
;       |   case 5:
;       |      goto for.check;
;       |   default:
;       |      break;
;       |   }
;       |   (%a)[i1] = i1;
;       |   for.check:
;       + END LOOP
; END REGION

; HIR from debug print. The loops in the Then branch for case 1 and in case 5
; are dead loops. They will be removed by the remove redundant nodes proccess.

; CHECK: BEGIN REGION { }
; CHECK:       switch(%n)
; CHECK:       {
; CHECK:       case 2:
; CHECK:          if (%t == 0)
; CHECK:          {
; CHECK:             + DO i1 = 0, 3, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:             |   goto cleanup;
; CHECK:             |   (%a)[i1] = i1;
; CHECK:             |   for.check.35:
; CHECK:             + END LOOP
; CHECK:          }
; CHECK:          else
; CHECK:          {
; CHECK:             + DO i1 = 0, 3, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:             |   (%a)[i1] = i1;
; CHECK:             |   for.check.35.48:
; CHECK:             + END LOOP
; CHECK:          }
; CHECK:          break;
; CHECK:       case 1:
; CHECK:          if (%t == 0)
; CHECK:          {
; CHECK:             + DO i1 = 0, 3, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:             |   goto for.check.39;
; CHECK:             |   (%a)[i1] = i1;
; CHECK:             |   for.check.39:
; CHECK:             + END LOOP
; CHECK:          }
; CHECK:          else
; CHECK:          {
; CHECK:             + DO i1 = 0, 3, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:             |   (%a)[i1] = i1;
; CHECK:             |   for.check.39.52:
; CHECK:             + END LOOP
; CHECK:          }
; CHECK:          break;
; CHECK:       case 5:
; CHECK:          + DO i1 = 0, 3, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:          |   goto for.check.43;
; CHECK:          |   (%a)[i1] = i1;
; CHECK:          |   for.check.43:
; CHECK:          + END LOOP
; CHECK:          break;
; CHECK:       default:
; CHECK:          + DO i1 = 0, 3, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:          |   (%a)[i1] = i1;
; CHECK:          |   for.check:
; CHECK:          + END LOOP
; CHECK:          break;
; CHECK:       }
; CHECK: END REGION

; HIR after transformation pass

; CHECK: BEGIN REGION { modified }
; CHECK:       switch(%n)
; CHECK:       {
; CHECK:       case 2:
; CHECK:          if (%t == 0)
; CHECK:          {
; CHECK:             goto cleanup;
; CHECK:          }
; CHECK:          else
; CHECK:          {
; CHECK:             + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK:             |   (%a)[i1] = i1;
; CHECK:             + END LOOP
; CHECK:          }
; CHECK:          break;
; CHECK:       case 1:
; CHECK:          if (%t != 0)
; CHECK:          {
; CHECK:             + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK:             |   (%a)[i1] = i1;
; CHECK:             + END LOOP
; CHECK:          }
; CHECK:          break;
; CHECK:       case 5:
; CHECK:          break;
; CHECK:       default:
; CHECK:          + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK:          |   (%a)[i1] = i1;
; CHECK:          + END LOOP
; CHECK:          break;
; CHECK:       }
; CHECK: END REGION

; Verify that we emit remarks for loops which are optimized away.

; Note that the versions are not printed in order which needs to be fixed.
; JR CMPLRLLVM-52909 was opened for this issue.

; Also, note that we cannot correctly distinguish between single-iteration loop
; and a dead loop in all cases. This depends on where the unconditional goto was
; found inside the loop body. v1 loop is reported as being a single iteration
; loop but is infact a dead loop as goto is the first child.

; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,hir-cg,simplifycfg,intel-ir-optreport-emitter" -intel-opt-report=low -disable-output %s 2>&1 | FileCheck %s -check-prefix=OPTREPORT

; OPTREPORT:      LOOP BEGIN
; OPTREPORT-NEXT: <Predicate Optimized v1>
; OPTREPORT-NEXT:     remark #25424: Invariant Switch condition at line 0 hoisted out of this loop
; OPTREPORT-NEXT:     remark #25423: Invariant If condition at line 0 hoisted out of this loop
; OPTREPORT-NEXT:     remark #25261: Single iteration loop optimized away
; OPTREPORT-NEXT: LOOP END

; OPTREPORT:      LOOP BEGIN
; OPTREPORT-NEXT: <Predicate Optimized v6>
; OPTREPORT-NEXT: LOOP END

; OPTREPORT:      LOOP BEGIN
; OPTREPORT-NEXT: <Predicate Optimized v3>
; OPTREPORT-NEXT:     remark #25260: Dead loop optimized away
; OPTREPORT-NEXT: LOOP END

; OPTREPORT:      LOOP BEGIN
; OPTREPORT-NEXT: <Predicate Optimized v5>
; OPTREPORT-NEXT: LOOP END

; OPTREPORT:      LOOP BEGIN
; OPTREPORT-NEXT: <Predicate Optimized v2>
; OPTREPORT-NEXT:     remark #25423: Invariant If condition at line 0 hoisted out of this loop
; OPTREPORT-NEXT:     remark #25260: Dead loop optimized away
; OPTREPORT-NEXT: LOOP END

; OPTREPORT:      LOOP BEGIN
; OPTREPORT-NEXT: <Predicate Optimized v4>
; OPTREPORT-NEXT: LOOP END

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


define dso_local i64 @main(ptr %a, i64 %n, i64 %t) {
entry:
  %tobool = icmp eq i64 %t, 0
  br label %for.body

for.body:                                         
  %iv = phi i64 [ 0, %entry ], [ %inc, %for.check ]
  %inc = add nuw nsw i64 %iv, 1
  switch i64 %n, label %default.case [
    i64 2, label %case.01
    i64 1, label %case.02
    i64 5, label %case.03
  ]

case.01:
  br i1 %tobool, label %cleanup, label %if.then

case.02:
  br i1 %tobool, label %for.check, label %if.then

case.03:
  br label %for.check

default.case:
  br label %if.then

if.then:                                          
  %arrayidx = getelementptr inbounds i64, ptr %a, i64 %iv
  store i64 %iv, ptr %arrayidx
  br label %for.check

for.check:
  %exitcond = icmp eq i64 %inc, 4
  br i1 %exitcond, label %cleanup, label %for.body

cleanup:                                          
  ret i64 0
}
