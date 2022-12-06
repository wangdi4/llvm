; RUN: opt -mtriple=spir64-- -passes="default<O0>" -paropt=63 -disable-intel-proprietary-opts -debug-pass-manager < %s -o /dev/null 2>&1 | FileCheck %s
; RUN: opt -mtriple=spir64-- -passes="default<O1>" -paropt=63 -disable-intel-proprietary-opts -debug-pass-manager < %s -o /dev/null 2>&1 | FileCheck %s
; RUN: opt -mtriple=spir64-- -passes="default<O2>" -paropt=63 -disable-intel-proprietary-opts -debug-pass-manager < %s -o /dev/null 2>&1 | FileCheck %s
; RUN: opt -mtriple=spir64-- -passes="default<O3>" -paropt=63 -disable-intel-proprietary-opts -debug-pass-manager < %s -o /dev/null 2>&1 | FileCheck %s

; CHECK: Running pass: VPODirectiveCleanupPass on foo (1 instruction)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

define void @foo() #0 {
entry:
  ret void
}

attributes #0 = { "openmp-target-declare"="true" }
