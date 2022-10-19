; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that safety analyzer handling of mem intrinsic calls with pointer
; operands of 'undef' or 'null' do not cause an assertion failure.

%struct.test01 = type { i64, i64, i64 }

define internal void @test01() {
  %l = alloca %struct.test01
  %l.i8 = bitcast %struct.test01* %l to i8*
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %l.i8, i8* null, i64 24, i1 false)
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* null, i8* %l.i8, i64 24, i1 false)
  ret void
}

declare !intel.dtrans.func.type !3 void @llvm.memmove.p0i8.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i64, i1)
declare !intel.dtrans.func.type !4 void @llvm.memcpy.p0i8.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i64, i1)

!intel.dtrans.types = !{!5}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{i8 0, i32 1}  ; i8*
!3 = distinct !{!2, !2}
!4 = distinct !{!2, !2}
!5 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i64, i64, i64 }


; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Local instance{{ *}}
; CHECK: End LLVMType: %struct.test01
