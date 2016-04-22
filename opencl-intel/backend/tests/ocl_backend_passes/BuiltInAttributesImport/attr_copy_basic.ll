; RUN: opt -runtimelib=%S/attr_copy_basic.rtl -builtin-attr-import -verify %s -S | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknonw-unknown"

define spir_func float @test_copy_attr_basic(float %x) {
  %1 = tail call spir_func float @_Z3cosf(float %x)
  ret float %1
}

declare spir_func float @_Z3cosf(float)
; CHECK: attributes {{.*}} nounwind readnone
