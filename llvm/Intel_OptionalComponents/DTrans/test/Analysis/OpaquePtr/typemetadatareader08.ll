; REQUIRES: asserts
; RUN: opt -opaque-pointers -passes=dtrans-typemetadatareader -dtrans-typemetadatareader-values -disable-output < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test checks that the DTrans type metadata reader can reconstruct
; missing types for types that do not involve any pointers when metadata
; is missing, and there are nested structure types.

%struct.test01 = type { i32, %struct.test02 }
%struct.test02 = type { %struct.test03, i32 }
%struct.test03 = type { i32 }

define void @test1() {
  %local = alloca %struct.test01
  ret void
}

!intel.dtrans.types = !{}

; CHECK: StructTypes

; CHECK: StructType: %struct.test01 = type { i32, %struct.test02 }
; CHECK: StructType: %struct.test02 = type { %struct.test03, i32 }
; CHECK: StructType: %struct.test03 = type { i32 }

; CHECK: dtrans-typemetadatareader: All structures types populated
