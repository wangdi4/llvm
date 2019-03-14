; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i32 @testu32lts(i32 %a, i32 %b) {
; CHECK-LABEL: testu32lts
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minu32 %[[RES]], %ign
  %cmp = icmp ult i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @testu32ltslt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32ltslt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minu32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ult i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ult i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32ltsle(i32 %a, i32 %b) {
; CHECK-LABEL: testu32ltsle
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minu32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ult i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ule i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32ltsgt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32ltsgt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minu32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ult i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ugt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32ltsge(i32 %a, i32 %b) {
; CHECK-LABEL: testu32ltsge
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minu32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ult i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp uge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define i32 @testu32les(i32 %a, i32 %b) {
; CHECK-LABEL: testu32les
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minu32 %[[RES]], %ign
  %cmp = icmp ule i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @testu32leslt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32leslt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minu32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ule i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ult i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32lesle(i32 %a, i32 %b) {
; CHECK-LABEL: testu32lesle
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minu32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ule i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ule i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32lesgt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32lesgt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minu32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ule i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ugt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32lesge(i32 %a, i32 %b) {
; CHECK-LABEL: testu32lesge
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minu32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ule i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp uge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define i32 @testu32gtr(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gtr
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minu32 %[[RES]], %ign
  %cmp = icmp ugt i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @testu32gtrlt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gtrlt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minu32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ugt i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ult i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32gtrle(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gtrle
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minu32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ugt i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ule i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32gtrgt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gtrgt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minu32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ugt i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ugt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32gtrge(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gtrge
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minu32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ugt i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp uge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define i32 @testu32ger(i32 %a, i32 %b) {
; CHECK-LABEL: testu32ger
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minu32 %[[RES]], %ign
  %cmp = icmp uge i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @testu32gerlt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gerlt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minu32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp uge i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ult i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32gerle(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gerle
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minu32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp uge i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ule i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32gergt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gergt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minu32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp uge i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ugt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32gerge(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gerge
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: minu32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp uge i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp uge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

