; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-outofboundsok=false -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check that the OUTERSTRUCT is marked Field Address Taken, with the second
; field specifically AddressTaken, and that the INNERSTRUCT is not Field
; Address Taken

%struct.OUTERSTRUCT = type { %struct.INNERSTRUCT, %struct.INNERSTRUCT }
%struct.INNERSTRUCT = type { i32, i32 }

@coxstruct = internal dso_local global %struct.OUTERSTRUCT { %struct.INNERSTRUCT { i32 1, i32 2 }, %struct.INNERSTRUCT { i32 3, i32 4 } }, align 4

declare i32 @foo(%struct.INNERSTRUCT* %pcox);

define i32 @main() {
  %call = tail call i32 @foo(%struct.INNERSTRUCT* getelementptr inbounds (%struct.OUTERSTRUCT, %struct.OUTERSTRUCT* @coxstruct, i64 0, i32 1))
  ret i32 %call
}
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.INNERSTRUCT = type { i32, i32 }
; CHECK: Field LLVM Type: i32
; CHECK: Field info:
; CHECK: Multiple Value
; CHECK: Field LLVM Type: i32
; CHECK: Field info:
; CHECK: Multiple Value
; CHECK: Safety data: Global instance | Has initializer list | Address taken | Nested structure

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.OUTERSTRUCT = type { %struct.INNERSTRUCT, %struct.INNERSTRUCT }
; CHECK: Field LLVM Type: %struct.INNERSTRUCT = type { i32, i32 }
; CHECK: Field info:
; CHECK: Multiple Value
; CHECK: Field LLVM Type: %struct.INNERSTRUCT = type { i32, i32 }
; CHECK: Field info: ComplexUse AddressTaken
; CHECK: Multiple Value
; CHECK: Safety data: Field address taken | Global instance | Has initializer list | Contains nested structure


