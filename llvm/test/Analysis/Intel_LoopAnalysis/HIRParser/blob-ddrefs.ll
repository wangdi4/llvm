; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-details -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s
; RUN: opt %s -passes="convert-to-subscript,hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -hir-details -disable-output  2>&1 | FileCheck %s

; CHECK: HasSignedIV: Yes

; Check parsing output for the temp blobs
; CHECK: DO i64 i1 = 0, sext.i32.i64((-1 + %n))
; CHECK: <BLOB> LINEAR i32 %n
; CHECK: <BLOB> LINEAR ptr %B
; CHECK: <BLOB> LINEAR ptr %C
; CHECK: <BLOB> LINEAR ptr %A
; CHECK-DAG: <BLOB> NON-LINEAR i32 %0
; CHECK-DAG: <BLOB> NON-LINEAR i32 %1


; ModuleID = 'non-linear-blobs.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr nocapture %A, ptr nocapture readonly %B, ptr nocapture readonly %C, i32 %n) {
entry:
  %cmp.10 = icmp sgt i32 %n, 0
  br i1 %cmp.10, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, ptr %C, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx2, align 4
  %add = add nsw i32 %1, %0
  %arrayidx4 = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  store i32 %add, ptr %arrayidx4, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
