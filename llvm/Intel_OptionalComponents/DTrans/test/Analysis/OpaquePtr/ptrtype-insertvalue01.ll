; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test pointer type recovery on insertvalue instructions

%struct.test01a = type { i32, i32 }

; Check that UNHANDLED is not marked for insertvalue.
define void @test01(ptr "intel_dtrans_func_index"="1" %p1, i32 %i1) personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !4 {
  %iv1 = insertvalue { ptr, i32 } undef, ptr %p1, 0
  %iv2 = insertvalue { ptr, i32 } %iv1, i32 %i1, 1
  resume { ptr, i32 } %iv2
  ret void
}

; CHECK-LABEL: void @test01
; CHECK: %iv1 = insertvalue { ptr, i32 } undef, ptr %p1, 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   { i8*, i32 } = type { i8*, i32 }
; CHECK-NEXT: No element pointees

; CHECK: %iv2 = insertvalue { ptr, i32 } %iv1, i32 %i1, 1
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   { i8*, i32 } = type { i8*, i32 }
; CHECK-NEXT: No element pointees


; Check that UNHANDLED is not marked for insertvalue.
; Handles constant value operands.
define void @test02() personality ptr @__gxx_personality_v0 {
  %iv1 = insertvalue { ptr, i32 } undef, ptr null, 0
  %iv2 = insertvalue { ptr, i32 } %iv1, i32 30, 1
  resume { ptr, i32 } %iv2
  ret void
}

; CHECK-LABEL: void @test02
; CHECK: %iv1 = insertvalue { ptr, i32 } undef, ptr null, 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   { i8*, i32 } = type { i8*, i32 }
; CHECK-NEXT: No element pointees

; CHECK: %iv2 = insertvalue { ptr, i32 } %iv1, i32 30, 1
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   { i8*, i32 } = type { i8*, i32 }
; CHECK-NEXT: No element pointees


; Check that UNHANDLED is marked for %iv1 since alias of %bc is not i8*.
define void @test03(ptr "intel_dtrans_func_index"="1" %pStructA) personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !4 {
  %addr1 = getelementptr %struct.test01a, ptr %pStructA, i64 0, i32 0
  %val1 = load i32, ptr %addr1
  %bc = bitcast ptr %pStructA to ptr
  %iv1 = insertvalue { ptr, i32 } undef, ptr %bc, 0
  %iv2 = insertvalue { ptr, i32 } %iv1, i32 %val1, 1
  resume { ptr, i32 } %iv2
  ret void
}

; CHECK-LABEL: void @test03
; CHECK: %iv1 = insertvalue { ptr, i32 } undef, ptr %bc, 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK: UNHANDLED

; CHECK: %iv2 = insertvalue { ptr, i32 } %iv1, i32 %val1, 1
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   { i8*, i32 } = type { i8*, i32 }
; CHECK-NEXT: No element pointees

; Check that UNHANDLED is marked for %iv1 & %iv2 since type is not {i8*, i32}.
define void @test04() {
  %iv1 = insertvalue { ptr, ptr } undef, ptr null, 0
  %iv2 = insertvalue { ptr, ptr } %iv1, ptr null, 1
  %ev1 = extractvalue { ptr, ptr } %iv2, 1
  ret void
}

; CHECK-LABEL: void @test04
; CHECK: %iv1 = insertvalue { ptr, ptr } undef, ptr null, 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK: UNHANDLED

; CHECK: %iv2 = insertvalue { ptr, ptr } %iv1, ptr null, 1
; CHECK-NEXT: LocalPointerInfo:
; CHECK: UNHANDLED

; Check that UNHANDLED is marked for %iv1 & %iv2 since %iv1 and %iv2 are
; not transitively used by ResumeInst.
define void @test05()  personality ptr @__gxx_personality_v0 {
  %iv1 = insertvalue { ptr, i32 } undef, ptr null, 0
  %iv2 = insertvalue { ptr, i32 } %iv1, i32 20, 1
  %ev1 = extractvalue { ptr, i32 } %iv2, 0
  ret void
}

; CHECK-LABEL: void @test05
; CHECK: %iv1 = insertvalue { ptr, i32 } undef, ptr null, 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK: UNHANDLED

; CHECK: %iv2 = insertvalue { ptr, i32 } %iv1, i32 20, 1
; CHECK-NEXT: LocalPointerInfo:
; CHECK: UNHANDLED


declare i32 @__gxx_personality_v0(...)
declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" ptr @_Znwm(i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!3 = !{i8 0, i32 1}  ; i8*
!4 = distinct !{!3}
!5 = distinct !{!2}
!6 = distinct !{!3}
!7 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!7}

