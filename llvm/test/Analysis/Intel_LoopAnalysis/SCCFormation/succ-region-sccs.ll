; RUN: opt < %s -loop-simplify | opt -analyze -hir-region-identification | FileCheck %s

; Check formation of two regions
; CHECK: Region 1
; CHECK: Region 2

; RUN: opt < %s -loop-simplify | opt -analyze -hir-scc-formation | FileCheck --check-prefix=SCC %s

; Check formation of two SCCs in the first region and none in the second.
; SCC: Region 1
; SCC-NEXT: SCC1
; SCC-DAG: %mul
; SCC-DAG: %b.addr.023
; SCC-NEXT: SCC2
; SCC-DAG: %add
; SCC-DAG: %a.addr.022
; SCC-NOT: SCC


; ModuleID = 'multireg.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [100 x i32] zeroinitializer, align 16
@A = common global [100 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo(i32 %m, i32 %n, i32 %a, i32 %b) {
entry:
  %cmp.21 = icmp sgt i32 %n, 0
  br i1 %cmp.21, label %for.body, label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.body, %entry
  %cmp2.19 = icmp sgt i32 %m, 0
  br i1 %cmp2.19, label %for.body.3, label %for.end.8

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv25 = phi i64 [ %indvars.iv.next26, %for.body ], [ 0, %entry ]
  %b.addr.023 = phi i32 [ %mul, %for.body ], [ %b, %entry ]
  %a.addr.022 = phi i32 [ %add, %for.body ], [ %a, %entry ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv25
  store i32 %b.addr.023, i32* %arrayidx, align 4
  %0 = trunc i64 %indvars.iv25 to i32
  %add = add nsw i32 %0, %a.addr.022
  %mul = mul nsw i32 %add, %b.addr.023
  %indvars.iv.next26 = add nuw nsw i64 %indvars.iv25, 1
  %lftr.wideiv27 = trunc i64 %indvars.iv.next26 to i32
  %exitcond28 = icmp eq i32 %lftr.wideiv27, %n
  br i1 %exitcond28, label %for.cond.1.preheader, label %for.body

for.body.3:                                       ; preds = %for.cond.1.preheader, %for.body.3
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body.3 ], [ 0, %for.cond.1.preheader ]
  %arrayidx5 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %m
  br i1 %exitcond, label %for.end.8, label %for.body.3

for.end.8:                                        ; preds = %for.body.3, %for.cond.1.preheader
  ret void
}

