; REQUIRES: asserts
; Test that checks if value names are preserved in linker when
; -plugin-opt=fintel-preserve-value-names is provided and if 
; value names are discarded otherwise

; RUN: llvm-as %s -o %t.bc
; RUN: %gold -m elf_x86_64  -plugin %llvmshlibdir/icx-lto%shlibext \
; RUN:    -plugin-opt=O3 \
; RUN:    -plugin-opt=new-pass-manager \
; RUN:    -plugin-opt=fintel-preserve-value-names \
; RUN:    -plugin-opt=-print-after-all  \
; RUN:    %t.bc -o %t \
; RUN:    2>&1 | FileCheck %s --check-prefix=CHECK-PRSV

; Check that value names are preserved
; CHECK-PRSV: %call1 = call i32 @add(i32 %argc)
; CHECK-PRSV: %call2 = call i32 @sub(i32 %call1)
; CHECK-PRSV: ret i32 %call2

; RUN: llvm-as %s -o %t.bc
; RUN: %gold -m elf_x86_64  -plugin %llvmshlibdir/icx-lto%shlibext \
; RUN:    -plugin-opt=O3 \
; RUN:    -plugin-opt=new-pass-manager \
; RUN:    -plugin-opt=-print-after-all  \
; RUN:    %t.bc -o %t \
; RUN:    2>&1 | FileCheck %s

; Check that value names are discarded
; CHECK-NOT: %call1 = call i32 @add(i32 %argc)
; CHECK-NOT: %call2 = call i32 @sub(i32 %call1)
; CHECK-NOT: ret i32 %call2
; CHECK: %3 = call i32 @add(i32 %0)
; CHECK: %4 = call i32 @sub(i32 %3)
; CHECK: ret i32 %4


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}

define internal i32 @sub(i32 %a) {
entry:
  %sub = add nsw i32 %a, -2
  ret i32 %sub
}

define i32 @wmain(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %call1 = call i32 @add(i32 %argc)
  %call2 = call i32 @sub(i32 %call1)
  ret i32 %call2
}
