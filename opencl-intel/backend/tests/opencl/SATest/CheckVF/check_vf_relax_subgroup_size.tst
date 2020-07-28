; RUN: SATest -BUILD --vectorizer-type=vpo --native-subgroups --config=%s.cfg |& FileCheck %s --check-prefix=LOG
; RUN: SATest -BUILD --cpuarch=corei7-avx --vectorizer-type=vpo --native-subgroups --config=%s.cfg |& FileCheck %s --check-prefix=LOG_2
; LOG: Error in kernel <test> _Z13sub_group_alli with vectorization factor 32 is unimplemented!
; LOG: CompilerException Checking vectorization factor failed
; LOG-NOT: Error in kernel <test_1>
; LOG_2-NOT: Error in kernel <test_2>
; LOG_2: Error in kernel <test_3> _Z13sub_group_anyi with vectorization factor 8 is unimplemented!
