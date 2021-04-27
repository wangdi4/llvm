; RUN: SATest -BUILD --enable-subgroup-emu=false --vectorizer-type=vpo --native-subgroups --tsize=1 --config=%s.cfg |& FileCheck %s --check-prefix=LOG
; RUN: SATest -BUILD --enable-subgroup-emu=false --vectorizer-type=vpo --native-subgroups --config=%s.cfg |& FileCheck %s --check-prefix=LOG_1
; LOG: Error in kernel <test> Subgroup calls in scalar function can't be resolved!
; LOG: CompilerException Checking vectorization factor failed
; LOG_1: Error in kernel <test_1> Subgroup calls in scalar function can't be resolved!
; LOG_1: CompilerException Checking vectorization factor failed
