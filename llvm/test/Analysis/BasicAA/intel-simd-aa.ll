; RUN: opt -S -aa-pipeline=basic-aa -passes="licm" %s | FileCheck %s

; Tests that basic-aa can properly test the load of %this, against the
; directive.region.entry and exit calls.
; AA should test %this against each of the operand bundles of
; directive.region.entry. The first function should have no alias, so the load
; can be moved to the entry block.
; The second function has a possible alias, and the movement should be blocked.

; CHECK-LABEL: licm
; CHECK-LABEL: entry
; CHECK: load i32, ptr %this
; CHECK-LABEL: bb23

; CHECK-LABEL: licm.neg
; CHECK-LABEL: entry
; CHECK-NOT: load i32, ptr %this
; CHECK-LABEL: bb23
; CHECK: load i32, ptr %this

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

define hidden void @licm(ptr %this) {
entry:
  %localvar = alloca i32, align 4
  br label %bb23

bb23:                                             ; preds = %bb46, %bb
  %tmp27 = load i32, ptr %this, align 4
  %tmp28 = icmp slt i32 undef, %tmp27
  br i1 %tmp28, label %bb31, label %bb49

bb31:                                             ; preds = %bb23
  %tmp32 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.ALIGNED:PTR_TO_PTR"(ptr null, i32 64), "QUAL.OMP.ALIGNED:PTR_TO_PTR"(ptr null, i32 64), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null), "QUAL.OMP.LINEAR:IV"(ptr %localvar, i32 1) ]
  br label %bb46

bb46:                                             ; preds = %bb31
  call void @llvm.directive.region.exit(token %tmp32) [ "DIR.OMP.END.SIMD"() ]
  br label %bb23

bb49:                                             ; preds = %bb23
  ret void
}

define hidden void @licm.neg(ptr %this, ptr %unknownptr) {
entry:
  br label %bb23

bb23:                                             ; preds = %bb46, %bb
  %tmp27 = load i32, ptr %this, align 4
  %tmp28 = icmp slt i32 undef, %tmp27
  br i1 %tmp28, label %bb31, label %bb49

bb31:                                             ; preds = %bb23
  %tmp32 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.ALIGNED:PTR_TO_PTR"(ptr null, i32 64), "QUAL.OMP.ALIGNED:PTR_TO_PTR"(ptr null, i32 64), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null), "QUAL.OMP.LINEAR:IV"(ptr %unknownptr, i32 1) ]
  br label %bb46

bb46:                                             ; preds = %bb31
  call void @llvm.directive.region.exit(token %tmp32) [ "DIR.OMP.END.SIMD"() ]
  br label %bb23

bb49:                                             ; preds = %bb23
  ret void
}

attributes #0 = { nounwind }
