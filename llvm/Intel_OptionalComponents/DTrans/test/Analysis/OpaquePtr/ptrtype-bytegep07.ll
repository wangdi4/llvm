; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-outofboundsok=false -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test identification of byte-flattened GEPs involving a nested structure.

%struct.test1 = type { i16, i16, i16, i16, %struct.test2 }
%struct.test2 = type { i32, i32, i32 }

; Function takes a void* as input, uses it to index into a nested type, and then performs
; element zero and byte flattened GEP accesses.
define void @test(ptr "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !5 {
  %ns = getelementptr inbounds %struct.test1, ptr %arg, i64 0, i32 4

; This GEP also ends up being used as an element-zero access into %struct.test2,
; but that is resolved by the safety analyzer when it is accessed that way, so
; it does not show "%struct.test2 @ 0" here.
; CHECK: %ns = getelementptr inbounds %struct.test1, ptr %arg, i64 0, i32 4
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test2*{{ *$}}
; CHECK-NEXT:        i32*{{ *$}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test1 @ 4

  %i44 = getelementptr inbounds i8, ptr %ns, i64 4
; CHECK:  %i44 = getelementptr inbounds i8, ptr %ns, i64 4
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i32*{{ *$}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test2 @ 1

  %i45 = getelementptr inbounds i8, ptr %ns, i64 8
; CHECK: %i45 = getelementptr inbounds i8, ptr %ns, i64 8
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i32*{{ *$}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test2 @ 2

  store i32 2, ptr %ns
  store i32 1, ptr %i44
  store i32 0, ptr %i45
  ret void
}

!1 = !{i16 0, i32 0}  ; i16
!2 = !{%struct.test2 zeroinitializer, i32 0}  ; %struct.test2
!3 = !{i32 0, i32 0}  ; i32
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4}
!6 = !{!"S", %struct.test1 zeroinitializer, i32 5, !1, !1, !1, !1, !2} ; { i16, i16, i16, i16, %struct.test2 }
!7 = !{!"S", %struct.test2 zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }

!intel.dtrans.types = !{!6, !7}

