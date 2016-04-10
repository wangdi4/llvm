; RUN: opt < %s -analyze -hir-scc-formation | FileCheck %s

; Check that no SCCs are formed for this loop. The SCC (%1 -> %2 -> %add.ptr1.i.520) should have been invalidated.
; CHECK-NOT: SCC1

; ModuleID = 'bugpoint-reduced-simplified.bc'
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @t_run_test() {
entry:
  br label %for.body.166

for.body.166:                                     ; preds = %WriteOut.exit521, %entry
  %inc169676 = phi i32 [ 0, %entry ], [ %inc169, %WriteOut.exit521 ]
  %0 = phi i32* [ undef, %entry ], [ %2, %WriteOut.exit521 ]
  %1 = phi i32* [ undef, %entry ], [ %add.ptr1.i.520, %WriteOut.exit521 ]
  br i1 undef, label %if.then.i.519, label %WriteOut.exit521

if.then.i.519:                                    ; preds = %for.body.166
  br label %WriteOut.exit521

WriteOut.exit521:                                 ; preds = %if.then.i.519, %for.body.166
  %2 = phi i32* [ null, %if.then.i.519 ], [ %1, %for.body.166 ]
  %add.ptr1.i.520 = getelementptr inbounds i32, i32* %2, i32 undef
  %inc169 = add nsw i32 %inc169676, 1
  %cmp164 = icmp slt i32 %inc169, 512
  br i1 %cmp164, label %for.body.166, label %for.end.170

for.end.170:                                      ; preds = %WriteOut.exit521
  ret void
}
