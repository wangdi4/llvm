; RUN: opt < %s -hir-ssa-deconstruction -analyze -hir-framework -hir-details-dims | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" -hir-details-dims 2>&1 -disable-output | FileCheck %s

; Check that phi based RegDDRef of opaque type is formed corectly with a zero stride.

;      BEGIN REGION { }
;        + DO i1 = 0, %n + -1, 1
; CHECK: |   (%b)[0:0:8(%struct.A**:0)] = &((%a.addr.07)[0:0:0(%struct.A*:0)]);
;        |   %1 = (@glob_a)[0:0:8(%struct.A**:0)];
;        |   %a.addr.07 = &((%1)[0:0:0(%struct.A*:0)]);
;        + END LOOP
;      END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.A = type opaque

@glob_a = external dso_local local_unnamed_addr global %struct.A*, align 8

define %struct.A* @foo(%struct.A* %a, %struct.A** nocapture %b, i32 %n) {
entry:
  %0 = load %struct.A*, %struct.A** @glob_a, align 8
  %cmp = icmp eq %struct.A* %0, %a
  %cmp16 = icmp sgt i32 %n, 0
  %or.cond = and i1 %cmp, %cmp16
  br i1 %or.cond, label %for.body.preheader, label %if.end

for.body.preheader:
  br label %for.body

for.body:
  %i.08 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %a.addr.07 = phi %struct.A* [ %1, %for.body ], [ %a, %for.body.preheader ]
  store %struct.A* %a.addr.07, %struct.A** %b, align 8
  %1 = load %struct.A*, %struct.A** @glob_a, align 8
  %inc = add nuw nsw i32 %i.08, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %if.end.loopexit, label %for.body

if.end.loopexit:
  %.lcssa = phi %struct.A* [ %1, %for.body ]
  br label %if.end

if.end:
  %a.addr.1 = phi %struct.A* [ %a, %entry ], [ %.lcssa, %if.end.loopexit ]
  ret %struct.A* %a.addr.1
}

