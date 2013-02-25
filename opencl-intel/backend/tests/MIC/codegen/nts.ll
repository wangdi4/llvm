; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN: | FileCheck %s

define void @Triad(<16 x float>* %pBuffer, <16 x float>%val) {
entry:
; CHECK: vmovdqa32 %zmm0, (%rdi){eh}
  store <16 x float> %val, <16 x float> * %pBuffer, align 64, !nontemporal !1

  ret void
}

!1 = metadata !{i32 1}

