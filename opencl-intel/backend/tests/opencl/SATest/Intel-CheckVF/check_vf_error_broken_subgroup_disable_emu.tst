; RUN: SATest -BUILD --enable-subgroup-emu=false --vectorizer-type=vpo --tsize=1 --config=%s.cfg 2>&1 | FileCheck %s --check-prefix=LOG
; LOG: error: kernel "test": Subgroup calls in scalar function can't be resolved
; LOG: CompilerException Checking vectorization factor failed
