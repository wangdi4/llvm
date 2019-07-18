; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define float @testf32lts(float %a, float %b) {
; CHECK-LABEL: testf32lts
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], %ign
  %cmp = fcmp olt float %a, %b
  %res = select i1 %cmp, float %a, float %b
  ret float %res
}

define {float, i1} @testf32ltslt(float %a, float %b) {
; CHECK-LABEL: testf32ltslt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = fcmp olt float %a, %b
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp olt float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32ltsle(float %a, float %b) {
; CHECK-LABEL: testf32ltsle
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], [[CMP:[^,]+]]
  %cmp = fcmp olt float %a, %b
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp ole float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32ltsgt(float %a, float %b) {
; CHECK-LABEL: testf32ltsgt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = fcmp olt float %a, %b
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp ogt float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32ltsge(float %a, float %b) {
; CHECK-LABEL: testf32ltsge
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], [[CMP:[^,]+]]
  %cmp = fcmp olt float %a, %b
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp oge float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define float @testf32les(float %a, float %b) {
; CHECK-LABEL: testf32les
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], %ign
  %cmp = fcmp ole float %a, %b
  %res = select i1 %cmp, float %a, float %b
  ret float %res
}

define {float, i1} @testf32leslt(float %a, float %b) {
; CHECK-LABEL: testf32leslt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = fcmp ole float %a, %b
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp olt float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32lesle(float %a, float %b) {
; CHECK-LABEL: testf32lesle
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], [[CMP:[^,]+]]
  %cmp = fcmp ole float %a, %b
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp ole float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32lesgt(float %a, float %b) {
; CHECK-LABEL: testf32lesgt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = fcmp ole float %a, %b
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp ogt float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32lesge(float %a, float %b) {
; CHECK-LABEL: testf32lesge
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], [[CMP:[^,]+]]
  %cmp = fcmp ole float %a, %b
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp oge float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define float @testf32gtr(float %a, float %b) {
; CHECK-LABEL: testf32gtr
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], %ign
  %cmp = fcmp ogt float %b, %a
  %res = select i1 %cmp, float %a, float %b
  ret float %res
}

define {float, i1} @testf32gtrlt(float %a, float %b) {
; CHECK-LABEL: testf32gtrlt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = fcmp ogt float %b, %a
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp olt float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32gtrle(float %a, float %b) {
; CHECK-LABEL: testf32gtrle
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], [[CMP:[^,]+]]
  %cmp = fcmp ogt float %b, %a
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp ole float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32gtrgt(float %a, float %b) {
; CHECK-LABEL: testf32gtrgt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = fcmp ogt float %b, %a
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp ogt float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32gtrge(float %a, float %b) {
; CHECK-LABEL: testf32gtrge
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], [[CMP:[^,]+]]
  %cmp = fcmp ogt float %b, %a
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp oge float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define float @testf32ger(float %a, float %b) {
; CHECK-LABEL: testf32ger
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], %ign
  %cmp = fcmp oge float %b, %a
  %res = select i1 %cmp, float %a, float %b
  ret float %res
}

define {float, i1} @testf32gerlt(float %a, float %b) {
; CHECK-LABEL: testf32gerlt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = fcmp oge float %b, %a
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp olt float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32gerle(float %a, float %b) {
; CHECK-LABEL: testf32gerle
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], [[CMP:[^,]+]]
  %cmp = fcmp oge float %b, %a
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp ole float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32gergt(float %a, float %b) {
; CHECK-LABEL: testf32gergt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = fcmp oge float %b, %a
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp ogt float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32gerge(float %a, float %b) {
; CHECK-LABEL: testf32gerge
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], [[CMP:[^,]+]]
  %cmp = fcmp oge float %b, %a
  %res = select i1 %cmp, float %a, float %b
  %cmp2 = fcmp oge float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp2, 1
  ret {float, i1} %ret1
}

define float @testf32fmin(float %a, float %b) {
; CHECK-LABEL: testf32fmin
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], %ign
  %res = tail call float @llvm.minnum.f32(float %a, float %b)
  ret float %res
}

define {float, i1} @testf32fminlt(float %a, float %b) {
; CHECK-LABEL: testf32fminlt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %res = tail call float @llvm.minnum.f32(float %a, float %b)
  %cmp = fcmp olt float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32fminle(float %a, float %b) {
; CHECK-LABEL: testf32fminle
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], [[CMP:[^,]+]]
  %res = tail call float @llvm.minnum.f32(float %a, float %b)
  %cmp = fcmp ole float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32fmingt(float %a, float %b) {
; CHECK-LABEL: testf32fmingt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %res = tail call float @llvm.minnum.f32(float %a, float %b)
  %cmp = fcmp ogt float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp, 1
  ret {float, i1} %ret1
}

define {float, i1} @testf32fminge(float %a, float %b) {
; CHECK-LABEL: testf32fminge
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minf32 %[[RES]], [[CMP:[^,]+]]
  %res = tail call float @llvm.minnum.f32(float %a, float %b)
  %cmp = fcmp oge float %a, %b
  %ret0 = insertvalue {float, i1} undef, float %res, 0
  %ret1 = insertvalue {float, i1} %ret0, i1 %cmp, 1
  ret {float, i1} %ret1
}

declare float @llvm.minnum.f32(float %a, float %b)
