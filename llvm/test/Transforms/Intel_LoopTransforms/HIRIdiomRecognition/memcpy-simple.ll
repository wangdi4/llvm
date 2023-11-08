; Check that simple memcpy is handled

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-idiom,print<hir>,hir-cg" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-idiom" -print-changed -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED

; HIR:
;           BEGIN REGION { }
; <14>            + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; <6>             |   (%p)[i1] = (%q)[i1];
; <14>            + END LOOP
;           END REGION

; CHECK: BEGIN REGION { modified }
; CHECK: memcpy

; Check the proper otpreport is emittted for Idiom Recognition (memcpy transformation).
; TODO: Remove "TODO"-OPTREPORT after preserveLostOptReport is used in HIRIdiomRecognition.cpp.
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-idiom,hir-cg,simplifycfg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -intel-opt-report=low 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT

;OPTREPORT: Global optimization report for : foo
;

; OPTREPORT: LOOP BEGIN
; OPTREPORT-NEXT: <Multiversioned v1>
; OPTREPORT-NEXT: remark #25399: memcopy generated
; OPTREPORT-NEXT: remark #25562: The loop has been multiversioned for the small trip count
; OPTREPORT-NEXT: remark #25260: Dead loop optimized away
; OPTREPORT-NEXT: LOOP END

; OPTREPORT: LOOP BEGIN
; OPTREPORT-NEXT: <Multiversioned v2>
; OPTREPORT-NEXT: LOOP END

; Verify that pass is dumped with print-changed when it triggers.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED: Dump After HIRIdiomRecognition

;Module Before HIR; ModuleID = 'memcpy.c'
source_filename = "memcpy.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr nocapture %p, ptr noalias nocapture readonly %q, i32 %n) local_unnamed_addr #0 {
entry:
  %tobool2 = icmp eq i32 %n, 0
  br i1 %tobool2, label %while.end, label %while.body.preheader

while.body.preheader:                             ; preds = %entry
  br label %while.body

while.body:                                       ; preds = %while.body.preheader, %while.body
  %n.addr.05 = phi i32 [ %dec, %while.body ], [ %n, %while.body.preheader ]
  %q.addr.04 = phi ptr [ %incdec.ptr, %while.body ], [ %q, %while.body.preheader ]
  %p.addr.03 = phi ptr [ %incdec.ptr1, %while.body ], [ %p, %while.body.preheader ]
  %dec = add nsw i32 %n.addr.05, -1
  %incdec.ptr = getelementptr inbounds i32, ptr %q.addr.04, i64 1
  %0 = load i32, ptr %q.addr.04, align 4
  %incdec.ptr1 = getelementptr inbounds i32, ptr %p.addr.03, i64 1
  store i32 %0, ptr %p.addr.03, align 4
  %tobool = icmp eq i32 %dec, 0
  br i1 %tobool, label %while.end.loopexit, label %while.body

while.end.loopexit:                               ; preds = %while.body
  br label %while.end

while.end:                                        ; preds = %while.end.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
