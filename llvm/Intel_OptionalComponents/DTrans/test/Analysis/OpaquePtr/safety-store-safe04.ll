; REQUIRES: asserts

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where element-pointee stored is from a byte-flattened GEP.

; Include a data layout so that padding will be inserted to align structure
; field members.
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test01 = type { i32, i64*, i32, i16, i16 } ; Offsets: 0, 8, 16, 20, 22
define internal void @test01() {
  %local = alloca %struct.test01
  %flat = bitcast %struct.test01* %local to i8*

  %addr0 = getelementptr i8, i8* %flat, i32 0
  %field0 = bitcast i8* %addr0 to i32*
  store i32 0, i32* %field0

  %addr2 = getelementptr i8, i8* %flat, i32 16
  %field2 = bitcast i8* %addr2 to i32*
  store i32 0, i32* %field2

  %addr4 = getelementptr i8, i8* %flat, i32 22
  %field4 = bitcast i8* %addr4 to i16*
  store i16 0, i16* %field4

  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK:   Field info: Written{{ *$}}
; CHECK: 1)Field LLVM Type: i64*
; CHECK:   Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: i32
; CHECK:   Field info: Written{{ *$}}
; CHECK: 3)Field LLVM Type: i16
; CHECK:   Field info:{{ *$}}
; CHECK: 4)Field LLVM Type: i16
; CHECK:   Field info: Written{{ *$}}
; CHECK: Safety data: Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test01


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 1}  ; i64*
!3 = !{i16 0, i32 0}  ; i16
!4 = !{!"S", %struct.test01 zeroinitializer, i32 5, !1, !2, !1, !3, !3} ; { i32, i64*, i32, i16, i16 }

!intel.dtrans.types = !{!4}
