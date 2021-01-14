; Check calls to fmod functions are converted to frem instructions correctly.
;
; RUN: opt < %s -instcombine -S | FileCheck %s

; CHECK-LABEL: @nofast_var(
; CHECK-NEXT:    %call = tail call float @fmodf(float %a, float %b)
define float @nofast_var(float %a, float %b) {
  %call = tail call float @fmodf(float %a, float %b)
  ret float %call
}

; CHECK-LABEL: @nofast_zero(
; CHECK-NEXT:    %call = tail call float @fmodf(float %a, float 0.000000e+00)
define float @nofast_zero(float %a) {
  %call = tail call float @fmodf(float %a, float 0.0)
  ret float %call
}

; CHECK-LABEL: @nofast_nonint(
; CHECK-NEXT:    %1 = frem float %a, 4.500000e+00
define float @nofast_nonint(float %a) {
  %call = tail call float @fmodf(float %a, float 4.5)
  ret float %call
}

; CHECK-LABEL: @nofast_int(
; CHECK-NEXT:    %1 = frem float %a, 4.000000e+00
define float @nofast_int(float %a) {
  %call = tail call float @fmodf(float %a, float 4.0)
  ret float %call
}

; CHECK-LABEL: @fast_var(
; CHECK-NEXT:    %1 = frem float %a, %b
define float @fast_var(float %a, float %b) {
  %call = tail call float @fmodf(float %a, float %b) #0
  ret float %call
}

; CHECK-LABEL: @fast_zero(
; CHECK-NEXT:    %1 = frem float %a, 0.000000e+00
define float @fast_zero(float %a) {
  %call = tail call float @fmodf(float %a, float 0.0) #0
  ret float %call
}

; CHECK-LABEL: @fast_nonint(
; CHECK-NEXT:    %1 = frem float %a, 4.500000e+00
define float @fast_nonint(float %a) {
  %call = tail call float @fmodf(float %a, float 4.5) #0
  ret float %call
}

; CHECK-LABEL: @fast_int(
; CHECK-NEXT:    %1 = frem float %a, 4.000000e+00
define float @fast_int(float %a) {
  %call = tail call float @fmodf(float %a, float 4.0) #0
  ret float %call
}

; CHECK-LABEL: @strict_var(
; CHECK-NEXT:    tail call float @fmodf(float %a, float %b) #0
define float @strict_var(float %a, float %b) {
  %call = tail call float @fmodf(float %a, float %b) #1
  ret float %call
}

; CHECK-LABEL: @strict_zero(
; CHECK-NEXT:    tail call float @fmodf(float %a, float 0.000000e+00) #0
define float @strict_zero(float %a) {
  %call = tail call float @fmodf(float %a, float 0.0) #1
  ret float %call
}

; CHECK-LABEL: @strict_nonint(
; CHECK-NEXT:    tail call float @fmodf(float %a, float 4.500000e+00) #0
define float @strict_nonint(float %a) {
  %call = tail call float @fmodf(float %a, float 4.5) #1
  ret float %call
}

; CHECK-LABEL: @strict_int(
; CHECK-NEXT:    tail call float @fmodf(float %a, float 4.000000e+00) #0
define float @strict_int(float %a) {
  %call = tail call float @fmodf(float %a, float 4.0) #1
  ret float %call
}

declare float @fmodf(float, float)

attributes #0 = { readnone }
attributes #1 = { strictfp }
