; RUN: opt -enable-new-pm=0 -mtriple=spir64-- -O0 -paropt=63 -disable-intel-proprietary-opts -debug-pass=Structure < %s -o /dev/null 2>&1 | FileCheck %s
; RUN: opt -enable-new-pm=0 -mtriple=spir64-- -O1 -paropt=63 -disable-intel-proprietary-opts -debug-pass=Structure < %s -o /dev/null 2>&1 | FileCheck %s
; RUN: opt -enable-new-pm=0 -mtriple=spir64-- -O2 -paropt=63 -disable-intel-proprietary-opts -debug-pass=Structure < %s -o /dev/null 2>&1 | FileCheck %s
; RUN: opt -enable-new-pm=0 -mtriple=spir64-- -O3 -paropt=63 -disable-intel-proprietary-opts -debug-pass=Structure < %s -o /dev/null 2>&1 | FileCheck %s

; CHECK-LABEL: Pass Arguments
; CHECK: VPO Directive Cleanup

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"
