; RUN: opt -basic-aa -xmain-opt-level=3 -aa-eval -print-all-alias-modref-info -disable-output %s 2>&1 | FileCheck %s

; CHECK-DAG: NoAlias:      double* %ptr, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr0, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr1, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr2, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr3, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr4, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr5, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr6, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr7, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr8, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr9, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr10, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr11, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr12, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr13, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr14, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr15, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr16, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr17, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr18, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr19, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr20, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr21, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr22, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr23, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr24, double** %ptr.addr
; CHECK-DAG: NoAlias:      double* %ptr25, double** %ptr.addr

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

  ret void
}
