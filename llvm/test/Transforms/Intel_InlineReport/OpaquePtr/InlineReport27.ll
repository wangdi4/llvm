; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -opaque-pointers -inline -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed < %s -S 2>&1 | FileCheck --check-prefix=CHECK-NEW %s
; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed < %s -S 2>&1 | FileCheck --check-prefix=CHECK-NEW %s
; Inline report via metadata
; RUN: opt -opaque-pointers -inlinereportsetup -inline-report=0xe886 < %s -S | opt -inline -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefix=CHECK-OLD
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefix=CHECK-OLD

target triple = "x86_64-unknown-linux-gnu"

; This test checks that none of the call sites of @mycaller are inlined
; due to the key switch heuristic.  It does this by checking that the
; calls inside @mycaller remain and that the inline report reports this.

%struct.tt = type { %struct.ss, ptr }
%struct.ss = type { i32, i64, ptr }

@myglobal1 = dso_local global %struct.tt zeroinitializer, align 16
@myglobal2 = dso_local global %struct.ss zeroinitializer, align 16

; Function Attrs: nounwind
define internal void @f(ptr byval(%struct.ss) %b) #0 {
entry:
  %tmp = getelementptr %struct.ss, ptr %b, i32 0, i32 0
  %tmp1 = load i32, ptr %tmp, align 4
  %tmp2 = add i32 %tmp1, 1
  store i32 %tmp2, ptr %tmp, align 4
  ret void
}

; Function Attrs: nounwind
define internal void @g(ptr byval(%struct.ss) align 32 %b) #0 {
entry:
  %tmp = getelementptr %struct.ss, ptr %b, i32 0, i32 0
  %tmp1 = load i32, ptr %tmp, align 4
  %tmp2 = sub i32 %tmp1, 1
  store i32 %tmp2, ptr %tmp, align 4
  ret void
}

define linkonce_odr dso_local i32 @mycallee(ptr %this, ptr %s0) align 2 {
bb:
  %tmp1 = getelementptr %struct.ss, ptr %this, i32 0, i32 2
  %tmp2 = load ptr, ptr %tmp1, align 4
  %call = call i32 %tmp2(ptr %s0)
  ret i32 %call
}

define dso_local zeroext i1 @mycaller(ptr %this, ptr %theXObject, i1 zeroext %fInReset) align 2 {
entry:
  %t0 = getelementptr inbounds %struct.tt, ptr %this, i64 0, i32 0
  %call = call i32 @mycallee(ptr %t0, ptr %theXObject)
  switch i32 %call, label %sw.default [
    i32 2, label %sw.bb2
    i32 0, label %sw.bb2
    i32 9, label %sw.bb2
    i32 11, label %sw.bb5
    i32 12, label %sw.bb8
    i32 4, label %sw.bb12
    i32 10, label %sw.bb17
    i32 8, label %sw.bb20
    i32 3, label %sw.bb23
    i32 5, label %sw.bb32
    i32 13, label %sw.bb41
  ]

sw.bb2:                                           ; preds = %entry, %entry, %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb5:                                           ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb8:                                           ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb12:                                          ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb17:                                          ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb20:                                          ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb23:                                          ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb32:                                          ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.bb41:                                          ; preds = %entry
  call void @f(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.default:                                       ; preds = %entry
  call void @g(ptr byval(%struct.ss) %theXObject) #0
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb41, %sw.bb32, %sw.bb23, %sw.bb20, %sw.bb17, %sw.bb12, %sw.bb8, %sw.bb5, %sw.bb2
  ret i1 false
}

; Function Attrs: nounwind
define i1 @main() #0 {
bb:
  %t0 = call i1 @mycaller(ptr @myglobal1, ptr @myglobal2, i1 false)
  ret i1 %t0
}

attributes #0 = { nounwind }

; Checking for old pass manager

; CHECK-OLD: COMPILE FUNC: mycaller
; CHECK-OLD: mycallee{{.*}}Callsite has key switch computations
; CHECK-OLD: f{{.*}}Callsite has key switch computations
; CHECK-OLD: g{{.*}}Callsite has key switch computations
; CHECK-OLD: f{{.*}}Callsite has key switch computations
; CHECK-OLD: g{{.*}}Callsite has key switch computations
; CHECK-OLD: f{{.*}}Callsite has key switch computations
; CHECK-OLD: g{{.*}}Callsite has key switch computations
; CHECK-OLD: f{{.*}}Callsite has key switch computations
; CHECK-OLD: g{{.*}}Callsite has key switch computations
; CHECK-OLD: f{{.*}}Callsite has key switch computations
; CHECK-OLD: g{{.*}}Callsite has key switch computations
; CHECK-OLD:{{.*}}define{{.*}}@mycaller
; CHECK-OLD: call void @f(ptr byval(%struct.ss) %theXObject)
; CHECK-OLD: call void @g(ptr byval(%struct.ss) %theXObject)
; CHECK-OLD: call void @f(ptr byval(%struct.ss) %theXObject)
; CHECK-OLD: call void @g(ptr byval(%struct.ss) %theXObject)
; CHECK-OLD: call void @f(ptr byval(%struct.ss) %theXObject)
; CHECK-OLD: call void @g(ptr byval(%struct.ss) %theXObject)
; CHECK-OLD: call void @f(ptr byval(%struct.ss) %theXObject)
; CHECK-OLD: call void @g(ptr byval(%struct.ss) %theXObject)

; Checking for new pass manager

; CHECK-NEW:{{.*}}define{{.*}}@mycaller
; CHECK-NEW: call void @f(ptr byval(%struct.ss) %theXObject)
; CHECK-NEW: call void @g(ptr byval(%struct.ss) %theXObject)
; CHECK-NEW: call void @f(ptr byval(%struct.ss) %theXObject)
; CHECK-NEW: call void @g(ptr byval(%struct.ss) %theXObject)
; CHECK-NEW: call void @f(ptr byval(%struct.ss) %theXObject)
; CHECK-NEW: call void @g(ptr byval(%struct.ss) %theXObject)
; CHECK-NEW: call void @f(ptr byval(%struct.ss) %theXObject)
; CHECK-NEW: call void @g(ptr byval(%struct.ss) %theXObject)
; CHECK-NEW: COMPILE FUNC: mycaller
; CHECK-NEW: mycallee{{.*}}Callsite has key switch computations
; CHECK-NEW: f{{.*}}Callsite has key switch computations
; CHECK-NEW: g{{.*}}Callsite has key switch computations
; CHECK-NEW: f{{.*}}Callsite has key switch computations
; CHECK-NEW: g{{.*}}Callsite has key switch computations
; CHECK-NEW: f{{.*}}Callsite has key switch computations
; CHECK-NEW: g{{.*}}Callsite has key switch computations
; CHECK-NEW: f{{.*}}Callsite has key switch computations
; CHECK-NEW: g{{.*}}Callsite has key switch computations
; CHECK-NEW: f{{.*}}Callsite has key switch computations
; CHECK-NEW: g{{.*}}Callsite has key switch computations

; end INTEL_FEATURE_SW_ADVANCED
