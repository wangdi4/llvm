; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test DTrans safety analyzer handling of global variable initialization with
; getelementptr operators to get the address of elements from global arrays where
; the types match the expected type.
; This should not result in "Bad casting" or "Unsafe pointer store" safety bits.

%class.Function = type { i32 (...)** }
%class.FunctionNodeSet.base = type <{ %class.Function, i8 }>
%class.FunctionNodeSet = type { %class.FunctionNodeSet.base, [7 x i8] }
%struct.FunctionTableEntry = type { i16*, %class.Function* }
%class.FunctionObjectType = type { %class.Function, i32 }

@FunctionNodes = internal global %class.FunctionNodeSet zeroinitializer, align 8
@TypeName = internal constant [2 x i16] [i16 111, i16 98], align 16
@SetName = internal constant [3 x i16] [i16 110, i16 111, i16 100], align 16
@objectTypeFunction = internal global %class.FunctionObjectType zeroinitializer, align 8

; Initialize the pointers of this variable using pointers of the appropriate
; type from other global variables.
@FunctionTable = internal constant [3 x %struct.FunctionTableEntry] [
  %struct.FunctionTableEntry {
    i16* getelementptr inbounds ([3 x i16], [3 x i16]* @SetName, i32 0, i32 0),
	%class.Function* getelementptr inbounds (
	  %class.FunctionNodeSet,
	  %class.FunctionNodeSet* @FunctionNodes, i32 0, i32 0, i32 0)
  },
  %struct.FunctionTableEntry {
    i16* getelementptr inbounds ([2 x i16], [2 x i16]* @TypeName, i32 0, i32 0),
	%class.Function* getelementptr inbounds (
	  %class.FunctionObjectType,
	  %class.FunctionObjectType* @objectTypeFunction, i32 0, i32 0)
  },
  %struct.FunctionTableEntry zeroinitializer], align 16

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %class.Function
; CHECK: Safety data: Global instance | Nested structure | Has vtable{{ *}}
; CHECK: End LLVMType: %class.Function

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %class.FunctionNodeSet
; CHECK: Safety data: Global instance | Contains nested structure{{ *}}
; CHECK: End LLVMType: %class.FunctionNodeSet

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %class.FunctionNodeSet.base
; CHECK: Safety data: Global instance | Nested structure | Contains nested structure{{ *}}
; CHECK: End LLVMType: %class.FunctionNodeSet.base

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %class.FunctionObjectType
; CHECK: Safety data: Global instance | Contains nested structure{{ *}}
; CHECK: End LLVMType: %class.FunctionObjectType

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.FunctionTableEntry
; CHECK: Safety data: Global instance | Has initializer list | Global array{{ *}}
; CHECK: End LLVMType: %struct.FunctionTableEntry


!1 = !{!"F", i1 true, i32 0, !2}  ; i32 (...)
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!1, i32 2}  ; i32 (...)**
!4 = !{%class.Function zeroinitializer, i32 0}  ; %class.Function
!5 = !{i8 0, i32 0}  ; i8
!6 = !{%class.FunctionNodeSet.base zeroinitializer, i32 0}  ; %class.FunctionNodeSet.base
!7 = !{!"A", i32 7, !5}  ; [7 x i8]
!8 = !{i16 0, i32 1}  ; i16*
!9 = !{%class.Function zeroinitializer, i32 1}  ; %class.Function*
!10 = !{!"S", %class.Function zeroinitializer, i32 1, !3} ; { i32 (...)** }
!11 = !{!"S", %class.FunctionNodeSet.base zeroinitializer, i32 2, !4, !5} ; <{ %class.Function, i8 }>
!12 = !{!"S", %class.FunctionNodeSet zeroinitializer, i32 2, !6, !7} ; { %class.FunctionNodeSet.base, [7 x i8] }
!13 = !{!"S", %struct.FunctionTableEntry zeroinitializer, i32 2, !8, !9} ; { i16*, %class.Function* }
!14 = !{!"S", %class.FunctionObjectType zeroinitializer, i32 2, !4, !2 }; { %class.Function, i32 }

!intel.dtrans.types = !{!10, !11, !12, !13, !14}
