; It checks specialization cloning is kicked-in for "foo", which is called
; in "bar". It expects "foo" function is cloned 4 times with
; specialization.

; REQUIRES: asserts
; RUN: opt < %s -ip-cloning -ip-specialization-cloning -debug-only=ipcloning  -disable-output  2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(ip-cloning)' -ip-specialization-cloning -debug-only=ipcloning  -disable-output  2>&1 | FileCheck %s

; CHECK: ClonedCall[
; CHECK: ClonedCall[
; CHECK: ClonedCall[
; CHECK: ClonedCall[

@Val = external local_unnamed_addr global i32, align 4
@a = external local_unnamed_addr global [2056 x i32], align 16
@b = external local_unnamed_addr global [2056 x i32], align 16
@c = external local_unnamed_addr global [2056 x i32], align 16

; Function Attrs: norecurse nounwind uwtable
define void @bar() local_unnamed_addr  {
entry:
  %0 = load i32, i32* @Val, align 4
  switch i32 %0, label %sw.epilog [
    i32 100, label %sw.bb
    i32 200, label %sw.bb1
    i32 300, label %sw.bb2
  ]

sw.bb:                                            ; preds = %entry
  br label %sw.epilog

sw.bb1:                                           ; preds = %entry
  br label %sw.epilog

sw.bb2:                                           ; preds = %entry
  br label %sw.epilog

sw.epilog:                                        ; preds = %entry, %sw.bb2, %sw.bb1, %sw.bb
  %TripC.0 = phi i32 [ 0, %entry ], [ 64, %sw.bb2 ], [ 32, %sw.bb1 ], [ 16, %sw.bb ]
  tail call fastcc void @foo(i32 %TripC.0)
  ret void
}

; Function Attrs: noinline norecurse nounwind uwtable
define internal fastcc void @foo(i32 %UB) unnamed_addr  {
entry:
  %cmp20 = icmp eq i32 %UB, 0
  br i1 %cmp20, label %for.cond.cleanup, label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.cond.cleanup3
  %j.021 = phi i32 [ %inc10, %for.cond.cleanup3 ], [ 0, %entry ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3, %entry
  ret void

for.cond.cleanup3:                                ; preds = %for.body4
  %inc10 = add nuw i32 %j.021, 1
  %exitcond22 = icmp eq i32 %inc10, %UB
  br i1 %exitcond22, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx = getelementptr inbounds [2056 x i32], [2056 x i32]* @a, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %arrayidx6 = getelementptr inbounds [2056 x i32], [2056 x i32]* @b, i64 0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx6, align 4
  %mul = mul nsw i32 %1, %0
  %arrayidx8 = getelementptr inbounds [2056 x i32], [2056 x i32]* @c, i64 0, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx8, align 4
  %add = add nsw i32 %2, %mul
  store i32 %add, i32* %arrayidx8, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 2056
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}
