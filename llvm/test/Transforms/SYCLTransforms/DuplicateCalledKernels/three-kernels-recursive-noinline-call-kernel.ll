; RUN: opt -passes=sycl-kernel-duplicate-called-kernels %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-duplicate-called-kernels %s -S | FileCheck %s

; Check the case that kernel 'test' is called by noinline function 'xyz', which
; is in a recursive scc. 'xyz' is called by kernels 'test2' and 'test3'.
;
;        test2  test3
;           |   /  \
;           |  /   bar
;           | /
;      --->xyz
;      |   /  \
;      |  xxx test
;      |   |   |
;      ---yyy foo

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@test.i = internal addrspace(3) global [100 x i32] undef, align 4
@test.j = internal addrspace(3) global i32 undef, align 4
@test3.s = internal addrspace(3) global [10 x double] undef, align 8
@test3.t = internal addrspace(3) global double undef, align 8

define internal fastcc zeroext i1 @foo() unnamed_addr {
; CHECK-LABEL: define internal fastcc zeroext i1 @foo(
; CHECK: load i32, ptr addrspace(3) @test.i,
; CHECK: load i32, ptr addrspace(3) @test.j,
;
entry:
  %0 = load i32, ptr addrspace(3) @test.i, align 4
  %1 = load i32, ptr addrspace(3) @test.j, align 4
  ret i1 false
}

define dso_local void @test(ptr addrspace(1) %results) local_unnamed_addr !kernel_arg_base_type !1 !arg_type_null_val !2 {
; CHECK-LABEL: define dso_local void @test(
; CHECK: store i32 {{.*}}, ptr addrspace(3) @test.i,
; CHECK: store i32 {{.*}}, ptr addrspace(3) @test.j,
; CHECK: call fastcc zeroext i1 @foo()
;
entry:
  store i32 0, ptr addrspace(3) @test.i, align 4
  store i32 0, ptr addrspace(3) @test.j, align 4
  %call2 = tail call fastcc zeroext i1 @foo()
  ret void
}

define internal fastcc void @xxx() unnamed_addr {
; CHECK-LABEL: define internal fastcc void @xxx(
; CHECK: call fastcc void @yyy(
;
entry:
  call fastcc void @yyy()
  ret void
}

define internal fastcc void @yyy() unnamed_addr {
; CHECK-LABEL: define internal fastcc void @yyy(
; CHECK: call fastcc void @xyz(
;
entry:
  call fastcc void @xyz()
  ret void
}

define internal fastcc void @xyz() unnamed_addr {
; CHECK-LABEL: define internal fastcc void @xyz(
; CHECK: call fastcc void @xxx(
; CHECK: call void @test.clone(
;
entry:
  call fastcc void @xxx()
  tail call void @test(ptr addrspace(1) null)
  ret void
}

define dso_local void @test2(ptr addrspace(1) %results) local_unnamed_addr !kernel_arg_base_type !1 !arg_type_null_val !2 {
; CHECK-LABEL: define dso_local void @test2(
; CHECK: call fastcc void @xyz(
;
entry:
  call fastcc void @xyz()
  ret void
}

define internal fastcc zeroext i1 @bar() unnamed_addr {
; CHECK-LABEL: define internal fastcc zeroext i1 @bar(
; CHECK: load double, ptr addrspace(3) @test3.s,
; CHECK: load double, ptr addrspace(3) @test3.t,
;
entry:
  %0 = load double, ptr addrspace(3) @test3.s, align 8
  %1 = load double, ptr addrspace(3) @test3.t, align 8
  ret i1 false
}

define dso_local void @test3(ptr addrspace(1) %results) local_unnamed_addr !kernel_arg_base_type !1 !arg_type_null_val !2 {
; CHECK-LABEL: define dso_local void @test3(
; CHECK: store double {{.*}}, ptr addrspace(3) @test3.s,
; CHECK: store double {{.*}}, ptr addrspace(3) @test3.t,
; CHECK: call fastcc zeroext i1 @bar()
; CHECK: call fastcc void @xyz.clone(
;
entry:
  store double 0.000000e+00, ptr addrspace(3) @test3.s, align 8
  store double 0.000000e+00, ptr addrspace(3) @test3.t, align 8
  %call4 = tail call fastcc zeroext i1 @bar()
  call fastcc void @xyz()
  ret void
}

; CHECK-LABEL: define internal fastcc zeroext i1 @foo.clone(
; CHECK: load i32, ptr addrspace(3) @test.i.clone,
; CHECK: load i32, ptr addrspace(3) @test.j.clone,

; CHECK-LABEL: define internal void @test.clone(
; CHECK: store i32 {{.*}}, ptr addrspace(3) @test.i.clone,
; CHECK: store i32 {{.*}}, ptr addrspace(3) @test.j.clone,
; CHECK: call fastcc zeroext i1 @foo.clone()

; CHECK-LABEL: define internal fastcc zeroext i1 @foo.clone.clone(
; CHECK: load i32, ptr addrspace(3) @test.i.clone.{{.*}},
; CHECK: load i32, ptr addrspace(3) @test.j.clone.{{.*}},

; CHECK-LABEL: define internal void @test.clone.clone(
; CHECK: store i32 {{.*}}, ptr addrspace(3) @test.i.clone.{{.*}},
; CHECK: store i32 {{.*}}, ptr addrspace(3) @test.j.clone.{{.*}},
; CHECK: call fastcc zeroext i1 @foo.clone.clone()

; CHECK-LABEL: define internal fastcc void @xyz.clone(
; CHECK: call fastcc void @xxx.clone(
; CHECK: call void @test.clone.clone(

; CHECK-LABEL: define internal fastcc void @yyy.clone(
; CHECK: call fastcc void @xyz.clone(

; CHECK-LABEL: define internal fastcc void @xxx.clone(
; CHECK: call fastcc void @yyy.clone(

!sycl.kernels = !{!0}

!0 = !{ptr @test, ptr @test2, ptr @test3}
!1 = !{!"int*"}
!2 = !{ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
