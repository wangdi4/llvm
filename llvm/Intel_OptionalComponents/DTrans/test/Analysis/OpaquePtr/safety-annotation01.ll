; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-safetyanalyzer-ir -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-safetyanalyzer-ir -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test case for the option 'dtrans-print-safetyanalyzer-ir' to verify the
; printing of the IR with comments that indicate the safety flags triggered by
; the instruction.

%struct.test01a = type { i32*, %struct.test01b }
%struct.test01b = type { i32, %struct.test01c }
%struct.test01c = type { i32 }
@globalInst01 = internal global %struct.test01a zeroinitializer
; CHECK: @globalInst01 = internal global %struct.test01a
; CHECK: ; -> Safety data: %struct.test01a : Global instance{{ *$}}
; CHECK: ; -> Safety data: %struct.test01b : Global instance[Cascaded]
; CHECK: ; -> Safety data: %struct.test01c : Global instance[Cascaded]

%struct.test02 = type { i64, i64 }
@globalPtrToAOS = internal global [2 x %struct.test02]* zeroinitializer, !intel_dtrans_type !6
; CHECK: @globalPtrToAOS = internal global [2 x %struct.test02]*
; CHECK: ; -> Safety data: %struct.test02 : Global pointer[Cascaded]
; CHECK: ; -> Safety data: [2 x %struct.test02] : Global pointer{{ *$}}

%struct.test03 = type { i32*, %struct.test03*, %struct.test03b* }
%struct.test03b = type { i32, i32 }
define void @test03(%struct.test03* "intel_dtrans_func_index"="1" %pStruct, i32* "intel_dtrans_func_index"="2" %p32) !intel.dtrans.func.type !11 {
  %pField = getelementptr %struct.test03, %struct.test03* %pStruct, i64 0, i32 1
  %pValue = load %struct.test03*, %struct.test03** %pField
  %badCast = bitcast %struct.test03* %pValue to i32*

  ; Test with a pointer carried safety condition.
  %badMerge = select i1 undef, i32* %p32, i32* %badCast

  ; Test with one pointer carried, and one non-pointer carried safety condition.
  %badLoad = load i32, i32* %badMerge
  ret void
}
; CHECK-LABEL: define void @test03
; CHECK-NEXT: %pField = getelementptr %struct.test03, %struct.test03* %pStruct, i64 0, i32 1
; CHECK-NEXT: %pValue = load %struct.test03*, %struct.test03** %pField
; CHECK-NEXT: %badCast = bitcast %struct.test03* %pValue to i32*
; CHECK-NEXT: %badMerge = select i1 undef, i32* %p32, i32* %badCast
; CHECK-NEXT: ; -> Safety data: %struct.test03 : Unsafe pointer merge{{ *$}}
; CHECK-NEXT: ; -> Safety data: %struct.test03b : Unsafe pointer merge[Cascaded][PtrCarried]
; CHECK-NEXT: %badLoad = load i32, i32* %badMerge
; CHECK-NEXT: ; -> Safety data: %struct.test03 : Bad casting{{ *$}}
; CHECK-NEXT: ; -> Safety data: %struct.test03 : Mismatched element access{{ *$}}
; CHECK-NEXT: ; -> Safety data: %struct.test03b : Bad casting[Cascaded][PtrCarried]
; CHECK-NEXT: ret void

%struct.test04a = type { i32, i32 }
%struct.test04b = type { i32, i32 }
define void @test04() {
  %pStruct = alloca %struct.test04a
  %pStruct.as.i64 = ptrtoint %struct.test04a* %pStruct to i64
  call void @testcallee04(i64 %pStruct.as.i64)
  ret void
}
; CHECK-LABEL: define void @test04
; CHECK-NEXT: %pStruct = alloca %struct.test04a
; CHECK-NEXT: ; -> Safety data: %struct.test04a : Local instance
; CHECK-NEXT: %pStruct.as.i64 = ptrtoint %struct.test04a* %pStruct to i64
; CHECK-NEXT: call void @testcallee04(i64 %pStruct.as.i64)
; CHECK-NEXT: ; -> Safety data: %struct.test04a : Address taken
; CHECK-NEXT: ret void

define void @testcallee04(i64 %in) {
  %pStruct = alloca %struct.test04b
  %pStruct.as.p64 = bitcast %struct.test04b* %pStruct to i64*
  store i64 %in, i64* %pStruct.as.p64
  ret void
}
; CHECK-LABEL: define void @testcallee04
; CHECK-NEXT: %pStruct = alloca %struct.test04b
; CHECK-NEXT: ; -> Safety data: %struct.test04b : Local instance{{ *$}}
; CHECK-NEXT: %pStruct.as.p64 = bitcast %struct.test04b* %pStruct to i64*
; CHECK-NEXT: store i64 %in, i64* %pStruct.as.p64
; CHECK-NEXT: ; -> Safety data: %struct.test04b : Bad casting{{ *$}}
; CHECK-NEXT: ; -> Safety data: %struct.test04b : Unsafe pointer store{{ *$}}
; CHECK-NEXT: ret void


!1 = !{i32 0, i32 1}  ; i32*
!2 = !{%struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!3 = !{i32 0, i32 0}  ; i32
!4 = !{%struct.test01c zeroinitializer, i32 0}  ; %struct.test01c
!5 = !{i64 0, i32 0}  ; i64
!6 = !{!7, i32 1}  ; [2 x %struct.test02]*
!7 = !{!"A", i32 2, !8}  ; [2 x %struct.test02]
!8 = !{%struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!9 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!10 = !{%struct.test03b zeroinitializer, i32 1}  ; %struct.test03b*
!11 = distinct !{!9, !1}
!12 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !2} ; { i32*, %struct.test01b }
!13 = !{!"S", %struct.test01b zeroinitializer, i32 2, !3, !4} ; { i32, %struct.test01c }
!14 = !{!"S", %struct.test01c zeroinitializer, i32 1, !3} ; { i32 }
!15 = !{!"S", %struct.test02 zeroinitializer, i32 2, !5, !5} ; { i64, i64 }
!16 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !9, !10} ; { i32*, %struct.test03*, %struct.test03b* }
!17 = !{!"S", %struct.test03b zeroinitializer, i32 2, !3, !3} ; { i32, i32 }
!18 = !{!"S", %struct.test04a zeroinitializer, i32 2, !3, !3} ; { i32, i32 }
!19 = !{!"S", %struct.test04b zeroinitializer, i32 2, !3, !3} ; { i32, i32 }

!intel.dtrans.types = !{!12, !13, !14, !15, !16, !17, !18, !19}
