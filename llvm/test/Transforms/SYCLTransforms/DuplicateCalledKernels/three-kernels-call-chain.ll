; RUN: opt -passes=sycl-kernel-duplicate-called-kernels %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-duplicate-called-kernels %s -S | FileCheck %s

; Check the case kernel 'test' is used in kernel 'test2' and 'test2' is used in
; kernel 'test3'.
; 'test' is cloned twice and 'test2' is cloned once.
;
;       test3
;       /   \
;    test2  bar
;      |
;    test
;      |
;     foo

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@test.i = internal addrspace(3) global [100 x i32] undef, align 4
@test.j = internal addrspace(3) global i32 undef, align 4
@test3.s = internal addrspace(3) global [10 x double] undef, align 8
@test3.t = internal addrspace(3) global double undef, align 8

define internal fastcc zeroext i1 @foo() {
; CHECK-LABEL: define internal fastcc zeroext i1 @foo(
; CHECK: load i32, ptr addrspace(3) @test.i,
; CHECK: load i32, ptr addrspace(3) @test.j,
;
entry:
  %0 = load i32, ptr addrspace(3) @test.i, align 4
  %1 = load i32, ptr addrspace(3) @test.j, align 4
  ret i1 false
}

define dso_local void @test(ptr addrspace(1) %results) {
; CHECK-LABEL: define dso_local void @test(
; CHECK: store i32 %0, ptr addrspace(3) @test.i,
; CHECK: store i32 %1, ptr addrspace(3) @test.j,
; CHECK: call fastcc zeroext i1 @foo()
;
entry:
  %0 = load i32, ptr addrspace(1) null, align 4294967296
  store i32 %0, ptr addrspace(3) @test.i, align 16
  %1 = load i32, ptr addrspace(1) null, align 4294967296
  store i32 %1, ptr addrspace(3) @test.j, align 4
  %call2 = tail call fastcc zeroext i1 @foo()
  ret void
}

define dso_local void @test2(ptr addrspace(1) %results) {
; CHECK-LABEL: define dso_local void @test2(
; CHECK: call void @test.clone(
;
entry:
  tail call void @test(ptr addrspace(1) null)
  ret void
}

define internal fastcc zeroext i1 @bar() {
; CHECK-LABEL: define internal fastcc zeroext i1 @bar(
; CHECK: load double, ptr addrspace(3) @test3.s,
; CHECK: load double, ptr addrspace(3) @test3.t,
;
entry:
  %0 = load double, ptr addrspace(3) @test3.s, align 8
  %1 = load double, ptr addrspace(3) @test3.t, align 8
  ret i1 false
}

define dso_local void @test3(ptr addrspace(1) %results) {
; CHECK-LABEL: define dso_local void @test3(
; CHECK: store double {{.*}}, ptr addrspace(3) @test3.s,
; CHECK: store double {{.*}}, ptr addrspace(3) @test3.t,
; CHECK: call fastcc zeroext i1 @bar()
; CHECK: call void @test2.clone(
;
entry:
  store double 0.000000e+00, ptr addrspace(3) @test3.s, align 8
  store double 0.000000e+00, ptr addrspace(3) @test3.t, align 8
  %call4 = tail call fastcc zeroext i1 @bar()
  tail call void @test2(ptr addrspace(1) null)
  ret void
}

; CHECK-LABEL: define internal fastcc zeroext i1 @foo.clone(
; CHECK: load i32, ptr addrspace(3) @test.i.clone,
; CHECK: load i32, ptr addrspace(3) @test.j.clone,

; CHECK-LABEL: define internal void @test.clone(
; CHECK: store i32 %0, ptr addrspace(3) @test.i.clone,
; CHECK: store i32 %1, ptr addrspace(3) @test.j.clone,
; CHECK: call fastcc zeroext i1 @foo.clone()

; CHECK-LABEL: define internal fastcc zeroext i1 @foo.clone.clone(
; CHECK: load i32, ptr addrspace(3) @test.i.clone.{{.*}},
; CHECK: load i32, ptr addrspace(3) @test.j.clone.{{.*}},

; CHECK-LABEL: define internal void @test.clone.clone(
; CHECK: store i32 %0, ptr addrspace(3) @test.i.clone.{{.*}},
; CHECK: store i32 %1, ptr addrspace(3) @test.j.clone.{{.*}},
; CHECK: tail call fastcc zeroext i1 @foo.clone.clone()

; CHECK-LABEL: define internal void @test2.clone(
; CHECK: call void @test.clone.clone(

!sycl.kernels = !{!0}

!0 = !{ptr @test, ptr @test2, ptr @test3}

; DEBUGIFY-NOT: WARNING
