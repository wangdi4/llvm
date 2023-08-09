; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1 -disable-output | FileCheck %s

; Check that we are able to form HIR successfully with %a.addr.07 phi which
; has an AddRec form and an opaque element type (ptr).

; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   (%b)[0] = &((%a)[4 * i1]);
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.A = type opaque

@glob_a = external dso_local local_unnamed_addr global ptr, align 8

define void @foo(ptr %a, ptr nocapture %b) {
entry:
  br label %for.body

for.body:
  %i.08 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  %a.addr.07 = phi ptr [ %gep, %for.body ], [ %a, %entry ]
  store ptr %a.addr.07, ptr %b, align 8
  %gep = getelementptr i8, ptr  %a.addr.07, i64 4
  %inc = add nuw nsw i32 %i.08, 1
  %exitcond = icmp eq i32 %inc, 10
  br i1 %exitcond, label %if.end.loopexit, label %for.body

if.end.loopexit:
  ret void
}

