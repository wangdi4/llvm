; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i16 @tests16ltr(i16 %a, i16 %b) {
; CHECK-LABEL: tests16ltr
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs16 %[[RES]], %ign
  %cmp = icmp slt i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  ret i16 %res
}

define {i16, i1} @tests16ltrlt(i16 %a, i16 %b) {
; CHECK-LABEL: tests16ltrlt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp slt i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp slt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16ltrle(i16 %a, i16 %b) {
; CHECK-LABEL: tests16ltrle
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp slt i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sle i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16ltrgt(i16 %a, i16 %b) {
; CHECK-LABEL: tests16ltrgt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp slt i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sgt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16ltrge(i16 %a, i16 %b) {
; CHECK-LABEL: tests16ltrge
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp slt i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sge i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define i16 @tests16ler(i16 %a, i16 %b) {
; CHECK-LABEL: tests16ler
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs16 %[[RES]], %ign
  %cmp = icmp sle i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  ret i16 %res
}

define {i16, i1} @tests16lerlt(i16 %a, i16 %b) {
; CHECK-LABEL: tests16lerlt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sle i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp slt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16lerle(i16 %a, i16 %b) {
; CHECK-LABEL: tests16lerle
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sle i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sle i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16lergt(i16 %a, i16 %b) {
; CHECK-LABEL: tests16lergt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sle i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sgt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16lerge(i16 %a, i16 %b) {
; CHECK-LABEL: tests16lerge
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sle i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sge i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define i16 @tests16gts(i16 %a, i16 %b) {
; CHECK-LABEL: tests16gts
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs16 %[[RES]], %ign
  %cmp = icmp sgt i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  ret i16 %res
}

define {i16, i1} @tests16gtslt(i16 %a, i16 %b) {
; CHECK-LABEL: tests16gtslt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sgt i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp slt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16gtsle(i16 %a, i16 %b) {
; CHECK-LABEL: tests16gtsle
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sgt i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sle i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16gtsgt(i16 %a, i16 %b) {
; CHECK-LABEL: tests16gtsgt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sgt i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sgt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16gtsge(i16 %a, i16 %b) {
; CHECK-LABEL: tests16gtsge
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sgt i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sge i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define i16 @tests16ges(i16 %a, i16 %b) {
; CHECK-LABEL: tests16ges
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs16 %[[RES]], %ign
  %cmp = icmp sge i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  ret i16 %res
}

define {i16, i1} @tests16geslt(i16 %a, i16 %b) {
; CHECK-LABEL: tests16geslt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sge i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp slt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16gesle(i16 %a, i16 %b) {
; CHECK-LABEL: tests16gesle
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sge i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sle i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16gesgt(i16 %a, i16 %b) {
; CHECK-LABEL: tests16gesgt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sge i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sgt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @tests16gesge(i16 %a, i16 %b) {
; CHECK-LABEL: tests16gesge
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sge i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp sge i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

