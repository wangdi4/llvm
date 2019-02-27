; RUN: llc < %s | FileCheck %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i1 @foeq(float %a, float %b) {
; CHECK-LABEL: foeq
; CHECK: cmpeqf32 %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, FLT_ORDERED, FLT_NONSIGNALING
  %res = fcmp oeq float %a, %b
  ret i1 %res
}

define i1 @fone(float %a, float %b) {
; CHECK-LABEL: fone
; CHECK: cmpnef32 %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, FLT_ORDERED, FLT_NONSIGNALING
  %res = fcmp one float %a, %b
  ret i1 %res
}

define i1 @folt(float %a, float %b) {
; CHECK-LABEL: folt
; CHECK: cmpltf32 %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, FLT_ORDERED, FLT_NONSIGNALING
  %res = fcmp olt float %a, %b
  ret i1 %res
}

define i1 @fogt(float %a, float %b) {
; CHECK-LABEL: fogt
; CHECK: cmpgtf32 %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, FLT_ORDERED, FLT_NONSIGNALING
  %res = fcmp ogt float %a, %b
  ret i1 %res
}

define i1 @fole(float %a, float %b) {
; CHECK-LABEL: fole
; CHECK: cmplef32 %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, FLT_ORDERED, FLT_NONSIGNALING
  %res = fcmp ole float %a, %b
  ret i1 %res
}

define i1 @foge(float %a, float %b) {
; CHECK-LABEL: foge
; CHECK: cmpgef32 %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, FLT_ORDERED, FLT_NONSIGNALING
  %res = fcmp oge float %a, %b
  ret i1 %res
}

define i1 @fueq(float %a, float %b) {
; CHECK-LABEL: fueq
; CHECK: cmpeqf32 %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, FLT_UNORDERED, FLT_NONSIGNALING
  %res = fcmp ueq float %a, %b
  ret i1 %res
}

define i1 @fune(float %a, float %b) {
; CHECK-LABEL: fune
; CHECK: cmpnef32 %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, FLT_UNORDERED, FLT_NONSIGNALING
  %res = fcmp une float %a, %b
  ret i1 %res
}

define i1 @fult(float %a, float %b) {
; CHECK-LABEL: fult
; CHECK: cmpltf32 %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, FLT_UNORDERED, FLT_NONSIGNALING
  %res = fcmp ult float %a, %b
  ret i1 %res
}

define i1 @fugt(float %a, float %b) {
; CHECK-LABEL: fugt
; CHECK: cmpgtf32 %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, FLT_UNORDERED, FLT_NONSIGNALING
  %res = fcmp ugt float %a, %b
  ret i1 %res
}

define i1 @fule(float %a, float %b) {
; CHECK-LABEL: fule
; CHECK: cmplef32 %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, FLT_UNORDERED, FLT_NONSIGNALING
  %res = fcmp ule float %a, %b
  ret i1 %res
}

define i1 @fuge(float %a, float %b) {
; CHECK-LABEL: fuge
; CHECK: cmpgef32 %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, FLT_UNORDERED, FLT_NONSIGNALING
  %res = fcmp uge float %a, %b
  ret i1 %res
}

define i1 @ford(float %a, float %b) {
; CHECK-LABEL: ford
; CHECK: cmpof32 %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, FLT_ORDERED, FLT_NONSIGNALING
  %res = fcmp ord float %a, %b
  ret i1 %res
}

define i1 @funo(float %a, float %b) {
; CHECK-LABEL: funo
; CHECK: cmpuof32 %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, %{{[0-9a-zA-Z_.]+}}, FLT_UNORDERED, FLT_NONSIGNALING
  %res = fcmp uno float %a, %b
  ret i1 %res
}
