; RUN: opt < %s -instcombine -S | FileCheck %s
; RUN: opt < %s -passes=instcombine -S | FileCheck %s

; The following test case checks that the SExt instruction in %tmp2 is
; preserved after InstCombine. The goal is this test case is to make sure that
; there is no infinite loop when converting between Select and SExt.

; CHECK:  [[CMP:%.*]] = icmp eq i32 [[TMP1:%.*]], -1
; CHECK:  [[TMP2:%.*]] = sext i1 [[CMP]] to i32

; CHECK-NOT: [[TMP2:%.*]] = select i1 [[CMP]], i32 -1, i32 0

target triple = "x86_64-unknown-linux-gnu"

@tf_1_var_128 = external dso_local local_unnamed_addr global i32, align 4
@tf_1_var_96 = external dso_local local_unnamed_addr global i32, align 4
@tf_1_var_168 = external dso_local local_unnamed_addr global i16, align 2

define dso_local void @_Z3foov() local_unnamed_addr {
entry:
  %tmp0 = load i32, i32* @tf_1_var_128, align 4
  %tmp1 = load i32, i32* @tf_1_var_96, align 4
  %cmp = icmp eq i32 %tmp1, -1
  %tmp2 = sext i1 %cmp to i32
  %sub = add i32 %tmp0, %tmp2
  %conv2 = trunc i32 %sub to i16
  store i16 %conv2, i16* @tf_1_var_168, align 2
  ret void
}
