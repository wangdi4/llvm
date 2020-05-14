; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-CUR

; Tests for inferring the type of an input argument declared as an i8*
; based on the usage types of the argument.

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are tests for the future opaque pointer form of IR.
; Lines marked with CHECK should remain the same when changing to use opaque pointers.

; Test inference based on direct use of the type in the getelementptr
; instruction.
%struct.test01 = type { i32, i64 }
define void @test01(i8* %arg) !dtrans_type !3  {
  %cast = bitcast i8* %arg to %struct.test01*
  %f0 = getelementptr %struct.test01, %struct.test01* %cast, i64 0, i32 0
  store i32 0, i32* %f0
  ret void
}
; CHECK-LABEL: Input Parameters: test01
; CHECK-CUR:    Arg 0: i8* %arg
; CHECK-FUT:    Arg 0: p0 %arg
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:        %struct.test01*{{ *$}}
; CHECK-NEXT:        i8*{{ *$}}
; CHECK-NEXT:  No element pointees.

; CHECK-CUR:  %cast = bitcast i8* %arg to %struct.test01*
; CHECK-FUT:  %cast = bitcast p0 %arg to p0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


; In this case, trying to infer the type of %arg, requires inferring the
; type of %cast. However, to infer the cast type, the store instruction
; needs to first analyze the pointer operand, %f0.
%struct.test02 = type { i32, i64, %struct.test02* }
@var02 = global %struct.test02 zeroinitializer
define void @test02(i8* %arg) !dtrans_type !3  {
  %cast = bitcast i8* %arg to %struct.test02*
  %f0 = getelementptr %struct.test02, %struct.test02* @var02, i64 0, i32 2
  store %struct.test02* %cast, %struct.test02** %f0
  ret void
}
; CHECK-LABEL: Input Parameters: test02
; CHECK-CUR:    Arg 0: i8* %arg
; CHECK-FUT:    Arg 0: p0 %arg
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test02*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-CUR:  %cast = bitcast i8* %arg to %struct.test02*
; CHECK-FUT:  %cast = bitcast p0 %arg to p0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:    %struct.test02*{{ *$}}
; CHECK-NEXT:    i8*{{ *$}}
; CHECK-NEXT: No element pointees.


; In this case, the type needs to be inferred from the value loaded,
; which needs to be inferred based on the comparison of the result
; with a pointer of a known type.
%struct.test03 = type { i32, i32 }
define i1 @test03(%struct.test03* %arg0, i8* %arg1) !dtrans_type !8  {
  %cast = bitcast i8* %arg1 to %struct.test03**
  %val = load %struct.test03*, %struct.test03** %cast
  %cmp = icmp eq %struct.test03* %arg0, %val
  ret i1 %cmp
}
; CHECK-LABEL:  Input Parameters: test03
; CHECK-CUR:    Arg 0: %struct.test03* %arg0
; CHECK-FUT:    Arg 0: p0 %arg0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test03*
; CHECK-NEXT: No element pointees.

