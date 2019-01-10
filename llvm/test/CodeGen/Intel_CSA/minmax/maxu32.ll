; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i32 @testu32ltr(i32 %a, i32 %b) {
; CHECK-LABEL: testu32ltr
; CHECK: maxu32 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp ult i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @testu32ltrlt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32ltrlt
; CHECK: maxu32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp ult i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ult i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32ltrle(i32 %a, i32 %b) {
; CHECK-LABEL: testu32ltrle
; CHECK: maxu32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp ult i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ule i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32ltrgt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32ltrgt
; CHECK: maxu32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp ult i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ugt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32ltrge(i32 %a, i32 %b) {
; CHECK-LABEL: testu32ltrge
; CHECK: maxu32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp ult i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp uge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define i32 @testu32ler(i32 %a, i32 %b) {
; CHECK-LABEL: testu32ler
; CHECK: maxu32 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp ule i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @testu32lerlt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32lerlt
; CHECK: maxu32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp ule i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ult i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32lerle(i32 %a, i32 %b) {
; CHECK-LABEL: testu32lerle
; CHECK: maxu32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp ule i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ule i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32lergt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32lergt
; CHECK: maxu32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp ule i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ugt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32lerge(i32 %a, i32 %b) {
; CHECK-LABEL: testu32lerge
; CHECK: maxu32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp ule i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp uge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define i32 @testu32gts(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gts
; CHECK: maxu32 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp ugt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @testu32gtslt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gtslt
; CHECK: maxu32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp ugt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ult i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32gtsle(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gtsle
; CHECK: maxu32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp ugt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ule i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32gtsgt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gtsgt
; CHECK: maxu32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp ugt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ugt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32gtsge(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gtsge
; CHECK: maxu32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp ugt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp uge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define i32 @testu32ges(i32 %a, i32 %b) {
; CHECK-LABEL: testu32ges
; CHECK: maxu32 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp uge i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @testu32geslt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32geslt
; CHECK: maxu32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp uge i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ult i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32gesle(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gesle
; CHECK: maxu32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp uge i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ule i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32gesgt(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gesgt
; CHECK: maxu32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp uge i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp ugt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @testu32gesge(i32 %a, i32 %b) {
; CHECK-LABEL: testu32gesge
; CHECK: maxu32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp uge i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp uge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

