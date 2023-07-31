; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" < %s -disable-output -xmain-opt-level=3 < %s 2>&1 | FileCheck %s

; Verify that loop unswitching correclty remaps loop early exit goto in the else case of the IF candidate.

; HIR:
; BEGIN REGION { }
;      + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
;      |   if (%m >= 1025)
;      |   {
;      |      + DO i2 = 0, sext.i32.i64(%n) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>
;      |      |   %1 = (%a)[i2];
;      |      |   (%a)[i2] = %1 + 1;
;      |      |   if (%n < 256)
;      |      |   {
;      |      |      (%a)[i1] = 1;
;      |      |   }
;      |      |   else
;      |      |   {
;      |      |      if ((%a)[i1] > 100)
;      |      |      {
;      |      |         goto L.loopexit;     << The problem happens when the exit goto is in else block
;      |      |                                 and the target label is not cloned.
;      |      |      }
;      |      |   }
;      |      |   %3 = (%a)[i2];
;      |      |   (%a)[i2] = %3 + 1;
;      |      + END LOOP
;      |
;      |      L.loopexit:                    << Target label is in outer loop, it will not be cloned.
;      |   }
;      |   %4 = (%a)[i1];
;      |   (%a)[i1] = %4 + 1;
;      + END LOOP
; END REGION

; CHECK: Function
; CHECK: BEGIN REGION { modified }
; CHECK:  if
; CHECK:   if
; CHECK:    DO i1
; CHECK:      DO i2
; CHECK:      END LOOP
; CHECK:    END LOOP
; CHECK:   else
; CHECK:    DO i1
; CHECK:     DO i2
; CHECK:      if
; CHECK:       goto [[L:.*]];
; CHECK:     END LOOP
; CHECK:     [[L]]:
; CHECK:    END LOOP
; CHECK:  else
; CHECK:   DO i1
; CHECK: END REGION

;Module Before HIR; ModuleID = 'multiexit-goto-remap.ll'
source_filename = "multiexit-goto-remap.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind uwtable
define void @foo(ptr nocapture %a, i32 %n, i32 %m) local_unnamed_addr #0 {
entry:
  %cmp14 = icmp sgt i32 %n, 0
  br i1 %cmp14, label %for.body.lr.ph, label %for.end24

for.body.lr.ph:                                   ; preds = %entry
  %cmp1 = icmp slt i32 %m, 1025
  %cmp5 = icmp slt i32 %n, 256
  %0 = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %L, %for.body.lr.ph
  %indvars.iv16 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next17, %L ]
  %.pre = getelementptr inbounds i32, ptr %a, i64 %indvars.iv16
  br i1 %cmp1, label %L, label %for.body4.preheader

for.body4.preheader:                              ; preds = %for.body
  br label %for.body4

for.body4:                                        ; preds = %for.body4.preheader, %if.end13
  %indvars.iv = phi i64 [ %indvars.iv.next, %if.end13 ], [ 0, %for.body4.preheader ]
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx, align 4
  %inc = add nsw i32 %1, 1
  store i32 %inc, ptr %arrayidx, align 4
  br i1 %cmp5, label %if.then6, label %if.else

if.then6:                                         ; preds = %for.body4
  store i32 1, ptr %.pre, align 4
  br label %if.end13

if.else:                                          ; preds = %for.body4
  %2 = load i32, ptr %.pre, align 4
  %cmp11 = icmp sgt i32 %2, 100
  br i1 %cmp11, label %L.loopexit, label %if.end13

if.end13:                                         ; preds = %if.else, %if.then6
  %3 = load i32, ptr %arrayidx, align 4
  %inc16 = add nsw i32 %3, 1
  store i32 %inc16, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp3 = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp3, label %for.body4, label %L.loopexit

L.loopexit:                                       ; preds = %if.end13, %if.else
  br label %L

L:                                                ; preds = %L.loopexit, %for.body
  %4 = load i32, ptr %.pre, align 4
  %inc21 = add nsw i32 %4, 1
  store i32 %inc21, ptr %.pre, align 4
  %indvars.iv.next17 = add nuw nsw i64 %indvars.iv16, 1
  %exitcond = icmp eq i64 %indvars.iv.next17, %0
  br i1 %exitcond, label %for.end24.loopexit, label %for.body

for.end24.loopexit:                               ; preds = %L
  br label %for.end24

for.end24:                                        ; preds = %for.end24.loopexit, %entry
  ret void
}

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (cfe/trunk)"}
