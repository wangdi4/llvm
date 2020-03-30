; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery for "phi" instruction which involve
; bitcast types.

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are tests for the future opaque pointer form of IR.
; Lines marked with CHECK should remain the same when changing to use opaque pointers.


; In this case a pointer to a structure type flows through a phi node, but is
; bitcast to a different type before use.
; NOTE: It will be the responsibility of the safety analyzer to mark a
; safety violation for the store due to the attempt to store an i32 into a
; pointer location that refers to a %struct.test02**.
%struct.test01 = type { i64*, %struct.test01*, i32 }
define internal void @test01(i64 %in, i32 %in32, %struct.test01* %struct1, %struct.test01* %struct2) !dtrans_type !1 {
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
; CHECK-CUR:   %zf1 = getelementptr %struct.test01, %struct.test01* %struct1, i64 0, i32 1
; CHECK-FUT:   %zf1 = getelementptr %struct.test01, p0 %struct1, i64 0, i32 1
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test01**{{ *$}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test01 @ 1

; CHECK-CUR:  %nzf1 = getelementptr %struct.test01, %struct.test01* %struct2, i64 0, i32 1
; CHECK-FUT:  %nzf1 = getelementptr %struct.test01, p0 %struct2, i64 0, i32 1
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test01**{{ *$}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test01 @ 1

; CHECK-CUR:  %ptr1 = phi %struct.test01** [ %zf1, %zlabel ], [ %nzf1, %nzlabel ]
; CHECK-FUT:  %ptr1 = phi p0 [ %zf1, %zlabel ], [ %nzf1, %nzlabel ]
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test01**{{ *$}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test01 @ 1

; CHECK-CUR: %ptr1_bc = bitcast %struct.test01** %ptr1 to i32*
; CHECK-FUT: %ptr1_bc = bitcast p0 %ptr1 to p0
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
define internal void @test02(i64 %in, i32 %in32, %struct.test02* %struct1, %struct.test02* %struct2) !dtrans_type !7 {
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
; CHECK-CUR:  %zf1 = getelementptr %struct.test02, %struct.test02* %struct1, i64 0, i32 1
; CHECK-FUT:  %zf1 = getelementptr %struct.test02, p0 %struct1, i64 0, i32 1
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test02**{{ *$}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test02 @ 1

; CHECK-CUR:  %nzf1 = getelementptr %struct.test02, %struct.test02* %struct2, i64 0, i32 1
; CHECK-FUT:  %nzf1 = getelementptr %struct.test02, p0 %struct2, i64 0, i32 1
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test02**{{ *$}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test02 @ 1

; CHECK-CUR:  %ptr1 = phi i32* [ %zf_bc1, %zlabel ], [ %nzf_bc1, %nzlabel ]
; CHECK-FUT:  %ptr1 = phi p0 [ %zf_bc1, %zlabel ], [ %nzf_bc1, %nzlabel ]
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test02**{{ *$}}
; CHECK-NEXT:        i32*{{ *$}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test02 @ 1


!1 = !{!"F", i1 false, i32 4, !2, !3, !4, !5, !5}  ; void (i64, i32, %struct.test01*, %struct.test01*)
!2 = !{!"void", i32 0}  ; void
!3 = !{i64 0, i32 0}  ; i64
!4 = !{i32 0, i32 0}  ; i32
!5 = !{!6, i32 1}  ; %struct.test01*
!6 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!7 = !{!"F", i1 false, i32 4, !2, !3, !4, !8, !8}  ; void (i64, i32, %struct.test02*, %struct.test02*)
!8 = !{!9, i32 1}  ; %struct.test02*
!9 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!10 = !{i64 0, i32 1}  ; i64*
!11 = !{!"S", %struct.test01 zeroinitializer, i32 3, !10, !5, !4} ; { i64*, %struct.test01*, i32 }
!12 = !{!"S", %struct.test02 zeroinitializer, i32 3, !10, !8, !4} ; { i64*, %struct.test02*, i32 }

!dtrans_types = !{!11, !12}
