; REQUIRES: x86

; RUN: llvm-as %s -o %t.o
; RUN: llvm-mc -triple=x86_64-pc-linux %p/Inputs/absolute.s -o %t2.o -filetype=obj
; INTEL_CUSTOMIZATION
; RUN: ld.lld -plugin-opt=opaque-pointers %t.o %t2.o -o %t3.out -pie
; end INTEL_CUSTOMIZATION

; RUN: echo "blah = 0xdeadfeef;" > %t.script
; INTEL_CUSTOMIZATION
; RUN: ld.lld -plugin-opt=opaque-pointers %t.o -T%t.script -o %t4.out -pie
; end INTEL_CUSTOMIZATION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@blah = external global i8, align 1

define ptr @_start() {
 ret ptr @blah
}
