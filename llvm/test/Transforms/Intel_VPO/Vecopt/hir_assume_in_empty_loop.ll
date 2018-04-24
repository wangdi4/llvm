; RUN: opt -hir-opt-var-predicate -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -vplan-force-vf=4 <%s -disable-output \
; RUN:     -print-after=VPlanDriverHIR 2>&1 | FileCheck %s

; Verify that vectorizer does not generate an empty HLIf node.
;
; HIR Opt Var Predicate changes the loop into
;
; BEGIN REGION { modified }
;       + DO i1 = 0, (-1 + (-1 * smax(-34, (-1 * %arg)))), 1   <DO_LOOP>
;       |   @llvm.assume(undef);
;       |   %mul = undef  *  undef;
;       + END LOOP
;
;       + DO i1 = 0, (-1 + (-1 * smax(-34, (-1 + (-1 * %arg))))) + -1 * smax(0, %arg), 1   <DO_LOOP>
;       |   @llvm.assume(undef);
;       + END LOOP
;
;       + DO i1 = 0, -1 * smax(0, (1 + %arg)) + 33, 1   <DO_LOOP>
;       |   @llvm.assume(undef);
;       |   %mul = undef  *  undef;
;       + END LOOP
; END REGION
;
; And vectorizer was making the following for the loop in the middle:

;   if (smax(0, %arg) < (-1 + (-1 * smax(-34, (-1 + (-1 * %arg))))) + 1)
;   {
;      %tgu = ((-1 + (-1 * smax(-34, (-1 + (-1 * %arg))))) + -1 * smax(0, %arg) + 1)/u4;
;      if (0 <u 4 * %tgu)   // This empty HLIf
;      {                    // was causing an assert in HIR verificaiton.
;      }
;
;      + DO i1 = 4 * %tgu, (-1 + (-1 * smax(-34, (-1 + (-1 * %arg))))) + -1 * smax(0, %arg), 1   <DO_LOOP>  <MAX_TC_EST = 3>
;      |   @llvm.assume(undef);
;      + END LOOP
;   }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind
declare void @llvm.assume(i1) #0

; Function Attrs: uwtable
define void @foo(i64 %arg) #1 {
; CHECK-LABEL: BEGIN REGION { modified }
; The first loop:
; CHECK:      if (
; CHECK:        DO i1 = 0, {{.*}}, 4  <DO_LOOP
; CHECK-NEXT:     undef  * undef
; CHECK-NEXT:   END LOOP
; CHECK:        DO i1 = {{.*}}, 1  <DO_LOOP
; CHECK-NEXT:     @llvm.assume(undef)
; CHECK-NEXT:     undef  * undef
; CHECK-NEXT:   END LOOP
; Verify that the second loop (from the three above) does not contain the vector one.
; CHECK:      if (
; CHECK-NEXT: {
; CHECK-NEXT:    %tgu
; CHECK-NEXT: <{{[0-9]*}}>
; CHECK-NEXT:    DO i1 = 4 * %tgu{{.*}}, 1  <DO_LOOP>
; CHECK-NEXT:      @llvm.assume(undef)
; CHECK-NEXT:    END LOOP
; CHECK-NEXT: }
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  tail call void @llvm.assume(i1 undef)
  %cmp2 = icmp eq i64 %indvars.iv, %arg
  br i1 %cmp2, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %mul = fmul double undef, undef
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 34
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}

attributes #0 = { nounwind }
attributes #1 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
