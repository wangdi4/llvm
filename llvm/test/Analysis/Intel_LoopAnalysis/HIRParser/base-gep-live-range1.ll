; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Verify that the loop is parsed correctly. We form the following SCC:
; %s.addr.3.lcssa -> %s.addr.1 -> %s.addr
; %s.addr.1 is marked with live range metadata. That should prevent the use
; of %s.addr.1 in %s.addr.2 from tracing back incorrectly.

; CHECK: + DO i1 = 0, %dec.in + -1, 1   <DO_LOOP>
; CHECK: |   %first.addr.out = &((%first.addr)[0]);
; CHECK: |   (%s.addr)[0] = %stval;
; CHECK: |   %0 = (%addr.in)[-1 * i1 + %dec.in + -1];
; CHECK: |   %s.addr = &((%s.addr)[1]);
; CHECK: |   %s.addr.out = &((%s.addr)[0]);
; CHECK: |   if (%0 > 0)
; CHECK: |   {
; CHECK: |      + DO i2 = 0, %0 + smax(-2, (-1 + (-1 * %0))) + 1, 1   <DO_LOOP>
; CHECK: |      |   %incdec.ptr38.i = &((%first.addr.out)[i2 + 1]);
; CHECK: |      |   %1 = (%first.addr.out)[i2];
; CHECK: |      |   (%s.addr.out)[i2] = %1;
; CHECK: |      |   %s.addr.3 = &((%s.addr.out)[i2 + 1]);
; CHECK: |      + END LOOP
; CHECK: |
; CHECK: |      %first.addr = &((%incdec.ptr38.i)[0]);
; CHECK: |      %s.addr = &((%s.addr.3)[0]);
; CHECK: |   }
; CHECK: + END LOOP


define void @foo(i64 %dec.in, i8* %first.in, i8* %s.in, i8 %stval, i8* %addr.in) {
entry:
  br label %while.body29.i

while.body29.i:                                   ; preds = %entry, %while.cond26.i.loopexit
  %dec27.i456.in = phi i64 [ %dec27.i456, %while.cond26.i.loopexit ], [ %dec.in, %entry ]
  %first.addr = phi i8* [ %first.addr.lcssa, %while.cond26.i.loopexit ], [ %first.in, %entry ]
  %s.addr = phi i8* [ %s.addr.3.lcssa, %while.cond26.i.loopexit ], [ %s.in, %entry ]
  %dec27.i456 = add i64 %dec27.i456.in, -1
  store i8 %stval, i8* %s.addr, align 1
  %arrayidx32.i = getelementptr inbounds i8, i8* %addr.in, i64 %dec27.i456
  %0 = load i8, i8* %arrayidx32.i, align 1
  %s.addr.1 = getelementptr inbounds i8, i8* %s.addr, i64 1
  %cmp35.i446 = icmp sgt i8 %0, 0
  br i1 %cmp35.i446, label %for.body37.i.preheader, label %while.cond26.i.loopexit

for.body37.i.preheader:                           ; preds = %while.body29.i
  br label %for.body37.i

for.body37.i:                                     ; preds = %for.body37.i.preheader, %for.body37.i
  %s.addr.2 = phi i8* [ %s.addr.3, %for.body37.i ], [ %s.addr.1, %for.body37.i.preheader ]
  %i31.0.i448 = phi i8 [ %dec41.i, %for.body37.i ], [ %0, %for.body37.i.preheader ]
  %first.addr.1 = phi i8* [ %incdec.ptr38.i, %for.body37.i ], [ %first.addr, %for.body37.i.preheader ]
  %incdec.ptr38.i = getelementptr inbounds i8, i8* %first.addr.1, i64 1
  %1 = load i8, i8* %first.addr.1, align 1
  store i8 %1, i8* %s.addr.2, align 1
  %dec41.i = add nsw i8 %i31.0.i448, -1
  %s.addr.3 = getelementptr inbounds i8, i8* %s.addr.2, i64 1
  %cmp35.i = icmp sgt i8 %i31.0.i448, 1
  br i1 %cmp35.i, label %for.body37.i, label %while.cond26.i.loopexit.loopexit

while.cond26.i.loopexit.loopexit:                 ; preds = %for.body37.i
  %incdec.ptr38.i.lcssa = phi i8* [ %incdec.ptr38.i, %for.body37.i ]
  %s.addr.3.lcssa558 = phi i8* [ %s.addr.3, %for.body37.i ]
  br label %while.cond26.i.loopexit

while.cond26.i.loopexit:                          ; preds = %while.cond26.i.loopexit.loopexit, %while.body29.i
  %first.addr.lcssa = phi i8* [ %first.addr, %while.body29.i ], [ %incdec.ptr38.i.lcssa, %while.cond26.i.loopexit.loopexit ]
  %s.addr.3.lcssa = phi i8* [ %s.addr.1, %while.body29.i ], [ %s.addr.3.lcssa558, %while.cond26.i.loopexit.loopexit ]
  %tobool28.i = icmp eq i64 %dec27.i456, 0
  br i1 %tobool28.i, label %loop.exit, label %while.body29.i

loop.exit:
  ret void
}
