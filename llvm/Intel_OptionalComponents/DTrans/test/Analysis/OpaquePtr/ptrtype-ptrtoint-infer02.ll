; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; This test is to verify that the inference for ptrtoint instructions does not
; result in a non-pointer type being inferred for the pointer argument.

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; This pattern was seen in benchmark code where memory allocated for the first
; field of a structure was stored directly to the address of the structure
; element.
%struct.test01 = type { i32*, i32* }
@var01 = internal global %struct.test01* null, !intel_dtrans_type !2
define internal void @test01() {
  %mem_struct = call i8* @malloc(i64 16)
  %struct = bitcast i8* %mem_struct to %struct.test01*
  ; Use pointer to establish that %mem_struct% & struct are %struct.test01*
  store %struct.test01* %struct, %struct.test01** @var01

  ; Allocation of a bunch of i32 objects.
  %mem01 = call i8* @malloc(i64 128)

  ; Save the i32 objects into the first element of the structure.
  %pti = ptrtoint i8* %mem01 to i64
  %mem_struct.as.p64 = bitcast i8* %mem_struct to i64*
  store i64 %pti, i64* %mem_struct.as.p64

  ret void
}
; CHECK-LABEL: define internal void @test01()
; CHECK-NONOPAQUE: %mem01 = call i8* @malloc(i64 128)
; CHECK-OPAQUE: %mem01 = call ptr @malloc(i64 128)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


; Test with nested types.
%struct.test02a = type { %struct.test02b, i32* }
%struct.test02b = type { [4 x %struct.test02c] }
%struct.test02c = type { i32*, i32* }
@var02a = internal global %struct.test02a* null, !intel_dtrans_type !6
define internal void @test02() {
  %mem_struct = call i8* @malloc(i64 72)
  %struct = bitcast i8* %mem_struct to %struct.test02a*
  ; Use pointer to establish that %mem_struct & %struct are %struct.test02a*
  store %struct.test02a* %struct, %struct.test02a** @var02a

  ; Allocation of a bunch of i32 objects.
  %mem02 = call i8* @malloc(i64 128)

  ; Save the i32 objects into the first element of the structure.
  %pti = ptrtoint i8* %mem02 to i64
  %mem_struct.as.p64 = bitcast i8* %mem_struct to i64*
  store i64 %pti, i64* %mem_struct.as.p64

  ret void
}
; CHECK-LABEL: define internal void @test02()
; CHECK-NONOPAQUE: %mem02 = call i8* @malloc(i64 128)
; CHECK-OPAQUE: %mem02 = call ptr @malloc(i64 128)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


; Test with a pointer to structure
%struct.test03a = type { %struct.test03b*, i32* }
%struct.test03b = type { i32, i32 }
@var03a = internal global %struct.test03a* null, !intel_dtrans_type !9
define internal void @test03() {
  %mem_struct = call i8* @malloc(i64 16)
  %struct = bitcast i8* %mem_struct to %struct.test03a*
  ; Use pointer to establish that %mem_struct & %struct are %struct.test03a*
  store %struct.test03a* %struct, %struct.test03a** @var03a

  ; Allocation of a %struct.test03b object.
  %mem03 = call i8* @malloc(i64 8)

  ; Save the i32 objects into the first element of the structure.
  %pti = ptrtoint i8* %mem03 to i64
  %mem_struct.as.p64 = bitcast i8* %mem_struct to i64*
  store i64 %pti, i64* %mem_struct.as.p64

  ret void
}
; CHECK-LABEL: define internal void @test03()
; CHECK-NONOPAQUE: %mem03 = call i8* @malloc(i64 8)
; CHECK-OPAQUE: %mem03 = call ptr @malloc(i64 8)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test03b*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


declare !intel.dtrans.func.type !11 "intel_dtrans_func_index"="1" i8* @malloc(i64)

!1 = !{i32 0, i32 1}  ; i32*
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{%struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!4 = !{!"A", i32 4, !5}  ; [4 x %struct.test02c]
!5 = !{%struct.test02c zeroinitializer, i32 0}  ; %struct.test02c
!6 = !{%struct.test02a zeroinitializer, i32 1}  ; %struct.test02a*
!7 = !{%struct.test03b zeroinitializer, i32 1}  ; %struct.test03b*
!8 = !{i32 0, i32 0}  ; i32
!9 = !{%struct.test03a zeroinitializer, i32 1}  ; %struct.test03a*
!10 = !{i8 0, i32 1}  ; i8*
!11 = distinct !{!10}
!12 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32*, i32* }
!13 = !{!"S", %struct.test02a zeroinitializer, i32 2, !3, !1} ; { %struct.test02b, i32* }
!14 = !{!"S", %struct.test02b zeroinitializer, i32 1, !4} ; { [4 x %struct.test02c] }
!15 = !{!"S", %struct.test02c zeroinitializer, i32 2, !1, !1} ; { i32*, i32* }
!16 = !{!"S", %struct.test03a zeroinitializer, i32 2, !7, !1} ; { %struct.test03b*, i32* }
!17 = !{!"S", %struct.test03b zeroinitializer, i32 2, !8, !8} ; { i32, i32 }

!intel.dtrans.types = !{!12, !13, !14, !15, !16, !17}
