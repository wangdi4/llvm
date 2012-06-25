; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN: | FileCheck %s

declare i32 @llvm.x86.mic.kortestc(i16, i16) nounwind readnone

define i32 @_Z3allDv16_l(i16 %x) nounwind readnone {
; CHECK: kortest   %k0, %k0
; CHECK: setb      %dl
; CHECK: orl       %edx, %edx
; CHECK: sete      %al
; CHECK: movzbl    %al, %eax
  %1 = tail call i32 @llvm.x86.mic.kortestc(i16 %x, i16 %x) nounwind
  %2 = tail call i32 @llvm.x86.mic.kortestc(i16 %x, i16 %x) nounwind
  %3 = or i32 %2, %1
  %4 = icmp eq i32 %3, 0
  %5 = zext i1 %4 to i32
  ret i32 %5
}

define i32 @B(i16 %x) nounwind readnone {
; CHECK: kortest   %k0, %k0
; CHECK: setb      %dl
; CHECK: addl      %edx, %edx
; CHECK: sete      %al
; CHECK: movzbl    %al, %eax
  %1 = tail call i32 @llvm.x86.mic.kortestc(i16 %x, i16 %x) nounwind
  %2 = tail call i32 @llvm.x86.mic.kortestc(i16 %x, i16 %x) nounwind
  %3 = add i32 %2, %1
  %4 = icmp eq i32 %3, 0
  %5 = zext i1 %4 to i32
  ret i32 %5
}

define i32 @AND(i16 %x) nounwind readnone {
; CHECK: kortest   %k0, %k0
; CHECK: setb      %dl
; CHECK: testl      %edx, %edx
; CHECK: sete      %al
; CHECK: movzbl    %al, %eax
  %1 = tail call i32 @llvm.x86.mic.kortestc(i16 %x, i16 %x) nounwind
  %2 = tail call i32 @llvm.x86.mic.kortestc(i16 %x, i16 %x) nounwind
  %3 = and i32 %2, %1
  %4 = icmp eq i32 %3, 0
  %5 = zext i1 %4 to i32
  ret i32 %5
}

