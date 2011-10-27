; RUN: llc -mcpu=sandybridge -relocation-model=static < %p/../../llvm/CodeGen/X86/asm-global-imm.ll | \
; RUN:   grep {test1 \$_GV}
; RUN: llc -mcpu=sandybridge -relocation-model=static < %p/../../llvm/CodeGen/X86/asm-global-imm.ll | \
; RUN:   grep {test2 _GV}
; PR882
