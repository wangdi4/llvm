; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not cmp --implicit-check-not merge %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i16 @testu16ltr(i16 %a, i16 %b) {
; CHECK-LABEL: testu16ltr
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu16 %[[RES]], %ign
  %cmp = icmp ult i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  ret i16 %res
}

define {i16, i1} @testu16ltrlt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16ltrlt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ult i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ult i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16ltrle(i16 %a, i16 %b) {
; CHECK-LABEL: testu16ltrle
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ult i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ule i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16ltrgt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16ltrgt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ult i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ugt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16ltrge(i16 %a, i16 %b) {
; CHECK-LABEL: testu16ltrge
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ult i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp uge i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define i16 @testu16ler(i16 %a, i16 %b) {
; CHECK-LABEL: testu16ler
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu16 %[[RES]], %ign
  %cmp = icmp ule i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  ret i16 %res
}

define {i16, i1} @testu16lerlt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16lerlt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ule i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ult i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16lerle(i16 %a, i16 %b) {
; CHECK-LABEL: testu16lerle
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ule i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ule i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16lergt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16lergt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ule i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ugt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16lerge(i16 %a, i16 %b) {
; CHECK-LABEL: testu16lerge
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ule i16 %b, %a
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp uge i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define i16 @testu16gts(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gts
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu16 %[[RES]], %ign
  %cmp = icmp ugt i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  ret i16 %res
}

define {i16, i1} @testu16gtslt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gtslt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ugt i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ult i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16gtsle(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gtsle
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ugt i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ule i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16gtsgt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gtsgt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp ugt i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ugt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16gtsge(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gtsge
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp ugt i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp uge i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define i16 @testu16ges(i16 %a, i16 %b) {
; CHECK-LABEL: testu16ges
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu16 %[[RES]], %ign
  %cmp = icmp uge i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  ret i16 %res
}

define {i16, i1} @testu16geslt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16geslt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp uge i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ult i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16gesle(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gesle
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp uge i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ule i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16gesgt(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gesgt
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu16 %[[RES]], [[CMP:[^,]+]]
; CHECK: not1 [[NOT:[^,]+]], [[CMP]]
  %cmp = icmp uge i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp ugt i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

define {i16, i1} @testu16gesge(i16 %a, i16 %b) {
; CHECK-LABEL: testu16gesge
; CHECK: .result .lic .i16 %[[RES:[a-z0-9_.]+]]
; CHECK: maxu16 %[[RES]], [[CMP:[^,]+]]
  %cmp = icmp uge i16 %a, %b
  %res = select i1 %cmp, i16 %a, i16 %b
  %cmp2 = icmp uge i16 %a, %b
  %ret0 = insertvalue {i16, i1} undef, i16 %res, 0
  %ret1 = insertvalue {i16, i1} %ret0, i1 %cmp2, 1
  ret {i16, i1} %ret1
}

