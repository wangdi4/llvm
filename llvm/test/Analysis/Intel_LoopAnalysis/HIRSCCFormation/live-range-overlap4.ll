; RUN: opt < %s -enable-new-pm=0 -analyze -hir-scc-formation | FileCheck %s

; We use the --allow-empty flag with FileCheck for the new-format opt because:
;
; - new-format opt output is empty for this test, (old-format opt emits just one
;       line: Printing analysis 'HIR SCC Formation' for function...).
; - The check consists of 'CHECK-NOT' only, and has no 'CHECK' lines.
;
; TODO: If the lit-test is modified, and new-format opt is no longer empty,
;     please make sure to remove the --allow-empty flag, and this comment.
;
; RUN: opt %s -passes="print<hir-scc-formation>" -disable-output 2>&1 | FileCheck --allow-empty %s

; Verify that we do not create SCC (%res.022 -> %inc4 -> %inc8 -> %res.1) which has live-range overlap because %res.022 is used in defining %inc8 after being killed by %inc4 in %for.end bblock.

; CHECK-NOT: SCC1

;Module Before HIR; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readnone uwtable
define i32 @main() local_unnamed_addr #0 {
entry:
  %ar = alloca [2 x i32], align 4
  %0 = bitcast [2 x i32]* %ar to i8*
  %arrayidx6 = getelementptr inbounds [2 x i32], [2 x i32]* %ar, i64 0, i64 0
  br label %for.body

for.body:                                         ; preds = %for.inc9, %entry
  %res.022 = phi i32 [ -4, %entry ], [ %res.1, %for.inc9 ]
  %i.021 = phi i32 [ 0, %entry ], [ %inc10, %for.inc9 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx = getelementptr inbounds [2 x i32], [2 x i32]* %ar, i64 0, i64 %indvars.iv
  store i32 1, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 2
  br i1 %exitcond, label %for.end, label %for.body3

for.end:                                          ; preds = %for.body3
  %inc4 = add nsw i32 %res.022, 1
  %cmp5 = icmp slt i32 %res.022, 2
  br i1 %cmp5, label %if.then, label %for.inc9

if.then:                                          ; preds = %for.end
  %1 = load i32, i32* %arrayidx6, align 4
  %inc7 = add nsw i32 %1, 1
  store i32 %inc7, i32* %arrayidx6, align 4
  %inc8 = add nsw i32 %res.022, 2
  br label %for.inc9

for.inc9:                                         ; preds = %for.end, %if.then
  %res.1 = phi i32 [ %inc8, %if.then ], [ %inc4, %for.end ]
  %inc10 = add nuw nsw i32 %i.021, 1
  %exitcond23 = icmp eq i32 %inc10, 2
  br i1 %exitcond23, label %for.end11, label %for.body

for.end11:                                        ; preds = %for.inc9
  %res.1.lcssa = phi i32 [ %res.1, %for.inc9 ]
  ret i32 %res.1.lcssa
}

