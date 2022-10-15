; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -debug-only=dtrans-safetyanalyzer -disable-output %s 2>&1 | FileCheck %s

; This test checks that the safety analyzer detects that pointers which
; are not in the default address space are used causing the DTrans safety
; info object to inhibit DTrans transformations.

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

; CHECK: DTransSafetyInfo: Unsupported address space seen
