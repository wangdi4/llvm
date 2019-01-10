; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i32 @tests32lts(i32 %a, i32 %b) {
; CHECK-LABEL: tests32lts
; CHECK: mins32 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp slt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @tests32ltslt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32ltslt
; CHECK: mins32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp slt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp slt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32ltsle(i32 %a, i32 %b) {
; CHECK-LABEL: tests32ltsle
; CHECK: mins32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp slt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sle i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32ltsgt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32ltsgt
; CHECK: mins32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp slt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sgt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32ltsge(i32 %a, i32 %b) {
; CHECK-LABEL: tests32ltsge
; CHECK: mins32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp slt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define i32 @tests32les(i32 %a, i32 %b) {
; CHECK-LABEL: tests32les
; CHECK: mins32 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp sle i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @tests32leslt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32leslt
; CHECK: mins32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp sle i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp slt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32lesle(i32 %a, i32 %b) {
; CHECK-LABEL: tests32lesle
; CHECK: mins32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp sle i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sle i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32lesgt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32lesgt
; CHECK: mins32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp sle i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sgt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32lesge(i32 %a, i32 %b) {
; CHECK-LABEL: tests32lesge
; CHECK: mins32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp sle i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define i32 @tests32gtr(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gtr
; CHECK: mins32 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp sgt i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @tests32gtrlt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gtrlt
; CHECK: mins32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp sgt i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp slt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32gtrle(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gtrle
; CHECK: mins32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp sgt i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sle i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32gtrgt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gtrgt
; CHECK: mins32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp sgt i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sgt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32gtrge(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gtrge
; CHECK: mins32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp sgt i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define i32 @tests32ger(i32 %a, i32 %b) {
; CHECK-LABEL: tests32ger
; CHECK: mins32 [[RES:[^,]+]], %ign
; CHECK: .return {{[^,]+}}, [[RES]]
  %cmp = icmp sge i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @tests32gerlt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gerlt
; CHECK: mins32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp sge i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp slt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32gerle(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gerle
; CHECK: mins32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp sge i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sle i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32gergt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gergt
; CHECK: mins32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
; CHECK: .return {{[^,]+}}, [[RES]], [[NOT]]
  %cmp = icmp sge i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sgt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32gerge(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gerge
; CHECK: mins32 [[RES:[^,]+]], [[CMP:[^,]+]]
; CHECK: .return {{[^,]+}}, [[RES]], [[CMP]]
  %cmp = icmp sge i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

