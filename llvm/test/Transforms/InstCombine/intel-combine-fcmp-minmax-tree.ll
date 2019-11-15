; RUN: opt < %s -instcombine -S | FileCheck %s

; Tree of 'OR/AND'
define i1 @OR_tree_convert(float %a, float %b, float %c, float %d, float %e)  {
; CHECK-LABEL: @OR_tree_convert(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP3:%.*]] = fcmp olt float %d, %e
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ole float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan olt float [[TMP1]], %c
; CHECK-NEXT:    [[OR:%.*]] = or i1 [[TMP2]], [[TMP3]]

entry:
  %cmp = fcmp nnan ogt float %c, %a
  %cmp1 = fcmp olt float %d, %e
  %or.cond39 = or i1 %cmp1, %cmp
  %cmp3 = fcmp nnan ogt float %c, %b
  %res = or i1 %or.cond39, %cmp3
  ret i1 %res
}

define i1 @AND_tree_convert(float %a, float %b, float %c, float %d, float %e)  {
; CHECK-LABEL: @AND_tree_convert(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP3:%.*]] = fcmp olt float %d, %e
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan oge float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan olt float [[TMP1]], %c
; CHECK-NEXT:    [[OR:%.*]] = and i1 [[TMP2]], [[TMP3]]

entry:
  %cmp = fcmp nnan ogt float %c, %a
  %cmp1 = fcmp olt float %d, %e
  %or.cond39 = and i1 %cmp1, %cmp
  %cmp3 = fcmp nnan ogt float %c, %b
  %res = and i1 %or.cond39, %cmp3
  ret i1 %res
}

