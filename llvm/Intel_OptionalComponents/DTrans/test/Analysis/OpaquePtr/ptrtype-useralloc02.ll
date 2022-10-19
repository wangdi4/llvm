; REQUIRES: asserts
; RUN: opt -disable-output -debug-only=dtrans-alloc-collector -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest < %s 2>&1 | FileCheck %s
; RUN: opt -disable-output -debug-only=dtrans-alloc-collector -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test recognition user functions that wrap the allocation/free calls when there
; is control flow.

; CHECK-DAG: Identified as user free function: ReleaseMagicMemory
; CHECK-DAG: Identified as user allocation function: AcquireMagicMemory

%struct.test = type { i64, i64 }

; User allocation call that returns either 'null' or allocated memory.
define internal "intel_dtrans_func_index"="1" i8* @AcquireMagicMemory(i64 %size) !intel.dtrans.func.type !3 {
  %mem = tail call i8* @malloc(i64 %size)
  %failed = icmp eq i8* %mem, null
  br i1 %failed, label %report, label %done
report:
  call void @reportError()
  ret i8* null
done:
  ret i8* %mem
}

; User free wrapper function that tests argument for 'null'
define internal void @ReleaseMagicMemory(i8* "intel_dtrans_func_index"="1" %mem) !intel.dtrans.func.type !6 {
  %is_null = icmp eq i8* %mem, null
  br i1 %is_null, label %done, label %do_free
do_free:
  call void @free(i8* %mem)
  br label %done
done:
  ret void
}

define void @reportError() {
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
