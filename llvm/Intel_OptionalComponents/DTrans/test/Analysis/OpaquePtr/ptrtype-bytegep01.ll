; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test pointer type recovery on byte flattened GEPs

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.


; Include a data layout so that padding will be inserted to align structure
; field members.
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Access to element of non-nested structure
%struct.test01 = type { i32, i64*, i32, i16, i16 } ; Offsets: 0, 8, 16, 20, 22
define internal void @test01() {
  %local = alloca %struct.test01
  %flat = bitcast %struct.test01* %local to i8*

  %addr0 = getelementptr i8, i8* %flat, i32 0
  %addr1 = getelementptr i8, i8* %flat, i32 8
  %addr2 = getelementptr i8, i8* %flat, i32 16
  %addr3 = getelementptr i8, i8* %flat, i32 20
  %addr4 = getelementptr i8, i8* %flat, i32 22
  call void @test01helper(i8* %addr0, i8* %addr1, i8* %addr2, i8* %addr3, i8* %addr4)
  ret void
}
define internal void @test01helper(i8* "intel_dtrans_func_index"="1" %arg0, i8* "intel_dtrans_func_index"="2" %arg1, i8* "intel_dtrans_func_index"="3" %arg2, i8* "intel_dtrans_func_index"="4" %arg3, i8* "intel_dtrans_func_index"="5" %arg4) !intel.dtrans.func.type !5 {
  ret void
}
; CHECK-LABEL: void @test01()
; CHECK-NONOPAQUE: %addr0 = getelementptr i8, i8* %flat, i32 0
; CHECK-OPAQUE: %addr0 = getelementptr i8, ptr %flat, i32 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test01 @ 0

