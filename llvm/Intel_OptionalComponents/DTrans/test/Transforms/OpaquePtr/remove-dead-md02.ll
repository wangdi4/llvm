; RUN: opt -dtransop-allow-typed-pointers -S -remove-dead-dtranstypemetadata %s | FileCheck %s
; RUN: opt -opaque-pointers -S -remove-dead-dtranstypemetadata %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that when removing dead DTrans type metadata, types used in
; GEP operators are maintained becuased these types may be different
; than the type the variable was declared as.
;
; %struct.test01 - Needs to be kept because of global variable
; %struct.test02 - Needs to be kept because a GEPOperator uses a global as this type.

%struct.test01 = type { i64, i64 }
%struct.test02 = type { i64, i64 }

@gVar = internal global %struct.test01 zeroinitializer

define i64 @test() {
  %ld = load i64, i64* getelementptr (%struct.test02, %struct.test02* bitcast (%struct.test01* @gVar to %struct.test02*), i64 0, i32 1)
  ret i64 %ld
}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!3 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }

!intel.dtrans.types = !{!2, !3}

; CHECK: !intel.dtrans.types = !{![[MD_TEST01:[0-9]+]], ![[MD_TEST02:[0-9]+]]}

; CHECK: ![[MD_TEST01]] = !{!"S", %struct.test01 zeroinitializer
; CHECK: ![[MD_TEST02]] = !{!"S", %struct.test02 zeroinitializer
