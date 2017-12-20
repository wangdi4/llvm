; RUN: opt < %s -hir-ssa-deconstruction -hir-cost-model-throttling=0 | opt -analyze -hir-parser -hir-cost-model-throttling=0 -hir-details | FileCheck %s

; Verify that we are able to apply NSW flag to this unknown multi-exit loop due to the presence of non-negative NSW IV %t15.
; CHECK: NSW: Yes
; CHECK: UNKNOWN LOOP i1

; ModuleID = 'red1.ll'
source_filename = "red.ll"

declare i32 @strcmp(i8* nocapture, i8* nocapture)

define void @func(i8* %t1, i8** %t6, i8* %t9) {
entry:
  br label %loop

loop:                                             ; preds = %backedge, %entry
  %t14 = phi i8* [ %t23, %backedge ], [ %t9, %entry ]
  %t15 = phi i32 [ %t21, %backedge ], [ 0, %entry ]
  %t16 = tail call i32 @strcmp(i8* %t1, i8* %t14)
  %t17 = icmp eq i32 %t16, 0
  br i1 %t17, label %if, label %backedge

if:                                               ; preds = %loop
  %t15.lcssa = phi i32 [ %t15, %loop ]
  %t19 = phi i32 [ %t15, %loop ]
  br label %exit

backedge:                                         ; preds = %loop
  %t21 = add nuw nsw i32 %t15, 2
  %t22 = getelementptr inbounds i8*, i8** %t6, i32 %t21
  %t23 = load i8*, i8** %t22, align 4
  %t24 = icmp eq i8* %t23, null
  br i1 %t24, label %exit.loopexit, label %loop

exit.loopexit:                                    ; preds = %backedge
  %t21.lcssa = phi i32 [ %t21, %backedge ]
  br label %exit

exit:                                             ; preds = %exit.loopexit, %if
  %t26 = phi i32 [ %t15.lcssa, %if ], [ %t21.lcssa, %exit.loopexit ]
  ret void
}
