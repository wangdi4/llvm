; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -passes='dtrans-deletefield' -S -o - %s | FileCheck %s

; This test verifies that the dtrans delete pass does not try to modify
; a structure when the allocation size is not a multiple of the structure
; size. This is a pattern of an allocation that is treated as safe by DTrans
; for the EliminateROFieldAccess transformation, but is not valid for the
; DeleteField transform. This is a regression test for CMPLRLLVM-23149.

; Field 1 of this structure is not used, but should not be deleted because
; the allocation size is not safe for the delete field transform.
%struct.test01 = type { i16, i64*, i32, i64 }

; Field 0 of this structure is not used, and delete field can delete it
; because there is only a global instance of the structure.
%struct.test01dep = type { i16, %struct.test01* }

@g_test01depptr = internal global %struct.test01dep zeroinitializer
define %struct.test01* @test01(i64 %num) {
  %size = mul nsw i64 %num, 32
  %size_plus = add i64 %size, 4
  %mem1 = call i8* @malloc(i64 %size_plus)

  ; Save the allocated memory into global variable's field
  %st_mem1 = bitcast i8* %mem1 to %struct.test01*
  store %struct.test01* %st_mem1, %struct.test01**
                                      getelementptr (%struct.test01dep,
                                          %struct.test01dep* @g_test01depptr,
                                          i64 0, i32 1)
  %f0 = getelementptr %struct.test01, %struct.test01* %st_mem1, i64 0, i32 0
  %f2 = getelementptr %struct.test01, %struct.test01* %st_mem1, i64 0, i32 2
  %f3 = getelementptr %struct.test01, %struct.test01* %st_mem1, i64 0, i32 3
  store i16 0, i16* %f0
  store i32 0, i32* %f2
  store i64 0, i64* %f3
  %v0 = load i16, i16* %f0
  %v2 = load i32, i32* %f2
  %v3 = load i64, i64* %f3

  %res = load %struct.test01*, %struct.test01**
                                   getelementptr (%struct.test01dep,
                                            %struct.test01dep* @g_test01depptr,
                                            i64 0, i32 1)
  ret %struct.test01* %res
}
; CHECK: %__DFT_struct.test01dep = type { %struct.test01* }
; CHECK-NOT: %__DFT_struct.test01 = type { i16, i32, i64 }

declare i8* @malloc(i64)
declare void @free(i8*)
