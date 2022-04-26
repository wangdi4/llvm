
; Check correct calling conventions are assigned to SVML calls of different
; calling conventions
; RUN: opt -enable-new-pm=0 -vector-library=SVML -iml-trans -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: @test_sin4_128
; CHECK: {{.*}} = call svml_cc <2 x double> @__svml_sin2_ha_l9(<2 x double> {{.*}})
; CHECK: {{.*}} = call svml_cc <2 x double> @__svml_sin2_ha_l9(<2 x double> {{.*}})

define <4 x double> @test_sin4_128(<4 x double> %A) #0 {
entry:
  %0 = tail call svml_cc <4 x double> @__svml_sin4(<4 x double> %A)
  ret <4 x double> %0
}

; CHECK-LABEL: @test_sin4_256
; CHECK: {{.*}} = call svml_avx_cc <4 x double> @__svml_sin4_ha_l9(<4 x double> {{.*}})

define <4 x double> @test_sin4_256(<4 x double> %A) #1 {
entry:
  %0 = tail call svml_cc <4 x double> @__svml_sin4(<4 x double> %A)
  ret <4 x double> %0
}

; CHECK-LABEL: @test_sin4_512
; CHECK: {{.*}} = call svml_avx512_cc <8 x double> @__svml_sin8_ha_z0(<8 x double> {{.*}})

define <4 x double> @test_sin4_512(<4 x double> %A) #2 {
entry:
  %0 = tail call svml_cc <4 x double> @__svml_sin4(<4 x double> %A)
  ret <4 x double> %0
}

; CHECK-LABEL: @test_sincosf8_mask_128
; CHECK: {{.*}} = call svml_cc { <4 x float>, <4 x float> } @__svml_sincosf4_ha_mask_e9(<4 x float> {{.*}}, <4 x i32> {{.*}})
; CHECK: {{.*}} = call svml_cc { <4 x float>, <4 x float> } @__svml_sincosf4_ha_mask_e9(<4 x float> {{.*}}, <4 x i32> {{.*}})

define <8 x float> @test_sincosf8_mask_128(<8 x float>* nocapture %A, <8 x float> %B, <8 x i32> %C) #0 {
entry:
  %0 = tail call svml_cc { <8 x float>, <8 x float> } @__svml_sincosf8_mask(<8 x float> %B, <8 x i32> %C)
  %1 = extractvalue { <8 x float>, <8 x float> } %0, 1
  store <8 x float> %1, <8 x float>* %A, align 32
  %2 = extractvalue { <8 x float>, <8 x float> } %0, 0
  ret <8 x float> %2
}

; CHECK-LABEL: @test_sincosf8_mask_256
; CHECK: {{.*}} = call svml_avx_cc { <8 x float>, <8 x float> } @__svml_sincosf8_ha_mask_e9(<8 x float> {{.*}}, <8 x i32> {{.*}})

define <8 x float> @test_sincosf8_mask_256(<8 x float>* nocapture %A, <8 x float> %B, <8 x i32> %C) #1 {
entry:
  %0 = tail call svml_cc { <8 x float>, <8 x float> } @__svml_sincosf8_mask(<8 x float> %B, <8 x i32> %C)
  %1 = extractvalue { <8 x float>, <8 x float> } %0, 1
  store <8 x float> %1, <8 x float>* %A, align 32
  %2 = extractvalue { <8 x float>, <8 x float> } %0, 0
  ret <8 x float> %2
}

; CHECK-LABEL: @test_sincosf8_mask_512
; CHECK: {{.*}} = call svml_avx512_cc { <16 x float>, <16 x float> } @__svml_sincosf16_ha_mask_z0({ <16 x float>, <16 x float> } undef, <16 x i1> {{.*}}, <16 x float> {{.*}})

define <8 x float> @test_sincosf8_mask_512(<8 x float>* nocapture %A, <8 x float> %B, <8 x i32> %C) #2 {
entry:
  %0 = tail call svml_cc { <8 x float>, <8 x float> } @__svml_sincosf8_mask(<8 x float> %B, <8 x i32> %C)
  %1 = extractvalue { <8 x float>, <8 x float> } %0, 1
  store <8 x float> %1, <8 x float>* %A, align 32
  %2 = extractvalue { <8 x float>, <8 x float> } %0, 0
  ret <8 x float> %2
}

