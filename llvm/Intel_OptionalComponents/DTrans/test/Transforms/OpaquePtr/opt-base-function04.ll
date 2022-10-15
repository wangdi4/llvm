; RUN: opt -dtransop-allow-typed-pointers -S -dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that function cloning is done by the base class when the
; function parameter type is changed as a result of being a
; type dependent on another type change.

; Where opaque pointers are used, this test case is not interesting because
; it will not clone the function or change the type of %struct.test01b.

%struct.test01a = type { i32, i32, i32 }
%struct.test01b = type { i32, %struct.test01a* }

define void @test01caller() {
  %a = alloca %struct.test01b*, !intel_dtrans_type !3
  %p = load %struct.test01b*, %struct.test01b** %a
  call void @test01callee(%struct.test01b* %p)
  ret void
}

define void @test01callee(%struct.test01b* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !4 {
  ret void
}

; CHECK-LABEL: define void @test01caller
; CHECK: %a = alloca %__DDT_struct.test01b*
; CHECK: %p = load %__DDT_struct.test01b*, %__DDT_struct.test01b** %a
; CHECK: call void @test01callee.{{[0-9]+}}(%__DDT_struct.test01b* %p)

; CHECK-NOT: define void @test01callee(%struct.test01b*
; CHECK: define internal void @test01callee.1(%__DDT_struct.test01b*

!intel.dtrans.types = !{!5, !6}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!3 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!4 = distinct !{!3}
!5 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!6 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !2} ; { i32, %struct.test01a* }
