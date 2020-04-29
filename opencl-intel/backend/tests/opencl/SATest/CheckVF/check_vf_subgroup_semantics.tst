; RUN: SATest -BUILD --vectorizer-type=vpo --native-subgroups --tsize=1 --config=%s.cfg |& FileCheck %s --check-prefix=LOG
; RUN: SATest -BUILD --vectorizer-type=vpo --native-subgroups --tsize=4 --config=%s.cfg |& FileCheck %s --check-prefix=LOG_1
; RUN: SATest -BUILD --vectorizer-type=vpo --native-subgroups --config=%s.cfg |& FileCheck %s --check-prefix=LOG_2
; LOG: Error in kernel <test> Subgroup calls in scalar kernel or non-inlined subroutine can't be resolved!
; LOG: CompilerException Checking vectorization factor failed
; LOG_1: Warning in kernel <test_1> Has unsupported patterns, can't vectorize.
; LOG_1: Error in kernel <test_1> Subgroup calls in scalar kernel or non-inlined subroutine can't be resolved!
; LOG_1: CompilerException Checking vectorization factor failed
; LOG_2: Error in kernel <test_2> Subgroup calls in scalar kernel or non-inlined subroutine can't be resolved!
; LOG_2: CompilerException Checking vectorization factor failed
