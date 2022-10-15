; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-print-callinfo -disable-output %s 2>&1 | FileCheck %s

; Test that calls to a user allocation/free wrappers are treated as being safe
; for the aggregate type used.

%struct.test = type { i64, i64 }

; User allocation wrapper function\
define internal "intel_dtrans_func_index"="1" i8* @AcquireMagicMemory(i64 %size) !intel.dtrans.func.type !3 {
  %mem = call i8* @malloc(i64 %size)
  ret i8* %mem
}

; User free wrapper function
define internal void @ReleaseMagicMemory(i8* "intel_dtrans_func_index"="1" %mem) !intel.dtrans.func.type !6 {
  call void @free(i8* %mem)
  ret void
}

define i64 @test() {
 ; Using the pointer as a structure type should be treated as safe because the
 ; function was recognized as an user allocation wrapper.
  %p = call i8* @AcquireMagicMemory(i64 16)

  %ps = bitcast i8* %p to %struct.test*
  %f0 = getelementptr %struct.test, %struct.test* %ps, i64 0, i32 0
  %f1 = getelementptr %struct.test, %struct.test* %ps, i64 0, i32 1
  %v = load i64, i64* %f1
  store i64 0, i64* %f0
  store i64 1, i64* %f1

  ; Using a pointer in a function taking an i8* should be treated as safe
  ; because the function was recognized as being a user free call.
  %orig = bitcast %struct.test* %ps to i8*
  call void @ReleaseMagicMemory(i8* %orig)

  ret i64 %v
}

declare !intel.dtrans.func.type !4 void @free(i8* "intel_dtrans_func_index"="1") #1
declare !intel.dtrans.func.type !5 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test = type { i64, i64 }
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test

; CHECK-LABEL: Function: AcquireMagicMemory
; CHECK: Instruction:   %mem = call {{.*}} @malloc
; CHECK: AllocCallInfo:
; CHECK:   Kind: Malloc
; CHECK:   Aliased types:
; CHECK:     Type: Non-aggregate

; CHECK-LABEL: Function: ReleaseMagicMemory
; CHECK: Instruction:   call void @free
; CHECK: FreeCallInfo:
; CHECK:   Kind: Free
; CHECK:   Aliased types:
; CHECK:     Type: Non-aggregate

; CHECK-LABEL: Function: test
; CHECK: Instruction:   %p = call {{.*}} @AcquireMagicMemory
; CHECK: AllocCallInfo:
; CHECK:   Kind: UserMalloc
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test = type { i64, i64 }

; CHECK-LABEL: Function: test
; CHECK: Instruction:   call void @ReleaseMagicMemory
; CHECK: FreeCallInfo:
; CHECK:   Kind: UserFree
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test = type { i64, i64 }

!intel.dtrans.types = !{!7}
!1 = !{i64 0, i32 0}  ; i64
!2 = !{i8 0, i32 1}  ; i8*
!3 = distinct !{!2}
!4 = distinct !{!2}
!5 = distinct !{!2}
!6 = distinct !{!2}
!7 = !{!"S", %struct.test zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
