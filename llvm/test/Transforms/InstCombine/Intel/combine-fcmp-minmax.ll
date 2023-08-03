; RUN: opt -passes="instcombine" < %s -S | FileCheck %s

; Both fcmp instructions should have nnan flag set
define i1 @no_convert1 (float %a, float %b, float %c) {
; CHECK-LABEL: @no_convert1(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan olt float %c, %a
; CHECK-NEXT:    [[CMP1:%.*]] = fcmp olt float %c, %b
; CHECK-NEXT:    [[TMP1:%.*]] = or i1 [[CMP]], [[CMP1]]
entry:
  %cmp = fcmp nnan olt float %c, %a
  %cmp1 = fcmp olt float %c, %b
  %0 = or i1 %cmp, %cmp1
  ret i1 %0
}

; Both fcmp instructions should have nnan flag set
define i1 @no_convert2 (float %a, float %b, float %c)  {
; CHECK-LABEL: @no_convert2(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp olt float %c, %a
; CHECK-NEXT:    [[CMP1:%.*]] = fcmp nnan olt float %c, %b
; CHECK-NEXT:    [[TMP1:%.*]] = or i1 [[CMP]], [[CMP1]]
entry:
  %cmp = fcmp olt float %c, %a
  %cmp1 = fcmp nnan olt float %c, %b
  %0 = or i1 %cmp, %cmp1
  ret i1 %0
}

; FCmp instructions should have common operands
define i1 @no_convert3 (float %a, float %b, float %c, float %d)  {
; CHECK-LABEL: @no_convert3(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge float %d, %a
; CHECK-NEXT:    [[CMP1:%.*]] = fcmp nnan oge float %c, %b
; CHECK-NEXT:    [[TMP1:%.*]] = and i1 [[CMP1]], [[CMP]]
entry:
  %cmp = fcmp nnan oge float %d, %a
  %cmp1 = fcmp nnan oge float %c, %b
  %0 = and i1 %cmp1, %cmp
  ret i1 %0
}

; FCmp instructions should have single common use
define i1 @no_convert4 (float %a, float %b, float %c, ptr %d)  {
; CHECK-LABEL: @no_convert4(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan olt float %c, %a
; CHECK-NEXT:    [[TMP1:%.*]] = zext i1 [[CMP]] to i8
; CHECK-NEXT:    [[CMP1:%.*]] = fcmp nnan olt float %c, %b
; CHECK-NEXT:    store i8 [[TMP1]], ptr %d
; CHECK-NEXT:    [[TMP2:%.*]] = or i1 [[CMP]], [[CMP1]]
entry:
  %cmp = fcmp nnan olt float %c, %a
  %frombool = zext i1 %cmp to i8
  %cmp3 = fcmp nnan olt float %c, %b
  store i8 %frombool, ptr %d, align 1
  %0 = or i1 %cmp, %cmp3
  ret i1 %0
}

; FCmp instructions should have identical or swapped predicates
; (c < a || c <= b) is skipped
define i1 @no_convert5 (float %a, float %b, float %c)  {
; CHECK-LABEL: @no_convert5(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan olt float %c, %a
; CHECK-NEXT:    [[CMP1:%.*]] = fcmp nnan ole float %c, %b
; CHECK-NEXT:    [[TMP1:%.*]] = and i1 [[CMP1]], [[CMP]]
entry:
  %cmp = fcmp nnan olt float %c, %a
  %cmp1 = fcmp nnan ole float %c, %b
  %0 = and i1 %cmp1, %cmp
  ret i1 %0
}

; *************************
; Common operand on the LHS
; *************************

; (c < a  || c < b ) => c < max(a,b)
define i1 @convert1 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert1(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ogt float [[TMP1]], %c
entry:
  %cmp = fcmp nnan olt float %c, %a
  %cmp1 = fcmp nnan olt float %c, %b
  %0 = or i1 %cmp, %cmp1
  ret i1 %0
}

; (c < a  && c < b ) => c < min(a,b)
define i1 @convert2 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert2(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ole float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ogt float [[TMP1]], %c
entry:
  %cmp = fcmp nnan olt float %c, %a
  %cmp1 = fcmp nnan olt float %c, %b
  %0 = and i1 %cmp, %cmp1
  ret i1 %0
}

; (c <= a || c <= b) => c <= max(a,b)
define i1 @convert3 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert3(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge float %b, %a
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %b, float %a
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan oge float [[TMP1]], %c
entry:
  %cmp = fcmp nnan ole float %c, %a
  %cmp1 = fcmp nnan ole float %c, %b
  %0 = or i1 %cmp1, %cmp
  ret i1 %0
}

; (c <= a && c <= b) => c <= min(a,b)
define i1 @convert4 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert4(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ole float %b, %a
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %b, float %a
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan oge float [[TMP1]], %c
entry:
  %cmp = fcmp nnan ole float %c, %a
  %cmp1 = fcmp nnan ole float %c, %b
  %0 = and i1 %cmp1, %cmp
  ret i1 %0
}

; (c > a  || c > b ) => c > min(a,b)
define i1 @convert5 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert5(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ole float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan olt float [[TMP1]], %c
entry:
  %cmp = fcmp nnan ogt float %c, %a
  %cmp1 = fcmp nnan ogt float %c, %b
  %0 = or i1 %cmp, %cmp1
  ret i1 %0
}

; (c > a  && c > b ) => c > max(a,b)
define i1 @convert6 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert6(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan olt float [[TMP1]], %c
entry:
  %cmp = fcmp nnan ogt float %c, %a
  %cmp1 = fcmp nnan ogt float %c, %b
  %0 = and i1 %cmp, %cmp1
  ret i1 %0
}

; (c >= a || c >= b) => c >= min(a,b)
define i1 @convert7 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert7(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ole float %b, %a
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %b, float %a
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ole float [[TMP1]], %c
entry:
  %cmp = fcmp nnan oge float %c, %a
  %cmp1 = fcmp nnan oge float %c, %b
  %0 = or i1 %cmp1, %cmp
  ret i1 %0
}

; (c >= a && c >= b) => c >= max(a,b)
define i1 @convert8 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert8(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge float %b, %a
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %b, float %a
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ole float [[TMP1]], %c
entry:
  %cmp = fcmp nnan oge float %c, %a
  %cmp1 = fcmp nnan oge float %c, %b
  %0 = and i1 %cmp1, %cmp
  ret i1 %0
}

; *************************
; Common operand on the RHS
; *************************

; (a > c  || b >  c )  => max(a,b) >  c
define i1 @convert21(float %a, float %b, float %c)  {
; CHECK-LABEL: @convert21(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ogt float [[TMP1]], %c
entry:
  %cmp = fcmp nnan ogt float %a, %c
  %cmp1 = fcmp nnan ogt float %b, %c
  %0 = or i1 %cmp, %cmp1
  ret i1 %0
}

; (a > c  && b >  c )  => min(a,b) >  c
define i1 @convert22(float %a, float %b, float %c)  {
; CHECK-LABEL: @convert22(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ole float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ogt float [[TMP1]], %c
entry:
  %cmp = fcmp nnan ogt float %a, %c
  %cmp1 = fcmp nnan ogt float %b, %c
  %0 = and i1 %cmp, %cmp1
  ret i1 %0
}

; (a >= c || b >= c )  => max(a,b) >=  c
define i1 @convert23(float %a, float %b, float %c)  {
; CHECK-LABEL: @convert23(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge float %b, %a
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %b, float %a
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan oge float [[TMP1]], %c
entry:
  %cmp = fcmp nnan oge float %a, %c
  %cmp1 = fcmp nnan oge float %b, %c
  %0 = or i1 %cmp1, %cmp
  ret i1 %0
}

; (a >= c && b >= c )  => min(a,b) >=  c
define i1 @convert24(float %a, float %b, float %c)  {
; CHECK-LABEL: @convert24(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ole float %b, %a
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %b, float %a
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan oge float [[TMP1]], %c
entry:
  %cmp = fcmp nnan oge float %a, %c
  %cmp1 = fcmp nnan oge float %b, %c
  %0 = and i1 %cmp1, %cmp
  ret i1 %0
}

; (a < c  || b <  c ) => min(a,b) < c
define i1 @convert25(float %a, float %b, float %c)  {
; CHECK-LABEL: @convert25(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ole float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan olt float [[TMP1]], %c
entry:
  %cmp = fcmp nnan olt float %a, %c
  %cmp1 = fcmp nnan olt float %b, %c
  %0 = or i1 %cmp, %cmp1
  ret i1 %0
}

; (a < c  && b <  c ) => max(a,b) < c
define i1 @convert26(float %a, float %b, float %c)  {
; CHECK-LABEL: @convert26(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan olt float [[TMP1]], %c
entry:
  %cmp = fcmp nnan olt float %a, %c
  %cmp1 = fcmp nnan olt float %b, %c
  %0 = and i1 %cmp, %cmp1
  ret i1 %0
}

; (a <= c || b <= c ) => min(a,b) <= c
define i1 @convert27(float %a, float %b, float %c)  {
; CHECK-LABEL: @convert27(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ole float %b, %a
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %b, float %a
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ole float [[TMP1]], %c
entry:
  %cmp = fcmp nnan ole float %a, %c
  %cmp1 = fcmp nnan ole float %b, %c
  %0 = or i1 %cmp1, %cmp
  ret i1 %0
}

; (a <= c && b <= c ) => max(a,b) <= c
define i1 @convert28(float %a, float %b, float %c)  {
; CHECK-LABEL: @convert28(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge float %b, %a
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %b, float %a
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ole float [[TMP1]], %c
entry:
  %cmp = fcmp nnan ole float %a, %c
  %cmp1 = fcmp nnan ole float %b, %c
  %0 = and i1 %cmp1, %cmp
  ret i1 %0
}

; *************************
; Common operand mixed LRHS
; *************************

; (c < a  || b > c ) => (c < a  || c < b ) => c < max(a,b)
define i1 @convert31 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert31(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ogt float [[TMP1]], %c
entry:
  %cmp = fcmp nnan olt float %c, %a
  %cmp1 = fcmp nnan ogt float %b, %c
  %0 = or i1 %cmp, %cmp1
  ret i1 %0
}

; (c < a  && b > c ) => (c < a  && c < b ) => c < min(a,b)
define i1 @convert32 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert32(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ole float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ogt float [[TMP1]], %c
entry:
  %cmp = fcmp nnan olt float %c, %a
  %cmp1 = fcmp nnan ogt float %b, %c
  %0 = and i1 %cmp, %cmp1
  ret i1 %0
}

; (c <= a || b => c) => (c <= a || c <= b) => c <= max(a,b)
define i1 @convert33 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert33(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge float %b, %a
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %b, float %a
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan oge float [[TMP1]], %c
entry:
  %cmp = fcmp nnan ole float %c, %a
  %cmp1 = fcmp nnan oge float %b, %c
  %0 = or i1 %cmp1, %cmp
  ret i1 %0
}

; (c <= a && b => c) => (c <= a && c <= b) => c <= min(a,b)
define i1 @convert34 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert34(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ole float %b, %a
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %b, float %a
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan oge float [[TMP1]], %c
entry:
  %cmp = fcmp nnan ole float %c, %a
  %cmp1 = fcmp nnan oge float %b, %c
  %0 = and i1 %cmp1, %cmp
  ret i1 %0
}

; (c > a  || b < c ) => (c > a  || c > b ) => c > min(a,b)
define i1 @convert35 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert35(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ole float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan olt float [[TMP1]], %c
entry:
  %cmp = fcmp nnan ogt float %c, %a
  %cmp1 = fcmp nnan olt float %b, %c
  %0 = or i1 %cmp, %cmp1
  ret i1 %0
}

; (c > a  && b < c ) => (c > a  && c > b ) => c > max(a,b)
define i1 @convert36 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert36(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan olt float [[TMP1]], %c
entry:
  %cmp = fcmp nnan ogt float %c, %a
  %cmp1 = fcmp nnan olt float %b, %c
  %0 = and i1 %cmp, %cmp1
  ret i1 %0
}

; (c >= a || b <= c) => (c >= a || c >= b) => c >= min(a,b)
define i1 @convert37 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert37(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ole float %b, %a
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %b, float %a
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ole float [[TMP1]], %c
entry:
  %cmp = fcmp nnan oge float %c, %a
  %cmp1 = fcmp nnan ole float %b, %c
  %0 = or i1 %cmp1, %cmp
  ret i1 %0
}

; (c >= a && b <= c) => (c >= a && c >= b) => c >= max(a,b)
define i1 @convert38 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert38(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge float %b, %a
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %b, float %a
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ole float [[TMP1]], %c
entry:
  %cmp = fcmp nnan oge float %c, %a
  %cmp1 = fcmp nnan ole float %b, %c
  %0 = and i1 %cmp1, %cmp
  ret i1 %0
}

; *************************
; Common operand mixed RLHS
; *************************

; (a > c  || c < b ) => (c < a  || c < b ) => c < max(a,b)
define i1 @convert41 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert41(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ogt float [[TMP1]], %c
entry:
  %cmp = fcmp nnan ogt float %a, %c
  %cmp1 = fcmp nnan olt float %c, %b
  %0 = or i1 %cmp, %cmp1
  ret i1 %0
}

; (a > c  && c < b ) => (c < a  && c < b ) => c < min(a,b)
define i1 @convert42 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert42(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ole float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ogt float [[TMP1]], %c
entry:
  %cmp = fcmp nnan ogt float %a, %c
  %cmp1 = fcmp nnan olt float %c, %b
  %0 = and i1 %cmp, %cmp1
  ret i1 %0
}

; (a >= c || c <= b) => (c <= a || c <= b) => c <= max(a,b)
define i1 @convert43 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert43(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge float %b, %a
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %b, float %a
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan oge float [[TMP1]], %c
entry:
  %cmp = fcmp nnan oge float %a, %c
  %cmp1 = fcmp nnan ole float %c, %b
  %0 = or i1 %cmp1, %cmp
  ret i1 %0
}

; (a >= c && c <= b) => (c <= a && c <= b) => c <= min(a,b)
define i1 @convert44 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert44(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ole float %b, %a
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %b, float %a
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan oge float [[TMP1]], %c
entry:
  %cmp = fcmp nnan oge float %a, %c
  %cmp1 = fcmp nnan ole float %c, %b
  %0 = and i1 %cmp1, %cmp
  ret i1 %0
}

; (a < c  || c > b ) => (c > a  || c > b ) => c > min(a,b)
define i1 @convert45 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert45(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ole float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan olt float [[TMP1]], %c
entry:
  %cmp = fcmp nnan olt float %a, %c
  %cmp1 = fcmp nnan ogt float %c, %b
  %0 = or i1 %cmp, %cmp1
  ret i1 %0
}

; (a < c  && c > b ) => (c > a  && c > b ) => c > max(a,b)
define i1 @convert46 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert46(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan olt float [[TMP1]], %c
entry:
  %cmp = fcmp nnan olt float %a, %c
  %cmp1 = fcmp nnan ogt float %c, %b
  %0 = and i1 %cmp, %cmp1
  ret i1 %0
}

; (a <= c || c >= b) => (c >= a || c >= b) => c >= min(a,b)
define i1 @convert47 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert47(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ole float %b, %a
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %b, float %a
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ole float [[TMP1]], %c
entry:
  %cmp = fcmp nnan ole float %a, %c
  %cmp1 = fcmp nnan oge float %c, %b
  %0 = or i1 %cmp1, %cmp
  ret i1 %0
}

; (a <= c && c >= b) => (c >= a && c >= b) => c >= max(a,b)
define i1 @convert48 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert48(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge float %b, %a
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %b, float %a
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ole float [[TMP1]], %c
entry:
  %cmp = fcmp nnan ole float %a, %c
  %cmp1 = fcmp nnan oge float %c, %b
  %0 = and i1 %cmp1, %cmp
  ret i1 %0
}

; It also works for vectors
define <4 x i1> @convert_vectors(<4 x float> %a, <4 x float> %b, <4 x float> %c)  {
; CHECK-LABEL: @convert_vectors(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge <4 x float> %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select <4 x i1> [[CMP]], <4 x float> %a, <4 x float> %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ogt <4 x float> [[TMP1]], %c

entry:
  %cmp = fcmp nnan olt <4 x float> %c, %a
  %cmp1 = fcmp nnan olt <4 x float> %c, %b
  %0 = or <4 x i1> %cmp, %cmp1
  ret <4 x i1> %0
}

; It also works for constants
define i1 @convert_constants1 (float %a, float %b)  {
; CHECK-LABEL: @convert_constants1(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ogt float [[TMP1]], 1.700000e+01
entry:
  %cmp = fcmp nnan olt float 1.700000e+01, %a
  %cmp1 = fcmp nnan olt float 1.700000e+01, %b
  %0 = or i1 %cmp, %cmp1
  ret i1 %0
}

define <4 x i1> @convert_constants2(<4 x float> %a, <4 x float> %b)  {
; CHECK-LABEL: @convert_constants2(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge <4 x float> %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select <4 x i1> [[CMP]], <4 x float> %a, <4 x float> %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ogt <4 x float> [[TMP1]], zeroinitializer

entry:
  %cmp = fcmp nnan olt <4 x float> zeroinitializer, %a
  %cmp1 = fcmp nnan olt <4 x float> zeroinitializer, %b
  %0 = or <4 x i1> %cmp, %cmp1
  ret <4 x i1> %0
}

; Transformation preserves orderness of compares
; first comparison was canonicalized from ule to ogt
define i1 @convert_preserve_orderness1 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert_preserve_orderness1(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ogt float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select nnan i1 [[CMP]], float %b, float %a
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ugt float [[TMP1]], %c
entry:
  %cmp = fcmp nnan ult float %c, %a
  %cmp1 = fcmp nnan ult float %c, %b
  %0 = and i1 %cmp, %cmp1
  ret i1 %0
}

; first comparison was canonicalized from uge to olt
define i1 @convert_preserve_orderness2 (float %a, float %b, float %c)  {
; CHECK-LABEL: @convert_preserve_orderness2(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan olt float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select nnan i1 [[CMP]], float %b, float %a
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan ugt float [[TMP1]], %c
entry:
  %cmp = fcmp nnan ult float %c, %a
  %cmp1 = fcmp nnan ult float %c, %b
  %0 = or i1 %cmp, %cmp1
  ret i1 %0
}
