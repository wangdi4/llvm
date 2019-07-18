; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i8 @testu8ltr(i8 %a, i8 %b) {
; CHECK-LABEL: testu8ltr
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu8 %[[RES]], %ign
  %cmp = icmp ult i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  ret i8 %res
}

define {i8, i1} @testu8ltrlt(i8 %a, i8 %b) {
; CHECK-LABEL: testu8ltrlt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ult i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ult i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8ltrle(i8 %a, i8 %b) {
; CHECK-LABEL: testu8ltrle
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ult i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ule i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8ltrgt(i8 %a, i8 %b) {
; CHECK-LABEL: testu8ltrgt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ult i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ugt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8ltrge(i8 %a, i8 %b) {
; CHECK-LABEL: testu8ltrge
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ult i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp uge i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define i8 @testu8ler(i8 %a, i8 %b) {
; CHECK-LABEL: testu8ler
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu8 %[[RES]], %ign
  %cmp = icmp ule i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  ret i8 %res
}

define {i8, i1} @testu8lerlt(i8 %a, i8 %b) {
; CHECK-LABEL: testu8lerlt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ule i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ult i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8lerle(i8 %a, i8 %b) {
; CHECK-LABEL: testu8lerle
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ule i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ule i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8lergt(i8 %a, i8 %b) {
; CHECK-LABEL: testu8lergt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ule i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ugt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8lerge(i8 %a, i8 %b) {
; CHECK-LABEL: testu8lerge
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ule i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp uge i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define i8 @testu8gts(i8 %a, i8 %b) {
; CHECK-LABEL: testu8gts
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu8 %[[RES]], %ign
  %cmp = icmp ugt i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  ret i8 %res
}

define {i8, i1} @testu8gtslt(i8 %a, i8 %b) {
; CHECK-LABEL: testu8gtslt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ugt i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ult i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8gtsle(i8 %a, i8 %b) {
; CHECK-LABEL: testu8gtsle
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ugt i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ule i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8gtsgt(i8 %a, i8 %b) {
; CHECK-LABEL: testu8gtsgt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ugt i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ugt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8gtsge(i8 %a, i8 %b) {
; CHECK-LABEL: testu8gtsge
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ugt i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp uge i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define i8 @testu8ges(i8 %a, i8 %b) {
; CHECK-LABEL: testu8ges
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu8 %[[RES]], %ign
  %cmp = icmp uge i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  ret i8 %res
}

define {i8, i1} @testu8geslt(i8 %a, i8 %b) {
; CHECK-LABEL: testu8geslt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp uge i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ult i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8gesle(i8 %a, i8 %b) {
; CHECK-LABEL: testu8gesle
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp uge i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ule i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8gesgt(i8 %a, i8 %b) {
; CHECK-LABEL: testu8gesgt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp uge i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ugt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8gesge(i8 %a, i8 %b) {
; CHECK-LABEL: testu8gesge
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp uge i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp uge i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

