; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-CUR

; This test is to verify that the inference for ptrtoint instructions does not
; result in a non-pointer type being inferred for the pointer argument.

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; This pattern was seen in benchmark code where memory allocated for the first
; field of a structure was stored directly to the address of the structure
; element.
%struct.test01 = type { i32*, i32* }
@var01 = internal global %struct.test01* null, !dtrans_type !2
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
; CHECK-CUR: %mem01 = call i8* @malloc(i64 128)
; CHECK-FUT: %mem01 = call p0 @malloc(i64 128)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


; Test with nested types.
%struct.test02a = type { %struct.test02b, i32* }
%struct.test02b = type { [4 x %struct.test02c] }
%struct.test02c = type { i32*, i32* }
@var02a = internal global %struct.test02a* null, !dtrans_type !7
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
; CHECK-CUR: %mem02 = call i8* @malloc(i64 128)
; CHECK-FUT: %mem02 = call p0 @malloc(i64 128)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


; Test with a pointer to structure
%struct.test03a = type { %struct.test03b*, i32* }
%struct.test03b = type { i32, i32 }
@var03a = internal global %struct.test03a* null, !dtrans_type !12
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
; CHECK-CUR: %mem03 = call i8* @malloc(i64 8)
; CHECK-FUT: %mem03 = call p0 @malloc(i64 8)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test03b*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


declare i8* @malloc(i64)

!1 = !{i32 0, i32 1}  ; i32*
!2 = !{!3, i32 1}  ; %struct.test01*
!3 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!4 = !{!"R", %struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!5 = !{!"A", i32 4, !6}  ; [4 x %struct.test02c]
!6 = !{!"R", %struct.test02c zeroinitializer, i32 0}  ; %struct.test02c
!7 = !{!8, i32 1}  ; %struct.test02a*
!8 = !{!"R", %struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!9 = !{!10, i32 1}  ; %struct.test03b*
!10 = !{!"R", %struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!11 = !{i32 0, i32 0}  ; i32
!12 = !{!13, i32 1}  ; %struct.test03a*
!13 = !{!"R", %struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!14 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32*, i32* }
!15 = !{!"S", %struct.test02a zeroinitializer, i32 2, !4, !1} ; { %struct.test02b, i32* }
!16 = !{!"S", %struct.test02b zeroinitializer, i32 1, !5} ; { [4 x %struct.test02c] }
!17 = !{!"S", %struct.test02c zeroinitializer, i32 2, !1, !1} ; { i32*, i32* }
!18 = !{!"S", %struct.test03a zeroinitializer, i32 2, !9, !1} ; { %struct.test03b*, i32* }
!19 = !{!"S", %struct.test03b zeroinitializer, i32 2, !11, !11} ; { i32, i32 }

!dtrans_types = !{!14, !15, !16, !17, !18, !19}
