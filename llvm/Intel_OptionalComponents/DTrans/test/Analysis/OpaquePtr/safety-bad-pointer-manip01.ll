; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test byte-flattened GEP cases that result in "Bad pointer manipulation"

; Byte-flattened GEP that cannot be analyzed because the offset not a known
; constant. This needs to marked as "Bad pointer manipulation"
%struct.test01 = type { i32, i64, i32 }
define internal void @test01(i32 %x, i32 %y) {
  %local = alloca %struct.test01
  %flat = bitcast %struct.test01* %local to i8*
  %offset = select i1 undef, i32 %x, i32 %y
  %faddr = getelementptr i8, i8* %flat, i32 %offset
  %addr = bitcast i8* %faddr to i32*
  store i32 0, i32* %addr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad pointer manipulation | Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test01


; Byte-flattened GEP that does not align to the start of a field, and is not
; for a memset. This needs to marked as "Bad pointer manipulation"
%struct.test02 = type { i32, i32, i32 }
define internal void @test02(i32 %x) {
  %local = alloca %struct.test02
  %flat = bitcast %struct.test02* %local to i8*
  %faddr = getelementptr i8, i8* %flat, i32 2
  %addr = bitcast i8* %faddr to i16*
  store i16 0, i16* %addr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad pointer manipulation | Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test02


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }
!4 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!intel.dtrans.types = !{!3, !4}
