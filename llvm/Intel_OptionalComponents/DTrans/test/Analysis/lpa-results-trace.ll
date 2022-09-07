; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -debug-only=dtrans-lpa-results -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -debug-only=dtrans-lpa-results -disable-output 2>&1 | FileCheck %s

; This test checks that -dtrans-lpa-results trace reports the LPA results
; intermixed with an IR dump for instructions that produce pointer values
; and constant expression operands of the instructions.

%struct.test = type { i64, double, %struct.test*, [25 x i8] }
@test_var = internal global %struct.test zeroinitializer

; Test pointer value producing instructions.
define internal void @test01() {
  %f0 = getelementptr %struct.test, %struct.test* @test_var, i64 0, i32 0
  %v0 = load i64, i64* %f0

  %f1 = getelementptr %struct.test, %struct.test* @test_var, i64 0, i32 1
  %v1 = load double, double* %f1

  %f2 = getelementptr %struct.test, %struct.test* @test_var, i64 0, i32 2
  %v2 = load %struct.test*, %struct.test** %f2

  %f3 = getelementptr %struct.test, %struct.test* @test_var, i64 0, i32 3, i32 6
  %v3 = load i8, i8* %f3

  ret void
}
; CHECK-LABEL: void @test01()
; CHECK:  %f0 = getelementptr %struct.test, %struct.test* @test_var, i64 0, i32 0
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        i64*
; CHECK:      Element pointees:
; CHECK:        %struct.test @ 0

; CHECK:  %f1 = getelementptr %struct.test, %struct.test* @test_var, i64 0, i32 1
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        double*
; CHECK:      Element pointees:
; CHECK:        %struct.test @ 1

; CHECK:  %f2 = getelementptr %struct.test, %struct.test* @test_var, i64 0, i32 2
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        %struct.test**
; CHECK:      Element pointees:
; CHECK:        %struct.test @ 2

; CHECK:  %v2 = load %struct.test*, %struct.test** %f2
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        %struct.test*
; CHECK:      No element pointees.

; CHECK:  %f3 = getelementptr %struct.test, %struct.test* @test_var, i64 0, i32 3, i32 6
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        i8*
; CHECK:      Element pointees:
; CHECK:        [25 x i8] @ 6


; Test output for constant expression GEPOperators
define internal void @test02() {
  %v0 = load i64, i64* getelementptr (%struct.test, %struct.test* @test_var, i64 0, i32 0)
  %v1 = load double, double* getelementptr (%struct.test, %struct.test* @test_var, i64 0, i32 1)
  %v3 = load i8, i8* getelementptr (%struct.test, %struct.test* @test_var, i64 0, i32 3, i32 6)
  %v2 = load %struct.test*, %struct.test** getelementptr (%struct.test, %struct.test* @test_var, i64 0, i32 2)

  ret void
}
; These cases should report the local pointer analysis type of the constant
; expression used in the instruction.
; CHECK-LABEL: void @test02()
; CHECK:  %v0 = load i64, i64* getelementptr inbounds (%struct.test, %struct.test* @test_var, i64 0, i32 0)
; CHECK:        CE: i64* getelementptr inbounds (%struct.test, %struct.test* @test_var, i64 0, i32 0)
; CHECK:          LocalPointerInfo:
; CHECK:            Aliased types:
; CHECK:              i64*
; CHECK:            Element pointees:
; CHECK:              %struct.test @ 0

; CHECK:  %v1 = load double, double* getelementptr inbounds (%struct.test, %struct.test* @test_var, i64 0, i32 1)
; CHECK:        CE: double* getelementptr inbounds (%struct.test, %struct.test* @test_var, i64 0, i32 1)
; CHECK:          LocalPointerInfo:
; CHECK:            Aliased types:
; CHECK:              double*
; CHECK:            Element pointees:
; CHECK:              %struct.test @ 1

; CHECK:  %v3 = load i8, i8* getelementptr inbounds (%struct.test, %struct.test* @test_var, i64 0, i32 3, i32 6)
; CHECK:        CE: i8* getelementptr inbounds (%struct.test, %struct.test* @test_var, i64 0, i32 3, i32 6)
; CHECK:          LocalPointerInfo:
; CHECK:            Aliased types:
; CHECK:              i8*
; CHECK:            Element pointees:
; CHECK:              [25 x i8] @ 6

; This case uses a constant expression, and produces a pointer value.
; Check both outputs.
; CHECK:  %v2 = load %struct.test*, %struct.test** getelementptr inbounds (%struct.test, %struct.test* @test_var, i64 0, i32 2)
; CHECK:        CE: %struct.test** getelementptr inbounds (%struct.test, %struct.test* @test_var, i64 0, i32 2)
; CHECK:          LocalPointerInfo:
; CHECK:            Aliased types:
; CHECK:              %struct.test**
; CHECK:            Element pointees:
; CHECK:;              %struct.test @ 2
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        %struct.test*
; CHECK:      No element pointees.


; Test that constant expressions contained within other constant expressions are
; printed.
%struct.test03 = type { i64*, %struct.test03* }
@test_var03 = internal global %struct.test03 zeroinitializer
define internal void @test03() {
  %local = alloca i64
  store i64 ptrtoint (%struct.test03** getelementptr (%struct.test03, %struct.test03* @test_var03, i64 0, i32 1) to i64), i64* %local
  ret void
}
; CHECK-LABEL: void @test03()
; CHECK: store i64 ptrtoint (%struct.test03** getelementptr inbounds (%struct.test03, %struct.test03* @test_var03, i64 0, i32 1) to i64), i64* %local
; CHECK:        CE: i64 ptrtoint (%struct.test03** getelementptr inbounds (%struct.test03, %struct.test03* @test_var03, i64 0, i32 1) to i64)
; CHECK:          LocalPointerInfo:
; CHECK:            Aliased types:
; CHECK:              %struct.test03**
; CHECK:            Element pointees:
; CHECK:              %struct.test03 @ 1

; CHECK:        CE: %struct.test03** getelementptr inbounds (%struct.test03, %struct.test03* @test_var03, i64 0, i32 1)
; CHECK:          LocalPointerInfo:
; CHECK:            Aliased types:
; CHECK:              %struct.test03**
; CHECK:            Element pointees:
; CHECK:              %struct.test03 @ 1
