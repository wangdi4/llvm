; RUN: opt -S -dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s
; RUN: opt -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s

; Test case for DTrans transformation base class implementation for opaque
; pointers. This test is to check rewriting types and dependent types.

; Simple case that does not involve opaque pointers
%struct.test01a = type { i32, i32, i32 }

; TODO: Add structure types involving pointers and type nesting

define void @test01() {
  %local = alloca %struct.test01a
  ret void;
}

; TODO: The base class does not support changing types yet. This test will be
; expanded as additional functionality is implemented. Right now, the test is
; just checking that the skeleton pass can be run.

; CHECK: %struct.test01a = type { i32, i32, i32 }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!intel.dtrans.types = !{!2}
