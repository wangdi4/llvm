; RUN: opt -passes="loop(loop-deletion)" < %s -S | FileCheck %s

; CHECK-NOT: while.body.test1
; CHECK: sub.ptr.div = lshr {{.*}} i64 2
; CHECK-NOT: while.body.test2
; CHECK: sub.ptr2.div = lshr {{.*}} i64 2

; The only live value out of the while.body.test1 loop is "%p.0". Some arithmetic is applied
; and then it is used to compute
; the icmp eq 0 instruction.
; We can prove that %p.0 must always be > 0 exiting the loop, and the specific value does not
; matter, even through the following computations.
; The loop can be deleted and replaced with a constant result for %p.0.

define zeroext i1 @_Z3fooPs(i16* %orig_p) local_unnamed_addr {
entry:
  br label %while.body.test1

while.body.test1:
  %orig_p.pn = phi i16* [ %orig_p, %entry ], [ %p.0, %while.body.test1 ]
  %p.0 = getelementptr inbounds i16, i16* %orig_p.pn, i64 1
  %0 = load i16, i16* %p.0, align 2
  %tobool = icmp eq i16 %0, 0
  br i1 %tobool, label %while.end, label %while.body.test1

while.end:
  %sub.ptr.lhs.cast = ptrtoint i16* %p.0 to i64 ; %p.0 must be at least %orig_p+2
  %sub.ptr.rhs.cast = ptrtoint i16* %orig_p to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast ; result must be > 2
  %sub.ptr.div = lshr exact i64 %sub.ptr.sub, 1
  %conv = trunc i64 %sub.ptr.div to i32
  %cmp = icmp eq i32 %conv, 0                 ; this eq is always false
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %while.end
  br label %cleanup

if.end:                                           ; preds = %while.end
  br label %cleanup

cleanup:                                          ; preds = %if.end, %if.then
  %retval.0 = phi i1 [ true, %if.then ], [ false, %if.end ]
  ret i1 %retval.0
}


; More complicated test, with some phi nodes and multiple uses of the loop result.
; All the paths out of the loop must have an icmp eq|ne 0.

define zeroext i1 @_Z3foo2Ps(i16* %orig_p) local_unnamed_addr {
entry:
  br label %while.body.test2

while.body.test2:
  %orig_p.pn = phi i16* [ %orig_p, %entry ], [ %p.0, %while.body.test2 ]
  %p.0 = getelementptr inbounds i16, i16* %orig_p.pn, i64 1
  %0 = load i16, i16* %p.0, align 2
  %tobool = icmp eq i16 %0, 0
  br i1 %tobool, label %while.end, label %while.body.test2

while.end:
  %join = phi i16* [ %p.0, %while.body.test2 ]
  %sub.ptr.lhs.cast = ptrtoint i16* %join to i64
  %sub.ptr.rhs.cast = ptrtoint i16* %orig_p to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  %sub.ptr2.div = lshr exact i64 %sub.ptr.sub, 1
  %conv = trunc i64 %sub.ptr2.div to i32
  br label %next.block

next.block:
  %join2 = phi i32 [ %conv, %while.end ]
  %orr = or i32 %join2, 4
  %cmp = icmp eq i32 %orr, 0
  br i1 %cmp, label %if.end, label %if.then

if.then:
  %cmp2 = icmp ne i32 %join2, 0
  br i1 %cmp2, label %if.end, label %cleanup

if.end:
  br label %cleanup

cleanup:
  %retval.0 = phi i1 [ true, %if.then ], [ false, %if.end ]
  ret i1 %retval.0
}

