; REQUIRES: asserts

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output -debug-only=dtransanalysis -dtrans-nosafetychecks-list="fsv:mystruct;fsaf:mystruct,str" 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output -debug-only=dtransanalysis -dtrans-nosafetychecks-list="fsv:mystruct;fsaf:mystruct,str" 2>&1 | FileCheck %s

; The test checks '-dtrans-nosafetychecks-list=' option functionality. This options allows ignoring safety check violations for types.

%struct.mystruct = type { i8*, i32 }

@globalstruct = internal global %struct.mystruct zeroinitializer, align 8

%__DFDT__struct.str = type { i32, i32 }
@b.str = internal unnamed_addr global %__DFDT__struct.str zeroinitializer

declare noalias i8* @malloc(i64)

define i32 @main() {
  %1 = tail call noalias i8* @malloc(i64 10)
  store i8* %1, i8** getelementptr inbounds (%struct.mystruct, %struct.mystruct* @globalstruct, i64 0, i32 0), align 8
  %s = bitcast i8** getelementptr (%struct.mystruct, %struct.mystruct* @globalstruct, i64 0, i32 0) to %__DFDT__struct.str*
  ret i32 0
}


; CHECK: dtrans-nosafetychecks-list:
; CHECK-NEXT:        Adding   'fsv:mystruct'
; CHECK-NEXT:        Adding   'fsaf:mystruct,str'

; This special remark would appear only if ignoring was effective
; CHECK: dtrans-fsv: ignoring struct.mystruct by user demand
; CHECK: dtrans-fsaf: ignoring struct.mystruct by user demand
; CHECK-NOT: dtrans-fsaf: ignoring struct.mystruct by user demand

; CHECK-LABEL:  LLVMType: %__DFDT__struct.str = type { i32, i32 }
; CHECK-NEXT:  Name: __DFDT__struct.str
; CHECK-NEXT:  (will be ignored in fsaf)
; CHECK-NEXT:  Number of fields: 2
; CHECK-NEXT:  Field LLVM Type: i32
; CHECK-NEXT:    Field info:
; CHECK-NEXT:    Frequency: 0
; CHECK-NEXT:    Multiple Value: [ 0 ] <incomplete>
; CHECK-NEXT:    Multiple IA Value: [  ] <incomplete>
; CHECK-NEXT:    Bottom Alloc Function (ignored)
; CHECK-NEXT:  Field LLVM Type: i32
; CHECK-NEXT:    Field info:
; CHECK-NEXT:    Frequency: 0
; CHECK-NEXT:    Multiple Value: [ 0 ] <incomplete>
; CHECK-NEXT:    Multiple IA Value: [  ] <incomplete>
; CHECK-NEXT:    Bottom Alloc Function (ignored)
; CHECK-NEXT:  Total Frequency: 0
; CHECK-NEXT:  Call graph:
; CHECK-NEXT:  Safety data: Bad casting | Global instance

; CHECK-LABEL:  LLVMType: %struct.mystruct = type { i8*, i32 }
; CHECK-NEXT:  Name: struct.mystruct
; CHECK-NEXT:  (will be ignored in fsv fsaf)
; CHECK-NEXT:  Number of fields: 2
; CHECK-NEXT:  Field LLVM Type: i8*
; CHECK-NEXT:    Field info: Written
; CHECK-NEXT:    Frequency: 1
; CHECK-NEXT:    Multiple Value: [ null ] <incomplete> (ignored)
; CHECK-NEXT:    Multiple IA Value: [  ] <incomplete>
; CHECK-NEXT:    Single Alloc Function: i8* (i64)* @malloc (ignored)
; CHECK-NEXT:  Field LLVM Type: i32
; CHECK-NEXT:    Field info:
; CHECK-NEXT:    Frequency: 0
; CHECK-NEXT:    Single Value: i32 0 (ignored)
; CHECK-NEXT:    Multiple IA Value: [  ] <incomplete>
; CHECK-NEXT:    Bottom Alloc Function (ignored)
; CHECK-NEXT:  Total Frequency: 1
; CHECK-NEXT:  Call graph:
; CHECK-NEXT:  Safety data: Bad casting | Global instance
