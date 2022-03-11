; RUN: opt < %s -enable-new-pm=0 -analyze -hir-region-identification | FileCheck %s --check-prefix=LEX
; RUN: opt < %s -passes='print<hir-region-identification>' -disable-output 2>&1 | FileCheck %s --check-prefix=LEX

; RUN: opt < %s -enable-new-pm=0 -analyze -hir-region-identification -hir-region-lexical-insertion-func-size-threshold=5 | FileCheck %s --check-prefix=NO-LEX
; RUN: opt < %s -passes='print<hir-region-identification>' -disable-output -hir-region-lexical-insertion-func-size-threshold=5 2>&1 | FileCheck %s --check-prefix=NO-LEX

; Verify that when function size is within the threshold, regions created for 
; materialization are inserted in lexical order otherwise they are inserted
; at the end of regular regions.


; LEX: Region 1
; LEX: EntryBB: %materialize

; LEX: Region 2
; LEX: EntryBB: %for.body3



; NO-LEX: Region 1
; NO-LEX: EntryBB: %for.body3

; NO-LEX: Region 2
; NO-LEX: EntryBB: %materialize


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32 %n, i32* nocapture %A) {
entry:
  br label %materialize

materialize:
  %ld1 = load i32, i32* %A, align 4
  %arrayidx1 = getelementptr inbounds i32, i32* %A, i64 1
  %ld2 = load i32, i32* %arrayidx1, align 4
  %add1 = add nsw i32 %ld1, %ld2
  br label %bb

bb:
  %cmp12 = icmp sgt i32 %n, 1
  br i1 %cmp12, label %for.preheader, label %for.end5

for.preheader:                    ; preds = %entry
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.preheader
  %indvars.iv = phi i64 [ 0, %for.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %0, %0
  store i32 %add, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.inc4, label %for.body3

for.inc4:                                         ; preds = %for.body3
  br label %for.end5

for.end5:                                         ; preds = %for.end5.loopexit, %entry
  ret void
}


