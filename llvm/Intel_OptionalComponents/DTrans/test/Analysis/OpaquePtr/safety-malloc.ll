; REQUIRES: asserts

; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes="require<dtrans-safetyanalyzer>" -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies correct identification of calls to malloc and analysis
; of arguments to those calls by the DTransSafetyAnalyzer.

; 'good' and 'bad' type variations are used for testing safety checking.
; Types named with 'good' do not have real safety issues.

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
  %a = alloca ptr, !intel_dtrans_type !5 ; %struct.good.S1*
  ; s1 = (struct S1*)malloc(sizeof(struct S1));
  %p = call noalias ptr @malloc(i64 8)
  store ptr %p, ptr %a
  ret void
}

define void @test2() {
  %a = alloca ptr, !intel_dtrans_type !6 ; %struct.good.S2*
  ; s2 = (struct S2*)malloc(sizeof(struct S2));
  %p = tail call noalias ptr @malloc(i64 136)
  store ptr %p, ptr %a
  ret void
}

; This test checks the case where GEP is used to access fields of an
; allocated structure.
define void @test3() {
  %a = alloca ptr, !intel_dtrans_type !6 ; %struct.good.S2*
  ; s2 = (struct S2*)malloc(sizeof(struct S2));
  %p = tail call noalias ptr @malloc(i64 136)
  store ptr %p, ptr %a

  ; s2->s1
  %p2 = getelementptr inbounds i8, ptr %p, i64 128

  ; s2->a[4]
  %p3 = getelementptr inbounds i8, ptr %p, i64 16

  ret void
}

define void @test4() {
  %a1 = alloca ptr, !intel_dtrans_type !7 ; %struct.bad.S1*
  %a2 = alloca ptr, !intel_dtrans_type !8 ; %struct.bad.S2*
  ; s2 = (struct S2*)malloc(sizeof(struct S2));
  %p = tail call noalias ptr @malloc(i64 136)
  store ptr %p, ptr %a1
  store ptr %p, ptr %a2
  ret void
}

; This test checks the case where an array of structures is allocated.
define void @test5() {
  %a = alloca ptr, !intel_dtrans_type !5 ; %struct.good.S1*
  ; s1 = (struct S1*)malloc(32*sizeof(struct S1));
  %p = tail call noalias ptr @malloc(i64 256)
  store ptr %p, ptr %a
  ret void
}

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

; This test checks the case where a pointer is used to access element zero
; of a nested structure.
define void @test6() {
  %o = alloca ptr, !intel_dtrans_type !13 ; %struct.good.Outer*
  %m = alloca ptr, !intel_dtrans_type !14 ; %struct.good.Middle*
  %i = alloca ptr, !intel_dtrans_type !15 ; %struct.good.Inner*
  ; out = (struct Outer*)malloc(sizeof(struct Outer));
  %p = tail call noalias ptr @malloc(i64 16)
  store ptr %p, ptr %o

  ; mid = out->mid
  store ptr %p, ptr %m
  
  ; in = out->mid->in
  store ptr %p, ptr %i
  ret void
}

; struct Outer2 {
;   int    c;
;   struct Inner in;
; };
%struct.bad.Outer2 = type { i32, %struct.bad.Inner }

; This test checks the case where a pointer is used that does not access
; element zero of a nested structure. Because the allocated pointer gets
; treated as multiple unrelated types here, the analysis cannot determine
; which type is being allocated, and will result in the 'Bad casting'
; safety flag.
define void @test7() {
  %a1 = alloca ptr, !intel_dtrans_type !16 ; %struct.bad.Outer2*
  %a2 = alloca ptr, !intel_dtrans_type !17 ; %struct.bad.Inner*
  ; out = (struct Outer2*)malloc(sizeof(struct Outer));
  %p = tail call noalias ptr @malloc(i64 16)
  store ptr %p, ptr %a1
  ; in = (struct Inner*)out
  store ptr %p, ptr %a2
  ret void
}

