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
  %tmp0 = load i32, ptr @tf_1_var_128, align 4
  %tmp1 = load i32, ptr @tf_1_var_96, align 4
  %cmp = icmp eq i32 %tmp1, -1
  %tmp2 = sext i1 %cmp to i32
  %sub = add i32 %tmp0, %tmp2
  %conv2 = trunc i32 %sub to i16
  store i16 %conv2, ptr @tf_1_var_168, align 2
  ret void
}

; This case has xor X,-1 / select xor, 0, 1 being re-written back into select.
; If we suppress the select transform, IC can actually remove most of the code.
; CHECK-LABEL: flex128_encode_int
; CHECK-NOT: xor
; CHECK-NOT: sext
; CHECK: ret void
define void @flex128_encode_int(i16 %type, ptr %src) {
entry:
  %type.off = add i16 %type, -10
  %switch = icmp ult i16 %type.off, 1
  br i1 %switch, label %do.body33, label %if.end68

do.body33:                                        ; preds = %entry
  %__tbuf34.0.copyload = load i64, ptr %src, align 1
  %tobool38.not = icmp sgt i64 %__tbuf34.0.copyload, -1
  %neg40 = xor i64 %__tbuf34.0.copyload, -1
  %spec.select52 = select i1 %tobool38.not, i64 %__tbuf34.0.copyload, i64 %neg40
  %spec.select53 = select i1 %tobool38.not, i64 0, i64 1
  %shl42 = shl i64 %spec.select52, 1
  %add44 = or i64 %shl42, %spec.select53
  br label %if.end68

if.end68:                                         ; preds = %do.body33, %entry
  %tmp.3.ph = phi i64 [ %add44, %do.body33 ], [ undef, %entry ]
  ret void
}
