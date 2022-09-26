; UNSUPPORTED: enable-opaque-pointers
; RUN:  opt < %s -S -o - -whole-program-assume -dtrans-resolvetypes 2>&1 | FileCheck %s
; RUN:  opt < %s -S -o - -whole-program-assume -passes=dtrans-resolvetypes 2>&1 | FileCheck %s

; This test verifies that the dtrans::ResolveTypes didn't combine the types
; since one of the types is used in a dllexport function. This is the same
; test case as test01 in resolve-types01.ll but the function test01_a is
; set as dllexport.

; These types should not be combined.
%struct.test01 = type { i32, i64, i32 }
%struct.test01.123 = type { i32, i64, i32 }
%struct.test01.456 = type { i32, i64, i32 }

; CHECK-NOT: %__DTRT_struct.test01 = type { i32, i64, i32 }
; CHECK-DAG: %struct.test01 = type { i32, i64, i32 }
; CHECK-DAG: %struct.test01.123 = type { i32, i64, i32 }
; CHECK-DAG: %struct.test01.456 = type { i32, i64, i32 }

; Check that the parameters for the functions weren't modifed.
define dllexport void @test01_a(%struct.test01* %p) {
  ret void
}
; CHECK: dllexport void @test01_a(%struct.test01* %p)

define void @test01_b(%struct.test01.123* %p) {
  ret void
}
; CHECK: void @test01_b(%struct.test01.123* %p)

define void @test01_c(%struct.test01.456* %p) {
  ret void
}
; CHECK: void @test01_c(%struct.test01.456* %p)

define void @test01() {
  %buf = call i8* @malloc(i32 16)
  %p = bitcast i8* %buf to %struct.test01*
  call void @test01_a(%struct.test01* %p)
  call void bitcast (void (%struct.test01.123*)* @test01_b
              to void (%struct.test01*)*) (%struct.test01* %p)
  call void bitcast (void (%struct.test01.456*)* @test01_c
              to void (%struct.test01*)*) (%struct.test01* %p)
  call void @free(i8* %buf)
  ret void
}

; Make sure that the callsites and the bitcasts didn't change.
; CHECK-LABEL: void @test01()
; CHECK:  %p = bitcast i8* %buf to %struct.test01*
; CHECK:  call void @test01_a(%struct.test01* %p)
; CHECK:  call void bitcast (void (%struct.test01.123*)* @test01_b to void (%struct.test01*)*)(%struct.test01* %p)
; CHECK:  call void bitcast (void (%struct.test01.456*)* @test01_c to void (%struct.test01*)*)(%struct.test01* %p)

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01()
  ret i32 0
}

declare i8* @malloc(i32)
declare void @free(i8*)