; struct Left {
;   struct Right *p;
;   int  a;
; };
%struct.good.Left = type { ptr, i32 }
%struct.bad.Left = type { ptr, i32 }

; struct Right {
;   struct Left *p;
;   int    b;
; };
%struct.good.Right = type { ptr, i32 }
%struct.bad.Right = type { ptr, i32 }

; This test checks the case where a pointer is used to access element zero
; where that element is a pointer to another structure.
define void @test8() {
  %a1 = alloca ptr, !intel_dtrans_type !20 ; %struct.good.Left*
  %a2 = alloca ptr, !intel_dtrans_type !22 ; %struct.good.Right**
  ; pl = (struct Left*)malloc(sizeof(struct Left));
  %p = tail call noalias ptr @malloc(i64 16)
  store ptr %p, ptr %a1

  ; ppr = pl->p
  store ptr %p, ptr %a2
  ret void
}

; This test checks the case where an allocated pointer is use as
; multiple types that are at a different level of indirection than element
; zero of these structure.
define void @test9() {
  %a1 = alloca ptr, !intel_dtrans_type !21 ; %struct.bad.Left*
  %a2 = alloca ptr, !intel_dtrans_type !19 ; %struct.bad.Right*
	
  ; pl = (struct Left*)malloc(sizeof(struct Left));
  %p = tail call noalias ptr @malloc(i64 16)
  store ptr %p, ptr %a1

  ; pr = (struct Right*)p
  store ptr %p, ptr %a2
  ret void
}

; This test checks the case where an allocated value is passed to a PHI node
; before it is returned as the allocated type.
define  "intel_dtrans_func_index"="1" ptr @test10(ptr "intel_dtrans_func_index"="2" %p) !intel.dtrans.func.type !23 {
entry:
  %isNull = icmp eq ptr %p, null
  br i1 %isNull, label %new, label %end

new:
  %newS = call ptr @malloc(i64 8)
  br label %end

end:
  %tmp = phi ptr [%p, %entry], [%newS, %new]
  ret ptr %tmp
}

; This structure will be allocated with a bad size argument.
%struct.badsize.S1 = type { i32, i32 }

define void @test11() {
  %a = alloca ptr, !intel_dtrans_type !24 ; %struct.badsize.S1*
  ; s1 = (struct S1*)malloc(<some random size>);
  %p = call noalias ptr @malloc(i64 19)
  store ptr %p, ptr %a
  ret void
}

; This %struct.good.S1 structure will be allocated with a varibale size
; that can be proven to be a multiple of the struct size.
define void @test12(i64 %num1, i64 %num2) {
  %a = alloca ptr, !intel_dtrans_type !5  ; %struct.good.S1*
  ; s1 = (struct S1*)malloc((num1 * 3 * sizeof(struct S1)) * num2);
  %mul1 = mul i64 %num1, 24
  %mul2 = mul i64 %mul1, %num2
  %p = call noalias ptr @malloc(i64 %mul2)
  store ptr %p, ptr %a
  ret void
}

; This structure will be allocated with a variable size argument that cannot
; be proven to be a multiple of the struct size.
%struct.badsize.S2 = type { i32, i32, i32 }

define void @test13(i64 %num1, i64 %num2) {
  %a = alloca ptr, !intel_dtrans_type !25 ; %struct.badsize.S2*
  ; s1 = (struct S1*)malloc((num1 * 19) * num2);
  %mul1 = mul i64 %num1, 19
  %mul2 = mul i64 %mul1, %num2
  %p = call noalias ptr @malloc(i64 %mul2)
  store ptr %p, ptr %a
  ret void
}

; This structure will be allocated with a variable size argument that we don't
; know anything about.
%struct.badsize.S3 = type { i32, i32 }

