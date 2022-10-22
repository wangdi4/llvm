; INTEL_FEATURE_SW_ADVANCED
; It checks two function clones are created even if zext is used for
; trip counter. Options -ip-cloning, ; -ip-cloning-after-inl and
; -ip-cloning-loop-heuristic are specified to enable generic cloning.
; -ip-cloning-after-inl option enables IP Cloning that runs after inlining.
; -ip-cloning-loop-heuristic option enables loop based heuristic for Cloning.
; This test expects "bar" function is cloned two times.

; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -passes='module(post-inline-ip-cloning)' -ip-cloning-loop-heuristic -S 2>&1 | FileCheck %s

; CHECK: tail call fastcc void @bar.
; CHECK: tail call fastcc void @bar.


@F_1 = external local_unnamed_addr global [100 x i32], align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo() local_unnamed_addr  {
entry:
  tail call fastcc void @bar(i32 10)
  tail call fastcc void @bar(i32 20)
  ret void
}

; Function Attrs: noinline norecurse nounwind uwtable
define internal fastcc void @bar(i32 %ub) unnamed_addr  {
entry:
  %add = add i32 %ub, 20
  %cmp6 = icmp eq i32 %add, 0
  br i1 %cmp6, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %add to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @F_1, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %1 = trunc i64 %indvars.iv to i32
  %add1 = add i32 %0, %1
  store i32 %add1, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}
; end INTEL_FEATURE_SW_ADVANCED
