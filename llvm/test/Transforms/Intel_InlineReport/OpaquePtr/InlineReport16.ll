; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-report=0xf847 -inline-threshold=10 -disable-output < %s 2>&1 | FileCheck %s
; Inline report via metadata
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xf8c6 < %s -S | opt -passes='cgscc(inline)' -inline-threshold=10 -inline-report=0xf8c6 -S | opt -passes='inlinereportemitter' -inline-report=0xf8c6 -disable-output 2>&1 | FileCheck %s

; CHECK: foo{{.*}}EE{{.*}}Inlining is not profitable

; This LIT test checks the early exit cost print out in the inlining report

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @foo(i32 %n, ptr nocapture readonly %str) local_unnamed_addr #0 {
entry:
  %0 = load i32, ptr %str, align 4
  %cmp11 = icmp eq i32 %0, 543
  br i1 %cmp11, label %while.end, label %while.body

while.body:                                       ; preds = %entry, %while.body.2
  %1 = phi i32 [ %.pre, %while.body.2 ], [ %0, %entry ]
  %indvars.iv = phi i64 [ %indvars.iv.next, %while.body.2 ], [ 0, %entry ]
  %total.013 = phi i64 [ %add, %while.body.2 ], [ 0, %entry ]
  %indvars.iv.next = add i64 %indvars.iv, 1
  %div = sdiv i32 %1, 21
  %mul3 = mul nsw i32 %div, %n
  %sub = add nsw i32 %mul3, -1
  %conv = sext i32 %sub to i64
  %add = add nsw i64 %total.013, %conv
  %2 = mul nsw i64 %indvars.iv.next, 3
  %arrayidx = getelementptr inbounds i32, ptr %str, i64 %2
  %3 = load i32, ptr %arrayidx, align 4
  %cmp = icmp eq i32 %3, 543
  br i1 %cmp, label %while.end.loopexit, label %while.body.2

while.body.2:                  ; preds = %while.body
  %arrayidx2.phi.trans.insert = getelementptr inbounds i32, ptr %str, i64 %indvars.iv.next
  %.pre = load i32, ptr %arrayidx2.phi.trans.insert, align 4
  br label %while.body

while.end.loopexit:                               ; preds = %while.body
  %phitmp = trunc i64 %add to i32
  br label %while.end

while.end:                                        ; preds = %while.end.loopexit, %entry
  %total.0.lcssa = phi i32 [ 0, %entry ], [ %phitmp, %while.end.loopexit ]
  ret i32 %total.0.lcssa
}

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @bar(ptr nocapture readonly %arr) local_unnamed_addr #0 {
entry:
  %0 = load i32, ptr %arr, align 4
  %cmp7 = icmp sgt i32 %0, 1
  br i1 %cmp7, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %diff.09 = phi i32 [ %add, %for.body ], [ 0, %entry ]
  %j.08 = phi i32 [ %inc, %for.body ], [ 1, %entry ]
  %call = tail call i32 @foo(i32 %j.08, ptr nonnull %arr)
  %add = add nsw i32 %call, %diff.09
  %inc = add nuw nsw i32 %j.08, 1
  %cmp = icmp slt i32 %inc, %0
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body, %entry
  %diff.0.lcssa = phi i32 [ 0, %entry ], [ %add, %for.body ]
  ret i32 %diff.0.lcssa
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 (cfe/trunk)"}
