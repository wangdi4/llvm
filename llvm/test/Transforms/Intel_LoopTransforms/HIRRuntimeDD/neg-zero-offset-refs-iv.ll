; REQUIRES: asserts
;
; RUN: opt %s -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -debug-only=hir-runtime-dd -disable-output 2>&1 | FileCheck %s

; ZeroOffsetRefs should not be treated specially when memrefs with IV exists
; even if they have the same shape.

;  BEGIN REGION { }
;        + DO i1 = 0, %n + -1, 1
;        |   %a = (%p)[0].0;
;        |   %b = (%p)[i1].0;
;        |   (%q)[i1] = %a + %b;
;        + END LOOP
;  END REGION

; CHECK: Group 0 contains (2) refs:
; CHECK: (%p)[0]
; CHECK: (%p)[i1].0
; CHECK: Group 1 contains (1) refs:
; CHECK: (%q)[i1]
; CHECK-NOT: MVTag

;Module Before HIR; ModuleID = 'ptr-types-struct.c'
source_filename = "ptr-types-struct.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.T1 = type { [3 x i64] }
%struct.T = type { i64, %struct.T1, i64, i32 }

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr noundef nonnull align 8 nocapture dereferenceable(44) %p, ptr nocapture %q, i64 %n) local_unnamed_addr #0 {
entry:
  %cmp12 = icmp sgt i64 %n, 0
  br i1 %cmp12, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %storeaddr = phi ptr [%q, %for.body.lr.ph], [%storeaddr.next, %for.body]
  %n_1 = sub nuw nsw i64 %n, 2
  %arrayidx.ii = getelementptr inbounds %struct.T, ptr %p, i64 %indvars.iv, i32 0
  %a = load i64, ptr %p, align 8
  %b = load i64, ptr %arrayidx.ii, align 8
  %tmp.b = add nuw nsw i64 %b, %a
  store i64 %tmp.b, ptr %storeaddr
  %storeaddr.next = getelementptr inbounds i64, ptr %storeaddr, i64 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
