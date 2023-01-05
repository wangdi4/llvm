; RUN: opt -passes="loop(loop-deletion)" < %s -S | FileCheck %s

; CHECK: while.body.neg:
; CHECK: while.body.neg2:

; This test validates that the optimization in intel-loop-delete.ll does /not/ trigger.

define zeroext i1 @no_delete_1(i16* %orig_p) local_unnamed_addr {
entry:
  br label %while.body.neg

while.body.neg:
  %orig_p.pn = phi i16* [ %orig_p, %entry ], [ %p.0, %while.body.neg]
  %p.0 = getelementptr inbounds i16, i16* %orig_p.pn, i64 1
  %0 = load i16, i16* %p.0, align 2
  %tobool = icmp eq i16 %0, 0
  br i1 %tobool, label %while.end, label %while.body.neg

while.end:
  %sub.ptr.lhs.cast = ptrtoint i16* %p.0 to i64 ; %p.0 must be at least %orig_p+2
  %sub.ptr.rhs.cast = ptrtoint i16* %orig_p to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast ; result must be > 2
 ; this shift may or may not zero the result, we don't know the result now
  %sub.ptr.div = lshr exact i64 %sub.ptr.sub, 4
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


define zeroext i1 @no_delete_2(i16* %orig_p) local_unnamed_addr {
entry:
  br label %while.body.neg2

while.body.neg2:
  %orig_p.pn = phi i16* [ %orig_p, %entry ], [ %p.0, %while.body.neg2 ]
  %p.0 = getelementptr inbounds i16, i16* %orig_p.pn, i64 1
  %0 = load i16, i16* %p.0, align 2
  %tobool = icmp eq i16 %0, 0
  br i1 %tobool, label %while.end, label %while.body.neg2

while.end:
  %join = phi i16* [ %p.0, %while.body.neg2 ]
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
; join2 is used in a compare 666. We don't know the result.
  %cmp2 = icmp ne i32 %join2, 666
  br i1 %cmp2, label %if.end, label %cleanup

if.end:
  br label %cleanup

cleanup:
  %retval.0 = phi i1 [ true, %if.then ], [ false, %if.end ]
  ret i1 %retval.0
}



