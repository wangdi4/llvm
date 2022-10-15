; REQUIRES: asserts
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Following the change to perform constant folding on GEP.x.0.0 to eliminate GEP
; operators, analysis of the effective types is complicated when a PHI or select
; node input is a global variable that is an aggregate type. The resolved type for
; the global could represent the type of the global variable or element zero
; within it. Adding both types as the resolved type can result in safety flags
; or uses of the merged variable not being able to be analyzed. Instead, we will
; use the context of the other values in the SelectInst/PHINode to resolve
; whether to use the type of the global variable, or the element zero type.

; This test is similar to ptrtype-globalstruct-gep01, but uses nested arrays.

@GetPageGeometry.PageSizes = internal global [76 x [2 x ptr]] zeroinitializer, !intel_dtrans_type !1

; Test a SelectInst using a global variable that is meant to be used as the
; inner zero type.
define void @test1() {
; CHECK-LABEL: define void @test1()
  %i4 = getelementptr inbounds [76 x [2 x ptr]], ptr @GetPageGeometry.PageSizes, i64 0, i64 1
  %i10 = select i1 undef, ptr %i4, ptr @GetPageGeometry.PageSizes
; CHECK: %i10 = select i1 undef, ptr %i4, ptr @GetPageGeometry.PageSizes
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        [2 x i8*]*{{ *}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        [76 x [2 x i8*]] @ 0
; CHECK-NEXT:        [76 x [2 x i8*]] @ 1

  %i26 = getelementptr inbounds [2 x ptr], ptr %i10, i64 0, i64 1
  %i27 = load ptr, ptr %i26, align 8
  %i28 = icmp eq ptr %i27, null
  ret void
}

; Test a SelectInst using a global variable that is meant to be used as the
; type stored within the inner zero type.
define void @test2() {
; CHECK-LABEL: define void @test2()
  %i4 = getelementptr inbounds [76 x [2 x ptr]], ptr @GetPageGeometry.PageSizes, i64 0, i64 1, i64 0
  %i10 = select i1 undef, ptr %i4, ptr @GetPageGeometry.PageSizes
; CHECK: %i10 = select i1 undef, ptr %i4, ptr @GetPageGeometry.PageSizes
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i8**{{ *}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        [2 x i8*] @ 0

  %i27 = load ptr, ptr %i10
  %i28 = icmp eq ptr %i27, null
  ret void
}

; Test a SelectInst using a global variable that is meant to be used as the type
; of the global variable, instead of as the element zero type.
define void @test3() {
; CHECK-LABEL: define void @test3()
  %local = alloca [76 x [2 x ptr]], !intel_dtrans_type !1
  %i10 = select i1 undef, ptr %local, ptr @GetPageGeometry.PageSizes
; CHECK: %i10 = select i1 undef, ptr %local, ptr @GetPageGeometry.PageSizes
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        [76 x [2 x i8*]]*{{ *}}
; CHECK-NEXT:      No element pointees.

  %i26 = getelementptr inbounds [76 x [2 x ptr]], ptr %i10, i64 0, i64 0, i64 1
  %i27 = load ptr, ptr %i26, align 8
  %i28 = icmp eq ptr %i27, null
  ret void
}

; Test a PHINode using a global variable that is meant to be used as the element
; zero type.
define void @test4() {
; CHECK-LABEL: define void @test4()
bb:
  br label %bb8

bb8:
; On this instruction, we want to treat the pointer to the global as being the
; element zero type based on the context because %i4 is type element zero type.
; As a result, we do not want "[76 x [2 x i8*]]*" to be included in the aliased
; types.
  %i10 = phi ptr [ @GetPageGeometry.PageSizes, %bb ], [ %i4, %bb2 ]
; CHECK: %i10 = phi ptr [ @GetPageGeometry.PageSizes, %bb ], [ %i4, %bb2 ]
; CHECK-NEXT:   LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        [2 x i8*]*{{ *}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        [76 x [2 x i8*]] @ 0
; CHECK-NEXT:        [76 x [2 x i8*]] @ UnknownOffset

  %i11 = phi i64 [ 0, %bb ], [ %i3, %bb2 ]
  %i26 = getelementptr inbounds [2 x ptr], ptr %i10, i64 0, i64 1
  %i27 = load ptr, ptr %i26, align 8
  %i28 = icmp eq ptr %i27, null
  br i1 %i28, label %bb2, label %bb140

bb2:
  %i3 = add nuw nsw i64 %i11, 1
  %i4 = getelementptr inbounds [76 x [2 x ptr]], ptr @GetPageGeometry.PageSizes, i64 0, i64 %i3
  %i7 = icmp eq i64 %i3, 75
  br i1 %i7, label %bb140, label %bb8

bb140:
  ret void
}

!1 = !{!"A", i32 76, !2}  ; [76 x [2 x i8*]]
!2 = !{!"A", i32 2, !3}  ; [2 x i8*]
!3 = !{i8 0, i32 1}  ; i8*

!intel.dtrans.types = !{}
