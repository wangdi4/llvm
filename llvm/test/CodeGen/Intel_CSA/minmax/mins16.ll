; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i16 @tests16lts(i16 %a, i16 %b) {
; CHECK-LABEL: tests16lts
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: mins16 %[[RES]], %ign
  %cmp = icmp slt i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  ret i16 %res
}

define {i16, i1} @tests16ltslt(i16 %a, i16 %b) {
; CHECK-LABEL: tests16ltslt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: mins16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp slt i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp slt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16ltsle(i16 %a, i16 %b) {
; CHECK-LABEL: tests16ltsle
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: mins16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp slt i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sle i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16ltsgt(i16 %a, i16 %b) {
; CHECK-LABEL: tests16ltsgt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: mins16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp slt i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sgt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16ltsge(i16 %a, i16 %b) {
; CHECK-LABEL: tests16ltsge
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: mins16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp slt i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sge i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define i16 @tests16les(i16 %a, i16 %b) {
; CHECK-LABEL: tests16les
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: mins16 %[[RES]], %ign
  %cmp = icmp sle i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  ret i16 %res
}

define {i16, i1} @tests16leslt(i16 %a, i16 %b) {
; CHECK-LABEL: tests16leslt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: mins16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sle i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp slt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16lesle(i16 %a, i16 %b) {
; CHECK-LABEL: tests16lesle
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: mins16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sle i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sle i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16lesgt(i16 %a, i16 %b) {
; CHECK-LABEL: tests16lesgt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: mins16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sle i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sgt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16lesge(i16 %a, i16 %b) {
; CHECK-LABEL: tests16lesge
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: mins16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sle i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sge i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define i16 @tests16gtr(i16 %a, i16 %b) {
; CHECK-LABEL: tests16gtr
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: mins16 %[[RES]], %ign
  %cmp = icmp sgt i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  ret i16 %res
}

define {i16, i1} @tests16gtrlt(i16 %a, i16 %b) {
; CHECK-LABEL: tests16gtrlt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: mins16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sgt i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp slt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16gtrle(i16 %a, i16 %b) {
; CHECK-LABEL: tests16gtrle
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: mins16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sgt i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sle i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16gtrgt(i16 %a, i16 %b) {
; CHECK-LABEL: tests16gtrgt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: mins16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sgt i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sgt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16gtrge(i16 %a, i16 %b) {
; CHECK-LABEL: tests16gtrge
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: mins16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sgt i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sge i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define i16 @tests16ger(i16 %a, i16 %b) {
; CHECK-LABEL: tests16ger
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: mins16 %[[RES]], %ign
  %cmp = icmp sge i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  ret i16 %res
}

define {i16, i1} @tests16gerlt(i16 %a, i16 %b) {
; CHECK-LABEL: tests16gerlt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: mins16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sge i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp slt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16gerle(i16 %a, i16 %b) {
; CHECK-LABEL: tests16gerle
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: mins16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sge i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sle i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16gergt(i16 %a, i16 %b) {
; CHECK-LABEL: tests16gergt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: mins16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sge i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sgt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16gerge(i16 %a, i16 %b) {
; CHECK-LABEL: tests16gerge
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: mins16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sge i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sge i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

