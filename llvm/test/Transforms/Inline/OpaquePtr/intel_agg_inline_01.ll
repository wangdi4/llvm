; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; This test case verifies that aggressive inlining is triggered using "Single
; Access Function GlobalVar Heuristic".

; Make sure bar1, bar2 and bar3 are not inlined without "aggressive inline
; analysis" by using "-inline-threshold=-50".
; RUN: opt -opaque-pointers < %s -passes='cgscc(inline)' -inline-threshold=-50 -whole-program-assume -intel-libirc-allowed -S | FileCheck  --check-prefix=CHECK-NO-AGG-INL %s

; Make sure bar1, bar2 and bar3 are inlined with "aggressive inliner"
; even though "-inline-threshold=-50" option is used.
; RUN: opt -opaque-pointers < %s -passes='module(agginliner),cgscc(inline)' -inline-threshold=-50 -whole-program-assume -intel-libirc-allowed -S | FileCheck --check-prefix=CHECK-WITH-AGG-INL %s

; Checks for without agg-inl
; CHECK-NO-AGG-INL: call void @bar1
; CHECK-NO-AGG-INL: call void @bar2
; CHECK-NO-AGG-INL: call void @bar3

; Checks for with agg-inl
; CHECK-WITH-AGG-INL-NOT: call void @bar1
; CHECK-WITH-AGG-INL-NOT: call void @bar2
; CHECK-WITH-AGG-INL-NOT: call void @bar3

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@grad = internal unnamed_addr global ptr null, align 8

define dso_local void @wfree(ptr nocapture %p) {
entry:
  tail call void @free(ptr %p)
  ret void
}

define dso_local noalias ptr @walloc() {
entry:
  %call = tail call noalias ptr @malloc(i64 2000)
  ret ptr %call
}

define dso_local void @foo2() {
entry:
  ret void
}

define dso_local void @bar1(ptr nocapture %d) {
entry:
  store double 1.000000e+01, ptr %d, align 8
  ret void
}

define dso_local void @bar2(ptr nocapture %d) {
entry:
  store double 2.000000e+01, ptr %d, align 8
  ret void
}

define dso_local void @bar3(ptr nocapture %d) {
entry:
  store double 3.000000e+01, ptr %d, align 8
  ret void
}

define dso_local void @foo1() {
entry:
  %i = load ptr, ptr @grad, align 8
  %cmp = icmp eq ptr %i, null
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @wfree(ptr nonnull %i)
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %call = tail call ptr @walloc()
  store ptr %call, ptr @grad, align 8
  %add.ptr = getelementptr inbounds double, ptr %call, i64 1
  tail call void @bar1(ptr nonnull %add.ptr)
  %add.ptr1 = getelementptr inbounds double, ptr %call, i64 2
  tail call void @bar2(ptr nonnull %add.ptr1)
  %add.ptr2 = getelementptr inbounds double, ptr %call, i64 3
  tail call void @bar3(ptr nonnull %add.ptr2)
  ret void
}

declare dso_local noalias ptr @malloc(i64)

declare dso_local void @free(ptr nocapture)
; end INTEL_FEATURE_SW_ADVANCED
