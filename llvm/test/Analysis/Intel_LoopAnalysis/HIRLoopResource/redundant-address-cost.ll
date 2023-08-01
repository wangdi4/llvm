; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-loop-resource>" -disable-output 2>&1 | FileCheck %s

; Check that the address cost of (%list.addr.011)[0].1 is accounted only once for the load.
; The addressOf ref &((%list.addr.011)[0].1) should be ignored.

; Input HIR-
; + UNKNOWN LOOP i1
; |   <i1 = 0>
; |   while.body:
; |   (%p)[i1] = (%list.addr.011)[0].1;
; |   (%q)[i1] = &((%list.addr.011)[0].1);
; |   %1 = (%list.addr.011)[0].0;
; |   %list.addr.011 = &((%1)[0]);
; |   if (&((%1)[0]) != null)
; |   {
; |      <i1 = i1 + 1>
; |      goto while.body;
; |   }
; + END LOOP

; Verify that the number of integer operations are equal to 6.
; (%p)[i1] - address cost of 2 for stride multiplification and offset addition.
; (%q)[i1] - address cost of 2 for stride multiplification and offset addition.
; (%list.addr.011)[0].1 - address address cost of 1 for trailing offset addition.
; &((%1)[0]) != null - cost of 1 for compare operation

; CHECK: + UNKNOWN LOOP i1
; CHECK: Integer Operations: 6
; CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.s1 = type { ptr, i32 }

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(ptr %list, i32 %t, ptr nocapture %p, ptr nocapture %q) {
entry:
  %cmp10 = icmp eq ptr %list, null
  br i1 %cmp10, label %while.end, label %while.body.preheader

while.body.preheader:                             ; preds = %entry
  br label %while.body

while.body:                                       ; preds = %while.body.preheader, %while.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %while.body ], [ 0, %while.body.preheader ]
  %list.addr.011 = phi ptr [ %1, %while.body ], [ %list, %while.body.preheader ]
  %a = getelementptr inbounds %struct.s1, ptr %list.addr.011, i64 0, i32 1
  %0 = load i32, ptr %a, align 8
  %arrayidx = getelementptr inbounds i32, ptr %p, i64 %indvars.iv
  store i32 %0, ptr %arrayidx, align 4
  %arrayidx3 = getelementptr inbounds ptr, ptr %q, i64 %indvars.iv
  store ptr %a, ptr %arrayidx3, align 8
  %next = getelementptr inbounds %struct.s1, ptr %list.addr.011, i64 0, i32 0
  %1 = load ptr, ptr %next, align 8
  %indvars.iv.next = add nuw i64 %indvars.iv, 1
  %cmp = icmp eq ptr %1, null
  br i1 %cmp, label %while.end.loopexit, label %while.body

while.end.loopexit:                               ; preds = %while.body
  br label %while.end

while.end:                                        ; preds = %while.end.loopexit, %entry
  ret void
}

