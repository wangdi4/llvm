; RUN: opt -dtransop-allow-typed-pointers -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Test that the base class clones functions which have parameter
; types or return values modified as a result of replacing types.
; Verify the metadata is updated to match the new type.

%struct.test01a = type { i32, i32, i32 }

define void @test01caller() {
  %a = call %struct.test01a* @test01getter()
  call void @test01callee(%struct.test01a* %a)
  ret void
}

; The following functions need to be cloned when opaque pointers are not in use
; because of the return or parameter type is being changed.
define "intel_dtrans_func_index"="1" %struct.test01a* @test01getter() !intel.dtrans.func.type !4 {
  %local = alloca %struct.test01a
  ret %struct.test01a* %local
}

define void @test01callee(%struct.test01a* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !3 {
  ret void
}

; CHECK-NONOPAQUE-LABEL: define void @test01caller()
; CHECK-NONOPAQUE: %a = call %__DTT_struct.test01a* @test01getter.1()
; CHECK-NONOPAQUE: call void @test01callee.2(%__DTT_struct.test01a* %a)

; CHECK-NONOPAQUE: define internal "intel_dtrans_func_index"="1" %__DTT_struct.test01a* @test01getter.1() !intel.dtrans.func.type ![[GETTER_MD:[0-9]+]] {
; CHECK-NONOPAQUE-LABEL: %local = alloca %__DTT_struct.test01a, align 8
; CHECK-NONOPAQUE-LABEL: ret %__DTT_struct.test01a* %local

; CHECK-NONOPAQUE: define internal void @test01callee.2(%__DTT_struct.test01a* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type ![[CALLEE_MD:[0-9]+]]


; The following checks can replace the above checks when opaque pointers are used.
; No clones are made when opaque pointers are in use, but metadata information should
; get updated.
; CHECK-OPAQUE-LABEL: define void @test01caller()
; CHECK-OPAQUE: %a = call ptr @test01getter()
; CHECK-OPAQUE call void @test01callee(ptr %a)

; CHECK-OPAQUE: define "intel_dtrans_func_index"="1" ptr @test01getter() !intel.dtrans.func.type ![[GETTER_MD:[0-9]+]]
; CHECK-OPAQUE %local = alloca %__DTT_struct.test01a, align 8
; CHECK-OPAQUE ret ptr %local

; CHECK-OPAQUE: define void @test01callee(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type ![[CALLEE_MD:[0-9]+]]

; The types within the metadata should be updated to use the remapped types.
; CHECK: ![[GETTER_MD]] = distinct !{![[PTR_S01A:[0-9]+]]}
; CHECK: ![[PTR_S01A]] = !{%__DTT_struct.test01a zeroinitializer, i32 1}
; CHECK: ![[CALLEE_MD]] = distinct !{![[PTR_S01A]]}


!intel.dtrans.types = !{!5}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!3 = distinct !{!2}
!4 = distinct !{!2}
!5 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

