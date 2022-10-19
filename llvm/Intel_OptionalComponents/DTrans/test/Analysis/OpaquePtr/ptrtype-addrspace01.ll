; REQUIRES: asserts
; RUN: opt -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -debug-only=dtrans-pta -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -debug-only=dtrans-pta -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test checks that the pointer type analyzer detects pointers that
; are not in the default address space, as those types are not supported
; by DTrans.

%struct.test01 = type { i64, i64 }
@var = internal addrspace(5) global %struct.test01 zeroinitializer
define void @test01() {
  %f = getelementptr %struct.test01, %struct.test01 addrspace(5)* @var, i64 0, i32 1
  store i64 0, i64 addrspace(5)* %f
  ret void
}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }

!intel.dtrans.types = !{!2}

; CHECK: Unsupported address space seen:
