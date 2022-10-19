; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test memory allocations that are ambiguous for DTrans because
; the result is used as multiple structure types.

%struct.test01a = type { i32, i32, i32, i32 }
%struct.test01b = type { i64, i64 }
define void @test01() {
  %mem = call i8* @malloc(i64 16)
  %mem.as.1a = bitcast i8* %mem to %struct.test01a*
  %a.addr = getelementptr %struct.test01a, %struct.test01a* %mem.as.1a, i64 0, i32 2
  store i32 1, i32* %a.addr

  %mem.as.1b = bitcast i8* %mem to %struct.test01b*
  %b.addr = getelementptr %struct.test01b, %struct.test01b* %mem.as.1b, i64 0, i32 0
  store i64 0, i64* %b.addr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}
; CHECK: End LLVMType: %struct.test01b


declare !intel.dtrans.func.type !4 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i8 0, i32 1}  ; i8*
!4 = distinct !{!3}
!5 = !{!"S", %struct.test01a zeroinitializer, i32 4, !1, !1, !1, !1} ; { i32, i32, i32, i32 }
!6 = !{!"S", %struct.test01b zeroinitializer, i32 2, !2, !2} ; { i64, i64 }

!intel.dtrans.types = !{!5, !6}
