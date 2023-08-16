; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" 2>&1 -disable-output | FileCheck %s

; Verify that parser deduces an implied bitcast (type mismatch) between
; geps %base and %arrayidx and gives up on combining them. This is only
; possible with opaque pointers.

; Previously, the store was incorrectly parsed like this-
; (%p)[1].1[i1]

; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   (%base)[0].1[i1] = i1;
; CHECK: + END LOOP

%struct = type {i32, [10 x i64]}

define void @foo(ptr %p) local_unnamed_addr #0 {
entry:
  br label %preheader

preheader:                                        ; preds = %entry
  %base = getelementptr inbounds i64, ptr %p, i64 1
  br label %for.body

for.body:                                         ; preds = %for.body, %preheader
  %indvars.iv = phi i64 [ 0, %preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds %struct, ptr %base, i64 0, i32 1, i64 %indvars.iv
  store i64 %indvars.iv, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

