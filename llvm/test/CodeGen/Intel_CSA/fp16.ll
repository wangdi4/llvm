; RUN: llc <%s | FileCheck %s
; ModuleID = 'test.cpp'
source_filename = "test.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "csa"

declare half @llvm.fma.f16(half, half, half)
declare half @llvm.fabs.f16(half)
declare half @llvm.sqrt.f16(half)

define half @addfp16(half %a, half %b) {
; CHECK-LABEL: addfp16
; CHECK: addf16
  %res = fadd half %a, %b
  ret half %res
}

define half @subfp16(half %a, half %b) {
; CHECK-LABEL: subfp16
; CHECK: subf16
  %res = fsub half %a, %b
  ret half %res
}

define half @mulfp16(half %a, half %b) {
; CHECK-LABEL: mulfp16
; CHECK: mulf16
  %res = fmul half %a, %b
  ret half %res
}

define half @divfp16(half %a, half %b) {
; CHECK-LABEL: divfp16
; CHECK: divf16
  %res = fdiv half %a, %b
  ret half %res
}

define half @sqrtfp16(half %a) {
; CHECK-LABEL: sqrtfp16
; CHECK: sqrtf16
  %res = call half @llvm.sqrt.f16(half %a)
  ret half %res
}

define half @absfp16(half %a) {
; CHECK-LABEL: absfp16
; CHECK: absf16
  %res = call half @llvm.fabs.f16(half %a)
  ret half %res
}

define half @negfp16(half %a) {
; CHECK-LABEL: negfp16
; CHECK: negf16
  %res = fneg half %a
  ret half %res
}

define half @selectfp16(i1 %a, half %b, half %c) {
; CHECK-LABEL: selectfp16
; CHECK: merge16
  %res = select i1 %a, half %b, half %c
  ret half %res
}

define half @fmafp16(half %a, half %b, half %c) {
; CHECK-LABEL: fmafp16
; CHECK: fmaf16
  %res = call half @llvm.fma.f16(half %a, half %b, half %c)
  ret half %res
}

define half @asmfp16(half %a, half %b, half %c) {
; CHECK-LABEL: asmfp16
; CHECK: fmsf16
  %res = call half asm sideeffect "fmsf16 $0, $1, $2, $3, 3", "=b,b,b,b"(half %a, half %b, half %c)
  ret half %res
}

define half @imm0fp16() {
; CHECK-LABEL: imm0fp16
; No actual check here: if this fails, we get a compilation error instead.
  ret half 0.0
}

define i1 @cmporderedfp16(half %a, half %b) {
; CHECK-LABEL: cmporderedfp16
; CHECK: cmpgtf16
; CHECK-SAME: FLT_ORDERED
  %res = fcmp ogt half %a, %b
  ret i1 %res
}

define i1 @cmpunorderedfp16(half %a, half %b) {
; CHECK-LABEL: cmpunorderedfp16
; CHECK: cmpgtf16
; CHECK-SAME: FLT_UNORDERED
  %res = fcmp ugt half %a, %b
  ret i1 %res
}

define i1 @cmpanyorderedfp16(half %a, half %b) #0 {
; CHECK-LABEL: cmpanyorderedfp16
; CHECK: cmpgtf16
  %res = fcmp ugt half %a, %b
  ret i1 %res
}

define float @f16tof32(half %a) {
; CHECK-LABEL: f16tof32
; CHECK: cvtf32f16
  %res = fpext half %a to float
  ret float %res
}

define half @f32tof16(float %a) {
; CHECK-LABEL: f32tof16
; CHECK: cvtf16f32
  %res = fptrunc float %a to half
  ret half %res
}

define double @f16tof64(half %a) {
; CHECK-LABEL: f16tof64
; CHECK: cvtf32f16
; CHECK: cvtf64f32
  %res = fpext half %a to double
  ret double %res
}

define half @f64tof16(double %a) {
; CHECK-LABEL: f64tof16
; CHECK: cvtf32f64
; CHECK: cvtf16f32
  %res = fptrunc double %a to half
  ret half %res
}

define i32 @f16toi32(half %a) {
; CHECK-LABEL: f16toi32
; CHECK: cvtf32f16
; CHECK: cvtu32f32
  %res = fptoui half %a to i32
  ret i32 %res
}

define half @i32tof16(i32 %a) {
; CHECK-LABEL: i32tof16
; CHECK: cvtf32u32
; CHECK: cvtf16f32
  %res = uitofp i32 %a to half
  ret half %res
}

define half @loadf16(half* %a) {
; CHECK-LABEL: loadf16
; CHECK: ld16
  %res = load half, half* %a, align 2
  ret half %res
}

define void @storef16(half* %a, half %val) {
; CHECK-LABEL: storef16
; CHECK: st16
  store half %val, half* %a, align 2
  ret void
}

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" "use-soft-float"="false" }