; CHECK-LABEL: @test_idiv8_128
; CHECK: {{.*}} = call svml_cc <4 x i32> @__svml_idiv4_e9(<4 x i32> {{.*}})
; CHECK: {{.*}} = call svml_cc <4 x i32> @__svml_idiv4_e9(<4 x i32> {{.*}})

define <8 x i32> @test_idiv8_128(<8 x i32> %a) #0 {
entry:
  %0 = tail call svml_cc <8 x i32> @__svml_idiv8(<8 x i32> %a)
  ret <8 x i32> %0
}

; CHECK-LABEL: @test_idiv8_256
; CHECK: {{.*}} = call svml_avx_cc <8 x i32> @__svml_idiv8_l9(<8 x i32> {{.*}})

define <8 x i32> @test_idiv8_256(<8 x i32> %a) #1 {
entry:
  %0 = tail call svml_cc <8 x i32> @__svml_idiv8(<8 x i32> %a)
  ret <8 x i32> %0
}

; CHECK-LABEL: @test_idiv8_512
; CHECK: {{.*}} = call svml_avx512_cc <16 x i32> @__svml_idiv16_z0(<16 x i32> {{.*}})

define <8 x i32> @test_idiv8_512(<8 x i32> %a) #2 {
entry:
  %0 = tail call svml_cc <8 x i32> @__svml_idiv8(<8 x i32> %a)
  ret <8 x i32> %0
}

; CHECK-LABEL: @test_cexpf4_mask_128
; CHECK: {{.*}} = call svml_cc <4 x float> @__svml_cexpf2_ha_mask_e9(<4 x float> {{.*}}, <2 x i64> {{.*}})
; CHECK: {{.*}} = call svml_cc <4 x float> @__svml_cexpf2_ha_mask_e9(<4 x float> {{.*}}, <2 x i64> {{.*}})

define <8 x float> @test_cexpf4_mask_128(<8 x float> %A, <4 x i64> %B) #0 {
entry:
  %0 = tail call svml_cc <8 x float> @__svml_cexpf4_mask(<8 x float> %A, <4 x i64> %B)
  ret <8 x float> %0
}

; CHECK-LABEL: @test_cexpf4_mask_256
; CHECK: {{.*}} = call svml_avx_cc <8 x float> @__svml_cexpf4_ha_mask_e9(<8 x float> {{.*}}, <4 x i64> {{.*}})

define <8 x float> @test_cexpf4_mask_256(<8 x float> %A, <4 x i64> %B) #1 {
entry:
  %0 = tail call svml_cc <8 x float> @__svml_cexpf4_mask(<8 x float> %A, <4 x i64> %B)
  ret <8 x float> %0
}

; CHECK-LABEL: @test_cexpf4_mask_512
; CHECK: {{.*}} = call svml_avx512_cc <16 x float> @__svml_cexpf8_ha_mask_z0(<16 x float> {{.*}}, <8 x i1> {{.*}}, <16 x float> {{.*}})

define <8 x float> @test_cexpf4_mask_512(<8 x float> %A, <4 x i64> %B) #2 {
entry:
  %0 = tail call svml_cc <8 x float> @__svml_cexpf4_mask(<8 x float> %A, <4 x i64> %B)
  ret <8 x float> %0
}

declare svml_cc <4 x double> @__svml_sin4(<4 x double>)

declare svml_cc { <8 x float>, <8 x float> } @__svml_sincosf8_mask(<8 x float>, <8 x i32>)

declare svml_cc <8 x i32> @__svml_idiv8(<8 x i32>)

declare svml_cc <8 x float> @__svml_cexpf4_mask(<8 x float>, <4 x i64>)

attributes #0 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="128" "prefer-vector-width"="128" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="256" "prefer-vector-width"="256" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="512" "prefer-vector-width"="512" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
