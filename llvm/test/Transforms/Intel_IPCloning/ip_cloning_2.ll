; It checks two function clones are created when options -ip-cloning,
; -ip-cloning-after-inl and -ip-cloning-loop-heuristic are enabled.
; -ip-cloning-after-inl option enables IP Cloning that runs after inlining.
; -ip-cloning-loop-heuristic option enables loop based heuristic for Cloning.
; This test expects "bar" function is cloned two times.

; REQUIRES: asserts
; RUN: opt < %s -ip-cloning -ip-cloning-after-inl -ip-cloning-loop-heuristic -debug-only=ipcloning -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(post-inline-ip-cloning)' -ip-cloning-loop-heuristic -debug-only=ipcloning -disable-output 2>&1 | FileCheck %s

; CHECK: Cloned call:
; CHECK: Cloned call:


@F_1 = external local_unnamed_addr global [100 x i32], align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo() local_unnamed_addr {
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
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i32 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @F_1, i32 0, i32 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %add1 = add i32 %0, %indvars.iv
  store i32 %add1, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond = icmp eq i32 %indvars.iv.next, %add
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}
