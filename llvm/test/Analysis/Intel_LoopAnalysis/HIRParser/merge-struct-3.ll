; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; Check that %x0 and %x1 are recognized as compatible and will be included into single GEP chain.

; BEGIN REGION { }
; CHECK:      + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:      |   (%x)[i1].1.0 = i1;
; CHECK:      + END LOOP
; END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.b = type { %struct.a, %struct.a }
%struct.a = type { i64, i64 }

define void @foo(ptr nocapture %x) {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %x0 = getelementptr inbounds %struct.b, ptr %x, i64 %indvars.iv, i32 1
  %x1 = getelementptr inbounds %struct.a, ptr %x0, i64 0, i32 0
  store i64 %indvars.iv, ptr %x1, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

