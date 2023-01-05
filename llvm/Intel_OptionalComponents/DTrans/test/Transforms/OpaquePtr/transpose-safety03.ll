; This test verifies the safety analysis for the use of dope vectors
; in a called function

target triple = "x86_64-unknown-linux-gnu"

; Run the test with all uses of the dope vector being supported.
; RUN: opt < %s -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck --check-prefix=CHECK-SAFE %s

; Use the same basic framework, but with instructions in the called function
; that should invalidate the candidate based on the dope vector analysis
; RUN: opt -opaque-pointers < %s -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck --check-prefix=CHECK-SAFE %s
; RUN: sed -e s/.TEST_WRITE1:// %s | opt -opaque-pointers -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck --check-prefix=CHECK-UNSAFE %s
; RUN: sed -e s/.TEST_CAST:// %s | opt -opaque-pointers -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck --check-prefix=CHECK-UNSAFE %s
; RUN: sed -e s/.TEST_WRITE2:// %s | opt -opaque-pointers -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck --check-prefix=CHECK-UNSAFE %s
; RUN: sed -e s/.TEST_CALL:// %s | opt -opaque-pointers -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck --check-prefix=CHECK-UNSAFE %s

; This test verifies the uses of the dope vector fields.
@test_var01 = internal global [9 x [9 x i32]] zeroinitializer

; This routine is just for setting up the dope vector, and passing it to a function.
define void @test01() {
bb:
  %"var$14" = alloca { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %"var$14_$field0$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$14", i64 0, i32 0
  %"var$14_$field1$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$14", i64 0, i32 1
  %"var$14_$field2$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$14", i64 0, i32 2
  %"var$14_$field3$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$14", i64 0, i32 3
  %"var$14_$field4$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$14", i64 0, i32 4
  %"var$14_$field6$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$14", i64 0, i32 6, i64 0
  %"var$14_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$14_$field6$", i64 0, i32 0
  %"var$14_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$14_$field6$", i64 0, i32 1
  %"var$14_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$14_$field6$", i64 0, i32 2
  store i64 4, ptr %"var$14_$field1$", align 8
  store i64 2, ptr %"var$14_$field4$", align 8
  store i64 0, ptr %"var$14_$field2$", align 8
  %t0 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$14_$field6$_$field1$", i32 0)
  store i64 4, ptr %t0, align 8
  %t1 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$14_$field6$_$field2$", i32 0)
  store i64 1, ptr %t1, align 8
  %t2 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$14_$field6$_$field0$", i32 0)
  store i64 9, ptr %t2, align 8
  %t3 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$14_$field6$_$field1$", i32 1)
  store i64 36, ptr %t3, align 8
  %t4 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$14_$field6$_$field2$", i32 1)
  store i64 1, ptr %t4, align 8
  %t5 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$14_$field6$_$field0$", i32 1)
  store i64 9, ptr %t5, align 8
  store ptr @test_var01, ptr %"var$14_$field0$", align 8
  store i64 1, ptr %"var$14_$field3$", align 8
  call void @test01dv(ptr %"var$14")
  ret void
}

; Use the dope vector. Test is run with different variations on instructions
; used to trigger different safety considerations.
define void @test01dv(ptr noalias nocapture readonly %MYBLOCK) {
bb:
  %"MYBLOCK_$field0$" = getelementptr { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %MYBLOCK, i64 0, i32 0
  %"MYBLOCK_$field6$_$field1$" = getelementptr { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %MYBLOCK, i64 0, i32 6, i64 0, i32 1
  %"MYBLOCK_$field0$1" = load ptr, ptr %"MYBLOCK_$field0$", align 8
  %t0 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"MYBLOCK_$field6$_$field1$", i32 0)
  %t1 = load i64, ptr %t0, align 8

; Variations that test changing the value of the dope vector field
;TEST_WRITE1:  store ptr null, ptr %"MYBLOCK_$field0$"
;TEST_WRITE2:  store i64 1, ptr %t0

; Variation that tests an unsupported instruction on the dope pointer address,
;TEST_CAST:  %bc = ptrtoint ptr %"MYBLOCK_$field0$" to i64

  %t2 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"MYBLOCK_$field6$_$field1$", i32 1)
  %t3 = load i64, ptr %t2, align 8
  %t4 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %t3, ptr elementtype(i32) %"MYBLOCK_$field0$1", i64 5)
  %t5 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %t1, ptr elementtype(i32) %t4, i64 5)
  store i32 5, ptr %t5, align 4

; Variation that tests forwarding the dope vector to another routine, which
; is currently not supported.
;TEST_CALL:   call void @test01fwd({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %MYBLOCK)
  ret void
}

define void @test01fwd(ptr noalias nocapture readonly %MYBLOCK) {
bb:
  ret void
}

; matches for when candidate is safe
; CHECK-SAFE: Transpose candidate: test_var01
; CHECK-SAFE: IsValid{{ *}}: true

; matches for when candidate is not safe
; CHECK-UNSAFE: Transpose candidate: test_var01
; CHECK-UNSAFE: IsValid{{ *}}: false

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #0

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

attributes #0 = { nounwind readnone speculatable }
