; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output %s 2>&1 | FileCheck %s

; Check that DD testing for fusion is successful for unknown loop inside fuse
; candidates. DD needs to test for (@global.9)[3 * i2 + %tmp76 * i3 + 3] &
; (@global.9)[i2]

; CHECK: Function: foo

; CHECK:          BEGIN REGION { }
; CHECK:                + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:                |   + DO i2 = 0, 499999, 1   <DO_LOOP>
; CHECK:                |   |   (@global.9)[0][i2] = 1;
; CHECK:                |   + END LOOP
; CHECK:                |   + DO i2 = 0, 499999, 1   <DO_LOOP>
; CHECK:                |   |   %tmp61 = (@global.9)[0][i2];
; CHECK:                |   |   if (%tmp61 != 0)
; CHECK:                |   |   {
; CHECK:                |   |      %tmp76 = zext.i32.i64(2 * i2 + 3);
; CHECK:                |   |      if (3 * i2 + 4 <u 500001)
; CHECK:                |   |      {
; CHECK:                |   |         + UNKNOWN LOOP i3
; CHECK:                |   |         |   <i3 = 0>
; CHECK:                |   |         |   bb63:
; CHECK:                |   |         |   (@global.9)[0][3 * i2 + %tmp76 * i3 + 3] = 0;
; CHECK:                |   |         |   if (5 * i2 + %tmp76 * i3 + 7 <u 500001)
; CHECK:                |   |         |   {
; CHECK:                |   |         |      <i3 = i3 + 1>
; CHECK:                |   |         |      goto bb63;
; CHECK:                |   |         |   }
; CHECK:                |   |         + END LOOP
; CHECK:                |   |      }
; CHECK:                |   |      %tmp47 = 2 * i2 + 3;
; CHECK:                |   |   }
; CHECK:                |   + END LOOP
; CHECK:                |   %tmp47.out = %tmp47;
; CHECK:                + END LOOP
; CHECK:          END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global.9 = external dso_local global [500000 x i32]

define void @foo() {
bb:
  br label %bb45

bb45:                                             ; preds = %bb87, %bb
  %tmp46 = phi i32 [ 1, %bb ], [ %tmp89, %bb87 ]
  %tmp47 = phi i32 [ 0, %bb ], [ %tmp88, %bb87 ]
  br label %bb48

bb48:                                             ; preds = %bb48, %bb45
  %tmp49 = phi i64 [ %tmp51, %bb48 ], [ 1, %bb45 ]
  %tmp50 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) @global.9, i64 %tmp49)
  store i32 1, ptr %tmp50, align 4
  %tmp51 = add i64 %tmp49, 1
  %tmp52 = icmp eq i64 %tmp51, 500001
  br i1 %tmp52, label %bb53, label %bb48

bb53:                                             ; preds = %bb48
  br label %bb54

bb54:                                             ; preds = %bb80, %bb53
  %tmp55 = phi i64 [ %tmp82, %bb80 ], [ 1, %bb53 ]
  %tmp56 = phi i64 [ %tmp85, %bb80 ], [ 3, %bb53 ]
  %tmp57 = phi i64 [ %tmp84, %bb80 ], [ 4, %bb53 ]
  %tmp58 = phi i32 [ %tmp83, %bb80 ], [ 1, %bb53 ]
  %tmp59 = phi i32 [ 0, %bb80 ], [ %tmp47, %bb53 ]
  %tmp60 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) @global.9, i64 %tmp55)
  %tmp61 = load i32, ptr %tmp60, align 4
  %tmp62 = icmp eq i32 %tmp61, 0
  br i1 %tmp62, label %bb80, label %bb73

bb63:                                             ; preds = %bb79, %bb63
  %tmp64 = phi i64 [ %tmp66, %bb63 ], [ %tmp57, %bb79 ]
  %tmp65 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) @global.9, i64 %tmp64)
  store i32 0, ptr %tmp65, align 4
  %tmp66 = add i64 %tmp64, %tmp56
  %tmp67 = icmp ult i64 %tmp66, 500001
  br i1 %tmp67, label %bb63, label %bb68

bb68:                                             ; preds = %bb63
  br label %bb69

bb69:                                             ; preds = %bb73, %bb68
  %tmp70 = trunc i64 %tmp55 to i32
  %tmp71 = shl i32 %tmp70, 1
  %tmp72 = or i32 %tmp71, 1
  br label %bb80

bb73:                                             ; preds = %bb54
  %tmp74 = shl i32 %tmp58, 1
  %tmp75 = or i32 %tmp74, 1
  %tmp76 = zext i32 %tmp75 to i64
  %tmp77 = add i64 %tmp55, %tmp76
  %tmp78 = icmp ult i64 %tmp77, 500001
  br i1 %tmp78, label %bb79, label %bb69

bb79:                                             ; preds = %bb73
  br label %bb63

bb80:                                             ; preds = %bb69, %bb54
  %tmp81 = phi i32 [ %tmp72, %bb69 ], [ %tmp59, %bb54 ]
  %tmp82 = add i64 %tmp55, 1
  %tmp83 = add i32 %tmp58, 1
  %tmp84 = add i64 %tmp57, 3
  %tmp85 = add i64 %tmp56, 2
  %tmp86 = icmp eq i64 %tmp82, 500001
  br i1 %tmp86, label %bb87, label %bb54

bb87:                                             ; preds = %bb80
  %tmp88 = phi i32 [ %tmp81, %bb80 ]
  %tmp89 = add i32 %tmp46, 1
  %tmp90 = icmp eq i32 %tmp89, 11
  br i1 %tmp90, label %bb91, label %bb45

bb91:                                             ; preds = %bb87
  %tmp92 = phi i32 [ %tmp88, %bb87 ]
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

; uselistorder directives
uselistorder ptr @llvm.intel.subscript.p0.i64.i64.p0.i64, { 2, 1, 0 }

attributes #0 = { nounwind readnone speculatable }
