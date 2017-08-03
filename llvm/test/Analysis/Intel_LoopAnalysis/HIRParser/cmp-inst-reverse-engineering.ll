; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Verify that we can parse the loopnest successfully. We need to allow tracing through compare instructions to reverse engineer blobs in some cases.


; CHECK: + DO i1 = 0, -1 * undef + smax(%ttop, undef), 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, -1 * umax((-1 + (-1 * smax(0, undef))), (-1 * smax(1, %t.1143))) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %t.1143 = i1 + undef + 1;
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @lev_iv_accounts(i32 %ttop) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry
  br label %for.cond33.preheader

for.cond33.preheader:                             ; preds = %for.body
  br i1 undef, label %for.end52, label %for.cond36.preheader.lr.ph

for.cond36.preheader.lr.ph:                       ; preds = %for.cond33.preheader
  br label %for.cond36.preheader

for.cond36.preheader:                             ; preds = %for.end48, %for.cond36.preheader.lr.ph
  %t.1143 = phi i32 [ undef, %for.cond36.preheader.lr.ph ], [ %inc51, %for.end48 ]
  br label %for.body41

for.body41:                                       ; preds = %for.body41, %for.cond36.preheader
  %k.1138 = phi i32 [ %inc47, %for.body41 ], [ 0, %for.cond36.preheader ]
  %inc47 = add nsw i32 %k.1138, 1
  %cmp37 = icmp slt i32 %k.1138, undef
  %cmp40 = icmp sgt i32 %t.1143, %inc47
  %or.cond = and i1 %cmp37, %cmp40
  br i1 %or.cond, label %for.body41, label %for.end48.loopexit

for.end48.loopexit:                               ; preds = %for.body41
  br label %for.end48

for.end48:                                        ; preds = %for.end48.loopexit
  %inc51 = add nsw i32 %t.1143, 1
  %cmp34 = icmp slt i32 %t.1143, %ttop
  br i1 %cmp34, label %for.cond36.preheader, label %for.end52.loopexit

for.end52.loopexit:                               ; preds = %for.end48
  unreachable

for.end52:                                        ; preds = %for.cond33.preheader
  ret void
}

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (trunk 21400) (llvm/branches/loopopt 21407)"}
