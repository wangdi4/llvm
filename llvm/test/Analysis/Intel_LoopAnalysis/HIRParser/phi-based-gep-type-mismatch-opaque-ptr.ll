; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1 -disable-output | FileCheck %s

; Verify that the the GEP %3 can be parsed as (%g)[i1].1 even though there is
; a mismatch between its base ptr element type ({ i32, i32 }) and the
; type obtained from %incdec.ptr (%struct.a) which is the update value of
; inductive phi %g.addr.04.

; CHECK: + DO i1 = 0, (-1 * ptrtoint.ptr.i64(%g) + ptrtoint.ptr.i64(%h) + -8)/u8, 1   <DO_LOOP>
; CHECK: |   %2 = (i32*)(%g)[i1];
; CHECK: |   (%o)[0].0 = %2;
; CHECK: |   %4 = (%g)[i1].1;
; CHECK: |   (%o)[0].1 = %4;
; CHECK: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.a = type { { float, float } }

define void @_Z2ajP1aS0_S0_(ptr readonly %g, ptr readnone %h, ptr nocapture %o) {
entry:
  %0 = getelementptr inbounds { i32, i32 }, ptr %o, i64 0, i32 0
  %1 = getelementptr inbounds { i32, i32 }, ptr %o, i64 0, i32 1
  %cmp.not3 = icmp eq ptr %g, %h
  br i1 %cmp.not3, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %g.addr.04 = phi ptr [ %incdec.ptr, %for.body ], [ %g, %for.body.preheader ]
  %2 = load i32, ptr %g.addr.04, align 4
  store i32 %2, ptr %0, align 4
  %3 = getelementptr inbounds { i32, i32 }, ptr %g.addr.04, i64 0, i32 1
  %4 = load i32, ptr %3, align 4
  store i32 %4, ptr %1, align 4
  %incdec.ptr = getelementptr inbounds %struct.a, ptr %g.addr.04, i64 1
  %cmp.not = icmp eq ptr %incdec.ptr, %h
  br i1 %cmp.not, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

