; RUN: llc -march=x86-64 < %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.8.0"

%struct.A = type { i8, i8, i8, i8, i8, i8, i8, i8 }

; This has byte loads interspersed with byte stores, in a single
; basic-block loop.  The upper portion should be dead, so the movb loads
; should have been changed into movzbl instead.
; CHECK-LABEL: foo1
; load:
; CHECK: movzbl
; store:
; CHECK: movb
; load, FIXME: this should be a movzbl, but the live-analysis used isn't good
;              enough to prove that upper portion of eax is unneeded.
; CHECK: movzbl
; store:
; CHECK: movb
; CHECK: ret
define void @foo1(i32 %count,
                  %struct.A* noalias nocapture %q,
                  %struct.A* noalias nocapture %p)
                    nounwind uwtable noinline ssp {
  %1 = icmp sgt i32 %count, 0
  br i1 %1, label %.lr.ph, label %._crit_edge

.lr.ph:                                           ; preds = %0
  %2 = getelementptr inbounds %struct.A, %struct.A* %q, i64 0, i32 0
  %3 = getelementptr inbounds %struct.A, %struct.A* %q, i64 0, i32 1
  br label %a4

a4:                                       ; preds = %4, %.lr.ph
  %i.02 = phi i32 [ 0, %.lr.ph ], [ %a9, %a4 ]
  %.01 = phi %struct.A* [ %p, %.lr.ph ], [ %a10, %a4 ]
  %a5 = load i8, i8* %2, align 1
  %a7 = getelementptr inbounds %struct.A, %struct.A* %.01, i64 0, i32 0
  store i8 %a5, i8* %a7, align 1
  %a8 = getelementptr inbounds %struct.A, %struct.A* %.01, i64 0, i32 1
  %a6 = load i8, i8* %3, align 1
  store i8 %a6, i8* %a8, align 1
  %a9 = add nsw i32 %i.02, 1
  %a10 = getelementptr inbounds %struct.A, %struct.A* %.01, i64 1
  %exitcond = icmp eq i32 %a9, %count
  br i1 %exitcond, label %._crit_edge, label %a4

._crit_edge:                                      ; preds = %4, %0
  ret void
}

%struct.B = type { i16, i16, i16, i16, i16, i16, i16, i16 }

; This has word loads interspersed with word stores.
; The upper portion should be dead, so the movw loads should have
; been changed into movzwl instead.
; CHECK-LABEL: foo2
; load:
; CHECK: movzwl
; store:
; CHECK: movw
; load, FIXME: this should be a movzwl, but the live-analysis used isn't good
;              enough to prove that upper portion of eax is unneeded.
; CHECK: movzwl
; store:
; CHECK: movw
; CHECK: ret
define void @foo2(i32 %count,
                  %struct.B* noalias nocapture %q,
                  %struct.B* noalias nocapture %p)
                    nounwind uwtable noinline ssp {
  %1 = icmp sgt i32 %count, 0
  br i1 %1, label %.lr.ph, label %._crit_edge

.lr.ph:                                           ; preds = %0
  %2 = getelementptr inbounds %struct.B, %struct.B* %q, i64 0, i32 0
  %3 = getelementptr inbounds %struct.B, %struct.B* %q, i64 0, i32 1
  br label %a4

a4:                                       ; preds = %4, %.lr.ph
  %i.02 = phi i32 [ 0, %.lr.ph ], [ %a9, %a4 ]
  %.01 = phi %struct.B* [ %p, %.lr.ph ], [ %a10, %a4 ]
  %a5 = load i16, i16* %2, align 2
  %a7 = getelementptr inbounds %struct.B, %struct.B* %.01, i64 0, i32 0
  store i16 %a5, i16* %a7, align 2
  %a8 = getelementptr inbounds %struct.B, %struct.B* %.01, i64 0, i32 1
  %a6 = load i16, i16* %3, align 2
  store i16 %a6, i16* %a8, align 2
  %a9 = add nsw i32 %i.02, 1
  %a10 = getelementptr inbounds %struct.B, %struct.B* %.01, i64 1
  %exitcond = icmp eq i32 %a9, %count
  br i1 %exitcond, label %._crit_edge, label %a4

._crit_edge:                                      ; preds = %4, %0
  ret void
}

