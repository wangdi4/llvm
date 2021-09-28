; RUN: opt -inline -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -passes='cgscc(inline)' -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -inlinereportsetup -inline-report=0xe886 < %s -S | opt -inline -inline-report=0xe886 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Check that caller/callee stack attribute mismatch inhibits inlining

; CHECK-CL-LABEL: define i32 @nossp_caller()
; CHECK-CL: call i32 @ssp()
; CHECK-CL-LABEL: define i32 @ssp_caller()
; CHECK-CL:  call i32 @nossp()
; CHECK-LABEL: COMPILE FUNC: nossp_caller
; CHECK: ssp{{.*}}Caller/callee stack protection mismatch
; CHECK-LABEL: COMPILE FUNC: ssp_caller
; CHECK:  nossp{{.*}}Caller/callee stack protection mismatch
; CHECK-MD-LABEL: define i32 @nossp_caller()
; CHECK-MD: call i32 @ssp()
; CHECK-MD-LABEL: define i32 @ssp_caller()
; CHECK-MD:  call i32 @nossp()

define i32 @ssp() sspstrong { ret i32 42 }

define i32 @nossp() { ret i32 41 }

define i32 @nossp_caller() {
  call i32 @ssp()
  ret i32 0
}

define i32 @ssp_caller() sspstrong {
  call i32 @nossp()
  ret i32 0
}
