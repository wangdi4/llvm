; Check runtime dd multiversioning for a case when pointers have different types

; RUN: opt -hir-ssa-deconstruction -hir-runtime-dd -print-after=hir-runtime-dd -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s

;void foo(int *a, long *b, int N) {
;  int i;
;  for (i=0;i<N;i++) {
;    a[i] = b[i];
;  }
;}

; CHECK: Function
; CHECK: %mv.upper.base = &((i32*)(%b)[sext.i32.i64((-1 + %N))]);
; CHECK: %mv.test = &((%mv.upper.base)[1]) >=u &((%a)[0]);
; CHECK: %mv.test1 = &((%a)[sext.i32.i64((-1 + %N))]) >=u &((i32*)(%b)[0]);
; CHECK: %mv.and = %mv.test  &  %mv.test1;
; CHECK: if (%mv.and == 0)

; ModuleID = 'ptr-types.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %a, i64* nocapture readonly %b, i32 %N) #0 {
entry:
  %cmp.7 = icmp sgt i32 %N, 0
  br i1 %cmp.7, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i64, i64* %b, i64 %indvars.iv
  %0 = load i64, i64* %arrayidx, align 8
  %conv = trunc i64 %0 to i32
  %arrayidx2 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  store i32 %conv, i32* %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %N
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

