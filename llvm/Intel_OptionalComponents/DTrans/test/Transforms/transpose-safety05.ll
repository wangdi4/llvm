; This test verifies the safety analysis for the usage of values
; loaded from a dope vector in called functions when tracing values
; through PHI and select instructions.

target triple = "x86_64-unknown-linux-gnu"

; Run the test with all uses of the dope vector being supported.
; RUN: sed -e s/.TEST_SAFE_SELECT:// %s | sed -e s/.TEST_SAFE_PHI:// | opt -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck --check-prefix=CHECK-SAFE %s

; Run the test with SelectInst that should invalidate the candidate
; RUN: sed -e s/.TEST_UNSAFE_SELECT:// %s | sed -e s/.TEST_SAFE_PHI:// | opt -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck --check-prefix=CHECK-UNSAFE %s

; Run the test with PHINode that should invalidate the candidate
; RUN: sed -e s/.TEST_UNSAFE_PHI:// %s | sed -e s/.TEST_SAFE_SELECT:// | opt -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck --check-prefix=CHECK-UNSAFE %s


; This case is to verify that PHI nodes and select instructions are allowed
; for array pointer. This case is a safe use.
@test_var03 = internal global [9 x [9 x i32]] zeroinitializer
define void @test03() {
  %"var$14" = alloca { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %otherval = alloca i32
  %"var$14_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14", i64 0, i32 0
  %"var$14_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14", i64 0, i32 1
  %"var$14_$field2$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14", i64 0, i32 2
  %"var$14_$field3$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14", i64 0, i32 3
  %"var$14_$field4$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14", i64 0, i32 4
  %"var$14_$field6$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14", i64 0, i32 6, i64 0
  %"var$14_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$14_$field6$", i64 0, i32 0
  %"var$14_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$14_$field6$", i64 0, i32 1
  %"var$14_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$14_$field6$", i64 0, i32 2
  store i64 4, i64* %"var$14_$field1$", align 8
  store i64 2, i64* %"var$14_$field4$", align 8
  store i64 0, i64* %"var$14_$field2$", align 8
  %t0 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$14_$field6$_$field1$", i32 0)
  store i64 4, i64* %t0, align 8
  %t1 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$14_$field6$_$field2$", i32 0)
  store i64 1, i64* %t1, align 8
  %t2 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$14_$field6$_$field0$", i32 0)
  store i64 9, i64* %t2, align 8
  %t3 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$14_$field6$_$field1$", i32 1)
  store i64 36, i64* %t3, align 8
  %t4 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$14_$field6$_$field2$", i32 1)
  store i64 1, i64* %t4, align 8
  %t5 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$14_$field6$_$field0$", i32 1)
  store i64 9, i64* %t5, align 8
  store i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @test_var03, i64 0, i64 0, i64 0), i32** %"var$14_$field0$", align 8
  store i64 1, i64* %"var$14_$field3$", align 8

  call void @test03dv({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14", i32* %otherval)
  ret void
}

; Move the array pointer around with PHI/select.
define void @test03dv({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* noalias nocapture readonly %MYBLOCK, i32* %othervar) {
entry:
  %"MYBLOCK_$field0$" = getelementptr { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %MYBLOCK, i64 0, i32 0
  %"MYBLOCK_$field6$_$field1$" = getelementptr { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %MYBLOCK, i64 0, i32 6, i64 0, i32 1
  %"MYBLOCK_$field0$1" = load i32*, i32** %"MYBLOCK_$field0$", align 8
  %"AnotherMYBLOCK_$field0$1" = load i32*, i32** %"MYBLOCK_$field0$", align 8

  ; Load strides
  %t0 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"MYBLOCK_$field6$_$field1$", i32 0)
  %t1 = load i64, i64* %t0, align 8
  %t2 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"MYBLOCK_$field6$_$field1$", i32 1)
  %t3 = load i64, i64* %t2, align 8

  br i1 undef, label %merge, label %altpath2

altpath1:
  br label %altpath2

altpath2:
;TEST_SAFE_PHI:  %addr = phi i32* [  %"AnotherMYBLOCK_$field0$1", %entry], [%"MYBLOCK_$field0$1", %altpath1]
;TEST_UNSAFE_PHI:  %addr = phi i32* [  %othervar, %entry], [%"MYBLOCK_$field0$1", %altpath1]

  br label %merge

merge:
  %addr2 = phi i32* [ %"AnotherMYBLOCK_$field0$1", %entry], [%addr, %altpath2]
  %"AnotherMYBLOCK_$field0$2" = load i32*, i32** %"MYBLOCK_$field0$", align 8
;TEST_SAFE_SELECT:  %addr3 = select i1 undef, i32* %addr2, i32* %"AnotherMYBLOCK_$field0$2"
;TEST_UNSAFE_SELECT:  %addr3 = select i1 undef, i32* %othervar, i32* %"AnotherMYBLOCK_$field0$2"

  %t4 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %t3, i32* elementtype(i32) %addr3, i64 5)
  %t5 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %t1, i32* elementtype(i32) %t4, i64 5)
  store i32 5, i32* %t5, align 4

  ret void
}

; matches for when candidate is safe
; CHECK-SAFE: Transpose candidate: test_var03
; CHECK-SAFE: IsValid{{ *}}: true

; matches for when candidate is not safe
; CHECK-UNSAFE: Transpose candidate: test_var03
; CHECK-UNSAFE: IsValid{{ *}}: false

 ; Args: Rank, Lower Bound, Unit Stride (in bytes), Ptr, Element Index
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32)
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64)
