; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i8 @tests8lts(i8 %a, i8 %b) {
; CHECK-LABEL: tests8lts
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: mins8 %[[RES]], %ign
  %cmp = icmp slt i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  ret i8 %res
}

define {i8, i1} @tests8ltslt(i8 %a, i8 %b) {
; CHECK-LABEL: tests8ltslt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: mins8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp slt i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp slt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8ltsle(i8 %a, i8 %b) {
; CHECK-LABEL: tests8ltsle
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: mins8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp slt i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sle i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8ltsgt(i8 %a, i8 %b) {
; CHECK-LABEL: tests8ltsgt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: mins8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp slt i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sgt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8ltsge(i8 %a, i8 %b) {
; CHECK-LABEL: tests8ltsge
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: mins8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp slt i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sge i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define i8 @tests8les(i8 %a, i8 %b) {
; CHECK-LABEL: tests8les
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: mins8 %[[RES]], %ign
  %cmp = icmp sle i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  ret i8 %res
}

define {i8, i1} @tests8leslt(i8 %a, i8 %b) {
; CHECK-LABEL: tests8leslt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: mins8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sle i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp slt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8lesle(i8 %a, i8 %b) {
; CHECK-LABEL: tests8lesle
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: mins8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sle i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sle i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8lesgt(i8 %a, i8 %b) {
; CHECK-LABEL: tests8lesgt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: mins8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sle i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sgt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8lesge(i8 %a, i8 %b) {
; CHECK-LABEL: tests8lesge
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: mins8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sle i8 %a, %b
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sge i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define i8 @tests8gtr(i8 %a, i8 %b) {
; CHECK-LABEL: tests8gtr
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: mins8 %[[RES]], %ign
  %cmp = icmp sgt i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  ret i8 %res
}

define {i8, i1} @tests8gtrlt(i8 %a, i8 %b) {
; CHECK-LABEL: tests8gtrlt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: mins8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sgt i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp slt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8gtrle(i8 %a, i8 %b) {
; CHECK-LABEL: tests8gtrle
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: mins8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sgt i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sle i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8gtrgt(i8 %a, i8 %b) {
; CHECK-LABEL: tests8gtrgt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: mins8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sgt i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sgt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8gtrge(i8 %a, i8 %b) {
; CHECK-LABEL: tests8gtrge
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: mins8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sgt i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sge i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define i8 @tests8ger(i8 %a, i8 %b) {
; CHECK-LABEL: tests8ger
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: mins8 %[[RES]], %ign
  %cmp = icmp sge i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  ret i8 %res
}

define {i8, i1} @tests8gerlt(i8 %a, i8 %b) {
; CHECK-LABEL: tests8gerlt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: mins8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sge i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp slt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8gerle(i8 %a, i8 %b) {
; CHECK-LABEL: tests8gerle
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: mins8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sge i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sle i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8gergt(i8 %a, i8 %b) {
; CHECK-LABEL: tests8gergt
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: mins8 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sge i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sgt i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

define {i8, i1} @tests8gerge(i8 %a, i8 %b) {
; CHECK-LABEL: tests8gerge
; CHECK: .result .lic .i8 %[[RES:[a-z0-9_.]+]]
; CHECK: mins8 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sge i8 %b, %a
  %res = select i1 %cmp, i8 %a, i8 %b
  %cmp2 = icmp sge i8 %a, %b
  %ret0 = insertvalue {i8, i1} undef, i8 %res, 0
  %ret1 = insertvalue {i8, i1} %ret0, i1 %cmp2, 1
  ret {i8, i1} %ret1
}

