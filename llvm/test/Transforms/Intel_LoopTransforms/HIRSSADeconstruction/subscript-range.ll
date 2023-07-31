; RUN: opt < %s -passes=hir-ssa-deconstruction -S | FileCheck %s

; Verify that %ptr and %ptr1 form an SCC and we add the metadata to prevent live range violation.
; Also check that the same metadata node is shared by all liveout and live range metadata.

; CHECK: !out.de.ssa [[MD:!.*]]
; CHECK: !out.de.ssa [[MD]]

; CHECK: %tmp31 =
; CHECK-SAME: %ptr.out

; CHECK: %ptr1 =
; CHECK-SAME: !live.range.de.ssa [[MD]]
; CHECK: !live.range.de.ssa [[MD]]

; HIR for reference:
;
; WRONG HIR:
;
; BEGIN REGION { }
;       + DO i1 = 0, 11, 1   <DO_LOOP>
;       |   %tmp32 = (%ptr)[0];
;       |   %ptr = &((%ptr)[2]);
;       |   %tmp35 = (%ptr)[1];
;       |   %accum = (%ptr)[0]  +  %tmp32 + %tmp35 + %accum;
;       + END LOOP
; END REGION
;
; CORRECT HIR:
;
; BEGIN REGION { }
;       + DO i1 = 0, 11, 1   <DO_LOOP>
;       |   %ptr.out = &((%ptr)[0]);
;       |   %tmp32 = (%ptr)[0];
;       |   %ptr = &((%ptr)[2]);
;       |   %tmp35 = (%ptr.out)[1];
;       |   %accum = (%ptr)[0]  +  %tmp32 + %tmp35 + %accum;
;       + END LOOP
; END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global = external hidden unnamed_addr constant [11 x i8], align 1

define dso_local i32 @main() local_unnamed_addr {
bb:
  %tmp = alloca [100 x i16], align 16
  %tmp1 = bitcast ptr %tmp to ptr
  br label %bb2

bb2:
  %ptr0 = getelementptr inbounds [100 x i16], ptr %tmp, i64 0, i64 0
  br label %loop

loop:
  %cnt = phi i32 [ 12, %bb2 ], [ %cnt_next, %loop ]
  %accum = phi i16 [ 0, %bb2 ], [ %sum, %loop ]
  %ptr = phi ptr [ %ptr0, %bb2 ], [ %ptr1, %loop ]

  %tmp30 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 2, ptr elementtype(i16) %ptr, i64 0)
  %tmp31 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 2, ptr elementtype(i16) %ptr, i64 1)
  %tmp32 = load i16, ptr %tmp30, align 2
  %tmp33 = add i16 %tmp32, %accum
  %ptr1 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 2, ptr elementtype(i16) %ptr, i64 2)
  %tmp35 = load i16, ptr %tmp31, align 2
  %tmp36 = add i16 %tmp33, %tmp35
  %tmp37 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 2, ptr elementtype(i16) %ptr, i64 3)
  %tmp38 = load i16, ptr %ptr1, align 2
  %sum = add i16 %tmp38, %tmp36

  %cnt_next = add nsw i32 %cnt, -1
  %cmp = icmp eq i32 %cnt_next, 0
  br i1 %cmp, label %bb50, label %loop

bb50:
  %tmp51 = phi i16 [ %sum, %loop ]
  %tmp52 = sext i16 %tmp51 to i32
  ret i32 %tmp52
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)

