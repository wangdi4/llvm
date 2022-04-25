; RUN: opt < %s -hir-ssa-deconstruction -xmain-opt-level=3 | opt -xmain-opt-level=3 -analyze -enable-new-pm=0 -hir-loop-statistics | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-loop-statistics>" -xmain-opt-level=3 -disable-output 2>&1 | FileCheck %s

; Check that we correctly parse the unknown loop. The bottom test "if (i1 < %n.addr.012)" has been shifted by -1 to adjust for the IV update copy which will be generated just before it during code gen.

; HIR-
; + UNKNOWN LOOP i1
; |   <i1 = 0>
; |   for.body:
; |   %0 = (%A)[i1];
; |   if (%0 < 0)
; |   {
; |      (%A)[i1] = %0 + -1;
; |      %n.addr.012 = %n.addr.012  +  -1;
; |   }
; |   if (i1 + 1 < %n.addr.012)
; |   {
; |      <i1 = i1 + 1>
; |      goto for.body;
; |   }
; + END LOOP

; CHECK: + UNKNOWN LOOP i1
; CHECK: Number of ifs: 2
; CHECK: Number of forward gotos: 0
; CHECK: Number of forward goto target labels: 0
; CHECK: + END LOOP

;Module Before HIR; ModuleID = 'unknown.c'
source_filename = "unknown.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %A, i32 %n) local_unnamed_addr {
entry:
  %cmp11 = icmp sgt i32 %n, 0
  br i1 %cmp11, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %for.body.preheader ]
  %n.addr.012 = phi i32 [ %n.addr.1, %for.inc ], [ %n, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp slt i32 %0, 0
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %dec = add nsw i32 %0, -1
  store i32 %dec, i32* %arrayidx, align 4
  %dec4 = add nsw i32 %n.addr.012, -1
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %n.addr.1 = phi i32 [ %dec4, %if.then ], [ %n.addr.012, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %1 = sext i32 %n.addr.1 to i64
  %cmp = icmp slt i64 %indvars.iv.next, %1
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

