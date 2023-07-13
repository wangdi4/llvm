; Test that the metadata and structure types that are only referenced by the
; !intel.dtrans.func.type tag are removed when the parameter that required the
; type is no longer present.
;
; The design of the attributes was to allow function parameters to be removed,
; while maintaining type metadata to still be looked up on the remaining parameters
; without parts of the compiler that rewrite function signatures needing to know
; about DTrans type metadata. However, this can lead to a situation where the
; type is left with the only reference being from the DTrans type metadata. This
; test is to verify that the DTrans pass that removes dead metadata removes these
; stale references, and updates the metadata, which can the allow the type to
; be completely removed from the IR.

; RUN: opt -S -passes=remove-dead-dtranstypemetadata %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

%struct.test01 = type { i64, i64 }
%struct.test02 = type { i32, i32 }

; Test that a function which has more metadata types than arguments marked with
; metadata gets updated. This to simulate a function that used to be "i64*,
; %struct.test01*, %struct.test02*, i64*"" but now is "%struct.test01*, i64*"
; which should cause the DTrans type metadata tracking of types to remove all
; references to "%struct.test02", since this type will no longer be part of the
; IR.
define i64 @test1(ptr "intel_dtrans_func_index"="2" %in, ptr "intel_dtrans_func_index"="4" %in2 ) !intel.dtrans.func.type !6 {
  %a = getelementptr %struct.test01, ptr %in, i64 0, i32 1
  %v1 = load i64, ptr %a
  %v2 = load i64, ptr %in2
  %sum = add i64 %v1, %v2
  ret i64 %sum
}

; This function no longer has any attributes that provide DTrans type metadata,
; and should get the !intel.dtrans.func.type metadata removed from it.
define void @test2() !intel.dtrans.func.type !9 {
	ret void
}

; There should no longer be any references to %struct.test02, so it should be not be
; emitted.

; CHECK: %struct.test01 = type { i64, i64 }
; CHECK-NOT: %struct.test02 = type { i32, i32 }

; CHECK: define i64 @test1(ptr "intel_dtrans_func_index"="1" %in, ptr "intel_dtrans_func_index"="2" %in2) !intel.dtrans.func.type ![[FUNC_MD:[0-9]+]]
; CHECK: define void @test2() {

; CHECK: !intel.dtrans.types = !{![[STY_MD:[0-9]+]]}
; CHECK: ![[STY_MD]] =  !{!"S", %struct.test01 zeroinitializer
; CHECK: ![[FUNC_MD]] = distinct !{![[STYPTR_MD:[0-9]+]], ![[I64PTR_MD:[0-9]+]]}
; CHECK: ![[STYPTR_MD]] = !{%struct.test01 zeroinitializer, i32 1}
; CHECK: ![[I64PTR_MD]] = !{i64 0, i32 1}

!intel.dtrans.types = !{!7, !8}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{i32 0, i32 0}  ; i64
!3 = !{i64 0, i32 1}  ; i64*
!4 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!5 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!6 = distinct !{!3, !4, !5, !3}
!7 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!8 = !{!"S", %struct.test01 zeroinitializer, i32 2, !2, !2} ; { i32, i32 }
!9 = !{!3, !4}
