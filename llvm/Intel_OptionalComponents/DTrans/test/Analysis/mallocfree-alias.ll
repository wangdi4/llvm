; This test is to verify that DTrans Analysis of allocation/free can analyze
; function calls made via a GlobalAlias definition.


; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s


; Call a malloc wrapper function via a GlobalAlias. DTrans analysis should
; look through the alias to identify the @malloc_wrapper function to resolve
; this as safe.
%struct.test01 = type { i32, i32 }
@malloc_alias = internal alias i8* (i64), i8* (i64)* @malloc_wrapper

define internal i8* @malloc_wrapper(i64 %s) {
  %p = call i8* @malloc(i64 %s)
  ret i8* %p
}

define internal void @test01() {
  %p = call i8* @malloc_alias(i64 160)
  %s1 = bitcast i8* %p to %struct.test01*
  ret void
}
; CHECK: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: No issues found


; Call a free wrapper function via a GlobalAlias. DTrans analysis should
; look through the alias to identify the @malloc_wrapper function to resolve
; this as safe.
%struct.test02 = type { i32, i32 }
@free_alias = internal alias void (i8*), void (i8*)* @free_wrapper

define internal void @free_wrapper(i8* %ptr) {
  call void @free(i8* %ptr)
  ret void
}


define internal void @test02(%struct.test02* %p) {
  %tmp = bitcast %struct.test02* %p to i8*
  call void @free_alias(i8* %tmp)
  ret void
}
; CHECK: LLVMType: %struct.test02 = type { i32, i32 }
; CHECK: Safety data: No issues found


declare void @free(i8*)
declare i8* @malloc(i64)
