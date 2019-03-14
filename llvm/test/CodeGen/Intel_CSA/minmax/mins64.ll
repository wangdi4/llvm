; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i64 @tests64lts(i64 %a, i64 %b) {
; CHECK-LABEL: tests64lts
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: mins64 %[[RES]], %ign
  %cmp = icmp slt i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  ret i64 %res
}

define {i64, i1} @tests64ltslt(i64 %a, i64 %b) {
; CHECK-LABEL: tests64ltslt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: mins64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp slt i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp slt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64ltsle(i64 %a, i64 %b) {
; CHECK-LABEL: tests64ltsle
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: mins64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp slt i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sle i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64ltsgt(i64 %a, i64 %b) {
; CHECK-LABEL: tests64ltsgt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: mins64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp slt i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sgt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64ltsge(i64 %a, i64 %b) {
; CHECK-LABEL: tests64ltsge
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: mins64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp slt i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sge i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define i64 @tests64les(i64 %a, i64 %b) {
; CHECK-LABEL: tests64les
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: mins64 %[[RES]], %ign
  %cmp = icmp sle i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  ret i64 %res
}

define {i64, i1} @tests64leslt(i64 %a, i64 %b) {
; CHECK-LABEL: tests64leslt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: mins64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sle i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp slt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64lesle(i64 %a, i64 %b) {
; CHECK-LABEL: tests64lesle
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: mins64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sle i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sle i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64lesgt(i64 %a, i64 %b) {
; CHECK-LABEL: tests64lesgt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: mins64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sle i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sgt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64lesge(i64 %a, i64 %b) {
; CHECK-LABEL: tests64lesge
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: mins64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sle i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sge i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define i64 @tests64gtr(i64 %a, i64 %b) {
; CHECK-LABEL: tests64gtr
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: mins64 %[[RES]], %ign
  %cmp = icmp sgt i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  ret i64 %res
}

define {i64, i1} @tests64gtrlt(i64 %a, i64 %b) {
; CHECK-LABEL: tests64gtrlt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: mins64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sgt i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp slt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64gtrle(i64 %a, i64 %b) {
; CHECK-LABEL: tests64gtrle
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: mins64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sgt i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sle i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64gtrgt(i64 %a, i64 %b) {
; CHECK-LABEL: tests64gtrgt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: mins64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sgt i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sgt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64gtrge(i64 %a, i64 %b) {
; CHECK-LABEL: tests64gtrge
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: mins64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sgt i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sge i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define i64 @tests64ger(i64 %a, i64 %b) {
; CHECK-LABEL: tests64ger
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: mins64 %[[RES]], %ign
  %cmp = icmp sge i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  ret i64 %res
}

define {i64, i1} @tests64gerlt(i64 %a, i64 %b) {
; CHECK-LABEL: tests64gerlt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: mins64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sge i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp slt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64gerle(i64 %a, i64 %b) {
; CHECK-LABEL: tests64gerle
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: mins64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sge i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sle i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64gergt(i64 %a, i64 %b) {
; CHECK-LABEL: tests64gergt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: mins64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp sge i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sgt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @tests64gerge(i64 %a, i64 %b) {
; CHECK-LABEL: tests64gerge
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: mins64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp sge i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp sge i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