define void @test14(i64 %size) {
  %a = alloca ptr, !intel_dtrans_type !26 ; %struct.badsize.S3*
  ; s1 = (struct S1*)malloc(<some random size>);
  %p = call noalias ptr @malloc(i64 %size)
  store ptr %p, ptr %a
  ret void
}

; This test checks a case where a store that allows us to determine the
; type of an allocated pointer is visited before the allocation call.
define void @test15() {
entry:
  %a = alloca ptr, !intel_dtrans_type !5 ; %struct.good.S1*
  br label %merge

start:
  store ptr %next.p, ptr %a
  br i1 undef, label %new, label %latch

new:
  %newP = tail call noalias ptr @malloc(i64 8)
  br label %merge

latch:
  br i1 undef, label %done, label %start

merge:
  %next.p = phi ptr [ null, %entry ], [ %newP, %new ]
  br label %start

done:
  ret void
}

; Here we must look through a sext to find the size being used.
define void @test16(i32 %num1, i64 %num2) {

  %a = alloca ptr, !intel_dtrans_type !5 ; %struct.good.S1*
  ; s1 = (struct S1*)malloc((num1 * 3 * sizeof(struct S1)) * num2);
  %mul1 = mul i32 %num1, 24
  %tmp = sext i32 %mul1 to i64
  %mul2 = mul i64 %tmp, %num2
  %p = call noalias ptr @malloc(i64 %mul2)
  store ptr %p, ptr %a
  ret void
}

; Here we must look through a zext to find the size being used.
define void @test17(i32 %num1, i64 %num2) {
  %a = alloca ptr, !intel_dtrans_type !5 ; %struct.good.S1*
  ; s1 = (struct S1*)malloc((num1 * 3 * sizeof(struct S1)) * num2);
  %mul1 = mul i32 %num1, 24
  %tmp = zext i32 %mul1 to i64
  %mul2 = mul i64 %tmp, %num2
  %p = call noalias ptr @malloc(i64 %mul2)
  store ptr %p, ptr %a
  ret void
}

; Here we make sure that our sext handling isn't masking a bad size.
%struct.badsize.S4 = type { i32, i32, i32 }
define void @test18(i32 %num1, i64 %num2) {
  %a = alloca ptr, !intel_dtrans_type !27 ; %struct.badsize.S4*
  ; s1 = (struct S4*)malloc((num1 * 19) * num2);
  %mul1 = mul i32 %num1, 19
  %tmp = sext i32 %mul1 to i64
  %mul2 = mul i64 %tmp, %num2
  %p = call noalias ptr @malloc(i64 %mul2)
  store ptr %p, ptr %a
  ret void
}

; Here we make sure that we don't fail on zero size arrays.
%struct.badsize.S5 = type { i32, i32, i32 }
define void @test19() {
  %a = alloca ptr, !intel_dtrans_type !28  ; [0 x %struct.badsize.S5]*
  %p = call noalias ptr @malloc(i64 0)
  store ptr %p, ptr %a
  ret void
}

; The primary thing being tested here is that we don't transfer
; bad alloca size to struct.S1. Since the type being allocated is
; a pointer to S1 (the malloc returns a pointer-to-pointer) there
; is no problem with transforming S1 even if we don't figure out
; the size for this malloc.
define void @test20(i64 %n) {
  %a = alloca ptr, !intel_dtrans_type !31 ; %struct.good.S1**
  ; s1 = (struct S1**)malloc(n);
  ; Since we're allocating pointers, n is assumed to be a multiple of
  ; the pointer size.
  %p = call noalias ptr @malloc(i64 %n)
  store ptr %p, ptr %a
  ret void
}

define void @test21(i32 %n) {
  %a = alloca ptr, !intel_dtrans_type !5 ; %struct.good.S1*
  ; size = n * 4 * sizeof(S1)
  ;   (becomes) size = n * 4 * 8
  ;   (becomes) size = n << 5
  %n64 = zext i32 %n to i64
  %size = shl i64 %n64, 5
  ; s1 = (struct S1*)malloc(size);
  %p = call noalias ptr @malloc(i64 %size)
  store ptr %p, ptr %a
  ret void
}