; CHECK-NONOPAQUE: %addr1 = getelementptr i8, i8* %flat, i32 8
; CHECK-OPAQUE: %addr1 = getelementptr i8, ptr %flat, i32 8
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64**{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test01 @ 1

; CHECK-NONOPAQUE: %addr2 = getelementptr i8, i8* %flat, i32 16
; CHECK-OPAQUE: %addr2 = getelementptr i8, ptr %flat, i32 16
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test01 @ 2

; CHECK-NONOPAQUE: %addr3 = getelementptr i8, i8* %flat, i32 20
; CHECK-OPAQUE: %addr3 = getelementptr i8, ptr %flat, i32 20
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i16*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test01 @ 3

; CHECK-NONOPAQUE: %addr4 = getelementptr i8, i8* %flat, i32 22
; CHECK-OPAQUE: %addr4 = getelementptr i8, ptr %flat, i32 22
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i16*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test01 @ 4


; Access to elements where indices need to be recovered based on constant
; values in the select instruction.
%struct.test02 = type { i32, i64, i32 } ; Offsets: 0, 8, 16
define internal void @test02() {
  %local = alloca %struct.test02
  %flat = bitcast %struct.test02* %local to i8*
  %offset = select i1 undef, i32 0, i32 16
  %faddr = getelementptr i8, i8* %flat, i32 %offset
  %addr = bitcast i8* %faddr to i32*
  store i32 0, i32* %addr
  ret void
}
; CHECK-LABEL: void @test02()
; CHECK-NONOPAQUE:  %faddr = getelementptr i8, i8* %flat, i32 %offset
; CHECK-OPAQUE:  %faddr = getelementptr i8, ptr %flat, i32 %offset
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test02 @ 0
; CHECK-NEXT:   %struct.test02 @ 2


; Non-constant index value that cannot be resolved to a byte-flattened GEP
; access. In this case, GEP should be marked as computing an unknown byte
; flattened address so the safety analyzer can mark a safety violation for the
; type. The "aliased results" will just contain i8*, and not the original
; pointer type of the GEP operand because any memory accesses using the
; instruction result are accessing the member of the aggregate. Keeping the
; aggregate pointer on the result just results in redundant safety violations
; for the type, rather than the root safety issue of not being able to analyze
; the GEP.
%struct.test03 = type { i32, i64, i32 } ; Offsets: 0, 8, 16
define internal void @test03(i32 %x, i32 %y) {
  %local = alloca %struct.test03
  %flat = bitcast %struct.test03* %local to i8*
  %offset = select i1 undef, i32 %x, i32 %y
  %faddr = getelementptr i8, i8* %flat, i32 %offset
  %addr = bitcast i8* %faddr to i32*
  store i32 0, i32* %addr
  ret void
}
; CHECK-LABEL: void @test03(i32 %x, i32 %y)
; CHECK-NONOPAQUE:  %faddr = getelementptr i8, i8* %flat, i32 %offset
; CHECK-OPAQUE:  %faddr = getelementptr i8, ptr %flat, i32 %offset
; CHECK-NEXT: LocalPointerInfo:
; CHECK-SAME: UNKNOWN BYTE FLATTENED GEP
; CHECK-NEXT:  Aliased types:
; CHECK-NEXT:    i8*{{ *$}}
; CHECK-NEXT:  No element pointees.


; Test where offsets do not match field offsets.
%struct.test04 = type { i32, i64, i32 } ; Offsets: 0, 8, 16
define internal void @test04() {
  %local = alloca %struct.test04
  %flat = bitcast %struct.test04* %local to i8*
  %offset = select i1 undef, i32 0, i32 12
  %faddr = getelementptr i8, i8* %flat, i32 %offset
  %addr = bitcast i8* %faddr to i32*
  store i32 0, i32* %addr
  ret void
}
; CHECK-LABEL: void @test04()
; CHECK-NONOPAQUE: %faddr = getelementptr i8, i8* %flat, i32 %offset
; CHECK-OPAQUE: %faddr = getelementptr i8, ptr %flat, i32 %offset
; CHECK-NEXT: LocalPointerInfo:
; CHECK-SAME: UNKNOWN BYTE FLATTENED GEP
; CHECK-NEXT:  Aliased types:
; CHECK-NEXT:    i8*{{ *$}}
; CHECK-NEXT:  No element pointees.


; Access to element of nested structure
%struct.test05a = type { i16, i32 } ; Offset: 0, 4
%struct.test05b = type { i16, %struct.test05a } ; Offsets 0, 4
define internal void @test05() {
  %local = alloca %struct.test05b
  %flat = bitcast %struct.test05b* %local to i8*
  %faddr = getelementptr i8, i8* %flat, i64 8

  %addr = bitcast i8* %faddr to i32*
  store i32 0, i32* %addr
  ret void
}
; CHECK-LABEL: void @test05()
; CHECK-NONOPAQUE: %faddr = getelementptr i8, i8* %flat, i64 8
; CHECK-OPAQUE: %faddr = getelementptr i8, ptr %flat, i64 8
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:  Aliased types:
; CHECK-NEXT:    i32*{{ *$}}
; CHECK-NEXT:  Element pointees:
; CHECK-NEXT:    %struct.test05a @ 1


; Access into an array of non-i8 types
define internal void @test06() {
  %local = alloca [32767 x i16]
  %flat = bitcast [32767 x i16]* %local to i8*
  %faddr = getelementptr i8, i8* %flat, i64 2
  %half = load i8, i8* %faddr
  ret void
}
; CHECK-LABEL: void @test06()
; CHECK-NONOPAQUE: %faddr = getelementptr i8, i8* %flat, i64 2
; CHECK-OPAQUE: %faddr = getelementptr i8, ptr %flat, i64 2
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:    i16*{{ *$}}
; CHECK-NEXT:    i8*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   [32767 x i16] @ 1


; Access into an array of non-i8 types with a non-constant index
define internal void @test07(i32 %idx) {
  %local = alloca [32767 x i16]
  %flat = bitcast [32767 x i16]* %local to i8*
  %faddr = getelementptr i8, i8* %flat, i32 %idx
  %half = load i8, i8* %faddr
  ret void
}
; CHECK-LABEL: void @test07(i32 %idx) {
; CHECK-NONOPAQUE: %faddr = getelementptr i8, i8* %flat, i32 %idx
; CHECK-OPAQUE: %faddr = getelementptr i8, ptr %flat, i32 %idx
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   [32767 x i16]*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


; Access into an array contained within a structure.
; Note: Neither the DTrans local pointer analyzer and pointer type analyzer
; record anything about the memory location being contained within a structure.
; Therefore, structure transformations cannot be applied that would change the
; address of an array element within a structure.
%struct.test08 = type { i32, [3 x i64] } ; Offsets: 0, [8, 16, 24]
define internal void @test08() {
  %local = alloca %struct.test08
  %flat = bitcast %struct.test08* %local to i8*
  %faddr = getelementptr i8, i8* %flat, i64 24
  %addr = bitcast i8* %faddr to i64*
  store i64 0, i64* %addr
  ret void
}
; CHECK-LABEL: void @test08()
; CHECK-NONOPAQUE: %faddr = getelementptr i8, i8* %flat, i64 24
; CHECK-OPAQUE: %faddr = getelementptr i8, ptr %flat, i64 24
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   [3 x i64] @ 2


; Access into a structure contained within an array with a constant offset
%struct.test09 = type { i32, i64, i32 } ; Size = 24
define internal void @test09() {
  %local = alloca [10 x %struct.test09]
  %flat = bitcast [10 x %struct.test09]* %local to i8*

  ; i64 field of 2nd array element
  %faddr = getelementptr i8, i8* %flat, i64 56
  %addr = bitcast i8* %faddr to i64*
  store i64 0, i64* %addr
  ret void
}
; CHECK-LABEL: void @test09()
; CHECK-NONOPAQUE: %faddr = getelementptr i8, i8* %flat, i64 56
; CHECK-OPAQUE: %faddr = getelementptr i8, ptr %flat, i64 56
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test09 @ 1


; Access into a structure contained within an array with a non-constant offset
%struct.test10 = type { i32, i64, i32 } ; Size = 24
define internal void @test10(i64 %idx) {
  %local = alloca [10 x %struct.test10]
  %flat = bitcast [10 x %struct.test10]* %local to i8*

  ; access unknown field
  %faddr = getelementptr i8, i8* %flat, i64 %idx
  %addr = bitcast i8* %faddr to i32*
  store i32 0, i32* %addr
  ret void
}
; CHECK-LABEL: void @test10(i64 %idx)
; CHECK-NONOPAQUE: %faddr = getelementptr i8, i8* %flat, i64 %idx
; CHECK-OPAQUE: %faddr = getelementptr i8, ptr %flat, i64 %idx
; CHECK-NEXT: LocalPointerInfo:
; CHECK-SAME: UNKNOWN BYTE FLATTENED GEP
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 1}  ; i64*
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4, !4, !4, !4, !4}
!6 = !{i64 0, i32 0}  ; i64
!7 = !{%struct.test05a zeroinitializer, i32 0}  ; %struct.test05a
!8 = !{!"A", i32 3, !6}  ; [3 x i64]
!9 = !{!"S", %struct.test01 zeroinitializer, i32 5, !1, !2, !1, !3, !3} ; { i32, i64*, i32, i16, i16 }
!10 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !6, !1} ; { i32, i64, i32 }
!11 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !6, !1} ; { i32, i64, i32 }
!12 = !{!"S", %struct.test04 zeroinitializer, i32 3, !1, !6, !1} ; { i32, i64, i32 }
!13 = !{!"S", %struct.test05a zeroinitializer, i32 2, !3, !1} ; { i16, i32 }
!14 = !{!"S", %struct.test05b zeroinitializer, i32 2, !3, !7} ; { i16, %struct.test05a }
!15 = !{!"S", %struct.test08 zeroinitializer, i32 2, !1, !8} ; { i32, [3 x i64] }
!16 = !{!"S", %struct.test09 zeroinitializer, i32 3, !1, !6, !1} ; { i32, i64, i32 }
!17 = !{!"S", %struct.test10 zeroinitializer, i32 3, !1, !6, !1} ; { i32, i64, i32 }

!intel.dtrans.types = !{!9, !10, !11, !12, !13, !14, !15, !16, !17}
