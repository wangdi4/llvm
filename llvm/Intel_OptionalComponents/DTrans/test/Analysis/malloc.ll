; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-allocations -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies correct identification of calls to malloc and analysis
; of arguments to those calls by the DTransAnalysis.

; 'good' and 'bad' type variations are used for testing safety checking.
; Types named with 'good' should have no safety issues.

; struct S1 {
;   int  a;
;   int  b;
; };
%struct.good.S1 = type { i32, i32 }
%struct.bad.S1 = type { i32, i32 }

; struct S2 {
;   int        a[32];
;   struct S1  s1;
; };
%struct.good.S2 = type { [32 x i32], %struct.good.S1 }
%struct.bad.S2 = type { [32 x i32], %struct.bad.S1 }


define void @test1() {
  ; s1 = (struct S1*)malloc(sizeof(struct S1));
  %p = call noalias i8* @malloc(i64 8)
  %s1 = bitcast i8* %p to %struct.good.S1*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.S1 = type { i32, i32 }

define void @test2() {
  ; s2 = (struct S2*)malloc(sizeof(struct S2));
  %p = tail call noalias i8* @malloc(i64 136)
  %s2 = bitcast i8* %p to %struct.good.S2*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.S2 = type { [32 x i32], %struct.good.S1 }

; This test checks the case where GEP is used to access fields of an
; allocated structure.
define void @test3() {
  ; s2 = (struct S2*)malloc(sizeof(struct S2));
  %p = tail call noalias i8* @malloc(i64 136)
  %s2 = bitcast i8* %p to %struct.good.S2*
  ; s2->s1
  %p2 = getelementptr inbounds i8, i8* %p, i64 128
  %s1 = bitcast i8* %p2 to %struct.good.S1*
  ; s2->a[4]
  %p3 = getelementptr inbounds i8, i8* %p, i64 16
  %pa4 = bitcast i8* %p3 to i32*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.S2 = type { [32 x i32], %struct.good.S1 }

define void @test4() {
  ; s2 = (struct S2*)malloc(sizeof(struct S2));
  %p = tail call noalias i8* @malloc(i64 136)
  %s1 = bitcast i8* %p to %struct.bad.S1*
  %s2 = bitcast i8* %p to %struct.bad.S2*
  ret void
}

; CHECK: dtrans: Detected allocation cast to multiple types
; CHECK-DAG: Detected type: %struct.bad.S1 = type { i32, i32 }
; CHECK-DAG: Detected type: %struct.bad.S2 = type { [32 x i32], %struct.bad.S1 }

; This test checks the case where an array of structures is allocated.
define void @test5() {
  ; s1 = (struct S1*)malloc(32*sizeof(struct S1));
  %p = tail call noalias i8* @malloc(i64 256)
  %s1 = bitcast i8* %p to %struct.good.S1*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.S1 = type { i32, i32 }
; FIXME: Print information about the number of elements allocated?


; struct Inner {
;   int  a;
;   int  b;
; };
%struct.good.Inner = type { i32, i32 }
%struct.bad.Inner = type { i32, i32 }

; struct Middle {
;   struct Inner in;
;   int    c;
; };
%struct.good.Middle = type { %struct.good.Inner, i32 }
%struct.bad.Middle = type { %struct.bad.Inner, i32 }

; struct Outer {
;   struct Middle mid;
;   int    d;
; };
%struct.good.Outer = type { %struct.good.Middle, i32 }
%struct.bad.Outer = type { %struct.bad.Middle, i32 }

; This test checks the case where a bitcast is used to access element zero
; of a nested structure.
define void @test6() {
  ; out = (struct Outer*)malloc(sizeof(struct Outer));
  %p = tail call noalias i8* @malloc(i64 16)
  %out = bitcast i8* %p to %struct.good.Outer*
  ; mid = out->mid
  %mid = bitcast i8* %p to %struct.good.Middle*
  ; in = out->mid->in
  %in = bitcast i8* %p to %struct.good.Inner*
  ret void
}

; CHECK-NOT: dtrans: Detected allocation cast to multiple types
; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.Outer = type { %struct.good.Middle, i32 }

; struct Outer2 {
;   int    c;
;   struct Inner in;
; };
%struct.bad.Outer2 = type { i32, %struct.bad.Inner }

; This test checks the case where a bitcast is used that does not access
; element zero of a nested structure.
define void @test7() {
  ; out = (struct Outer2*)malloc(sizeof(struct Outer));
  %p = tail call noalias i8* @malloc(i64 16)
  %out = bitcast i8* %p to %struct.bad.Outer2*
  ; in = (struct Inner*)out
  %in = bitcast i8* %p to %struct.bad.Inner*
  ret void
}

; CHECK: dtrans: Detected allocation cast to multiple types
; CHECK-DAG: Detected type: %struct.bad.Outer2 = type { i32, %struct.bad.Inner }
; CHECK-DAG: Detected type: %struct.bad.Inner = type { i32, i32 }

; struct Left {
;   struct Right *p;
;   int  a;
; };
%struct.good.Left = type { %struct.good.Right*, i32 }
%struct.bad.Left = type { %struct.bad.Right*, i32 }

; struct Right {
;   struct Left *p;
;   int    b;
; };
%struct.good.Right = type { %struct.good.Left*, i32 }
%struct.bad.Right = type { %struct.bad.Left*, i32 }

; This test checks the case where a bitcast is used to access element zero
; where that element is a pointer to another structure.
define void @test8() {
  ; pl = (struct Left*)malloc(sizeof(struct Left));
  %p = tail call noalias i8* @malloc(i64 16)
  %pl = bitcast i8* %p to %struct.good.Left*
  ; ppr = pl->p
  %ppr = bitcast i8* %p to %struct.good.Right**
  ret void
}

; CHECK-NOT: dtrans: Detected allocation cast to multiple types
; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.Left = type { %struct.good.Right*, i32 }

; This test checks the case where an allocated pointer is bitcast to
; multiple types that are at a different level of indirection than element
; zero of these structure.
define void @test9() {
  ; pl = (struct Left*)malloc(sizeof(struct Left));
  %p = tail call noalias i8* @malloc(i64 16)
  %pl = bitcast i8* %p to %struct.bad.Left*
  ; pr = (struct Right*)p
  %pr = bitcast i8* %p to %struct.bad.Right*
  ret void
}

; CHECK: dtrans: Detected allocation cast to multiple types
; CHECK-DAG: Detected type: %struct.bad.Left = type { %struct.bad.Right*, i32 }
; CHECK-DAG: Detected type: %struct.bad.Right = type { %struct.bad.Left*, i32 }

; This test checks the case where an allocated value is passed to a PHI node
; before it is bitcast to the allocated type.
define %struct.good.S1* @test10(%struct.good.S1* %p) {
entry:
  %origS = bitcast %struct.good.S1* %p to i8*
  %isNull = icmp eq %struct.good.S1* %p, null
  br i1 %isNull, label %new, label %end

new:
  %newS = call i8* @malloc(i64 8)
  br label %end

end:
  %tmp = phi i8* [%origS, %entry], [%newS, %new]
  %val = bitcast i8* %tmp to %struct.good.S1*
  ret %struct.good.S1* %val
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.S1 = type { i32, i32 }

; This structure will be allocated with a bad size argument.
%struct.badsize.S1 = type { i32, i32 }

define void @test11() {
  ; s1 = (struct S1*)malloc(<some random size>);
  %p = call noalias i8* @malloc(i64 19)
  %s1 = bitcast i8* %p to %struct.badsize.S1*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.badsize.S1 = type { i32, i32 }

; This %struct.good.S1 structure will be allocated with a varibale size
; that can be proven to be a multiple of the struct size.
define void @test12(i64 %num1, i64 %num2) {
  ; s1 = (struct S1*)malloc((num1 * 3 * sizeof(struct S1)) * num2);
  %mul1 = mul i64 %num1, 24
  %mul2 = mul i64 %mul1, %num2
  %p = call noalias i8* @malloc(i64 %mul2)
  %s1 = bitcast i8* %p to %struct.good.S1*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.S1 = type { i32, i32 }

; This structure will be allocated with a variable size argument that cannot
; be proven to be a multiple of the struct size.
%struct.badsize.S2 = type { i32, i32, i32 }

define void @test13(i64 %num1, i64 %num2) {
  ; s1 = (struct S1*)malloc((num1 * 19) * num2);
  %mul1 = mul i64 %num1, 19
  %mul2 = mul i64 %mul1, %num2
  %p = call noalias i8* @malloc(i64 %mul2)
  %s1 = bitcast i8* %p to %struct.badsize.S2*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.badsize.S2 = type { i32, i32, i32 }

; This structure will be allocated with a variable size argument that we don't
; know anything about.
%struct.badsize.S3 = type { i32, i32 }

define void @test14(i64 %size) {
  ; s1 = (struct S1*)malloc(<some random size>);
  %p = call noalias i8* @malloc(i64 %size)
  %s1 = bitcast i8* %p to %struct.badsize.S3*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.badsize.S3 = type { i32, i32 }

; This test checks a case where a bitcast that allows us to determine the
; type of an allocated pointer is visited before the allocation call.
; When the bitcast is visited for analysis, the LocalPointerAnalyzer must
; proactively find the allocation call. If we wait until the allocation is
; visited during analysis, the bitcast will be incorrectly flagged as a
; bad bitcast.
define void @test15() {
entry:
  br label %merge

start:
  %curP = bitcast i8* %next.p to %struct.good.S1*
  br i1 undef, label %new, label %latch

new:
  %newP = tail call noalias i8* @malloc(i64 8)
  br label %merge

latch:
  br i1 undef, label %done, label %start

merge:
  %next.p = phi i8* [ null, %entry ], [ %newP, %new ]
  br label %start

done:
  ret void
}

; Here we must look through a sext to find the size being used.
define void @test16(i32 %num1, i64 %num2) {
  ; s1 = (struct S1*)malloc((num1 * 3 * sizeof(struct S1)) * num2);
  %mul1 = mul i32 %num1, 24
  %tmp = sext i32 %mul1 to i64
  %mul2 = mul i64 %tmp, %num2
  %p = call noalias i8* @malloc(i64 %mul2)
  %s1 = bitcast i8* %p to %struct.good.S1*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.S1 = type { i32, i32 }

; Here we must look through a zext to find the size being used.
define void @test17(i32 %num1, i64 %num2) {
  ; s1 = (struct S1*)malloc((num1 * 3 * sizeof(struct S1)) * num2);
  %mul1 = mul i32 %num1, 24
  %tmp = zext i32 %mul1 to i64
  %mul2 = mul i64 %tmp, %num2
  %p = call noalias i8* @malloc(i64 %mul2)
  %s1 = bitcast i8* %p to %struct.good.S1*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.S1 = type { i32, i32 }

; Here we make sure that our sext handling isn't masking a bad size.
%struct.badsize.S4 = type { i32, i32, i32 }
define void @test18(i32 %num1, i64 %num2) {
  ; s1 = (struct S4*)malloc((num1 * 19) * num2);
  %mul1 = mul i32 %num1, 19
  %tmp = sext i32 %mul1 to i64
  %mul2 = mul i64 %tmp, %num2
  %p = call noalias i8* @malloc(i64 %mul2)
  %s1 = bitcast i8* %p to %struct.badsize.S4*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.badsize.S4 = type { i32, i32, i32 }

; Here we make sure that we don't fail on zero size arrays.
%struct.badsize.S5 = type { i32, i32, i32 }
define void @test19() {
  %p1 = call noalias i8* @malloc(i64 0)
  %p2 = bitcast i8* %p1 to [0 x %struct.badsize.S5]*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: [0 x %struct.badsize.S5]

; The primary thing being tested here is that we don't transfer
; bad alloc size to struct.S1. Since the type being allocated is
; a pointer to S1 (the malloc returns a pointer-to-pointer) there
; is no problem with transforming S1 even if we don't figure out
; the size for this malloc.
define void @test20(i64 %n) {
  ; s1 = (struct S1**)malloc(n);
  ; Since we're allocating pointers, n is assumed to be a multiple of
  ; the pointer size.
  %p = call noalias i8* @malloc(i64 %n)
  %s1 = bitcast i8* %p to %struct.good.S1**
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.S1*

define void @test21(i32 %n) {
  ; size = n * 4 * sizeof(S1)
  ;   (becomes) size = n * 4 * 8
  ;   (becomes) size = n << 5
  %n64 = zext i32 %n to i64
  %size = shl i64 %n64, 5
  ; s1 = (struct S1*)malloc(size);
  %p = call noalias i8* @malloc(i64 %size)
  %s1 = bitcast i8* %p to %struct.good.S1*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.S1 = type { i32, i32 }

define void @test22(i32 %n) {
  ; size = n * 4 * sizeof(S1)
  ;   (becomes) size = n * 4 * 8
  ;   (becomes) size = n << 5
  %size32 = shl i32 %n, 5
  %size = zext i32 %size32 to i64
  ; s1 = (struct S1*)malloc(size);
  %p = call noalias i8* @malloc(i64 %size)
  %s1 = bitcast i8* %p to %struct.good.S1*
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.good.S1 = type { i32, i32 }

; The allocation output immediately follows the test where the allocation
; occurs. All type safety info is printed at the end. We check that here.
; Types are sorted alphabetically for output.

; 'bad' types should have 'Bad casting'
; CHECK: DTRANS Analysis Types Created
; CHECK: LLVMType: %struct.bad.Inner
; CHECK: Safety data: Bad casting | Bad alloc size | Nested structure
; CHECK: LLVMType: %struct.bad.Left
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.bad.Outer2
; CHECK: Safety data: Bad casting | Bad alloc size | Contains nested structure
; CHECK: LLVMType: %struct.bad.Right
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.bad.S1
; CHECK: Safety data: Bad casting | Nested structure
; CHECK: LLVMType: %struct.bad.S2
; CHECK: Safety data: Bad casting | Contains nested structure

; 'badsize' types should have 'Bad alloc size'
; CHECK: LLVMType: %struct.badsize.S1
; CHECK: Safety data: Bad alloc size
; CHECK: LLVMType: %struct.badsize.S2
; CHECK: Safety data: Bad alloc size
; CHECK: LLVMType: %struct.badsize.S3
; CHECK: Safety data: Bad alloc size
; CHECK: LLVMType: %struct.badsize.S4
; CHECK: Safety data: Bad alloc size
; CHECK: LLVMType: %struct.badsize.S5
; CHECK: Safety data: Bad alloc size

; 'good' types should have 'No issues found'
; CHECK: LLVMType: %struct.good.Inner
; CHECK: Safety data: Nested structure
; CHECK: LLVMType: %struct.good.Left
; CHECK: Safety data: No issues found
; CHECK: LLVMType: %struct.good.Middle
; CHECK: Safety data: Nested structure | Contains nested structure
; CHECK: LLVMType: %struct.good.Outer
; CHECK: Safety data: Contains nested structure
; CHECK: LLVMType: %struct.good.Right
; CHECK: Safety data: No issues found
; CHECK: LLVMType: %struct.good.S1
; CHECK: Safety data: Nested structure
; CHECK: LLVMType: %struct.good.S2
; CHECK: Safety data: Contains nested structure

declare noalias i8* @malloc(i64)
