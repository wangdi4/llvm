; This test case verifies that aggressive inlining is triggered using "Single
; Access Function GlobalVar Heuristic".

; Make sure bar1, bar2 and bar3 are inlined with "aggressive inliner"
; even though "-inline-threshold=-50" option is used. Also make sure that
; foo1 is NOT inlined, even though it contains the calls to bar1, bar2, and
; bar3.

; RUN: opt < %s -agginliner -inline -inline-threshold=-50 -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S | FileCheck %s
; RUN: opt < %s -passes='module(agginliner),cgscc(inline)' -inline-threshold=-50 -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S | FileCheck %s

; Checks for with agg-inl
; CHECK: call void @foo1
; CHECK-NOT: call void @bar1
; CHECK-NOT: call void @bar2
; CHECK-NOT: call void @bar3

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@grad = internal unnamed_addr global double* null, align 8

define dso_local void @wfree(i8* nocapture %p) {
entry:
  tail call void @free(i8* %p)
  ret void
}

define dso_local noalias double* @walloc() {
entry:
  %call = tail call noalias i8* @malloc(i64 2000)
  %0 = bitcast i8* %call to double*
  ret double* %0
}

define dso_local void @bar1(double* nocapture %d) {
entry:
  store double 1.000000e+01, double* %d, align 8
  ret void
}

define dso_local void @bar2(double* nocapture %d) {
entry:
  store double 2.000000e+01, double* %d, align 8
  ret void
}

define dso_local void @bar3(double* nocapture %d) {
entry:
  store double 3.000000e+01, double* %d, align 8
  ret void
}

define dso_local void @foo1() {
entry:
  %0 = load double*, double** @grad, align 8
  %cmp = icmp eq double* %0, null
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %1 = bitcast double* %0 to i8*
  tail call void @wfree(i8* nonnull %1)
  br label %if.end

if.end:                                           ; preds = %entry, %if.then
  %call = tail call double* @walloc()
  store double* %call, double** @grad, align 8
  %add.ptr = getelementptr inbounds double, double* %call, i64 1
  tail call void @bar1(double* nonnull %add.ptr)
  %add.ptr1 = getelementptr inbounds double, double* %call, i64 2
  tail call void @bar2(double* nonnull %add.ptr1)
  %add.ptr2 = getelementptr inbounds double, double* %call, i64 3
  tail call void @bar3(double* nonnull %add.ptr2)
  ret void
}

define dso_local i32 @main(i32 %0, i8** nocapture readonly %1) {
  tail call void @foo1()
  ret i32 0
}

declare dso_local noalias i8* @malloc(i64)
declare dso_local void @free(i8* nocapture)
