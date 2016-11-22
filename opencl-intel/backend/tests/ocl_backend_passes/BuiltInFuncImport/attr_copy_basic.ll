; RUN: llvm-as %S/attr_copy_basic.rtl -o %t.attr_copy_basic.rtl.bc
; RUN: opt -runtimelib=%t.attr_copy_basic.rtl.bc -builtin-import -verify %s -S | FileCheck %s

; ensure that linker inside builtin-import pass imports function attributes

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknonw-unknown"

define spir_func float @test_copy_attr_basic(float %x) {
  %1 = tail call spir_func float @_Z3cosf(float %x)
  ret float %1
}

declare spir_func float @_Z3cosf(float)
; CHECK: define {{.*}} @_Z3cosf
; CHECK: attributes {{.*}} nounwind readnone
