; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-array-transpose,print<hir>" -disable-output -hir-details-dims 2>&1 | FileCheck %s

; Check that Transposing does not occur for large const dims that would result in
; large allocas.

; debug - [Profit] Surpasses MaxTC threshold!

; CHECK:       BEGIN REGION { }
; CHECK:                  + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK-NEXT:             |   + DO i2 = 0, 1299, 1   <DO_LOOP>
; CHECK-NEXT:             |   |   (%tmp173)[0:i2:0(i32:0)][0:i1:0(i32:0)] = 0;
; CHECK:                  |   |
; CHECK:                  |   |      %tmp256 = 0;
; CHECK-NEXT:             |   |   + DO i3 = 0, sext.i32.i64(%tmp220) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2000>
; CHECK-NEXT:             |   |   |   %tmp260 = (%tmp84)[0:i3:4000(i32:0)][0:i1:4(i32:1000)];
; CHECK-NEXT:             |   |   |   %tmp262 = (%tmp97)[0:i2:8000(i32:0)][0:i3:4(i32:2000)];
; CHECK-NEXT:             |   |   |   %tmp256 = (%tmp262 * %tmp260)  +  %tmp256;
; CHECK-NEXT:             |   |   + END LOOP
; CHECK-NEXT:             |   |      (%tmp173)[0:i2:0(i32:0)][0:i1:0(i32:0)] = %tmp256;
; CHECK:             |   + END LOOP
; CHECK:             + END LOOP
; CHECK:       END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @barney(i32 %tmp220, ptr %tmp97, ptr %tmp84) #0 {
bb:
  %tmp173 = getelementptr [130000 x i32], ptr null, i64 0, i64 0
  %tmp243 = add nsw i32 %tmp220, 1
  %tmp244 = sext i32 %tmp243 to i64
  br label %bb245

bb245:                                            ; preds = %bb270, %bb
  %tmp246 = phi i64 [ 1, %bb ], [ %tmp271, %bb270 ]
  br label %bb247

bb247:                                            ; preds = %bb268, %bb245
  %tmp248 = phi i64 [ %tmp249, %bb268 ], [ 1, %bb245 ]
  %tmp249 = add i64 %tmp248, 1
  %tmp250 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 0, ptr elementtype(i32) %tmp173, i64 %tmp248)
  %tmp251 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 0, ptr elementtype(i32) %tmp250, i64 %tmp246)
  store i32 0, ptr %tmp251, align 1
  br i1 true, label %bb268, label %bb252

bb252:                                            ; preds = %bb247
  %tmp253 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 8000, ptr elementtype(i32) %tmp97, i64 %tmp248)
  br label %bb254

bb254:                                            ; preds = %bb254, %bb252
  %tmp255 = phi i64 [ 1, %bb252 ], [ %tmp257, %bb254 ]
  %tmp256 = phi i32 [ 0, %bb252 ], [ %tmp264, %bb254 ]
  %tmp257 = add i64 %tmp255, 1
  %tmp258 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 4000, ptr elementtype(i32) %tmp84, i64 %tmp255)
  %tmp259 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %tmp258, i64 %tmp246)
  %tmp260 = load i32, ptr %tmp259, align 1
  %tmp261 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %tmp253, i64 %tmp255)
  %tmp262 = load i32, ptr %tmp261, align 1
  %tmp263 = mul nsw i32 %tmp262, %tmp260
  %tmp264 = add i32 %tmp263, %tmp256
  %tmp265 = icmp eq i64 %tmp257, %tmp244
  br i1 %tmp265, label %bb266, label %bb254

bb266:                                            ; preds = %bb254
  store i32 %tmp264, ptr %tmp251, align 1
  br label %bb268

bb268:                                            ; preds = %bb266, %bb247
  %tmp269 = icmp eq i64 %tmp249, 1301
  br i1 %tmp269, label %bb270, label %bb247

bb270:                                            ; preds = %bb268
  %tmp271 = add i64 %tmp246, 1
  %tmp272 = icmp eq i64 %tmp271, 101
  br i1 %tmp272, label %bb273, label %bb245

bb273:                                            ; preds = %bb270
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

; uselistorder directives
uselistorder ptr @llvm.intel.subscript.p0.i64.i64.p0.i64, { 5, 4, 3, 2, 1, 0 }

attributes #0 = { "intel-lang"="fortran" }
attributes #1 = { nounwind readnone speculatable }
