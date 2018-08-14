; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i32 @tests32ltr(i32 %a, i32 %b) {
; CHECK-LABEL: tests32ltr
; CHECK: maxs32 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp slt i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @tests32ltrlt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32ltrlt
; CHECK: maxs32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp slt i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp slt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32ltrle(i32 %a, i32 %b) {
; CHECK-LABEL: tests32ltrle
; CHECK: maxs32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp slt i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sle i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32ltrgt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32ltrgt
; CHECK: maxs32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp slt i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sgt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32ltrge(i32 %a, i32 %b) {
; CHECK-LABEL: tests32ltrge
; CHECK: maxs32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp slt i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define i32 @tests32ler(i32 %a, i32 %b) {
; CHECK-LABEL: tests32ler
; CHECK: maxs32 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp sle i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @tests32lerlt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32lerlt
; CHECK: maxs32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp sle i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp slt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32lerle(i32 %a, i32 %b) {
; CHECK-LABEL: tests32lerle
; CHECK: maxs32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp sle i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sle i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32lergt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32lergt
; CHECK: maxs32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp sle i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sgt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32lerge(i32 %a, i32 %b) {
; CHECK-LABEL: tests32lerge
; CHECK: maxs32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp sle i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define i32 @tests32gts(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gts
; CHECK: maxs32 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp sgt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @tests32gtslt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gtslt
; CHECK: maxs32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp sgt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp slt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32gtsle(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gtsle
; CHECK: maxs32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp sgt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sle i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32gtsgt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gtsgt
; CHECK: maxs32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp sgt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sgt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32gtsge(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gtsge
; CHECK: maxs32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp sgt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define i32 @tests32ges(i32 %a, i32 %b) {
; CHECK-LABEL: tests32ges
; CHECK: maxs32 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp sge i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @tests32geslt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32geslt
; CHECK: maxs32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp sge i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp slt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32gesle(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gesle
; CHECK: maxs32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp sge i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sle i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32gesgt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gesgt
; CHECK: maxs32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp sge i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sgt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32gesge(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gesge
; CHECK: maxs32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp sge i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

