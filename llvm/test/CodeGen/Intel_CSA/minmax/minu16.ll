; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i16 @testu16lts(i16 %a, i16 %b) {
; CHECK-LABEL: testu16lts
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: minu16 %[[RES]], %ign
  %cmp = icmp ult i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  ret i16 %res
}

define {i16, i1} @testu16ltslt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16ltslt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: minu16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ult i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ult i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16ltsle(i16 %a, i16 %b) {
; CHECK-LABEL: testu16ltsle
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: minu16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ult i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ule i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16ltsgt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16ltsgt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: minu16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ult i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ugt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16ltsge(i16 %a, i16 %b) {
; CHECK-LABEL: testu16ltsge
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: minu16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ult i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp uge i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define i16 @testu16les(i16 %a, i16 %b) {
; CHECK-LABEL: testu16les
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: minu16 %[[RES]], %ign
  %cmp = icmp ule i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  ret i16 %res
}

define {i16, i1} @testu16leslt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16leslt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: minu16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ule i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ult i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16lesle(i16 %a, i16 %b) {
; CHECK-LABEL: testu16lesle
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: minu16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ule i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ule i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16lesgt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16lesgt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: minu16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ule i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ugt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16lesge(i16 %a, i16 %b) {
; CHECK-LABEL: testu16lesge
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: minu16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ule i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp uge i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define i16 @testu16gtr(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gtr
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: minu16 %[[RES]], %ign
  %cmp = icmp ugt i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  ret i16 %res
}

define {i16, i1} @testu16gtrlt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gtrlt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: minu16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ugt i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ult i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16gtrle(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gtrle
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: minu16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ugt i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ule i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16gtrgt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gtrgt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: minu16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ugt i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ugt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16gtrge(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gtrge
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: minu16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ugt i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp uge i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define i16 @testu16ger(i16 %a, i16 %b) {
; CHECK-LABEL: testu16ger
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: minu16 %[[RES]], %ign
  %cmp = icmp uge i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  ret i16 %res
}

define {i16, i1} @testu16gerlt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gerlt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: minu16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp uge i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ult i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16gerle(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gerle
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: minu16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp uge i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ule i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16gergt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gergt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: minu16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp uge i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ugt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16gerge(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gerge
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: minu16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp uge i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp uge i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

