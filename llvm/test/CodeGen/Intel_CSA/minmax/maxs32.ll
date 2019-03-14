; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i32 @tests32ltr(i32 %a, i32 %b) {
; CHECK-LABEL: tests32ltr
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs32 %[[RES]], %ign
  %cmp = icmp slt i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @tests32ltrlt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32ltrlt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp slt i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp slt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32ltrle(i32 %a, i32 %b) {
; CHECK-LABEL: tests32ltrle
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp slt i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sle i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32ltrgt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32ltrgt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp slt i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sgt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32ltrge(i32 %a, i32 %b) {
; CHECK-LABEL: tests32ltrge
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp slt i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define i32 @tests32ler(i32 %a, i32 %b) {
; CHECK-LABEL: tests32ler
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs32 %[[RES]], %ign
  %cmp = icmp sle i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @tests32lerlt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32lerlt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sle i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp slt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32lerle(i32 %a, i32 %b) {
; CHECK-LABEL: tests32lerle
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sle i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sle i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32lergt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32lergt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sle i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sgt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32lerge(i32 %a, i32 %b) {
; CHECK-LABEL: tests32lerge
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sle i32 %b, %a
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define i32 @tests32gts(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gts
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs32 %[[RES]], %ign
  %cmp = icmp sgt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @tests32gtslt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gtslt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sgt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp slt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32gtsle(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gtsle
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sgt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sle i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32gtsgt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gtsgt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sgt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sgt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32gtsge(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gtsge
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sgt i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define i32 @tests32ges(i32 %a, i32 %b) {
; CHECK-LABEL: tests32ges
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs32 %[[RES]], %ign
  %cmp = icmp sge i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  ret i32 %res
}

define {i32, i1} @tests32geslt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32geslt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sge i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp slt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32gesle(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gesle
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sge i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sle i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32gesgt(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gesgt
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs32 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sge i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sgt i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

define {i32, i1} @tests32gesge(i32 %a, i32 %b) {
; CHECK-LABEL: tests32gesge
; CHECK: .result .lic .i32 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs32 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sge i32 %a, %b
  %res = select i1 %cmp, i32 %a, i32 %b
  %cmp2 = icmp sge i32 %a, %b
  %ret0 = insertvalue {i32, i1} undef, i32 %res, 0
  %ret1 = insertvalue {i32, i1} %ret0, i1 %cmp2, 1
  ret {i32, i1} %ret1
}