define void @test22(i32 %n) {
  %a = alloca ptr, !intel_dtrans_type !5 ; %struct.good.S1*
  ; size = n * 4 * sizeof(S1)
  ;   (becomes) size = n * 4 * 8
  ;   (becomes) size = n << 5
  %size32 = shl i32 %n, 5
  %size = zext i32 %size32 to i64
  ; s1 = (struct S1*)malloc(size);
  %p = call noalias ptr @malloc(i64 %size)
  store ptr %p, ptr %a
  ret void
}

; Check with invalid shift amount which would cause the transformations
; to produce invalid code if accepted.
%struct.badsize.S6 = type { i32, i32 }
define void @test23(i32 %n) {
  %a = alloca ptr, !intel_dtrans_type !32 ; %struct.badsize.S6*
  %n64 = zext i32 %n to i64
  %size = shl i64 %n64, -5
  ; s1 = (struct S1*)malloc(size);
  %p = call noalias ptr @malloc(i64 %size)
  store ptr %p, ptr %a
  ret void
}

; Check with invalid shift amount which would cause the transformations
; to produce invalid code if accepted.
%struct.badsize.S7 = type { i32, i32 }
define void @test24(i32 %n) {
  %a = alloca ptr, !intel_dtrans_type !33 ; %struct.badsize.S7*
  %n64 = zext i32 %n to i64
  %size = shl i64 %n64, 80
  ; s1 = (struct S1*)malloc(size);
  %p = call noalias ptr @malloc(i64 %size)
  store ptr %p, ptr %a
  ret void
}

; CHECK: DTRANS Analysis Types Created

; 'bad' types should have 'Bad casting'. They may also have other safety flags
; set.

; CHECK: LLVMType: %struct.bad.Inner
; CHECK: Safety data: Bad casting
; CHECK: End LLVMType: %struct.bad.Inner

; CHECK: LLVMType: %struct.bad.Left
; CHECK: Safety data: Bad casting
; CHECK: End LLVMType: %struct.bad.Left

; CHECK: LLVMType: %struct.bad.Outer2
; CHECK: Safety data: Bad casting
; CHECK: End LLVMType: %struct.bad.Outer2

; CHECK: LLVMType: %struct.bad.Right
; CHECK: Safety data: Bad casting
; CHECK: End LLVMType: %struct.bad.Right

; CHECK: LLVMType: %struct.bad.S1
; CHECK: Safety data: Bad casting
; CHECK: End LLVMType: %struct.bad.S1

; CHECK: LLVMType: %struct.bad.S2
; CHECK: Safety data: Bad casting
; CHECK: End LLVMType: %struct.bad.S2

; 'badsize' types should have 'Bad alloc size'
; CHECK: LLVMType: %struct.badsize.S1
; CHECK: Safety data: Bad alloc size
; CHECK: End LLVMType: %struct.badsize.S1

; CHECK: LLVMType: %struct.badsize.S2
; CHECK: Safety data: Bad alloc size
; CHECK: End LLVMType: %struct.badsize.S2

; CHECK: LLVMType: %struct.badsize.S3
; CHECK: Safety data: Bad alloc size
; CHECK: End LLVMType: %struct.badsize.S3

; CHECK: LLVMType: %struct.badsize.S4
; CHECK: Safety data: Bad alloc size
; CHECK: End LLVMType: %struct.badsize.S4

; CHECK: LLVMType: %struct.badsize.S5
; CHECK: Safety data: Bad alloc size
; CHECK: End LLVMType: %struct.badsize.S5

; CHECK: LLVMType: %struct.badsize.S6
; CHECK: Safety data: Bad alloc size
; CHECK: End LLVMType: %struct.badsize.S6

