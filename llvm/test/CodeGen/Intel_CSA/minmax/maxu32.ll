; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i32 @testu32ltr(i32 %a, i32 %b) {
; CHECK-LABEL: testu32ltr
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu32 %[[RES]], %ign
  %cmp = icmp ult i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @testu32ltrlt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32ltrlt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ult i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ult i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32ltrle(i32 %a, i32 %b) {
; CHECK-LABEL: testu32ltrle
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ult i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ule i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32ltrgt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32ltrgt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ult i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ugt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32ltrge(i32 %a, i32 %b) {
; CHECK-LABEL: testu32ltrge
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ult i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp uge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define i32 @testu32ler(i32 %a, i32 %b) {
; CHECK-LABEL: testu32ler
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu32 %[[RES]], %ign
  %cmp = icmp ule i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @testu32lerlt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32lerlt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ule i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ult i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32lerle(i32 %a, i32 %b) {
; CHECK-LABEL: testu32lerle
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ule i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ule i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32lergt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32lergt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ule i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ugt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32lerge(i32 %a, i32 %b) {
; CHECK-LABEL: testu32lerge
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ule i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp uge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define i32 @testu32gts(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gts
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu32 %[[RES]], %ign
  %cmp = icmp ugt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @testu32gtslt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gtslt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ugt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ult i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32gtsle(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gtsle
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ugt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ule i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32gtsgt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gtsgt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ugt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ugt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32gtsge(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gtsge
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ugt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp uge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define i32 @testu32ges(i32 %a, i32 %b) {
; CHECK-LABEL: testu32ges
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu32 %[[RES]], %ign
  %cmp = icmp uge i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @testu32geslt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32geslt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp uge i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ult i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32gesle(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gesle
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp uge i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ule i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32gesgt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gesgt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp uge i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ugt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32gesge(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gesge
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp uge i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp uge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

