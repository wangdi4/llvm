; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test where the result of a GEP does not lead directly to the load/store.

; The fields indexed by the GEP in this case need to be marked with the
; 'ComplexUse' flag because transformations such as 'Delete field' only
; support cases where the GEP result is used for a load/store because
; those instructions can easily be deleted.
%struct.test01 = type { i32, i32, i32 }
define internal i32 @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !3 {
  %field0 = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 0
  %field2 = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 2
  %fieldAddr = select i1 undef, i32* %field0, i32* %field2
  %val = load i32, i32* %fieldAddr
  ret i32 %val
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:
; CHECK-SAME: ComplexUse
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:
; CHECK-NOT: ComplexUse
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info:
; CHECK-SAME: ComplexUse
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01


; Getting the address of a field for a call to 'memset' is also considered a
; 'ComplexUse'
%struct.test02 = type { i32, i16, i32 }
define internal void @test02(i32 %x) {
  %local = alloca %struct.test02
  %flat = bitcast %struct.test02* %local to i8*
  %pad_addr = getelementptr i8, i8* %flat, i32 0
  call void @llvm.memset.p0i8.i64(i8* %pad_addr, i8 0, i64 12, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:
; CHECK-SAME: ComplexUse
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info:
; CHECK-NOT: ComplexUse
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info:
; CHECK-NOT: ComplexUse
; CHECK: End LLVMType: %struct.test02

declare !intel.dtrans.func.type !6 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{i16 0, i32 0}  ; i16
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!8 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !4, !1} ; { i32, i16, i32 }

!intel.dtrans.types = !{!7, !8}
