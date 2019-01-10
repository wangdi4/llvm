; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define double @testf64lts(double %a, double %b) {
; CHECK-LABEL: testf64lts
; CHECK: minf64 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = fcmp olt double %a, %b
  %res = select i1 %cmp, double %a, double %b
  ret double %res
}

define {double, i1} @testf64ltslt(double %a, double %b) {
; CHECK-LABEL: testf64ltslt
; CHECK: minf64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = fcmp olt double %a, %b
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp olt double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64ltsle(double %a, double %b) {
; CHECK-LABEL: testf64ltsle
; CHECK: minf64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = fcmp olt double %a, %b
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp ole double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64ltsgt(double %a, double %b) {
; CHECK-LABEL: testf64ltsgt
; CHECK: minf64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = fcmp olt double %a, %b
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp ogt double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64ltsge(double %a, double %b) {
; CHECK-LABEL: testf64ltsge
; CHECK: minf64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = fcmp olt double %a, %b
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp oge double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define double @testf64les(double %a, double %b) {
; CHECK-LABEL: testf64les
; CHECK: minf64 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = fcmp ole double %a, %b
  %res = select i1 %cmp, double %a, double %b
  ret double %res
}

define {double, i1} @testf64leslt(double %a, double %b) {
; CHECK-LABEL: testf64leslt
; CHECK: minf64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = fcmp ole double %a, %b
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp olt double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64lesle(double %a, double %b) {
; CHECK-LABEL: testf64lesle
; CHECK: minf64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = fcmp ole double %a, %b
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp ole double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64lesgt(double %a, double %b) {
; CHECK-LABEL: testf64lesgt
; CHECK: minf64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = fcmp ole double %a, %b
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp ogt double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64lesge(double %a, double %b) {
; CHECK-LABEL: testf64lesge
; CHECK: minf64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = fcmp ole double %a, %b
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp oge double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define double @testf64gtr(double %a, double %b) {
; CHECK-LABEL: testf64gtr
; CHECK: minf64 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = fcmp ogt double %b, %a
  %res = select i1 %cmp, double %a, double %b
  ret double %res
}

define {double, i1} @testf64gtrlt(double %a, double %b) {
; CHECK-LABEL: testf64gtrlt
; CHECK: minf64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = fcmp ogt double %b, %a
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp olt double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64gtrle(double %a, double %b) {
; CHECK-LABEL: testf64gtrle
; CHECK: minf64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = fcmp ogt double %b, %a
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp ole double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64gtrgt(double %a, double %b) {
; CHECK-LABEL: testf64gtrgt
; CHECK: minf64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = fcmp ogt double %b, %a
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp ogt double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64gtrge(double %a, double %b) {
; CHECK-LABEL: testf64gtrge
; CHECK: minf64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = fcmp ogt double %b, %a
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp oge double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define double @testf64ger(double %a, double %b) {
; CHECK-LABEL: testf64ger
; CHECK: minf64 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = fcmp oge double %b, %a
  %res = select i1 %cmp, double %a, double %b
  ret double %res
}

define {double, i1} @testf64gerlt(double %a, double %b) {
; CHECK-LABEL: testf64gerlt
; CHECK: minf64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = fcmp oge double %b, %a
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp olt double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64gerle(double %a, double %b) {
; CHECK-LABEL: testf64gerle
; CHECK: minf64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = fcmp oge double %b, %a
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp ole double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64gergt(double %a, double %b) {
; CHECK-LABEL: testf64gergt
; CHECK: minf64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = fcmp oge double %b, %a
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp ogt double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64gerge(double %a, double %b) {
; CHECK-LABEL: testf64gerge
; CHECK: minf64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = fcmp oge double %b, %a
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp oge double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define double @testf64fmin(double %a, double %b) {
; CHECK-LABEL: testf64fmin
; CHECK: minf64 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %res = tail call double @llvm.minnum.f64(double %a, double %b)
  ret double %res
}

define {double, i1} @testf64fminlt(double %a, double %b) {
; CHECK-LABEL: testf64fminlt
; CHECK: minf64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %res = tail call double @llvm.minnum.f64(double %a, double %b)
  %cmp = fcmp olt double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64fminle(double %a, double %b) {
; CHECK-LABEL: testf64fminle
; CHECK: minf64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %res = tail call double @llvm.minnum.f64(double %a, double %b)
  %cmp = fcmp ole double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64fmingt(double %a, double %b) {
; CHECK-LABEL: testf64fmingt
; CHECK: minf64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %res = tail call double @llvm.minnum.f64(double %a, double %b)
  %cmp = fcmp ogt double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64fminge(double %a, double %b) {
; CHECK-LABEL: testf64fminge
; CHECK: minf64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %res = tail call double @llvm.minnum.f64(double %a, double %b)
  %cmp = fcmp oge double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp, 1
  ret {double, i1} %ret1
}

declare double @llvm.minnum.f64(double %a, double %b)
