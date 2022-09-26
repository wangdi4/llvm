; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test pointer type recovery on insertvalue instructions

%struct.test01a = type { i32, i32 }

; Check that UNHANDLED is not marked for insertvalue.
define void @test01(i8* "intel_dtrans_func_index"="1" %p1, i32 %i1) personality i32 (...)* @__gxx_personality_v0 !intel.dtrans.func.type !4 {
  %iv1 = insertvalue { i8*, i32 } undef, i8* %p1, 0
  %iv2 = insertvalue { i8*, i32 } %iv1, i32 %i1, 1
  resume { i8*, i32 } %iv2
  ret void
}

; CHECK-LABEL: void @test01
; CHECK-NONOPAQUE: %iv1 = insertvalue { i8*, i32 } undef, i8* %p1, 0
; CHECK-OPAQUE: %iv1 = insertvalue { ptr, i32 } undef, ptr %p1, 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   { i8*, i32 } = type { i8*, i32 }
; CHECK-NEXT: No element pointees

; CHECK-NONOPAQUE: %iv2 = insertvalue { i8*, i32 } %iv1, i32 %i1, 1
; CHECK-OPAQUE: %iv2 = insertvalue { ptr, i32 } %iv1, i32 %i1, 1
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   { i8*, i32 } = type { i8*, i32 }
; CHECK-NEXT: No element pointees


; Check that UNHANDLED is not marked for insertvalue.
; Handles constant value operands.
define void @test02() personality i32 (...)* @__gxx_personality_v0 {
  %iv1 = insertvalue { i8*, i32 } undef, i8* null, 0
  %iv2 = insertvalue { i8*, i32 } %iv1, i32 30, 1
  resume { i8*, i32 } %iv2
  ret void
}

; CHECK-LABEL: void @test02
; CHECK-NONOPAQUE: %iv1 = insertvalue { i8*, i32 } undef, i8* null, 0
; CHECK-OPAQUE: %iv1 = insertvalue { ptr, i32 } undef, ptr null, 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   { i8*, i32 } = type { i8*, i32 }
; CHECK-NEXT: No element pointees

; CHECK-NONOPAQUE: %iv2 = insertvalue { i8*, i32 } %iv1, i32 30, 1
; CHECK-OPAQUE: %iv2 = insertvalue { ptr, i32 } %iv1, i32 30, 1
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   { i8*, i32 } = type { i8*, i32 }
; CHECK-NEXT: No element pointees


; Check that UNHANDLED is marked for %iv1 since alias of %bc is not i8*.
define void @test03(%struct.test01a* "intel_dtrans_func_index"="1" %pStructA) personality i32 (...)* @__gxx_personality_v0 !intel.dtrans.func.type !4 {
  %addr1 = getelementptr %struct.test01a, %struct.test01a* %pStructA, i64 0, i32 0
  %val1 = load i32, i32* %addr1
  %bc = bitcast %struct.test01a* %pStructA to i8*
  %iv1 = insertvalue { i8*, i32 } undef, i8* %bc, 0
  %iv2 = insertvalue { i8*, i32 } %iv1, i32 %val1, 1
  resume { i8*, i32 } %iv2
  ret void
}

; CHECK-LABEL: void @test03
; CHECK-NONOPAQUE: %iv1 = insertvalue { i8*, i32 } undef, i8* %bc, 0
; CHECK-OPAQUE: %iv1 = insertvalue { ptr, i32 } undef, ptr %bc, 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK: UNHANDLED

; CHECK-NONOPAQUE: %iv2 = insertvalue { i8*, i32 } %iv1, i32 %val1, 1
; CHECK-OPAQUE: %iv2 = insertvalue { ptr, i32 } %iv1, i32 %val1, 1
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   { i8*, i32 } = type { i8*, i32 }
; CHECK-NEXT: No element pointees

; Check that UNHANDLED is marked for %iv1 & %iv2 since type is not {i8*, i32}.
define void @test04() {
  %iv1 = insertvalue { i8*, i8* } undef, i8* null, 0
  %iv2 = insertvalue { i8*, i8* } %iv1, i8* null, 1
  %ev1 = extractvalue { i8*, i8* } %iv2, 1
  ret void
}

; CHECK-LABEL: void @test04
; CHECK-NONOPAQUE: %iv1 = insertvalue { i8*, i8* } undef, i8* null, 0
; CHECK-OPAQUE: %iv1 = insertvalue { ptr, ptr } undef, ptr null, 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK: UNHANDLED

; CHECK-NONOPAQUE: %iv2 = insertvalue { i8*, i8* } %iv1, i8* null, 1
; CHECK-OPAQUE: %iv2 = insertvalue { ptr, ptr } %iv1, ptr null, 1
; CHECK-NEXT: LocalPointerInfo:
; CHECK: UNHANDLED

; Check that UNHANDLED is marked for %iv1 & %iv2 since %iv1 and %iv2 are
; not transitively used by ResumeInst.
define void @test05()  personality i32 (...)* @__gxx_personality_v0 {
  %iv1 = insertvalue { i8*, i32 } undef, i8* null, 0
  %iv2 = insertvalue { i8*, i32 } %iv1, i32 20, 1
  %ev1 = extractvalue { i8*, i32 } %iv2, 0
  ret void
}

; CHECK-LABEL: void @test05
; CHECK-NONOPAQUE: %iv1 = insertvalue { i8*, i32 } undef, i8* null, 0
; CHECK-OPAQUE: %iv1 = insertvalue { ptr, i32 } undef, ptr null, 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK: UNHANDLED

; CHECK-NONOPAQUE: %iv2 = insertvalue { i8*, i32 } %iv1, i32 20, 1
; CHECK-OPAQUE: %iv2 = insertvalue { ptr, i32 } %iv1, i32 20, 1
; CHECK-NEXT: LocalPointerInfo:
; CHECK: UNHANDLED


declare i32 @__gxx_personality_v0(...)
declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" i8* @_Znwm(i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!3 = !{i8 0, i32 1}  ; i8*
!4 = distinct !{!3}
!5 = distinct !{!2}
!6 = distinct !{!3}
!7 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!7}

