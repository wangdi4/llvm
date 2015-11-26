; RUN: opt < %s -loop-simplify | opt -analyze -hir-scc-formation | FileCheck %s

; Check formation of one SCC
; CHECK: Region 1
; CHECK-NEXT: SCC1
; CHECK-DAG: inc
; CHECK-NOT: SCC
; CHECK-DAG: a.addr.1
; CHECK-NOT: SCC
; CHECK-DAG: a.addr.08
; CHECK-NOT: SCC


; ModuleID = 'de_ssa.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @foo(i32* nocapture %A, i32 %a, i32 %b, i32 %n) {
entry:
  %cmp7 = icmp sgt i32 %n, 0
  br i1 %cmp7, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %entry ]
  %a.addr.08 = phi i32 [ %a.addr.1, %for.inc ], [ %a, %entry ]
  %cmp1 = icmp sgt i64 %indvars.iv, 77
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %inc = add nsw i32 %a.addr.08, 1
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  store i32 %inc, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %a.addr.1 = phi i32 [ %inc, %if.then ], [ %a.addr.08, %for.body ]
  %output.1 = phi i32 [ %a.addr.08, %if.then ], [ %b, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc, %entry
  %output.0.lcssa = phi i32 [ -1, %entry ], [ %output.1, %for.inc ]
  ret i32 %output.0.lcssa
}