; CHECK: LLVMType: %struct.badsize.S7
; CHECK: Safety data: Bad alloc size
; CHECK: End LLVMType: %struct.badsize.S7

; 'good' types should have no issues found, however the analysis does not currently
; consider element zero pointer types when analyzing these cases.
; TODO: Improve the element zero access analysis for these.
; CHECK: LLVMType: %struct.good.Inner
; Impvoved analysis could yield: Safety data: Nested structure | Local pointer
; CHECK: Safety data: Bad casting | Unsafe pointer store | Nested structure | Local pointer
; CHECK: End LLVMType: %struct.good.Inner

; CHECK: LLVMType: %struct.good.Left
; TODO: Impvoved analysis could yield: Safety data: Local pointer
; CHECK: Safety data: Bad casting | Unsafe pointer store | Local pointer
; CHECK: End LLVMType: %struct.good.Left

; CHECK: LLVMType: %struct.good.Middle
; TODO: Impvoved analysis could yield: Safety data: Nested structure | Contains nested structure | Local Pointer
; CHECK:  Safety data: Bad casting | Unsafe pointer store | Nested structure | Contains nested structure | Local pointer
; CHECK: End LLVMType: %struct.good.Middle

; CHECK: LLVMType: %struct.good.Outer
; TODO: Impvoved analysis could yield: Safety data: Contains nested structure | Local pointer
; CHECK: Safety data: Bad casting | Unsafe pointer store | Contains nested structure | Local pointer
; CHECK: End LLVMType: %struct.good.Outer

; CHECK: LLVMType: %struct.good.Right
; TODO: Impvoved analysis could yield: Safety data: Local pointer
; CHECK:  Safety data: Bad casting | Unsafe pointer store | Local pointer
; CHECK: End LLVMType: %struct.good.Right

; CHECK: LLVMType: %struct.good.S1
; CHECK:  Safety data: Nested structure | Local pointer
; CHECK: End LLVMType: %struct.good.S1

; CHECK: LLVMType: %struct.good.S2
; CHECK: Safety data: Contains nested structure | Local pointer
; CHECK: End LLVMType: %struct.good.S2

