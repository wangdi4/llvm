; Check that cases with no mem refs do not render asserts or errors

; REQUIRES: asserts
; RUN: opt -hir-ssa-deconstruction -hir-runtime-dd -print-after=hir-runtime-dd -debug-only=hir-runtime-dd -S < %s 2>&1 | FileCheck %s

; int foo(int N) {
;   int i, r = 0;
;   for(i=0;i<N;i++) {
;     r += 2*i + i;
;   }
;   return r;
; }

; CHECK: Runtime DD for loop [[LOOP:[0-9]+]]:
; CHECK: LOOPOPT_OPTREPORT: [RTDD] Loop [[LOOP]]: No opportunities

; ModuleID = 'one-ref.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @foo(i32 %N) #0 {
entry:
  %cmp.1 = icmp slt i32 0, %N
  br i1 %cmp.1, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %r.03 = phi i32 [ 0, %for.body.lr.ph ], [ %add1, %for.inc ]
  %i.02 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.inc ]
  %mul = mul nsw i32 2, %i.02
  %add = add nsw i32 %mul, %i.02
  %add1 = add nsw i32 %r.03, %add
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.02, 1
  %cmp = icmp slt i32 %inc, %N
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.inc
  %split = phi i32 [ %add1, %for.inc ]
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  %r.0.lcssa = phi i32 [ %split, %for.cond.for.end_crit_edge ], [ 0, %entry ]
  ret i32 %r.0.lcssa
}

