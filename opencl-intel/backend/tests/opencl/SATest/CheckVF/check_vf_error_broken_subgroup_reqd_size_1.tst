; RUN: SATest -BUILD --enable-subgroup-emu=false --vectorizer-type=vpo --native-subgroups --config=%s.cfg |& FileCheck %s --check-prefix=LOG
; LOG: error: kernel "test": Required subgroup size can't be 1 for subgroup calls
; LOG: CompilerException Checking vectorization factor failed
