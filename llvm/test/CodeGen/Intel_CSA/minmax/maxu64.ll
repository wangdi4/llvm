; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i64 @testu64ltr(i64 %a, i64 %b) {
; CHECK-LABEL: testu64ltr
; CHECK: maxu64 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp ult i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  ret i64 %res
}

define {i64, i1} @testu64ltrlt(i64 %a, i64 %b) {
; CHECK-LABEL: testu64ltrlt
; CHECK: maxu64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp ult i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ult i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64ltrle(i64 %a, i64 %b) {
; CHECK-LABEL: testu64ltrle
; CHECK: maxu64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp ult i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ule i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64ltrgt(i64 %a, i64 %b) {
; CHECK-LABEL: testu64ltrgt
; CHECK: maxu64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp ult i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ugt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64ltrge(i64 %a, i64 %b) {
; CHECK-LABEL: testu64ltrge
; CHECK: maxu64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp ult i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp uge i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define i64 @testu64ler(i64 %a, i64 %b) {
; CHECK-LABEL: testu64ler
; CHECK: maxu64 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp ule i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  ret i64 %res
}

define {i64, i1} @testu64lerlt(i64 %a, i64 %b) {
; CHECK-LABEL: testu64lerlt
; CHECK: maxu64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp ule i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ult i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64lerle(i64 %a, i64 %b) {
; CHECK-LABEL: testu64lerle
; CHECK: maxu64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp ule i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ule i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64lergt(i64 %a, i64 %b) {
; CHECK-LABEL: testu64lergt
; CHECK: maxu64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp ule i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ugt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64lerge(i64 %a, i64 %b) {
; CHECK-LABEL: testu64lerge
; CHECK: maxu64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp ule i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp uge i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define i64 @testu64gts(i64 %a, i64 %b) {
; CHECK-LABEL: testu64gts
; CHECK: maxu64 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp ugt i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  ret i64 %res
}

define {i64, i1} @testu64gtslt(i64 %a, i64 %b) {
; CHECK-LABEL: testu64gtslt
; CHECK: maxu64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp ugt i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ult i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64gtsle(i64 %a, i64 %b) {
; CHECK-LABEL: testu64gtsle
; CHECK: maxu64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp ugt i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ule i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64gtsgt(i64 %a, i64 %b) {
; CHECK-LABEL: testu64gtsgt
; CHECK: maxu64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp ugt i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ugt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64gtsge(i64 %a, i64 %b) {
; CHECK-LABEL: testu64gtsge
; CHECK: maxu64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp ugt i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp uge i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define i64 @testu64ges(i64 %a, i64 %b) {
; CHECK-LABEL: testu64ges
; CHECK: maxu64 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp uge i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  ret i64 %res
}

define {i64, i1} @testu64geslt(i64 %a, i64 %b) {
; CHECK-LABEL: testu64geslt
; CHECK: maxu64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp uge i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ult i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64gesle(i64 %a, i64 %b) {
; CHECK-LABEL: testu64gesle
; CHECK: maxu64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp uge i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ule i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64gesgt(i64 %a, i64 %b) {
; CHECK-LABEL: testu64gesgt
; CHECK: maxu64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp uge i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ugt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64gesge(i64 %a, i64 %b) {
; CHECK-LABEL: testu64gesge
; CHECK: maxu64 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp uge i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp uge i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

