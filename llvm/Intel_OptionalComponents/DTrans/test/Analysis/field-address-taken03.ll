; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -dtrans-outofboundsok -disable-output 2>&1 | FileCheck %s

; Check that the OUTERSTRUCT is marked Field Address Taken, with the second
; field specifically AddressTaken, and that the INNERSTRUCT is Field
; Address Taken due to cascading from OUTERSTRUCT to INNERSTRUCT
; because of -dtrans-outofboundsok

%struct.OUTERSTRUCT = type { %struct.INNERSTRUCT, %struct.INNERSTRUCT }
%struct.INNERSTRUCT = type { i32, i32 }

@coxstruct = internal dso_local global %struct.OUTERSTRUCT { %struct.INNERSTRUCT { i32 1, i32 2 }, %struct.INNERSTRUCT { i32 3, i32 4 } }, align 4

define dso_local i32 @foo(%struct.INNERSTRUCT* nocapture readonly %pcox) local_unnamed_addr {
  %myint1 = getelementptr inbounds %struct.INNERSTRUCT, %struct.INNERSTRUCT* %pcox, i64 0, i32 1
  %t0 = load i32, i32* %myint1, align 4
  ret i32 %t0
}

define dso_local i32 @main() local_unnamed_addr {
  %call = tail call i32 @foo(%struct.INNERSTRUCT* getelementptr inbounds (%struct.OUTERSTRUCT, %struct.OUTERSTRUCT* @coxstruct, i64 0, i32 1))
  ret i32 %call
}
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.INNERSTRUCT = type { i32, i32 }
; CHECK: Field LLVM Type: i32
; CHECK: Field info:
; CHECK: Multiple Value
; CHECK: Field LLVM Type: i32
; CHECK: Field info: Read
; CHECK: Multiple Value
; CHECK: Safety data: Field address taken | Global instance | Has initializer list | Nested structure

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.OUTERSTRUCT = type { %struct.INNERSTRUCT, %struct.INNERSTRUCT }
; CHECK: Field LLVM Type: %struct.INNERSTRUCT = type { i32, i32 }
; CHECK: Field info:
; CHECK: Multiple Value
; CHECK: Field LLVM Type: %struct.INNERSTRUCT = type { i32, i32 }
; CHECK: Field info: ComplexUse AddressTaken
; CHECK: Multiple Value
; CHECK: Safety data: Field address taken | Global instance | Has initializer list | Contains nested structure


