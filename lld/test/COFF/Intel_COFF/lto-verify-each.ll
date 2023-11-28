; REQUIRES: x86
; UNSUPPORTED: !system-windows

; RUN: llvm-as %s -o %t.obj
; RUN: lld-link /subsystem:console /entry:main -opt:ltodebugpassmanager -opt:llvm-verify-each %t.obj 2>&1 | FileCheck %s

; This checks that the pass manager instrumentation callbacks for enabling
; execution of the verifier pass between transformations can be enabled
; for the LTO step when using '-opt:llvm-verify-each'

; A snippet of module passes
; CHECK: Verifying module
; CHECK: Running pass: GlobalDCEPass on [module]
; CHECK: Verifying module
; CHECK: Running pass: IntelIPOPrefetchPass on [module]
; CHECK: Verifying module
; CHECK: Running pass: IntelFoldWPIntrinsicPass on [module]
; CHECK: Verifying module

; A snippet of function passes
; CHECK: Verifying function main
; CHECK: Running pass: InstCombinePass on main
; CHECK: Verifying function main
; CHECK: Running pass: SimplifyCFGPass on main
; CHECK: Verifying function main
; CHECK: Running pass: SCCPPass on main
; CHECK: Verifying function main

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30146"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}

define i32 @main(i32 %argc, ptr nocapture readnone %argv) {
entry:
  %call1 = call i32 @add(i32 %argc)
  ret i32 %call1
}
