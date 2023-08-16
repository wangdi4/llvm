; RUN: opt  -passes="hir-ssa-deconstruction,print<hir>" < %s 2>&1 -disable-output | FileCheck %s

; Verify that the load of %e is parsed in terms of %add.ptr rather than @h.
; The element type obtained through %incdec.ptr2 phi's update value is %struct.f
; which does not match the result element type of the gep %add.ptr. This means
; that there is an implied bitcast between %add.ptr and its use in %incdec.ptr2.

; Previously, the ref was being parsed as (@h)[i1 + sext.i32.i64(%0)].1 and
; parsing was hitting an assertion failure.

; CHECK: + DO i1 = 0, -1 * %.pr + -1, 1   <DO_LOOP>
; CHECK: |   %1 = (%add.ptr)[i1].1;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.f = type { i8, i8 }
%struct.anon = type { [4 x %struct.a] }
%struct.a = type { i8 }

@h = dso_local global i32 0, align 4
@i = dso_local local_unnamed_addr global i32 0, align 4
@j = dso_local local_unnamed_addr global ptr null, align 8
@g = dso_local local_unnamed_addr global i32 0, align 4
@c = dso_local local_unnamed_addr global %struct.anon zeroinitializer, align 1

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @k() local_unnamed_addr {
entry:
  %0 = load i32, ptr @i, align 4
  %idx.ext = sext i32 %0 to i64
  %add.ptr = getelementptr inbounds i32, ptr @h, i64 %idx.ext
  store ptr %add.ptr, ptr @j, align 8
  %.pr = load i32, ptr @g, align 4
  %tobool.not1 = icmp eq i32 %.pr, 0
  br i1 %tobool.not1, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %inc3 = phi i32 [ %inc, %for.body ], [ %.pr, %for.body.preheader ]
  %incdec.ptr2 = phi ptr [ %incdec.ptr, %for.body ], [ %add.ptr, %for.body.preheader ]
  %e = getelementptr inbounds %struct.f, ptr %incdec.ptr2, i64 0, i32 1
  %1 = load i8, ptr %e, align 1
  %incdec.ptr = getelementptr inbounds %struct.f, ptr %incdec.ptr2, i64 1
  %inc = add nsw i32 %inc3, 1
  %tobool.not = icmp eq i32 %inc, 0
  br i1 %tobool.not, label %for.cond.for.end_crit_edge, label %for.body

for.cond.for.end_crit_edge:                       ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  ret void
}

