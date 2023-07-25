; INTEL_FEATURE_SW_DTRANS
; REQUIRES: asserts, intel_feature_sw_dtrans

; RUN: opt -passes="padded-pointer-prop-op,hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-vec-dir-insert,hir-vplan-vec,hir-cg" \
; RUN:     -dtrans-test-paddedmalloc -vplan-force-vf=4 -disable-output \
; RUN:     -debug-only=vplan-idioms -vplan-use-padding-info=true < %s 2>&1 | FileCheck %s

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-vec-dir-insert,hir-vplan-vec,hir-cg" \
; RUN:     -dtrans-test-paddedmalloc -vplan-force-vf=4 -disable-output \
; RUN:     -debug-only=vplan-idioms -vplan-use-padding-info=true < %s 2>&1 | FileCheck %s

; This test case tests padding propagation and use of it in vectorizer.
;
; -----------------------------------------------------------------------------
;
; Before VPlan:
; BEGIN REGION { }
;       @llvm.intel.directive(!0);
;       @llvm.intel.directive(!1);
;
;       + DO i1 = 0, 9, 1   <DO_MULTI_EXIT_LOOP>
;       |   if ((%pv1)[i1] != (%pv2)[i1])
;       |   {
;       |      goto bb3;
;       |   }
;       + END LOOP
;
;       @llvm.intel.directive(!2);
;       @llvm.intel.directive(!1);
; END REGION
;
; -----------------------------------------------------------------------------
;
; CHECK: Search loop was recognized

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@0 = private unnamed_addr constant [15 x i8] c"padded 4 bytes\00"
@.str = private unnamed_addr constant [10 x i8] c"bitcast.c\00"

define internal zeroext i1 @searchloop(ptr %p) {
entry:
  %t0 = tail call ptr @llvm.ptr.annotation.p0(ptr %p, ptr getelementptr inbounds ([15 x i8], ptr @0, i64 0, i64 0), ptr getelementptr inbounds ([10 x i8], ptr @.str, i64 0, i64 0), i32 2, ptr null)
  %pv1 = bitcast ptr %t0 to ptr
  br label %bb2

bb1:
  %t2 = icmp ult i64 %t10, 10
  br i1 %t2, label %bb2, label %bb3

bb2:
  %t4 = phi i64 [ 0, %entry ], [ %t10, %bb1 ]
  %t5 = getelementptr inbounds i8, ptr %pv1, i64 %t4
  %t6 = load i8, ptr %t5, align 4
  %t7 = getelementptr inbounds i8, ptr %pv1, i64 %t4
  %t8 = load i8, ptr %t7, align 4
  %t9 = icmp ne i8 %t6, %t8
  %t10 = add nuw nsw i64 %t4, 1
  br i1 %t9, label %bb3, label %bb1

bb3:
  %t12 = phi i1 [ true, %bb2 ], [ false, %bb1 ]
  ret i1 %t12
}

declare ptr @llvm.ptr.annotation.p0(ptr, ptr, ptr, i32, ptr)

; end INTEL_FEATURE_SW_DTRANS
