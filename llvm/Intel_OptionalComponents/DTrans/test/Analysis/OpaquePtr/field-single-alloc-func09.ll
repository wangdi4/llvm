; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Test that fields that are aggregate types get marked as "Bottom Alloc Function".

%struct.INNER = type { ptr, ptr }
%struct.MYSTRUCT = type { ptr, %struct.INNER, [8 x i64] }
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { ptr, %struct.INNER, [8 x i64] }
; CHECK: Name: struct.MYSTRUCT
; CHECK: Number of fields: 3
; CHECK: 0)Field LLVM Type: ptr
; CHECK: DTrans Type: i8*
; CHECK: Multiple Value: [ null ] <incomplete>
; CHECK: Single Alloc Function: ptr @malloc
; CHECK: 1)Field LLVM Type: %struct.INNER = type { ptr, ptr }
; CHECK: DTrans Type: %struct.INNER = type { i64*, i64* }
; CHECK: Multiple Value: [  ] <incomplete>
; CHECK: Bottom Alloc Function
; CHECK: 2)Field LLVM Type: [8 x i64]
; CHECK: DTrans Type: [8 x i64]
; CHECK: Multiple Value: [  ] <incomplete>
; CHECK: Bottom Alloc Function
; CHECK: Safety data: Contains nested structure | Local instance
; CHECK: End LLVMType: %struct.MYSTRUCT

define i32 @main() {
  %localstruct = alloca %struct.MYSTRUCT
  %mem = tail call noalias ptr @malloc(i64 100)
  %field_addr = getelementptr %struct.MYSTRUCT, ptr %localstruct, i64 0, i32 0
  store ptr %mem, ptr %field_addr, align 8
  tail call void @free(ptr %mem)
  store ptr null, ptr %field_addr, align 8
  ret i32 0
}

declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0
declare !intel.dtrans.func.type !7 void @free(ptr "intel_dtrans_func_index"="1" nocapture) #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i64 0, i32 1}  ; i64*
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{%struct.INNER zeroinitializer, i32 0}  ; %struct.INNER
!4 = !{!"A", i32 8, !5}  ; [8 x i64]
!5 = !{i64 0, i32 0}  ; i64
!6 = distinct !{!2}
!7 = distinct !{!2}
!8 = !{!"S", %struct.INNER zeroinitializer, i32 2, !1, !1} ; { i64*, i64* }
!9 = !{!"S", %struct.MYSTRUCT zeroinitializer, i32 3, !2, !3, !4} ; { i8*, %struct.INNER, [8 x i64] }

!intel.dtrans.types = !{!8, !9}

