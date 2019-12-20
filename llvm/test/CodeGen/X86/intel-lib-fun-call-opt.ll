;RUN: llc < %s -mtriple=x86_64-pc-linux --enable-intel-advanced-opts -O3 | FileCheck %s --check-prefixes=CHECK

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: test:

; Verify that there are no spills or reloads across call to libimf AVX2 lib fun
; CHECK-NOT: {{Spill|Reload}}

; Function Attrs: nounwind readnone uwtable
define dso_local double @test(double %d1, double %d2, double %d3) local_unnamed_addr #0 {
entry:
  %mul = fmul fast double %d2, %d1
  %add = fadd fast double %mul, %d3
  %mul1 = fmul fast double %d3, %d2
  %sub = fsub fast double %mul1, %d1
  %0 = tail call fast double @llvm.log.f64(double %sub)
  %div = fdiv fast double %add, %0
  ret double %div
}

; Function Attrs: nounwind readnone speculatable willreturn
declare double @llvm.log.f64(double) #1

attributes #0 = { nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "prefer-vector-width"="256" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable willreturn }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
