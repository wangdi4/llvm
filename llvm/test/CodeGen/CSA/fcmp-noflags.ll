; RUN: llc -csa-df-calls=0 -O1 < %s
; Note, in the off chance that anyone actually reads this file:
; This is checking the reduction of fast-math FCMP instructions. SelIDAG lowers
; these to setgt et al. instead of setogt and setugt.
; Interestingly enough, these are keyed off of the function fast-math flags, not
; the flags on individual operations.
; ModuleID = './bugpoint-reduced-simplified.bc'
source_filename = "bugpoint-output-d6e8b68.bc"
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define void @advance_p_pipeline_csa() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry
  %mul178 = fmul fast float undef, undef
  br i1 undef, label %if.then198, label %if.else

if.then198:                                       ; preds = %for.body
  unreachable

if.else:                                          ; preds = %for.body
  %cmp5.i = fcmp fast ogt float %mul178, 0.000000e+00
  %conv7.i = select i1 %cmp5.i, float 1.000000e+00, float -1.000000e+00
  store float %conv7.i, float* undef, align 4
  unreachable
}

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" "use-soft-float"="false" }
