; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,print<hir>" 2>&1 < %s | FileCheck %s

; Verify that last value computation doesn't move %t.liveout = 1; to postexit
; incorrectly. Domination utility was incorrectly returning true for temp
; definition dominating the last child ('latch:' label) of the loop.

; CHECK-NOT: modified

; CHECK: + DO i1 = 0, -1 * ptrtoint.ptr.i64(%ptr1) + ptrtoint.ptr.i64(%ptr.end) + -1, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   switch((%ptr1)[i1])
; CHECK: |   {
; CHECK: |   case 126:
; CHECK: |      if ((%ptr1)[i1 + 1] == 126)
; CHECK: |      {
; CHECK: |         %ptr.ld = (%ptr2)[0];
; CHECK: |         if ((%ptr.ld)[2] == 0)
; CHECK: |         {
; CHECK: |            goto latch;
; CHECK: |         }
; CHECK: |         goto early.exit;
; CHECK: |      }
; CHECK: |      goto latch;
; CHECK: |   case 64:
; CHECK: |      break;
; CHECK: |   case 94:
; CHECK: |      break;
; CHECK: |   default:
; CHECK: |      goto latch;
; CHECK: |   }
; CHECK: |   %t.liveout = 1;
; CHECK: |   latch:
; CHECK: + END LOOP


define void @foo(ptr %ptr1, ptr %ptr2, i8 %t1952, ptr %ptr.end) {
entry:
  br label %loop

loop:                                             ; preds = %latch, %entry
  %t.liveout = phi i8 [ %t.liveout.merge, %latch ], [ %t1952, %entry ]
  %t1999 = phi ptr [ %t2015, %latch ], [ %ptr1, %entry ]
  %t2000 = load i8, ptr %t1999, align 1
  switch i8 %t2000, label %latch [
    i8 126, label %t2001
    i8 64, label %t2010
    i8 94, label %t2010
  ]

t2001:                                             ; preds = %loop
  %t2002 = getelementptr inbounds i8, ptr %t1999, i64 1
  %t2003 = load i8, ptr %t2002, align 1
  %t2004 = icmp eq i8 %t2003, 126
  br i1 %t2004, label %t2005, label %latch

t2005:                                             ; preds = %t2001
  %ptr.ld = load ptr, ptr %ptr2, align 8
  %t2007 = getelementptr inbounds i64, ptr %ptr.ld, i64 2
  %t2008 = load i64, ptr %t2007, align 8
  %t2009 = icmp eq i64 %t2008, 0
  br i1 %t2009, label %latch, label %early.exit

t2010:                                             ; preds = %loop, %loop
  br label %latch

latch:                                             ; preds = %t2010, %t2005, %t2001, %loop
  %t.liveout.merge = phi i8 [ 1, %t2010 ], [ %t.liveout, %t2005 ], [ %t.liveout, %t2001 ], [ %t.liveout, %loop ]
  %t2015 = getelementptr inbounds i8, ptr %t1999, i64 1
  %t2016 = icmp eq ptr %t2015, %ptr.end
  br i1 %t2016, label %exit, label %loop

exit:                                             ; preds = %latch
  %t2019 = phi i8 [ %t.liveout.merge, %latch ]
  ret void

early.exit:                                             ; preds = %2005
  %t2272 = phi ptr [ %ptr.ld, %t2005 ]
  ret void
}

