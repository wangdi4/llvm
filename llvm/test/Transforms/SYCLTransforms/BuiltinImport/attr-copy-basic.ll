; RUN: llvm-as %s.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-builtin-import %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-builtin-import %s -S | FileCheck %s

; Check that linker inside sycl-kernel-builtin-import pass imports function attributes.

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknonw-unknown"

define spir_func float @test_copy_attr_basic(float %x) {
  %1 = tail call spir_func float @_Z3cosf(float %x)
  ret float %1
}

declare spir_func float @_Z3cosf(float)
; CHECK: define {{.*}} @_Z3cosf
; CHECK: attributes {{.*}} nounwind memory(none)

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _Z3cosf --  ret float %x
; DEBUGIFY-NOT: WARNING
