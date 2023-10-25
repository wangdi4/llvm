; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-idiom,print<hir>" -debug-only=hir-idiom -disable-output 2>&1 < %s | FileCheck %s

; CHECK: Skipping small trip count loops

; CHECK-NOT: modified

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>  <LEGAL_MAX_TC = 4294967295> <avg_trip_count = 6>
; CHECK: |   (%p)[i1 + 1] = null;
; CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind uwtable
define void @foo(ptr nocapture %p, i32 %n) local_unnamed_addr #0 {
entry:
  %tobool1 = icmp eq i32 %n, 0
  br i1 %tobool1, label %while.end, label %while.body.preheader

while.body.preheader:                             ; preds = %entry
  br label %while.body

while.body:                                       ; preds = %while.body.preheader, %while.body
  %p.addr.03 = phi ptr [ %incdec.ptr, %while.body ], [ %p, %while.body.preheader ]
  %n.addr.02 = phi i32 [ %dec, %while.body ], [ %n, %while.body.preheader ]
  %dec = add nsw i32 %n.addr.02, -1
  %incdec.ptr = getelementptr inbounds ptr, ptr %p.addr.03, i64 1
  store ptr null, ptr %incdec.ptr, align 8
  %tobool = icmp eq i32 %dec, 0
  br i1 %tobool, label %while.end.loopexit, label %while.body, !llvm.loop !0

while.end.loopexit:                               ; preds = %while.body
  br label %while.end

while.end:                                        ; preds = %while.end.loopexit, %entry
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.intel.loopcount_average", i32 6}

