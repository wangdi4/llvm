; RUN: llvm-as -opaque-pointers=0 %s -o %t-typed.bc
; RUN: llvm-as -opaque-pointers=1 %S/Inputs/opaque-pointers.ll -o %t-opaque.bc
; INTEL_CUSTOMIZATION
; The test is modified by swapping the order of the input files to llvm-lto2,
; because commit '33e1af65481ef' has temporarily reenabled opaque pointer
; mode detection based on the first pointer read by the bitcode reader, and
; the command line option to force opaque pointer mode in llvm-lto2 is no
; longer supported. By passing the opaque pointer input file first, we
; guarantee use of opaque pointer mode, which is the expected behavior for
; this test.
; RUN: llvm-lto2 run -o %t-lto.bc %t-opaque.bc %t-typed.bc -save-temps \
; RUN:     -r %t-typed.bc,call_foo,px -r %t-typed.bc,foo,l \
; RUN:     -r %t-opaque.bc,foo,px
; RUN: opt -S -o - %t-lto.bc.0.4.opt.bc | FileCheck %s
; end INTEL_CUSTOMIZATION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare i64 @foo(i64* %p);

define i64 @call_foo(i64* %p) {
  ; CHECK-LABEL: define i64 @call_foo(ptr nocapture readonly %p) local_unnamed_addr #0 {
  ; CHECK-NEXT: %t.i = load i64, ptr %p, align 8
  %t = call i64 @foo(i64* %p)
  ret i64 %t
}
