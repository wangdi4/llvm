; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA transformation handles a bitcast function call,
; and that attributes on the call get updated.

; This test will no longer be relevant when the compiler only supports opaque
; pointers because the bitcast function call will be a direct call.

%struct.test01 = type { i64, %struct.test01* }
%struct.test01dep = type { %struct.test01*, %struct.test01* }

@var01 = internal global %struct.test01dep zeroinitializer
@var02 = internal global double zeroinitializer
define i32 @test01() {
  %field = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 1
  %st = load %struct.test01*, %struct.test01** %field

  ; Note, when opaque pointers are the default, this is just a direct call to @test02.
  call void bitcast (void (%struct.test01*, %struct.test01dep*, i64*)* @test02 to void(%struct.test01*, %struct.test01dep*, double*)*)(%struct.test01* nonnull %st, %struct.test01dep* @var01, double* @var02)
  ret i32 0
}
; CHECK-LABEL: define i32 @test01()
; CHECK: call void bitcast (void (i64, %__SOADT_struct.test01dep*, i64*)* @test02.1 to void (i64, %__SOADT_struct.test01dep*, double*)*)(i64 %st, %__SOADT_struct.test01dep* @var01, double* @var02)

define void @test02(%struct.test01* "intel_dtrans_func_index"="1" %st, %struct.test01dep* "intel_dtrans_func_index"="2" %in, i64* "intel_dtrans_func_index"="3" %val) !intel.dtrans.func.type !5 {
  %p1 = getelementptr %struct.test01dep, %struct.test01dep* %in, i64 0, i32 1
  store %struct.test01* %st, %struct.test01** %p1
  ret void
}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{%struct.test01dep zeroinitializer, i32 1}  ; %struct.test01dep*
!4 = !{i64 0, i32 1}  ; i64*
!5 = distinct !{!2, !3, !4}
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i64, %struct.test01* }
!7 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !2} ; { %struct.test01*, %struct.test01* }

!intel.dtrans.types = !{!6, !7}
