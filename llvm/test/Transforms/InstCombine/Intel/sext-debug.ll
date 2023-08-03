; REQUIRES: asserts

; RUN: opt < %s -passes=instcombine -debug-only=instcombine -S 2>&1 | FileCheck %s

; The following test case checks that the SExt instruction in %tmp2 is
; preserved after InstCombine. The goal is this test case is to make sure that
; there is no infinite loop when converting between Select and SExt. This is
; the same test case as test-sext.ll but it checks the debug output.

; Check the debug output

; CHECK-LABEL: INSTCOMBINE ITERATION #1 on _Z3foov
; CHECK: IC: Visiting:   [[CMP:%.*]] = icmp eq i32 [[TMP1:%.*]], -1
; CHECK: IC: Visiting: [[TMP2:%.*]] = sext i1 [[CMP]] to i32
; CHECK-NOT: IC: ADD DEFERRED: [[TMP3:%.*]] = select i1 [[CMP]], i32 65535, i32 0
; CHECK-NOT: IC: ERASE [[TMP2]] = sext i1 [[CMP]] to i32

; Check the IR
; CHECK: [[CMP]] = icmp eq i32 [[TMP1]], -1
; CHECK: [[TMP2]] = sext i1 [[CMP]] to i32

target triple = "x86_64-unknown-linux-gnu"

@tf_1_var_128 = external dso_local local_unnamed_addr global i32, align 4
@tf_1_var_96 = external dso_local local_unnamed_addr global i32, align 4
@tf_1_var_168 = external dso_local local_unnamed_addr global i16, align 2

define dso_local void @_Z3foov() local_unnamed_addr {
entry:
  %tmp0 = load i32, ptr @tf_1_var_128, align 4
  %tmp1 = load i32, ptr @tf_1_var_96, align 4
  %cmp = icmp eq i32 %tmp1, -1
  %tmp2 = sext i1 %cmp to i32
  %sub = add i32 %tmp0, %tmp2
  %conv2 = trunc i32 %sub to i16
  store i16 %conv2, ptr @tf_1_var_168, align 2
  ret void
}
