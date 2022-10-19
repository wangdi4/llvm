; This test case verifies that aggressive inlining is not triggered using
; "Single Access Function GlobalVar Heuristic" since "bar3" is not leaf
; as expected by the heuristic. "bar3" calls "bar4" that calls "bar5".

; REQUIRES: asserts

; RUN: opt -opaque-pointers < %s -agginliner -debug-only=agginliner -whole-program-assume -intel-libirc-allowed -disable-output  2>&1 | FileCheck %s
; RUN: opt -opaque-pointers < %s -passes='module(agginliner)' -debug-only=agginliner -whole-program-assume -intel-libirc-allowed -disable-output  2>&1 | FileCheck %s

; CHECK: AggInl: SingleAccessFunctionGlobalVarHeuristic
; CHECK:  GV selected as candidate: grad
; CHECK:  Ignored GV ... calls are not okay to inline

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@grad = internal unnamed_addr global ptr null, align 8

define dso_local void @bar5(ptr nocapture %d) {
entry:
  store double 4.000000e+01, ptr %d, align 8
  ret void
}

define dso_local void @bar4(ptr nocapture %d) {
entry:
  tail call void @bar5(ptr %d)
  ret void
}

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
  tail call void @bar4(ptr %d)
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
