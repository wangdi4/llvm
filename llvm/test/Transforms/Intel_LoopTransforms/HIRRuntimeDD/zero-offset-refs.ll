; REQUIRES: asserts
;
; RUN: opt %s -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -debug-only=hir-runtime-dd -disable-output 2>&1 | FileCheck %s

; Make sure zero offset memrefs are sorted and DD tests are generated.
; Zero offset memrefs in the example are {(%p)[0],(%p)[0].0}.

; BEGIN REGION { }
;       + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;       |   %a = (%p)[0];
;       |   %b = (%p)[0].1.0[1];
;       |   %c = (%p)[0].0;
;       |   (%q)[i1] = %b + -1 * (%a + %c);
;       + END LOOP
; END REGION

; CHECK: Group 0 contains
; CHECK: (%p)[0]
; CHECK: (%p)[0].1.0[1]
; CHECK: (%p)[0]
; CHECK: Group 1 contains
; CHECK: (%q)[i1]

; CHECK:  %mv.test = &((%p)[0].1.0[1]) >=u &((%q)[0]);
; CHECK:  %mv.test3 = &((%q)[%n + -1]) >=u &((%p)[0]);
; CHECK:  %mv.and = %mv.test  &  %mv.test3;
; CHECK:  if (%mv.and == 0)  <MVTag:

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
  %a = load i64, ptr %p, align 8
  %arrayidx.i = getelementptr inbounds %struct.T, ptr %p, i64 0, i32 1, i32 0, i64 1
  %b = load i64, ptr %arrayidx.i, align 8
  %c = load i64, ptr %p, align 8
  %tmp.b = add nuw nsw i64 %c, %a
  %res   =sub i64 %b, %tmp.b
  store i64 %res, ptr %storeaddr
  %storeaddr.next = getelementptr inbounds i64, ptr %storeaddr, i64 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
