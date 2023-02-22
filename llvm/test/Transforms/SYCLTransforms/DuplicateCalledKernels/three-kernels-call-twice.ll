; RUN: opt -opaque-pointers=0 -passes=sycl-kernel-duplicate-called-kernels %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers=0 -passes=sycl-kernel-duplicate-called-kernels %s -S | FileCheck %s

; Check the case that kernel 'test' is called in two kernels: 'test2' and
; 'test3'. So 'test/foo' and their associated local variables are cloned twice.
;
;    test2    test3
;       \     /  \
;        \   /   bar
;        test
;          |
;         foo

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@test.i = internal addrspace(3) global [100 x i32] undef, align 4
@test.j = internal addrspace(3) global i32 undef, align 4
@test3.s = internal addrspace(3) global [10 x double] undef, align 8
@test3.t = internal addrspace(3) global double undef, align 8

define internal fastcc zeroext i1 @foo() unnamed_addr {
; CHECK-LABEL: define internal fastcc zeroext i1 @foo(
; CHECK: load i32, i32 addrspace(3)* getelementptr inbounds ([100 x i32], [100 x i32] addrspace(3)* @test.i,
; CHECK: load i32, i32 addrspace(3)* @test.j,
;
entry:
  %0 = load i32, i32 addrspace(3)* getelementptr inbounds ([100 x i32], [100 x i32] addrspace(3)* @test.i, i64 0, i64 0), align 4
  %1 = load i32, i32 addrspace(3)* @test.j, align 4
  ret i1 false
}

define dso_local void @test(i32 addrspace(1)* %results) local_unnamed_addr {
; CHECK-LABEL: define dso_local void @test(
; CHECK: store i32 {{.*}}, i32 addrspace(3)* getelementptr inbounds ([100 x i32], [100 x i32] addrspace(3)* @test.i,
; CHECK: store i32 {{.*}}, i32 addrspace(3)* @test.j,
; CHECK: call fastcc zeroext i1 @foo()
;
entry:
  store i32 0, i32 addrspace(3)* getelementptr inbounds ([100 x i32], [100 x i32] addrspace(3)* @test.i, i64 0, i64 0), align 4
  store i32 0, i32 addrspace(3)* @test.j, align 4
  %call2 = tail call fastcc zeroext i1 @foo()
  ret void
}

define dso_local void @test2(i32 addrspace(1)* %results) local_unnamed_addr {
; CHECK-LABEL: define dso_local void @test2(
; CHECK: call void @test.clone(
;
entry:
  tail call void @test(i32 addrspace(1)* null)
  ret void
}

define internal fastcc zeroext i1 @bar() unnamed_addr {
; CHECK-LABEL: define internal fastcc zeroext i1 @bar(
; CHECK: load double, double addrspace(3)* getelementptr inbounds ([10 x double], [10 x double] addrspace(3)* @test3.s,
; CHECK: load double, double addrspace(3)* @test3.t,
;
entry:
  %0 = load double, double addrspace(3)* getelementptr inbounds ([10 x double], [10 x double] addrspace(3)* @test3.s, i64 0, i64 0), align 8
  %1 = load double, double addrspace(3)* @test3.t, align 8
  ret i1 false
}

define dso_local void @test3(i32 addrspace(1)* %results) local_unnamed_addr {
; CHECK-LABEL: define dso_local void @test3(
; CHECK: store double {{.*}}, double addrspace(3)* getelementptr inbounds ([10 x double], [10 x double] addrspace(3)* @test3.s,
; CHECK: store double {{.*}}, double addrspace(3)* @test3.t,
; CHECK: call fastcc zeroext i1 @bar()
; CHECK: call void @test.clone.clone(
;
entry:
  store double 0.000000e+00, double addrspace(3)* getelementptr inbounds ([10 x double], [10 x double] addrspace(3)* @test3.s, i64 0, i64 0), align 8
  store double 0.000000e+00, double addrspace(3)* @test3.t, align 8
  %call4 = tail call fastcc zeroext i1 @bar()
  tail call void @test(i32 addrspace(1)* null)
  ret void
}

; CHECK-LABEL: define internal fastcc zeroext i1 @foo.clone(
; CHECK: load i32, i32 addrspace(3)* getelementptr inbounds ([100 x i32], [100 x i32] addrspace(3)* @test.i.clone,
; CHECK: load i32, i32 addrspace(3)* @test.j.clone,

; CHECK-LABEL: define internal void @test.clone(
; CHECK: store i32 {{.*}}, i32 addrspace(3)* getelementptr inbounds ([100 x i32], [100 x i32] addrspace(3)* @test.i.clone,
; CHECK: store i32 {{.*}}, i32 addrspace(3)* @test.j.clone,
; CHECK: call fastcc zeroext i1 @foo.clone()

; CHECK-LABEL: define internal fastcc zeroext i1 @foo.clone.clone(
; CHECK: load i32, i32 addrspace(3)* getelementptr inbounds ([100 x i32], [100 x i32] addrspace(3)* @test.i.clone.{{.*}},
; CHECK: load i32, i32 addrspace(3)* @test.j.clone.{{.*}},

; CHECK-LABEL: define internal void @test.clone.clone(
; CHECK: store i32 {{.*}}, i32 addrspace(3)* getelementptr inbounds ([100 x i32], [100 x i32] addrspace(3)* @test.i.clone.{{.*}},
; CHECK: store i32 {{.*}}, i32 addrspace(3)* @test.j.clone.{{.*}},
; CHECK: call fastcc zeroext i1 @foo.clone.clone()

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*)* @test, void (i32 addrspace(1)*)* @test2, void (i32 addrspace(1)*)* @test3}

; DEBUGIFY-NOT: WARNING
