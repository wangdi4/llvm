; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that memcpy analysis does not crash when the destination pointer
; type collected by the PtrTypeAnalyzer is an opaque structure type.
; (CMPLRLLVM-33257)

%struct.test01 = type opaque
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01 = type opaque

@ppCCGElem = internal global %struct.test01* zeroinitializer, !intel_dtrans_type !1
@p81 = internal global i8 zeroinitializer
@p82 = internal global i8 zeroinitializer

define void @test1(i64 %offset) {
  %i105 = call %struct.test01** @getter1()

  %i111 = call i8* @getter2()
  %i112 = bitcast i8* %i111 to %struct.test01**

  %i123 = call i8* @getter3()
  %i124 = bitcast i8* %i123 to %struct.test01*
  store %struct.test01* %i124, %struct.test01** @ppCCGElem

  %i125 = getelementptr inbounds %struct.test01*, %struct.test01** %i112, i64 %offset
  store %struct.test01* %i124, %struct.test01** %i125

  %i126 = getelementptr inbounds %struct.test01*, %struct.test01** %i105, i64 %offset
  %i127 = bitcast %struct.test01** %i126 to i8**
  %i128 = load i8*, i8** %i127

  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %i123, i8* %i128, i64 8, i1 false)

  ret void
}

define "intel_dtrans_func_index"="1" %struct.test01** @getter1() !intel.dtrans.func.type !3 {
  ret %struct.test01** @ppCCGElem
}

define "intel_dtrans_func_index"="1" i8* @getter2() !intel.dtrans.func.type !5 {
  ret i8* @p81
}

define "intel_dtrans_func_index"="1" i8* @getter3() !intel.dtrans.func.type !6 {
  ret i8* @p82
}

define void @user(%struct.test01** "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !7 {
  ret void
}

declare !intel.dtrans.func.type !8 void @llvm.memcpy.p0i8.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i64, i1)

!1 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!2 = !{%struct.test01 zeroinitializer, i32 2}  ; %struct.test01**
!3 = distinct !{!2}
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4}
!6 = distinct !{!4}
!7 = distinct !{!2}
!8 = distinct !{!4, !4}
!9 = !{!"S", %struct.test01 zeroinitializer, i32 0} ; opaque

!intel.dtrans.types = !{!9}
