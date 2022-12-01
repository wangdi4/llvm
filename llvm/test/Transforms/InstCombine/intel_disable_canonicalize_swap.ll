; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; RUN: opt -opaque-pointers -S -passes=instcombine -disable-canonicalize-swap=false %s 2>&1 | FileCheck %s --check-prefix=CHECK-ENABLE
; RUN: opt -opaque-pointers -S -passes=instcombine -disable-canonicalize-swap=true %s 2>&1 | FileCheck %s --check-prefix=CHECK-DISABLE

; Test that InstCombine canonicalization that performs swaps of GEP of GEP
; instructions can be controlled to inhibit the transformation in some cases.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS9_NodeInfo._NodeInfo.972 = type { ptr, [16 x ptr], i64, %struct._ZTS16_RealPixelPacket._RealPixelPacket, float, i64, i64, i64 }
%struct._ZTS16_RealPixelPacket._RealPixelPacket = type { float, float, float, float }

define fastcc void @PruneLevel(ptr %0, ptr %1, ptr %2, i32 %3, i1 %4, i1 %5) {
  %7 = getelementptr inbounds %struct._ZTS9_NodeInfo._NodeInfo.972, ptr %1, i64 0, i32 1
  br label %8

8:                                                ; preds = %8, %6
  %9 = phi i64 [ 0, %6 ], [ %11, %8 ]
  %10 = getelementptr [16 x ptr], ptr %7, i64 0, i64 %9
  call fastcc void @PruneLevel(ptr null, ptr %10, ptr null, i32 0, i1 false, i1 false)
  %11 = add i64 %9, 1
  br i1 %4, label %12, label %8

12:                                               ; preds = %8
  ret void
}

; When canonicalization of swaps is enabled, the GEPs are reordered to place the
; the constant indices last.
; CHECK-ENABLE: %[[GEP1:[0-9]+]] = getelementptr [16 x ptr], ptr %1, i64 0, i64 %[[TMP1:[0-9]+]]
; CHECK-ENABLE: getelementptr %struct._ZTS9_NodeInfo._NodeInfo.972, ptr %[[GEP1]], i64 0, i32 1

; When canonicalization of swaps is disabled, the GEPs are simply merged.
; CHECK-DISABLE: getelementptr %struct._ZTS9_NodeInfo._NodeInfo.972, ptr %[[TMP:[0-9]+]], i64 0, i32 1, i64 %[[TMP1:[0-9]+]]

; end INTEL_FEATURE_SW_DTRANS
