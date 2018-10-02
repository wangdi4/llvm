; RUN: opt < %s -disable-output -whole-program-assume -dtransanalysis -dtrans-print-types | FileCheck %s
; RUN: opt < %s -disable-output -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types | FileCheck %s

; This test verifies that annotation intrinsics are handled by the
; DTrans analysis.

; Strings to use for annotation calls.
@__DTransAOSToSOAIndexFieldName = private constant [25 x i8] c"DTrans: AOS-to-SOA Index\00"
@__FileAnnot = private constant [13 x i8] c"AOSToSOA.cpp\00"

; Verify that a llvm.ptr.annotation call does not result in unhandled use.
%struct.test01 = type { i32, i32 }
define dso_local i32 @test01() {
  %ptr = alloca %struct.test01

  %field1 = getelementptr inbounds %struct.test01, %struct.test01* %ptr, i64 0, i32 1
  %t1 = call i32* @llvm.ptr.annotation.p0i32(i32* %field1,
    i8* getelementptr inbounds ([25 x i8], [25 x i8]* @__DTransAOSToSOAIndexFieldName, i32 0, i32 0),
    i8* getelementptr inbounds ([13 x i8], [13 x i8]* @__FileAnnot, i32 0, i32 0), i32 0)

  ret i32 0
}
; CHECK-LABEL: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data:
; CHECK-NOT: Unhandled use


; This test verifies that type information is propagated from the first
; argument of the ptr annotation to the result.
%struct.test02a = type { i32, i32 }
%struct.test02b = type { i32 }
define dso_local i32 @test02() {
  %ptr = alloca %struct.test02a
  %ptr_i8 = bitcast %struct.test02a* %ptr to i8*
  %annot = call i8* @llvm.ptr.annotation.p0i8(i8* %ptr_i8,
    i8* getelementptr inbounds ([25 x i8], [25 x i8]* @__DTransAOSToSOAIndexFieldName, i32 0, i32 0),
    i8* getelementptr inbounds ([13 x i8], [13 x i8]* @__FileAnnot, i32 0, i32 0), i32 0)

  ; This should trigger a bad bitcast on %struct.test02a because %annot should
  ; have acquired the type information carried by %ptr_i8.
  %annot_st = bitcast i8* %annot to %struct.test02b*

  ret i32 0
}
; CHECK-LABEL: LLVMType: %struct.test02a = type { i32, i32 }
; CHECK: Safety data:
; CHECK-SAME: Bad casting
; CHECK-LABEL: LLVMType: %struct.test02b = type { i32 }
; CHECK: Safety data:
; CHECK-SAME: Bad casting


; Verify that a llvm.var.annotation call does not result in unhandled use.
%struct.test03 = type { i64, i32 }
define dso_local i32 @test03() {
  %ptr = alloca %struct.test03

  %ptr_i8 = bitcast %struct.test03* %ptr to i8*
  call void @llvm.var.annotation(i8* %ptr_i8,
    i8* getelementptr inbounds ([25 x i8], [25 x i8]* @__DTransAOSToSOAIndexFieldName, i32 0, i32 0),
    i8* getelementptr inbounds ([13 x i8], [13 x i8]* @__FileAnnot, i32 0, i32 0), i32 0)

  ret i32 0
}
; CHECK-LABEL: LLVMType: %struct.test03 = type { i64, i32 }
; CHECK: Safety data:
; CHECK-NOT: Unhandled use

declare i32* @llvm.ptr.annotation.p0i32(i32*, i8*, i8*, i32)
declare i8* @llvm.ptr.annotation.p0i8(i8*, i8*, i8*, i32)
declare void @llvm.var.annotation(i8*, i8*, i8*, i32)