; When hoisting fcmp instruction up through the tree,
; all operands need to be available at insertion point
define i1 @OR_tree_no_convert(float %a, float %b, float %c, float %d, float %e)  {
; CHECK-LABEL: @OR_tree_no_convert(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ogt float %c, %a
; CHECK-NEXT:    [[ADD1:%.*]] = fadd float %d, 1.000000e+00
; CHECK-NEXT:    [[CMP1:%.*]] = fcmp olt float [[ADD1]], %e
; CHECK-NEXT:    [[OR1:%.*]] = or i1 [[CMP1]], [[CMP]]
; CHECK-NEXT:    [[ADD2:%.*]] = fadd float %b, 1.000000e+00
; CHECK-NEXT:    [[CMP3:%.*]] = fcmp nnan olt float [[ADD2]], %c
; CHECK-NEXT:    [[RES:%.*]] = or i1 [[OR1]], [[CMP3]]

entry:
  %cmp = fcmp nnan ogt float %c, %a
  %add1 = fadd float %d, 1.000000e+00
  %cmp1 = fcmp olt float %add1, %e
  %or.cond39 = or i1 %cmp1, %cmp
  %add2 = fadd float %b, 1.000000e+00
  %cmp3 = fcmp nnan olt float %add2, %c
  %res = or i1 %or.cond39, %cmp3
  ret i1 %res
}

; Mixing logical operators prevents combining
define i1 @mixed_tree_no_convert1(float %a, float %b, float %c, float %d, float %e)  {
; CHECK-LABEL: @mixed_tree_no_convert1(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ogt float %c, %a
; CHECK-NEXT:    [[CMP1:%.*]] = fcmp olt float %d, %e
; CHECK-NEXT:    [[OR1:%.*]] = and i1 [[CMP1]], [[CMP]]
; CHECK-NEXT:    [[CMP3:%.*]] = fcmp nnan olt float %b, %c
; CHECK-NEXT:    [[RES:%.*]] = or i1 [[OR1]], [[CMP3]]

entry:
  %cmp = fcmp nnan ogt float %c, %a
  %cmp1 = fcmp olt float %d, %e
  %or.cond39 = and i1 %cmp1, %cmp
  %cmp3 = fcmp nnan olt float %b, %c
  %res = or i1 %or.cond39, %cmp3
  ret i1 %res
}

define i1 @mixed_tree_no_convert2(float %a, float %b, float %c, float %d, float %e)  {
; CHECK-LABEL: @mixed_tree_no_convert2(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ogt float %c, %a
; CHECK-NEXT:    [[CMP1:%.*]] = fcmp olt float %d, %e
; CHECK-NEXT:    [[OR1:%.*]] = or i1 [[CMP1]], [[CMP]]
; CHECK-NEXT:    [[CMP3:%.*]] = fcmp nnan olt float %b, %c
; CHECK-NEXT:    [[RES:%.*]] = and i1 [[OR1]], [[CMP3]]

entry:
  %cmp = fcmp nnan ogt float %c, %a
  %cmp1 = fcmp olt float %d, %e
  %or.cond39 = or i1 %cmp1, %cmp
  %cmp3 = fcmp nnan olt float %b, %c
  %res = and i1 %or.cond39, %cmp3
  ret i1 %res
}

; FCmp instruction can't be hoisted if it has multiple uses
define i1 @multi_use_tree_no_convert (float %a, float %b, float %c, float %d, float %e, i8* %f)  {
; CHECK-LABEL: @multi_use_tree_no_convert(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ogt float %c, %a
; CHECK-NEXT:    [[CMP1:%.*]] = fcmp olt float %d, %e
; CHECK-NEXT:    [[OR1:%.*]] = or i1 [[CMP1]], [[CMP]]
; CHECK-NEXT:    [[CMP3:%.*]] = fcmp nnan olt float %b, %c
; CHECK-NEXT:    [[TMP1:%.*]] = zext i1 [[CMP3]] to i8
; CHECK-NEXT:    store i8 [[TMP1]], i8* %f
; CHECK-NEXT:    [[RES:%.*]] = or i1 [[OR1]], [[CMP3]]

entry:
  %cmp = fcmp nnan ogt float %c, %a
  %cmp1 = fcmp olt float %d, %e
  %or.cond39 = or i1 %cmp1, %cmp
  %cmp3 = fcmp nnan olt float %b, %c
  %frombool = zext i1 %cmp3 to i8
  store i8 %frombool, i8* %f, align 1
  %res = or i1 %or.cond39, %cmp3
  ret i1 %res
}

define i1 @logical_tree_convert_hoist_cmp1(float %a, float %b, float %c, i1 %d)  {
; CHECK-LABEL: @logical_tree_convert_hoist_cmp1(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ole float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan olt float [[TMP1]], %c
; CHECK-NEXT:    [[OR:%.*]] = or i1 [[TMP2]], %d

entry:
  %cmp = fcmp nnan ogt float %c, %a
  %or.cond39 = or i1 %d, %cmp
  %cmp3 = fcmp nnan ogt float %c, %b
  %res = or i1 %or.cond39, %cmp3
  ret i1 %res
}

define i1 @logical_tree_convert_hoist_cmp2(float %a, float %b, float %c, i1 %d)  {
; CHECK-LABEL: @logical_tree_convert_hoist_cmp2(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = fcmp nnan ole float %a, %b
; CHECK-NEXT:    [[TMP1:%.*]] = select i1 [[CMP]], float %a, float %b
; CHECK-NEXT:    [[TMP2:%.*]] = fcmp nnan olt float [[TMP1]], %c
; CHECK-NEXT:    [[OR:%.*]] = or i1 [[TMP2]], %d

entry:
  %cmp = fcmp nnan ogt float %c, %a
  %or.cond39 = or i1 %cmp, %d
  %cmp3 = fcmp nnan ogt float %c, %b
  %res = or i1 %or.cond39, %cmp3
  ret i1 %res
}

; Check that fcmp min/max idiom check won't crash compilation in case if one
; of operands is evaluated to constant value.
define zeroext i1 @no_fcmp_minmax_crash(float %x) {
; CHECK-LABEL: @no_fcmp_minmax_crash(
; CHECK-NEXT:    [[FCMP:%.*]] = fcmp {{oeq|une}} float [[X:%.*]], 1.000000e+01
; CHECK-NEXT:    [[OR:%.*]] = or i1 [[FCMP]], or (i1 icmp ne (i64 addrspace(3)* addrspacecast (i64 addrspace(4)* null to i64 addrspace(3)*), i64 addrspace(3)* null), i1 icmp ne (i64* addrspacecast (i64 addrspace(4)* null to i64*), i64* null))
; CHECK-NEXT:    ret i1 [[OR]]
;
  %cmp = fcmp une float %x, 1.000000e+01
  %or.cond1 = or i1 %cmp, or (i1 icmp ne (i64 addrspace(3)* addrspacecast (i64 addrspace(4)* null to i64 addrspace(3)*), i64 addrspace(3)* null), i1 icmp ne (i64* addrspacecast (i64 addrspace(4)* null to i64*), i64* null))
  ret i1 %or.cond1
}