declare !intel.dtrans.func.type !35 noalias  "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"A", i32 32, !1}  ; [32 x i32]
!3 = !{%struct.good.S1 zeroinitializer, i32 0}  ; %struct.good.S1
!4 = !{%struct.bad.S1 zeroinitializer, i32 0}  ; %struct.bad.S1
!5 = !{%struct.good.S1 zeroinitializer, i32 1}  ; %struct.good.S1*
!6 = !{%struct.good.S2 zeroinitializer, i32 1}  ; %struct.good.S2*
!7 = !{%struct.bad.S1 zeroinitializer, i32 1}  ; %struct.bad.S1*
!8 = !{%struct.bad.S2 zeroinitializer, i32 1}  ; %struct.bad.S2*
!9 = !{%struct.good.Inner zeroinitializer, i32 0}  ; %struct.good.Inner
!10 = !{%struct.bad.Inner zeroinitializer, i32 0}  ; %struct.bad.Inner
!11 = !{%struct.good.Middle zeroinitializer, i32 0}  ; %struct.good.Middle
!12 = !{%struct.bad.Middle zeroinitializer, i32 0}  ; %struct.bad.Middle
!13 = !{%struct.good.Outer zeroinitializer, i32 1}  ; %struct.good.Outer*
!14 = !{%struct.good.Middle zeroinitializer, i32 1}  ; %struct.good.Middle*
!15 = !{%struct.good.Inner zeroinitializer, i32 1}  ; %struct.good.Inner*
!16 = !{%struct.bad.Outer2 zeroinitializer, i32 1}  ; %struct.bad.Outer2*
!17 = !{%struct.bad.Inner zeroinitializer, i32 1}  ; %struct.bad.Inner*
!18 = !{%struct.good.Right zeroinitializer, i32 1}  ; %struct.good.Right*
!19 = !{%struct.bad.Right zeroinitializer, i32 1}  ; %struct.bad.Right*
!20 = !{%struct.good.Left zeroinitializer, i32 1}  ; %struct.good.Left*
!21 = !{%struct.bad.Left zeroinitializer, i32 1}  ; %struct.bad.Left*
!22 = !{%struct.good.Right zeroinitializer, i32 2}  ; %struct.good.Right**
!23 = distinct !{!5, !5}
!24 = !{%struct.badsize.S1 zeroinitializer, i32 1}  ; %struct.badsize.S1*
!25 = !{%struct.badsize.S2 zeroinitializer, i32 1}  ; %struct.badsize.S2*
!26 = !{%struct.badsize.S3 zeroinitializer, i32 1}  ; %struct.badsize.S3*
!27 = !{%struct.badsize.S4 zeroinitializer, i32 1}  ; %struct.badsize.S4*
!28 = !{!29, i32 1}  ; [0 x %struct.badsize.S5]*
!29 = !{!"A", i32 0, !30}  ; [0 x %struct.badsize.S5]
!30 = !{%struct.badsize.S5 zeroinitializer, i32 0}  ; %struct.badsize.S5
!31 = !{%struct.good.S1 zeroinitializer, i32 2}  ; %struct.good.S1**
!32 = !{%struct.badsize.S6 zeroinitializer, i32 1}  ; %struct.badsize.S6*
!33 = !{%struct.badsize.S7 zeroinitializer, i32 1}  ; %struct.badsize.S7*
!34 = !{i8 0, i32 1}  ; i8*
!35 = distinct !{!34}
!36 = !{!"S", %struct.good.S1 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!37 = !{!"S", %struct.bad.S1 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!38 = !{!"S", %struct.good.S2 zeroinitializer, i32 2, !2, !3} ; { [32 x i32], %struct.good.S1 }
!39 = !{!"S", %struct.bad.S2 zeroinitializer, i32 2, !2, !4} ; { [32 x i32], %struct.bad.S1 }
!40 = !{!"S", %struct.good.Inner zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!41 = !{!"S", %struct.bad.Inner zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!42 = !{!"S", %struct.good.Middle zeroinitializer, i32 2, !9, !1} ; { %struct.good.Inner, i32 }
!43 = !{!"S", %struct.bad.Middle zeroinitializer, i32 2, !10, !1} ; { %struct.bad.Inner, i32 }
!44 = !{!"S", %struct.good.Outer zeroinitializer, i32 2, !11, !1} ; { %struct.good.Middle, i32 }
!45 = !{!"S", %struct.bad.Outer zeroinitializer, i32 2, !12, !1} ; { %struct.bad.Middle, i32 }
!46 = !{!"S", %struct.bad.Outer2 zeroinitializer, i32 2, !1, !10} ; { i32, %struct.bad.Inner }
!47 = !{!"S", %struct.good.Left zeroinitializer, i32 2, !18, !1} ; { %struct.good.Right*, i32 }
!48 = !{!"S", %struct.bad.Left zeroinitializer, i32 2, !19, !1} ; { %struct.bad.Right*, i32 }
!49 = !{!"S", %struct.good.Right zeroinitializer, i32 2, !20, !1} ; { %struct.good.Left*, i32 }
!50 = !{!"S", %struct.bad.Right zeroinitializer, i32 2, !21, !1} ; { %struct.bad.Left*, i32 }
!51 = !{!"S", %struct.badsize.S1 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!52 = !{!"S", %struct.badsize.S2 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!53 = !{!"S", %struct.badsize.S3 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!54 = !{!"S", %struct.badsize.S4 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!55 = !{!"S", %struct.badsize.S5 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!56 = !{!"S", %struct.badsize.S6 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!57 = !{!"S", %struct.badsize.S7 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57}

