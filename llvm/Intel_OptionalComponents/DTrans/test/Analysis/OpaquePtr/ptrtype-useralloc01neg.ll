; REQUIRES: asserts
; RUN: opt -disable-output -debug-only=dtrans-alloc-collector -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Negative test for user allocation/free recognition where allocated memory is
; not returned or freed.

; CHECK: Not user free function: ReleaseMagicMemory
; CHECK: Not user allocation function: AcquireMagicMemory

%struct.test = type { i64, i64 }

; Disallowed as user malloc because allocated memory is not returned.
define internal "intel_dtrans_func_index"="1" i8* @AcquireMagicMemory(i64 %size) !intel.dtrans.func.type !3 {
  %mem = call i8* @malloc(i64 %size)
  ret i8* null
}

; Disallowed as user free because memory freed is loaded from structure field
; when input argument represents %struct.test
define internal void @ReleaseMagicMemory(i8* "intel_dtrans_func_index"="1" %mem) !intel.dtrans.func.type !6 {
  %gep = getelementptr i8, i8* %mem, i64 8
  %cast = bitcast i8* %gep to i8**
  %val = load i8*, i8** %cast
  call void @free(i8* %val)
  ret void
}

define i64 @test() {
  %p = call i8* @AcquireMagicMemory(i64 16)

  %ps = bitcast i8* %p to %struct.test*
  %f0 = getelementptr %struct.test, %struct.test* %ps, i64 0, i32 0
  %f1 = getelementptr %struct.test, %struct.test* %ps, i64 0, i32 1
  %v = load i64, i64* %f1
  store i64 0, i64* %f0
  store i64 1, i64* %f1

  %orig = bitcast %struct.test* %ps to i8*
  call void @ReleaseMagicMemory(i8* %orig)

  ret i64 %v
}

declare !intel.dtrans.func.type !4 void @free(i8* "intel_dtrans_func_index"="1") #1
declare !intel.dtrans.func.type !5 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!intel.dtrans.types = !{!7}
!1 = !{i64 0, i32 0}  ; i64
!2 = !{i8 0, i32 1}  ; i8*
!3 = distinct !{!2}
!4 = distinct !{!2}
!5 = distinct !{!2}
!6 = distinct !{!2}
!7 = !{!"S", %struct.test zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
