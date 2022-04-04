; RUN: opt < %s -analyze -enable-new-pm=0 -scalar-evolution | FileCheck %s
; RUN: opt < %s -disable-output "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s

; Verify that the smax expr in backedge taken count of the loop is simplified.
; This is done by deducing nsw flag directly from %add even though the its SCEV
; form does not have it.

; Original expr was like this-
; (-2 + (2 smax (1 + %n)))<nsw>

; CHECK: backedge-taken count is (-1 + %n)

define void @foo(i32 %n, i32* nocapture %A) {
entry:
  %add = add nsw i32 %n, 1
  %cmp5 = icmp slt i32 %n, 1
  br i1 %cmp5, label %for.end, label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.06 = phi i32 [ %inc, %for.body ], [ 2, %entry ]
  %idxprom = zext i32 %i.06 to i64
  %ptridx = getelementptr inbounds i32, i32* %A, i64 %idxprom
  store i32 %i.06, i32* %ptridx, align 4
  %inc = add nuw nsw i32 %i.06, 1
  %cmp = icmp sgt i32 %inc, %add
  br i1 %cmp, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

