; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i64 @testu64lts(i64 %a, i64 %b) {
; CHECK-LABEL: testu64lts
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: minu64 %[[RES]], %ign
  %cmp = icmp ult i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  ret i64 %res
}

define {i64, i1} @testu64ltslt(i64 %a, i64 %b) {
; CHECK-LABEL: testu64ltslt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: minu64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ult i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ult i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64ltsle(i64 %a, i64 %b) {
; CHECK-LABEL: testu64ltsle
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: minu64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ult i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ule i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64ltsgt(i64 %a, i64 %b) {
; CHECK-LABEL: testu64ltsgt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: minu64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ult i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ugt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64ltsge(i64 %a, i64 %b) {
; CHECK-LABEL: testu64ltsge
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: minu64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ult i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp uge i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define i64 @testu64les(i64 %a, i64 %b) {
; CHECK-LABEL: testu64les
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: minu64 %[[RES]], %ign
  %cmp = icmp ule i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  ret i64 %res
}

define {i64, i1} @testu64leslt(i64 %a, i64 %b) {
; CHECK-LABEL: testu64leslt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: minu64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ule i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ult i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64lesle(i64 %a, i64 %b) {
; CHECK-LABEL: testu64lesle
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: minu64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ule i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ule i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64lesgt(i64 %a, i64 %b) {
; CHECK-LABEL: testu64lesgt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: minu64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ule i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ugt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64lesge(i64 %a, i64 %b) {
; CHECK-LABEL: testu64lesge
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: minu64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ule i64 %a, %b
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp uge i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define i64 @testu64gtr(i64 %a, i64 %b) {
; CHECK-LABEL: testu64gtr
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: minu64 %[[RES]], %ign
  %cmp = icmp ugt i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  ret i64 %res
}

define {i64, i1} @testu64gtrlt(i64 %a, i64 %b) {
; CHECK-LABEL: testu64gtrlt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: minu64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ugt i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ult i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64gtrle(i64 %a, i64 %b) {
; CHECK-LABEL: testu64gtrle
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: minu64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ugt i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ule i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64gtrgt(i64 %a, i64 %b) {
; CHECK-LABEL: testu64gtrgt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: minu64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ugt i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ugt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64gtrge(i64 %a, i64 %b) {
; CHECK-LABEL: testu64gtrge
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: minu64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ugt i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp uge i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define i64 @testu64ger(i64 %a, i64 %b) {
; CHECK-LABEL: testu64ger
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: minu64 %[[RES]], %ign
  %cmp = icmp uge i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  ret i64 %res
}

define {i64, i1} @testu64gerlt(i64 %a, i64 %b) {
; CHECK-LABEL: testu64gerlt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: minu64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp uge i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ult i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64gerle(i64 %a, i64 %b) {
; CHECK-LABEL: testu64gerle
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: minu64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp uge i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ule i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64gergt(i64 %a, i64 %b) {
; CHECK-LABEL: testu64gergt
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: minu64 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp uge i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp ugt i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

define {i64, i1} @testu64gerge(i64 %a, i64 %b) {
; CHECK-LABEL: testu64gerge
; CHECK: .result .lic .i64 %[[RES:[a-z0-9_.]+]]
; CHECK: minu64 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp uge i64 %b, %a
  %res = select i1 %cmp, i64 %a, i64 %b
  %cmp2 = icmp uge i64 %a, %b
  %ret0 = insertvalue {i64, i1} undef, i64 %res, 0
  %ret1 = insertvalue {i64, i1} %ret0, i1 %cmp2, 1
  ret {i64, i1} %ret1
}

