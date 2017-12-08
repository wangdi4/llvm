; Verify that we build unknown loop with user calls at O3 but not at O2.

; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s --check-prefix=O2

; O2-NOT: UNKNOWN

; RUN: opt < %s -hir-ssa-deconstruction -xmain-opt-level=3 | opt -analyze -hir-parser -xmain-opt-level=3 | FileCheck %s --check-prefix=O3

; O3: + UNKNOWN LOOP i1
; O3: |   <i1 = 0>
; O3: |   for.body:
; O3: |   %call = @bar(%i.06);
; O3: |   (%A)[%i.06] = %call;
; O3: |   %i.06 = %i.06  <<  1;
; O3: |   if (%i.06 < %n)
; O3: |   {
; O3: |      <i1 = i1 + 1>
; O3: |      goto for.body;
; O3: |   }
; O3: + END LOOP


; ModuleID = 'unknown-user-call-loop.c'
source_filename = "unknown-user-call-loop.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %A, i32 %n) {
entry:
  %cmp5 = icmp sgt i32 %n, 1
  br i1 %cmp5, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.06 = phi i32 [ %mul, %for.body ], [ 1, %for.body.preheader ]
  %call = tail call i32 @bar(i32 %i.06) #2
  %idxprom = sext i32 %i.06 to i64
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %idxprom
  store i32 %call, i32* %arrayidx, align 4
  %mul = shl nsw i32 %i.06, 1
  %cmp = icmp slt i32 %mul, %n
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

declare i32 @bar(i32) local_unnamed_addr #1

