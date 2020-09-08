; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Store a pointer to a structure, which has multiple type aliases that do not
; match the expected type for the pointer-to-pointer location used in the store
; instruction. This should result in the types being marked with "Unsafe
; pointer store"

%struct.test01a = type { i32, i32 }
%struct.test01b = type { i16, i16, i32 }
%struct.test01c = type { i16, i16, i16, i16 }

@varTest01 = internal global %struct.test01a* null, !dtrans_type !3
define void @test01() {
  %var1 = call %struct.test01b* @test01b()
  %var2 = call %struct.test01c* @test01c()
  %val1.as.i8 = bitcast %struct.test01b* %var1 to i8*
  %val2.as.i8 = bitcast %struct.test01c* %var2 to i8*
  %valueToStore.as.i8 = select i1 undef, i8* %val1.as.i8, i8* %val2.as.i8
  %valueToStore = bitcast i8* %valueToStore.as.i8 to %struct.test01a*
  store %struct.test01a* %valueToStore, %struct.test01a** @varTest01
  ret void
}

define %struct.test01b* @test01b() !dtrans_type !5 {
  ret %struct.test01b* null
}

define %struct.test01c* @test01c() !dtrans_type !8 {
  ret %struct.test01c* null
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Bad casting | Unsafe pointer store | Global pointer | Unsafe pointer merge{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Bad casting | Unsafe pointer store | Unsafe pointer merge{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01c
; CHECK: Safety data: Bad casting | Unsafe pointer store | Unsafe pointer merge{{ *$}}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{!4, i32 1}  ; %struct.test01a*
!4 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!5 = !{!"F", i1 false, i32 0, !6}  ; %struct.test01b* ()
!6 = !{!7, i32 1}  ; %struct.test01b*
!7 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!8 = !{!"F", i1 false, i32 0, !9}  ; %struct.test01c* ()
!9 = !{!10, i32 1}  ; %struct.test01c*
!10 = !{!"R", %struct.test01c zeroinitializer, i32 0}  ; %struct.test01c
!11 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!12 = !{!"S", %struct.test01b zeroinitializer, i32 3, !2, !2, !1} ; { i16, i16, i32 }
!13 = !{!"S", %struct.test01c zeroinitializer, i32 4, !2, !2, !2, !2} ; { i16, i16, i16, i16 }

!dtrans_types = !{!11, !12, !13}
