; RUN: llvm-as %s.1.rtl -o %t.1.rtl.bc
; RUN: llvm-as %s.2.rtl -o %t.2.rtl.bc

; RUN: opt -sycl-kernel-builtin-lib=%t.1.rtl.bc,%t.2.rtl.bc -passes=sycl-kernel-builtin-import %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.1.rtl.bc,%t.2.rtl.bc -passes=sycl-kernel-builtin-import %s -S | FileCheck %s --check-prefixes=CHECK,CHECK1

; RUN: opt -sycl-kernel-builtin-lib=%t.2.rtl.bc,%t.1.rtl.bc -passes=sycl-kernel-builtin-import %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.2.rtl.bc,%t.1.rtl.bc -passes=sycl-kernel-builtin-import %s -S | FileCheck %s --check-prefixes=CHECK,CHECK2

; Checks that the order of builtin libs matters -- the function defined in a
; precedent module will be imported of a higher priority.

; CHECK: define {{.*}}@_Z4acosDh
; CHECK-NEXT: entry:

; The definition in 1.rtl is imported.
; CHECK1-NEXT: ret half %x

; The definition in 2.rtl is imported.
; CHECK2-NEXT: %y = call half @__ocl_svml_z1_acoss(half %x)
; CHECK2-NEXT: ret half %y

define half @kernel_function(half %x) nounwind {
entry:
  %call01 = call half @_Z4acosDh(half %x)
  ret half %call01
}

declare half @_Z4acosDh(half %x)

; DEBUGIFY-NOT: WARNING{{.*}}kernel_function
