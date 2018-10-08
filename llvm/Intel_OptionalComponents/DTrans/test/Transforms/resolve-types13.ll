; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s

; This test verifies that the dtrans::ResolveTypes does not combine types when
; there are array fields with different numbers of elements.

%struct.A = type { [4 x i32], [1 x %struct.A*], [1 x float] }
%struct.A.11 = type { [4 x i32], [2 x %struct.A.11*], [1 x float] }

; CHECK: %struct.A = type { [4 x i32], [1 x %struct.A*], [1 x float] }
; CHECK: %struct.A.11 = type { [4 x i32], [2 x %struct.A.11*], [1 x float] }

define void @test_a(%struct.A* %p) {
  ret void
}

define void @test_a11(%struct.A.11* %p) {
  ret void
}

define void @test() {
  %bufa = call i8* @malloc(i32 32)
  %pa = bitcast i8* %bufa to %struct.A*
  %bufa11 = call i8* @malloc(i32 24)
  %pa11 = bitcast i8* %bufa11 to %struct.A.11*

  call void @test_a(%struct.A* %pa)
  call void @test_a11(%struct.A.11* %pa11)
  call void @free(i8* %bufa)
  call void @free(i8* %bufa11)
  ret void
}

define i32 @main(i32 %argc, i8** %argv) {
  call void @test()
  ret i32 0
}

declare i8* @malloc(i32)
declare void @free(i8*)
