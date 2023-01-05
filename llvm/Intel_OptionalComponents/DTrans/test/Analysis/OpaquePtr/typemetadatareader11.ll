; REQUIRES: asserts
; RUN: not --crash opt -opaque-pointers -passes=dtrans-typemetadatareader -disable-output < %s 2>&1 | FileCheck %s

; Test that invalid DTrans type metadata is caught by the metadata reader.
; In the assertion enabled builds, this should trigger an error message.

; CHECK: Expected metadata constant

target triple = "x86_64-unknown-linux-gnu"

%struct.test = type { i32, i32 }

define void @test(ptr "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !2 {
  ret void
}

!intel.dtrans.types = !{ !0 }
 
!0 = !{!"S", %struct.test zeroinitializer, i32 2, !1, !1 } ; %struct.test01
!1 = !{i32 0, i32 0 }  ; i32

; The metadata for this function is intentionally invalid to check that the
; error triggers an assertion in the builds with assertions enabled when
; the 2nd field of the DTrans metadata is not an integer.
!2 = distinct !{!3}
!3 = !{i32 0, %struct.test zeroinitializer}
