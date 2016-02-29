; INTEL_CUSTOMIZATION: Check that Global FMA is skipped for functions with
; INTEL_CUSTOMIZATION: operations with unset 'fast-math' settings.

; REQUIRES: asserts
; RUN: llc -verify-machineinstrs -mtriple=x86_64-apple-darwin -mcpu=corei7-avx -mattr=+fma -do-x86-global-fma -debug-x86-global-fma -enable-unsafe-fp-math -fp-contract=fast -debug -o /dev/null < %s 2>&1 | FileCheck %s

define double @func(double %a, double %b, double %c) #0 {
entry:
  %mul = fmul double %b, %a
  %add = fadd fast double %mul, %c
  ret double %add
; CHECK: Exit because found mixed fast-math settings.
}

attributes #0 = { nounwind readnone uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+aes,+avx,+avx2,+bmi,+bmi2,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+lzcnt,+mmx,+pclmul,+popcnt,+rdrnd,+rtm,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
