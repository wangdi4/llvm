; REQUIRES: asserts
; RUN: opt -opaque-pointers -passes=dtrans-typemetadatareader -dtrans-typemetadatareader-values -disable-output < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test checks that a mismatch between the DTrans type metadata
; and the actual type of a structure in the IR is detected by the
; type metadata reader.
;
; This case declares %struct.test01 to contain a nested structure,
; but then intentionally incorrectly sets the type as a pointer
; to the structure in the metadata version.

%struct.test01 = type { i32, %struct.test02 }
%struct.test02 = type { %struct.test03, i32 }
%struct.test03 = type { i32 }

!intel.dtrans.types = !{ !0 }

!0 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2 } ; %struct.test01
!1 = !{i32 0, i32 0 }  ; i32
!2 = !{%struct.test02 zeroinitializer, i32 1} ; %struct.test02*

; CHECK: StructType: Metadata mismatch: %struct.test01 = type { i32, %struct.test02 }

; CHECK: dtrans-typemetadatareader: All structures types NOT populated
