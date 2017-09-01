; RUN: opt -hir-ssa-deconstruction -hir-general-unroll -print-after=hir-general-unroll < %s 2>&1 | FileCheck %s

; HIR-
; + DO i1 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>  <MAX_TC_EST = 100>
; |   (@arr)[0][i1].0 = i1;
; + END LOOP


; Check that general unroll supports structure accesses.

; CHECK: BEGIN REGION { modified }
; CHECK: (@arr)[0][8 * i1].0 = 8 * i1;


; ModuleID = 'struct_array.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S = type { i32 }

@arr = common local_unnamed_addr global [100 x %struct.S] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture readnone %A, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp5 = icmp sgt i32 %n, 0
  br i1 %cmp5, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %a = getelementptr inbounds [100 x %struct.S], [100 x %struct.S]* @arr, i64 0, i64 %indvars.iv, i32 0
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %a, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

