; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -dtrans-print-callinfo -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-print-callinfo -disable-output 2>&1 | FileCheck %s

; Regression test for CMPLRLLVM-31233 to ensure that the pattern matching
; of the user allocation function does not result in an infinite loop.

%struct.test = type { i64, i64 }
define void @test() {
    %raw = call i8* @test_alloc(i64 16)
    %st = bitcast i8* %raw to %struct.test*
    %f0 = getelementptr %struct.test, %struct.test* %st, i64 0, i32 0
    %f1 = getelementptr %struct.test, %struct.test* %st, i64 0, i32 1
    store i64 0, i64* %f0
    store i64 0, i64* %f1
    ret void
}

; This function should not be detected as a user allocation function
; due to the multiply instruction.
define i8* @test_alloc(i64 %size) {
  %multiple = mul i64 %size, 8
  %with_extra = add i64 %multiple, 24
  %raw = call i8* @malloc(i64 %with_extra)
  %isnull = icmp eq i8* %raw, null
  br i1 %isnull, label %error, label %success
error:
  call void @fatal()
  br label %done

success:
  br label %done

done:
  %mem = getelementptr i8, i8* %raw, i64 24
  ret i8* %mem
}

declare void @fatal()
declare i8* @malloc(i64)

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test = type { i64, i64 }
; CHECK: Name: struct.test
; CHECK:   Safety data: Bad casting

; CHECK-NOT: Kind: UserMalloc
