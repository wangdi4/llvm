; RUN: opt -vector-library=SVML       -inject-tli-mappings        -S < %s | FileCheck %s  --check-prefixes=COMMON,SVML
; RUN: opt -vector-library=SVML       -passes=inject-tli-mappings -S < %s | FileCheck %s  --check-prefixes=COMMON,SVML
; RUN: opt -vector-library=MASSV      -inject-tli-mappings        -S < %s | FileCheck %s  --check-prefixes=COMMON,MASSV
; RUN: opt -vector-library=MASSV      -passes=inject-tli-mappings -S < %s | FileCheck %s  --check-prefixes=COMMON,MASSV
; RUN: opt -vector-library=Accelerate -inject-tli-mappings        -S < %s | FileCheck %s  --check-prefixes=COMMON,ACCELERATE
; RUN: opt -vector-library=Accelerate -passes=inject-tli-mappings -S < %s | FileCheck %s  --check-prefixes=COMMON,ACCELERATE

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; COMMON-LABEL: @llvm.compiler.used = appending global
; INTEL_CUSTOMIZATION
; SVML-SAME:        [12 x i8*] [
; end INTEL_CUSTOMIZATION
; SVML-SAME:          i8* bitcast (<2 x double> (<2 x double>)* @__svml_sin2 to i8*),
; SVML-SAME:          i8* bitcast (<4 x double> (<4 x double>)* @__svml_sin4 to i8*),
; SVML-SAME:          i8* bitcast (<8 x double> (<8 x double>)* @__svml_sin8 to i8*)
; MASSV-SAME:       [2 x i8*] [
; MASSV-SAME:         i8* bitcast (<2 x double> (<2 x double>)* @__sind2_massv to i8*),
; MASSV-SAME:         i8* bitcast (<4 x float> (<4 x float>)* @__log10f4_massv to i8*)
; ACCELERATE-SAME:  [1 x i8*] [
; ACCELERATE-SAME:    i8* bitcast (<4 x float> (<4 x float>)* @vlog10f to i8*)
; COMMON-SAME:      ], section "llvm.metadata"

define double @sin_f64(double %in) {
; COMMON-LABEL: @sin_f64(
; SVML:         call double @sin(double %{{.*}}) #[[SIN:[0-9]+]]
; MASSV:        call double @sin(double %{{.*}}) #[[SIN:[0-9]+]]
; ACCELERATE:   call double @sin(double %{{.*}})
; No mapping of "sin" to a vector function for Accelerate.
; ACCELERATE-NOT: _ZGV_LLVM_{{.*}}_sin({{.*}})
  %call = tail call double @sin(double %in)
  ret double %call
}

declare double @sin(double) #0

define float @call_llvm.log10.f32(float %in) {
; COMMON-LABEL: @call_llvm.log10.f32(
; SVML:         call float @llvm.log10.f32(float %{{.*}}) #[[LOG10:[0-9]+]] ;INTEL
; MASSV:        call float @llvm.log10.f32(float %{{.*}}) #[[LOG10:[0-9]+]]
; ACCELERATE:   call float @llvm.log10.f32(float %{{.*}}) #[[LOG10:[0-9]+]]
; SVML is mapped ;INTEL
; S;VML-NOT:     _ZGV_LLVM_{{.*}}_llvm.log10.f32({{.*}}) ;INTEL
  %call = tail call float @llvm.log10.f32(float %in)
  ret float %call
}

declare float @llvm.log10.f32(float) #0
attributes #0 = { nounwind readnone }

; SVML:      attributes #[[SIN]] = { "vector-function-abi-variant"=
; SVML-SAME:   "_ZGV_LLVM_N2v_sin(__svml_sin2),
; SVML-SAME:   _ZGV_LLVM_N4v_sin(__svml_sin4),
; INTEL_CUSTOMIZATION
; SVML-SAME:   _ZGV_LLVM_N8v_sin(__svml_sin8)
; SVML:      attributes #[[LOG10]] = { "vector-function-abi-variant"=
; SVML-SAME:   "_ZGV_LLVM_N2v_llvm.log10.f32(__svml_log10f2),
; SVML-SAME:   _ZGV_LLVM_N4v_llvm.log10.f32(__svml_log10f4),
; SVML-SAME:   _ZGV_LLVM_N8v_llvm.log10.f32(__svml_log10f8)
; SVML-SAME:   _ZGV_LLVM_N16v_llvm.log10.f32(__svml_log10f16)
; end INTEL_CUSTOMIZATION

; MASSV:      attributes #[[SIN]] = { "vector-function-abi-variant"=
; MASSV-SAME:   "_ZGV_LLVM_N2v_sin(__sind2_massv)" }
; MASSV:      attributes #[[LOG10]] = { "vector-function-abi-variant"=
; MASSV-SAME:   "_ZGV_LLVM_N4v_llvm.log10.f32(__log10f4_massv)" }

; ACCELERATE:      attributes #[[LOG10]] = { "vector-function-abi-variant"=
; ACCELERATE-SAME:   "_ZGV_LLVM_N4v_llvm.log10.f32(vlog10f)" }
