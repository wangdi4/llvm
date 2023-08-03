; RUN: SATest -BUILD --enable-subgroup-emu=false --vectorizer-type=vpo --config=%s.cfg 2>&1 | FileCheck %s --check-prefix=LOG
; LOG: error: kernel "test": Required subgroup size can't be 1 for subgroup calls
; LOG: CompilerException Checking vectorization factor failed
