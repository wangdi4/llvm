; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i64 @tests64ltr(i64 %a, i64 %b) {
; CHECK-LABEL: tests64ltr
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs64 %[[RES]], %ign
  %cmp = icmp slt i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  ret i64 %res
}

define {i64, i1} @tests64ltrlt(i64 %a, i64 %b) {
; CHECK-LABEL: tests64ltrlt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp slt i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp slt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64ltrle(i64 %a, i64 %b) {
; CHECK-LABEL: tests64ltrle
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp slt i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sle i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64ltrgt(i64 %a, i64 %b) {
; CHECK-LABEL: tests64ltrgt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp slt i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sgt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64ltrge(i64 %a, i64 %b) {
; CHECK-LABEL: tests64ltrge
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp slt i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sge i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define i64 @tests64ler(i64 %a, i64 %b) {
; CHECK-LABEL: tests64ler
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs64 %[[RES]], %ign
  %cmp = icmp sle i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  ret i64 %res
}

define {i64, i1} @tests64lerlt(i64 %a, i64 %b) {
; CHECK-LABEL: tests64lerlt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sle i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp slt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64lerle(i64 %a, i64 %b) {
; CHECK-LABEL: tests64lerle
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sle i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sle i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64lergt(i64 %a, i64 %b) {
; CHECK-LABEL: tests64lergt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sle i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sgt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64lerge(i64 %a, i64 %b) {
; CHECK-LABEL: tests64lerge
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sle i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sge i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define i64 @tests64gts(i64 %a, i64 %b) {
; CHECK-LABEL: tests64gts
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs64 %[[RES]], %ign
  %cmp = icmp sgt i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  ret i64 %res
}

define {i64, i1} @tests64gtslt(i64 %a, i64 %b) {
; CHECK-LABEL: tests64gtslt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sgt i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp slt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64gtsle(i64 %a, i64 %b) {
; CHECK-LABEL: tests64gtsle
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sgt i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sle i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64gtsgt(i64 %a, i64 %b) {
; CHECK-LABEL: tests64gtsgt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sgt i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sgt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64gtsge(i64 %a, i64 %b) {
; CHECK-LABEL: tests64gtsge
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sgt i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sge i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define i64 @tests64ges(i64 %a, i64 %b) {
; CHECK-LABEL: tests64ges
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs64 %[[RES]], %ign
  %cmp = icmp sge i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  ret i64 %res
}

define {i64, i1} @tests64geslt(i64 %a, i64 %b) {
; CHECK-LABEL: tests64geslt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sge i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp slt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64gesle(i64 %a, i64 %b) {
; CHECK-LABEL: tests64gesle
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sge i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sle i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64gesgt(i64 %a, i64 %b) {
; CHECK-LABEL: tests64gesgt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sge i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sgt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64gesge(i64 %a, i64 %b) {
; CHECK-LABEL: tests64gesge
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: maxs64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sge i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sge i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

