; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA transformation with a function that takes a parameter of the
; type being transformed from a pointer to an integer when a 'null' value is
; passed.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32*, %struct.test01*, i32* }
%struct.test01dep = type { %struct.test01*, %struct.test01* }

@var01 = internal global %struct.test01dep zeroinitializer
define i32 @test01() {
  ; The 1st parameter should be changed to an integer, the 2nd shouldn't.
  call void @test02(%struct.test01* null, i32* null)
  ret i32 0
}
; CHECK-LABEL: define i32 @test01()
; CHECK-NONOPAQUE: call void @test02.1(i64 0, i32* null)
; CHECK-OPAQUE: call void @test02.1(i64 0, ptr null)

define void @test02(%struct.test01* "intel_dtrans_func_index"="1" %in, i32* "intel_dtrans_func_index"="2" %in1) !intel.dtrans.func.type !3 {
  %p1 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 1
  store  %struct.test01* %in, %struct.test01** %p1
  ret void
}
; The function should be cloned because the parameter type is changing.

; Currently, we do not modify the metadata typelist on the function to remove
; unused index values, so the pointer will still use index "2". It's still
; valid because there will be no references to index "1".
; CHECK-NONOPAQUE-LABEL: define internal void @test02.1(i64 %in, i32* "intel_dtrans_func_index"="1" %in1)
; CHECK-NONOPAQUE: %p1 = getelementptr %__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @var01, i64 0, i32 1
; CHECK-NONOPAQUE: %alloc_idx = call i64* @llvm.ptr.annotation.p0i64(i64* %p1, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, i8* null)
; CHECK-NONOPAQUE: store i64 %in, i64* %p1, align 8
; CHECK-NONOPAQUE: ret void

; CHECK-OPAQUE-LABEL: define internal void @test02.1(i64 %in, ptr "intel_dtrans_func_index"="1" %in1)
; CHECK-OPAQUE: %p1 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 1
; CHECK-OPAQUE: %alloc_idx = call ptr @llvm.ptr.annotation.p0(ptr %p1, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK-OPAQUE: store i64 %in, ptr %p1, align 8
; CHECK-OPAQUE: ret void

!1 = !{i32 0, i32 1}  ; i32*
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2, !1}
!4 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.test01*, i32* }
!5 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !2} ; { %struct.test01*, %struct.test01* }

!intel.dtrans.types = !{!4, !5}
