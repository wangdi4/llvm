; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s

; This test verifies that the dtrans::ResolveTypes correctly does not attempt
; to combine types that meet its criteria but either are used directly by
; external function calls or have a dependent type which is used by an external
; function call.

%struct.test01 = type { i32, i64, i32 }
%struct.test01.123 = type { i32, i64, i32 }

declare void @use01(%struct.test01*)

define void @test01(%struct.test01* %p) {
  call void @use01(%struct.test01* %p)
  ret void
}

define void @test01_b(%struct.test01.123* %p) {
  call void bitcast (void (%struct.test01*)* @use01
              to void (%struct.test01.123*)*) (%struct.test01.123* %p)
  ret void
}

; CHECK-DAG: %struct.test01 = type { i32, i64, i32 }
; CHECK-DAG: %struct.test01.123 = type { i32, i64, i32 }


%struct.test02 = type { i32, i64, i32 }
%struct.test02.456 = type { i32, i64, i32 }

declare void @use02(%struct.test02.456*)

define void @test02(%struct.test02* %p) {
  call void bitcast (void (%struct.test02.456*)* @use02
              to void (%struct.test02*)*) (%struct.test02* %p)
  ret void
}

define void @test02_b(%struct.test02.456* %p) {
  call void @use02(%struct.test02.456* %p)
  ret void
}

; CHECK-DAG: %struct.test02 = type { i32, i64, i32 }
; CHECK-DAG: %struct.test02.456 = type { i32, i64, i32 }


%struct.test03 = type { i32, i64, i32 }
%struct.test03.789 = type { i32, i64, i32 }
%struct.test03dep = type { i32, %struct.test03* }

declare void @use03dep(%struct.test03dep*)

define void @test03(%struct.test03* %p, %struct.test03dep* %p2) {
  call void @use03dep(%struct.test03dep* %p2)
  ret void
}

define void @test03_b(%struct.test03.789* %p, %struct.test03dep* %p2) {
  call void @use03dep(%struct.test03dep* %p2)
  ret void
}

; CHECK-DAG: %struct.test03 = type { i32, i64, i32 }
; CHECK-DAG: %struct.test03.789 = type { i32, i64, i32 }
; CHECK-DAG: %struct.test03dep = type { i32, %struct.test03* }


%struct.test04 = type { i32, i64, i32 }
%struct.test04.987 = type { i32, i64, i32 }
%struct.test04dep = type { i32, %struct.test04.987* }

declare void @use04dep(%struct.test04dep*)

define void @test04(%struct.test04* %p, %struct.test04dep* %p2) {
  call void @use04dep(%struct.test04dep* %p2)
  ret void
}

define void @test04_b(%struct.test04.987* %p, %struct.test04dep* %p2) {
  call void @use04dep(%struct.test04dep* %p2)
  ret void
}

; CHECK-DAG: %struct.test04 = type { i32, i64, i32 }
; CHECK-DAG: %struct.test04.987 = type { i32, i64, i32 }
; CHECK-DAG: %struct.test04dep = type { i32, %struct.test04.987* }
