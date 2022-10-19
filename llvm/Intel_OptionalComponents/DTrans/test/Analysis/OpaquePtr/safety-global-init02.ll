; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test is to check that a global variable that is a structure type gets the
; 'Bad casting' safety flag when the variable is used as an initializer of a
; structure field that does not match the expected type.

%struct.test01.eh.ThrowInfo = type { i32, i8*, i8*, i8* }
%struct.test01.eh.CatchableType = type { i32, i8*, i32, i32, i32, i32, i8* }
%struct.test01.eh.CatchableTypeArray.1 = type { i32, [1 x %struct.test01.eh.CatchableType*] }

; The bitcast of symbol _CTA1H to a different type than the field type should
; trigger a safety violation for the type it is declared as,
; %struct.test01.eh.CatchableType. (The 'bad casting' also gets pointer carried
; to %struct.test01.eh.CatchableType.1)
@_TI1H = internal unnamed_addr constant %struct.test01.eh.ThrowInfo { i32 0, i8* null, i8* null, i8* bitcast (%struct.test01.eh.CatchableTypeArray.1* @_CTA1H to i8*) }
@_CTA1H = internal unnamed_addr constant %struct.test01.eh.CatchableTypeArray.1 { i32 1, [1 x %struct.test01.eh.CatchableType*] [%struct.test01.eh.CatchableType* null ] }

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01.eh.CatchableType
; CHECK: Safety data: Bad casting{{ *$}}
; CHECK: End LLVMType: %struct.test01.eh.CatchableType

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01.eh.CatchableTypeArray.1
; CHECK: Safety data: Bad casting | Global instance | Has initializer list{{ *$}}
; CHECK: End LLVMType: %struct.test01.eh.CatchableTypeArray.1

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01.eh.ThrowInfo
; CHECK: Safety data: Unsafe pointer store | Global instance | Has initializer list{{ *$}}
; CHECK: End LLVMType: %struct.test01.eh.ThrowInfo

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{!"A", i32 1, !4}  ; [1 x %struct.test01.eh.CatchableType*]
!4 = !{%struct.test01.eh.CatchableType zeroinitializer, i32 1}  ; %struct.test01.eh.CatchableType*
!5 = !{!"S", %struct.test01.eh.ThrowInfo zeroinitializer, i32 4, !1, !2, !2, !2} ; { i32, i8*, i8*, i8* }
!6 = !{!"S", %struct.test01.eh.CatchableType zeroinitializer, i32 7, !1, !2, !1, !1, !1, !1, !2} ; { i32, i8*, i32, i32, i32, i32, i8* }
!7 = !{!"S", %struct.test01.eh.CatchableTypeArray.1 zeroinitializer, i32 2, !1, !3} ; { i32, [1 x %struct.test01.eh.CatchableType*] }

!intel.dtrans.types = !{!5, !6, !7}
