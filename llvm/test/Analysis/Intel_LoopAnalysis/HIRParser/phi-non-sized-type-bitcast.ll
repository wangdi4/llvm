; RUN: opt -opaque-pointers=0 < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1 -disable-output | FileCheck %s

; Check that we are able to form HIR successfully with %a.addr.07 phi which
; has an AddRec form and an opaque element type (%struct.A*).

; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   (%b)[0] = &((%a.addr.07)[0]);
; CHECK: |   %bc1 = bitcast.%struct.A*.i8*(&((%a.addr.07)[0]));
; CHECK: |   %a.addr.07 = &((%struct.A*)(%bc1)[4]);
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.A = type opaque

@glob_a = external dso_local local_unnamed_addr global %struct.A*, align 8

define void @foo(%struct.A* %a, %struct.A** nocapture %b) {
entry:
  br label %for.body

for.body:
  %i.08 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  %a.addr.07 = phi %struct.A* [ %bc2, %for.body ], [ %a, %entry ]
  store %struct.A* %a.addr.07, %struct.A** %b, align 8
  %bc1 = bitcast %struct.A* %a.addr.07 to i8*
  %gep = getelementptr i8, i8*  %bc1, i64 4
  %bc2 = bitcast i8* %gep to %struct.A*
  %inc = add nuw nsw i32 %i.08, 1
  %exitcond = icmp eq i32 %inc, 10
  br i1 %exitcond, label %if.end.loopexit, label %for.body

if.end.loopexit:
  ret void
}

