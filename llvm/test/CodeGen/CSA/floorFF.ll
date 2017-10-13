; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK

; ModuleID = 'MathOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind readnone
define float @floorFF(float %x) local_unnamed_addr #0 {
; CSA_CHECK-label: floorFF
; CSA_CHECK: floorf32

entry:
  %call = tail call float @floor(float %x) #2
  ret float %call
}

; Function Attrs: nounwind readnone
declare float @floor(float) local_unnamed_addr #1

attributes #0 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 "}
