; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate" -print-before=hir-opt-predicate, -print-after=hir-opt-predicate -disable-output < %s 2>&1 | FileCheck %s

; Verify that removeRedundantNodes() utility is able to get rid of the else-case
; dead loop after opt-predicate. Even though the loop contained %t.015 symbase in
; its liveout set, there was no actual definition of %t.015 inside the loop.
; Since it is complicated to keep the liveout set precise, we check the actual
; definitions inside the loop. It may be possible for removeRedundantNodes() to
; remove 'dead' liveouts from the loop liveout set.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %ld = (%A)[i1];
; CHECK: |   if (%c != 0)
; CHECK: |   {
; CHECK: |      %t.015 = %ld  +  %t.015;
; CHECK: |      (%A)[i1] = %ld + 1;
; CHECK: |   }
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK: if (%c != 0)
; CHECK: {
; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %ld = (%A)[i1];
; CHECK: |   %t.015 = %ld  +  %t.015;
; CHECK: |   (%A)[i1] = %ld + 1;
; CHECK: + END LOOP
; CHECK: }

; CHECK-NOT: else

; Verify that we emit an opt-report remark about loops removed by removeRedundantNodes().

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,hir-cg,simplifycfg,intel-ir-optreport-emitter" -intel-opt-report=low -disable-output %s 2>&1 | FileCheck %s -check-prefix=OPTREPORT

; OPTREPORT:      LOOP BEGIN
; OPTREPORT-NEXT: <Predicate Optimized v1>
; OPTREPORT-NEXT:     remark #25423: Invariant If condition at line 0 hoisted out of this loop
; OPTREPORT-NEXT: LOOP END

; OPTREPORT:      LOOP BEGIN
; OPTREPORT-NEXT: <Predicate Optimized v2>
; OPTREPORT-NEXT:     remark #25260: Dead loop optimized away
; OPTREPORT-NEXT: LOOP END


source_filename = "cond-redn.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo(ptr nocapture noundef %A, i1 %c) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %t.015 = phi i32 [ 0, %entry ], [ %t.1, %for.inc ]
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %ld = load i32, ptr %arrayidx, align 4
  br i1 %c, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %add = add nsw i32 %ld, %t.015
  %add1 = add i32 %ld, 1
  store i32 %add1, ptr %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %t.1 = phi i32 [ %add, %if.then ], [ %t.015, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  %t.1.lcssa = phi i32 [ %t.1, %for.inc ]
  ret i32 %t.1.lcssa
}

