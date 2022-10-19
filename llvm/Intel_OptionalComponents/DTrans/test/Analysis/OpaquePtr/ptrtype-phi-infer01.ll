; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test pointer type recovery for "phi" instruction which involve
; bitcast types.

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are tests for the future opaque pointer form of IR.
; Lines marked with CHECK should remain the same when changing to use opaque pointers.


; In this case a pointer to a structure type flows through a phi node, but is
; bitcast to a different type before use.
; NOTE: It will be the responsibility of the safety analyzer to mark a
; safety violation for the store due to the attempt to store an i32 into a
; pointer location that refers to a %struct.test02**.
%struct.test01 = type { i64*, %struct.test01*, i32 }
define internal void @test01(i64 %in, i32 %in32, %struct.test01* "intel_dtrans_func_index"="1" %struct1, %struct.test01* "intel_dtrans_func_index"="2" %struct2) !intel.dtrans.func.type !4 {
entry:
  %cmp = icmp eq i64 %in, 0
  br i1 %cmp, label %zlabel, label %nzlabel

zlabel:
  %zf1 = getelementptr %struct.test01, %struct.test01* %struct1, i64 0, i32 1
  br label %merge

nzlabel:
  %nzf1 = getelementptr %struct.test01, %struct.test01* %struct2, i64 0, i32 1
  br label %merge

merge:
  %ptr1 = phi %struct.test01** [ %zf1, %zlabel ], [ %nzf1, %nzlabel ]
  %ptr1_bc = bitcast %struct.test01** %ptr1 to i32*
  store i32 %in32, i32* %ptr1_bc

  ret void
}
; CHECK-LABEL: void @test01
; CHECK-NONOPAQUE:   %zf1 = getelementptr %struct.test01, %struct.test01* %struct1, i64 0, i32 1
; CHECK-OPAQUE:   %zf1 = getelementptr %struct.test01, ptr %struct1, i64 0, i32 1
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test01**{{ *$}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test01 @ 1

; CHECK-NONOPAQUE:  %nzf1 = getelementptr %struct.test01, %struct.test01* %struct2, i64 0, i32 1
; CHECK-OPAQUE:  %nzf1 = getelementptr %struct.test01, ptr %struct2, i64 0, i32 1
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test01**{{ *$}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test01 @ 1

; CHECK-NONOPAQUE:  %ptr1 = phi %struct.test01** [ %zf1, %zlabel ], [ %nzf1, %nzlabel ]
; CHECK-OPAQUE:  %ptr1 = phi ptr [ %zf1, %zlabel ], [ %nzf1, %nzlabel ]
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test01**{{ *$}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test01 @ 1

; CHECK-NONOPAQUE: %ptr1_bc = bitcast %struct.test01** %ptr1 to i32*
; CHECK-OPAQUE: %ptr1_bc = bitcast ptr %ptr1 to ptr
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test01**{{ *$}}
; CHECK-NEXT:        i32*{{ *$}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test01 @ 1


; In this case a pointer to a structure type flows through a phi node after
; being bitcast to a different type.
;
; The phi node result should record that the result is used as an i32*.
; NOTE: It will be the responsibility of the safety analyzer should mark a
; safety violation for the store due to the attempt to store an i32 into a
; pointer location that refers to a %struct.test02**.
%struct.test02 = type { i64*, %struct.test02*, i32 }
define internal void @test02(i64 %in, i32 %in32, %struct.test02* "intel_dtrans_func_index"="1" %struct1, %struct.test02* "intel_dtrans_func_index"="2" %struct2) !intel.dtrans.func.type !6 {
entry:
  %cmp = icmp eq i64 %in, 0
  br i1 %cmp, label %zlabel, label %nzlabel

zlabel:
  %zf1 = getelementptr %struct.test02, %struct.test02* %struct1, i64 0, i32 1
  %zf_bc1 = bitcast %struct.test02** %zf1 to i32*
  br label %merge

nzlabel:
  %nzf1 = getelementptr %struct.test02, %struct.test02* %struct2, i64 0, i32 1
  %nzf_bc1 = bitcast %struct.test02** %nzf1 to i32*
  br label %merge

merge:
  %ptr1 = phi i32* [ %zf_bc1, %zlabel ], [ %nzf_bc1, %nzlabel ]
  store i32 %in32, i32* %ptr1

  ret void
}
; CHECK-LABEL: void @test02
; CHECK-NONOPAQUE:  %zf1 = getelementptr %struct.test02, %struct.test02* %struct1, i64 0, i32 1
; CHECK-OPAQUE:  %zf1 = getelementptr %struct.test02, ptr %struct1, i64 0, i32 1
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test02**{{ *$}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test02 @ 1

; CHECK-NONOPAQUE:  %nzf1 = getelementptr %struct.test02, %struct.test02* %struct2, i64 0, i32 1
; CHECK-OPAQUE:  %nzf1 = getelementptr %struct.test02, ptr %struct2, i64 0, i32 1
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test02**{{ *$}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test02 @ 1

; CHECK-NONOPAQUE:  %ptr1 = phi i32* [ %zf_bc1, %zlabel ], [ %nzf_bc1, %nzlabel ]
; CHECK-OPAQUE:  %ptr1 = phi ptr [ %zf_bc1, %zlabel ], [ %nzf_bc1, %nzlabel ]
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test02**{{ *$}}
; CHECK-NEXT:        i32*{{ *$}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test02 @ 1


!1 = !{i64 0, i32 1}  ; i64*
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{i32 0, i32 0}  ; i32
!4 = distinct !{!2, !2}
!5 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!6 = distinct !{!5, !5}
!7 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i64*, %struct.test01*, i32 }
!8 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !5, !3} ; { i64*, %struct.test02*, i32 }

!intel.dtrans.types = !{!7, !8}
