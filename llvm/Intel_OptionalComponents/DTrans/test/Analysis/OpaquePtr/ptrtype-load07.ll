; REQUIRES: asserts
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that getting the dominant usage type of an pointer is deterministic when
; the pointer is also used as a generic type.

; In this case the load from a structure field declared as i64* is used an i8*.
%struct.test01 = type { ptr, ptr } ; i64*. i64*
define void @test01(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !3 {
  %pField = getelementptr %struct.test01, ptr %pStruct, i64 0, i32 1
  %mem = load ptr, ptr %pField
  call void @llvm.memset.p0i8.i64(ptr %mem, i8 0, i64 256, i1 false)
  ret void
}
; CHECK-LABEL: define void @test01
; CHECK:  %mem = load ptr, ptr %pField
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i64*
; CHECK-NEXT:        i8*
; CHECK-NEXT:      No element pointees.
; CHECK-NEXT:      DomTy: i64*

; In this case the load from a structure field declared as i32* is used an i8*.
%struct.test02 = type { ptr, ptr } ; i32*, i32*
define void @test02(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !6 {
  %pField = getelementptr %struct.test02, ptr %pStruct, i32 0, i32 1
  %mem = load ptr, ptr %pField
  call void @llvm.memset.p0i8.i64(ptr %mem, i8 0, i64 256, i1 false)

  ret void
}
; CHECK-LABEL: define void @test02
; CHECK:  %mem = load ptr, ptr %pField
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i32*
; CHECK-NEXT:        i8*
; CHECK-NEXT:      No element pointees.
; CHECK-NEXT:      DomTy: i32*

declare !intel.dtrans.func.type !8 void @llvm.memset.p0i8.i64(ptr "intel_dtrans_func_index"="1", i8, i64, i1)

!1 = !{i64 0, i32 1}  ; i64*
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{i32 0, i32 1}  ; i32*
!5 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!6 = distinct !{!5}
!7 = !{i8 0, i32 1}  ; i8*
!8 = distinct !{!7}
!9 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64*, i64* }
!10 = !{!"S", %struct.test02 zeroinitializer, i32 2, !4, !4} ; { i32*, i32* }

!intel.dtrans.types = !{!9, !10}

