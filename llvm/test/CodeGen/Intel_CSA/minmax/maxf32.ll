; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define float @testf32ltr(float %a, float %b) {
; CHECK-LABEL: testf32ltr
; CHECK: maxf32 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = fcmp olt float %b, %a
  %res = select i1 %cmp, float %a, float %b
  ret float %res
}

define {float, i1} @testf32ltrlt(float %a, float %b) {
; CHECK-LABEL: testf32ltrlt
; CHECK: maxf32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = fcmp olt float %b, %a
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp olt float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32ltrle(float %a, float %b) {
; CHECK-LABEL: testf32ltrle
; CHECK: maxf32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = fcmp olt float %b, %a
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp ole float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32ltrgt(float %a, float %b) {
; CHECK-LABEL: testf32ltrgt
; CHECK: maxf32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = fcmp olt float %b, %a
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp ogt float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32ltrge(float %a, float %b) {
; CHECK-LABEL: testf32ltrge
; CHECK: maxf32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = fcmp olt float %b, %a
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp oge float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define float @testf32ler(float %a, float %b) {
; CHECK-LABEL: testf32ler
; CHECK: maxf32 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = fcmp ole float %b, %a
  %res = select i1 %cmp, float %a, float %b
  ret float %res
}

define {float, i1} @testf32lerlt(float %a, float %b) {
; CHECK-LABEL: testf32lerlt
; CHECK: maxf32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = fcmp ole float %b, %a
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp olt float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32lerle(float %a, float %b) {
; CHECK-LABEL: testf32lerle
; CHECK: maxf32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = fcmp ole float %b, %a
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp ole float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32lergt(float %a, float %b) {
; CHECK-LABEL: testf32lergt
; CHECK: maxf32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = fcmp ole float %b, %a
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp ogt float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32lerge(float %a, float %b) {
; CHECK-LABEL: testf32lerge
; CHECK: maxf32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = fcmp ole float %b, %a
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp oge float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define float @testf32gts(float %a, float %b) {
; CHECK-LABEL: testf32gts
; CHECK: maxf32 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = fcmp ogt float %a, %b
  %res = select i1 %cmp, float %a, float %b
  ret float %res
}

define {float, i1} @testf32gtslt(float %a, float %b) {
; CHECK-LABEL: testf32gtslt
; CHECK: maxf32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = fcmp ogt float %a, %b
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp olt float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32gtsle(float %a, float %b) {
; CHECK-LABEL: testf32gtsle
; CHECK: maxf32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = fcmp ogt float %a, %b
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp ole float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32gtsgt(float %a, float %b) {
; CHECK-LABEL: testf32gtsgt
; CHECK: maxf32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = fcmp ogt float %a, %b
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp ogt float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32gtsge(float %a, float %b) {
; CHECK-LABEL: testf32gtsge
; CHECK: maxf32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = fcmp ogt float %a, %b
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp oge float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define float @testf32ges(float %a, float %b) {
; CHECK-LABEL: testf32ges
; CHECK: maxf32 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = fcmp oge float %a, %b
  %res = select i1 %cmp, float %a, float %b
  ret float %res
}

define {float, i1} @testf32geslt(float %a, float %b) {
; CHECK-LABEL: testf32geslt
; CHECK: maxf32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = fcmp oge float %a, %b
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp olt float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32gesle(float %a, float %b) {
; CHECK-LABEL: testf32gesle
; CHECK: maxf32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = fcmp oge float %a, %b
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp ole float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32gesgt(float %a, float %b) {
; CHECK-LABEL: testf32gesgt
; CHECK: maxf32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = fcmp oge float %a, %b
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp ogt float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32gesge(float %a, float %b) {
; CHECK-LABEL: testf32gesge
; CHECK: maxf32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = fcmp oge float %a, %b
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp oge float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define float @testf32fmax(float %a, float %b) {
; CHECK-LABEL: testf32fmax
; CHECK: maxf32 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %res = tail call float @llvm.maxnum.f32(float %a, float %b)
  ret float %res
}

define {float, i1} @testf32fmaxlt(float %a, float %b) {
; CHECK-LABEL: testf32fmaxlt
; CHECK: maxf32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %res = tail call float @llvm.maxnum.f32(float %a, float %b)
  %cmp = fcmp olt float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32fmaxle(float %a, float %b) {
; CHECK-LABEL: testf32fmaxle
; CHECK: maxf32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %res = tail call float @llvm.maxnum.f32(float %a, float %b)
  %cmp = fcmp ole float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32fmaxgt(float %a, float %b) {
; CHECK-LABEL: testf32fmaxgt
; CHECK: maxf32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %res = tail call float @llvm.maxnum.f32(float %a, float %b)
  %cmp = fcmp ogt float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32fmaxge(float %a, float %b) {
; CHECK-LABEL: testf32fmaxge
; CHECK: maxf32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %res = tail call float @llvm.maxnum.f32(float %a, float %b)
  %cmp = fcmp oge float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp, 1
  ret {float, i1} %ret1
}

declare float @llvm.maxnum.f32(float %a, float %b)
