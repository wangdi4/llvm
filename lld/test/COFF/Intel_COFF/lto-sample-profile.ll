; This test checks that the option /lto-sample-profile:file
; runs the sample profile passes of the LTO compile step to
; generate the ProfileSummary metadata.

; RUN: llvm-as %s -o %t.bc
; RUN: lld-link /lldsavetemps /subsystem:console /entry:add /lto-sample-profile:'%p/Inputs/lto-sample-profile.prof' %t.bc /out:%t.exe
; RUN: opt -S %t.exe.0.4.opt.bc | FileCheck %s

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

; CHECK: ![[#]] = !{i32 1, !"ProfileSummary", ![[#]]}

define i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}