; CHECK-CUR:    Arg 1: i8* %arg1
; CHECK-FUT:    Arg 1: p0 %arg1
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test03**{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-CUR: cast = bitcast i8* %arg1 to %struct.test03**
; CHECK-FUT: cast = bitcast p0 %arg1 to p0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test03**{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


; In this case, trying to infer the type of the %arg, requires looking
; through PHINodes, and a call instruction.
%struct.test04 = type { i32, i32 }
define void @test04(i8* %arg) !dtrans_type !3  {
entry:
  ; This ptrtoint is modeled off of spec_qsort.40, where the first use of
  ; %arg, which is declared as an i8*, but actually represents a pointer
  ; to a structure is processed by a ptrtoint instruction.
  %tmp1 = ptrtoint i8* %arg to i64
  br i1 undef, label %block1, label %block2

block1:
  %phi1 = phi i8* [ %arg, %entry ], [ %phi2, %block3 ]
  br i1 undef, label %block2, label %block3

block2:
  %tmp2 = getelementptr i8, i8* %arg, i64 8
  br label %block3

block3:
  %phi2 = phi i8* [ %phi1, %block1 ], [ %tmp2, %block2]
  %cast = bitcast i8* %phi2 to %struct.test04*
  call void @test04use(%struct.test04* %cast)
  br i1 undef, label %block1, label %exit

exit:
  ret void
}

define void @test04use(%struct.test04* %in) !dtrans_type !12  {
  %f0 = getelementptr %struct.test04, %struct.test04* %in, i64 0, i32 0
  store i32 0, i32* %f0
  ret void
}
; CHECK-LABEL:  Input Parameters: test04
; CHECK-CUR:    Arg 0: i8* %arg
; CHECK-FUT:    Arg 0: p0 %arg
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test04*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-CUR:  %tmp1 = ptrtoint i8* %arg to i64
; CHECK-FUT:  %tmp1 = ptrtoint p0 %arg to i64
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test04*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


; In this case, there is a recursive call which leads back to trying to
; analyze the original pointer being inferred.
%struct.test05 = type { i32, i32 }
define void @test05(i8* %arg) !dtrans_type !3  {
entry:
  ; This ptrtoint is modeled off of spec_qsort.40, where the first use of
  ; %arg is declared as an i8*, but actually represents a pointer
  ; to a structure is processed by a ptrtoint instruction.
  %tmp1 = ptrtoint i8* %arg to i64
  br i1 undef, label %block1, label %block2

block1:
  %phi1 = phi i8* [ %arg, %entry ], [ %phi2, %block3 ]
  br i1 undef, label %block2, label %block3

block2:
  %tmp2 = getelementptr i8, i8* %arg, i64 8
  ; recursive call that leads back to value being inferred.
  call void @test05(i8* %tmp2)
  br label %block3

block3:
  %phi2 = phi i8* [ %phi1, %block1 ], [ %tmp2, %block2]
  %cast = bitcast i8* %phi2 to %struct.test05*
  call void @test05use(%struct.test05* %cast)
  br i1 undef, label %block1, label %exit

exit:
  ret void
}

define void @test05use(%struct.test05* %in) !dtrans_type !15  {
  %f0 = getelementptr %struct.test05, %struct.test05* %in, i64 0, i32 0
  store i32 0, i32* %f0
  ret void
}
; CHECK-LABEL:  Input Parameters: test05
; CHECK-CUR:    Arg 0: i8* %arg
; CHECK-FUT:    Arg 0: p0 %arg
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test05*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-CUR:  %tmp1 = ptrtoint i8* %arg to i64
; CHECK-FUT:  %tmp1 = ptrtoint p0 %arg to i64
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test05*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


; In this case, the type of bitcast needs to be inferred from the
; value type being stored into the pointer location.
%struct.test06 = type { i32, i64, %struct.test06* }
@var06 = global %struct.test06 zeroinitializer
define void @test06(i8* %arg) !dtrans_type !3  {
  %cast = bitcast i8* %arg to %struct.test06**
  store %struct.test06* @var06, %struct.test06** %cast
  ret void
}
; CHECK-LABEL:  Input Parameters: test06
; CHECK-CUR:    Arg 0: i8* %arg
; CHECK-FUT:    Arg 0: p0 %arg
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test06**{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-CUR:  %cast = bitcast i8* %arg to %struct.test06**
; CHECK-FUT:  %cast = bitcast p0 %arg to p0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test06**{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


; This case is like @test06, except that there is also a 'null'
; pointer being stored to a location that will be visited while
; trying to infer the pointer type. Note, the 'null' type will
; be p0 with opaque pointers, so will not contribute useful
; information.
%struct.test07 = type { i32, i64, %struct.test07* }
@var07 = global %struct.test07 zeroinitializer
define void @test07(i8* %arg) !dtrans_type !3  {
  %cast = bitcast i8* %arg to %struct.test07**
  br i1 undef, label %block1, label %block2
block1:
  store %struct.test07* null, %struct.test07** %cast
  br label %exit

block2:
  store %struct.test07* @var07, %struct.test07** %cast
  br label %exit

exit:
  ret void
}
; CHECK-LABEL:  Input Parameters: test07
; CHECK-CUR:    Arg 0: i8* %arg
; CHECK-FUT:    Arg 0: p0 %arg
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test07**{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-CUR:  %cast = bitcast i8* %arg to %struct.test07**
; CHECK-FUT:  %cast = bitcast p0 %arg to p0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test07**{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.

; In this case, trying to infer the type of %pUnknown requires looking for
; the store instruction that is used following the conversion of the value
; to a pointer-sized integer.
%struct.test08 = type { i32, i32 }
define void @test08(%struct.test03** %ppS, i8* %pUnknown)  !dtrans_type !22 {
  %tmp = bitcast %struct.test03** %ppS to i64*
  %unknown = ptrtoint i8* %pUnknown to i64
  store i64 %unknown, i64* %tmp
  ret void
}
; CHECK-LABEL: Input Parameters: test08
; CHECK-CUR:    Arg 0: %struct.test03** %ppS
; CHECK-FUT:    Arg 0: p0 %ppS
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     %struct.test03**{{ *$}}
; CHECK-NEXT:   No element pointees.

; CHECK-CUR:    Arg 1: i8* %pUnknown
; CHECK-FUT:    Arg 1: p0 %pUnknown
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     %struct.test03*{{ *$}}
; CHECK-NEXT:     i8*{{ *$}}
; CHECK-NEXT:   No element pointees.

; CHECK-CUR: %unknown = ptrtoint i8* %pUnknown to i64
; CHECK-FUT: %unknown = ptrtoint p0 %pUnknown to i64
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     %struct.test03*{{ *$}}
; CHECK-NEXT:     i8*{{ *$}}
; CHECK-NEXT:   No element pointees.


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{!"F", i1 false, i32 1, !4, !5}  ; void (i8*)
!4 = !{!"void", i32 0}  ; void
!5 = !{i8 0, i32 1}  ; i8*
!6 = !{!7, i32 1}  ; %struct.test02*
!7 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!8 = !{!"F", i1 false, i32 2, !9, !10, !5}  ; i1 (%struct.test03*, i8*)
!9 = !{i1 0, i32 0}  ; i1
!10 = !{!11, i32 1}  ; %struct.test03*
!11 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!12 = !{!"F", i1 false, i32 1, !4, !13}  ; void (%struct.test04*)
!13 = !{!14, i32 1}  ; %struct.test04*
!14 = !{!"R", %struct.test04 zeroinitializer, i32 0}  ; %struct.test04
!15 = !{!"F", i1 false, i32 1, !4, !16}  ; void (%struct.test05*)
!16 = !{!17, i32 1}  ; %struct.test05*
!17 = !{!"R", %struct.test05 zeroinitializer, i32 0}  ; %struct.test05
!18 = !{!19, i32 1}  ; %struct.test06*
!19 = !{!"R", %struct.test06 zeroinitializer, i32 0}  ; %struct.test06
!20 = !{!21, i32 1}  ; %struct.test07*
!21 = !{!"R", %struct.test07 zeroinitializer, i32 0}  ; %struct.test07
!22 = !{!"F", i1 false, i32 2, !4, !23, !5}  ; void (%struct.test03**, i8*)
!23 = !{!11, i32 2}  ; %struct.test03**
!24 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i32, i64 }
!25 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !2, !6} ; { i32, i64, %struct.test02* }
!26 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!27 = !{!"S", %struct.test04 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!28 = !{!"S", %struct.test05 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!29 = !{!"S", %struct.test06 zeroinitializer, i32 3, !1, !2, !18} ; { i32, i64, %struct.test06* }
!30 = !{!"S", %struct.test07 zeroinitializer, i32 3, !1, !2, !20} ; { i32, i64, %struct.test07* }
!31 = !{!"S", %struct.test08 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!24, !25, !26, !27, !28, !29, !30, !31}
