; REQUIRES: asserts
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type analyzer for "atomicrmw" instruction.

; Test simple scalar variable use. This case should not get marked as
; 'UNHANDLED' because the value at the memory location is not being used as a
; pointer type.
define void @test0() {
  %var0 = alloca i64
  %r0 = atomicrmw xchg ptr %var0, i64 0 seq_cst
  ret void
}
; CHECK-LABEL: define void @test0
; CHECK: %var0 = alloca i64
; CHECK-NOT: UNHANDLED
; CHECK:      Aliased types:
; CHECK:        i64*
; CHECK:      No element pointees.

; CHECK: %r0 = atomicrmw xchg ptr %var0, i64 0 seq_cst
; CHECK-NOT: UNHANDLED


; Test with a pointer that increments the value of a structure field member.
;
; This case should not get marked as 'UNHANDLED' because the value at the memory
; location is not being used as a pointer type. However, the DTransSafetyAnalyzer
; will treat the structure type as 'Unhandled use' because it would require
; processing the instruction as a load and store for type checking and field
; value tracking.
%struct.test1 = type { i64, i64 }
define void @test1() {
  %var1 = alloca %struct.test1
  %f0 = getelementptr %struct.test1, ptr %var1, i64 0, i32 0
  %r0 = atomicrmw add ptr %f0, i64 1 seq_cst
  ret void
}
; CHECK-LABEL: define void @test1
; CHECK: %f0 = getelementptr %struct.test1, ptr %var1, i64 0, i32 0
; CHECK-NOT: UNHANDLED
; CHECK:      Aliased types:
; CHECK:        i64*
; CHECK:      Element pointees:
; CHECK:        %struct.test1 @ 0

; CHECK: %r0 = atomicrmw add ptr %f0, i64 1 seq_cst
; CHECK-NOT: UNHANDLED


; Test with pointer that is a structure field member of a pointer type. This
; case is should be marked as 'UNHANDLED' because it does not appear in
; benchmarks that are cared about. To handle it properly would require the
; DTransSafetyAnalyzer to process the instruction for value tracking, and check
; the data types.
%struct.test2 = type { ptr, ptr }
define void @test2() {
  %var2 = alloca %struct.test2
  %f1 = getelementptr %struct.test2, ptr %var2, i64 0, i32 1
  %r1 = atomicrmw xchg ptr %f1, ptr null seq_cst
  ret void
}
; CHECK-LABEL: define void @test2
; CHECK: %f1 = getelementptr %struct.test2, ptr %var2, i64 0, i32 1
; CHECK: UNHANDLED

; CHECK: %r1 = atomicrmw xchg ptr %f1, null seq_cst
; CHECK: UNHANDLED


; Test with GEP-less access to modify a value in a structure field. The pointer
; type analyzer should not mark it as 'UNHANDLED', since it is using integer
; memory values. However, the DTransSafetyAnalyzer will treat the structure type
; as 'Unhandled use' because it would require processing the instruction as a
; load and store for type checking and field value tracking.
%struct.test3 = type { i64, i64 }
define void @test3() {
  %var3 = alloca %struct.test3
  %r0 = atomicrmw xor ptr %var3, i64 5 seq_cst
  ret void
}
; CHECK-LABEL: define void @test3
; CHECK: %var3 = alloca %struct.test3
; CHECK-NOT: UNHANDLED
; CHECK:      Aliased types:
; CHECK:        %struct.test3*
; CHECK:        i64*
; CHECK:      No element pointees.

; CHECK: %r0 = atomicrmw xor ptr %var3, i64 5 seq_cst, align 8
; CHECK-NOT: UNHANDLED


; Test with using the pointer with a type that is different than how it was
; defined. The pointer type analyzer should collect the types, but should not
; mark it as 'UNHANDLED', since it is using integer memory values.
define void @test4() {
  %var4 = alloca i64
  %r0 = atomicrmw xchg ptr %var4, i32 0 seq_cst
  ret void
}
; CHECK-LABEL: define void @test4
; CHECK: %var4 = alloca i64
; CHECK-NOT: UNHANDLED
; CHECK:      Aliased types:
; CHECK:        i32*
; CHECK:        i64*
; CHECK:      No element pointees.

; CHECK: %r0 = atomicrmw xchg ptr %var4, i32 0 seq_cst
; CHECK-NOT: UNHANDLED


; Test with using a pointer to a structure. This case should be marked as
; 'UNHANDLED' because it does not appear in benchmarks that are cared about.
; To handle it properly would require the DTransSafetyAnalyzer to process
; the instruction, and check the data types.
%struct.test5 = type { i64, i64 }
define void @test5() {
  %var2 = alloca %struct.test5
  %r1 = atomicrmw xchg ptr %var2, ptr null seq_cst
  ret void
}
; CHECK-LABEL: define void @test5
; CHECK: %var2 = alloca %struct.test5, align 8
; CHECK: UNHANDLED

; CHECK: %r1 = atomicrmw xchg ptr %var2, null seq_cst,
; CHECK: UNHANDLED

!intel.dtrans.types = !{!3, !4, !5, !6}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{i64 0, i32 1}  ; i64*
!3 = !{!"S", %struct.test1 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!4 = !{!"S", %struct.test2 zeroinitializer, i32 2, !2, !2} ; { i64*, i64* }
!5 = !{!"S", %struct.test3 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!6 = !{!"S", %struct.test5 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }

