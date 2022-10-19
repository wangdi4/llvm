; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test pointer type recovery on load instructions of a variety of types.

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.


; The loads in this function generate types involving pointers, which
; need to be collected by the pointer type analyzer.
%struct.test01 = type { i16, i16 }
define internal void @test01() {
  %p32 = alloca i32*, !intel_dtrans_type !2
  %vp16 = alloca <2 x i16*>, !intel_dtrans_type !3
  %ap16 = alloca [2 x i16*], !intel_dtrans_type !5
  %litp = alloca {i16*, i16*}, !intel_dtrans_type !6

  %val_p32 = load i32*, i32** %p32
  %val_vp16 = load <2 x i16*>, <2 x i16*>* %vp16
  %val_ap16 = load [2 x i16*], [2 x i16*]* %ap16
  %val_litp9 = load { i16*, i16* }, { i16*, i16* }* %litp

  ret void
}
; CHECK-LABEL: define internal void @test01
; CHECK-NONOPAQUE: %val_p32 = load i32*, i32** %p32
; CHECK-OPAQUE: %val_p32 = load ptr, ptr %p32
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-NONOPAQUE:  %val_vp16 = load <2 x i16*>, <2 x i16*>* %vp16
; CHECK-OPAQUE:  %val_vp16 = load <2 x ptr>, ptr %vp16
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT: <2 x i16*>{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-NONOPAQUE: %val_ap16 = load [2 x i16*], [2 x i16*]* %ap16
; CHECK-OPAQUE: %val_ap16 = load [2 x ptr], ptr %ap16
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT: [2 x i16*]{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-NONOPAQUE: %val_litp9 = load { i16*, i16* }, { i16*, i16* }* %litp
; CHECK-OPAQUE: %val_litp9 = load { ptr, ptr }, ptr %litp
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT: { i16*, i16* }{{ *$}}
; CHECK-NEXT: No element pointees.


; The loads in this function generate types that do not involve pointers, and
; do not need to be collected by the pointer type analyzer.
define internal void @test02() {
  %s32 = alloca i32
  %v16 = alloca <2 x i16>
  %a16 = alloca [2 x i16]
  %st = alloca %struct.test01
  %lit = alloca {i16, i16}

  %val_32 = load i32, i32* %s32
  %val_v16 = load <2 x i16>, <2 x i16>* %v16
  %val_a16 = load [2 x i16], [2 x i16]* %a16
  %val_lit = load {i16, i16}, {i16, i16}* %lit

  ret void
}
; There should be no aliased type lists reported between the first 'load'
; and the 'ret'
; CHECK-LABEL: define internal void @test02
; CHECK: load i32
; CHECK-NOT: Aliased types
; CHECK: ret void


; The GEP in this case will be treated as being either a i8* or a [8 x i8]*, but
; the load should be just an i8, not a [8 x i8], so should not show up with an
; alias type.
@test_var03 = internal global [64 x [8 x i8]] zeroinitializer
define internal i8 @test03(i64 %table, i32 %idx) {
  %arr_ptr = getelementptr inbounds [64 x [8 x i8]], [64 x [8 x i8]]* @test_var03, i64 0, i64 %table
  %z_idx = zext i32 %idx to i64
  %ptr = getelementptr inbounds [8 x i8], [8 x i8]* %arr_ptr, i64 0, i64 %z_idx
  %val = load i8, i8* %ptr
  ret i8 %val
}
; There should be no aliased type lists reported between the 'load'
; and the 'ret'
; CHECK-LABEL: define internal i8 @test03
; CHECK: load i8
; CHECK-NOT: Aliased types
; CHECK: ret i8


!1 = !{i16 0, i32 0}  ; i16
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{!"V", i32 2, !4}  ; <2 x i16*>
!4 = !{i16 0, i32 1}  ; i16*
!5 = !{!"A", i32 2, !4}  ; [2 x i16*]
!6 = !{!"L", i32 2, !4, !4}  ; {i16*, i16*}
!7 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i16, i16 }

!intel.dtrans.types = !{!7}
