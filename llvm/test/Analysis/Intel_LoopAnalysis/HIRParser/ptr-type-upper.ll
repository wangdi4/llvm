; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -disable-output 2>&1 | FileCheck %s

; Verify that we do not compfail on this loop.
; The trip count of this loop is formed using the combination of pointer IV
; %in.addr.4444.i and integer IV %lc.4442.i.

; The trip count expression is assigned a pointer type by ScalarEvolution
; which doesn't make much sense.

; Loop formation picked up the IV type as i32 as it found integer IV(phi)
; first. This inconsistency in IV and trip count type triggered assertion
; in parser.

; CHECK: DO i1 = 0, trunc.i64.i32(umin((-1 + (-1 * ptrtoint.ptr.i64(%in.addr.3454.i)) + umax((1 + ptrtoint.ptr.i64(%in.addr.3454.i)), ptrtoint.ptr.i64(%add.ptr2.i))), (zext.i32.i64((-1 + (-1 * %lc.3452.i) + smax(zext.i6.i32(trunc.i64.i6(%t80)), (8 + %lc.3452.i)))) /u 8))), 1


define void @foo(ptr %in.addr.3454.i, i64 %c.3453.i, i32 %lc.3452.i, ptr %add.ptr2.i, i64 %t80) {
entry:
  %and.i.i210 = and i64 %t80, 63
  %conv63.i = trunc i64 %and.i.i210 to i32
  br label %while.body67.i

while.body67.i:                                   ; preds = %while.body67.i, %entry
  %in.addr.4444.i = phi ptr [ %incdec.ptr69.i, %while.body67.i ], [ %in.addr.3454.i, %entry ]
  %c.4443.i = phi i64 [ %or71.i, %while.body67.i ], [ %c.3453.i, %entry ]
  %lc.4442.i = phi i32 [ %add72.i, %while.body67.i ], [ %lc.3452.i, %entry ]
  %shl68.i = shl i64 %c.4443.i, 8
  %incdec.ptr69.i = getelementptr inbounds i8, ptr %in.addr.4444.i, i64 1
  %ld = load i8, ptr %in.addr.4444.i, align 1
  %conv70.i = zext i8 %ld to i64
  %or71.i = or i64 %shl68.i, %conv70.i
  %add72.i = add nsw i32 %lc.4442.i, 8
  %cmp65.i = icmp slt i32 %add72.i, %conv63.i
  %cmp66.i = icmp ult ptr %incdec.ptr69.i, %add.ptr2.i
  %cmp3 = and i1 %cmp65.i, %cmp66.i
  br i1 %cmp3, label %while.body67.i, label %while.end73.i.loopexit

while.end73.i.loopexit:                           ; preds = %while.body67.i
  %incdec.ptr69.i.lcssa = phi ptr [ %incdec.ptr69.i, %while.body67.i ]
  %or71.i.lcssa = phi i64 [ %or71.i, %while.body67.i ]
  %add72.i.lcssa = phi i32 [ %add72.i, %while.body67.i ]
  %cmp65.i.lcssa = phi i1 [ %cmp65.i, %while.body67.i ]
  ret void
}
