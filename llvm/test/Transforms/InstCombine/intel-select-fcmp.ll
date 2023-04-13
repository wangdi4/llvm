; RUN: opt -passes="instcombine" -S < %s | FileCheck %s

; This test verifies if instcombine can simplify rounding conversions
; (rounding float/double to nearest floor/ceil float/double) with a single
; rint(float/double).

; CHECK-LABEL: @MagickRound(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[R:%.*]] = call fast double @llvm.rint.f64(double %x)
; CHECK-NEXT:    ret double [[R]]

define internal fastcc double @MagickRound(double %x) unnamed_addr #0 {
entry:
  %0 = call fast double @llvm.floor.f64(double %x)
  %sub = fsub fast double %x, %0
  %1 = call fast double @llvm.ceil.f64(double %x)
  %sub1 = fsub fast double %1, %x
  %cmp = fcmp fast olt double %sub, %sub1
  %. = select i1 %cmp, double %0, double %1
  ret double %.
}

; Function Attrs: nounwind readnone speculatable
declare double @llvm.floor.f64(double)

; Function Attrs: nounwind readnone speculatable
declare double @llvm.ceil.f64(double)

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
