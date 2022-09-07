; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-allocations -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-allocations -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies correct identification of calls to calloc and analysis
; of arguments to those calls by the DTransAnalysis.

; 'good' and 'bad' type variations are used for testing safety checking.
; Types named with 'good' should have no safety issues.

; struct S1 {
;   int  a;
;   int  b;
; };
%struct.good.S1 = type { i32, i32 }

define void @test1() {
  ; s1 = (struct S1*)calloc(sizeof(struct S1), 1);
  %p = call noalias i8* @calloc(i64 8, i64 1)
  %s1 = bitcast i8* %p to %struct.good.S1*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.S1 = type { i32, i32 }

define void @test2() {
  ; s1 = (struct S1*)calloc(1, sizeof(struct S1));
  %p = call noalias i8* @calloc(i64 1, i64 8)
  %s1 = bitcast i8* %p to %struct.good.S1*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.S1 = type { i32, i32 }

; This test checks the case where an array of structures is allocated.
define void @test3() {
  ; s1 = (struct S1*)calloc(sizeof(struct S1), 6);
  %p = tail call noalias i8* @calloc(i64 8, i64 6)
  %s1 = bitcast i8* %p to %struct.good.S1*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.S1 = type { i32, i32 }

; This test also checks the case where an array of structures is allocated
; but with the arguments reversed.
define void @test4() {
  ; s1 = (struct S1*)calloc(6, sizeof(struct S1));
  %p = tail call noalias i8* @calloc(i64 6, i64 8)
  %s1 = bitcast i8* %p to %struct.good.S1*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.S1 = type { i32, i32 }

; This test checks the case where an array of structures is allocated and cast
; to an array pointer.
define void @test5() {
  ; s1 = (struct S1*)calloc(6*sizeof(struct S1), 1);
  %p = tail call noalias i8* @calloc(i64 48, i64 1)
  %s1 = bitcast i8* %p to [6 x %struct.good.S1]*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: [6 x %struct.good.S1]

; This test checks the case where an array of structures is allocated with
; separate size and count arguments then cast to an array pointer.
define void @test6() {
  ; s1 = (struct S1*)calloc(sizeof(struct S1), 6);
  %p = tail call noalias i8* @calloc(i64 8, i64 6)
  %s1 = bitcast i8* %p to [6 x %struct.good.S1]*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: [6 x %struct.good.S1]

; This test checks the case where an array of structures is allocated with
; separate and reversed size and count arguments then cast to an array pointer.
define void @test7() {
  ; s1 = (struct S1*)calloc(6, sizeof(struct S1));
  %p = tail call noalias i8* @calloc(i64 6, i64 8)
  %s1 = bitcast i8* %p to [6 x %struct.good.S1]*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: [6 x %struct.good.S1]

; This test checks the case where the count argument is an unknown value.
define void @test8(i64 %count) {
  ; s1 = (struct S1*)calloc(sizeof(struct S1), count);
  %p = call noalias i8* @calloc(i64 8, i64 %count)
  %s1 = bitcast i8* %p to %struct.good.S1*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.S1 = type { i32, i32 }

; This test checks the case where the arguments are reversed and an unknown
; value is used for the size but the count is a multiple of the element size.
define void @test9(i64 %count) {
  ; s1 = (struct S1*)calloc(count, 2*sizeof(struct S1));
  %p = call noalias i8* @calloc(i64 %count, i64 16)
  %s1 = bitcast i8* %p to %struct.good.S1*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.S1 = type { i32, i32 }

; This test checks the case where the count argument is an unknown value
; and the size argument is an unknown value multiplied by the element size.
define void @test10(i64 %x, i64 %count) {
  ; s1 = (struct S1*)calloc(x*sizeof(struct S1), count);
  %mul = mul i64 %x, 8
  %p = call noalias i8* @calloc(i64 %mul, i64 %count)
  %s1 = bitcast i8* %p to %struct.good.S1*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.S1 = type { i32, i32 }

; This test checks the case where the arguments are reversed and an unknown
; value is used for the size and the count is an unknown multiple of the
; element size.
define void @test11(i64 %x, i64 %count) {
  ; s1 = (struct S1*)calloc(count, x*sizeof(struct S1));
  %mul = mul i64 16, %x
  %p = call noalias i8* @calloc(i64 %count, i64 %mul)
  %s1 = bitcast i8* %p to %struct.good.S1*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.S1 = type { i32, i32 }

; This test checks the case where neither argument is a multiple of the
; element size.
%struct.bad.S1 = type { i32, i32 }
define void @test12() {
  ; s1 = (struct S1*)calloc(12, 2);
  %p = tail call noalias i8* @calloc(i64 12, i64 2)
  %s1 = bitcast i8* %p to %struct.bad.S1*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.bad.S1 = type { i32, i32 }

; This test checks the case where both arguments are unknown.
; element size.
%struct.bad.S2 = type { i32, i32 }
define void @test13(i64 %X, i64 %Y) {
  ; s1 = (struct S1*)calloc(X, Y);
  %p = tail call noalias i8* @calloc(i64 %X, i64 %Y)
  %s1 = bitcast i8* %p to %struct.bad.S2*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.bad.S2 = type { i32, i32 }

; The allocation output immediately follows the test where the allocation
; occurs. All type safety info is printed at the end. We check that here.
; Types are sorted alphabetically for output.


; CHECK: LLVMType: %struct.bad.S1
; CHECK: Safety data: Bad alloc size
; CHECK: LLVMType: %struct.bad.S2
; CHECK: Safety data: Bad alloc size

; CHECK: LLVMType: %struct.good.S1
; CHECK: Safety data: No issues found

; CHECK: LLVMType: [6 x %struct.good.S1]
; CHECK: Safety data: No issues found

declare noalias i8* @calloc(i64, i64)