; This test contains nothing but a simple byte load and store.  Since
; movb encodes smaller, we do not want to use movzbl unless in a tight loop.
; So this test checks that movb is used.
; CHECK-LABEL: foo3:
; CHECK: movb
; CHECK: movb
define void @foo3(i8 *%dst, i8 *%src) {
  %t0 = load i8, i8 *%src, align 1
  store i8 %t0, i8 *%dst, align 1
  ret void
}

; This test contains nothing but a simple word load and store.  Since
; movw and movzwl are the same size, we should always choose to use
; movzwl instead.
; CHECK-LABEL: foo4:
; CHECK: movzwl
; CHECK: movw
define void @foo4(i16 *%dst, i16 *%src) {
  %t0 = load i16, i16 *%src, align 2
  store i16 %t0, i16 *%dst, align 2
  ret void
}

; This test contains two nested loops and a byte load in the inner loop.
; The upper portion should be dead, so the movb load should have been changed
; into movzbl instead.
; CHECK-LABEL: test_bytemov_inner_loop:
; CHECK: movzbl
define void @test_bytemov_inner_loop([100 x i32]* %a, [100 x i8]* %b) nounwind uwtable {
entry:
  br label %BB1

BB1:                             ; preds = %entry, %BB4
  %i.1 = phi i32 [ 0, %entry ], [ %inc9, %BB4 ]
  br label %BB3

BB2:                                       ; preds = %BB3
  %cmp2 = icmp slt i64 %i.next, 100
  br i1 %cmp2, label %BB3, label %BB4

BB3:                                       ; preds = %BB1, %BB2
  %i = phi i64 [ 0, %BB1 ], [ %i.next, %BB2 ]
  %arrayidx1 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %i
  %arrayidx2 = getelementptr inbounds [100 x i8], [100 x i8]* %b, i64 0, i64 %i
  %0 = load i32, i32* %arrayidx1, align 4
  %1 = trunc i32 %0 to i8 
  %2 = load i8, i8* %arrayidx2, align 4
  %cmp7 = icmp sgt i8 %2, %1
  %i.next = add nuw nsw i64 %i, 1
  br i1 %cmp7, label %end, label %BB2

BB4:                                        ; preds = %BB2
  %inc9 = add nuw nsw i32 %i.1, 1
  %cmp = icmp slt i32 %inc9, 100
  br i1 %cmp, label %BB1, label %end

end:                                       ; preds = %BB4
  ret void
}

; This test contains two nested loops and a byte load in the outer loop.
; CHECK-LABEL: test_bytemov_outer_loop:
; CHECK: movzbl
define void @test_bytemov_outer_loop([100 x i32]* %a, [100 x i8]* %b) nounwind uwtable {
entry:
  br label %BB2

BB1:                                       ; preds = %BB2
  %i = phi i64 [ 0, %BB2 ], [ %i.next, %BB1 ]
  %i.next = add nuw nsw i64 %i, 1
  %cmp2 = icmp slt i64 %i.next, 100
  br i1 %cmp2, label %BB1, label %BB2

BB2:                                        ; preds = %entry
  %i.1 = phi i64 [ 0, %entry ], [ %i.1.next, %BB1 ]
  %arrayidx1 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %i.1
  %arrayidx2 = getelementptr inbounds [100 x i8], [100 x i8]* %b, i64 0, i64 %i.1
  %0 = load i32, i32* %arrayidx1, align 4
  %1 = trunc i32 %0 to i8 
  %2 = load i8, i8* %arrayidx2, align 4
  %cmp = icmp sgt i8 %2, %1
  %i.1.next = add nuw nsw i64 %i.1, 1
  br i1 %cmp, label %BB1, label %end

end:                                       ; preds = %BB2
  ret void
}


