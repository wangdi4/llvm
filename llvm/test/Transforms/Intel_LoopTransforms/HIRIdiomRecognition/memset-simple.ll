; Test that simple memset is handled.

; RUN: opt -hir-ssa-deconstruction -hir-idiom -hir-cg -print-after=hir-idiom -disable-output 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-idiom,print<hir>,hir-cg" -disable-output 2>&1 < %s | FileCheck %s

; HIR:
;           BEGIN REGION { }
; <14>            + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; <6>             |   (%p)[i1 + 1] = null;
; <14>            + END LOOP
;           END REGION

; CHECK: BEGIN REGION { modified }
; CHECK:      if (%n >u 12)
; CHECK:      {
; CHECK:         @llvm.memset.p0i8.i32(&((i8*)(%p)[1]),  0,  8 * %n,  0);
; CHECK:      }
; CHECK:      else
; CHECK:      {
; CHECK:         + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK:         |   (%p)[i1 + 1] = null;
; CHECK:         + END LOOP
; CHECK:      }
; CHECK: END REGION

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-idiom -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter -disable-output 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-idiom,hir-cg,simplifycfg,intel-ir-optreport-emitter" -intel-opt-report=low -disable-output 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT
;
;OPTREPORT: Global optimization report for : foo
;
;OPTREPORT: LOOP BEGIN
;OPTREPORT-NEXT: <Multiversioned v1>
;OPTREPORT-NEXT:     remark #25408: memset generated
;OPTREPORT-NEXT:     remark #25562: The loop has been multiversioned for the small trip count
;OPTREPORT-NEXT: LOOP END
;
;OPTREPORT: LOOP BEGIN
;OPTREPORT-NEXT: <Multiversioned v2>
;OPTREPORT-NEXT: LOOP END


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind uwtable
define void @foo(i8** nocapture %p, i32 %n) local_unnamed_addr #0 {
entry:
  %tobool1 = icmp eq i32 %n, 0
  br i1 %tobool1, label %while.end, label %while.body.preheader

while.body.preheader:                             ; preds = %entry
  br label %while.body

while.body:                                       ; preds = %while.body.preheader, %while.body
  %p.addr.03 = phi i8** [ %incdec.ptr, %while.body ], [ %p, %while.body.preheader ]
  %n.addr.02 = phi i32 [ %dec, %while.body ], [ %n, %while.body.preheader ]
  %dec = add nsw i32 %n.addr.02, -1
  %incdec.ptr = getelementptr inbounds i8*, i8** %p.addr.03, i64 1
  store i8* null, i8** %incdec.ptr, align 8
  %tobool = icmp eq i32 %dec, 0
  br i1 %tobool, label %while.end.loopexit, label %while.body

while.end.loopexit:                               ; preds = %while.body
  br label %while.end

while.end:                                        ; preds = %while.end.loopexit, %entry
  ret void
}

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20641) (llvm/branches/loopopt 20650)"}
