; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define double @testf64ltr(double %a, double %b) {
; CHECK-LABEL: testf64ltr
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], %ign
  %cmp = fcmp olt double %b, %a
  %res = select i1 %cmp, double %a, double %b
  ret double %res
}

define {double, i1} @testf64ltrlt(double %a, double %b) {
; CHECK-LABEL: testf64ltrlt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = fcmp olt double %b, %a
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp olt double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64ltrle(double %a, double %b) {
; CHECK-LABEL: testf64ltrle
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], [[CMP:[^,]+]]
  %cmp = fcmp olt double %b, %a
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp ole double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64ltrgt(double %a, double %b) {
; CHECK-LABEL: testf64ltrgt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = fcmp olt double %b, %a
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp ogt double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64ltrge(double %a, double %b) {
; CHECK-LABEL: testf64ltrge
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], [[CMP:[^,]+]]
  %cmp = fcmp olt double %b, %a
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp oge double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define double @testf64ler(double %a, double %b) {
; CHECK-LABEL: testf64ler
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], %ign
  %cmp = fcmp ole double %b, %a
  %res = select i1 %cmp, double %a, double %b
  ret double %res
}

define {double, i1} @testf64lerlt(double %a, double %b) {
; CHECK-LABEL: testf64lerlt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = fcmp ole double %b, %a
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp olt double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64lerle(double %a, double %b) {
; CHECK-LABEL: testf64lerle
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], [[CMP:[^,]+]]
  %cmp = fcmp ole double %b, %a
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp ole double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64lergt(double %a, double %b) {
; CHECK-LABEL: testf64lergt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = fcmp ole double %b, %a
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp ogt double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64lerge(double %a, double %b) {
; CHECK-LABEL: testf64lerge
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], [[CMP:[^,]+]]
  %cmp = fcmp ole double %b, %a
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp oge double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define double @testf64gts(double %a, double %b) {
; CHECK-LABEL: testf64gts
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], %ign
  %cmp = fcmp ogt double %a, %b
  %res = select i1 %cmp, double %a, double %b
  ret double %res
}

define {double, i1} @testf64gtslt(double %a, double %b) {
; CHECK-LABEL: testf64gtslt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = fcmp ogt double %a, %b
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp olt double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64gtsle(double %a, double %b) {
; CHECK-LABEL: testf64gtsle
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], [[CMP:[^,]+]]
  %cmp = fcmp ogt double %a, %b
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp ole double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64gtsgt(double %a, double %b) {
; CHECK-LABEL: testf64gtsgt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = fcmp ogt double %a, %b
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp ogt double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64gtsge(double %a, double %b) {
; CHECK-LABEL: testf64gtsge
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], [[CMP:[^,]+]]
  %cmp = fcmp ogt double %a, %b
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp oge double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define double @testf64ges(double %a, double %b) {
; CHECK-LABEL: testf64ges
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], %ign
  %cmp = fcmp oge double %a, %b
  %res = select i1 %cmp, double %a, double %b
  ret double %res
}

define {double, i1} @testf64geslt(double %a, double %b) {
; CHECK-LABEL: testf64geslt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = fcmp oge double %a, %b
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp olt double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64gesle(double %a, double %b) {
; CHECK-LABEL: testf64gesle
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], [[CMP:[^,]+]]
  %cmp = fcmp oge double %a, %b
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp ole double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64gesgt(double %a, double %b) {
; CHECK-LABEL: testf64gesgt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = fcmp oge double %a, %b
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp ogt double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64gesge(double %a, double %b) {
; CHECK-LABEL: testf64gesge
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], [[CMP:[^,]+]]
  %cmp = fcmp oge double %a, %b
  %res = select i1 %cmp, double %a, double %b
  %cmp2 = fcmp oge double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp2, 1
  ret {double, i1} %ret1
}

define double @testf64fmax(double %a, double %b) {
; CHECK-LABEL: testf64fmax
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], %ign
  %res = tail call double @llvm.maxnum.f64(double %a, double %b)
  ret double %res
}

define {double, i1} @testf64fmaxlt(double %a, double %b) {
; CHECK-LABEL: testf64fmaxlt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %res = tail call double @llvm.maxnum.f64(double %a, double %b)
  %cmp = fcmp olt double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64fmaxle(double %a, double %b) {
; CHECK-LABEL: testf64fmaxle
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], [[CMP:[^,]+]]
  %res = tail call double @llvm.maxnum.f64(double %a, double %b)
  %cmp = fcmp ole double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64fmaxgt(double %a, double %b) {
; CHECK-LABEL: testf64fmaxgt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %res = tail call double @llvm.maxnum.f64(double %a, double %b)
  %cmp = fcmp ogt double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp, 1
  ret {double, i1} %ret1
}

define {double, i1} @testf64fmaxge(double %a, double %b) {
; CHECK-LABEL: testf64fmaxge
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxf64 %[[RES]], [[CMP:[^,]+]]
  %res = tail call double @llvm.maxnum.f64(double %a, double %b)
  %cmp = fcmp oge double %a, %b
  %ret0 = insertvalue {double, i1} undef, double %res, 0
  %ret1 = insertvalue {double, i1} %ret0, i1 %cmp, 1
  ret {double, i1} %ret1
}

declare double @llvm.maxnum.f64(double %a, double %b)
