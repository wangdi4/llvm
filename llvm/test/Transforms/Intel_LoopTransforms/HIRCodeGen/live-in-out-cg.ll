; ModuleID = '<stdin>'
; RUN: opt -passes="hir-ssa-deconstruction,hir-cg" -force-hir-cg -S < %s | FileCheck %s

; original bblocks will precede new ones from ir, so liveout replacement checks
; are before live in initializations

; Check the added incoming value for the liveout use of %output.1
; CHECK: for.end.loopexit:
; CHECK-NEXT: %output.1.lcssa = phi i32 [ %output.1, %if.end ], [ [[LIVEOUT:.*]], %[[REGIONEXIT:.*]] ]

; Live in value is %a, it should be immediately stored in symbase memory slot
; CHECK: region.0:
; CHECK: store i32 %a, ptr %t
; CHECK: br label %loop

; CG creates a load for %output.1 in region exit bblock
; CHECK: [[REGIONEXIT]]:
; CHECK-NEXT: [[LIVEOUT]] = load i32, ptr %t

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(ptr nocapture %A, ptr nocapture %B, i32 %a, i32 %b, i32 %n) {
entry:
  %cmp13 = icmp sgt i32 %n, 0
  br i1 %cmp13, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %if.end, %for.body.preheader
  %indvars.iv = phi i64 [ %indvars.iv.next, %if.end ], [ 0, %for.body.preheader ]
  %a.addr.014 = phi i32 [ %a.addr.1, %if.end ], [ %a, %for.body.preheader ]
  %cmp1 = icmp sgt i64 %indvars.iv, 77
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %inc = add nsw i32 %a.addr.014, 1
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  store i32 %inc, ptr %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %a.addr.1 = phi i32 [ %inc, %if.then ], [ %a.addr.014, %for.body ]
  %output.1 = phi i32 [ %a.addr.014, %if.then ], [ %b, %for.body ]
  %arrayidx3 = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  store i32 %output.1, ptr %arrayidx3, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %if.end
  %output.1.lcssa = phi i32 [ %output.1, %if.end ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %output.0.lcssa = phi i32 [ -1, %entry ], [ %output.1.lcssa, %for.end.loopexit ]
  ret i32 %output.0.lcssa
}

