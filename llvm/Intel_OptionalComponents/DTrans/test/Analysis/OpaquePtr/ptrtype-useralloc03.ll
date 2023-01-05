; REQUIRES: asserts
; RUN: opt -disable-output -debug-only=dtrans-alloc-collector -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test recognition of user function that wraps the allocation call. This case
; checks the control flow handling where a 'null' value comes from a phi node.

; CHECK: Identified as user allocation function: AcquireMagicMemory

%struct.test = type { i64, i64 }

define internal "intel_dtrans_func_index"="1" ptr @AcquireMagicMemory(i64 %size) !intel.dtrans.func.type !3 {
entry:
%mem = tail call ptr @malloc(i64 %size)
  %failed = icmp eq ptr %mem, null
  br i1 %failed, label %report, label %done
report:
  call void @reportError()
  br label %done
done:
  %res = phi ptr [ %mem, %entry ], [ null, %report ]
  ret ptr %res
}

define void @reportError() {
	ret void
}

define i64 @test() {
  %p = call ptr @AcquireMagicMemory(i64 16)

  %ps = bitcast ptr %p to ptr
  %f0 = getelementptr %struct.test, ptr %ps, i64 0, i32 0
  %f1 = getelementptr %struct.test, ptr %ps, i64 0, i32 1
  %v = load i64, ptr %f1
  store i64 0, ptr %f0
  store i64 1, ptr %f1

  ret i64 %v
}

declare !intel.dtrans.func.type !4 void @free(ptr "intel_dtrans_func_index"="1") #1
declare !intel.dtrans.func.type !5 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!intel.dtrans.types = !{!6}
!1 = !{i64 0, i32 0}  ; i64
!2 = !{i8 0, i32 1}  ; i8*
!3 = distinct !{!2}
!4 = distinct !{!2}
!5 = distinct !{!2}
!6 = !{!"S", %struct.test zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
