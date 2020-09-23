; RUN: SATest -BUILD --vectorizer-type=vpo --config=%s.cfg -tsize=0 --dump-llvm-file - | FileCheck %s

; CHECK: call void asm sideeffect "nop\0A", ""()
