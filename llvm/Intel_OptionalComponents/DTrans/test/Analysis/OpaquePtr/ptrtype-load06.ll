; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery when loading the element-zero member of an array
; using a pointer to the array itself.

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; This case loads the structure pointer from the array.
%struct.test01inner = type { %struct.test01inner_impl* }
%struct.test01inner_impl = type { i32, i32, i32 }

define internal void @test01([2 x %struct.test01inner]* %p) !dtrans_type !4 {
  %addr1 = getelementptr [2 x %struct.test01inner], [2 x %struct.test01inner]* %p, i64 0, i32 0, i32 0
  %val1 = load %struct.test01inner_impl*, %struct.test01inner_impl** %addr1

  %addr2 = bitcast [2 x %struct.test01inner]* %p to %struct.test01inner_impl**
  %val2 = load %struct.test01inner_impl*, %struct.test01inner_impl** %addr2

  ret void
}
; CHECK-LABEL: define internal void @test01
; CHECK-CUR: %val1 = load %struct.test01inner_impl*, %struct.test01inner_impl** %addr1
; CHECK-FUT: %val1 = load p0, p0 %addr1
; CHECK-NEXT:  LocalPointerInfo:
; CHECK-NEXT:    Aliased types:
; CHECK-NEXT:      %struct.test01inner_impl*{{ *$}}
; CHECK-NEXT:    No element pointees.

; CHECK-CUR: %val2 = load %struct.test01inner_impl*, %struct.test01inner_impl** %addr2
; CHECK-FUT: %val2 = load p0, p0 %addr2
; CHECK-NEXT:  LocalPointerInfo:
; CHECK-NEXT:    Aliased types:
; CHECK-NEXT:      %struct.test01inner_impl*{{ *$}}
; CHECK-NEXT:    No element pointees.


; This case loads the element-zero member of the stucture contained within the
; array.
%struct.test02inner = type { %struct.test02inner_impl }
%struct.test02inner_impl = type { i32*, i32*, i32* }

define internal void @test02([2 x %struct.test02inner]* %p) !dtrans_type !11 {
  %addr1 = getelementptr [2 x %struct.test02inner], [2 x %struct.test02inner]* %p, i64 0, i32 0, i32 0, i32 0
  %val1 = load i32*, i32** %addr1

  %addr2 = bitcast [2 x %struct.test02inner]* %p to i32**
  %val2 = load i32*, i32** %addr2

  ret void
}
; CHECK-LABEL: define internal void @test02
; CHECK-CUR: %val1 = load i32*, i32** %addr1
; CHECK-FUT: %val1 = load p0, p0 %addr1
; CHECK-NEXT:  LocalPointerInfo:
; CHECK-NEXT:    Aliased types:
; CHECK-NEXT:      i32*
; CHECK-NEXT:    No element pointees.

; CHECK-CUR: %val2 = load i32*, i32** %addr2
; CHECK-FUT: %val2 = load p0, p0 %addr2
; CHECK-NEXT:  LocalPointerInfo:
; CHECK-NEXT:    Aliased types:
; CHECK-NEXT:      i32*
; CHECK-NEXT:    No element pointees.


!1 = !{!2, i32 1}  ; %struct.test01inner_impl*
!2 = !{!"R", %struct.test01inner_impl zeroinitializer, i32 0}  ; %struct.test01inner_impl
!3 = !{i32 0, i32 0}  ; i32
!4 = !{!"F", i1 false, i32 1, !5, !6}  ; void ([2 x %struct.test01inner]*)
!5 = !{!"void", i32 0}  ; void
!6 = !{!7, i32 1}  ; [2 x %struct.test01inner]*
!7 = !{!"A", i32 2, !8}  ; [2 x %struct.test01inner]
!8 = !{!"R", %struct.test01inner zeroinitializer, i32 0}  ; %struct.test01inner
!9 = !{!"R", %struct.test02inner_impl zeroinitializer, i32 0}  ; %struct.test02inner_impl
!10 = !{i32 0, i32 1}  ; i32*
!11 = !{!"F", i1 false, i32 1, !5, !12}  ; void ([2 x %struct.test02inner]*)
!12 = !{!13, i32 1}  ; [2 x %struct.test02inner]*
!13 = !{!"A", i32 2, !14}  ; [2 x %struct.test02inner]
!14 = !{!"R", %struct.test02inner zeroinitializer, i32 0}  ; %struct.test02inner
!15 = !{!"S", %struct.test01inner zeroinitializer, i32 1, !1} ; { %struct.test01inner_impl* }
!16 = !{!"S", %struct.test01inner_impl zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }
!17 = !{!"S", %struct.test02inner zeroinitializer, i32 1, !9} ; { %struct.test02inner_impl }
!18 = !{!"S", %struct.test02inner_impl zeroinitializer, i32 3, !10, !10, !10} ; { i32*, i32*, i32* }

!dtrans_types = !{!15, !16, !17, !18}
