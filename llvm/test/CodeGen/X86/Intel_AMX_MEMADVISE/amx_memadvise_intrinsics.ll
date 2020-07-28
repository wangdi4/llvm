; REQUIRES: intel_feature_isa_amx_memadvise
; RUN: llc < %s -O0 -mtriple=x86_64-unknown-unknown -mattr=+amx-tile,+amx-memadvise | FileCheck %s

define void @test_amx(i8* %base, i64 %stride){
; CHECK-LABEL: test_amx:
; CHECK:       # %bb.0:
; CHECK:    tmovadvise      $16, (%rdi,%rsi), %tmm5
call void @llvm.x86.tmovadvise.load(i8 5, i8* %base, i64 %stride, i8 16)

; CHECK:    tmovadvise      $32, %tmm7, (%rdi,%rsi)
call void @llvm.x86.tmovadvise.store(i8* %base, i64 %stride, i8 7, i8 32)
ret void
}
declare void @llvm.x86.tmovadvise.load(i8 %tile0, i8* %base, i64 %stride, i8 %imm)
declare void @llvm.x86.tmovadvise.store(i8* %base, i64 %stride, i8 %tile0, i8 %imm)
