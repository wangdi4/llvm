; RUN: opt -passes=sycl-kernel-duplicate-called-kernels %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-duplicate-called-kernels %s -S | FileCheck %s

; Check the case that kernel 'test' is called by a noninline function 'foo',
; which is called by another kernel 'test2'.
; 'test/bar' and their associated local variables are cloned once.
;
;        test2
;          |
;         foo
;          |
;        test
;          |
;         bar

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@test.i = internal addrspace(3) global [100 x i32] undef, align 4
@test.j = internal addrspace(3) global i32 undef, align 4

define internal fastcc zeroext i1 @bar() unnamed_addr {
; CHECK-LABEL: define internal fastcc zeroext i1 @bar(
; CHECK: load i32, ptr addrspace(3) @test.i,
; CHECK: load i32, ptr addrspace(3) @test.j,
;
entry:
  %0 = load i32, ptr addrspace(3) @test.i, align 4
  %1 = load i32, ptr addrspace(3) @test.j, align 4
  ret i1 false
}

define dso_local void @test(ptr addrspace(1) %results) local_unnamed_addr {
; CHECK-LABEL: define dso_local void @test(
; CHECK: store i32 {{.*}}, ptr addrspace(3) @test.i,
; CHECK: store i32 {{.*}}, ptr addrspace(3) @test.j,
; CHECK: call fastcc zeroext i1 @bar()
;
entry:
  store i32 0, ptr addrspace(3) @test.i, align 4
  store i32 0, ptr addrspace(3) @test.j, align 4
  %call2 = tail call fastcc zeroext i1 @bar()
  ret void
}

define internal fastcc void @foo() unnamed_addr {
; CHECK-LABEL: define internal fastcc void @foo(
; CHECK: call void @test.clone(
;
entry:
  tail call void @test(ptr addrspace(1) null)
  ret void
}

define dso_local void @test2(ptr addrspace(1) %results) local_unnamed_addr {
; CHECK-LABEL: define dso_local void @test2(
; CHECK: call fastcc void @foo(
;
entry:
  call fastcc void @foo()
  ret void
}

; CHECK-LABEL: define internal fastcc zeroext i1 @bar.clone(
; CHECK: load i32, ptr addrspace(3) @test.i.clone,
; CHECK: load i32, ptr addrspace(3) @test.j.clone,

; CHECK-LABEL: define internal void @test.clone(
; CHECK: store i32 {{.*}}, ptr addrspace(3) @test.i.clone,
; CHECK: store i32 {{.*}}, ptr addrspace(3) @test.j.clone,
; CHECK: call fastcc zeroext i1 @bar.clone()

!sycl.kernels = !{!0}

!0 = !{ptr @test, ptr @test2}

; DEBUGIFY-NOT: WARNING
