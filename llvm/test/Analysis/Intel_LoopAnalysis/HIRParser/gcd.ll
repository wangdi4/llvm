; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check that the HIR for this loop is constructed in a reasonable amount of time which depends on the gcd computation of the subsrcipt expression with the big coefficient finishing in a reasonable amount of time.

; CHECK: + DO i1 = 0, (%div3.i + umax(-18014398509481984, (18014398509481984 + (-1 * %div3.i))) + -1)/u18014398509481984, 1   <DO_LOOP>
; CHECK: |   (%p1)[18014398509481984 * i1] = -512;
; CHECK: |   %0 = ptrtoint.i64*.i64(&((%p1)[18014398509481984 * i1 + 18014398509481985]));
; CHECK: |   (%p1)[18014398509481984 * i1 + 1] = %0;
; CHECK: |   (%p2)[0] = %0;
; CHECK: + END LOOP


define void @caml_alloc_shr_aux(i64 %div3.i, i64* %p1, i64* %p2) {
entry:
  br label %while.body.lr.ph.i

while.body.lr.ph.i:
  br label %while.body.i

while.body.i:                                     ; preds = %while.body.i, %while.body.lr.ph.i
  %remain.082.i = phi i64 [ %div3.i, %while.body.lr.ph.i ], [ %sub5.i, %while.body.i ]
  %prev.081.i = phi i64* [ %p1, %while.body.lr.ph.i ], [ %add.ptr.i, %while.body.i ]
  store i64 -512, i64* %prev.081.i, align 8
  %add.ptr.i = getelementptr inbounds i64, i64* %prev.081.i, i64 18014398509481984
  %sub5.i = add i64 %remain.082.i, -18014398509481984
  %add.ptr6.i = getelementptr inbounds i64, i64* %prev.081.i, i64 18014398509481985
  %0 = ptrtoint i64* %add.ptr6.i to i64
  %add.ptr7.i = getelementptr inbounds i64, i64* %prev.081.i, i64 1
  store i64 %0, i64* %add.ptr7.i, align 8
  store i64 %0, i64* %p2, align 8
  %sub.i = add i64 %remain.082.i, -18014398509481985
  %cmp4.i = icmp ugt i64 %sub.i, 18014398509481983
  br i1 %cmp4.i, label %while.body.i, label %while.end.loopexit.i

while.end.loopexit.i:
  br label %exit

exit:
  ret void
}
