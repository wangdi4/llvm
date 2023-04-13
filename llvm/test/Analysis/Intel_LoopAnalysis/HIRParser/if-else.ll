; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; This command checks that -hir-ssa-deconstruction invalidates SCEV so that the parser doesn't pick up the cached version. HIR output should be the same as for the above command.
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-post-vec-complete-unroll" -print-before=hir-post-vec-complete-unroll 2>&1 | FileCheck %s

; Check parsing output for the loop

; CHECK: + DO i1 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; CHECK: |   %a.addr.014.out = %a.addr.014;
; CHECK: |   %output.1 = %b;
; CHECK: |   if (i1 > 77)
; CHECK: |   {
; CHECK: |      %a.addr.014 = %a.addr.014  +  1;
; CHECK: |      (%A)[i1] = %a.addr.014;
; CHECK: |      %output.1 = %a.addr.014.out;
; CHECK: |   }
; CHECK: |   (%B)[i1] = %output.1;
; CHECK: + END LOOP


; ModuleID = 'de_ssa1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(i32* nocapture %A, i32* nocapture %B, i32 %a, i32 %b, i32 %n) {
entry:
  %cmp13 = icmp sgt i32 %n, 0
  br i1 %cmp13, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %if.end
  %indvars.iv = phi i64 [ %indvars.iv.next, %if.end ], [ 0, %for.body.preheader ]
  %a.addr.014 = phi i32 [ %a.addr.1, %if.end ], [ %a, %for.body.preheader ]
  %cmp1 = icmp sgt i64 %indvars.iv, 77
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %inc = add nsw i32 %a.addr.014, 1
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  store i32 %inc, i32* %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %a.addr.1 = phi i32 [ %inc, %if.then ], [ %a.addr.014, %for.body ]
  %output.1 = phi i32 [ %a.addr.014, %if.then ], [ %b, %for.body ]
  %arrayidx3 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  store i32 %output.1, i32* %arrayidx3, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %if.end
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %output.0.lcssa = phi i32 [ -1, %entry ], [ %output.1, %for.end.loopexit ]
  ret i32 %output.0.lcssa
}
