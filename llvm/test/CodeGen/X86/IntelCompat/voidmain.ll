; RUN: llc < %s -mtriple=i686-pc-linux | FileCheck %s
; RUN: llc < %s -O0 -mtriple=i686-pc-linux | FileCheck %s
; RUN: llc < %s -mtriple=x86_64-pc-linux | FileCheck %s
; RUN: llc < %s -O0 -mtriple=x86_64-pc-linux | FileCheck %s

define void @main() {
; CHECK-LABEL: main:
; CHECK: xorl %eax, %eax
  ret void
}

define void @notmain() {
; CHECK-LABEL: notmain:
; CHECK-NOT: xor
; CHECK: # BB#0:
; CHECK-NEXT: ret
  ret void
}
