; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,print<hir>" < %s 2>&1 | FileCheck %s
;
; LIT test to check that reassoc flag is sufficient to mark conditionally updated FP
; reductions as safe to vectorize.
;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK:        Function: reassocloop
; CHECK:           BEGIN REGION { }
; CHECK-NEXT:            %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; CHECK:                 DO i1 = 0, 1023, 1   <DO_LOOP>
define float @reassocloop(ptr %fp1, ptr %fp2, i1 %both) {
entry:
  br label %for.body

for.body:
  %sum.09 = phi float [ 0.000000e+00, %entry ], [ %sum.1, %for.inc ]
  %index.08 = phi i64 [ 0, %entry ], [ %inc, %for.inc ]
  %arrayidx = getelementptr inbounds float, ptr %fp1, i64 %index.08
  %0 = load float, ptr %arrayidx, align 4
  %cmp1 = fcmp fast ogt float %0, 0.000000e+00
  br i1 %cmp1, label %if.then, label %for.inc

if.then:
  %arrayidx1 = getelementptr inbounds float, ptr %fp2, i64 %index.08
  %1 = load float, ptr %arrayidx1, align 4
  %add2 = fadd reassoc float %1, %sum.09
  br label %for.inc

for.inc:
  %sum.1 = phi float [ %add2, %if.then ], [ %sum.09, %for.body ]
  %inc = add nuw nsw i64 %index.08, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:
  %sum.1.lcssa = phi float [ %sum.1, %for.inc ]
  ret float %sum.1.lcssa
}

; CHECK:        Function: noreassocloop
; CHECK:           BEGIN REGION { }
; CHECK-NEXT:             DO i1 = 0, 1023, 1   <DO_LOOP>
define float @noreassocloop(ptr %fp1, ptr %fp2, i1 %both) {
entry:
  br label %for.body

for.body:
  %sum.09 = phi float [ 0.000000e+00, %entry ], [ %sum.1, %for.inc ]
  %index.08 = phi i64 [ 0, %entry ], [ %inc, %for.inc ]
  %arrayidx = getelementptr inbounds float, ptr %fp1, i64 %index.08
  %0 = load float, ptr %arrayidx, align 4
  %cmp1 = fcmp fast ogt float %0, 0.000000e+00
  br i1 %cmp1, label %if.then, label %for.inc

if.then:
  %arrayidx1 = getelementptr inbounds float, ptr %fp2, i64 %index.08
  %1 = load float, ptr %arrayidx1, align 4
  %add2 = fadd float %1, %sum.09
  br label %for.inc

for.inc:
  %sum.1 = phi float [ %add2, %if.then ], [ %sum.09, %for.body ]
  %inc = add nuw nsw i64 %index.08, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:
  %sum.1.lcssa = phi float [ %sum.1, %for.inc ]
  ret float %sum.1.lcssa
}
