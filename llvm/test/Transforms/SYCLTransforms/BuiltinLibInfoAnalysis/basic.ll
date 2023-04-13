; RUN: llvm-as %S/builtin-0.rtl -o %t.0.rtl.bc
; RUN: llvm-as %S/builtin-1.rtl -o %t.1.rtl.bc
; RUN: llvm-as %S/builtin-2.rtl -o %t.2.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.0.rtl.bc,%t.1.rtl.bc,%t.2.rtl.bc -passes='print<sycl-kernel-builtin-info-analysis>' %s -disable-output 2>&1 | FileCheck %s

; Check that BuiltinLibInfoAnalysis loads builtin RTLs successfully.

; CHECK: BuiltinLibInfo: number of builtin runtime libraries is 3

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"
