; RUN: opt < %s -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Show escaping conditions do invalidate the single alloc function

%struct.MYSTRUCT = type { i8* }

@globalstruct = internal global %struct.MYSTRUCT zeroinitializer, align 8

declare noalias i8* @malloc(i64)

declare void @notafree(i8* nocapture)

define i32 @main() {
  %1 = tail call noalias i8* @malloc(i64 100)
  store i8* %1, i8** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globalstruct, i64 0, i32 0), align 8
  %t1 = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* @globalstruct, i64 0, i32 0
  %t2 = load i8*, i8** %t1, align 8
  %2 = ptrtoint i8* %t2 to i64
  %3 = icmp eq i8* %t2, %t2
  br i1 %3, label %5, label %4
; <label>:4:
  tail call void @notafree(i8* %1)
  br label %5
; <label>:5:
  store i8* null, i8** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globalstruct, i64 0, i32 0), align 8
  %6 = trunc i64 %2 to i32
  ret i32 %6
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i8* }
; CHECK: Number of fields: 1
; CHECK: Field LLVM Type: i8*
; CHECK: Field info: Read Written
; CHECK: Multiple Value
; CHECK: Bottom Alloc Function
; CHECK: Safety data: Global instance
