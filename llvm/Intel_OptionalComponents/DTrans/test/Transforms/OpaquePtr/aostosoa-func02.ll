; REQUIRES: asserts
; RUN: opt -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA transformation with a function that takes a parameter of the
; type being transformed from a pointer to an integer when a 'null' value is
; passed.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { ptr, ptr, ptr }
%struct.test01dep = type { ptr, ptr }

@var01 = internal global %struct.test01dep zeroinitializer
define i32 @test01() {
  ; The 1st parameter should be changed to an integer, the 2nd shouldn't.
  call void @test02(ptr null, ptr null)
  ret i32 0
}
; CHECK-LABEL: define i32 @test01()
; CHECK: call void @test02.1(i64 0, ptr null)

define void @test02(ptr "intel_dtrans_func_index"="1" %in, ptr "intel_dtrans_func_index"="2" %in1) !intel.dtrans.func.type !3 {
  %p1 = getelementptr %struct.test01dep, ptr @var01, i64 0, i32 1
  store  ptr %in, ptr %p1
  ret void
}
; The function should be cloned because the parameter type is changing.

; Currently, we do not modify the metadata typelist on the function to remove
; unused index values, so the pointer will still use index "2". It's still
; valid because there will be no references to index "1".

; CHECK-LABEL: define internal void @test02.1(i64 %in, ptr "intel_dtrans_func_index"="1" %in1)
; CHECK: %p1 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 1
; CHECK: %alloc_idx = call ptr @llvm.ptr.annotation.p0.p0(ptr %p1, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: store i64 %in, ptr %p1, align 8
; CHECK: ret void

!1 = !{i32 0, i32 1}  ; i32*
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2, !1}
!4 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.test01*, i32* }
!5 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !2} ; { %struct.test01*, %struct.test01* }

!intel.dtrans.types = !{!4, !5}
