; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s

declare void @llvm.x86.mic.fence() nounwind
declare void @llvm.x86.mic.lfence() nounwind
declare void @llvm.x86.mic.sfence() nounwind

define i64 @mem_fence(i64 %flags) nounwind {
entry:
; CHECK: mem_fence
  tail call void @llvm.x86.mic.fence() nounwind
  ret i64 %flags
}

define i64 @mem_lfence(i64 %flags) nounwind {
entry:
; CHECK: mem_lfence
  tail call void @llvm.x86.mic.lfence() nounwind
  ret i64 %flags
}

define i64 @mem_sfence(i64 %flags) nounwind {
entry:
; CHECK: mem_sfence
  tail call void @llvm.x86.mic.sfence() nounwind
  ret i64 %flags
}

