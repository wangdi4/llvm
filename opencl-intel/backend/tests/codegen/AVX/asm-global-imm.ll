; RUN: llc -mcpu=corei7-avx -relocation-model=static < %p/../../../llvm/test/CodeGen/X86/asm-global-imm.ll | \
; RUN:   grep {test1 \$_GV}
; RUN: llc -mcpu=corei7-avx -relocation-model=static < %p/../../../llvm/test/CodeGen/X86/asm-global-imm.ll | \
; RUN:   grep {test2 _GV}
; PR882
