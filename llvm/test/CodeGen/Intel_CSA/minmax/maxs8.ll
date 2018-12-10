; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i8 @tests8ltr(i8 %a, i8 %b) {
; CHECK-LABEL: tests8ltr
; CHECK: maxs8 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp slt i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  ret i8 %res
}

define {i8, i1} @tests8ltrlt(i8 %a, i8 %b) {
; CHECK-LABEL: tests8ltrlt
; CHECK: maxs8 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp slt i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp slt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8ltrle(i8 %a, i8 %b) {
; CHECK-LABEL: tests8ltrle
; CHECK: maxs8 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp slt i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sle i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8ltrgt(i8 %a, i8 %b) {
; CHECK-LABEL: tests8ltrgt
; CHECK: maxs8 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp slt i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sgt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8ltrge(i8 %a, i8 %b) {
; CHECK-LABEL: tests8ltrge
; CHECK: maxs8 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp slt i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sge i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define i8 @tests8ler(i8 %a, i8 %b) {
; CHECK-LABEL: tests8ler
; CHECK: maxs8 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp sle i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  ret i8 %res
}

define {i8, i1} @tests8lerlt(i8 %a, i8 %b) {
; CHECK-LABEL: tests8lerlt
; CHECK: maxs8 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp sle i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp slt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8lerle(i8 %a, i8 %b) {
; CHECK-LABEL: tests8lerle
; CHECK: maxs8 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp sle i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sle i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8lergt(i8 %a, i8 %b) {
; CHECK-LABEL: tests8lergt
; CHECK: maxs8 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp sle i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sgt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8lerge(i8 %a, i8 %b) {
; CHECK-LABEL: tests8lerge
; CHECK: maxs8 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp sle i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sge i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define i8 @tests8gts(i8 %a, i8 %b) {
; CHECK-LABEL: tests8gts
; CHECK: maxs8 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp sgt i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  ret i8 %res
}

define {i8, i1} @tests8gtslt(i8 %a, i8 %b) {
; CHECK-LABEL: tests8gtslt
; CHECK: maxs8 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp sgt i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp slt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8gtsle(i8 %a, i8 %b) {
; CHECK-LABEL: tests8gtsle
; CHECK: maxs8 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp sgt i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sle i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8gtsgt(i8 %a, i8 %b) {
; CHECK-LABEL: tests8gtsgt
; CHECK: maxs8 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp sgt i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sgt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8gtsge(i8 %a, i8 %b) {
; CHECK-LABEL: tests8gtsge
; CHECK: maxs8 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp sgt i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sge i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define i8 @tests8ges(i8 %a, i8 %b) {
; CHECK-LABEL: tests8ges
; CHECK: maxs8 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp sge i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  ret i8 %res
}

define {i8, i1} @tests8geslt(i8 %a, i8 %b) {
; CHECK-LABEL: tests8geslt
; CHECK: maxs8 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp sge i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp slt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8gesle(i8 %a, i8 %b) {
; CHECK-LABEL: tests8gesle
; CHECK: maxs8 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp sge i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sle i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8gesgt(i8 %a, i8 %b) {
; CHECK-LABEL: tests8gesgt
; CHECK: maxs8 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp sge i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sgt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8gesge(i8 %a, i8 %b) {
; CHECK-LABEL: tests8gesge
; CHECK: maxs8 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp sge i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sge i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

