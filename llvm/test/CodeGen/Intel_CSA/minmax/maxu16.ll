; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i16 @testu16ltr(i16 %a, i16 %b) {
; CHECK-LABEL: testu16ltr
; CHECK: maxu16 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp ult i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  ret i16 %res
}

define {i16, i1} @testu16ltrlt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16ltrlt
; CHECK: maxu16 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp ult i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ult i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16ltrle(i16 %a, i16 %b) {
; CHECK-LABEL: testu16ltrle
; CHECK: maxu16 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp ult i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ule i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16ltrgt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16ltrgt
; CHECK: maxu16 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp ult i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ugt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16ltrge(i16 %a, i16 %b) {
; CHECK-LABEL: testu16ltrge
; CHECK: maxu16 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp ult i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp uge i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define i16 @testu16ler(i16 %a, i16 %b) {
; CHECK-LABEL: testu16ler
; CHECK: maxu16 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp ule i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  ret i16 %res
}

define {i16, i1} @testu16lerlt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16lerlt
; CHECK: maxu16 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp ule i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ult i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16lerle(i16 %a, i16 %b) {
; CHECK-LABEL: testu16lerle
; CHECK: maxu16 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp ule i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ule i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16lergt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16lergt
; CHECK: maxu16 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp ule i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ugt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16lerge(i16 %a, i16 %b) {
; CHECK-LABEL: testu16lerge
; CHECK: maxu16 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp ule i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp uge i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define i16 @testu16gts(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gts
; CHECK: maxu16 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp ugt i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  ret i16 %res
}

define {i16, i1} @testu16gtslt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gtslt
; CHECK: maxu16 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp ugt i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ult i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16gtsle(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gtsle
; CHECK: maxu16 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp ugt i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ule i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16gtsgt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gtsgt
; CHECK: maxu16 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp ugt i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ugt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16gtsge(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gtsge
; CHECK: maxu16 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp ugt i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp uge i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define i16 @testu16ges(i16 %a, i16 %b) {
; CHECK-LABEL: testu16ges
; CHECK: maxu16 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp uge i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  ret i16 %res
}

define {i16, i1} @testu16geslt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16geslt
; CHECK: maxu16 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp uge i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ult i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16gesle(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gesle
; CHECK: maxu16 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp uge i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ule i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16gesgt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gesgt
; CHECK: maxu16 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp uge i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ugt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16gesge(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gesge
; CHECK: maxu16 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp uge i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp uge i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

