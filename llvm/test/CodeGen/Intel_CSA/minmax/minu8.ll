; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i8 @testu8lts(i8 %a, i8 %b) {
; CHECK-LABEL: testu8lts
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: minu8 %[[RES]], %ign
  %cmp = icmp ult i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  ret i8 %res
}

define {i8, i1} @testu8ltslt(i8 %a, i8 %b) {
; CHECK-LABEL: testu8ltslt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: minu8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ult i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ult i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8ltsle(i8 %a, i8 %b) {
; CHECK-LABEL: testu8ltsle
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: minu8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ult i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ule i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8ltsgt(i8 %a, i8 %b) {
; CHECK-LABEL: testu8ltsgt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: minu8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ult i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ugt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8ltsge(i8 %a, i8 %b) {
; CHECK-LABEL: testu8ltsge
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: minu8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ult i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp uge i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define i8 @testu8les(i8 %a, i8 %b) {
; CHECK-LABEL: testu8les
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: minu8 %[[RES]], %ign
  %cmp = icmp ule i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  ret i8 %res
}

define {i8, i1} @testu8leslt(i8 %a, i8 %b) {
; CHECK-LABEL: testu8leslt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: minu8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ule i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ult i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8lesle(i8 %a, i8 %b) {
; CHECK-LABEL: testu8lesle
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: minu8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ule i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ule i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8lesgt(i8 %a, i8 %b) {
; CHECK-LABEL: testu8lesgt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: minu8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ule i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ugt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8lesge(i8 %a, i8 %b) {
; CHECK-LABEL: testu8lesge
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: minu8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ule i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp uge i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define i8 @testu8gtr(i8 %a, i8 %b) {
; CHECK-LABEL: testu8gtr
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: minu8 %[[RES]], %ign
  %cmp = icmp ugt i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  ret i8 %res
}

define {i8, i1} @testu8gtrlt(i8 %a, i8 %b) {
; CHECK-LABEL: testu8gtrlt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: minu8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ugt i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ult i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8gtrle(i8 %a, i8 %b) {
; CHECK-LABEL: testu8gtrle
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: minu8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ugt i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ule i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8gtrgt(i8 %a, i8 %b) {
; CHECK-LABEL: testu8gtrgt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: minu8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ugt i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ugt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8gtrge(i8 %a, i8 %b) {
; CHECK-LABEL: testu8gtrge
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: minu8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ugt i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp uge i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define i8 @testu8ger(i8 %a, i8 %b) {
; CHECK-LABEL: testu8ger
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: minu8 %[[RES]], %ign
  %cmp = icmp uge i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  ret i8 %res
}

define {i8, i1} @testu8gerlt(i8 %a, i8 %b) {
; CHECK-LABEL: testu8gerlt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: minu8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp uge i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ult i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8gerle(i8 %a, i8 %b) {
; CHECK-LABEL: testu8gerle
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: minu8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp uge i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ule i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8gergt(i8 %a, i8 %b) {
; CHECK-LABEL: testu8gergt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: minu8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp uge i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp ugt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @testu8gerge(i8 %a, i8 %b) {
; CHECK-LABEL: testu8gerge
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: minu8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp uge i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp uge i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

