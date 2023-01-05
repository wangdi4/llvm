; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -disable-output -dtrans-print-types 2>&1 | FileCheck %s

; This test case checks that the field 0 in %struct.test.b is marked as "Read"
; since the load in %tmp4 is loading the zero element.

; CHECK:  LLVMType: %struct.test.b = type { i32, i32, i32, i32 }
; CHECK:  0)Field LLVM Type: i32
; CHECK:    Field info: Read UnusedValue NonGEPAccess{{ *$}}
; CHECK:    Readers: test{{ *$}}
; CHECK:  1)Field LLVM Type: i32
; CHECK:    Field info:{{ *$}}
; CHECK:    Readers:{{ *$}}
; CHECK:  2)Field LLVM Type: i32
; CHECK:    Field info:{{ *$}}
; CHECK:    Readers:{{ *$}}
; CHECK:  3)Field LLVM Type: i32
; CHECK:    Field info:{{ *$}}
; CHECK:    Readers:{{ *$}}

%struct.test.a = type { i32, i32, ptr }
%struct.test.b = type { i32, i32, i32, i32 }

define void @test(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !5  {
  %tmp1 = getelementptr %struct.test.a, ptr %p, i64 0, i32 2
  %tmp2 = bitcast ptr %tmp1 to ptr
  %tmp3 = load ptr, ptr %tmp2, align 8
  %tmp4 = load i32, ptr %tmp3, align 4
  ret void
}

!intel.dtrans.types = !{!1, !3}

!0 = !{i32 0, i32 0}
!1 = !{!"S", %struct.test.b zeroinitializer, i32 4, !0, !0, !0, !0}
!2 = !{%struct.test.b zeroinitializer, i32 1}
!3 = !{!"S", %struct.test.a zeroinitializer, i32 3, !0, !0, !2}
!4 = !{%struct.test.a zeroinitializer, i32 1}

!5 = distinct !{!4}
