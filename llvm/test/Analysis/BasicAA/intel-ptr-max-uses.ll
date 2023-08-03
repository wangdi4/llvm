; RUN: opt -aa-pipeline="basic-aa" -passes="aa-eval" -xmain-opt-level=3 -print-all-alias-modref-info -disable-output %s 2>&1 | FileCheck %s

; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr12
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr11
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr13
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr16
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr7
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr24
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr2
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr10
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr6
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr15
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr14
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr5
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr21
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr8
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr17
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr1
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr23
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr0
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr20
; CHECK-DAG:   NoAlias:	double* %ptr, ptr* %ptr.addr
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr18
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr3
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr4
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr9
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr25
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr22
; CHECK-DAG:   NoAlias:	ptr* %ptr.addr, double* %ptr19

define void @foo(ptr %ptr) {
  %ptr.addr = alloca ptr, align 8
  store ptr %ptr, ptr %ptr.addr, align 8

  %ptr0 = load ptr, ptr %ptr.addr, align 8
  %ptr1 = load ptr, ptr %ptr.addr, align 8
  %ptr2 = load ptr, ptr %ptr.addr, align 8
  %ptr3 = load ptr, ptr %ptr.addr, align 8
  %ptr4 = load ptr, ptr %ptr.addr, align 8
  %ptr5 = load ptr, ptr %ptr.addr, align 8
  %ptr6 = load ptr, ptr %ptr.addr, align 8
  %ptr7 = load ptr, ptr %ptr.addr, align 8
  %ptr8 = load ptr, ptr %ptr.addr, align 8
  %ptr9 = load ptr, ptr %ptr.addr, align 8
  %ptr10 = load ptr, ptr %ptr.addr, align 8
  %ptr11 = load ptr, ptr %ptr.addr, align 8
  %ptr12 = load ptr, ptr %ptr.addr, align 8
  %ptr13 = load ptr, ptr %ptr.addr, align 8
  %ptr14 = load ptr, ptr %ptr.addr, align 8
  %ptr15 = load ptr, ptr %ptr.addr, align 8
  %ptr16 = load ptr, ptr %ptr.addr, align 8
  %ptr17 = load ptr, ptr %ptr.addr, align 8
  %ptr18 = load ptr, ptr %ptr.addr, align 8
  %ptr19 = load ptr, ptr %ptr.addr, align 8
  %ptr20 = load ptr, ptr %ptr.addr, align 8
  %ptr21 = load ptr, ptr %ptr.addr, align 8
  %ptr22 = load ptr, ptr %ptr.addr, align 8
  %ptr23 = load ptr, ptr %ptr.addr, align 8
  %ptr24 = load ptr, ptr %ptr.addr, align 8
  %ptr25 = load ptr, ptr %ptr.addr, align 8

; dead loads, needed to get aa-eval to trigger
  %ld.ptr12 = load double, ptr %ptr12, align 8
  %ld.ptr11 = load double, ptr %ptr11, align 8
  %ld.ptr13 = load double, ptr %ptr13, align 8
  %ld.ptr16 = load double, ptr %ptr16, align 8
  %ld.ptr7 = load double, ptr %ptr7, align 8
  %ld.ptr24 = load double, ptr %ptr24, align 8
  %ld.ptr2 = load double, ptr %ptr2, align 8
  %ld.ptr10 = load double, ptr %ptr10, align 8
  %ld.ptr6 = load double, ptr %ptr6, align 8
  %ld.ptr15 = load double, ptr %ptr15, align 8
  %ld.ptr14 = load double, ptr %ptr14, align 8
  %ld.ptr5 = load double, ptr %ptr5, align 8
  %ld.ptr21 = load double, ptr %ptr21, align 8
  %ld.ptr8 = load double, ptr %ptr8, align 8
  %ld.ptr17 = load double, ptr %ptr17, align 8
  %ld.ptr1 = load double, ptr %ptr1, align 8
  %ld.ptr23 = load double, ptr %ptr23, align 8
  %ld.ptr0 = load double, ptr %ptr0, align 8
  %ld.ptr20 = load double, ptr %ptr20, align 8
  %ld.ptr = load double, ptr %ptr, align 8
  %ld.ptr18 = load double, ptr %ptr18, align 8
  %ld.ptr3 = load double, ptr %ptr3, align 8
  %ld.ptr4 = load double, ptr %ptr4, align 8
  %ld.ptr9 = load double, ptr %ptr9, align 8
  %ld.ptr25 = load double, ptr %ptr25, align 8
  %ld.ptr22 = load double, ptr %ptr22, align 8
  %ld.ptr19 = load double, ptr %ptr19, align 8

  ret void
}
