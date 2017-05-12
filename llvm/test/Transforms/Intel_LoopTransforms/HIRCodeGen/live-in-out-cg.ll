; ModuleID = '<stdin>'
; RUN: opt -hir-ssa-deconstruction -hir-cg -force-hir-cg -S < %s | FileCheck %s

; original bblocks will precede new ones from ir, so liveout replacement checks
; are before live in initializations

; output.1 is liveout, CG creates a load in successor bblock
; CHECK: for.end.loopexit:
; CHECK: [[LIVEOUT:%[0-9]+]] = load i32, i32* %t

; and uses it to replace any uses outside of region
; CHECK: for.end:
; CHECK: %output.0.lcssa = phi i32 [ -1, %entry ], [ [[LIVEOUT]], %for.end.loopexit ]

; Live in value is %a, it should be immediately stored in symbase memory slot
; CHECK: region.0:
; CHECK: store i32 %a, i32* %t
; CHECK: br label %loop

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
  %a.addr.014 = phi i32 [ %a.addr.1, %if.end ], [ %a, %for.body.preheader ], !in.de.ssa !0
  %a.addr.014.out = bitcast i32 %a.addr.014 to i32, !out.de.ssa !0
  %cmp1 = icmp sgt i64 %indvars.iv, 77
  %output.1.in.1 = bitcast i32 %b to i32, !in.de.ssa !1
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %inc = add nsw i32 %a.addr.014, 1
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  store i32 %inc, i32* %arrayidx, align 4
  %output.1.in = bitcast i32 %a.addr.014.out to i32, !in.de.ssa !1
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %a.addr.1 = phi i32 [ %inc, %if.then ], [ %a.addr.014, %for.body ]
  %output.1 = phi i32 [ %a.addr.014.out, %if.then ], [ %b, %for.body ], !in.de.ssa !1
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

!0 = !{!"a.addr.014.de.ssa"}
!1 = !{!"output.1.de.ssa"}
