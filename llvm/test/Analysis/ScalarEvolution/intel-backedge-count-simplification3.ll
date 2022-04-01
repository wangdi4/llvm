; RUN: opt < %s -analyze -enable-new-pm=0 -scalar-evolution | FileCheck %s
; RUN: opt < %s -analyze -enable-new-pm=0 -scalar-evolution -scalar-evolution-print-scoped-mode | FileCheck %s --check-prefix=SCOPED-MODE
; RUN: opt < %s -disable-output "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -scalar-evolution-print-scoped-mode "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s --check-prefix=SCOPED-MODE


; Verify that in scoped mode, the constants in backedge taken count expr are
; simplified by replacing zext by sext in a proven non-negative add expr with
; <nsw> flag. This helps loopopt identify loop ZTTs.

; Original backedge count expr-
; CHECK: (-2 + (zext i32 (1 + %n)<nsw> to i64))<nsw>


; SCOPED-MODE: backedge-taken count is (-1 + (sext i32 %n to i64))<nsw>


define void @foo(i32 %n1, i32* nocapture %A) {
entry:
  %cmp1 = icmp sgt i32 %n1, 1
  %n = select i1 %cmp1, i32 -5, i32 5
  %add = add nsw i32 %n, 1
  %wide.tc = zext i32 %add to i64
  %cmp5 = icmp slt i32 %n, 1
  br i1 %cmp5, label %for.end, label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.06 = phi i64 [ %inc, %for.body ], [ 1, %entry ]
  %ptridx = getelementptr inbounds i32, i32* %A, i64 %i.06
  store i32 %n, i32* %ptridx, align 4
  %inc = add nuw nsw i64 %i.06, 1
  %cmp = icmp eq i64 %inc, %wide.tc
  br i1 %cmp, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

