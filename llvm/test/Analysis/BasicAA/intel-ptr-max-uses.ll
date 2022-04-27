; RUN: opt -basic-aa -xmain-opt-level=3 -aa-eval -print-all-alias-modref-info -disable-output %s 2>&1 | FileCheck %s

; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr12
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr11
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr13
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr16
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr7
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr24
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr2
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr10
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr6
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr15
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr14
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr5
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr21
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr8
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr17
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr1
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr23
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr0
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr20
; CHECK-DAG:   NoAlias:	double* %ptr, double** %ptr.addr
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr18
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr3
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr4
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr9
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr25
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr22
; CHECK-DAG:   NoAlias:	double** %ptr.addr, double* %ptr19

define void @foo(double* %ptr) {
  %ptr.addr = alloca double*, align 8
  store double* %ptr, double** %ptr.addr, align 8

  %ptr0 = load double*, double** %ptr.addr, align 8
  %ptr1 = load double*, double** %ptr.addr, align 8
  %ptr2 = load double*, double** %ptr.addr, align 8
  %ptr3 = load double*, double** %ptr.addr, align 8
  %ptr4 = load double*, double** %ptr.addr, align 8
  %ptr5 = load double*, double** %ptr.addr, align 8
  %ptr6 = load double*, double** %ptr.addr, align 8
  %ptr7 = load double*, double** %ptr.addr, align 8
  %ptr8 = load double*, double** %ptr.addr, align 8
  %ptr9 = load double*, double** %ptr.addr, align 8
  %ptr10 = load double*, double** %ptr.addr, align 8
  %ptr11 = load double*, double** %ptr.addr, align 8
  %ptr12 = load double*, double** %ptr.addr, align 8
  %ptr13 = load double*, double** %ptr.addr, align 8
  %ptr14 = load double*, double** %ptr.addr, align 8
  %ptr15 = load double*, double** %ptr.addr, align 8
  %ptr16 = load double*, double** %ptr.addr, align 8
  %ptr17 = load double*, double** %ptr.addr, align 8
  %ptr18 = load double*, double** %ptr.addr, align 8
  %ptr19 = load double*, double** %ptr.addr, align 8
  %ptr20 = load double*, double** %ptr.addr, align 8
  %ptr21 = load double*, double** %ptr.addr, align 8
  %ptr22 = load double*, double** %ptr.addr, align 8
  %ptr23 = load double*, double** %ptr.addr, align 8
  %ptr24 = load double*, double** %ptr.addr, align 8
  %ptr25 = load double*, double** %ptr.addr, align 8

; dead loads, needed to get aa-eval to trigger
  %ld.ptr12 = load double, double* %ptr12, align 8
  %ld.ptr11 = load double, double* %ptr11, align 8
  %ld.ptr13 = load double, double* %ptr13, align 8
  %ld.ptr16 = load double, double* %ptr16, align 8
  %ld.ptr7 = load double, double* %ptr7, align 8
  %ld.ptr24 = load double, double* %ptr24, align 8
  %ld.ptr2 = load double, double* %ptr2, align 8
  %ld.ptr10 = load double, double* %ptr10, align 8
  %ld.ptr6 = load double, double* %ptr6, align 8
  %ld.ptr15 = load double, double* %ptr15, align 8
  %ld.ptr14 = load double, double* %ptr14, align 8
  %ld.ptr5 = load double, double* %ptr5, align 8
  %ld.ptr21 = load double, double* %ptr21, align 8
  %ld.ptr8 = load double, double* %ptr8, align 8
  %ld.ptr17 = load double, double* %ptr17, align 8
  %ld.ptr1 = load double, double* %ptr1, align 8
  %ld.ptr23 = load double, double* %ptr23, align 8
  %ld.ptr0 = load double, double* %ptr0, align 8
  %ld.ptr20 = load double, double* %ptr20, align 8
  %ld.ptr = load double, double* %ptr, align 8
  %ld.ptr18 = load double, double* %ptr18, align 8
  %ld.ptr3 = load double, double* %ptr3, align 8
  %ld.ptr4 = load double, double* %ptr4, align 8
  %ld.ptr9 = load double, double* %ptr9, align 8
  %ld.ptr25 = load double, double* %ptr25, align 8
  %ld.ptr22 = load double, double* %ptr22, align 8
  %ld.ptr19 = load double, double* %ptr19, align 8

  ret void
}
