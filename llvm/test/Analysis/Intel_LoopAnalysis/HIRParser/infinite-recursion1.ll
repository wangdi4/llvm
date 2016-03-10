; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Verify that we do not get stuck in infinite recursion when processing this file.
; CHECK: UNKNOWN LOOP i1
; CHECK: END LOOP


; ModuleID = 'bugpoint-reduced-simplified.bc'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: inlinehint norecurse nounwind uwtable
define fastcc void @cftmdl() unnamed_addr #0 {
entry:
  br i1 undef, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  br i1 undef, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  br i1 undef, label %for.body81.preheader, label %for.end170

for.body81.preheader:                             ; preds = %for.end
  br label %for.body81

for.body81:                                       ; preds = %for.body81, %for.body81.preheader
  br i1 undef, label %for.body81, label %for.end170.loopexit

for.end170.loopexit:                              ; preds = %for.body81
  br label %for.end170

for.end170:                                       ; preds = %for.end170.loopexit, %for.end
  br i1 undef, label %for.body174.preheader, label %for.end419

for.body174.preheader:                            ; preds = %for.end170
  br label %for.body174

for.body174:                                      ; preds = %for.inc417, %for.body174.preheader
  %indvars.iv828 = phi i64 [ undef, %for.body174.preheader ], [ %indvars.iv.next829, %for.inc417 ]
  br i1 undef, label %for.body196.preheader, label %for.end296

for.body196.preheader:                            ; preds = %for.body174
  br label %for.body196

for.body196:                                      ; preds = %for.body196, %for.body196.preheader
  br i1 undef, label %for.body196, label %for.end296.loopexit

for.end296.loopexit:                              ; preds = %for.body196
  br label %for.end296

for.end296:                                       ; preds = %for.end296.loopexit, %for.body174
  %0 = add nsw i64 %indvars.iv828, undef
  %1 = add nsw i64 %0, undef
  br i1 undef, label %for.body314.lr.ph, label %for.inc417

for.body314.lr.ph:                                ; preds = %for.end296
  br label %for.body314

for.body314:                                      ; preds = %for.body314, %for.body314.lr.ph
  %j.3813 = phi i32 [ undef, %for.body314.lr.ph ], [ %add415, %for.body314 ], !in.de.ssa !1
  %add415 = add nsw i32 %j.3813, 2
  %2 = sext i32 %add415 to i64
  %cmp313 = icmp slt i64 %2, %1
  br i1 %cmp313, label %for.body314, label %for.inc417.loopexit

for.inc417.loopexit:                              ; preds = %for.body314
  br label %for.inc417

for.inc417:                                       ; preds = %for.inc417.loopexit, %for.end296
  %indvars.iv.next829 = add i64 %indvars.iv828, undef
  br i1 undef, label %for.body174, label %for.end419.loopexit

for.end419.loopexit:                              ; preds = %for.inc417
  br label %for.end419

for.end419:                                       ; preds = %for.end419.loopexit, %for.end170
  ret void
}

attributes #0 = { inlinehint norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 2039) (llvm/branches/loopopt 2048)"}
!1 = !{!"j.3813.de.ssa"}
