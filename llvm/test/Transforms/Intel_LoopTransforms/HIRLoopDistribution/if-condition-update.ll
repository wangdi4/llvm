; RUN: opt -xmain-opt-level=3 -disable-output -S -hir-loop-distribute-max-mem=2 -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-distribute-memrec -print-after=hir-loop-distribute-memrec < %s 2>&1 | FileCheck %s
; RUN: opt -xmain-opt-level=3 -disable-output -S -hir-loop-distribute-max-mem=2 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Check that loop will not be distributed over the IF statement because of condition update "%t.08" inside the loop.

; CHECK: BEGIN REGION
; CHECK:      + DO i1 = 0, 31, 1
;             |   if (%t.08 == 66)
;             |   {
;             |      %putchar = @putchar(46);
;             |      %r.09 = 1;
;             |      %t.08 = (%p)[i1 + 1];
;             |   }
; CHECK:      + END LOOP
; CHECK-NOT:  DO i1
; CHECK:END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo(i32* nocapture readonly %p) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  %conv = fptosi float %r.1 to i32
  ret i32 %conv

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %r.010 = phi float [ 0.000000e+00, %entry ], [ %r.1, %for.inc ]
  %t.09 = phi i32 [ 66, %entry ], [ %t.1, %for.inc ]
  %cmp1 = icmp eq i32 %t.09, 66
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %putchar = tail call i32 @putchar(i32 46)
  %inc = fadd float %r.010, 1.000000e+00
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body
  %t.1 = phi i32 [ %0, %if.then ], [ %t.09, %for.body ]
  %r.1 = phi float [ %inc, %if.then ], [ %r.010, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 32
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: nofree nounwind
declare i32 @putchar(i32) local_unnamed_addr #1

