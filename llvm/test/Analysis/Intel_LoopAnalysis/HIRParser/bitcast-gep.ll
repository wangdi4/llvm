; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-cost-model-throttling=0 -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s
; RUN: opt %s -passes="convert-to-subscript,hir-ssa-deconstruction,print<hir-framework>" -hir-cost-model-throttling=0 -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Check parsing output for the loop verifying that we create a GEP DDRef for @bar's argument which is a bitcast of a gep instruction.
; Verify that bitcast is eliminated for addressOf refs with opaque pointers.
; CHECK: DO i1 = 0, sext.i32.i64((-1 + %n))
; CHECK-NEXT: @bar(&((%A)[i1]))
; CHECK-NEXT: END LOOP


; ModuleID = 'bitcast-gep.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr %A, i32 %n) {
entry:
  %cmp.4 = icmp sgt i32 %n, 0
  br i1 %cmp.4, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %0 = bitcast ptr %arrayidx to ptr
  tail call void @bar(ptr %0)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

declare void @bar(ptr)
