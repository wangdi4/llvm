; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup 2>&1 | FileCheck %s

; Verify that we are able to replace multiple uses of liveout copy %ll.addr.034.out in a single canon expr successfully and eliminate it.

; CHECK: Dump Before HIR Temp Cleanup

; CHECK: + DO i1 = 0, %0 + -1, 1   <DO_LOOP>
; CHECK: |   %ll.addr.034.out = %ll.addr.034;
; CHECK: |   %lp.addr.036.out1 = &((%lp.addr.036)[0]);
; CHECK: |   %1 = (%rnp)[i1];
; CHECK: |   %sub = %ll.addr.034.out  -  -1 * smax((-1 + (-1 * %ll.addr.034.out)), (-1 + (-1 * %1))) + -1;
; CHECK: |   if (-1 * smax((-1 + (-1 * %ll.addr.034.out)), (-1 + (-1 * %1))) + -1 > 0)
; CHECK: |   {
; CHECK: |      %2 = (%rpp)[i1];
; CHECK: |
; CHECK: |      + DO i2 = 0, -1 * smax((-1 + (-1 * %ll.addr.034.out)), (-1 + (-1 * %1))) + smax(-2, (-1 + (-1 * %ll.addr.034.out)), (-1 + (-1 * %1))), 1   <DO_LOOP>
; CHECK: |      |   %3 = (%2)[i2];
; CHECK: |      |   %incdec.ptr5 = &((%lp.addr.036.out1)[i2 + 1]);
; CHECK: |      |   (%lp.addr.036.out1)[i2] = %3;
; CHECK: |      + END LOOP
; CHECK: |
; CHECK: |      %lp.addr.036 = &((%incdec.ptr5)[0]);
; CHECK: |   }
; CHECK: |   %lp.addr.036.out = &((%lp.addr.036)[0]);
; CHECK: |   %ll.addr.034 = %ll.addr.034.out + smax((-1 + (-1 * %ll.addr.034.out)), (-1 + (-1 * %1))) + 1;
; CHECK: + END LOOP


; CHECK: Dump After HIR Temp Cleanup

; CHECK: + DO i1 = 0, %0 + -1, 1   <DO_LOOP>
; CHECK: |   %1 = (%rnp)[i1];
; CHECK: |   %sub = %ll.addr.034  -  -1 * smax((-1 + (-1 * %ll.addr.034)), (-1 + (-1 * %1))) + -1;
; CHECK: |   if (-1 * smax((-1 + (-1 * %ll.addr.034)), (-1 + (-1 * %1))) + -1 > 0)
; CHECK: |   {
; CHECK: |      %2 = (%rpp)[i1];
; CHECK: |
; CHECK: |      + DO i2 = 0, -1 * smax((-1 + (-1 * %ll.addr.034)), (-1 + (-1 * %1))) + smax(-2, (-1 + (-1 * %ll.addr.034)), (-1 + (-1 * %1))), 1   <DO_LOOP>
; CHECK: |      |   %incdec.ptr5 = &((%lp.addr.036)[i2 + 1]);
; CHECK: |      |   (%lp.addr.036)[i2] = (%2)[i2];
; CHECK: |      + END LOOP
; CHECK: |
; CHECK: |      %lp.addr.036 = &((%incdec.ptr5)[0]);
; CHECK: |   }
; CHECK: |   %ll.addr.034 = %ll.addr.034 + smax((-1 + (-1 * %ll.addr.034)), (-1 + (-1 * %1))) + 1;
; CHECK: + END LOOP



; ModuleID = 's_cat.c'
source_filename = "s_cat.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @s_cat(i8* nocapture %lp, i8** nocapture readonly %rpp, i64* nocapture readonly %rnp, i64* nocapture readonly %np, i64 %ll) local_unnamed_addr #0 {
entry:
  %0 = load i64, i64* %np, align 8
  %cmp33 = icmp sgt i64 %0, 0
  br i1 %cmp33, label %for.body.preheader, label %while.end11

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %lp.addr.036 = phi i8* [ %lp.addr.1.lcssa, %for.inc ], [ %lp, %for.body.preheader ]
  %i.035 = phi i64 [ %inc, %for.inc ], [ 0, %for.body.preheader ]
  %ll.addr.034 = phi i64 [ %sub, %for.inc ], [ %ll, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i64, i64* %rnp, i64 %i.035
  %1 = load i64, i64* %arrayidx, align 8
  %cmp1 = icmp slt i64 %1, %ll.addr.034
  %.ll.addr.0 = select i1 %cmp1, i64 %1, i64 %ll.addr.034
  %sub = sub nsw i64 %ll.addr.034, %.ll.addr.0
  %cmp429 = icmp sgt i64 %.ll.addr.0, 0
  br i1 %cmp429, label %while.body.preheader, label %for.inc

while.body.preheader:                             ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i8*, i8** %rpp, i64 %i.035
  %2 = load i8*, i8** %arrayidx3, align 8
  br label %while.body

while.body:                                       ; preds = %while.body.preheader, %while.body
  %f__rp.032 = phi i8* [ %incdec.ptr, %while.body ], [ %2, %while.body.preheader ]
  %nc.131 = phi i64 [ %dec, %while.body ], [ %.ll.addr.0, %while.body.preheader ]
  %lp.addr.130 = phi i8* [ %incdec.ptr5, %while.body ], [ %lp.addr.036, %while.body.preheader ]
  %dec = add nsw i64 %nc.131, -1
  %incdec.ptr = getelementptr inbounds i8, i8* %f__rp.032, i64 1
  %3 = load i8, i8* %f__rp.032, align 1
  %incdec.ptr5 = getelementptr inbounds i8, i8* %lp.addr.130, i64 1
  store i8 %3, i8* %lp.addr.130, align 1
  %cmp4 = icmp sgt i64 %nc.131, 1
  br i1 %cmp4, label %while.body, label %for.inc.loopexit

for.inc.loopexit:                                 ; preds = %while.body
  %incdec.ptr5.lcssa = phi i8* [ %incdec.ptr5, %while.body ]
  br label %for.inc

for.inc:                                          ; preds = %for.inc.loopexit, %for.body
  %lp.addr.1.lcssa = phi i8* [ %lp.addr.036, %for.body ], [ %incdec.ptr5.lcssa, %for.inc.loopexit ]
  %inc = add nuw nsw i64 %i.035, 1
  %exitcond = icmp eq i64 %inc, %0
  br i1 %exitcond, label %while.cond6.preheader.loopexit, label %for.body

while.cond6.preheader.loopexit:                   ; preds = %for.inc
  %lp.addr.1.lcssa.lcssa = phi i8* [ %lp.addr.1.lcssa, %for.inc ]
  %sub.lcssa = phi i64 [ %sub, %for.inc ]
  br label %while.end11

while.end11:                                      ; preds = %while.body9.preheader, %while.cond6.preheader
  ret void
}

